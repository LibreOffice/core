/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/macros.h>

// necessary to include system headers without warnings
#ifdef _MSC_VER
#pragma warning(disable:4668 4917)
#endif

#include <unotools/moduleoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>

#undef WB_LEFT
#undef WB_RIGHT

#include "shutdownicon.hxx"
#include "app.hrc"
#include <shlobj.h>
#include <objidl.h>
#include <osl/thread.h>
#include <systools/win32/qswin32.h>
#include <comphelper/sequenceashashmap.hxx>

#include <set>

using namespace ::osl;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;


#define EXECUTER_WINDOWCLASS    "SO Executer Class"
#define EXECUTER_WINDOWNAME     "SO Executer Window"


#define ID_QUICKSTART               1
#define IDM_EXIT                    2
#define IDM_OPEN                    3
#define IDM_WRITER                  4
#define IDM_CALC                    5
#define IDM_IMPRESS                 6
#define IDM_DRAW                    7
#define IDM_BASE                    8
#define IDM_TEMPLATE                9
#define IDM_MATH                    12
#define IDM_INSTALL                 10


#define ICON_LO_DEFAULT                 1
#define ICON_TEXT_DOCUMENT              2
#define ICON_SPREADSHEET_DOCUMENT       4
#define ICON_DRAWING_DOCUMENT           6
#define ICON_PRESENTATION_DOCUMENT      8
#define ICON_TEMPLATE                   11
#define ICON_DATABASE_DOCUMENT          12
#define ICON_MATH_DOCUMENT              13
#define ICON_OPEN                       5   // See index of open folder icon in shell32.dll

#define SFX_TASKBAR_NOTIFICATION    WM_USER+1

static HWND  aListenerWindow = nullptr;
static HWND  aExecuterWindow = nullptr;
static HMENU popupMenu = nullptr;

static void OnMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmis);
static void OnDrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpdis);

typedef struct tagMYITEM
{
    OUString text;
    OUString module;
    UINT iconId;
} MYITEM;


static void addMenuItem( HMENU hMenu, UINT id, UINT iconId, const OUString& text, int& pos, bool bOwnerdraw, const OUString& module )
{
    MENUITEMINFOW mi;
    memset( &mi, 0, sizeof( MENUITEMINFOW ) );

    mi.cbSize = sizeof( MENUITEMINFOW );
    if( id == static_cast<UINT>( -1 ) )
    {
        mi.fMask=MIIM_TYPE;
        mi.fType=MFT_SEPARATOR;
    }
    else
    {
        if( bOwnerdraw )
        {
            mi.fMask=MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_DATA;
            mi.fType=MFT_OWNERDRAW;
            mi.fState=MFS_ENABLED;
            mi.wID = id;

            MYITEM *pMyItem = new MYITEM;
            pMyItem->text = text;
            pMyItem->iconId = iconId;
            pMyItem->module = module;
            mi.dwItemData = reinterpret_cast<DWORD_PTR>(pMyItem);
        }
        else
        {
            mi.fMask=MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_DATA;
            mi.fType=MFT_STRING;
            mi.fState=MFS_ENABLED;
            mi.wID = id;
            mi.dwTypeData = reinterpret_cast<wchar_t *>(
                const_cast<sal_Unicode *>(text.getStr()));
            mi.cch = text.getLength();
        }

        if ( IDM_TEMPLATE == id )
            mi.fState |= MFS_DEFAULT;
    }

    InsertMenuItemW( hMenu, pos++, TRUE, &mi );
}


