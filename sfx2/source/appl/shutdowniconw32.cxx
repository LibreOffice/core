/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <sal/macros.h>

#ifdef WNT

// necessary to include system headers without warnings
#ifdef _MSC_VER
#pragma warning(disable:4668 4917)
#endif

// Support Windows 95 too
#undef WINVER
#define WINVER 0x0400
#define USE_APP_SHORTCUTS
//
// the systray icon is only available on windows
//

#include <unotools/moduleoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>

#include "shutdownicon.hxx"
#include "app.hrc"
#include <shlobj.h>
#include <objidl.h>
#include <stdio.h>
#include <io.h>
#include <osl/thread.h>
#include <setup_native/qswin32.h>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <set>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::osl;


#define EXECUTER_WINDOWCLASS    "SO Executer Class"
#define EXECUTER_WINDOWNAME     "SO Executer Window"


#define ID_QUICKSTART               1
#define IDM_EXIT                    2
#if defined(USE_APP_SHORTCUTS)
#   define IDM_OPEN                    3
#   define IDM_WRITER                  4
#   define IDM_CALC                    5
#   define IDM_IMPRESS                 6
#   define IDM_DRAW                    7
#   define IDM_BASE                    8
#   define IDM_TEMPLATE                9
#   define IDM_MATH                   12
#endif
#define IDM_INSTALL                 10
#define IDM_UNINSTALL               11


#define ICON_SO_DEFAULT                 1
#define ICON_TEXT_DOCUMENT              2
#define ICON_TEXT_TEMPLATE              3
#define ICON_SPREADSHEET_DOCUMENT       4
#define ICON_SPREADSHEET_TEMPLATE       5
#define ICON_DRAWING_DOCUMENT           6
#define ICON_DRAWING_TEMPLATE           7
#define ICON_PRESENTATION_DOCUMENT      8
#define ICON_PRESENTATION_TEMPLATE      9
#define ICON_PRESENTATION_COMPRESSED    10
#define ICON_GLOBAL_DOCUMENT            11
#define ICON_HTML_DOCUMENT              12
#define ICON_CHART_DOCUMENT             13
#define ICON_DATABASE_DOCUMENT          14
#define ICON_MATH_DOCUMENT              15
#define ICON_TEMPLATE                   16
#define ICON_MACROLIBRARY               17
#define ICON_CONFIGURATION              18
#define ICON_OPEN                       5   // See index of open folder icon in shell32.dll
#define ICON_SETUP                      500

#define SFX_TASKBAR_NOTIFICATION    WM_USER+1

static HWND  aListenerWindow = NULL;
static HWND  aExecuterWindow = NULL;
static HMENU popupMenu = NULL;

static void OnMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmis);
static void OnDrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpdis);

typedef struct tagMYITEM
{
    OUString text;
    OUString module;
    UINT iconId;
} MYITEM;

// -------------------------------

static bool isNT()
{
    static bool bInitialized    = false;
    static bool bWnt            = false;

    if( !bInitialized )
    {
        bInitialized = true;

        OSVERSIONINFO   aVerInfo;
        aVerInfo.dwOSVersionInfoSize = sizeof( aVerInfo );
        if ( GetVersionEx( &aVerInfo ) )
        {
            if ( aVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
                bWnt = true;
        }
    }
    return bWnt;
}


// -------------------------------

static void addMenuItem( HMENU hMenu, UINT id, UINT iconId, const OUString& text, int& pos, int bOwnerdraw, const OUString& module )
{
    MENUITEMINFOW mi;
    memset( &mi, 0, sizeof( MENUITEMINFOW ) );

    mi.cbSize = sizeof( MENUITEMINFOW );
    if( id == -1 )
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
            mi.dwItemData = (DWORD) pMyItem;
        }
        else
        {
            mi.fMask=MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_DATA;
            mi.fType=MFT_STRING;
            mi.fState=MFS_ENABLED;
            mi.wID = id;
            mi.dwTypeData = (LPWSTR) text.getStr();
            mi.cch = text.getLength();
        }

#if defined(USE_APP_SHORTCUTS)
        if ( IDM_TEMPLATE == id )
            mi.fState |= MFS_DEFAULT;
#endif
    }

    InsertMenuItemW( hMenu, pos++, TRUE, &mi );
}

