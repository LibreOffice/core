/*************************************************************************
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:35 $
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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ZTC__
#define _Seg16  _far16
#define _Far16  _far16
#define _System _syscall
#define _Pascal _pascal
#define _Cdecl  _cdecl
#endif

#define BOOL     PM_BOOL
#define BYTE     PM_BYTE
#define USHORT   PM_USHORT
#define ULONG    PM_ULONG

#define INCL_DOS
#define INCL_PM
#include <os2.h>
#include <os2im.h>

#undef BOOL
#undef BYTE
#undef USHORT
#undef ULONG

#ifdef __cplusplus
}
#endif

// =======================================================================

#define _SV_SALFRAME_CXX

#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define private public

#ifndef _SV_SALLANG_HXX
#include <sallang.hxx>
#endif
#ifndef _SV_SALIDS_HRC
#include <salids.hrc>
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
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
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

// =======================================================================

// Toolkit4 defines
#ifndef SV_FULLWINDOWDRAG
#define SV_FULLWINDOWDRAG          99
#endif

// =======================================================================

static eImplKeyboardLanguage = LANGUAGE_DONTKNOW;

// =======================================================================

#ifdef ENABLE_IME

struct ImplSalIMEProc
{
    ULONG       nOrd;
    PFN*        pProc;
};

#define SAL_IME_PROC_COUNT          12

// -----------------------------------------------------------------------

static SalIMEData* GetSalIMEData()
{
    SalData* pSalData = GetSalData();

    if ( !pSalData->mbIMEInit )
    {
        pSalData->mbIMEInit = TRUE;

        HMODULE hMod = 0;
        if ( 0 == DosLoadModule( NULL, 0, "OS2IM", &hMod ) )
        {
            SalIMEData*     pIMEData = new SalIMEData;
            BOOL            bError = FALSE;
            ImplSalIMEProc  aProcAry[SAL_IME_PROC_COUNT] =
            {
            { 101, (PFN*)&(pIMEData->mpAssocIME) },
            { 104, (PFN*)&(pIMEData->mpGetIME) },
            { 106, (PFN*)&(pIMEData->mpReleaseIME) },
            { 117, (PFN*)&(pIMEData->mpSetConversionFont) },
            { 144, (PFN*)&(pIMEData->mpSetConversionFontSize) },
            { 118, (PFN*)&(pIMEData->mpGetConversionString) },
            { 122, (PFN*)&(pIMEData->mpGetResultString) },
            { 115, (PFN*)&(pIMEData->mpSetCandidateWin) },
            { 130, (PFN*)&(pIMEData->mpQueryIMEProperty) },
            { 131, (PFN*)&(pIMEData->mpRequestIME) },
            { 128, (PFN*)&(pIMEData->mpSetIMEMode) },
            { 127, (PFN*)&(pIMEData->mpQueryIMEMode) }
            };

            pIMEData->mhModIME = hMod;
            for ( USHORT i = 0; i < SAL_IME_PROC_COUNT; i++ )
            {
                if ( 0 != DosQueryProcAddr( pIMEData->mhModIME, aProcAry[i].nOrd, 0, aProcAry[i].pProc ) )
                {
                    bError = TRUE;
                    break;
                }
            }

            if ( bError )
            {
                DosFreeModule( pIMEData->mhModIME );
                delete pIMEData;
            }
            else
                pSalData->mpIMEData = pIMEData;
        }
    }

    return pSalData->mpIMEData;
}

// -----------------------------------------------------------------------

void ImplReleaseSALIMEData()
{
    SalData* pSalData = GetSalData();

    if ( pSalData->mpIMEData )
    {
        DosFreeModule( pSalData->mpIMEData->mhModIME );
        delete pSalData->mpIMEData;
    }
}

#endif

// =======================================================================

static void ImplSaveFrameState( SalFrame* pFrame )
{
    if ( !pFrame->maFrameData.mbFullScreen )
    {
        SWP aSWP;

        // Query actual state
        WinQueryWindowPos( pFrame->maFrameData.mhWndFrame, &aSWP );

        if ( aSWP.fl & SWP_MINIMIZE )
            pFrame->maFrameData.maState.mnState |= SAL_FRAMESTATE_MINIMIZED;
        else if ( aSWP.fl & SWP_MAXIMIZE )
        {
            pFrame->maFrameData.maState.mnState &= ~SAL_FRAMESTATE_MINIMIZED;
            pFrame->maFrameData.maState.mnState |= SAL_FRAMESTATE_MAXIMIZED;
            pFrame->maFrameData.mbRestoreMaximize = TRUE;
        }
        else
        {
            pFrame->maFrameData.maState.mnState &= ~(SAL_FRAMESTATE_MINIMIZED | SAL_FRAMESTATE_MAXIMIZED);
            pFrame->maFrameData.maState.mnX      = aSWP.x;
            pFrame->maFrameData.maState.mnY      = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN )-(aSWP.y+aSWP.cy);
            pFrame->maFrameData.maState.mnWidth  = aSWP.cx;
            pFrame->maFrameData.maState.mnHeight = aSWP.cy;
            pFrame->maFrameData.mbRestoreMaximize = FALSE;
        }
    }
}

// -----------------------------------------------------------------------

long ImplSalCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
    return 0;
}

// -----------------------------------------------------------------------

SalFrame* GetSalDefaultFrame()
{
    SalData* pSalData = GetSalData();

    // Wenn kein Dummy-Frame existiert, dann legen wir uns einen an
    if ( !pSalData->mpDummyFrame )
        pSalData->mpDummyFrame = pSalData->mpFirstInstance->CreateFrame( NULL, 0 );

    return pSalData->mpDummyFrame;
}

// -----------------------------------------------------------------------

static void ImplSalCalcFrameSize( const SalFrame* pFrame,
                                  LONG& rFrameX, LONG& rFrameY, LONG& rCaptionY )
{
    if ( pFrame->maFrameData.mnOS2Style & FCF_SIZEBORDER )
    {
        rFrameX = WinQuerySysValue( HWND_DESKTOP, SV_CXSIZEBORDER );
        rFrameY = WinQuerySysValue( HWND_DESKTOP, SV_CYSIZEBORDER );
    }
    else if ( pFrame->maFrameData.mnOS2Style & FCF_DLGBORDER )
    {
        rFrameX = WinQuerySysValue( HWND_DESKTOP, SV_CXDLGFRAME );
        rFrameY = WinQuerySysValue( HWND_DESKTOP, SV_CYDLGFRAME );
    }
    else if ( pFrame->maFrameData.mnOS2Style & FCF_BORDER )
    {
        rFrameX = WinQuerySysValue( HWND_DESKTOP, SV_CXBORDER );
        rFrameY = WinQuerySysValue( HWND_DESKTOP, SV_CYBORDER );
    }
    else
    {
        rFrameX = 0;
        rFrameY = 0;
    }
    if ( pFrame->maFrameData.mnOS2Style & (FCF_TITLEBAR | FCF_SYSMENU) )
        rCaptionY = WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );
    else
        rCaptionY = 0;
}

// -----------------------------------------------------------------------

static void ImplSalCalcFullScreenSize( const SalFrame* pFrame,
                                       LONG& rX, LONG& rY, LONG& rDX, LONG& rDY )
{
    // set window to screen size
    LONG nFrameX;
    LONG nFrameY;
    LONG nCaptionY;
    LONG nScreenDX = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    LONG nScreenDY = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    // Framegroessen berechnen
    ImplSalCalcFrameSize( pFrame, nFrameX, nFrameY, nCaptionY );

    rX  = -nFrameX;
    rY  = -nFrameY;
    rDX = nScreenDX+(nFrameX*2);
    rDY = nScreenDY+(nFrameY*2)+nCaptionY;
}

// -----------------------------------------------------------------------

static void ImplSalFrameFullScreenPos( SalFrame* pFrame, BOOL bAlways = FALSE )
{
    SWP aSWP;
    WinQueryWindowPos( pFrame->maFrameData.mhWndFrame, &aSWP );
    if ( bAlways || !(aSWP.fl & SWP_MINIMIZE) )
    {
        // set window to screen size
        LONG nX;
        LONG nY;
        LONG nWidth;
        LONG nHeight;
        ImplSalCalcFullScreenSize( pFrame, nX, nY, nWidth, nHeight );
        WinSetWindowPos( pFrame->maFrameData.mhWndFrame, 0,
                         nX, nY, nWidth, nHeight,
                         SWP_MOVE | SWP_SIZE );
    }
}

// -----------------------------------------------------------------------

// Uebersetzungstabelle von System-Keycodes in StarView-Keycodes
#define KEY_TAB_SIZE     (VK_ENDDRAG+1)

static USHORT TranslateKey[KEY_TAB_SIZE] =
{
    // StarView-Code      System-Code                         Index
    0,                    //                                  0x00
    0,                    // VK_BUTTON1                       0x01
    0,                    // VK_BUTTON2                       0x02
    0,                    // VK_BUTTON3                       0x03
    0,                    // VK_BREAK                         0x04
    KEY_BACKSPACE,        // VK_BACKSPACE                     0x05
    KEY_TAB,              // VK_TAB                           0x06
    KEY_TAB,              // VK_BACKTAB                       0x07
    KEY_RETURN,           // VK_NEWLINE                       0x08
    0,                    // VK_SHIFT                         0x09
    0,                    // VK_CTRL                          0x0A
    0,                    // VK_ALT                           0x0B
    0,                    // VK_ALTGRAF                       0x0C
    0,                    // VK_PAUSE                         0x0D
    0,                    // VK_CAPSLOCK                      0x0E
    KEY_ESCAPE,           // VK_ESC                           0x0F
    KEY_SPACE,            // VK_SPACE                         0x10
    KEY_PAGEUP,           // VK_PAGEUP                        0x11
    KEY_PAGEDOWN,         // VK_PAGEDOWN                      0x12
    KEY_END,              // VK_END                           0x13
    KEY_HOME,             // VK_HOME                          0x14
    KEY_LEFT,             // VK_LEFT                          0x15
    KEY_UP,               // VK_UP                            0x16
    KEY_RIGHT,            // VK_RIGHT                         0x17
    KEY_DOWN,             // VK_DOWN                          0x18
    0,                    // VK_PRINTSCRN                     0x19
    KEY_INSERT,           // VK_INSERT                        0x1A
    KEY_DELETE,           // VK_DELETE                        0x1B
    0,                    // VK_SCRLLOCK                      0x1C
    0,                    // VK_NUMLOCK                       0x1D
    KEY_RETURN,           // VK_ENTER                         0x1E
    0,                    // VK_SYSRQ                         0x1F
    KEY_F1,               // VK_F1                            0x20
    KEY_F2,               // VK_F2                            0x21
    KEY_F3,               // VK_F3                            0x22
    KEY_F4,               // VK_F4                            0x23
    KEY_F5,               // VK_F5                            0x24
    KEY_F6,               // VK_F6                            0x25
    KEY_F7,               // VK_F7                            0x26
    KEY_F8,               // VK_F8                            0x27
    KEY_F9,               // VK_F9                            0x28
    KEY_F10,              // VK_F10                           0x29
    KEY_F11,              // VK_F11                           0x2A
    KEY_F12,              // VK_F12                           0x2B
    KEY_F13,              // VK_F13                           0x2C
    KEY_F14,              // VK_F14                           0x2D
    KEY_F15,              // VK_F15                           0x2E
    KEY_F16,              // VK_F16                           0x2F
    KEY_F17,              // VK_F17                           0x30
    KEY_F18,              // VK_F18                           0x31
    KEY_F19,              // VK_F19                           0x32
    KEY_F20,              // VK_F20                           0x33
    KEY_F21,              // VK_F21                           0x34
    KEY_F22,              // VK_F22                           0x35
    KEY_F23,              // VK_F23                           0x36
    KEY_F24,              // VK_F24                           0x37
    0                     // VK_ENDDRAG                       0x38
};

// =======================================================================

SalFrame* ImplSalCreateFrame( SalInstance* pInst, SalFrame* pParent, ULONG nSalFrameStyle )
{
    SalData*    pSalData = GetSalData();
    SalFrame*   pFrame = new SalFrame;
    HWND        hWndFrame;
    HWND        hWndClient;
    PM_ULONG    nFrameFlags = FCF_NOBYTEALIGN | FCF_NOMOVEWITHOWNER | FCF_SCREENALIGN;
    PM_ULONG    nFrameStyle = 0;
    PM_ULONG    nClientStyle = WS_CLIPSIBLINGS;

    // determine creation data (bei Moveable nehmen wir DLG-Border, damit
    // es besser aussieht)
    if ( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE )
        nFrameFlags |= FCF_SIZEBORDER | FCF_SYSMENU;
    else if ( nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
        nFrameFlags |= FCF_DLGBORDER;
    else if ( nSalFrameStyle & SAL_FRAME_STYLE_BORDER )
        nFrameFlags |= FCF_BORDER;
    if ( nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
        nFrameFlags |= FCF_TITLEBAR | FCF_SYSMENU;
    if ( nSalFrameStyle & SAL_FRAME_STYLE_MINABLE )
        nFrameFlags |= FCF_MINBUTTON | FCF_SYSMENU;
    if ( nSalFrameStyle & SAL_FRAME_STYLE_MAXABLE )
        nFrameFlags |= FCF_MAXBUTTON | FCF_SYSMENU;
    if ( nSalFrameStyle & SAL_FRAME_STYLE_DEFAULT )
        nFrameFlags |= FCF_TASKLIST;

    // create frame
    pSalData->mpCreateFrame = pFrame;
    hWndFrame = WinCreateStdWindow( HWND_DESKTOP, nFrameStyle, &nFrameFlags,
                                    (PSZ)SAL_FRAME_CLASSNAME, (PSZ)"",
                                    nClientStyle, 0, 0, &hWndClient );

    if ( !hWndFrame )
    {
        delete pFrame;
        return NULL;
    }

    // Parent setzen (Owner)
    if ( pParent )
        WinSetOwner( hWndFrame, pParent->maFrameData.mhWndClient );

    // Icon setzen
    if ( nFrameFlags & FCF_MINBUTTON )
        WinSendMsg( hWndFrame, WM_SETICON, (MPARAM)pInst->maInstData.mhAppIcon, (MPARAM)0 );

    // Frames subclassen, da wir auch dort die eine oder andere Message
    // abfangen wollen oder anders behandeln wollen
    aSalShlData.mpOldFrameProc = WinSubclassWindow( hWndFrame, SalFrameFrameProc );

    // init frame data
    pFrame->maFrameData.mhWndFrame      = hWndFrame;
    pFrame->maFrameData.mhWndClient     = hWndClient;
    pFrame->maFrameData.mhAB            = pInst->maInstData.mhAB;
    pFrame->maFrameData.mnStyle         = nSalFrameStyle;
    pFrame->maFrameData.mnOS2Style      = nFrameFlags;
    pFrame->maFrameData.maSysData.hWnd  = hWndClient;

    // determine show style
    if ( nSalFrameStyle & SAL_FRAME_STYLE_DEFAULT )
    {
        pSalData->mpDefaultFrame = pFrame;

        // restore saved position
        SWP aSWP;
        memset( &aSWP, 0, sizeof( aSWP ) );
        WinQueryTaskSizePos( pInst->maInstData.mhAB, 0, &aSWP );
        WinSetWindowPos( hWndFrame, HWND_TOP,
                         aSWP.x, aSWP.y, aSWP.cx, aSWP.cy,
                         SWP_ZORDER | SWP_MOVE | SWP_SIZE |
                         (aSWP.fl & (SWP_RESTORE | SWP_MINIMIZE | SWP_MAXIMIZE)) );
        if ( aSWP.fl & (SWP_MINIMIZE | SWP_MAXIMIZE) )
            pFrame->maFrameData.mbOverwriteState = FALSE;
        if ( !(aSWP.fl & SWP_MINIMIZE) )
            WinSetWindowPos( hWndFrame, 0, 0, 0, 0, 0, SWP_MAXIMIZE );
    }
    else
    {
        SWP aSWP;
        RECTL rectl;
        memset( &aSWP, 0, sizeof( aSWP ) );
        WinQueryWindowRect( HWND_DESKTOP, &rectl );
        aSWP.x = rectl.xLeft + 10;
        aSWP.y = rectl.yBottom + 10;
        aSWP.cx = rectl.xRight - rectl.xLeft - 20;
        aSWP.cy = rectl.yTop - rectl.yBottom - 20;
        WinSetWindowPos( hWndFrame, HWND_TOP,
                         aSWP.x, aSWP.y, aSWP.cx, aSWP.cy,
                         SWP_MOVE | SWP_SIZE | SWP_ZORDER );
    }

    // disable close
    if ( !(nSalFrameStyle & SAL_FRAME_STYLE_CLOSEABLE) )
    {
        HWND hSysMenu = WinWindowFromID( hWndFrame, FID_SYSMENU );
        if ( hSysMenu )
        {
            WinSendMsg( hSysMenu,
                        MM_SETITEMATTR,
                        MPFROM2SHORT( SC_CLOSE, TRUE ),
                        MPFROM2SHORT( MIA_DISABLED, MIA_DISABLED ) );
        }
    }

#ifdef ENABLE_IME
/*
    // Input-Context einstellen
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        pFrame->maFrameData.mhIMEContext = 0;
        if ( 0 != pIMEData->mpAssocIME( hWndClient, pFrame->maFrameData.mhIMEContext, &pFrame->maFrameData.mhDefIMEContext ) )
            pFrame->maFrameData.mhDefIMEContext = 0;
    }
    else
    {
        pFrame->maFrameData.mhIMEContext = 0;
        pFrame->maFrameData.mhDefIMEContext = 0;
    }
*/
#endif

    ImplSaveFrameState( pFrame );
    pFrame->maFrameData.mbDefPos = TRUE;

    return pFrame;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

