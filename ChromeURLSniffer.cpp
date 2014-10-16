// ChromeURLSniffer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ChromeURLSniffer.h"
#include <Windows.h>
#include <Oleacc.h>
HWINEVENTHOOK LHook;
#pragma comment( lib,"Oleacc.lib")

#define MAX_LOADSTRING 100

#include <Stdio.h>

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

VOID CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook,
				DWORD event,
				HWND hwnd,
				LONG idObject,
				LONG idChild,
				DWORD dwEventThread,
				DWORD dwmsEventTime);

//GUI Handlers
HWND	ghWnd = NULL;
HWND	hWndEdit = NULL;

#define WM_UPDATECAREPOS	(WM_USER + 1234)

void Hook() {
	if (LHook != 0) return;
	CoInitialize(NULL);
	LHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_VALUECHANGE, 0, WinEventProc, 0, 0, WINEVENT_SKIPOWNPROCESS);
}

void Unhook() {
	if (LHook == 0) return;
	UnhookWinEvent(LHook);
	CoUninitialize();
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CHROMEURLSNIFFER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_CHROMEURLSNIFFER);

	Hook();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	Unhook();


	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_CHROMEURLSNIFFER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   ghWnd = CreateWindowEx(WS_EX_TOPMOST, szWindowClass, szTitle, WS_CAPTION|WS_SYSMENU|WS_VISIBLE|WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, 640, 128, NULL, NULL, hInstance, NULL);

   if (!ghWnd)
   {
      return FALSE;
   }
   
   //Add Edit text box
   hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT(""),
	   WS_CHILD | WS_VISIBLE, 20, 40, 580,
	   20, ghWnd, NULL, NULL, NULL);

   ShowWindow(ghWnd, nCmdShow);
   UpdateWindow(ghWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{

	case WM_UPDATECAREPOS:
		{
			WCHAR sz[1024];

			LPCWSTR lpszString = (LPCWSTR)lParam;
			_swprintf(sz, L"%s", lpszString);
			
			SetWindowTextW(hWndEdit, sz);

			break;
		}
	case WM_PAINT:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			DrawText(hdc, _T("Open Chrome Browser and observe the text box"), 49, &rect, DT_CENTER|DT_WORDBREAK);
			EndPaint(hWnd, &ps);
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


VOID CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook,
				DWORD event,
				HWND hwnd,
				LONG idObject,
				LONG idChild,
				DWORD dwEventThread,
				DWORD dwmsEventTime)
{
	IAccessible* pAcc = NULL;
	VARIANT varChild;
	HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);
	if ((hr == S_OK) && (pAcc != NULL)) {
		BSTR bstrName, bstrValue;
		
		pAcc->get_accValue(varChild, &bstrValue);
		pAcc->get_accName(varChild, &bstrName);

		char className[50];
		GetClassNameA(hwnd, className, 50);

		if ((strcmp(className, "Chrome_WidgetWin_1") == 0) && (wcscmp(bstrName, L"Address and search bar") == 0)) {
			SendMessage(ghWnd, WM_UPDATECAREPOS, NULL, (WPARAM)(bstrValue));//LPCWSTR
			printf("URL change: %ls\n", bstrValue);
		}
		pAcc->Release();
	}

	return;
}