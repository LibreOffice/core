// QuickStart.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <systools/win32/uwinapi.h>
#include <stdio.h>

#define MY_TASKBAR_NOTIFICATION         WM_USER+1

#define MAX_LOADSTRING 100

// message used to communicate with soffice
#define TERMINATIONVETO_MESSAGE "SO TerminationVeto"
#define TERMINATE_MESSAGE       "SO Terminate"
#define LISTENER_WINDOWCLASS    "SO Listener Class"
#define KILLTRAY_MESSAGE        "SO KillTray"

static  UINT aTerminationVetoMessage = 0x7FFF;
static  UINT aTerminateMessage = 0x7FFF;
static  HMENU popupMenu = NULL;
static  bool bTerminateVeto = true;

#define UPDATE_TIMER   1

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // The title bar text

TCHAR szExitString[MAX_LOADSTRING];
TCHAR szTooltipString[MAX_LOADSTRING];

// Foward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

bool SofficeRuns()
{
    // check for soffice by searching the communication window
    return ( FindWindowEx( NULL, NULL, LISTENER_WINDOWCLASS, NULL ) == NULL ) ? false : true;
}

bool launchSoffice( )
{
    if ( !SofficeRuns() )
    {
        // UINT ret = WinExec( "h:\\office60.630b\\program\\swriter.exe -bean", SW_SHOW );
        char filename[_MAX_PATH + 1];

        filename[_MAX_PATH] = 0;
        GetModuleFileName( NULL, filename, _MAX_PATH ); // soffice resides in the same dir
        char *p = strrchr( filename, '\\' );
        if ( !p )
            return false;

        strncpy( p+1, "soffice.exe", _MAX_PATH - (p+1 - filename) );

        char imagename[_MAX_PATH + 1];

        imagename[_MAX_PATH] = 0;
        _snprintf(imagename, _MAX_PATH, "\"%s\" -quickstart", filename );

        UINT ret = WinExec( imagename, SW_SHOW );
        if ( ret < 32 )
            return false;
/*
        // wait until we can communicate
        int retry = 30;
        while (retry-- && !SofficeRuns() )
            Sleep(1000);

        return SofficeRuns();
        */
        return true;
    }
    else
        return true;
}

void NotifyListener( HWND hWnd )
{
    static HICON hIconActive=NULL;
    //static HICON hIconInActive=NULL;

    if( !hIconActive )
    {
        hIconActive = (HICON)LoadImage( GetModuleHandle( NULL ), MAKEINTRESOURCE( ICON_ACTIVE ),
            IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ),
            LR_DEFAULTCOLOR | LR_SHARED );

/*        hIconInActive = (HICON)LoadImage( GetModuleHandle( NULL ), MAKEINTRESOURCE( ICON_INACTIVE ),
            IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ),
            LR_DEFAULTCOLOR | LR_SHARED );
            */
    }

    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd   = hWnd;
    nid.uID    = IDM_QUICKSTART;
    nid.szTip[elementsof(nid.szTip) - 1] = 0;
//    nid.hIcon = bTerminateVeto ? hIconActive : hIconInActive;
//    strncpy(nid.szTip, bTerminateVeto ? STRING_QUICKSTARTACTIVE : STRING_QUICKSTARTINACTIVE, elementsof(nid.szTip) - 1 );
    nid.hIcon = hIconActive;
    strncpy(nid.szTip, szTooltipString, elementsof(nid.szTip) - 1);
    nid.uFlags = NIF_TIP|NIF_ICON;

    // update systray
    Shell_NotifyIcon( NIM_MODIFY, &nid );
    //CheckMenuItem( popupMenu, IDM_QUICKSTART, bTerminateVeto ? MF_CHECKED : MF_UNCHECKED );

    // notify listener
    SendMessage( HWND_BROADCAST, aTerminationVetoMessage, (WORD) bTerminateVeto, 0L );
}



int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPSTR     /*lpCmdLine*/,
                     int       nCmdShow)
{
    // Look for -killtray argument

    for ( int i = 1; i < __argc; i++ )
    {
        if ( 0 == strcmp( __argv[i], "-killtray" ) )
        {
            HWND    hwndTray = FindWindow( LISTENER_WINDOWCLASS, NULL );

            if ( hwndTray )
            {
                UINT    uMsgKillTray = RegisterWindowMessage( KILLTRAY_MESSAGE );
                SendMessage( hwndTray, uMsgKillTray, 0, 0 );
            }

            return 0;
        }
    }

    launchSoffice();
    return 0;

     // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;
    aTerminationVetoMessage = RegisterWindowMessage( TERMINATIONVETO_MESSAGE );
    aTerminateMessage       = RegisterWindowMessage( TERMINATE_MESSAGE );

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_QUICKSTART, szWindowClass, MAX_LOADSTRING);

    LoadString(hInstance, IDS_EXIT,    szExitString, MAX_LOADSTRING);
    LoadString(hInstance, IDS_TOOLTIP, szTooltipString, MAX_LOADSTRING);

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_QUICKSTART);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_QUICKSTART);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   nCmdShow = SW_HIDE;   // hide main window, we only need the taskbar icon
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            // make sure soffice runs
            if( !launchSoffice() )
                return -1;

            // create popup menu
            popupMenu = CreatePopupMenu();
            static int count=0;

            // Add my items
            MENUITEMINFO mi;
            mi.cbSize = sizeof(MENUITEMINFO);
            mi.fMask=MIIM_TYPE|MIIM_STATE|MIIM_ID;
            mi.fType=MFT_STRING;
            mi.fState=MFS_ENABLED|MFS_DEFAULT;
            mi.wID = IDM_QUICKSTART;
            mi.hSubMenu=NULL;
            mi.hbmpChecked=NULL;
            mi.hbmpUnchecked=NULL;
            mi.dwItemData=NULL;
            mi.dwTypeData = "QuickStart";
            mi.cch = strlen(mi.dwTypeData);
