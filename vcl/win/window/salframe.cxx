/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/misccfg.hxx>

#include <officecfg/Office/Common.hxx>

#include <memory>
#include <string.h>
#include <limits.h>

#include <svsys.h>

#include <comphelper/windowserrorstring.hxx>

#include <rtl/string.h>
#include <rtl/ustring.h>

#include <osl/module.h>

#include <tools/debug.hxx>
#include <o3tl/enumarray.hxx>

#include <vcl/sysdata.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>

// Warning in SDK header
#ifdef _MSC_VER
#pragma warning( disable: 4242 )
#endif
#include <win/wincomp.hxx>
#include <win/salids.hrc>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salbmp.h>
#include <win/salgdi.h>
#include <win/salsys.h>
#include <win/salframe.h>
#include <win/salvd.h>
#include <win/salmenu.h>
#include <win/salobj.h>
#include <win/saltimer.h>

#include <impbmp.hxx>
#include <window.h>
#include <sallayout.hxx>

#define COMPILE_MULTIMON_STUBS
#pragma warning(push)
#pragma warning(disable:4996)
#include <multimon.h>
#pragma warning(pop)
#include <vector>
#ifdef __MINGW32__
#include <algorithm>
using ::std::max;
#endif

#include <com/sun/star/uno/Exception.hpp>

#include <oleacc.h>
#include <com/sun/star/accessibility/XMSAAService.hpp>
#ifndef WM_GETOBJECT // TESTME does this ever happen ?
#  define WM_GETOBJECT  0x003D
#endif

#include <time.h>

#if defined ( __MINGW32__ )
#include <sehandler.hxx>
#endif

#include <windows.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shellapi.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

#ifndef SPI_GETWHEELSCROLLCHARS
# define SPI_GETWHEELSCROLLCHARS   0x006C
#endif
#ifndef SPI_SETWHEELSCROLLCHARS
# define SPI_SETWHEELSCROLLCHARS   0x006D
#endif
#ifndef WM_MOUSEHWHEEL
# define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef IDC_PEN
# define IDC_PEN MAKEINTRESOURCE(32631)
#endif

const unsigned int WM_USER_SYSTEM_WINDOW_ACTIVATED = RegisterWindowMessageA("SYSTEM_WINDOW_ACTIVATED");

bool WinSalFrame::mbInReparent = FALSE;

// Macros for support of WM_UNICHAR & Keyman 6.0
//#define Uni_UTF32ToSurrogate1(ch)   (((unsigned long) (ch) - 0x10000) / 0x400 + 0xD800)
#define Uni_UTF32ToSurrogate2(ch)   (((unsigned long) (ch) - 0x10000) % 0x400 + 0xDC00)
#define Uni_SupplementaryPlanesStart    0x10000

static void UpdateFrameGeometry( HWND hWnd, WinSalFrame* pFrame );
static void SetMaximizedFrameGeometry( HWND hWnd, WinSalFrame* pFrame, RECT* pParentRect = nullptr );

static void ImplSaveFrameState( WinSalFrame* pFrame )
{
    // save position, size and state for GetWindowState()
    if ( !pFrame->mbFullScreen )
    {
        bool bVisible = (GetWindowStyle( pFrame->mhWnd ) & WS_VISIBLE) != 0;
        if ( IsIconic( pFrame->mhWnd ) )
        {
            pFrame->maState.mnState |= WindowStateState::Minimized;
            if ( bVisible )
                pFrame->mnShowState = SW_SHOWMAXIMIZED;
        }
        else if ( IsZoomed( pFrame->mhWnd ) )
        {
            pFrame->maState.mnState &= ~WindowStateState::Minimized;
            pFrame->maState.mnState |= WindowStateState::Maximized;
            if ( bVisible )
                pFrame->mnShowState = SW_SHOWMAXIMIZED;
            pFrame->mbRestoreMaximize = TRUE;

            WINDOWPLACEMENT aPlacement;
            aPlacement.length = sizeof(aPlacement);
            if( GetWindowPlacement( pFrame->mhWnd, &aPlacement ) )
            {
                RECT aRect = aPlacement.rcNormalPosition;
                RECT aRect2 = aRect;
                AdjustWindowRectEx( &aRect2, GetWindowStyle( pFrame->mhWnd ),
                                    FALSE,  GetWindowExStyle( pFrame->mhWnd ) );
                long nTopDeco = abs( aRect.top - aRect2.top );
                long nLeftDeco = abs( aRect.left - aRect2.left );
                long nBottomDeco = abs( aRect.bottom - aRect2.bottom );
                long nRightDeco = abs( aRect.right - aRect2.right );

                pFrame->maState.mnX      = aRect.left + nLeftDeco;
                pFrame->maState.mnY      = aRect.top + nTopDeco;
                pFrame->maState.mnWidth  = aRect.right - aRect.left - nLeftDeco - nRightDeco;
                pFrame->maState.mnHeight = aRect.bottom - aRect.top - nTopDeco - nBottomDeco;
            }
        }
        else
        {
            RECT aRect;
            GetWindowRect( pFrame->mhWnd, &aRect );

            // to be consistent with Unix, the frame state is without(!) decoration
            RECT aRect2 = aRect;
            AdjustWindowRectEx( &aRect2, GetWindowStyle( pFrame->mhWnd ),
                            FALSE,     GetWindowExStyle( pFrame->mhWnd ) );
            long nTopDeco = abs( aRect.top - aRect2.top );
            long nLeftDeco = abs( aRect.left - aRect2.left );
            long nBottomDeco = abs( aRect.bottom - aRect2.bottom );
            long nRightDeco = abs( aRect.right - aRect2.right );

            pFrame->maState.mnState &= ~WindowStateState(WindowStateState::Minimized | WindowStateState::Maximized);
            // subtract decoration
            pFrame->maState.mnX      = aRect.left+nLeftDeco;
            pFrame->maState.mnY      = aRect.top+nTopDeco;
            pFrame->maState.mnWidth  = aRect.right-aRect.left-nLeftDeco-nRightDeco;
            pFrame->maState.mnHeight = aRect.bottom-aRect.top-nTopDeco-nBottomDeco;
            if ( bVisible )
                pFrame->mnShowState = SW_SHOWNORMAL;
            pFrame->mbRestoreMaximize = FALSE;
        }
    }
}

// if pParentRect is set, the workarea of the monitor that contains pParentRect is returned
void ImplSalGetWorkArea( HWND hWnd, RECT *pRect, const RECT *pParentRect )
{
    // check if we or our parent is fullscreen, then the taskbar should be ignored
    bool bIgnoreTaskbar = false;
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if( pFrame )
    {
        vcl::Window *pWin = pFrame->GetWindow();
        while( pWin )
        {
            WorkWindow *pWorkWin = (pWin->GetType() == WINDOW_WORKWINDOW) ? static_cast<WorkWindow *>(pWin) : nullptr;
            if( pWorkWin && pWorkWin->ImplGetWindowImpl()->mbReallyVisible && pWorkWin->IsFullScreenMode() )
            {
                bIgnoreTaskbar = true;
                break;
            }
            else
                pWin = pWin->ImplGetWindowImpl()->mpParent;
        }
    }

    // calculates the work area taking multiple monitors into account
    static int nMonitors = GetSystemMetrics( SM_CMONITORS );
    if( nMonitors == 1 )
    {
        if( bIgnoreTaskbar )
        {
            pRect->left = pRect->top = 0;
            pRect->right   = GetSystemMetrics( SM_CXSCREEN );
            pRect->bottom  = GetSystemMetrics( SM_CYSCREEN );
        }
        else
            SystemParametersInfo( SPI_GETWORKAREA, 0, pRect, 0 );
    }
    else
    {
        if( pParentRect != nullptr )
        {
            // return the size of the monitor where pParentRect lives
            HMONITOR hMonitor;
            MONITORINFO mi;

            // get the nearest monitor to the passed rect.
            hMonitor = MonitorFromRect(pParentRect, MONITOR_DEFAULTTONEAREST);

            // get the work area or entire monitor rect.
            mi.cbSize = sizeof(mi);
            GetMonitorInfo(hMonitor, &mi);
            if( !bIgnoreTaskbar )
                *pRect = mi.rcWork;
            else
                *pRect = mi.rcMonitor;
        }
        else
        {
            // return the union of all monitors
            pRect->left = GetSystemMetrics( SM_XVIRTUALSCREEN );
            pRect->top = GetSystemMetrics( SM_YVIRTUALSCREEN );
            pRect->right = pRect->left + GetSystemMetrics( SM_CXVIRTUALSCREEN );
            pRect->bottom = pRect->top + GetSystemMetrics( SM_CYVIRTUALSCREEN );

            // virtualscreen does not take taskbar into account, so use the corresponding
            // diffs between screen and workarea from the default screen
            // however, this is still not perfect: the taskbar might not be on the primary screen
            if( !bIgnoreTaskbar )
            {
                RECT wRect, scrRect;
                SystemParametersInfo( SPI_GETWORKAREA, 0, &wRect, 0 );
                scrRect.left = 0;
                scrRect.top = 0;
                scrRect.right = GetSystemMetrics( SM_CXSCREEN );
                scrRect.bottom = GetSystemMetrics( SM_CYSCREEN );

                pRect->left += wRect.left;
                pRect->top += wRect.top;
                pRect->right -= scrRect.right - wRect.right;
                pRect->bottom -= scrRect.bottom - wRect.bottom;
            }
        }
    }
}

SalFrame* ImplSalCreateFrame( WinSalInstance* pInst,
                              HWND hWndParent, SalFrameStyleFlags nSalFrameStyle )
{
    WinSalFrame*   pFrame = new WinSalFrame;
    HWND        hWnd;
    DWORD       nSysStyle = 0;
    DWORD       nExSysStyle = 0;
    bool        bSubFrame = FALSE;

    static const char* pEnvSynchronize = getenv("SAL_SYNCHRONIZE");
    if ( pEnvSynchronize )   // no buffering of drawing commands
        GdiSetBatchLimit( 1 );

    static const char* pEnvTransparentFloats = getenv("SAL_TRANSPARENT_FLOATS" );

    // determine creation data
    if ( nSalFrameStyle & (SalFrameStyleFlags::PLUG | SalFrameStyleFlags::SYSTEMCHILD) )
    {
        nSysStyle |= WS_CHILD;
        if( nSalFrameStyle & SalFrameStyleFlags::SYSTEMCHILD )
            nSysStyle |= WS_CLIPSIBLINGS;
    }
    else
    {
        // #i87402# commenting out WS_CLIPCHILDREN
        // this breaks SalFrameStyleFlags::SYSTEMCHILD handling, which is not
        // used currently. Probably SalFrameStyleFlags::SYSTEMCHILD should be
        // removed again.

        // nSysStyle  |= WS_CLIPCHILDREN;
        if ( hWndParent )
        {
            nSysStyle |= WS_POPUP;
            bSubFrame = TRUE;
            pFrame->mbNoIcon = TRUE;
        }
        else
        {
            // Only with WS_OVRLAPPED we get a useful default position/size
            if ( (nSalFrameStyle & (SalFrameStyleFlags::SIZEABLE | SalFrameStyleFlags::MOVEABLE)) ==
                 (SalFrameStyleFlags::SIZEABLE | SalFrameStyleFlags::MOVEABLE) )
                nSysStyle |= WS_OVERLAPPED;
            else
            {
                nSysStyle |= WS_POPUP;
                if ( !(nSalFrameStyle & SalFrameStyleFlags::MOVEABLE) )
                    nExSysStyle |= WS_EX_TOOLWINDOW;    // avoid taskbar appearance, for eg splash screen
            }
        }

        if ( nSalFrameStyle & SalFrameStyleFlags::MOVEABLE )
        {
            pFrame->mbCaption = TRUE;
            nSysStyle |= WS_SYSMENU | WS_CAPTION;
            if ( !hWndParent )
                nSysStyle |= WS_SYSMENU | WS_MINIMIZEBOX;
            else
                nExSysStyle |= WS_EX_DLGMODALFRAME;

            if ( nSalFrameStyle & SalFrameStyleFlags::SIZEABLE )
            {
                pFrame->mbSizeBorder = TRUE;
                nSysStyle |= WS_THICKFRAME;
                if ( !hWndParent )
                    nSysStyle |= WS_MAXIMIZEBOX;
            }
            else
                pFrame->mbFixBorder = TRUE;

            if ( nSalFrameStyle & SalFrameStyleFlags::DEFAULT )
                nExSysStyle |= WS_EX_APPWINDOW;
        }
        if( nSalFrameStyle & SalFrameStyleFlags::TOOLWINDOW
            // #100656# toolwindows lead to bad alt-tab behaviour, if they have the focus
            // you must press it twice to leave the application
            // so toolwindows are only used for non sizeable windows
            // which are typically small, so a small caption makes sense

            // #103578# looked too bad - above changes reverted
            /* && !(nSalFrameStyle & SalFrameStyleFlags::SIZEABLE) */ )
        {
            pFrame->mbNoIcon = TRUE;
            nExSysStyle |= WS_EX_TOOLWINDOW;
            if ( pEnvTransparentFloats /*&& !(nSalFrameStyle & SalFrameStyleFlags::MOVEABLE) */)
                nExSysStyle |= WS_EX_LAYERED;
        }
    }
    if ( nSalFrameStyle & SalFrameStyleFlags::FLOAT )
    {
        nExSysStyle |= WS_EX_TOOLWINDOW;
        pFrame->mbFloatWin = TRUE;

        if (pEnvTransparentFloats)
            nExSysStyle |= WS_EX_LAYERED;

    }
    if (nSalFrameStyle & SalFrameStyleFlags::TOOLTIP)
        nExSysStyle |= WS_EX_TOPMOST;

    // init frame data
    pFrame->mnStyle = nSalFrameStyle;

    // determine show style
    pFrame->mnShowState = SW_SHOWNORMAL;
    if ( (nSysStyle & (WS_POPUP | WS_MAXIMIZEBOX | WS_THICKFRAME)) == (WS_MAXIMIZEBOX | WS_THICKFRAME) )
    {
        if ( GetSystemMetrics( SM_CXSCREEN ) <= 1024 )
            pFrame->mnShowState = SW_SHOWMAXIMIZED;
        else
        {
            if ( nSalFrameStyle & SalFrameStyleFlags::DEFAULT )
            {
                SalData* pSalData = GetSalData();
                pFrame->mnShowState = pSalData->mnCmdShow;
                if ( (pFrame->mnShowState != SW_SHOWMINIMIZED) &&
                     (pFrame->mnShowState != SW_MINIMIZE) &&
                     (pFrame->mnShowState != SW_SHOWMINNOACTIVE) )
                {
                    if ( (pFrame->mnShowState == SW_SHOWMAXIMIZED) ||
                         (pFrame->mnShowState == SW_MAXIMIZE) )
                        pFrame->mbOverwriteState = FALSE;
                    pFrame->mnShowState = SW_SHOWMAXIMIZED;
                }
                else
                    pFrame->mbOverwriteState = FALSE;
            }
            else
            {
                // Document Windows are also maximized, if the current Document Window
                // is also maximized
                HWND hWnd2 = GetForegroundWindow();
                if ( hWnd2 && IsMaximized( hWnd2 ) &&
                     (GetWindowInstance( hWnd2 ) == pInst->mhInst) &&
                     ((GetWindowStyle( hWnd2 ) & (WS_POPUP | WS_MAXIMIZEBOX | WS_THICKFRAME)) == (WS_MAXIMIZEBOX | WS_THICKFRAME)) )
                    pFrame->mnShowState = SW_SHOWMAXIMIZED;
            }
        }
    }

    // create frame
    LPCWSTR pClassName;
    if ( bSubFrame )
    {
        if ( nSalFrameStyle & (SalFrameStyleFlags::MOVEABLE|SalFrameStyleFlags::NOSHADOW) ) // check if shadow not wanted
            pClassName = SAL_SUBFRAME_CLASSNAMEW;
        else
            pClassName = SAL_TMPSUBFRAME_CLASSNAMEW;    // undecorated floaters will get shadow on XP
    }
    else
    {
        if ( nSalFrameStyle & SalFrameStyleFlags::MOVEABLE )
            pClassName = SAL_FRAME_CLASSNAMEW;
        else
            pClassName = SAL_TMPSUBFRAME_CLASSNAMEW;
    }
    hWnd = CreateWindowExW( nExSysStyle, pClassName, L"", nSysStyle,
                            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                            hWndParent, nullptr, pInst->mhInst, pFrame );
    SAL_WARN_IF(!hWnd, "vcl", "CreateWindowExW failed: " << WindowsErrorString(GetLastError()));

#if OSL_DEBUG_LEVEL > 1
    // set transparency value
    if( GetWindowExStyle( hWnd ) & WS_EX_LAYERED )
        SetLayeredWindowAttributes( hWnd, 0, 230, 0x00000002 /*LWA_ALPHA*/ );
#endif
    if ( !hWnd )
    {
        delete pFrame;
        return nullptr;
    }

    // If we have an Window with an Caption Bar and without
    // an MaximizeBox, we change the SystemMenu
    if ( (nSysStyle & (WS_CAPTION | WS_MAXIMIZEBOX)) == (WS_CAPTION) )
    {
        HMENU hSysMenu = GetSystemMenu( hWnd, FALSE );
        if ( hSysMenu )
        {
            if ( !(nSysStyle & (WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) )
                DeleteMenu( hSysMenu, SC_RESTORE, MF_BYCOMMAND );
            else
                EnableMenuItem( hSysMenu, SC_RESTORE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED );
            if ( !(nSysStyle & WS_MINIMIZEBOX) )
                DeleteMenu( hSysMenu, SC_MINIMIZE, MF_BYCOMMAND );
            if ( !(nSysStyle & WS_MAXIMIZEBOX) )
                DeleteMenu( hSysMenu, SC_MAXIMIZE, MF_BYCOMMAND );
            if ( !(nSysStyle & WS_THICKFRAME) )
                DeleteMenu( hSysMenu, SC_SIZE, MF_BYCOMMAND );
        }
    }
    if ( (nSysStyle & WS_SYSMENU) && !(nSalFrameStyle & SalFrameStyleFlags::CLOSEABLE) )
    {
        HMENU hSysMenu = GetSystemMenu( hWnd, FALSE );
        if ( hSysMenu )
            EnableMenuItem( hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED );
    }

    // reset input context
    pFrame->mhDefIMEContext = ImmAssociateContext( hWnd, nullptr );

    // determine output size and state
    RECT aRect;
    GetClientRect( hWnd, &aRect );
    pFrame->mnWidth  = aRect.right;
    pFrame->mnHeight = aRect.bottom;
    ImplSaveFrameState( pFrame );
    pFrame->mbDefPos = TRUE;

    UpdateFrameGeometry( hWnd, pFrame );

    if( pFrame->mnShowState == SW_SHOWMAXIMIZED )
    {
        // #96084 set a useful internal window size because
        // the window will not be maximized (and the size updated) before show()

        SetMaximizedFrameGeometry( hWnd, pFrame );
    }

    return pFrame;
}

// helper that only creates the HWND
// to allow for easy reparenting of system windows, (i.e. destroy and create new)
HWND ImplSalReCreateHWND( HWND hWndParent, HWND oldhWnd, bool bAsChild )
{
    HINSTANCE hInstance = GetSalData()->mhInst;
    sal_uLong nSysStyle     = GetWindowLong( oldhWnd, GWL_STYLE );
    sal_uLong nExSysStyle   = GetWindowLong( oldhWnd, GWL_EXSTYLE );

    if( bAsChild )
    {
        nSysStyle = WS_CHILD;
        nExSysStyle = 0;
    }

    LPCWSTR pClassName = SAL_SUBFRAME_CLASSNAMEW;
    return CreateWindowExW( nExSysStyle, pClassName, L"", nSysStyle,
                            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                            hWndParent, nullptr, hInstance, GetWindowPtr( oldhWnd ) );
}

// translation table from System keycodes into StartView keycodes
#define KEY_TAB_SIZE     146

static const sal_uInt16 aImplTranslateKeyTab[KEY_TAB_SIZE] =
{
    // StarView-Code      System-Code                         Index
    0,                    //                                  0
    0,                    // VK_LBUTTON                       1
    0,                    // VK_RBUTTON                       2
    0,                    // VK_CANCEL                        3
    0,                    // VK_MBUTTON                       4
    0,                    //                                  5
    0,                    //                                  6
    0,                    //                                  7
    KEY_BACKSPACE,        // VK_BACK                          8
    KEY_TAB,              // VK_TAB                           9
    0,                    //                                  10
    0,                    //                                  11
    0,                    // VK_CLEAR                         12
    KEY_RETURN,           // VK_RETURN                        13
    0,                    //                                  14
    0,                    //                                  15
    0,                    // VK_SHIFT                         16
    0,                    // VK_CONTROL                       17
    0,                    // VK_MENU                          18
    0,                    // VK_PAUSE                         19
    0,                    // VK_CAPITAL                       20
    0,                    // VK_HANGUL                        21
    0,                    //                                  22
    0,                    //                                  23
    0,                    //                                  24
    KEY_HANGUL_HANJA,     // VK_HANJA                         25
    0,                    //                                  26
    KEY_ESCAPE,           // VK_ESCAPE                        27
    0,                    //                                  28
    0,                    //                                  29
    0,                    //                                  30
    0,                    //                                  31
    KEY_SPACE,            // VK_SPACE                         32
    KEY_PAGEUP,           // VK_PRIOR                         33
    KEY_PAGEDOWN,         // VK_NEXT                          34
    KEY_END,              // VK_END                           35
    KEY_HOME,             // VK_HOME                          36
    KEY_LEFT,             // VK_LEFT                          37
    KEY_UP,               // VK_UP                            38
    KEY_RIGHT,            // VK_RIGHT                         39
    KEY_DOWN,             // VK_DOWN                          40
    0,                    // VK_SELECT                        41
    0,                    // VK_PRINT                         42
    0,                    // VK_EXECUTE                       43
    0,                    // VK_SNAPSHOT                      44
    KEY_INSERT,           // VK_INSERT                        45
    KEY_DELETE,           // VK_DELETE                        46
    KEY_HELP,             // VK_HELP                          47
    KEY_0,                //                                  48
    KEY_1,                //                                  49
    KEY_2,                //                                  50
    KEY_3,                //                                  51
    KEY_4,                //                                  52
    KEY_5,                //                                  53
    KEY_6,                //                                  54
    KEY_7,                //                                  55
    KEY_8,                //                                  56
    KEY_9,                //                                  57
    0,                    //                                  58
    0,                    //                                  59
    0,                    //                                  60
    0,                    //                                  61
    0,                    //                                  62
    0,                    //                                  63
    0,                    //                                  64
    KEY_A,                //                                  65
    KEY_B,                //                                  66
    KEY_C,                //                                  67
    KEY_D,                //                                  68
    KEY_E,                //                                  69
    KEY_F,                //                                  70
    KEY_G,                //                                  71
    KEY_H,                //                                  72
    KEY_I,                //                                  73
    KEY_J,                //                                  74
    KEY_K,                //                                  75
    KEY_L,                //                                  76
    KEY_M,                //                                  77
    KEY_N,                //                                  78
    KEY_O,                //                                  79
    KEY_P,                //                                  80
    KEY_Q,                //                                  81
    KEY_R,                //                                  82
    KEY_S,                //                                  83
    KEY_T,                //                                  84
    KEY_U,                //                                  85
    KEY_V,                //                                  86
    KEY_W,                //                                  87
    KEY_X,                //                                  88
    KEY_Y,                //                                  89
    KEY_Z,                //                                  90
    0,                    // VK_LWIN                          91
    0,                    // VK_RWIN                          92
    KEY_CONTEXTMENU,      // VK_APPS                          93
    0,                    //                                  94
    0,                    //                                  95
    KEY_0,                // VK_NUMPAD0                       96
    KEY_1,                // VK_NUMPAD1                       97
    KEY_2,                // VK_NUMPAD2                       98
    KEY_3,                // VK_NUMPAD3                       99
    KEY_4,                // VK_NUMPAD4                      100
    KEY_5,                // VK_NUMPAD5                      101
    KEY_6,                // VK_NUMPAD6                      102
    KEY_7,                // VK_NUMPAD7                      103
    KEY_8,                // VK_NUMPAD8                      104
    KEY_9,                // VK_NUMPAD9                      105
    KEY_MULTIPLY,         // VK_MULTIPLY                     106
    KEY_ADD,              // VK_ADD                          107
    KEY_DECIMAL,          // VK_SEPARATOR                    108
    KEY_SUBTRACT,         // VK_SUBTRACT                     109
    KEY_DECIMAL,          // VK_DECIMAL                      110
    KEY_DIVIDE,           // VK_DIVIDE                       111
    KEY_F1,               // VK_F1                           112
    KEY_F2,               // VK_F2                           113
    KEY_F3,               // VK_F3                           114
    KEY_F4,               // VK_F4                           115
    KEY_F5,               // VK_F5                           116
    KEY_F6,               // VK_F6                           117
    KEY_F7,               // VK_F7                           118
    KEY_F8,               // VK_F8                           119
    KEY_F9,               // VK_F9                           120
    KEY_F10,              // VK_F10                          121
    KEY_F11,              // VK_F11                          122
    KEY_F12,              // VK_F12                          123
    KEY_F13,              // VK_F13                          124
    KEY_F14,              // VK_F14                          125
    KEY_F15,              // VK_F15                          126
    KEY_F16,              // VK_F16                          127
    KEY_F17,              // VK_F17                          128
    KEY_F18,              // VK_F18                          129
    KEY_F19,              // VK_F19                          130
    KEY_F20,              // VK_F20                          131
    KEY_F21,              // VK_F21                          132
    KEY_F22,              // VK_F22                          133
    KEY_F23,              // VK_F23                          134
    KEY_F24,              // VK_F24                          135
    0,                    //                                 136
    0,                    //                                 137
    0,                    //                                 138
    0,                    //                                 139
    0,                    //                                 140
    0,                    //                                 141
    0,                    //                                 142
    0,                    //                                 143
    0,                    // NUMLOCK                         144
    0                     // SCROLLLOCK                      145
};

static UINT ImplSalGetWheelScrollLines()
{
    UINT nScrLines = 0;
    HWND hWndMsWheel = FindWindowA( MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE );
    if ( hWndMsWheel )
    {
        UINT nGetScrollLinesMsgId = RegisterWindowMessage( MSH_SCROLL_LINES );
        nScrLines = (UINT)SendMessageW( hWndMsWheel, nGetScrollLinesMsgId, 0, 0 );
    }

    if ( !nScrLines )
        if( !SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &nScrLines, 0 ) )
            nScrLines = 0 ;

    if ( !nScrLines )
        nScrLines = 3;

    return nScrLines;
}

static UINT ImplSalGetWheelScrollChars()
{
    UINT nScrChars = 0;
    if( !SystemParametersInfo( SPI_GETWHEELSCROLLCHARS, 0, &nScrChars, 0 ) )
    {
        // Depending on Windows version, use proper default or 1 (when
        // driver emulates hscroll)
        if (!aSalShlData.mbWVista)
        {
            // Windows 2000 & WinXP : emulating driver, use step size
            // of 1
            return 1;
        }
        else
        {
            // Longhorn or above: use proper default value of 3
            return 3;
        }
    }

    // system settings successfully read
    return nScrChars;
}

static void ImplSalAddBorder( const WinSalFrame* pFrame, int& width, int& height )
{
    // transform client size into window size
    RECT    aWinRect;
    aWinRect.left   = 0;
    aWinRect.right  = width-1;
    aWinRect.top    = 0;
    aWinRect.bottom = height-1;
    AdjustWindowRectEx( &aWinRect, GetWindowStyle( pFrame->mhWnd ),
                        FALSE,     GetWindowExStyle( pFrame->mhWnd ) );
    width  = aWinRect.right - aWinRect.left + 1;
    height = aWinRect.bottom - aWinRect.top + 1;
}

