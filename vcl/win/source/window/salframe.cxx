/*************************************************************************
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.97 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:03:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string.h>
#include <limits.h>

#ifdef DBG_UTIL
#include <stdio.h>
#endif

#ifndef _SVWIN_HXX
#include <tools/svwin.h>
#endif

#define _SV_SALFRAME_CXX

#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define private public
#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _SV_SALIDS_HRC
#include <salids.hrc>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALVD_H
#include <salvd.h>
#endif
#ifndef _SV_SALMENU_HXX
#include <salmenu.hxx>
#endif
#ifndef _SV_IMPBMP_HXX
#include <impbmp.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_KEYCOES_HXX
#include <keycodes.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>

// misssing prototypes and constants for LayeredWindows
extern "C" {
    //WINUSERAPI BOOL WINAPI SetLayeredWindowAttributes(HWND,COLORREF,BYTE,DWORD);
    typedef BOOL ( WINAPI * SetLayeredWindowAttributes_Proc_T ) (HWND,COLORREF,BYTE,DWORD);
    static SetLayeredWindowAttributes_Proc_T lpfnSetLayeredWindowAttributes;
};

#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002
#define ULW_COLORKEY            0x00000001
#define ULW_ALPHA               0x00000002
#define ULW_OPAQUE              0x00000004
#define WS_EX_LAYERED           0x00080000


// =======================================================================

const unsigned int WM_USER_SYSTEM_WINDOW_ACTIVATED = RegisterWindowMessageA("SYSTEM_WINDOW_ACTIVATED");

// =======================================================================

// Wegen Fehler in Windows-Headerfiles
#ifndef IMN_OPENCANDIDATE
#define IMN_OPENCANDIDATE               0x0005
#endif
#ifndef IMN_CLOSECANDIDATE
#define IMN_CLOSECANDIDATE              0x0004
#endif

// Macros for support of WM_UNICHAR & Keyman 6.0
#define Uni_UTF32ToSurrogate1(ch)   (((unsigned long) (ch) - 0x10000) / 0x400 + 0xD800)
#define Uni_UTF32ToSurrogate2(ch)   (((unsigned long) (ch) - 0x10000) % 0x400 + 0xDC00)
#define Uni_SupplementaryPlanesStart    0x10000
#ifndef WM_UNICHAR
#define WM_UNICHAR          0x0109
#define UNICODE_NOCHAR      0xFFFF
#endif

#if OSL_DEBUG_LEVEL > 1
void MyOutputDebugString(const char *buffer)
{
    OutputDebugString( buffer );
}
#endif

// =======================================================================
BOOL WinSalFrame::mbInReparent = FALSE;

static void UpdateFrameGeometry( HWND hWnd, SalFrame* pFrame );

static void ImplSaveFrameState( WinSalFrame* pFrame )
{
    // Position, Groesse und Status fuer GetWindowState() merken
    if ( !pFrame->mbFullScreen )
    {
        BOOL bVisible = (GetWindowStyle( pFrame->mhWnd ) & WS_VISIBLE) != 0;
        if ( IsIconic( pFrame->mhWnd ) )
        {
            pFrame->maState.mnState |= SAL_FRAMESTATE_MINIMIZED;
            if ( bVisible )
                pFrame->mnShowState = SW_SHOWMAXIMIZED;
        }
        else if ( IsZoomed( pFrame->mhWnd ) )
        {
            pFrame->maState.mnState &= ~SAL_FRAMESTATE_MINIMIZED;
            pFrame->maState.mnState |= SAL_FRAMESTATE_MAXIMIZED;
            if ( bVisible )
                pFrame->mnShowState = SW_SHOWMAXIMIZED;
            pFrame->mbRestoreMaximize = TRUE;
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

            pFrame->maState.mnState &= ~(SAL_FRAMESTATE_MINIMIZED | SAL_FRAMESTATE_MAXIMIZED);
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

// -----------------------------------------------------------------------

// if pParentRect is set, the workarea of the monitor that contains pParentRect is returned
void ImplSalGetWorkArea( HWND hWnd, RECT *pRect, const RECT *pParentRect )
{
    static int winVerChecked = 0;
    static int winVerOk = 0;

    // check if we or our parent is fullscreen, then the taskbar should be ignored
    bool bIgnoreTaskbar = false;
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if( pFrame )
    {
        Window *pWin = ((Window*)pFrame->GetInstance());
        while( pWin )
        {
            WorkWindow *pWorkWin = (pWin->GetType() == WINDOW_WORKWINDOW) ? (WorkWindow *) pWin : NULL;
            if( pWorkWin && pWorkWin->mbReallyVisible && pWorkWin->mbFullScreenMode )
            {
                bIgnoreTaskbar = true;
                break;
            }
            else
                pWin = pWin->mpParent;
        }
    }

    if( !winVerChecked )
    {
        winVerChecked = 1;
        winVerOk = 1;

        // multi monitor calls not available on Win95/NT
        OSVERSIONINFO aVerInfo;
        aVerInfo.dwOSVersionInfoSize = sizeof( aVerInfo );
        if ( GetVersionEx( &aVerInfo ) )
        {
            if ( aVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
            {
                if ( aVerInfo.dwMajorVersion <= 4 )
                    winVerOk = 0;   // NT
            }
            else if( aVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
            {
                if ( aVerInfo.dwMajorVersion == 4 && aVerInfo.dwMinorVersion == 0 )
                    winVerOk = 0;   // Win95
            }
        }
    }

    // calculates the work area taking multiple monitors into account
    if( winVerOk )
    {
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
            if( pParentRect != NULL )
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
    else
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
}

// =======================================================================

SalFrame* ImplSalCreateFrame( WinSalInstance* pInst,
                              HWND hWndParent, ULONG nSalFrameStyle )
{
    WinSalFrame*   pFrame = new WinSalFrame;
    HWND        hWnd;
    DWORD       nSysStyle = 0;
    DWORD       nExSysStyle = 0;
    BOOL        bSubFrame = FALSE;

    static int bLayeredAPI = -1;
    if( bLayeredAPI == -1 )
    {
        bLayeredAPI = 0;
        OSVERSIONINFO aVerInfo;
        aVerInfo.dwOSVersionInfoSize = sizeof( aVerInfo );
        if ( GetVersionEx( &aVerInfo ) )
            // check for W2k and XP
            if ( aVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && aVerInfo.dwMajorVersion >= 5 )
            {
                bLayeredAPI = 1;
                HMODULE hModule = LoadLibrary("user32");
                if( !(lpfnSetLayeredWindowAttributes = ( SetLayeredWindowAttributes_Proc_T )GetProcAddress( hModule, "SetLayeredWindowAttributes" ) ) )
                    bLayeredAPI = 0;
            }
    }
    static const char* pEnvTransparentFloats = getenv("SAL_TRANSPARENT_FLOATS" );

    // determine creation data
    if ( nSalFrameStyle & SAL_FRAME_STYLE_CHILD )
        nSysStyle |= WS_CHILD;
    else
    {
        if ( hWndParent )
        {
            nSysStyle |= WS_POPUP;
            bSubFrame = TRUE;
            pFrame->mbNoIcon = TRUE;
        }
        else
        {
            // Only with WS_OVRLAPPED we get a useful default position/size
            if ( (nSalFrameStyle & (SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_MOVEABLE)) ==
                 (SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_MOVEABLE) )
                nSysStyle |= WS_OVERLAPPED;
            else
            {
                nSysStyle |= WS_POPUP;
                if ( !(nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE) )
                    nExSysStyle |= WS_EX_TOOLWINDOW;    // avoid taskbar appearance, for eg splash screen
            }
        }

        if ( nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
        {
            pFrame->mbCaption = TRUE;
            nSysStyle |= WS_SYSMENU | WS_CAPTION;
            if ( !hWndParent )
                nSysStyle |= WS_SYSMENU | WS_MINIMIZEBOX;
            else
                nExSysStyle |= WS_EX_DLGMODALFRAME;

            if ( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE )
            {
                pFrame->mbSizeBorder = TRUE;
                nSysStyle |= WS_THICKFRAME;
                if ( !hWndParent )
                    nSysStyle |= WS_MAXIMIZEBOX;
            }
            else
                pFrame->mbFixBorder = TRUE;

            if ( nSalFrameStyle & SAL_FRAME_STYLE_DEFAULT )
                nExSysStyle |= WS_EX_APPWINDOW;
        }
        if( nSalFrameStyle & SAL_FRAME_STYLE_TOOLWINDOW
            // #100656# toolwindows lead to bad alt-tab behaviour, if they have the focus
            // you must press it twice to leave the application
            // so toolwindows are only used for non sizeable windows
            // which are typically small, so a small caption makes sense

            // #103578# looked too bad - above changes reverted
            /* && !(nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE) */ )
        {
            pFrame->mbNoIcon = TRUE;
            nExSysStyle |= WS_EX_TOOLWINDOW;
            if ( pEnvTransparentFloats && bLayeredAPI == 1 /*&& !(nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE) */)
                nExSysStyle |= WS_EX_LAYERED;
        }
    }
    if ( nSalFrameStyle & SAL_FRAME_STYLE_FLOAT )
    {
        nExSysStyle |= WS_EX_TOOLWINDOW;
        pFrame->mbFloatWin = TRUE;

        if ( pEnvTransparentFloats && bLayeredAPI == 1 /*&& !(nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE) */)
            nExSysStyle |= WS_EX_LAYERED;

    }
    if( nSalFrameStyle & SAL_FRAME_STYLE_TOOLTIP )
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
            if ( nSalFrameStyle & SAL_FRAME_STYLE_DEFAULT )
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
                HWND hWnd = GetForegroundWindow();
                if ( hWnd && IsMaximized( hWnd ) &&
                     (GetWindowInstance( hWnd ) == pInst->mhInst) &&
                     ((GetWindowStyle( hWnd ) & (WS_POPUP | WS_MAXIMIZEBOX | WS_THICKFRAME)) == (WS_MAXIMIZEBOX | WS_THICKFRAME)) )
                    pFrame->mnShowState = SW_SHOWMAXIMIZED;
            }
        }
    }

    // create frame
    if ( aSalShlData.mbWNT )
    {
        LPCWSTR pClassName;
        if ( bSubFrame )
        {
            if ( nSalFrameStyle & (SAL_FRAME_STYLE_MOVEABLE|SAL_FRAME_STYLE_NOSHADOW) ) // check if shadow not wanted
                pClassName = SAL_SUBFRAME_CLASSNAMEW;
            else
                pClassName = SAL_TMPSUBFRAME_CLASSNAMEW;    // undecorated floaters will get shadow on XP
        }
        else
        {
            if ( nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
                pClassName = SAL_FRAME_CLASSNAMEW;
            else
                pClassName = SAL_TMPSUBFRAME_CLASSNAMEW;
        }
        hWnd = CreateWindowExW( nExSysStyle, pClassName, L"", nSysStyle,
                                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                                hWndParent, 0, pInst->mhInst, (void*)pFrame );
#if OSL_DEBUG_LEVEL > 1
        // set transparency value
        if( bLayeredAPI == 1 && GetWindowExStyle( hWnd ) & WS_EX_LAYERED )
            lpfnSetLayeredWindowAttributes( hWnd, 0, 230, LWA_ALPHA );
#endif
    }
    else
    {
        LPCSTR pClassName;
        if ( bSubFrame )
            pClassName = SAL_SUBFRAME_CLASSNAMEA;
        else
            pClassName = SAL_FRAME_CLASSNAMEA;
        hWnd = CreateWindowExA( nExSysStyle, pClassName, "", nSysStyle,
                                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                                hWndParent, 0, pInst->mhInst, (void*)pFrame );
    }
    if ( !hWnd )
    {
        delete pFrame;
        return NULL;
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
    if ( (nSysStyle & WS_SYSMENU) && !(nSalFrameStyle & SAL_FRAME_STYLE_CLOSEABLE) )
    {
        HMENU hSysMenu = GetSystemMenu( hWnd, FALSE );
        if ( hSysMenu )
            EnableMenuItem( hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED );
    }

    // reset input context
    pFrame->mhDefIMEContext = ImmAssociateContext( hWnd, 0 );

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
        RECT aRect;
        ImplSalGetWorkArea( pFrame->mhWnd, &aRect, NULL );
        AdjustWindowRectEx( &aRect, GetWindowStyle( hWnd ),
                            FALSE,     GetWindowExStyle( hWnd ) );
        pFrame->maGeometry.nX = aRect.left;
        pFrame->maGeometry.nY = aRect.top;;
        pFrame->maGeometry.nWidth = aRect.right - aRect.left + 1;
        pFrame->maGeometry.nHeight = aRect.bottom - aRect.top + 1;
    }

    return pFrame;
}

// helper that only creates the HWND
// to allow for easy reparenting of system windows, (i.e. destroy and create new)
HWND ImplSalReCreateHWND( HWND hWndParent, HWND oldhWnd, BOOL bAsChild )
{
    HINSTANCE hInstance = GetSalData()->mhInst;
    ULONG nSysStyle     = GetWindowLong( oldhWnd, GWL_STYLE );
    ULONG nExSysStyle   = GetWindowLong( oldhWnd, GWL_EXSTYLE );

    if( bAsChild )
    {
        nSysStyle = WS_CHILD;
        nExSysStyle = 0;
    }

    HWND hWnd = NULL;
    if ( aSalShlData.mbWNT )
    {
        LPCWSTR pClassName = SAL_SUBFRAME_CLASSNAMEW;
        hWnd = CreateWindowExW( nExSysStyle, pClassName, L"", nSysStyle,
                                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                                hWndParent, 0, hInstance, (void*)GetWindowPtr( oldhWnd ) );
    }
    else
    {
        LPCSTR pClassName = SAL_SUBFRAME_CLASSNAMEA;
        hWnd = CreateWindowExA( nExSysStyle, pClassName, "", nSysStyle,
                                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                                hWndParent, 0, hInstance, (void*)GetWindowPtr( oldhWnd ) );
    }
    return hWnd;
}

// =======================================================================

// Uebersetzungstabelle von System-Keycodes in StarView-Keycodes
#define KEY_TAB_SIZE     146

static USHORT aImplTranslateKeyTab[KEY_TAB_SIZE] =
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
    0,                    //                                  21
    0,                    //                                  22
    0,                    //                                  23
    0,                    //                                  24
    0,                    //                                  25
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
    KEY_COMMA,            // VK_SEPARATOR                    108
    KEY_SUBTRACT,         // VK_SUBTRACT                     109
    KEY_POINT,            // VK_DECIMAL                      110
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

// =======================================================================

static UINT ImplSalGetWheelScrollLines()
{
    UINT nScrLines = 0;
    HWND hWndMsWheel = WIN_FindWindow( MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE );
    if ( hWndMsWheel )
    {
        UINT nGetScrollLinesMsgId = RegisterWindowMessage( MSH_SCROLL_LINES );
        nScrLines = (UINT)ImplSendMessage( hWndMsWheel, nGetScrollLinesMsgId, 0, 0 );
    }

    if ( !nScrLines )
        if( !SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &nScrLines, 0 ) )
            nScrLines = 0 ;

    if ( !nScrLines )
        nScrLines = 3;

    return nScrLines;
}

// -----------------------------------------------------------------------

static void ImplSalCalcBorder( const WinSalFrame* pFrame,
                               int& rLeft, int& rTop, int& rRight, int& rBottom )
{
    // set window to screen size
    int nFrameX;
    int nFrameY;
    int nCaptionY;

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

    rLeft   = nFrameX;
    rTop    = nFrameY+nCaptionY;
    rRight  = nFrameX;
    rBottom = nFrameY;
}

// -----------------------------------------------------------------------

static void ImplSalCalcFullScreenSize( const WinSalFrame* pFrame,
                                       int& rX, int& rY, int& rDX, int& rDY )
{
    // set window to screen size
    int nFrameX;
    int nFrameY;
    int nCaptionY;
    int nScreenDX;
    int nScreenDY;

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

    nScreenDX   = GetSystemMetrics( SM_CXSCREEN );
    nScreenDY   = GetSystemMetrics( SM_CYSCREEN );

    rX  = -nFrameX;
    rY  = -(nFrameY+nCaptionY);
    rDX = nScreenDX+(nFrameX*2);
    rDY = nScreenDY+(nFrameY*2)+nCaptionY;
}

// -----------------------------------------------------------------------

static void ImplSalFrameFullScreenPos( WinSalFrame* pFrame, BOOL bAlways = FALSE )
{
    if ( bAlways || !IsIconic( pFrame->mhWnd ) )
    {
        // set window to screen size
        int nX;
        int nY;
        int nWidth;
        int nHeight;
        ImplSalCalcFullScreenSize( pFrame, nX, nY, nWidth, nHeight );
        SetWindowPos( pFrame->mhWnd, 0,
                      nX, nY, nWidth, nHeight,
                      SWP_NOZORDER | SWP_NOACTIVATE );
    }
}

// -----------------------------------------------------------------------

WinSalFrame::WinSalFrame()
{
    SalData* pSalData = GetSalData();

    maFrameData.mhWnd               = 0;
    maFrameData.mhCursor            = LoadCursor( 0, IDC_ARROW );
    maFrameData.mhDefIMEContext     = 0;
    maFrameData.mpGraphics          = NULL;
    maFrameData.mpGraphics2         = NULL;
    maFrameData.mnShowState         = SW_SHOWNORMAL;
    maFrameData.mnWidth             = 0;
    maFrameData.mnHeight            = 0;
    maFrameData.mnMinWidth          = 0;
    maFrameData.mnMinHeight         = 0;
    maFrameData.mnInputLang         = 0;
    maFrameData.mnInputCodePage     = 0;
    maFrameData.mbGraphics          = FALSE;
    maFrameData.mbCaption           = FALSE;
    maFrameData.mbBorder            = FALSE;
    maFrameData.mbFixBorder         = FALSE;
    maFrameData.mbSizeBorder        = FALSE;
    maFrameData.mbFullScreen        = FALSE;
    maFrameData.mbPresentation      = FALSE;
    maFrameData.mbInShow            = FALSE;
    maFrameData.mbRestoreMaximize   = FALSE;
    maFrameData.mbInMoveMsg         = FALSE;
    maFrameData.mbInSizeMsg         = FALSE;
    maFrameData.mbFullScreenToolWin = FALSE;
    maFrameData.mbDefPos            = TRUE;
    maFrameData.mbOverwriteState    = TRUE;
    maFrameData.mbIME               = FALSE;
    maFrameData.mbHandleIME         = FALSE;
    maFrameData.mbSpezIME           = FALSE;
    maFrameData.mbAtCursorIME       = FALSE;
    maFrameData.mbCandidateMode     = FALSE;
    maFrameData.mbFloatWin          = FALSE;
    maFrameData.mbNoIcon            = FALSE;
    maFrameData.mSelectedhMenu      = 0;
    maFrameData.mLastActivatedhMenu = 0;

    memset( &maState, 0, sizeof( SalFrameState ) );
    maSysData.nSize     = sizeof( SystemEnvData );

    memset( &maGeometry, 0, sizeof( maGeometry ) );

    // Daten ermitteln, wenn erster Frame angelegt wird
    if ( !pSalData->mpFirstFrame )
    {
        if ( !aSalShlData.mnWheelMsgId )
            aSalShlData.mnWheelMsgId = RegisterWindowMessage( MSH_MOUSEWHEEL );
        if ( !aSalShlData.mnWheelScrollLines )
            aSalShlData.mnWheelScrollLines = ImplSalGetWheelScrollLines();
    }

    // insert frame in framelist
    mpNextFrame = pSalData->mpFirstFrame;
    pSalData->mpFirstFrame = this;
}

// -----------------------------------------------------------------------