// =======================================================================

SalFrame::SalFrame()
{
    SalData* pSalData = GetSalData();

    maFrameData.mbGraphics          = NULL;
    maFrameData.mhPointer           = WinQuerySysPointer( HWND_DESKTOP, SPTR_ARROW, FALSE );
    maFrameData.mpGraphics          = NULL;
    maFrameData.mpInst              = NULL;
    maFrameData.mpProc              = ImplSalCallbackDummy;
    maFrameData.mbFullScreen        = FALSE;
    maFrameData.mbAllwayOnTop       = FALSE;
    maFrameData.mbVisible           = FALSE;
    maFrameData.mbMinHide           = FALSE;
    maFrameData.mbInShow            = FALSE;
    maFrameData.mbRestoreMaximize   = FALSE;
    maFrameData.mbInMoveMsg         = FALSE;
    maFrameData.mbInSizeMsg         = FALSE;
    maFrameData.mbDefPos            = TRUE;
    maFrameData.mbOverwriteState    = TRUE;
    maFrameData.mbHandleIME         = FALSE;
    maFrameData.mbConversionMode    = FALSE;
    maFrameData.mbCandidateMode     = FALSE;
    memset( &maFrameData.maState, 0, sizeof( SalFrameState ) );
    maFrameData.maSysData.nSize     = sizeof( SystemEnvData );

    // insert frame in framelist
    maFrameData.mpNextFrame = pSalData->mpFirstFrame;
    pSalData->mpFirstFrame = this;
}

// -----------------------------------------------------------------------

