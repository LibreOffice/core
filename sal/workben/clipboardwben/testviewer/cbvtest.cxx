// TestWin32.cpp : Definiert den Einsprungpunkt für die Anwendung.
//

#define _WIN32_DCOM

#include "stdafx.h"

#include <windows.h>

#include <ole2.h>
#include <objidl.h>
#include <objbase.h>
#include <process.h>
#include <olectl.h>
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>
#include <..\..\inc\systools\win32\MtaOleClipb.h>

#include "resource.h"

#define MAX_LOADSTRING 100

// Globale Variablen:
HINSTANCE           g_hInst;                        // aktuelle Instanz
HWND                g_hwndMain;
WCHAR               szTitle[MAX_LOADSTRING];            // Text der Titelzeile
WCHAR               szWindowClass[MAX_LOADSTRING];  // Text der Titelzeile
LPSTREAM            g_pStm    = NULL;
char*               pTextBuff = NULL;
DWORD               lData     = 0;
UINT                g_nCBChanges = 0;

// forward declaration
ATOM  MyRegisterClass( HINSTANCE hInstance );
BOOL  InitInstance( HINSTANCE, int );
HMENU GetSubMenuHandle( HWND hwndParent, UINT uiTopLevelIndex );
BOOL  IsClipboardViewer( HWND hwndParent );
void  SwitchMenuState( HWND hwndParent );
void  RegisterClipboardViewer( BOOL bRegister );
void ShowAvailableClipboardFormats( HWND hWnd, HDC hdc, PAINTSTRUCT ps, RECT rcWnd );
void ClearClipboardContent( HWND hWnd );

void    CALLBACK OnClipboardContentChange( void );
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK About( HWND, UINT, WPARAM, LPARAM );

//----------------------------------------------------
// WinMain
//----------------------------------------------------

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
{
     // ZU ERLEDIGEN: Fügen Sie hier den Code ein.
    MSG     msg;
    HACCEL  hAccelTable;
    HRESULT hr = E_FAIL;

    // it's important to initialize ole
    // in order to use the clipboard
    //hr = OleInitialize( NULL );
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );

    g_hInst = hInstance;

    // Globale Zeichenfolgen initialisieren
    LoadStringW(g_hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(g_hInst, IDC_TESTWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(g_hInst);

    // Initialisierung der Anwendung durchführen:
    if( !InitInstance( g_hInst, nCmdShow ) )
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TESTWIN32);

    // Hauptnachrichtenschleife:
    while( GetMessage(&msg, NULL, 0, 0) )
    {
        if( !TranslateAccelerator (msg.hwnd, hAccelTable, &msg) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    // uninitializing the ole libraries
    //OleUninitialize( );
    CoUninitialize( );

    return msg.wParam;
}



//
//  FUNKTION: MyRegisterClass()
//
//  AUFGABE: Registriert die Fensterklasse.
//
//  KOMMENTARE:
//
//    Diese Funktion und ihre Verwendung sind nur notwendig, wenn dieser Code
//    mit Win32-Systemen vor der 'RegisterClassEx'-Funktion kompatibel sein soll,
//    die zu Windows 95 hinzugefügt wurde. Es ist wichtig diese Funktion aufzurufen,
//    damit der Anwendung kleine Symbole mit den richtigen Proportionen zugewiesen
//    werden.
//
ATOM MyRegisterClass( HINSTANCE hInstance )
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_TESTWIN32);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = (LPCWSTR)IDC_TESTWIN32;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassExW(&wcex);
}

//
//   FUNKTION: InitInstance(HANDLE, int)
//
//   AUFGABE: Speichert die Instanzzugriffsnummer und erstellt das Hauptfenster
//
//   KOMMENTARE:
//
//        In dieser Funktion wird die Instanzzugriffsnummer in einer globalen Variable
//        gespeichert und das Hauptprogrammfenster erstellt und angezeigt.
//
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
   g_hwndMain = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if( !IsWindow( g_hwndMain ) )
   {
      return FALSE;
   }

   ShowWindow( g_hwndMain, nCmdShow );
   UpdateWindow( g_hwndMain );

   return TRUE;
}