WinSalFrame::~WinSalFrame()
{
    SalData* pSalData = GetSalData();

    // Release Cache DC
    if ( mpGraphics2 &&
         mpGraphics2->mhDC )
        ReleaseGraphics( mpGraphics2 );

    // destroy saved DC
    if ( mpGraphics )
    {
        if ( mpGraphics->mhDefPal )
            SelectPalette( mpGraphics->mhDC, mpGraphics->mhDefPal, TRUE );
        ImplSalDeInitGraphics( mpGraphics );
        ReleaseDC( mhWnd, mpGraphics->mhDC );
        delete mpGraphics;
    }

    if ( mhWnd )
    {
        // reset mouse leave data
        if ( pSalData->mhWantLeaveMsg == mhWnd )
        {
            pSalData->mhWantLeaveMsg = 0;
            if ( pSalData->mpMouseLeaveTimer )
            {
                delete pSalData->mpMouseLeaveTimer;
                pSalData->mpMouseLeaveTimer = NULL;
            }
        }

        // destroy system frame
        if ( !DestroyWindow( mhWnd ) )
            SetWindowPtr( mhWnd, 0 );
    }

    // remove frame from framelist
    if ( this == pSalData->mpFirstFrame )
        pSalData->mpFirstFrame = mpNextFrame;
    else
    {
        WinSalFrame* pTempFrame = pSalData->mpFirstFrame;
        while ( pTempFrame->mpNextFrame != this )
            pTempFrame = pTempFrame->mpNextFrame;

        pTempFrame->mpNextFrame = mpNextFrame;
    }
}

// -----------------------------------------------------------------------

SalGraphics* WinSalFrame::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    // Other threads get an own DC, because Windows modify in the
    // other case our DC (changing clip region), when they send a
    // WM_ERASEBACKGROUND message
    SalData* pSalData = GetSalData();
    if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
    {
        // We use only three CacheDC's for all threads, because W9x is limited
        // to max. 5 Cache DC's per thread
        if ( pSalData->mnCacheDCInUse >= 3 )
            return NULL;

        if ( !mpGraphics2 )
        {
            mpGraphics2 = new WinSalGraphics;
            mpGraphics2->mhDC        = 0;
            mpGraphics2->mhWnd       = mhWnd;
            mpGraphics2->mbPrinter   = FALSE;
            mpGraphics2->mbVirDev    = FALSE;
            mpGraphics2->mbWindow    = TRUE;
            mpGraphics2->mbScreen    = TRUE;
        }

        HDC hDC = (HDC)ImplSendMessage( pSalData->mpFirstInstance->mhComWnd,
                                        SAL_MSG_GETDC,
                                        (WPARAM)mhWnd, 0 );
        if ( hDC )
        {
            mpGraphics2->mhDC = hDC;
            if ( pSalData->mhDitherPal )
            {
                mpGraphics2->mhDefPal = SelectPalette( hDC, pSalData->mhDitherPal, TRUE );
                RealizePalette( hDC );
            }
            ImplSalInitGraphics( mpGraphics2 );
            mbGraphics = TRUE;
            pSalData->mnCacheDCInUse++;
            return mpGraphics2;
        }
        else
            return NULL;
    }
    else
    {
        if ( !mpGraphics )
        {
            HDC hDC = GetDC( mhWnd );
            if ( hDC )
            {
                mpGraphics = new WinSalGraphics;
                mpGraphics->mhDC      = hDC;
                mpGraphics->mhWnd     = mhWnd;
                mpGraphics->mbPrinter = FALSE;
                mpGraphics->mbVirDev  = FALSE;
                mpGraphics->mbWindow  = TRUE;
                mpGraphics->mbScreen  = TRUE;
                if ( pSalData->mhDitherPal )
                {
                    mpGraphics->mhDefPal = SelectPalette( hDC, pSalData->mhDitherPal, TRUE );
                    RealizePalette( hDC );
                }
                ImplSalInitGraphics( mpGraphics );
                mbGraphics = TRUE;
            }
        }
        else
            mbGraphics = TRUE;

        return mpGraphics;
    }
}

// -----------------------------------------------------------------------

void WinSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    if ( mpGraphics2 == pGraphics )
    {
        if ( mpGraphics2->mhDC )
        {
            SalData* pSalData = GetSalData();
            if ( mpGraphics2->mhDefPal )
                SelectPalette( mpGraphics2->mhDC, mpGraphics2->mhDefPal, TRUE );
            ImplSalDeInitGraphics( mpGraphics2 );
            ImplSendMessage( pSalData->mpFirstInstance->mhComWnd,
                             SAL_MSG_RELEASEDC,
                             (WPARAM)mhWnd,
                             (LPARAM)mpGraphics2->mhDC );
            mpGraphics2->mhDC = 0;
            pSalData->mnCacheDCInUse--;
        }
    }

    mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL WinSalFrame::PostEvent( void* pData )
{
    return (BOOL)ImplPostMessage( mhWnd, SAL_MSG_USEREVENT, 0, (LPARAM)pData );
}

// -----------------------------------------------------------------------

void WinSalFrame::SetTitle( const XubString& rTitle )
{
    DBG_ASSERT( sizeof( WCHAR ) == sizeof( xub_Unicode ), "WinSalFrame::SetTitle(): WCHAR != sal_Unicode" );

    if ( !SetWindowTextW( mhWnd, rTitle.GetBuffer() ) )
    {
        ByteString aAnsiTitle = ImplSalGetWinAnsiString( rTitle );
        SetWindowTextA( mhWnd, aAnsiTitle.GetBuffer() );
    }
}

// -----------------------------------------------------------------------

void WinSalFrame::SetIcon( USHORT nIcon )
{
    // If we have a window without an Icon (for example a dialog), ignore this call
    if ( mbNoIcon )
        return;

    // 0 means default (class) icon
    HICON hIcon = NULL, hSmIcon = NULL;
    if ( !nIcon )
        nIcon = 1;

    ImplLoadSalIcon( nIcon, hIcon, hSmIcon );

    DBG_ASSERT( hIcon ,   "WinSalFrame::SetIcon(): Could not load large icon !" );
    DBG_ASSERT( hSmIcon , "WinSalFrame::SetIcon(): Could not load small icon !" );

    ImplSendMessage( mhWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon );
    ImplSendMessage( mhWnd, WM_SETICON, ICON_SMALL, (LPARAM)hSmIcon );
}

// -----------------------------------------------------------------------

void WinSalFrame::SetMenu( SalMenu* pSalMenu )
{
    if( pSalMenu && pSalMenu->maData.mbMenuBar )
        ::SetMenu( maFrameData.mhWnd, pSalMenu->maData.mhMenu );
}

void WinSalFrame::DrawMenuBar()
{
    ::DrawMenuBar( maFrameData.mhWnd );
}

// -----------------------------------------------------------------------
HWND ImplGetParentHwnd( HWND hWnd )
{
#ifndef REMOTE_APPSERVER
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if( !pFrame || !pFrame->GetInstance())
        return ::GetParent( hWnd );
    Window *pRealParent = ((Window*)pFrame->GetInstance())->mpRealParent;
    if( pRealParent )
        return static_cast<WinSalFrame*>(pRealParent->mpFrame)->mhWnd;
    else
        return ::GetParent( hWnd );
#else
    return ::GetParent( hWnd ); // just to get it compiled for remote case, this function is never executed...
#endif

}

// -----------------------------------------------------------------------

SalFrame* WinSalFrame::GetParent() const
{
    return GetWindowPtr( ImplGetParentHwnd( mhWnd ) );
}

// -----------------------------------------------------------------------

static void ImplSalShow( HWND hWnd, BOOL bVisible, BOOL bNoActivate )
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

        if( bNoActivate )
            ShowWindow( hWnd, SW_SHOWNOACTIVATE );
        else
            ShowWindow( hWnd, pFrame->mnShowState );

        if ( aSalShlData.mbWXP && pFrame->mbFloatWin && !(pFrame->mnStyle & SAL_FRAME_STYLE_NOSHADOW))
        {
            // erase the window immediately to improve XP shadow effect
            // otherwise the shadow may appears long time before the rest of the window
            // especially when accessibility is on
            HDC dc = GetDC( hWnd );
            RECT aRect;
            GetClientRect( hWnd, &aRect );
            FillRect( dc, &aRect, (HBRUSH) (COLOR_MENU+1) ); // choose the menucolor, because its mostly noticeable for menues
            ReleaseDC( hWnd, dc );
        }

        // #i4715, matrox centerpopup might have changed our position
        // reposition popups without caption (menues, dropdowns, tooltips)
        GetWindowRect( hWnd, &aRectPostMatrox );
        if( (GetWindowStyle( hWnd ) & WS_POPUP) &&
            !pFrame->mbCaption &&
            (aRectPreMatrox.left != aRectPostMatrox.left || aRectPreMatrox.top != aRectPostMatrox.top) )
            SetWindowPos( hWnd, 0, aRectPreMatrox.left, aRectPreMatrox.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE  );

        Window *pClientWin = ((Window*)pFrame->GetInstance())->ImplGetClientWindow();
        if ( pFrame->mbFloatWin || ( pClientWin && (pClientWin->GetStyle() & WB_SYSTEMFLOATWIN) ) )
            pFrame->mnShowState = SW_SHOWNOACTIVATE;
        else
            pFrame->mnShowState = SW_SHOW;
        // Damit Taskleiste unter W98 auch gleich ausgeblendet wird
        if ( pFrame->mbPresentation )
        {
            HWND hWndParent = ::GetParent( hWnd );
            if ( hWndParent )
                SetForegroundWindow( hWndParent );
            SetForegroundWindow( hWnd );
        }

        pFrame->mbInShow = FALSE;


        // Direct Paint only, if we get the SolarMutx
        if ( ImplSalYieldMutexTryToAcquire() )
        {
            UpdateWindow( hWnd );
            ImplSalYieldMutexRelease();
        }
    }
    else
    {
        // See also Bug #91813# and #68467#
        if ( pFrame->mbFullScreen &&
             pFrame->mbPresentation &&
             (aSalShlData.mnVersion < 500) &&
             !::GetParent( hWnd ) )
        {
            // Damit im Impress-Player in der Taskleiste nicht durch
            // einen Windows-Fehler hin- und wieder mal ein leerer
            // Button stehen bleibt, muessen wir hier die Taskleiste
            // etwas austricksen. Denn wenn wir im FullScreenMode sind
            // und das Fenster hiden kommt Windows anscheinend etwas aus
            // dem tritt und somit minimieren wir das Fenster damit es
            // nicht flackert
            ANIMATIONINFO aInfo;
            aInfo.cbSize = sizeof( aInfo );
            SystemParametersInfo( SPI_GETANIMATION, 0, &aInfo, 0 );
            if ( aInfo.iMinAnimate )
            {
                int nOldAni = aInfo.iMinAnimate;
                aInfo.iMinAnimate = 0;
                SystemParametersInfo( SPI_SETANIMATION, 0, &aInfo, 0 );
                ShowWindow( pFrame->mhWnd, SW_SHOWMINNOACTIVE );
                aInfo.iMinAnimate = nOldAni;
                SystemParametersInfo( SPI_SETANIMATION, 0, &aInfo, 0 );
            }
            else
                ShowWindow( hWnd, SW_SHOWMINNOACTIVE );
            ShowWindow( hWnd, SW_HIDE );
        }
        else
            ShowWindow( hWnd, SW_HIDE );
    }
}

// -----------------------------------------------------------------------

void WinSalFrame::Show( BOOL bVisible, BOOL bNoActivate )
{
    // Post this Message to the window, because this only works
    // in the thread of the window, which has create this window.
    // We post this message to avoid deadlocks
    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
        ImplPostMessage( mhWnd, SAL_MSG_SHOW, bVisible, bNoActivate );
    else
        ImplSalShow( mhWnd, bVisible, bNoActivate );
}

// -----------------------------------------------------------------------

void WinSalFrame::Enable( BOOL bEnable )
{
    EnableWindow( mhWnd, bEnable );
}

// -----------------------------------------------------------------------

void WinSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    mnMinWidth  = nWidth;
    mnMinHeight = nHeight;
}

// -----------------------------------------------------------------------

void WinSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight,
                                                   USHORT nFlags )
{
    BOOL bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    if ( !bVisible )
    {
        Window *pClientWin = ((Window*)GetInstance())->ImplGetClientWindow();
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

    USHORT nEvent = 0;
    UINT    nPosSize = 0;
    RECT    aClientRect, aWindowRect;
    GetClientRect( mhWnd, &aClientRect );   // x,y always 0,0, but width and height without border
    GetWindowRect( mhWnd, &aWindowRect );   // x,y in screen coordinates, width and height with border

    if ( !(nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y)) )
        nPosSize |= SWP_NOMOVE;
    else
    {
        //DBG_ASSERT( nX && nY, " Windowposition of (0,0) requested!" );
        nEvent = SALEVENT_MOVE;
    }
    if ( !(nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT)) )
        nPosSize |= SWP_NOSIZE;
    else
        nEvent = (nEvent == SALEVENT_MOVE) ? SALEVENT_MOVERESIZE : SALEVENT_RESIZE;

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
            if( Application::GetSettings().GetLayoutRTL() )
                nX = (aParentRect.right - aParentRect.left) - nWidth-1 - nX;

            //#110386#, do not transform coordinates for system child windows
            if( !(GetWindowStyle( maFrameData.mhWnd ) & WS_CHILD) )
            {
                POINT aPt;
                aPt.x = nX;
                aPt.y = nY;
                ClientToScreen( ImplGetParentHwnd( mhWnd ), &aPt );
                nX = aPt.x;
                nY = aPt.y;
            }
    }

    // #i3338# to be conformant to UNIX we must position the client window, ie without the decoration
    nX += aWinRect.left;
    nY += aWinRect.top;

    int     nScreenX;
    int     nScreenY;
    int     nScreenWidth;
    int     nScreenHeight;


    RECT aRect;
    ImplSalGetWorkArea( mhWnd, &aRect, NULL );
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
            RECT aRect;
            aRect.left = pt.x;
            aRect.top = pt.y;
            aRect.right = pt.x+2;
            aRect.bottom = pt.y+2;

            // dualmonitor support:
            // Get screensize of the monitor whith the mouse pointer
            ImplSalGetWorkArea( mhWnd, &aRect, &aRect );

            nX = ((aRect.right-aRect.left)-nWidth)/2 + aRect.left;
            nY = ((aRect.bottom-aRect.top)-nHeight)/2 + aRect.top;
        }


        //if ( bVisible )
        //    mbDefPos = FALSE;

        mbDefPos = FALSE;   // center only once
        nPosSize &= ~SWP_NOMOVE;        // activate positioning
        nEvent = SALEVENT_MOVERESIZE;
    }


    // Adjust Window in the screen
    if ( nX+nWidth > nScreenX+nScreenWidth )
        nX = (nScreenX+nScreenWidth) - nWidth;
    if ( nY+nHeight > nScreenY+nScreenHeight )
        nY = (nScreenY+nScreenHeight) - nHeight;
    if ( nX < nScreenX )
        nX = nScreenX;
    if ( nY < nScreenY )
        nY = nScreenY;

    UINT nPosFlags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | nPosSize;
    // bring floating windows always to top
    if( !(mnStyle & SAL_FRAME_STYLE_FLOAT) )
        nPosFlags |= SWP_NOZORDER; // do not change z-order

    SetWindowPos( mhWnd, HWND_TOP, nX, nY, (int)nWidth, (int)nHeight, nPosFlags  );

    UpdateFrameGeometry( mhWnd, this );

    // Notification -- really ???
    if( nEvent )
        CallCallback( nEvent, NULL );
}

// -----------------------------------------------------------------------

static void ImplSetParentFrame( WinSalFrame* pThis, HWND hNewParentWnd, BOOL bAsChild )
{
    pThis->mbInReparent = TRUE;

    // save hwnd, will be overwritten in WM_CREATE during createwindow
    HWND hWndOld = pThis->mhWnd;
    BOOL bNeedGraphics = pThis->mbGraphics;
    HFONT   hFont   = NULL;
    HPEN    hPen    = NULL;
    HBRUSH  hBrush  = NULL;

    // Release Cache DC
    if ( pThis->mpGraphics2 &&
         pThis->mpGraphics2->mhDC )
    {
        // save current gdi objects before hdc is gone
        hFont   = (HFONT)   GetCurrentObject( pThis->mpGraphics2->mhDC, OBJ_FONT);
        hPen    = (HPEN)    GetCurrentObject( pThis->mpGraphics2->mhDC, OBJ_PEN);
        hBrush  = (HBRUSH)  GetCurrentObject( pThis->mpGraphics2->mhDC, OBJ_BRUSH);
        pThis->ReleaseGraphics( pThis->mpGraphics2 );
    }

    // destroy saved DC
    if ( pThis->mpGraphics )
    {
        if ( pThis->mpGraphics->mhDefPal )
            SelectPalette( pThis->mpGraphics->mhDC, pThis->mpGraphics->mhDefPal, TRUE );
        ImplSalDeInitGraphics( pThis->mpGraphics );
        ReleaseDC( pThis->mhWnd, pThis->mpGraphics->mhDC );
    }

    // create a new hwnd with the same styles
    HWND hWndParent = hNewParentWnd;
    // forward to main thread
    HWND hWnd = (HWND) ImplSendMessage( GetSalData()->mpFirstInstance->mhComWnd,
                                        bAsChild ? SAL_MSG_RECREATECHILDHWND : SAL_MSG_RECREATEHWND,
                                        (WPARAM) hWndParent, (LPARAM)pThis->mhWnd );

    // succeeded ?
    hWndParent = ::GetParent( hWnd );
    DBG_ASSERT( hWndParent == hNewParentWnd, "WinSalFrame::SetParent not successful");

    // recreate DCs
    if( bNeedGraphics )
    {
        if( pThis->mpGraphics2 )
        {
            // re-create cached DC
            HDC hDC = (HDC)ImplSendMessage( GetSalData()->mpFirstInstance->mhComWnd,
                                            SAL_MSG_GETDC,
                                            (WPARAM) hWnd, 0 );
            pThis->mpGraphics2->mhWnd = hWnd;
            if ( hDC )
            {
                pThis->mpGraphics2->mhDC = hDC;
                if ( GetSalData()->mhDitherPal )
                {
                    pThis->mpGraphics2->mhDefPal = SelectPalette( hDC, GetSalData()->mhDitherPal, TRUE );
                    RealizePalette( hDC );
                }
                ImplSalInitGraphics( pThis->mpGraphics2 );

                // re-select saved gdi objects
                if( hFont )
                    SelectObject( hDC, hFont );
                if( hPen )
                    SelectObject( hDC, hPen );
                if( hBrush )
                    SelectObject( hDC, hBrush );

                pThis->mbGraphics = TRUE;
                GetSalData()->mnCacheDCInUse++;
            }
        }

        if( pThis->mpGraphics )
        {
            // re-create DC
            pThis->mpGraphics->mhWnd = hWnd;
            pThis->mpGraphics->mhDC = GetDC( hWnd );
            if ( GetSalData()->mhDitherPal )
            {
                pThis->mpGraphics->mhDefPal = SelectPalette( pThis->mpGraphics->mhDC, GetSalData()->mhDitherPal, TRUE );
                RealizePalette( pThis->mpGraphics->mhDC );
            }
            ImplSalInitGraphics( pThis->mpGraphics );
            pThis->mbGraphics = TRUE;
        }
    }

    // now destroy original hwnd
    DestroyWindow( hWndOld );

    pThis->mbInReparent = FALSE;
}

// -----------------------------------------------------------------------

void WinSalFrame::SetParent( SalFrame* pNewParent )
{
    ImplSetParentFrame( this, static_cast<WinSalFrame*>(pNewParent)->mhWnd, FALSE );
}

bool WinSalFrame::SetPluginParent( SystemParentData* pNewParent )
{
    ImplSetParentFrame( this, pNewParent->hWnd, TRUE );
    return true;
}


// -----------------------------------------------------------------------

void WinSalFrame::GetWorkArea( Rectangle &rRect )
{
    RECT aRect;
    ImplSalGetWorkArea( mhWnd, &aRect, NULL );
    rRect.nLeft     = aRect.left;
    rRect.nRight    = aRect.right-1;
    rRect.nTop      = aRect.top;
    rRect.nBottom   = aRect.bottom-1;
}

// -----------------------------------------------------------------------

void WinSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    rWidth  = maGeometry.nWidth;
    rHeight = maGeometry.nHeight;
}

// -----------------------------------------------------------------------

