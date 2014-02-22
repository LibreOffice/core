/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/misccfg.hxx>

#include <officecfg/Office/Common.hxx>

#include <string.h>
#include <limits.h>

#include <stdio.h>

#include <svsys.h>

#include <rtl/string.h>
#include <rtl/ustring.h>

#include <osl/module.h>

#include <tools/debug.hxx>

#include <vcl/sysdata.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>


#ifdef _MSC_VER
#pragma warning( disable: 4242 4244 )
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
#include <multimon.h>
#include <vector>
#ifdef __MINGW32__
#include <algorithm>
using ::std::max;
#endif

#include <com/sun/star/uno/Exception.hpp>

#include <oleacc.h>
#include <com/sun/star/accessibility/XMSAAService.hpp>
#ifndef WM_GETOBJECT 
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



const unsigned int WM_USER_SYSTEM_WINDOW_ACTIVATED = RegisterWindowMessageA("SYSTEM_WINDOW_ACTIVATED");

sal_Bool WinSalFrame::mbInReparent = FALSE;





#define Uni_UTF32ToSurrogate2(ch)   (((unsigned long) (ch) - 0x10000) % 0x400 + 0xDC00)
#define Uni_SupplementaryPlanesStart    0x10000



static void UpdateFrameGeometry( HWND hWnd, WinSalFrame* pFrame );
static void SetMaximizedFrameGeometry( HWND hWnd, WinSalFrame* pFrame, RECT* pParentRect = NULL );

static void ImplSaveFrameState( WinSalFrame* pFrame )
{
    
    if ( !pFrame->mbFullScreen )
    {
        sal_Bool bVisible = (GetWindowStyle( pFrame->mhWnd ) & WS_VISIBLE) != 0;
        if ( IsIconic( pFrame->mhWnd ) )
        {
            pFrame->maState.mnState |= WINDOWSTATE_STATE_MINIMIZED;
            if ( bVisible )
                pFrame->mnShowState = SW_SHOWMAXIMIZED;
        }
        else if ( IsZoomed( pFrame->mhWnd ) )
        {
            pFrame->maState.mnState &= ~WINDOWSTATE_STATE_MINIMIZED;
            pFrame->maState.mnState |= WINDOWSTATE_STATE_MAXIMIZED;
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

            
            RECT aRect2 = aRect;
            AdjustWindowRectEx( &aRect2, GetWindowStyle( pFrame->mhWnd ),
                            FALSE,     GetWindowExStyle( pFrame->mhWnd ) );
            long nTopDeco = abs( aRect.top - aRect2.top );
            long nLeftDeco = abs( aRect.left - aRect2.left );
            long nBottomDeco = abs( aRect.bottom - aRect2.bottom );
            long nRightDeco = abs( aRect.right - aRect2.right );

            pFrame->maState.mnState &= ~(WINDOWSTATE_STATE_MINIMIZED | WINDOWSTATE_STATE_MAXIMIZED);
            
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




void ImplSalGetWorkArea( HWND hWnd, RECT *pRect, const RECT *pParentRect )
{
    
    bool bIgnoreTaskbar = false;
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if( pFrame )
    {
        Window *pWin = pFrame->GetWindow();
        while( pWin )
        {
            WorkWindow *pWorkWin = (pWin->GetType() == WINDOW_WORKWINDOW) ? (WorkWindow *) pWin : NULL;
            if( pWorkWin && pWorkWin->ImplGetWindowImpl()->mbReallyVisible && pWorkWin->IsFullScreenMode() )
            {
                bIgnoreTaskbar = true;
                break;
            }
            else
                pWin = pWin->ImplGetWindowImpl()->mpParent;
        }
    }

    
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
            
            HMONITOR hMonitor;
            MONITORINFO mi;

            
            hMonitor = MonitorFromRect(pParentRect, MONITOR_DEFAULTTONEAREST);

            
            mi.cbSize = sizeof(mi);
            GetMonitorInfo(hMonitor, &mi);
            if( !bIgnoreTaskbar )
                *pRect = mi.rcWork;
            else
                *pRect = mi.rcMonitor;
        }
        else
        {
            
            pRect->left = GetSystemMetrics( SM_XVIRTUALSCREEN );
            pRect->top = GetSystemMetrics( SM_YVIRTUALSCREEN );
            pRect->right = pRect->left + GetSystemMetrics( SM_CXVIRTUALSCREEN );
            pRect->bottom = pRect->top + GetSystemMetrics( SM_CYVIRTUALSCREEN );

            
            
            
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
                              HWND hWndParent, sal_uLong nSalFrameStyle )
{
    WinSalFrame*   pFrame = new WinSalFrame;
    HWND        hWnd;
    DWORD       nSysStyle = 0;
    DWORD       nExSysStyle = 0;
    sal_Bool        bSubFrame = FALSE;

    static const char* pEnvSynchronize = getenv("SAL_SYNCHRONIZE");
    if ( pEnvSynchronize )   
        GdiSetBatchLimit( 1 );

    static const char* pEnvTransparentFloats = getenv("SAL_TRANSPARENT_FLOATS" );

    
    if ( nSalFrameStyle & (SAL_FRAME_STYLE_PLUG | SAL_FRAME_STYLE_SYSTEMCHILD) )
    {
        nSysStyle |= WS_CHILD;
        if( nSalFrameStyle & SAL_FRAME_STYLE_SYSTEMCHILD )
            nSysStyle |= WS_CLIPSIBLINGS;
    }
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
            
            if ( (nSalFrameStyle & (SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_MOVEABLE)) ==
                 (SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_MOVEABLE) )
                nSysStyle |= WS_OVERLAPPED;
            else
            {
                nSysStyle |= WS_POPUP;
                if ( !(nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE) )
                    nExSysStyle |= WS_EX_TOOLWINDOW;    
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
            
            
            
            

            
            /* && !(nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE) */ )
        {
            pFrame->mbNoIcon = TRUE;
            nExSysStyle |= WS_EX_TOOLWINDOW;
            if ( pEnvTransparentFloats /*&& !(nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE) */)
                nExSysStyle |= WS_EX_LAYERED;
        }
    }
    if ( nSalFrameStyle & SAL_FRAME_STYLE_FLOAT )
    {
        nExSysStyle |= WS_EX_TOOLWINDOW;
        pFrame->mbFloatWin = TRUE;

        if ( (pEnvTransparentFloats /* does not work remote! || (nSalFrameStyle & SAL_FRAME_STYLE_FLOAT_FOCUSABLE) */ )  )
            nExSysStyle |= WS_EX_LAYERED;

    }
    if( (nSalFrameStyle & SAL_FRAME_STYLE_TOOLTIP) || (nSalFrameStyle & SAL_FRAME_STYLE_FLOAT_FOCUSABLE) )
        nExSysStyle |= WS_EX_TOPMOST;

    
    pFrame->mnStyle = nSalFrameStyle;

    
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
                
                
                HWND hWnd2 = GetForegroundWindow();
                if ( hWnd2 && IsMaximized( hWnd2 ) &&
                     (GetWindowInstance( hWnd2 ) == pInst->mhInst) &&
                     ((GetWindowStyle( hWnd2 ) & (WS_POPUP | WS_MAXIMIZEBOX | WS_THICKFRAME)) == (WS_MAXIMIZEBOX | WS_THICKFRAME)) )
                    pFrame->mnShowState = SW_SHOWMAXIMIZED;
            }
        }
    }

    
    LPCWSTR pClassName;
    if ( bSubFrame )
    {
        if ( nSalFrameStyle & (SAL_FRAME_STYLE_MOVEABLE|SAL_FRAME_STYLE_NOSHADOW) ) 
            pClassName = SAL_SUBFRAME_CLASSNAMEW;
        else
            pClassName = SAL_TMPSUBFRAME_CLASSNAMEW;    
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
    if( !hWnd )
        ImplWriteLastError( GetLastError(), "CreateWindowEx" );
#if OSL_DEBUG_LEVEL > 1
    
    if( GetWindowExStyle( hWnd ) & WS_EX_LAYERED )
        SetLayeredWindowAttributes( hWnd, 0, 230, 0x00000002 /*LWA_ALPHA*/ );
#endif
    if ( !hWnd )
    {
        delete pFrame;
        return NULL;
    }

    
    
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

    
    pFrame->mhDefIMEContext = ImmAssociateContext( hWnd, 0 );

    
    RECT aRect;
    GetClientRect( hWnd, &aRect );
    pFrame->mnWidth  = aRect.right;
    pFrame->mnHeight = aRect.bottom;
    ImplSaveFrameState( pFrame );
    pFrame->mbDefPos = TRUE;

    UpdateFrameGeometry( hWnd, pFrame );

    if( pFrame->mnShowState == SW_SHOWMAXIMIZED )
    {
        
        

        SetMaximizedFrameGeometry( hWnd, pFrame );
    }

    return pFrame;
}



HWND ImplSalReCreateHWND( HWND hWndParent, HWND oldhWnd, sal_Bool bAsChild )
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
                            hWndParent, 0, hInstance, (void*)GetWindowPtr( oldhWnd ) );
}




#define KEY_TAB_SIZE     146

static const sal_uInt16 aImplTranslateKeyTab[KEY_TAB_SIZE] =
{
    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    KEY_BACKSPACE,        
    KEY_TAB,              
    0,                    
    0,                    
    0,                    
    KEY_RETURN,           
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    KEY_HANGUL_HANJA,     
    0,                    
    KEY_ESCAPE,           
    0,                    
    0,                    
    0,                    
    0,                    
    KEY_SPACE,            
    KEY_PAGEUP,           
    KEY_PAGEDOWN,         
    KEY_END,              
    KEY_HOME,             
    KEY_LEFT,             
    KEY_UP,               
    KEY_RIGHT,            
    KEY_DOWN,             
    0,                    
    0,                    
    0,                    
    0,                    
    KEY_INSERT,           
    KEY_DELETE,           
    KEY_HELP,             
    KEY_0,                
    KEY_1,                
    KEY_2,                
    KEY_3,                
    KEY_4,                
    KEY_5,                
    KEY_6,                
    KEY_7,                
    KEY_8,                
    KEY_9,                
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    KEY_A,                
    KEY_B,                
    KEY_C,                
    KEY_D,                
    KEY_E,                
    KEY_F,                
    KEY_G,                
    KEY_H,                
    KEY_I,                
    KEY_J,                
    KEY_K,                
    KEY_L,                
    KEY_M,                
    KEY_N,                
    KEY_O,                
    KEY_P,                
    KEY_Q,                
    KEY_R,                
    KEY_S,                
    KEY_T,                
    KEY_U,                
    KEY_V,                
    KEY_W,                
    KEY_X,                
    KEY_Y,                
    KEY_Z,                
    0,                    
    0,                    
    KEY_CONTEXTMENU,      
    0,                    
    0,                    
    KEY_0,                
    KEY_1,                
    KEY_2,                
    KEY_3,                
    KEY_4,                
    KEY_5,                
    KEY_6,                
    KEY_7,                
    KEY_8,                
    KEY_9,                
    KEY_MULTIPLY,         
    KEY_ADD,              
    KEY_DECIMAL,          
    KEY_SUBTRACT,         
    KEY_DECIMAL,          
    KEY_DIVIDE,           
    KEY_F1,               
    KEY_F2,               
    KEY_F3,               
    KEY_F4,               
    KEY_F5,               
    KEY_F6,               
    KEY_F7,               
    KEY_F8,               
    KEY_F9,               
    KEY_F10,              
    KEY_F11,              
    KEY_F12,              
    KEY_F13,              
    KEY_F14,              
    KEY_F15,              
    KEY_F16,              
    KEY_F17,              
    KEY_F18,              
    KEY_F19,              
    KEY_F20,              
    KEY_F21,              
    KEY_F22,              
    KEY_F23,              
    KEY_F24,              
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0,                    
    0                     
};



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



static UINT ImplSalGetWheelScrollChars()
{
    UINT nScrChars = 0;
    if( !SystemParametersInfo( SPI_GETWHEELSCROLLCHARS, 0, &nScrChars, 0 ) )
    {
        
        
        if( aSalShlData.maVersionInfo.dwMajorVersion < 6 )
        {
            
            
            return 1;
        }
        else
        {
            
            return 3;
        }
    }

    
    return nScrChars;
}