//            InsertMenuItem(popupMenu, count++, TRUE, &mi);

            mi.cbSize = sizeof(MENUITEMINFO);
            mi.fMask=MIIM_TYPE|MIIM_STATE|MIIM_ID;
            mi.fType=MFT_STRING;
            mi.fState=MFS_ENABLED;
            mi.wID = IDM_ABOUT;
            mi.hSubMenu=NULL;
            mi.hbmpChecked=NULL;
            mi.hbmpUnchecked=NULL;
            mi.dwItemData=NULL;
            mi.dwTypeData = "Info...";
            mi.cch = strlen(mi.dwTypeData);
//            InsertMenuItem(popupMenu, count++, TRUE, &mi);

            mi.cbSize = sizeof(MENUITEMINFO);
            mi.fMask=MIIM_TYPE;
            mi.fType=MFT_SEPARATOR;
            mi.hSubMenu=NULL;
            mi.hbmpChecked=NULL;
            mi.hbmpUnchecked=NULL;
            mi.dwItemData=NULL;
//            InsertMenuItem(popupMenu, count++, TRUE, &mi);

            mi.cbSize = sizeof(MENUITEMINFO);
            mi.fMask=MIIM_TYPE|MIIM_STATE|MIIM_ID;
            mi.fType=MFT_STRING;
            mi.fState=MFS_ENABLED;
            mi.wID = IDM_EXIT;
            mi.hSubMenu=NULL;
            mi.hbmpChecked=NULL;
            mi.hbmpUnchecked=NULL;
            mi.dwItemData=NULL;
            mi.dwTypeData = szExitString;
            mi.cch = strlen(mi.dwTypeData);
            InsertMenuItem(popupMenu, count++, TRUE, &mi);

            // add taskbar icon
            NOTIFYICONDATA nid;
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd   = hWnd;
            nid.uID    = IDM_QUICKSTART;
            nid.uFlags = NIF_MESSAGE;
            nid.uCallbackMessage=MY_TASKBAR_NOTIFICATION;
            Shell_NotifyIcon(NIM_ADD, &nid);

            // and update state
            NotifyListener( hWnd );

            // check for soffice
            SetTimer(hWnd, UPDATE_TIMER, 3000, NULL);
        }
        break;

        case MY_TASKBAR_NOTIFICATION: // message from taskbar
            switch(lParam)
            {
/*
                case WM_LBUTTONDBLCLK:
                    bTerminateVeto = bTerminateVeto ? false : true;
                    NotifyListener( hWnd );
                    break;
                    */

                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    SetForegroundWindow( hWnd );
                    int m = TrackPopupMenuEx(popupMenu, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON,
                        pt.x, pt.y, hWnd, NULL);
                    // BUGFIX: See Q135788 (PRB: Menus for Notification Icons Don't Work Correctly)
                    PostMessage(hWnd, NULL, 0, 0);
                    switch(m)
                    {
                    case IDM_QUICKSTART:
                        bTerminateVeto = bTerminateVeto ? false : true;
                        NotifyListener( hWnd );
                        break;
                    case IDM_ABOUT:
                        DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
                        break;
                    case IDM_EXIT:
                        DestroyWindow(hWnd);
                        break;
                    }
                }
                break;
            }
            break;

        case WM_TIMER:
            if( wParam == UPDATE_TIMER )
            {
                // update state
                NotifyListener( hWnd );
            }
            break;

        case WM_DESTROY:
            // try to terminate office
            SendMessage( HWND_BROADCAST, aTerminateMessage, 0, 0L );

            // delete taskbar icon
            NOTIFYICONDATA nid;
            nid.cbSize=sizeof(NOTIFYICONDATA);
            nid.hWnd = hWnd;
            nid.uID = IDM_QUICKSTART;
            Shell_NotifyIcon(NIM_DELETE, &nid);

            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM)
{
    switch (message)
    {
        case WM_INITDIALOG:
                return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}
