/*************************************************************************
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-01 03:12:46 $
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

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define private public
#ifndef _SV_SALAQUA_HXX
#include <salaqua.hxx>
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
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
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
#ifndef _SV_VCLWINDOW_H
#include <VCLWindow.h>
#endif

// =======================================================================

// Wegen Fehler in Windows-Headerfiles
#ifndef IMN_OPENCANDIDATE
#define IMN_OPENCANDIDATE               0x0005
#endif
#ifndef IMN_CLOSECANDIDATE
#define IMN_CLOSECANDIDATE              0x0004
#endif

// =======================================================================

static void ImplSaveFrameState( SalFrame* pFrame )
{
#ifdef WIN
    // Position, Groesse und Status fuer GetWindowState() merken
    if ( !pFrame->maFrameData.mbFullScreen )
    {
        BOOL bVisible = (GetWindowStyle( pFrame->maFrameData.mhWnd ) & WS_VISIBLE) != 0;
        if ( IsIconic( pFrame->maFrameData.mhWnd ) )
        {
            pFrame->maFrameData.maState.mnState |= SAL_FRAMESTATE_MINIMIZED;
            if ( bVisible )
                pFrame->maFrameData.mnShowState = SW_SHOWMAXIMIZED;
        }
        else if ( IsZoomed( pFrame->maFrameData.mhWnd ) )
        {
            pFrame->maFrameData.maState.mnState &= ~SAL_FRAMESTATE_MINIMIZED;
            pFrame->maFrameData.maState.mnState |= SAL_FRAMESTATE_MAXIMIZED;
            if ( bVisible )
                pFrame->maFrameData.mnShowState = SW_SHOWMAXIMIZED;
            pFrame->maFrameData.mbRestoreMaximize = TRUE;
        }
        else
        {
            RECT aRect;
            GetWindowRect( pFrame->maFrameData.mhWnd, &aRect );
            pFrame->maFrameData.maState.mnState &= ~(SAL_FRAMESTATE_MINIMIZED | SAL_FRAMESTATE_MAXIMIZED);
            pFrame->maFrameData.maState.mnX      = aRect.left;
            pFrame->maFrameData.maState.mnY      = aRect.top;
            pFrame->maFrameData.maState.mnWidth  = aRect.right-aRect.left;
            pFrame->maFrameData.maState.mnHeight = aRect.bottom-aRect.top;
            if ( bVisible )
                pFrame->maFrameData.mnShowState = SW_SHOWNORMAL;
            pFrame->maFrameData.mbRestoreMaximize = FALSE;
        }
    }
#endif
}

// =======================================================================

SalFrame* ImplSalCreateFrame( SalInstance* pInst,
                              VCLWINDOW hWndParent, ULONG nSalFrameStyle )
{
    SalFrame*   pFrame = new SalFrame;
#ifdef WIN
    VCLWINDOW       hWnd;
    DWORD       nSysStyle = 0;
    DWORD       nExSysStyle = 0;
    BOOL        bSaveBits = FALSE;

    // determine creation data
    if ( nSalFrameStyle & SAL_FRAME_STYLE_CHILD )
        nSysStyle |= WS_CHILD;
    else if ( nSalFrameStyle & SAL_FRAME_STYLE_DEFAULT )
    {
        pFrame->maFrameData.mbCaption = TRUE;
        nSysStyle |= WS_OVERLAPPED;
        nExSysStyle |= WS_EX_APPWINDOW;
    }
    else
        nSysStyle |= WS_POPUP;
    if ( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE )
    {
        pFrame->maFrameData.mbSizeBorder = TRUE;
        nSysStyle |= WS_THICKFRAME | WS_SYSMENU;
    }
    else if ( nSalFrameStyle & SAL_FRAME_STYLE_BORDER )
    {
        pFrame->maFrameData.mbBorder = TRUE;
        nSysStyle |= WS_BORDER;
    }
    if ( nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
    {
        pFrame->maFrameData.mbCaption = TRUE;
        nSysStyle |= WS_CAPTION | WS_SYSMENU;
    }
    else
        nExSysStyle |= WS_EX_TOOLWINDOW;
    if ( nSalFrameStyle & SAL_FRAME_STYLE_MINABLE )
        nSysStyle |= WS_MINIMIZEBOX | WS_SYSMENU;
    if ( nSalFrameStyle & SAL_FRAME_STYLE_MAXABLE )
        nSysStyle |= WS_MAXIMIZEBOX | WS_SYSMENU;

    // init frame data
    pFrame->maFrameData.mnStyle = nSalFrameStyle;

    // determine show style
    if ( nSalFrameStyle & SAL_FRAME_STYLE_DEFAULT )
    {
        SalData* pSalData = GetSalData();
        pFrame->maFrameData.mnShowState = pSalData->mnCmdShow;
        if ( (pFrame->maFrameData.mnShowState != SW_SHOWMINIMIZED) &&
             (pFrame->maFrameData.mnShowState != SW_MINIMIZE) &&
             (pFrame->maFrameData.mnShowState != SW_SHOWMINNOACTIVE) )
        {
            if ( (pFrame->maFrameData.mnShowState == SW_SHOWMAXIMIZED) ||
                 (pFrame->maFrameData.mnShowState == SW_MAXIMIZE) )
                pFrame->maFrameData.mbOverwriteState = FALSE;
            pFrame->maFrameData.mnShowState = SW_SHOWMAXIMIZED;
        }
        else
            pFrame->maFrameData.mbOverwriteState = FALSE;
    }
    else
        pFrame->maFrameData.mnShowState = SW_SHOWNORMAL;

    // create frame
    LPCSTR pClassName;
    if ( bSaveBits )
        pClassName = SAL_FRAME_CLASSNAME_SBA;
    else
        pClassName = SAL_FRAME_CLASSNAMEA;
    hWnd = CreateWindowExA( nExSysStyle, pClassName, "", nSysStyle,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        hWndParent, 0, pInst->maInstData.mhInst, (void*)pFrame );
    }
    if ( !hWnd )
    {
        delete pFrame;
        return NULL;
    }

    // disable close
    if ( !(nSalFrameStyle & SAL_FRAME_STYLE_CLOSEABLE) )
    {
        HMENU hSysMenu = GetSystemMenu( hWnd, FALSE );
        if ( hSysMenu )
            EnableMenuItem( hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
    }

    // reset input context
#ifdef WIN
    pFrame->maFrameData.mhDefIMEContext = ImmAssociateContext( hWnd, 0 );
#endif

    // determine output size and state
    RECT aRect;
    GetClientRect( hWnd, &aRect );
    pFrame->maFrameData.mnWidth  = aRect.right;
    pFrame->maFrameData.mnHeight = aRect.bottom;
    ImplSaveFrameState( pFrame );
    pFrame->maFrameData.mbDefPos = TRUE;

    // CreateVCLVIEW in the main thread
    pFrame->ReleaseGraphics( pFrame->GetGraphics() );
#endif

    return pFrame;
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

long ImplSalCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
    return 0;
}

// -----------------------------------------------------------------------

static UINT ImplSalGetWheelScrollLines()
{
    UINT nScrLines = 0;
#ifdef WIN
    VCLWINDOW hWndMsWheel = WIN_FindWindow( MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE );
    if ( hWndMsWheel )
    {
        UINT nGetScrollLinesMsgId = RegisterWindowMessage( MSH_SCROLL_LINES );
        nScrLines = (UINT)ImplSendMessage( hWndMsWheel, nGetScrollLinesMsgId, 0, 0 );
    }

    if ( !nScrLines )
        nScrLines = SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &nScrLines, 0 );

    if ( !nScrLines )
        nScrLines = 3;
#endif

    return nScrLines;
}

// -----------------------------------------------------------------------

static void ImplSalCalcFullScreenSize( const SalFrame* pFrame,
    int& rX, int& rY, int& rDX, int& rDY )
{
#ifdef WIN
    // set window to screen size
    int nFrameX;
    int nFrameY;
    int nCaptionY;
    int nScreenDX;
    int nScreenDY;

    if ( pFrame->maFrameData.mbSizeBorder )
    {
        nFrameX = GetSystemMetrics( SM_CXFRAME );
        nFrameY = GetSystemMetrics( SM_CYFRAME );
    }
    else if ( pFrame->maFrameData.mbBorder )
    {
        nFrameX = GetSystemMetrics( SM_CXBORDER );
        nFrameY = GetSystemMetrics( SM_CYBORDER );
    }
    else
    {
        nFrameX = 0;
        nFrameY = 0;
    }
    if ( pFrame->maFrameData.mbCaption )
        nCaptionY = GetSystemMetrics( SM_CYCAPTION );
    else
        nCaptionY = 0;

    nScreenDX   = GetSystemMetrics( SM_CXSCREEN );
    nScreenDY   = GetSystemMetrics( SM_CYSCREEN );

    rX  = -nFrameX;
    rY  = -(nFrameY+nCaptionY);
    rDX = nScreenDX+(nFrameX*2);
    rDY = nScreenDY+(nFrameY*2)+nCaptionY;
#endif
}

// -----------------------------------------------------------------------

static void ImplSalFrameFullScreenPos( SalFrame* pFrame, BOOL bAlways = FALSE )
{
#ifdef WIN
    if ( bAlways || !IsIconic( pFrame->maFrameData.mhWnd ) )
    {
        // set window to screen size
        int nX;
        int nY;
        int nWidth;
        int nHeight;
        ImplSalCalcFullScreenSize( pFrame, nX, nY, nWidth, nHeight );
        SetWindowPos( pFrame->maFrameData.mhWnd, 0,
                      nX, nY, nWidth, nHeight,
                      SWP_NOZORDER | SWP_NOACTIVATE );
    }
#endif
}

// -----------------------------------------------------------------------

SalFrame::SalFrame()
{
    SalData* pSalData = GetSalData();

    maFrameData.mhWnd               = 0;
#ifdef WIN
    maFrameData.mhCursor            = LoadCursor( 0, IDC_ARROW );
#endif
    maFrameData.mhDefIMEContext     = 0;
    maFrameData.mpGraphics          = NULL;
    maFrameData.mpInst              = NULL;
    maFrameData.mpProc              = ImplSalCallbackDummy;
    maFrameData.mnInputLang         = 0;
    maFrameData.mnInputCodePage     = 0;
    maFrameData.mbGraphics          = FALSE;
    maFrameData.mbCaption           = FALSE;
    maFrameData.mbBorder            = FALSE;
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
    maFrameData.mbCompositionMode   = FALSE;
    maFrameData.mbCandidateMode     = FALSE;
    memset( &maFrameData.maState, 0, sizeof( SalFrameState ) );
    maFrameData.maSysData.nSize     = sizeof( SystemEnvData );

    // Daten ermitteln, wenn erster Frame angelegt wird
    if ( !pSalData->mpFirstFrame )
    {
#ifdef WIN
        if ( !aSalShlData.mnWheelMsgId )
            aSalShlData.mnWheelMsgId = RegisterWindowMessage( MSH_MOUSEWHEEL );
#endif
        if ( !aSalShlData.mnWheelScrollLines )
            aSalShlData.mnWheelScrollLines = ImplSalGetWheelScrollLines();
    }

    // insert frame in framelist
    maFrameData.mpNextFrame = pSalData->mpFirstFrame;
    pSalData->mpFirstFrame = this;
}

// -----------------------------------------------------------------------

SalFrame::~SalFrame()
{
    SalData* pSalData = GetSalData();

    // destroy saved DC
    if ( maFrameData.mpGraphics )
    {
#ifdef WIN
        if ( maFrameData.mpGraphics->maGraphicsData.mhDefPal )
            SelectPalette( maFrameData.mpGraphics->maGraphicsData.mhDC, maFrameData.mpGraphics->maGraphicsData.mhDefPal, TRUE );
#endif
        ImplSalDeInitGraphics( &(maFrameData.mpGraphics->maGraphicsData) );
#ifdef WIN
        ReleaseDC( maFrameData.mhWnd, maFrameData.mpGraphics->maGraphicsData.mhDC );
#endif
        delete maFrameData.mpGraphics;
    }

    if ( maFrameData.mhWnd )
    {
        // reset mouse leave data
        if ( pSalData->mhWantLeaveMsg == maFrameData.mhWnd )
        {
            pSalData->mhWantLeaveMsg = 0;
            if ( pSalData->mpMouseLeaveTimer )
            {
                delete pSalData->mpMouseLeaveTimer;
                pSalData->mpMouseLeaveTimer = NULL;
            }
        }

#ifdef WIN
        // destroy system frame
        if ( !DestroyWindow( maFrameData.mhWnd ) )
            SetWindowPtr( maFrameData.mhWnd, 0 );
#endif
    }

    // remove frame from framelist
    if ( this == pSalData->mpFirstFrame )
        pSalData->mpFirstFrame = maFrameData.mpNextFrame;
    else
    {
        SalFrame* pTempFrame = pSalData->mpFirstFrame;
        while ( pTempFrame->maFrameData.mpNextFrame != this )
            pTempFrame = pTempFrame->maFrameData.mpNextFrame;

        pTempFrame->maFrameData.mpNextFrame = maFrameData.mpNextFrame;
    }
}

// -----------------------------------------------------------------------

SalGraphics* SalFrame::GetGraphics()
{
    if ( maFrameData.mbGraphics )
        return NULL;

    if ( !maFrameData.mpGraphics )
    {
        VCLVIEW hView = VCLWindow_contentView( maFrameData.mhWnd );
        if ( hView )
        {
            SalData* pSalData = GetSalData();
            maFrameData.mpGraphics = new SalGraphics;
            maFrameData.mpGraphics->maGraphicsData.mhDC      = hView;
            maFrameData.mpGraphics->maGraphicsData.mhWnd     = maFrameData.mhWnd;
            maFrameData.mpGraphics->maGraphicsData.mbPrinter = FALSE;
            maFrameData.mpGraphics->maGraphicsData.mbVirDev  = FALSE;
            maFrameData.mpGraphics->maGraphicsData.mbWindow  = TRUE;
            maFrameData.mpGraphics->maGraphicsData.mbScreen  = TRUE;
#ifdef WIN
            if ( pSalData->mhDitherPal )
            {
                maFrameData.mpGraphics->maGraphicsData.mhDefPal = SelectPalette( hView, pSalData->mhDitherPal, TRUE );
                RealizePalette( hView );
            }
            ImplSalInitGraphics( &(maFrameData.mpGraphics->maGraphicsData) );
#endif
            maFrameData.mbGraphics = TRUE;
        }
    }
    else
        maFrameData.mbGraphics = TRUE;

    return maFrameData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalFrame::ReleaseGraphics( SalGraphics* )
{
    maFrameData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL SalFrame::PostEvent( void* pData )
{
#ifdef WIN
    return (BOOL)ImplPostMessage( maFrameData.mhWnd, SAL_MSG_USEREVENT, 0, (LPARAM)pData );
#else
    return FALSE;
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetTitle( const XubString& rTitle )
{
    DBG_ASSERT( sizeof( wchar_t ) == sizeof( xub_Unicode ), "SalFrame::SetTitle(): wchar_t != sal_Unicode" );

#ifdef WIN
    if ( !SetWindowTextW( maFrameData.mhWnd, rTitle.GetBuffer() ) )
    {
        ByteString aAnsiTitle = ImplSalGetWinAnsiString( rTitle );
        SetWindowTextA( maFrameData.mhWnd, aAnsiTitle.GetBuffer() );
    }
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetIcon( USHORT nIcon )
{
//    ImplSendMessage( maFrameData.mhWnd, WM_SETICON, FALSE, hSmIcon );
//    ImplSendMessage( maFrameData.mhWnd, WM_SETICON, TRUE, hIcon );
}

// -----------------------------------------------------------------------

static void ImplSalShow( VCLWINDOW hWnd, BOOL bVisible )
{
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return;

    if ( bVisible )
    {
        pFrame->maFrameData.mbDefPos = FALSE;
        pFrame->maFrameData.mbOverwriteState = TRUE;
        pFrame->maFrameData.mbInShow = TRUE;
#ifdef WIN
        ShowWindow( hWnd, pFrame->maFrameData.mnShowState );
#endif
        // Damit Taskleiste unter W98 auch gleich ausgeblendet wird
        if ( pFrame->maFrameData.mbPresentation )
        {
#ifdef WIN
            VCLWINDOW hWndParent = ::GetParent( hWnd );
            if ( hWndParent )
                SetForegroundWindow( hWndParent );
            SetForegroundWindow( hWnd );
#endif
        }
#ifdef WIN
        pFrame->maFrameData.mnShowState = SW_SHOW;
        pFrame->maFrameData.mbInShow = FALSE;
        UpdateWindow( hWnd );
#endif
    }
    else
    {
#ifdef WIN
        if ( pFrame->maFrameData.mbFullScreen &&
             pFrame->maFrameData.mbPresentation &&
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
                ShowWindow( pFrame->maFrameData.mhWnd, SW_SHOWMINNOACTIVE );
                aInfo.iMinAnimate = nOldAni;
                SystemParametersInfo( SPI_SETANIMATION, 0, &aInfo, 0 );
            }
            else
                ShowWindow( hWnd, SW_SHOWMINNOACTIVE );
            ShowWindow( hWnd, SW_HIDE );
        }
        else
            ShowWindow( hWnd, SW_HIDE );
#endif
    }
}

// -----------------------------------------------------------------------

void SalFrame::Show( BOOL bVisible )
{
    if ( bVisible )
        VCLWindow_makeKeyAndOrderFront( maFrameData.mhWnd );
    else
        VCLWindow_close( maFrameData.mhWnd );
}

// -----------------------------------------------------------------------

void SalFrame::Enable( BOOL bEnable )
{
#ifdef WIN
    EnableWindow( maFrameData.mhWnd, bEnable );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetMinClientSize( long nWidth, long nHeight )
{
}

// -----------------------------------------------------------------------

void SalFrame::SetClientSize( long nWidth, long nHeight )
{
#ifdef WIN
    BOOL bVisible = (GetWindowStyle( maFrameData.mhWnd ) & WS_VISIBLE) != 0;
    if ( !bVisible )
        maFrameData.mnShowState = SW_SHOWNORMAL;
    else
    {
        if ( IsIconic( maFrameData.mhWnd ) || IsZoomed( maFrameData.mhWnd ) )
            ShowWindow( maFrameData.mhWnd, SW_RESTORE );
    }

    // Fenstergroesse berechnen
    RECT aWinRect;
    aWinRect.left   = 0;
    aWinRect.right  = (int)nWidth-1;
    aWinRect.top    = 0;
    aWinRect.bottom = (int)nHeight-1;
    AdjustWindowRectEx( &aWinRect,
                        GetWindowStyle( maFrameData.mhWnd ),
                        FALSE,
                        GetWindowExStyle( maFrameData.mhWnd ) );
    nWidth  = aWinRect.right - aWinRect.left + 1;
    nHeight = aWinRect.bottom - aWinRect.top + 1;

    // Position so berechnen, das Fenster zentiert auf dem Desktop
    // angezeigt wird
    int     nX;
    int     nY;
    int     nScreenX;
    int     nScreenY;
    int     nScreenWidth;
    int     nScreenHeight;

    RECT aRect;
    SystemParametersInfo( SPI_GETWORKAREA, 0, &aRect, 0 );
    nScreenX        = aRect.left;
    nScreenY        = aRect.top;
    nScreenWidth    = aRect.right-aRect.left;
    nScreenHeight   = aRect.bottom-aRect.top;

    if ( maFrameData.mbDefPos )
    {
        nX = (nScreenWidth-nWidth)/2 +  nScreenX;
        nY = (nScreenHeight-nHeight)/2 +  nScreenY;
        if ( bVisible )
            maFrameData.mbDefPos = FALSE;
    }
    else
    {
        RECT aWinRect;
        GetWindowRect( maFrameData.mhWnd, &aWinRect );
        nX = aWinRect.left;
        nY = aWinRect.top;
        if ( nX+nWidth > nScreenX+nScreenWidth )
            nX = (nScreenX+nScreenWidth) - nWidth;
        if ( nY+nHeight > nScreenY+nScreenHeight )
            nY = (nScreenY+nScreenHeight) - nHeight;
        if ( nX < nScreenX )
            nX = nScreenX;
        if ( nY < nScreenY )
            nY = nScreenY;
    }

    SetWindowPos( maFrameData.mhWnd, 0, nX, nY, (int)nWidth, (int)nHeight, SWP_NOZORDER | SWP_NOACTIVATE );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    rWidth  = maFrameData.mnWidth;
    rHeight = maFrameData.mnHeight;
}

// -----------------------------------------------------------------------

void SalFrame::SetWindowState( const SalFrameState* pState )
{
#ifdef WIN
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
    SystemParametersInfo( SPI_GETWORKAREA, 0, &aRect, 0 );
    nScreenX        = aRect.left;
    nScreenY        = aRect.top;
    nScreenWidth    = aRect.right-aRect.left;
    nScreenHeight   = aRect.bottom-aRect.top;

    // Fenster-Position/Groesse in den Bildschirm einpassen
    nX = (int)pState->mnX;
    nY = (int)pState->mnY;
    nWidth = (int)pState->mnWidth;
    nHeight = (int)pState->mnHeight;
    if ( nX < nScreenX )
        nX = nScreenX;
    if ( nY < nScreenY )
        nY = nScreenY;
    if ( nScreenWidth < nWidth )
        nWidth = nScreenWidth;
    if ( nScreenHeight < nHeight )
        nHeight = nScreenHeight;

    // Restore-Position setzen
    WINDOWPLACEMENT aPlacement;
    aPlacement.length = sizeof( aPlacement );
    GetWindowPlacement( maFrameData.mhWnd, &aPlacement );

    // Status setzen
    BOOL bVisible = (GetWindowStyle( maFrameData.mhWnd ) & WS_VISIBLE) != 0;
    if ( !bVisible )
    {
        aPlacement.showCmd = SW_HIDE;

        if ( maFrameData.mbOverwriteState )
        {
            if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
                maFrameData.mnShowState = SW_SHOWMINIMIZED;
            else if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
                maFrameData.mnShowState = SW_SHOWMAXIMIZED;
            else
                maFrameData.mnShowState = SW_SHOWNORMAL;
        }
    }
    else
    {
        if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
        {
            if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
                aPlacement.flags |= WPF_RESTORETOMAXIMIZED;
            aPlacement.showCmd = SW_SHOWMINIMIZED;
        }
        else if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
            aPlacement.showCmd = SW_SHOWMAXIMIZED;
        else
            aPlacement.showCmd = SW_RESTORE;
    }

    // Wenn Fenster nicht minimiert/maximiert ist oder nicht optisch
    // umgesetzt werden muss, dann SetWindowPos() benutzen, da
    // SetWindowPlacement() die TaskBar mit einrechnet
    if ( !IsIconic( maFrameData.mhWnd ) && !IsZoomed( maFrameData.mhWnd ) &&
         (!bVisible || (aPlacement.showCmd == SW_RESTORE)) )
    {
        SetWindowPos( maFrameData.mhWnd, 0,
                      nX, nY, nWidth, nHeight,
                      SWP_NOZORDER | SWP_NOACTIVATE );
    }
    else
    {
        aPlacement.rcNormalPosition.left    = nX-nScreenX;
        aPlacement.rcNormalPosition.top     = nY-nScreenY;
        aPlacement.rcNormalPosition.right   = nX+nWidth-nScreenX;
        aPlacement.rcNormalPosition.bottom  = nY+nHeight-nScreenY;
        SetWindowPlacement( maFrameData.mhWnd, &aPlacement );
    }
#endif
}

// -----------------------------------------------------------------------

BOOL SalFrame::GetWindowState( SalFrameState* pState )
{
    if ( maFrameData.maState.mnWidth && maFrameData.maState.mnHeight )
    {
        *pState = maFrameData.maState;
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void SalFrame::ShowFullScreen( BOOL bFullScreen )
{
    if ( maFrameData.mbFullScreen == bFullScreen )
        return;

    maFrameData.mbFullScreen = bFullScreen;
#ifdef WIN
    if ( bFullScreen )
    {
        // Damit Taskleiste von Windows ausgeblendet wird
        DWORD nExStyle = GetWindowExStyle( maFrameData.mhWnd );
        if ( nExStyle & WS_EX_TOOLWINDOW )
        {
            maFrameData.mbFullScreenToolWin = TRUE;
            nExStyle &= ~WS_EX_TOOLWINDOW;
            SetWindowExStyle( maFrameData.mhWnd, nExStyle );
        }

        // save old position
        GetWindowRect( maFrameData.mhWnd, &maFrameData.maFullScreenRect );

        // save show state
        maFrameData.mnFullScreenShowState = maFrameData.mnShowState;
        if ( !(GetWindowStyle( maFrameData.mhWnd ) & WS_VISIBLE) )
            maFrameData.mnShowState = SW_SHOW;

        // set window to screen size
        ImplSalFrameFullScreenPos( this, TRUE );
    }
    else
    {
        // wenn ShowState wieder hergestellt werden muss, hiden wir zuerst
        // das Fenster, damit es nicht so sehr flackert
        BOOL bVisible = (GetWindowStyle( maFrameData.mhWnd ) & WS_VISIBLE) != 0;
        if ( bVisible && (maFrameData.mnShowState != maFrameData.mnFullScreenShowState) )
            ShowWindow( maFrameData.mhWnd, SW_HIDE );

        if ( maFrameData.mbFullScreenToolWin )
            SetWindowExStyle( maFrameData.mhWnd, GetWindowExStyle( maFrameData.mhWnd ) | WS_EX_TOOLWINDOW );
        maFrameData.mbFullScreenToolWin = FALSE;

        SetWindowPos( maFrameData.mhWnd, 0,
                      maFrameData.maFullScreenRect.left,
                      maFrameData.maFullScreenRect.top,
                      maFrameData.maFullScreenRect.right-maFrameData.maFullScreenRect.left,
                      maFrameData.maFullScreenRect.bottom-maFrameData.maFullScreenRect.top,
                      SWP_NOZORDER | SWP_NOACTIVATE );

        // restore show state
        if ( maFrameData.mnShowState != maFrameData.mnFullScreenShowState )
        {
            maFrameData.mnShowState = maFrameData.mnFullScreenShowState;
            if ( bVisible )
            {
                maFrameData.mbInShow = TRUE;
                ShowWindow( maFrameData.mhWnd, maFrameData.mnShowState );
                maFrameData.mbInShow = FALSE;
                UpdateWindow( maFrameData.mhWnd );
            }
        }
    }
#endif
}

// -----------------------------------------------------------------------

void SalFrame::StartPresentation( BOOL bStart )
{
#ifdef WIN
    if ( maFrameData.mbPresentation == bStart )
        return;

    maFrameData.mbPresentation = bStart;

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

        // Systemagenten wieder aktivieren
        if ( pSalData->mnSageStatus == ENABLE_AGENT )
            pSalData->mpSageEnableProc( pSalData->mnSageStatus );
    }
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
#ifdef WIN
    VCLWINDOW hWnd;
    if ( bOnTop )
        hWnd = VCLWINDOW_TOPMOST;
    else
        hWnd = VCLWINDOW_NOTOPMOST;
    SetWindowPos( maFrameData.mhWnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
#endif
}

// -----------------------------------------------------------------------

static void ImplSalToTop( VCLWINDOW hWnd, USHORT nFlags )
{
#ifdef WIN
    if ( nFlags & SAL_FRAME_TOTOP_FOREGROUNDTASK )
        SetForegroundWindow( hWnd );
    if ( !IsIconic( hWnd ) )
    {
        SetFocus( hWnd );

        // Windows behauptet oefters mal, das man den Focus hat, obwohl
        // man diesen nicht hat. Wenn dies der Fall ist, dann versuchen
        // wir diesen auch ganz richtig zu bekommen.
        if ( ::GetFocus() == hWnd )
            SetForegroundWindow( hWnd );
    }
    else
    {
        if ( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
        {
            if ( GetWindowPtr( hWnd )->maFrameData.mbRestoreMaximize )
                ShowWindow( hWnd, SW_MAXIMIZE );
            else
                ShowWindow( hWnd, SW_RESTORE );
        }
    }
#endif
}

// -----------------------------------------------------------------------

void SalFrame::ToTop( USHORT nFlags )
{
#ifdef WIN
    // Send this Message to the window, because SetFocus() only work
    // in the thread of the window, which has create this window
    ImplSendMessage( maFrameData.mhWnd, SAL_MSG_TOTOP, nFlags, 0 );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetPointer( PointerStyle ePointerStyle )
{
#ifdef WIN
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
    { 0, 0, SAL_RESID_POINTER_HELP },               // POINTER_HELP
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
    { 0, 0, SAL_RESID_POINTER_AIRBRUSH }            // POINTER_AIRBRUSH
    };

#if POINTER_COUNT != 86
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
    if ( maFrameData.mhCursor != aImplPtrTab[ePointerStyle].mhCursor )
    {
        maFrameData.mhCursor = aImplPtrTab[ePointerStyle].mhCursor;
        SetCursor( maFrameData.mhCursor );
    }
#endif
}

// -----------------------------------------------------------------------

void SalFrame::CaptureMouse( BOOL bCapture )
{
#ifdef WIN
    // Send this Message to the window, because CaptureMouse() only work
    // in the thread of the window, which has create this window
    int nMsg;
    if ( bCapture )
        nMsg = SAL_MSG_CAPTUREMOUSE;
    else
        nMsg = SAL_MSG_RELEASEMOUSE;
    ImplSendMessage( maFrameData.mhWnd, nMsg, 0, 0 );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetPointerPos( long nX, long nY )
{
    POINT aPt;
    aPt.x = (int)nX;
    aPt.y = (int)nY;
#ifdef WIN
    ClientToScreen( maFrameData.mhWnd, &aPt );
    SetCursorPos( aPt.x, aPt.y );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::Flush()
{
#ifdef WIN
    GdiFlush();
#endif
}

// -----------------------------------------------------------------------

void SalFrame::Sync()
{
#ifdef WIN
    GdiFlush();
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetInputContext( SalInputContext* pContext )
{
    BOOL bIME = pContext->mnOptions != 0;
    if ( bIME == maFrameData.mbIME )
        return;

    maFrameData.mbIME = bIME;
    if ( !bIME )
    {
#ifdef WIN
        ImmAssociateContext( maFrameData.mhWnd, 0 );
#endif
        maFrameData.mbHandleIME = FALSE;
    }
    else
    {
        if ( maFrameData.mhDefIMEContext )
        {
#ifdef WIN
            ImmAssociateContext( maFrameData.mhWnd, maFrameData.mhDefIMEContext );
            UINT nImeProps = ImmGetProperty( GetKeyboardLayout( 0 ), IGP_PROPERTY );
            maFrameData.mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
            maFrameData.mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
            maFrameData.mbHandleIME = !maFrameData.mbSpezIME;
#endif
        }
    }
}

// -----------------------------------------------------------------------

void SalFrame::UpdateExtTextInputArea()
{
}

// -----------------------------------------------------------------------

void SalFrame::EndExtTextInput( USHORT nFlags )
{
#ifdef WIN
    VCLWINDOW hWnd = maFrameData.mhWnd;
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
#endif
}

// -----------------------------------------------------------------------

static void ImplGetKeyNameText( LONG lParam, sal_Unicode* pBuf,
                                UINT& rCount, UINT nMaxSize,
                                const sal_Char* pReplace )
{
#ifdef WIN
    DBG_ASSERT( sizeof( wchar_t ) == sizeof( xub_Unicode ), "SalFrame::ImplGetKeyNameTextW(): wchar_t != sal_Unicode" );

    wchar_t aKeyBuf[350];
    int nKeyLen = 0;
    if ( lParam )
    {
        nKeyLen = GetKeyNameTextW( lParam, aKeyBuf, sizeof( aKeyBuf ) / sizeof( sal_Unicode ) );
        if ( nKeyLen > 0 )
        {
            // Convert name, so that the keyname start with an upper
            // char and the rest of the word are in lower chars
            CharLowerBuffW( aKeyBuf, nKeyLen );
            CharUpperBuffW( aKeyBuf, 1 );
            wchar_t cTempChar;
            wchar_t* pKeyBuf = aKeyBuf;
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
#endif
}

// -----------------------------------------------------------------------

XubString SalFrame::GetKeyName( USHORT nKeyCode )
{
    XubString   aKeyCode;
#ifdef WIN
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
#endif

    return aKeyCode;
}

// -----------------------------------------------------------------------

XubString SalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
    return GetKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

inline Color ImplWinColorToSal( COLORREF nColor )
{
#ifdef WIN
    return Color( GetRValue( nColor ), GetGValue( nColor ), GetBValue( nColor ) );
#else
    return NULL;
#endif
}

// -----------------------------------------------------------------------

#ifdef WIN
static void ImplSalUpdateStyleFontA( const LOGFONTA& rLogFont, Font& rFont,
    BOOL bOverwriteSystemCharSet )
{
    ImplSalLogFontToFontA( rLogFont, rFont );
    if ( bOverwriteSystemCharSet && (rFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL) )
        rFont.SetCharSet( gsl_getSystemTextEncoding() );
    // Da bei einigen Windows-Einstellungen 6 Punkt eingetragen ist,
    // obwohl im Dialog 8 Punkt angezeigt werden (da MS Sans Serif
    // nicht skalierbar ist) vergroessern wir hier das als Hack, da
    // ansonsten in russisch Symbolunterschriften nicht lesbar sind
    if ( (rFont.GetName().EqualsIgnoreCaseAscii( "MS Sans Serif" ) ) &&
         (rFont.GetHeight() < 8) )
        rFont.SetHeight( 8 );
}
#endif

// -----------------------------------------------------------------------

#ifdef WIN
static void ImplSalUpdateStyleFontW( const LOGFONTW& rLogFont, Font& rFont,
    BOOL bOverwriteSystemCharSet )
{
    ImplSalLogFontToFontW( rLogFont, rFont );
    if ( bOverwriteSystemCharSet && (rFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL) )
        rFont.SetCharSet( gsl_getSystemTextEncoding() );
    // Da bei einigen Windows-Einstellungen 6 Punkt eingetragen ist,
    // obwohl im Dialog 8 Punkt angezeigt werden (da MS Sans Serif
    // nicht skalierbar ist) vergroessern wir hier das als Hack, da
    // ansonsten in russisch Symbolunterschriften nicht lesbar sind
    if ( (rFont.GetName().EqualsIgnoreCaseAscii( "MS Sans Serif" ) ) &&
         (rFont.GetHeight() < 8) )
        rFont.SetHeight( 8 );
}
#endif

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

void SalFrame::UpdateSettings( AllSettings& rSettings )
{
#ifdef WIN
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
    aStyleSettings.SetScrollBarSize( GetSystemMetrics( SM_CXVSCROLL ) );
    aStyleSettings.SetSpinSize( GetSystemMetrics( SM_CXVSCROLL ) );
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
        aStyleSettings.SetMenuTextColor( ImplWinColorToSal( GetSysColor( COLOR_MENUTEXT ) ) );
        aStyleSettings.SetActiveColor( ImplWinColorToSal( GetSysColor( COLOR_ACTIVECAPTION ) ) );
        aStyleSettings.SetActiveTextColor( ImplWinColorToSal( GetSysColor( COLOR_CAPTIONTEXT ) ) );
        aStyleSettings.SetDeactiveColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVECAPTION ) ) );
        aStyleSettings.SetDeactiveTextColor( ImplWinColorToSal( GetSysColor( COLOR_INACTIVECAPTIONTEXT ) ) );
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

    // Query Fonts
    int bOverwriteSystemCharSet = getenv("LC_CHARSET") != 0;
    Font aMenuFont = aStyleSettings.GetMenuFont();
    Font aTitleFont = aStyleSettings.GetTitleFont();
    Font aFloatTitleFont = aStyleSettings.GetFloatTitleFont();
    Font aHelpFont = aStyleSettings.GetHelpFont();
    Font aAppFont = aStyleSettings.GetAppFont();
    Font aIconFont = aStyleSettings.GetIconFont();
    if ( aSalShlData.mbWNT )
    {
        NONCLIENTMETRICSW aNonClientMetrics;
        aNonClientMetrics.cbSize = sizeof( aNonClientMetrics );
        if ( SystemParametersInfoW( SPI_GETNONCLIENTMETRICS, sizeof( aNonClientMetrics ), &aNonClientMetrics, 0 ) )
        {
            ImplSalUpdateStyleFontW( aNonClientMetrics.lfMenuFont, aMenuFont, bOverwriteSystemCharSet );
            ImplSalUpdateStyleFontW( aNonClientMetrics.lfCaptionFont, aTitleFont, bOverwriteSystemCharSet );
            ImplSalUpdateStyleFontW( aNonClientMetrics.lfSmCaptionFont, aFloatTitleFont, bOverwriteSystemCharSet );
            ImplSalUpdateStyleFontW( aNonClientMetrics.lfStatusFont, aHelpFont, bOverwriteSystemCharSet );
            ImplSalUpdateStyleFontW( aNonClientMetrics.lfMessageFont, aAppFont, bOverwriteSystemCharSet );

            LOGFONTW aLogFont;
            if ( SystemParametersInfoW( SPI_GETICONTITLELOGFONT, 0, &aLogFont, 0 ) )
                ImplSalUpdateStyleFontW( aLogFont, aIconFont, bOverwriteSystemCharSet );
        }
    }
    else
    {
        NONCLIENTMETRICSA aNonClientMetrics;
        aNonClientMetrics.cbSize = sizeof( aNonClientMetrics );
        if ( SystemParametersInfoA( SPI_GETNONCLIENTMETRICS, sizeof( aNonClientMetrics ), &aNonClientMetrics, 0 ) )
        {
            ImplSalUpdateStyleFontA( aNonClientMetrics.lfMenuFont, aMenuFont, bOverwriteSystemCharSet );
            ImplSalUpdateStyleFontA( aNonClientMetrics.lfCaptionFont, aTitleFont, bOverwriteSystemCharSet );
            ImplSalUpdateStyleFontA( aNonClientMetrics.lfSmCaptionFont, aFloatTitleFont, bOverwriteSystemCharSet );
            ImplSalUpdateStyleFontA( aNonClientMetrics.lfStatusFont, aHelpFont, bOverwriteSystemCharSet );
            ImplSalUpdateStyleFontA( aNonClientMetrics.lfMessageFont, aAppFont, bOverwriteSystemCharSet );

            LOGFONTA aLogFont;
            if ( SystemParametersInfoA( SPI_GETICONTITLELOGFONT, 0, &aLogFont, 0 ) )
                ImplSalUpdateStyleFontA( aLogFont, aIconFont, bOverwriteSystemCharSet );
        }
    }
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
#endif
}

// -----------------------------------------------------------------------

const SystemEnvData* SalFrame::GetSystemData() const
{
    return &maFrameData.maSysData;
}

// -----------------------------------------------------------------------

void SalFrame::Beep( SoundType eSoundType )
{
#ifdef WIN
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
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetCallback( void* pInst, SALFRAMEPROC pProc )
{
    maFrameData.mpInst = pInst;
    if ( pProc )
        maFrameData.mpProc = pProc;
    else
        maFrameData.mpProc = ImplSalCallbackDummy;
}

// -----------------------------------------------------------------------

static long ImplHandleMouseMsg( VCLWINDOW hWnd, UINT nMsg,
    WPARAM wParam, LPARAM lParam )
{
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    SalMouseEvent   aMouseEvt;
    long            nRet;
#ifdef WIN
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

    if ( bCall )
    {
        if ( nEvent == SALEVENT_MOUSEBUTTONDOWN )
            UpdateWindow( hWnd );

        nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                           nEvent, &aMouseEvt );
        if ( nMsg == WM_MOUSEMOVE )
            SetCursor( pFrame->maFrameData.mhCursor );
    }
    else
        nRet = 0;

    return nRet;
#endif
}

// -----------------------------------------------------------------------

static long ImplHandleMouseActivateMsg( VCLWINDOW hWnd )
{
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    SalMouseActivateEvent   aMouseActivateEvt;
    POINT                   aPt;
#ifdef WIN
    GetCursorPos( &aPt );
    ScreenToClient( hWnd, &aPt );
#endif
    aMouseActivateEvt.mnX = aPt.x;
    aMouseActivateEvt.mnY = aPt.y;
    return pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                       SALEVENT_MOUSEACTIVATE, &aMouseActivateEvt );
}

// -----------------------------------------------------------------------

static long ImplHandleWheelMsg( VCLWINDOW hWnd, WPARAM wParam, LPARAM lParam )
{
    ImplSalYieldMutexAcquireWithWait();

    long        nRet = 0;
#ifdef WIN
    SalFrame*   pFrame = GetWindowPtr( hWnd );
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

        nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                           SALEVENT_WHEELMOUSE, &aWheelEvt );
    }

    ImplSalYieldMutexRelease();
#endif

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

static sal_Unicode ImplGetCharCode( SalFrame* pFrame, WPARAM nCharCode )
{
#ifdef WIN
    UINT nLang = LOWORD( GetKeyboardLayout( 0 ) );
    if ( !nLang )
    {
        pFrame->maFrameData.mnInputLang     = 0;
        pFrame->maFrameData.mnInputCodePage = GetACP();
    }
    else if ( nLang != pFrame->maFrameData.mnInputLang )
    {
        pFrame->maFrameData.mnInputLang = nLang;
        sal_Char aBuf[10];
        if ( GetLocaleInfoA( MAKELCID( nLang, SORT_DEFAULT ), LOCALE_IDEFAULTANSICODEPAGE,
                             aBuf, sizeof(aBuf) ) > 0 )
        {
            pFrame->maFrameData.mnInputCodePage = ImplStrToNum( aBuf );
            if ( !pFrame->maFrameData.mnInputCodePage )
                pFrame->maFrameData.mnInputCodePage = GetACP();
        }
        else
            pFrame->maFrameData.mnInputCodePage = GetACP();
    }

    sal_Char    aCharBuf[2];
    int         nCharLen;
    wchar_t     c;
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
    if ( ::MultiByteToWideChar( pFrame->maFrameData.mnInputCodePage,
                                MB_PRECOMPOSED,
                                aCharBuf, nCharLen, &c, 1 ) )
        return (sal_Unicode)c;
    else
#endif
        return (sal_Unicode)nCharCode;
}

// -----------------------------------------------------------------------

static long ImplHandleKeyMsg( VCLWINDOW hWnd, UINT nMsg,
    WPARAM wParam, LPARAM lParam )
{
#ifdef WIN
    static BOOL     bIgnoreCharMsg  = FALSE;
    static WPARAM   nDeadChar       = 0;
    static WPARAM   nLastVKChar     = 0;
    static USHORT   nLastChar       = 0;
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

    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    // Wir restaurieren den Background-Modus bei jeder Texteingabe,
    // da einige Tools wie RichWin uns diesen hin- und wieder umsetzen
    if ( pFrame->maFrameData.mpGraphics &&
        pFrame->maFrameData.mpGraphics->maGraphicsData.mhDC )
        SetBkMode( pFrame->maFrameData.mpGraphics->maGraphicsData.mhDC, TRANSPARENT );

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
            return 0;
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
        long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                SALEVENT_KEYINPUT, &aKeyEvt );
        pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                    SALEVENT_KEYUP, &aKeyEvt );
        return nRet;
    }
    else
    {
        // Bei Shift, Control und Menu schicken wir einen KeyModChange-Event
        if ( (wParam == VK_SHIFT) || (wParam == VK_CONTROL) || (wParam == VK_MENU) )
        {
            SalKeyModEvent aModEvt;
            aModEvt.mnTime = GetMessageTime();
            aModEvt.mnCode = nModCode;
            return pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                               SALEVENT_KEYMODCHANGE, &aModEvt );
        }
        else
        {
            SalKeyEvent     aKeyEvt;
            USHORT          nEvent;
            MSG             aCharMsg;
            WIN_BOOL        bCharPeek = FALSE;
            UINT            nCharMsg = WM_CHAR;
            BOOL            bKeyUp = (nMsg == WM_KEYUP) || (nMsg == WM_SYSKEYUP);

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
                long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                        nEvent, &aKeyEvt );
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
#else
    return 0;
#endif
}

// -----------------------------------------------------------------------

long ImplHandleSalObjKeyMsg( VCLWINDOW hWnd, UINT nMsg,
    WPARAM wParam, LPARAM lParam )
{
#ifdef WIN
    if ( (nMsg == WM_KEYDOWN) || (nMsg == WM_KEYUP) )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
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
                long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                        nEvent, &aKeyEvt );
                return nRet;
            }
            else
                return 0;
        }
    }
#endif

    return 0;
}

// -----------------------------------------------------------------------

long ImplHandleSalObjSysCharMsg( VCLWINDOW hWnd, WPARAM wParam, LPARAM lParam )
{
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

#ifdef WIN
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
    long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_KEYINPUT, &aKeyEvt );
    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                SALEVENT_KEYUP, &aKeyEvt );
    return nRet;
#else
    return 0;
#endif
}

// -----------------------------------------------------------------------

static void ImplHandlePaintMsg( VCLWINDOW hWnd )
{
    // Clip-Region muss zurueckgesetzt werden, da wir sonst kein
    // ordentliches Bounding-Rectangle bekommen
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && pFrame->maFrameData.mpGraphics )
        {
#ifdef WIN
            if ( pFrame->maFrameData.mpGraphics->maGraphicsData.mhRegion )
                SelectClipRgn( pFrame->maFrameData.mpGraphics->maGraphicsData.mhDC, 0 );
#endif
        }
        ImplSalYieldMutexRelease();
    }

#ifdef WIN
    // Laut Window-Doku soll man erst abfragen, ob ueberhaupt eine
    // Paint-Region anliegt
    if ( !GetUpdateRect( hWnd, NULL, FALSE ) )
        return;

    // BeginPaint
    PAINTSTRUCT aPs;
    BeginPaint( hWnd, &aPs );

    // Paint
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            // ClipRegion wieder herstellen
            if ( pFrame->maFrameData.mpGraphics )
            {
                if ( pFrame->maFrameData.mpGraphics->maGraphicsData.mhRegion )
                {
#ifdef WIN
                    SelectClipRgn( pFrame->maFrameData.mpGraphics->maGraphicsData.mhDC,
                                   pFrame->maFrameData.mpGraphics->maGraphicsData.mhRegion );
#endif
                }
            }

            SalPaintEvent aPEvt;
            aPEvt.mnBoundX          = aPs.rcPaint.left;
            aPEvt.mnBoundY          = aPs.rcPaint.top;
            aPEvt.mnBoundWidth      = aPs.rcPaint.right-aPs.rcPaint.left;
            aPEvt.mnBoundHeight     = aPs.rcPaint.bottom-aPs.rcPaint.top;
            pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_PAINT, &aPEvt );
        }

        ImplSalYieldMutexRelease();
    }
    else
    {
        RECT* pRect = new RECT;
        *pRect = aPs.rcPaint;
        ImplPostMessage( hWnd, SAL_MSG_POSTPAINT, (WPARAM)pRect, 0 );
    }

    // EndPaint
    EndPaint( hWnd, &aPs );
#endif
}

// -----------------------------------------------------------------------

static void ImplHandlePaintMsg2( VCLWINDOW hWnd, RECT* pRect )
{
    // Paint
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            SalPaintEvent aPEvt;
            aPEvt.mnBoundX          = pRect->left;
            aPEvt.mnBoundY          = pRect->top;
            aPEvt.mnBoundWidth      = pRect->right-pRect->left;
            aPEvt.mnBoundHeight     = pRect->bottom-pRect->top;
            pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_PAINT, &aPEvt );
        }
        ImplSalYieldMutexRelease();
        delete pRect;
    }
#ifdef WIN
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTPAINT, (WPARAM)pRect, 0 );
#endif
}

// -----------------------------------------------------------------------

static void ImplHandleMoveMsg( VCLWINDOW hWnd )
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
#ifdef WIN
            if ( GetWindowStyle( hWnd ) & WS_VISIBLE )
                pFrame->maFrameData.mbDefPos = FALSE;
#endif

            // Gegen moegliche Rekursionen sichern
            if ( !pFrame->maFrameData.mbInMoveMsg )
            {
                // Fenster im FullScreenModus wieder einpassen
                pFrame->maFrameData.mbInMoveMsg = TRUE;
                if ( pFrame->maFrameData.mbFullScreen )
                    ImplSalFrameFullScreenPos( pFrame );
                pFrame->maFrameData.mbInMoveMsg = FALSE;
            }

            // Status merken
            ImplSaveFrameState( pFrame );
        }

        ImplSalYieldMutexRelease();
    }
#ifdef WIN
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTMOVE, 0, 0 );
#endif
}

// -----------------------------------------------------------------------

static void ImplCallSizeHdl( VCLWINDOW hWnd )
{
    // Da Windows diese Messages auch senden kann, muss hier auch die
    // Solar-Semaphore beruecksichtigt werden
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_RESIZE, 0 );
#ifdef WIN
            // Um doppelte Paints von VCL und SAL zu vermeiden
            if ( IsWindowVisible( hWnd ) && !pFrame->maFrameData.mbInShow )
                UpdateWindow( hWnd );
#endif
        }

        ImplSalYieldMutexRelease();
    }
#ifdef WIN
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTCALLSIZE, 0, 0 );
#endif
}

// -----------------------------------------------------------------------

static void ImplHandleSizeMsg( VCLWINDOW hWnd, WPARAM wParam, LPARAM lParam )
{
#ifdef WIN
    if ( (wParam != SIZE_MAXSHOW) && (wParam != SIZE_MAXHIDE) )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->maFrameData.mnWidth  = (int)LOWORD(lParam);
            pFrame->maFrameData.mnHeight = (int)HIWORD(lParam);
            // Status merken
            ImplSaveFrameState( pFrame );
            // Call Hdl
            ImplCallSizeHdl( hWnd );
        }
    }
#endif
}

// -----------------------------------------------------------------------

static void ImplHandleFocusMsg( VCLWINDOW hWnd )
{
#ifdef WIN
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            // Query the actual status
            if ( ::GetFocus() == hWnd )
            {
                if ( IsWindowVisible( hWnd ) && !pFrame->maFrameData.mbInShow )
                    UpdateWindow( hWnd );
                pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_GETFOCUS, 0 );
            }
            else
            {
                pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_LOSEFOCUS, 0 );
            }
        }

        ImplSalYieldMutexRelease();
    }
    else
        ImplPostMessage( hWnd, SAL_MSG_POSTFOCUS, 0, 0 );
#endif
}

// -----------------------------------------------------------------------

static void ImplHandleCloseMsg( VCLWINDOW hWnd )
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_CLOSE, 0 );
        }

        ImplSalYieldMutexRelease();
    }
#ifdef WIN
    else
        ImplPostMessage( hWnd, WM_CLOSE, 0, 0 );
#endif
}

// -----------------------------------------------------------------------

static long ImplHandleShutDownMsg( VCLWINDOW hWnd )
{
    ImplSalYieldMutexAcquireWithWait();
    long        nRet = 0;
    SalFrame*   pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                           SALEVENT_SHUTDOWN, 0 );
    }
    ImplSalYieldMutexRelease();
    return nRet;
}

// -----------------------------------------------------------------------

static void ImplHandleSettingsChangeMsg( VCLWINDOW hWnd, UINT nMsg,
    WPARAM wParam, LPARAM lParam )
{
#ifdef WIN
    USHORT nSalEvent = SALEVENT_SETTINGSCHANGED;

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

    if ( nMsg == WM_SETTINGCHANGE )
    {
        if ( wParam == SPI_SETWHEELSCROLLLINES )
            aSalShlData.mnWheelScrollLines = ImplSalGetWheelScrollLines();
    }

    if ( WM_SYSCOLORCHANGE == nMsg && GetSalData()->mhDitherPal )
        ImplUpdateSysColorEntries();

    ImplSalYieldMutexAcquireWithWait();

    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        if ( (nMsg == WM_DISPLAYCHANGE) || (nMsg == WM_WININICHANGE) )
        {
            if ( pFrame->maFrameData.mbFullScreen )
                ImplSalFrameFullScreenPos( pFrame );
        }

        pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                    nSalEvent, 0 );
    }

    ImplSalYieldMutexRelease();
#endif
}

// -----------------------------------------------------------------------

static void ImplHandleUserEvent( VCLWINDOW hWnd, LPARAM lParam )
{
    ImplSalYieldMutexAcquireWithWait();
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                    SALEVENT_USEREVENT, (void*)lParam );
    }
    ImplSalYieldMutexRelease();
}

// -----------------------------------------------------------------------

static void ImplHandleForcePalette( VCLWINDOW hWnd )
{
    SalData*    pSalData = GetSalData();
    HPALETTE    hPal = pSalData->mhDitherPal;
    if ( hPal )
    {
        if ( !ImplSalYieldMutexTryToAcquire() )
        {
#ifdef WIN
            ImplPostMessage( hWnd, SAL_MSG_FORCEPALETTE, 0, 0 );
#endif
            return;
        }

        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && pFrame->maFrameData.mpGraphics )
        {
            SalGraphics* pGraphics = pFrame->maFrameData.mpGraphics;
            if ( pGraphics && pGraphics->maGraphicsData.mhDefPal )
            {
#ifdef WIN
                SelectPalette( pGraphics->maGraphicsData.mhDC, hPal, FALSE );
                if ( RealizePalette( pGraphics->maGraphicsData.mhDC ) )
                {
                    InvalidateRect( hWnd, NULL, FALSE );
                    UpdateWindow( hWnd );
                    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                SALEVENT_DISPLAYCHANGED, 0 );
                }
#endif
            }
        }

        ImplSalYieldMutexRelease();
    }
}

// -----------------------------------------------------------------------

static LRESULT ImplHandlePalette( BOOL bFrame, VCLWINDOW hWnd, UINT nMsg,
    WPARAM wParam, LPARAM lParam, int& rDef )
{
    SalData*    pSalData = GetSalData();
    HPALETTE    hPal = pSalData->mhDitherPal;
    if ( !hPal )
        return 0;

    rDef = FALSE;
    if ( pSalData->mbInPalChange )
        return 0;
#ifdef WIN
    if ( (nMsg == WM_PALETTECHANGED) || (nMsg == SAL_MSG_POSTPALCHANGED) )
    {
        if ( (VCLWINDOW)wParam == hWnd )
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

    SalVirtualDevice*   pTempVD;
    SalFrame*           pTempFrame;
    SalGraphics*        pGraphics;
    VCLVIEW                 hDC;
    HPALETTE            hOldPal;
    UINT                nCols;
    BOOL                bStdDC;
    BOOL                bUpdate;

    pSalData->mbInPalChange = TRUE;

    // Alle Paletten in VirDevs und Frames zuruecksetzen
    pTempVD = pSalData->mpFirstVD;
    while ( pTempVD )
    {
        pGraphics = pTempVD->maVirDevData.mpGraphics;
        if ( pGraphics->maGraphicsData.mhDefPal )
        {
            SelectPalette( pGraphics->maGraphicsData.mhDC,
                           pGraphics->maGraphicsData.mhDefPal,
                           TRUE );
        }
        pTempVD = pTempVD->maVirDevData.mpNext;
    }
    pTempFrame = pSalData->mpFirstFrame;
    while ( pTempFrame )
    {
        pGraphics = pTempFrame->maFrameData.mpGraphics;
        if ( pGraphics && pGraphics->maGraphicsData.mhDefPal )
        {
            SelectPalette( pGraphics->maGraphicsData.mhDC,
                           pGraphics->maGraphicsData.mhDefPal,
                           TRUE );
        }
        pTempFrame = pTempFrame->maFrameData.mpNextFrame;
    }

    // Palette neu realizen
    SalFrame* pFrame = NULL;
    if ( bFrame )
        pFrame = GetWindowPtr( hWnd );
    if ( pFrame && pFrame->maFrameData.mpGraphics )
    {
        hDC = pFrame->maFrameData.mpGraphics->maGraphicsData.mhDC;
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
        pGraphics = pTempVD->maVirDevData.mpGraphics;
        if ( pGraphics->maGraphicsData.mhDefPal )
        {
            SelectPalette( pGraphics->maGraphicsData.mhDC, hPal, TRUE );
            RealizePalette( pGraphics->maGraphicsData.mhDC );
        }
        pTempVD = pTempVD->maVirDevData.mpNext;
    }
    pTempFrame = pSalData->mpFirstFrame;
    while ( pTempFrame )
    {
        if ( pTempFrame != pFrame )
        {
            pGraphics = pTempFrame->maFrameData.mpGraphics;
            if ( pGraphics && pGraphics->maGraphicsData.mhDefPal )
            {
                SelectPalette( pGraphics->maGraphicsData.mhDC, hPal, TRUE );
                if ( RealizePalette( pGraphics->maGraphicsData.mhDC ) )
                    bUpdate = TRUE;
            }
        }
        pTempFrame = pTempFrame->maFrameData.mpNextFrame;
    }

    // Wenn sich Farben geaendert haben, dann die Fenster updaten
    if ( bUpdate )
    {
        pTempFrame = pSalData->mpFirstFrame;
        while ( pTempFrame )
        {
            pGraphics = pTempFrame->maFrameData.mpGraphics;
            if ( pGraphics && pGraphics->maGraphicsData.mhDefPal )
            {
                InvalidateRect( pTempFrame->maFrameData.mhWnd, NULL, FALSE );
                UpdateWindow( pTempFrame->maFrameData.mhWnd );
                pTempFrame->maFrameData.mpProc( pTempFrame->maFrameData.mpInst, pTempFrame,
                                                SALEVENT_DISPLAYCHANGED, 0 );
            }
            pTempFrame = pTempFrame->maFrameData.mpNextFrame;
        }
    }

    pSalData->mbInPalChange = FALSE;

    if ( bReleaseMutex )
        ImplSalYieldMutexRelease();

    if ( nMsg == WM_PALETTECHANGED )
        return 0;
    else
        return nCols;
#else
        return 0;
#endif
}

// -----------------------------------------------------------------------

static int ImplHandleMinMax( VCLWINDOW hWnd, LPARAM lParam )
{
    int bRet = FALSE;

    if ( ImplSalYieldMutexTryToAcquire() )
    {
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            if ( pFrame->maFrameData.mbFullScreen )
            {
#ifdef WIN
                MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
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
#endif
            }
        }

        ImplSalYieldMutexRelease();
    }

    return bRet;
}

// -----------------------------------------------------------------------

static int ImplHandleSysCommand( VCLWINDOW hWnd, WPARAM wParam, LPARAM lParam )
{
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    WPARAM nCommand = wParam & 0xFFF0;

#ifdef WIN
    if ( pFrame->maFrameData.mbFullScreen )
    {
        WIN_BOOL    bMaximize = IsZoomed( pFrame->maFrameData.mhWnd );
        WIN_BOOL    bMinimize = IsIconic( pFrame->maFrameData.mhWnd );
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
                 (GetKeyState( VK_MBUTTON ) & 0x8000) )
                return 1;

            SalKeyEvent aKeyEvt;
            aKeyEvt.mnTime      = GetMessageTime();
            aKeyEvt.mnCode      = KEY_MENU;
            aKeyEvt.mnCharCode  = 0;
            aKeyEvt.mnRepeat    = 0;
            long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                    SALEVENT_KEYINPUT, &aKeyEvt );
            pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_KEYUP, &aKeyEvt );
            return (nRet != 0);
        }
        else
        {
            // Testen, ob ein SysChild den Focus hat
            VCLWINDOW hFocusWnd = ::GetFocus();
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
                    long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                            SALEVENT_KEYINPUT, &aKeyEvt );
                    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                SALEVENT_KEYUP, &aKeyEvt );
                    return (nRet != 0);
                }
            }
        }
    }
#endif

    return FALSE;
}

// -----------------------------------------------------------------------

static void ImplHandleInputLangChange( VCLWINDOW hWnd, WPARAM wParam, LPARAM lParam )
{
    ImplSalYieldMutexAcquireWithWait();

    // Feststellen, ob wir IME unterstuetzen
    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame && pFrame->maFrameData.mbIME && pFrame->maFrameData.mhDefIMEContext )
    {
#ifdef WIN
        VCLWINDOW   hWnd = pFrame->maFrameData.mhWnd;
        HKL     hKL = (HKL)lParam;
        UINT    nImeProps = ImmGetProperty( hKL, IGP_PROPERTY );

        pFrame->maFrameData.mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
        pFrame->maFrameData.mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
        pFrame->maFrameData.mbHandleIME = !pFrame->maFrameData.mbSpezIME;
#endif
    }

    ImplSalYieldMutexRelease();
}

// -----------------------------------------------------------------------

static BOOL ImplHandleIMEStartComposition( VCLWINDOW hWnd )
{
    BOOL bDef = TRUE;

    ImplSalYieldMutexAcquireWithWait();

    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        if ( pFrame->maFrameData.mbHandleIME )
        {
#ifdef WIN
            HIMC hIMC = ImmGetContext( hWnd );
            if ( hIMC )
            {
                // Cursor-Position ermitteln und aus der die Default-Position fuer
                // das Composition-Fenster berechnen
                SalCursorPosEvent aCursorPosEvt;
                pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame, SALEVENT_CURSORPOS, (void*)&aCursorPosEvt );
                COMPOSITIONFORM aForm;
                memset( &aForm, 0, sizeof( aForm ) );
                if ( !aCursorPosEvt.mnWidth || !aCursorPosEvt.mnHeight )
                    aForm.dwStyle |= CFS_DEFAULT;
                else
                {
                    aForm.dwStyle          |= CFS_POINT;
                    aForm.ptCurrentPos.x    = aCursorPosEvt.mnX;
                    aForm.ptCurrentPos.y    = aCursorPosEvt.mnY;
                }
                ImmSetCompositionWindow( hIMC, &aForm );

                // Den InputContect-Font ermitteln und diesem dem Composition-Fenster
                // bekannt machen

                ImmReleaseContext( hWnd, hIMC );
            }

            pFrame->maFrameData.mbCompositionMode = TRUE;
            pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_STARTEXTTEXTINPUT, (void*)NULL );
            if ( pFrame->maFrameData.mbAtCursorIME )
                bDef = FALSE;
#endif
        }
    }

    ImplSalYieldMutexRelease();

    return bDef;
}

// -----------------------------------------------------------------------

static BOOL ImplHandleIMEComposition( VCLWINDOW hWnd, LPARAM lParam )
{
    BOOL bDef = TRUE;
#ifdef WIN
    if ( lParam & (GCS_RESULTSTR | GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS) )
    {
        ImplSalYieldMutexAcquireWithWait();

        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && pFrame->maFrameData.mbHandleIME &&
            (pFrame->maFrameData.mbCompositionMode || !(lParam & GCS_RESULTSTR)) )
        {
            HIMC hIMC = ImmGetContext( hWnd );
            if ( hIMC )
            {
                SalExtTextInputEvent aEvt;
                aEvt.mnTime             = GetMessageTime();
                aEvt.mpTextAttr         = NULL;
                aEvt.mnCursorPos        = 0;
                aEvt.mnDeltaStart       = 0;
                aEvt.mbOnlyCursor       = FALSE;
                aEvt.mbCursorVisible    = !pFrame->maFrameData.mbCandidateMode;

                LONG            nTextLen;
                xub_Unicode*    pTextBuf = NULL;
                LONG            nAttrLen;
                WIN_BYTE*       pAttrBuf = NULL;
                BOOL            bLastCursor = FALSE;
                if ( lParam & GCS_RESULTSTR )
                {
                    nTextLen = ImmGetCompositionStringW( hIMC, GCS_RESULTSTR, 0, 0 ) / sizeof( wchar_t );
                    if ( nTextLen >= 0 )
                    {
                        pTextBuf = new xub_Unicode[nTextLen];
                        ImmGetCompositionStringW( hIMC, GCS_RESULTSTR, pTextBuf, nTextLen*sizeof( wchar_t ) );
                    }

                    bLastCursor = TRUE;
                    aEvt.mbCursorVisible = TRUE;
                    bDef = FALSE;
                }
                else if ( pFrame->maFrameData.mbAtCursorIME )
                {
                    bDef = FALSE;
                    if ( lParam & (GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS) )
                    {
                        nTextLen = ImmGetCompositionStringW( hIMC, GCS_COMPSTR, 0, 0 ) / sizeof( wchar_t );
                        if ( nTextLen >= 0 )
                        {
                            pTextBuf = new xub_Unicode[nTextLen];
                            ImmGetCompositionStringW( hIMC, GCS_COMPSTR, pTextBuf, nTextLen*sizeof( wchar_t ) );
                        }

                        nAttrLen = ImmGetCompositionStringW( hIMC, GCS_COMPATTR, 0, 0 );
                        if ( nAttrLen >= 0 )
                        {
                            pAttrBuf = new WIN_BYTE[nAttrLen];
                            ImmGetCompositionStringW( hIMC, GCS_COMPATTR, pAttrBuf, nAttrLen );
                        }

                        aEvt.mnCursorPos = LOWORD( ImmGetCompositionStringW( hIMC, GCS_CURSORPOS, 0, 0 ) );
                        aEvt.mnDeltaStart = LOWORD( ImmGetCompositionStringW( hIMC, GCS_DELTASTART, 0, 0 ) );

                        if ( lParam == GCS_CURSORPOS )
                            aEvt.mbOnlyCursor = TRUE;
                    }
                }

                USHORT* pSalAttrAry = NULL;
                if ( pTextBuf )
                {
                    aEvt.maText = XubString( pTextBuf, (USHORT)nTextLen );
                    delete pTextBuf;
                    if ( pAttrBuf )
                    {
                        xub_StrLen nTextLen = aEvt.maText.Len();
                        if ( nTextLen )
                        {
                            pSalAttrAry = new USHORT[nTextLen];
                            memset( pSalAttrAry, 0, nTextLen*sizeof( USHORT ) );
                            for ( xub_StrLen i = 0; (i < nTextLen) && (i < nAttrLen); i++ )
                            {
                                WIN_BYTE nWinAttr = pAttrBuf[i];
                                USHORT   nSalAttr;
                                if ( nWinAttr == ATTR_TARGET_CONVERTED )
                                {
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_TARGETCONVERTED | SAL_EXTTEXTINPUT_ATTR_UNDERLINE | SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
                                    aEvt.mbCursorVisible = FALSE;
                                }
                                else if ( nWinAttr == ATTR_CONVERTED )
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_CONVERTED | SAL_EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE;
                                else if ( nWinAttr == ATTR_TARGET_NOTCONVERTED )
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_TARGETNOTCONVERTED | SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                                else if ( nWinAttr == ATTR_INPUT_ERROR )
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_INPUTERROR | SAL_EXTTEXTINPUT_ATTR_REDTEXT | SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                                else /* ( nWinAttr == ATTR_INPUT ) */
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_INPUT | SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                                pSalAttrAry[i] = nSalAttr;
                            }
                            aEvt.mpTextAttr = pSalAttrAry;
                        }
                        delete pAttrBuf;
                    }
                    if ( bLastCursor )
                        aEvt.mnCursorPos = aEvt.maText.Len();
                }

                ImmReleaseContext( hWnd, hIMC );

                // Handler rufen und wenn wir ein Attribute-Array haben, danach
                // wieder zerstoeren
                if ( !bDef )
                {
                    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
                }
                if ( pSalAttrAry )
                    delete pSalAttrAry;
            }

            ImplSalYieldMutexRelease();
        }
    }