SalFrame::~SalFrame()
{
    SalData* pSalData = GetSalData();

    // destroy DC
    if ( maFrameData.mpGraphics )
    {
        ImplSalDeInitGraphics( &(maFrameData.mpGraphics->maGraphicsData) );
        WinReleasePS( maFrameData.mpGraphics->maGraphicsData.mhPS );
        delete maFrameData.mpGraphics;
    }

    // destroy system frame
    WinDestroyWindow( maFrameData.mhWndFrame );

    // reset default and dummy frame
    if ( pSalData->mpDefaultFrame == this )
        pSalData->mpDefaultFrame = 0;
    if ( pSalData->mpDummyFrame == this )
        pSalData->mpDummyFrame = 0;

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

static HDC ImplWinGetDC( HWND hWnd )
{
    HDC hDC = WinQueryWindowDC( hWnd );
    if ( !hDC )
        hDC = WinOpenWindowDC( hWnd );
    return hDC;
}

// -----------------------------------------------------------------------

SalGraphics* SalFrame::GetGraphics()
{
    if ( maFrameData.mbGraphics )
        return NULL;

    if ( !maFrameData.mpGraphics )
    {
        SalData* pSalData = GetSalData();
        maFrameData.mpGraphics = new SalGraphics;
        maFrameData.mpGraphics->maGraphicsData.mhPS      = WinGetPS( maFrameData.mhWndClient );
        maFrameData.mpGraphics->maGraphicsData.mhDC      = ImplWinGetDC( maFrameData.mhWndClient );
        maFrameData.mpGraphics->maGraphicsData.mhWnd     = maFrameData.mhWndClient;
        maFrameData.mpGraphics->maGraphicsData.mnHeight  = maFrameData.mnHeight;
        maFrameData.mpGraphics->maGraphicsData.mbPrinter = FALSE;
        maFrameData.mpGraphics->maGraphicsData.mbVirDev  = FALSE;
        maFrameData.mpGraphics->maGraphicsData.mbWindow  = TRUE;
        maFrameData.mpGraphics->maGraphicsData.mbScreen  = TRUE;
        ImplSalInitGraphics( &(maFrameData.mpGraphics->maGraphicsData) );
        maFrameData.mbGraphics = TRUE;
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
    return (BOOL)WinPostMsg( maFrameData.mhWndClient, SAL_MSG_USEREVENT, 0, (MPARAM)pData );
}

// -----------------------------------------------------------------------

void SalFrame::SetTitle( const XubString& rTitle )
{
    // set window title
    WinSetWindowText( maFrameData.mhWndFrame, (PSZ)(const char*)rTitle );
}

// -----------------------------------------------------------------------

void SalFrame::Show( BOOL bVisible )
{
    maFrameData.mbVisible = bVisible;
    if ( bVisible )
    {
        maFrameData.mbOverwriteState = TRUE;
        maFrameData.mbInShow = TRUE;
        WinSetWindowPos( maFrameData.mhWndFrame, 0, 0, 0, 0, 0, SWP_SHOW | SWP_ACTIVATE );
        maFrameData.mbInShow = FALSE;
        WinUpdateWindow( maFrameData.mhWndClient );
    }
    else
    {
        maFrameData.mbInShow = TRUE;
        WinSetWindowPos( maFrameData.mhWndFrame, 0, 0, 0, 0, 0, SWP_HIDE );
        maFrameData.mbInShow = FALSE;
    }
}

// -----------------------------------------------------------------------

void SalFrame::SetClientSize( long nWidth, long nHeight )
{
    // calculation frame size
    LONG    nX;
    LONG    nY;
    LONG    nFrameX;
    LONG    nFrameY;
    LONG    nCaptionY;
    LONG    nScreenWidth   = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    LONG    nScreenHeight  = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    // Framegroessen berechnen
    ImplSalCalcFrameSize( this, nFrameX, nFrameY, nCaptionY );

    // adjust give size
    nWidth  += 2*nFrameX;
    nHeight += 2*nFrameY + nCaptionY;

    // Default-Position, dann zentrieren, ansonsten Position beibehalten
    if ( maFrameData.mbDefPos )
    {
        // calculate bottom left corner of frame
        nX = (nScreenWidth-nWidth)/2;
        nY = (nScreenHeight-nHeight)/2;
        maFrameData.mbDefPos = FALSE;
    }
    else
    {
        SWP aSWP;
        WinQueryWindowPos( maFrameData.mhWndFrame, &aSWP );

        nX = aSWP.x;
        nY = aSWP.y+(aSWP.cy-nHeight);

        // Fenster einpassen
        if ( nX + nWidth > nScreenWidth )
            nX = nScreenWidth - nWidth;
        if ( nY < 0 )
            nY = 0;
    }

    // Fenster einpassen
    if ( nX < 0 )
        nX = 0;
    if ( nY > nScreenHeight-nHeight )
        nY = nScreenHeight-nHeight;

    // set new position
    WinSetWindowPos( maFrameData.mhWndFrame,
                     0,
                     nX, nY,
                     nWidth, nHeight,
                     SWP_MOVE | SWP_SIZE | SWP_RESTORE );
}

// -----------------------------------------------------------------------

void SalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    // Wenn wir von aussen gehidet werden (beispielsweise Hide-Button)
    // muessen wir eine Groesse von 0,0 vorgaukeln, damit Dialoge noch
    // als System-Fenster angezeigt werden
    if ( maFrameData.mbMinHide )
    {
        rWidth  = 0;
        rHeight = 0;
    }
    else
    {
        rWidth  = maFrameData.mnWidth;
        rHeight = maFrameData.mnHeight;
    }
}

// -----------------------------------------------------------------------

void SalFrame::SetWindowState( const SalFrameState* pState )
{
    LONG nX     = pState->mnX;
    LONG nY     = pState->mnY;
    LONG nCX    = pState->mnWidth;
    LONG nCY    = pState->mnHeight;

    // Fenster-Position/Groesse in den Bildschirm einpassen
    LONG nScreenWidth  = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    LONG nScreenHeight = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
    if ( nCX > nScreenWidth )
    {
        nX  = 0;
        nCX = nScreenWidth;
    }
    if ( nCY > nScreenHeight )
    {
        nY  = 0;
        nCY = nScreenHeight;
    }

    // Y is under OS2 bottom align
    nY = nScreenHeight-(nY+nCY);

    if ( !maFrameData.mbOverwriteState ||
         (pState->mnState & (SAL_FRAMESTATE_MINIMIZED | SAL_FRAMESTATE_MAXIMIZED)) )
    {
        if ( maFrameData.mbOverwriteState )
        {
            PM_ULONG nSizeStyle;
            if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
                nSizeStyle = SWP_MINIMIZE;
            else
                nSizeStyle = SWP_MAXIMIZE;
            WinSetWindowPos( maFrameData.mhWndFrame, 0, 0, 0, 0, 0, nSizeStyle );
        }

        WinSetWindowUShort( maFrameData.mhWndFrame, QWS_XRESTORE, nX );
        WinSetWindowUShort( maFrameData.mhWndFrame, QWS_YRESTORE, nY );
        WinSetWindowUShort( maFrameData.mhWndFrame, QWS_CXRESTORE, nCX );
        WinSetWindowUShort( maFrameData.mhWndFrame, QWS_CYRESTORE, nCY );
    }
    else
    {
        PM_ULONG nSizeStyle = SWP_MOVE | SWP_SIZE | SWP_RESTORE;
        WinSetWindowPos( maFrameData.mhWndFrame, 0, nX, nY, nCX, nCY, nSizeStyle );
    }
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
    if ( bFullScreen )
    {
        // save old position
        memset( &maFrameData.maFullScreenRect, 0, sizeof( SWP ) );
        WinQueryWindowPos( maFrameData.mhWndFrame, &maFrameData.maFullScreenRect );

        // set window to screen size
        ImplSalFrameFullScreenPos( this, TRUE );
    }
    else
    {
        WinSetWindowPos( maFrameData.mhWndFrame,
                         0,
                         maFrameData.maFullScreenRect.x, maFrameData.maFullScreenRect.y,
                         maFrameData.maFullScreenRect.cx, maFrameData.maFullScreenRect.cy,
                         SWP_MOVE | SWP_SIZE );
    }
}

// -----------------------------------------------------------------------

void SalFrame::StartPresentation( BOOL bStart )
{
    // SysSetObjectData("<WP_DESKTOP>","Autolockup=no"); oder OS2.INI: PM_Lockup
}

// -----------------------------------------------------------------------

void SalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
    maFrameData.mbAllwayOnTop = bOnTop;
}

// -----------------------------------------------------------------------

void SalFrame::ToTop( USHORT nFlags )
{
    PM_ULONG    nStyle = 0;
    SWP         aSWP;
    WinQueryWindowPos( maFrameData.mhWndFrame, &aSWP );
    if ( aSWP.fl & SWP_MINIMIZE )
    {
        if ( !(nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN) )
            return;

        if ( maFrameData.mbRestoreMaximize )
            nStyle |= SWP_MAXIMIZE;
        else
            nStyle |= SWP_RESTORE;
    }

    WinSetWindowPos( maFrameData.mhWndFrame, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE | SWP_ZORDER | nStyle );
    WinSetFocus( HWND_DESKTOP, maFrameData.mhWndClient );
}

// -----------------------------------------------------------------------

void SalFrame::SetPointer( PointerStyle ePointerStyle )
{
    struct ImplPtrData
    {
        HPOINTER        mhPointer;
        PM_ULONG        mnSysId;
        PM_ULONG        mnOwnId;
    };

    static ImplPtrData aImplPtrTab[POINTER_COUNT] =
    {
    { 0, SPTR_ARROW, 0 },                           // POINTER_ARROW
    { 0, 0, SAL_RESID_POINTER_NULL },               // POINTER_NULL
    { 0, SPTR_WAIT, 0 },                            // POINTER_WAIT
    { 0, SPTR_TEXT, 0 },                            // POINTER_BEAM
    { 0, 0, SAL_RESID_POINTER_HELP },               // POINTER_HELP
    { 0, 0, SAL_RESID_POINTER_CROSS },              // POINTER_CROSS
    { 0, 0, SAL_RESID_POINTER_MOVE },               // POINTER_MOVE
    { 0, SPTR_SIZENS, 0 },                          // POINTER_NSIZE
    { 0, SPTR_SIZENS, 0 },                          // POINTER_SSIZE
    { 0, SPTR_SIZEWE, 0 },                          // POINTER_WSIZE
    { 0, SPTR_SIZEWE, 0 },                          // POINTER_ESIZE
    { 0, SPTR_SIZENWSE, 0 },                        // POINTER_NWSIZE
    { 0, SPTR_SIZENESW, 0 },                        // POINTER_NESIZE
    { 0, SPTR_SIZENESW, 0 },                        // POINTER_SWSIZE
    { 0, SPTR_SIZENWSE, 0 },                        // POINTER_SESIZE
    { 0, SPTR_SIZENS, 0 },                          // POINTER_WINDOW_NSIZE
    { 0, SPTR_SIZENS, 0 },                          // POINTER_WINDOW_SSIZE
    { 0, SPTR_SIZEWE, 0 },                          // POINTER_WINDOW_WSIZE
    { 0, SPTR_SIZEWE, 0 },                          // POINTER_WINDOW_ESIZE
    { 0, SPTR_SIZENWSE, 0 },                        // POINTER_WINDOW_NWSIZE
    { 0, SPTR_SIZENESW, 0 },                        // POINTER_WINDOW_NESIZE
    { 0, SPTR_SIZENESW, 0 },                        // POINTER_WINDOW_SWSIZE
    { 0, SPTR_SIZENWSE, 0 },                        // POINTER_WINDOW_SESIZE
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
    { 0, SPTR_ILLEGAL, 0 },                         // POINTER_NOTALLOWED
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
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NSWE }     // POINTER_AUTOSCROLL_NSWE
    };

#if POINTER_COUNT != 85
#error New Pointer must be defined!
#endif

    // Mousepointer loaded ?
    if ( !aImplPtrTab[ePointerStyle].mhPointer )
    {
        if ( aImplPtrTab[ePointerStyle].mnOwnId )
            aImplPtrTab[ePointerStyle].mhPointer = ImplLoadPointer( aImplPtrTab[ePointerStyle].mnOwnId );
        else
            aImplPtrTab[ePointerStyle].mhPointer = WinQuerySysPointer( HWND_DESKTOP, aImplPtrTab[ePointerStyle].mnSysId, FALSE );
    }

    // Unterscheidet sich der Mauspointer, dann den neuen setzen
    if ( maFrameData.mhPointer != aImplPtrTab[ePointerStyle].mhPointer )
    {
        maFrameData.mhPointer = aImplPtrTab[ePointerStyle].mhPointer;
        WinSetPointer( HWND_DESKTOP, maFrameData.mhPointer );
    }
}

// -----------------------------------------------------------------------

void SalFrame::CaptureMouse( BOOL bCapture )
{
    if ( bCapture )
        WinSetCapture( HWND_DESKTOP, maFrameData.mhWndClient );
    else
        WinSetCapture( HWND_DESKTOP, 0 );
}

// -----------------------------------------------------------------------

void SalFrame::SetPointerPos( long nX, long nY )
{
    POINTL aPt;
    aPt.x = nX;
    aPt.y = maFrameData.mnHeight - nY - 1;  // convert sal coords to sys
    WinMapWindowPoints( maFrameData.mhWndClient, HWND_DESKTOP, &aPt, 1 );
    WinSetPointerPos( HWND_DESKTOP, aPt.x, aPt.y );
}

// -----------------------------------------------------------------------

void SalFrame::Flush()
{
}

// -----------------------------------------------------------------------

void SalFrame::Sync()
{
}

// -----------------------------------------------------------------------

void SalFrame::SetInputContext( SalInputContext* pContext )
{
#ifdef ENABLE_IME
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND hWnd = maFrameData.mhWndClient;
        HIMI hIMI = 0;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            PM_ULONG nInputMode;
            PM_ULONG nConversionMode;
            if ( 0 == pIMEData->mpQueryIMEMode( hIMI, &nInputMode, &nConversionMode ) )
            {
                if ( pContext->mnOptions & SAL_INPUTCONTEXT_TEXT )
                {
                    nInputMode &= ~IMI_IM_IME_DISABLE;
                    if ( pContext->mnOptions & SAL_INPUTCONTEXT_EXTTEXTINPUT_OFF )
                        nInputMode &= ~IMI_IM_IME_ON;
// !!! Da derzeit ueber das OS2-IME-UI der IME-Mode nicht einschaltbar ist !!!
//                    if ( SAL_INPUTCONTEXT_EXTTEXTINPUT_ON )
                        nInputMode |= IMI_IM_IME_ON;
                }
                else
                    nInputMode |= IMI_IM_IME_DISABLE;
                pIMEData->mpSetIMEMode( hIMI, nInputMode, nConversionMode );
            }

            pIMEData->mpReleaseIME( hWnd, hIMI );
        }
    }
#endif
}

// -----------------------------------------------------------------------

void SalFrame::UpdateExtTextInputArea()
{
#ifdef ENABLE_IME
#endif
}

// -----------------------------------------------------------------------

void SalFrame::EndExtTextInput( USHORT nFlags )
{
#ifdef ENABLE_IME
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND hWnd = maFrameData.mhWndClient;
        HIMI hIMI = 0;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            PM_ULONG nIndex;
            if ( nFlags & SAL_FRAME_ENDEXTTEXTINPUT_COMPLETE )
                nIndex = CNV_COMPLETE;
            else
                nIndex = CNV_CANCEL;

            pIMEData->mpRequestIME( hIMI, REQ_CONVERSIONSTRING, nIndex, 0 );
            pIMEData->mpReleaseIME( hWnd, hIMI );
        }
    }
#endif
}