void WinSalFrame::SetWindowState( const SalFrameState* pState )
{
    // Wir testen, ob das Fenster ueberhaupt auf den Bildschirm passt, damit
    // nicht wenn die Bildschirm-Aufloesung geaendert wurde, das Fenster aus
    // diesem herausragt
    int     nX;
    int     nY;
    int     nWidth;
    int     nHeight;
    int     nScreenX;
    int     nScreenY;
    int     nScreenWidth;
    int     nScreenHeight;

    RECT aRect;
    ImplSalGetWorkArea( mhWnd, &aRect, NULL );
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

    // Fenster-Position/Groesse in den Bildschirm einpassen
    if ( !(pState->mnMask & (SAL_FRAMESTATE_MASK_X | SAL_FRAMESTATE_MASK_Y)) )
        nPosSize |= SWP_NOMOVE;
    if ( !(pState->mnMask & (SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT)) )
        nPosSize |= SWP_NOSIZE;
    if ( pState->mnMask & SAL_FRAMESTATE_MASK_X )
        nX = (int)pState->mnX - nLeftDeco;
    else
        nX = aWinRect.left;
    if ( pState->mnMask & SAL_FRAMESTATE_MASK_Y )
        nY = (int)pState->mnY - nTopDeco;
    else
        nY = aWinRect.top;
    if ( pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH )
        nWidth = (int)pState->mnWidth + nLeftDeco + nRightDeco;
    else
        nWidth = aWinRect.right-aWinRect.left;
    if ( pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT )
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

    // Restore-Position setzen
    WINDOWPLACEMENT aPlacement;
    aPlacement.length = sizeof( aPlacement );
    GetWindowPlacement( mhWnd, &aPlacement );

    // Status setzen
    BOOL bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    BOOL bUpdateHiddenFramePos = FALSE;
    if ( !bVisible )
    {
        aPlacement.showCmd = SW_HIDE;

        if ( mbOverwriteState )
        {
            if ( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
            {
                if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
                    mnShowState = SW_SHOWMINIMIZED;
                else if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
                {
                    mnShowState = SW_SHOWMAXIMIZED;
                    bUpdateHiddenFramePos = TRUE;
                }
                else if ( pState->mnState & SAL_FRAMESTATE_NORMAL )
                    mnShowState = SW_SHOWNORMAL;
            }
        }
    }
    else
    {
        if ( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
        {
            if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
            {
                if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
                    aPlacement.flags |= WPF_RESTORETOMAXIMIZED;
                aPlacement.showCmd = SW_SHOWMINIMIZED;
            }
            else if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
                aPlacement.showCmd = SW_SHOWMAXIMIZED;
            else if ( pState->mnState & SAL_FRAMESTATE_NORMAL )
                aPlacement.showCmd = SW_RESTORE;
        }
    }

    // Wenn Fenster nicht minimiert/maximiert ist oder nicht optisch
    // umgesetzt werden muss, dann SetWindowPos() benutzen, da
    // SetWindowPlacement() die TaskBar mit einrechnet
    if ( !IsIconic( mhWnd ) && !IsZoomed( mhWnd ) &&
         (!bVisible || (aPlacement.showCmd == SW_RESTORE)) )
    {
        if( bUpdateHiddenFramePos )
        {
            // #96084 set a useful internal window size because
            // the window will not be maximized (and the size updated) before show()
            POINT pt;
            GetCursorPos( &pt );
            RECT aRectMouse;
            aRectMouse.left = pt.x;
            aRectMouse.top = pt.y;
            aRectMouse.right = pt.x+2;
            aRectMouse.bottom = pt.y+2;

            // dualmonitor support:
            // Get screensize of the monitor whith the mouse pointer

            RECT aRect;
            ImplSalGetWorkArea( mhWnd, &aRect, &aRectMouse );
            AdjustWindowRectEx( &aRect, GetWindowStyle( mhWnd ),
                                FALSE,     GetWindowExStyle( mhWnd ) );
            maGeometry.nX = aRect.left;
            maGeometry.nY = aRect.top;;
            maGeometry.nWidth = aRect.right - aRect.left + 1;
            maGeometry.nHeight = aRect.bottom - aRect.top + 1;
        }
        else
            SetWindowPos( mhWnd, 0,
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

// -----------------------------------------------------------------------

BOOL WinSalFrame::GetWindowState( SalFrameState* pState )
{
    if ( maState.mnWidth && maState.mnHeight )
    {
        *pState = maState;
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - Don't save minimize
        //if ( !(pState->mnState & SAL_FRAMESTATE_MAXIMIZED) )
        if ( !(pState->mnState & (SAL_FRAMESTATE_MINIMIZED | SAL_FRAMESTATE_MAXIMIZED)) )
            pState->mnState |= SAL_FRAMESTATE_NORMAL;
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void WinSalFrame::ShowFullScreen( BOOL bFullScreen )
{
    if ( mbFullScreen == bFullScreen )
        return;

    mbFullScreen = bFullScreen;
    if ( bFullScreen )
    {
#if ( WINVER >= 0x0400 )
        // Damit Taskleiste von Windows ausgeblendet wird
        DWORD nExStyle = GetWindowExStyle( mhWnd );
        if ( nExStyle & WS_EX_TOOLWINDOW )
        {
            mbFullScreenToolWin = TRUE;
            nExStyle &= ~WS_EX_TOOLWINDOW;
            SetWindowExStyle( mhWnd, nExStyle );
        }
#endif

        // save old position
        GetWindowRect( mhWnd, &maFullScreenRect );

        // save show state
        mnFullScreenShowState = mnShowState;
        if ( !(GetWindowStyle( mhWnd ) & WS_VISIBLE) )
            mnShowState = SW_SHOW;

        // set window to screen size
        ImplSalFrameFullScreenPos( this, TRUE );
    }
    else
    {
        // wenn ShowState wieder hergestellt werden muss, hiden wir zuerst
        // das Fenster, damit es nicht so sehr flackert
        BOOL bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
        if ( bVisible && (mnShowState != mnFullScreenShowState) )
            ShowWindow( mhWnd, SW_HIDE );

#if ( WINVER >= 0x0400 )
        if ( mbFullScreenToolWin )
            SetWindowExStyle( mhWnd, GetWindowExStyle( mhWnd ) | WS_EX_TOOLWINDOW );
        mbFullScreenToolWin = FALSE;
#endif

        SetWindowPos( mhWnd, 0,
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

// -----------------------------------------------------------------------

void WinSalFrame::StartPresentation( BOOL bStart )
{
    if ( mbPresentation == bStart )
        return;

    mbPresentation = bStart;

    SalData* pSalData = GetSalData();
    if ( bStart )
    {
#if ( WINVER >= 0x0400 )
        if ( !pSalData->mpSageEnableProc )
        {
            if ( pSalData->mnSageStatus != DISABLE_AGENT )
            {
                OFSTRUCT aOS;
                OpenFile( "SAGE.DLL", &aOS, OF_EXIST );

                if ( !aOS.nErrCode )
                {
                    pSalData->mhSageInst = LoadLibrary( aOS.szPathName );
                    pSalData->mpSageEnableProc = (SysAgt_Enable_PROC)GetProcAddress( pSalData->mhSageInst, "System_Agent_Enable" );
                }
                else
                    pSalData->mnSageStatus = DISABLE_AGENT;
            }
        }

        if ( pSalData->mpSageEnableProc )
        {
            pSalData->mnSageStatus = pSalData->mpSageEnableProc( GET_AGENT_STATUS );
            if ( pSalData->mnSageStatus == ENABLE_AGENT )
                pSalData->mpSageEnableProc( DISABLE_AGENT );
        }
#endif

        // Bildschirmschoner ausschalten, wenn Praesentation laueft
        SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, 0,
                              &(pSalData->mbScrSvrEnabled), 0 );
        if ( pSalData->mbScrSvrEnabled )
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0 );
    }
    else
    {
        // Bildschirmschoner wieder einschalten
        if ( pSalData->mbScrSvrEnabled )
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, pSalData->mbScrSvrEnabled, 0, 0 );

#if ( WINVER >= 0x0400 )
        // Systemagenten wieder aktivieren
        if ( pSalData->mnSageStatus == ENABLE_AGENT )
            pSalData->mpSageEnableProc( pSalData->mnSageStatus );
#endif
    }
}

// -----------------------------------------------------------------------

void WinSalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
    HWND hWnd;
    if ( bOnTop )
        hWnd = HWND_TOPMOST;
    else
        hWnd = HWND_NOTOPMOST;
    SetWindowPos( mhWnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
}

// -----------------------------------------------------------------------

static void ImplSalToTop( HWND hWnd, USHORT nFlags )
{
    if ( nFlags & SAL_FRAME_TOTOP_FOREGROUNDTASK )
        SetForegroundWindow( hWnd );

    if ( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
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

    if ( !IsIconic( hWnd ) )
    {
        SetFocus( hWnd );

        // Windows behauptet oefters mal, das man den Focus hat, obwohl
        // man diesen nicht hat. Wenn dies der Fall ist, dann versuchen
        // wir diesen auch ganz richtig zu bekommen.
        if ( ::GetFocus() == hWnd )
            SetForegroundWindow( hWnd );
    }
}

// -----------------------------------------------------------------------

void WinSalFrame::ToTop( USHORT nFlags )
{
    nFlags &= ~SAL_FRAME_TOTOP_GRABFOCUS;   // this flag is not needed on win32
    // Post this Message to the window, because this only works
    // in the thread of the window, which has create this window.
    // We post this message to avoid deadlocks
    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
        ImplPostMessage( mhWnd, SAL_MSG_TOTOP, nFlags, 0 );
    else
        ImplSalToTop( mhWnd, nFlags );
}

// -----------------------------------------------------------------------

void WinSalFrame::SetPointer( PointerStyle ePointerStyle )
{
    struct ImplPtrData
    {
        HCURSOR         mhCursor;
        LPCSTR          mnSysId;
        UINT            mnOwnId;
    };

    static ImplPtrData aImplPtrTab[POINTER_COUNT] =
    {
    { 0, IDC_ARROW, 0 },                            // POINTER_ARROW
    { 0, 0, SAL_RESID_POINTER_NULL },               // POINTER_NULL
    { 0, IDC_WAIT, 0 },                             // POINTER_WAIT
    { 0, IDC_IBEAM, 0 },                            // POINTER_TEXT
#if ( WINVER >= 0x0400 )
    { 0, IDC_HELP, 0 },                             // POINTER_HELP
#else
    { 0, 0, SAL_RESID_POINTER_HELP },               // POINTER_HELP
#endif
    { 0, 0, SAL_RESID_POINTER_CROSS },              // POINTER_CROSS
    { 0, 0, SAL_RESID_POINTER_MOVE },               // POINTER_MOVE
    { 0, IDC_SIZENS, 0 },                           // POINTER_NSIZE
    { 0, IDC_SIZENS, 0 },                           // POINTER_SSIZE
    { 0, IDC_SIZEWE, 0 },                           // POINTER_WSIZE
    { 0, IDC_SIZEWE, 0 },                           // POINTER_ESIZE
    { 0, IDC_SIZENWSE, 0 },                         // POINTER_NWSIZE
    { 0, IDC_SIZENESW, 0 },                         // POINTER_NESIZE
    { 0, IDC_SIZENESW, 0 },                         // POINTER_SWSIZE
    { 0, IDC_SIZENWSE, 0 },                         // POINTER_SESIZE
    { 0, IDC_SIZENS, 0 },                           // POINTER_WINDOW_NSIZE
    { 0, IDC_SIZENS, 0 },                           // POINTER_WINDOW_SSIZE
    { 0, IDC_SIZEWE, 0 },                           // POINTER_WINDOW_WSIZE
    { 0, IDC_SIZEWE, 0 },                           // POINTER_WINDOW_ESIZE
    { 0, IDC_SIZENWSE, 0 },                         // POINTER_WINDOW_NWSIZE
    { 0, IDC_SIZENESW, 0 },                         // POINTER_WINDOW_NESIZE
    { 0, IDC_SIZENESW, 0 },                         // POINTER_WINDOW_SWSIZE
    { 0, IDC_SIZENWSE, 0 },                         // POINTER_WINDOW_SESIZE
    { 0, 0, SAL_RESID_POINTER_HSPLIT },             // POINTER_HSPLIT
    { 0, 0, SAL_RESID_POINTER_VSPLIT },             // POINTER_VSPLIT
    { 0, 0, SAL_RESID_POINTER_HSIZEBAR },           // POINTER_HSIZEBAR
    { 0, 0, SAL_RESID_POINTER_VSIZEBAR },           // POINTER_VSIZEBAR
    { 0, 0, SAL_RESID_POINTER_HAND },               // POINTER_HAND
    { 0, 0, SAL_RESID_POINTER_REFHAND },            // POINTER_REFHAND
    { 0, 0, SAL_RESID_POINTER_PEN },                // POINTER_PEN
    { 0, 0, SAL_RESID_POINTER_MAGNIFY },            // POINTER_MAGNIFY
    { 0, 0, SAL_RESID_POINTER_FILL },               // POINTER_FILL
    { 0, 0, SAL_RESID_POINTER_ROTATE },             // POINTER_ROTATE
    { 0, 0, SAL_RESID_POINTER_HSHEAR },             // POINTER_HSHEAR
    { 0, 0, SAL_RESID_POINTER_VSHEAR },             // POINTER_VSHEAR
    { 0, 0, SAL_RESID_POINTER_MIRROR },             // POINTER_MIRROR
    { 0, 0, SAL_RESID_POINTER_CROOK },              // POINTER_CROOK
    { 0, 0, SAL_RESID_POINTER_CROP },               // POINTER_CROP
    { 0, 0, SAL_RESID_POINTER_MOVEPOINT },          // POINTER_MOVEPOINT
    { 0, 0, SAL_RESID_POINTER_MOVEBEZIERWEIGHT },   // POINTER_MOVEBEZIERWEIGHT
    { 0, 0, SAL_RESID_POINTER_MOVEDATA },           // POINTER_MOVEDATA
    { 0, 0, SAL_RESID_POINTER_COPYDATA },           // POINTER_COPYDATA
    { 0, 0, SAL_RESID_POINTER_LINKDATA },           // POINTER_LINKDATA
    { 0, 0, SAL_RESID_POINTER_MOVEDATALINK },       // POINTER_MOVEDATALINK
    { 0, 0, SAL_RESID_POINTER_COPYDATALINK },       // POINTER_COPYDATALINK
    { 0, 0, SAL_RESID_POINTER_MOVEFILE },           // POINTER_MOVEFILE
    { 0, 0, SAL_RESID_POINTER_COPYFILE },           // POINTER_COPYFILE
    { 0, 0, SAL_RESID_POINTER_LINKFILE },           // POINTER_LINKFILE
    { 0, 0, SAL_RESID_POINTER_MOVEFILELINK },       // POINTER_MOVEFILELINK
    { 0, 0, SAL_RESID_POINTER_COPYFILELINK },       // POINTER_COPYFILELINK
    { 0, 0, SAL_RESID_POINTER_MOVEFILES },          // POINTER_MOVEFILES
    { 0, 0, SAL_RESID_POINTER_COPYFILES },          // POINTER_COPYFILES
    { 0, 0, SAL_RESID_POINTER_NOTALLOWED },         // POINTER_NOTALLOWED
    { 0, 0, SAL_RESID_POINTER_DRAW_LINE },          // POINTER_DRAW_LINE
    { 0, 0, SAL_RESID_POINTER_DRAW_RECT },          // POINTER_DRAW_RECT
    { 0, 0, SAL_RESID_POINTER_DRAW_POLYGON },       // POINTER_DRAW_POLYGON
    { 0, 0, SAL_RESID_POINTER_DRAW_BEZIER },        // POINTER_DRAW_BEZIER
    { 0, 0, SAL_RESID_POINTER_DRAW_ARC },           // POINTER_DRAW_ARC
    { 0, 0, SAL_RESID_POINTER_DRAW_PIE },           // POINTER_DRAW_PIE
    { 0, 0, SAL_RESID_POINTER_DRAW_CIRCLECUT },     // POINTER_DRAW_CIRCLECUT
    { 0, 0, SAL_RESID_POINTER_DRAW_ELLIPSE },       // POINTER_DRAW_ELLIPSE
    { 0, 0, SAL_RESID_POINTER_DRAW_FREEHAND },      // POINTER_DRAW_FREEHAND
    { 0, 0, SAL_RESID_POINTER_DRAW_CONNECT },       // POINTER_DRAW_CONNECT
    { 0, 0, SAL_RESID_POINTER_DRAW_TEXT },          // POINTER_DRAW_TEXT
    { 0, 0, SAL_RESID_POINTER_DRAW_CAPTION },       // POINTER_DRAW_CAPTION
    { 0, 0, SAL_RESID_POINTER_CHART },              // POINTER_CHART
    { 0, 0, SAL_RESID_POINTER_DETECTIVE },          // POINTER_DETECTIVE
    { 0, 0, SAL_RESID_POINTER_PIVOT_COL },          // POINTER_PIVOT_COL
    { 0, 0, SAL_RESID_POINTER_PIVOT_ROW },          // POINTER_PIVOT_ROW
    { 0, 0, SAL_RESID_POINTER_PIVOT_FIELD },        // POINTER_PIVOT_FIELD
    { 0, 0, SAL_RESID_POINTER_CHAIN },              // POINTER_CHAIN
    { 0, 0, SAL_RESID_POINTER_CHAIN_NOTALLOWED },   // POINTER_CHAIN_NOTALLOWED
    { 0, 0, SAL_RESID_POINTER_TIMEEVENT_MOVE },     // POINTER_TIMEEVENT_MOVE
    { 0, 0, SAL_RESID_POINTER_TIMEEVENT_SIZE },     // POINTER_TIMEEVENT_SIZE
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_N },       // POINTER_AUTOSCROLL_N
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_S },       // POINTER_AUTOSCROLL_S
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_W },       // POINTER_AUTOSCROLL_W
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_E },       // POINTER_AUTOSCROLL_E
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NW },      // POINTER_AUTOSCROLL_NW
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NE },      // POINTER_AUTOSCROLL_NE
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_SW },      // POINTER_AUTOSCROLL_SW
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_SE },      // POINTER_AUTOSCROLL_SE
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NS },      // POINTER_AUTOSCROLL_NS
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_WE },      // POINTER_AUTOSCROLL_WE
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NSWE },    // POINTER_AUTOSCROLL_NSWE
    { 0, 0, SAL_RESID_POINTER_AIRBRUSH },           // POINTER_AIRBRUSH
    { 0, 0, SAL_RESID_POINTER_TEXT_VERTICAL },      // POINTER_TEXT_VERTICAL
    { 0, 0, SAL_RESID_POINTER_PIVOT_DELETE }        // POINTER_PIVOT_DELETE
    };

#if POINTER_COUNT != 88
#error New Pointer must be defined!
#endif

    // Mousepointer loaded ?
    if ( !aImplPtrTab[ePointerStyle].mhCursor )
    {
        if ( aImplPtrTab[ePointerStyle].mnOwnId )
            aImplPtrTab[ePointerStyle].mhCursor = ImplLoadSalCursor( aImplPtrTab[ePointerStyle].mnOwnId );
        else
            aImplPtrTab[ePointerStyle].mhCursor = LoadCursor( 0, aImplPtrTab[ePointerStyle].mnSysId );
    }

    // Unterscheidet sich der Mauspointer, dann den neuen setzen
    if ( mhCursor != aImplPtrTab[ePointerStyle].mhCursor )
    {
        mhCursor = aImplPtrTab[ePointerStyle].mhCursor;
        SetCursor( mhCursor );
    }
}

// -----------------------------------------------------------------------

void WinSalFrame::CaptureMouse( BOOL bCapture )
{
    // Send this Message to the window, because CaptureMouse() only work
    // in the thread of the window, which has create this window
    int nMsg;
    if ( bCapture )
        nMsg = SAL_MSG_CAPTUREMOUSE;
    else
        nMsg = SAL_MSG_RELEASEMOUSE;
    ImplSendMessage( mhWnd, nMsg, 0, 0 );
}

// -----------------------------------------------------------------------

void WinSalFrame::SetPointerPos( long nX, long nY )
{
    POINT aPt;
    aPt.x = (int)nX;
    aPt.y = (int)nY;
    ClientToScreen( mhWnd, &aPt );
    SetCursorPos( aPt.x, aPt.y );
}

