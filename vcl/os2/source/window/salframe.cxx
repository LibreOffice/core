/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#define INCL_DOS
#define INCL_PM
#define INCL_WIN
#define VCL_OS2
#include <svpm.h>

#include <string.h>

#include <tools/svwin.h>

// =======================================================================

#define _SV_SALFRAME_CXX

#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define private public

#include "os2/sallang.hxx"
#include "os2/salids.hrc"
#include "os2/saldata.hxx"
#include "os2/salinst.h"
#include "os2/salgdi.h"
#include "os2/salframe.h"
#include "os2/saltimer.h"

#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/keycodes.hxx>

#if OSL_DEBUG_LEVEL>10
extern "C" int debug_printf(const char *f, ...);

static sal_Bool _bCapture;

#else
#define debug_printf( ...) { 1; }
#endif

// =======================================================================

#undef WinEnableMenuItem
#define WinEnableMenuItem(hwndMenu,id,fEnable) \
    ((PM_BOOL)WinSendMsg (hwndMenu, MM_SETITEMATTR, MPFROM2SHORT (id, TRUE), \
               MPFROM2SHORT (MIA_DISABLED, \
                     ((USHORT)(fEnable) ? 0 : MIA_DISABLED))))

// =======================================================================

HPOINTER ImplLoadPointer( ULONG nId );

static void SetMaximizedFrameGeometry( HWND hWnd, Os2SalFrame* pFrame );
static void UpdateFrameGeometry( HWND hWnd, Os2SalFrame* pFrame );
static void ImplSalCalcFrameSize( HWND hWnd,
                                  LONG& nFrameX, LONG& nFrameY, LONG& nCaptionY );
static void ImplSalCalcFrameSize( const Os2SalFrame* pFrame,
                                  LONG& nFrameX, LONG& nFrameY, LONG& nCaptionY );
MRESULT EXPENTRY SalFrameSubClassWndProc( HWND hWnd, ULONG nMsg,
                                  MPARAM nMP1, MPARAM nMP2 );

// =======================================================================

static LanguageType eImplKeyboardLanguage = LANGUAGE_DONTKNOW;
sal_Bool Os2SalFrame::mbInReparent = FALSE;
ULONG Os2SalFrame::mnInputLang = 0;

// =======================================================================

// define a new flag
#define SWP_CENTER          (SWP_NOAUTOCLOSE<<4)
#define SWP_SHOWMAXIMIZED   (SWP_ACTIVATE | SWP_SHOW | SWP_MAXIMIZE)
#define SWP_SHOWMINIMIZED   (SWP_ACTIVATE | SWP_SHOW | SWP_MINIMIZE)
#define SWP_SHOWNORMAL      (SWP_ACTIVATE | SWP_SHOW | SWP_RESTORE)

static LONG nScreenHeight  = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN);
static LONG nScreenWidth   = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );

sal_Bool APIENTRY _WinQueryWindowRect( HWND hwnd, PRECTL prclDest)
{
    sal_Bool rc = WinQueryWindowRect( hwnd, prclDest);
    ULONG tmp = prclDest->yBottom;
    prclDest->yBottom = prclDest->yTop;
    prclDest->yTop = tmp;
    return rc;
}

sal_Bool APIENTRY _WinQueryPointerPos (HWND hwndDesktop, PPOINTL pptl)
{
    sal_Bool rc = WinQueryPointerPos( hwndDesktop, pptl);
    pptl->y = nScreenHeight - pptl->y;
    return rc;
}

sal_Bool APIENTRY _WinQueryWindowPos( Os2SalFrame* pFrame, PSWP pswp)
{
    SWP swpOwner;
    sal_Bool rc = WinQueryWindowPos( pFrame->mhWndFrame, pswp);

#if OSL_DEBUG_LEVEL>1
    debug_printf( "> WinQueryWindowPos hwnd %x at %d,%d (%dx%d)\n",
                    pFrame->mhWndFrame, pswp->x, pswp->y, pswp->cx, pswp->cy);
#endif

    Os2SalFrame* pParentFrame = pFrame->mpParentFrame;

    //YD adjust to owner coordinates
    if ( pParentFrame )
    {
        POINTL ptlOwner = {0};

        // coords are relative to screen, map to parent frame client area
        ptlOwner.x = pswp->x;
        ptlOwner.y = pswp->y;
        WinMapWindowPoints( HWND_DESKTOP, pParentFrame->mhWndClient, &ptlOwner, 1);
        pswp->x = ptlOwner.x;
        pswp->y = ptlOwner.y;
        // get parent client area size
        WinQueryWindowPos( pParentFrame->mhWndClient, &swpOwner);
    } else
    {
        // no owner info, use DESKTOP????
        swpOwner.cx = nScreenWidth;
        swpOwner.cy = nScreenHeight;
    }

    // invert Y coordinate
    pswp->y = swpOwner.cy - (pswp->y + pswp->cy);

#if OSL_DEBUG_LEVEL>1
    debug_printf( "< WinQueryWindowPos hwnd %x at %d,%d (%dx%d)\n",
                    pFrame->mhWndFrame, pswp->x, pswp->y, pswp->cx, pswp->cy);
#endif
    return rc;
}

sal_Bool APIENTRY _WinSetWindowPos( Os2SalFrame* pFrame, HWND hwndInsertBehind, LONG x, LONG y,
    LONG cx, LONG cy, ULONG fl)
{
    SWP     swpOwner = {0};
    POINTL  ptlOwner = {0};
    HWND    hParent = NULL;

#if OSL_DEBUG_LEVEL>1
    debug_printf( ">WinSetWindowPos hwnd %x at %d,%d (%dx%d) fl 0x%08x\n",
                    pFrame->mhWndFrame, x, y, cx, cy, fl);
#endif

    // first resize window if requested
    if ( (fl & SWP_SIZE) ) {
        ULONG   flag = SWP_SIZE;
        LONG    nX = 0, nY = 0;
        LONG    frameFrameX, frameFrameY, frameCaptionY;

        ImplSalCalcFrameSize( pFrame, frameFrameX, frameFrameY, frameCaptionY );
        // if we change y size, we need to move the window down
        // because os2 window origin is lower left corner
        if (pFrame->maGeometry.nHeight != cy) {
            SWP     aSWP;
            WinQueryWindowPos( pFrame->mhWndFrame, &aSWP);
            nX = aSWP.x;
            nY = aSWP.y - (cy + 2*frameFrameY + frameCaptionY - aSWP.cy);
            flag |= SWP_MOVE;
        }
        WinSetWindowPos( pFrame->mhWndFrame, NULL, nX, nY,
            cx+2*frameFrameX, cy+2*frameFrameY+frameCaptionY, flag);
        fl = fl & ~SWP_SIZE;
    }
    else // otherwise get current size
    {
        SWP swp = {0};
        WinQueryWindowPos( pFrame->mhWndClient, &swp);
        cx = swp.cx;
        cy = swp.cy;
    }

    // get parent window handle
    Os2SalFrame* pParentFrame = pFrame->mpParentFrame;

    // use desktop if parent is not defined
    hParent = pParentFrame ? pParentFrame->mhWndClient : HWND_DESKTOP;
    // if parent is not visible, use desktop as reference
    hParent = WinIsWindowVisible( hParent) ? hParent : HWND_DESKTOP;

    WinQueryWindowPos( hParent, &swpOwner);

    //YD adjust to owner coordinates only when moving and not centering
    //if (!(fl & SWP_CENTER) && (fl & SWP_MOVE))
    if ((fl & SWP_MOVE))
    {

        // if SWP_CENTER is specified, change position to parent center
        if (fl & SWP_CENTER) {
            ptlOwner.x = (swpOwner.cx - cx) / 2;
            ptlOwner.y = (swpOwner.cy - cy) / 2;
#if OSL_DEBUG_LEVEL>0
            debug_printf( "_WinSetWindowPos SWP_CENTER\n");
#endif
            fl = fl & ~SWP_CENTER;
        } else {
            // coords are relative to parent frame client area, map to screen
            // map Y to OS/2 system coordinates
            ptlOwner.x = x;
            ptlOwner.y = swpOwner.cy - (y + cy);

#if OSL_DEBUG_LEVEL>0
            debug_printf( "_WinSetWindowPos owner 0x%x at %d,%d (%dx%d) OS2\n",
                hParent, ptlOwner.x, ptlOwner.y, swpOwner.cx, swpOwner.cy);
#endif
        }
        // map from client area to screen
        WinMapWindowPoints( hParent, HWND_DESKTOP, &ptlOwner, 1);
        x = ptlOwner.x;
        y = ptlOwner.y;

#if OSL_DEBUG_LEVEL>0
        debug_printf( "_WinSetWindowPos owner 0x%x at %d,%d (%dx%d) MAPPED OS2\n",
            hParent, ptlOwner.x, ptlOwner.y, swpOwner.cx, swpOwner.cy);
#endif
    }

#if OSL_DEBUG_LEVEL>0
    debug_printf( "<WinSetWindowPos hwnd %x at %d,%d (%dx%d) fl=%x\n",
                    pFrame->mhWndFrame, x, y, cx, cy, fl);
#endif
    return WinSetWindowPos( pFrame->mhWndFrame, hwndInsertBehind, x, y, 0, 0, fl);
}

// =======================================================================

#if OSL_DEBUG_LEVEL > 0
static void dumpWindowInfo( char* fnc, HWND hwnd)
{
    SWP aSWP;
    HWND    hwnd2;
    char    szTitle[256];

#if 0
    _WinQueryWindowPos( hwnd, &aSWP );
    strcpy(szTitle,"");
    WinQueryWindowText(hwnd, sizeof(szTitle), szTitle);
    debug_printf( "%s: window %08x at %d,%d (size %dx%d) '%s'\n", fnc, hwnd,
                aSWP.x, aSWP.y, aSWP.cx, aSWP.cy, szTitle);
    hwnd2 = WinQueryWindow(hwnd, QW_PARENT);
    _WinQueryWindowPos( hwnd2, &aSWP );
    strcpy(szTitle,"");
    WinQueryWindowText(hwnd2, sizeof(szTitle), szTitle);
    debug_printf( "%s: parent %08x at %d,%d (size %dx%d) '%s'\n", fnc, hwnd2,
                aSWP.x, aSWP.y, aSWP.cx, aSWP.cy, szTitle);
    hwnd2 = WinQueryWindow(hwnd, QW_OWNER);
    _WinQueryWindowPos( hwnd2, &aSWP );
    strcpy(szTitle,"");
    WinQueryWindowText(hwnd2, sizeof(szTitle), szTitle);
    debug_printf( "%s: owner %08x at %d,%d (size %dx%d) '%s'\n", fnc, hwnd2,
                aSWP.x, aSWP.y, aSWP.cx, aSWP.cy, szTitle);
#endif
}
#endif

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
            sal_Bool            bError = FALSE;
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

static void ImplSaveFrameState( Os2SalFrame* pFrame )
{
    // Position, Groesse und Status fuer GetWindowState() merken
    if ( !pFrame->mbFullScreen )
    {
        SWP aSWP;
        sal_Bool bVisible = WinIsWindowVisible( pFrame->mhWndFrame);

        // Query actual state (maState uses screen coords)
        WinQueryWindowPos( pFrame->mhWndFrame, &aSWP );

        if ( aSWP.fl & SWP_MINIMIZE )
        {
#if OSL_DEBUG_LEVEL>0
            debug_printf("Os2SalFrame::GetWindowState %08x SAL_FRAMESTATE_MINIMIZED\n",
                    pFrame->mhWndFrame);
#endif
            pFrame->maState.mnState |= SAL_FRAMESTATE_MINIMIZED;
            if ( bVisible )
                pFrame->mnShowState = SWP_SHOWMAXIMIZED;
        }
        else if ( aSWP.fl & SWP_MAXIMIZE )
        {
#if OSL_DEBUG_LEVEL>0
            debug_printf("Os2SalFrame::GetWindowState %08x SAL_FRAMESTATE_MAXIMIZED\n",
                    pFrame->mhWndFrame);
#endif
            pFrame->maState.mnState &= ~SAL_FRAMESTATE_MINIMIZED;
            pFrame->maState.mnState |= SAL_FRAMESTATE_MAXIMIZED;
            if ( bVisible )
                pFrame->mnShowState = SWP_SHOWMINIMIZED;
            pFrame->mbRestoreMaximize = TRUE;
        }
        else
        {
            LONG nFrameX, nFrameY, nCaptionY;
            ImplSalCalcFrameSize( pFrame, nFrameX, nFrameY, nCaptionY );
            // to be consistent with Unix, the frame state is without(!) decoration
            long nTopDeco = nFrameY + nCaptionY;
            long nLeftDeco = nFrameX;
            long nBottomDeco = nFrameY;
            long nRightDeco = nFrameX;

            pFrame->maState.mnState &= ~(SAL_FRAMESTATE_MINIMIZED | SAL_FRAMESTATE_MAXIMIZED);
            // subtract decoration, store screen coords
            pFrame->maState.mnX      = aSWP.x+nLeftDeco;
            pFrame->maState.mnY      = nScreenHeight - (aSWP.y+aSWP.cy)+nTopDeco;
            pFrame->maState.mnWidth  = aSWP.cx-nLeftDeco-nRightDeco;
            pFrame->maState.mnHeight = aSWP.cy-nTopDeco-nBottomDeco;
#if OSL_DEBUG_LEVEL>0
            debug_printf("Os2SalFrame::GetWindowState %08x (%dx%d) at %d,%d VCL\n",
                    pFrame->mhWndFrame,
                    pFrame->maState.mnWidth,pFrame->maState.mnHeight,pFrame->maState.mnX,pFrame->maState.mnY);
#endif
            if ( bVisible )
                pFrame->mnShowState = SWP_SHOWNORMAL;
            pFrame->mbRestoreMaximize = FALSE;
            //debug_printf( "ImplSaveFrameState: window %08x at %d,%d (size %dx%d)\n",
            //  pFrame->mhWndFrame,
            //  pFrame->maState.mnX, pFrame->maState.mnY, pFrame->maState.mnWidth, pFrame->maState.mnHeight);
        }
    }
}

