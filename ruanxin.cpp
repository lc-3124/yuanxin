#define UNICODE
#include <windows.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

const int TIMER_ID = 1;
const int DELTA = 3; // 每次移动像素点
const wchar_t* IMAGE_PATH = L"test.png";

ULONG_PTR gdiplusToken;
Gdiplus::Image* g_pImage = nullptr;
int imgWidth, imgHeight;
int dx = DELTA, dy = DELTA; // 速度方向

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int x = 100, y = 100;
	switch (message) {
	case WM_CREATE:
		SetTimer(hwnd, TIMER_ID, 10, NULL);
		break;
	case WM_TIMER:
		if (g_pImage) {
			RECT rc;
			GetWindowRect(hwnd, &rc);
			int sw = GetSystemMetrics(SM_CXSCREEN);
			int sh = GetSystemMetrics(SM_CYSCREEN);
			// 判断边界碰撞
			if (x + dx < 0 || x + imgWidth + dx > sw) dx = -dx;
			if (y + dy < 0 || y + imgHeight + dy > sh) dy = -dy;
			x += dx;
			y += dy;
			SetWindowPos(hwnd, HWND_TOPMOST, x, y, imgWidth, imgHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		break;
	case WM_PAINT:
		if (g_pImage) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			Graphics graphics(hdc);
			graphics.SetCompositingMode(CompositingModeSourceOver);
			graphics.DrawImage(g_pImage, 0, 0, imgWidth, imgHeight);
			EndPaint(hwnd, &ps);
		}
		break;
	case WM_DESTROY:
		KillTimer(hwnd, TIMER_ID);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
	// 初始化 GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
	g_pImage = new Gdiplus::Image(IMAGE_PATH);
	imgWidth = g_pImage->GetWidth();
	imgHeight = g_pImage->GetHeight();
	
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"BouncingImage";
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH); // 透明
	RegisterClass(&wc);
	
	HWND hwnd = CreateWindowEx(
							   WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, // Layered支持透明/无任务栏
							   wc.lpszClassName, L"Bouncing Image", 
							   WS_POPUP,
							   100, 100, imgWidth, imgHeight,
							   NULL, NULL, hInstance, NULL);
	
	// 设置窗口透明（仅显示图片内容）
	SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
	
	ShowWindow(hwnd, SW_SHOWNOACTIVATE);
	
	// 消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	delete g_pImage;
	GdiplusShutdown(gdiplusToken);
	return 0;
}