static void ImplSalCalcFullScreenSize( const WinSalFrame* pFrame,
                                       int& rX, int& rY, int& rDX, int& rDY )
{
    // set window to screen size
    int nFrameX;
    int nFrameY;
    int nCaptionY;
    int nScreenX = 0;
    int nScreenY = 0;
    int nScreenDX = 0;
    int nScreenDY = 0;

    if ( pFrame->mbSizeBorder )
    {
        nFrameX = GetSystemMetrics( SM_CXSIZEFRAME );
        nFrameY = GetSystemMetrics( SM_CYSIZEFRAME );
    }
    else if ( pFrame->mbFixBorder )
    {
        nFrameX = GetSystemMetrics( SM_CXFIXEDFRAME );
        nFrameY = GetSystemMetrics( SM_CYFIXEDFRAME );
    }
    else if ( pFrame->mbBorder )
    {
        nFrameX = GetSystemMetrics( SM_CXBORDER );
        nFrameY = GetSystemMetrics( SM_CYBORDER );
    }
    else
    {
        nFrameX = 0;
        nFrameY = 0;
    }
    if ( pFrame->mbCaption )
        nCaptionY = GetSystemMetrics( SM_CYCAPTION );
    else
        nCaptionY = 0;

    try
    {
        sal_Int32 nMonitors = Application::GetScreenCount();
        if( (pFrame->mnDisplay >= 0) && (pFrame->mnDisplay < nMonitors) )
        {
            Rectangle aRect = Application::GetScreenPosSizePixel( pFrame->mnDisplay );
            nScreenX = aRect.Left();
            nScreenY = aRect.Top();
            nScreenDX = aRect.getWidth()+1;  // difference between java/awt convention and vcl
            nScreenDY = aRect.getHeight()+1; // difference between java/awt convention and vcl
        }
        else
        {
            Rectangle aCombined = Application::GetScreenPosSizePixel( 0 );
            for( sal_Int32 i = 1 ; i < nMonitors ; i++ )
            {
                aCombined.Union( Application::GetScreenPosSizePixel( i ) );
            }
            nScreenX  = aCombined.Left();
            nScreenY  = aCombined.Top();
            nScreenDX = aCombined.getWidth();
            nScreenDY = aCombined.getHeight();
        }
    }
    catch( Exception& )
    {
    }

    if( !nScreenDX || !nScreenDY )
    {
        nScreenDX   = GetSystemMetrics( SM_CXSCREEN );
        nScreenDY   = GetSystemMetrics( SM_CYSCREEN );
    }

    rX  = nScreenX -nFrameX;
    rY  = nScreenY -(nFrameY+nCaptionY);
    rDX = nScreenDX+(nFrameX*2);
    rDY = nScreenDY+(nFrameY*2)+nCaptionY;
}

static void ImplSalFrameFullScreenPos( WinSalFrame* pFrame, bool bAlways = FALSE )
{
    if ( bAlways || !IsIconic( pFrame->mhWnd ) )
    {
        // set window to screen size
        int nX;
        int nY;
        int nWidth;
        int nHeight;
        ImplSalCalcFullScreenSize( pFrame, nX, nY, nWidth, nHeight );
        SetWindowPos( pFrame->mhWnd, nullptr,
                      nX, nY, nWidth, nHeight,
                      SWP_NOZORDER | SWP_NOACTIVATE );
    }
}

namespace {

void SetForegroundWindow_Impl(HWND hwnd)
{
    static bool bUseForegroundWindow = !std::getenv("VCL_HIDE_WINDOWS");
    if (bUseForegroundWindow)
        SetForegroundWindow(hwnd);
}

}

WinSalFrame::WinSalFrame()
{
    SalData* pSalData = GetSalData();

    mhWnd               = nullptr;
    mhCursor            = LoadCursor( nullptr, IDC_ARROW );
    mhDefIMEContext     = nullptr;
    mpGraphics          = nullptr;
    mpGraphics2         = nullptr;
    mnShowState         = SW_SHOWNORMAL;
    mnWidth             = 0;
    mnHeight            = 0;
    mnMinWidth          = 0;
    mnMinHeight         = 0;
    mnMaxWidth          = SHRT_MAX;
    mnMaxHeight         = SHRT_MAX;
    mnInputLang         = 0;
    mnInputCodePage     = 0;
    mbGraphics          = FALSE;
    mbCaption           = FALSE;
    mbBorder            = FALSE;
    mbFixBorder         = FALSE;
    mbSizeBorder        = FALSE;
    mbFullScreen        = false;
    mbPresentation      = FALSE;
    mbInShow            = FALSE;
    mbRestoreMaximize   = FALSE;
    mbInMoveMsg         = FALSE;
    mbInSizeMsg         = FALSE;
    mbFullScreenToolWin = FALSE;
    mbDefPos            = TRUE;
    mbOverwriteState    = TRUE;
    mbIME               = FALSE;
    mbHandleIME         = FALSE;
    mbSpezIME           = FALSE;
    mbAtCursorIME       = FALSE;
    mbCandidateMode     = FALSE;
    mbFloatWin          = FALSE;
    mbNoIcon            = FALSE;
    mSelectedhMenu      = nullptr;
    mLastActivatedhMenu = nullptr;
    mpClipRgnData       = nullptr;
    mbFirstClipRect     = TRUE;
    mpNextClipRect      = nullptr;
    mnDisplay           = 0;
    mbPropertiesStored  = FALSE;

    memset( &maState, 0, sizeof( SalFrameState ) );
    maSysData.nSize     = sizeof( SystemEnvData );

    memset( &maGeometry, 0, sizeof( maGeometry ) );

    // get data, when making 1st frame
    if ( !pSalData->mpFirstFrame )
    {
        if ( !aSalShlData.mnWheelScrollLines )
            aSalShlData.mnWheelScrollLines = ImplSalGetWheelScrollLines();
        if ( !aSalShlData.mnWheelScrollChars )
            aSalShlData.mnWheelScrollChars = ImplSalGetWheelScrollChars();
    }

    // insert frame in framelist
    mpNextFrame = pSalData->mpFirstFrame;
    pSalData->mpFirstFrame = this;
}

void WinSalFrame::updateScreenNumber()
{
    if( mnDisplay == -1 ) // spans all monitors
        return;
    WinSalSystem* pSys = static_cast<WinSalSystem*>(ImplGetSalSystem());
    if( pSys )
    {
        const std::vector<WinSalSystem::DisplayMonitor>& rMonitors =
            pSys->getMonitors();
        Point aPoint( maGeometry.nX, maGeometry.nY );
        size_t nMon = rMonitors.size();
        for( size_t i = 0; i < nMon; i++ )
        {
            if( rMonitors[i].m_aArea.IsInside( aPoint ) )
            {
                mnDisplay = static_cast<sal_Int32>(i);
                maGeometry.nDisplayScreenNumber = static_cast<unsigned int>(i);
            }
        }
    }
}

WinSalFrame::~WinSalFrame()
{
    SalData* pSalData = GetSalData();

    if( mpClipRgnData )
        delete [] reinterpret_cast<BYTE*>(mpClipRgnData);

    // remove frame from framelist
    WinSalFrame** ppFrame = &pSalData->mpFirstFrame;
    for(; (*ppFrame != this) && *ppFrame; ppFrame = &(*ppFrame)->mpNextFrame );
    if( *ppFrame )
        *ppFrame = mpNextFrame;
    mpNextFrame = nullptr;

    // Release Cache DC
    if ( mpGraphics2 &&
         mpGraphics2->getHDC() )
        ReleaseGraphics( mpGraphics2 );

    // destroy saved DC
    if ( mpGraphics )
    {
        if ( mpGraphics->getDefPal() )
            SelectPalette( mpGraphics->getHDC(), mpGraphics->getDefPal(), TRUE );
        mpGraphics->DeInitGraphics();
        ReleaseDC( mhWnd, mpGraphics->getHDC() );
        delete mpGraphics;
        mpGraphics = nullptr;
    }

    if ( mhWnd )
    {
        // reset mouse leave data
        if ( pSalData->mhWantLeaveMsg == mhWnd )
        {
            pSalData->mhWantLeaveMsg = nullptr;
            if ( pSalData->mpMouseLeaveTimer )
            {
                delete pSalData->mpMouseLeaveTimer;
                pSalData->mpMouseLeaveTimer = nullptr;
            }
        }

        // remove windows properties
        if ( mbPropertiesStored )
            SetApplicationID( OUString() );

        // destroy system frame
        if ( !DestroyWindow( mhWnd ) )
            SetWindowPtr( mhWnd, nullptr );

        mhWnd = nullptr;
    }
}

SalGraphics* WinSalFrame::AcquireGraphics()
{
    if ( mbGraphics )
        return nullptr;

    // Other threads get an own DC, because Windows modify in the
    // other case our DC (changing clip region), when they send a
    // WM_ERASEBACKGROUND message
    SalData* pSalData = GetSalData();
    if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
    {
        // We use only three CacheDC's for all threads, because W9x is limited
        // to max. 5 Cache DC's per thread
        if ( pSalData->mnCacheDCInUse >= 3 )
            return nullptr;

        if ( !mpGraphics2 )
        {
            mpGraphics2 = new WinSalGraphics(WinSalGraphics::WINDOW, true, mhWnd, this);
            mpGraphics2->setHDC(nullptr);
        }

        HDC hDC = reinterpret_cast<HDC>((sal_IntPtr)SendMessageW( pSalData->mpFirstInstance->mhComWnd,
                                        SAL_MSG_GETDC,
                                        reinterpret_cast<WPARAM>(mhWnd), 0 ));
        if ( hDC )
        {
            mpGraphics2->setHDC(hDC);
            if ( pSalData->mhDitherPal )
            {
                mpGraphics2->setDefPal(SelectPalette( hDC, pSalData->mhDitherPal, TRUE ));
                RealizePalette( hDC );
            }
            mpGraphics2->InitGraphics();
            mbGraphics = TRUE;

            pSalData->mnCacheDCInUse++;
            return mpGraphics2;
        }
        else
            return nullptr;
    }
    else
    {
        if ( !mpGraphics )
        {
            HDC hDC = GetDC( mhWnd );
            if ( hDC )
            {
                mpGraphics = new WinSalGraphics(WinSalGraphics::WINDOW, true, mhWnd, this);
                mpGraphics->setHDC(hDC);
                if ( pSalData->mhDitherPal )
                {
                    mpGraphics->setDefPal(SelectPalette( hDC, pSalData->mhDitherPal, TRUE ));
                    RealizePalette( hDC );
                }
                mpGraphics->InitGraphics();
                mbGraphics = TRUE;
            }
        }
        else
            mbGraphics = TRUE;

        return mpGraphics;
    }
}

void WinSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    if ( mpGraphics2 == pGraphics )
    {
        if ( mpGraphics2->getHDC() )
        {
            SalData* pSalData = GetSalData();
            if ( mpGraphics2->getDefPal() )
                SelectPalette( mpGraphics2->getHDC(), mpGraphics2->getDefPal(), TRUE );
            mpGraphics2->DeInitGraphics();
            SendMessageW( pSalData->mpFirstInstance->mhComWnd,
                             SAL_MSG_RELEASEDC,
                             reinterpret_cast<WPARAM>(mhWnd),
                             reinterpret_cast<LPARAM>(mpGraphics2->getHDC()) );
            mpGraphics2->setHDC(nullptr);
            pSalData->mnCacheDCInUse--;
        }
    }

    mbGraphics = FALSE;
}

bool WinSalFrame::PostEvent(ImplSVEvent* pData)
{
    BOOL const ret = PostMessageW(mhWnd, SAL_MSG_USEREVENT, 0, reinterpret_cast<LPARAM>(pData));
    SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    return static_cast<bool>(ret);
}

void WinSalFrame::SetTitle( const OUString& rTitle )
{
    static_assert( sizeof( WCHAR ) == sizeof( sal_Unicode ), "must be the same size" );

    SetWindowTextW( mhWnd, reinterpret_cast<LPCWSTR>(rTitle.getStr()) );
}

void WinSalFrame::SetIcon( sal_uInt16 nIcon )
{
    // If we have a window without an Icon (for example a dialog), ignore this call
    if ( mbNoIcon )
        return;

    // 0 means default (class) icon
    HICON hIcon = nullptr, hSmIcon = nullptr;
    if ( !nIcon )
        nIcon = 1;

    ImplLoadSalIcon( nIcon, hIcon, hSmIcon );

    SAL_WARN_IF( !hIcon , "vcl",   "WinSalFrame::SetIcon(): Could not load large icon !" );
    SAL_WARN_IF( !hSmIcon , "vcl", "WinSalFrame::SetIcon(): Could not load small icon !" );

    SendMessageW( mhWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon) );
    SendMessageW( mhWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hSmIcon) );
}

void WinSalFrame::SetMenu( SalMenu* pSalMenu )
{
    WinSalMenu* pWMenu = static_cast<WinSalMenu*>(pSalMenu);
    if( pSalMenu && pWMenu->mbMenuBar )
        ::SetMenu( mhWnd, pWMenu->mhMenu );
}

void WinSalFrame::DrawMenuBar()
{
    ::DrawMenuBar( mhWnd );
}

HWND ImplGetParentHwnd( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if( !pFrame || !pFrame->GetWindow())
        return ::GetParent( hWnd );
    vcl::Window *pRealParent = pFrame->GetWindow()->ImplGetWindowImpl()->mpRealParent;
    if( pRealParent )
        return static_cast<WinSalFrame*>(pRealParent->ImplGetWindowImpl()->mpFrame)->mhWnd;
    else
        return ::GetParent( hWnd );

}

SalFrame* WinSalFrame::GetParent() const
{
    return GetWindowPtr( ImplGetParentHwnd( mhWnd ) );
}

static void ImplSalShow( HWND hWnd, bool bVisible, bool bNoActivate )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return;

    if ( bVisible )
    {
        pFrame->mbDefPos = FALSE;
        pFrame->mbOverwriteState = TRUE;
        pFrame->mbInShow = TRUE;

        // #i4715, save position
        RECT aRectPreMatrox, aRectPostMatrox;
        GetWindowRect( hWnd, &aRectPreMatrox );

        vcl::DeletionListener aDogTag( pFrame );
        if( bNoActivate )
            ShowWindow( hWnd, SW_SHOWNOACTIVATE );
        else
            ShowWindow( hWnd, pFrame->mnShowState );
        if( aDogTag.isDeleted() )
            return;

        if ( aSalShlData.mbWXP && pFrame->mbFloatWin && !(pFrame->mnStyle & SalFrameStyleFlags::NOSHADOW))
        {
            // erase the window immediately to improve XP shadow effect
            // otherwise the shadow may appears long time before the rest of the window
            // especially when accessibility is on
            HDC dc = GetDC( hWnd );
            RECT aRect;
            GetClientRect( hWnd, &aRect );
            FillRect( dc, &aRect, reinterpret_cast<HBRUSH>(COLOR_MENU+1) ); // choose the menucolor, because its mostly noticeable for menus
            ReleaseDC( hWnd, dc );
        }

        // #i4715, matrox centerpopup might have changed our position
        // reposition popups without caption (menus, dropdowns, tooltips)
        GetWindowRect( hWnd, &aRectPostMatrox );
        if( (GetWindowStyle( hWnd ) & WS_POPUP) &&
            !pFrame->mbCaption &&
            (aRectPreMatrox.left != aRectPostMatrox.left || aRectPreMatrox.top != aRectPostMatrox.top) )
            SetWindowPos( hWnd, nullptr, aRectPreMatrox.left, aRectPreMatrox.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE  );

        if( aDogTag.isDeleted() )
            return;
        vcl::Window *pClientWin = pFrame->GetWindow()->ImplGetClientWindow();
        if ( pFrame->mbFloatWin || ( pClientWin && (pClientWin->GetStyle() & WB_SYSTEMFLOATWIN) ) )
            pFrame->mnShowState = SW_SHOWNOACTIVATE;
        else
            pFrame->mnShowState = SW_SHOW;
        // hide toolbar for W98
        if ( pFrame->mbPresentation )
        {
            HWND hWndParent = ::GetParent( hWnd );
            if ( hWndParent )
                SetForegroundWindow_Impl( hWndParent );
            SetForegroundWindow_Impl( hWnd );
        }

        pFrame->mbInShow = FALSE;
        pFrame->updateScreenNumber();

        // Direct Paint only, if we get the SolarMutx
        if ( ImplSalYieldMutexTryToAcquire() )
        {
            UpdateWindow( hWnd );
            ImplSalYieldMutexRelease();
        }
    }
    else
    {
        ShowWindow( hWnd, SW_HIDE );
    }
}

void WinSalFrame::SetExtendedFrameStyle( SalExtStyle )
{
}

void WinSalFrame::Show( bool bVisible, bool bNoActivate )
{
    // Post this Message to the window, because this only works
    // in the thread of the window, which has create this window.
    // We post this message to avoid deadlocks
    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
    {
        BOOL const ret = PostMessageW(mhWnd, SAL_MSG_SHOW, WPARAM(bVisible), LPARAM(bNoActivate));
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
    else
        ImplSalShow( mhWnd, bVisible, bNoActivate );
}

void WinSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    mnMinWidth  = nWidth;
    mnMinHeight = nHeight;
}

void WinSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    mnMaxWidth  = nWidth;
    mnMaxHeight = nHeight;
}

void WinSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight,
                                                   sal_uInt16 nFlags )
{
    bool bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    if ( !bVisible )
    {
        vcl::Window *pClientWin = GetWindow()->ImplGetClientWindow();
        if ( mbFloatWin || ( pClientWin && (pClientWin->GetStyle() & WB_SYSTEMFLOATWIN) ) )
                mnShowState = SW_SHOWNOACTIVATE;
        else
                mnShowState = SW_SHOWNORMAL;
    }
    else
    {
        if ( IsIconic( mhWnd ) || IsZoomed( mhWnd ) )
                ShowWindow( mhWnd, SW_RESTORE );
    }

    SalEvent nEvent = SalEvent::NONE;
    UINT    nPosSize = 0;
    RECT    aClientRect, aWindowRect;
    GetClientRect( mhWnd, &aClientRect );   // x,y always 0,0, but width and height without border
    GetWindowRect( mhWnd, &aWindowRect );   // x,y in screen coordinates, width and height with border

    if ( !(nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y)) )
        nPosSize |= SWP_NOMOVE;
    else
    {
        //SAL_WARN_IF( !nX || !nY, "vcl", " Windowposition of (0,0) requested!" );
        nEvent = SalEvent::Move;
    }
    if ( !(nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT)) )
        nPosSize |= SWP_NOSIZE;
    else
        nEvent = (nEvent == SalEvent::Move) ? SalEvent::MoveResize : SalEvent::Resize;

    if ( !(nFlags & SAL_FRAME_POSSIZE_X) )
        nX = aWindowRect.left;
    if ( !(nFlags & SAL_FRAME_POSSIZE_Y) )
        nY = aWindowRect.top;
    if ( !(nFlags & SAL_FRAME_POSSIZE_WIDTH) )
        nWidth = aClientRect.right-aClientRect.left;
    if ( !(nFlags & SAL_FRAME_POSSIZE_HEIGHT) )
        nHeight = aClientRect.bottom-aClientRect.top;

    // Calculate window size including the border
    RECT    aWinRect;
    aWinRect.left   = 0;
    aWinRect.right  = (int)nWidth-1;
    aWinRect.top    = 0;
    aWinRect.bottom = (int)nHeight-1;
    AdjustWindowRectEx( &aWinRect, GetWindowStyle( mhWnd ),
                        FALSE,     GetWindowExStyle( mhWnd ) );
    nWidth  = aWinRect.right - aWinRect.left + 1;
    nHeight = aWinRect.bottom - aWinRect.top + 1;

    if ( !(nPosSize & SWP_NOMOVE) && ::GetParent( mhWnd ) )
    {
            // --- RTL --- (mirror window pos)
            RECT aParentRect;
            GetClientRect( ImplGetParentHwnd( mhWnd ), &aParentRect );
            if( AllSettings::GetLayoutRTL() )
                nX = (aParentRect.right - aParentRect.left) - nWidth-1 - nX;

            //#110386#, do not transform coordinates for system child windows
            if( !(GetWindowStyle( mhWnd ) & WS_CHILD) )
            {
                POINT aPt;
                aPt.x = nX;
                aPt.y = nY;

                HWND parentHwnd = ImplGetParentHwnd( mhWnd );
                WinSalFrame* pParentFrame = GetWindowPtr( parentHwnd );
                if ( pParentFrame && pParentFrame->mnShowState == SW_SHOWMAXIMIZED )
                {
                    // #i42485#: parent will be shown maximized in which case
                    // a ClientToScreen uses the wrong coordinates (i.e. those from the restore pos)
                    // so use the (already updated) frame geometry for the transformation
                    aPt.x +=  pParentFrame->maGeometry.nX;
                    aPt.y +=  pParentFrame->maGeometry.nY;
                }
                else
                    ClientToScreen( parentHwnd, &aPt );

                nX = aPt.x;
                nY = aPt.y;

                // the position is set
                mbDefPos = false;
            }
    }

    // #i3338# to be conformant to UNIX we must position the client window, ie without the decoration
    // #i43250# if the position was read from the system (GetWindowRect(), see above), it must not be modified
    if ( nFlags & SAL_FRAME_POSSIZE_X )
        nX += aWinRect.left;
    if ( nFlags & SAL_FRAME_POSSIZE_Y )
        nY += aWinRect.top;

    int     nScreenX;
    int     nScreenY;
    int     nScreenWidth;
    int     nScreenHeight;

    RECT aRect;
    ImplSalGetWorkArea( mhWnd, &aRect, nullptr );
    nScreenX        = aRect.left;
    nScreenY        = aRect.top;
    nScreenWidth    = aRect.right-aRect.left;
    nScreenHeight   = aRect.bottom-aRect.top;

    if ( mbDefPos && (nPosSize & SWP_NOMOVE)) // we got no positioning request, so choose default position
    {
        // center window

        HWND hWndParent = ::GetParent( mhWnd );
        // Search for TopLevel Frame
        while ( hWndParent && (GetWindowStyle( hWndParent ) & WS_CHILD) )
            hWndParent = ::GetParent( hWndParent );
        // if the Window has a Parent, than center the window to
        // the parent, in the other case to the screen
        if ( hWndParent && !IsIconic( hWndParent ) &&
             (GetWindowStyle( hWndParent ) & WS_VISIBLE) )
        {
            RECT aParentRect;
            GetWindowRect( hWndParent, &aParentRect );
            int nParentWidth    = aParentRect.right-aParentRect.left;
            int nParentHeight   = aParentRect.bottom-aParentRect.top;

            // We don't center, when Parent is smaller than our window
            if ( (nParentWidth-GetSystemMetrics( SM_CXFIXEDFRAME ) <= nWidth) &&
                 (nParentHeight-GetSystemMetrics( SM_CYFIXEDFRAME ) <= nHeight) )
            {
                int nOff = GetSystemMetrics( SM_CYSIZEFRAME ) + GetSystemMetrics( SM_CYCAPTION );
                nX = aParentRect.left+nOff;
                nY = aParentRect.top+nOff;
            }
            else
            {
                nX = (nParentWidth-nWidth)/2 + aParentRect.left;
                nY = (nParentHeight-nHeight)/2 + aParentRect.top;
            }
        }
        else
        {
            POINT pt;
            GetCursorPos( &pt );
            RECT aRect2;
            aRect2.left = pt.x;
            aRect2.top = pt.y;
            aRect2.right = pt.x+2;
            aRect2.bottom = pt.y+2;

            // dualmonitor support:
            // Get screensize of the monitor with the mouse pointer
            ImplSalGetWorkArea( mhWnd, &aRect2, &aRect2 );

            nX = ((aRect2.right-aRect2.left)-nWidth)/2 + aRect2.left;
            nY = ((aRect2.bottom-aRect2.top)-nHeight)/2 + aRect2.top;
        }

        //if ( bVisible )
        //    mbDefPos = FALSE;

        mbDefPos = FALSE;   // center only once
        nPosSize &= ~SWP_NOMOVE;        // activate positioning
        nEvent = SalEvent::MoveResize;
    }

    // Adjust Window in the screen
    bool bCheckOffScreen = TRUE;

    // but don't do this for floaters or ownerdraw windows that are currently moved interactively
    if( (mnStyle & SalFrameStyleFlags::FLOAT) && !(mnStyle & SalFrameStyleFlags::OWNERDRAWDECORATION) )
        bCheckOffScreen = FALSE;

    if( mnStyle & SalFrameStyleFlags::OWNERDRAWDECORATION )
    {
        // may be the window is currently being moved (mouse is captured), then no check is required
        if( mhWnd == ::GetCapture() )
            bCheckOffScreen = FALSE;
        else
            bCheckOffScreen = TRUE;
    }

    if( bCheckOffScreen )
    {
        if ( nX+nWidth > nScreenX+nScreenWidth )
            nX = (nScreenX+nScreenWidth) - nWidth;
        if ( nY+nHeight > nScreenY+nScreenHeight )
            nY = (nScreenY+nScreenHeight) - nHeight;
        if ( nX < nScreenX )
            nX = nScreenX;
        if ( nY < nScreenY )
            nY = nScreenY;
    }

    UINT nPosFlags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | nPosSize;
    // bring floating windows always to top
    if( !(mnStyle & SalFrameStyleFlags::FLOAT) )
        nPosFlags |= SWP_NOZORDER; // do not change z-order

    SetWindowPos( mhWnd, HWND_TOP, nX, nY, (int)nWidth, (int)nHeight, nPosFlags  );

    UpdateFrameGeometry( mhWnd, this );

    // Notification -- really ???
    if( nEvent != SalEvent::NONE )
        CallCallback( nEvent, nullptr );
}