// -----------------------------------------------------------------------

long ImplSalCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
    return 0;
}

// -----------------------------------------------------------------------

static void ImplSalCalcFrameSize( HWND hWnd,
                                  LONG& nFrameX, LONG& nFrameY, LONG& nCaptionY )
{
    Os2SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return;
    return ImplSalCalcFrameSize( pFrame, nFrameX, nFrameY, nCaptionY );
}

static void ImplSalCalcFrameSize( const Os2SalFrame* pFrame,
                                  LONG& nFrameX, LONG& nFrameY, LONG& nCaptionY )
{
    if ( pFrame->mbSizeBorder )
    {
        nFrameX = WinQuerySysValue( HWND_DESKTOP, SV_CXSIZEBORDER );
        nFrameY = WinQuerySysValue( HWND_DESKTOP, SV_CYSIZEBORDER );
    }
    else if ( pFrame->mbFixBorder )
    {
        nFrameX = WinQuerySysValue( HWND_DESKTOP, SV_CXDLGFRAME );
        nFrameY = WinQuerySysValue( HWND_DESKTOP, SV_CYDLGFRAME );
    }
    else if ( pFrame->mbBorder )
    {
        nFrameX = WinQuerySysValue( HWND_DESKTOP, SV_CXBORDER );
        nFrameY = WinQuerySysValue( HWND_DESKTOP, SV_CYBORDER );
    }
    else
    {
        nFrameX = 0;
        nFrameY = 0;
    }
    if ( pFrame->mbCaption )
        nCaptionY = WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );
    else
        nCaptionY = 0;

#if OSL_DEBUG_LEVEL>0
    //if (_bCapture)
        debug_printf("ImplSalCalcFrameSize 0x%08x x=%d y=%d t=%d\n", pFrame->mhWndFrame, nFrameX, nFrameY, nCaptionY);
#endif
}

// -----------------------------------------------------------------------

static void ImplSalCalcFullScreenSize( const Os2SalFrame* pFrame,
                                       LONG& rX, LONG& rY, LONG& rDX, LONG& rDY )
{
    // set window to screen size
    LONG nFrameX, nFrameY, nCaptionY;
    LONG rScreenDX = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    LONG rScreenDY = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    // Framegroessen berechnen
    ImplSalCalcFrameSize( pFrame, nFrameX, nFrameY, nCaptionY );

    rX  = -nFrameX;
    rY  = -(nFrameY+nCaptionY);
    rDX = rScreenDX+(nFrameX*2);
    rDY = rScreenDY+(nFrameY*2)+nCaptionY;
}

// -----------------------------------------------------------------------

static void ImplSalFrameFullScreenPos( Os2SalFrame* pFrame, sal_Bool bAlways = FALSE )
{
    SWP aSWP;
    _WinQueryWindowPos( pFrame, &aSWP );
    if ( bAlways || !(aSWP.fl & SWP_MINIMIZE) )
    {
        // set window to screen size
        LONG nX;
        LONG nY;
        LONG nWidth;
        LONG nHeight;
        ImplSalCalcFullScreenSize( pFrame, nX, nY, nWidth, nHeight );
        _WinSetWindowPos( pFrame, 0,
                         nX, nY, nWidth, nHeight,
                         SWP_MOVE | SWP_SIZE );
    }
}

// -----------------------------------------------------------------------

// Uebersetzungstabelle von System-Keycodes in StarView-Keycodes
#define KEY_TAB_SIZE     (VK_ENDDRAG+1)

static USHORT aImplTranslateKeyTab[KEY_TAB_SIZE] =
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

SalFrame* ImplSalCreateFrame( Os2SalInstance* pInst, HWND hWndParent, ULONG nSalFrameStyle )
{
    SalData*        pSalData = GetSalData();
    Os2SalFrame*    pFrame = new Os2SalFrame;
    HWND            hWndFrame;
    HWND            hWndClient;
    ULONG           nFrameFlags = FCF_NOBYTEALIGN | FCF_SCREENALIGN;
    ULONG           nFrameStyle = 0;
    ULONG           nClientStyle = WS_CLIPSIBLINGS;
    sal_Bool            bSubFrame = FALSE;

#if OSL_DEBUG_LEVEL>0
    debug_printf(">ImplSalCreateFrame hWndParent 0x%x, nSalFrameStyle 0x%x\n", hWndParent, nSalFrameStyle);
#endif

    if ( hWndParent )
    {
        bSubFrame = TRUE;
        pFrame->mbNoIcon = TRUE;
    }

    // determine creation data (bei Moveable nehmen wir DLG-Border, damit
    // es besser aussieht)
    if ( nSalFrameStyle & SAL_FRAME_STYLE_CLOSEABLE )
        nFrameFlags |= FCF_CLOSEBUTTON;

    if ( nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE ) {
        pFrame->mbCaption = TRUE;
        nFrameStyle = WS_ANIMATE;
        nFrameFlags |= FCF_SYSMENU | FCF_TITLEBAR | FCF_DLGBORDER;
        if ( !hWndParent )
            nFrameFlags |= FCF_MINBUTTON;

        if ( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE )
        {
            pFrame->mbSizeBorder = TRUE;
            nFrameFlags |= FCF_SIZEBORDER;
            if ( !hWndParent )
                nFrameFlags |= FCF_MAXBUTTON;
        }
        else
            pFrame->mbFixBorder = TRUE;

        // add task list style if not a tool window
        if ( !(nSalFrameStyle & SAL_FRAME_STYLE_TOOLWINDOW) ) {
            nFrameFlags |= FCF_TASKLIST;
        }
    }

    if( nSalFrameStyle & SAL_FRAME_STYLE_TOOLWINDOW )
    {
        pFrame->mbNoIcon = TRUE;
        // YD gives small caption -> nExSysStyle |= WS_EX_TOOLWINDOW;
    }

    if ( nSalFrameStyle & SAL_FRAME_STYLE_FLOAT )
    {
        //nExSysStyle |= WS_EX_TOOLWINDOW;
        pFrame->mbFloatWin = TRUE;
    }
    //if( nSalFrameStyle & SAL_FRAME_STYLE_TOOLTIP )
    //    nExSysStyle |= WS_EX_TOPMOST;

    // init frame data
    pFrame->mnStyle = nSalFrameStyle;

    // determine show style
    pFrame->mnShowState = SWP_SHOWNORMAL;

    // create frame
    //YD FIXME this is a potential bug with multiple threads and cuncurrent
    //window creation, because this field is accessed in
    //WM_CREATE to get window data,
    pSalData->mpCreateFrame = pFrame;

    //YD FIXME if SAL_FRAME_CHILD is specified, use hWndParent as parent handle...
    hWndFrame = WinCreateStdWindow( HWND_DESKTOP, nFrameStyle, &nFrameFlags,
                    (PSZ)(bSubFrame ? SAL_SUBFRAME_CLASSNAME : SAL_FRAME_CLASSNAME),
                    NULL,
                    nClientStyle, 0, 0, &hWndClient );
    debug_printf("ImplSalCreateFrame hWndParent 0x%x, hWndFrame 0x%x, hWndClient 0x%x\n", hWndParent, hWndFrame, hWndClient);
    if ( !hWndFrame )
    {
        delete pFrame;
        return NULL;
    }

    // Parent setzen (Owner)
    if ( hWndParent != 0 && hWndParent != HWND_DESKTOP )
        WinSetOwner( hWndFrame, hWndParent );

    Os2SalFrame* pParentFrame = GetWindowPtr( hWndParent );
    if ( pParentFrame )
        pFrame->mpParentFrame = pParentFrame;

    // Icon setzen (YD win32 does it in the class registration)
    if ( nFrameFlags & FCF_MINBUTTON )
        WinSendMsg( hWndFrame, WM_SETICON, (MPARAM)pInst->mhAppIcon, (MPARAM)0 );

    // If we have an Window with an Caption Bar and without
    // an MaximizeBox, we change the SystemMenu
    if ( (nFrameFlags & (FCF_TITLEBAR | FCF_MAXBUTTON)) == (FCF_TITLEBAR) )
    {
        HWND hSysMenu = WinWindowFromID( hWndFrame, FID_SYSMENU );
        if ( hSysMenu )
        {
            if ( !(nFrameFlags & (FCF_MINBUTTON | FCF_MAXBUTTON)) )
                WinEnableMenuItem(hSysMenu, SC_RESTORE, FALSE);
            if ( !(nFrameFlags & FCF_MINBUTTON) )
                WinEnableMenuItem(hSysMenu, SC_MINIMIZE, FALSE);
            if ( !(nFrameFlags & FCF_MAXBUTTON) )
                WinEnableMenuItem(hSysMenu, SC_MAXIMIZE, FALSE);
            if ( !(nFrameFlags & FCF_SIZEBORDER) )
                WinEnableMenuItem(hSysMenu, SC_SIZE, FALSE);
        }
    }
    if ( (nFrameFlags & FCF_SYSMENU) && !(nSalFrameStyle & SAL_FRAME_STYLE_CLOSEABLE) )
    {
        HWND hSysMenu = WinWindowFromID( hWndFrame, FID_SYSMENU );
        if ( hSysMenu )
        {
            WinEnableMenuItem(hSysMenu, SC_CLOSE, FALSE);
        }
    }

    // ticket#124 subclass frame window: we need to intercept TRACK message
    aSalShlData.mpFrameProc = WinSubclassWindow( hWndFrame, SalFrameSubClassWndProc);

    // init OS/2 frame data
    pFrame->mhAB            = pInst->mhAB;

    // YD 18/08 under OS/2, invisible frames have size 0,0 at 0,0, so
    // we need to set an initial size/position manually
    SWP aSWP;
    memset( &aSWP, 0, sizeof( aSWP ) );
    WinQueryTaskSizePos( pInst->mhAB, 0, &aSWP );
    WinSetWindowPos( hWndFrame, NULL, aSWP.x, aSWP.y, aSWP.cx, aSWP.cy,
                     SWP_MOVE | SWP_SIZE);

#ifdef ENABLE_IME
    // Input-Context einstellen
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        pFrame->mhIMEContext = 0;
        if ( 0 != pIMEData->mpAssocIME( hWndClient, pFrame->mhIMEContext, &pFrame->mhDefIMEContext ) )
            pFrame->mhDefIMEContext = 0;
    }
    else
    {
        pFrame->mhIMEContext = 0;
        pFrame->mhDefIMEContext = 0;
    }
#endif

    RECTL rectl;
    _WinQueryWindowRect( hWndClient, &rectl );
    pFrame->mnWidth  = rectl.xRight;
    pFrame->mnHeight = rectl.yBottom;
    debug_printf( "ImplSalCreateFrame %dx%d\n", pFrame->mnWidth, pFrame->mnHeight);
    ImplSaveFrameState( pFrame );
    pFrame->mbDefPos = TRUE;

    UpdateFrameGeometry( hWndFrame, pFrame );

    if( pFrame->mnShowState == SWP_SHOWMAXIMIZED )
    {
        // #96084 set a useful internal window size because
        // the window will not be maximized (and the size updated) before show()
        SetMaximizedFrameGeometry( hWndFrame, pFrame );
    }

#if OSL_DEBUG_LEVEL > 1
    dumpWindowInfo( "<ImplSalCreateFrame (exit)", hWndFrame);
#endif

    return pFrame;
}

// =======================================================================