// -----------------------------------------------------------------------

XubString SalFrame::GetKeyName( USHORT nCode )
{
    if ( eImplKeyboardLanguage == LANGUAGE_DONTKNOW )
        eImplKeyboardLanguage = GetSystemLanguage();

    XubString        aKeyCode;
    XubString        aCode;
    const char**    pLangTab = ImplGetLangTab( eImplKeyboardLanguage );

    if ( nCode & KEY_SHIFT )
        aKeyCode = pLangTab[LSTR_KEY_SHIFT];

    if ( nCode & KEY_MOD1 )
    {
        if ( !aKeyCode )
            aKeyCode = pLangTab[LSTR_KEY_CTRL];
        else
        {
            aKeyCode += '+';
            aKeyCode += pLangTab[LSTR_KEY_CTRL];
        }
    }

    if ( nCode & KEY_MOD2 )
    {
        if ( !aKeyCode )
            aKeyCode = pLangTab[LSTR_KEY_ALT];
        else
        {
            aKeyCode += '+';
            aKeyCode += pLangTab[LSTR_KEY_ALT];
        }
    }

    USHORT nKeyCode = nCode & 0x0FFF;
    switch ( nKeyCode )
    {
        case KEY_0:
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
            aCode += (char)('0' + (nKeyCode - KEY_0));
            break;

        case KEY_A:
        case KEY_B:
        case KEY_C:
        case KEY_D:
        case KEY_E:
        case KEY_F:
        case KEY_G:
        case KEY_H:
        case KEY_I:
        case KEY_J:
        case KEY_K:
        case KEY_L:
        case KEY_M:
        case KEY_N:
        case KEY_O:
        case KEY_P:
        case KEY_Q:
        case KEY_R:
        case KEY_S:
        case KEY_T:
        case KEY_U:
        case KEY_V:
        case KEY_W:
        case KEY_X:
        case KEY_Y:
        case KEY_Z:
            aCode += (char)('A' + (nKeyCode - KEY_A));
            break;

        case KEY_F1:
        case KEY_F2:
        case KEY_F3:
        case KEY_F4:
        case KEY_F5:
        case KEY_F6:
        case KEY_F7:
        case KEY_F8:
        case KEY_F9:
        case KEY_F10:
        case KEY_F11:
        case KEY_F12:
        case KEY_F13:
        case KEY_F14:
        case KEY_F15:
        case KEY_F16:
        case KEY_F17:
        case KEY_F18:
        case KEY_F19:
        case KEY_F20:
        case KEY_F21:
        case KEY_F22:
        case KEY_F23:
        case KEY_F24:
        case KEY_F25:
        case KEY_F26:
            aCode += 'F';
            aCode += (USHORT)nKeyCode - KEY_F1 + 1;
            break;

        case KEY_DOWN:
            aCode = pLangTab[LSTR_KEY_DOWN];
            break;
        case KEY_UP:
            aCode = pLangTab[LSTR_KEY_UP];
            break;
        case KEY_LEFT:
            aCode = pLangTab[LSTR_KEY_LEFT];
            break;
        case KEY_RIGHT:
            aCode = pLangTab[LSTR_KEY_RIGHT];
            break;
        case KEY_HOME:
            aCode = pLangTab[LSTR_KEY_HOME];
            break;
        case KEY_END:
            aCode = pLangTab[LSTR_KEY_END];
            break;
        case KEY_PAGEUP:
            aCode = pLangTab[LSTR_KEY_PAGEUP];
            break;
        case KEY_PAGEDOWN:
            aCode = pLangTab[LSTR_KEY_PAGEDOWN];
            break;
        case KEY_RETURN:
            aCode = pLangTab[LSTR_KEY_RETURN];
            break;
        case KEY_ESCAPE:
            aCode = pLangTab[LSTR_KEY_ESC];
            break;
        case KEY_TAB:
            aCode = pLangTab[LSTR_KEY_TAB];
            break;
        case KEY_BACKSPACE:
            aCode = pLangTab[LSTR_KEY_BACKSPACE];
            break;
        case KEY_SPACE:
            aCode = pLangTab[LSTR_KEY_SPACE];
            break;
        case KEY_INSERT:
            aCode = pLangTab[LSTR_KEY_INSERT];
            break;
        case KEY_DELETE:
            aCode = pLangTab[LSTR_KEY_DELETE];
            break;

        case KEY_ADD:
            aCode += '+';
            break;
        case KEY_SUBTRACT:
            aCode += '-';
            break;
        case KEY_MULTIPLY:
            aCode += '*';
            break;
        case KEY_DIVIDE:
            aCode += '/';
            break;
        case KEY_POINT:
            aCode += '.';
            break;
        case KEY_COMMA:
            aCode += ',';
            break;
        case KEY_LESS:
            aCode += '<';
            break;
        case KEY_GREATER:
            aCode += '>';
            break;
        case KEY_EQUAL:
            aCode += '=';
            break;
    }

    if ( aCode.Len() )
    {
        if ( !aKeyCode )
            aKeyCode = aCode;
        else
        {
            aKeyCode += '+';
            aKeyCode += aCode;
        }
    }

    return aKeyCode;
}

// -----------------------------------------------------------------------

XubString SalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
    return GetKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

inline long ImplOS2ColorToSal( long nOS2Color )
{
    return MAKE_SALCOLOR( (BYTE)( nOS2Color>>16), (BYTE)(nOS2Color>>8), (BYTE)nOS2Color );
}

// -----------------------------------------------------------------------