static void ImplSetParentFrame( WinSalFrame* pThis, HWND hNewParentWnd, bool bAsChild )
{
    // save hwnd, will be overwritten in WM_CREATE during createwindow
    HWND hWndOld = pThis->mhWnd;
    HWND hWndOldParent = ::GetParent( hWndOld );
    SalData* pSalData = GetSalData();

    if( hNewParentWnd == hWndOldParent )
        return;

    ::std::vector< WinSalFrame* > children;
    ::std::vector< WinSalObject* > systemChildren;

    // search child windows
    WinSalFrame *pFrame = pSalData->mpFirstFrame;
    while( pFrame )
    {
        HWND hWndParent = ::GetParent( pFrame->mhWnd );
        if( pThis->mhWnd == hWndParent )
            children.push_back( pFrame );
        pFrame = pFrame->mpNextFrame;
    }

    // search system child windows (plugins etc.)
    WinSalObject *pObject = pSalData->mpFirstObject;
    while( pObject )
    {
        HWND hWndParent = ::GetParent( pObject->mhWnd );
        if( pThis->mhWnd == hWndParent )
            systemChildren.push_back( pObject );
        pObject = pObject->mpNextObject;
    }

    bool bNeedGraphics = pThis->mbGraphics;
    bool bNeedCacheDC  = FALSE;

    HFONT   hFont   = nullptr;
    HPEN    hPen    = nullptr;
    HBRUSH  hBrush  = nullptr;

    int oldCount = pSalData->mnCacheDCInUse;

    // Release Cache DC
    if ( pThis->mpGraphics2 &&
         pThis->mpGraphics2->getHDC() )
    {
        // save current gdi objects before hdc is gone
        hFont   = static_cast<HFONT>(GetCurrentObject( pThis->mpGraphics2->getHDC(), OBJ_FONT));
        hPen    = static_cast<HPEN>(GetCurrentObject( pThis->mpGraphics2->getHDC(), OBJ_PEN));
        hBrush  = static_cast<HBRUSH>(GetCurrentObject( pThis->mpGraphics2->getHDC(), OBJ_BRUSH));
        pThis->ReleaseGraphics( pThis->mpGraphics2 );

        // recreate cache dc only if it was destroyed
        bNeedCacheDC  = TRUE;
    }

    // destroy saved DC
    if ( pThis->mpGraphics )
    {
        if ( pThis->mpGraphics->getDefPal() )
            SelectPalette( pThis->mpGraphics->getHDC(), pThis->mpGraphics->getDefPal(), TRUE );
        pThis->mpGraphics->DeInitGraphics();
        ReleaseDC( pThis->mhWnd, pThis->mpGraphics->getHDC() );
    }

    // create a new hwnd with the same styles
    HWND hWndParent = hNewParentWnd;
    // forward to main thread
    HWND hWnd = reinterpret_cast<HWND>((sal_IntPtr) SendMessageW( pSalData->mpFirstInstance->mhComWnd,
                                        bAsChild ? SAL_MSG_RECREATECHILDHWND : SAL_MSG_RECREATEHWND,
                                        reinterpret_cast<WPARAM>(hWndParent), reinterpret_cast<LPARAM>(pThis->mhWnd) ));

    // succeeded ?
    SAL_WARN_IF( !IsWindow( hWnd ), "vcl", "WinSalFrame::SetParent not successful");

    // recreate DCs
    if( bNeedGraphics )
    {
        if( pThis->mpGraphics2 )
        {
            pThis->mpGraphics2->setHWND(hWnd);

            if( bNeedCacheDC )
            {
                // re-create cached DC
                HDC hDC = reinterpret_cast<HDC>((sal_IntPtr)SendMessageW( pSalData->mpFirstInstance->mhComWnd,
                                                SAL_MSG_GETDC,
                                                reinterpret_cast<WPARAM>(hWnd), 0 ));
                if ( hDC )
                {
                    pThis->mpGraphics2->setHDC(hDC);
                    if ( pSalData->mhDitherPal )
                    {
                        pThis->mpGraphics2->setDefPal(SelectPalette( hDC, pSalData->mhDitherPal, TRUE ));
                        RealizePalette( hDC );
                    }
                    pThis->mpGraphics2->InitGraphics();

                    // re-select saved gdi objects
                    if( hFont )
                        SelectObject( hDC, hFont );
                    if( hPen )
                        SelectObject( hDC, hPen );
                    if( hBrush )
                        SelectObject( hDC, hBrush );

                    pThis->mbGraphics = TRUE;

                    pSalData->mnCacheDCInUse++;

                    SAL_WARN_IF( oldCount != pSalData->mnCacheDCInUse, "vcl", "WinSalFrame::SetParent() hDC count corrupted");
                }
            }
        }

        if( pThis->mpGraphics )
        {
            // re-create DC
            pThis->mpGraphics->setHWND(hWnd);
            pThis->mpGraphics->setHDC( GetDC( hWnd ) );
            if ( GetSalData()->mhDitherPal )
            {
                pThis->mpGraphics->setDefPal(SelectPalette( pThis->mpGraphics->getHDC(), GetSalData()->mhDitherPal, TRUE ));
                RealizePalette( pThis->mpGraphics->getHDC() );
            }
            pThis->mpGraphics->InitGraphics();
            pThis->mbGraphics = TRUE;
        }
    }

    // TODO: add SetParent() call for SalObjects
    SAL_WARN_IF( !systemChildren.empty(), "vcl", "WinSalFrame::SetParent() parent of living system child window will be destroyed!");

    // reparent children before old parent is destroyed
    for( ::std::vector< WinSalFrame* >::iterator iChild = children.begin(); iChild != children.end(); ++iChild )
        ImplSetParentFrame( *iChild, hWnd, false );

    children.clear();
    systemChildren.clear();

    // Now destroy original HWND in the thread where it was created.
    SendMessageW( GetSalData()->mpFirstInstance->mhComWnd,
                     SAL_MSG_DESTROYHWND, (WPARAM) 0, reinterpret_cast<LPARAM>(hWndOld));
}

void WinSalFrame::SetParent( SalFrame* pNewParent )
{
    WinSalFrame::mbInReparent = TRUE;
    ImplSetParentFrame( this, static_cast<WinSalFrame*>(pNewParent)->mhWnd, false );
    WinSalFrame::mbInReparent = FALSE;
}

bool WinSalFrame::SetPluginParent( SystemParentData* pNewParent )
{
    if ( pNewParent->hWnd == nullptr )
    {
        pNewParent->hWnd = GetDesktopWindow();
    }

    WinSalFrame::mbInReparent = TRUE;
    ImplSetParentFrame( this, pNewParent->hWnd, true );
    WinSalFrame::mbInReparent = FALSE;
    return true;
}

void WinSalFrame::GetWorkArea( Rectangle &rRect )
{
    RECT aRect;
    ImplSalGetWorkArea( mhWnd, &aRect, nullptr );
    rRect.Left()     = aRect.left;
    rRect.Right()    = aRect.right-1;
    rRect.Top()      = aRect.top;
    rRect.Bottom()   = aRect.bottom-1;
}

void WinSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    rWidth  = maGeometry.nWidth;
    rHeight = maGeometry.nHeight;
}

void WinSalFrame::SetWindowState( const SalFrameState* pState )
{
    // Check if the window fits into the screen, in case the screen
    // resolution changed
    int     nX;
    int     nY;
    int     nWidth;
    int     nHeight;
    int     nScreenX;
    int     nScreenY;
    int     nScreenWidth;
    int     nScreenHeight;

    RECT aRect;
    ImplSalGetWorkArea( mhWnd, &aRect, nullptr );
    // #102500# allow some overlap, the window could have been made a little larger than the physical screen
    nScreenX        = aRect.left-10;
    nScreenY        = aRect.top-10;
    nScreenWidth    = aRect.right-aRect.left+20;
    nScreenHeight   = aRect.bottom-aRect.top+20;

    UINT    nPosSize    = 0;
    RECT    aWinRect;
    GetWindowRect( mhWnd, &aWinRect );

    // to be consistent with Unix, the frame state is without(!) decoration
    // ->add the decoration
    RECT aRect2 = aWinRect;
    AdjustWindowRectEx( &aRect2, GetWindowStyle( mhWnd ),
                    FALSE,     GetWindowExStyle( mhWnd ) );
    long nTopDeco = abs( aWinRect.top - aRect2.top );
    long nLeftDeco = abs( aWinRect.left - aRect2.left );
    long nBottomDeco = abs( aWinRect.bottom - aRect2.bottom );
    long nRightDeco = abs( aWinRect.right - aRect2.right );

    // adjust window position/size to fit the screen
    if ( !(pState->mnMask & (WindowStateMask::X | WindowStateMask::Y)) )
        nPosSize |= SWP_NOMOVE;
    if ( !(pState->mnMask & (WindowStateMask::Width | WindowStateMask::Height)) )
        nPosSize |= SWP_NOSIZE;
    if ( pState->mnMask & WindowStateMask::X )
        nX = (int)pState->mnX - nLeftDeco;
    else
        nX = aWinRect.left;
    if ( pState->mnMask & WindowStateMask::Y )
        nY = (int)pState->mnY - nTopDeco;
    else
        nY = aWinRect.top;
    if ( pState->mnMask & WindowStateMask::Width )
        nWidth = (int)pState->mnWidth + nLeftDeco + nRightDeco;
    else
        nWidth = aWinRect.right-aWinRect.left;
    if ( pState->mnMask & WindowStateMask::Height )
        nHeight = (int)pState->mnHeight + nTopDeco + nBottomDeco;
    else
        nHeight = aWinRect.bottom-aWinRect.top;

    // Adjust Window in the screen:
    // if it does not fit into the screen do nothing, ie default pos/size will be used
    // if there is an overlap with the screen border move the window while keeping its size

    if( nWidth > nScreenWidth || nHeight > nScreenHeight )
        nPosSize |= (SWP_NOMOVE | SWP_NOSIZE);

    if ( nX+nWidth > nScreenX+nScreenWidth )
        nX = (nScreenX+nScreenWidth) - nWidth;
    if ( nY+nHeight > nScreenY+nScreenHeight )
        nY = (nScreenY+nScreenHeight) - nHeight;
    if ( nX < nScreenX )
        nX = nScreenX;
    if ( nY < nScreenY )
        nY = nScreenY;

    // set Restore-Position
    WINDOWPLACEMENT aPlacement;
    aPlacement.length = sizeof( aPlacement );
    GetWindowPlacement( mhWnd, &aPlacement );

    // set State
    bool bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    bool bUpdateHiddenFramePos = FALSE;
    if ( !bVisible )
    {
        aPlacement.showCmd = SW_HIDE;

        if ( mbOverwriteState )
        {
            if ( pState->mnMask & WindowStateMask::State )
            {
                if ( pState->mnState & WindowStateState::Minimized )
                    mnShowState = SW_SHOWMINIMIZED;
                else if ( pState->mnState & WindowStateState::Maximized )
                {
                    mnShowState = SW_SHOWMAXIMIZED;
                    bUpdateHiddenFramePos = TRUE;
                }
                else if ( pState->mnState & WindowStateState::Normal )
                    mnShowState = SW_SHOWNORMAL;
            }
        }
    }
    else
    {
        if ( pState->mnMask & WindowStateMask::State )
        {
            if ( pState->mnState & WindowStateState::Minimized )
            {
                if ( pState->mnState & WindowStateState::Maximized )
                    aPlacement.flags |= WPF_RESTORETOMAXIMIZED;
                aPlacement.showCmd = SW_SHOWMINIMIZED;
            }
            else if ( pState->mnState & WindowStateState::Maximized )
                aPlacement.showCmd = SW_SHOWMAXIMIZED;
            else if ( pState->mnState & WindowStateState::Normal )
                aPlacement.showCmd = SW_RESTORE;
        }
    }

    // if a window is neither minimized nor maximized or need not be
    // positioned visibly (that is in visible state), do not use
    // SetWindowPlacement since it calculates including the TaskBar
    if ( !IsIconic( mhWnd ) && !IsZoomed( mhWnd ) &&
         (!bVisible || (aPlacement.showCmd == SW_RESTORE)) )
    {
        if( bUpdateHiddenFramePos )
        {
            RECT aStateRect;
            aStateRect.left   = nX;
            aStateRect.top    = nY;
            aStateRect.right  = nX+nWidth;
            aStateRect.bottom = nY+nHeight;
            // #96084 set a useful internal window size because
            // the window will not be maximized (and the size updated) before show()
            SetMaximizedFrameGeometry( mhWnd, this, &aStateRect );
            SetWindowPos( mhWnd, nullptr,
                          maGeometry.nX, maGeometry.nY, maGeometry.nWidth, maGeometry.nHeight,
                          SWP_NOZORDER | SWP_NOACTIVATE | nPosSize );
        }
        else
            SetWindowPos( mhWnd, nullptr,
                      nX, nY, nWidth, nHeight,
                      SWP_NOZORDER | SWP_NOACTIVATE | nPosSize );
    }
    else
    {
        if( !(nPosSize & (SWP_NOMOVE|SWP_NOSIZE)) )
        {
            aPlacement.rcNormalPosition.left    = nX-nScreenX;
            aPlacement.rcNormalPosition.top     = nY-nScreenY;
            aPlacement.rcNormalPosition.right   = nX+nWidth-nScreenX;
            aPlacement.rcNormalPosition.bottom  = nY+nHeight-nScreenY;
        }
        SetWindowPlacement( mhWnd, &aPlacement );
    }

    if( !(nPosSize & SWP_NOMOVE) )
        mbDefPos = FALSE; // window was positioned
}

bool WinSalFrame::GetWindowState( SalFrameState* pState )
{
    if ( maState.mnWidth && maState.mnHeight )
    {
        *pState = maState;
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - Don't save minimize
        //if ( !(pState->mnState & WindowStateState::Maximized) )
        if ( !(pState->mnState & (WindowStateState::Minimized | WindowStateState::Maximized)) )
            pState->mnState |= WindowStateState::Normal;
        return TRUE;
    }

    return FALSE;
}

void WinSalFrame::SetScreenNumber( unsigned int nNewScreen )
{
    WinSalSystem* pSys = static_cast<WinSalSystem*>(ImplGetSalSystem());
    if( pSys )
    {
        const std::vector<WinSalSystem::DisplayMonitor>& rMonitors =
            pSys->getMonitors();
        size_t nMon = rMonitors.size();
        if( nNewScreen < nMon )
        {
            Point aOldMonPos, aNewMonPos( rMonitors[nNewScreen].m_aArea.TopLeft() );
            Point aCurPos( maGeometry.nX, maGeometry.nY );
            for( size_t i = 0; i < nMon; i++ )
            {
                if( rMonitors[i].m_aArea.IsInside( aCurPos ) )
                {
                    aOldMonPos = rMonitors[i].m_aArea.TopLeft();
                    break;
                }
            }
            mnDisplay = nNewScreen;
            maGeometry.nDisplayScreenNumber = nNewScreen;
            SetPosSize( aNewMonPos.X() + (maGeometry.nX - aOldMonPos.X()),
                        aNewMonPos.Y() + (maGeometry.nY - aOldMonPos.Y()),
                        0, 0,
                        SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y );
        }
    }
}

void WinSalFrame::SetApplicationID( const OUString &rApplicationID )
{
    if ( aSalShlData.mbW7 )
    {
        // http://msdn.microsoft.com/en-us/library/windows/desktop/dd378430(v=vs.85).aspx
        // A window's properties must be removed before the window is closed.

        typedef HRESULT ( WINAPI *SHGETPROPERTYSTOREFORWINDOW )( HWND, REFIID, void ** );
        SHGETPROPERTYSTOREFORWINDOW pSHGetPropertyStoreForWindow;
        pSHGetPropertyStoreForWindow = reinterpret_cast<SHGETPROPERTYSTOREFORWINDOW>(GetProcAddress(
                                       GetModuleHandleW (L"shell32.dll"), "SHGetPropertyStoreForWindow" ));

        if( pSHGetPropertyStoreForWindow )
        {
            IPropertyStore *pps;
            HRESULT hr = pSHGetPropertyStoreForWindow ( mhWnd, IID_PPV_ARGS(&pps) );
            if ( SUCCEEDED(hr) )
            {
                PROPVARIANT pv;
                if ( !rApplicationID.isEmpty() )
                {
                    hr = InitPropVariantFromString( rApplicationID.getStr(), &pv );
                    mbPropertiesStored = TRUE;
                }
                else
                    // if rApplicationID we remove the property from the window, if present
                    PropVariantInit( &pv );

                if ( SUCCEEDED(hr) )
                {
                    hr = pps->SetValue( PKEY_AppUserModel_ID, pv );
                    PropVariantClear( &pv );
                }
                pps->Release();
            }
        }
    }
}

void WinSalFrame::ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay )
{
    if ( (mbFullScreen == bFullScreen) && (!bFullScreen || (mnDisplay == nDisplay)) )
        return;

    mbFullScreen = bFullScreen;
    mnDisplay = nDisplay;

    if ( bFullScreen )
    {
        // to hide the Windows taskbar
        DWORD nExStyle = GetWindowExStyle( mhWnd );
        if ( nExStyle & WS_EX_TOOLWINDOW )
        {
            mbFullScreenToolWin = TRUE;
            nExStyle &= ~WS_EX_TOOLWINDOW;
            SetWindowExStyle( mhWnd, nExStyle );
        }
        // save old position
        GetWindowRect( mhWnd, &maFullScreenRect );

        // save show state
        mnFullScreenShowState = mnShowState;
        if ( !(GetWindowStyle( mhWnd ) & WS_VISIBLE) )
            mnShowState = SW_SHOW;

        // set window to screen size
        ImplSalFrameFullScreenPos( this, true );
    }
    else
    {
        // when the ShowState has to be reset, hide the window first to
        // reduce flicker
        bool bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
        if ( bVisible && (mnShowState != mnFullScreenShowState) )
            ShowWindow( mhWnd, SW_HIDE );

        if ( mbFullScreenToolWin )
            SetWindowExStyle( mhWnd, GetWindowExStyle( mhWnd ) | WS_EX_TOOLWINDOW );
        mbFullScreenToolWin = FALSE;

        SetWindowPos( mhWnd, nullptr,
                      maFullScreenRect.left,
                      maFullScreenRect.top,
                      maFullScreenRect.right-maFullScreenRect.left,
                      maFullScreenRect.bottom-maFullScreenRect.top,
                      SWP_NOZORDER | SWP_NOACTIVATE );

        // restore show state
        if ( mnShowState != mnFullScreenShowState )
        {
            mnShowState = mnFullScreenShowState;
            if ( bVisible )
            {
                mbInShow = TRUE;
                ShowWindow( mhWnd, mnShowState );
                mbInShow = FALSE;
                UpdateWindow( mhWnd );
            }
        }
    }
}

void WinSalFrame::StartPresentation( bool bStart )
{
    if ( mbPresentation == bStart )
        return;

    mbPresentation = bStart;

    SalData* pSalData = GetSalData();
    if ( bStart )
    {
        // turn off screen-saver when in Presentation mode
        SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, 0,
                              &(pSalData->mbScrSvrEnabled), 0 );
        if ( pSalData->mbScrSvrEnabled )
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, nullptr, 0 );
    }
    else
    {
        // turn on screen-saver
        if ( pSalData->mbScrSvrEnabled )
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, pSalData->mbScrSvrEnabled, nullptr, 0 );
    }
}

void WinSalFrame::SetAlwaysOnTop( bool bOnTop )
{
    HWND hWnd;
    if ( bOnTop )
        hWnd = HWND_TOPMOST;
    else
        hWnd = HWND_NOTOPMOST;
    SetWindowPos( mhWnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
}

static void ImplSalToTop( HWND hWnd, SalFrameToTop nFlags )
{
    WinSalFrame* pToTopFrame = GetWindowPtr( hWnd );
    if( pToTopFrame && (pToTopFrame->mnStyle & SalFrameStyleFlags::SYSTEMCHILD) )
        BringWindowToTop( hWnd );

    if ( nFlags & SalFrameToTop::ForegroundTask )
    {
        // This magic code is necessary to connect the input focus of the
        // current window thread and the thread which owns the window that
        // should be the new foreground window.
        HWND   hCurrWnd     = GetForegroundWindow();
        DWORD  myThreadID   = GetCurrentThreadId();
        DWORD  currThreadID = GetWindowThreadProcessId(hCurrWnd,nullptr);
        AttachThreadInput(myThreadID, currThreadID,TRUE);
        SetForegroundWindow_Impl(hWnd);
        AttachThreadInput(myThreadID,currThreadID,FALSE);
    }

    if ( nFlags & SalFrameToTop::RestoreWhenMin )
    {
        HWND hIconicWnd = hWnd;
        while ( hIconicWnd )
        {
            if ( IsIconic( hIconicWnd ) )
            {
                WinSalFrame* pFrame = GetWindowPtr( hIconicWnd );
                if ( pFrame )
                {
                    if ( GetWindowPtr( hWnd )->mbRestoreMaximize )
                        ShowWindow( hIconicWnd, SW_MAXIMIZE );
                    else
                        ShowWindow( hIconicWnd, SW_RESTORE );
                }
                else
                    ShowWindow( hIconicWnd, SW_RESTORE );
            }

            hIconicWnd = ::GetParent( hIconicWnd );
        }
    }

    if ( !IsIconic( hWnd ) && IsWindowVisible( hWnd ) )
    {
        SetFocus( hWnd );

        // Windows sometimes incorrectly reports to have the focus;
        // thus make sure to really get the focus
        if ( ::GetFocus() == hWnd )
            SetForegroundWindow_Impl( hWnd );
    }
}

void WinSalFrame::ToTop( SalFrameToTop nFlags )
{
    nFlags &= ~SalFrameToTop::GrabFocus;   // this flag is not needed on win32
    // Post this Message to the window, because this only works
    // in the thread of the window, which has create this window.
    // We post this message to avoid deadlocks
    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
    {
        BOOL const ret = PostMessageW( mhWnd, SAL_MSG_TOTOP, (WPARAM)nFlags, 0 );
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
    else
        ImplSalToTop( mhWnd, nFlags );
}

void WinSalFrame::SetPointer( PointerStyle ePointerStyle )
{
    struct ImplPtrData
    {
        HCURSOR         mhCursor;
        LPCSTR          mnSysId;
        UINT            mnOwnId;
    };

    static o3tl::enumarray<PointerStyle, ImplPtrData> aImplPtrTab =
    {
    ImplPtrData{ nullptr, IDC_ARROW, 0 },                       // POINTER_ARROW
    { nullptr, nullptr, SAL_RESID_POINTER_NULL },               // POINTER_NULL
    { nullptr, IDC_WAIT, 0 },                                   // POINTER_WAIT
    { nullptr, IDC_IBEAM, 0 },                                  // POINTER_TEXT
    { nullptr, IDC_HELP, 0 },                                   // POINTER_HELP
    { nullptr, IDC_CROSS, 0 },                                  // POINTER_CROSS
    { nullptr, IDC_SIZEALL, 0 },                                // POINTER_MOVE
    { nullptr, IDC_SIZENS, 0 },                                 // POINTER_NSIZE
    { nullptr, IDC_SIZENS, 0 },                                 // POINTER_SSIZE
    { nullptr, IDC_SIZEWE, 0 },                                 // POINTER_WSIZE
    { nullptr, IDC_SIZEWE, 0 },                                 // POINTER_ESIZE
    { nullptr, IDC_SIZENWSE, 0 },                               // POINTER_NWSIZE
    { nullptr, IDC_SIZENESW, 0 },                               // POINTER_NESIZE
    { nullptr, IDC_SIZENESW, 0 },                               // POINTER_SWSIZE
    { nullptr, IDC_SIZENWSE, 0 },                               // POINTER_SESIZE
    { nullptr, IDC_SIZENS, 0 },                                 // POINTER_WINDOW_NSIZE
    { nullptr, IDC_SIZENS, 0 },                                 // POINTER_WINDOW_SSIZE
    { nullptr, IDC_SIZEWE, 0 },                                 // POINTER_WINDOW_WSIZE
    { nullptr, IDC_SIZEWE, 0 },                                 // POINTER_WINDOW_ESIZE
    { nullptr, IDC_SIZENWSE, 0 },                               // POINTER_WINDOW_NWSIZE
    { nullptr, IDC_SIZENESW, 0 },                               // POINTER_WINDOW_NESIZE
    { nullptr, IDC_SIZENESW, 0 },                               // POINTER_WINDOW_SWSIZE
    { nullptr, IDC_SIZENWSE, 0 },                               // POINTER_WINDOW_SESIZE
    { nullptr, IDC_SIZEWE, 0 },                                 // POINTER_HSPLIT
    { nullptr, IDC_SIZENS, 0 },                                 // POINTER_VSPLIT
    { nullptr, IDC_SIZEWE, 0 },                                 // POINTER_HSIZEBAR
    { nullptr, IDC_SIZENS, 0 },                                 // POINTER_VSIZEBAR
    { nullptr, IDC_HAND, 0 },                                   // POINTER_HAND
    { nullptr, IDC_HAND, 0 },                                   // POINTER_REFHAND
    { nullptr, IDC_PEN, 0 },                                    // POINTER_PEN
    { nullptr, nullptr, SAL_RESID_POINTER_MAGNIFY },            // POINTER_MAGNIFY
    { nullptr, nullptr, SAL_RESID_POINTER_FILL },               // POINTER_FILL
    { nullptr, nullptr, SAL_RESID_POINTER_ROTATE },             // POINTER_ROTATE
    { nullptr, nullptr, SAL_RESID_POINTER_HSHEAR },             // POINTER_HSHEAR
    { nullptr, nullptr, SAL_RESID_POINTER_VSHEAR },             // POINTER_VSHEAR
    { nullptr, nullptr, SAL_RESID_POINTER_MIRROR },             // POINTER_MIRROR
    { nullptr, nullptr, SAL_RESID_POINTER_CROOK },              // POINTER_CROOK
    { nullptr, nullptr, SAL_RESID_POINTER_CROP },               // POINTER_CROP
    { nullptr, nullptr, SAL_RESID_POINTER_MOVEPOINT },          // POINTER_MOVEPOINT
    { nullptr, nullptr, SAL_RESID_POINTER_MOVEBEZIERWEIGHT },   // POINTER_MOVEBEZIERWEIGHT
    { nullptr, nullptr, SAL_RESID_POINTER_MOVEDATA },           // POINTER_MOVEDATA
    { nullptr, nullptr, SAL_RESID_POINTER_COPYDATA },           // POINTER_COPYDATA
    { nullptr, nullptr, SAL_RESID_POINTER_LINKDATA },           // POINTER_LINKDATA
    { nullptr, nullptr, SAL_RESID_POINTER_MOVEDATALINK },       // POINTER_MOVEDATALINK
    { nullptr, nullptr, SAL_RESID_POINTER_COPYDATALINK },       // POINTER_COPYDATALINK
    { nullptr, nullptr, SAL_RESID_POINTER_MOVEFILE },           // POINTER_MOVEFILE
    { nullptr, nullptr, SAL_RESID_POINTER_COPYFILE },           // POINTER_COPYFILE
    { nullptr, nullptr, SAL_RESID_POINTER_LINKFILE },           // POINTER_LINKFILE
    { nullptr, nullptr, SAL_RESID_POINTER_MOVEFILELINK },       // POINTER_MOVEFILELINK
    { nullptr, nullptr, SAL_RESID_POINTER_COPYFILELINK },       // POINTER_COPYFILELINK
    { nullptr, nullptr, SAL_RESID_POINTER_MOVEFILES },          // POINTER_MOVEFILES
    { nullptr, nullptr, SAL_RESID_POINTER_COPYFILES },          // POINTER_COPYFILES
    { nullptr, IDC_NO, 0 },                                     // POINTER_NOTALLOWED
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_LINE },          // POINTER_DRAW_LINE
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_RECT },          // POINTER_DRAW_RECT
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_POLYGON },       // POINTER_DRAW_POLYGON
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_BEZIER },        // POINTER_DRAW_BEZIER
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_ARC },           // POINTER_DRAW_ARC
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_PIE },           // POINTER_DRAW_PIE
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_CIRCLECUT },     // POINTER_DRAW_CIRCLECUT
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_ELLIPSE },       // POINTER_DRAW_ELLIPSE
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_FREEHAND },      // POINTER_DRAW_FREEHAND
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_CONNECT },       // POINTER_DRAW_CONNECT
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_TEXT },          // POINTER_DRAW_TEXT
    { nullptr, nullptr, SAL_RESID_POINTER_DRAW_CAPTION },       // POINTER_DRAW_CAPTION
    { nullptr, nullptr, SAL_RESID_POINTER_CHART },              // POINTER_CHART
    { nullptr, nullptr, SAL_RESID_POINTER_DETECTIVE },          // POINTER_DETECTIVE
    { nullptr, nullptr, SAL_RESID_POINTER_PIVOT_COL },          // POINTER_PIVOT_COL
    { nullptr, nullptr, SAL_RESID_POINTER_PIVOT_ROW },          // POINTER_PIVOT_ROW
    { nullptr, nullptr, SAL_RESID_POINTER_PIVOT_FIELD },        // POINTER_PIVOT_FIELD
    { nullptr, nullptr, SAL_RESID_POINTER_CHAIN },              // POINTER_CHAIN
    { nullptr, nullptr, SAL_RESID_POINTER_CHAIN_NOTALLOWED },   // POINTER_CHAIN_NOTALLOWED
    { nullptr, nullptr, SAL_RESID_POINTER_TIMEEVENT_MOVE },     // POINTER_TIMEEVENT_MOVE
    { nullptr, nullptr, SAL_RESID_POINTER_TIMEEVENT_SIZE },     // POINTER_TIMEEVENT_SIZE
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_N },       // POINTER_AUTOSCROLL_N
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_S },       // POINTER_AUTOSCROLL_S
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_W },       // POINTER_AUTOSCROLL_W
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_E },       // POINTER_AUTOSCROLL_E
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_NW },      // POINTER_AUTOSCROLL_NW
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_NE },      // POINTER_AUTOSCROLL_NE
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_SW },      // POINTER_AUTOSCROLL_SW
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_SE },      // POINTER_AUTOSCROLL_SE
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_NS },      // POINTER_AUTOSCROLL_NS
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_WE },      // POINTER_AUTOSCROLL_WE
    { nullptr, nullptr, SAL_RESID_POINTER_AUTOSCROLL_NSWE },    // POINTER_AUTOSCROLL_NSWE
    { nullptr, nullptr, SAL_RESID_POINTER_AIRBRUSH },           // POINTER_AIRBRUSH
    { nullptr, nullptr, SAL_RESID_POINTER_TEXT_VERTICAL },      // POINTER_TEXT_VERTICAL
    { nullptr, nullptr, SAL_RESID_POINTER_PIVOT_DELETE },       // POINTER_PIVOT_DELETE

     // #i32329#
    { nullptr, nullptr, SAL_RESID_POINTER_TAB_SELECT_S },       // POINTER_TAB_SELECT_S
    { nullptr, nullptr, SAL_RESID_POINTER_TAB_SELECT_E },       // POINTER_TAB_SELECT_E
    { nullptr, nullptr, SAL_RESID_POINTER_TAB_SELECT_SE },      // POINTER_TAB_SELECT_SE
    { nullptr, nullptr, SAL_RESID_POINTER_TAB_SELECT_W },       // POINTER_TAB_SELECT_W
    { nullptr, nullptr, SAL_RESID_POINTER_TAB_SELECT_SW },      // POINTER_TAB_SELECT_SW

     // #i20119#
    { nullptr, nullptr, SAL_RESID_POINTER_PAINTBRUSH },         // POINTER_PAINTBRUSH

    { nullptr, nullptr, SAL_RESID_POINTER_HIDEWHITESPACE },     // POINTER_HIDEWHITESPACE
    { nullptr, nullptr, SAL_RESID_POINTER_SHOWWHITESPACE }      // POINTER_UNHIDEWHITESPACE
    };

    // Mousepointer loaded ?
    if ( !aImplPtrTab[ePointerStyle].mhCursor )
    {
        if ( aImplPtrTab[ePointerStyle].mnOwnId )
            aImplPtrTab[ePointerStyle].mhCursor = ImplLoadSalCursor( aImplPtrTab[ePointerStyle].mnOwnId );
        else
            aImplPtrTab[ePointerStyle].mhCursor = LoadCursor( nullptr, aImplPtrTab[ePointerStyle].mnSysId );
    }

    // change the mouse pointer if different
    if ( mhCursor != aImplPtrTab[ePointerStyle].mhCursor )
    {
        mhCursor = aImplPtrTab[ePointerStyle].mhCursor;
        SetCursor( mhCursor );
    }
}