// -----------------------------------------------------------------------

void WinSalFrame::Flush()
{
    GdiFlush();
}

// -----------------------------------------------------------------------

void WinSalFrame::Sync()
{
    GdiFlush();
}

// -----------------------------------------------------------------------

static void ImplSalFrameSetInputContext( HWND hWnd, const SalInputContext* pContext )
{
    WinSalFrame*   pFrame = GetWindowPtr( hWnd );
    BOOL        bIME = (pContext->mnOptions & SAL_INPUTCONTEXT_TEXT) != 0;
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
        if ( !(pContext->mnOptions & SAL_INPUTCONTEXT_EXTTEXTINPUT) )
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
            ImmAssociateContext( pFrame->mhWnd, 0 );
        }
    }
}

// -----------------------------------------------------------------------

void WinSalFrame::SetInputContext( SalInputContext* pContext )
{
    // Must be called in the main thread!
    ImplSendMessage( mhWnd, SAL_MSG_SETINPUTCONTEXT, 0, (LPARAM)(void*)pContext );
}

// -----------------------------------------------------------------------

static void ImplSalFrameEndExtTextInput( HWND hWnd, USHORT nFlags )
{
    HIMC hIMC = ImmGetContext( hWnd );
    if ( hIMC )
    {
        DWORD nIndex;
        if ( nFlags & SAL_FRAME_ENDEXTTEXTINPUT_COMPLETE )
            nIndex = CPS_COMPLETE;
        else
            nIndex = CPS_CANCEL;

        ImmNotifyIME( hIMC, NI_COMPOSITIONSTR, nIndex, 0 );
        ImmReleaseContext( hWnd, hIMC );
    }
}

// -----------------------------------------------------------------------

void WinSalFrame::EndExtTextInput( USHORT nFlags )
{
    // Must be called in the main thread!
    ImplSendMessage( mhWnd, SAL_MSG_ENDEXTTEXTINPUT, (WPARAM)nFlags, 0 );
}

// -----------------------------------------------------------------------

static void ImplGetKeyNameText( LONG lParam, sal_Unicode* pBuf,
                                UINT& rCount, UINT nMaxSize,
                                const sal_Char* pReplace )
{
    DBG_ASSERT( sizeof( WCHAR ) == sizeof( xub_Unicode ), "WinSalFrame::ImplGetKeyNameTextW(): WCHAR != sal_Unicode" );

    WCHAR aKeyBuf[350];
    int nKeyLen = 0;
    if ( lParam )
    {
        nKeyLen = GetKeyNameTextW( lParam, aKeyBuf, sizeof( aKeyBuf ) / sizeof( sal_Unicode ) );
        // #i12401# the current unicows.dll has a bug in CharUpperBuffW, which corrupts the stack
        // fall back to the ANSI version instead
        if ( aSalShlData.mbWNT && nKeyLen > 0 )
        {
            // Convert name, so that the keyname start with an upper
            // char and the rest of the word are in lower chars
            CharLowerBuffW( aKeyBuf, nKeyLen );
            CharUpperBuffW( aKeyBuf, 1 );
            WCHAR cTempChar;
            WCHAR* pKeyBuf = aKeyBuf;
            while ( (cTempChar = *pKeyBuf) != 0 )
            {
                if ( (cTempChar == '+') || (cTempChar == '-') ||
                     (cTempChar == ' ') || (cTempChar == '.') )
                    CharUpperBuffW( pKeyBuf+1, 1 );
                pKeyBuf++;
            }
        }
        else
        {
            sal_Char aAnsiKeyBuf[250];
            int nAnsiKeyLen = GetKeyNameTextA( lParam, aAnsiKeyBuf, sizeof( aAnsiKeyBuf ) / sizeof( sal_Char ) );
            if ( nAnsiKeyLen )
            {
                // Convert name, so that the keyname start with an upper
                // char and the rest of the word are in lower chars
                CharLowerBuffA( aAnsiKeyBuf, nAnsiKeyLen );
                CharUpperBuffA( aAnsiKeyBuf, 1 );
                sal_Char cTempChar;
                sal_Char* pAnsiKeyBuf = aAnsiKeyBuf;
                while ( (cTempChar = *pAnsiKeyBuf) != 0 )
                {
                    if ( (cTempChar == '+') || (cTempChar == '-') ||
                         (cTempChar == ' ') || (cTempChar == '.') )
                        CharUpperBuffA( pAnsiKeyBuf+1, 1 );
                    pAnsiKeyBuf++;
                }

                // Convert to Unicode and copy the data in the Unicode Buffer
                nKeyLen = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, aAnsiKeyBuf, nAnsiKeyLen, aKeyBuf, sizeof( aKeyBuf ) / sizeof( sal_Unicode ) );
            }
        }
    }

    if ( (nKeyLen > 0) || pReplace )
    {
        if ( rCount )
        {
            pBuf[rCount] = '+';
            rCount++;
        }

        if ( nKeyLen )
        {
            memcpy( pBuf+rCount, aKeyBuf, nKeyLen*sizeof( sal_Unicode ) );
            rCount += nKeyLen;
        }
        else
        {
            while ( *pReplace )
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

// -----------------------------------------------------------------------

XubString WinSalFrame::GetKeyName( USHORT nKeyCode )
{
    XubString   aKeyCode;
    sal_Unicode aKeyBuf[350];
    UINT        nKeyBufLen = 0;
    UINT        nSysCode;

    if ( nKeyCode & KEY_MOD2 )
    {
        nSysCode = MapVirtualKey( VK_MENU, 0 );
        nSysCode = (nSysCode << 16) | (((ULONG)1) << 25);
        ImplGetKeyNameText( nSysCode, aKeyBuf, nKeyBufLen,
                            sizeof( aKeyBuf ) / sizeof( sal_Unicode ),
                            "Alt" );
    }

    if ( nKeyCode & KEY_MOD1 )
    {
        nSysCode = MapVirtualKey( VK_CONTROL, 0 );
        nSysCode = (nSysCode << 16) | (((ULONG)1) << 25);
        ImplGetKeyNameText( nSysCode, aKeyBuf, nKeyBufLen,
                            sizeof( aKeyBuf ) / sizeof( sal_Unicode ),
                            "Ctrl" );
    }

    if ( nKeyCode & KEY_SHIFT )
    {
        nSysCode = MapVirtualKey( VK_SHIFT, 0 );
        nSysCode = (nSysCode << 16) | (((ULONG)1) << 25);
        ImplGetKeyNameText( nSysCode, aKeyBuf, nKeyBufLen,
                            sizeof( aKeyBuf ) / sizeof( sal_Unicode ),
                            "Shift" );
    }

    USHORT      nCode = nKeyCode & 0x0FFF;
    ULONG       nSysCode2 = 0;
    sal_Char*   pReplace = NULL;
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
            aFBuf[1] = '1' + (nCode - KEY_F1);
            aFBuf[2] = 0;
        }
        else if ( (nCode >= KEY_F10) && (nCode <= KEY_F19) )
        {
            aFBuf[1] = '1';
            aFBuf[2] = '0' + (nCode - KEY_F10);
            aFBuf[3] = 0;
        }
        else
        {
            aFBuf[1] = '2';
            aFBuf[2] = '0' + (nCode - KEY_F20);
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
                nSysCode2 = (((ULONG)1) << 24);
                pReplace = "Down";
                break;
            case KEY_UP:
                nSysCode = VK_UP;
                nSysCode2 = (((ULONG)1) << 24);
                pReplace = "Up";
                break;
            case KEY_LEFT:
                nSysCode = VK_LEFT;
                nSysCode2 = (((ULONG)1) << 24);
                pReplace = "Left";
                break;
            case KEY_RIGHT:
                nSysCode = VK_RIGHT;
                nSysCode2 = (((ULONG)1) << 24);
                pReplace = "Right";
                break;
            case KEY_HOME:
                nSysCode = VK_HOME;
                nSysCode2 = (((ULONG)1) << 24);
                pReplace = "Home";
                break;
            case KEY_END:
                nSysCode = VK_END;
                nSysCode2 = (((ULONG)1) << 24);
                pReplace = "End";
                break;
            case KEY_PAGEUP:
                nSysCode = VK_PRIOR;
                nSysCode2 = (((ULONG)1) << 24);
                pReplace = "Page Up";
                break;
            case KEY_PAGEDOWN:
                nSysCode = VK_NEXT;
                nSysCode2 = (((ULONG)1) << 24);
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
                nSysCode2 = (((ULONG)1) << 24);
                pReplace = "Insert";
                break;
            case KEY_DELETE:
                nSysCode = VK_DELETE;
                nSysCode2 = (((ULONG)1) << 24);
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
        }
    }

    if ( nSysCode )
    {
        nSysCode = MapVirtualKey( (UINT)nSysCode, 0 );
        if ( nSysCode )
            nSysCode = (nSysCode << 16) | nSysCode2;
        ImplGetKeyNameText( nSysCode, aKeyBuf, nKeyBufLen,
                            sizeof( aKeyBuf ) / sizeof( sal_Unicode ),
                            pReplace );
    }
    else
    {
        if ( cSVCode )
        {
            if ( !nKeyBufLen )
            {
                aKeyBuf[0] = cSVCode;
                nKeyBufLen = 1;
            }
            else
            {
                aKeyBuf[nKeyBufLen] = '+';
                nKeyBufLen++;
                aKeyBuf[nKeyBufLen] = cSVCode;
                nKeyBufLen++;
            }
        }
    }

    if ( nKeyBufLen )
        aKeyCode.Assign( (const sal_Unicode*)aKeyBuf, nKeyBufLen );

    return aKeyCode;
}

// -----------------------------------------------------------------------

XubString WinSalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
    return GetKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

inline Color ImplWinColorToSal( COLORREF nColor )
{
    return Color( GetRValue( nColor ), GetGValue( nColor ), GetBValue( nColor ) );
}

// -----------------------------------------------------------------------

static void ImplSalUpdateStyleFontA( HDC hDC, const LOGFONTA& rLogFont, Font& rFont,
                                     BOOL bReplaceFont )
{
    ImplSalLogFontToFontA( hDC, rLogFont, rFont, bReplaceFont );

    // On Windows 9x, Windows NT we get sometimes very small sizes
    // (for example for the small Caption height).
    // So if it is MS Sans Serif, a none scalable font we use
    // 8 Point as the minimum control height, in all other cases
    // 6 Point is the smallest one
    if ( rFont.GetHeight() < 8 )
    {
        if ( rtl_str_compareIgnoreAsciiCase( rLogFont.lfFaceName, "MS Sans Serif" ) == 0 )
            rFont.SetHeight( 8 );
        else if ( rFont.GetHeight() < 6 )
            rFont.SetHeight( 6 );
    }
}

// -----------------------------------------------------------------------

static void ImplSalUpdateStyleFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont,
                                     BOOL bReplaceFont )
{
    ImplSalLogFontToFontW( hDC, rLogFont, rFont, bReplaceFont );

    // On Windows 9x, Windows NT we get sometimes very small sizes
    // (for example for the small Caption height).
    // So if it is MS Sans Serif, a none scalable font we use
    // 8 Point as the minimum control height, in all other cases
    // 6 Point is the smallest one
    if ( rFont.GetHeight() < 8 )
    {
        if ( rtl_ustr_compareIgnoreAsciiCase( rLogFont.lfFaceName, L"MS Sans Serif" ) == 0 )
            rFont.SetHeight( 8 );
        else if ( rFont.GetHeight() < 6 )
            rFont.SetHeight( 6 );
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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
        if ( RegQueryValueEx( hRegKey, "MenuShowDelay", 0,
                              &nType, aValueBuf, &nValueSize ) == ERROR_SUCCESS )
        {
            if ( nType == REG_SZ )
                aMouseSettings.SetMenuDelay( (ULONG)ImplA2I( aValueBuf ) );
        }

        RegCloseKey( hRegKey );
    }

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    BOOL bCompBorder = (aStyleSettings.GetOptions() & (STYLE_OPTION_MACSTYLE | STYLE_OPTION_UNIXSTYLE)) == 0;
#if (_MSC_VER < 1300)
    aStyleSettings.SetScrollBarSize( std::min( GetSystemMetrics( SM_CXVSCROLL ), 20 ) ); // #99956# do not allow huge scrollbars, most of the UI is not scaled anymore
    aStyleSettings.SetSpinSize( std::min( GetSystemMetrics( SM_CXVSCROLL ), 20 ) );
#else
    aStyleSettings.SetScrollBarSize( min( GetSystemMetrics( SM_CXVSCROLL ), 20 ) ); // #99956# do not allow huge scrollbars, most of the UI is not scaled anymore
    aStyleSettings.SetSpinSize( min( GetSystemMetrics( SM_CXVSCROLL ), 20 ) );
#endif
    aStyleSettings.SetCursorBlinkTime( GetCaretBlinkTime() );
    if ( bCompBorder )
    {
        aStyleSettings.SetFloatTitleHeight( GetSystemMetrics( SM_CYSMCAPTION ) );
        aStyleSettings.SetTitleHeight( GetSystemMetrics( SM_CYCAPTION ) );
        aStyleSettings.SetActiveBorderColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVEBORDER ) ) );
        aStyleSettings.SetDeactiveBorderColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVEBORDER ) ) );
        if ( aSalShlData.mnVersion >= 410 )
        {
            aStyleSettings.SetActiveColor2( ImplWinColorToSal( GetSysColor( COLOR_GRADIENTACTIVECAPTION ) ) );
            aStyleSettings.SetDeactiveColor( ImplWinColorToSal( GetSysColor( COLOR_GRADIENTINACTIVECAPTION ) ) );
        }
        aStyleSettings.SetFaceColor( ImplWinColorToSal( GetSysColor( COLOR_3DFACE ) ) );
        aStyleSettings.SetLightColor( ImplWinColorToSal( GetSysColor( COLOR_3DHILIGHT ) ) );
        aStyleSettings.SetLightBorderColor( ImplWinColorToSal( GetSysColor( COLOR_3DLIGHT ) ) );
        aStyleSettings.SetShadowColor( ImplWinColorToSal( GetSysColor( COLOR_3DSHADOW ) ) );
        aStyleSettings.SetDarkShadowColor( ImplWinColorToSal( GetSysColor( COLOR_3DDKSHADOW ) ) );
    }
    aStyleSettings.SetWorkspaceColor( ImplWinColorToSal( GetSysColor( COLOR_APPWORKSPACE ) ) );
    aStyleSettings.SetHelpColor( ImplWinColorToSal( GetSysColor( COLOR_INFOBK ) ) );
    aStyleSettings.SetHelpTextColor( ImplWinColorToSal( GetSysColor( COLOR_INFOTEXT ) ) );
    aStyleSettings.SetDialogColor( aStyleSettings.GetFaceColor() );
    aStyleSettings.SetDialogTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetButtonTextColor( ImplWinColorToSal( GetSysColor( COLOR_BTNTEXT ) ) );
    aStyleSettings.SetRadioCheckTextColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOWTEXT ) ) );
    aStyleSettings.SetGroupTextColor( aStyleSettings.GetRadioCheckTextColor() );
    aStyleSettings.SetLabelTextColor( aStyleSettings.GetRadioCheckTextColor() );
    aStyleSettings.SetInfoTextColor( aStyleSettings.GetRadioCheckTextColor() );
    aStyleSettings.SetWindowColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOW ) ) );
    aStyleSettings.SetWindowTextColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOWTEXT ) ) );
    aStyleSettings.SetFieldColor( aStyleSettings.GetWindowColor() );
    aStyleSettings.SetFieldTextColor( aStyleSettings.GetWindowTextColor() );
    aStyleSettings.SetHighlightColor( ImplWinColorToSal( GetSysColor( COLOR_HIGHLIGHT ) ) );
    aStyleSettings.SetHighlightTextColor( ImplWinColorToSal( GetSysColor( COLOR_HIGHLIGHTTEXT ) ) );
    aStyleSettings.SetMenuHighlightColor( aStyleSettings.GetHighlightColor() );
    aStyleSettings.SetMenuHighlightTextColor( aStyleSettings.GetHighlightTextColor() );
    if ( bCompBorder )
    {
        aStyleSettings.SetMenuColor( ImplWinColorToSal( GetSysColor( COLOR_MENU ) ) );
        aStyleSettings.SetMenuBarColor( aStyleSettings.GetMenuColor() );
        aStyleSettings.SetMenuBorderColor( aStyleSettings.GetLightBorderColor() ); // overriden below for flat menus
        aStyleSettings.SetUseFlatMenues( FALSE );
        aStyleSettings.SetMenuTextColor( ImplWinColorToSal( GetSysColor( COLOR_MENUTEXT ) ) );
        aStyleSettings.SetActiveColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVECAPTION ) ) );
        aStyleSettings.SetActiveTextColor( ImplWinColorToSal( GetSysColor( COLOR_CAPTIONTEXT ) ) );
        aStyleSettings.SetDeactiveColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVECAPTION ) ) );
        aStyleSettings.SetDeactiveTextColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVECAPTIONTEXT ) ) );
        if ( aSalShlData.mbWXP )
        {
            // only xp supports a different menu bar color
            long bFlatMenues = 0;
            SystemParametersInfo( SPI_GETFLATMENU, 0, &bFlatMenues, 0);
            if( bFlatMenues )
            {
                aStyleSettings.SetUseFlatMenues( TRUE );
                aStyleSettings.SetMenuBarColor( ImplWinColorToSal( GetSysColor( COLOR_MENUBAR ) ) );
                aStyleSettings.SetMenuHighlightColor( ImplWinColorToSal( GetSysColor( COLOR_MENUHILIGHT ) ) );
                aStyleSettings.SetMenuBorderColor( ImplWinColorToSal( GetSysColor( COLOR_3DSHADOW ) ) );
            }
        }
    }
    // Bei hellgrau geben wir die Farbe vor, damit es besser aussieht
    if ( aStyleSettings.GetFaceColor() == COL_LIGHTGRAY )
        aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
    else
    {
        // Checked-Color berechnen
        Color   aColor1 = aStyleSettings.GetFaceColor();
        Color   aColor2 = aStyleSettings.GetLightColor();
        BYTE    nRed    = (BYTE)(((USHORT)aColor1.GetRed()   + (USHORT)aColor2.GetRed())/2);
        BYTE    nGreen  = (BYTE)(((USHORT)aColor1.GetGreen() + (USHORT)aColor2.GetGreen())/2);
        BYTE    nBlue   = (BYTE)(((USHORT)aColor1.GetBlue()  + (USHORT)aColor2.GetBlue())/2);
        aStyleSettings.SetCheckedColor( Color( nRed, nGreen, nBlue ) );
    }

    // High contrast
    HIGHCONTRAST hc;
    hc.cbSize = sizeof( HIGHCONTRAST );
    if( SystemParametersInfo( SPI_GETHIGHCONTRAST, hc.cbSize, &hc, 0) && (hc.dwFlags & HCF_HIGHCONTRASTON) )
        aStyleSettings.SetHighContrastMode( 1 );
    else
        aStyleSettings.SetHighContrastMode( 0 );


    // Query Fonts
    Font    aMenuFont = aStyleSettings.GetMenuFont();
    Font    aTitleFont = aStyleSettings.GetTitleFont();
    Font    aFloatTitleFont = aStyleSettings.GetFloatTitleFont();
    Font    aHelpFont = aStyleSettings.GetHelpFont();
    Font    aAppFont = aStyleSettings.GetAppFont();
    Font    aIconFont = aStyleSettings.GetIconFont();
    HDC     hDC = GetDC( 0 );
    BOOL    bReplaceFont = !ImplIsFontAvailable( hDC, XubString( RTL_CONSTASCII_USTRINGPARAM( "Andale Sans UI" ) ) );

    bReplaceFont |= aStyleSettings.GetUseSystemUIFonts();

    if ( aSalShlData.mbWNT )
    {
        NONCLIENTMETRICSW aNonClientMetrics;
        aNonClientMetrics.cbSize = sizeof( aNonClientMetrics );
        if ( SystemParametersInfoW( SPI_GETNONCLIENTMETRICS, sizeof( aNonClientMetrics ), &aNonClientMetrics, 0 ) )
        {
            ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfMenuFont, aMenuFont, bReplaceFont );
            ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfCaptionFont, aTitleFont, bReplaceFont );
            ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfSmCaptionFont, aFloatTitleFont, bReplaceFont );
            ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfStatusFont, aHelpFont, bReplaceFont );
            ImplSalUpdateStyleFontW( hDC, aNonClientMetrics.lfMessageFont, aAppFont, bReplaceFont );

            LOGFONTW aLogFont;
            if ( SystemParametersInfoW( SPI_GETICONTITLELOGFONT, 0, &aLogFont, 0 ) )
                ImplSalUpdateStyleFontW( hDC, aLogFont, aIconFont, bReplaceFont );
        }
    }
    else
    {
        NONCLIENTMETRICSA aNonClientMetrics;
        aNonClientMetrics.cbSize = sizeof( aNonClientMetrics );
        if ( SystemParametersInfoA( SPI_GETNONCLIENTMETRICS, sizeof( aNonClientMetrics ), &aNonClientMetrics, 0 ) )
        {
            ImplSalUpdateStyleFontA( hDC, aNonClientMetrics.lfMenuFont, aMenuFont, bReplaceFont );
            ImplSalUpdateStyleFontA( hDC, aNonClientMetrics.lfCaptionFont, aTitleFont, bReplaceFont );
            ImplSalUpdateStyleFontA( hDC, aNonClientMetrics.lfSmCaptionFont, aFloatTitleFont, bReplaceFont );
            ImplSalUpdateStyleFontA( hDC, aNonClientMetrics.lfStatusFont, aHelpFont, bReplaceFont );
            ImplSalUpdateStyleFontA( hDC, aNonClientMetrics.lfMessageFont, aAppFont, bReplaceFont );

            LOGFONTA aLogFont;
            if ( SystemParametersInfoA( SPI_GETICONTITLELOGFONT, 0, &aLogFont, 0 ) )
                ImplSalUpdateStyleFontA( hDC, aLogFont, aIconFont, bReplaceFont );
        }
    }

    // get screen font resolution to calculate toolbox item size
    long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );

    ReleaseDC( 0, hDC );

    long nHeightPx = aMenuFont.GetHeight() * nDPIY / 72;
    aStyleSettings.SetToolbarIconSize( (((nHeightPx-1)*2) >= 28) ? STYLE_TOOLBAR_ICONSIZE_LARGE : STYLE_TOOLBAR_ICONSIZE_SMALL );

    aStyleSettings.SetMenuFont( aMenuFont );
    aStyleSettings.SetTitleFont( aTitleFont );
    aStyleSettings.SetFloatTitleFont( aFloatTitleFont );
    aStyleSettings.SetHelpFont( aHelpFont );
    aStyleSettings.SetIconFont( aIconFont );
    // We prefer Arial in the russian version, because MS Sans Serif
    // is to wide for the dialogs
    if ( rSettings.GetInternational().GetLanguage() == LANGUAGE_RUSSIAN )
    {
        XubString aFontName = aAppFont.GetName();
        XubString aFirstName = aFontName.GetToken( 0, ';' );
        if ( aFirstName.EqualsIgnoreCaseAscii( "MS Sans Serif" ) )
        {
            aFontName.InsertAscii( "Arial;", 0 );
            aAppFont.SetName( aFontName );
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
    aStyleSettings.SetInfoFont( aAppFont );
    aStyleSettings.SetToolFont( aAppFont );

    WIN_BOOL bDragFull;
    if ( SystemParametersInfo( SPI_GETDRAGFULLWINDOWS, 0, &bDragFull, 0 ) )
    {
        ULONG nDragFullOptions = aStyleSettings.GetDragFullOptions();
        if ( bDragFull )
            nDragFullOptions |=  DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE | DRAGFULL_OPTION_DOCKING | DRAGFULL_OPTION_SPLIT;
        else
            nDragFullOptions &= ~(DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE | DRAGFULL_OPTION_DOCKING | DRAGFULL_OPTION_SPLIT);
        aStyleSettings.SetDragFullOptions( nDragFullOptions );
    }

    aStyleSettings.SetIconHorzSpace( GetSystemMetrics( SM_CXICONSPACING ) );
    aStyleSettings.SetIconVertSpace( GetSystemMetrics( SM_CYICONSPACING ) );
    if ( RegOpenKey( HKEY_CURRENT_USER,
                     "Control Panel\\International\\Calendars\\TwoDigitYearMax",
                     &hRegKey ) == ERROR_SUCCESS )
    {
        BYTE    aValueBuf[10];
        DWORD   nValue;
        DWORD   nValueSize = sizeof( aValueBuf );
        DWORD   nType;
        if ( RegQueryValueEx( hRegKey, "1", 0,
                              &nType, aValueBuf, &nValueSize ) == ERROR_SUCCESS )
        {
            if ( nType == REG_SZ )
            {
                nValue = (ULONG)ImplA2I( aValueBuf );
                if ( (nValue > 1000) && (nValue < 10000) )
                {
                    MiscSettings aMiscSettings = rSettings.GetMiscSettings();
                    aMiscSettings.SetTwoDigitYearStart( (USHORT)(nValue-99) );
                    rSettings.SetMiscSettings( aMiscSettings );
                }
            }
        }

        RegCloseKey( hRegKey );
    }

    rSettings.SetMouseSettings( aMouseSettings );
    rSettings.SetStyleSettings( aStyleSettings );
}

// -----------------------------------------------------------------------

SalBitmap* WinSalFrame::SnapShot()
{
    WinSalBitmap* pSalBitmap = NULL;

    RECT aRect;
    GetWindowRect( mhWnd, &aRect );

    int     nDX = aRect.right-aRect.left;
    int     nDY = aRect.bottom-aRect.top;
    HDC     hDC = GetWindowDC( mhWnd );
    HBITMAP hBmpBitmap = CreateCompatibleBitmap( hDC, nDX, nDY );
    HDC     hBmpDC = ImplGetCachedDC( CACHED_HDC_1, hBmpBitmap );
    BOOL    bRet;

    bRet = BitBlt( hBmpDC, 0, 0, nDX, nDY, hDC, 0, 0, SRCCOPY );
    ImplReleaseCachedDC( CACHED_HDC_1 );

    if ( bRet )
    {
        pSalBitmap = new WinSalBitmap;

        if ( !pSalBitmap->Create( hBmpBitmap, FALSE, FALSE ) )
        {
            delete pSalBitmap;
            pSalBitmap = NULL;
        }
    }

    return pSalBitmap;
}

// -----------------------------------------------------------------------

const SystemEnvData* WinSalFrame::GetSystemData() const
{
    return &maSysData;
}

// -----------------------------------------------------------------------

void WinSalFrame::Beep( SoundType eSoundType )
{
    static UINT aImplSoundTab[5] =
    {
        0,                              // SOUND_DEFAULT
        MB_ICONASTERISK,                // SOUND_INFO
        MB_ICONEXCLAMATION,             // SOUND_WARNING
        MB_ICONHAND,                    // SOUND_ERROR
        MB_ICONQUESTION                 // SOUND_QUERY
    };

#if SOUND_COUNT != 5
#error New Sound must be defined!
#endif

    MessageBeep( aImplSoundTab[eSoundType] );
}

// -----------------------------------------------------------------------

ULONG WinSalFrame::GetCurrentModButtons()
{
    ULONG nMod = 0;

    if ( GetKeyState( VK_LBUTTON ) & 0x8000 )
        nMod |= MOUSE_LEFT;
    if ( GetKeyState( VK_MBUTTON ) & 0x8000 )
        nMod |= MOUSE_MIDDLE;
    if ( GetKeyState( VK_RBUTTON ) & 0x8000 )
        nMod |= MOUSE_RIGHT;
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        nMod |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        nMod |= KEY_MOD1;
    if ( GetKeyState( VK_MENU ) & 0x8000 )
        nMod |= KEY_MOD2;
    return nMod;
}

// -----------------------------------------------------------------------

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
        Window *pWin = (Window*) pFrame->GetInstance();
        if( pWin && pWin->mpFrameData->mnFocusId )
        {
            ImplPostMessage( hWnd, nMsg, wParam, lParam );
            return 1;
        }
    }
    SalMouseEvent   aMouseEvt;
    long            nRet;
    USHORT          nEvent;
    BOOL            bCall = TRUE;

    aMouseEvt.mnX       = (short)LOWORD( lParam );
    aMouseEvt.mnY       = (short)HIWORD( lParam );
    aMouseEvt.mnCode    = 0;
    aMouseEvt.mnTime    = GetMessageTime();

    // Wegen (Logitech-)MouseTreiber ueber GetKeyState() gehen, die auf
    // mittlerer Maustaste Doppelklick simulieren und den KeyStatus nicht
    // beruecksichtigen

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
            // Da bei Druecken von Modifier-Tasten die MouseEvents
            // nicht zusammengefast werden (da diese durch KeyEvents
            // unterbrochen werden), machen wir dieses hier selber
            if ( aMouseEvt.mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2) )
            {
                MSG aTempMsg;
                if ( ImplPeekMessage( &aTempMsg, hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE | PM_NOYIELD ) )
                {
                    if ( (aTempMsg.message == WM_MOUSEMOVE) &&
                         (aTempMsg.wParam == wParam) )
                        return 1;
                }
            }

            SalData* pSalData = GetSalData();
            // Test for MouseLeave
            if ( pSalData->mhWantLeaveMsg && (pSalData->mhWantLeaveMsg != hWnd) )
                ImplSendMessage( pSalData->mhWantLeaveMsg, SAL_MSG_MOUSELEAVE, 0, GetMessagePos() );

            pSalData->mhWantLeaveMsg = hWnd;
            // Start MouseLeave-Timer
            if ( !pSalData->mpMouseLeaveTimer )
            {
                pSalData->mpMouseLeaveTimer = new AutoTimer;
                pSalData->mpMouseLeaveTimer->SetTimeout( SAL_MOUSELEAVE_TIMEOUT );
                pSalData->mpMouseLeaveTimer->Start();
                // We dont need to set a timeout handler, because we test
                // for mouseleave in the timeout callback
            }
            aMouseEvt.mnButton = 0;
            nEvent = SALEVENT_MOUSEMOVE;
            }
            break;

        case WM_NCMOUSEMOVE:
        case SAL_MSG_MOUSELEAVE:
            {
            SalData* pSalData = GetSalData();
            if ( pSalData->mhWantLeaveMsg == hWnd )
            {
                pSalData->mhWantLeaveMsg = 0;
                if ( pSalData->mpMouseLeaveTimer )
                {
                    delete pSalData->mpMouseLeaveTimer;
                    pSalData->mpMouseLeaveTimer = NULL;
                }
                // Mouse-Coordinaates are relativ to the screen
                POINT aPt;
                aPt.x = (short)LOWORD( lParam );
                aPt.y = (short)HIWORD( lParam );
                ScreenToClient( hWnd, &aPt );
                aMouseEvt.mnX = aPt.x;
                aMouseEvt.mnY = aPt.y;
                aMouseEvt.mnButton = 0;
                nEvent = SALEVENT_MOUSELEAVE;
            }
            else
                bCall = FALSE;
            }
            break;

        case WM_LBUTTONDOWN:
            aMouseEvt.mnButton = MOUSE_LEFT;
            nEvent = SALEVENT_MOUSEBUTTONDOWN;
            break;

        case WM_MBUTTONDOWN:
            aMouseEvt.mnButton = MOUSE_MIDDLE;
            nEvent = SALEVENT_MOUSEBUTTONDOWN;
            break;

        case WM_RBUTTONDOWN:
            aMouseEvt.mnButton = MOUSE_RIGHT;
            nEvent = SALEVENT_MOUSEBUTTONDOWN;
            break;

        case WM_LBUTTONUP:
            aMouseEvt.mnButton = MOUSE_LEFT;
            nEvent = SALEVENT_MOUSEBUTTONUP;
            break;

        case WM_MBUTTONUP:
            aMouseEvt.mnButton = MOUSE_MIDDLE;
            nEvent = SALEVENT_MOUSEBUTTONUP;
            break;

        case WM_RBUTTONUP:
            aMouseEvt.mnButton = MOUSE_RIGHT;
            nEvent = SALEVENT_MOUSEBUTTONUP;
            break;
    }

    // check if this window was destroyed - this might happen if we are the help window
    // and sent a mouse leave message to the application which killed the help window, ie ourself
    if( !IsWindow( hWnd ) )
        return 0;

    if ( bCall )
    {
        if ( nEvent == SALEVENT_MOUSEBUTTONDOWN )
            UpdateWindow( hWnd );

        // --- RTL --- (mirror mouse pos)
        if( Application::GetSettings().GetLayoutRTL() )
            aMouseEvt.mnX = pFrame->maGeometry.nWidth-1-aMouseEvt.mnX;

        nRet = pFrame->CallCallback( nEvent, &aMouseEvt );
        if ( nMsg == WM_MOUSEMOVE )
            SetCursor( pFrame->mhCursor );
    }
    else
        nRet = 0;

    return nRet;
}

// -----------------------------------------------------------------------

static long ImplHandleMouseActivateMsg( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    if ( pFrame->mbFloatWin )
        return TRUE;

    SalMouseActivateEvent   aMouseActivateEvt;
    POINT                   aPt;
    GetCursorPos( &aPt );
    ScreenToClient( hWnd, &aPt );
    aMouseActivateEvt.mnX = aPt.x;
    aMouseActivateEvt.mnY = aPt.y;
    return pFrame->CallCallback( SALEVENT_MOUSEACTIVATE, &aMouseActivateEvt );
}

// -----------------------------------------------------------------------

static long ImplHandleWheelMsg( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
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
        if ( aSalShlData.mnWheelScrollLines == WHEEL_PAGESCROLL )
            aWheelEvt.mnScrollLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
        else
            aWheelEvt.mnScrollLines = aSalShlData.mnWheelScrollLines;
        aWheelEvt.mbHorz        = FALSE;

        if ( nWinModCode & MK_SHIFT )
            aWheelEvt.mnCode |= KEY_SHIFT;
        if ( nWinModCode & MK_CONTROL )
            aWheelEvt.mnCode |= KEY_MOD1;
        if ( GetKeyState( VK_MENU ) & 0x8000 )
            aWheelEvt.mnCode |= KEY_MOD2;

        // --- RTL --- (mirror mouse pos)
        if( Application::GetSettings().GetLayoutRTL() )
            aWheelEvt.mnX = pFrame->maGeometry.nWidth-1-aWheelEvt.mnX;

        nRet = pFrame->CallCallback( SALEVENT_WHEELMOUSE, &aWheelEvt );
    }

    ImplSalYieldMutexRelease();

    return nRet;
}

// -----------------------------------------------------------------------

static USHORT ImplSalGetKeyCode( WPARAM wParam )
{
    USHORT nKeyCode;

    // convert KeyCode
    if ( wParam < KEY_TAB_SIZE )
        nKeyCode = aImplTranslateKeyTab[wParam];
    else if ( wParam == aSalShlData.mnVKAdd )
        nKeyCode = KEY_ADD;
    else if ( wParam == aSalShlData.mnVKSubtract )
        nKeyCode = KEY_SUBTRACT;
    else if ( wParam == aSalShlData.mnVKMultiply )
        nKeyCode = KEY_MULTIPLY;
    else if ( wParam == aSalShlData.mnVKDivide )
        nKeyCode = KEY_DIVIDE;
    else if ( wParam == aSalShlData.mnVKPoint )
        nKeyCode = KEY_POINT;
    else if ( wParam == aSalShlData.mnVKComma )
        nKeyCode = KEY_COMMA;
    else if ( wParam == aSalShlData.mnVKLess )
        nKeyCode = KEY_LESS;
    else if ( wParam == aSalShlData.mnVKGreater )
        nKeyCode = KEY_GREATER;
    else if ( wParam == aSalShlData.mnVKEqual )
        nKeyCode = KEY_EQUAL;
    else
        nKeyCode = 0;

    return nKeyCode;
}

// -----------------------------------------------------------------------

static UINT ImplStrToNum( const sal_Char* pStr )
{
    USHORT n = 0;

    // Solange es sich um eine Ziffer handelt, String umwandeln
    while( (*pStr >= 48) && (*pStr <= 57) )
    {
        n *= 10;
        n += ((*pStr) - 48);
        pStr++;
    }

    return n;
}

// -----------------------------------------------------------------------

LanguageType WinSalFrame::GetInputLanguage()
{
    if( !mnInputLang )
        return LANGUAGE_DONTKNOW;
    else
        return (LanguageType) mnInputLang;
}

// -----------------------------------------------------------------------

static sal_Unicode ImplGetCharCode( WinSalFrame* pFrame, WPARAM nCharCode )
{
    BOOL bLanguageChange = FALSE;
    UINT nLang = LOWORD( GetKeyboardLayout( 0 ) );
    if ( nLang && nLang != pFrame->mnInputLang )
    {
        // keep input lang up-to-date
        pFrame->mnInputLang = nLang;
        bLanguageChange = TRUE;
    }

    // If we are on Windows NT we use Unicode FrameProcs and so we
    // get Unicode charcodes directly from Windows
    if ( aSalShlData.mbWNT )
        return (sal_Unicode)nCharCode;

    if ( !nLang )
    {
        pFrame->mnInputLang     = 0;
        pFrame->mnInputCodePage = GetACP();
    }
    else if ( bLanguageChange )
    {
        sal_Char aBuf[10];
        if ( GetLocaleInfoA( MAKELCID( nLang, SORT_DEFAULT ), LOCALE_IDEFAULTANSICODEPAGE,
                             aBuf, sizeof(aBuf) ) > 0 )
        {
            pFrame->mnInputCodePage = ImplStrToNum( aBuf );
            if ( !pFrame->mnInputCodePage )
                pFrame->mnInputCodePage = GetACP();
        }
        else
            pFrame->mnInputCodePage = GetACP();
    }

    sal_Char    aCharBuf[2];
    int         nCharLen;
    WCHAR       c;
    if ( nCharCode > 0xFF )
    {
        aCharBuf[0] = (sal_Char)(nCharCode>>8);
        aCharBuf[1] = (sal_Char)nCharCode;
        nCharLen = 2;
    }
    else
    {
        aCharBuf[0] = (sal_Char)nCharCode;
        nCharLen = 1;
    }
    if ( ::MultiByteToWideChar( pFrame->mnInputCodePage,
                                MB_PRECOMPOSED,
                                aCharBuf, nCharLen, &c, 1 ) )
        return (sal_Unicode)c;
    else
        return (sal_Unicode)nCharCode;
}

// -----------------------------------------------------------------------