static void ImplSalAddBorder( const WinSalFrame* pFrame, int& width, int& height )
{
    
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
            nScreenDX = aRect.getWidth()+1;  
            nScreenDY = aRect.getHeight()+1; 
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



static void ImplSalFrameFullScreenPos( WinSalFrame* pFrame, sal_Bool bAlways = FALSE )
{
    if ( bAlways || !IsIconic( pFrame->mhWnd ) )
    {
        
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



WinSalFrame::WinSalFrame()
{
    SalData* pSalData = GetSalData();

    mhWnd               = 0;
    mhCursor            = LoadCursor( 0, IDC_ARROW );
    mhDefIMEContext     = 0;
    mpGraphics          = NULL;
    mpGraphics2         = NULL;
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
    mbFullScreen        = FALSE;
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
    mSelectedhMenu      = 0;
    mLastActivatedhMenu = 0;
    mpClipRgnData       = NULL;
    mbFirstClipRect     = TRUE;
    mpNextClipRect      = NULL;
    mnDisplay           = 0;
    mbPropertiesStored  = FALSE;

    memset( &maState, 0, sizeof( SalFrameState ) );
    maSysData.nSize     = sizeof( SystemEnvData );

    memset( &maGeometry, 0, sizeof( maGeometry ) );

    
    if ( !pSalData->mpFirstFrame )
    {
        if ( !aSalShlData.mnWheelMsgId )
            aSalShlData.mnWheelMsgId = RegisterWindowMessage( MSH_MOUSEWHEEL );
        if ( !aSalShlData.mnWheelScrollLines )
            aSalShlData.mnWheelScrollLines = ImplSalGetWheelScrollLines();
        if ( !aSalShlData.mnWheelScrollChars )
            aSalShlData.mnWheelScrollChars = ImplSalGetWheelScrollChars();
    }

    
    mpNextFrame = pSalData->mpFirstFrame;
    pSalData->mpFirstFrame = this;
}


void WinSalFrame::updateScreenNumber()
{
    if( mnDisplay == -1 ) 
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
        delete [] (BYTE*)mpClipRgnData;

    
    WinSalFrame** ppFrame = &pSalData->mpFirstFrame;
    for(; (*ppFrame != this) && *ppFrame; ppFrame = &(*ppFrame)->mpNextFrame );
    if( *ppFrame )
        *ppFrame = mpNextFrame;
    mpNextFrame = NULL;

    
    if ( mpGraphics2 &&
         mpGraphics2->getHDC() )
        ReleaseGraphics( mpGraphics2 );

    
    if ( mpGraphics )
    {
        if ( mpGraphics->mhDefPal )
            SelectPalette( mpGraphics->getHDC(), mpGraphics->mhDefPal, TRUE );
        ImplSalDeInitGraphics( mpGraphics );
        ReleaseDC( mhWnd, mpGraphics->getHDC() );
        delete mpGraphics;
        mpGraphics = NULL;
    }

    if ( mhWnd )
    {
        
        if ( pSalData->mhWantLeaveMsg == mhWnd )
        {
            pSalData->mhWantLeaveMsg = 0;
            if ( pSalData->mpMouseLeaveTimer )
            {
                delete pSalData->mpMouseLeaveTimer;
                pSalData->mpMouseLeaveTimer = NULL;
            }
        }

        
        if ( mbPropertiesStored )
            SetApplicationID( OUString() );

        
        if ( !DestroyWindow( mhWnd ) )
            SetWindowPtr( mhWnd, 0 );

        mhWnd = 0;
    }
}



SalGraphics* WinSalFrame::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    
    
    
    SalData* pSalData = GetSalData();
    if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
    {
        
        
        if ( pSalData->mnCacheDCInUse >= 3 )
            return NULL;

        if ( !mpGraphics2 )
        {
            mpGraphics2 = new WinSalGraphics;
            mpGraphics2->setHDC(0);
            mpGraphics2->mhWnd       = mhWnd;
            mpGraphics2->mbPrinter   = FALSE;
            mpGraphics2->mbVirDev    = FALSE;
            mpGraphics2->mbWindow    = TRUE;
            mpGraphics2->mbScreen    = TRUE;
        }

        HDC hDC = (HDC)(sal_IntPtr)ImplSendMessage( pSalData->mpFirstInstance->mhComWnd,
                                        SAL_MSG_GETDC,
                                        (WPARAM)mhWnd, 0 );
        if ( hDC )
        {
            mpGraphics2->setHDC(hDC);
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
                mpGraphics->setHDC(hDC);
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



void WinSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    if ( mpGraphics2 == pGraphics )
    {
        if ( mpGraphics2->getHDC() )
        {
            SalData* pSalData = GetSalData();
            if ( mpGraphics2->mhDefPal )
                SelectPalette( mpGraphics2->getHDC(), mpGraphics2->mhDefPal, TRUE );
            ImplSalDeInitGraphics( mpGraphics2 );
            ImplSendMessage( pSalData->mpFirstInstance->mhComWnd,
                             SAL_MSG_RELEASEDC,
                             (WPARAM)mhWnd,
                             (LPARAM)mpGraphics2->getHDC() );
            mpGraphics2->setHDC(0);
            pSalData->mnCacheDCInUse--;
        }
    }

    mbGraphics = FALSE;
}



bool WinSalFrame::PostEvent( void* pData )
{
    return (sal_Bool)ImplPostMessage( mhWnd, SAL_MSG_USEREVENT, 0, (LPARAM)pData );
}



void WinSalFrame::SetTitle( const OUString& rTitle )
{
    DBG_ASSERT( sizeof( WCHAR ) == sizeof( sal_Unicode ), "WinSalFrame::SetTitle(): WCHAR != sal_Unicode" );

    SetWindowTextW( mhWnd, reinterpret_cast<LPCWSTR>(rTitle.getStr()) );
}



void WinSalFrame::SetIcon( sal_uInt16 nIcon )
{
    
    if ( mbNoIcon )
        return;

    
    HICON hIcon = NULL, hSmIcon = NULL;
    if ( !nIcon )
        nIcon = 1;

    ImplLoadSalIcon( nIcon, hIcon, hSmIcon );

    DBG_ASSERT( hIcon ,   "WinSalFrame::SetIcon(): Could not load large icon !" );
    DBG_ASSERT( hSmIcon , "WinSalFrame::SetIcon(): Could not load small icon !" );

    ImplSendMessage( mhWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon );
    ImplSendMessage( mhWnd, WM_SETICON, ICON_SMALL, (LPARAM)hSmIcon );
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
    Window *pRealParent = pFrame->GetWindow()->ImplGetWindowImpl()->mpRealParent;
    if( pRealParent )
        return static_cast<WinSalFrame*>(pRealParent->ImplGetWindowImpl()->mpFrame)->mhWnd;
    else
        return ::GetParent( hWnd );

}



SalFrame* WinSalFrame::GetParent() const
{
    return GetWindowPtr( ImplGetParentHwnd( mhWnd ) );
}



static void ImplSalShow( HWND hWnd, sal_Bool bVisible, sal_Bool bNoActivate )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return;

    if ( bVisible )
    {
        pFrame->mbDefPos = FALSE;
        pFrame->mbOverwriteState = TRUE;
        pFrame->mbInShow = TRUE;

        
        RECT aRectPreMatrox, aRectPostMatrox;
        GetWindowRect( hWnd, &aRectPreMatrox );

        vcl::DeletionListener aDogTag( pFrame );
        if( bNoActivate )
            ShowWindow( hWnd, SW_SHOWNOACTIVATE );
        else
            ShowWindow( hWnd, pFrame->mnShowState );
        if( aDogTag.isDeleted() )
            return;

        if ( aSalShlData.mbWXP && pFrame->mbFloatWin && !(pFrame->mnStyle & SAL_FRAME_STYLE_NOSHADOW))
        {
            
            
            
            HDC dc = GetDC( hWnd );
            RECT aRect;
            GetClientRect( hWnd, &aRect );
            FillRect( dc, &aRect, (HBRUSH) (COLOR_MENU+1) ); 
            ReleaseDC( hWnd, dc );
        }

        
        
        GetWindowRect( hWnd, &aRectPostMatrox );
        if( (GetWindowStyle( hWnd ) & WS_POPUP) &&
            !pFrame->mbCaption &&
            (aRectPreMatrox.left != aRectPostMatrox.left || aRectPreMatrox.top != aRectPostMatrox.top) )
            SetWindowPos( hWnd, 0, aRectPreMatrox.left, aRectPreMatrox.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE  );

        if( aDogTag.isDeleted() )
            return;
        Window *pClientWin = pFrame->GetWindow()->ImplGetClientWindow();
        if ( pFrame->mbFloatWin || ( pClientWin && (pClientWin->GetStyle() & WB_SYSTEMFLOATWIN) ) )
            pFrame->mnShowState = SW_SHOWNOACTIVATE;
        else
            pFrame->mnShowState = SW_SHOW;
        
        if ( pFrame->mbPresentation )
        {
            HWND hWndParent = ::GetParent( hWnd );
            if ( hWndParent )
                SetForegroundWindow( hWndParent );
            SetForegroundWindow( hWnd );
        }

        pFrame->mbInShow = FALSE;
        pFrame->updateScreenNumber();

        
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



void WinSalFrame::Show( sal_Bool bVisible, sal_Bool bNoActivate )
{
    
    
    
    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
        ImplPostMessage( mhWnd, SAL_MSG_SHOW, bVisible, bNoActivate );
    else
        ImplSalShow( mhWnd, bVisible, bNoActivate );
}



void WinSalFrame::Enable( sal_Bool bEnable )
{
    EnableWindow( mhWnd, bEnable );
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
    sal_Bool bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    if ( !bVisible )
    {
        Window *pClientWin = GetWindow()->ImplGetClientWindow();
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

    sal_uInt16 nEvent = 0;
    UINT    nPosSize = 0;
    RECT    aClientRect, aWindowRect;
    GetClientRect( mhWnd, &aClientRect );   
    GetWindowRect( mhWnd, &aWindowRect );   

    if ( !(nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y)) )
        nPosSize |= SWP_NOMOVE;
    else
    {
        
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
            
            RECT aParentRect;
            GetClientRect( ImplGetParentHwnd( mhWnd ), &aParentRect );
            if( Application::GetSettings().GetLayoutRTL() )
                nX = (aParentRect.right - aParentRect.left) - nWidth-1 - nX;

            
            if( !(GetWindowStyle( mhWnd ) & WS_CHILD) )
            {
                POINT aPt;
                aPt.x = nX;
                aPt.y = nY;

                HWND parentHwnd = ImplGetParentHwnd( mhWnd );
                WinSalFrame* pParentFrame = GetWindowPtr( parentHwnd );
                if ( pParentFrame && pParentFrame->mnShowState == SW_SHOWMAXIMIZED )
                {
                    
                    
                    
                    aPt.x +=  pParentFrame->maGeometry.nX;
                    aPt.y +=  pParentFrame->maGeometry.nY;
                }
                else
                    ClientToScreen( parentHwnd, &aPt );

                nX = aPt.x;
                nY = aPt.y;
            }
    }

    
    
    if ( nFlags & SAL_FRAME_POSSIZE_X )
        nX += aWinRect.left;
    if ( nFlags & SAL_FRAME_POSSIZE_Y )
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

    if ( mbDefPos && (nPosSize & SWP_NOMOVE)) 
    {
        

        HWND hWndParent = ::GetParent( mhWnd );
        
        while ( hWndParent && (GetWindowStyle( hWndParent ) & WS_CHILD) )
            hWndParent = ::GetParent( hWndParent );
        
        
        if ( hWndParent && !IsIconic( hWndParent ) &&
             (GetWindowStyle( hWndParent ) & WS_VISIBLE) )
        {
            RECT aParentRect;
            GetWindowRect( hWndParent, &aParentRect );
            int nParentWidth    = aParentRect.right-aParentRect.left;
            int nParentHeight   = aParentRect.bottom-aParentRect.top;

            
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

            
            
            ImplSalGetWorkArea( mhWnd, &aRect2, &aRect2 );

            nX = ((aRect2.right-aRect2.left)-nWidth)/2 + aRect2.left;
            nY = ((aRect2.bottom-aRect2.top)-nHeight)/2 + aRect2.top;
        }


        
        

        mbDefPos = FALSE;   
        nPosSize &= ~SWP_NOMOVE;        
        nEvent = SALEVENT_MOVERESIZE;
    }


    
    sal_Bool bCheckOffScreen = TRUE;

    
    if( (mnStyle & SAL_FRAME_STYLE_FLOAT) && !(mnStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
        bCheckOffScreen = FALSE;

    if( mnStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION )
    {
        
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
    
    if( !(mnStyle & SAL_FRAME_STYLE_FLOAT) )
        nPosFlags |= SWP_NOZORDER; 

    SetWindowPos( mhWnd, HWND_TOP, nX, nY, (int)nWidth, (int)nHeight, nPosFlags  );

    UpdateFrameGeometry( mhWnd, this );

    
    if( nEvent )
        CallCallback( nEvent, NULL );
}



static void ImplSetParentFrame( WinSalFrame* pThis, HWND hNewParentWnd, sal_Bool bAsChild )
{
    
    HWND hWndOld = pThis->mhWnd;
    HWND hWndOldParent = ::GetParent( hWndOld );
    SalData* pSalData = GetSalData();

    if( hNewParentWnd == hWndOldParent )
        return;

    ::std::vector< WinSalFrame* > children;
    ::std::vector< WinSalObject* > systemChildren;

    
    WinSalFrame *pFrame = pSalData->mpFirstFrame;
    while( pFrame )
    {
        HWND hWndParent = ::GetParent( pFrame->mhWnd );
        if( pThis->mhWnd == hWndParent )
            children.push_back( pFrame );
        pFrame = pFrame->mpNextFrame;
    }

    
    WinSalObject *pObject = pSalData->mpFirstObject;
    while( pObject )
    {
        HWND hWndParent = ::GetParent( pObject->mhWnd );
        if( pThis->mhWnd == hWndParent )
            systemChildren.push_back( pObject );
        pObject = pObject->mpNextObject;
    }

    sal_Bool bNeedGraphics = pThis->mbGraphics;
    sal_Bool bNeedCacheDC  = FALSE;

    HFONT   hFont   = NULL;
    HPEN    hPen    = NULL;
    HBRUSH  hBrush  = NULL;

    #if OSL_DEBUG_LEVEL > 0
    int oldCount = pSalData->mnCacheDCInUse;
    (void)oldCount;
    #endif

    
    if ( pThis->mpGraphics2 &&
         pThis->mpGraphics2->getHDC() )
    {
        
        hFont   = (HFONT)   GetCurrentObject( pThis->mpGraphics2->getHDC(), OBJ_FONT);
        hPen    = (HPEN)    GetCurrentObject( pThis->mpGraphics2->getHDC(), OBJ_PEN);
        hBrush  = (HBRUSH)  GetCurrentObject( pThis->mpGraphics2->getHDC(), OBJ_BRUSH);
        pThis->ReleaseGraphics( pThis->mpGraphics2 );

        
        bNeedCacheDC  = TRUE;
    }

    
    if ( pThis->mpGraphics )
    {
        if ( pThis->mpGraphics->mhDefPal )
            SelectPalette( pThis->mpGraphics->getHDC(), pThis->mpGraphics->mhDefPal, TRUE );
        ImplSalDeInitGraphics( pThis->mpGraphics );
        ReleaseDC( pThis->mhWnd, pThis->mpGraphics->getHDC() );
    }

    
    HWND hWndParent = hNewParentWnd;
    
    HWND hWnd = (HWND) (sal_IntPtr) ImplSendMessage( pSalData->mpFirstInstance->mhComWnd,
                                        bAsChild ? SAL_MSG_RECREATECHILDHWND : SAL_MSG_RECREATEHWND,
                                        (WPARAM) hWndParent, (LPARAM)pThis->mhWnd );

    
    DBG_ASSERT( IsWindow( hWnd ), "WinSalFrame::SetParent not successful");

    
    if( bNeedGraphics )
    {
        if( pThis->mpGraphics2 )
        {
            pThis->mpGraphics2->mhWnd = hWnd;

            if( bNeedCacheDC )
            {
                
                HDC hDC = (HDC)(sal_IntPtr)ImplSendMessage( pSalData->mpFirstInstance->mhComWnd,
                                                SAL_MSG_GETDC,
                                                (WPARAM) hWnd, 0 );
                if ( hDC )
                {
                    pThis->mpGraphics2->setHDC(hDC);
                    if ( pSalData->mhDitherPal )
                    {
                        pThis->mpGraphics2->mhDefPal = SelectPalette( hDC, pSalData->mhDitherPal, TRUE );
                        RealizePalette( hDC );
                    }
                    ImplSalInitGraphics( pThis->mpGraphics2 );

                    
                    if( hFont )
                        SelectObject( hDC, hFont );
                    if( hPen )
                        SelectObject( hDC, hPen );
                    if( hBrush )
                        SelectObject( hDC, hBrush );

                    pThis->mbGraphics = TRUE;

                    pSalData->mnCacheDCInUse++;

                    DBG_ASSERT( oldCount == pSalData->mnCacheDCInUse, "WinSalFrame::SetParent() hDC count corrupted");
                }
            }
        }

        if( pThis->mpGraphics )
        {
            
            pThis->mpGraphics->mhWnd = hWnd;
            pThis->mpGraphics->setHDC( GetDC( hWnd ) );
            if ( GetSalData()->mhDitherPal )
            {
                pThis->mpGraphics->mhDefPal = SelectPalette( pThis->mpGraphics->getHDC(), GetSalData()->mhDitherPal, TRUE );
                RealizePalette( pThis->mpGraphics->getHDC() );
            }
            ImplSalInitGraphics( pThis->mpGraphics );
            pThis->mbGraphics = TRUE;
        }
    }


    
    DBG_ASSERT( systemChildren.empty(), "WinSalFrame::SetParent() parent of living system child window will be destroyed!");

    
    for( ::std::vector< WinSalFrame* >::iterator iChild = children.begin(); iChild != children.end(); ++iChild )
        ImplSetParentFrame( *iChild, hWnd, FALSE );

    children.clear();
    systemChildren.clear();

    
    ImplSendMessage( GetSalData()->mpFirstInstance->mhComWnd,
                     SAL_MSG_DESTROYHWND, (WPARAM) 0, (LPARAM)hWndOld);
}



void WinSalFrame::SetParent( SalFrame* pNewParent )
{
    WinSalFrame::mbInReparent = TRUE;
    ImplSetParentFrame( this, static_cast<WinSalFrame*>(pNewParent)->mhWnd, FALSE );
    WinSalFrame::mbInReparent = FALSE;
}

bool WinSalFrame::SetPluginParent( SystemParentData* pNewParent )
{
    if ( pNewParent->hWnd == 0 )
    {
        pNewParent->hWnd = GetDesktopWindow();
    }

    WinSalFrame::mbInReparent = TRUE;
    ImplSetParentFrame( this, pNewParent->hWnd, TRUE );
    WinSalFrame::mbInReparent = FALSE;
    return true;
}




void WinSalFrame::GetWorkArea( Rectangle &rRect )
{
    RECT aRect;
    ImplSalGetWorkArea( mhWnd, &aRect, NULL );
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
    
    nScreenX        = aRect.left-10;
    nScreenY        = aRect.top-10;
    nScreenWidth    = aRect.right-aRect.left+20;
    nScreenHeight   = aRect.bottom-aRect.top+20;

    UINT    nPosSize    = 0;
    RECT    aWinRect;
    GetWindowRect( mhWnd, &aWinRect );

    
    
    RECT aRect2 = aWinRect;
    AdjustWindowRectEx( &aRect2, GetWindowStyle( mhWnd ),
                    FALSE,     GetWindowExStyle( mhWnd ) );
    long nTopDeco = abs( aWinRect.top - aRect2.top );
    long nLeftDeco = abs( aWinRect.left - aRect2.left );
    long nBottomDeco = abs( aWinRect.bottom - aRect2.bottom );
    long nRightDeco = abs( aWinRect.right - aRect2.right );

    
    if ( !(pState->mnMask & (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y)) )
        nPosSize |= SWP_NOMOVE;
    if ( !(pState->mnMask & (WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT)) )
        nPosSize |= SWP_NOSIZE;
    if ( pState->mnMask & WINDOWSTATE_MASK_X )
        nX = (int)pState->mnX - nLeftDeco;
    else
        nX = aWinRect.left;
    if ( pState->mnMask & WINDOWSTATE_MASK_Y )
        nY = (int)pState->mnY - nTopDeco;
    else
        nY = aWinRect.top;
    if ( pState->mnMask & WINDOWSTATE_MASK_WIDTH )
        nWidth = (int)pState->mnWidth + nLeftDeco + nRightDeco;
    else
        nWidth = aWinRect.right-aWinRect.left;
    if ( pState->mnMask & WINDOWSTATE_MASK_HEIGHT )
        nHeight = (int)pState->mnHeight + nTopDeco + nBottomDeco;
    else
        nHeight = aWinRect.bottom-aWinRect.top;

    
    
    

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

    
    WINDOWPLACEMENT aPlacement;
    aPlacement.length = sizeof( aPlacement );
    GetWindowPlacement( mhWnd, &aPlacement );

    
    sal_Bool bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    sal_Bool bUpdateHiddenFramePos = FALSE;
    if ( !bVisible )
    {
        aPlacement.showCmd = SW_HIDE;

        if ( mbOverwriteState )
        {
            if ( pState->mnMask & WINDOWSTATE_MASK_STATE )
            {
                if ( pState->mnState & WINDOWSTATE_STATE_MINIMIZED )
                    mnShowState = SW_SHOWMINIMIZED;
                else if ( pState->mnState & WINDOWSTATE_STATE_MAXIMIZED )
                {
                    mnShowState = SW_SHOWMAXIMIZED;
                    bUpdateHiddenFramePos = TRUE;
                }
                else if ( pState->mnState & WINDOWSTATE_STATE_NORMAL )
                    mnShowState = SW_SHOWNORMAL;
            }
        }
    }
    else
    {
        if ( pState->mnMask & WINDOWSTATE_MASK_STATE )
        {
            if ( pState->mnState & WINDOWSTATE_STATE_MINIMIZED )
            {
                if ( pState->mnState & WINDOWSTATE_STATE_MAXIMIZED )
                    aPlacement.flags |= WPF_RESTORETOMAXIMIZED;
                aPlacement.showCmd = SW_SHOWMINIMIZED;
            }
            else if ( pState->mnState & WINDOWSTATE_STATE_MAXIMIZED )
                aPlacement.showCmd = SW_SHOWMAXIMIZED;
            else if ( pState->mnState & WINDOWSTATE_STATE_NORMAL )
                aPlacement.showCmd = SW_RESTORE;
        }
    }

    
    
    
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
            
            
            SetMaximizedFrameGeometry( mhWnd, this, &aStateRect );
            SetWindowPos( mhWnd, 0,
                          maGeometry.nX, maGeometry.nY, maGeometry.nWidth, maGeometry.nHeight,
                          SWP_NOZORDER | SWP_NOACTIVATE | nPosSize );
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
        mbDefPos = FALSE; 
}



bool WinSalFrame::GetWindowState( SalFrameState* pState )
{
    if ( maState.mnWidth && maState.mnHeight )
    {
        *pState = maState;
        
        
        
        if ( !(pState->mnState & (WINDOWSTATE_STATE_MINIMIZED | WINDOWSTATE_STATE_MAXIMIZED)) )
            pState->mnState |= WINDOWSTATE_STATE_NORMAL;
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
        
        

        typedef HRESULT ( WINAPI *SHGETPROPERTYSTOREFORWINDOW )( HWND, REFIID, void ** );
        SHGETPROPERTYSTOREFORWINDOW pSHGetPropertyStoreForWindow;
        pSHGetPropertyStoreForWindow = ( SHGETPROPERTYSTOREFORWINDOW )GetProcAddress(
                                       GetModuleHandleW (L"shell32.dll"), "SHGetPropertyStoreForWindow" );

        if( pSHGetPropertyStoreForWindow )
        {
            IPropertyStore *pps;
            HRESULT hr = pSHGetPropertyStoreForWindow ( mhWnd, IID_PPV_ARGS(&pps) );
            if ( SUCCEEDED(hr) )
            {
                PROPVARIANT pv;
                if ( !rApplicationID.isEmpty() )
                {
                    hr = InitPropVariantFromString( ( PCWSTR )rApplicationID.getStr(), &pv );
                    mbPropertiesStored = TRUE;
                }
                else
                    
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
        
        DWORD nExStyle = GetWindowExStyle( mhWnd );
        if ( nExStyle & WS_EX_TOOLWINDOW )
        {
            mbFullScreenToolWin = TRUE;
            nExStyle &= ~WS_EX_TOOLWINDOW;
            SetWindowExStyle( mhWnd, nExStyle );
        }
        
        GetWindowRect( mhWnd, &maFullScreenRect );

        
        mnFullScreenShowState = mnShowState;
        if ( !(GetWindowStyle( mhWnd ) & WS_VISIBLE) )
            mnShowState = SW_SHOW;

        
        ImplSalFrameFullScreenPos( this, TRUE );
    }
    else
    {
        
        
        sal_Bool bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
        if ( bVisible && (mnShowState != mnFullScreenShowState) )
            ShowWindow( mhWnd, SW_HIDE );

        if ( mbFullScreenToolWin )
            SetWindowExStyle( mhWnd, GetWindowExStyle( mhWnd ) | WS_EX_TOOLWINDOW );
        mbFullScreenToolWin = FALSE;

        SetWindowPos( mhWnd, 0,
                      maFullScreenRect.left,
                      maFullScreenRect.top,
                      maFullScreenRect.right-maFullScreenRect.left,
                      maFullScreenRect.bottom-maFullScreenRect.top,
                      SWP_NOZORDER | SWP_NOACTIVATE );

        
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



void WinSalFrame::StartPresentation( sal_Bool bStart )
{
    if ( mbPresentation == bStart )
        return;

    mbPresentation = bStart;

    SalData* pSalData = GetSalData();
    if ( bStart )
    {
        if ( !pSalData->mpSageEnableProc )
        {
            if ( pSalData->mnSageStatus != DISABLE_AGENT )
            {
                OFSTRUCT aOS;
                OpenFile( "SAGE.DLL", &aOS, OF_EXIST );

                if ( !aOS.nErrCode )
                {
                    OUString aLibraryName( OUString::createFromAscii( aOS.szPathName ) );
                    oslModule mhSageInst = osl_loadModule( aLibraryName.pData, SAL_LOADMODULE_DEFAULT );
                    pSalData->mpSageEnableProc = (SysAgt_Enable_PROC)osl_getAsciiFunctionSymbol( mhSageInst, "System_Agent_Enable" );
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

        
        SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, 0,
                              &(pSalData->mbScrSvrEnabled), 0 );
        if ( pSalData->mbScrSvrEnabled )
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0 );
    }
    else
    {
        
        if ( pSalData->mbScrSvrEnabled )
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, pSalData->mbScrSvrEnabled, 0, 0 );

        
        if ( pSalData->mnSageStatus == ENABLE_AGENT )
            pSalData->mpSageEnableProc( pSalData->mnSageStatus );
    }
}



void WinSalFrame::SetAlwaysOnTop( sal_Bool bOnTop )
{
    HWND hWnd;
    if ( bOnTop )
        hWnd = HWND_TOPMOST;
    else
        hWnd = HWND_NOTOPMOST;
    SetWindowPos( mhWnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
}



static void ImplSalToTop( HWND hWnd, sal_uInt16 nFlags )
{
    WinSalFrame* pToTopFrame = GetWindowPtr( hWnd );
    if( pToTopFrame && (pToTopFrame->mnStyle & SAL_FRAME_STYLE_SYSTEMCHILD) != 0 )
        BringWindowToTop( hWnd );

    if ( nFlags & SAL_FRAME_TOTOP_FOREGROUNDTASK )
    {
        
        
        
        HWND   hCurrWnd     = GetForegroundWindow();
        DWORD  myThreadID   = GetCurrentThreadId();
        DWORD  currThreadID = GetWindowThreadProcessId(hCurrWnd,NULL);
        AttachThreadInput(myThreadID, currThreadID,TRUE);
        SetForegroundWindow(hWnd);
        AttachThreadInput(myThreadID,currThreadID,FALSE);
    }

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

    if ( !IsIconic( hWnd ) && IsWindowVisible( hWnd ) )
    {
        SetFocus( hWnd );

        
        
        if ( ::GetFocus() == hWnd )
            SetForegroundWindow( hWnd );
    }
}



void WinSalFrame::ToTop( sal_uInt16 nFlags )
{
    nFlags &= ~SAL_FRAME_TOTOP_GRABFOCUS;   
    
    
    
    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
        ImplPostMessage( mhWnd, SAL_MSG_TOTOP, nFlags, 0 );
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

    static ImplPtrData aImplPtrTab[POINTER_COUNT] =
    {
    { 0, IDC_ARROW, 0 },                            
    { 0, 0, SAL_RESID_POINTER_NULL },               
    { 0, IDC_WAIT, 0 },                             
    { 0, IDC_IBEAM, 0 },                            
    { 0, IDC_HELP, 0 },                             
    { 0, 0, SAL_RESID_POINTER_CROSS },              
    { 0, 0, SAL_RESID_POINTER_MOVE },               
    { 0, IDC_SIZENS, 0 },                           
    { 0, IDC_SIZENS, 0 },                           
    { 0, IDC_SIZEWE, 0 },                           
    { 0, IDC_SIZEWE, 0 },                           
    { 0, IDC_SIZENWSE, 0 },                         
    { 0, IDC_SIZENESW, 0 },                         
    { 0, IDC_SIZENESW, 0 },                         
    { 0, IDC_SIZENWSE, 0 },                         
    { 0, IDC_SIZENS, 0 },                           
    { 0, IDC_SIZENS, 0 },                           
    { 0, IDC_SIZEWE, 0 },                           
    { 0, IDC_SIZEWE, 0 },                           
    { 0, IDC_SIZENWSE, 0 },                         
    { 0, IDC_SIZENESW, 0 },                         
    { 0, IDC_SIZENESW, 0 },                         
    { 0, IDC_SIZENWSE, 0 },                         
    { 0, 0, SAL_RESID_POINTER_HSPLIT },             
    { 0, 0, SAL_RESID_POINTER_VSPLIT },             
    { 0, 0, SAL_RESID_POINTER_HSIZEBAR },           
    { 0, 0, SAL_RESID_POINTER_VSIZEBAR },           
    { 0, 0, SAL_RESID_POINTER_HAND },               
    { 0, 0, SAL_RESID_POINTER_REFHAND },            
    { 0, 0, SAL_RESID_POINTER_PEN },                
    { 0, 0, SAL_RESID_POINTER_MAGNIFY },            
    { 0, 0, SAL_RESID_POINTER_FILL },               
    { 0, 0, SAL_RESID_POINTER_ROTATE },             
    { 0, 0, SAL_RESID_POINTER_HSHEAR },             
    { 0, 0, SAL_RESID_POINTER_VSHEAR },             
    { 0, 0, SAL_RESID_POINTER_MIRROR },             
    { 0, 0, SAL_RESID_POINTER_CROOK },              
    { 0, 0, SAL_RESID_POINTER_CROP },               
    { 0, 0, SAL_RESID_POINTER_MOVEPOINT },          
    { 0, 0, SAL_RESID_POINTER_MOVEBEZIERWEIGHT },   
    { 0, 0, SAL_RESID_POINTER_MOVEDATA },           
    { 0, 0, SAL_RESID_POINTER_COPYDATA },           
    { 0, 0, SAL_RESID_POINTER_LINKDATA },           
    { 0, 0, SAL_RESID_POINTER_MOVEDATALINK },       
    { 0, 0, SAL_RESID_POINTER_COPYDATALINK },       
    { 0, 0, SAL_RESID_POINTER_MOVEFILE },           
    { 0, 0, SAL_RESID_POINTER_COPYFILE },           
    { 0, 0, SAL_RESID_POINTER_LINKFILE },           
    { 0, 0, SAL_RESID_POINTER_MOVEFILELINK },       
    { 0, 0, SAL_RESID_POINTER_COPYFILELINK },       
    { 0, 0, SAL_RESID_POINTER_MOVEFILES },          
    { 0, 0, SAL_RESID_POINTER_COPYFILES },          
    { 0, 0, SAL_RESID_POINTER_NOTALLOWED },         
    { 0, 0, SAL_RESID_POINTER_DRAW_LINE },          
    { 0, 0, SAL_RESID_POINTER_DRAW_RECT },          
    { 0, 0, SAL_RESID_POINTER_DRAW_POLYGON },       
    { 0, 0, SAL_RESID_POINTER_DRAW_BEZIER },        
    { 0, 0, SAL_RESID_POINTER_DRAW_ARC },           
    { 0, 0, SAL_RESID_POINTER_DRAW_PIE },           
    { 0, 0, SAL_RESID_POINTER_DRAW_CIRCLECUT },     
    { 0, 0, SAL_RESID_POINTER_DRAW_ELLIPSE },       
    { 0, 0, SAL_RESID_POINTER_DRAW_FREEHAND },      
    { 0, 0, SAL_RESID_POINTER_DRAW_CONNECT },       
    { 0, 0, SAL_RESID_POINTER_DRAW_TEXT },          
    { 0, 0, SAL_RESID_POINTER_DRAW_CAPTION },       
    { 0, 0, SAL_RESID_POINTER_CHART },              
    { 0, 0, SAL_RESID_POINTER_DETECTIVE },          
    { 0, 0, SAL_RESID_POINTER_PIVOT_COL },          
    { 0, 0, SAL_RESID_POINTER_PIVOT_ROW },          
    { 0, 0, SAL_RESID_POINTER_PIVOT_FIELD },        
    { 0, 0, SAL_RESID_POINTER_CHAIN },              
    { 0, 0, SAL_RESID_POINTER_CHAIN_NOTALLOWED },   
    { 0, 0, SAL_RESID_POINTER_TIMEEVENT_MOVE },     
    { 0, 0, SAL_RESID_POINTER_TIMEEVENT_SIZE },     
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_N },       
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_S },       
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_W },       
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_E },       
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NW },      
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NE },      
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_SW },      
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_SE },      
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NS },      
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_WE },      
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NSWE },    
    { 0, 0, SAL_RESID_POINTER_AIRBRUSH },           
    { 0, 0, SAL_RESID_POINTER_TEXT_VERTICAL },      
    { 0, 0, SAL_RESID_POINTER_PIVOT_DELETE },       

    
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_S },       
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_E },       
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_SE },      
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_W },       
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_SW },      

    
    { 0, 0, SAL_RESID_POINTER_PAINTBRUSH }          

    };