// -------------------------------

static HMENU createSystrayMenu( )
{
    SvtModuleOptions    aModuleOptions;

    HMENU hMenu = CreatePopupMenu();
    int pos=0;

    ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();
    OSL_ENSURE( pShutdownIcon, "ShutdownIcon instance empty!");

    if( !pShutdownIcon )
        return NULL;

#if defined(USE_APP_SHORTCUTS)
    // collect the URLs of the entries in the File/New menu
    ::std::set< ::rtl::OUString > aFileNewAppsAvailable;
    SvtDynamicMenuOptions aOpt;
    Sequence < Sequence < PropertyValue > > aNewMenu = aOpt.GetMenu( E_NEWMENU );
    const ::rtl::OUString sURLKey( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );

    const Sequence< PropertyValue >* pNewMenu = aNewMenu.getConstArray();
    const Sequence< PropertyValue >* pNewMenuEnd = aNewMenu.getConstArray() + aNewMenu.getLength();
    for ( ; pNewMenu != pNewMenuEnd; ++pNewMenu )
    {
        ::comphelper::SequenceAsHashMap aEntryItems( *pNewMenu );
        ::rtl::OUString sURL( aEntryItems.getUnpackedValueOrDefault( sURLKey, ::rtl::OUString() ) );
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
        { SvtModuleOptions::E_SWRITER,    IDM_WRITER, ICON_TEXT_DOCUMENT,         WRITER_URL },
        { SvtModuleOptions::E_SCALC,      IDM_CALC,   ICON_SPREADSHEET_DOCUMENT,  CALC_URL },
        { SvtModuleOptions::E_SIMPRESS,   IDM_IMPRESS,ICON_PRESENTATION_DOCUMENT, IMPRESS_WIZARD_URL },
        { SvtModuleOptions::E_SDRAW,      IDM_DRAW,   ICON_DRAWING_DOCUMENT,      DRAW_URL },
        { SvtModuleOptions::E_SDATABASE,  IDM_BASE,   ICON_DATABASE_DOCUMENT,     BASE_URL },
        { SvtModuleOptions::E_SMATH,      IDM_MATH,   ICON_MATH_DOCUMENT,         MATH_URL },
    };

    OUString aEmpty;

    // insert the menu entries for launching the applications
    for ( size_t i = 0; i < SAL_N_ELEMENTS( aMenuItems ); ++i )
    {
        if ( !aModuleOptions.IsModuleInstalled( aMenuItems[i].eModuleIdentifier ) )
            // the complete application is not even installed
            continue;

        ::rtl::OUString sURL( ::rtl::OUString::createFromAscii( aMenuItems[i].pAsciiURLDescription ) );

        if ( aFileNewAppsAvailable.find( sURL ) == aFileNewAppsAvailable.end() )
            // the application is installed, but the entry has been configured to *not* appear in the File/New
            // menu => also let not appear it in the quickstarter
            continue;

        addMenuItem( hMenu, aMenuItems[i].nMenuItemID, aMenuItems[i].nMenuIconID,
            pShutdownIcon->GetUrlDescription( sURL ), pos, true, aEmpty );
    }



    // insert the remaining menu entries
    addMenuItem( hMenu, IDM_TEMPLATE, ICON_TEMPLATE,
        pShutdownIcon->GetResString( STR_QUICKSTART_FROMTEMPLATE ), pos, true, aEmpty);
    addMenuItem( hMenu, static_cast< UINT >( -1 ), 0, OUString(), pos, false, aEmpty );
    addMenuItem( hMenu, IDM_OPEN,   ICON_OPEN, pShutdownIcon->GetResString( STR_QUICKSTART_FILEOPEN ), pos, true, OUString(RTL_CONSTASCII_USTRINGPARAM("SHELL32")));
    addMenuItem( hMenu, static_cast< UINT >( -1 ), 0, OUString(), pos, false, aEmpty );
#endif
    addMenuItem( hMenu, IDM_INSTALL,0, pShutdownIcon->GetResString( STR_QUICKSTART_PRELAUNCH ), pos, false, aEmpty );
    addMenuItem( hMenu, static_cast< UINT >( -1 ), 0, OUString(), pos, false, aEmpty );
    addMenuItem( hMenu, IDM_EXIT,   0, pShutdownIcon->GetResString( STR_QUICKSTART_EXIT ), pos, false, aEmpty );

    // indicate status of autostart folder
    CheckMenuItem( hMenu, IDM_INSTALL, MF_BYCOMMAND | (ShutdownIcon::GetAutostart() ? MF_CHECKED : MF_UNCHECKED) );

    return hMenu;
}