//
//  FUNKTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  AUFGABE:  Verarbeitet Nachrichten für das Hauptfenster.
//
//  WM_COMMAND  - Anwendungsmenü verarbeiten
//  WM_PAINT    - Hauptfenster darstellen
//  WM_DESTROY  - Beendigungsnachricht ausgeben und zurückkehren
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int         wmId;
    int         wmEvent;
    PAINTSTRUCT ps;
    HDC         hdc;
    WCHAR       szHello[MAX_LOADSTRING];


    LoadStringW(g_hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

    switch( message )
    {
    case WM_CREATE:
        ClearClipboardContent( hWnd );
        break;

        case WM_COMMAND:
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // Menüauswahlen analysieren:
            switch( wmId )
            {
                case IDD_CBVIEWER:
                    SwitchMenuState( hWnd );
                    RegisterClipboardViewer( IsClipboardViewer( hWnd ) );
                    break;

                case IDM_EXIT:
                   DestroyWindow( hWnd );
                   break;

                default:
                   return DefWindowProc( hWnd, message, wParam, lParam );
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint (hWnd, &ps);
            // ZU ERLEDIGEN: Hier beliebigen Code zum Zeichnen hinzufügen...
            RECT rt;
            GetClientRect( hWnd, &rt );

            if ( IsClipboardViewer( g_hwndMain ) )
            {
                ShowAvailableClipboardFormats( hWnd, hdc, ps, rt );
            }
            else
            {
                WCHAR wcString[MAX_LOADSTRING];
                LoadStringW(g_hInst, IDS_MSG_CBVIEWER_IS_OFF, wcString, MAX_LOADSTRING);
                DrawTextW( hdc, wcString, wcslen( wcString ), &rt, DT_CENTER );
            }

            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}

HMENU GetSubMenuHandle( HWND hwndParent, UINT uiTopLevelIndex )
{
    HMENU hMenuMain = GetMenu( hwndParent );
    _ASSERT( IsMenu( hMenu ) );

    HMENU hSubMenu = GetSubMenu( hMenuMain, uiTopLevelIndex );
    _ASSERT( IsMenu( hSubMenu ) );

    return hSubMenu;
}

BOOL IsClipboardViewer( HWND hwndParent )
{
    HMENU hSubMenu = GetSubMenuHandle( hwndParent, 0 );
    UINT uiMState = GetMenuState( hSubMenu, 0, MF_BYPOSITION );
    return ( MF_CHECKED == uiMState );
}

void SwitchMenuState( HWND hwndParent )
{
    HMENU hSubMenu = GetSubMenuHandle( hwndParent, 0 );
    WCHAR wcMenuString[MAX_LOADSTRING];

    if ( IsClipboardViewer( hwndParent ) )
    {
        LoadStringW(g_hInst, IDS_CBVIEWER_OFF, wcMenuString, MAX_LOADSTRING);
        ModifyMenuW( hSubMenu, 0, MF_BYPOSITION | MF_STRING, IDD_CBVIEWER, wcMenuString );
        CheckMenuItem( hSubMenu, 0, MF_BYPOSITION | MF_UNCHECKED );
    }
    else
    {
        LoadStringW(g_hInst, IDS_CBVIEWER_ON, wcMenuString, MAX_LOADSTRING);
        ModifyMenuW( hSubMenu, 0, MF_BYPOSITION | MF_STRING, IDD_CBVIEWER, wcMenuString );
        CheckMenuItem( hSubMenu, 0, MF_BYPOSITION | MF_CHECKED );
    }
}

void RegisterClipboardViewer( BOOL bRegister )
{
    if ( bRegister )
        MTARegisterClipboardViewer( OnClipboardContentChange );
    else // unregister
        MTARegisterClipboardViewer( NULL );

    InvalidateRect( g_hwndMain, NULL, TRUE );
    UpdateWindow( g_hwndMain );
}

void ShowAvailableClipboardFormats( HWND hWnd, HDC hdc, PAINTSTRUCT ps, RECT rcWnd )
{
    if ( !OpenClipboard( hWnd ) )
    {
        WCHAR szErrMsg[] = { L"Couldn't open the clipboard" };
        DrawTextW( hdc, szErrMsg, wcslen( szErrMsg ), &rcWnd, DT_CENTER );
        return;
    }
    else
    {
        WCHAR szCBChangedMsg[100];

        wsprintfW( szCBChangedMsg, L"Clipboard content changed %d", g_nCBChanges );
        DrawTextW( hdc, szCBChangedMsg, wcslen( szCBChangedMsg ), &rcWnd, DT_CENTER );

        CloseClipboard( );
    }
}

void ClearClipboardContent( HWND hWnd )
{
    if ( OpenClipboard( hWnd ) )
    {
        EmptyClipboard( );
        CloseClipboard( );
    }
}

// clipboard viewer callback function
void CALLBACK OnClipboardContentChange( void )
{
    ++g_nCBChanges;
    InvalidateRect( g_hwndMain, NULL, TRUE );
    UpdateWindow( g_hwndMain );
}