Os2SalFrame::Os2SalFrame()
{
    SalData* pSalData = GetSalData();

    mbGraphics          = NULL;
    mhPointer           = WinQuerySysPointer( HWND_DESKTOP, SPTR_ARROW, FALSE );
    mpGraphics          = NULL;
    mpInst              = NULL;
    mbFullScreen        = FALSE;
    mbAllwayOnTop       = FALSE;
    mbVisible           = FALSE;
    mbMinHide           = FALSE;
    mbInShow            = FALSE;
    mbRestoreMaximize   = FALSE;
    mbInMoveMsg         = FALSE;
    mbInSizeMsg         = FALSE;
    mbDefPos            = TRUE;
    mbOverwriteState    = TRUE;
    mbHandleIME         = FALSE;
    mbConversionMode    = FALSE;
    mbCandidateMode     = FALSE;
    mbCaption           = FALSE;
    //mhDefIMEContext     = 0;
    mpGraphics          = NULL;
    mnShowState         = SWP_SHOWNORMAL;
    mnWidth             = 0;
    mnHeight            = 0;
    mnMinWidth          = 0;
    mnMinHeight         = 0;
    mnMaxWidth          = SHRT_MAX;
    mnMaxHeight         = SHRT_MAX;
    mnInputLang         = 0;
    mnKeyboardHandle    = 0;
    mbGraphics          = FALSE;
    mbCaption           = FALSE;
    mbBorder            = FALSE;
    mbFixBorder         = FALSE;
    mbSizeBorder        = FALSE;
    mbFullScreen        = FALSE;
    //mbPresentation      = FALSE;
    mbInShow            = FALSE;
    mbRestoreMaximize   = FALSE;
    mbInMoveMsg         = FALSE;
    mbInSizeMsg         = FALSE;
    //mbFullScreenToolWin = FALSE;
    mbDefPos            = TRUE;
    mbOverwriteState    = TRUE;
    //mbIME               = FALSE;
    mbHandleIME         = FALSE;
    //mbSpezIME           = FALSE;
    //mbAtCursorIME       = FALSE;
    mbCandidateMode     = FALSE;
    mbFloatWin          = FALSE;
    mbNoIcon            = FALSE;
    //mSelectedhMenu      = 0;
    //mLastActivatedhMenu = 0;
    mpParentFrame       = NULL;

    memset( &maState, 0, sizeof( SalFrameState ) );
    maSysData.nSize     = sizeof( SystemEnvData );
    memset( &maGeometry, 0, sizeof( maGeometry ) );

    // insert frame in framelist
    mpNextFrame = pSalData->mpFirstFrame;
    pSalData->mpFirstFrame = this;
}

// -----------------------------------------------------------------------

Os2SalFrame::~Os2SalFrame()
{
    SalData* pSalData = GetSalData();

    // destroy DC
    if ( mpGraphics )
    {
        ImplSalDeInitGraphics( mpGraphics );
        WinReleasePS( mpGraphics->mhPS );
        delete mpGraphics;
    }

    // destroy system frame
    WinDestroyWindow( mhWndFrame );

    // remove frame from framelist
    if ( this == pSalData->mpFirstFrame )
        pSalData->mpFirstFrame = mpNextFrame;
    else
    {
        Os2SalFrame* pTempFrame = pSalData->mpFirstFrame;
        while ( pTempFrame->mpNextFrame != this )
            pTempFrame = pTempFrame->mpNextFrame;

        pTempFrame->mpNextFrame = mpNextFrame;
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

SalGraphics* Os2SalFrame::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    if ( !mpGraphics )
    {
        SalData* pSalData = GetSalData();
        mpGraphics = new Os2SalGraphics;
        mpGraphics->mhPS      = WinGetPS( mhWndClient );
        mpGraphics->mhDC      = ImplWinGetDC( mhWndClient );
        mpGraphics->mhWnd     = mhWndClient;
        mpGraphics->mnHeight  = mnHeight;
        mpGraphics->mbPrinter = FALSE;
        mpGraphics->mbVirDev  = FALSE;
        mpGraphics->mbWindow  = TRUE;
        mpGraphics->mbScreen  = TRUE;
        ImplSalInitGraphics( mpGraphics );
        mbGraphics = TRUE;
    }
    else
        mbGraphics = TRUE;

    return mpGraphics;
}

// -----------------------------------------------------------------------

void Os2SalFrame::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

sal_Bool Os2SalFrame::PostEvent( void* pData )
{
    return (sal_Bool)WinPostMsg( mhWndClient, SAL_MSG_USEREVENT, 0, (MPARAM)pData );
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetTitle( const XubString& rTitle )
{
    // set window title
    ByteString title( rTitle, gsl_getSystemTextEncoding() );
    debug_printf("Os2SalFrame::SetTitle %x '%s'\n", mhWndFrame, title.GetBuffer() );
    WinSetWindowText( mhWndFrame, title.GetBuffer() );
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetIcon( USHORT nIcon )
{
    debug_printf("Os2SalFrame::SetIcon\n");

    // If we have a window without an Icon (for example a dialog), ignore this call
    if ( mbNoIcon )
        return;

    // 0 means default (class) icon
    HPOINTER hIcon = NULL;
    if ( !nIcon )
        nIcon = 1;

    ImplLoadSalIcon( nIcon, hIcon );

    DBG_ASSERT( hIcon , "Os2SalFrame::SetIcon(): Could not load icon !" );

    // Icon setzen
    WinSendMsg( mhWndFrame, WM_SETICON, (MPARAM)hIcon, (MPARAM)0 );
}

// -----------------------------------------------------------------------

SalFrame* Os2SalFrame::GetParent() const
{
    //debug_printf("Os2SalFrame::GetParent\n");
    return GetWindowPtr( WinQueryWindow(mhWndFrame, QW_OWNER) );
}

// -----------------------------------------------------------------------

static void ImplSalShow( HWND hWnd, ULONG bVisible, ULONG bNoActivate )
{
    Os2SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return;

    if ( bVisible )
    {
        pFrame->mbDefPos = FALSE;
        pFrame->mbOverwriteState = TRUE;
        pFrame->mbInShow = TRUE;

#if OSL_DEBUG_LEVEL > 0
        debug_printf( "ImplSalShow hwnd %x visible flag %d, no activate: flag %d\n", hWnd, bVisible, bNoActivate);
#endif

        if( bNoActivate )
            WinSetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_SHOW);
        else
            WinSetWindowPos(hWnd, NULL, 0, 0, 0, 0, pFrame->mnShowState);

        pFrame->mbInShow = FALSE;

        // Direct Paint only, if we get the SolarMutx
        if ( ImplSalYieldMutexTryToAcquire() )
        {
            WinUpdateWindow( hWnd );
            ImplSalYieldMutexRelease();
        }
    }
    else
    {
#if OSL_DEBUG_LEVEL > 0
        debug_printf( "ImplSalShow hwnd %x HIDE\n");
#endif
        WinSetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_HIDE);
    }
}


// -----------------------------------------------------------------------


void Os2SalFrame::SetExtendedFrameStyle( SalExtStyle nExtStyle )
{
}

// -----------------------------------------------------------------------

void Os2SalFrame::Show( sal_Bool bVisible, sal_Bool bNoActivate )
{
    // Post this Message to the window, because this only works
    // in the thread of the window, which has create this window.
    // We post this message to avoid deadlocks
    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
        WinPostMsg( mhWndFrame, SAL_MSG_SHOW, (MPARAM)bVisible, (MPARAM)bNoActivate );
    else
        ImplSalShow( mhWndFrame, bVisible, bNoActivate );
}

// -----------------------------------------------------------------------

void Os2SalFrame::Enable( sal_Bool bEnable )
{
    WinEnableWindow( mhWndFrame, bEnable );
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    debug_printf("Os2SalFrame::SetMinClientSize\n");
    mnMinWidth  = nWidth;
    mnMinHeight = nHeight;
}

void Os2SalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    debug_printf("Os2SalFrame::SetMaxClientSize\n");
    mnMaxWidth  = nWidth;
    mnMaxHeight = nHeight;
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight,
                                                   USHORT nFlags )
{
    // calculation frame size
    USHORT  nEvent = 0;
    ULONG   nPosFlags = 0;

#if OSL_DEBUG_LEVEL > 0
    //dumpWindowInfo( "-Os2SalFrame::SetPosSize", mhWndFrame);
    debug_printf( ">Os2SalFrame::SetPosSize go to %d,%d (%dx%d) VCL\n",nX,nY,nWidth,nHeight);
#endif

    SWP aSWP;
    _WinQueryWindowPos( this, &aSWP );
    sal_Bool bVisible = WinIsWindowVisible( mhWndFrame );
    if ( !bVisible )
    {
        if ( mbFloatWin )
            mnShowState = SWP_SHOW;
        else
            mnShowState = SWP_SHOWNORMAL;
    }
    else
    {
        if ( (aSWP.fl & SWP_MINIMIZE) || (aSWP.fl & SWP_MAXIMIZE) )
            WinSetWindowPos(mhWndFrame, NULL, 0, 0, 0, 0, SWP_RESTORE );
    }

    if ( (nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y)) ) {
        nPosFlags |= SWP_MOVE;
#if OSL_DEBUG_LEVEL > 0
        debug_printf( "-Os2SalFrame::SetPosSize MOVE to %d,%d\n", nX, nY);
#endif
        //DBG_ASSERT( nX && nY, " Windowposition of (0,0) requested!" );
        nEvent = SALEVENT_MOVE;
    }

    if ( (nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT)) ) {
        nPosFlags |= SWP_SIZE;
#if OSL_DEBUG_LEVEL > 0
        debug_printf( "-Os2SalFrame::SetPosSize SIZE to %d,%d\n", nWidth,nHeight);
#endif
        nEvent = (nEvent == SALEVENT_MOVE) ? SALEVENT_MOVERESIZE : SALEVENT_RESIZE;
    }

    // Default-Position, dann zentrieren, ansonsten Position beibehalten
    if ( mbDefPos  && !(nPosFlags & SWP_MOVE))
    {
        // calculate bottom left corner of frame
        mbDefPos = FALSE;
        nPosFlags |= SWP_MOVE | SWP_CENTER;
        nEvent = SALEVENT_MOVERESIZE;
#if OSL_DEBUG_LEVEL > 10
        debug_printf( "-Os2SalFrame::SetPosSize CENTER\n");
        debug_printf( "-Os2SalFrame::SetPosSize default position to %d,%d\n", nX, nY);
#endif
    }

    // Adjust Window in the screen
    sal_Bool bCheckOffScreen = TRUE;

    // but don't do this for floaters or ownerdraw windows that are currently moved interactively
    if( (mnStyle & SAL_FRAME_STYLE_FLOAT) && !(mnStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
        bCheckOffScreen = FALSE;

    if( mnStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION )
    {
        // may be the window is currently being moved (mouse is captured), then no check is required
        if( mhWndClient == WinQueryCapture( HWND_DESKTOP) )
            bCheckOffScreen = FALSE;
        else
            bCheckOffScreen = TRUE;
    }

    if( bCheckOffScreen )
    {
        if ( nX+nWidth > nScreenWidth )
            nX = nScreenWidth - nWidth;
        if ( nY+nHeight > nScreenHeight )
            nY = nScreenHeight - nHeight;
        if ( nX < 0 )
            nX = 0;
        if ( nY < 0 )
            nY = 0;
    }

    // bring floating windows always to top
    // do not change zorder, otherwise tooltips will bring main window to top (ticket:14)
    //if( (mnStyle & SAL_FRAME_STYLE_FLOAT) )
    //    nPosFlags |= SWP_ZORDER; // do not change z-order

    // set new position
    _WinSetWindowPos( this, HWND_TOP, nX, nY, nWidth, nHeight, nPosFlags); // | SWP_RESTORE

    UpdateFrameGeometry( mhWndFrame, this );

    // Notification -- really ???
    if( nEvent )
        CallCallback( nEvent, NULL );

#if OSL_DEBUG_LEVEL > 0
    dumpWindowInfo( "<Os2SalFrame::SetPosSize (exit)", mhWndFrame);
#endif

}

// -----------------------------------------------------------------------

void Os2SalFrame::SetParent( SalFrame* pNewParent )
{
    APIRET rc;
#if OSL_DEBUG_LEVEL>0
    debug_printf("Os2SalFrame::SetParent mhWndFrame 0x%08x to 0x%08x\n",
            static_cast<Os2SalFrame*>(this)->mhWndFrame,
            static_cast<Os2SalFrame*>(pNewParent)->mhWndClient);
#endif
    Os2SalFrame::mbInReparent = TRUE;
    //rc = WinSetParent(static_cast<Os2SalFrame*>(this)->mhWndFrame,
    //                  static_cast<Os2SalFrame*>(pNewParent)->mhWndClient, TRUE);
    rc = WinSetOwner(static_cast<Os2SalFrame*>(this)->mhWndFrame,
                      static_cast<Os2SalFrame*>(pNewParent)->mhWndClient);
    mpParentFrame = static_cast<Os2SalFrame*>(pNewParent);
    Os2SalFrame::mbInReparent = FALSE;
}

bool Os2SalFrame::SetPluginParent( SystemParentData* pNewParent )
{
    APIRET rc;
    if ( pNewParent->hWnd == 0 )
    {
        pNewParent->hWnd = HWND_DESKTOP;
    }

    Os2SalFrame::mbInReparent = TRUE;
    rc = WinSetOwner(static_cast<Os2SalFrame*>(this)->mhWndFrame,
                      pNewParent->hWnd);
    Os2SalFrame::mbInReparent = FALSE;
    return true;
}


// -----------------------------------------------------------------------

void Os2SalFrame::GetWorkArea( RECTL &rRect )
{
    rRect.xLeft     = rRect.yTop = 0;
    rRect.xRight    = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN )-1;
    rRect.yBottom   = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN )-1;
}

// -----------------------------------------------------------------------

void Os2SalFrame::GetWorkArea( Rectangle &rRect )
{
    RECTL aRect;
    GetWorkArea( aRect);
    rRect.nLeft     = aRect.xLeft;
    rRect.nRight    = aRect.xRight; // win -1;
    rRect.nTop      = aRect.yTop;
    rRect.nBottom   = aRect.yBottom; // win -1;
}