// -------------------------------

static void deleteSystrayMenu( HMENU hMenu )
{
    if( !hMenu || !IsMenu( hMenu ))
        return;

    MENUITEMINFOW mi;
    MYITEM *pMyItem;
    int pos=0;
    memset( &mi, 0, sizeof( mi ) );
    mi.cbSize = sizeof( mi );
    mi.fMask = MIIM_DATA;

    while( GetMenuItemInfoW( hMenu, pos++, true, &mi ) )
    {
        pMyItem = (MYITEM*) mi.dwItemData;
        if( pMyItem )
        {
            pMyItem->text = OUString();
            delete pMyItem;
        }
        mi.fMask = MIIM_DATA;
    }
}

// -------------------------------

static void addTaskbarIcon( HWND hWnd )
{
    OUString strTip;
    if( ShutdownIcon::getInstance() )
        strTip = ShutdownIcon::getInstance()->GetResString( STR_QUICKSTART_TIP );

    // add taskbar icon
    NOTIFYICONDATAA nid;
    nid.hIcon = (HICON)LoadImageA( GetModuleHandle( NULL ), MAKEINTRESOURCE( ICON_SO_DEFAULT ),
        IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ),
        LR_DEFAULTCOLOR | LR_SHARED );

    // better use unicode wrapper here ?
    strncpy( nid.szTip, ( OUStringToOString(strTip, osl_getThreadTextEncoding()).getStr() ), 64 );

    nid.cbSize              = sizeof(nid);
    nid.hWnd                = hWnd;
    nid.uID                 = ID_QUICKSTART;
    nid.uCallbackMessage    = SFX_TASKBAR_NOTIFICATION;
    nid.uFlags              = NIF_MESSAGE|NIF_TIP|NIF_ICON;

    Shell_NotifyIconA(NIM_ADD, &nid);
}

