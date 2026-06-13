#include <windows.h>
#include <gdiplus.h>
#include <shellapi.h>
#include <shlwapi.h>

#pragma comment (lib, "Gdiplus.lib")
#pragma comment (lib, "Shell32.lib")
#pragma comment (lib, "Shlwapi.lib")
#pragma comment (lib, "User32.lib")
#pragma comment (lib, "Gdi32.lib")

using namespace Gdiplus;

#define ID_MENU_YOUTUBE_FIRST 1
#define ID_MENU_YOUTUBE_SECOND 2
#define ID_MENU_MUSIC 3

// Resource IDs (must match those in resource.rc)
#define IDR_ICON_DATA 101
#define IDR_LAGLIFE_DATA 102

// Helper function to load a binary resource into a GDI+ compatible IStream
IStream* LoadResourceToStream(int resourceId, LPCSTR resourceType) {
    HRSRC hResInfo = FindResourceA(NULL, MAKEINTRESOURCEA(resourceId), resourceType);
    if (!hResInfo) return NULL;

    HGLOBAL hResData = LoadResource(NULL, hResInfo);
    if (!hResData) return NULL;

    void* pResourceData = LockResource(hResData);
    DWORD resourceSize = SizeofResource(NULL, hResInfo);
    if (pResourceData == NULL || resourceSize == 0) return NULL;

    // SHCreateMemStream creates a memory copy of the resource data.
    // This ensures the stream is safe to use with GDI+ objects.
    return SHCreateMemStream((const BYTE*)pResourceData, resourceSize);
}

// Step 1: Handle window events (like clicking the close button)
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_COMMAND: {
            switch (LOWORD(wp)) {
                case ID_MENU_YOUTUBE_FIRST:
                    ShellExecuteA(NULL, "open", "https://www.youtube.com/@laglife/videos", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case ID_MENU_YOUTUBE_SECOND:
                    ShellExecuteA(NULL, "open", "https://www.youtube.com/@laglife2/videos", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case ID_MENU_MUSIC:
                    ShellExecuteA(NULL, "open", "https://www.youtube.com/playlist?list=PLRFvftGDNBvhlTgByxp_4S6V52mwfgYuQ", NULL, NULL, SW_SHOWNORMAL);
                    break;
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            Graphics graphics(hdc);
            IStream* pStream = LoadResourceToStream(IDR_LAGLIFE_DATA, (LPCSTR)RT_RCDATA);
            if (pStream) {
                Image image(pStream);
                graphics.DrawImage(&image, 0, 0, rect.right, rect.bottom);
                pStream->Release();
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

// Step 2: Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    const char* CLASS_NAME = "AlwaysShowClass";

    // Register the window class
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    // Load Icon from memory
    IStream* pIconStream = LoadResourceToStream(IDR_ICON_DATA, (LPCSTR)RT_RCDATA);
    if (pIconStream) {
        Bitmap bitmap(pIconStream);
        HICON hIcon;
        if (bitmap.GetHICON(&hIcon) == Ok) {
            wc.hIcon = hIcon;
        }
        pIconStream->Release();
    }

    RegisterClassA(&wc);

    // Create the menu bar and add the "youtube" sub-menu
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();
    AppendMenuA(hSubMenu, MF_STRING, ID_MENU_YOUTUBE_FIRST, "first channel");
    AppendMenuA(hSubMenu, MF_STRING, ID_MENU_YOUTUBE_SECOND, "second channel");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "youtube");
    AppendMenuA(hMenu, MF_STRING, ID_MENU_MUSIC, "music");

    // Step 3: Create the window
    HWND hwnd = CreateWindowExA(
        WS_EX_TOPMOST,                  // Optional extended styles
        CLASS_NAME,                     // Window class name
        "my pet laglife",               // Window title
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // Fixed size style
        GetSystemMetrics(SM_CXSCREEN) - 320, 0, // Position (x, y) at top right
        320, 256,                       // Size (width, height)
        NULL,                           // Parent window
        hMenu,                          // Menu
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Step 4: Show and update the window
    // SW_SHOW guarantees the window pops up and stays on the screen
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Step 5: Run the message loop to keep the window active
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);

    return 0;
}