// -----------------------------------------------------------------------

void Os2SalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    rWidth  = maGeometry.nWidth;
    rHeight = maGeometry.nHeight;
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetWindowState( const SalFrameState* pState )
{
    LONG    nX;
    LONG    nY;
    LONG    nWidth;
    LONG    nHeight;
    ULONG   nPosSize = 0;

#if OSL_DEBUG_LEVEL>0
    debug_printf("Os2SalFrame::SetWindowState\n");
    debug_printf("Os2SalFrame::SetWindowState %08x (%dx%d) at %d,%d VCL\n",
        mhWndFrame,
        pState->mnWidth,pState->mnHeight,pState->mnX,pState->mnY);
#endif

    sal_Bool bVisible = WinIsWindowVisible( mhWndFrame );

    // get screen coordinates
    SWP aSWP;
    WinQueryWindowPos( mhWndFrame, &aSWP );
    LONG nFrameX, nFrameY, nCaptionY;
    ImplSalCalcFrameSize( this, nFrameX, nFrameY, nCaptionY );

    long nTopDeco = nFrameY + nCaptionY;
    long nLeftDeco = nFrameX;
    long nBottomDeco = nFrameY;
    long nRightDeco = nFrameX;

    // Fenster-Position/Groesse in den Bildschirm einpassen
    if ((pState->mnMask & (SAL_FRAMESTATE_MASK_X | SAL_FRAMESTATE_MASK_Y)) )
        nPosSize |= SWP_MOVE;
    if ((pState->mnMask & (SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT)) )
        nPosSize |= SWP_SIZE;

    if ( pState->mnMask & SAL_FRAMESTATE_MASK_X )
        nX = (int)pState->mnX - nLeftDeco;
    else
        nX = aSWP.x;

    // keep Y inverted since height is still unknown, will invert later
    if ( pState->mnMask & SAL_FRAMESTATE_MASK_Y )
        nY = (int)pState->mnY - nTopDeco;
    else
        nY = nScreenHeight - (aSWP.y+aSWP.cy);

    if ( pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH )
        nWidth = (int)pState->mnWidth + nLeftDeco + nRightDeco;
    else
        nWidth = aSWP.cx;
    if ( pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT )
        nHeight = (int)pState->mnHeight + nTopDeco + nBottomDeco;
    else
        nHeight = aSWP.cy;

#if OSL_DEBUG_LEVEL>0
    debug_printf("Os2SalFrame::SetWindowState (%dx%d) at %d,%d\n", nWidth,nHeight,nX,nY);
#endif

    // Adjust Window in the screen:
    // if it does not fit into the screen do nothing, ie default pos/size will be used
    // if there is an overlap with the screen border move the window while keeping its size

    //if( nWidth > nScreenWidth || nHeight > nScreenHeight )
    //    nPosSize |= (SWP_NOMOVE | SWP_NOSIZE);

    if ( nX+nWidth > nScreenWidth )
        nX = (nScreenWidth) - nWidth;
    if ( nY+nHeight > nScreenHeight )
        nY = (nScreenHeight) - nHeight;
    if ( nX < 0 )
        nX = 0;
    if ( nY < 0 )
        nY = 0;

    // Restore-Position setzen
    SWP aPlacement;
    WinQueryWindowPos( mhWndFrame, &aPlacement );

    // Status setzen
    bVisible = WinIsWindowVisible( mhWndFrame);
    sal_Bool bUpdateHiddenFramePos = FALSE;
    if ( !bVisible )
    {
        aPlacement.fl = SWP_HIDE;

        if ( mbOverwriteState )
        {
            if ( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
            {
                if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
                    mnShowState = SWP_SHOWMINIMIZED;
                else if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
                {
                    mnShowState = SWP_SHOWMAXIMIZED;
                    bUpdateHiddenFramePos = TRUE;
                }
                else if ( pState->mnState & SAL_FRAMESTATE_NORMAL )
                    mnShowState = SWP_SHOWNORMAL;
            }
        }
    }
    else
    {
        if ( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
        {
            if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
            {
                //if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
                //    aPlacement.flags |= WPF_RESTORETOMAXIMIZED;
                aPlacement.fl = SWP_SHOWMINIMIZED;
            }
            else if ( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
                aPlacement.fl = SWP_SHOWMAXIMIZED;
            else if ( pState->mnState & SAL_FRAMESTATE_NORMAL )
                aPlacement.fl = SWP_RESTORE;
        }
    }

    // Wenn Fenster nicht minimiert/maximiert ist oder nicht optisch
    // umgesetzt werden muss, dann SetWindowPos() benutzen, da
    // SetWindowPlacement() die TaskBar mit einrechnet
    if ( !(aPlacement.fl & SWP_MINIMIZE)
         && !( aPlacement.fl & SWP_MAXIMIZE )
          && (!bVisible || (aPlacement.fl == SWP_RESTORE)) )
    {
        if( bUpdateHiddenFramePos )
        {
            // #96084 set a useful internal window size because
            // the window will not be maximized (and the size updated) before show()
            SetMaximizedFrameGeometry( mhWndFrame, this );
        }
        else
            WinSetWindowPos( mhWndFrame, 0, nX,
                nScreenHeight - (nY+nHeight), nWidth, nHeight, nPosSize);
    }
    else
    {
        if( (nPosSize & (SWP_MOVE|SWP_SIZE)) )
        {
            aPlacement.x = nX;
            aPlacement.y = nScreenHeight-(nY+nHeight);
            aPlacement.cx = nWidth;
            aPlacement.cy = nHeight;
        }
        WinSetWindowPos( mhWndFrame, 0, aPlacement.x, aPlacement.y,
                         aPlacement.cx, aPlacement.cy, aPlacement.fl );
    }

#if OSL_DEBUG_LEVEL>0
    debug_printf("Os2SalFrame::SetWindowState DONE\n");
#endif
}

// -----------------------------------------------------------------------

sal_Bool Os2SalFrame::GetWindowState( SalFrameState* pState )
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

void Os2SalFrame::SetScreenNumber( unsigned int nNewScreen )
{
#if 0
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
            maGeometry.nScreenNumber = nNewScreen;
            SetPosSize( aNewMonPos.X() + (maGeometry.nX - aOldMonPos.X()),
                        aNewMonPos.Y() + (maGeometry.nY - aOldMonPos.Y()),
                        0, 0,
                        SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y );
        }
    }
#endif
}

// -----------------------------------------------------------------------

// native menu implementation - currently empty
void Os2SalFrame::DrawMenuBar()
{
}

void Os2SalFrame::SetMenu( SalMenu* pSalMenu )
{
}

// -----------------------------------------------------------------------

void Os2SalFrame::ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nDisplay )
{
    if ( mbFullScreen == bFullScreen )
        return;

    mbFullScreen = bFullScreen;
    if ( bFullScreen )
    {
        // save old position
        memset( &maFullScreenRect, 0, sizeof( SWP ) );
        _WinQueryWindowPos( this, &maFullScreenRect );

        // set window to screen size
        ImplSalFrameFullScreenPos( this, TRUE );
    }
    else
    {
        _WinSetWindowPos( this,
                         0,
                         maFullScreenRect.x, maFullScreenRect.y,
                         maFullScreenRect.cx, maFullScreenRect.cy,
                         SWP_MOVE | SWP_SIZE );
    }
}

// -----------------------------------------------------------------------

void Os2SalFrame::StartPresentation( sal_Bool bStart )
{
    // SysSetObjectData("<WP_DESKTOP>","Autolockup=no"); oder OS2.INI: PM_Lockup
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetAlwaysOnTop( sal_Bool bOnTop )
{
    mbAllwayOnTop = bOnTop;
#if 0
    HWND hWnd;
    if ( bOnTop )
        hWnd = HWND_TOPMOST;
    else
        hWnd = HWND_NOTOPMOST;
    SetWindowPos( mhWnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
#endif
}


// -----------------------------------------------------------------------

static void ImplSalToTop( HWND hWnd, ULONG nFlags )
{
    Os2SalFrame* pFrame = GetWindowPtr( hWnd );
#if OSL_DEBUG_LEVEL>0
    debug_printf("ImplSalToTop hWnd %08x, nFlags %x\n", hWnd, nFlags);
#endif

    // if window is minimized, first restore it
    SWP aSWP;
    WinQueryWindowPos( hWnd, &aSWP );
    if ( aSWP.fl & SWP_MINIMIZE )
        WinSetWindowPos( hWnd, NULL, 0, 0, 0, 0, SWP_RESTORE );

    if ( nFlags & SAL_FRAME_TOTOP_FOREGROUNDTASK )
        WinSetWindowPos( pFrame->mhWndFrame, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE | SWP_ZORDER);

    if ( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
    {
        ULONG   nStyle;
        if ( pFrame->mbRestoreMaximize )
            nStyle = SWP_MAXIMIZE;
        else
            nStyle = SWP_RESTORE;

        WinSetWindowPos( pFrame->mhWndFrame, NULL, 0, 0, 0, 0, nStyle );
    }
    WinSetFocus( HWND_DESKTOP, pFrame->mhWndClient );
}

// -----------------------------------------------------------------------

void Os2SalFrame::ToTop( USHORT nFlags )
{
    nFlags &= ~SAL_FRAME_TOTOP_GRABFOCUS;   // this flag is not needed on win32
    // Post this Message to the window, because this only works
    // in the thread of the window, which has create this window.
    // We post this message to avoid deadlocks
    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
        WinPostMsg( mhWndFrame, SAL_MSG_TOTOP, (MPARAM)nFlags, 0 );
    else
        ImplSalToTop( mhWndFrame, nFlags );
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetPointer( PointerStyle ePointerStyle )
{
    struct ImplPtrData
    {
        HPOINTER    mhPointer;
        ULONG       mnSysId;
        ULONG       mnOwnId;
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
    { 0, 0, SAL_RESID_POINTER_AUTOSCROLL_NSWE },     // POINTER_AUTOSCROLL_NSWE
    { 0, 0, SAL_RESID_POINTER_AIRBRUSH },           // POINTER_AIRBRUSH
    { 0, 0, SAL_RESID_POINTER_TEXT_VERTICAL },      // POINTER_TEXT_VERTICAL
    { 0, 0, SAL_RESID_POINTER_PIVOT_DELETE },       // POINTER_PIVOT_DELETE

    // --> FME 2004-07-30 #i32329# Enhanced table selection
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_S },       // POINTER_TAB_SELECT_S
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_E },       // POINTER_TAB_SELECT_E
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_SE },      // POINTER_TAB_SELECT_SE
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_W },       // POINTER_TAB_SELECT_W
    { 0, 0, SAL_RESID_POINTER_TAB_SELECT_SW },      // POINTER_TAB_SELECT_SW
    // <--

    // --> FME 2004-08-16 #i20119# Paintbrush tool
    { 0, 0, SAL_RESID_POINTER_PAINTBRUSH }          // POINTER_PAINTBRUSH
    // <--
    };

#if POINTER_COUNT != 94
#error New Pointer must be defined!
#endif

    //debug_printf("Os2SalFrame::SetPointer\n");

    // Mousepointer loaded ?
    if ( !aImplPtrTab[ePointerStyle].mhPointer )
    {
        if ( aImplPtrTab[ePointerStyle].mnOwnId )
            aImplPtrTab[ePointerStyle].mhPointer = ImplLoadSalCursor( (ULONG)aImplPtrTab[ePointerStyle].mnOwnId );
        else
            aImplPtrTab[ePointerStyle].mhPointer = WinQuerySysPointer( HWND_DESKTOP, aImplPtrTab[ePointerStyle].mnSysId, FALSE );
    }
    if (aImplPtrTab[ePointerStyle].mhPointer == 0) {
        debug_printf( "SetPointer ePointerStyle %d unknown\n", ePointerStyle);
        aImplPtrTab[ePointerStyle].mhPointer = SPTR_ICONERROR;
    }

    // Unterscheidet sich der Mauspointer, dann den neuen setzen
    if ( mhPointer != aImplPtrTab[ePointerStyle].mhPointer )
    {
        mhPointer = aImplPtrTab[ePointerStyle].mhPointer;
        WinSetPointer( HWND_DESKTOP, mhPointer );
    }
}

// -----------------------------------------------------------------------