// -------------------------------

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
                // request notfication when taskbar is recreated
                // we then have to add our icon again
                s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
                s_uMsgKillTray = RegisterWindowMessage( SHUTDOWN_QUICKSTART_MESSAGE );

                // create the menu
                if( !popupMenu )
                    if( (popupMenu = createSystrayMenu( )) == NULL )
                        return -1;

                // and the icon
                addTaskbarIcon( hWnd );

                // disable shutdown
                ShutdownIcon::getInstance()->SetVeto( true );
                ShutdownIcon::getInstance()->addTerminateListener();
            }
            return 0;

        case WM_MEASUREITEM:
            OnMeasureItem(hWnd, (LPMEASUREITEMSTRUCT) lParam);
            return TRUE;

        case WM_DRAWITEM:
            OnDrawItem(hWnd, (LPDRAWITEMSTRUCT) lParam);
            return TRUE;

        case SFX_TASKBAR_NOTIFICATION:
            switch( lParam )
            {
                case WM_LBUTTONDBLCLK:
#if defined(USE_APP_SHORTCUTS)
                    PostMessage( aExecuterWindow, WM_COMMAND, IDM_TEMPLATE, (LPARAM)hWnd );
#endif
                    break;

                case WM_RBUTTONDOWN:
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    SetForegroundWindow( hWnd );

                    // update status before showing menu, could have been changed from option page
                    CheckMenuItem( popupMenu, IDM_INSTALL, MF_BYCOMMAND| (ShutdownIcon::GetAutostart() ? MF_CHECKED : MF_UNCHECKED) );

                    EnableMenuItem( popupMenu, IDM_EXIT, MF_BYCOMMAND | (ShutdownIcon::bModalMode ? MF_GRAYED : MF_ENABLED) );
#if defined(USE_APP_SHORTCUTS)
                    EnableMenuItem( popupMenu, IDM_OPEN, MF_BYCOMMAND | (ShutdownIcon::bModalMode ? MF_GRAYED : MF_ENABLED) );
                    EnableMenuItem( popupMenu, IDM_TEMPLATE, MF_BYCOMMAND | (ShutdownIcon::bModalMode ? MF_GRAYED : MF_ENABLED) );
#endif
                    int m = TrackPopupMenuEx( popupMenu, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON,
                                              pt.x, pt.y, hWnd, NULL );
                    PostMessage( hWnd, NULL, 0, 0 );
                    switch( m )
                    {
#if defined(USE_APP_SHORTCUTS)
                        case IDM_OPEN:
                        case IDM_WRITER:
                        case IDM_CALC:
                        case IDM_IMPRESS:
                        case IDM_DRAW:
                        case IDM_TEMPLATE:
                        case IDM_BASE:
                        case IDM_MATH:
                            break;
#endif
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

                    PostMessage( aExecuterWindow, WM_COMMAND, m, (LPARAM)hWnd );
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

                PostMessage( aExecuterWindow, WM_COMMAND, IDM_EXIT, (LPARAM)hWnd );
            }
            else
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// -------------------------------

static sal_Bool checkOEM() {
    Reference<XMultiServiceFactory> rFactory = ::comphelper::getProcessServiceFactory();
    Reference<XJob> rOemJob(rFactory->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.office.OEMPreloadJob"))),
        UNO_QUERY );
    Sequence<NamedValue> args;
    sal_Bool bResult = sal_False;
    if (rOemJob.is())
    {
        Any aResult = rOemJob->execute(args);
        aResult >>= bResult;
    } else bResult = sal_True;
    return bResult;
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
#if defined(USE_APP_SHORTCUTS)
                case IDM_OPEN:
                    if ( !ShutdownIcon::bModalMode && checkOEM() )
                        ShutdownIcon::FileOpen();
                break;
                case IDM_WRITER:
                    if (checkOEM())
                    ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( WRITER_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
                break;
                case IDM_CALC:
                    if (checkOEM())
                    ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( CALC_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
                break;
                case IDM_IMPRESS:
                    if (checkOEM())
                    ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( IMPRESS_WIZARD_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
                break;
                case IDM_DRAW:
                    if (checkOEM())
                    ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( DRAW_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
                break;
                case IDM_BASE:
                    if (checkOEM())
                    ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( BASE_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
                break;
                case IDM_MATH:
                    if (checkOEM())
                    ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( MATH_URL ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
                break;
                case IDM_TEMPLATE:
                    if ( !ShutdownIcon::bModalMode && checkOEM())
                        ShutdownIcon::FromTemplate();
                break;
#endif
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

// -------------------------------


DWORD WINAPI SystrayThread( LPVOID /*lpParam*/ )
{
    aListenerWindow = CreateWindowExA(0,
        QUICKSTART_CLASSNAME,       // registered class name
        QUICKSTART_WINDOWNAME,        // window name
        0,                          // window style
        CW_USEDEFAULT,              // horizontal position of window
        CW_USEDEFAULT,              // vertical position of window
        CW_USEDEFAULT,              // window width
        CW_USEDEFAULT,              // window height
        (HWND) NULL,                // handle to parent or owner window
        NULL,                       // menu handle or child identifier
        (HINSTANCE) GetModuleHandle( NULL ),    // handle to application instance
        NULL                        // window-creation data
        );

    MSG msg;

    while ( GetMessage( &msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return msg.wParam; // Exit code of WM_QUIT
}

// -------------------------------

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
        listenerClass.hInstance     = (HINSTANCE) GetModuleHandle( NULL );
        listenerClass.hIcon         = NULL;
        listenerClass.hCursor       = NULL;
        listenerClass.hbrBackground = NULL;
        listenerClass.lpszMenuName  = NULL;
        listenerClass.lpszClassName = QUICKSTART_CLASSNAME;
        listenerClass.hIconSm       = NULL;

        RegisterClassExA(&listenerClass);

        WNDCLASSEXA executerClass;
        executerClass.cbSize        = sizeof(WNDCLASSEX);
        executerClass.style         = 0;
        executerClass.lpfnWndProc   = executerWndProc;
        executerClass.cbClsExtra    = 0;
        executerClass.cbWndExtra    = 0;
        executerClass.hInstance     = (HINSTANCE) GetModuleHandle( NULL );
        executerClass.hIcon         = NULL;
        executerClass.hCursor       = NULL;
        executerClass.hbrBackground = NULL;
        executerClass.lpszMenuName  = NULL;
        executerClass.lpszClassName = EXECUTER_WINDOWCLASS;
        executerClass.hIconSm       = NULL;

        RegisterClassExA( &executerClass );

        aExecuterWindow = CreateWindowExA(0,
            EXECUTER_WINDOWCLASS,       // registered class name
            EXECUTER_WINDOWNAME,        // window name
            0,                          // window style
            CW_USEDEFAULT,              // horizontal position of window
            CW_USEDEFAULT,              // vertical position of window
            CW_USEDEFAULT,              // window width
            CW_USEDEFAULT,              // window height
            (HWND) NULL,                // handle to parent or owner window
            NULL,                       // menu handle or child identifier
            (HINSTANCE) GetModuleHandle( NULL ),    // handle to application instance
            NULL                        // window-creation data
            );

        DWORD   dwThreadId;
        CreateThread( NULL, 0, SystrayThread, NULL, 0, &dwThreadId );
    }
}

// -------------------------------

void win32_shutdown_sys_tray()
{
    if ( ShutdownIcon::IsQuickstarterInstalled() )
    {
        if( IsWindow( aListenerWindow ) )
        {
            DestroyWindow( aListenerWindow );
            aListenerWindow = NULL;
            DestroyWindow( aExecuterWindow );
            aExecuterWindow = NULL;
        }
        UnregisterClassA( QUICKSTART_CLASSNAME, GetModuleHandle( NULL ) );
        UnregisterClassA( EXECUTER_WINDOWCLASS, GetModuleHandle( NULL ) );
    }
}



// -------------------------------

void OnMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmis)
{
    MYITEM *pMyItem = (MYITEM *) lpmis->itemData;
    HDC hdc = GetDC(hwnd);
    SIZE size;

    NONCLIENTMETRICS ncm;
    memset(&ncm, 0, sizeof(ncm));
    ncm.cbSize = sizeof(ncm);

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, (PVOID) &ncm, 0);

    // Assume every menu item can be default and printed bold
    ncm.lfMenuFont.lfWeight = FW_BOLD;

    HFONT hfntOld = (HFONT) SelectObject(hdc, (HFONT) CreateFontIndirect( &ncm.lfMenuFont ));

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
    MYITEM *pMyItem = (MYITEM *) lpdis->itemData;
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

    hbrOld = (HBRUSH)SelectObject( lpdis->hDC, CreateSolidBrush( GetBkColor( lpdis->hDC ) ) );

    // Fill background
    PatBlt(lpdis->hDC, aRect.left, aRect.top, aRect.right-aRect.left, aRect.bottom-aRect.top, PATCOPY);

    int height = aRect.bottom-aRect.top;

    x = aRect.left;
    y = aRect.top;

    int     cx = GetSystemMetrics( SM_CXSMICON );
    int     cy = GetSystemMetrics( SM_CYSMICON );
    HICON   hIcon( 0 );
    HMODULE hModule( GetModuleHandle( NULL ) );

    if ( pMyItem->module.getLength() > 0 )
    {
        LPCWSTR pModuleName = reinterpret_cast<LPCWSTR>( pMyItem->module.getStr() );
        hModule = GetModuleHandleW( pModuleName );
        if ( hModule == NULL )
        {
            LoadLibraryW( pModuleName );
            hModule = GetModuleHandleW( pModuleName );
        }
    }

    hIcon = (HICON) LoadImageA( hModule, MAKEINTRESOURCE( pMyItem->iconId ),
                                IMAGE_ICON, cx, cy,
                                LR_DEFAULTCOLOR | LR_SHARED );

    // DrawIconEx( lpdis->hDC, x, y+(height-cy)/2, hIcon, cx, cy, 0, NULL, DI_NORMAL );

    HBRUSH hbrIcon = CreateSolidBrush( GetSysColor( COLOR_GRAYTEXT ) );

    DrawStateW( lpdis->hDC, (HBRUSH)hbrIcon, (DRAWSTATEPROC)NULL, (LPARAM)hIcon, (WPARAM)0, x, y+(height-cy)/2, 0, 0, DST_ICON | (fDisabled ? (fSelected ? DSS_MONO : DSS_DISABLED) : DSS_NORMAL) );

    DeleteObject( hbrIcon );

    x += cx + 4;    // space for icon
    aRect.left = x;

    NONCLIENTMETRICS ncm;
    memset(&ncm, 0, sizeof(ncm));
    ncm.cbSize = sizeof(ncm);

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, (PVOID) &ncm, 0);

    // Print default menu entry with bold font
    if ( lpdis->itemState & ODS_DEFAULT )
        ncm.lfMenuFont.lfWeight = FW_BOLD;

    hfntOld = (HFONT) SelectObject(lpdis->hDC, (HFONT) CreateFontIndirect( &ncm.lfMenuFont ));


    SIZE    size;
    GetTextExtentPointW( lpdis->hDC, reinterpret_cast<LPCWSTR>(pMyItem->text.getStr()), pMyItem->text.getLength(), &size );

    DrawStateW( lpdis->hDC, (HBRUSH)NULL, (DRAWSTATEPROC)NULL, (LPARAM)pMyItem->text.getStr(), (WPARAM)0, aRect.left, aRect.top + (height - size.cy)/2, 0, 0, DST_TEXT | (fDisabled && !fSelected ? DSS_DISABLED : DSS_NORMAL) );

    // Restore the original font and colors.
    DeleteObject( SelectObject( lpdis->hDC, hbrOld ) );
    DeleteObject( SelectObject( lpdis->hDC, hfntOld) );
    SetTextColor(lpdis->hDC, clrPrevText);
    SetBkColor(lpdis->hDC, clrPrevBkgnd);
}

// -------------------------------
// code from setup2 project
// -------------------------------

void _SHFree( void *pv )
{
    IMalloc *pMalloc;
    if( NOERROR == SHGetMalloc(&pMalloc) )
    {
        pMalloc->Free( pv );
        pMalloc->Release();
    }
}

#define ALLOC(type, n) ((type *) HeapAlloc(GetProcessHeap(), 0, sizeof(type) * n ))
#define FREE(p) HeapFree(GetProcessHeap(), 0, p)

static OUString _SHGetSpecialFolder( int nFolderID )
{

    LPITEMIDLIST    pidl;
    HRESULT         hHdl = SHGetSpecialFolderLocation( NULL, nFolderID, &pidl );
    OUString        aFolder;

    if( hHdl == NOERROR )
    {
        WCHAR *lpFolderA;
        lpFolderA = ALLOC( WCHAR, 16000 );

        SHGetPathFromIDListW( pidl, lpFolderA );
        aFolder = OUString( reinterpret_cast<const sal_Unicode*>(lpFolderA) );

        FREE( lpFolderA );
        _SHFree( pidl );
    }
    return aFolder;
}

OUString ShutdownIcon::GetAutostartFolderNameW32()
{
    return _SHGetSpecialFolder(CSIDL_STARTUP);
}

static HRESULT WINAPI SHCoCreateInstance( LPVOID lpszReserved, REFCLSID clsid, LPUNKNOWN pUnkUnknown, REFIID iid, LPVOID *ppv )
{
    HRESULT hResult = E_NOTIMPL;
    HMODULE hModShell = GetModuleHandle( "SHELL32" );

    if ( hModShell != NULL )
    {
        typedef HRESULT (WINAPI *SHCoCreateInstance_PROC)( LPVOID lpszReserved, REFCLSID clsid, LPUNKNOWN pUnkUnknwon, REFIID iid, LPVOID *ppv );

        SHCoCreateInstance_PROC lpfnSHCoCreateInstance = (SHCoCreateInstance_PROC)GetProcAddress( hModShell, MAKEINTRESOURCE(102) );

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

    hres = CoCreateInstance( clsid_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                             clsid_IShellLink, (void**)&psl );
    if( FAILED(hres) )
        hres = SHCoCreateInstance( NULL, clsid_ShellLink, NULL, clsid_IShellLink, (void**)&psl );

    if( SUCCEEDED(hres) )
    {
        IPersistFile* ppf;
        psl->SetPath( OUStringToOString(rAbsObject, osl_getThreadTextEncoding()).getStr() );
        psl->SetWorkingDirectory( OUStringToOString(rAbsObjectPath, osl_getThreadTextEncoding()).getStr() );
        psl->SetDescription( OUStringToOString(rDescription, osl_getThreadTextEncoding()).getStr() );
        if( rParameter.getLength() )
            psl->SetArguments( OUStringToOString(rParameter, osl_getThreadTextEncoding()).getStr() );

        CLSID clsid_IPersistFile = IID_IPersistFile;
        hres = psl->QueryInterface( clsid_IPersistFile, (void**)&ppf );

        if( SUCCEEDED(hres) )
        {
            hres = ppf->Save( reinterpret_cast<LPCOLESTR>(rAbsShortcut.getStr()), TRUE );
            ppf->Release();
        } else return FALSE;
        psl->Release();
    } else return FALSE;
    return TRUE;
}

// ------------------
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
    if( isNT() )
    {
        GetModuleFileNameW( NULL, aPath, _MAX_PATH-1);
    }
    else
    {
        char szPathA[_MAX_PATH];
        GetModuleFileNameA( NULL, szPathA, _MAX_PATH-1);

        // calc the string wcstr len
        int nNeededWStrBuffSize = MultiByteToWideChar( CP_ACP, 0, szPathA, -1, NULL, 0 );

        // copy the string if necessary
        if ( nNeededWStrBuffSize > 0 )
            MultiByteToWideChar( CP_ACP, 0, szPathA, -1, aPath, nNeededWStrBuffSize );
    }

    OUString aOfficepath( reinterpret_cast<const sal_Unicode*>(aPath) );
    int i = aOfficepath.lastIndexOf((sal_Char) '\\');
    if( i != -1 )
        aOfficepath = aOfficepath.copy(0, i);

    OUString quickstartExe(aOfficepath);
    quickstartExe += OUString( RTL_CONSTASCII_USTRINGPARAM( "\\quickstart.exe" ) );

    return FileExistsW( reinterpret_cast<LPCWSTR>(quickstartExe.getStr()) );
}

void ShutdownIcon::EnableAutostartW32( const rtl::OUString &aShortcut )
{
    wchar_t aPath[_MAX_PATH];
    if( isNT() )
        GetModuleFileNameW( NULL, aPath, _MAX_PATH-1);
    else
    {
        char szPathA[_MAX_PATH];
        GetModuleFileNameA( NULL, szPathA, _MAX_PATH-1);

        // calc the string wcstr len
        int nNeededWStrBuffSize = MultiByteToWideChar( CP_ACP, 0, szPathA, -1, NULL, 0 );

        // copy the string if necessary
        if ( nNeededWStrBuffSize > 0 )
            MultiByteToWideChar( CP_ACP, 0, szPathA, -1, aPath, nNeededWStrBuffSize );
    }

    OUString aOfficepath( reinterpret_cast<const sal_Unicode*>(aPath) );
    int i = aOfficepath.lastIndexOf((sal_Char) '\\');
    if( i != -1 )
        aOfficepath = aOfficepath.copy(0, i);

    OUString quickstartExe(aOfficepath);
    quickstartExe += OUString( RTL_CONSTASCII_USTRINGPARAM( "\\quickstart.exe" ) );

    CreateShortcut( quickstartExe, aOfficepath, aShortcut, OUString(), OUString() );
}

#endif // WNT


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