static long ImplHandleKeyMsg( HWND hWnd, UINT nMsg,
                              WPARAM wParam, LPARAM lParam, LRESULT& rResult )
{
    static BOOL     bIgnoreCharMsg  = FALSE;
    static WPARAM   nDeadChar       = 0;
    static WPARAM   nLastVKChar     = 0;
    static USHORT   nLastChar       = 0;
    static USHORT   nLastModKeyCode = 0;
    static bool     bWaitForModKeyRelease = false;
    USHORT          nRepeat         = LOWORD( lParam )-1;
    USHORT          nModCode        = 0;

    // Key wurde evtl. durch SysChild an uns weitergeleitet und
    // darf somit dann nicht doppelt verarbeitet werden
    GetSalData()->mnSalObjWantKeyEvt = 0;

    if ( nMsg == WM_DEADCHAR )
    {
        nDeadChar = wParam;
        return 0;
    }

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    // Wir restaurieren den Background-Modus bei jeder Texteingabe,
    // da einige Tools wie RichWin uns diesen hin- und wieder umsetzen
    if ( pFrame->mpGraphics &&
         pFrame->mpGraphics->mhDC )
        SetBkMode( pFrame->mpGraphics->mhDC, TRANSPARENT );

    // determine modifiers
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        nModCode |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        nModCode |= KEY_MOD1;
    if ( GetKeyState( VK_MENU ) & 0x8000 )
    {
        nModCode |= KEY_MOD2;
        if ( !(nModCode & KEY_MOD1) &&
             ((nMsg == WM_SYSKEYDOWN) || (nMsg == WM_SYSKEYUP)) )
            nModCode |= KEY_CONTROLMOD;
    }

    if ( (nMsg == WM_CHAR) || (nMsg == WM_SYSCHAR) )
    {
        nDeadChar = 0;

        if ( bIgnoreCharMsg )
        {
            bIgnoreCharMsg = FALSE;
            // #101635# if zero is returned here for WM_SYSCHAR (ALT+<key>) Windows will beep
            // becaus this 'hotkey' was not processed -> better return 1
            // except for Alt-SPACE which should always open the sysmenu (#104616#)

            // also return zero if a system menubar is available that might process this hotkey
            // this also applies to the OLE inplace embedding where we are a child window
            if( (GetWindowStyle( hWnd ) & WS_CHILD) || GetMenu( hWnd ) || (wParam == 0x20) )
                return 0;
            else
                return 1;
        }

        // Backspace ignorieren wir als eigenstaendige Taste,
        // damit wir keine Probleme in Kombination mit einem
        // DeadKey bekommen
        if ( wParam == 0x08 )    // BACKSPACE
            return 0;

        // Hier kommen nur "freifliegende" WM_CHAR Message an, die durch
        // eintippen einer ALT-NUMPAD Kombination erzeugt wurden
        SalKeyEvent aKeyEvt;

        if ( (wParam >= '0') && (wParam <= '9') )
            aKeyEvt.mnCode = KEYGROUP_NUM + wParam - '0';
        else if ( (wParam >= 'A') && (wParam <= 'Z') )
            aKeyEvt.mnCode = KEYGROUP_ALPHA + wParam - 'A';
        else if ( (wParam >= 'a') && (wParam <= 'z') )
            aKeyEvt.mnCode = KEYGROUP_ALPHA + wParam - 'a';
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
        long nRet = pFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
        pFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
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
            sal_Unicode ch = (sal_Unicode) Uni_UTF32ToSurrogate1(wParam);
             nLastChar = 0;
             nLastVKChar = 0;
             long nRet = pFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
             pFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
            wParam = (sal_Unicode) Uni_UTF32ToSurrogate2( wParam );
         }

         aKeyEvt.mnCharCode = (sal_Unicode) wParam;

         nLastChar = 0;
         nLastVKChar = 0;
         long nRet = pFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
         pFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );

         return nRet;
     }
     // MCD, 2003-01-13, Support for WM_UNICHAR & Keyman 6.0; addition ends
    else
    {
        // Bei Shift, Control und Menu schicken wir einen KeyModChange-Event
        if ( (wParam == VK_SHIFT) || (wParam == VK_CONTROL) || (wParam == VK_MENU) )
        {
            SalKeyModEvent aModEvt;
            aModEvt.mnTime = GetMessageTime();
            aModEvt.mnCode = nModCode;
            aModEvt.mnModKeyCode = 0;   // no command events will be sent if this member is 0

            USHORT tmpCode = 0;
            if( GetKeyState( VK_LSHIFT )  & 0x8000 )
                tmpCode |= MODKEY_LSHIFT;
            if( GetKeyState( VK_RSHIFT )  & 0x8000 )
                tmpCode |= MODKEY_RSHIFT;
            if( GetKeyState( VK_LCONTROL ) & 0x8000 )
                tmpCode |= MODKEY_LMOD1;
            if( GetKeyState( VK_RCONTROL ) & 0x8000 )
                tmpCode |= MODKEY_RMOD1;
            if( GetKeyState( VK_LMENU )  & 0x8000 )
                tmpCode |= MODKEY_LMOD2;
            if( GetKeyState( VK_RMENU )  & 0x8000 )
                tmpCode |= MODKEY_RMOD2;

            if( tmpCode < nLastModKeyCode )
            {
                aModEvt.mnModKeyCode = nLastModKeyCode;
                nLastModKeyCode = 0;
                bWaitForModKeyRelease = true;
            }
            else
            {
                if( !bWaitForModKeyRelease )
                    nLastModKeyCode = tmpCode;
            }

            if( !tmpCode )
                bWaitForModKeyRelease = false;

            return pFrame->CallCallback( SALEVENT_KEYMODCHANGE, &aModEvt );
        }
        else
        {
            SalKeyEvent     aKeyEvt;
            USHORT          nEvent;
            MSG             aCharMsg;
            WIN_BOOL        bCharPeek = FALSE;
            UINT            nCharMsg = WM_CHAR;
            BOOL            bKeyUp = (nMsg == WM_KEYUP) || (nMsg == WM_SYSKEYUP);

            nLastModKeyCode = 0; // make sure no modkey messages are sent if they belong to a hotkey (see above)
            bWaitForModKeyRelease = true;

            aKeyEvt.mnCode = ImplSalGetKeyCode( wParam );
            if ( !bKeyUp )
            {
                // check for charcode
                // Mit Hilfe von PeekMessage holen wir uns jetzt die
                // zugehoerige WM_CHAR Message, wenn vorhanden.
                // Diese WM_CHAR Message steht immer am Anfang der
                // Messagequeue. Ausserdem ist sichergestellt, dass immer
                // nur eine WM_CHAR Message in der Queue steht.
                bCharPeek = ImplPeekMessage( &aCharMsg, hWnd,
                                             WM_CHAR, WM_CHAR, PM_NOREMOVE | PM_NOYIELD );
                if ( bCharPeek && (nDeadChar == aCharMsg.wParam) )
                {
                    bCharPeek = FALSE;
                    nDeadChar = 0;

                    if ( wParam == VK_BACK )
                    {
                        ImplPeekMessage( &aCharMsg, hWnd,
                                         nCharMsg, nCharMsg, PM_REMOVE | PM_NOYIELD );
                        return 0;
                    }
                }
                else
                {
                    if ( !bCharPeek )
                    {
                        bCharPeek = ImplPeekMessage( &aCharMsg, hWnd,
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
                    nEvent = SALEVENT_KEYUP;
                else
                    nEvent = SALEVENT_KEYINPUT;

                aKeyEvt.mnTime      = GetMessageTime();
                aKeyEvt.mnCode     |= nModCode;
                aKeyEvt.mnRepeat    = nRepeat;
                bIgnoreCharMsg = bCharPeek;
                long nRet = pFrame->CallCallback( nEvent, &aKeyEvt );
                bIgnoreCharMsg = FALSE;

                // char-message, than remove or ignore
                if ( bCharPeek )
                {
                    nDeadChar = 0;
                    if ( nRet )
                    {
                        ImplPeekMessage( &aCharMsg, hWnd,
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

// -----------------------------------------------------------------------

long ImplHandleSalObjKeyMsg( HWND hWnd, UINT nMsg,
                             WPARAM wParam, LPARAM lParam )
{
    if ( (nMsg == WM_KEYDOWN) || (nMsg == WM_KEYUP) )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( !pFrame )
            return 0;

        USHORT  nRepeat     = LOWORD( lParam )-1;
        USHORT  nModCode    = 0;

        // determine modifiers
        if ( GetKeyState( VK_SHIFT ) & 0x8000 )
            nModCode |= KEY_SHIFT;
        if ( GetKeyState( VK_CONTROL ) & 0x8000 )
            nModCode |= KEY_MOD1;
        if ( GetKeyState( VK_MENU ) & 0x8000 )
        {
            nModCode |= KEY_MOD2;
            if ( !(nModCode & KEY_MOD1) )
                nModCode |= KEY_CONTROLMOD;
        }

        if ( (wParam != VK_SHIFT) && (wParam != VK_CONTROL) && (wParam != VK_MENU) )
        {
            SalKeyEvent     aKeyEvt;
            USHORT          nEvent;
            BOOL            bKeyUp = (nMsg == WM_KEYUP) || (nMsg == WM_SYSKEYUP);

            // convert KeyCode
            aKeyEvt.mnCode      = ImplSalGetKeyCode( wParam );
            aKeyEvt.mnCharCode  = 0;

            if ( aKeyEvt.mnCode )
            {
                if ( bKeyUp )
                    nEvent = SALEVENT_KEYUP;
                else
                    nEvent = SALEVENT_KEYINPUT;

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

// -----------------------------------------------------------------------

long ImplHandleSalObjSysCharMsg( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    USHORT  nRepeat     = LOWORD( lParam )-1;
    USHORT  nModCode    = 0;
    USHORT  cKeyCode    = (USHORT)wParam;

    // determine modifiers
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        nModCode |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        nModCode |= KEY_MOD1;
    nModCode |= KEY_MOD2;
    if ( !(nModCode & KEY_MOD1) )
        nModCode |= KEY_CONTROLMOD;

    // KeyEvent zusammenbauen
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
    long nRet = pFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
    pFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
    return nRet;
}

// -----------------------------------------------------------------------

static void ImplHandlePaintMsg( HWND hWnd )
{
    BOOL bMutex = FALSE;
    if ( ImplSalYieldMutexTryToAcquire() )
        bMutex = TRUE;

    // if we don't get the mutex, we can also change the clip region,
    // because other threads doesn't use the mutex from the main
    // thread --> see GetGraphics()

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        // Clip-Region muss zurueckgesetzt werden, da wir sonst kein
        // ordentliches Bounding-Rectangle bekommen
        if ( pFrame->mpGraphics && pFrame->mpGraphics->mhRegion )
            SelectClipRgn( pFrame->mpGraphics->mhDC, 0 );

        // Laut Window-Doku soll man erst abfragen, ob ueberhaupt eine
        // Paint-Region anliegt
        if ( GetUpdateRect( hWnd, NULL, FALSE ) )
        {
            // Call BeginPaint/EndPaint to query the rect and send
            // this Notofication to rect
            RECT aUpdateRect;
            PAINTSTRUCT aPs;
            BeginPaint( hWnd, &aPs );
            CopyRect( &aUpdateRect, &aPs.rcPaint );
            EndPaint( hWnd, &aPs );

            // Paint
            // ClipRegion wieder herstellen
            if ( pFrame->mpGraphics && pFrame->mpGraphics->mhRegion )
            {
                SelectClipRgn( pFrame->mpGraphics->mhDC,
                               pFrame->mpGraphics->mhRegion );
            }

            if ( bMutex )
            {
                SalPaintEvent aPEvt;
                aPEvt.mnBoundX          = aUpdateRect.left;
                aPEvt.mnBoundY          = aUpdateRect.top;
                aPEvt.mnBoundWidth      = aUpdateRect.right-aUpdateRect.left;
                aPEvt.mnBoundHeight     = aUpdateRect.bottom-aUpdateRect.top;

                // --- RTL --- (mirror paint rect)
                if( Application::GetSettings().GetLayoutRTL() )
                    aPEvt.mnBoundX = pFrame->maGeometry.nWidth-aPEvt.mnBoundWidth-aPEvt.mnBoundX;

                pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
            }
            else
            {
                RECT* pRect = new RECT;
                CopyRect( pRect, &aUpdateRect );
                ImplPostMessage( hWnd, SAL_MSG_POSTPAINT, (WPARAM)pRect, 0 );
            }
        }
        else
        {
            // ClipRegion wieder herstellen
            if ( pFrame->mpGraphics && pFrame->mpGraphics->mhRegion )
            {
                SelectClipRgn( pFrame->mpGraphics->mhDC,
                               pFrame->mpGraphics->mhRegion );
            }
        }
    }

    if ( bMutex )
        ImplSalYieldMutexRelease();
}

// -----------------------------------------------------------------------

static void ImplHandlePaintMsg2( HWND hWnd, RECT* pRect )
{
    // Paint
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            SalPaintEvent aPEvt;
            aPEvt.mnBoundX          = pRect->left;
            aPEvt.mnBoundY          = pRect->top;
            aPEvt.mnBoundWidth      = pRect->right-pRect->left;
            aPEvt.mnBoundHeight     = pRect->bottom-pRect->top;

            // --- RTL --- (mirror paint rect)
            if( Application::GetSettings().GetLayoutRTL() )
                aPEvt.mnBoundX = pFrame->maGeometry.nWidth-aPEvt.mnBoundWidth-aPEvt.mnBoundX;

            pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
        }
        ImplSalYieldMutexRelease();
        delete pRect;
    }
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTPAINT, (WPARAM)pRect, 0 );
}

// -----------------------------------------------------------------------

static void UpdateFrameGeometry( HWND hWnd, SalFrame* pFrame )
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
}

// -----------------------------------------------------------------------

static void ImplCallMoveHdl( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SALEVENT_MOVE, 0 );
        // Um doppelte Paints von VCL und SAL zu vermeiden
        //if ( IsWindowVisible( hWnd ) && !pFrame->mbInShow )
        //    UpdateWindow( hWnd );
    }
}

// -----------------------------------------------------------------------

static void ImplCallClosePopupsHdl( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SALEVENT_CLOSEPOPUPS, 0 );
    }
}

// -----------------------------------------------------------------------

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

            // Gegen moegliche Rekursionen sichern
            if ( !pFrame->mbInMoveMsg )
            {
                // Fenster im FullScreenModus wieder einpassen
                pFrame->mbInMoveMsg = TRUE;
                if ( pFrame->mbFullScreen )
                    ImplSalFrameFullScreenPos( pFrame );
                pFrame->mbInMoveMsg = FALSE;
            }

            // Status merken
            ImplSaveFrameState( pFrame );

            // Call Hdl
            //#93851 if we call this handler, VCL floating windows are not updated correctly
            ImplCallMoveHdl( hWnd );

        }

        ImplSalYieldMutexRelease();
    }
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTMOVE, 0, 0 );
}

// -----------------------------------------------------------------------

static void ImplCallSizeHdl( HWND hWnd )
{
    // Da Windows diese Messages auch senden kann, muss hier auch die
    // Solar-Semaphore beruecksichtigt werden
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->CallCallback( SALEVENT_RESIZE, 0 );
            // Um doppelte Paints von VCL und SAL zu vermeiden
            if ( IsWindowVisible( hWnd ) && !pFrame->mbInShow )
                UpdateWindow( hWnd );
        }

        ImplSalYieldMutexRelease();
    }
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTCALLSIZE, 0, 0 );
}

// -----------------------------------------------------------------------

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
            // Status merken
            ImplSaveFrameState( pFrame );
            // Call Hdl
            ImplCallSizeHdl( hWnd );
        }
    }
}

// -----------------------------------------------------------------------

static void ImplHandleFocusMsg( HWND hWnd )
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && !WinSalFrame::mbInReparent )
        {
            // Query the actual status
            if ( ::GetFocus() == hWnd )
            {
                if ( IsWindowVisible( hWnd ) && !pFrame->mbInShow )
                    UpdateWindow( hWnd );

                // Feststellen, ob wir IME unterstuetzen
                if ( pFrame->mbIME && pFrame->mhDefIMEContext )
                {
                    UINT nImeProps = ImmGetProperty( GetKeyboardLayout( 0 ), IGP_PROPERTY );

                    pFrame->mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
                    pFrame->mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
                    pFrame->mbHandleIME = !pFrame->mbSpezIME;
                }

                pFrame->CallCallback( SALEVENT_GETFOCUS, 0 );
            }
            else
            {
                pFrame->CallCallback( SALEVENT_LOSEFOCUS, 0 );
            }
        }

        ImplSalYieldMutexRelease();
    }
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTFOCUS, 0, 0 );
}

// -----------------------------------------------------------------------

static void ImplHandleCloseMsg( HWND hWnd )
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->CallCallback( SALEVENT_CLOSE, 0 );
        }

        ImplSalYieldMutexRelease();
    }
    else
        ImplPostMessage( hWnd, WM_CLOSE, 0, 0 );
}

// -----------------------------------------------------------------------

static long ImplHandleShutDownMsg( HWND hWnd )
{
    ImplSalYieldMutexAcquireWithWait();
    long        nRet = 0;
    WinSalFrame*   pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        nRet = pFrame->CallCallback( SALEVENT_SHUTDOWN, 0 );
    }
    ImplSalYieldMutexRelease();
    return nRet;
}

// -----------------------------------------------------------------------

static void ImplHandleSettingsChangeMsg( HWND hWnd, UINT nMsg,
                                         WPARAM wParam, LPARAM lParam )
{
    USHORT nSalEvent = SALEVENT_SETTINGSCHANGED;

    if ( nMsg == WM_DEVMODECHANGE )
        nSalEvent = SALEVENT_PRINTERCHANGED;
#ifdef WM_DISPLAYCHANGE
    else if ( nMsg == WM_DISPLAYCHANGE )
        nSalEvent = SALEVENT_DISPLAYCHANGED;
#endif
    else if ( nMsg == WM_FONTCHANGE )
        nSalEvent = SALEVENT_FONTCHANGED;
    else if ( nMsg == WM_TIMECHANGE )
        nSalEvent = SALEVENT_DATETIMECHANGED;
    else if ( nMsg == WM_WININICHANGE )
    {
        if ( lParam )
        {
            if ( aSalShlData.mbWNT )
            {
                if ( ImplSalWICompareAscii( (const wchar_t*)lParam, "devices" ) == 0 )
                    nSalEvent = SALEVENT_PRINTERCHANGED;
            }
            else
            {
                if ( stricmp( (const char*)lParam, "devices" ) == 0 )
                    nSalEvent = SALEVENT_PRINTERCHANGED;
            }
        }
    }

#ifdef WM_SETTINGCHANGE
    if ( nMsg == WM_SETTINGCHANGE )
    {
        if ( wParam == SPI_SETWHEELSCROLLLINES )
            aSalShlData.mnWheelScrollLines = ImplSalGetWheelScrollLines();
    }
#endif

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

        pFrame->CallCallback( nSalEvent, 0 );
    }

    ImplSalYieldMutexRelease();
}

// -----------------------------------------------------------------------

static void ImplHandleUserEvent( HWND hWnd, LPARAM lParam )
{
    ImplSalYieldMutexAcquireWithWait();
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SALEVENT_USEREVENT, (void*)lParam );
    }
    ImplSalYieldMutexRelease();
}

// -----------------------------------------------------------------------

static void ImplHandleForcePalette( HWND hWnd )
{
    SalData*    pSalData = GetSalData();
    HPALETTE    hPal = pSalData->mhDitherPal;
    if ( hPal )
    {
        if ( !ImplSalYieldMutexTryToAcquire() )
        {
            ImplPostMessage( hWnd, SAL_MSG_FORCEPALETTE, 0, 0 );
            return;
        }

        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && pFrame->mpGraphics )
        {
            WinSalGraphics* pGraphics = pFrame->mpGraphics;
            if ( pGraphics && pGraphics->mhDefPal )
            {
                SelectPalette( pGraphics->mhDC, hPal, FALSE );
                if ( RealizePalette( pGraphics->mhDC ) )
                {
                    InvalidateRect( hWnd, NULL, FALSE );
                    UpdateWindow( hWnd );
                    pFrame->CallCallback( SALEVENT_DISPLAYCHANGED, 0 );
                }
            }
        }

        ImplSalYieldMutexRelease();
    }
}

// -----------------------------------------------------------------------