#if POINTER_COUNT != 94
#error New Pointer must be defined!
#endif

    
    if ( !aImplPtrTab[ePointerStyle].mhCursor )
    {
        if ( aImplPtrTab[ePointerStyle].mnOwnId )
            aImplPtrTab[ePointerStyle].mhCursor = ImplLoadSalCursor( aImplPtrTab[ePointerStyle].mnOwnId );
        else
            aImplPtrTab[ePointerStyle].mhCursor = LoadCursor( 0, aImplPtrTab[ePointerStyle].mnSysId );
    }

    
    if ( mhCursor != aImplPtrTab[ePointerStyle].mhCursor )
    {
        mhCursor = aImplPtrTab[ePointerStyle].mhCursor;
        SetCursor( mhCursor );
    }
}



void WinSalFrame::CaptureMouse( sal_Bool bCapture )
{
    
    
    int nMsg;
    if ( bCapture )
        nMsg = SAL_MSG_CAPTUREMOUSE;
    else
        nMsg = SAL_MSG_RELEASEMOUSE;
    ImplSendMessage( mhWnd, nMsg, 0, 0 );
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



void WinSalFrame::Sync()
{
    GdiFlush();
}



static void ImplSalFrameSetInputContext( HWND hWnd, const SalInputContext* pContext )
{
    WinSalFrame*   pFrame = GetWindowPtr( hWnd );
    sal_Bool        bIME = (pContext->mnOptions & SAL_INPUTCONTEXT_TEXT) != 0;
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

        
        
        if ( !(pContext->mnOptions & SAL_INPUTCONTEXT_EXTTEXTINPUT) )
            pFrame->mbHandleIME = FALSE;

        
        if ( pContext->mpFont )
        {
            HIMC hIMC = ImmGetContext( pFrame->mhWnd );
            if ( hIMC )
            {
                LOGFONTW aLogFont;
                HDC hDC = GetDC( pFrame->mhWnd );
                
                
                
                
                
                
                
                
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



void WinSalFrame::SetInputContext( SalInputContext* pContext )
{
    
    ImplSendMessage( mhWnd, SAL_MSG_SETINPUTCONTEXT, 0, (LPARAM)(void*)pContext );
}



static void ImplSalFrameEndExtTextInput( HWND hWnd, sal_uInt16 nFlags )
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



void WinSalFrame::EndExtTextInput( sal_uInt16 nFlags )
{
    
    ImplSendMessage( mhWnd, SAL_MSG_ENDEXTTEXTINPUT, (WPARAM)nFlags, 0 );
}



static void ImplGetKeyNameText( LONG lParam, sal_Unicode* pBuf,
                                UINT& rCount, UINT nMaxSize,
                                const sal_Char* pReplace )
{
    DBG_ASSERT( sizeof( WCHAR ) == sizeof( sal_Unicode ), "WinSalFrame::ImplGetKeyNameTextW(): WCHAR != sal_Unicode" );

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
            DBG_ASSERT( nKeyLen <= nMaxKeyLen, "Invalid key name length!" );
            if( nKeyLen > nMaxKeyLen )
                nKeyLen = 0;
            else if( nKeyLen > 0 )
            {
                
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
            while( *pW && (pW < pE) && (rCount < nMaxSize) )
                pBuf[rCount++] = *pW++;
        }
        else 
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
    const sal_Char*   pReplace = NULL;
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



static void ImplSalUpdateStyleFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont )
{
    ImplSalLogFontToFontW( hDC, rLogFont, rFont );

    
    
    
    
    
    if ( rFont.GetHeight() < 8 )
    {
        if ( rtl_ustr_compareIgnoreAsciiCase( reinterpret_cast<const sal_Unicode*>(rLogFont.lfFaceName), reinterpret_cast<const sal_Unicode*>(L"MS Sans Serif") ) == 0 )
            rFont.SetHeight( 8 );
        else if ( rFont.GetHeight() < 6 )
            rFont.SetHeight( 6 );
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
        if ( RegQueryValueEx( hRegKey, "MenuShowDelay", 0,
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
    aStyleSettings.SetCursorBlinkTime( GetCaretBlinkTime() );
    aStyleSettings.SetFloatTitleHeight( GetSystemMetrics( SM_CYSMCAPTION ) );
    aStyleSettings.SetTitleHeight( GetSystemMetrics( SM_CYCAPTION ) );
    aStyleSettings.SetActiveBorderColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVEBORDER ) ) );
    aStyleSettings.SetDeactiveBorderColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVEBORDER ) ) );
    aStyleSettings.SetActiveColor2( ImplWinColorToSal( GetSysColor( COLOR_GRADIENTACTIVECAPTION ) ) );
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
    aStyleSettings.SetRadioCheckTextColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOWTEXT ) ) );
    aStyleSettings.SetGroupTextColor( aStyleSettings.GetRadioCheckTextColor() );
    aStyleSettings.SetLabelTextColor( aStyleSettings.GetRadioCheckTextColor() );
    aStyleSettings.SetInfoTextColor( aStyleSettings.GetRadioCheckTextColor() );
    aStyleSettings.SetWindowColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOW ) ) );
    aStyleSettings.SetActiveTabColor( aStyleSettings.GetWindowColor() );
    aStyleSettings.SetWindowTextColor( ImplWinColorToSal( GetSysColor( COLOR_WINDOWTEXT ) ) );
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
    GetSalData()->mbThemeMenuSupport = FALSE;
    if (officecfg::Office::Common::Accessibility::AutoDetectSystemHC::get())
    {
        aStyleSettings.SetShadowColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVEBORDER ) ) );
        aStyleSettings.SetWorkspaceColor( ImplWinColorToSal( GetSysColor( COLOR_MENU ) ) );
    }
    aStyleSettings.SetMenuColor( ImplWinColorToSal( GetSysColor( COLOR_MENU ) ) );
    aStyleSettings.SetMenuBarColor( aStyleSettings.GetMenuColor() );
    aStyleSettings.SetMenuBarRolloverColor( aStyleSettings.GetHighlightColor() );
    aStyleSettings.SetMenuBorderColor( aStyleSettings.GetLightBorderColor() ); 
    aStyleSettings.SetUseFlatBorders( FALSE );
    aStyleSettings.SetUseFlatMenus( FALSE );
    aStyleSettings.SetMenuTextColor( ImplWinColorToSal( GetSysColor( COLOR_MENUTEXT ) ) );
    aStyleSettings.SetMenuBarTextColor( ImplWinColorToSal( GetSysColor( COLOR_MENUTEXT ) ) );
    aStyleSettings.SetMenuBarRolloverTextColor( ImplWinColorToSal( GetSysColor( COLOR_HIGHLIGHTTEXT ) ) );
    aStyleSettings.SetActiveColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVECAPTION ) ) );
    aStyleSettings.SetActiveTextColor( ImplWinColorToSal( GetSysColor( COLOR_CAPTIONTEXT ) ) );
    aStyleSettings.SetDeactiveColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVECAPTION ) ) );
    aStyleSettings.SetDeactiveTextColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVECAPTIONTEXT ) ) );
    if ( aSalShlData.mbWXP )
    {
        
        long bFlatMenus = 0;
        SystemParametersInfo( SPI_GETFLATMENU, 0, &bFlatMenus, 0);
        if( bFlatMenus )
        {
            aStyleSettings.SetUseFlatMenus( TRUE );
            aStyleSettings.SetMenuBarColor( ImplWinColorToSal( GetSysColor( COLOR_MENUBAR ) ) );
            aStyleSettings.SetMenuHighlightColor( ImplWinColorToSal( GetSysColor( COLOR_MENUHILIGHT ) ) );
            aStyleSettings.SetMenuBarRolloverColor( ImplWinColorToSal( GetSysColor( COLOR_MENUHILIGHT ) ) );
            aStyleSettings.SetMenuBorderColor( ImplWinColorToSal( GetSysColor( COLOR_3DSHADOW ) ) );

            
            
            aStyleSettings.SetUseFlatBorders( TRUE );
        }
    }
    aStyleSettings.SetCheckedColorSpecialCase( );

    
    DWORD nCaretWidth = 2;
    if( SystemParametersInfo( SPI_GETCARETWIDTH, 0, &nCaretWidth, 0 ) )
        aStyleSettings.SetCursorSize( nCaretWidth );

    
    HIGHCONTRAST hc;
    hc.cbSize = sizeof( HIGHCONTRAST );
    if( SystemParametersInfo( SPI_GETHIGHCONTRAST, hc.cbSize, &hc, 0 )
            && (hc.dwFlags & HCF_HIGHCONTRASTON) )
        aStyleSettings.SetHighContrastMode( true );
    else
        aStyleSettings.SetHighContrastMode( false );

    
    Font    aMenuFont = aStyleSettings.GetMenuFont();
    Font    aTitleFont = aStyleSettings.GetTitleFont();
    Font    aFloatTitleFont = aStyleSettings.GetFloatTitleFont();
    Font    aHelpFont = aStyleSettings.GetHelpFont();
    Font    aAppFont = aStyleSettings.GetAppFont();
    Font    aIconFont = aStyleSettings.GetIconFont();
    HDC     hDC = GetDC( 0 );
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

    
    long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );

    ReleaseDC( 0, hDC );

    long nHeightPx = aMenuFont.GetHeight() * nDPIY / 72;
    aStyleSettings.SetToolbarIconSize( (((nHeightPx-1)*2) >= 28) ? STYLE_TOOLBAR_ICONSIZE_LARGE : STYLE_TOOLBAR_ICONSIZE_SMALL );

    aStyleSettings.SetMenuFont( aMenuFont );
    aStyleSettings.SetTitleFont( aTitleFont );
    aStyleSettings.SetFloatTitleFont( aFloatTitleFont );
    aStyleSettings.SetHelpFont( aHelpFont );
    aStyleSettings.SetIconFont( aIconFont );
    
    
    if ( rSettings.GetLanguageTag().getLanguageType() == LANGUAGE_RUSSIAN )
    {
        OUString aFontName = aAppFont.GetName();
        OUString aFirstName = aFontName.getToken( 0, ';' );
        if ( aFirstName.equalsIgnoreAsciiCase( "MS Sans Serif" ) )
        {
            aFontName = "Arial;" + aFontName;
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

    BOOL bDragFull;
    if ( SystemParametersInfo( SPI_GETDRAGFULLWINDOWS, 0, &bDragFull, 0 ) )
    {
        sal_uLong nDragFullOptions = aStyleSettings.GetDragFullOptions();
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

    
    WinSalGraphics::updateSettingsNative( rSettings );
}



const SystemEnvData* WinSalFrame::GetSystemData() const
{
    return &maSysData;
}



void WinSalFrame::Beep()
{
    
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



SalFrame::SalIndicatorState WinSalFrame::GetIndicatorState()
{
    SalIndicatorState aState;
    aState.mnState = 0;
    if (::GetKeyState(VK_CAPITAL))
        aState.mnState |= INDICATOR_CAPSLOCK;

    if (::GetKeyState(VK_NUMLOCK))
        aState.mnState |= INDICATOR_NUMLOCK;

    if (::GetKeyState(VK_SCROLL))
        aState.mnState |= INDICATOR_SCROLLLOCK;

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
    SetWindowRgn( mhWnd, 0, TRUE );
}



void WinSalFrame::BeginSetClipRegion( sal_uLong nRects )
{
    if( mpClipRgnData )
        delete [] (BYTE*)mpClipRgnData;
    sal_uLong nRectBufSize = sizeof(RECT)*nRects;
    mpClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+nRectBufSize];
    mpClipRgnData->rdh.dwSize     = sizeof( RGNDATAHEADER );
    mpClipRgnData->rdh.iType      = RDH_RECTANGLES;
    mpClipRgnData->rdh.nCount     = nRects;
    mpClipRgnData->rdh.nRgnSize  = nRectBufSize;
    SetRectEmpty( &(mpClipRgnData->rdh.rcBound) );
    mpNextClipRect        = (RECT*)(&(mpClipRgnData->Buffer));
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
    if( (mpNextClipRect  - (RECT*)(&mpClipRgnData->Buffer)) < (int)mpClipRgnData->rdh.nCount )
        mpNextClipRect++;
}



void WinSalFrame::EndSetClipRegion()
{
    if( ! mpClipRgnData )
        return;

    HRGN hRegion;

    
    if ( mpClipRgnData->rdh.nCount == 1 )
    {
        RECT* pRect = &(mpClipRgnData->rdh.rcBound);
        hRegion = CreateRectRgn( pRect->left, pRect->top,
                                 pRect->right, pRect->bottom );
    }
    else
    {
        sal_uLong nSize = mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
        hRegion = ExtCreateRegion( NULL, nSize, mpClipRgnData );
    }
    delete [] (BYTE*)mpClipRgnData;
    mpClipRgnData = NULL;

    DBG_ASSERT( hRegion, "WinSalFrame::EndSetClipRegion() - Can't create ClipRegion" );
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
        
        
        
        Window *pWin = pFrame->GetWindow();
        if( pWin && pWin->ImplGetWindowImpl()->mpFrameData->mnFocusId )
        {
            ImplPostMessage( hWnd, nMsg, wParam, lParam );
            return 1;
        }
    }
    SalMouseEvent   aMouseEvt;
    long            nRet;
    sal_uInt16          nEvent = 0;
    sal_Bool            bCall = TRUE;

    aMouseEvt.mnX       = (short)LOWORD( lParam );
    aMouseEvt.mnY       = (short)HIWORD( lParam );
    aMouseEvt.mnCode    = 0;
    aMouseEvt.mnTime    = GetMessageTime();

    
    

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
            
            if ( pSalData->mhWantLeaveMsg && (pSalData->mhWantLeaveMsg != hWnd) )
                ImplSendMessage( pSalData->mhWantLeaveMsg, SAL_MSG_MOUSELEAVE, 0, GetMessagePos() );

            pSalData->mhWantLeaveMsg = hWnd;
            
            if ( !pSalData->mpMouseLeaveTimer )
            {
                pSalData->mpMouseLeaveTimer = new AutoTimer;
                pSalData->mpMouseLeaveTimer->SetTimeout( SAL_MOUSELEAVE_TIMEOUT );
                pSalData->mpMouseLeaveTimer->Start();
                
                
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

    
    
    if( !IsWindow( hWnd ) )
        return 0;

    if ( bCall )
    {
        if ( nEvent == SALEVENT_MOUSEBUTTONDOWN )
            UpdateWindow( hWnd );

        
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

            
            aWheelEvt.mnDelta *= -1;
            aWheelEvt.mnNotchDelta *= -1;
        }

        if ( nWinModCode & MK_SHIFT )
            aWheelEvt.mnCode |= KEY_SHIFT;
        if ( nWinModCode & MK_CONTROL )
            aWheelEvt.mnCode |= KEY_MOD1;
        if ( GetKeyState( VK_MENU ) & 0x8000 )
            aWheelEvt.mnCode |= KEY_MOD2;

        
        if( Application::GetSettings().GetLayoutRTL() )
            aWheelEvt.mnX = pFrame->maGeometry.nWidth-1-aWheelEvt.mnX;

        nRet = pFrame->CallCallback( SALEVENT_WHEELMOUSE, &aWheelEvt );
    }

    ImplSalYieldMutexRelease();

    return nRet;
}



static sal_uInt16 ImplSalGetKeyCode( WPARAM wParam )
{
    sal_uInt16 nKeyCode;

    
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
        
        pFrame->mnInputLang = nLang;
    }

    
    
    
    return;
}