void Os2SalFrame::CaptureMouse( sal_Bool bCapture )
{
#if OSL_DEBUG_LEVEL>10
    _bCapture=bCapture;
    debug_printf("Os2SalFrame::CaptureMouse bCapture %d\n", bCapture);
#endif
    if ( bCapture )
        WinSetCapture( HWND_DESKTOP, mhWndClient );
    else
        WinSetCapture( HWND_DESKTOP, 0 );
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetPointerPos( long nX, long nY )
{
    POINTL aPt;
    aPt.x = nX;
    aPt.y = mnHeight - nY - 1;  // convert sal coords to sys
    WinMapWindowPoints( mhWndClient, HWND_DESKTOP, &aPt, 1 );
    WinSetPointerPos( HWND_DESKTOP, aPt.x, aPt.y );
}

// -----------------------------------------------------------------------

void Os2SalFrame::Flush()
{
}

// -----------------------------------------------------------------------

void Os2SalFrame::Sync()
{
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetInputContext( SalInputContext* pContext )
{
#ifdef ENABLE_IME
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND hWnd = mhWndClient;
        HIMI hIMI = 0;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            ULONG nInputMode;
            ULONG nConversionMode;
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
#if 0
void Os2SalFrame::UpdateExtTextInputArea()
{
#ifdef ENABLE_IME
#endif
}
#endif

// -----------------------------------------------------------------------

void Os2SalFrame::EndExtTextInput( USHORT nFlags )
{
#ifdef ENABLE_IME
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND hWnd = mhWndClient;
        HIMI hIMI = 0;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            ULONG nIndex;
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

XubString Os2SalFrame::GetKeyName( USHORT nCode )
{
    if ( eImplKeyboardLanguage == LANGUAGE_DONTKNOW )
        eImplKeyboardLanguage = MsLangId::getSystemLanguage();

    XubString        aKeyCode;
    XubString        aCode;
    const sal_Unicode**    pLangTab = ImplGetLangTab( eImplKeyboardLanguage );

    if ( nCode & KEY_SHIFT )
        aKeyCode = pLangTab[LSTR_KEY_SHIFT];

    if ( nCode & KEY_MOD1 )
    {
        if ( aKeyCode.Len() == 0 )
            aKeyCode = pLangTab[LSTR_KEY_CTRL];
        else
        {
            aKeyCode += '+';
            aKeyCode += pLangTab[LSTR_KEY_CTRL];
        }
    }

    if ( nCode & KEY_MOD2 )
    {
        if ( aKeyCode.Len() == 0 )
            aKeyCode = pLangTab[LSTR_KEY_ALT];
        else
        {
            aKeyCode += '+';
            aKeyCode += pLangTab[LSTR_KEY_ALT];
        }
    }

    USHORT nKeyCode = nCode & 0x0FFF;
    if ( (nKeyCode >= KEY_0) && (nKeyCode <= KEY_9) )
        aCode = sal::static_int_cast<sal_Char>('0' + (nKeyCode - KEY_0));
    else if ( (nKeyCode >= KEY_A) && (nKeyCode <= KEY_Z) )
        aCode = sal::static_int_cast<sal_Char>('A' + (nKeyCode - KEY_A));
    else if ( (nKeyCode >= KEY_F1) && (nKeyCode <= KEY_F26) )
    {
        aCode += 'F';
        if ( (nKeyCode >= KEY_F1) && (nKeyCode <= KEY_F9) )
        {
            aCode += sal::static_int_cast<sal_Char>('1' + (nKeyCode - KEY_F1));
        }
        else if ( (nKeyCode >= KEY_F10) && (nKeyCode <= KEY_F19) )
        {
            aCode += '1';
            aCode += sal::static_int_cast<sal_Char>('0' + (nKeyCode - KEY_F10));
        }
        else
        {
            aCode += '2';
            aCode += sal::static_int_cast<sal_Char>('0' + (nKeyCode - KEY_F20));
        }
    }
    else
    {
        switch ( nKeyCode )
        {
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
    }

    if ( aCode.Len() )
    {
        if ( aKeyCode.Len() == 0 )
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

XubString Os2SalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
    return GetKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

inline long ImplOS2ColorToSal( long nOS2Color )
{
    return MAKE_SALCOLOR( (PM_BYTE)( nOS2Color>>16), (PM_BYTE)(nOS2Color>>8), (PM_BYTE)nOS2Color );
}

// -----------------------------------------------------------------------

static USHORT ImplMouseSysValueToSAL( int iSysValue, USHORT& rCode, USHORT& rClicks, sal_Bool& rDown )
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

static sal_Bool ImplSalIsSameColor( const Color& rColor1, const Color& rColor2 )
{
    ULONG nWrong = 0;
    nWrong += Abs( (short)rColor1.GetRed()-(short)rColor2.GetRed() );
    nWrong += Abs( (short)rColor1.GetGreen()-(short)rColor2.GetGreen() );
    nWrong += Abs( (short)rColor1.GetBlue()-(short)rColor2.GetBlue() );
    return (nWrong < 30);
}

// -----------------------------------------------------------------------

static sal_Bool ImplOS2NameFontToVCLFont( const char* pFontName, Font& rFont )
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
        rFont.SetFamily( FAMILY_DONTKNOW );
        rFont.SetWeight( WEIGHT_NORMAL );
        rFont.SetItalic( ITALIC_NONE );
        // search for a style embedded in the name, e.g. 'WarpSans Bold'
        // because we need to split the style from the family name
        if (strstr( pFontName, " Bold")
            || strstr( pFontName, " Italic")
            || strstr( pFontName, "-Normal"))
        {
            char* fontName = strdup( pFontName);
            char* style = strstr( fontName, " Bold");
            if (style)
                rFont.SetWeight( WEIGHT_BOLD );

            if (!style)
                style = strstr( fontName, " Italic");
            if (style)
                rFont.SetItalic( ITALIC_NORMAL );

            if (!style)
                style = strstr( fontName, "-Normal");
            // store style, skip whitespace char
            rFont.SetStyleName( ::rtl::OStringToOUString ( style+1, gsl_getSystemTextEncoding()) );
            // truncate name
            *style = 0;
            // store family name
            rFont.SetName( ::rtl::OStringToOUString ( fontName, gsl_getSystemTextEncoding()) );
            free( fontName);
        }
        else
        {
            rFont.SetName( ::rtl::OStringToOUString (pFontName, gsl_getSystemTextEncoding()) );
            rFont.SetStyleName( ::rtl::OStringToOUString ("", gsl_getSystemTextEncoding()) );
        }

        rFont.SetSize( Size( 0, nFontHeight ) );
        return TRUE;
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void Os2SalFrame::UpdateSettings( AllSettings& rSettings )
{
    static char aControlPanel[] = "PM_ControlPanel";
    static char aSystemFonts[]  = "PM_SystemFonts";
    char aDummyStr[] = "";

    // --- Mouse setting ---
    USHORT  nCode;
    USHORT  nClicks;
    sal_Bool    bDown;
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
    if ( WinQuerySysValue( HWND_DESKTOP, SV_DYNAMICDRAG ) )
        nDragFullOptions |= DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE | DRAGFULL_OPTION_DOCKING | DRAGFULL_OPTION_SPLIT;
    else
        nDragFullOptions &= ~(DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE | DRAGFULL_OPTION_DOCKING | DRAGFULL_OPTION_SPLIT);
    aStyleSettings.SetDragFullOptions( nDragFullOptions );

    // Size settings
    aStyleSettings.SetScrollBarSize( WinQuerySysValue( HWND_DESKTOP, SV_CYHSCROLL ) );
    aStyleSettings.SetTitleHeight( WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR ) );

    // Color settings
    aStyleSettings.SetFaceColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0 ) ) );
    aStyleSettings.SetInactiveTabColor( aStyleSettings.GetFaceColor() );
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
    aStyleSettings.SetMenuBarTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUTEXT, 0 ) ) );
    aStyleSettings.SetDialogTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetRadioCheckTextColor( aStyleSettings.GetButtonTextColor() );
    aStyleSettings.SetGroupTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_WINDOWSTATICTEXT, 0 ) ) );
    aStyleSettings.SetLabelTextColor( aStyleSettings.GetGroupTextColor() );
    aStyleSettings.SetInfoTextColor( aStyleSettings.GetGroupTextColor() );
    aStyleSettings.SetWindowColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_WINDOW, 0 ) ) );
    aStyleSettings.SetActiveTabColor( aStyleSettings.GetWindowColor() );
    aStyleSettings.SetWindowTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_WINDOWTEXT, 0 ) ) );
    aStyleSettings.SetFieldColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_ENTRYFIELD, 0 ) ) );
    aStyleSettings.SetFieldTextColor( aStyleSettings.GetWindowTextColor() );
    aStyleSettings.SetDisableColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUDISABLEDTEXT, 0 ) ) );
    aStyleSettings.SetHighlightColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_HILITEBACKGROUND, 0 ) ) );
    aStyleSettings.SetHighlightTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_HILITEFOREGROUND, 0 ) ) );
    Color aMenuHighColor = ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUHILITEBGND, 0 ) );
    if ( ImplSalIsSameColor( aMenuHighColor, aStyleSettings.GetMenuColor() ) )
    {
        aStyleSettings.SetMenuHighlightColor( Color( COL_BLUE ) );
        aStyleSettings.SetMenuHighlightTextColor( Color( COL_WHITE ) );
    }
    else
    {
        aStyleSettings.SetMenuHighlightColor( aMenuHighColor );
        aStyleSettings.SetMenuHighlightTextColor( ImplOS2ColorToSal( WinQuerySysColor( HWND_DESKTOP, SYSCLR_MENUHILITE, 0 ) ) );
    }
    // Checked-Color berechnen
    Color   aColor1 = aStyleSettings.GetFaceColor();
    Color   aColor2 = aStyleSettings.GetLightColor();
    PM_BYTE    nRed    = (PM_BYTE)(((USHORT)aColor1.GetRed()   + (USHORT)aColor2.GetRed())/2);
    PM_BYTE    nGreen  = (PM_BYTE)(((USHORT)aColor1.GetGreen() + (USHORT)aColor2.GetGreen())/2);
    PM_BYTE    nBlue   = (PM_BYTE)(((USHORT)aColor1.GetBlue()  + (USHORT)aColor2.GetBlue())/2);
    aStyleSettings.SetCheckedColor( Color( nRed, nGreen, nBlue ) );

    // Fonts updaten
    Font    aFont;
    char    aFontNameBuf[255];
    aFont = aStyleSettings.GetMenuFont();
    if ( PrfQueryProfileString( HINI_PROFILE, (PSZ)aSystemFonts, (PSZ)"Menus", aDummyStr, aFontNameBuf, sizeof( aFontNameBuf ) ) > 5 )
    {
        if ( ImplOS2NameFontToVCLFont( aFontNameBuf, aFont ) ) {
#if 0
            // Add Workplace Sans if not already listed
            if ( aFont.GetName().Search( (sal_Unicode*)L"WorkPlace Sans" ) == STRING_NOTFOUND )
            {
                XubString aFontName = aFont.GetName();
                aFontName.Insert( (sal_Unicode*)L"WorkPlace Sans;", 0 );
                aFont.SetName( aFontName );
                aFont.SetSize( Size( 0, 9 ) );
            }
#endif
            aStyleSettings.SetMenuFont( aFont );
        }
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
            // Add Workplace Sans if not already listed
            if ( aFont.GetName().Search( (sal_Unicode*)L"WorkPlace Sans" ) == STRING_NOTFOUND )
            {
                XubString aFontName = aFont.GetName();
                aFontName.Insert( (sal_Unicode*)L"WorkPlace Sans;", 0 );
                aFont.SetName( aFontName );
                aFont.SetSize( Size( 0, 9 ) );
                aFont.SetWeight( WEIGHT_BOLD );
                aFont.SetItalic( ITALIC_NONE );
            }
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
            aHelpFont.SetName( (sal_Unicode*)L"Helv;WarpSans" );
            aHelpFont.SetSize( Size( 0, 8 ) );
            aHelpFont.SetWeight( WEIGHT_NORMAL );
            aHelpFont.SetItalic( ITALIC_NONE );
            aStyleSettings.SetHelpFont( aHelpFont );

            // Add Workplace Sans if not already listed
            if ( aFont.GetName().Search( (sal_Unicode*)L"WorkPlace Sans" ) == STRING_NOTFOUND )
            {
                XubString aFontName = aFont.GetName();
                aFontName.Insert( (sal_Unicode*)L"WorkPlace Sans;", 0 );
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

SalBitmap* Os2SalFrame::SnapShot()
{
debug_printf("Os2SalFrame::SnapShot\n");
return NULL;
}

// -----------------------------------------------------------------------

const SystemEnvData* Os2SalFrame::GetSystemData() const
{
    return &maSysData;
}

// -----------------------------------------------------------------------

void Os2SalFrame::Beep( SoundType eSoundType )
{
    static ULONG aImplSoundTab[5] =
    {
        WA_NOTE,                        // SOUND_DEFAULT
        WA_NOTE,                        // SOUND_INFO
        WA_WARNING,                     // SOUND_WARNING
        WA_ERROR,                       // SOUND_ERROR
        WA_NOTE                         // SOUND_QUERY
    };

#if 0
#if SOUND_COUNT != 5
#error New Sound must be defined!
#endif
#endif

    debug_printf("Os2SalFrame::Beep %d\n", eSoundType);
    WinAlarm( HWND_DESKTOP, aImplSoundTab[eSoundType] );
}

// -----------------------------------------------------------------------

SalFrame::SalPointerState Os2SalFrame::GetPointerState()
{
    SalPointerState aState;
    aState.mnState = 0;

    // MausModus feststellen und setzen
    if ( WinGetKeyState( HWND_DESKTOP, VK_BUTTON1 ) & 0x8000 )
        aState.mnState |= MOUSE_LEFT;
    if ( WinGetKeyState( HWND_DESKTOP, VK_BUTTON2 ) & 0x8000 )
        aState.mnState |= MOUSE_RIGHT;
    if ( WinGetKeyState( HWND_DESKTOP, VK_BUTTON3 ) & 0x8000 )
        aState.mnState |= MOUSE_MIDDLE;
    // Modifier-Tasten setzen
    if ( WinGetKeyState( HWND_DESKTOP, VK_SHIFT ) & 0x8000 )
        aState.mnState |= KEY_SHIFT;
    if ( WinGetKeyState( HWND_DESKTOP, VK_CTRL ) & 0x8000 )
        aState.mnState |= KEY_MOD1;
    if ( WinGetKeyState( HWND_DESKTOP, VK_ALT ) & 0x8000 )
        aState.mnState |= KEY_MOD2;

    POINTL pt;
    _WinQueryPointerPos( HWND_DESKTOP, &pt );

    aState.maPos = Point( pt.x - maGeometry.nX, pt.y - maGeometry.nY );
    return aState;
}

// -----------------------------------------------------------------------

void Os2SalFrame::SetBackgroundBitmap( SalBitmap* )
{
}

// -----------------------------------------------------------------------

void SalTestMouseLeave()
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

static long ImplHandleMouseMsg( HWND hWnd,
                                UINT nMsg, MPARAM nMP1, MPARAM nMP2 )
{
    SalMouseEvent   aMouseEvt;
    long            nRet;
    USHORT          nEvent;
    sal_Bool            bCall = TRUE;
    USHORT          nFlags = SHORT2FROMMP( nMP2 );
    Os2SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    aMouseEvt.mnX       = (short)SHORT1FROMMP( nMP1 );
    aMouseEvt.mnY       = pFrame->mnHeight - (short)SHORT2FROMMP( nMP1 ) - 1;
    aMouseEvt.mnCode    = 0;
    aMouseEvt.mnTime    = WinQueryMsgTime( pFrame->mhAB );

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
                                 pFrame->mhWndClient,
                                 WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE ) )
                {
                    if ( (aTempMsg.msg == WM_MOUSEMOVE) &&
                         (aTempMsg.mp2 == nMP2) )
                        return 1;
                }
            }

            // Test for MouseLeave
            if ( pSalData->mhWantLeaveMsg &&
                (pSalData->mhWantLeaveMsg != pFrame->mhWndClient) )
            {
                POINTL aMousePoint;
                WinQueryMsgPos( pFrame->mhAB, &aMousePoint );
                WinSendMsg( pSalData->mhWantLeaveMsg,
                            SAL_MSG_MOUSELEAVE,
                            0, MPFROM2SHORT( aMousePoint.x, aMousePoint.y ) );
            }
            pSalData->mhWantLeaveMsg = pFrame->mhWndClient;
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
            if ( pSalData->mhWantLeaveMsg == pFrame->mhWndClient )
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
                WinMapWindowPoints( HWND_DESKTOP, pFrame->mhWndClient, &aPt, 1 );
                aPt.y = pFrame->mnHeight - aPt.y - 1;
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
            break;

        case WM_BUTTON2DBLCLK:
        case WM_BUTTON2DOWN:
            aMouseEvt.mnButton = MOUSE_RIGHT;
            nEvent = SALEVENT_MOUSEBUTTONDOWN;
            break;

        case WM_BUTTON3DBLCLK:
        case WM_BUTTON3DOWN:
            aMouseEvt.mnButton = MOUSE_MIDDLE;
            nEvent = SALEVENT_MOUSEBUTTONDOWN;
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

    // check if this window was destroyed - this might happen if we are the help window
    // and sent a mouse leave message to the application which killed the help window, ie ourself
    if( !WinIsWindow( pFrame->mhAB, hWnd ) )
        return 0;