static HMENU createSystrayMenu( )
{
    SvtModuleOptions    aModuleOptions;

    HMENU hMenu = CreatePopupMenu();
    int pos=0;

    ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();
    OSL_ENSURE( pShutdownIcon, "ShutdownIcon instance empty!");

    if( !pShutdownIcon )
        return nullptr;

    // collect the URLs of the entries in the File/New menu
    ::std::set< OUString > aFileNewAppsAvailable;
    SvtDynamicMenuOptions aOpt;
    Sequence < Sequence < PropertyValue > > aNewMenu = aOpt.GetMenu( EDynamicMenuType::NewMenu );
    const OUString sURLKey( "URL"  );

    for ( auto const & newMenuProp : aNewMenu )
    {
        ::comphelper::SequenceAsHashMap aEntryItems( newMenuProp );
        OUString sURL( aEntryItems.getUnpackedValueOrDefault( sURLKey, OUString() ) );
        if ( sURL.getLength() )
            aFileNewAppsAvailable.insert( sURL );
    }

    // describe the menu entries for launching the applications
    struct MenuEntryDescriptor
    {
        SvtModuleOptions::EModule   eModuleIdentifier;
        UINT                        nMenuItemID;
        UINT                        nMenuIconID;
        const char*                 pAsciiURLDescription;
    }   aMenuItems[] =
    {
        { SvtModuleOptions::EModule::WRITER,    IDM_WRITER, ICON_TEXT_DOCUMENT,         WRITER_URL },
        { SvtModuleOptions::EModule::CALC,      IDM_CALC,   ICON_SPREADSHEET_DOCUMENT,  CALC_URL },
        { SvtModuleOptions::EModule::IMPRESS,   IDM_IMPRESS,ICON_PRESENTATION_DOCUMENT, IMPRESS_WIZARD_URL },
        { SvtModuleOptions::EModule::DRAW,      IDM_DRAW,   ICON_DRAWING_DOCUMENT,      DRAW_URL },
        { SvtModuleOptions::EModule::DATABASE,  IDM_BASE,   ICON_DATABASE_DOCUMENT,     BASE_URL },
        { SvtModuleOptions::EModule::MATH,      IDM_MATH,   ICON_MATH_DOCUMENT,         MATH_URL },
    };

    // insert the menu entries for launching the applications
    for ( size_t i = 0; i < SAL_N_ELEMENTS(aMenuItems); ++i )
    {
        if ( !aModuleOptions.IsModuleInstalled( aMenuItems[i].eModuleIdentifier ) )
            // the complete application is not even installed
            continue;

        OUString sURL( OUString::createFromAscii( aMenuItems[i].pAsciiURLDescription ) );

        if ( aFileNewAppsAvailable.find( sURL ) == aFileNewAppsAvailable.end() )
            // the application is installed, but the entry has been configured to *not* appear in the File/New
            // menu => also let not appear it in the quickstarter
            continue;

        addMenuItem( hMenu, aMenuItems[i].nMenuItemID, aMenuItems[i].nMenuIconID,
            ShutdownIcon::GetUrlDescription( sURL ), pos, true, "" );
    }


    // insert the remaining menu entries
    addMenuItem( hMenu, IDM_TEMPLATE, ICON_TEMPLATE,
        pShutdownIcon->GetResString( STR_QUICKSTART_FROMTEMPLATE ), pos, true, "");
    addMenuItem( hMenu, static_cast< UINT >( -1 ), 0, OUString(), pos, false, "" );
    addMenuItem( hMenu, IDM_OPEN,   ICON_OPEN, pShutdownIcon->GetResString( STR_QUICKSTART_FILEOPEN ), pos, true, "SHELL32");
    addMenuItem( hMenu, static_cast< UINT >( -1 ), 0, OUString(), pos, false, "" );
    addMenuItem( hMenu, IDM_INSTALL,0, pShutdownIcon->GetResString( STR_QUICKSTART_PRELAUNCH ), pos, false, "" );
    addMenuItem( hMenu, static_cast< UINT >( -1 ), 0, OUString(), pos, false, "" );
    addMenuItem( hMenu, IDM_EXIT,   0, pShutdownIcon->GetResString( STR_QUICKSTART_EXIT ), pos, false, "" );

    // indicate status of autostart folder
    CheckMenuItem( hMenu, IDM_INSTALL, MF_BYCOMMAND | (ShutdownIcon::GetAutostart() ? MF_CHECKED : MF_UNCHECKED) );

    return hMenu;
}