void WinSalFrame::CaptureMouse( bool bCapture )
{
    // Send this Message to the window, because CaptureMouse() only work
    // in the thread of the window, which has create this window
    int nMsg;
    if ( bCapture )
        nMsg = SAL_MSG_CAPTUREMOUSE;
    else
        nMsg = SAL_MSG_RELEASEMOUSE;
    SendMessageW( mhWnd, nMsg, 0, 0 );
}

void WinSalFrame::SetPointerPos( long nX, long nY )
{
    POINT aPt;
    aPt.x = (int)nX;
    aPt.y = (int)nY;
    ClientToScreen( mhWnd, &aPt );
    SetCursorPos( aPt.x, aPt.y );
}

void WinSalFrame::Flush()
{
    GdiFlush();
}

static void ImplSalFrameSetInputContext( HWND hWnd, const SalInputContext* pContext )
{
    WinSalFrame*   pFrame = GetWindowPtr( hWnd );
    bool           bIME(pContext->mnOptions & InputContextFlags::Text);
    if ( bIME )
    {
        if ( !pFrame->mbIME )
        {
            pFrame->mbIME = TRUE;

            if ( pFrame->mhDefIMEContext )
            {
                ImmAssociateContext( pFrame->mhWnd, pFrame->mhDefIMEContext );
                UINT nImeProps = ImmGetProperty( GetKeyboardLayout( 0 ), IGP_PROPERTY );
                pFrame->mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
                pFrame->mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
                pFrame->mbHandleIME = !pFrame->mbSpezIME;
            }
        }

        // When the application can't handle IME messages, then the
        // System should handle the IME handling
        if ( !(pContext->mnOptions & InputContextFlags::ExtText) )
            pFrame->mbHandleIME = FALSE;

        // Set the Font for IME Handling
        if ( pContext->mpFont )
        {
            HIMC hIMC = ImmGetContext( pFrame->mhWnd );
            if ( hIMC )
            {
                LOGFONTW aLogFont;
                HDC hDC = GetDC( pFrame->mhWnd );
                // In case of vertical writing, always append a '@' to the
                // Windows font name, not only if such a Windows font really is
                // available (bTestVerticalAvail == false in the below call):
                // The Windows IME's candidates window seems to always use a
                // font that has all necessary glyphs, not necessarily the one
                // specified by this font name; but it seems to decide whether
                // to use that font's horizontal or vertical variant based on a
                // '@' in front of this font name.
                ImplGetLogFontFromFontSelect( hDC, pContext->mpFont, aLogFont,
                                              false );
                ReleaseDC( pFrame->mhWnd, hDC );
                ImmSetCompositionFontW( hIMC, &aLogFont );
                ImmReleaseContext( pFrame->mhWnd, hIMC );
            }
        }
    }
    else
    {
        if ( pFrame->mbIME )
        {
            pFrame->mbIME = FALSE;
            pFrame->mbHandleIME = FALSE;
            ImmAssociateContext( pFrame->mhWnd, nullptr );
        }
    }
}

void WinSalFrame::SetInputContext( SalInputContext* pContext )
{
    // Must be called in the main thread!
    SendMessageW( mhWnd, SAL_MSG_SETINPUTCONTEXT, 0, reinterpret_cast<LPARAM>(pContext) );
}

static void ImplSalFrameEndExtTextInput( HWND hWnd, EndExtTextInputFlags nFlags )
{
    HIMC hIMC = ImmGetContext( hWnd );
    if ( hIMC )
    {
        DWORD nIndex;
        if ( nFlags & EndExtTextInputFlags::Complete )
            nIndex = CPS_COMPLETE;
        else
            nIndex = CPS_CANCEL;

        ImmNotifyIME( hIMC, NI_COMPOSITIONSTR, nIndex, 0 );
        ImmReleaseContext( hWnd, hIMC );
    }
}

void WinSalFrame::EndExtTextInput( EndExtTextInputFlags nFlags )
{
    // Must be called in the main thread!
    SendMessageW( mhWnd, SAL_MSG_ENDEXTTEXTINPUT, (WPARAM)nFlags, 0 );
}

static void ImplGetKeyNameText( LONG lParam, sal_Unicode* pBuf,
                                UINT& rCount, UINT nMaxSize,
                                const sal_Char* pReplace )
{
    static_assert( sizeof( WCHAR ) == sizeof( sal_Unicode ), "must be the same size" );

    static const int nMaxKeyLen = 350;
    WCHAR aKeyBuf[ nMaxKeyLen ];
    int nKeyLen = 0;
    if ( lParam )
    {
        OUString aLang = Application::GetSettings().GetUILanguageTag().getLanguage();
        OUString aRet;

        aRet = ::vcl_sal::getKeysReplacementName( aLang, lParam );
        if( aRet.isEmpty() )
        {
            nKeyLen = GetKeyNameTextW( lParam, aKeyBuf, nMaxKeyLen );
            SAL_WARN_IF( nKeyLen > nMaxKeyLen, "vcl", "Invalid key name length!" );
            if( nKeyLen > nMaxKeyLen )
                nKeyLen = 0;
            else if( nKeyLen > 0 )
            {
                // Capitalize just the first letter of key names
                CharLowerBuffW( aKeyBuf, nKeyLen );

                bool bUpper = true;
                for( WCHAR *pW=aKeyBuf, *pE=pW+nKeyLen; pW < pE; ++pW )
                {
                    if( bUpper )
                        CharUpperBuffW( pW, 1 );
                    bUpper = (*pW=='+') || (*pW=='-') || (*pW==' ') || (*pW=='.');
                }
            }
        }
        else
        {
            nKeyLen = aRet.getLength();
            wcscpy( aKeyBuf, reinterpret_cast< const wchar_t* >( aRet.getStr() ));
        }
    }

    if ( (nKeyLen > 0) || pReplace )
    {
        if( (rCount > 0) && (rCount < nMaxSize) )
        {
            pBuf[rCount] = '+';
            rCount++;
        }

        if( nKeyLen > 0 )
        {
            WCHAR *pW = aKeyBuf, *pE = aKeyBuf + nKeyLen;
            while ((pW < pE) && *pW && (rCount < nMaxSize))
                pBuf[rCount++] = *pW++;
        }
        else // fall back to provided default name
        {
            while( *pReplace && (rCount < nMaxSize) )
            {
                pBuf[rCount] = *pReplace;
                rCount++;
                pReplace++;
            }
        }
    }
    else
        rCount = 0;
}

OUString WinSalFrame::GetKeyName( sal_uInt16 nKeyCode )
{
    static const UINT nMaxKeyLen = 350;
    sal_Unicode aKeyBuf[ nMaxKeyLen ];
    UINT        nKeyBufLen = 0;
    UINT        nSysCode = 0;

    if ( nKeyCode & KEY_MOD1 )
    {
        nSysCode = MapVirtualKey( VK_CONTROL, 0 );
        nSysCode = (nSysCode << 16) | (((sal_uLong)1) << 25);
        ImplGetKeyNameText( nSysCode, aKeyBuf, nKeyBufLen, nMaxKeyLen, "Ctrl" );
    }

    if ( nKeyCode & KEY_MOD2 )
    {
        nSysCode = MapVirtualKey( VK_MENU, 0 );
        nSysCode = (nSysCode << 16) | (((sal_uLong)1) << 25);
        ImplGetKeyNameText( nSysCode, aKeyBuf, nKeyBufLen, nMaxKeyLen, "Alt" );
    }

    if ( nKeyCode & KEY_SHIFT )
    {
        nSysCode = MapVirtualKey( VK_SHIFT, 0 );
        nSysCode = (nSysCode << 16) | (((sal_uLong)1) << 25);
        ImplGetKeyNameText( nSysCode, aKeyBuf, nKeyBufLen, nMaxKeyLen, "Shift" );
    }

    sal_uInt16      nCode = nKeyCode & 0x0FFF;
    sal_uLong       nSysCode2 = 0;
    const sal_Char*   pReplace = nullptr;
    sal_Unicode cSVCode = 0;
    sal_Char    aFBuf[4];
    nSysCode = 0;

    if ( (nCode >= KEY_0) && (nCode <= KEY_9) )
        cSVCode = '0' + (nCode - KEY_0);
    else if ( (nCode >= KEY_A) && (nCode <= KEY_Z) )
        cSVCode = 'A' + (nCode - KEY_A);
    else if ( (nCode >= KEY_F1) && (nCode <= KEY_F26) )
    {
        nSysCode = VK_F1 + (nCode - KEY_F1);
        aFBuf[0] = 'F';
        if ( (nCode >= KEY_F1) && (nCode <= KEY_F9) )
        {
            aFBuf[1] = sal::static_int_cast<sal_Char>('1' + (nCode - KEY_F1));
            aFBuf[2] = 0;
        }
        else if ( (nCode >= KEY_F10) && (nCode <= KEY_F19) )
        {
            aFBuf[1] = '1';
            aFBuf[2] = sal::static_int_cast<sal_Char>('0' + (nCode - KEY_F10));
            aFBuf[3] = 0;
        }
        else
        {
            aFBuf[1] = '2';
            aFBuf[2] = sal::static_int_cast<sal_Char>('0' + (nCode - KEY_F20));
            aFBuf[3] = 0;
        }
        pReplace = aFBuf;
    }
    else
    {
        switch ( nCode )
        {
            case KEY_DOWN:
                nSysCode = VK_DOWN;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Down";
                break;
            case KEY_UP:
                nSysCode = VK_UP;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Up";
                break;
            case KEY_LEFT:
                nSysCode = VK_LEFT;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Left";
                break;
            case KEY_RIGHT:
                nSysCode = VK_RIGHT;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Right";
                break;
            case KEY_HOME:
                nSysCode = VK_HOME;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Home";
                break;
            case KEY_END:
                nSysCode = VK_END;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "End";
                break;
            case KEY_PAGEUP:
                nSysCode = VK_PRIOR;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Page Up";
                break;
            case KEY_PAGEDOWN:
                nSysCode = VK_NEXT;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Page Down";
                break;
            case KEY_RETURN:
                nSysCode = VK_RETURN;
                pReplace = "Enter";
                break;
            case KEY_ESCAPE:
                nSysCode = VK_ESCAPE;
                pReplace = "Escape";
                break;
            case KEY_TAB:
                nSysCode = VK_TAB;
                pReplace = "Tab";
                break;
            case KEY_BACKSPACE:
                nSysCode = VK_BACK;
                pReplace = "Backspace";
                break;
            case KEY_SPACE:
                nSysCode = VK_SPACE;
                pReplace = "Space";
                break;
            case KEY_INSERT:
                nSysCode = VK_INSERT;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Insert";
                break;
            case KEY_DELETE:
                nSysCode = VK_DELETE;
                nSysCode2 = (((sal_uLong)1) << 24);
                pReplace = "Delete";
                break;

            case KEY_ADD:
                cSVCode  = '+';
                break;
            case KEY_SUBTRACT:
                cSVCode  = '-';
                break;
            case KEY_MULTIPLY:
                cSVCode  = '*';
                break;
            case KEY_DIVIDE:
                cSVCode  = '/';
                break;
            case KEY_POINT:
                cSVCode  = '.';
                break;
            case KEY_COMMA:
                cSVCode  = ',';
                break;
            case KEY_LESS:
                cSVCode  = '<';
                break;
            case KEY_GREATER:
                cSVCode  = '>';
                break;
            case KEY_EQUAL:
                cSVCode  = '=';
                break;
            case KEY_SEMICOLON:
                cSVCode = ';';
                break;
            case KEY_QUOTERIGHT:
                cSVCode = '\'';
                break;
            case KEY_BRACKETLEFT:
                cSVCode = '[';
                break;
            case KEY_BRACKETRIGHT:
                cSVCode = ']';
                break;
        }
    }

    if ( nSysCode )
    {
        nSysCode = MapVirtualKey( (UINT)nSysCode, 0 );
        if ( nSysCode )
            nSysCode = (nSysCode << 16) | nSysCode2;
        ImplGetKeyNameText( nSysCode, aKeyBuf, nKeyBufLen, nMaxKeyLen, pReplace );
    }
    else
    {
        if ( cSVCode )
        {
            if ( nKeyBufLen > 0 )
                aKeyBuf[ nKeyBufLen++ ] = '+';
            if( nKeyBufLen < nMaxKeyLen )
                aKeyBuf[ nKeyBufLen++ ] = cSVCode;
        }
    }

    if( !nKeyBufLen )
        return OUString();

    return OUString( aKeyBuf, sal::static_int_cast< sal_uInt16 >(nKeyBufLen) );
}

inline Color ImplWinColorToSal( COLORREF nColor )
{
    return Color( GetRValue( nColor ), GetGValue( nColor ), GetBValue( nColor ) );
}

static void ImplSalUpdateStyleFontW( HDC hDC, const LOGFONTW& rLogFont, vcl::Font& rFont )
{
    ImplSalLogFontToFontW( hDC, rLogFont, rFont );

    // On Windows 9x, Windows NT we get sometimes very small sizes
    // (for example for the small Caption height).
    // So if it is MS Sans Serif, a none scalable font we use
    // 8 Point as the minimum control height, in all other cases
    // 6 Point is the smallest one
    if ( rFont.GetFontHeight() < 8 )
    {
        if ( rtl_ustr_compareIgnoreAsciiCase( reinterpret_cast<const sal_Unicode*>(rLogFont.lfFaceName), reinterpret_cast<const sal_Unicode*>(L"MS Sans Serif") ) == 0 )
            rFont.SetFontHeight( 8 );
        else if ( rFont.GetFontHeight() < 6 )
            rFont.SetFontHeight( 6 );
    }
}

static long ImplA2I( const BYTE* pStr )
{
    long    n = 0;
    int     nSign = 1;

    if ( *pStr == '-' )
    {
        nSign = -1;
        pStr++;
    }

    while( (*pStr >= 48) && (*pStr <= 57) )
    {
        n *= 10;
        n += ((*pStr) - 48);
        pStr++;
    }

    n *= nSign;

    return n;
}

void WinSalFrame::UpdateSettings( AllSettings& rSettings )
{
    MouseSettings aMouseSettings = rSettings.GetMouseSettings();
    aMouseSettings.SetDoubleClickTime( GetDoubleClickTime() );
    aMouseSettings.SetDoubleClickWidth( GetSystemMetrics( SM_CXDOUBLECLK ) );
    aMouseSettings.SetDoubleClickHeight( GetSystemMetrics( SM_CYDOUBLECLK ) );
    long nDragWidth = GetSystemMetrics( SM_CXDRAG );
    long nDragHeight = GetSystemMetrics( SM_CYDRAG );
    if ( nDragWidth )
        aMouseSettings.SetStartDragWidth( nDragWidth );
    if ( nDragHeight )
        aMouseSettings.SetStartDragHeight( nDragHeight );
    HKEY hRegKey;
    if ( RegOpenKey( HKEY_CURRENT_USER,
                     "Control Panel\\Desktop",
                     &hRegKey ) == ERROR_SUCCESS )
    {
        BYTE    aValueBuf[10];
        DWORD   nValueSize = sizeof( aValueBuf );
        DWORD   nType;
        if ( RegQueryValueEx( hRegKey, "MenuShowDelay", nullptr,
                              &nType, aValueBuf, &nValueSize ) == ERROR_SUCCESS )
        {
            if ( nType == REG_SZ )
                aMouseSettings.SetMenuDelay( (sal_uLong)ImplA2I( aValueBuf ) );
        }

        RegCloseKey( hRegKey );
    }

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();

    aStyleSettings.SetScrollBarSize( GetSystemMetrics( SM_CXVSCROLL ) );
    aStyleSettings.SetSpinSize( GetSystemMetrics( SM_CXVSCROLL ) );
    UINT blinkTime = GetCaretBlinkTime();
    aStyleSettings.SetCursorBlinkTime(
        blinkTime == 0 || blinkTime == INFINITE // 0 indicates error
        ? STYLE_CURSOR_NOBLINKTIME : blinkTime );
    aStyleSettings.SetFloatTitleHeight( GetSystemMetrics( SM_CYSMCAPTION ) );
    aStyleSettings.SetTitleHeight( GetSystemMetrics( SM_CYCAPTION ) );
    aStyleSettings.SetActiveBorderColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVEBORDER ) ) );
    aStyleSettings.SetDeactiveBorderColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVEBORDER ) ) );
    aStyleSettings.SetDeactiveColor( ImplWinColorToSal( GetSysColor( COLOR_GRADIENTINACTIVECAPTION ) ) );
    aStyleSettings.SetFaceColor( ImplWinColorToSal( GetSysColor( COLOR_3DFACE ) ) );
    aStyleSettings.SetInactiveTabColor( aStyleSettings.GetFaceColor() );
    aStyleSettings.SetLightColor( ImplWinColorToSal( GetSysColor( COLOR_3DHILIGHT ) ) );
    aStyleSettings.SetLightBorderColor( ImplWinColorToSal( GetSysColor( COLOR_3DLIGHT ) ) );
    aStyleSettings.SetShadowColor( ImplWinColorToSal( GetSysColor( COLOR_3DSHADOW ) ) );
    aStyleSettings.SetDarkShadowColor( ImplWinColorToSal( GetSysColor( COLOR_3DDKSHADOW ) ) );
    aStyleSettings.SetHelpColor( ImplWinColorToSal( GetSysColor( COLOR_INFOBK ) ) );
    aStyleSettings.SetHelpTextColor( ImplWinColorToSal( GetSysColor( COLOR_INFOTEXT ) ) );
    aStyleSettings.SetDialogColor( aStyleSettings.GetFaceColor() );
    aStyleSettings.SetDialogTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetButtonTextColor( ImplWinColorToSal( GetSysColor( COLOR_BTNTEXT ) ) );
    aStyleSettings.SetButtonRolloverTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetTabTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetTabRolloverTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetTabHighlightTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetRadioCheckTextColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOWTEXT ) ) );
    aStyleSettings.SetGroupTextColor( aStyleSettings.GetRadioCheckTextColor() );
    aStyleSettings.SetLabelTextColor( aStyleSettings.GetRadioCheckTextColor() );
    aStyleSettings.SetWindowColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOW ) ) );
    aStyleSettings.SetActiveTabColor( aStyleSettings.GetWindowColor() );
    aStyleSettings.SetWindowTextColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOWTEXT ) ) );
    aStyleSettings.SetToolTextColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOWTEXT ) ) );
    aStyleSettings.SetFieldColor( aStyleSettings.GetWindowColor() );
    aStyleSettings.SetFieldTextColor( aStyleSettings.GetWindowTextColor() );
    aStyleSettings.SetFieldRolloverTextColor( aStyleSettings.GetFieldTextColor() );
    aStyleSettings.SetHighlightColor( ImplWinColorToSal( GetSysColor( COLOR_HIGHLIGHT ) ) );
    aStyleSettings.SetHighlightTextColor( ImplWinColorToSal( GetSysColor( COLOR_HIGHLIGHTTEXT ) ) );
    aStyleSettings.SetMenuHighlightColor( aStyleSettings.GetHighlightColor() );
    aStyleSettings.SetMenuHighlightTextColor( aStyleSettings.GetHighlightTextColor() );

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mnMenuFormatBorderX = 0;
    pSVData->maNWFData.mnMenuFormatBorderY = 0;
    pSVData->maNWFData.maMenuBarHighlightTextColor = Color( COL_TRANSPARENT );
    GetSalData()->mbThemeMenuSupport = false;
    if (officecfg::Office::Common::Accessibility::AutoDetectSystemHC::get())
    {
        aStyleSettings.SetShadowColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVEBORDER ) ) );
        aStyleSettings.SetWorkspaceColor( ImplWinColorToSal( GetSysColor( COLOR_MENU ) ) );
    }
    aStyleSettings.SetMenuColor( ImplWinColorToSal( GetSysColor( COLOR_MENU ) ) );
    aStyleSettings.SetMenuBarColor( aStyleSettings.GetMenuColor() );
    aStyleSettings.SetMenuBarRolloverColor( aStyleSettings.GetHighlightColor() );
    aStyleSettings.SetMenuBorderColor( aStyleSettings.GetLightBorderColor() ); // overridden below for flat menus
    aStyleSettings.SetUseFlatBorders( FALSE );
    aStyleSettings.SetUseFlatMenus( FALSE );
    aStyleSettings.SetMenuTextColor( ImplWinColorToSal( GetSysColor( COLOR_MENUTEXT ) ) );
    if ( boost::optional<Color> aColor = aStyleSettings.GetPersonaMenuBarTextColor() )
    {
        aStyleSettings.SetMenuBarTextColor( *aColor );
        aStyleSettings.SetMenuBarRolloverTextColor( *aColor );
    }
    else
    {
        aStyleSettings.SetMenuBarTextColor( ImplWinColorToSal( GetSysColor( COLOR_MENUTEXT ) ) );
        aStyleSettings.SetMenuBarRolloverTextColor( ImplWinColorToSal( GetSysColor( COLOR_HIGHLIGHTTEXT ) ) );
    }
    aStyleSettings.SetMenuBarHighlightTextColor(aStyleSettings.GetMenuHighlightTextColor());
    aStyleSettings.SetActiveColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVECAPTION ) ) );
    aStyleSettings.SetActiveTextColor( ImplWinColorToSal( GetSysColor( COLOR_CAPTIONTEXT ) ) );
    aStyleSettings.SetDeactiveColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVECAPTION ) ) );
    aStyleSettings.SetDeactiveTextColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVECAPTIONTEXT ) ) );
    if ( aSalShlData.mbWXP )
    {
        // only xp supports a different menu bar color
        long bFlatMenus = 0;
        SystemParametersInfo( SPI_GETFLATMENU, 0, &bFlatMenus, 0);
        if( bFlatMenus )
        {
            aStyleSettings.SetUseFlatMenus( TRUE );
            aStyleSettings.SetMenuBarColor( ImplWinColorToSal( GetSysColor( COLOR_MENUBAR ) ) );
            aStyleSettings.SetMenuHighlightColor( ImplWinColorToSal( GetSysColor( COLOR_MENUHILIGHT ) ) );
            aStyleSettings.SetMenuBarRolloverColor( ImplWinColorToSal( GetSysColor( COLOR_MENUHILIGHT ) ) );
            aStyleSettings.SetMenuBorderColor( ImplWinColorToSal( GetSysColor( COLOR_3DSHADOW ) ) );

            // flat borders for our controls etc. as well in this mode (ie, no 3d borders)
            // this is not active in the classic style appearance
            aStyleSettings.SetUseFlatBorders( TRUE );
        }
    }
    aStyleSettings.SetCheckedColorSpecialCase( );

    // caret width
    DWORD nCaretWidth = 2;
    if( SystemParametersInfo( SPI_GETCARETWIDTH, 0, &nCaretWidth, 0 ) )
        aStyleSettings.SetCursorSize( nCaretWidth );

    // High contrast
    HIGHCONTRAST hc;
    hc.cbSize = sizeof( HIGHCONTRAST );
    if( SystemParametersInfo( SPI_GETHIGHCONTRAST, hc.cbSize, &hc, 0 )
            && (hc.dwFlags & HCF_HIGHCONTRASTON) )
        aStyleSettings.SetHighContrastMode( true );
    else
        aStyleSettings.SetHighContrastMode( false );

    // Query Fonts
    vcl::Font    aMenuFont = aStyleSettings.GetMenuFont();
    vcl::Font    aTitleFont = aStyleSettings.GetTitleFont();
    vcl::Font    aFloatTitleFont = aStyleSettings.GetFloatTitleFont();
    vcl::Font    aHelpFont = aStyleSettings.GetHelpFont();
    vcl::Font    aAppFont = aStyleSettings.GetAppFont();
    vcl::Font    aIconFont = aStyleSettings.GetIconFont();
    HDC     hDC = GetDC( nullptr );
    NONCLIENTMETRICSW aNonClientMetrics;
    aNonClientMetrics.cbSize = sizeof( aNonClientMetrics );
    if ( SystemParametersInfoW( SPI_GETNONCLIENTMETRICS, sizeof( aNonClientMetrics ), &aNonClientMetrics, 0 ) )
    {
        ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfMenuFont, aMenuFont );
        ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfCaptionFont, aTitleFont );
        ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfSmCaptionFont, aFloatTitleFont );
        ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfStatusFont, aHelpFont );
        ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfMessageFont, aAppFont );

        LOGFONTW aLogFont;
        if ( SystemParametersInfoW( SPI_GETICONTITLELOGFONT, 0, &aLogFont, 0 ) )
            ImplSalUpdateStyleFontW( hDC, aLogFont, aIconFont );
    }

    ReleaseDC( nullptr, hDC );

    aStyleSettings.SetToolbarIconSize(ToolbarIconSize::Large);

    aStyleSettings.SetMenuFont( aMenuFont );
    aStyleSettings.SetTitleFont( aTitleFont );
    aStyleSettings.SetFloatTitleFont( aFloatTitleFont );
    aStyleSettings.SetHelpFont( aHelpFont );
    aStyleSettings.SetIconFont( aIconFont );
    // We prefer Arial in the russian version, because MS Sans Serif
    // is to wide for the dialogs
    if ( rSettings.GetLanguageTag().getLanguageType() == LANGUAGE_RUSSIAN )
    {
        OUString aFontName = aAppFont.GetFamilyName();
        OUString aFirstName = aFontName.getToken( 0, ';' );
        if ( aFirstName.equalsIgnoreAsciiCase( "MS Sans Serif" ) )
        {
            aFontName = "Arial;" + aFontName;
            aAppFont.SetFamilyName( aFontName );
        }
    }
    aStyleSettings.SetAppFont( aAppFont );
    aStyleSettings.SetGroupFont( aAppFont );
    aStyleSettings.SetLabelFont( aAppFont );
    aStyleSettings.SetRadioCheckFont( aAppFont );
    aStyleSettings.SetPushButtonFont( aAppFont );
    aStyleSettings.SetFieldFont( aAppFont );
    if ( aAppFont.GetWeight() > WEIGHT_NORMAL )
        aAppFont.SetWeight( WEIGHT_NORMAL );
    aStyleSettings.SetToolFont( aAppFont );
    aStyleSettings.SetTabFont( aAppFont );

    BOOL bDragFull;
    if ( SystemParametersInfo( SPI_GETDRAGFULLWINDOWS, 0, &bDragFull, 0 ) )
    {
        DragFullOptions nDragFullOptions = aStyleSettings.GetDragFullOptions();
        if ( bDragFull )
            nDragFullOptions |= DragFullOptions::WindowMove | DragFullOptions::WindowSize | DragFullOptions::Docking | DragFullOptions::Split;
        else
            nDragFullOptions &= ~DragFullOptions(DragFullOptions::WindowMove | DragFullOptions::WindowSize | DragFullOptions::Docking | DragFullOptions::Split);
        aStyleSettings.SetDragFullOptions( nDragFullOptions );
    }

    if ( RegOpenKey( HKEY_CURRENT_USER,
                     "Control Panel\\International\\Calendars\\TwoDigitYearMax",
                     &hRegKey ) == ERROR_SUCCESS )
    {
        BYTE    aValueBuf[10];
        DWORD   nValue;
        DWORD   nValueSize = sizeof( aValueBuf );
        DWORD   nType;
        if ( RegQueryValueEx( hRegKey, "1", nullptr,
                              &nType, aValueBuf, &nValueSize ) == ERROR_SUCCESS )
        {
            if ( nType == REG_SZ )
            {
                nValue = (sal_uLong)ImplA2I( aValueBuf );
                if ( (nValue > 1000) && (nValue < 10000) )
                {
                    MiscSettings aMiscSettings = rSettings.GetMiscSettings();
                    utl::MiscCfg().SetYear2000( (sal_Int32)(nValue-99) );
                    rSettings.SetMiscSettings( aMiscSettings );
                }
            }
        }

        RegCloseKey( hRegKey );
    }

    rSettings.SetMouseSettings( aMouseSettings );
    rSettings.SetStyleSettings( aStyleSettings );

    // now apply the values from theming, if available
    WinSalGraphics::updateSettingsNative( rSettings );
}