#if OSL_DEBUG_LEVEL>10
    //if (_bCapture)
        debug_printf("ImplHandleMouseMsg mouse %d,%d\n",aMouseEvt.mnX,aMouseEvt.mnY);
#endif

    if ( bCall )
    {
        if ( nEvent == SALEVENT_MOUSEBUTTONDOWN )
            WinUpdateWindow( pFrame->mhWndClient );

        // --- RTL --- (mirror mouse pos)
        //if( Application::GetSettings().GetLayoutRTL() )
        //    aMouseEvt.mnX = pFrame->maGeometry.nWidth-1-aMouseEvt.mnX;

        nRet = pFrame->CallCallback( nEvent, &aMouseEvt );
        if ( nMsg == WM_MOUSEMOVE )
        {
            WinSetPointer( HWND_DESKTOP, pFrame->mhPointer );
            nRet = TRUE;
        }
    }
    else
        nRet = 0;

    return nRet;
}

// -----------------------------------------------------------------------

static long ImplHandleWheelMsg( HWND hWnd, UINT nMsg, MPARAM nMP1, MPARAM nMP2 )
{

    ImplSalYieldMutexAcquireWithWait();

    long        nRet = 0;
    Os2SalFrame*   pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {

        // Mouse-Coordinaates are relativ to the screen
        POINTL aPt;
        WinQueryMsgPos( pFrame->mhAB, &aPt );
        WinMapWindowPoints( HWND_DESKTOP, pFrame->mhWndClient, &aPt, 1 );
        aPt.y = pFrame->mnHeight - aPt.y - 1;

        SalWheelMouseEvent aWheelEvt;
        aWheelEvt.mnTime            = WinQueryMsgTime( pFrame->mhAB );
        aWheelEvt.mnX               = aPt.x;
        aWheelEvt.mnY               = aPt.y;
        aWheelEvt.mnCode            = 0;
        bool bNeg = (SHORT2FROMMP(nMP2) == SB_LINEDOWN || SHORT2FROMMP(nMP2) == SB_PAGEDOWN );
        aWheelEvt.mnDelta           = bNeg ? -120 : 120;
        aWheelEvt.mnNotchDelta      = bNeg ? -1 : 1;
        if (SHORT2FROMMP(nMP2) == SB_PAGEUP || SHORT2FROMMP(nMP2) == SB_PAGEDOWN)
            aWheelEvt.mnScrollLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
        else
            aWheelEvt.mnScrollLines = 1;

        if( nMsg == WM_HSCROLL )
            aWheelEvt.mbHorz        = TRUE;

        // Modifier-Tasten setzen
        if ( WinGetKeyState( HWND_DESKTOP, VK_SHIFT ) & 0x8000 )
            aWheelEvt.mnCode |= KEY_SHIFT;
        if ( WinGetKeyState( HWND_DESKTOP, VK_CTRL ) & 0x8000 )
            aWheelEvt.mnCode |= KEY_MOD1;
        if ( WinGetKeyState( HWND_DESKTOP, VK_ALT ) & 0x8000 )
            aWheelEvt.mnCode |= KEY_MOD2;

        nRet = pFrame->CallCallback( SALEVENT_WHEELMOUSE, &aWheelEvt );
    }

    ImplSalYieldMutexRelease();

    return nRet;
}


// -----------------------------------------------------------------------

static USHORT ImplSalGetKeyCode( Os2SalFrame* pFrame, MPARAM aMP1, MPARAM aMP2 )
{
    USHORT  nKeyFlags   = SHORT1FROMMP( aMP1 );
    UCHAR   nCharCode   = (UCHAR)SHORT1FROMMP( aMP2 );
    USHORT  nKeyCode    = (UCHAR)SHORT2FROMMP( aMP2 );
    UCHAR   nScanCode   = (UCHAR)CHAR4FROMMP( aMP1 );
    USHORT  rSVCode = 0;

    // Ist virtueller KeyCode gesetzt und befindet sich der KeyCode in der
    // Tabelle, dann mappen
    if ( (nKeyFlags & KC_VIRTUALKEY) && (nKeyCode < KEY_TAB_SIZE) )
        rSVCode = aImplTranslateKeyTab[nKeyCode];

    // Wenn kein KeyCode ermittelt werden konnte, versuchen wir aus dem
    // CharCode einen zu erzeugen
    if ( !rSVCode && nCharCode )
    {
        // Bei 0-9, a-z und A-Z auch KeyCode setzen
        if ( (nCharCode >= '0') && (nCharCode <= '9') && (!rSVCode || !(nKeyFlags & KC_SHIFT)) )
            rSVCode = KEYGROUP_NUM + (nCharCode-'0');
        else if ( (nCharCode >= 'a') && (nCharCode <= 'z') )
            rSVCode = KEYGROUP_ALPHA + (nCharCode-'a');
        else if ( (nCharCode >= 'A') && (nCharCode <= 'Z') )
            rSVCode = KEYGROUP_ALPHA + (nCharCode-'A');
        else
        {
            switch ( nCharCode )
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
    if ( (nCharCode >= '0') && (nCharCode <= '9') && !(nKeyFlags & KC_SHIFT) )
        rSVCode = KEYGROUP_NUM + (nCharCode-'0');
    if ( nCharCode == ',' )
        rSVCode = KEY_COMMA;
    if ( nCharCode == '.' )
        rSVCode = KEY_POINT;

    return rSVCode;
}

// -----------------------------------------------------------------------

static void ImplUpdateInputLang( Os2SalFrame* pFrame )
{
    sal_Bool    bLanguageChange = FALSE;
    ULONG   nLang = 0;
    APIRET  rc;
    UconvObject  uconv_object = NULL;
    LocaleObject locale_object = NULL;
    UniChar     *pinfo_item;

    // we do not support change of input language while working,
    // so exit if already defined (mnInputLang is a static class field)
    if (pFrame->mnInputLang)
        return;

    // get current locale
    rc = UniCreateLocaleObject(UNI_UCS_STRING_POINTER, (UniChar *)L"", &locale_object);
    // get Win32 locale id and sublanguage (hex uni string)
    rc = UniQueryLocaleItem(locale_object, LOCI_xWinLocale, &pinfo_item);
    // convert uni string to integer
    rc = UniStrtoul(locale_object, pinfo_item, &pinfo_item, 16, &nLang);
    rc = UniFreeMem(pinfo_item);
#if OSL_DEBUG_LEVEL>10
    debug_printf("ImplUpdateInputLang nLang %04x\n", nLang);
    char         char_buffer[256];
    rc = UniCreateUconvObject((UniChar *)L"", &uconv_object);
    rc = UniQueryLocaleItem(locale_object, LOCI_sKeyboard, &pinfo_item);
    rc = UniStrFromUcs(uconv_object, char_buffer, pinfo_item, sizeof(char_buffer));
    debug_printf("Keyboard name is: %s\n", char_buffer );
    rc = UniFreeMem(pinfo_item);
#endif
    rc = UniFreeLocaleObject(locale_object);

    // keep input lang up-to-date
#if OSL_DEBUG_LEVEL>10
    debug_printf("ImplUpdateInputLang pFrame %08x lang changed from %d to %d\n",
        pFrame, pFrame->mnInputLang, nLang);
#endif
    pFrame->mnInputLang = nLang;
}


static sal_Unicode ImplGetCharCode( Os2SalFrame* pFrame, USHORT nKeyFlags,
                                    sal_Char nCharCode, UCHAR nScanCode )
{
    ImplUpdateInputLang( pFrame );
#if OSL_DEBUG_LEVEL>10
    debug_printf("ImplGetCharCode nCharCode %c, %04x\n", nCharCode, nCharCode);
#endif
    return OUString( &nCharCode, 1, gsl_getSystemTextEncoding()).toChar();
}

// -----------------------------------------------------------------------

LanguageType Os2SalFrame::GetInputLanguage()
{
    if( !mnInputLang )
        ImplUpdateInputLang( this );

    if( !mnInputLang )
        return LANGUAGE_DONTKNOW;
    else
        return (LanguageType) mnInputLang;
}

// -----------------------------------------------------------------------

sal_Bool Os2SalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , KeyCode& )
{
    // not supported yet
    return FALSE;
}

// -----------------------------------------------------------------------

static sal_Unicode ImplConvertKey( Os2SalFrame* pFrame, MPARAM aMP1, MPARAM aMP2 )
{
    USHORT  nKeyFlags   = SHORT1FROMMP( aMP1 );
    UCHAR   nCharCode   = (UCHAR)SHORT1FROMMP( aMP2 );
    USHORT  nKeyCode    = (UCHAR)SHORT2FROMMP( aMP2 );
    UCHAR   nScanCode   = (UCHAR)CHAR4FROMMP( aMP1 );
    sal_Unicode rSVCharCode = 0;

    // Ist Character-Code gesetzt
    // !!! Bei CTRL/ALT ist KC_CHAR nicht gesetzt, jedoch moechten wir
    // !!! dann auch einen CharCode und machen die Behandlung deshalb
    // !!! selber
    if ( (nKeyFlags & KC_CHAR) || (nKeyFlags & KC_CTRL) || (nKeyFlags & KC_ALT) )
        rSVCharCode = ImplGetCharCode( pFrame, nKeyFlags, nCharCode, nScanCode);

    // ret unicode
    return rSVCharCode;
}

// -----------------------------------------------------------------------

static long ImplHandleKeyMsg( HWND hWnd,
                              UINT nMsg, MPARAM nMP1, MPARAM nMP2 )
{
    static USHORT   nLastOS2KeyChar = 0;
    static sal_Unicode   nLastChar       = 0;
    USHORT          nRepeat         = CHAR3FROMMP( nMP1 ) - 1;
    SHORT           nFlags          = SHORT1FROMMP( nMP1 );
    USHORT          nModCode        = 0;
    USHORT          nSVCode         = 0;
    USHORT          nOS2KeyCode     = (UCHAR)SHORT2FROMMP( nMP2 );
    sal_Unicode     nSVCharCode     = 0;
    long            nRet            = 0;

    Os2SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( !pFrame )
        return 0;

    // determine modifiers
    if ( nFlags & KC_SHIFT )
        nModCode |= KEY_SHIFT;
    if ( nFlags & KC_CTRL )
        nModCode |= KEY_MOD1;
    if ( nFlags & KC_ALT )
        nModCode |= KEY_MOD2;

    // Bei Shift, Control und Alt schicken wir einen KeyModChange-Event
    if ( (nOS2KeyCode == VK_SHIFT) || (nOS2KeyCode == VK_CTRL) ||
         (nOS2KeyCode == VK_ALT) || (nOS2KeyCode == VK_ALTGRAF) )
    {
        SalKeyModEvent aModEvt;
        aModEvt.mnTime = WinQueryMsgTime( pFrame->mhAB );
        aModEvt.mnCode = nModCode;
#if OSL_DEBUG_LEVEL>10
        debug_printf("SALEVENT_KEYMODCHANGE\n");
#endif
        nRet = pFrame->CallCallback( SALEVENT_KEYMODCHANGE, &aModEvt );
    }
    else
    {
        nSVCode = ImplSalGetKeyCode( pFrame, nMP1, nMP2 );
        nSVCharCode = ImplConvertKey( pFrame, nMP1, nMP2 );
#if OSL_DEBUG_LEVEL>10
        debug_printf("nSVCode %04x nSVCharCode %04x\n",nSVCode,nSVCharCode );
#endif

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
            aKeyEvt.mnTime      = WinQueryMsgTime( pFrame->mhAB );
            aKeyEvt.mnCode     |= nModCode;
            aKeyEvt.mnCharCode  = nSVCharCode;
            aKeyEvt.mnRepeat    = nRepeat;

#if OSL_DEBUG_LEVEL>10
            debug_printf( (nFlags & KC_KEYUP) ? "SALEVENT_KEYUP\n" : "SALEVENT_KEYINPUT\n");
#endif
            nRet = pFrame->CallCallback( (nFlags & KC_KEYUP) ? SALEVENT_KEYUP : SALEVENT_KEYINPUT,
                                               &aKeyEvt );
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

static bool ImplHandlePaintMsg( HWND hWnd )
{
    sal_Bool bMutex = FALSE;

    if ( ImplSalYieldMutexTryToAcquire() )
        bMutex = TRUE;

    // if we don't get the mutex, we can also change the clip region,
    // because other threads doesn't use the mutex from the main
    // thread --> see GetGraphics()

    Os2SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        // Laut Window-Doku soll man erst abfragen, ob ueberhaupt eine
        // Paint-Region anliegt
        if ( WinQueryUpdateRect( hWnd, NULL ) )
        {
            // Call BeginPaint/EndPaint to query the rect and send
            // this Notofication to rect
            HPS     hPS;
            RECTL   aUpdateRect;
            hPS = WinBeginPaint( hWnd, NULLHANDLE, &aUpdateRect );
            WinEndPaint( hPS );

            // Paint
            if ( bMutex )
            {
        SalPaintEvent aPEvt( aUpdateRect.xLeft, pFrame->mnHeight - aUpdateRect.yTop, aUpdateRect.xRight- aUpdateRect.xLeft, aUpdateRect.yTop - aUpdateRect.yBottom );

                pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
            }
            else
            {
                RECTL* pRect = new RECTL;
                WinCopyRect( pFrame->mhAB, pRect, &aUpdateRect );
                WinPostMsg( hWnd, SAL_MSG_POSTPAINT, (MPARAM)pRect, 0 );
            }
        }
    }

    if ( bMutex )
        ImplSalYieldMutexRelease();

    return bMutex ? true : false;
}

// -----------------------------------------------------------------------

static void ImplHandlePaintMsg2( HWND hWnd, RECTL* pRect )
{
    // Paint
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        Os2SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            SalPaintEvent aPEvt( pRect->xLeft, pFrame->mnHeight - pRect->yTop, pRect->xRight - pRect->xLeft, pRect->yTop - pRect->yBottom );
            pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
        }
        ImplSalYieldMutexRelease();
        delete pRect;
    }
    else
        WinPostMsg( hWnd, SAL_MSG_POSTPAINT, (MPARAM)pRect, 0 );
}