#endif

    return bDef;
}

// -----------------------------------------------------------------------

static BOOL ImplHandleIMEEndComposition( VCLWINDOW hWnd )
{
    BOOL bDef = TRUE;

    ImplSalYieldMutexAcquireWithWait();

    SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame && pFrame->maFrameData.mbHandleIME )
    {
#ifdef WIN
        // Wir restaurieren den Background-Modus bei jeder Texteingabe,
        // da einige Tools wie RichWin uns diesen hin- und wieder umsetzen
        if ( pFrame->maFrameData.mpGraphics &&
             pFrame->maFrameData.mpGraphics->maGraphicsData.mhDC )
            SetBkMode( pFrame->maFrameData.mpGraphics->maGraphicsData.mhDC, TRANSPARENT );

        pFrame->maFrameData.mbCompositionMode = FALSE;
        pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                    SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
        if ( pFrame->maFrameData.mbAtCursorIME )
            bDef = FALSE;
#endif
    }

    ImplSalYieldMutexRelease();

    return bDef;
}

// -----------------------------------------------------------------------

static void ImplHandleIMENotify( VCLWINDOW hWnd, WPARAM wParam )
{
    if ( wParam == (WPARAM)IMN_OPENCANDIDATE )
    {
        ImplSalYieldMutexAcquireWithWait();

        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame && pFrame->maFrameData.mbHandleIME &&
             pFrame->maFrameData.mbAtCursorIME )
        {
#ifdef WIN
            // Wir wollen den Cursor hiden
            pFrame->maFrameData.mbCandidateMode = TRUE;
            ImplHandleIMEComposition( hWnd, GCS_CURSORPOS );

            VCLWINDOW hWnd = pFrame->maFrameData.mhWnd;
            HIMC hIMC = ImmGetContext( hWnd );
            if ( hIMC )
            {
                LONG nBufLen = ImmGetCompositionStringW( hIMC, GCS_COMPSTR, 0, 0 );
                if ( nBufLen >= 1 )
                {
                    USHORT nCursorPos = LOWORD( ImmGetCompositionStringW( hIMC, GCS_CURSORPOS, 0, 0 ) );
                    SalExtTextInputPosEvent aEvt;
                    aEvt.mnTime         = GetMessageTime();
                    aEvt.mnFirstPos     = nCursorPos;
                    aEvt.mnChars        = nBufLen/sizeof(sal_Unicode) - nCursorPos;
                    aEvt.mpPosAry       = new SalExtCharPos[aEvt.mnChars];
                    memset( aEvt.mpPosAry, 0, aEvt.mnChars*sizeof(SalExtCharPos) );

                    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                SALEVENT_EXTTEXTINPUTPOS, (void*)&aEvt );

                    long nMinLeft   = aEvt.mpPosAry[0].mnX;
                    long nMinTop    = aEvt.mpPosAry[0].mnY;
                    long nMaxBottom = aEvt.mpPosAry[0].mnY+aEvt.mpPosAry[0].mnHeight;
                    long nMaxRight  = nMinLeft;
                    USHORT i = 0;
                    while ( i < aEvt.mnChars )
                    {
                        // Solange wir uns auf der gleichen Zeile bewegen,
                        // ermitteln wir die Rechteck-Grenzen
                        if ( !aEvt.mpPosAry[i].mnHeight ||
                             (aEvt.mpPosAry[i].mnY < nMaxBottom-1) )
                        {
                            if ( aEvt.mpPosAry[i].mnX < nMinLeft )
                                nMinLeft = aEvt.mpPosAry[i].mnX;
                            if ( aEvt.mpPosAry[i].mnX+aEvt.mpPosAry[0].mnWidth > nMaxRight )
                                nMaxRight = aEvt.mpPosAry[i].mnX+aEvt.mpPosAry[0].mnWidth;
                            if ( aEvt.mpPosAry[i].mnY < nMinTop )
                                nMinTop = aEvt.mpPosAry[i].mnY;
                            i++;
                        }
                        else
                            break;
                    }

                    CANDIDATEFORM aForm;
                    aForm.dwIndex           = 0;
                    aForm.dwStyle           = CFS_EXCLUDE;
                    aForm.ptCurrentPos.x    = aEvt.mpPosAry[0].mnX;
                    aForm.ptCurrentPos.y    = nMaxBottom+1;
                    aForm.rcArea.left       = nMinLeft;
                    aForm.rcArea.top        = nMinTop;
                    aForm.rcArea.right      = nMaxRight+1;
                    aForm.rcArea.bottom     = nMaxBottom+1;
                    ImmSetCandidateWindow( hIMC, &aForm );

                    delete aEvt.mpPosAry;
                }

                ImmReleaseContext( hWnd, hIMC );
            }
#endif
        }

        ImplSalYieldMutexRelease();
    }
    else if ( wParam == (WPARAM)IMN_CLOSECANDIDATE )
    {
        ImplSalYieldMutexAcquireWithWait();
        SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
            pFrame->maFrameData.mbCandidateMode = FALSE;
        ImplSalYieldMutexRelease();
    }
}