static sal_Unicode ImplGetCharCode( WinSalFrame* pFrame, WPARAM nCharCode )
{
    ImplUpdateInputLang( pFrame );

    
    
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



bool WinSalFrame::MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode )
{
    sal_Bool bRet = FALSE;
    sal_IntPtr nLangType = aLangType;
    
    HKL hkl = (HKL) nLangType;

    if( hkl )
    {
        SHORT scan = VkKeyScanExW( aUnicode, hkl );
        if( LOWORD(scan) == 0xFFFF )
            
            bRet = FALSE;
        else
        {
            BYTE vkeycode   = LOBYTE(scan);
            BYTE shiftstate = HIBYTE(scan);

            
            
            
            
            
            
            rKeyCode = KeyCode( ImplSalGetKeyCode( vkeycode ),
                (shiftstate & 0x01) ? TRUE : FALSE,     
                (shiftstate & 0x02) ? TRUE : FALSE,     
                (shiftstate & 0x04) ? TRUE : FALSE,     
                FALSE );
            bRet = TRUE;
        }
    }

    return bRet;
}



static long ImplHandleKeyMsg( HWND hWnd, UINT nMsg,
                              WPARAM wParam, LPARAM lParam, LRESULT& rResult )
{
    static sal_Bool     bIgnoreCharMsg  = FALSE;
    static WPARAM   nDeadChar       = 0;
    static WPARAM   nLastVKChar     = 0;
    static sal_uInt16   nLastChar       = 0;
    static sal_uInt16   nLastModKeyCode = 0;
    static bool     bWaitForModKeyRelease = false;
    sal_uInt16          nRepeat         = LOWORD( lParam )-1;
    sal_uInt16          nModCode        = 0;

    
    
    GetSalData()->mnSalObjWantKeyEvt = 0;

    if ( nMsg == WM_DEADCHAR )
    {
        nDeadChar = wParam;
        return 0;
    }

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    
    
    if ( pFrame->mpGraphics &&
         pFrame->mpGraphics->getHDC() )
        SetBkMode( pFrame->mpGraphics->getHDC(), TRANSPARENT );

    
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        nModCode |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        nModCode |= KEY_MOD1;
    if ( GetKeyState( VK_MENU ) & 0x8000 )
        nModCode |= KEY_MOD2;

    if ( (nMsg == WM_CHAR) || (nMsg == WM_SYSCHAR) )
    {
        nDeadChar = 0;

        if ( bIgnoreCharMsg )
        {
            bIgnoreCharMsg = FALSE;
            
            
            

            
            
            if( (GetWindowStyle( hWnd ) & WS_CHILD) || GetMenu( hWnd ) || (wParam == 0x20) )
                return 0;
            else
                return 1;
        }

        
        
        if ( wParam == 0x08 )    
            return 0;

        
        
        SalKeyEvent aKeyEvt;

        if ( (wParam >= '0') && (wParam <= '9') )
            aKeyEvt.mnCode = sal::static_int_cast<sal_uInt16>(KEYGROUP_NUM + wParam - '0');
        else if ( (wParam >= 'A') && (wParam <= 'Z') )
            aKeyEvt.mnCode = sal::static_int_cast<sal_uInt16>(KEYGROUP_ALPHA + wParam - 'A');
        else if ( (wParam >= 'a') && (wParam <= 'z') )
            aKeyEvt.mnCode = sal::static_int_cast<sal_uInt16>(KEYGROUP_ALPHA + wParam - 'a');
        else if ( wParam == 0x0D )    
            aKeyEvt.mnCode = KEY_RETURN;
        else if ( wParam == 0x1B )    
            aKeyEvt.mnCode = KEY_ESCAPE;
        else if ( wParam == 0x09 )    
            aKeyEvt.mnCode = KEY_TAB;
        else if ( wParam == 0x20 )    
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
     
    else if( nMsg == WM_UNICHAR )
     {
         
         if(wParam == UNICODE_NOCHAR)
        {
            rResult = TRUE; 
            return 1;       
        }

         SalKeyEvent aKeyEvt;
         aKeyEvt.mnCode     = nModCode; 
         aKeyEvt.mnTime     = GetMessageTime();
         aKeyEvt.mnRepeat   = 0;

        if( wParam >= Uni_SupplementaryPlanesStart )
        {
            
            
             nLastChar = 0;
             nLastVKChar = 0;
             pFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
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
     
    else
    {
        
        if ( (wParam == VK_SHIFT) || (wParam == VK_CONTROL) || (wParam == VK_MENU) )
        {
            SalKeyModEvent aModEvt;
            aModEvt.mnTime = GetMessageTime();
            aModEvt.mnCode = nModCode;
            aModEvt.mnModKeyCode = 0;   

            sal_uInt16 tmpCode = 0;
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
            sal_uInt16          nEvent;
            MSG             aCharMsg;
            BOOL        bCharPeek = FALSE;
            UINT            nCharMsg = WM_CHAR;
            sal_Bool            bKeyUp = (nMsg == WM_KEYUP) || (nMsg == WM_SYSKEYUP);

            nLastModKeyCode = 0; 
            aKeyEvt.mnCharCode = 0;
            aKeyEvt.mnCode = 0;

            aKeyEvt.mnCode = ImplSalGetKeyCode( wParam );
            if ( !bKeyUp )
            {
                
                
                
                
                
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

                if( (nModCode & (KEY_MOD1|KEY_MOD2)) == (KEY_MOD1|KEY_MOD2) &&
                    aKeyEvt.mnCharCode )
                {
                     
                     aKeyEvt.mnCode &= ~(KEY_MOD1|KEY_MOD2);
                }

                bIgnoreCharMsg = bCharPeek ? TRUE : FALSE;
                long nRet = pFrame->CallCallback( nEvent, &aKeyEvt );
                
                
                if( aKeyEvt.mnCode == KEY_HANGUL_HANJA )
                    nRet = pFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );

                bIgnoreCharMsg = FALSE;

                
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

        
        if ( GetKeyState( VK_SHIFT ) & 0x8000 )
            nModCode |= KEY_SHIFT;
        if ( GetKeyState( VK_CONTROL ) & 0x8000 )
            nModCode |= KEY_MOD1;
        if ( GetKeyState( VK_MENU ) & 0x8000 )
            nModCode |= KEY_MOD2;

        if ( (wParam != VK_SHIFT) && (wParam != VK_CONTROL) && (wParam != VK_MENU) )
        {
            SalKeyEvent     aKeyEvt;
            sal_uInt16          nEvent;
            sal_Bool            bKeyUp = (nMsg == WM_KEYUP) || (nMsg == WM_SYSKEYUP);

            
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



long ImplHandleSalObjSysCharMsg( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    sal_uInt16  nRepeat     = LOWORD( lParam )-1;
    sal_uInt16  nModCode    = 0;
    sal_uInt16  cKeyCode    = (sal_uInt16)wParam;

    
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
        nModCode |= KEY_SHIFT;
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
        nModCode |= KEY_MOD1;
    nModCode |= KEY_MOD2;

    
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



static bool ImplHandlePaintMsg( HWND hWnd )
{
    sal_Bool bMutex = FALSE;
    if ( ImplSalYieldMutexTryToAcquire() )
        bMutex = TRUE;

    
    
    

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        
        
        if ( pFrame->mpGraphics && pFrame->mpGraphics->mhRegion )
            SelectClipRgn( pFrame->mpGraphics->getHDC(), 0 );

        
        
        if ( GetUpdateRect( hWnd, NULL, FALSE ) )
        {
            
            
            RECT aUpdateRect;
            PAINTSTRUCT aPs;
            BeginPaint( hWnd, &aPs );
            CopyRect( &aUpdateRect, &aPs.rcPaint );

            
            
            if ( pFrame->mpGraphics && pFrame->mpGraphics->mhRegion )
            {
                SelectClipRgn( pFrame->mpGraphics->getHDC(),
                               pFrame->mpGraphics->mhRegion );
            }

            if ( bMutex )
            {
                SalPaintEvent aPEvt( aUpdateRect.left, aUpdateRect.top, aUpdateRect.right-aUpdateRect.left, aUpdateRect.bottom-aUpdateRect.top, pFrame->mbPresentation );
                pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
            }
            else
            {
                RECT* pRect = new RECT;
                CopyRect( pRect, &aUpdateRect );
                ImplPostMessage( hWnd, SAL_MSG_POSTPAINT, (WPARAM)pRect, 0 );
            }
            EndPaint( hWnd, &aPs );
        }
        else
        {
            
            if ( pFrame->mpGraphics && pFrame->mpGraphics->mhRegion )
            {
                SelectClipRgn( pFrame->mpGraphics->getHDC(),
                               pFrame->mpGraphics->mhRegion );
            }
        }
    }

    if ( bMutex )
        ImplSalYieldMutexRelease();

    return bMutex ? true : false;
}



static void ImplHandlePaintMsg2( HWND hWnd, RECT* pRect )
{
    
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            SalPaintEvent aPEvt( pRect->left, pRect->top, pRect->right-pRect->left, pRect->bottom-pRect->top );
            pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
        }
        ImplSalYieldMutexRelease();
        delete pRect;
    }
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTPAINT, (WPARAM)pRect, 0 );
}



static void SetMaximizedFrameGeometry( HWND hWnd, WinSalFrame* pFrame, RECT* pParentRect )
{
    

    
    

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
        
        aPt.x=aInnerRect.right;
        aPt.y=aInnerRect.top;
        ClientToScreen(hWnd, &aPt);
        pFrame->maGeometry.nRightDecoration = aRect.right - aPt.x;
    }
    if( aInnerRect.bottom ) 
        pFrame->maGeometry.nBottomDecoration += aRect.bottom - aPt.y - aInnerRect.bottom;
    else
        
        pFrame->maGeometry.nBottomDecoration = pFrame->maGeometry.nLeftDecoration;

    int nWidth  = aRect.right - aRect.left
        - pFrame->maGeometry.nRightDecoration - pFrame->maGeometry.nLeftDecoration;
    int nHeight = aRect.bottom - aRect.top
        - pFrame->maGeometry.nBottomDecoration - pFrame->maGeometry.nTopDecoration;
    
    pFrame->maGeometry.nHeight = nHeight < 0 ? 0 : nHeight;
    pFrame->maGeometry.nWidth = nWidth < 0 ? 0 : nWidth;
    pFrame->updateScreenNumber();
}



static void ImplCallMoveHdl( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SALEVENT_MOVE, 0 );
        
        
        
    }
}



static void ImplCallClosePopupsHdl( HWND hWnd )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SALEVENT_CLOSEPOPUPS, 0 );
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

            
            if ( !pFrame->mbInMoveMsg )
            {
                
                pFrame->mbInMoveMsg = TRUE;
                if ( pFrame->mbFullScreen )
                    ImplSalFrameFullScreenPos( pFrame );
                pFrame->mbInMoveMsg = FALSE;
            }

            
            ImplSaveFrameState( pFrame );

            
            
            ImplCallMoveHdl( hWnd );

        }

        ImplSalYieldMutexRelease();
    }
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTMOVE, 0, 0 );
}