static void deleteSystrayMenu( HMENU hMenu )
{
    if( !hMenu || !IsMenu( hMenu ))
        return;

    MENUITEMINFOW mi;
    int pos=0;
    memset( &mi, 0, sizeof( mi ) );
    mi.cbSize = sizeof( mi );
    mi.fMask = MIIM_DATA;

    while( GetMenuItemInfoW( hMenu, pos++, true, &mi ) )
    {
        MYITEM *pMyItem = reinterpret_cast<MYITEM*>(mi.dwItemData);
        if( pMyItem )
        {
            (pMyItem->text).clear();
            delete pMyItem;
        }
        mi.fMask = MIIM_DATA;
    }
}


static void addTaskbarIcon( HWND hWnd )
{
    OUString strTip;
    if( ShutdownIcon::getInstance() )
        strTip = ShutdownIcon::getInstance()->GetResString( STR_QUICKSTART_TIP );

    // add taskbar icon
    NOTIFYICONDATAW nid;
    nid.hIcon = static_cast<HICON>(LoadImageA( GetModuleHandle( nullptr ), MAKEINTRESOURCE( ICON_LO_DEFAULT ),
        IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ),
        LR_DEFAULTCOLOR | LR_SHARED ));

    wcsncpy( nid.szTip, reinterpret_cast<LPCWSTR>(strTip.getStr()), 64 );

    nid.cbSize              = sizeof(nid);
    nid.hWnd                = hWnd;
    nid.uID                 = ID_QUICKSTART;
    nid.uCallbackMessage    = SFX_TASKBAR_NOTIFICATION;
    nid.uFlags              = NIF_MESSAGE|NIF_TIP|NIF_ICON;

    Shell_NotifyIconW(NIM_ADD, &nid);
}