static USHORT ImplMouseSysValueToSAL( int iSysValue, USHORT& rCode, USHORT& rClicks, BOOL& rDown )
{
    LONG lValue = WinQuerySysValue( HWND_DESKTOP, iSysValue );

    rCode   = 0;
    rClicks = 1;
    rDown   = TRUE;

    switch ( lValue & 0xFFFF )
    {
        case WM_BUTTON1UP:
        case WM_BUTTON1CLICK:
            rCode = MOUSE_LEFT;
            rDown = FALSE;
            break;
        case WM_BUTTON1DOWN:
        case WM_BUTTON1MOTIONSTART:
            rCode = MOUSE_LEFT;
            break;
        case WM_BUTTON1DBLCLK:
            rCode = MOUSE_LEFT;
            rClicks = 2;
            break;

        case WM_BUTTON2UP:
        case WM_BUTTON2CLICK:
            rCode = MOUSE_RIGHT;
            rDown = FALSE;
            break;
        case WM_BUTTON2DOWN:
        case WM_BUTTON2MOTIONSTART:
            rCode = MOUSE_RIGHT;
            break;
        case WM_BUTTON2DBLCLK:
            rCode = MOUSE_RIGHT;
            rClicks = 2;
            break;

        case WM_BUTTON3UP:
        case WM_BUTTON3CLICK:
            rCode = MOUSE_MIDDLE;
            rDown = FALSE;
            break;
        case WM_BUTTON3DOWN:
        case WM_BUTTON3MOTIONSTART:
            rCode = MOUSE_MIDDLE;
            break;
        case WM_BUTTON3DBLCLK:
            rCode = MOUSE_MIDDLE;
            rClicks = 2;
            break;
    }

    if ( !rCode )
        return FALSE;

    lValue = (lValue & 0xFFFF0000) >> 16;
    if ( lValue != 0xFFFF )
    {
        if ( lValue & KC_SHIFT )
            rCode |= KEY_SHIFT;
        if ( lValue & KC_CTRL )
            rCode |= KEY_MOD1;
        if ( lValue & KC_ALT )
            rCode |= KEY_MOD2;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

static BOOL ImplSalIsSameColor( const Color& rColor1, const Color& rColor2 )
{
    ULONG nWrong = 0;
    nWrong += Abs( (short)rColor1.GetRed()-(short)rColor2.GetRed() );
    nWrong += Abs( (short)rColor1.GetGreen()-(short)rColor2.GetGreen() );
    nWrong += Abs( (short)rColor1.GetBlue()-(short)rColor2.GetBlue() );
    return (nWrong < 30);
}

// -----------------------------------------------------------------------

static BOOL ImplOS2NameFontToVCLFont( const char* pFontName, Font& rFont )
{
    char aNumBuf[10];
    int  nNumBufLen = 0;

    while ( *pFontName && (*pFontName != '.') &&
            (nNumBufLen < sizeof(aNumBuf)-1) )
    {
        aNumBuf[nNumBufLen] = *pFontName;
        nNumBufLen++;
        pFontName++;
    }
    aNumBuf[nNumBufLen] = '\0';

    pFontName++;
    while ( *pFontName == ' ' )
        pFontName++;

    int nFontHeight = atoi( aNumBuf );
    int nFontNameLen = strlen( pFontName );
    if ( nFontHeight && nFontNameLen )
    {
        rFont.SetName( pFontName );
        rFont.SetSize( Size( 0, nFontHeight ) );
        rFont.SetFamily( FAMILY_DONTKNOW );
        rFont.SetWeight( WEIGHT_NORMAL );
        rFont.SetItalic( ITALIC_NONE );
        return TRUE;
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void SalFrame::UpdateSettings( AllSettings& rSettings )
{
    static char aControlPanel[] = "PM_ControlPanel";
    static char aSystemFonts[]  = "PM_SystemFonts";
    char aDummyStr[] = "";

    // --- Mouse setting ---
    USHORT  nCode;
    USHORT  nClicks;
    BOOL    bDown;
    MouseSettings aMouseSettings = rSettings.GetMouseSettings();
    aMouseSettings.SetDoubleClickTime( WinQuerySysValue( HWND_DESKTOP, SV_DBLCLKTIME ) );
    if ( ImplMouseSysValueToSAL( SV_BEGINDRAG, nCode, nClicks, bDown ) )
        aMouseSettings.SetStartDragCode( nCode );
    if ( ImplMouseSysValueToSAL( SV_CONTEXTMENU, nCode, nClicks, bDown ) )
    {
        aMouseSettings.SetContextMenuCode( nCode );
        aMouseSettings.SetContextMenuClicks( nClicks );
        aMouseSettings.SetContextMenuDown( bDown );
    }
    aMouseSettings.SetButtonStartRepeat( WinQuerySysValue( HWND_DESKTOP, SV_FIRSTSCROLLRATE ) );
    aMouseSettings.SetButtonRepeat( WinQuerySysValue( HWND_DESKTOP, SV_SCROLLRATE ) );
    rSettings.SetMouseSettings( aMouseSettings );

    // --- Style settings ---
    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    BOOL bCompBorder = (aStyleSettings.GetOptions() & (STYLE_OPTION_MACSTYLE | STYLE_OPTION_UNIXSTYLE)) == 0;

    // General settings
    LONG    nDisplayTime = PrfQueryProfileInt( HINI_PROFILE, (PSZ)aControlPanel, (PSZ)"LogoDisplayTime", -1 );
    ULONG   nSalDisplayTime;
    if ( nDisplayTime < 0 )
        nSalDisplayTime = LOGO_DISPLAYTIME_STARTTIME;
    else if ( !nDisplayTime )
        nSalDisplayTime = LOGO_DISPLAYTIME_NOLOGO;
    else
        nSalDisplayTime = (ULONG)nDisplayTime;
    aStyleSettings.SetLogoDisplayTime( nSalDisplayTime );

    aStyleSettings.SetCursorBlinkTime( WinQuerySysValue( HWND_DESKTOP, SV_CURSORRATE ) );
    ULONG nDragFullOptions = aStyleSettings.GetDragFullOptions();
    if ( WinQuerySysValue( HWND_DESKTOP, SV_FULLWINDOWDRAG ) )
        nDragFullOptions |= DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE | DRAGFULL_OPTION_DOCKING | DRAGFULL_OPTION_SPLIT;
    else
        nDragFullOptions &= ~(DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE | DRAGFULL_OPTION_DOCKING | DRAGFULL_OPTION_SPLIT);
    aStyleSettings.SetDragFullOptions( nDragFullOptions );

    // Size settings
    aStyleSettings.SetScrollBarSize( WinQuerySysValue( HWND_DESKTOP, SV_CYHSCROLL ) );
    if ( bCompBorder )
    {
        aStyleSettings.SetTitleHeight( WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR ) );
    }

    // Color settings
    if ( bCompBorder )
    {
        aStyleSettings.SetFaceColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0 ) ) );
        aStyleSettings.SetLightColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0 ) ) );
        aStyleSettings.SetLightBorderColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0 ) ) );
        aStyleSettings.SetShadowColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_BUTTONDARK, 0 ) ) );
        aStyleSettings.SetDarkShadowColor( Color( COL_BLACK ) );
        aStyleSettings.SetDialogColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0 ) ) );
        aStyleSettings.SetButtonTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUTEXT, 0 ) ) );
        aStyleSettings.SetActiveColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_ACTIVETITLE, 0 ) ) );
        aStyleSettings.SetActiveTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_ACTIVETITLETEXT, 0 ) ) );
        aStyleSettings.SetActiveBorderColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_ACTIVEBORDER, 0 ) ) );
        aStyleSettings.SetDeactiveColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_INACTIVETITLE, 0 ) ) );
        aStyleSettings.SetDeactiveTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_INACTIVETITLETEXT, 0 ) ) );
        aStyleSettings.SetDeactiveBorderColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_INACTIVEBORDER, 0 ) ) );
        aStyleSettings.SetMenuColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENU, 0 ) ) );
        aStyleSettings.SetMenuTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUTEXT, 0 ) ) );
    }
    aStyleSettings.SetDialogTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetRadioCheckTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetGroupTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_WINDOWSTATICTEXT, 0 ) ) );
    aStyleSettings.SetLabelTextColor( aStyleSettings.GetGroupTextColor() );
    aStyleSettings.SetInfoTextColor( aStyleSettings.GetGroupTextColor() );
    aStyleSettings.SetWindowColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_WINDOW, 0 ) ) );
    aStyleSettings.SetWindowTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_WINDOWTEXT, 0 ) ) );
    aStyleSettings.SetFieldColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_ENTRYFIELD, 0 ) ) );
    aStyleSettings.SetFieldTextColor( aStyleSettings.GetWindowTextColor() );
    aStyleSettings.SetDisableColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUDISABLEDTEXT, 0 ) ) );
    aStyleSettings.SetHighlightColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_HILITEBACKGROUND, 0 ) ) );
    aStyleSettings.SetHighlightTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_HILITEFOREGROUND, 0 ) ) );
    Color aMenuHighColor = ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUHILITEBGND, 0 ) );
    if ( ImplSalIsSameColor( aMenuHighColor, aStyleSettings.GetMenuColor() ) )
    {
        if ( bCompBorder )
        {
            aStyleSettings.SetMenuHighlightColor( Color( COL_BLUE ) );
            aStyleSettings.SetMenuHighlightTextColor( Color( COL_WHITE ) );
        }
    }
    else
    {
        aStyleSettings.SetMenuHighlightColor( aMenuHighColor );
        aStyleSettings.SetMenuHighlightTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUHILITE, 0 ) ) );
    }
    // Checked-Color berechnen
    Color   aColor1 = aStyleSettings.GetFaceColor();
    Color   aColor2 = aStyleSettings.GetLightColor();
    BYTE    nRed    = (BYTE)(((USHORT)aColor1.GetRed()   + (USHORT)aColor2.GetRed())/2);
    BYTE    nGreen  = (BYTE)(((USHORT)aColor1.GetGreen() + (USHORT)aColor2.GetGreen())/2);
    BYTE    nBlue   = (BYTE)(((USHORT)aColor1.GetBlue()  + (USHORT)aColor2.GetBlue())/2);
    aStyleSettings.SetCheckedColor( Color( nRed, nGreen, nBlue ) );

    // Fonts updaten
    Font    aFont;
    char    aFontNameBuf[255];
    aFont = aStyleSettings.GetMenuFont();
    if ( PrfQueryProfileString( HINI_PROFILE, (PSZ)aSystemFonts, (PSZ)"Menus", aDummyStr, aFontNameBuf, sizeof( aFontNameBuf ) ) > 5 )
    {
        if ( ImplOS2NameFontToVCLFont( aFontNameBuf, aFont ) )
            aStyleSettings.SetMenuFont( aFont );
    }
    aFont = aStyleSettings.GetIconFont();
    if ( PrfQueryProfileString( HINI_PROFILE, (PSZ)aSystemFonts, (PSZ)"IconText", aDummyStr, aFontNameBuf, sizeof( aFontNameBuf ) ) > 5 )
    {
        if ( ImplOS2NameFontToVCLFont( aFontNameBuf, aFont ) )
            aStyleSettings.SetIconFont( aFont );
    }
    aFont = aStyleSettings.GetTitleFont();
    if ( PrfQueryProfileString( HINI_PROFILE, (PSZ)aSystemFonts, (PSZ)"WindowTitles", aDummyStr, aFontNameBuf, sizeof( aFontNameBuf ) ) > 5 )
    {
        if ( ImplOS2NameFontToVCLFont( aFontNameBuf, aFont ) )
        {
            aStyleSettings.SetTitleFont( aFont );
            aStyleSettings.SetFloatTitleFont( aFont );
        }
    }
    aFont = aStyleSettings.GetAppFont();
    if ( PrfQueryProfileString( HINI_PROFILE, (PSZ)aSystemFonts, (PSZ)"WindowText", aDummyStr, aFontNameBuf, sizeof( aFontNameBuf ) ) > 5 )
    {
        if ( ImplOS2NameFontToVCLFont( aFontNameBuf, aFont ) )
        {
            Font aHelpFont = aFont;
            aHelpFont.SetName( "Helv;WarpSans" );
            aHelpFont.SetSize( Size( 0, 8 ) );
            aHelpFont.SetWeight( WEIGHT_NORMAL );
            aHelpFont.SetItalic( ITALIC_NONE );
            aStyleSettings.SetHelpFont( aHelpFont );

            // Bei System mappen wir direkt auf WarpSans/Helv, da diese
            // unserer Meinung besser aussehen
            if ( aFont.GetName().Search( "System" ) != STRING_NOTFOUND )
            {
                XubString aFontName = aFont.GetName();
                aFontName.Insert( "WarpSans;Helv;" );
                aFont.SetName( aFontName );
                aFont.SetSize( Size( 0, 9 ) );
            }
            aStyleSettings.SetAppFont( aFont );
            aStyleSettings.SetToolFont( aFont );
            aStyleSettings.SetLabelFont( aFont );
            aStyleSettings.SetInfoFont( aFont );
            aStyleSettings.SetRadioCheckFont( aFont );
            aStyleSettings.SetPushButtonFont( aFont );
            aStyleSettings.SetFieldFont( aFont );
            aStyleSettings.SetGroupFont( aFont );
        }
    }

    rSettings.SetStyleSettings( aStyleSettings );
}

// -----------------------------------------------------------------------

const SystemEnvData* SalFrame::GetSystemData() const
{
    return &maFrameData.maSysData;
}

// -----------------------------------------------------------------------

void SalFrame::Beep( SoundType eSoundType )
{
    static PM_ULONG aImplSoundTab[5] =
    {
        WA_NOTE,                        // SOUND_DEFAULT
        WA_NOTE,                        // SOUND_INFO
        WA_WARNING,                     // SOUND_WARNING
        WA_ERROR,                       // SOUND_ERROR
        WA_NOTE                         // SOUND_QUERY
    };

#if SOUND_COUNT != 5
#error New Sound must be defined!
#endif

    WinAlarm( HWND_DESKTOP, aImplSoundTab[eSoundType] );
}

// -----------------------------------------------------------------------

void SalFrame::SetCallback( void* pInst, SALFRAMEPROC pProc )
{
    if( pProc == NULL )
        maFrameData.mpProc  = ImplSalCallbackDummy;
    else
    {
        maFrameData.mpInst  = pInst;
        maFrameData.mpProc  = pProc;
    }
}

// -----------------------------------------------------------------------

static void SalTestMouseLeave()
{
    SalData* pSalData = GetSalData();

    if ( pSalData->mhWantLeaveMsg && !::WinQueryCapture( HWND_DESKTOP ) )
    {
        POINTL aPt;
        WinQueryPointerPos( HWND_DESKTOP, &aPt );
        if ( pSalData->mhWantLeaveMsg != WinWindowFromPoint( HWND_DESKTOP, &aPt, TRUE ) )
            WinSendMsg( pSalData->mhWantLeaveMsg, SAL_MSG_MOUSELEAVE, 0, MPFROM2SHORT( aPt.x, aPt.y ) );
    }
}

// -----------------------------------------------------------------------