static void ImplCallSizeHdl( HWND hWnd )
{
    
    
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        WinSalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->CallCallback( SALEVENT_RESIZE, 0 );
            
            if ( IsWindowVisible( hWnd ) && !pFrame->mbInShow )
                UpdateWindow( hWnd );
        }

        ImplSalYieldMutexRelease();
    }
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTCALLSIZE, 0, 0 );
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
            
            ImplSaveFrameState( pFrame );
            
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



static void ImplHandleSettingsChangeMsg( HWND hWnd, UINT nMsg,
                                         WPARAM wParam, LPARAM lParam )
{
    sal_uInt16 nSalEvent = SALEVENT_SETTINGSCHANGED;

    if ( nMsg == WM_DEVMODECHANGE )
        nSalEvent = SALEVENT_PRINTERCHANGED;
    else if ( nMsg == WM_DISPLAYCHANGE )
        nSalEvent = SALEVENT_DISPLAYCHANGED;
    else if ( nMsg == WM_FONTCHANGE )
        nSalEvent = SALEVENT_FONTCHANGED;
    else if ( nMsg == WM_TIMECHANGE )
        nSalEvent = SALEVENT_DATETIMECHANGED;
    else if ( nMsg == WM_WININICHANGE )
    {
        if ( lParam )
        {
            if ( ImplSalWICompareAscii( (const wchar_t*)lParam, "devices" ) == 0 )
                nSalEvent = SALEVENT_PRINTERCHANGED;
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

        pFrame->CallCallback( nSalEvent, 0 );
    }

    ImplSalYieldMutexRelease();
}



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
                SelectPalette( pGraphics->getHDC(), hPal, FALSE );
                if ( RealizePalette( pGraphics->getHDC() ) )
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



static LRESULT ImplHandlePalette( sal_Bool bFrame, HWND hWnd, UINT nMsg,
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

    sal_Bool bReleaseMutex = FALSE;
    if ( (nMsg == WM_QUERYNEWPALETTE) || (nMsg == WM_PALETTECHANGED) )
    {
        
        
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
    sal_Bool                bStdDC;
    sal_Bool                bUpdate;

    pSalData->mbInPalChange = TRUE;

    
    pTempVD = pSalData->mpFirstVD;
    while ( pTempVD )
    {
        pGraphics = pTempVD->mpGraphics;
        if ( pGraphics->mhDefPal )
        {
            SelectPalette( pGraphics->getHDC(),
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
            SelectPalette( pGraphics->getHDC(),
                           pGraphics->mhDefPal,
                           TRUE );
        }
        pTempFrame = pTempFrame->mpNextFrame;
    }

    
    WinSalFrame* pFrame = NULL;
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

    
    pTempVD = pSalData->mpFirstVD;
    while ( pTempVD )
    {
        pGraphics = pTempVD->mpGraphics;
        if ( pGraphics->mhDefPal )
        {
            SelectPalette( pGraphics->getHDC(), hPal, TRUE );
            RealizePalette( pGraphics->getHDC() );
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
                SelectPalette( pGraphics->getHDC(), hPal, TRUE );
                if ( RealizePalette( pGraphics->getHDC() ) )
                    bUpdate = TRUE;
            }
        }
        pTempFrame = pTempFrame->mpNextFrame;
    }

    
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

                if( nWidth > 0 && nHeight > 0 ) 
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







static WinSalMenuItem* ImplGetSalMenuItem( HMENU hMenu, UINT nPos, sal_Bool bByPosition=TRUE )
{
    MENUITEMINFOW mi;
    memset(&mi, 0, sizeof(mi));
    mi.cbSize = sizeof( mi );
    mi.fMask = MIIM_DATA;
    if( !GetMenuItemInfoW( hMenu, nPos, bByPosition, &mi) )
        ImplWriteLastError( GetLastError(), "ImplGetSalMenuItem" );

    return (WinSalMenuItem *) mi.dwItemData;
}


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
                ImplWriteLastError( GetLastError(), "ImplGetSelectedIndex" );
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
    HMENU hMenu = (HMENU) lParam;
    OUString aMnemonic( "&" + (sal_Unicode) LOWORD(wParam) );
    aMnemonic = aMnemonic.toAsciiLowerCase();   

    
    int nItemCount = GetMenuItemCount( hMenu );
    int nFound = 0;
    int idxFound = -1;
    int idxSelected = ImplGetSelectedIndex( hMenu );
    int idx = idxSelected != -1 ? idxSelected+1 : 0;    
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
                break;  
        }
    }
    if( nFound == 1 )
        nRet = MAKELRESULT( idxFound, MNC_EXECUTE );
    else
        
        nRet = MAKELRESULT( idxFound, MNC_SELECT );

    return nRet;
}

