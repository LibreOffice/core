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
#include <..\..\inc\systools\win32\MtaOleClipb.h>
#include "XTDataObject.hxx"

#include "resource.h"

#define MAX_LOADSTRING 100
#undef USE_MTACB

// Globale Variablen:
HINSTANCE           hInst;                      // aktuelle Instanz
WCHAR               szTitle[MAX_LOADSTRING];            // Text der Titelzeile
WCHAR               szWindowClass[MAX_LOADSTRING];  // Text der Titelzeile
ATOM                MyRegisterClass( HINSTANCE hInstance );
BOOL                InitInstance( HINSTANCE, int );
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK    About( HWND, UINT, WPARAM, LPARAM );
void                CopyClipboardData(HWND hwndParent);

LPSTREAM            g_pStm    = NULL;
char*               pTextBuff = NULL;
DWORD               lData     = 0;
CXTDataObject*      g_xtDo    = NULL;

//----------------------------------------------------
// a thread function
//----------------------------------------------------

unsigned int _stdcall ThreadProc(LPVOID pParam)
{
    IDataObject* pIDataObj = NULL;
    FORMATETC    formatETC;
    STGMEDIUM    stgMedium;
    LPVOID       pGlobMem;
    HWND         hwnd;
    DWORD        sizeGlobBuff;
    HRESULT      hr;

    hwnd = (HWND)pParam;

    OleInitialize( NULL );

    hr = OleGetClipboard( &pIDataObj );

    hr = CoGetInterfaceAndReleaseStream(
        g_pStm,
        __uuidof(IDataObject),
        reinterpret_cast<LPVOID*>(&pIDataObj));

    formatETC.cfFormat = CF_TEXT;
    formatETC.ptd      = NULL;
    formatETC.dwAspect = DVASPECT_CONTENT;
    formatETC.lindex   = -1;
    formatETC.tymed    = TYMED_HGLOBAL;

    hr = pIDataObj->GetData( &formatETC, &stgMedium );
    pGlobMem = GlobalLock( stgMedium.hGlobal );
    if ( NULL != pGlobMem )
    {
        if ( NULL != pTextBuff )
        {
            free( pTextBuff );
        }

        sizeGlobBuff = GlobalSize( stgMedium.hGlobal );
        pTextBuff = (char*)malloc( sizeGlobBuff + 1 );
        ZeroMemory( pTextBuff, sizeGlobBuff + 1 );

        memcpy( pTextBuff, pGlobMem, sizeGlobBuff );
        lData = sizeGlobBuff;

        InvalidateRect( hwnd, NULL, TRUE );
        UpdateWindow( hwnd );
    }

    GlobalUnlock( stgMedium.hGlobal );

    ReleaseStgMedium( &stgMedium );

    pIDataObj->Release();

    //CoUninitialize( );

    OleUninitialize( );

    return 0;
}

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
#ifdef USE_MTACB
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
#else
    hr = OleInitialize( NULL );
#endif


    // Globale Zeichenfolgen initialisieren
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Initialisierung der Anwendung durchführen:
    if( !InitInstance( hInstance, nCmdShow ) )
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
#ifdef USE_MTACB
    CoUninitialize( );
#else
    OleUninitialize( );
#endif

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
   HWND hWnd;

   hInst = hInstance; // Instanzzugriffsnummer in unserer globalen Variable speichern

   hWnd = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if( !hWnd )
   {
      return FALSE;
   }

   ShowWindow( hWnd, nCmdShow );
   UpdateWindow( hWnd );

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
    TCHAR       szHello[MAX_LOADSTRING];


    LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

    switch( message )
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // Menüauswahlen analysieren:
            switch( wmId )
            {
                case IDD_COPY:
                    //PasteClipboardData(hWnd);
                    CopyClipboardData(hWnd);
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

            if ( NULL != pTextBuff )
            {
                DrawText( hdc, pTextBuff, lData, &rt, DT_CENTER );
            }
            else
            {
                DrawText( hdc, szHello, strlen(szHello), &rt, DT_CENTER );
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

void CopyClipboardData( HWND hWnd )
{
    g_xtDo = new CXTDataObject;
#ifdef USE_MTACB
    MTASetClipboard( static_cast< IDataObject* >( g_xtDo ) );
    MTAFlushClipboard( );
#else
    OleSetClipboard( static_cast< IDataObject* >( g_xtDo ) );
    OleFlushClipboard( );
#endif
}

void PasteClipboardData2(HWND hwndParent)
{
    IDataObject* pIDataObject;
    HRESULT      hr;
    FORMATETC    formatETC;
    STGMEDIUM    stgMedium;
    LPVOID       pGlobMem;
    HWND         hwnd;
    DWORD        sizeGlobBuff;

    //hr = MTAGetClipboard( &pIDataObject );
    if ( SUCCEEDED( hr ) )
    {
        formatETC.cfFormat = CF_TEXT;
        formatETC.ptd      = NULL;
        formatETC.dwAspect = DVASPECT_CONTENT;
        formatETC.lindex   = -1;
        formatETC.tymed    = TYMED_HGLOBAL;

        hr = pIDataObject->GetData( &formatETC, &stgMedium );
        pGlobMem = GlobalLock( stgMedium.hGlobal );
        if ( NULL != pGlobMem )
        {
            if ( NULL != pTextBuff )
            {
                free( pTextBuff );
            }

            sizeGlobBuff = GlobalSize( stgMedium.hGlobal );
            pTextBuff = (char*)malloc( sizeGlobBuff + 1 );
            ZeroMemory( pTextBuff, sizeGlobBuff + 1 );

            memcpy( pTextBuff, pGlobMem, sizeGlobBuff );
            lData = sizeGlobBuff;

            InvalidateRect( hwndParent, NULL, TRUE );
            UpdateWindow( hwndParent );
        }

        GlobalUnlock( stgMedium.hGlobal );

        ReleaseStgMedium( &stgMedium );

        pIDataObject->Release();
    }
}