static long ImplHandleMouseMsg( SalFrame* pFrame,
                                UINT nMsg, MPARAM nMP1, MPARAM nMP2 )
{
    SalMouseEvent   aMouseEvt;
    long            nRet;
    USHORT          nEvent;
    BOOL            bRetTRUE = FALSE;
    BOOL            bCall = TRUE;
    BOOL            bActivate = FALSE;
    USHORT          nFlags = SHORT2FROMMP( nMP2 );

    aMouseEvt.mnX       = (short)SHORT1FROMMP( nMP1 );
    aMouseEvt.mnY       = pFrame->maFrameData.mnHeight - (short)SHORT2FROMMP( nMP1 ) - 1;
    aMouseEvt.mnCode    = 0;
    aMouseEvt.mnTime    = WinQueryMsgTime( pFrame->maFrameData.mhAB );

    // MausModus feststellen und setzen
    if ( WinGetKeyState( HWND_DESKTOP, VK_BUTTON1 ) & 0x8000 )
        aMouseEvt.mnCode |= MOUSE_LEFT;
    if ( WinGetKeyState( HWND_DESKTOP, VK_BUTTON2 ) & 0x8000 )
        aMouseEvt.mnCode |= MOUSE_RIGHT;
    if ( WinGetKeyState( HWND_DESKTOP, VK_BUTTON3 ) & 0x8000 )
        aMouseEvt.mnCode |= MOUSE_MIDDLE;
    // Modifier-Tasten setzen
    if ( WinGetKeyState( HWND_DESKTOP, VK_SHIFT ) & 0x8000 )
        aMouseEvt.mnCode |= KEY_SHIFT;
    if ( WinGetKeyState( HWND_DESKTOP, VK_CTRL ) & 0x8000 )
        aMouseEvt.mnCode |= KEY_MOD1;
    if ( WinGetKeyState( HWND_DESKTOP, VK_ALT ) & 0x8000 )
        aMouseEvt.mnCode |= KEY_MOD2;

    switch ( nMsg )
    {
        case WM_MOUSEMOVE:
            {
            SalData* pSalData = GetSalData();

            // Da bei Druecken von Modifier-Tasten die MouseEvents
            // nicht zusammengefast werden (da diese durch KeyEvents
            // unterbrochen werden), machen wir dieses hier selber
            if ( aMouseEvt.mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2) )
            {
                QMSG aTempMsg;
                if ( WinPeekMsg( pSalData->mhAB, &aTempMsg,
                                 pFrame->maFrameData.mhWndClient,
                                 WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE ) )
                {
                    if ( (aTempMsg.msg == WM_MOUSEMOVE) &&
                         (aTempMsg.mp2 == nMP2) )
                        return 1;
                }
            }

            // Test for MouseLeave
            if ( pSalData->mhWantLeaveMsg &&
                (pSalData->mhWantLeaveMsg != pFrame->maFrameData.mhWndClient) )
            {
                POINTL aMousePoint;
                WinQueryMsgPos( pFrame->maFrameData.mhAB, &aMousePoint );
                WinSendMsg( pSalData->mhWantLeaveMsg,
                            SAL_MSG_MOUSELEAVE,
                            0, MPFROM2SHORT( aMousePoint.x, aMousePoint.y ) );
            }
            pSalData->mhWantLeaveMsg = pFrame->maFrameData.mhWndClient;
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

        case SAL_MSG_MOUSELEAVE:
            {
            SalData* pSalData = GetSalData();
            if ( pSalData->mhWantLeaveMsg == pFrame->maFrameData.mhWndClient )
            {
                pSalData->mhWantLeaveMsg = 0;
                if ( pSalData->mpMouseLeaveTimer )
                {
                    delete pSalData->mpMouseLeaveTimer;
                    pSalData->mpMouseLeaveTimer = NULL;
                }

                // Mouse-Coordinaates are relativ to the screen
                POINTL aPt;
                aPt.x = (short)SHORT1FROMMP( nMP2 );
                aPt.y = (short)SHORT2FROMMP( nMP2 );
                WinMapWindowPoints( HWND_DESKTOP, pFrame->maFrameData.mhWndClient, &aPt, 1 );
                aPt.y = pFrame->maFrameData.mnHeight - aPt.y - 1;
                aMouseEvt.mnX = aPt.x;
                aMouseEvt.mnY = aPt.y;
                aMouseEvt.mnButton = 0;
                nEvent = SALEVENT_MOUSELEAVE;
            }
            else
                bCall = FALSE;
            }
            break;

        case WM_BUTTON1DBLCLK:
        case WM_BUTTON1DOWN:
            aMouseEvt.mnButton = MOUSE_LEFT;
            nEvent = SALEVENT_MOUSEBUTTONDOWN;
            bActivate = TRUE;
            break;

        case WM_BUTTON2DBLCLK:
        case WM_BUTTON2DOWN:
            aMouseEvt.mnButton = MOUSE_RIGHT;
            nEvent = SALEVENT_MOUSEBUTTONDOWN;
            bActivate = TRUE;
            break;

        case WM_BUTTON3DBLCLK:
        case WM_BUTTON3DOWN:
            aMouseEvt.mnButton = MOUSE_MIDDLE;
            nEvent = SALEVENT_MOUSEBUTTONDOWN;
            bActivate = TRUE;
            break;

        case WM_BUTTON1UP:
            aMouseEvt.mnButton = MOUSE_LEFT;
            nEvent = SALEVENT_MOUSEBUTTONUP;
            break;

        case WM_BUTTON2UP:
            aMouseEvt.mnButton = MOUSE_RIGHT;
            nEvent = SALEVENT_MOUSEBUTTONUP;
            break;

        case WM_BUTTON3UP:
            aMouseEvt.mnButton = MOUSE_MIDDLE;
            nEvent = SALEVENT_MOUSEBUTTONUP;
            break;
    }

    // Vorsichtshalber machen wir dies hier noch und gehen ueber
    // den neuen SAL-Event. Eigentlich muesste dies immer durch
    // den unabhaengigen Teil ausgeloest werden!
    if ( bActivate )
    {
        SalMouseActivateEvent aMouseActivateEvt;
        aMouseActivateEvt.mnX = aMouseEvt.mnX;
        aMouseActivateEvt.mnY = aMouseEvt.mnY;
        if ( !pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                          SALEVENT_MOUSEACTIVATE, &aMouseActivateEvt ) )
            WinSetWindowPos( pFrame->maFrameData.mhWndFrame, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE | SWP_ZORDER );
        else
            bRetTRUE = TRUE;
    }

    if ( bCall )
    {
        if ( nEvent == SALEVENT_MOUSEBUTTONDOWN )
            WinUpdateWindow( pFrame->maFrameData.mhWndClient );

        nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                           nEvent, &aMouseEvt );
        if ( nMsg == WM_MOUSEMOVE )
        {
            WinSetPointer( HWND_DESKTOP, pFrame->maFrameData.mhPointer );
            nRet = TRUE;
        }
    }
    else
        nRet = 0;

    if ( bRetTRUE )
        nRet = TRUE;

    return nRet;
}

// -----------------------------------------------------------------------

static void ImplConvertKey( MPARAM aMP1, MPARAM aMP2, USHORT& rSVCode, xub_Unicode& rSVCharCode )
{
    USHORT  nKeyFlags   = SHORT1FROMMP( aMP1 );
    UCHAR   nCharCode   = (UCHAR)SHORT1FROMMP( aMP2 );
    USHORT  nKeyCode    = (UCHAR)SHORT2FROMMP( aMP2 );

    // Ist virtueller KeyCode gesetzt und befindet sich der KeyCode in der
    // Tabelle, dann mappen
    if ( (nKeyFlags & KC_VIRTUALKEY) && (nKeyCode < KEY_TAB_SIZE) )
        rSVCode = TranslateKey[nKeyCode];

    // Ist Character-Code gesetzt
    // !!! Bei CTRL/ALT ist KC_CHAR nicht gesetzt, jedoch moechten wir
    // !!! dann auch einen CharCode und machen die Behandlung deshalb
    // !!! selber
    if ( (nKeyFlags & KC_CHAR) || (nKeyFlags & KC_CTRL) || (nKeyFlags & KC_ALT) )
        rSVCharCode = (xub_Unicode)nCharCode;

    // Bei KeyUp muessen wir ein paar andere Ausnahmen machen, da
    // uns in den meisten Faellen kein KeyCode geliefert wird, aber
    // dafuer ein CharCode, wo jedoch nicht KC_CHAR gesetzt ist.
    if ( nKeyFlags & KC_KEYUP )
    {
        if ( !rSVCode )
        {
            // Hier nur CharCode zuweisen, der KeyCode wird im unteren
            // Teil dieser Function dann aus dem CharCode ermittelt
            if ( !rSVCharCode && nCharCode )
                rSVCharCode = (xub_Unicode)nCharCode;
        }
    }

    // Wenn kein KeyCode ermittelt werden konnte, versuchen wir aus dem
    // CharCode einen zu erzeugen
    if ( !rSVCode && rSVCharCode )
    {
        // Bei 0-9, a-z und A-Z auch KeyCode setzen
        if ( (rSVCharCode >= '0') && (rSVCharCode <= '9') && (!rSVCode || !(nKeyFlags & KC_SHIFT)) )
            rSVCode = KEY_0 + (rSVCharCode-'0');
        else if ( (rSVCharCode >= 'a') && (rSVCharCode <= 'z') )
            rSVCode = KEY_A + (rSVCharCode-'a');
        else if ( (rSVCharCode >= 'A') && (rSVCharCode <= 'Z') )
            rSVCode = KEY_A + (rSVCharCode-'A');
        else
        {
            switch ( rSVCharCode )
            {
                case '+':
                    rSVCode = KEY_ADD;
                    break;
                case '-':
                    rSVCode = KEY_SUBTRACT;
                    break;
                case '*':
                    rSVCode = KEY_MULTIPLY;
                    break;
                case '/':
                    rSVCode = KEY_DIVIDE;
                    break;
                case '.':
                    rSVCode = KEY_POINT;
                    break;
                case ',':
                    rSVCode = KEY_COMMA;
                    break;
                case '<':
                    rSVCode = KEY_LESS;
                    break;
                case '>':
                    rSVCode = KEY_GREATER;
                    break;
                case '=':
                    rSVCode = KEY_EQUAL;
                    break;
            }
        }
    }

    // "Numlock-Hack": we want to get correct keycodes from the numpad
    if ( (rSVCharCode >= '0') && (rSVCharCode <= '9') )
        rSVCode = KEY_0 + (rSVCharCode-'0');
    if ( rSVCharCode == ',' )
        rSVCode = KEY_COMMA;
    if ( rSVCharCode == '.' )
        rSVCode = KEY_POINT;

    if ( nKeyFlags & KC_CTRL )
    {
        // Ist CTRL-Taste gedrueckt, dann Char-Code korrigieren
        if ( (rSVCharCode >= 'a') && (rSVCharCode <= 'z') )
            rSVCharCode -= ('a' - 1);

        // Ist CTRL-Taste gedrueckt, dann Char-Code auf 0 setzen, wenn
        // der CharCode nicht < 32 ist
        if ( ((unsigned char)rSVCharCode) >= 32 )
            rSVCharCode = 0;
    }
}

// -----------------------------------------------------------------------

static long ImplHandleKeyMsg( SalFrame* pFrame,
                              UINT nMsg, MPARAM nMP1, MPARAM nMP2 )
{
    static USHORT   nLastOS2KeyChar = 0;
    static xub_Unicode   nLastChar       = 0;
    USHORT          nRepeat         = CHAR3FROMMP( nMP1 ) - 1;
    SHORT           nFlags          = SHORT1FROMMP( nMP1 );
    USHORT          nModCode        = 0;
    USHORT          nSVCode         = 0;
    USHORT          nOS2KeyCode     = (UCHAR)SHORT2FROMMP( nMP2 );
    xub_Unicode          nSVCharCode     = 0;
    long            nRet            = 0;

    // determine modifiers
    if ( nFlags & KC_SHIFT )
        nModCode |= KEY_SHIFT;
    if ( nFlags & KC_CTRL )
        nModCode |= KEY_MOD1;
    if ( nFlags & KC_ALT )
    {
        nModCode |= KEY_MOD2;
        // Nur wenn nicht Control und kein auswertbarer CharCode
        // Wegen AltGr (vorallem wegen 122-Tastaturen auch KC_CHAR testen)
        if ( !(nModCode & KEY_MOD1) && !(nFlags & KC_CHAR) )
            nModCode |= KEY_CONTROLMOD;
    }

    // Bei Shift, Control und Alt schicken wir einen KeyModChange-Event
    if ( (nOS2KeyCode == VK_SHIFT) || (nOS2KeyCode == VK_CTRL) ||
         (nOS2KeyCode == VK_ALT) || (nOS2KeyCode == VK_ALTGRAF) )
    {
        SalKeyModEvent aModEvt;
        aModEvt.mnTime = WinQueryMsgTime( pFrame->maFrameData.mhAB );
        aModEvt.mnCode = nModCode;
        nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                           SALEVENT_KEYMODCHANGE, &aModEvt );
    }
    else
    {
        ImplConvertKey( nMP1, nMP2, nSVCode, nSVCharCode );

        // Fuer Java muessen wir bei KeyUp einen CharCode liefern
        if ( nFlags & KC_KEYUP )
        {
            if ( !nSVCharCode )
            {
                if ( nLastOS2KeyChar == nOS2KeyCode )
                {
                    nSVCharCode     = nLastChar;
                    nLastOS2KeyChar = 0;
                    nLastChar       = 0;
                }
            }
            else
            {
                nLastOS2KeyChar = 0;
                nLastChar       = 0;
            }
        }
        else
        {
            nLastOS2KeyChar = nOS2KeyCode;
            nLastChar       = nSVCharCode;
        }

        if ( nSVCode || nSVCharCode )
        {
            SalKeyEvent aKeyEvt;
            aKeyEvt.mnCode      = nSVCode;
            aKeyEvt.mnTime      = WinQueryMsgTime( pFrame->maFrameData.mhAB );
            aKeyEvt.mnCode     |= nModCode;
            aKeyEvt.mnCharCode  = nSVCharCode;
            aKeyEvt.mnRepeat    = nRepeat;

            nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                               (nFlags & KC_KEYUP) ? SALEVENT_KEYUP : SALEVENT_KEYINPUT,
                                               &aKeyEvt );
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

static void ImplHandlePaintMsg( SalFrame* pFrame )
{
    HPS     hPS;
    RECTL   aRect;

    hPS = WinBeginPaint( pFrame->maFrameData.mhWndClient, NULLHANDLE, &aRect );

    // convert rectangle sys -> sal
    aRect.yTop      = pFrame->maFrameData.mnHeight - aRect.yTop;
    aRect.yBottom   = pFrame->maFrameData.mnHeight - aRect.yBottom;

    // Paint
    SalPaintEvent aPEvt;
    aPEvt.mnBoundX          = aRect.xLeft;
    aPEvt.mnBoundY          = aRect.yTop;
    aPEvt.mnBoundWidth      = aRect.xRight  - aRect.xLeft;
    aPEvt.mnBoundHeight     = aRect.yBottom - aRect.yTop;
    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                SALEVENT_PAINT, &aPEvt );

    WinEndPaint( hPS );
}