static int ImplMeasureItem( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    int nRet = 0;
    if( !wParam )
    {
        
        nRet = 1;
        MEASUREITEMSTRUCT *pMI = (LPMEASUREITEMSTRUCT) lParam;
        if( pMI->CtlType != ODT_MENU )
            return 0;

        WinSalMenuItem *pSalMenuItem = (WinSalMenuItem *) pMI->itemData;
        if( !pSalMenuItem )
            return 0;

        HDC hdc = GetDC( hWnd );
        SIZE strSize;

        NONCLIENTMETRICS ncm;
        memset( &ncm, 0, sizeof(ncm) );
        ncm.cbSize = sizeof( ncm );
        SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, (PVOID) &ncm, 0 );

        
        

        HFONT hfntOld = (HFONT) SelectObject(hdc, (HFONT) CreateFontIndirect( &ncm.lfMenuFont ));

        
        OUString aStr(pSalMenuItem->mText);
        if( pSalMenuItem->mAccelText.getLength() )
        {
            aStr += " ";
            aStr += pSalMenuItem->mAccelText;
        }
        GetTextExtentPoint32W( hdc, (LPWSTR) aStr.getStr(),
                                aStr.getLength(), &strSize );

        
        Size bmpSize( 16, 16 );
        
        

        
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
        
        nRet = 1;
        DRAWITEMSTRUCT *pDI = (LPDRAWITEMSTRUCT) lParam;
        if( pDI->CtlType != ODT_MENU )
            return 0;

        WinSalMenuItem *pSalMenuItem = (WinSalMenuItem *) pDI->itemData;
        if( !pSalMenuItem )
            return 0;

        COLORREF clrPrevText, clrPrevBkgnd;
        HFONT hfntOld;
        HBRUSH hbrOld;
        sal_Bool    fChecked = (pDI->itemState & ODS_CHECKED) ? TRUE : FALSE;
        sal_Bool    fSelected = (pDI->itemState & ODS_SELECTED) ? TRUE : FALSE;
        sal_Bool    fDisabled = (pDI->itemState & (ODS_DISABLED | ODS_GRAYED)) ? TRUE : FALSE;

        
        RECT aRect = pDI->rcItem;

        clrPrevBkgnd = SetBkColor( pDI->hDC, GetSysColor( COLOR_MENU ) );

        if ( fDisabled )
            clrPrevText = SetTextColor( pDI->hDC, GetSysColor( COLOR_GRAYTEXT ) );
        else
            clrPrevText = SetTextColor( pDI->hDC, GetSysColor( fSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

        DWORD colBackground = GetSysColor( fSelected ? COLOR_HIGHLIGHT : COLOR_MENU );
        clrPrevBkgnd = SetBkColor( pDI->hDC, colBackground );

        hbrOld = (HBRUSH)SelectObject( pDI->hDC, CreateSolidBrush( GetBkColor( pDI->hDC ) ) );

        
        if(!PatBlt( pDI->hDC, aRect.left, aRect.top, aRect.right-aRect.left, aRect.bottom-aRect.top, PATCOPY ))
            ImplWriteLastError(GetLastError(), "ImplDrawItem");

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

        
        Size bmpSize(16, 16);
        if( !!pSalMenuItem->maBitmap )
        {
            Bitmap aBitmap( pSalMenuItem->maBitmap );

            
            if( fDisabled )
                colBackground = RGB(255,255,255);
            aBitmap.Replace( Color( COL_LIGHTMAGENTA ),
                Color( GetRValue(colBackground),GetGValue(colBackground),GetBValue(colBackground) ), 0);

            WinSalBitmap* pSalBmp = static_cast<WinSalBitmap*>(aBitmap.ImplGetImpBitmap()->ImplGetSalBitmap());
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

        
        
        

        hfntOld = (HFONT) SelectObject(pDI->hDC, (HFONT) CreateFontIndirect( &ncm.lfMenuFont ));

        SIZE strSize;
        OUString aStr( pSalMenuItem->mText );
        GetTextExtentPoint32W( pDI->hDC, (LPWSTR) aStr.getStr(),
                                aStr.getLength(), &strSize );

        if(!DrawStateW( pDI->hDC, (HBRUSH)NULL, (DRAWSTATEPROC)NULL,
            (LPARAM)(LPWSTR) aStr.getStr(),
            (WPARAM)0, aRect.left, aRect.top + (lineHeight - strSize.cy)/2, 0, 0,
            DST_PREFIXTEXT | (fDisabled && !fSelected ? DSS_DISABLED : DSS_NORMAL) ) )
            ImplWriteLastError(GetLastError(), "ImplDrawItem");

        if( pSalMenuItem->mAccelText.getLength() )
        {
            SIZE strSizeA;
            aStr = pSalMenuItem->mAccelText;
            GetTextExtentPoint32W( pDI->hDC, (LPWSTR) aStr.getStr(),
                                    aStr.getLength(), &strSizeA );
            TEXTMETRIC tm;
            GetTextMetrics( pDI->hDC, &tm );

            
            
            if(!DrawStateW( pDI->hDC, (HBRUSH)NULL, (DRAWSTATEPROC)NULL,
                (LPARAM)(LPWSTR) aStr.getStr(),
                (WPARAM)0, aRect.right-strSizeA.cx-tm.tmMaxCharWidth, aRect.top + (lineHeight - strSizeA.cy)/2, 0, 0,
                DST_TEXT | (fDisabled && !fSelected ? DSS_DISABLED : DSS_NORMAL) ) )
                ImplWriteLastError(GetLastError(), "ImplDrawItem");
        }

        
        DeleteObject( SelectObject( pDI->hDC, hbrOld ) );
        DeleteObject( SelectObject( pDI->hDC, hfntOld) );
        SetTextColor(pDI->hDC, clrPrevText);
        SetBkColor(pDI->hDC, clrPrevBkgnd);
    }
    return nRet;
}

static int ImplHandleMenuActivate( HWND hWnd, WPARAM wParam, LPARAM )
{
    
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    HMENU hMenu = (HMENU) wParam;
    
    

    
    
    SalMenuEvent aMenuEvt;
    WinSalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, 0 );
    if( pSalMenuItem )
        aMenuEvt.mpMenu = pSalMenuItem->mpMenu;
    else
        aMenuEvt.mpMenu = NULL;

    long nRet = pFrame->CallCallback( SALEVENT_MENUACTIVATE, &aMenuEvt );
    if( nRet )
        nRet = pFrame->CallCallback( SALEVENT_MENUDEACTIVATE, &aMenuEvt );
    if( nRet )
        pFrame->mLastActivatedhMenu = hMenu;

    return (nRet!=0);
}

static int ImplHandleMenuSelect( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    WORD nId = LOWORD(wParam);      
    WORD nFlags = HIWORD(wParam);
    HMENU hMenu = (HMENU) lParam;

    
    if( !GetSalData()->IsKnownMenuHandle( hMenu ) )
        return 0;

    sal_Bool bByPosition = FALSE;
    if( nFlags & MF_POPUP )
        bByPosition = TRUE;

    long nRet = 0;
    if ( hMenu && !pFrame->mLastActivatedhMenu )
    {
        
        
        SalMenuEvent aMenuEvt;
        WinSalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, nId, bByPosition );
        if( pSalMenuItem )
            aMenuEvt.mpMenu = pSalMenuItem->mpMenu;
        else
            aMenuEvt.mpMenu = NULL;

        nRet = pFrame->CallCallback( SALEVENT_MENUACTIVATE, &aMenuEvt );
        if( nRet )
            nRet = pFrame->CallCallback( SALEVENT_MENUDEACTIVATE, &aMenuEvt );
        if( nRet )
            pFrame->mLastActivatedhMenu = hMenu;
    }

    if( !hMenu && nFlags == 0xFFFF )
    {
        
        pFrame->mLastActivatedhMenu = NULL;
    }

    if( hMenu )
    {
        
        
        
        pFrame->mSelectedhMenu = hMenu;

        
        if( nFlags & MF_POPUP )
        {
            
            
            MENUITEMINFOW mi;
            memset(&mi, 0, sizeof(mi));
            mi.cbSize = sizeof( mi );
            mi.fMask = MIIM_ID;
            if( GetMenuItemInfoW( hMenu, LOWORD(wParam), TRUE, &mi) )
                nId = sal::static_int_cast<WORD>(mi.wID);
        }

        SalMenuEvent aMenuEvt;
        aMenuEvt.mnId   = nId;
        WinSalMenuItem *pSalMenuItem = ImplGetSalMenuItem( hMenu, nId, FALSE );
        if( pSalMenuItem )
            aMenuEvt.mpMenu = pSalMenuItem->mpMenu;
        else
            aMenuEvt.mpMenu = NULL;

        nRet = pFrame->CallCallback( SALEVENT_MENUHIGHLIGHT, &aMenuEvt );
    }

    return (nRet != 0);
}