static LRESULT ImplHandlePalette( BOOL bFrame, HWND hWnd, UINT nMsg,
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
        if ( (HWND)wParam == hWnd )
            return 0;
    }

    BOOL bReleaseMutex = FALSE;
    if ( (nMsg == WM_QUERYNEWPALETTE) || (nMsg == WM_PALETTECHANGED) )
    {
        // Da Windows diese Messages auch sendet, muss hier auch die
        // Solar-Semaphore beruecksichtigt werden
        if ( ImplSalYieldMutexTryToAcquire() )
            bReleaseMutex = TRUE;
        else if ( nMsg == WM_QUERYNEWPALETTE )
            ImplPostMessage( hWnd, SAL_MSG_POSTQUERYNEWPAL, wParam, lParam );
        else /* ( nMsg == WM_PALETTECHANGED ) */
            ImplPostMessage( hWnd, SAL_MSG_POSTPALCHANGED, wParam, lParam );
    }

    WinSalVirtualDevice*pTempVD;
    WinSalFrame*        pTempFrame;
    WinSalGraphics*     pGraphics;
    HDC                 hDC;
    HPALETTE            hOldPal;
    UINT                nCols;
    BOOL                bStdDC;
    BOOL                bUpdate;

    pSalData->mbInPalChange = TRUE;

    // Alle Paletten in VirDevs und Frames zuruecksetzen
    pTempVD = pSalData->mpFirstVD;
    while ( pTempVD )
    {
        pGraphics = pTempVD->mpGraphics;
        if ( pGraphics->mhDefPal )
        {
            SelectPalette( pGraphics->mhDC,
                           pGraphics->mhDefPal,
                           TRUE );
        }
        pTempVD = pTempVD->mpNext;
    }
    pTempFrame = pSalData->mpFirstFrame;
    while ( pTempFrame )
    {
        pGraphics = pTempFrame->mpGraphics;
        if ( pGraphics && pGraphics->mhDefPal )
        {
            SelectPalette( pGraphics->mhDC,
                           pGraphics->mhDefPal,
                           TRUE );
        }
        pTempFrame = pTempFrame->mpNextFrame;
    }

    // Palette neu realizen
    WinSalFrame* pFrame = NULL;
    if ( bFrame )
        pFrame = GetWindowPtr( hWnd );
    if ( pFrame && pFrame->mpGraphics )
    {
        hDC = pFrame->mpGraphics->mhDC;
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

    // Alle Paletten in VirDevs und Frames neu setzen
    pTempVD = pSalData->mpFirstVD;
    while ( pTempVD )
    {
        pGraphics = pTempVD->mpGraphics;
        if ( pGraphics->mhDefPal )
        {
            SelectPalette( pGraphics->mhDC, hPal, TRUE );
            RealizePalette( pGraphics->mhDC );
        }
        pTempVD = pTempVD->mpNext;
    }
    pTempFrame = pSalData->mpFirstFrame;
    while ( pTempFrame )
    {
        if ( pTempFrame != pFrame )
        {
            pGraphics = pTempFrame->mpGraphics;
            if ( pGraphics && pGraphics->mhDefPal )
            {
                SelectPalette( pGraphics->mhDC, hPal, TRUE );
                if ( RealizePalette( pGraphics->mhDC ) )
                    bUpdate = TRUE;
            }
        }
        pTempFrame = pTempFrame->mpNextFrame;
    }

    // Wenn sich Farben geaendert haben, dann die Fenster updaten
    if ( bUpdate )
    {
        pTempFrame = pSalData->mpFirstFrame;
        while ( pTempFrame )
        {
            pGraphics = pTempFrame->mpGraphics;
            if ( pGraphics && pGraphics->mhDefPal )
            {
                InvalidateRect( pTempFrame->mhWnd, NULL, FALSE );
                UpdateWindow( pTempFrame->mhWnd );
                pTempFrame->CallCallback( SALEVENT_DISPLAYCHANGED, 0 );
            }
            pTempFrame = pTempFrame->mpNextFrame;
        }
    }

    pSalData->mbInPalChange = FALSE;

    if ( bReleaseMutex )
        ImplSalYieldMutexRelease();

    if ( nMsg == WM_PALETTECHANGED )
        return 0;
    else
        return nCols;
}

// -----------------------------------------------------------------------

static int ImplHandleMinMax( HWND hWnd, LPARAM lParam )
{
    int bRet = FALSE;

    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;

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
                int nMinWidth   = pFrame->mnMinWidth;
                int nMinHeight  = pFrame->mnMinHeight;
                int nLeft;
                int nTop;
                int nRight;
                int nBottom;
                ImplSalCalcBorder( pFrame, nLeft, nTop, nRight, nBottom );
                nMinWidth  += nLeft+nRight;
                nMinHeight += nTop+nBottom;
                if ( pMinMax->ptMinTrackSize.x < nMinWidth )
                     pMinMax->ptMinTrackSize.x = nMinWidth;
                if (  pMinMax->ptMinTrackSize.y < nMinHeight )
                     pMinMax->ptMinTrackSize.y = nMinHeight;
            }
        }

        ImplSalYieldMutexRelease();
    }

    return bRet;
}

// -----------------------------------------------------------------------

// retrieves the SalMenuItem pointer from a hMenu
// the pointer is stored in every item, so if no position
// is specified we just use the first item (ie, pos=0)
// if bByPosition is FALSE then nPos denotes a menu id instead of a position
static SalMenuItem* ImplGetSalMenuItem( HMENU hMenu, UINT nPos, BOOL bByPosition=TRUE )
{
    DWORD err=0;

    MENUITEMINFOW mi;
    memset(&mi, 0, sizeof(mi));
    mi.cbSize = sizeof( mi );
    mi.fMask = MIIM_DATA;
    if( !GetMenuItemInfoW( hMenu, nPos, bByPosition, &mi) )
        err = GetLastError();

    return (SalMenuItem *) mi.dwItemData;
}

// returns the index of the currently selected item if any or -1
static int ImplGetSelectedIndex( HMENU hMenu )
{
    DWORD err=0;

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
                err = GetLastError();
            else
            {
                if( mi.fState & MFS_HILITE )
                    return i;
            }
        }
    }
    return -1;
}

static int ImplMenuChar( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    int nRet = MNC_IGNORE;
    HMENU hMenu = (HMENU) lParam;
    String aMnemonic;
    aMnemonic.AssignAscii("&");
    aMnemonic.Append( (sal_Unicode) LOWORD(wParam) );
    aMnemonic.ToLowerAscii();   // we only have ascii mnemonics

    // search the mnemonic in the current menu
    int nItemCount = GetMenuItemCount( hMenu );
    int nFound = 0;
    int idxFound = -1;
    int idxSelected = ImplGetSelectedIndex( hMenu );
    int idx = idxSelected != -1 ? idxSelected+1 : 0;    // if duplicate mnemonics cycle through menu
    for( int i=0; i< nItemCount; i++, idx++ )
    {
        SalMenuItem* pSalMenuItem = ImplGetSalMenuItem( hMenu, idx % nItemCount );
        if( !pSalMenuItem )
            continue;
        String aStr = pSalMenuItem->maData.mText;
        aStr.ToLowerAscii();
        if( aStr.Search( aMnemonic ) != STRING_NOTFOUND)
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
        // duplicate mnemonics, just select the next occurence
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
        MEASUREITEMSTRUCT *pMI = (LPMEASUREITEMSTRUCT) lParam;
        if( pMI->CtlType != ODT_MENU )
            return 0;

        SalMenuItem *pSalMenuItem = (SalMenuItem *) pMI->itemData;
        if( !pSalMenuItem )
            return 0;

        HDC hdc = GetDC( hWnd );
        SIZE strSize;

        NONCLIENTMETRICS ncm;
        memset( &ncm, 0, sizeof(ncm) );
        ncm.cbSize = sizeof( ncm );
        SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, (PVOID) &ncm, 0 );

        // Assume every menu item can be default and printed bold
        //ncm.lfMenuFont.lfWeight = FW_BOLD;

        HFONT hfntOld = (HFONT) SelectObject(hdc, (HFONT) CreateFontIndirect( &ncm.lfMenuFont ));

        // menu text and accelerator
        String aStr(pSalMenuItem->maData.mText.GetBuffer() );
        if( pSalMenuItem->maData.mAccelText.Len() )
        {
            aStr.AppendAscii(" ");
            aStr.Append( pSalMenuItem->maData.mAccelText );
        }
        GetTextExtentPoint32W( hdc, (LPWSTR) aStr.GetBuffer(),
                                aStr.Len(), &strSize );

        // image
        Size bmpSize( 16, 16 );
        //if( !!pSalMenuItem->maData.maBitmap )
        //    bmpSize = pSalMenuItem->maData.maBitmap.GetSizePixel();

        // checkmark
        Size checkSize( GetSystemMetrics( SM_CXMENUCHECK ), GetSystemMetrics( SM_CYMENUCHECK ) );

        pMI->itemWidth = checkSize.Width() + 3 + bmpSize.Width() + 3 + strSize.cx;
        pMI->itemHeight = max( max( checkSize.Height(), bmpSize.Height() ), strSize.cy );
        pMI->itemHeight += 4;

        DeleteObject( SelectObject(hdc, hfntOld) );
        ReleaseDC( hWnd, hdc );
    }

    return nRet;
}

static int ImplDrawItem(HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    int nRet = 0;
    DWORD err = 0;
    if( !wParam )
    {
        // request was sent by a menu
        nRet = 1;
        DRAWITEMSTRUCT *pDI = (LPDRAWITEMSTRUCT) lParam;
        if( pDI->CtlType != ODT_MENU )
            return 0;

        SalMenuItem *pSalMenuItem = (SalMenuItem *) pDI->itemData;
        if( !pSalMenuItem )
            return 0;

        COLORREF clrPrevText, clrPrevBkgnd;
        HFONT hfntOld;
        HBRUSH hbrOld;
        BOOL    fChecked = pDI->itemState & ODS_CHECKED;
        BOOL    fSelected = pDI->itemState & ODS_SELECTED;
        BOOL    fDisabled = pDI->itemState & (ODS_DISABLED | ODS_GRAYED);

        // Set the appropriate foreground and background colors.
        RECT aRect = pDI->rcItem;

        clrPrevBkgnd = SetBkColor( pDI->hDC, GetSysColor( COLOR_MENU ) );

        if ( fDisabled )
            clrPrevText = SetTextColor( pDI->hDC, GetSysColor( COLOR_GRAYTEXT ) );
        else
            clrPrevText = SetTextColor( pDI->hDC, GetSysColor( fSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

        DWORD colBackground = GetSysColor( fSelected ? COLOR_HIGHLIGHT : COLOR_MENU );
        if ( fSelected )
            clrPrevBkgnd = SetBkColor( pDI->hDC, colBackground );
        else
            clrPrevBkgnd = SetBkColor( pDI->hDC, colBackground );

        hbrOld = (HBRUSH)SelectObject( pDI->hDC, CreateSolidBrush( GetBkColor( pDI->hDC ) ) );

        // Fill background
        if(!PatBlt( pDI->hDC, aRect.left, aRect.top, aRect.right-aRect.left, aRect.bottom-aRect.top, PATCOPY ))
            err = GetLastError();

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
            HBITMAP hOldBmp = (HBITMAP) SelectObject( memDC, memBmp );
            DrawFrameControl( memDC, &r, DFC_MENU, DFCS_MENUCHECK );
            BitBlt( pDI->hDC, x, y+(lineHeight-checkHeight)/2, checkWidth, checkHeight, memDC, 0, 0, SRCAND );
            DeleteObject( SelectObject( memDC, hOldBmp ) );
            DeleteDC( memDC );
        }
        x += checkWidth+3;

        //Size bmpSize = aBitmap.GetSizePixel();
        Size bmpSize(16, 16);
        if( !!pSalMenuItem->maData.maBitmap )
        {
            Bitmap aBitmap( pSalMenuItem->maData.maBitmap );

            // set transparent pixels to background color
            if( fDisabled )
                colBackground = RGB(255,255,255);
            aBitmap.Replace( Color( COL_LIGHTMAGENTA ),
                Color( GetRValue(colBackground),GetGValue(colBackground),GetBValue(colBackground) ), 0);

            SalBitmap* pSalBmp = aBitmap.ImplGetImpBitmap()->ImplGetSalBitmap();
            HGLOBAL hDrawDIB = pSalBmp->ImplGethDIB();

            if( hDrawDIB )
            {
                PBITMAPINFO         pBI = (PBITMAPINFO) GlobalLock( hDrawDIB );
                PBITMAPINFOHEADER   pBIH = (PBITMAPINFOHEADER) pBI;
                PBYTE               pBits = (PBYTE) pBI + *(DWORD*) pBI +
                                            pSalBmp->ImplGetDIBColorCount( hDrawDIB ) * sizeof( RGBQUAD );

                HBITMAP hBmp = CreateDIBitmap( pDI->hDC, pBIH, CBM_INIT, pBits, pBI, DIB_RGB_COLORS );
                GlobalUnlock( hDrawDIB );

                HBRUSH hbrIcon = CreateSolidBrush( GetSysColor( COLOR_GRAYTEXT ) );
                DrawStateW( pDI->hDC, (HBRUSH)hbrIcon, (DRAWSTATEPROC)NULL, (LPARAM)hBmp, (WPARAM)0,
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
        SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, (PVOID) &ncm, 0 );

        // Print default menu entry with bold font
        //if ( pDI->itemState & ODS_DEFAULT )
        //    ncm.lfMenuFont.lfWeight = FW_BOLD;

        hfntOld = (HFONT) SelectObject(pDI->hDC, (HFONT) CreateFontIndirect( &ncm.lfMenuFont ));

        SIZE strSize;
        String aStr( pSalMenuItem->maData.mText.GetBuffer() );
        GetTextExtentPoint32W( pDI->hDC, (LPWSTR) aStr.GetBuffer(),
                                aStr.Len(), &strSize );

        if(!DrawStateW( pDI->hDC, (HBRUSH)NULL, (DRAWSTATEPROC)NULL,
            (LPARAM)(LPWSTR) aStr.GetBuffer(),
            (WPARAM)0, aRect.left, aRect.top + (lineHeight - strSize.cy)/2, 0, 0,
            DST_PREFIXTEXT | (fDisabled && !fSelected ? DSS_DISABLED : DSS_NORMAL) ) )
            err = GetLastError();

        if( pSalMenuItem->maData.mAccelText.Len() )
        {
            SIZE strSizeA;
            aStr = pSalMenuItem->maData.mAccelText;
            GetTextExtentPoint32W( pDI->hDC, (LPWSTR) aStr.GetBuffer(),
                                    aStr.Len(), &strSizeA );
            TEXTMETRIC tm;
            GetTextMetrics( pDI->hDC, &tm );

            // position the accelerator string to the right but leave space for the
            // (potential) submenu arrow (tm.tmMaxCharWidth)
            if(!DrawStateW( pDI->hDC, (HBRUSH)NULL, (DRAWSTATEPROC)NULL,
                (LPARAM)(LPWSTR) aStr.GetBuffer(),
                (WPARAM)0, aRect.right-strSizeA.cx-tm.tmMaxCharWidth, aRect.top + (lineHeight - strSizeA.cy)/2, 0, 0,
                DST_TEXT | (fDisabled && !fSelected ? DSS_DISABLED : DSS_NORMAL) ) )
                err = GetLastError();
        }

        // Restore the original font and colors.
        DeleteObject( SelectObject( pDI->hDC, hbrOld ) );
        DeleteObject( SelectObject( pDI->hDC, hfntOld) );
        SetTextColor(pDI->hDC, clrPrevText);
        SetBkColor(pDI->hDC, clrPrevBkgnd);
    }
    return nRet;
}

static int ImplHandleMenuActivate( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    // Menu activation
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    HMENU hMenu = (HMENU) wParam;
    WORD nPos = LOWORD (lParam);
    BOOL bWindowMenu = (BOOL) HIWORD(lParam);

    // Send activate and deactivate together, so we have not keep track of opened menues
    // this will be enough to have the menues updated correctly
    SalMenuEvent aMenuEvt;
    SalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, 0 );
    if( pSalMenuItem )
        aMenuEvt.mpMenu = pSalMenuItem->maData.mpMenu;
    else
        aMenuEvt.mpMenu = NULL;

    long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_MENUACTIVATE, &aMenuEvt );
    if( nRet )
        nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_MENUDEACTIVATE, &aMenuEvt );
    if( nRet )
        pFrame->maFrameData.mLastActivatedhMenu = hMenu;

    return (nRet!=0);
}

static int ImplHandleMenuSelect( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    // Menu selection
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    WORD nId = LOWORD(wParam);      // menu item or submenu index
    WORD nFlags = HIWORD(wParam);
    HMENU hMenu = (HMENU) lParam;
    BOOL bByPosition = FALSE;
    if( nFlags & MF_POPUP )
        bByPosition = TRUE;

    long nRet = 0;
    if ( hMenu && !pFrame->maFrameData.mLastActivatedhMenu )
    {
        // we never activated a menu (ie, no WM_INITMENUPOPUP has occured yet)
        // which means this must be the menubar -> send activation/deactivation
        SalMenuEvent aMenuEvt;
        SalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, nId, bByPosition );
        if( pSalMenuItem )
            aMenuEvt.mpMenu = pSalMenuItem->maData.mpMenu;
        else
            aMenuEvt.mpMenu = NULL;

        nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                SALEVENT_MENUACTIVATE, &aMenuEvt );
        if( nRet )
            nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                               SALEVENT_MENUDEACTIVATE, &aMenuEvt );
        if( nRet )
            pFrame->maFrameData.mLastActivatedhMenu = hMenu;
    }

    if( !hMenu && nFlags == 0xFFFF )
    {
        // all menus are closed, reset activation logic
        pFrame->maFrameData.mLastActivatedhMenu = NULL;
    }

    if( hMenu )
    {
        // hMenu must be saved, as it is not passed in WM_COMMAND which always occurs after a selection
        // if a menu is closed due to a command selection then hMenu is NULL, but WM_COMMAND comes later
        // so we must not overwrite it in this case
        pFrame->maFrameData.mSelectedhMenu = hMenu;

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
                nId = mi.wID;
        }

        SalMenuEvent aMenuEvt;
        aMenuEvt.mnId   = nId;
        SalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, nId, FALSE );
        if( pSalMenuItem )
            aMenuEvt.mpMenu = pSalMenuItem->maData.mpMenu;
        else
            aMenuEvt.mpMenu = NULL;

        nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_MENUHIGHLIGHT, &aMenuEvt );
    }

    return (nRet != 0);
}