// -----------------------------------------------------------------------

static void ImplHandleMoveMsg( SalFrame* pFrame )
{
    pFrame->maFrameData.mbDefPos = FALSE;

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

// -----------------------------------------------------------------------

static long ImplHandleSizeMsg( SalFrame* pFrame, MPARAM nMP2 )
{
    pFrame->maFrameData.mbDefPos = FALSE;
    pFrame->maFrameData.mnWidth  = (short)SHORT1FROMMP( nMP2 );
    pFrame->maFrameData.mnHeight = (short)SHORT2FROMMP( nMP2 );
    if ( pFrame->maFrameData.mpGraphics )
        pFrame->maFrameData.mpGraphics->maGraphicsData.mnHeight = (int)SHORT2FROMMP(nMP2);
    // Status merken
    ImplSaveFrameState( pFrame );
    long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_RESIZE, 0 );
    if ( WinIsWindowVisible( pFrame->maFrameData.mhWndFrame ) && !pFrame->maFrameData.mbInShow )
        WinUpdateWindow( pFrame->maFrameData.mhWndClient );
    return nRet;
}

// -----------------------------------------------------------------------

static void ImplHandleShowMsg( SalFrame* pFrame, MPARAM nMP1 )
{
    if ( !pFrame->maFrameData.mbInShow )
    {
        // Wenn wir von aussen gehidet/geshowed werden (beispielsweise
        // Hide-Button oder Taskleiste), loesen wir einen Resize mit 0,0 aus,
        // damit Dialoge trotzdem als System-Fenster angezeigt werden, oder
        // lehnen das Show ab
        if ( SHORT1FROMMP( nMP1 ) )
        {
            // Show ablehen, wenn wir garnicht sichtbar sind
            if ( !pFrame->maFrameData.mbVisible )
            {
                pFrame->maFrameData.mbInShow = TRUE;
                WinSetWindowPos( pFrame->maFrameData.mhWndFrame, 0, 0, 0, 0, 0, SWP_HIDE );
                pFrame->maFrameData.mbInShow = FALSE;
            }
            else
            {
                // Resize ausloesen, damit alter Status wieder
                // hergestellt wird
                pFrame->maFrameData.mbMinHide = FALSE;
                pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_RESIZE, 0 );
            }
        }
        else
        {
            // Resize ausloesen, damit VCL mitbekommt, das Fenster
            // gehidet ist, bzw. keine Groesse mehr hat
            pFrame->maFrameData.mbMinHide = TRUE;
            pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_RESIZE, 0 );
        }
    }
}

// -----------------------------------------------------------------------

static long ImplHandleFocusMsg( SalFrame* pFrame, MPARAM nMP2 )
{
    if ( SHORT1FROMMP( nMP2 ) )
    {
        if ( WinIsWindowVisible( pFrame->maFrameData.mhWndFrame ) && !pFrame->maFrameData.mbInShow )
            WinUpdateWindow( pFrame->maFrameData.mhWndClient );
        return pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                           SALEVENT_GETFOCUS, 0 );
    }
    else
    {
        return pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                           SALEVENT_LOSEFOCUS, 0 );
    }
}

// -----------------------------------------------------------------------

inline long ImplHandleCloseMsg( SalFrame* pFrame )
{
    return pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                       SALEVENT_CLOSE, 0 );
}

// -----------------------------------------------------------------------

inline void ImplHandleUserEvent( SalFrame* pFrame, MPARAM nMP2 )
{
    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                SALEVENT_USEREVENT, (void*)nMP2 );
}

// -----------------------------------------------------------------------

#ifdef ENABLE_IME

static long ImplHandleIMEStartConversion( SalFrame* pFrame )
{
    long        nRet = FALSE;
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND hWnd = pFrame->maFrameData.mhWndClient;
        HIMI hIMI = 0;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            PM_ULONG nProp;
            if ( 0 != pIMEData->mpQueryIMEProperty( hIMI, QIP_PROPERTY, &nProp ) )
                pFrame->maFrameData.mbHandleIME = FALSE;
            else
            {
                pFrame->maFrameData.mbHandleIME = !(nProp & PRP_SPECIALUI);

            }
            if ( pFrame->maFrameData.mbHandleIME )
            {
/* Windows-Code, der noch nicht angepasst wurde !!!
                // Cursor-Position ermitteln und aus der die Default-Position fuer
                // das Composition-Fenster berechnen
                SalCursorPosEvent aCursorPosEvt;
                pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_CURSORPOS, (void*)&aCursorPosEvt );
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
*/

                pFrame->maFrameData.mbConversionMode = TRUE;
                pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_STARTEXTTEXTINPUT, (void*)NULL );
                nRet = TRUE;
            }

            pIMEData->mpReleaseIME( hWnd, hIMI );
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

static long ImplHandleIMEConversion( SalFrame* pFrame, MPARAM nMP2Param )
{
    long        nRet = FALSE;
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND        hWnd = pFrame->maFrameData.mhWndClient;
        HIMI        hIMI = 0;
        PM_ULONG    nMP2 = (PM_ULONG)nMP2Param;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            if ( nMP2 & (IMR_RESULT_RESULTSTRING |
                         IMR_CONV_CONVERSIONSTRING | IMR_CONV_CONVERSIONATTR |
                         IMR_CONV_CURSORPOS | IMR_CONV_CURSORATTR) )
            {
                SalExtTextInputEvent aEvt;
                aEvt.mnTime             = WinQueryMsgTime( pFrame->maFrameData.mhAB );
                aEvt.mpTextAttr         = NULL;
                aEvt.mnCursorPos        = 0;
                aEvt.mnDeltaStart       = 0;
                aEvt.mbOnlyCursor       = FALSE;
                aEvt.mbCursorVisible    = TRUE;

                PM_ULONG    nBufLen = 0;
                xub_Unicode*     pBuf = NULL;
                PM_ULONG    nAttrBufLen = 0;
                PM_BYTE*    pAttrBuf = NULL;
                BOOL        bLastCursor = FALSE;
                if ( nMP2 & IMR_RESULT_RESULTSTRING )
                {
                    pIMEData->mpGetResultString( hIMI, IMR_RESULT_RESULTSTRING, 0, &nBufLen );
                    if ( nBufLen > 0 )
                    {
                        pBuf = new xub_Unicode[nBufLen];
                        pIMEData->mpGetResultString( hIMI, IMR_RESULT_RESULTSTRING, pBuf, &nBufLen );
                    }

                    bLastCursor = TRUE;
                    aEvt.mbCursorVisible = TRUE;
                }
                else if ( nMP2 & (IMR_CONV_CONVERSIONSTRING | IMR_CONV_CONVERSIONATTR |
                                  IMR_CONV_CURSORPOS | IMR_CONV_CURSORATTR) )
                {
                    pIMEData->mpGetConversionString( hIMI, IMR_CONV_CONVERSIONSTRING, 0, &nBufLen );
                    if ( nBufLen > 0 )
                    {
                        pBuf = new xub_Unicode[nBufLen];
                        pIMEData->mpGetConversionString( hIMI, IMR_CONV_CONVERSIONSTRING, pBuf, &nBufLen );
                    }

                    pIMEData->mpGetConversionString( hIMI, IMR_CONV_CONVERSIONATTR, 0, &nAttrBufLen );
                    if ( nAttrBufLen > 0 )
                    {
                        pAttrBuf = new PM_BYTE[nAttrBufLen];
                        pIMEData->mpGetConversionString( hIMI, IMR_CONV_CONVERSIONATTR, pAttrBuf, &nAttrBufLen );
                    }

/* !!! Wir bekommen derzeit nur falsche Daten, deshalb zeigen wir derzeit
   !!! auch keine Cursor an
                    PM_ULONG nTempBufLen;
                    PM_ULONG nCursorPos = 0;
                    PM_ULONG nCursorAttr = 0;
                    PM_ULONG nChangePos = 0;
                    nTempBufLen = sizeof( PM_ULONG );
                    pIMEData->mpGetConversionString( hIMI, IMR_CONV_CURSORPOS, &nCursorPos, &nTempBufLen );
                    nTempBufLen = sizeof( PM_ULONG );
                    pIMEData->mpGetConversionString( hIMI, IMR_CONV_CURSORATTR, &nCursorAttr, &nTempBufLen );
                    nTempBufLen = sizeof( PM_ULONG );
                    pIMEData->mpGetConversionString( hIMI, IMR_CONV_CHANGESTART, &nChangePos, &nTempBufLen );

                    aEvt.mnCursorPos = nCursorPos;
                    aEvt.mnDeltaStart = nChangePos;
                    if ( nCursorAttr & CP_CURSORATTR_INVISIBLE )
                        aEvt.mbCursorVisible = FALSE;
*/
                    aEvt.mnCursorPos = 0;
                    aEvt.mnDeltaStart = 0;
                    aEvt.mbCursorVisible = FALSE;

                    if ( (nMP2 == IMR_CONV_CURSORPOS) ||
                         (nMP2 == IMR_CONV_CURSORATTR) )
                        aEvt.mbOnlyCursor = TRUE;
                }

                USHORT* pSalAttrAry = NULL;
                if ( pBuf )
                {
                    aEvt.maText = XubString( pBuf, (USHORT)nBufLen );
                    delete pBuf;
                    if ( pAttrBuf )
                    {
                        USHORT nTextLen = aEvt.maText.Len();
                        if ( nTextLen )
                        {
                            pSalAttrAry = new USHORT[nTextLen];
                            memset( pSalAttrAry, 0, nTextLen*sizeof( USHORT ) );
                            for ( USHORT i = 0; (i < nTextLen) && (i < nAttrBufLen); i++ )
                            {
                                PM_BYTE nOS2Attr = pAttrBuf[i];
                                USHORT  nSalAttr;
                                if ( nOS2Attr == CP_ATTR_TARGET_CONVERTED )
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_TARGETCONVERTED | SAL_EXTTEXTINPUT_ATTR_UNDERLINE | SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
                                else if ( nOS2Attr == CP_ATTR_CONVERTED )
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_CONVERTED | SAL_EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE;
                                else if ( nOS2Attr == CP_ATTR_TARGET_NOTCONVERTED )
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_TARGETNOTCONVERTED | SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                                else if ( nOS2Attr == CP_ATTR_INPUT_ERROR )
                                    nSalAttr = SAL_EXTTEXTINPUT_ATTR_INPUTERROR | SAL_EXTTEXTINPUT_ATTR_REDTEXT | SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
                                else /* ( nOS2Attr == CP_ATTR_INPUT ) */
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

                pIMEData->mpReleaseIME( hWnd, hIMI );

                // Handler rufen und wenn wir ein Attribute-Array haben, danach
                // wieder zerstoeren
                pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                            SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
                if ( pSalAttrAry )
                    delete pSalAttrAry;
            }
            else
                pIMEData->mpReleaseIME( hWnd, hIMI );
        }

        nRet = TRUE;
    }

    return nRet;
}

// -----------------------------------------------------------------------

inline long ImplHandleIMEEndConversion( SalFrame* pFrame )
{
    pFrame->maFrameData.mbConversionMode = FALSE;
    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
    return TRUE;
}

// -----------------------------------------------------------------------

static void ImplHandleIMEOpenCandidate( SalFrame* pFrame )
{
    pFrame->maFrameData.mbCandidateMode = TRUE;

    long        nRet = FALSE;
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND        hWnd = pFrame->maFrameData.mhWndClient;
        HIMI        hIMI = 0;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            PM_ULONG nBufLen = 0;
            pIMEData->mpGetConversionString( hIMI, IMR_CONV_CONVERSIONSTRING, 0, &nBufLen );
            if ( nBufLen > 0 )
            {
/* !!! Wir bekommen derzeit nur falsche Daten steht der Cursor immer bei 0
                PM_ULONG nTempBufLen = sizeof( PM_ULONG );
                PM_ULONG nCursorPos = 0;
                pIMEData->mpGetConversionString( hIMI, IMR_CONV_CURSORPOS, &nCursorPos, &nTempBufLen );
*/
                PM_ULONG nCursorPos = 0;

                SalExtTextInputPosEvent aEvt;
                aEvt.mnTime         = WinQueryMsgTime( pFrame->maFrameData.mhAB );
                aEvt.mnFirstPos     = nCursorPos;
                aEvt.mnChars        = nBufLen-nCursorPos;
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

                CANDIDATEPOS aForm;
                aForm.ulIndex           = 0;
                aForm.ulStyle           = CPS_EXCLUDE;
                aForm.ptCurrentPos.x    = aEvt.mpPosAry[0].mnX;
                aForm.ptCurrentPos.y    = pFrame->maFrameData.mnHeight - (nMaxBottom+1) - 1;
                aForm.rcArea.xLeft      = nMinLeft;
                aForm.rcArea.yBottom    = pFrame->maFrameData.mnHeight - nMaxBottom - 1;
                aForm.rcArea.xRight     = nMaxRight+1;
                aForm.rcArea.yTop       = pFrame->maFrameData.mnHeight - nMinTop - 1;
                pIMEData->mpSetCandidateWin( hIMI, &aForm );

                delete aEvt.mpPosAry;
            }

            pIMEData->mpReleaseIME( hWnd, hIMI );
        }
    }
}