// -----------------------------------------------------------------------

void SalTestMouseLeave()
{
    SalData* pSalData = GetSalData();

#ifdef WIN
    if ( pSalData->mhWantLeaveMsg && !::GetCapture() )
    {
        POINT aPt;
        GetCursorPos( &aPt );
        if ( pSalData->mhWantLeaveMsg != WindowFromPoint( aPt ) )
            ImplSendMessage( pSalData->mhWantLeaveMsg, SAL_MSG_MOUSELEAVE, 0, MAKELPARAM( aPt.x, aPt.y ) );
    }
#endif
}

// -----------------------------------------------------------------------

static int ImplSalWheelMousePos( VCLWINDOW hWnd, UINT nMsg, WPARAM wParam,
    LPARAM lParam, LRESULT& rResult )
{
#ifdef WIN
    POINT aPt;
    POINT aScreenPt;
    aScreenPt.x = (short)LOWORD( lParam );
    aScreenPt.y = (short)HIWORD( lParam );
    // Child-Fenster suchen, welches an der entsprechenden
    // Position liegt
    VCLWINDOW hChildWnd;
    VCLWINDOW hWheelWnd = hWnd;
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
#endif

    return TRUE;
}

// -----------------------------------------------------------------------

LRESULT CALLBACK SalFrameWndProc( VCLWINDOW hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT     nRet = 0;
#ifdef WIN
    static int  bInWheelMsg = FALSE;

    // By WM_CRETAE we connect the frame with the window handle
    if ( nMsg == WM_CREATE )
    {
        // Window-Instanz am Windowhandle speichern
        // Can also be used for the W-Version, because the struct
        // to access lpCreateParams is the same structure
        CREATESTRUCTA* pStruct = (CREATESTRUCTA*)lParam;
        SalFrame* pFrame = (SalFrame*)pStruct->lpCreateParams;
        SetWindowPtr( hWnd, pFrame );
        // VCLWINDOW schon hier setzen, da schon auf den Instanzdaten
        // gearbeitet werden kann, wenn Messages waehrend
        // CreateWindow() gesendet werden
        pFrame->maFrameData.mhWnd = hWnd;
        pFrame->maFrameData.maSysData.hWnd = hWnd;
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
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
            ImplSalYieldMutexAcquireWithWait();
            rDef = !ImplHandleKeyMsg( hWnd, nMsg, wParam, lParam );
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

        case WM_SYSCOMMAND:
            ImplSalYieldMutexAcquireWithWait();
            nRet = ImplHandleSysCommand( hWnd, wParam, lParam );
            ImplSalYieldMutexRelease();
            if ( nRet )
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

        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        case SAL_MSG_POSTFOCUS:
            ImplHandleFocusMsg( hWnd );
            rDef = FALSE;
            break;

        case WM_CLOSE:
            ImplHandleCloseMsg( hWnd );
            rDef = FALSE;
            break;

        case WM_QUERYENDSESSION:
            nRet = !ImplHandleShutDownMsg( hWnd );
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
            ImplSalShow( hWnd, (BOOL)wParam );
            rDef = FALSE;
            break;

        case WM_INPUTLANGCHANGE:
            ImplHandleInputLangChange( hWnd, wParam, lParam );
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
                VCLWINDOW hWheelWnd = ::GetFocus();
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
#endif

    return nRet;
}

#ifdef WIN
LRESULT CALLBACK SalFrameWndProcA( VCLWINDOW hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalFrameWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
        nRet = DefWindowProcA( hWnd, nMsg, wParam, lParam );
    return nRet;
}
#endif

#ifdef WIN
LRESULT CALLBACK SalFrameWndProcW( VCLWINDOW hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalFrameWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
        nRet = DefWindowProcW( hWnd, nMsg, wParam, lParam );
    return nRet;
}
#endif

// -----------------------------------------------------------------------

BOOL ImplHandleGlobalMsg( VCLWINDOW hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& rlResult )
{
#ifdef WIN
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
#endif
        return FALSE;
}