static int ImplHandleCommand( HWND hWnd, WPARAM wParam, LPARAM )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    long nRet = 0;
    if( !HIWORD(wParam) )
    {
        
        WORD nId = LOWORD(wParam);
        if( nId )   
        {
            SalMenuEvent aMenuEvt;
            aMenuEvt.mnId   = nId;
            WinSalMenuItem *pSalMenuItem = ImplGetSalMenuItem( pFrame->mSelectedhMenu, nId, FALSE );
            if( pSalMenuItem )
                aMenuEvt.mpMenu = pSalMenuItem->mpMenu;
            else
                aMenuEvt.mpMenu = NULL;

            nRet = pFrame->CallCallback( SALEVENT_MENUCOMMAND, &aMenuEvt );
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
        
        
        if( GetMenu( hWnd ) )
            return FALSE;

        
        
        
        if ( !LOWORD( lParam ) )
        {
            
            
            
            
            
            if ( GetKeyState( VK_SPACE ) & 0x8000 )
                return 0;

            
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
            
            HWND hFocusWnd = ::GetFocus();
            if ( hFocusWnd && ImplFindSalObject( hFocusWnd ) )
            {
                char cKeyCode = (char)(unsigned char)LOWORD( lParam );
                
                if ( (cKeyCode >= 65) && (cKeyCode <= 90) )
                    cKeyCode += 32;
                
                
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
                    long nRet = pFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
                    pFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
                    return (nRet != 0);
                }
            }
        }
    }

    return FALSE;
}



static void ImplHandleInputLangChange( HWND hWnd, WPARAM, LPARAM lParam )
{
    ImplSalYieldMutexAcquireWithWait();

    
    WinSalFrame* pFrame = GetWindowPtr( hWnd );

    if ( !pFrame )
        return;

    if ( pFrame->mbIME && pFrame->mhDefIMEContext )
    {
        HKL     hKL = (HKL)lParam;
        UINT    nImeProps = ImmGetProperty( hKL, IGP_PROPERTY );

        pFrame->mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
        pFrame->mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
        pFrame->mbHandleIME = !pFrame->mbSpezIME;
    }

    
    UINT nLang = pFrame->mnInputLang;
    ImplUpdateInputLang( pFrame );

    
    if( nLang != pFrame->mnInputLang )
        pFrame->CallCallback( SALEVENT_INPUTLANGUAGECHANGE, 0 );

    ImplSalYieldMutexRelease();
}



static void ImplUpdateIMECursorPos( WinSalFrame* pFrame, HIMC hIMC )
{
    COMPOSITIONFORM aForm;
    memset( &aForm, 0, sizeof( aForm ) );

    
    
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

    
    
    if ( GetFocus() == pFrame->mhWnd )
    {
        CreateCaret( pFrame->mhWnd, 0,
                     aPosEvt.mnWidth, aPosEvt.mnHeight );
        SetCaretPos( aPosEvt.mnX, aPosEvt.mnY );
    }
}