// -----------------------------------------------------------------------

static void SetMaximizedFrameGeometry( HWND hWnd, Os2SalFrame* pFrame )
{
    // calculate and set frame geometry of a maximized window - useful if the window is still hidden

    RECTL aRect;
    pFrame->GetWorkArea( aRect);

    // a maximized window has no other borders than the caption
    pFrame->maGeometry.nLeftDecoration = pFrame->maGeometry.nRightDecoration = pFrame->maGeometry.nBottomDecoration = 0;
    pFrame->maGeometry.nTopDecoration = pFrame->mbCaption ? WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR ) : 0;

    aRect.yTop += pFrame->maGeometry.nTopDecoration;
    pFrame->maGeometry.nX = aRect.xLeft;
    pFrame->maGeometry.nY = aRect.yBottom;
    pFrame->maGeometry.nWidth = aRect.xRight - aRect.xLeft + 1;
    pFrame->maGeometry.nHeight = aRect.yBottom - aRect.yTop + 1;
}

static void UpdateFrameGeometry( HWND hWnd, Os2SalFrame* pFrame )
{
    if( !pFrame )
        return;

    //SalFrame has a
    //maGeometry member that holds absolute screen positions (and needs to be
    //updated if the window is moved by the way).

    // reset data
    memset(&pFrame->maGeometry, 0, sizeof(SalFrameGeometry) );

    SWP swp;
    LONG nFrameX, nFrameY, nCaptionY;

    // get frame size
    WinQueryWindowPos(pFrame->mhWndFrame, &swp);
    if (swp.fl & SWP_MINIMIZE)
      return;

    // map from client area to screen
    ImplSalCalcFrameSize( pFrame, nFrameX, nFrameY, nCaptionY);
    pFrame->maGeometry.nTopDecoration = nFrameY + nCaptionY;
    pFrame->maGeometry.nLeftDecoration = nFrameX;
    pFrame->maGeometry.nRightDecoration = nFrameX;
    pFrame->maGeometry.nBottomDecoration = nFrameY;

    // position of client area, not of frame corner!
    pFrame->maGeometry.nX = swp.x + nFrameX;
    pFrame->maGeometry.nY = nScreenHeight - (swp.y + swp.cy) + nFrameY + nCaptionY;

    int nWidth  = swp.cx - pFrame->maGeometry.nRightDecoration - pFrame->maGeometry.nLeftDecoration;
    int nHeight = swp.cy - pFrame->maGeometry.nBottomDecoration - pFrame->maGeometry.nTopDecoration;

    // clamp to zero
    pFrame->maGeometry.nHeight = nHeight < 0 ? 0 : nHeight;
    pFrame->maGeometry.nWidth = nWidth < 0 ? 0 : nWidth;
#if OSL_DEBUG_LEVEL>0
    debug_printf( "UpdateFrameGeometry: hwnd %x, frame %x at %d,%d (%dx%d)\n",
        hWnd, pFrame->mhWndFrame,
        pFrame->maGeometry.nX, pFrame->maGeometry.nY,
        pFrame->maGeometry.nWidth,pFrame->maGeometry.nHeight);
#endif
}

// -----------------------------------------------------------------------

static void ImplHandleMoveMsg( HWND hWnd)
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        Os2SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            UpdateFrameGeometry( hWnd, pFrame );

            if ( WinIsWindowVisible( hWnd ))
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
            //ImplCallMoveHdl( hWnd );

        }

        ImplSalYieldMutexRelease();
    }
    else
        WinPostMsg( hWnd, SAL_MSG_POSTMOVE, 0, 0 );
}

// -----------------------------------------------------------------------

static void ImplHandleSizeMsg( HWND hWnd, MPARAM nMP2 )
{
        Os2SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            UpdateFrameGeometry( hWnd, pFrame );
            pFrame->mbDefPos = FALSE;
            pFrame->mnWidth  = (short)SHORT1FROMMP( nMP2 );
            pFrame->mnHeight = (short)SHORT2FROMMP( nMP2 );
            if ( pFrame->mpGraphics )
                pFrame->mpGraphics->mnHeight = (int)SHORT2FROMMP(nMP2);
            // Status merken
            ImplSaveFrameState( pFrame );
            pFrame->CallCallback( SALEVENT_RESIZE, 0 );
            if ( WinIsWindowVisible( pFrame->mhWndFrame ) && !pFrame->mbInShow )
                WinUpdateWindow( pFrame->mhWndClient );
        }
}

// -----------------------------------------------------------------------

static long ImplHandleFocusMsg( Os2SalFrame* pFrame, MPARAM nMP2 )
{
if ( pFrame && !Os2SalFrame::mbInReparent )
{
    if ( SHORT1FROMMP( nMP2 ) )
    {
        if ( WinIsWindowVisible( pFrame->mhWndFrame ) && !pFrame->mbInShow )
            WinUpdateWindow( pFrame->mhWndClient );
        return pFrame->CallCallback( SALEVENT_GETFOCUS, 0 );
    }
    else
    {
        return pFrame->CallCallback( SALEVENT_LOSEFOCUS, 0 );
    }
}
}

// -----------------------------------------------------------------------

static void ImplHandleCloseMsg( HWND hWnd )
{
    if ( ImplSalYieldMutexTryToAcquire() )
    {
        Os2SalFrame* pFrame = GetWindowPtr( hWnd );
        if ( pFrame )
        {
            pFrame->CallCallback( SALEVENT_CLOSE, 0 );
        }

        ImplSalYieldMutexRelease();
    }
    else
        WinPostMsg( hWnd, WM_CLOSE, 0, 0 );
}

// -----------------------------------------------------------------------

inline void ImplHandleUserEvent( HWND hWnd, MPARAM nMP2 )
{
    ImplSalYieldMutexAcquireWithWait();
    Os2SalFrame* pFrame = GetWindowPtr( hWnd );
    if ( pFrame )
    {
        pFrame->CallCallback( SALEVENT_USEREVENT, (void*)nMP2 );
    }
    ImplSalYieldMutexRelease();
}

// -----------------------------------------------------------------------