LRESULT CALLBACK listenerWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static UINT s_uTaskbarRestart = 0;
    static UINT s_uMsgKillTray = 0;

    switch (uMsg)
    {
        case WM_NCCREATE:
            return TRUE;
        case WM_CREATE:
            {
                // request notification when taskbar is recreated
                // we then have to add our icon again
                s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
                s_uMsgKillTray = RegisterWindowMessage( SHUTDOWN_QUICKSTART_MESSAGE );

                // create the menu
                if( !popupMenu )
                    if( (popupMenu = createSystrayMenu( )) == nullptr )
                        return -1;

                // and the icon
                addTaskbarIcon( hWnd );

                // disable shutdown
                ShutdownIcon::getInstance()->SetVeto( true );
                ShutdownIcon::addTerminateListener();
            }
            return 0;

        case WM_MEASUREITEM:
            OnMeasureItem(hWnd, reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam));
            return TRUE;

        case WM_DRAWITEM:
            OnDrawItem(hWnd, reinterpret_cast<LPDRAWITEMSTRUCT>(lParam));
            return TRUE;

        case SFX_TASKBAR_NOTIFICATION:
            switch( lParam )
            {
                case WM_LBUTTONDBLCLK:
                {
                    BOOL const ret = PostMessage(aExecuterWindow, WM_COMMAND, IDM_TEMPLATE, reinterpret_cast<LPARAM>(hWnd));
                    SAL_WARN_IF(0 == ret, "sfx.appl", "ERROR: PostMessage() failed!");
                    break;
                }

                case WM_RBUTTONDOWN:
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    SetForegroundWindow( hWnd );

                    // update status before showing menu, could have been changed from option page
                    CheckMenuItem( popupMenu, IDM_INSTALL, MF_BYCOMMAND| (ShutdownIcon::GetAutostart() ? MF_CHECKED : MF_UNCHECKED) );

                    EnableMenuItem( popupMenu, IDM_EXIT, MF_BYCOMMAND | (ShutdownIcon::bModalMode ? MF_GRAYED : MF_ENABLED) );
                    EnableMenuItem( popupMenu, IDM_OPEN, MF_BYCOMMAND | (ShutdownIcon::bModalMode ? MF_GRAYED : MF_ENABLED) );
                    EnableMenuItem( popupMenu, IDM_TEMPLATE, MF_BYCOMMAND | (ShutdownIcon::bModalMode ? MF_GRAYED : MF_ENABLED) );
                    int m = TrackPopupMenuEx( popupMenu, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON,
                                              pt.x, pt.y, hWnd, nullptr );
                    BOOL const ret = PostMessage( hWnd, 0, 0, 0 );
                    SAL_WARN_IF(0 == ret, "sfx.appl", "ERROR: PostMessage() failed!");
                    switch( m )
                    {
                        case IDM_OPEN:
                        case IDM_WRITER:
                        case IDM_CALC:
                        case IDM_IMPRESS:
                        case IDM_DRAW:
                        case IDM_TEMPLATE:
                        case IDM_BASE:
                        case IDM_MATH:
                            break;
                        case IDM_INSTALL:
                            CheckMenuItem( popupMenu, IDM_INSTALL, MF_BYCOMMAND| (ShutdownIcon::GetAutostart() ? MF_CHECKED : MF_UNCHECKED) );
                            break;
                        case IDM_EXIT:
                            // delete taskbar icon
                            NOTIFYICONDATAA nid;
                            nid.cbSize=sizeof(NOTIFYICONDATA);
                            nid.hWnd = hWnd;
                            nid.uID = ID_QUICKSTART;
                            Shell_NotifyIconA(NIM_DELETE, &nid);
                            break;
                    }

                    BOOL const ret2 = PostMessage(aExecuterWindow, WM_COMMAND, m, reinterpret_cast<LPARAM>(hWnd));
                    SAL_WARN_IF(0 == ret2, "sfx.appl", "ERROR: PostMessage() failed!");
                }
                break;
            }
            break;
        case WM_DESTROY:
            deleteSystrayMenu( popupMenu );
            // We don't need the Systray Thread anymore
            PostQuitMessage( 0 );
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        default:
            if( uMsg == s_uTaskbarRestart )
            {
                // re-create taskbar icon
                addTaskbarIcon( hWnd );
            }
            else if ( uMsg == s_uMsgKillTray )
            {
                // delete taskbar icon
                NOTIFYICONDATAA nid;
                nid.cbSize=sizeof(NOTIFYICONDATA);
                nid.hWnd = hWnd;
                nid.uID = ID_QUICKSTART;
                Shell_NotifyIconA(NIM_DELETE, &nid);

                BOOL const ret = PostMessage(aExecuterWindow, WM_COMMAND, IDM_EXIT, reinterpret_cast<LPARAM>(hWnd));
                SAL_WARN_IF(0 == ret, "sfx.appl", "ERROR: PostMessage() failed!");
            }
            else
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}


LRESULT CALLBACK executerWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_NCCREATE:
            return TRUE;
        case WM_CREATE:
            return 0;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDM_OPEN:
                    if ( !ShutdownIcon::bModalMode )
                        ShutdownIcon::FileOpen();
                break;
                case IDM_WRITER:
                    ShutdownIcon::OpenURL( WRITER_URL, "_default" );
                break;
                case IDM_CALC:
                    ShutdownIcon::OpenURL( CALC_URL, "_default" );
                break;
                case IDM_IMPRESS:
                    ShutdownIcon::OpenURL( IMPRESS_WIZARD_URL, "_default" );
                break;
                case IDM_DRAW:
                    ShutdownIcon::OpenURL( DRAW_URL, "_default" );
                break;
                case IDM_BASE:
                    ShutdownIcon::OpenURL( BASE_URL, "_default" );
                break;
                case IDM_MATH:
                    ShutdownIcon::OpenURL( MATH_URL, "_default" );
                break;
                case IDM_TEMPLATE:
                    if ( !ShutdownIcon::bModalMode )
                        ShutdownIcon::FromTemplate();
                break;
                case IDM_INSTALL:
                    ShutdownIcon::SetAutostart( !ShutdownIcon::GetAutostart() );
                    break;
                case IDM_EXIT:
                    // remove listener and
                    //  terminate office if running in background
                    if ( !ShutdownIcon::bModalMode )
                        ShutdownIcon::terminateDesktop();
                    break;
            }
            break;
        case WM_DESTROY:
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}