static int ImplHandleCommand( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    DWORD err=0;
    long nRet = 0;
    if( !HIWORD(wParam) )
    {
        // Menu command
        WORD nId = LOWORD(wParam);
        if( nId )   // zero for separators
        {
            SalMenuEvent aMenuEvt;
            aMenuEvt.mnId   = nId;
            SalMenuItem *pSalMenuItem = ImplGetSalMenuItem( pFrame->maFrameData.mSelectedhMenu, nId, FALSE );
            if( pSalMenuItem )
                aMenuEvt.mpMenu = pSalMenuItem->maData.mpMenu;
            else
                aMenuEvt.mpMenu = NULL;

            nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                SALEVENT_MENUCOMMAND, &aMenuEvt );
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
        WIN_BOOL    bMaximize = IsZoomed( pFrame->mhWnd );
        WIN_BOOL    bMinimize = IsIconic( pFrame->mhWnd );
        if ( (nCommand == SC_SIZE) ||
             (!bMinimize && (nCommand == SC_MOVE)) ||
             (!bMaximize && (nCommand == SC_MAXIMIZE)) ||
             (bMaximize && (nCommand == SC_RESTORE)) )
        {
            MessageBeep( 0 );
            return TRUE;
        }
    }

    if ( nCommand == SC_KEYMENU )
    {
        // do not process SC_KEYMENU if we have a native menu
        // Windows should handle this
        if( GetMenu( hWnd ) )
            return FALSE;

        // Hier verarbeiten wir nur KeyMenu-Events fuer Alt um
        // den MenuBar zu aktivieren, oder wenn ein SysChild-Fenster
        // den Focus hat, da diese Alt+Tasten-Kombinationen nur
        // ueber diesen Event verarbeitet werden
        if ( !LOWORD( lParam ) )
        {
            // Nur ausloesen, wenn keine weitere Taste gedrueckt ist. Im
            // Gegensatz zur Doku wird in der X-Koordinaate der CharCode
            // geliefert, der zusaetzlich gedrueckt ist
            // Also 32 fuer Space, 99 fuer c, 100 fuer d, ...
            // Da dies nicht dokumentiert ist, fragen wir vorsichtshalber
            // auch den Status der Space-Taste ab
            if ( GetKeyState( VK_SPACE ) & 0x8000 )
                return 0;

            // Damit nicht bei Alt+Maustaste auch der MenuBar aktiviert wird
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
            long nRet = pFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
            pFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
            return (nRet != 0);
        }
        else
        {
            // Testen, ob ein SysChild den Focus hat
            HWND hFocusWnd = ::GetFocus();
            if ( hFocusWnd && ImplFindSalObject( hFocusWnd ) )
            {
                char cKeyCode = (char)(unsigned char)LOWORD( lParam );
                // LowerCase
                if ( (cKeyCode >= 65) && (cKeyCode <= 90) )
                    cKeyCode += 32;
                // Wir nehmen nur 0-9 und A-Z, alle anderen Tasten muessen durch
                // den Hook vom SalObj verarbeitet werden
                if ( ((cKeyCode >= 48) && (cKeyCode <= 57)) ||
                     ((cKeyCode >= 97) && (cKeyCode <= 122)) )
                {
                    USHORT nModCode = 0;
                    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
                        nModCode |= KEY_SHIFT;
                    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
                        nModCode |= KEY_MOD1;
                    nModCode |= KEY_MOD2;
                    if ( !(nModCode & KEY_MOD1) )
                        nModCode |= KEY_CONTROLMOD;

                    SalKeyEvent aKeyEvt;
                    aKeyEvt.mnTime      = GetMessageTime();
                    if ( (cKeyCode >= 48) && (cKeyCode <= 57) )
                        aKeyEvt.mnCode = KEY_0+(cKeyCode-48);
                    else
                        aKeyEvt.mnCode = KEY_A+(cKeyCode-97);
                    aKeyEvt.mnCode     |= nModCode;
                    aKeyEvt.mnCharCode  = cKeyCode;
                    aKeyEvt.mnRepeat    = 0;
                    long nRet = pFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
                    pFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
                    return (nRet != 0);
                }
            }
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static void ImplHandleInputLangChange( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    ImplSalYieldMutexAcquireWithWait();

    // Feststellen, ob wir IME unterstuetzen
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame && pFrame->mbIME && pFrame->mhDefIMEContext )
    {
        HWND    hWnd = pFrame->mhWnd;
        HKL     hKL = (HKL)lParam;
        UINT    nImeProps = ImmGetProperty( hKL, IGP_PROPERTY );

        pFrame->mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
        pFrame->mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
        pFrame->mbHandleIME = !pFrame->mbSpezIME;
    }

    ImplSalYieldMutexRelease();
}

// -----------------------------------------------------------------------

static void ImplUpdateIMECursorPos( WinSalFrame* pFrame, HIMC hIMC )
{
    COMPOSITIONFORM aForm;
    memset( &aForm, 0, sizeof( aForm ) );

    // Cursor-Position ermitteln und aus der die Default-Position fuer
    // das Composition-Fenster berechnen
    SalExtTextInputPosEvent aPosEvt;
    pFrame->CallCallback( SALEVENT_EXTTEXTINPUTPOS, (void*)&aPosEvt );
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
        CreateCaret( pFrame->mhWnd, 0,
                     aPosEvt.mnWidth, aPosEvt.mnHeight );
        SetCaretPos( aPosEvt.mnX, aPosEvt.mnY );
    }
}

// -----------------------------------------------------------------------

static BOOL ImplHandleIMEStartComposition( HWND hWnd )
{
    BOOL bDef = TRUE;

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

// -----------------------------------------------------------------------

static BOOL ImplHandleIMECompositionInput( WinSalFrame* pFrame,
                                           HIMC hIMC, LPARAM lParam )
{
    BOOL bDef = TRUE;

    // Init Event
    SalExtTextInputEvent    aEvt;
    aEvt.mnTime             = GetMessageTime();
    aEvt.mpTextAttr         = NULL;
    aEvt.mnCursorPos        = 0;
    aEvt.mnDeltaStart       = 0;
    aEvt.mbOnlyCursor       = FALSE;
    aEvt.mnCursorFlags      = 0;

    // If we get a result string, then we handle this input
    if ( lParam & GCS_RESULTSTR )
    {
        bDef = FALSE;

        LONG nTextLen = ImmGetCompositionStringW( hIMC, GCS_RESULTSTR, 0, 0 ) / sizeof( WCHAR );
        if ( nTextLen >= 0 )
        {
            WCHAR* pTextBuf = new WCHAR[nTextLen];
            ImmGetCompositionStringW( hIMC, GCS_RESULTSTR, pTextBuf, nTextLen*sizeof( WCHAR ) );
            aEvt.maText = XubString( pTextBuf, (xub_StrLen)nTextLen );
            delete pTextBuf;
        }

        aEvt.mnCursorPos = aEvt.maText.Len();
        pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
        pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
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

        USHORT* pSalAttrAry = NULL;
        LONG    nTextLen = ImmGetCompositionStringW( hIMC, GCS_COMPSTR, 0, 0 ) / sizeof( WCHAR );
        if ( nTextLen > 0 )
        {
            WCHAR* pTextBuf = new WCHAR[nTextLen];
            ImmGetCompositionStringW( hIMC, GCS_COMPSTR, pTextBuf, nTextLen*sizeof( WCHAR ) );
            aEvt.maText = XubString( pTextBuf, (xub_StrLen)nTextLen );
            delete pTextBuf;

            WIN_BYTE*   pAttrBuf = NULL;
            LONG        nAttrLen = ImmGetCompositionStringW( hIMC, GCS_COMPATTR, 0, 0 );
            if ( nAttrLen > 0 )
            {
                pAttrBuf = new WIN_BYTE[nAttrLen];
                ImmGetCompositionStringW( hIMC, GCS_COMPATTR, pAttrBuf, nAttrLen );
            }

            if ( pAttrBuf )
            {
                xub_StrLen nTextLen = aEvt.maText.Len();
                pSalAttrAry = new USHORT[nTextLen];
                memset( pSalAttrAry, 0, nTextLen*sizeof( USHORT ) );
                for ( xub_StrLen i = 0; (i < nTextLen) && (i < nAttrLen); i++ )
                {
                    WIN_BYTE nWinAttr = pAttrBuf[i];
                    USHORT   nSalAttr;
                    if ( nWinAttr == ATTR_TARGET_CONVERTED )
                    {
                        nSalAttr = SAL_EXTTEXTINPUT_ATTR_BOLDUNDERLINE;
                        aEvt.mnCursorFlags |= SAL_EXTTEXTINPUT_CURSOR_INVISIBLE;
                    }
                    else if ( nWinAttr == ATTR_CONVERTED )
                        nSalAttr = SAL_EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE;
                    else if ( nWinAttr == ATTR_TARGET_NOTCONVERTED )
                        nSalAttr = SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
                    else if ( nWinAttr == ATTR_INPUT_ERROR )
                        nSalAttr = SAL_EXTTEXTINPUT_ATTR_REDTEXT | SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                    else /* ( nWinAttr == ATTR_INPUT ) */
                        nSalAttr = SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                    pSalAttrAry[i] = nSalAttr;
                }

                aEvt.mpTextAttr = pSalAttrAry;
                delete pAttrBuf;
            }
        }

        // Only when we get new composition data, we must send this event
        if ( (nTextLen > 0) || !(lParam & GCS_RESULTSTR) )
        {
            // End the mode, if the last character is deleted
            if ( !nTextLen && !pFrame->mbCandidateMode )
            {
                pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
                pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
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
                    aEvt.mnCursorPos = LOWORD( ImmGetCompositionStringW( hIMC, GCS_CURSORPOS, 0, 0 ) );

                if ( pFrame->mbCandidateMode )
                    aEvt.mnCursorFlags |= SAL_EXTTEXTINPUT_CURSOR_INVISIBLE;
                if ( lParam & CS_NOMOVECARET )
                    aEvt.mnCursorFlags |= SAL_EXTTEXTINPUT_CURSOR_OVERWRITE;

                pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
            }
            ImplUpdateIMECursorPos( pFrame, hIMC );
        }

        if ( pSalAttrAry )
            delete pSalAttrAry;
    }

    return !bDef;
}

// -----------------------------------------------------------------------

static BOOL ImplHandleIMEComposition( HWND hWnd, LPARAM lParam )
{
    BOOL bDef = TRUE;
    ImplSalYieldMutexAcquireWithWait();

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame && (!lParam || (lParam & GCS_RESULTSTR)) )
    {
        // Wir restaurieren den Background-Modus bei jeder Texteingabe,
        // da einige Tools wie RichWin uns diesen hin- und wieder umsetzen
        if ( pFrame->mpGraphics &&
             pFrame->mpGraphics->mhDC )
            SetBkMode( pFrame->mpGraphics->mhDC, TRANSPARENT );
    }

    if ( pFrame && pFrame->mbHandleIME )
    {
        if ( !lParam )
        {
            SalExtTextInputEvent aEvt;
            aEvt.mnTime             = GetMessageTime();
            aEvt.mpTextAttr         = NULL;
            aEvt.mnCursorPos        = 0;
            aEvt.mnDeltaStart       = 0;
            aEvt.mbOnlyCursor       = FALSE;
            aEvt.mnCursorFlags      = 0;
            pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
            pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
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

// -----------------------------------------------------------------------

static BOOL ImplHandleIMEEndComposition( HWND hWnd )
{
    BOOL bDef = TRUE;

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

// -----------------------------------------------------------------------

static void ImplHandleIMENotify( HWND hWnd, WPARAM wParam )
{
    if ( wParam == (WPARAM)IMN_OPENCANDIDATE )
    {
        ImplSalYieldMutexAcquireWithWait();

        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && pFrame->mbHandleIME &&
             pFrame->mbAtCursorIME )
        {
            // Wir wollen den Cursor hiden
            pFrame->mbCandidateMode = TRUE;
            ImplHandleIMEComposition( hWnd, GCS_CURSORPOS );

            HWND hWnd = pFrame->mhWnd;
            HIMC hIMC = ImmGetContext( hWnd );
            if ( hIMC )
            {
                LONG nBufLen = ImmGetCompositionStringW( hIMC, GCS_COMPSTR, 0, 0 );
                if ( nBufLen >= 1 )
                {
                    SalExtTextInputPosEvent aPosEvt;
                    pFrame->CallCallback( SALEVENT_EXTTEXTINPUTPOS, (void*)&aPosEvt );

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

                ImmReleaseContext( hWnd, hIMC );
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

// -----------------------------------------------------------------------

void SalTestMouseLeave()
{
    SalData* pSalData = GetSalData();

    if ( pSalData->mhWantLeaveMsg && !::GetCapture() )
    {
        POINT aPt;
        GetCursorPos( &aPt );
        if ( pSalData->mhWantLeaveMsg != WindowFromPoint( aPt ) )
            ImplSendMessage( pSalData->mhWantLeaveMsg, SAL_MSG_MOUSELEAVE, 0, MAKELPARAM( aPt.x, aPt.y ) );
    }
}

// -----------------------------------------------------------------------

static int ImplSalWheelMousePos( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam ,
                                 LRESULT& rResult )
{
    POINT aPt;
    POINT aScreenPt;
    aScreenPt.x = (short)LOWORD( lParam );
    aScreenPt.y = (short)HIWORD( lParam );
    // Child-Fenster suchen, welches an der entsprechenden
    // Position liegt
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
        rResult = ImplSendMessage( hWheelWnd, nMsg, wParam, lParam );
        return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

LRESULT CALLBACK SalFrameWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT     nRet = 0;
    static int  bInWheelMsg = FALSE;
    static int  bInQueryEnd = FALSE;

    // By WM_CRETAE we connect the frame with the window handle
    if ( nMsg == WM_CREATE )
    {
        // Window-Instanz am Windowhandle speichern
        // Can also be used for the W-Version, because the struct
        // to access lpCreateParams is the same structure
        CREATESTRUCTA* pStruct = (CREATESTRUCTA*)lParam;
        WinSalFrame* pFrame = (WinSalFrame*)pStruct->lpCreateParams;
        SetWindowPtr( hWnd, pFrame );
        // HWND schon hier setzen, da schon auf den Instanzdaten
        // gearbeitet werden kann, wenn Messages waehrend
        // CreateWindow() gesendet werden
        pFrame->mhWnd = hWnd;
        pFrame->maSysData.hWnd = hWnd;
        return 0;
    }

    if ( WM_USER_SYSTEM_WINDOW_ACTIVATED == nMsg )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->mpIntroWindow)
            pSVData->mpIntroWindow->Hide();

        return 0;
    }

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
            rDef = !ImplHandleMouseMsg( hWnd, nMsg, wParam, lParam );
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
            rDef = !ImplHandleKeyMsg( hWnd, nMsg, wParam, lParam, nRet );
            ImplSalYieldMutexRelease();
            break;

        case WM_MOUSEWHEEL:
            // Gegen Rekursion absichern, falls wir vom IE oder dem externen
            // Fenster die Message wieder zurueckbekommen
            if ( !bInWheelMsg )
            {
                bInWheelMsg++;
                rDef = !ImplHandleWheelMsg( hWnd, wParam, lParam );
                // Wenn wir die Message nicht ausgewertet haben, schauen wir
                // noch einmal nach, ob dort ein geplugtes Fenster steht,
                // welches wir dann benachrichtigen
                if ( rDef )
                    rDef = ImplSalWheelMousePos( hWnd, nMsg, wParam, lParam, nRet );
                bInWheelMsg--;
            }
            break;

        case WM_COMMAND:
            ImplSalYieldMutexAcquireWithWait();
            rDef = !ImplHandleCommand( hWnd, wParam, lParam );
            ImplSalYieldMutexRelease();
            break;

        case WM_INITMENUPOPUP:
            ImplSalYieldMutexAcquireWithWait();
            rDef = !ImplHandleMenuActivate( hWnd, wParam, lParam );
            ImplSalYieldMutexRelease();
            break;

        case WM_MENUSELECT:
            ImplSalYieldMutexAcquireWithWait();
            rDef = !ImplHandleMenuSelect( hWnd, wParam, lParam );
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
            ImplHandlePaintMsg( hWnd );
            rDef = FALSE;
            break;
        case SAL_MSG_POSTPAINT:
            ImplHandlePaintMsg2( hWnd, (RECT*)wParam );
            rDef = FALSE;
            break;

        case SAL_MSG_FORCEPALETTE:
            ImplHandleForcePalette( hWnd );
            rDef = FALSE;
            break;

        case WM_QUERYNEWPALETTE:
        case SAL_MSG_POSTQUERYNEWPAL:
            nRet = ImplHandlePalette( TRUE, hWnd, nMsg, wParam, lParam, rDef );
            break;

        case WM_ACTIVATE:
            // Wenn wir aktiviert werden, dann wollen wir auch unsere
            // Palette setzen. Wir machen dieses in Activate,
            // damit andere externe Child-Fenster auch unsere Palette
            // ueberschreiben koennen. So wird unsere jedenfalls nur einmal
            // gesetzt und nicht immer rekursiv, da an allen anderen Stellen
            // diese nur als Background-Palette gesetzt wird
            if ( LOWORD( wParam ) != WA_INACTIVE )
                ImplSendMessage( hWnd, SAL_MSG_FORCEPALETTE, 0, 0 );
            break;

        case WM_ENABLE:
            // #95133# a system dialog is opened/closed, using our app window as parent
            {
                WinSalFrame* pFrame = GetWindowPtr( hWnd );
                Window *pWin = NULL;
                if( pFrame )
                    pWin = ((Window*)pFrame->GetInstance());

                if( !wParam )
                {
                    ImplSVData* pSVData = ImplGetSVData();
                    pSVData->maAppData.mnModalMode++;

                    // #106431#, hide SplashScreen
                    if( pSVData->mpIntroWindow )
                        pSVData->mpIntroWindow->Hide();

                    if( pWin )
                    {
                        pWin->EnableInput( FALSE, TRUE, TRUE, NULL );
                        pWin->ImplIncModalCount();  // #106303# support frame based modal count
                    }
                }
                else
                {
                    ImplGetSVData()->maAppData.mnModalMode--;
                    if( pWin )
                    {
                        pWin->EnableInput( TRUE, TRUE, TRUE, NULL );
                        pWin->ImplDecModalCount();  // #106303# support frame based modal count
                    }
                }
            }
            break;

        case WM_KILLFOCUS:
            DestroyCaret();
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
                nRet = !ImplHandleShutDownMsg( hWnd );
                rDef = FALSE;

                // Issue #16314#: ImplHandleShutDownMsg causes a PostMessage in case of allowing shutdown.
                // This posted message was never processed and cause Windows XP to hang after log off
                // if there are multiple sessions and the current session wasn't the first one started.
                // So if shutdown is allowed we assume that a post message was done and retrieve all
                // messages in the message queue and dispatch them before we return control to the system.

                if ( nRet )
                {
                    MSG msg;

                    while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
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

#ifdef WM_DISPLAYCHANGE
        case WM_DISPLAYCHANGE:
#endif
#ifdef WM_SETTINGCHANGE
        case WM_SETTINGCHANGE:
#else
        case WM_WININICHANGE:
#endif
        case WM_DEVMODECHANGE:
        case WM_FONTCHANGE:
        case WM_SYSCOLORCHANGE:
        case WM_TIMECHANGE:
            ImplHandleSettingsChangeMsg( hWnd, nMsg, wParam, lParam );
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
            ImplSalToTop( hWnd, (USHORT)wParam );
            rDef = FALSE;
            break;
        case SAL_MSG_SHOW:
            ImplSalShow( hWnd, (BOOL)wParam, (BOOL)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_SETINPUTCONTEXT:
            ImplSalFrameSetInputContext( hWnd, (const SalInputContext*)(void*)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_ENDEXTTEXTINPUT:
            ImplSalFrameEndExtTextInput( hWnd, (USHORT)(ULONG)(void*)wParam );
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
            rDef = !ImplHandleKeyMsg( hWnd, WM_CHAR, wParam, lParam, nRet );
            ImplSalYieldMutexRelease();
            break;

         case WM_IME_STARTCOMPOSITION:
            rDef = ImplHandleIMEStartComposition( hWnd );
            break;

        case WM_IME_COMPOSITION:
            rDef = ImplHandleIMEComposition( hWnd, lParam );
            break;

        case WM_IME_ENDCOMPOSITION:
            rDef = ImplHandleIMEEndComposition( hWnd );
            break;

        case WM_IME_NOTIFY:
            ImplHandleIMENotify( hWnd, wParam );
            break;
    }

    // WheelMouse-Message abfangen
    if ( rDef && (nMsg == aSalShlData.mnWheelMsgId) && aSalShlData.mnWheelMsgId )
    {
        // Gegen Rekursion absichern, falls wir vom IE oder dem externen
        // Fenster die Message wieder zurueckbekommen
        if ( !bInWheelMsg )
        {
            bInWheelMsg++;
            // Zuerst wollen wir die Message dispatchen und dann darf auch
            // das SystemWindow drankommen
            WORD nKeyState = 0;
            if ( GetKeyState( VK_SHIFT ) & 0x8000 )
                nKeyState |= MK_SHIFT;
            if ( GetKeyState( VK_CONTROL ) & 0x8000 )
                nKeyState |= MK_CONTROL;
            // Mutex handling is inside from this call
            rDef = !ImplHandleWheelMsg( hWnd, MAKEWPARAM( nKeyState, (WORD)wParam ), lParam );
            if ( rDef )
            {
                HWND hWheelWnd = ::GetFocus();
                if ( hWheelWnd && (hWheelWnd != hWnd) )
                {
                    nRet = ImplSendMessage( hWheelWnd, nMsg, wParam, lParam );
                    rDef = FALSE;
                }
                else
                    rDef = ImplSalWheelMousePos( hWnd, nMsg, wParam, lParam, nRet );
            }
            bInWheelMsg--;
        }
    }

    return nRet;
}

LRESULT CALLBACK SalFrameWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalFrameWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
        nRet = DefWindowProcA( hWnd, nMsg, wParam, lParam );
    return nRet;
}

LRESULT CALLBACK SalFrameWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalFrameWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
        nRet = DefWindowProcW( hWnd, nMsg, wParam, lParam );
    return nRet;
}

// -----------------------------------------------------------------------

BOOL ImplHandleGlobalMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& rlResult )
{
    // Hier verarbeiten wir alle Messages, die fuer alle Frame-Fenster gelten,
    // damit diese nur einmal verarbeitet werden
    // Must work for Unicode and none Unicode
    if ( (nMsg == WM_PALETTECHANGED) || (nMsg == SAL_MSG_POSTPALCHANGED) )
    {
        int bDef = TRUE;
        rlResult = ImplHandlePalette( FALSE, hWnd, nMsg, wParam, lParam, bDef );
        return (bDef != 0);
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------