const SystemEnvData* WinSalFrame::GetSystemData() const
{
    return &maSysData;
}

void WinSalFrame::Beep()
{
    // a simple beep
    MessageBeep( 0 );
}

SalFrame::SalPointerState WinSalFrame::GetPointerState()
{
    SalPointerState aState;
    aState.mnState = 0;

    if ( GetKeyState( VK_LBUTTON ) & 0x8000 )
        aState.mnState |= MOUSE_LEFT;
    if ( GetKeyState( VK_MBUTTON ) & 0x8000 )
        aState.mnState |= MOUSE_MIDDLE;
    if ( GetKeyState( VK_RBUTTON ) & 0x8000 )
        aState.mnState |= MOUSE_RIGHT;
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        aState.mnState |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        aState.mnState |= KEY_MOD1;
    if ( GetKeyState( VK_MENU ) & 0x8000 )
        aState.mnState |= KEY_MOD2;

    POINT pt;
    GetCursorPos( &pt );

    aState.maPos = Point( pt.x - maGeometry.nX, pt.y - maGeometry.nY );
    return aState;
}

KeyIndicatorState WinSalFrame::GetIndicatorState()
{
    KeyIndicatorState aState = KeyIndicatorState::NONE;
    if (::GetKeyState(VK_CAPITAL))
        aState |= KeyIndicatorState::CAPSLOCK;

    if (::GetKeyState(VK_NUMLOCK))
        aState |= KeyIndicatorState::NUMLOCK;

    if (::GetKeyState(VK_SCROLL))
        aState |= KeyIndicatorState::SCROLLLOCK;

    return aState;
}

void WinSalFrame::SimulateKeyPress( sal_uInt16 nKeyCode )
{
    BYTE nVKey = 0;
    switch (nKeyCode)
    {
        case KEY_CAPSLOCK:
            nVKey = VK_CAPITAL;
        break;
    }

    if (nVKey > 0 && nVKey < 255)
    {
        ::keybd_event(nVKey, 0x45, KEYEVENTF_EXTENDEDKEY, 0);
        ::keybd_event(nVKey, 0x45, KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP, 0);
    }
}

void WinSalFrame::ResetClipRegion()
{
    SetWindowRgn( mhWnd, nullptr, TRUE );
}

void WinSalFrame::BeginSetClipRegion( sal_uLong nRects )
{
    if( mpClipRgnData )
        delete [] reinterpret_cast<BYTE*>(mpClipRgnData);
    sal_uLong nRectBufSize = sizeof(RECT)*nRects;
    mpClipRgnData = reinterpret_cast<RGNDATA*>(new BYTE[sizeof(RGNDATA)-1+nRectBufSize]);
    mpClipRgnData->rdh.dwSize     = sizeof( RGNDATAHEADER );
    mpClipRgnData->rdh.iType      = RDH_RECTANGLES;
    mpClipRgnData->rdh.nCount     = nRects;
    mpClipRgnData->rdh.nRgnSize  = nRectBufSize;
    SetRectEmpty( &(mpClipRgnData->rdh.rcBound) );
    mpNextClipRect        = reinterpret_cast<RECT*>(&(mpClipRgnData->Buffer));
    mbFirstClipRect       = TRUE;
}

void WinSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if( ! mpClipRgnData )
        return;

    RECT*       pRect = mpNextClipRect;
    RECT*       pBoundRect = &(mpClipRgnData->rdh.rcBound);
    long        nRight = nX + nWidth;
    long        nBottom = nY + nHeight;

    if ( mbFirstClipRect )
    {
        pBoundRect->left    = nX;
        pBoundRect->top     = nY;
        pBoundRect->right   = nRight;
        pBoundRect->bottom  = nBottom;
        mbFirstClipRect = FALSE;
    }
    else
    {
        if ( nX < pBoundRect->left )
            pBoundRect->left = (int)nX;

        if ( nY < pBoundRect->top )
            pBoundRect->top = (int)nY;

        if ( nRight > pBoundRect->right )
            pBoundRect->right = (int)nRight;

        if ( nBottom > pBoundRect->bottom )
            pBoundRect->bottom = (int)nBottom;
    }

    pRect->left     = (int)nX;
    pRect->top      = (int)nY;
    pRect->right    = (int)nRight;
    pRect->bottom   = (int)nBottom;
    if( (mpNextClipRect  - reinterpret_cast<RECT*>(&mpClipRgnData->Buffer)) < (int)mpClipRgnData->rdh.nCount )
        mpNextClipRect++;
}

void WinSalFrame::EndSetClipRegion()
{
    if( ! mpClipRgnData )
        return;

    HRGN hRegion;

    // create region from accumulated rectangles
    if ( mpClipRgnData->rdh.nCount == 1 )
    {
        RECT* pRect = &(mpClipRgnData->rdh.rcBound);
        hRegion = CreateRectRgn( pRect->left, pRect->top,
                                 pRect->right, pRect->bottom );
    }
    else
    {
        sal_uLong nSize = mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
        hRegion = ExtCreateRegion( nullptr, nSize, mpClipRgnData );
    }
    delete [] reinterpret_cast<BYTE*>(mpClipRgnData);
    mpClipRgnData = nullptr;

    SAL_WARN_IF( !hRegion, "vcl", "WinSalFrame::EndSetClipRegion() - Can't create ClipRegion" );
    if( hRegion )
    {
        RECT aWindowRect;
        GetWindowRect( mhWnd, &aWindowRect );
        POINT aPt;
        aPt.x=0;
        aPt.y=0;
        ClientToScreen( mhWnd, &aPt );
        OffsetRgn( hRegion, aPt.x - aWindowRect.left, aPt.y - aWindowRect.top );

        if( SetWindowRgn( mhWnd, hRegion, TRUE ) == 0 )
            DeleteObject( hRegion );
    }
}

static long ImplHandleMouseMsg( HWND hWnd, UINT nMsg,
                                WPARAM wParam, LPARAM lParam )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    if( nMsg == WM_LBUTTONDOWN || nMsg == WM_MBUTTONDOWN || nMsg == WM_RBUTTONDOWN )
    {
        // #103168# post again if async focus has not arrived yet
        // hopefully we will not receive the corresponding button up before this
        // button down arrives again
        vcl::Window *pWin = pFrame->GetWindow();
        if( pWin && pWin->ImplGetWindowImpl()->mpFrameData->mnFocusId )
        {
            BOOL const ret = PostMessageW( hWnd, nMsg, wParam, lParam );
            SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
            return 1;
        }
    }
    SalMouseEvent   aMouseEvt;
    long            nRet;
    SalEvent        nEvent = SalEvent::NONE;
    bool            bCall = TRUE;

    aMouseEvt.mnX       = (short)LOWORD( lParam );
    aMouseEvt.mnY       = (short)HIWORD( lParam );
    aMouseEvt.mnCode    = 0;
    aMouseEvt.mnTime    = GetMessageTime();

    // Use GetKeyState(), as some Logitech mouse drivers do not check
    // KeyState when simulating double-click with center mouse button

    if ( GetKeyState( VK_LBUTTON ) & 0x8000 )
        aMouseEvt.mnCode |= MOUSE_LEFT;
    if ( GetKeyState( VK_MBUTTON ) & 0x8000 )
        aMouseEvt.mnCode |= MOUSE_MIDDLE;
    if ( GetKeyState( VK_RBUTTON ) & 0x8000 )
        aMouseEvt.mnCode |= MOUSE_RIGHT;
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        aMouseEvt.mnCode |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        aMouseEvt.mnCode |= KEY_MOD1;
    if ( GetKeyState( VK_MENU ) & 0x8000 )
        aMouseEvt.mnCode |= KEY_MOD2;

    switch ( nMsg )
    {
        case WM_MOUSEMOVE:
            {
            // As the mouse events are not collected correctly when
            // pressing modifier keys (as interrupted by KeyEvents)
            // we do this here ourselves
            if ( aMouseEvt.mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2) )
            {
                MSG aTempMsg;
                if ( PeekMessageW( &aTempMsg, hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE | PM_NOYIELD ) )
                {
                    if ( (aTempMsg.message == WM_MOUSEMOVE) &&
                         (aTempMsg.wParam == wParam) )
                        return 1;
                }
            }

            SalData* pSalData = GetSalData();
            // Test for MouseLeave
            if ( pSalData->mhWantLeaveMsg && (pSalData->mhWantLeaveMsg != hWnd) )
                SendMessageW( pSalData->mhWantLeaveMsg, SAL_MSG_MOUSELEAVE, 0, GetMessagePos() );

            pSalData->mhWantLeaveMsg = hWnd;
            // Start MouseLeave-Timer
            if ( !pSalData->mpMouseLeaveTimer )
            {
                pSalData->mpMouseLeaveTimer = new AutoTimer;
                pSalData->mpMouseLeaveTimer->SetTimeout( SAL_MOUSELEAVE_TIMEOUT );
                pSalData->mpMouseLeaveTimer->Start();
                // We don't need to set a timeout handler, because we test
                // for mouseleave in the timeout callback
            }
            aMouseEvt.mnButton = 0;
            nEvent = SalEvent::MouseMove;
            }
            break;

        case WM_NCMOUSEMOVE:
        case SAL_MSG_MOUSELEAVE:
            {
            SalData* pSalData = GetSalData();
            if ( pSalData->mhWantLeaveMsg == hWnd )
            {
                pSalData->mhWantLeaveMsg = nullptr;
                if ( pSalData->mpMouseLeaveTimer )
                {
                    delete pSalData->mpMouseLeaveTimer;
                    pSalData->mpMouseLeaveTimer = nullptr;
                }
                // Mouse-Coordinates are relative to the screen
                POINT aPt;
                aPt.x = (short)LOWORD( lParam );
                aPt.y = (short)HIWORD( lParam );
                ScreenToClient( hWnd, &aPt );
                aMouseEvt.mnX = aPt.x;
                aMouseEvt.mnY = aPt.y;
                aMouseEvt.mnButton = 0;
                nEvent = SalEvent::MouseLeave;
            }
            else
                bCall = FALSE;
            }
            break;

        case WM_LBUTTONDOWN:
            aMouseEvt.mnButton = MOUSE_LEFT;
            nEvent = SalEvent::MouseButtonDown;
            break;

        case WM_MBUTTONDOWN:
            aMouseEvt.mnButton = MOUSE_MIDDLE;
            nEvent = SalEvent::MouseButtonDown;
            break;

        case WM_RBUTTONDOWN:
            aMouseEvt.mnButton = MOUSE_RIGHT;
            nEvent = SalEvent::MouseButtonDown;
            break;

        case WM_LBUTTONUP:
            aMouseEvt.mnButton = MOUSE_LEFT;
            nEvent = SalEvent::MouseButtonUp;
            break;

        case WM_MBUTTONUP:
            aMouseEvt.mnButton = MOUSE_MIDDLE;
            nEvent = SalEvent::MouseButtonUp;
            break;

        case WM_RBUTTONUP:
            aMouseEvt.mnButton = MOUSE_RIGHT;
            nEvent = SalEvent::MouseButtonUp;
            break;
    }

    // check if this window was destroyed - this might happen if we are the help window
    // and sent a mouse leave message to the application which killed the help window, ie ourselves
    if( !IsWindow( hWnd ) )
        return 0;

    if ( bCall )
    {
        if ( nEvent == SalEvent::MouseButtonDown )
            UpdateWindow( hWnd );

        // --- RTL --- (mirror mouse pos)
        if( AllSettings::GetLayoutRTL() )
            aMouseEvt.mnX = pFrame->maGeometry.nWidth-1-aMouseEvt.mnX;

        nRet = pFrame->CallCallback( nEvent, &aMouseEvt );
        if ( nMsg == WM_MOUSEMOVE )
            SetCursor( pFrame->mhCursor );
    }
    else
        nRet = 0;

    return nRet;
}

static long ImplHandleMouseActivateMsg( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    if ( pFrame->mbFloatWin )
        return TRUE;

    return pFrame->CallCallback( SalEvent::MouseActivate, nullptr );
}

static long ImplHandleWheelMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    DBG_ASSERT( nMsg == WM_MOUSEWHEEL ||
                nMsg == WM_MOUSEHWHEEL,
                "ImplHandleWheelMsg() called with no wheel mouse event" );

    ImplSalYieldMutexAcquireWithWait();

    long        nRet = 0;
    WinSalFrame*   pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        WORD    nWinModCode = LOWORD( wParam );
        POINT   aWinPt;
        aWinPt.x    = (short)LOWORD( lParam );
        aWinPt.y    = (short)HIWORD( lParam );
        ScreenToClient( hWnd, &aWinPt );

        SalWheelMouseEvent aWheelEvt;
        aWheelEvt.mnTime        = GetMessageTime();
        aWheelEvt.mnX           = aWinPt.x;
        aWheelEvt.mnY           = aWinPt.y;
        aWheelEvt.mnCode        = 0;
        aWheelEvt.mnDelta       = (short)HIWORD( wParam );
        aWheelEvt.mnNotchDelta  = aWheelEvt.mnDelta/WHEEL_DELTA;
        if( aWheelEvt.mnNotchDelta == 0 )
        {
            if( aWheelEvt.mnDelta > 0 )
                aWheelEvt.mnNotchDelta = 1;
            else if( aWheelEvt.mnDelta < 0 )
                aWheelEvt.mnNotchDelta = -1;
        }

        if( nMsg == WM_MOUSEWHEEL )
        {
            if ( aSalShlData.mnWheelScrollLines == WHEEL_PAGESCROLL )
                aWheelEvt.mnScrollLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
            else
                aWheelEvt.mnScrollLines = aSalShlData.mnWheelScrollLines;
            aWheelEvt.mbHorz        = FALSE;
        }
        else
        {
            aWheelEvt.mnScrollLines = aSalShlData.mnWheelScrollChars;
            aWheelEvt.mbHorz        = TRUE;

            // fdo#36380 - seems horiz scrolling has swapped direction
            aWheelEvt.mnDelta *= -1;
            aWheelEvt.mnNotchDelta *= -1;
        }

        if ( nWinModCode & MK_SHIFT )
            aWheelEvt.mnCode |= KEY_SHIFT;
        if ( nWinModCode & MK_CONTROL )
            aWheelEvt.mnCode |= KEY_MOD1;
        if ( GetKeyState( VK_MENU ) & 0x8000 )
            aWheelEvt.mnCode |= KEY_MOD2;

        // --- RTL --- (mirror mouse pos)
        if( AllSettings::GetLayoutRTL() )
            aWheelEvt.mnX = pFrame->maGeometry.nWidth-1-aWheelEvt.mnX;

        nRet = pFrame->CallCallback( SalEvent::WheelMouse, &aWheelEvt );
    }

    ImplSalYieldMutexRelease();

    return nRet;
}

static sal_uInt16 ImplSalGetKeyCode( WPARAM wParam )
{
    sal_uInt16 nKeyCode;

    // convert KeyCode
    if ( wParam < KEY_TAB_SIZE )
        nKeyCode = aImplTranslateKeyTab[wParam];
    else
    {
        SalData* pSalData = GetSalData();
        std::map< UINT, sal_uInt16 >::const_iterator it = pSalData->maVKMap.find( (UINT)wParam );
        if( it != pSalData->maVKMap.end() )
            nKeyCode = it->second;
        else
            nKeyCode = 0;
    }

    return nKeyCode;
}

static void ImplUpdateInputLang( WinSalFrame* pFrame )
{
    UINT nLang = LOWORD( GetKeyboardLayout( 0 ) );
    if ( nLang && nLang != pFrame->mnInputLang )
    {
        // keep input lang up-to-date
        pFrame->mnInputLang = nLang;
    }

    // We are on Windows NT so we use Unicode FrameProcs and get
    // Unicode charcodes directly from Windows no need to set up a
    // code page
    return;
}

static sal_Unicode ImplGetCharCode( WinSalFrame* pFrame, WPARAM nCharCode )
{
    ImplUpdateInputLang( pFrame );

    // We are on Windows NT so we use Unicode FrameProcs and we
    // get Unicode charcodes directly from Windows
    return (sal_Unicode)nCharCode;
}

LanguageType WinSalFrame::GetInputLanguage()
{
    if( !mnInputLang )
        ImplUpdateInputLang( this );

    if( !mnInputLang )
        return LANGUAGE_DONTKNOW;
    else
        return (LanguageType) mnInputLang;
}

bool WinSalFrame::MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode )
{
    bool bRet = FALSE;
    sal_IntPtr nLangType = aLangType;
    // just use the passed language identifier, do not try to load additional keyboard support
    HKL hkl = reinterpret_cast<HKL>(nLangType);

    if( hkl )
    {
        SHORT scan = VkKeyScanExW( aUnicode, hkl );
        if( LOWORD(scan) == 0xFFFF )
            // keyboard not loaded or key cannot be mapped
            bRet = FALSE;
        else
        {
            BYTE vkeycode   = LOBYTE(scan);
            BYTE shiftstate = HIBYTE(scan);

            // Last argument is set to FALSE, because there's no decision made
            // yet which key should be assigned to MOD3 modifier on Windows.
            // Windows key - user's can be confused, because it should display
            //               Windows menu (applies to both left/right key)
            // Menu key    - this key is used to display context menu
            // AltGr key   - probably it has no sense
            rKeyCode = vcl::KeyCode( ImplSalGetKeyCode( vkeycode ),
                (shiftstate & 0x01) ? TRUE : FALSE,     // shift
                (shiftstate & 0x02) ? TRUE : FALSE,     // ctrl
                (shiftstate & 0x04) ? TRUE : FALSE,     // alt
                FALSE );
            bRet = TRUE;
        }
    }

    return bRet;
}

static long ImplHandleKeyMsg( HWND hWnd, UINT nMsg,
                              WPARAM wParam, LPARAM lParam, LRESULT& rResult )
{
    static bool         bIgnoreCharMsg  = FALSE;
    static WPARAM       nDeadChar       = 0;
    static WPARAM       nLastVKChar     = 0;
    static sal_uInt16   nLastChar       = 0;
    static ModKeyFlags  nLastModKeyCode = ModKeyFlags::NONE;
    static bool         bWaitForModKeyRelease = false;
    sal_uInt16          nRepeat         = LOWORD( lParam )-1;
    sal_uInt16          nModCode        = 0;

    // this key might have been relayed by SysChild and thus
    // may not be processed twice
    GetSalData()->mnSalObjWantKeyEvt = 0;

    if ( nMsg == WM_DEADCHAR )
    {
        nDeadChar = wParam;
        return 0;
    }

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    // reset the background mode for each text input,
    // as some tools such as RichWin may have changed it
    if ( pFrame->mpGraphics &&
         pFrame->mpGraphics->getHDC() )
        SetBkMode( pFrame->mpGraphics->getHDC(), TRANSPARENT );

    // determine modifiers
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        nModCode |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        nModCode |= KEY_MOD1;
    if ( GetKeyState( VK_LMENU ) & 0x8000 )
        nModCode |= KEY_MOD2;
    if ( GetKeyState( VK_RMENU ) & 0x8000 )  // this is the ALTGR-Key in this case
        nModCode &= ~KEY_MOD1;               // remove the Control flag

    if ( (nMsg == WM_CHAR) || (nMsg == WM_SYSCHAR) )
    {
        nDeadChar = 0;

        if ( bIgnoreCharMsg )
        {
            bIgnoreCharMsg = FALSE;
            // #101635# if zero is returned here for WM_SYSCHAR (ALT+<key>) Windows will beep
            // because this 'hotkey' was not processed -> better return 1
            // except for Alt-SPACE which should always open the sysmenu (#104616#)

            // also return zero if a system menubar is available that might process this hotkey
            // this also applies to the OLE inplace embedding where we are a child window
            if( (GetWindowStyle( hWnd ) & WS_CHILD) || GetMenu( hWnd ) || (wParam == 0x20) )
                return 0;
            else
                return 1;
        }

        // ignore backspace as a single key, so that
        // we do not get problems for combinations w/ a DeadKey
        if ( wParam == 0x08 )    // BACKSPACE
            return 0;

        // only "free flying" WM_CHAR messages arrive here, that are
        // created by typing a ALT-NUMPAD combination
        SalKeyEvent aKeyEvt;

        if ( (wParam >= '0') && (wParam <= '9') )
            aKeyEvt.mnCode = sal::static_int_cast<sal_uInt16>(KEYGROUP_NUM + wParam - '0');
        else if ( (wParam >= 'A') && (wParam <= 'Z') )
            aKeyEvt.mnCode = sal::static_int_cast<sal_uInt16>(KEYGROUP_ALPHA + wParam - 'A');
        else if ( (wParam >= 'a') && (wParam <= 'z') )
            aKeyEvt.mnCode = sal::static_int_cast<sal_uInt16>(KEYGROUP_ALPHA + wParam - 'a');
        else if ( wParam == 0x0D )    // RETURN
            aKeyEvt.mnCode = KEY_RETURN;
        else if ( wParam == 0x1B )    // ESCAPE
            aKeyEvt.mnCode = KEY_ESCAPE;
        else if ( wParam == 0x09 )    // TAB
            aKeyEvt.mnCode = KEY_TAB;
        else if ( wParam == 0x20 )    // SPACE
            aKeyEvt.mnCode = KEY_SPACE;
        else
            aKeyEvt.mnCode = 0;

        aKeyEvt.mnTime      = GetMessageTime();
        aKeyEvt.mnCode     |= nModCode;
        aKeyEvt.mnCharCode  = ImplGetCharCode( pFrame, wParam );
        aKeyEvt.mnRepeat    = nRepeat;
        nLastChar = 0;
        nLastVKChar = 0;
        long nRet = pFrame->CallCallback( SalEvent::KeyInput, &aKeyEvt );
        pFrame->CallCallback( SalEvent::KeyUp, &aKeyEvt );
        return nRet;
    }
     // #i11583#, MCD, 2003-01-13, Support for WM_UNICHAR & Keyman 6.0; addition begins
    else if( nMsg == WM_UNICHAR )
     {
         // If Windows is asking if we accept WM_UNICHAR, return TRUE
         if(wParam == UNICODE_NOCHAR)
        {
            rResult = TRUE; // ssa: this will actually return TRUE to windows
            return 1;       // ...but this will only avoid calling the defwindowproc
        }

         SalKeyEvent aKeyEvt;
         aKeyEvt.mnCode     = nModCode; // Or should it be 0? - as this is always a character returned
         aKeyEvt.mnTime     = GetMessageTime();
         aKeyEvt.mnRepeat   = 0;

        if( wParam >= Uni_SupplementaryPlanesStart )
        {
            // character is supplementary char in UTF-32 format - must be converted to UTF-16 supplementary pair
            // sal_Unicode ch = (sal_Unicode) Uni_UTF32ToSurrogate1(wParam);
             nLastChar = 0;
             nLastVKChar = 0;
             pFrame->CallCallback( SalEvent::KeyInput, &aKeyEvt );
             pFrame->CallCallback( SalEvent::KeyUp, &aKeyEvt );
            wParam = (sal_Unicode) Uni_UTF32ToSurrogate2( wParam );
         }

         aKeyEvt.mnCharCode = (sal_Unicode) wParam;

         nLastChar = 0;
         nLastVKChar = 0;
         long nRet = pFrame->CallCallback( SalEvent::KeyInput, &aKeyEvt );
         pFrame->CallCallback( SalEvent::KeyUp, &aKeyEvt );

         return nRet;
     }
     // MCD, 2003-01-13, Support for WM_UNICHAR & Keyman 6.0; addition ends
    else
    {
        // for shift, control and menu we issue a KeyModChange event
        if ( (wParam == VK_SHIFT) || (wParam == VK_CONTROL) || (wParam == VK_MENU) )
        {
            SalKeyModEvent aModEvt;
            aModEvt.mnTime = GetMessageTime();
            aModEvt.mnCode = nModCode;
            aModEvt.mnModKeyCode = ModKeyFlags::NONE;   // no command events will be sent if this member is 0

            ModKeyFlags tmpCode = ModKeyFlags::NONE;
            if( GetKeyState( VK_LSHIFT )  & 0x8000 )
                tmpCode |= ModKeyFlags::LeftShift;
            if( GetKeyState( VK_RSHIFT )  & 0x8000 )
                tmpCode |= ModKeyFlags::RightShift;
            if( GetKeyState( VK_LCONTROL ) & 0x8000 )
                tmpCode |= ModKeyFlags::LeftMod1;
            if( GetKeyState( VK_RCONTROL ) & 0x8000 )
                tmpCode |= ModKeyFlags::RightMod1;
            if( GetKeyState( VK_LMENU )  & 0x8000 )
                tmpCode |= ModKeyFlags::LeftMod2;
            if( GetKeyState( VK_RMENU )  & 0x8000 )
                tmpCode |= ModKeyFlags::RightMod2;

            if( tmpCode < nLastModKeyCode )
            {
                aModEvt.mnModKeyCode = nLastModKeyCode;
                nLastModKeyCode = ModKeyFlags::NONE;
                bWaitForModKeyRelease = true;
            }
            else
            {
                if( !bWaitForModKeyRelease )
                    nLastModKeyCode = tmpCode;
            }

            if( tmpCode == ModKeyFlags::NONE )
                bWaitForModKeyRelease = false;

            return pFrame->CallCallback( SalEvent::KeyModChange, &aModEvt );
        }
        else
        {
            SalKeyEvent     aKeyEvt;
            SalEvent        nEvent;
            MSG             aCharMsg;
            BOOL            bCharPeek = FALSE;
            UINT            nCharMsg = WM_CHAR;
            bool            bKeyUp = (nMsg == WM_KEYUP) || (nMsg == WM_SYSKEYUP);

            nLastModKeyCode = ModKeyFlags::NONE; // make sure no modkey messages are sent if they belong to a hotkey (see above)
            aKeyEvt.mnCharCode = 0;
            aKeyEvt.mnCode = 0;

            aKeyEvt.mnCode = ImplSalGetKeyCode( wParam );
            if ( !bKeyUp )
            {
                // check for charcode
                // Get the related WM_CHAR message using PeekMessage, if available.
                // The WM_CHAR message is always at the beginning of the
                // message queue. Also it is made certain that there is always only
                // one WM_CHAR message in the queue.
                bCharPeek = PeekMessageW( &aCharMsg, hWnd,
                                             WM_CHAR, WM_CHAR, PM_NOREMOVE | PM_NOYIELD );
                if ( bCharPeek && (nDeadChar == aCharMsg.wParam) )
                {
                    bCharPeek = FALSE;
                    nDeadChar = 0;

                    if ( wParam == VK_BACK )
                    {
                        PeekMessageW( &aCharMsg, hWnd,
                                         nCharMsg, nCharMsg, PM_REMOVE | PM_NOYIELD );
                        return 0;
                    }
                }
                else
                {
                    if ( !bCharPeek )
                    {
                        bCharPeek = PeekMessageW( &aCharMsg, hWnd,
                                                    WM_SYSCHAR, WM_SYSCHAR, PM_NOREMOVE | PM_NOYIELD );
                        nCharMsg = WM_SYSCHAR;
                    }
                }
                if ( bCharPeek )
                    aKeyEvt.mnCharCode = ImplGetCharCode( pFrame, aCharMsg.wParam );
                else
                    aKeyEvt.mnCharCode = 0;

                nLastChar = aKeyEvt.mnCharCode;
                nLastVKChar = wParam;
            }
            else
            {
                if ( wParam == nLastVKChar )
                {
                    aKeyEvt.mnCharCode = nLastChar;
                    nLastChar = 0;
                    nLastVKChar = 0;
                }
            }

            if ( aKeyEvt.mnCode || aKeyEvt.mnCharCode )
            {
                if ( bKeyUp )
                    nEvent = SalEvent::KeyUp;
                else
                    nEvent = SalEvent::KeyInput;

                aKeyEvt.mnTime      = GetMessageTime();
                aKeyEvt.mnCode     |= nModCode;
                aKeyEvt.mnRepeat    = nRepeat;

                bIgnoreCharMsg = bCharPeek ? TRUE : FALSE;
                long nRet = pFrame->CallCallback( nEvent, &aKeyEvt );
                // independent part only reacts on keyup but Windows does not send
                // keyup for VK_HANJA
                if( aKeyEvt.mnCode == KEY_HANGUL_HANJA )
                    nRet = pFrame->CallCallback( SalEvent::KeyUp, &aKeyEvt );

                bIgnoreCharMsg = FALSE;

                // char-message, than remove or ignore
                if ( bCharPeek )
                {
                    nDeadChar = 0;
                    if ( nRet )
                    {
                        PeekMessageW( &aCharMsg, hWnd,
                                         nCharMsg, nCharMsg, PM_REMOVE | PM_NOYIELD );
                    }
                    else
                        bIgnoreCharMsg = TRUE;
                }

                return nRet;
            }
            else
                return 0;
        }
    }
}