DWORD WINAPI SystrayThread( LPVOID /*lpParam*/ )
{
    osl_setThreadName("SystrayThread");

    aListenerWindow = CreateWindowExA(0,
        QUICKSTART_CLASSNAME,       // registered class name
        QUICKSTART_WINDOWNAME,        // window name
        0,                          // window style
        CW_USEDEFAULT,              // horizontal position of window
        CW_USEDEFAULT,              // vertical position of window
        CW_USEDEFAULT,              // window width
        CW_USEDEFAULT,              // window height
        nullptr,                    // handle to parent or owner window
        nullptr,                    // menu handle or child identifier
        GetModuleHandle( nullptr ), // handle to application instance
        nullptr                     // window-creation data
        );

    MSG msg;

    while ( GetMessage( &msg, nullptr, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return msg.wParam; // Exit code of WM_QUIT
}


void win32_init_sys_tray()
{
    if ( ShutdownIcon::IsQuickstarterInstalled() )
    {
        WNDCLASSEXA listenerClass;
        listenerClass.cbSize        = sizeof(WNDCLASSEX);
        listenerClass.style         = 0;
        listenerClass.lpfnWndProc   = listenerWndProc;
        listenerClass.cbClsExtra    = 0;
        listenerClass.cbWndExtra    = 0;
        listenerClass.hInstance     = GetModuleHandle( nullptr );
        listenerClass.hIcon         = nullptr;
        listenerClass.hCursor       = nullptr;
        listenerClass.hbrBackground = nullptr;
        listenerClass.lpszMenuName  = nullptr;
        listenerClass.lpszClassName = QUICKSTART_CLASSNAME;
        listenerClass.hIconSm       = nullptr;

        RegisterClassExA(&listenerClass);

        WNDCLASSEXA executerClass;
        executerClass.cbSize        = sizeof(WNDCLASSEX);
        executerClass.style         = 0;
        executerClass.lpfnWndProc   = executerWndProc;
        executerClass.cbClsExtra    = 0;
        executerClass.cbWndExtra    = 0;
        executerClass.hInstance     = GetModuleHandle( nullptr );
        executerClass.hIcon         = nullptr;
        executerClass.hCursor       = nullptr;
        executerClass.hbrBackground = nullptr;
        executerClass.lpszMenuName  = nullptr;
        executerClass.lpszClassName = EXECUTER_WINDOWCLASS;
        executerClass.hIconSm       = nullptr;

        RegisterClassExA( &executerClass );

        aExecuterWindow = CreateWindowExA(0,
            EXECUTER_WINDOWCLASS,       // registered class name
            EXECUTER_WINDOWNAME,        // window name
            0,                          // window style
            CW_USEDEFAULT,              // horizontal position of window
            CW_USEDEFAULT,              // vertical position of window
            CW_USEDEFAULT,              // window width
            CW_USEDEFAULT,              // window height
            nullptr,                    // handle to parent or owner window
            nullptr,                    // menu handle or child identifier
            GetModuleHandle( nullptr ), // handle to application instance
            nullptr                     // window-creation data
            );

        DWORD   dwThreadId;
        CreateThread( nullptr, 0, SystrayThread, nullptr, 0, &dwThreadId );
    }
}


void win32_shutdown_sys_tray()
{
    if ( ShutdownIcon::IsQuickstarterInstalled() )
    {
        if( IsWindow( aListenerWindow ) )
        {
            DestroyWindow( aListenerWindow );
            aListenerWindow = nullptr;
            DestroyWindow( aExecuterWindow );
            aExecuterWindow = nullptr;
        }
        UnregisterClassA( QUICKSTART_CLASSNAME, GetModuleHandle( nullptr ) );
        UnregisterClassA( EXECUTER_WINDOWCLASS, GetModuleHandle( nullptr ) );
    }
}


void OnMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmis)
{
    MYITEM *pMyItem = reinterpret_cast<MYITEM *>(lpmis->itemData);
    HDC hdc = GetDC(hwnd);
    SIZE size;

    NONCLIENTMETRICS ncm;
    memset(&ncm, 0, sizeof(ncm));
    ncm.cbSize = sizeof(ncm);

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

    // Assume every menu item can be default and printed bold
    ncm.lfMenuFont.lfWeight = FW_BOLD;

    HFONT hfntOld = static_cast<HFONT>(SelectObject(hdc, CreateFontIndirect( &ncm.lfMenuFont )));

    GetTextExtentPoint32W(hdc, reinterpret_cast<LPCWSTR>(pMyItem->text.getStr()),
            pMyItem->text.getLength(), &size);

    lpmis->itemWidth = size.cx + 4 + GetSystemMetrics( SM_CXSMICON );
    lpmis->itemHeight = (size.cy > GetSystemMetrics( SM_CYSMICON )) ? size.cy : GetSystemMetrics( SM_CYSMICON );
    lpmis->itemHeight += 4;

    DeleteObject( SelectObject(hdc, hfntOld) );
    ReleaseDC(hwnd, hdc);
}

void OnDrawItem(HWND /*hwnd*/, LPDRAWITEMSTRUCT lpdis)
{
    MYITEM *pMyItem = reinterpret_cast<MYITEM *>(lpdis->itemData);
    COLORREF clrPrevText, clrPrevBkgnd;
    HFONT hfntOld;
    HBRUSH hbrOld;
    int x, y;
    BOOL    fSelected = lpdis->itemState & ODS_SELECTED;
    BOOL    fDisabled = lpdis->itemState & (ODS_DISABLED | ODS_GRAYED);

    // Set the appropriate foreground and background colors.

    RECT aRect = lpdis->rcItem;

    clrPrevBkgnd = SetBkColor( lpdis->hDC, GetSysColor(COLOR_MENU) );

    if ( fDisabled )
        clrPrevText = SetTextColor( lpdis->hDC, GetSysColor( COLOR_GRAYTEXT ) );
    else
        clrPrevText = SetTextColor( lpdis->hDC, GetSysColor( fSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

    if ( fSelected )
        clrPrevBkgnd = SetBkColor( lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT) );
    else
        clrPrevBkgnd = SetBkColor( lpdis->hDC, GetSysColor(COLOR_MENU) );

    hbrOld = static_cast<HBRUSH>(SelectObject( lpdis->hDC, CreateSolidBrush( GetBkColor( lpdis->hDC ) ) ));

    // Fill background
    PatBlt(lpdis->hDC, aRect.left, aRect.top, aRect.right-aRect.left, aRect.bottom-aRect.top, PATCOPY);

    int height = aRect.bottom-aRect.top;

    x = aRect.left;
    y = aRect.top;

    int     cx = GetSystemMetrics( SM_CXSMICON );
    int     cy = GetSystemMetrics( SM_CYSMICON );
    HICON   hIcon( nullptr );
    HMODULE hModule( GetModuleHandle( nullptr ) );

    if ( pMyItem->module.getLength() > 0 )
    {
        LPCWSTR pModuleName = reinterpret_cast<LPCWSTR>( pMyItem->module.getStr() );
        hModule = GetModuleHandleW( pModuleName );
        if ( hModule == nullptr )
        {
            LoadLibraryW( pModuleName );
            hModule = GetModuleHandleW( pModuleName );
        }
    }

    hIcon = static_cast<HICON>(LoadImageA( hModule, MAKEINTRESOURCE( pMyItem->iconId ),
                                IMAGE_ICON, cx, cy,
                                LR_DEFAULTCOLOR | LR_SHARED ));


    HBRUSH hbrIcon = CreateSolidBrush( GetSysColor( COLOR_GRAYTEXT ) );

    DrawStateW( lpdis->hDC, hbrIcon, nullptr, reinterpret_cast<LPARAM>(hIcon), (WPARAM)0, x, y+(height-cy)/2, 0, 0, DST_ICON | (fDisabled ? (fSelected ? DSS_MONO : DSS_DISABLED) : DSS_NORMAL) );

    DeleteObject( hbrIcon );

    x += cx + 4;    // space for icon
    aRect.left = x;

    NONCLIENTMETRICS ncm;
    memset(&ncm, 0, sizeof(ncm));
    ncm.cbSize = sizeof(ncm);

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

    // Print default menu entry with bold font
    if ( lpdis->itemState & ODS_DEFAULT )
        ncm.lfMenuFont.lfWeight = FW_BOLD;

    hfntOld = static_cast<HFONT>(SelectObject(lpdis->hDC, CreateFontIndirect( &ncm.lfMenuFont )));


    SIZE    size;
    GetTextExtentPointW( lpdis->hDC, reinterpret_cast<LPCWSTR>(pMyItem->text.getStr()), pMyItem->text.getLength(), &size );

    DrawStateW( lpdis->hDC, nullptr, nullptr, reinterpret_cast<LPARAM>(pMyItem->text.getStr()), (WPARAM)0, aRect.left, aRect.top + (height - size.cy)/2, 0, 0, DST_TEXT | (fDisabled && !fSelected ? DSS_DISABLED : DSS_NORMAL) );

    // Restore the original font and colors.
    DeleteObject( SelectObject( lpdis->hDC, hbrOld ) );
    DeleteObject( SelectObject( lpdis->hDC, hfntOld) );
    SetTextColor(lpdis->hDC, clrPrevText);
    SetBkColor(lpdis->hDC, clrPrevBkgnd);
}


// code from setup2 project


void SHFree_( void *pv )
{
    IMalloc *pMalloc;
    if( NOERROR == SHGetMalloc(&pMalloc) )
    {
        pMalloc->Free( pv );
        pMalloc->Release();
    }
}

#define ALLOC(type, n) static_cast<type *>(HeapAlloc(GetProcessHeap(), 0, sizeof(type) * n ))
#define FREE(p) HeapFree(GetProcessHeap(), 0, p)

static OUString SHGetSpecialFolder( int nFolderID )
{

    LPITEMIDLIST    pidl;
    HRESULT         hHdl = SHGetSpecialFolderLocation( nullptr, nFolderID, &pidl );
    OUString        aFolder;

    if( hHdl == NOERROR )
    {
        WCHAR *lpFolderA;
        lpFolderA = ALLOC( WCHAR, 16000 );

        SHGetPathFromIDListW( pidl, lpFolderA );
        aFolder = OUString( reinterpret_cast<const sal_Unicode*>(lpFolderA) );

        FREE( lpFolderA );
        SHFree_( pidl );
    }
    return aFolder;
}

OUString ShutdownIcon::GetAutostartFolderNameW32()
{
    return SHGetSpecialFolder(CSIDL_STARTUP);
}

static HRESULT WINAPI SHCoCreateInstance( LPVOID lpszReserved, REFCLSID clsid, LPUNKNOWN pUnkUnknown, REFIID iid, LPVOID *ppv )
{
    HRESULT hResult = E_NOTIMPL;
    HMODULE hModShell = GetModuleHandle( "SHELL32" );

    if ( hModShell != nullptr )
    {
        typedef HRESULT (WINAPI *SHCoCreateInstance_PROC)( LPVOID lpszReserved, REFCLSID clsid, LPUNKNOWN pUnkUnknwon, REFIID iid, LPVOID *ppv );

        SHCoCreateInstance_PROC lpfnSHCoCreateInstance = reinterpret_cast<SHCoCreateInstance_PROC>(GetProcAddress( hModShell, MAKEINTRESOURCE(102) ));

        if ( lpfnSHCoCreateInstance )
            hResult = lpfnSHCoCreateInstance( lpszReserved, clsid, pUnkUnknown, iid, ppv );
    }
    return hResult;
}

BOOL CreateShortcut( const OUString& rAbsObject, const OUString& rAbsObjectPath,
    const OUString& rAbsShortcut, const OUString& rDescription, const OUString& rParameter )
{
    HRESULT hres;
    IShellLink* psl;
    CLSID clsid_ShellLink = CLSID_ShellLink;
    CLSID clsid_IShellLink = IID_IShellLink;

    hres = CoCreateInstance( clsid_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                             clsid_IShellLink, reinterpret_cast<void**>(&psl) );
    if( FAILED(hres) )
        hres = SHCoCreateInstance( nullptr, clsid_ShellLink, nullptr, clsid_IShellLink, reinterpret_cast<void**>(&psl) );

    if( SUCCEEDED(hres) )
    {
        IPersistFile* ppf;
        psl->SetPath( OUStringToOString(rAbsObject, osl_getThreadTextEncoding()).getStr() );
        psl->SetWorkingDirectory( OUStringToOString(rAbsObjectPath, osl_getThreadTextEncoding()).getStr() );
        psl->SetDescription( OUStringToOString(rDescription, osl_getThreadTextEncoding()).getStr() );
        if( rParameter.getLength() )
            psl->SetArguments( OUStringToOString(rParameter, osl_getThreadTextEncoding()).getStr() );

        CLSID clsid_IPersistFile = IID_IPersistFile;
        hres = psl->QueryInterface( clsid_IPersistFile, reinterpret_cast<void**>(&ppf) );

        if( SUCCEEDED(hres) )
        {
            hres = ppf->Save( reinterpret_cast<LPCOLESTR>(rAbsShortcut.getStr()), TRUE );
            ppf->Release();
        } else return FALSE;
        psl->Release();
    } else return FALSE;
    return TRUE;
}


// install/uninstall

static bool FileExistsW( LPCWSTR lpPath )
{
    bool    bExists = false;
    WIN32_FIND_DATAW    aFindData;

    HANDLE  hFind = FindFirstFileW( lpPath, &aFindData );

    if ( INVALID_HANDLE_VALUE != hFind )
    {
        bExists = true;
        FindClose( hFind );
    }

    return bExists;
}

bool ShutdownIcon::IsQuickstarterInstalled()
{
    wchar_t aPath[_MAX_PATH];
    GetModuleFileNameW( nullptr, aPath, _MAX_PATH-1);

    OUString aOfficepath( reinterpret_cast<const sal_Unicode*>(aPath) );
    int i = aOfficepath.lastIndexOf('\\');
    if( i != -1 )
        aOfficepath = aOfficepath.copy(0, i);

    OUString quickstartExe(aOfficepath);
    quickstartExe += "\\quickstart.exe";

    return FileExistsW( reinterpret_cast<LPCWSTR>(quickstartExe.getStr()) );
}

void ShutdownIcon::EnableAutostartW32( const OUString &aShortcut )
{
    wchar_t aPath[_MAX_PATH];
    GetModuleFileNameW( nullptr, aPath, _MAX_PATH-1);

    OUString aOfficepath( reinterpret_cast<const sal_Unicode*>(aPath) );
    int i = aOfficepath.lastIndexOf('\\');
    if( i != -1 )
        aOfficepath = aOfficepath.copy(0, i);

    OUString quickstartExe(aOfficepath);
    quickstartExe += "\\quickstart.exe";

    CreateShortcut( quickstartExe, aOfficepath, aShortcut, OUString(), OUString() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