static int SalImplHandleProcessMenu( Os2SalFrame* pFrame, ULONG nMsg, MPARAM nMP1, MPARAM nMP2)
{
    long nRet = 0;
debug_printf("SalImplHandleProcessMenu\n");
#if 0
    DWORD err=0;
    if( !HIWORD(wParam) )
    {
        // Menu command
        WORD nId = LOWORD(wParam);
        if( nId )   // zero for separators
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
#endif
    //return (nRet != 0);
    return (nRet == 0);
}

// -----------------------------------------------------------------------

static void ImplHandleInputLangChange( HWND hWnd )
{
    ImplSalYieldMutexAcquireWithWait();

    // Feststellen, ob wir IME unterstuetzen
    Os2SalFrame* pFrame = GetWindowPtr( hWnd );
#if 0
    if ( pFrame && pFrame->mbIME && pFrame->mhDefIMEContext )
    {
        HWND    hWnd = pFrame->mhWnd;
        HKL     hKL = (HKL)lParam;
        UINT    nImeProps = ImmGetProperty( hKL, IGP_PROPERTY );

        pFrame->mbSpezIME = (nImeProps & IME_PROP_SPECIAL_UI) != 0;
        pFrame->mbAtCursorIME = (nImeProps & IME_PROP_AT_CARET) != 0;
        pFrame->mbHandleIME = !pFrame->mbSpezIME;
    }
#endif

    // trigger input language and codepage update
    UINT nLang = pFrame->mnInputLang;
    ImplUpdateInputLang( pFrame );
    debug_printf("ImplHandleInputLangChange new language 0x%04x\n",pFrame->mnInputLang);

    // notify change
    if( nLang != pFrame->mnInputLang )
        pFrame->CallCallback( SALEVENT_INPUTLANGUAGECHANGE, 0 );

    ImplSalYieldMutexRelease();
}

// -----------------------------------------------------------------------

#ifdef ENABLE_IME

static long ImplHandleIMEStartConversion( Os2SalFrame* pFrame )
{
    long        nRet = FALSE;
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND hWnd = pFrame->mhWndClient;
        HIMI hIMI = 0;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            ULONG nProp;
            if ( 0 != pIMEData->mpQueryIMEProperty( hIMI, QIP_PROPERTY, &nProp ) )
                pFrame->mbHandleIME = FALSE;
            else
            {
                pFrame->mbHandleIME = !(nProp & PRP_SPECIALUI);

            }
            if ( pFrame->mbHandleIME )
            {
/* Windows-Code, der noch nicht angepasst wurde !!!
                // Cursor-Position ermitteln und aus der die Default-Position fuer
                // das Composition-Fenster berechnen
                SalCursorPosEvent aCursorPosEvt;
                pFrame->CallCallback( pFrame->mpInst, pFrame,
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

                pFrame->mbConversionMode = TRUE;
                pFrame->CallCallback( SALEVENT_STARTEXTTEXTINPUT, (void*)NULL );
                nRet = TRUE;
            }

            pIMEData->mpReleaseIME( hWnd, hIMI );
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

static long ImplHandleIMEConversion( Os2SalFrame* pFrame, MPARAM nMP2Param )
{
    long        nRet = FALSE;
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND        hWnd = pFrame->mhWndClient;
        HIMI        hIMI = 0;
        ULONG    nMP2 = (ULONG)nMP2Param;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            if ( nMP2 & (IMR_RESULT_RESULTSTRING |
                         IMR_CONV_CONVERSIONSTRING | IMR_CONV_CONVERSIONATTR |
                         IMR_CONV_CURSORPOS | IMR_CONV_CURSORATTR) )
            {
                SalExtTextInputEvent aEvt;
                aEvt.mnTime             = WinQueryMsgTime( pFrame->mhAB );
                aEvt.mpTextAttr         = NULL;
                aEvt.mnCursorPos        = 0;
                aEvt.mnDeltaStart       = 0;
                aEvt.mbOnlyCursor       = FALSE;
                aEvt.mbCursorVisible    = TRUE;

                ULONG    nBufLen = 0;
                xub_Unicode*     pBuf = NULL;
                ULONG    nAttrBufLen = 0;
                PM_BYTE*    pAttrBuf = NULL;
                sal_Bool        bLastCursor = FALSE;
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
                    ULONG nTempBufLen;
                    ULONG nCursorPos = 0;
                    ULONG nCursorAttr = 0;
                    ULONG nChangePos = 0;
                    nTempBufLen = sizeof( ULONG );
                    pIMEData->mpGetConversionString( hIMI, IMR_CONV_CURSORPOS, &nCursorPos, &nTempBufLen );
                    nTempBufLen = sizeof( ULONG );
                    pIMEData->mpGetConversionString( hIMI, IMR_CONV_CURSORATTR, &nCursorAttr, &nTempBufLen );
                    nTempBufLen = sizeof( ULONG );
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
                    delete [] pBuf;
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
                        delete [] pAttrBuf;
                    }
                    if ( bLastCursor )
                        aEvt.mnCursorPos = aEvt.maText.Len();
                }

                pIMEData->mpReleaseIME( hWnd, hIMI );

                // Handler rufen und wenn wir ein Attribute-Array haben, danach
                // wieder zerstoeren
                pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEvt );
                if ( pSalAttrAry )
                    delete [] pSalAttrAry;
            }
            else
                pIMEData->mpReleaseIME( hWnd, hIMI );
        }

        nRet = TRUE;
    }

    return nRet;
}

// -----------------------------------------------------------------------

inline long ImplHandleIMEEndConversion( Os2SalFrame* pFrame )
{
    pFrame->mbConversionMode = FALSE;
    pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
    return TRUE;
}

// -----------------------------------------------------------------------

static void ImplHandleIMEOpenCandidate( Os2SalFrame* pFrame )
{
    pFrame->mbCandidateMode = TRUE;

    long        nRet = FALSE;
    SalIMEData* pIMEData = GetSalIMEData();
    if ( pIMEData )
    {
        HWND        hWnd = pFrame->mhWndClient;
        HIMI        hIMI = 0;
        pIMEData->mpGetIME( hWnd, &hIMI );
        if ( hIMI )
        {
            ULONG nBufLen = 0;
            pIMEData->mpGetConversionString( hIMI, IMR_CONV_CONVERSIONSTRING, 0, &nBufLen );
            if ( nBufLen > 0 )
            {
/* !!! Wir bekommen derzeit nur falsche Daten steht der Cursor immer bei 0
                ULONG nTempBufLen = sizeof( ULONG );
                ULONG nCursorPos = 0;
                pIMEData->mpGetConversionString( hIMI, IMR_CONV_CURSORPOS, &nCursorPos, &nTempBufLen );
*/
                ULONG nCursorPos = 0;

                SalExtTextInputPosEvent aEvt;
                aEvt.mnTime         = WinQueryMsgTime( pFrame->mhAB );
                aEvt.mnFirstPos     = nCursorPos;
                aEvt.mnChars        = nBufLen-nCursorPos;
                aEvt.mpPosAry       = new SalExtCharPos[aEvt.mnChars];
                memset( aEvt.mpPosAry, 0, aEvt.mnChars*sizeof(SalExtCharPos) );

                pFrame->CallCallback( SALEVENT_EXTTEXTINPUTPOS, (void*)&aEvt );

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
                aForm.ptCurrentPos.y    = pFrame->mnHeight - (nMaxBottom+1) - 1;
                aForm.rcArea.xLeft      = nMinLeft;
                aForm.rcArea.yBottom    = pFrame->mnHeight - nMaxBottom - 1;
                aForm.rcArea.xRight     = nMaxRight+1;
                aForm.rcArea.yTop       = pFrame->mnHeight - nMinTop - 1;
                pIMEData->mpSetCandidateWin( hIMI, &aForm );

                delete aEvt.mpPosAry;
            }

            pIMEData->mpReleaseIME( hWnd, hIMI );
        }
    }
}

// -----------------------------------------------------------------------

inline void ImplHandleIMECloseCandidate( Os2SalFrame* pFrame )
{
    pFrame->mbCandidateMode = FALSE;
}

#endif

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalFrameWndProc( HWND hWnd, ULONG nMsg,
                                  MPARAM nMP1, MPARAM nMP2 )
{
    Os2SalFrame*    pFrame      = (Os2SalFrame*)GetWindowPtr( hWnd );
    MRESULT         nRet        = (MRESULT)0;
    sal_Bool            bDef        = TRUE;
    bool            bCheckTimers= false;

#if OSL_DEBUG_LEVEL>10
    if (nMsg!=WM_TIMER && nMsg!=WM_MOUSEMOVE)
        debug_printf( "SalFrameWndProc hWnd 0x%x nMsg 0x%x\n", hWnd, nMsg);
#endif

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
            ImplSalYieldMutexAcquireWithWait();
            bDef = !ImplHandleMouseMsg( hWnd, nMsg, nMP1, nMP2 );
            ImplSalYieldMutexRelease();
            break;

        case WM_CHAR:
            if ( pFrame->mbConversionMode )
                bDef = FALSE;
            else
                bDef = !ImplHandleKeyMsg( hWnd, nMsg, nMP1, nMP2 );
            break;

        case WM_ERASEBACKGROUND:
            nRet = (MRESULT)FALSE;
            bDef = FALSE;
            break;

        case WM_PAINT:
            bCheckTimers = ImplHandlePaintMsg( hWnd );
            bDef = FALSE;
            break;
        case SAL_MSG_POSTPAINT:
            ImplHandlePaintMsg2( hWnd, (RECTL*)nMP1 );
            bCheckTimers = true;
            bDef = FALSE;
            break;

        case WM_MOVE:
        case SAL_MSG_POSTMOVE:
            ImplHandleMoveMsg( hWnd );
            bDef = FALSE;
            break;

        case WM_SIZE:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                ImplHandleSizeMsg( hWnd, nMP2 );
                ImplSalYieldMutexRelease();
            }
            else
                WinPostMsg( hWnd, SAL_MSG_POSTSIZE, nMP1, nMP2 );
            break;
        case SAL_MSG_POSTSIZE:
            ImplHandleSizeMsg( hWnd, nMP2 );
            break;
        case WM_MINMAXFRAME:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                PSWP pswp = (PSWP) nMP1;
                ImplHandleSizeMsg( hWnd, MPFROM2SHORT( pswp->cx, pswp->cy) );
                ImplSalYieldMutexRelease();
            }
            else
                WinPostMsg( hWnd, SAL_MSG_POSTSIZE, 0, nMP2 );
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
                HWND hWndFrame = WinQueryWindow(hWnd, QW_PARENT);
                if (hWndFrame == 0)
                    debug_printf(" WARNING NULL FRAME!!\n");
                SalData* pSalData = GetSalData();
                // Window-Instanz am Windowhandle speichern
                pFrame = pSalData->mpCreateFrame;
                pSalData->mpCreateFrame = NULL;
                SetWindowPtr( hWnd, pFrame );
                SetWindowPtr( hWndFrame, pFrame);
                // HWND schon hier setzen, da schon auf den Instanzdaten
                // gearbeitet werden kann, wenn Messages waehrend
                // CreateWindow() gesendet werden
                pFrame->mhWndClient = hWnd;
                pFrame->mhWndFrame = hWndFrame;
                pFrame->maSysData.hWnd = hWnd;
            }
            break;

        case WM_CLOSE:
            ImplHandleCloseMsg( hWnd );
            bDef = FALSE;
            break;

        case WM_SYSVALUECHANGED:
            if ( pFrame->mbFullScreen )
                ImplSalFrameFullScreenPos( pFrame );
            // kein break, da der Rest auch noch verarbeitet werden soll
        case PL_ALTERED:
        case WM_SYSCOLORCHANGE:
            ImplSalYieldMutexAcquire();
            pFrame->CallCallback( SALEVENT_SETTINGSCHANGED, 0 );
            ImplSalYieldMutexRelease();
            break;

        case SAL_MSG_USEREVENT:
            ImplHandleUserEvent( hWnd, nMP2 );
            bDef = FALSE;
            break;
        case SAL_MSG_TOTOP:
            ImplSalToTop( hWnd, (ULONG)nMP1 );
            bDef = FALSE;
            break;
        case SAL_MSG_SHOW:
            ImplSalShow( hWnd, (ULONG)nMP1, (ULONG)nMP2 );
            bDef = FALSE;
            break;

        case WM_KBDLAYERCHANGED:
            debug_printf("hWnd 0x%08x WM_KBDLAYERCHANGED\n", hWnd);
            ImplHandleInputLangChange( hWnd );
            break;

        case WM_HSCROLL:
        case WM_VSCROLL:
            ImplHandleWheelMsg( hWnd, nMsg, nMP1, nMP2 );
            bDef = FALSE;
            break;

        case WM_COMMAND:
        case SAL_MSG_SYSPROCESSMENU:
            if ( SalImplHandleProcessMenu( pFrame, nMsg, nMP1, nMP2 ) )
            {
                bDef = FALSE;
                nRet = (MRESULT)1;
            }
            break;

#ifdef ENABLE_IME
        case WM_IMEREQUEST:
            if ( (ULONG)nMP1 == IMR_CONVRESULT )
            {
                if ( pFrame->mbHandleIME )
                {
                    // Nur im Conversionmodus akzeptieren wir den IME-Input
                    if ( pFrame->mbConversionMode )
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
            else if ( (ULONG)nMP1 == IMR_CANDIDATE )
            {
                if ( pFrame->mbHandleIME )
                {
                    ImplSalYieldMutexAcquire();
                    if ( (ULONG)nMP2 & IMR_CANDIDATE_SHOW )
                        ImplHandleIMEOpenCandidate( pFrame );
                    else if ( (ULONG)nMP2 & IMR_CANDIDATE_HIDE )
                        ImplHandleIMECloseCandidate( pFrame );
                    ImplSalYieldMutexRelease();
                }
            }
            break;

        case WM_IMENOTIFY:
            if ( (ULONG)nMP1 == IMN_STARTCONVERSION )
            {
                ImplSalYieldMutexAcquire();
                if ( ImplHandleIMEStartConversion( pFrame ) )
                {
                    bDef = FALSE;
                    nRet = (MRESULT)TRUE;
                }
                ImplSalYieldMutexRelease();
            }
            else if ( (ULONG)nMP1 == IMN_ENDCONVERSION )
            {
                if ( pFrame->mbHandleIME )
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

    if( bCheckTimers )
    {
        SalData* pSalData = GetSalData();
        if( pSalData->mnNextTimerTime )
        {
            ULONG nCurTime;
            DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, (PVOID)&nCurTime, sizeof(ULONG));
            if( pSalData->mnNextTimerTime < nCurTime )
            {
                QMSG aMsg;
                if (!WinPeekMsg( pFrame->mhAB, &aMsg, 0, WM_PAINT, WM_PAINT, PM_NOREMOVE ) )
                    WinPostMsg( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_POSTTIMER, 0, (MPARAM)nCurTime );
            }
        }
    }

    if ( bDef )
        nRet = WinDefWindowProc( hWnd, nMsg, nMP1, nMP2 );

    return nRet;
}

// -----------------------------------------------------------------------

void Os2SalFrame::ResetClipRegion()
{
}

void Os2SalFrame::BeginSetClipRegion( ULONG )
{
}

void Os2SalFrame::UnionClipRegion( long, long, long, long )
{
}

void Os2SalFrame::EndSetClipRegion()
{
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalFrameSubClassWndProc( HWND hWnd, ULONG nMsg,
                                  MPARAM nMP1, MPARAM nMP2 )
{
    MRESULT mReturn = 0L;

    // ticket#124 min size of 132 px is too much
    if (nMsg == WM_QUERYTRACKINFO) {
    PTRACKINFO pti;
    // first, let PM initialize TRACKINFO
    mReturn = aSalShlData.mpFrameProc( hWnd, nMsg, nMP1, nMP2 );
    // now change default min size
    pti = (PTRACKINFO) nMP2;
    pti->ptlMinTrackSize.x = 64L;
    // now return to PM
    return mReturn;
    }

    return aSalShlData.mpFrameProc( hWnd, nMsg, nMP1, nMP2 );
}

// -----------------------------------------------------------------------