long ImplHandleSalObjKeyMsg( HWND hWnd, UINT nMsg,
                             WPARAM wParam, LPARAM lParam )
{
    if ( (nMsg == WM_KEYDOWN) || (nMsg == WM_KEYUP) )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( !pFrame )
            return 0;

        sal_uInt16  nRepeat     = LOWORD( lParam )-1;
        sal_uInt16  nModCode    = 0;

        // determine modifiers
        if ( GetKeyState( VK_SHIFT ) & 0x8000 )
            nModCode |= KEY_SHIFT;
        if ( GetKeyState( VK_CONTROL ) & 0x8000 )
            nModCode |= KEY_MOD1;
        if ( GetKeyState( VK_MENU ) & 0x8000 )
            nModCode |= KEY_MOD2;

        if ( (wParam != VK_SHIFT) && (wParam != VK_CONTROL) && (wParam != VK_MENU) )
        {
            SalKeyEvent     aKeyEvt;
            SalEvent        nEvent;
            bool            bKeyUp = (nMsg == WM_KEYUP) || (nMsg == WM_SYSKEYUP);

            // convert KeyCode
            aKeyEvt.mnCode      = ImplSalGetKeyCode( wParam );
            aKeyEvt.mnCharCode  = 0;

            if ( aKeyEvt.mnCode )
            {
                if ( bKeyUp )
                    nEvent = SalEvent::KeyUp;
                else
                    nEvent = SalEvent::KeyInput;

                aKeyEvt.mnTime      = GetMessageTime();
                aKeyEvt.mnCode     |= nModCode;
                aKeyEvt.mnRepeat    = nRepeat;
                long nRet = pFrame->CallCallback( nEvent, &aKeyEvt );
                return nRet;
            }
            else
                return 0;
        }
    }

    return 0;
}

long ImplHandleSalObjSysCharMsg( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    sal_uInt16  nRepeat     = LOWORD( lParam )-1;
    sal_uInt16  nModCode    = 0;
    sal_uInt16  cKeyCode    = (sal_uInt16)wParam;

    // determine modifiers
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        nModCode |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        nModCode |= KEY_MOD1;
    nModCode |= KEY_MOD2;

    // assemble KeyEvent
    SalKeyEvent aKeyEvt;
    aKeyEvt.mnTime      = GetMessageTime();
    if ( (cKeyCode >= 48) && (cKeyCode <= 57) )
        aKeyEvt.mnCode = KEY_0+(cKeyCode-48);
    else if ( (cKeyCode >= 65) && (cKeyCode <= 90) )
        aKeyEvt.mnCode = KEY_A+(cKeyCode-65);
    else  if ( (cKeyCode >= 97) && (cKeyCode <= 122) )
        aKeyEvt.mnCode = KEY_A+(cKeyCode-97);
    else
        aKeyEvt.mnCode = 0;
    aKeyEvt.mnCode     |= nModCode;
    aKeyEvt.mnCharCode  = ImplGetCharCode( pFrame, cKeyCode );
    aKeyEvt.mnRepeat    = nRepeat;
    long nRet = pFrame->CallCallback( SalEvent::KeyInput, &aKeyEvt );
    pFrame->CallCallback( SalEvent::KeyUp, &aKeyEvt );
    return nRet;
}

static bool ImplHandlePaintMsg( HWND hWnd )
{
    bool bMutex = FALSE;
    if ( ImplSalYieldMutexTryToAcquire() )
        bMutex = TRUE;

    // if we don't get the mutex, we can also change the clip region,
    // because other threads doesn't use the mutex from the main
    // thread --> see AcquireGraphics()

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        // clip-region must be reset, as we do not get a proper
        // bounding-rectangle otherwise
        if ( pFrame->mpGraphics && pFrame->mpGraphics->getRegion() )
            SelectClipRgn( pFrame->mpGraphics->getHDC(), nullptr );

        // according to Window-Documentation one shall check first if
        // there really is a paint-region
        if ( GetUpdateRect( hWnd, nullptr, FALSE ) )
        {
            // Call BeginPaint/EndPaint to query the rect and send
            // this Notofication to rect
            RECT aUpdateRect;
            PAINTSTRUCT aPs;
            BeginPaint( hWnd, &aPs );
            CopyRect( &aUpdateRect, &aPs.rcPaint );

            // Paint
            // reset ClipRegion
            if ( pFrame->mpGraphics && pFrame->mpGraphics->getRegion() )
            {
                SelectClipRgn( pFrame->mpGraphics->getHDC(),
                               pFrame->mpGraphics->getRegion() );
            }

            if ( bMutex )
            {
                SalPaintEvent aPEvt( aUpdateRect.left, aUpdateRect.top, aUpdateRect.right-aUpdateRect.left, aUpdateRect.bottom-aUpdateRect.top, pFrame->mbPresentation );
                pFrame->CallCallback( SalEvent::Paint, &aPEvt );
            }
            else
            {
                RECT* pRect = new RECT;
                CopyRect( pRect, &aUpdateRect );
                BOOL const ret = PostMessageW(hWnd, SAL_MSG_POSTPAINT, reinterpret_cast<WPARAM>(pRect), 0);
                SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
            }
            EndPaint( hWnd, &aPs );
        }
        else
        {
            // reset ClipRegion
            if ( pFrame->mpGraphics && pFrame->mpGraphics->getRegion() )
            {
                SelectClipRgn( pFrame->mpGraphics->getHDC(),
                               pFrame->mpGraphics->getRegion() );
            }
        }
    }

    if ( bMutex )
        ImplSalYieldMutexRelease();

    return bMutex;
}

static void ImplHandlePaintMsg2( HWND hWnd, RECT* pRect )
{
    // Paint
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            SalPaintEvent aPEvt( pRect->left, pRect->top, pRect->right-pRect->left, pRect->bottom-pRect->top );
            pFrame->CallCallback( SalEvent::Paint, &aPEvt );
        }
        ImplSalYieldMutexRelease();
        delete pRect;
    }
    else
    {
        BOOL const ret = PostMessageW(hWnd, SAL_MSG_POSTPAINT, reinterpret_cast<WPARAM>(pRect), 0);
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
}

static void SetMaximizedFrameGeometry( HWND hWnd, WinSalFrame* pFrame, RECT* pParentRect )
{
    // calculate and set frame geometry of a maximized window - useful if the window is still hidden

    // dualmonitor support:
    // Get screensize of the monitor with the mouse pointer

    RECT aRectMouse;
    if( ! pParentRect )
    {
        POINT pt;
        GetCursorPos( &pt );
        aRectMouse.left = pt.x;
        aRectMouse.top = pt.y;
        aRectMouse.right = pt.x+2;
        aRectMouse.bottom = pt.y+2;
        pParentRect = &aRectMouse;
    }

    RECT aRect;
    ImplSalGetWorkArea( hWnd, &aRect, pParentRect );

    // a maximized window has no other borders than the caption
    pFrame->maGeometry.nLeftDecoration = pFrame->maGeometry.nRightDecoration = pFrame->maGeometry.nBottomDecoration = 0;
    pFrame->maGeometry.nTopDecoration = pFrame->mbCaption ? GetSystemMetrics( SM_CYCAPTION ) : 0;

    aRect.top += pFrame->maGeometry.nTopDecoration;
    pFrame->maGeometry.nX = aRect.left;
    pFrame->maGeometry.nY = aRect.top;
    pFrame->maGeometry.nWidth = aRect.right - aRect.left;
    pFrame->maGeometry.nHeight = aRect.bottom - aRect.top;
}

static void UpdateFrameGeometry( HWND hWnd, WinSalFrame* pFrame )
{
    if( !pFrame )
        return;

    RECT aRect;
    GetWindowRect( hWnd, &aRect );
    memset(&pFrame->maGeometry, 0, sizeof(SalFrameGeometry) );

    if ( IsIconic( hWnd ) )
        return;

    POINT aPt;
    aPt.x=0;
    aPt.y=0;
    ClientToScreen(hWnd, &aPt);
    int cx = aPt.x - aRect.left;
    pFrame->maGeometry.nTopDecoration = aPt.y - aRect.top;

    pFrame->maGeometry.nLeftDecoration = cx;
    pFrame->maGeometry.nRightDecoration = cx;

    pFrame->maGeometry.nX = aPt.x;
    pFrame->maGeometry.nY = aPt.y;

    RECT aInnerRect;
    GetClientRect( hWnd, &aInnerRect );
    if( aInnerRect.right )
    {
        // improve right decoration
        aPt.x=aInnerRect.right;
        aPt.y=aInnerRect.top;
        ClientToScreen(hWnd, &aPt);
        pFrame->maGeometry.nRightDecoration = aRect.right - aPt.x;
    }
    if( aInnerRect.bottom ) // may be zero if window was not shown yet
        pFrame->maGeometry.nBottomDecoration += aRect.bottom - aPt.y - aInnerRect.bottom;
    else
        // bottom border is typically the same as left/right
        pFrame->maGeometry.nBottomDecoration = pFrame->maGeometry.nLeftDecoration;

    int nWidth  = aRect.right - aRect.left
        - pFrame->maGeometry.nRightDecoration - pFrame->maGeometry.nLeftDecoration;
    int nHeight = aRect.bottom - aRect.top
        - pFrame->maGeometry.nBottomDecoration - pFrame->maGeometry.nTopDecoration;
    // clamp to zero
    pFrame->maGeometry.nHeight = nHeight < 0 ? 0 : nHeight;
    pFrame->maGeometry.nWidth = nWidth < 0 ? 0 : nWidth;
    pFrame->updateScreenNumber();
}

static void ImplCallMoveHdl( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SalEvent::Move, nullptr );
        // to avoid doing Paint twice by VCL and SAL
        //if ( IsWindowVisible( hWnd ) && !pFrame->mbInShow )
        //    UpdateWindow( hWnd );
    }
}

static void ImplCallClosePopupsHdl( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SalEvent::ClosePopups, nullptr );
    }
}

static void ImplHandleMoveMsg( HWND hWnd )
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            UpdateFrameGeometry( hWnd, pFrame );

            if ( GetWindowStyle( hWnd ) & WS_VISIBLE )
                pFrame->mbDefPos = FALSE;

            // protect against recursion
            if ( !pFrame->mbInMoveMsg )
            {
                // adjust window again for FullScreenMode
                pFrame->mbInMoveMsg = TRUE;
                if ( pFrame->mbFullScreen )
                    ImplSalFrameFullScreenPos( pFrame );
                pFrame->mbInMoveMsg = FALSE;
            }

            // save state
            ImplSaveFrameState( pFrame );

            // Call Hdl
            //#93851 if we call this handler, VCL floating windows are not updated correctly
            ImplCallMoveHdl( hWnd );

        }

        ImplSalYieldMutexRelease();
    }
    else
    {
        BOOL const ret = PostMessageW( hWnd, SAL_MSG_POSTMOVE, 0, 0 );
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
}

static void ImplCallSizeHdl( HWND hWnd )
{
    // as Windows can send these messages also, we have to use
    // the Solar semaphore
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->CallCallback( SalEvent::Resize, nullptr );
            // to avoid double Paints by VCL and SAL
            if ( IsWindowVisible( hWnd ) && !pFrame->mbInShow )
                UpdateWindow( hWnd );
        }

        ImplSalYieldMutexRelease();
    }
    else
    {
        BOOL const ret = PostMessageW( hWnd, SAL_MSG_POSTCALLSIZE, 0, 0 );
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
}

static void ImplHandleSizeMsg( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    if ( (wParam != SIZE_MAXSHOW) && (wParam != SIZE_MAXHIDE) )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            UpdateFrameGeometry( hWnd, pFrame );

            pFrame->mnWidth  = (int)LOWORD(lParam);
            pFrame->mnHeight = (int)HIWORD(lParam);
            // save state
            ImplSaveFrameState( pFrame );
            // Call Hdl
            ImplCallSizeHdl( hWnd );
        }
    }
}

static void ImplHandleFocusMsg( HWND hWnd )
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && !WinSalFrame::mbInReparent )
        {
            if ( ::GetFocus() == hWnd )
            {
                if ( IsWindowVisible( hWnd ) && !pFrame->mbInShow )
                    UpdateWindow( hWnd );

                // do we support IME?
                if ( pFrame->mbIME && pFrame->mhDefIMEContext )
                {
                    UINT nImeProps = ImmGetProperty( GetKeyboardLayout( 0 ), IGP_PROPERTY );

                    pFrame->mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
                    pFrame->mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
                    pFrame->mbHandleIME = !pFrame->mbSpezIME;
                }

                pFrame->CallCallback( SalEvent::GetFocus, nullptr );
            }
            else
            {
                pFrame->CallCallback( SalEvent::LoseFocus, nullptr );
            }
        }

        ImplSalYieldMutexRelease();
    }
    else
    {
        BOOL const ret = PostMessageW( hWnd, SAL_MSG_POSTFOCUS, 0, 0 );
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
}

static void ImplHandleCloseMsg( HWND hWnd )
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->CallCallback( SalEvent::Close, nullptr );
        }

        ImplSalYieldMutexRelease();
    }
    else
    {
        BOOL const ret = PostMessageW( hWnd, WM_CLOSE, 0, 0 );
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
}

static long ImplHandleShutDownMsg( HWND hWnd )
{
    ImplSalYieldMutexAcquireWithWait();
    long        nRet = 0;
    WinSalFrame*   pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        nRet = pFrame->CallCallback( SalEvent::Shutdown, nullptr );
    }
    ImplSalYieldMutexRelease();
    return nRet;
}

static void ImplHandleSettingsChangeMsg( HWND hWnd, UINT nMsg,
                                         WPARAM wParam, LPARAM lParam )
{
    SalEvent nSalEvent = SalEvent::SettingsChanged;

    if ( nMsg == WM_DEVMODECHANGE )
        nSalEvent = SalEvent::PrinterChanged;
    else if ( nMsg == WM_DISPLAYCHANGE )
        nSalEvent = SalEvent::DisplayChanged;
    else if ( nMsg == WM_FONTCHANGE )
        nSalEvent = SalEvent::FontChanged;
    else if ( nMsg == WM_WININICHANGE )
    {
        if ( lParam )
        {
            if ( ImplSalWICompareAscii( reinterpret_cast<const wchar_t*>(lParam), "devices" ) == 0 )
                nSalEvent = SalEvent::PrinterChanged;
        }
    }

    if ( nMsg == WM_SETTINGCHANGE )
    {
        if ( wParam == SPI_SETWHEELSCROLLLINES )
            aSalShlData.mnWheelScrollLines = ImplSalGetWheelScrollLines();
        else if( wParam == SPI_SETWHEELSCROLLCHARS )
            aSalShlData.mnWheelScrollChars = ImplSalGetWheelScrollChars();
    }

    if ( WM_SYSCOLORCHANGE == nMsg && GetSalData()->mhDitherPal )
        ImplUpdateSysColorEntries();

    ImplSalYieldMutexAcquireWithWait();

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        if ( (nMsg == WM_DISPLAYCHANGE) || (nMsg == WM_WININICHANGE) )
        {
            if ( pFrame->mbFullScreen )
                ImplSalFrameFullScreenPos( pFrame );
        }

        pFrame->CallCallback( nSalEvent, nullptr );
    }

    ImplSalYieldMutexRelease();
}

static void ImplHandleUserEvent( HWND hWnd, LPARAM lParam )
{
    ImplSalYieldMutexAcquireWithWait();
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SalEvent::UserEvent, reinterpret_cast<void*>(lParam) );
    }
    ImplSalYieldMutexRelease();
}

static void ImplHandleForcePalette( HWND hWnd )
{
    SalData*    pSalData = GetSalData();
    HPALETTE    hPal = pSalData->mhDitherPal;
    if ( hPal )
    {
        if ( !ImplSalYieldMutexTryToAcquire() )
        {
            BOOL const ret = PostMessageW( hWnd, SAL_MSG_FORCEPALETTE, 0, 0 );
            SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
            return;
        }

        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && pFrame->mpGraphics )
        {
            WinSalGraphics* pGraphics = pFrame->mpGraphics;
            if ( pGraphics && pGraphics->getDefPal() )
            {
                SelectPalette( pGraphics->getHDC(), hPal, FALSE );
                if ( RealizePalette( pGraphics->getHDC() ) )
                {
                    InvalidateRect( hWnd, nullptr, FALSE );
                    UpdateWindow( hWnd );
                    pFrame->CallCallback( SalEvent::DisplayChanged, nullptr );
                }
            }
        }

        ImplSalYieldMutexRelease();
    }
}

static LRESULT ImplHandlePalette( bool bFrame, HWND hWnd, UINT nMsg,
                                  WPARAM wParam, LPARAM lParam, int& rDef )
{
    SalData*    pSalData = GetSalData();
    HPALETTE    hPal = pSalData->mhDitherPal;
    if ( !hPal )
        return 0;

    rDef = FALSE;
    if ( pSalData->mbInPalChange )
        return 0;

    if ( (nMsg == WM_PALETTECHANGED) || (nMsg == SAL_MSG_POSTPALCHANGED) )
    {
        if ( reinterpret_cast<HWND>(wParam) == hWnd )
            return 0;
    }

    bool bReleaseMutex = FALSE;
    if ( (nMsg == WM_QUERYNEWPALETTE) || (nMsg == WM_PALETTECHANGED) )
    {
        // as Windows can send these messages also, we have to use
        // the Solar semaphore
        if ( ImplSalYieldMutexTryToAcquire() )
            bReleaseMutex = TRUE;
        else if ( nMsg == WM_QUERYNEWPALETTE )
        {
            BOOL const ret = PostMessageW(hWnd, SAL_MSG_POSTQUERYNEWPAL, wParam, lParam);
            SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
        }
        else /* ( nMsg == WM_PALETTECHANGED ) */
        {
            BOOL const ret = PostMessageW(hWnd, SAL_MSG_POSTPALCHANGED, wParam, lParam);
            SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
        }
    }

    WinSalVirtualDevice*pTempVD;
    WinSalFrame*        pTempFrame;
    WinSalGraphics*     pGraphics;
    HDC                 hDC;
    HPALETTE            hOldPal;
    UINT                nCols;
    bool                bStdDC;
    bool                bUpdate;

    pSalData->mbInPalChange = true;

    // reset all palettes in VirDevs and Frames
    pTempVD = pSalData->mpFirstVD;
    while ( pTempVD )
    {
        pGraphics = pTempVD->getGraphics();
        if ( pGraphics->getDefPal() )
        {
            SelectPalette( pGraphics->getHDC(),
                           pGraphics->getDefPal(),
                           TRUE );
        }
        pTempVD = pTempVD->getNext();
    }
    pTempFrame = pSalData->mpFirstFrame;
    while ( pTempFrame )
    {
        pGraphics = pTempFrame->mpGraphics;
        if ( pGraphics && pGraphics->getDefPal() )
        {
            SelectPalette( pGraphics->getHDC(),
                           pGraphics->getDefPal(),
                           TRUE );
        }
        pTempFrame = pTempFrame->mpNextFrame;
    }

    // re-initialize palette
    WinSalFrame* pFrame = nullptr;
    if ( bFrame )
        pFrame = GetWindowPtr( hWnd );
    if ( pFrame && pFrame->mpGraphics )
    {
        hDC = pFrame->mpGraphics->getHDC();
        bStdDC = TRUE;
    }
    else
    {
        hDC = GetDC( hWnd );
        bStdDC = FALSE;
    }
    UnrealizeObject( hPal );
    hOldPal = SelectPalette( hDC, hPal, TRUE );
    nCols = RealizePalette( hDC );
    bUpdate = nCols != 0;
    if ( !bStdDC )
    {
        SelectPalette( hDC, hOldPal, TRUE );
        ReleaseDC( hWnd, hDC );
    }

    // reset all palettes in VirDevs and Frames
    pTempVD = pSalData->mpFirstVD;
    while ( pTempVD )
    {
        pGraphics = pTempVD->getGraphics();
        if ( pGraphics->getDefPal() )
        {
            SelectPalette( pGraphics->getHDC(), hPal, TRUE );
            RealizePalette( pGraphics->getHDC() );
        }
        pTempVD = pTempVD->getNext();
    }
    pTempFrame = pSalData->mpFirstFrame;
    while ( pTempFrame )
    {
        if ( pTempFrame != pFrame )
        {
            pGraphics = pTempFrame->mpGraphics;
            if ( pGraphics && pGraphics->getDefPal() )
            {
                SelectPalette( pGraphics->getHDC(), hPal, TRUE );
                if ( RealizePalette( pGraphics->getHDC() ) )
                    bUpdate = TRUE;
            }
        }
        pTempFrame = pTempFrame->mpNextFrame;
    }

    // if colors changed, update the window
    if ( bUpdate )
    {
        pTempFrame = pSalData->mpFirstFrame;
        while ( pTempFrame )
        {
            pGraphics = pTempFrame->mpGraphics;
            if ( pGraphics && pGraphics->getDefPal() )
            {
                InvalidateRect( pTempFrame->mhWnd, nullptr, FALSE );
                UpdateWindow( pTempFrame->mhWnd );
                pTempFrame->CallCallback( SalEvent::DisplayChanged, nullptr );
            }
            pTempFrame = pTempFrame->mpNextFrame;
        }
    }

    pSalData->mbInPalChange = false;

    if ( bReleaseMutex )
        ImplSalYieldMutexRelease();

    if ( nMsg == WM_PALETTECHANGED )
        return 0;
    else
        return nCols;
}

static int ImplHandleMinMax( HWND hWnd, LPARAM lParam )
{
    int bRet = FALSE;

    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            MINMAXINFO* pMinMax = reinterpret_cast<MINMAXINFO*>(lParam);

            if ( pFrame->mbFullScreen )
            {
                int         nX;
                int         nY;
                int         nDX;
                int         nDY;
                ImplSalCalcFullScreenSize( pFrame, nX, nY, nDX, nDY );

                if ( pMinMax->ptMaxPosition.x > nX )
                    pMinMax->ptMaxPosition.x = nX;
                if ( pMinMax->ptMaxPosition.y > nY )
                    pMinMax->ptMaxPosition.y = nY;

                if ( pMinMax->ptMaxSize.x < nDX )
                    pMinMax->ptMaxSize.x = nDX;
                if ( pMinMax->ptMaxSize.y < nDY )
                    pMinMax->ptMaxSize.y = nDY;
                if ( pMinMax->ptMaxTrackSize.x < nDX )
                    pMinMax->ptMaxTrackSize.x = nDX;
                if ( pMinMax->ptMaxTrackSize.y < nDY )
                    pMinMax->ptMaxTrackSize.y = nDY;

                pMinMax->ptMinTrackSize.x = nDX;
                pMinMax->ptMinTrackSize.y = nDY;

                bRet = TRUE;
            }

            if ( pFrame->mnMinWidth || pFrame->mnMinHeight )
            {
                int nWidth   = pFrame->mnMinWidth;
                int nHeight  = pFrame->mnMinHeight;

                ImplSalAddBorder( pFrame, nWidth, nHeight );

                if ( pMinMax->ptMinTrackSize.x < nWidth )
                     pMinMax->ptMinTrackSize.x = nWidth;
                if ( pMinMax->ptMinTrackSize.y < nHeight )
                     pMinMax->ptMinTrackSize.y = nHeight;
            }

            if ( pFrame->mnMaxWidth || pFrame->mnMaxHeight )
            {
                int nWidth   = pFrame->mnMaxWidth;
                int nHeight  = pFrame->mnMaxHeight;

                ImplSalAddBorder( pFrame, nWidth, nHeight );

                if( nWidth > 0 && nHeight > 0 ) // protect against int overflow due to INT_MAX initialisation
                {
                    if ( pMinMax->ptMaxTrackSize.x > nWidth )
                        pMinMax->ptMaxTrackSize.x = nWidth;
                    if ( pMinMax->ptMaxTrackSize.y > nHeight )
                        pMinMax->ptMaxTrackSize.y = nHeight;
                }
            }
        }

        ImplSalYieldMutexRelease();
    }

    return bRet;
}

// retrieves the SalMenuItem pointer from a hMenu
// the pointer is stored in every item, so if no position
// is specified we just use the first item (ie, pos=0)
// if bByPosition is FALSE then nPos denotes a menu id instead of a position
static WinSalMenuItem* ImplGetSalMenuItem( HMENU hMenu, UINT nPos, bool bByPosition=TRUE )
{
    MENUITEMINFOW mi;
    memset(&mi, 0, sizeof(mi));
    mi.cbSize = sizeof( mi );
    mi.fMask = MIIM_DATA;
    if( !GetMenuItemInfoW( hMenu, nPos, bByPosition, &mi) )
        SAL_WARN("vcl", "GetMenuItemInfoW failed: " << WindowsErrorString(GetLastError()));

    return reinterpret_cast<WinSalMenuItem *>(mi.dwItemData);
}

// returns the index of the currently selected item if any or -1
static int ImplGetSelectedIndex( HMENU hMenu )
{
    MENUITEMINFOW mi;
    memset(&mi, 0, sizeof(mi));
    mi.cbSize = sizeof( mi );
    mi.fMask = MIIM_STATE;
    int n = GetMenuItemCount( hMenu );
    if( n != -1 )
    {
        for(int i=0; i<n; i++ )
        {
            if( !GetMenuItemInfoW( hMenu, i, TRUE, &mi) )
                SAL_WARN( "vcl", "GetMenuItemInfoW faled: " << WindowsErrorString( GetLastError() ) );
            else
            {
                if( mi.fState & MFS_HILITE )
                    return i;
            }
        }
    }
    return -1;
}