static sal_Bool ImplHandleIMEStartComposition( HWND hWnd )
{
    sal_Bool bDef = TRUE;

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



static sal_Bool ImplHandleIMECompositionInput( WinSalFrame* pFrame,
                                               HIMC hIMC, LPARAM lParam )
{
    sal_Bool bDef = TRUE;

    
    SalExtTextInputEvent    aEvt;
    aEvt.mnTime             = GetMessageTime();
    aEvt.mpTextAttr         = NULL;
    aEvt.mnCursorPos        = 0;
    aEvt.mbOnlyCursor       = FALSE;
    aEvt.mnCursorFlags      = 0;

    
    if ( lParam & GCS_RESULTSTR )
    {
        bDef = FALSE;

        LONG nTextLen = ImmGetCompositionStringW( hIMC, GCS_RESULTSTR, 0, 0 ) / sizeof( WCHAR );
        if ( nTextLen >= 0 )
        {
            WCHAR* pTextBuf = new WCHAR[nTextLen];
            ImmGetCompositionStringW( hIMC, GCS_RESULTSTR, pTextBuf, nTextLen*sizeof( WCHAR ) );
            aEvt.maText = OUString( reinterpret_cast<const sal_Unicode*>(pTextBuf), (sal_Int32)nTextLen );
            delete [] pTextBuf;
        }

        aEvt.mnCursorPos = aEvt.maText.getLength();
        pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
        pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
        ImplUpdateIMECursorPos( pFrame, hIMC );
    }

    
    if ( !pFrame->mbAtCursorIME )
        return !bDef;

    
    if ( (lParam & (GCS_COMPSTR | GCS_COMPATTR)) ||
         ((lParam & GCS_CURSORPOS) && !(lParam & GCS_RESULTSTR)) )
    {
        bDef = FALSE;

        sal_uInt16* pSalAttrAry = NULL;
        LONG    nTextLen = ImmGetCompositionStringW( hIMC, GCS_COMPSTR, 0, 0 ) / sizeof( WCHAR );
        if ( nTextLen > 0 )
        {
            WCHAR* pTextBuf = new WCHAR[nTextLen];
            ImmGetCompositionStringW( hIMC, GCS_COMPSTR, pTextBuf, nTextLen*sizeof( WCHAR ) );
            aEvt.maText = OUString( reinterpret_cast<const sal_Unicode*>(pTextBuf), (sal_Int32)nTextLen );
            delete [] pTextBuf;

            BYTE*   pAttrBuf = NULL;
            LONG        nAttrLen = ImmGetCompositionStringW( hIMC, GCS_COMPATTR, 0, 0 );
            if ( nAttrLen > 0 )
            {
                pAttrBuf = new BYTE[nAttrLen];
                ImmGetCompositionStringW( hIMC, GCS_COMPATTR, pAttrBuf, nAttrLen );
            }

            if ( pAttrBuf )
            {
                sal_Int32 nTextLen2 = aEvt.maText.getLength();
                pSalAttrAry = new sal_uInt16[nTextLen2];
                memset( pSalAttrAry, 0, nTextLen2*sizeof( sal_uInt16 ) );
                for( sal_Int32 i = 0; (i < nTextLen2) && (i < nAttrLen); i++ )
                {
                    BYTE nWinAttr = pAttrBuf[i];
                    sal_uInt16   nSalAttr;
                    if ( nWinAttr == ATTR_TARGET_CONVERTED )
                    {
                        nSalAttr = EXTTEXTINPUT_ATTR_BOLDUNDERLINE;
                        aEvt.mnCursorFlags |= EXTTEXTINPUT_CURSOR_INVISIBLE;
                    }
                    else if ( nWinAttr == ATTR_CONVERTED )
                        nSalAttr = EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE;
                    else if ( nWinAttr == ATTR_TARGET_NOTCONVERTED )
                        nSalAttr = EXTTEXTINPUT_ATTR_HIGHLIGHT;
                    else if ( nWinAttr == ATTR_INPUT_ERROR )
                        nSalAttr = EXTTEXTINPUT_ATTR_REDTEXT | EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                    else /* ( nWinAttr == ATTR_INPUT ) */
                        nSalAttr = EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                    pSalAttrAry[i] = nSalAttr;
                }

                aEvt.mpTextAttr = pSalAttrAry;
                delete [] pAttrBuf;
            }
        }

        
        if ( (nTextLen > 0) || !(lParam & GCS_RESULTSTR) )
        {
            
            if ( !nTextLen && !pFrame->mbCandidateMode )
            {
                pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
                pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
            }
            else
            {
                
                
                if ( lParam & CS_INSERTCHAR )
                {
                    aEvt.mnCursorPos = nTextLen;
                    if ( aEvt.mnCursorPos && (lParam & CS_NOMOVECARET) )
                        aEvt.mnCursorPos--;
                }
                else
                    aEvt.mnCursorPos = LOWORD( ImmGetCompositionStringW( hIMC, GCS_CURSORPOS, 0, 0 ) );

                if ( pFrame->mbCandidateMode )
                    aEvt.mnCursorFlags |= EXTTEXTINPUT_CURSOR_INVISIBLE;
                if ( lParam & CS_NOMOVECARET )
                    aEvt.mnCursorFlags |= EXTTEXTINPUT_CURSOR_OVERWRITE;

                pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
            }
            ImplUpdateIMECursorPos( pFrame, hIMC );
        }

        if ( pSalAttrAry )
            delete [] pSalAttrAry;
    }

    return !bDef;
}



static sal_Bool ImplHandleIMEComposition( HWND hWnd, LPARAM lParam )
{
    sal_Bool bDef = TRUE;
    ImplSalYieldMutexAcquireWithWait();

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame && (!lParam || (lParam & GCS_RESULTSTR)) )
    {
        
        
        if ( pFrame->mpGraphics &&
             pFrame->mpGraphics->getHDC() )
            SetBkMode( pFrame->mpGraphics->getHDC(), TRANSPARENT );
    }

    if ( pFrame && pFrame->mbHandleIME )
    {
        if ( !lParam )
        {
            SalExtTextInputEvent aEvt;
            aEvt.mnTime             = GetMessageTime();
            aEvt.mpTextAttr         = NULL;
            aEvt.mnCursorPos        = 0;
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



static sal_Bool ImplHandleIMEEndComposition( HWND hWnd )
{
    sal_Bool bDef = TRUE;

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



static boolean ImplHandleAppCommand( HWND hWnd, LPARAM lParam )
{
    sal_Int16 nCommand = 0;
    switch( GET_APPCOMMAND_LPARAM(lParam) )
    {
    case APPCOMMAND_MEDIA_CHANNEL_DOWN:         nCommand = MEDIA_COMMAND_CHANNEL_DOWN; break;
    case APPCOMMAND_MEDIA_CHANNEL_UP:           nCommand = MEDIA_COMMAND_CHANNEL_UP; break;
    case APPCOMMAND_MEDIA_NEXTTRACK:            nCommand = MEDIA_COMMAND_NEXTTRACK; break;
    case APPCOMMAND_MEDIA_PAUSE:                nCommand = MEDIA_COMMAND_PAUSE; break;
    case APPCOMMAND_MEDIA_PLAY:                 nCommand = MEDIA_COMMAND_PLAY; break;
    case APPCOMMAND_MEDIA_PLAY_PAUSE:           nCommand = MEDIA_COMMAND_PLAY_PAUSE; break;
    case APPCOMMAND_MEDIA_PREVIOUSTRACK:        nCommand = MEDIA_COMMAND_PREVIOUSTRACK; break;
    case APPCOMMAND_MEDIA_RECORD:               nCommand = MEDIA_COMMAND_RECORD; break;
    case APPCOMMAND_MEDIA_REWIND:               nCommand = MEDIA_COMMAND_REWIND; break;
    case APPCOMMAND_MEDIA_STOP:                 nCommand = MEDIA_COMMAND_STOP; break;
    case APPCOMMAND_MIC_ON_OFF_TOGGLE:          nCommand = MEDIA_COMMAND_MIC_ON_OFF_TOGGLE; break;
    case APPCOMMAND_MICROPHONE_VOLUME_DOWN:     nCommand = MEDIA_COMMAND_MICROPHONE_VOLUME_DOWN; break;
    case APPCOMMAND_MICROPHONE_VOLUME_MUTE:     nCommand = MEDIA_COMMAND_MICROPHONE_VOLUME_MUTE; break;
    case APPCOMMAND_MICROPHONE_VOLUME_UP:       nCommand = MEDIA_COMMAND_MICROPHONE_VOLUME_UP; break;
    case APPCOMMAND_VOLUME_DOWN:                nCommand = MEDIA_COMMAND_VOLUME_DOWN; break;
    case APPCOMMAND_VOLUME_MUTE:                nCommand = MEDIA_COMMAND_VOLUME_MUTE; break;
    case APPCOMMAND_VOLUME_UP:                  nCommand = MEDIA_COMMAND_VOLUME_UP; break;
    default:
        return false;
    }

    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    Window *pWindow = pFrame ? pFrame->GetWindow() : NULL;

    if( pWindow )
    {
        const Point aPoint;
        CommandEvent aCEvt( aPoint, COMMAND_MEDIA, FALSE, &nCommand );
        NotifyEvent aNCmdEvt( EVENT_COMMAND, pWindow, &aCEvt );

        if ( !ImplCallPreNotify( aNCmdEvt ) )
        {
            pWindow->Command( aCEvt );
            return true;
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
            
            pFrame->mbCandidateMode = TRUE;
            ImplHandleIMEComposition( hWnd, GCS_CURSORPOS );

            HWND hWnd2 = pFrame->mhWnd;
            HIMC hIMC = ImmGetContext( hWnd2 );
            if ( hIMC )
            {
                LONG nBufLen = ImmGetCompositionStringW( hIMC, GCS_COMPSTR, 0, 0 );
                if ( nBufLen >= 1 )
                {
                    SalExtTextInputPosEvent aPosEvt;
                    pFrame->CallCallback( SALEVENT_EXTTEXTINPUTPOS, (void*)&aPosEvt );

                    
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
    
    AllSettings aSettings = Application::GetSettings();
    MiscSettings aMisc = aSettings.GetMiscSettings();
    aMisc.SetEnableATToolSupport( sal_True );
    aSettings.SetMiscSettings( aMisc );
    Application::SetSettings( aSettings );

    if (!Application::GetSettings().GetMiscSettings().GetEnableATToolSupport())
        return false; 

    ImplSVData* pSVData = ImplGetSVData();

    
    
    if ( !pSVData->mxAccessBridge.is() )
    {
        if( !InitAccessBridge() )
            return false;
    }

    uno::Reference< accessibility::XMSAAService > xMSAA( pSVData->mxAccessBridge, uno::UNO_QUERY );
    if ( xMSAA.is() )
    {
        
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
    LPRECONVERTSTRING pReconvertString = (LPRECONVERTSTRING) lParam;
    LRESULT nRet = 0;
    SalSurroundingTextRequestEvent aEvt;
    aEvt.maText = OUString();
    aEvt.mnStart = aEvt.mnEnd = 0;

    UINT nImeProps = ImmGetProperty( GetKeyboardLayout( 0 ), IGP_SETCOMPSTR );
    if( (nImeProps & SCS_CAP_SETRECONVERTSTRING) == 0 )
    {
    
    return 0;
    }

    if( !pReconvertString )
    {
    
    pFrame->CallCallback( SALEVENT_STARTRECONVERSION, (void*)NULL );

    
    pFrame->CallCallback( SALEVENT_SURROUNDINGTEXTREQUEST, (void*)&aEvt );

    if( aEvt.maText.isEmpty())
    {
        return 0;
    }

    nRet = sizeof(RECONVERTSTRING) + (aEvt.maText.getLength() + 1) * sizeof(WCHAR);
    }
    else
    {
    

    
    pFrame->CallCallback( SALEVENT_SURROUNDINGTEXTREQUEST, (void*)&aEvt );
    nRet = sizeof(RECONVERTSTRING) + (aEvt.maText.getLength() + 1) * sizeof(WCHAR);

    pReconvertString->dwStrOffset = sizeof(RECONVERTSTRING);
    pReconvertString->dwStrLen = aEvt.maText.getLength();
    pReconvertString->dwCompStrOffset = aEvt.mnStart * sizeof(WCHAR);
    pReconvertString->dwCompStrLen = aEvt.mnEnd - aEvt.mnStart;
    pReconvertString->dwTargetStrOffset = pReconvertString->dwCompStrOffset;
    pReconvertString->dwTargetStrLen = pReconvertString->dwCompStrLen;

    memcpy( (LPWSTR)(pReconvertString + 1), aEvt.maText.getStr(), (aEvt.maText.getLength() + 1) * sizeof(WCHAR) );
    }

    
    return nRet;
}



static LRESULT ImplHandleIMEConfirmReconvertString( HWND hWnd, LPARAM lParam )
{
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    LPRECONVERTSTRING pReconvertString = (LPRECONVERTSTRING) lParam;
    SalSurroundingTextRequestEvent aEvt;
    aEvt.maText = OUString();
    aEvt.mnStart = aEvt.mnEnd = 0;

    pFrame->CallCallback( SALEVENT_SURROUNDINGTEXTREQUEST, (void*)&aEvt );

    sal_uLong nTmpStart = pReconvertString->dwCompStrOffset / sizeof(WCHAR);
    sal_uLong nTmpEnd = nTmpStart + pReconvertString->dwCompStrLen;

    if( nTmpStart != aEvt.mnStart || nTmpEnd != aEvt.mnEnd )
    {
    SalSurroundingTextSelectionChangeEvent aSelEvt;
    aSelEvt.mnStart = nTmpStart;
    aSelEvt.mnEnd = nTmpEnd;

    pFrame->CallCallback( SALEVENT_SURROUNDINGTEXTSELECTIONCHANGE, (void*)&aSelEvt );
    }

    return TRUE;
}

static LRESULT ImplHandleIMEQueryCharPosition( HWND hWnd, LPARAM lParam ) {
    WinSalFrame* pFrame = GetWindowPtr( hWnd );
    PIMECHARPOSITION pQueryCharPosition = (PIMECHARPOSITION) lParam;
    if ( pQueryCharPosition->dwSize < sizeof(IMECHARPOSITION) )
        return FALSE;

    SalQueryCharPositionEvent aEvt;
    aEvt.mbValid = false;
    aEvt.mnCharPos = pQueryCharPosition->dwCharPos;

    pFrame->CallCallback( SALEVENT_QUERYCHARPOSITION, (void*)&aEvt );

    if ( !aEvt.mbValid )
        return FALSE;

    if ( aEvt.mbVertical )
    {
        
        
        pQueryCharPosition->pt.x = aEvt.mnCursorBoundX + aEvt.mnCursorBoundWidth;
        pQueryCharPosition->pt.y = aEvt.mnCursorBoundY;
        pQueryCharPosition->cLineHeight = aEvt.mnCursorBoundWidth;
    }
    else
    {
        
        
        pQueryCharPosition->pt.x = aEvt.mnCursorBoundX;
        pQueryCharPosition->pt.y = aEvt.mnCursorBoundY;
        pQueryCharPosition->cLineHeight = aEvt.mnCursorBoundHeight;
    }

    
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
            ImplSendMessage( pSalData->mhWantLeaveMsg, SAL_MSG_MOUSELEAVE, 0, MAKELPARAM( aPt.x, aPt.y ) );
    }
}



static int ImplSalWheelMousePos( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam ,
                                 LRESULT& rResult )
{
    POINT aPt;
    POINT aScreenPt;
    aScreenPt.x = (short)LOWORD( lParam );
    aScreenPt.y = (short)HIWORD( lParam );
    
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



LRESULT CALLBACK SalFrameWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT     nRet = 0;
    static int  bInWheelMsg = FALSE;
    static int  bInQueryEnd = FALSE;

    
    if ( nMsg == WM_CREATE )
    {
        
        
        
        CREATESTRUCTA* pStruct = (CREATESTRUCTA*)lParam;
        WinSalFrame* pFrame = (WinSalFrame*)pStruct->lpCreateParams;
        if ( pFrame != 0 )
        {
            SetWindowPtr( hWnd, pFrame );
            
            
            pFrame->mhWnd = hWnd;
            pFrame->maSysData.hWnd = hWnd;
        }
        return 0;
    }

    ImplSVData* pSVData = ImplGetSVData();
    
    
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
            rDef = !ImplHandleMouseMsg( hWnd, nMsg, wParam, lParam );
            ImplSalYieldMutexRelease();
            break;

        case WM_NCLBUTTONDOWN:
        case WM_NCMBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
            ImplSalYieldMutexAcquireWithWait();
            ImplCallClosePopupsHdl( hWnd );   
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
        case WM_UNICHAR:    
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
            ImplSalYieldMutexAcquireWithWait();
            rDef = !ImplHandleKeyMsg( hWnd, nMsg, wParam, lParam, nRet );
            ImplSalYieldMutexRelease();
            break;

        case WM_MOUSEWHEEL:
            
        case WM_MOUSEHWHEEL:
            
            
            if ( !bInWheelMsg )
            {
                bInWheelMsg++;
                rDef = !ImplHandleWheelMsg( hWnd, nMsg, wParam, lParam );
                
                
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
            bCheckTimers = ImplHandlePaintMsg( hWnd );
            rDef = FALSE;
            break;
        case SAL_MSG_POSTPAINT:
            ImplHandlePaintMsg2( hWnd, (RECT*)wParam );
            bCheckTimers = true;
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
            
            
            
            
            
            if ( LOWORD( wParam ) != WA_INACTIVE )
                ImplSendMessage( hWnd, SAL_MSG_FORCEPALETTE, 0, 0 );
            break;

        case WM_ENABLE:
            
            {
                WinSalFrame* pFrame = GetWindowPtr( hWnd );
                Window *pWin = NULL;
                if( pFrame )
                    pWin = pFrame->GetWindow();

                if( !wParam )
                {
                    pSVData->maAppData.mnModalMode++;

                    ImplHideSplash();
                    if( pWin )
                    {
                        pWin->EnableInput( FALSE, TRUE, TRUE, NULL );
                        pWin->ImplIncModalCount();  
                    }
                }
                else
                {
                    ImplGetSVData()->maAppData.mnModalMode--;
                    if( pWin )
                    {
                        pWin->EnableInput( TRUE, TRUE, TRUE, NULL );
                        pWin->ImplDecModalCount();  
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
                
                bInQueryEnd = TRUE;
                nRet = !ImplHandleShutDownMsg( hWnd );
                rDef = FALSE;

                
                
                
                
                

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
                bInQueryEnd = FALSE; 
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
            GetSalData()->mbThemeChanged = TRUE;
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
            ImplSalToTop( hWnd, (sal_uInt16)wParam );
            rDef = FALSE;
            break;
        case SAL_MSG_SHOW:
            ImplSalShow( hWnd, (sal_Bool)wParam, (sal_Bool)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_SETINPUTCONTEXT:
            ImplSalFrameSetInputContext( hWnd, (const SalInputContext*)(void*)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_ENDEXTTEXTINPUT:
            ImplSalFrameEndExtTextInput( hWnd, (sal_uInt16)(sal_uLong)(void*)wParam );
            rDef = FALSE;
            break;

        case WM_INPUTLANGCHANGE:
            ImplHandleInputLangChange( hWnd, wParam, lParam );
            break;

        case WM_IME_CHAR:
            
            
            
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

        case WM_GETOBJECT:
            ImplSalYieldMutexAcquireWithWait();
            if ( ImplHandleGetObject( hWnd, lParam, wParam, nRet ) )
            {
                rDef = false;
            }
            ImplSalYieldMutexRelease();
            break;

        case WM_APPCOMMAND:
            if( ImplHandleAppCommand( hWnd, lParam ) )
            {
                rDef = false;
                nRet = 1;
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
                nRet = ImplHandleIMEQueryCharPosition( hWnd, lParam );
                rDef = FALSE;
            }
            break;
    }

    
    if ( rDef && (nMsg == aSalShlData.mnWheelMsgId) && aSalShlData.mnWheelMsgId )
    {
        
        
        if ( !bInWheelMsg )
        {
            bInWheelMsg++;
            
            WORD nKeyState = 0;
            if ( GetKeyState( VK_SHIFT ) & 0x8000 )
                nKeyState |= MK_SHIFT;
            if ( GetKeyState( VK_CONTROL ) & 0x8000 )
                nKeyState |= MK_CONTROL;
            
            rDef = !ImplHandleWheelMsg( hWnd,
                                        WM_MOUSEWHEEL,
                                        MAKEWPARAM( nKeyState, (WORD)wParam ),
                                        lParam );
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

    if( bCheckTimers )
    {
        SalData* pSalData = GetSalData();
        if( pSalData->mnNextTimerTime )
        {
            DWORD nCurTime = GetTickCount();
            if( pSalData->mnNextTimerTime < nCurTime )
            {
                MSG aMsg;
                if( ! ImplPeekMessage( &aMsg, 0, WM_PAINT, WM_PAINT, PM_NOREMOVE | PM_NOYIELD ) )
                    ImplPostMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_POSTTIMER, 0, nCurTime );
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



sal_Bool ImplHandleGlobalMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& rlResult )
{
    
    
    sal_Bool bResult = FALSE;
    if ( (nMsg == WM_PALETTECHANGED) || (nMsg == SAL_MSG_POSTPALCHANGED) )
    {
        int bDef = TRUE;
        rlResult = ImplHandlePalette( FALSE, hWnd, nMsg, wParam, lParam, bDef );
        bResult = (bDef != 0);
    }
    else if( nMsg == WM_DISPLAYCHANGE )
    {
        WinSalSystem* pSys = static_cast<WinSalSystem*>(ImplGetSalSystem());
        if( pSys )
            pSys->clearMonitors();
        bResult = (pSys != NULL);
    }
    return bResult;
}



sal_Bool ImplWriteLastError( DWORD lastError, const char *szApiCall )
{
    static int first=1;
    
    
    static char *logEnabled = getenv("VCL_LOGFILE_ENABLED");
    if( logEnabled )
    {
        sal_Bool bSuccess = FALSE;
        static char *szTmp = getenv("TMP");
        if( !szTmp || !*szTmp )
            szTmp = getenv("TEMP");
        if( szTmp && *szTmp )
        {
            char fname[5000];
            strcpy( fname, szTmp );
            if( fname[strlen(fname) - 1] != '\\' )
                strcat( fname, "\\");
            strcat( fname, "vcl.log" );
            FILE *fp = fopen( fname, "a" ); 
            if( fp )
            {
                if( first )
                {
                    first = 0;
                    fprintf( fp, "Process ID: %ld (0x%lx)\n", GetCurrentProcessId(), GetCurrentProcessId() );
                }
                time_t aclock;
                time( &aclock );                           
                struct tm *newtime = localtime( &aclock ); 
                fprintf( fp, asctime( newtime ) );         

                fprintf( fp, "%s returned %lu (0x%lx)\n", szApiCall, lastError, lastError );
                bSuccess = TRUE;    

                LPVOID lpMsgBuf;
                if (FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    lastError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                    (LPSTR) &lpMsgBuf,
                    0,
                    NULL ))
                {
                    fprintf( fp, " %s\n", (LPSTR)lpMsgBuf );
                    LocalFree( lpMsgBuf );
                }

                fclose( fp );
            }
        }
        return bSuccess;
    }
    else
        return TRUE;
}



#ifdef _WIN32
bool HasAtHook()
{
    BOOL bIsRunning = FALSE;
    
    return SystemParametersInfo(SPI_GETSCREENREADER, 0, &bIsRunning, 0)
        && bIsRunning;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