// -----------------------------------------------------------------------

inline void ImplHandleIMECloseCandidate( SalFrame* pFrame )
{
    pFrame->maFrameData.mbCandidateMode = FALSE;
}

#endif

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalFrameWndProc( HWND hWnd, PM_ULONG nMsg,
                                  MPARAM nMP1, MPARAM nMP2 )
{
    SalFrame*   pFrame      = GetWindowPtr( hWnd );
    MRESULT     nRet        = (MRESULT)0;
    BOOL        bDef        = TRUE;

    switch( nMsg )
    {
        case WM_MOUSEMOVE:
        case WM_BUTTON1DOWN:
        case WM_BUTTON2DOWN:
        case WM_BUTTON3DOWN:
        case WM_BUTTON1DBLCLK:
        case WM_BUTTON2DBLCLK:
        case WM_BUTTON3DBLCLK:
        case WM_BUTTON1UP:
        case WM_BUTTON2UP:
        case WM_BUTTON3UP:
        case SAL_MSG_MOUSELEAVE:
            // ButtonUp/Down nie an die WinDefWindowProc weiterleiten, weil sonst
            // die Message an den Owner weitergeleitet wird
            bDef = !ImplHandleMouseMsg( pFrame, nMsg, nMP1, nMP2 );
            break;

        case WM_CHAR:
            if ( pFrame->maFrameData.mbConversionMode )
                bDef = FALSE;
            else
                bDef = !ImplHandleKeyMsg( pFrame, nMsg, nMP1, nMP2 );
            break;

        case WM_ERASEBACKGROUND:
            nRet = (MRESULT)FALSE;
            bDef = FALSE;
            break;

        case WM_PAINT:
            ImplSalYieldMutexAcquire();
            ImplHandlePaintMsg( pFrame );
            ImplSalYieldMutexRelease();
            bDef = FALSE;
            break;

        case WM_TIMER:
            {
            SalData* pSalData = GetSalData();
            // Test for MouseLeave
            SalTestMouseLeave();
            if ( pSalData->mnTimerId == SHORT1FROMMP( nMP1 ) )
                pSalData->mpTimerProc();
            }
            break;

        case WM_MOVE:
            ImplHandleMoveMsg( pFrame );
            bDef = FALSE;
            break;

        case WM_SIZE:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                ImplHandleSizeMsg( pFrame, nMP2 );
                ImplSalYieldMutexRelease();
            }
            else
                WinPostMsg( hWnd, SAL_MSG_POSTSIZE, nMP1, nMP2 );
            break;
        case SAL_MSG_POSTSIZE:
            ImplHandleSizeMsg( pFrame, nMP2 );
            break;

        case WM_CALCVALIDRECTS:
            return (MRESULT)(CVR_ALIGNLEFT | CVR_ALIGNTOP);

        case WM_SETFOCUS:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                ImplHandleFocusMsg( pFrame, nMP2 );
                ImplSalYieldMutexRelease();
            }
            else
                WinPostMsg( hWnd, SAL_MSG_POSTFOCUS, 0, nMP2 );
            break;
        case SAL_MSG_POSTFOCUS:
            ImplHandleFocusMsg( pFrame, nMP2 );
            break;

        case WM_TRANSLATEACCEL:
            {
            // Da uns OS/2 zu viele Tasten abfaegnt, unternehmen wir etwas,
            // damit wir Shift+F1, Shift+F10 und Shift+Enter bekommen
            PQMSG   pMsg        = (PQMSG)nMP1;
            USHORT  nKeyFlags   = SHORT1FROMMP( pMsg->mp1 );
            USHORT  nKeyCode    = (UCHAR)SHORT2FROMMP( pMsg->mp2 );

            if ( !(nKeyFlags & KC_KEYUP) && (nKeyFlags & KC_VIRTUALKEY) &&
                 (nKeyFlags & KC_SHIFT) && (nKeyCode != VK_ESC) )
                return (MRESULT)FALSE;

            if ( nKeyCode == VK_F1 )
                return (MRESULT)FALSE;
            }
            break;

        case WM_CREATE:
            {
            SalData* pSalData = GetSalData();
            // Window-Instanz am Windowhandle speichern
            pFrame = pSalData->mpCreateFrame;
            pSalData->mpCreateFrame = NULL;
            SetWindowPtr( hWnd, pFrame );
            }
            break;

        case WM_CLOSE:
            ImplSalYieldMutexAcquire();
            ImplHandleCloseMsg( pFrame );
            ImplSalYieldMutexRelease();
            bDef = FALSE;
            break;

        case WM_SYSVALUECHANGED:
            if ( pFrame->maFrameData.mbFullScreen )
                ImplSalFrameFullScreenPos( pFrame );
            // kein break, da der Rest auch noch verarbeitet werden soll
        case PL_ALTERED:
        case WM_SYSCOLORCHANGE:
            ImplSalYieldMutexAcquire();
            pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                        SALEVENT_SETTINGSCHANGED, 0 );
            ImplSalYieldMutexRelease();
            break;

        case SAL_MSG_USEREVENT:
            ImplHandleUserEvent( pFrame, nMP2 );
            bDef = FALSE;
            break;

        case WM_COMMAND:
        case SAL_MSG_SYSPROCESSMENU:
            if ( SalImplHandleProcessMenu( hWnd, nMsg, nMP1, nMP2 ) )
            {
                bDef = FALSE;
                nRet = (MRESULT)1;
            }
            break;

#ifdef ENABLE_IME
        case WM_IMEREQUEST:
            if ( (PM_ULONG)nMP1 == IMR_CONVRESULT )
            {
                if ( pFrame->maFrameData.mbHandleIME )
                {
                    // Nur im Conversionmodus akzeptieren wir den IME-Input
                    if ( pFrame->maFrameData.mbConversionMode )
                    {
                        ImplSalYieldMutexAcquire();
                        if ( ImplHandleIMEConversion( pFrame, nMP2 ) )
                        {
                            bDef = FALSE;
                            nRet = (MRESULT)TRUE;
                        }
                        ImplSalYieldMutexRelease();
                    }
                }
            }
            else if ( (PM_ULONG)nMP1 == IMR_CANDIDATE )
            {
                if ( pFrame->maFrameData.mbHandleIME )
                {
                    ImplSalYieldMutexAcquire();
                    if ( (PM_ULONG)nMP2 & IMR_CANDIDATE_SHOW )
                        ImplHandleIMEOpenCandidate( pFrame );
                    else if ( (PM_ULONG)nMP2 & IMR_CANDIDATE_HIDE )
                        ImplHandleIMECloseCandidate( pFrame );
                    ImplSalYieldMutexRelease();
                }
            }
            break;

        case WM_IMENOTIFY:
            if ( (PM_ULONG)nMP1 == IMN_STARTCONVERSION )
            {
                ImplSalYieldMutexAcquire();
                if ( ImplHandleIMEStartConversion( pFrame ) )
                {
                    bDef = FALSE;
                    nRet = (MRESULT)TRUE;
                }
                ImplSalYieldMutexRelease();
            }
            else if ( (PM_ULONG)nMP1 == IMN_ENDCONVERSION )
            {
                if ( pFrame->maFrameData.mbHandleIME )
                {
                    ImplSalYieldMutexAcquire();
                    if ( ImplHandleIMEEndConversion( pFrame ) )
                    {
                        bDef = FALSE;
                        nRet = (MRESULT)TRUE;
                    }
                    ImplSalYieldMutexRelease();
                }
            }
            break;
#endif
    }

    if ( bDef )
        nRet = WinDefWindowProc( hWnd, nMsg, nMP1, nMP2 );

    return nRet;
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalFrameFrameProc( HWND hWnd, PM_ULONG nMsg,
                                    MPARAM nMP1, MPARAM nMP2 )
{
    if ( nMsg == WM_SYSCOMMAND )
    {
        HWND hWndClient = WinWindowFromID( hWnd, FID_CLIENT );
        if( hWndClient )
        {
            SalFrame* pFrame = GetWindowPtr( hWndClient );
            if ( pFrame )
            {
                USHORT nCmd = SHORT1FROMMP( nMP1 );
                if ( pFrame->maFrameData.mbFullScreen )
                {
                    if ( (nCmd == SC_SIZE) || (nCmd == SC_MOVE) ||
                         (nCmd == SC_RESTORE) ||
                         (nCmd == SC_MINIMIZE) || (nCmd == SC_MAXIMIZE) )
                    {
                        WinAlarm( HWND_DESKTOP, WA_NOTE );
                        return 0;
                    }
                }

                if ( nCmd == SC_APPMENU )
                {
                    // KeyInput mit MENU-Key rufen
                    SalKeyEvent aKeyEvt;
                    aKeyEvt.mnTime      = WinQueryMsgTime( pFrame->maFrameData.mhAB );
                    aKeyEvt.mnCode      = KEY_MENU;
                    aKeyEvt.mnCharCode  = 0;
                    aKeyEvt.mnRepeat    = 0;
                    long nRet = pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                            SALEVENT_KEYINPUT, &aKeyEvt );
                    pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame,
                                                SALEVENT_KEYUP, &aKeyEvt );
                    if ( nRet )
                        return (MRESULT)0;
                }
            }
        }
    }
    else if ( nMsg == WM_SHOW )
    {
        HWND hWndClient = WinWindowFromID( hWnd, FID_CLIENT );
        if( hWndClient )
        {
            SalFrame* pFrame = GetWindowPtr( hWndClient );
            if ( pFrame )
                ImplHandleShowMsg( pFrame, nMP1 );
        }
    }

    return aSalShlData.mpOldFrameProc( hWnd, nMsg, nMP1, nMP2 );
}