static int ImplMenuChar( HWND, WPARAM wParam, LPARAM lParam )
{
    int nRet = MNC_IGNORE;
    HMENU hMenu = reinterpret_cast<HMENU>(lParam);
    OUString aMnemonic( "&" + OUStringLiteral1((sal_Unicode) LOWORD(wParam)) );
    aMnemonic = aMnemonic.toAsciiLowerCase();   // we only have ascii mnemonics

    // search the mnemonic in the current menu
    int nItemCount = GetMenuItemCount( hMenu );
    int nFound = 0;
    int idxFound = -1;
    int idxSelected = ImplGetSelectedIndex( hMenu );
    int idx = idxSelected != -1 ? idxSelected+1 : 0;    // if duplicate mnemonics cycle through menu
    for( int i=0; i< nItemCount; i++, idx++ )
    {
        WinSalMenuItem* pSalMenuItem = ImplGetSalMenuItem( hMenu, idx % nItemCount );
        if( !pSalMenuItem )
            continue;
        OUString aStr = pSalMenuItem->mText;
        aStr = aStr.toAsciiLowerCase();
        if( aStr.indexOf( aMnemonic ) != -1 )
        {
            if( idxFound == -1 )
                idxFound = idx % nItemCount;
            if( nFound++ )
                break;  // duplicate found
        }
    }
    if( nFound == 1 )
        nRet = MAKELRESULT( idxFound, MNC_EXECUTE );
    else
        // duplicate mnemonics, just select the next occurrence
        nRet = MAKELRESULT( idxFound, MNC_SELECT );

    return nRet;
}

static int ImplMeasureItem( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    int nRet = 0;
    if( !wParam )
    {
        // request was sent by a menu
        nRet = 1;
        MEASUREITEMSTRUCT *pMI = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
        if( pMI->CtlType != ODT_MENU )
            return 0;

        WinSalMenuItem *pSalMenuItem = reinterpret_cast<WinSalMenuItem *>(pMI->itemData);
        if( !pSalMenuItem )
            return 0;

        HDC hdc = GetDC( hWnd );
        SIZE strSize;

        NONCLIENTMETRICS ncm;
        memset( &ncm, 0, sizeof(ncm) );
        ncm.cbSize = sizeof( ncm );
        SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );

        // Assume every menu item can be default and printed bold
        //ncm.lfMenuFont.lfWeight = FW_BOLD;

        HFONT hfntOld = static_cast<HFONT>(SelectObject(hdc, CreateFontIndirect( &ncm.lfMenuFont )));

        // menu text and accelerator
        OUString aStr(pSalMenuItem->mText);
        if( pSalMenuItem->mAccelText.getLength() )
        {
            aStr += " ";
            aStr += pSalMenuItem->mAccelText;
        }
        GetTextExtentPoint32W( hdc, aStr.getStr(),
                                aStr.getLength(), &strSize );

        // image
        Size bmpSize( 16, 16 );
        //if( !!pSalMenuItem->maBitmap )
        //    bmpSize = pSalMenuItem->maBitmap.GetSizePixel();

        // checkmark
        Size checkSize( GetSystemMetrics( SM_CXMENUCHECK ), GetSystemMetrics( SM_CYMENUCHECK ) );

        pMI->itemWidth = checkSize.Width() + 3 + bmpSize.Width() + 3 + strSize.cx;
        pMI->itemHeight = std::max( std::max( checkSize.Height(), bmpSize.Height() ), strSize.cy );
        pMI->itemHeight += 4;

        DeleteObject( SelectObject(hdc, hfntOld) );
        ReleaseDC( hWnd, hdc );
    }

    return nRet;
}

static int ImplDrawItem(HWND, WPARAM wParam, LPARAM lParam )
{
    int nRet = 0;
    if( !wParam )
    {
        // request was sent by a menu
        nRet = 1;
        DRAWITEMSTRUCT *pDI = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
        if( pDI->CtlType != ODT_MENU )
            return 0;

        WinSalMenuItem *pSalMenuItem = reinterpret_cast<WinSalMenuItem *>(pDI->itemData);
        if( !pSalMenuItem )
            return 0;

        COLORREF clrPrevText, clrPrevBkgnd;
        HFONT hfntOld;
        HBRUSH hbrOld;
        bool    fChecked = (pDI->itemState & ODS_CHECKED) ? TRUE : FALSE;
        bool    fSelected = (pDI->itemState & ODS_SELECTED) ? TRUE : FALSE;
        bool    fDisabled = (pDI->itemState & (ODS_DISABLED | ODS_GRAYED)) ? TRUE : FALSE;

        // Set the appropriate foreground and background colors.
        RECT aRect = pDI->rcItem;

        clrPrevBkgnd = SetBkColor( pDI->hDC, GetSysColor( COLOR_MENU ) );

        if ( fDisabled )
            clrPrevText = SetTextColor( pDI->hDC, GetSysColor( COLOR_GRAYTEXT ) );
        else
            clrPrevText = SetTextColor( pDI->hDC, GetSysColor( fSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

        DWORD colBackground = GetSysColor( fSelected ? COLOR_HIGHLIGHT : COLOR_MENU );
        clrPrevBkgnd = SetBkColor( pDI->hDC, colBackground );

        hbrOld = static_cast<HBRUSH>(SelectObject( pDI->hDC, CreateSolidBrush( GetBkColor( pDI->hDC ) ) ));

        // Fill background
        if(!PatBlt( pDI->hDC, aRect.left, aRect.top, aRect.right-aRect.left, aRect.bottom-aRect.top, PATCOPY ))
            SAL_WARN("vcl", "PatBlt failed: " << WindowsErrorString(GetLastError()));

        int lineHeight = aRect.bottom-aRect.top;

        int x = aRect.left;
        int y = aRect.top;

        int checkWidth  = GetSystemMetrics( SM_CXMENUCHECK );
        int checkHeight = GetSystemMetrics( SM_CYMENUCHECK );
        if( fChecked )
        {
            RECT r;
            r.left = 0;
            r.top = 0;
            r.right = checkWidth;
            r.bottom = checkWidth;
            HDC memDC = CreateCompatibleDC( pDI->hDC );
            HBITMAP memBmp = CreateCompatibleBitmap( pDI->hDC, checkWidth, checkHeight );
            HBITMAP hOldBmp = static_cast<HBITMAP>(SelectObject( memDC, memBmp ));
            DrawFrameControl( memDC, &r, DFC_MENU, DFCS_MENUCHECK );
            BitBlt( pDI->hDC, x, y+(lineHeight-checkHeight)/2, checkWidth, checkHeight, memDC, 0, 0, SRCAND );
            DeleteObject( SelectObject( memDC, hOldBmp ) );
            DeleteDC( memDC );
        }
        x += checkWidth+3;

        //Size bmpSize = aBitmap.GetSizePixel();
        Size bmpSize(16, 16);
        if( !!pSalMenuItem->maBitmap )
        {
            Bitmap aBitmap( pSalMenuItem->maBitmap );

            // set transparent pixels to background color
            if( fDisabled )
                colBackground = RGB(255,255,255);
            aBitmap.Replace( Color( COL_LIGHTMAGENTA ),
                Color( GetRValue(colBackground),GetGValue(colBackground),GetBValue(colBackground) ));

            WinSalBitmap* pSalBmp = static_cast<WinSalBitmap*>(aBitmap.ImplGetImpBitmap()->ImplGetSalBitmap());
            HGLOBAL hDrawDIB = pSalBmp->ImplGethDIB();

            if( hDrawDIB )
            {
                PBITMAPINFO         pBI = static_cast<PBITMAPINFO>(GlobalLock( hDrawDIB ));
                PBYTE               pBits = reinterpret_cast<PBYTE>(pBI) + pBI->bmiHeader.biSize +
                                            WinSalBitmap::ImplGetDIBColorCount( hDrawDIB ) * sizeof( RGBQUAD );

                HBITMAP hBmp = CreateDIBitmap( pDI->hDC, &pBI->bmiHeader, CBM_INIT, pBits, pBI, DIB_RGB_COLORS );
                GlobalUnlock( hDrawDIB );

                HBRUSH hbrIcon = CreateSolidBrush( GetSysColor( COLOR_GRAYTEXT ) );
                DrawStateW( pDI->hDC, hbrIcon, nullptr, reinterpret_cast<LPARAM>(hBmp), (WPARAM)0,
                    x, y+(lineHeight-bmpSize.Height())/2, bmpSize.Width(), bmpSize.Height(),
                     DST_BITMAP | (fDisabled ? (fSelected ? DSS_MONO : DSS_DISABLED) : DSS_NORMAL) );

                DeleteObject( hbrIcon );
                DeleteObject( hBmp );
            }

        }
        x += bmpSize.Width() + 3;
        aRect.left = x;

        NONCLIENTMETRICS ncm;
        memset( &ncm, 0, sizeof(ncm) );
        ncm.cbSize = sizeof( ncm );
        SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );

        // Print default menu entry with bold font
        //if ( pDI->itemState & ODS_DEFAULT )
        //    ncm.lfMenuFont.lfWeight = FW_BOLD;

        hfntOld = static_cast<HFONT>(SelectObject(pDI->hDC, CreateFontIndirect( &ncm.lfMenuFont )));

        SIZE strSize;
        OUString aStr( pSalMenuItem->mText );
        GetTextExtentPoint32W( pDI->hDC, aStr.getStr(),
                                aStr.getLength(), &strSize );

        if(!DrawStateW( pDI->hDC, nullptr, nullptr,
            reinterpret_cast<LPARAM>(aStr.getStr()),
            (WPARAM)0, aRect.left, aRect.top + (lineHeight - strSize.cy)/2, 0, 0,
            DST_PREFIXTEXT | (fDisabled && !fSelected ? DSS_DISABLED : DSS_NORMAL) ) )
            SAL_WARN("vcl", "DrawStateW failed: " << WindowsErrorString(GetLastError()));

        if( pSalMenuItem->mAccelText.getLength() )
        {
            SIZE strSizeA;
            aStr = pSalMenuItem->mAccelText;
            GetTextExtentPoint32W( pDI->hDC, aStr.getStr(),
                                    aStr.getLength(), &strSizeA );
            TEXTMETRIC tm;
            GetTextMetrics( pDI->hDC, &tm );

            // position the accelerator string to the right but leave space for the
            // (potential) submenu arrow (tm.tmMaxCharWidth)
            if(!DrawStateW( pDI->hDC, nullptr, nullptr,
                reinterpret_cast<LPARAM>(aStr.getStr()),
                (WPARAM)0, aRect.right-strSizeA.cx-tm.tmMaxCharWidth, aRect.top + (lineHeight - strSizeA.cy)/2, 0, 0,
                DST_TEXT | (fDisabled && !fSelected ? DSS_DISABLED : DSS_NORMAL) ) )
                SAL_WARN("vcl", "DrawStateW failed: " << WindowsErrorString(GetLastError()));
        }

        // Restore the original font and colors.
        DeleteObject( SelectObject( pDI->hDC, hbrOld ) );
        DeleteObject( SelectObject( pDI->hDC, hfntOld) );
        SetTextColor(pDI->hDC, clrPrevText);
        SetBkColor(pDI->hDC, clrPrevBkgnd);
    }
    return nRet;
}

static bool ImplHandleMenuActivate( HWND hWnd, WPARAM wParam, LPARAM )
{
    // Menu activation
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return false;

    HMENU hMenu = reinterpret_cast<HMENU>(wParam);
    // WORD nPos = LOWORD (lParam);
    // bool bWindowMenu = (bool) HIWORD(lParam);

    // Send activate and deactivate together, so we have not keep track of opened menus
    // this will be enough to have the menus updated correctly
    SalMenuEvent aMenuEvt;
    WinSalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, 0 );
    if( pSalMenuItem )
        aMenuEvt.mpMenu = pSalMenuItem->mpMenu;
    else
        aMenuEvt.mpMenu = nullptr;

    long nRet = pFrame->CallCallback( SalEvent::MenuActivate, &aMenuEvt );
    if( nRet )
        nRet = pFrame->CallCallback( SalEvent::MenuDeactivate, &aMenuEvt );
    if( nRet )
        pFrame->mLastActivatedhMenu = hMenu;

    return (nRet!=0);
}

static bool ImplHandleMenuSelect( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    // Menu selection
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return false;

    WORD nId = LOWORD(wParam);      // menu item or submenu index
    WORD nFlags = HIWORD(wParam);
    HMENU hMenu = reinterpret_cast<HMENU>(lParam);

    // check if we have to process the message
    if( !GetSalData()->IsKnownMenuHandle( hMenu ) )
        return false;

    bool bByPosition = FALSE;
    if( nFlags & MF_POPUP )
        bByPosition = TRUE;

    long nRet = 0;
    if ( hMenu && !pFrame->mLastActivatedhMenu )
    {
        // we never activated a menu (ie, no WM_INITMENUPOPUP has occurred yet)
        // which means this must be the menubar -> send activation/deactivation
        SalMenuEvent aMenuEvt;
        WinSalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, nId, bByPosition );
        if( pSalMenuItem )
            aMenuEvt.mpMenu = pSalMenuItem->mpMenu;
        else
            aMenuEvt.mpMenu = nullptr;

        nRet = pFrame->CallCallback( SalEvent::MenuActivate, &aMenuEvt );
        if( nRet )
            nRet = pFrame->CallCallback( SalEvent::MenuDeactivate, &aMenuEvt );
        if( nRet )
            pFrame->mLastActivatedhMenu = hMenu;
    }

    if( !hMenu && nFlags == 0xFFFF )
    {
        // all menus are closed, reset activation logic
        pFrame->mLastActivatedhMenu = nullptr;
    }

    if( hMenu )
    {
        // hMenu must be saved, as it is not passed in WM_COMMAND which always occurs after a selection
        // if a menu is closed due to a command selection then hMenu is NULL, but WM_COMMAND comes later
        // so we must not overwrite it in this case
        pFrame->mSelectedhMenu = hMenu;

        // send highlight event
        if( nFlags & MF_POPUP )
        {
            // submenu selected
            // wParam now carries an index instead of an id -> retrieve id
            MENUITEMINFOW mi;
            memset(&mi, 0, sizeof(mi));
            mi.cbSize = sizeof( mi );
            mi.fMask = MIIM_ID;
            if( GetMenuItemInfoW( hMenu, LOWORD(wParam), TRUE, &mi) )
                nId = sal::static_int_cast<WORD>(mi.wID);
        }

        SalMenuEvent aMenuEvt;
        aMenuEvt.mnId   = nId;
        WinSalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, nId, false );
        if( pSalMenuItem )
            aMenuEvt.mpMenu = pSalMenuItem->mpMenu;
        else
            aMenuEvt.mpMenu = nullptr;

        nRet = pFrame->CallCallback( SalEvent::MenuHighlight, &aMenuEvt );
    }

    return (nRet != 0);
}

static bool ImplHandleCommand( HWND hWnd, WPARAM wParam, LPARAM )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return false;

    long nRet = 0;
    if( !HIWORD(wParam) )
    {
        // Menu command
        WORD nId = LOWORD(wParam);
        if( nId )   // zero for separators
        {
            SalMenuEvent aMenuEvt;
            aMenuEvt.mnId   = nId;
            WinSalMenuItem *pSalMenuItem = ImplGetSalMenuItem( pFrame->mSelectedhMenu, nId, false );
            if( pSalMenuItem )
                aMenuEvt.mpMenu = pSalMenuItem->mpMenu;
            else
                aMenuEvt.mpMenu = nullptr;

            nRet = pFrame->CallCallback( SalEvent::MenuCommand, &aMenuEvt );
        }
    }
    return (nRet != 0);
}

static int ImplHandleSysCommand( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    WPARAM nCommand = wParam & 0xFFF0;

    if ( pFrame->mbFullScreen )
    {
        BOOL    bMaximize = IsZoomed( pFrame->mhWnd );
        BOOL    bMinimize = IsIconic( pFrame->mhWnd );
        if ( (nCommand == SC_SIZE) ||
             (!bMinimize && (nCommand == SC_MOVE)) ||
             (!bMaximize && (nCommand == SC_MAXIMIZE)) ||
             (bMaximize && (nCommand == SC_RESTORE)) )
        {
            return TRUE;
        }
    }

    if ( nCommand == SC_KEYMENU )
    {
        // do not process SC_KEYMENU if we have a native menu
        // Windows should handle this
        if( GetMenu( hWnd ) )
            return FALSE;

        // Process here KeyMenu events only for Alt to activate the MenuBar,
        // or if a SysChild window is in focus, as Alt-key-combinations are
        // only processed via this event
        if ( !LOWORD( lParam ) )
        {
            // Only trigger if no other key is pressed.
            // Contrary to Docu the CharCode is delivered with the x-coordinate
            // that is pressed in addition.
            // Also 32 for space, 99 for c, 100 for d, ...
            // As this is not documented, we check the state of the space-bar
            if ( GetKeyState( VK_SPACE ) & 0x8000 )
                return 0;

            // to avoid activating the MenuBar for Alt+MouseKey
            if ( (GetKeyState( VK_LBUTTON ) & 0x8000) ||
                 (GetKeyState( VK_RBUTTON ) & 0x8000) ||
                 (GetKeyState( VK_MBUTTON ) & 0x8000) ||
                 (GetKeyState( VK_SHIFT )   & 0x8000) )
                return 1;

            SalKeyEvent aKeyEvt;
            aKeyEvt.mnTime      = GetMessageTime();
            aKeyEvt.mnCode      = KEY_MENU;
            aKeyEvt.mnCharCode  = 0;
            aKeyEvt.mnRepeat    = 0;
            long nRet = pFrame->CallCallback( SalEvent::KeyInput, &aKeyEvt );
            pFrame->CallCallback( SalEvent::KeyUp, &aKeyEvt );
            return int(nRet != 0);
        }
        else
        {
            // check if a SysChild is in focus
            HWND hFocusWnd = ::GetFocus();
            if ( hFocusWnd && ImplFindSalObject( hFocusWnd ) )
            {
                char cKeyCode = (char)(unsigned char)LOWORD( lParam );
                // LowerCase
                if ( (cKeyCode >= 65) && (cKeyCode <= 90) )
                    cKeyCode += 32;
                // We only accept 0-9 and A-Z; all other keys have to be
                // processed by the SalObj hook
                if ( ((cKeyCode >= 48) && (cKeyCode <= 57)) ||
                     ((cKeyCode >= 97) && (cKeyCode <= 122)) )
                {
                    sal_uInt16 nModCode = 0;
                    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
                        nModCode |= KEY_SHIFT;
                    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
                        nModCode |= KEY_MOD1;
                    nModCode |= KEY_MOD2;

                    SalKeyEvent aKeyEvt;
                    aKeyEvt.mnTime      = GetMessageTime();
                    if ( (cKeyCode >= 48) && (cKeyCode <= 57) )
                        aKeyEvt.mnCode = KEY_0+(cKeyCode-48);
                    else
                        aKeyEvt.mnCode = KEY_A+(cKeyCode-97);
                    aKeyEvt.mnCode     |= nModCode;
                    aKeyEvt.mnCharCode  = cKeyCode;
                    aKeyEvt.mnRepeat    = 0;
                    long nRet = pFrame->CallCallback( SalEvent::KeyInput, &aKeyEvt );
                    pFrame->CallCallback( SalEvent::KeyUp, &aKeyEvt );
                    return int(nRet != 0);
                }
            }
        }
    }

    return FALSE;
}

static void ImplHandleInputLangChange( HWND hWnd, WPARAM, LPARAM lParam )
{
    ImplSalYieldMutexAcquireWithWait();

    // check if we support IME
    WinSalFrame* pFrame = GetWindowPtr( hWnd );

    if ( !pFrame )
        return;

    if ( pFrame->mbIME && pFrame->mhDefIMEContext )
    {
        HKL     hKL = reinterpret_cast<HKL>(lParam);
        UINT    nImeProps = ImmGetProperty( hKL, IGP_PROPERTY );

        pFrame->mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
        pFrame->mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
        pFrame->mbHandleIME = !pFrame->mbSpezIME;
    }

    // trigger input language and codepage update
    UINT nLang = pFrame->mnInputLang;
    ImplUpdateInputLang( pFrame );

    // notify change
    if( nLang != pFrame->mnInputLang )
        pFrame->CallCallback( SalEvent::InputLanguageChange, nullptr );

    // reinit spec. keys
    GetSalData()->initKeyCodeMap();

    ImplSalYieldMutexRelease();
}

static void ImplUpdateIMECursorPos( WinSalFrame* pFrame, HIMC hIMC )
{
    COMPOSITIONFORM aForm;
    memset( &aForm, 0, sizeof( aForm ) );

    // get cursor position and from it calculate default position
    // for the composition window
    SalExtTextInputPosEvent aPosEvt;
    pFrame->CallCallback( SalEvent::ExtTextInputPos, &aPosEvt );
    if ( (aPosEvt.mnX == -1) && (aPosEvt.mnY == -1) )
        aForm.dwStyle |= CFS_DEFAULT;
    else
    {
        aForm.dwStyle          |= CFS_POINT;
        aForm.ptCurrentPos.x    = aPosEvt.mnX;
        aForm.ptCurrentPos.y    = aPosEvt.mnY;
    }
    ImmSetCompositionWindow( hIMC, &aForm );

    // Because not all IME's use this values, we create
    // a Windows caret to force the Position from the IME
    if ( GetFocus() == pFrame->mhWnd )
    {
        CreateCaret( pFrame->mhWnd, nullptr,
                     aPosEvt.mnWidth, aPosEvt.mnHeight );
        SetCaretPos( aPosEvt.mnX, aPosEvt.mnY );
    }
}

static bool ImplHandleIMEStartComposition( HWND hWnd )
{
    bool bDef = TRUE;

    ImplSalYieldMutexAcquireWithWait();

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        HIMC hIMC = ImmGetContext( hWnd );
        if ( hIMC )
        {
            ImplUpdateIMECursorPos( pFrame, hIMC );
            ImmReleaseContext( hWnd, hIMC );
        }

        if ( pFrame->mbHandleIME )
        {
            if ( pFrame->mbAtCursorIME )
                bDef = FALSE;
        }
    }

    ImplSalYieldMutexRelease();

    return bDef;
}

static bool ImplHandleIMECompositionInput( WinSalFrame* pFrame,
                                               HIMC hIMC, LPARAM lParam )
{
    bool bDef = TRUE;

    // Init Event
    SalExtTextInputEvent    aEvt;
    aEvt.mpTextAttr         = nullptr;
    aEvt.mnCursorPos        = 0;
    aEvt.mnCursorFlags      = 0;

    // If we get a result string, then we handle this input
    if ( lParam & GCS_RESULTSTR )
    {
        bDef = FALSE;

        LONG nTextLen = ImmGetCompositionStringW( hIMC, GCS_RESULTSTR, nullptr, 0 ) / sizeof( WCHAR );
        if ( nTextLen >= 0 )
        {
            auto pTextBuf = std::unique_ptr<WCHAR[]>(new WCHAR[nTextLen]);
            ImmGetCompositionStringW( hIMC, GCS_RESULTSTR, pTextBuf.get(), nTextLen*sizeof( WCHAR ) );
            aEvt.maText = OUString( reinterpret_cast<const sal_Unicode*>(pTextBuf.get()), (sal_Int32)nTextLen );
        }

        aEvt.mnCursorPos = aEvt.maText.getLength();
        pFrame->CallCallback( SalEvent::ExtTextInput, &aEvt );
        pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
        ImplUpdateIMECursorPos( pFrame, hIMC );
    }

    // If the IME doesn't support OnSpot input, then there is nothing to do
    if ( !pFrame->mbAtCursorIME )
        return !bDef;

    // If we get new Composition data, then we handle this new input
    if ( (lParam & (GCS_COMPSTR | GCS_COMPATTR)) ||
         ((lParam & GCS_CURSORPOS) && !(lParam & GCS_RESULTSTR)) )
    {
        bDef = FALSE;

        ExtTextInputAttr* pSalAttrAry = nullptr;
        LONG    nTextLen = ImmGetCompositionStringW( hIMC, GCS_COMPSTR, nullptr, 0 ) / sizeof( WCHAR );
        if ( nTextLen > 0 )
        {
            WCHAR* pTextBuf = new WCHAR[nTextLen];
            ImmGetCompositionStringW( hIMC, GCS_COMPSTR, pTextBuf, nTextLen*sizeof( WCHAR ) );
            aEvt.maText = OUString( reinterpret_cast<const sal_Unicode*>(pTextBuf), (sal_Int32)nTextLen );
            delete [] pTextBuf;

            BYTE*   pAttrBuf = nullptr;
            LONG        nAttrLen = ImmGetCompositionStringW( hIMC, GCS_COMPATTR, nullptr, 0 );
            if ( nAttrLen > 0 )
            {
                pAttrBuf = new BYTE[nAttrLen];
                ImmGetCompositionStringW( hIMC, GCS_COMPATTR, pAttrBuf, nAttrLen );
            }

            if ( pAttrBuf )
            {
                sal_Int32 nTextLen2 = aEvt.maText.getLength();
                pSalAttrAry = new ExtTextInputAttr[nTextLen2];
                memset( pSalAttrAry, 0, nTextLen2*sizeof( sal_uInt16 ) );
                for( sal_Int32 i = 0; (i < nTextLen2) && (i < nAttrLen); i++ )
                {
                    BYTE nWinAttr = pAttrBuf[i];
                    ExtTextInputAttr   nSalAttr;
                    if ( nWinAttr == ATTR_TARGET_CONVERTED )
                    {
                        nSalAttr = ExtTextInputAttr::BoldUnderline;
                        aEvt.mnCursorFlags |= EXTTEXTINPUT_CURSOR_INVISIBLE;
                    }
                    else if ( nWinAttr == ATTR_CONVERTED )
                        nSalAttr = ExtTextInputAttr::DashDotUnderline;
                    else if ( nWinAttr == ATTR_TARGET_NOTCONVERTED )
                        nSalAttr = ExtTextInputAttr::Highlight;
                    else if ( nWinAttr == ATTR_INPUT_ERROR )
                        nSalAttr = ExtTextInputAttr::RedText | ExtTextInputAttr::DottedUnderline;
                    else /* ( nWinAttr == ATTR_INPUT ) */
                        nSalAttr = ExtTextInputAttr::DottedUnderline;
                    pSalAttrAry[i] = nSalAttr;
                }

                aEvt.mpTextAttr = pSalAttrAry;
                delete [] pAttrBuf;
            }
        }

        // Only when we get new composition data, we must send this event
        if ( (nTextLen > 0) || !(lParam & GCS_RESULTSTR) )
        {
            // End the mode, if the last character is deleted
            if ( !nTextLen && !pFrame->mbCandidateMode )
            {
                pFrame->CallCallback( SalEvent::ExtTextInput, &aEvt );
                pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
            }
            else
            {
                // Because Cursor-Position and DeltaStart never updated
                // from the korean input engine, we must handle this here
                if ( lParam & CS_INSERTCHAR )
                {
                    aEvt.mnCursorPos = nTextLen;
                    if ( aEvt.mnCursorPos && (lParam & CS_NOMOVECARET) )
                        aEvt.mnCursorPos--;
                }
                else
                    aEvt.mnCursorPos = LOWORD( ImmGetCompositionStringW( hIMC, GCS_CURSORPOS, nullptr, 0 ) );

                if ( pFrame->mbCandidateMode )
                    aEvt.mnCursorFlags |= EXTTEXTINPUT_CURSOR_INVISIBLE;
                if ( lParam & CS_NOMOVECARET )
                    aEvt.mnCursorFlags |= EXTTEXTINPUT_CURSOR_OVERWRITE;

                pFrame->CallCallback( SalEvent::ExtTextInput, &aEvt );
            }
            ImplUpdateIMECursorPos( pFrame, hIMC );
        }

        if ( pSalAttrAry )
            delete [] pSalAttrAry;
    }

    return !bDef;
}

static bool ImplHandleIMEComposition( HWND hWnd, LPARAM lParam )
{
    bool bDef = TRUE;
    ImplSalYieldMutexAcquireWithWait();

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame && (!lParam || (lParam & GCS_RESULTSTR)) )
    {
        // reset the background mode for each text input,
        // as some tools such as RichWin may have changed it
        if ( pFrame->mpGraphics &&
             pFrame->mpGraphics->getHDC() )
            SetBkMode( pFrame->mpGraphics->getHDC(), TRANSPARENT );
    }

    if ( pFrame && pFrame->mbHandleIME )
    {
        if ( !lParam )
        {
            SalExtTextInputEvent aEvt;
            aEvt.mpTextAttr         = nullptr;
            aEvt.mnCursorPos        = 0;
            aEvt.mnCursorFlags      = 0;
            pFrame->CallCallback( SalEvent::ExtTextInput, &aEvt );
            pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
        }
        else if ( lParam & (GCS_RESULTSTR | GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS) )
        {
            HIMC hIMC = ImmGetContext( hWnd );
            if ( hIMC )
            {
                if ( ImplHandleIMECompositionInput( pFrame, hIMC, lParam ) )
                    bDef = FALSE;

                ImmReleaseContext( hWnd, hIMC );
            }
        }
    }

    ImplSalYieldMutexRelease();
    return bDef;
}

static bool ImplHandleIMEEndComposition( HWND hWnd )
{
    bool bDef = TRUE;

    ImplSalYieldMutexAcquireWithWait();

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame && pFrame->mbHandleIME )
    {
        if ( pFrame->mbAtCursorIME )
            bDef = FALSE;
    }

    ImplSalYieldMutexRelease();

    return bDef;
}

static bool ImplHandleAppCommand( HWND hWnd, LPARAM lParam, LRESULT & nRet )
{
    MediaCommand nCommand;
    switch( GET_APPCOMMAND_LPARAM(lParam) )
    {
    case APPCOMMAND_MEDIA_CHANNEL_DOWN:         nCommand = MediaCommand::ChannelDown; break;
    case APPCOMMAND_MEDIA_CHANNEL_UP:           nCommand = MediaCommand::ChannelUp; break;
    case APPCOMMAND_MEDIA_NEXTTRACK:            nCommand = MediaCommand::NextTrack; break;
    case APPCOMMAND_MEDIA_PAUSE:                nCommand = MediaCommand::Pause; break;
    case APPCOMMAND_MEDIA_PLAY:                 nCommand = MediaCommand::Play; break;
    case APPCOMMAND_MEDIA_PLAY_PAUSE:           nCommand = MediaCommand::PlayPause; break;
    case APPCOMMAND_MEDIA_PREVIOUSTRACK:        nCommand = MediaCommand::PreviousTrack; break;
    case APPCOMMAND_MEDIA_RECORD:               nCommand = MediaCommand::Record; break;
    case APPCOMMAND_MEDIA_REWIND:               nCommand = MediaCommand::Rewind; break;
    case APPCOMMAND_MEDIA_STOP:                 nCommand = MediaCommand::Stop; break;
    case APPCOMMAND_MIC_ON_OFF_TOGGLE:          nCommand = MediaCommand::MicOnOffToggle; break;
    case APPCOMMAND_MICROPHONE_VOLUME_DOWN:     nCommand = MediaCommand::MicrophoneVolumeDown; break;
    case APPCOMMAND_MICROPHONE_VOLUME_MUTE:     nCommand = MediaCommand::MicrophoneVolumeMute; break;
    case APPCOMMAND_MICROPHONE_VOLUME_UP:       nCommand = MediaCommand::MicrophoneVolumeUp; break;
    case APPCOMMAND_VOLUME_DOWN:                nCommand = MediaCommand::VolumeDown; break;
    case APPCOMMAND_VOLUME_MUTE:                nCommand = MediaCommand::VolumeMute; break;
    case APPCOMMAND_VOLUME_UP:                  nCommand = MediaCommand::VolumeUp; break;
    default:
        return false;
    }

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    vcl::Window *pWindow = pFrame ? pFrame->GetWindow() : nullptr;

    if( pWindow )
    {
        const Point aPoint;
        CommandMediaData aMediaData(nCommand);
        CommandEvent aCEvt( aPoint, CommandEventId::Media, FALSE, &aMediaData );
        NotifyEvent aNCmdEvt( MouseNotifyEvent::COMMAND, pWindow, &aCEvt );

        if ( !ImplCallPreNotify( aNCmdEvt ) )
        {
            pWindow->Command( aCEvt );
            nRet = 1;
            return !aMediaData.GetPassThroughToOS();
        }
    }

    return false;
}

static void ImplHandleIMENotify( HWND hWnd, WPARAM wParam )
{
    if ( wParam == (WPARAM)IMN_OPENCANDIDATE )
    {
        ImplSalYieldMutexAcquireWithWait();

        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && pFrame->mbHandleIME &&
             pFrame->mbAtCursorIME )
        {
            // we want to hide der cursor
            pFrame->mbCandidateMode = TRUE;
            ImplHandleIMEComposition( hWnd, GCS_CURSORPOS );

            HWND hWnd2 = pFrame->mhWnd;
            HIMC hIMC = ImmGetContext( hWnd2 );
            if ( hIMC )
            {
                LONG nBufLen = ImmGetCompositionStringW( hIMC, GCS_COMPSTR, nullptr, 0 );
                if ( nBufLen >= 1 )
                {
                    SalExtTextInputPosEvent aPosEvt;
                    pFrame->CallCallback( SalEvent::ExtTextInputPos, &aPosEvt );

                    // Vertical !!!
                    CANDIDATEFORM aForm;
                    aForm.dwIndex           = 0;
                    aForm.dwStyle           = CFS_EXCLUDE;
                    aForm.ptCurrentPos.x    = aPosEvt.mnX;
                    aForm.ptCurrentPos.y    = aPosEvt.mnY+1;
                    aForm.rcArea.left       = aPosEvt.mnX;
                    aForm.rcArea.top        = aPosEvt.mnY;
                    aForm.rcArea.right      = aForm.rcArea.left+aPosEvt.mnExtWidth+1;
                    aForm.rcArea.bottom     = aForm.rcArea.top+aPosEvt.mnHeight+1;
                    ImmSetCandidateWindow( hIMC, &aForm );
                }

                ImmReleaseContext( hWnd2, hIMC );
            }
        }

        ImplSalYieldMutexRelease();
    }
    else if ( wParam == (WPARAM)IMN_CLOSECANDIDATE )
    {
        ImplSalYieldMutexAcquireWithWait();
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
            pFrame->mbCandidateMode = FALSE;
        ImplSalYieldMutexRelease();
    }
}

static bool
ImplHandleGetObject(HWND hWnd, LPARAM lParam, WPARAM wParam, LRESULT & nRet)
{
    // IA2 should be enabled automatically
    AllSettings aSettings = Application::GetSettings();
    MiscSettings aMisc = aSettings.GetMiscSettings();
    aMisc.SetEnableATToolSupport( true );
    aSettings.SetMiscSettings( aMisc );
    Application::SetSettings( aSettings );

    if (!Application::GetSettings().GetMiscSettings().GetEnableATToolSupport())
        return false; // locked down somehow ?

    ImplSVData* pSVData = ImplGetSVData();

    // Make sure to launch Accessibility only the following criteria are satisfied
    // to avoid RFT interrupts regular accessibility processing
    if ( !pSVData->mxAccessBridge.is() )
    {
        if( !InitAccessBridge() )
            return false;
    }

    uno::Reference< accessibility::XMSAAService > xMSAA( pSVData->mxAccessBridge, uno::UNO_QUERY );
    if ( xMSAA.is() )
    {
        // mhOnSetTitleWnd not set to reasonable value anywhere...
        if ( lParam == OBJID_CLIENT )
        {
            nRet = xMSAA->getAccObjectPtr(
                    reinterpret_cast<sal_Int64>(hWnd), lParam, wParam);
            if( nRet != 0 )
                return true;
        }
    }
    return false;
}

static LRESULT ImplHandleIMEReconvertString( HWND hWnd, LPARAM lParam )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    LPRECONVERTSTRING pReconvertString = reinterpret_cast<LPRECONVERTSTRING>(lParam);
    LRESULT nRet = 0;
    SalSurroundingTextRequestEvent aEvt;
    aEvt.maText.clear();
    aEvt.mnStart = aEvt.mnEnd = 0;

    UINT nImeProps = ImmGetProperty( GetKeyboardLayout( 0 ), IGP_SETCOMPSTR );
    if( (nImeProps & SCS_CAP_SETRECONVERTSTRING) == 0 )
    {
    // This IME does not support reconversion.
    return 0;
    }

    if( !pReconvertString )
    {
    // The first call for reconversion.
    pFrame->CallCallback( SalEvent::StartReconversion, nullptr );

    // Retrieve the surrounding text from the focused control.
    pFrame->CallCallback( SalEvent::SurroundingTextRequest, &aEvt );

    if( aEvt.maText.isEmpty())
    {
        return 0;
    }

    nRet = sizeof(RECONVERTSTRING) + (aEvt.maText.getLength() + 1) * sizeof(WCHAR);
    }
    else
    {
    // The second call for reconversion.

    // Retrieve the surrounding text from the focused control.
    pFrame->CallCallback( SalEvent::SurroundingTextRequest, &aEvt );
    nRet = sizeof(RECONVERTSTRING) + (aEvt.maText.getLength() + 1) * sizeof(WCHAR);

    pReconvertString->dwStrOffset = sizeof(RECONVERTSTRING);
    pReconvertString->dwStrLen = aEvt.maText.getLength();
    pReconvertString->dwCompStrOffset = aEvt.mnStart * sizeof(WCHAR);
    pReconvertString->dwCompStrLen = aEvt.mnEnd - aEvt.mnStart;
    pReconvertString->dwTargetStrOffset = pReconvertString->dwCompStrOffset;
    pReconvertString->dwTargetStrLen = pReconvertString->dwCompStrLen;

    memcpy( pReconvertString + 1, aEvt.maText.getStr(), (aEvt.maText.getLength() + 1) * sizeof(WCHAR) );
    }

    // just return the required size of buffer to reconvert.
    return nRet;
}

static LRESULT ImplHandleIMEConfirmReconvertString( HWND hWnd, LPARAM lParam )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    LPRECONVERTSTRING pReconvertString = reinterpret_cast<LPRECONVERTSTRING>(lParam);
    SalSurroundingTextRequestEvent aEvt;
    aEvt.maText.clear();
    aEvt.mnStart = aEvt.mnEnd = 0;

    pFrame->CallCallback( SalEvent::SurroundingTextRequest, &aEvt );

    sal_uLong nTmpStart = pReconvertString->dwCompStrOffset / sizeof(WCHAR);
    sal_uLong nTmpEnd = nTmpStart + pReconvertString->dwCompStrLen;

    if( nTmpStart != aEvt.mnStart || nTmpEnd != aEvt.mnEnd )
    {
    SalSurroundingTextSelectionChangeEvent aSelEvt;
    aSelEvt.mnStart = nTmpStart;
    aSelEvt.mnEnd = nTmpEnd;

    pFrame->CallCallback( SalEvent::SurroundingTextSelectionChange, &aSelEvt );
    }

    return TRUE;
}

static LRESULT ImplHandleIMEQueryCharPosition( HWND hWnd, LPARAM lParam ) {
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    PIMECHARPOSITION pQueryCharPosition = reinterpret_cast<PIMECHARPOSITION>(lParam);
    if ( pQueryCharPosition->dwSize < sizeof(IMECHARPOSITION) )
        return FALSE;

    SalQueryCharPositionEvent aEvt;
    aEvt.mbValid = false;
    aEvt.mnCharPos = pQueryCharPosition->dwCharPos;

    pFrame->CallCallback( SalEvent::QueryCharPosition, &aEvt );

    if ( !aEvt.mbValid )
        return FALSE;

    if ( aEvt.mbVertical )
    {
        // For vertical writing, the base line is left edge of the rectangle
        // and the target position is top-right corner.
        pQueryCharPosition->pt.x = aEvt.mnCursorBoundX + aEvt.mnCursorBoundWidth;
        pQueryCharPosition->pt.y = aEvt.mnCursorBoundY;
        pQueryCharPosition->cLineHeight = aEvt.mnCursorBoundWidth;
    }
    else
    {
        // For horizontal writing, the base line is the bottom edge of the rectangle.
        // and the target position is top-left corner.
        pQueryCharPosition->pt.x = aEvt.mnCursorBoundX;
        pQueryCharPosition->pt.y = aEvt.mnCursorBoundY;
        pQueryCharPosition->cLineHeight = aEvt.mnCursorBoundHeight;
    }

    // Currently not supported but many IMEs usually ignore them.
    pQueryCharPosition->rcDocument.left = 0;
    pQueryCharPosition->rcDocument.top = 0;
    pQueryCharPosition->rcDocument.right = 0;
    pQueryCharPosition->rcDocument.bottom = 0;

    return TRUE;
}

void SalTestMouseLeave()
{
    SalData* pSalData = GetSalData();

    if ( pSalData->mhWantLeaveMsg && !::GetCapture() )
    {
        POINT aPt;
        GetCursorPos( &aPt );
        if ( pSalData->mhWantLeaveMsg != WindowFromPoint( aPt ) )
            SendMessageW( pSalData->mhWantLeaveMsg, SAL_MSG_MOUSELEAVE, 0, MAKELPARAM( aPt.x, aPt.y ) );
    }
}

static int ImplSalWheelMousePos( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam ,
                                 LRESULT& rResult )
{
    POINT aPt;
    POINT aScreenPt;
    aScreenPt.x = (short)LOWORD( lParam );
    aScreenPt.y = (short)HIWORD( lParam );
    // find child window that is at this position
    HWND hChildWnd;
    HWND hWheelWnd = hWnd;
    do
    {
        hChildWnd = hWheelWnd;
        aPt = aScreenPt;
        ScreenToClient( hChildWnd, &aPt );
        hWheelWnd = ChildWindowFromPointEx( hChildWnd, aPt, CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT );
    }
    while ( hWheelWnd && (hWheelWnd != hChildWnd) );
    if ( hWheelWnd && (hWheelWnd != hWnd) &&
         (hWheelWnd != ::GetFocus()) && IsWindowEnabled( hWheelWnd ) )
    {
        rResult = SendMessageW( hWheelWnd, nMsg, wParam, lParam );
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK SalFrameWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT     nRet = 0;
    static int  bInWheelMsg = FALSE;
    static int  bInQueryEnd = FALSE;

    // By WM_CRETAE we connect the frame with the window handle
    if ( nMsg == WM_CREATE )
    {
        // Save Window-Instance in Windowhandle
        // Can also be used for the W-Version, because the struct
        // to access lpCreateParams is the same structure
        CREATESTRUCTA* pStruct = reinterpret_cast<CREATESTRUCTA*>(lParam);
        WinSalFrame* pFrame = static_cast<WinSalFrame*>(pStruct->lpCreateParams);
        if ( pFrame != nullptr )
        {
            SetWindowPtr( hWnd, pFrame );
            // Set HWND already here, as data might be used already
            // when messages are being sent by CreateWindow()
            pFrame->mhWnd = hWnd;
            pFrame->maSysData.hWnd = hWnd;
        }
        return 0;
    }

    ImplSVData* pSVData = ImplGetSVData();
    // #i72707# TODO: the mbDeInit check will not be needed
    // once all windows that are not properly closed on exit got fixed
    if( pSVData->mbDeInit )
        return 0;

    if ( WM_USER_SYSTEM_WINDOW_ACTIVATED == nMsg )
    {
        ImplHideSplash();
        return 0;
    }

    bool bCheckTimers = false;

    switch( nMsg )
    {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_NCMOUSEMOVE:
        case SAL_MSG_MOUSELEAVE:
            ImplSalYieldMutexAcquireWithWait();
            rDef = int(!ImplHandleMouseMsg( hWnd, nMsg, wParam, lParam ));
            ImplSalYieldMutexRelease();
            break;

        case WM_NCLBUTTONDOWN:
        case WM_NCMBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
            ImplSalYieldMutexAcquireWithWait();
            ImplCallClosePopupsHdl( hWnd );   // close popups...
            ImplSalYieldMutexRelease();
            break;

        case WM_MOUSEACTIVATE:
            if ( LOWORD( lParam ) == HTCLIENT )
            {
                ImplSalYieldMutexAcquireWithWait();
                nRet = ImplHandleMouseActivateMsg( hWnd );
                ImplSalYieldMutexRelease();
                if ( nRet )
                {
                    nRet = MA_NOACTIVATE;
                    rDef = FALSE;
                }
            }
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_DEADCHAR:
        case WM_CHAR:
        case WM_UNICHAR:    // MCD, 2003-01-13, Support for WM_UNICHAR & Keyman 6.0
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
            ImplSalYieldMutexAcquireWithWait();
            rDef = int(!ImplHandleKeyMsg( hWnd, nMsg, wParam, lParam, nRet ));
            ImplSalYieldMutexRelease();
            break;

        case WM_MOUSEWHEEL:
            // FALLTHROUGH intended
        case WM_MOUSEHWHEEL:
            // protect against recursion, in case the message is returned
            // by IE or the external window
            if ( !bInWheelMsg )
            {
                bInWheelMsg++;
                rDef = int(!ImplHandleWheelMsg( hWnd, nMsg, wParam, lParam ));
                // If we did not process the message, re-check if here is a
                // connected (?) window that we have to notify.
                if ( rDef )
                    rDef = ImplSalWheelMousePos( hWnd, nMsg, wParam, lParam, nRet );
                bInWheelMsg--;
            }
            break;

        case WM_COMMAND:
            ImplSalYieldMutexAcquireWithWait();
            rDef = int(!ImplHandleCommand( hWnd, wParam, lParam ));
            ImplSalYieldMutexRelease();
            break;

        case WM_INITMENUPOPUP:
            ImplSalYieldMutexAcquireWithWait();
            rDef = int(!ImplHandleMenuActivate( hWnd, wParam, lParam ));
            ImplSalYieldMutexRelease();
            break;

        case WM_MENUSELECT:
            ImplSalYieldMutexAcquireWithWait();
            rDef = int(!ImplHandleMenuSelect( hWnd, wParam, lParam ));
            ImplSalYieldMutexRelease();
            break;

        case WM_SYSCOMMAND:
            ImplSalYieldMutexAcquireWithWait();
            nRet = ImplHandleSysCommand( hWnd, wParam, lParam );
            ImplSalYieldMutexRelease();
            if ( nRet )
                rDef = FALSE;
            break;

        case WM_MENUCHAR:
            nRet = ImplMenuChar( hWnd, wParam, lParam );
            if( nRet )
                rDef = FALSE;
            break;

        case WM_MEASUREITEM:
            nRet = ImplMeasureItem(hWnd, wParam, lParam);
            if( nRet )
                rDef = FALSE;
            break;

        case WM_DRAWITEM:
            nRet = ImplDrawItem(hWnd, wParam, lParam);
            if( nRet )
                rDef = FALSE;
            break;

        case WM_MOVE:
        case SAL_MSG_POSTMOVE:
            ImplHandleMoveMsg( hWnd );
            rDef = FALSE;
            break;
        case WM_SIZE:
            ImplHandleSizeMsg( hWnd, wParam, lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_POSTCALLSIZE:
            ImplCallSizeHdl( hWnd );
            rDef = FALSE;
            break;

        case WM_GETMINMAXINFO:
            if ( ImplHandleMinMax( hWnd, lParam ) )
                rDef = FALSE;
            break;

        case WM_ERASEBKGND:
            nRet = 1;
            rDef = FALSE;
            break;
        case WM_PAINT:
            bCheckTimers = ImplHandlePaintMsg( hWnd );
            rDef = FALSE;
            break;
        case SAL_MSG_POSTPAINT:
            ImplHandlePaintMsg2( hWnd, reinterpret_cast<RECT*>(wParam) );
            bCheckTimers = true;
            rDef = FALSE;
            break;

        case SAL_MSG_FORCEPALETTE:
            ImplHandleForcePalette( hWnd );
            rDef = FALSE;
            break;

        case WM_QUERYNEWPALETTE:
        case SAL_MSG_POSTQUERYNEWPAL:
            nRet = ImplHandlePalette( true, hWnd, nMsg, wParam, lParam, rDef );
            break;

        case WM_ACTIVATE:
            // Getting activated, we also want to set our palette.
            // We do this in Activate, so that other external child windows
            // can overwrite our palette. Thus our palette is set only once
            // and not recursively, as at all other places it is set only as
            // the background palette.
            if ( LOWORD( wParam ) != WA_INACTIVE )
                SendMessageW( hWnd, SAL_MSG_FORCEPALETTE, 0, 0 );
            break;

        case WM_ENABLE:
            // #95133# a system dialog is opened/closed, using our app window as parent
            {
                WinSalFrame* pFrame = GetWindowPtr( hWnd );
                vcl::Window *pWin = nullptr;
                if( pFrame )
                    pWin = pFrame->GetWindow();

                if( !wParam )
                {
                    pSVData->maAppData.mnModalMode++;

                    ImplHideSplash();
                    if( pWin )
                    {
                        pWin->EnableInput( FALSE, nullptr );
                        pWin->ImplIncModalCount();  // #106303# support frame based modal count
                    }
                }
                else
                {
                    ImplGetSVData()->maAppData.mnModalMode--;
                    if( pWin )
                    {
                        pWin->EnableInput( TRUE, nullptr );
                        pWin->ImplDecModalCount();  // #106303# support frame based modal count
                    }
                }
            }
            break;

        case WM_KILLFOCUS:
            DestroyCaret();
            SAL_FALLTHROUGH;
        case WM_SETFOCUS:
        case SAL_MSG_POSTFOCUS:
            ImplHandleFocusMsg( hWnd );
            rDef = FALSE;
            break;

        case WM_CLOSE:
            ImplHandleCloseMsg( hWnd );
            rDef = FALSE;
            break;

        case WM_QUERYENDSESSION:
            if( !bInQueryEnd )
            {
                // handle queryendsession only once
                bInQueryEnd = TRUE;
                nRet = LRESULT(!ImplHandleShutDownMsg( hWnd ));
                rDef = FALSE;

                // Issue #16314#: ImplHandleShutDownMsg causes a PostMessage in case of allowing shutdown.
                // This posted message was never processed and cause Windows XP to hang after log off
                // if there are multiple sessions and the current session wasn't the first one started.
                // So if shutdown is allowed we assume that a post message was done and retrieve all
                // messages in the message queue and dispatch them before we return control to the system.

                if ( nRet )
                {
                    MSG msg;

                    while( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
                    {
                        DispatchMessage( &msg );
                    }
                }
            }
            else
            {
                ImplSalYieldMutexAcquireWithWait();
                ImplSalYieldMutexRelease();
                rDef = TRUE;
            }
            break;

        case WM_ENDSESSION:
            if( !wParam )
                bInQueryEnd = FALSE; // no shutdown: allow query again
            nRet = FALSE;
            rDef = FALSE;
            break;

        case WM_DISPLAYCHANGE:
        case WM_SETTINGCHANGE:
        case WM_DEVMODECHANGE:
        case WM_FONTCHANGE:
        case WM_SYSCOLORCHANGE:
        case WM_TIMECHANGE:
            ImplHandleSettingsChangeMsg( hWnd, nMsg, wParam, lParam );
            break;

        case WM_THEMECHANGED:
            GetSalData()->mbThemeChanged = true;
            break;

        case SAL_MSG_USEREVENT:
            ImplHandleUserEvent( hWnd, lParam );
            rDef = FALSE;
            break;

        case SAL_MSG_CAPTUREMOUSE:
            SetCapture( hWnd );
            rDef = FALSE;
            break;
        case SAL_MSG_RELEASEMOUSE:
            if ( ::GetCapture() == hWnd )
                ReleaseCapture();
            rDef = FALSE;
            break;
        case SAL_MSG_TOTOP:
            ImplSalToTop( hWnd, (SalFrameToTop)wParam );
            rDef = FALSE;
            break;
        case SAL_MSG_SHOW:
            ImplSalShow( hWnd, (bool)wParam, (bool)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_SETINPUTCONTEXT:
            ImplSalFrameSetInputContext( hWnd, reinterpret_cast<const SalInputContext*>(lParam) );
            rDef = FALSE;
            break;
        case SAL_MSG_ENDEXTTEXTINPUT:
            ImplSalFrameEndExtTextInput( hWnd, (EndExtTextInputFlags)wParam );
            rDef = FALSE;
            break;

        case WM_INPUTLANGCHANGE:
            ImplHandleInputLangChange( hWnd, wParam, lParam );
            break;

        case WM_IME_CHAR:
            // #103487#, some IMEs (eg, those that do not work onspot)
            //           may send WM_IME_CHAR instead of WM_IME_COMPOSITION
            // we just handle it like a WM_CHAR message - seems to work fine
            ImplSalYieldMutexAcquireWithWait();
            rDef = int(!ImplHandleKeyMsg( hWnd, WM_CHAR, wParam, lParam, nRet ));
            ImplSalYieldMutexRelease();
            break;

         case WM_IME_STARTCOMPOSITION:
            rDef = int(ImplHandleIMEStartComposition( hWnd ));
            break;

        case WM_IME_COMPOSITION:
            rDef = int(ImplHandleIMEComposition( hWnd, lParam ));
            break;

        case WM_IME_ENDCOMPOSITION:
            rDef = int(ImplHandleIMEEndComposition( hWnd ));
            break;

        case WM_IME_NOTIFY:
            ImplHandleIMENotify( hWnd, wParam );
            break;

        case WM_GETOBJECT:
            ImplSalYieldMutexAcquireWithWait();
            if ( ImplHandleGetObject( hWnd, lParam, wParam, nRet ) )
            {
                rDef = int(false);
            }
            ImplSalYieldMutexRelease();
            break;

        case WM_APPCOMMAND:
            if( ImplHandleAppCommand( hWnd, lParam, nRet ) )
            {
                rDef = int(false);
            }
            break;
        case WM_IME_REQUEST:
            if ( (sal_uIntPtr)( wParam ) == IMR_RECONVERTSTRING )
            {
                nRet = ImplHandleIMEReconvertString( hWnd, lParam );
                rDef = FALSE;
            }
            else if( (sal_uIntPtr)( wParam ) == IMR_CONFIRMRECONVERTSTRING )
            {
                nRet = ImplHandleIMEConfirmReconvertString( hWnd, lParam );
                rDef = FALSE;
            }
            else if ( (sal_uIntPtr)( wParam ) == IMR_QUERYCHARPOSITION )
            {
                if ( ImplSalYieldMutexTryToAcquire() )
                {
                    nRet = ImplHandleIMEQueryCharPosition( hWnd, lParam );
                    ImplSalYieldMutexRelease();
                }
                else
                    nRet = FALSE;
                rDef = FALSE;
            }
            break;
    }

    if( bCheckTimers )
    {
        SalData* pSalData = GetSalData();
        if( pSalData->mnNextTimerTime )
        {
            DWORD nCurTime = GetTickCount();
            if( pSalData->mnNextTimerTime < nCurTime )
            {
                MSG aMsg;
                if( ! PeekMessageW( &aMsg, nullptr, WM_PAINT, WM_PAINT, PM_NOREMOVE | PM_NOYIELD ) )
                {
                    BOOL const ret = PostMessageW(pSalData->mpFirstInstance->mhComWnd, SAL_MSG_POSTTIMER, 0, nCurTime);
                    SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
                }
            }
        }
    }

    return nRet;
}

LRESULT CALLBACK SalFrameWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = 0;
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        nRet = SalFrameWndProc( hWnd, nMsg, wParam, lParam, bDef );
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    han.Reset();
#else
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
#endif

    if ( bDef )
        nRet = DefWindowProcW( hWnd, nMsg, wParam, lParam );
    return nRet;
}

bool ImplHandleGlobalMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& rlResult )
{
    // handle all messages concerning all frames so they get processed only once
    // Must work for Unicode and none Unicode
    bool bResult = FALSE;
    if ( (nMsg == WM_PALETTECHANGED) || (nMsg == SAL_MSG_POSTPALCHANGED) )
    {
        int bDef = TRUE;
        rlResult = ImplHandlePalette( false, hWnd, nMsg, wParam, lParam, bDef );
        bResult = (bDef != 0);
    }
    else if( nMsg == WM_DISPLAYCHANGE )
    {
        WinSalSystem* pSys = static_cast<WinSalSystem*>(ImplGetSalSystem());
        if( pSys )
            pSys->clearMonitors();
        bResult = (pSys != nullptr);
    }
    return bResult;
}

#ifdef _WIN32
bool HasAtHook()
{
    BOOL bIsRunning = FALSE;
    // pvParam must be BOOL
    return SystemParametersInfo(SPI_GETSCREENREADER, 0, &bIsRunning, 0)
        && bIsRunning;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
