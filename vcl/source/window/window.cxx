/*************************************************************************
 *
 *  $RCSfile: window.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: th $ $Date: 2000-11-06 22:27:10 $
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

#define _SV_WINDOW_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGTYPE_HXX
#include <salgtype.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_WINDATA_HXX
#include <windata.hxx>
#endif
#ifndef _SV_DBGGUI_HXX
#include <dbggui.hxx>
#endif
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_REGION_H
#include <region.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_CURSOR_HXX
#include <cursor.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <syswin.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_HELPWIN_HXX
#include <helpwin.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_DRAG_HXX
#include <drag.hxx>
#endif

#define SYSDATA_ONLY_BASETYPE
#include <sysdata.hxx>

#include <com/sun/star/awt/XWindowPeer.hpp>

#ifdef REMOTE_APPSERVER
#include "rmwindow.hxx"
#include "xevthdl.hxx"
#include "rmevents.hxx"
#include "rmoutdev.hxx"
#endif

#include <unowrap.hxx>

#pragma hdrstop

// =======================================================================

DBG_NAME( Window );

// =======================================================================

#define IMPL_PAINT_PAINT            ((USHORT)0x0001)
#define IMPL_PAINT_PAINTALL         ((USHORT)0x0002)
#define IMPL_PAINT_PAINTALLCHILDS   ((USHORT)0x0004)
#define IMPL_PAINT_PAINTCHILDS      ((USHORT)0x0008)
#define IMPL_PAINT_ERASE            ((USHORT)0x0010)

// -----------------------------------------------------------------------

typedef Window* PWINDOW;

// -----------------------------------------------------------------------

struct ImplCalcToTopData
{
    ImplCalcToTopData*  mpNext;
    Window*             mpWindow;
    Region*             mpInvalidateRegion;
};

// -----------------------------------------------------------------------

#ifdef DBG_UTIL
const char* ImplDbgCheckWindow( const void* pObj )
{
    DBG_TESTSOLARMUTEX();

    const Window* pWindow = (Window*)pObj;

    if ( (pWindow->GetType() < WINDOW_FIRST) || (pWindow->GetType() > WINDOW_LAST) )
        return "Window data overwrite";

    // Fenster-Verkettung ueberpruefen
    Window* pChild = pWindow->mpFirstChild;
    while ( pChild )
    {
        if ( pChild->mpParent != pWindow )
            return "Child-Window-Parent wrong";
        pChild = pChild->mpNext;
    }

    return NULL;
}
#endif

// =======================================================================

static void ImplInitAppFontData( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    long nTextHeight = pWindow->GetTextHeight();
    long nTextWidth = pWindow->GetTextWidth( XubString( RTL_CONSTASCII_USTRINGPARAM( "aemnnxEM" ) ) );
    long nSymHeight = nTextHeight*4;
    // Falls Font zu schmal ist, machen wir die Basis breiter,
    // damit die Dialoge symetrisch aussehen und nicht zu schmal
    // werden. Wenn der Dialog die gleiche breite hat, geben wir
    // noch etwas Spielraum dazu, da etwas mehr Platz besser ist.
    if ( nSymHeight > nTextWidth )
        nTextWidth = nSymHeight;
    else if ( nSymHeight+5 > nTextWidth )
        nTextWidth = nSymHeight+5;
    pSVData->maGDIData.mnAppFontX = nTextWidth * 10 / 8;
    pSVData->maGDIData.mnAppFontY = nTextHeight * 10;

    pSVData->maGDIData.mnRealAppFontX = pSVData->maGDIData.mnAppFontX;
    if ( pSVData->maAppData.mnDialogScaleX )
        pSVData->maGDIData.mnAppFontX += (pSVData->maGDIData.mnAppFontX*pSVData->maAppData.mnDialogScaleX)/100;
}

// -----------------------------------------------------------------------

MouseEvent ImplTranslateMouseEvent( const MouseEvent& rE, Window* pSource, Window* pDest )
{
    Point aPos = pSource->OutputToScreenPixel( rE.GetPosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return MouseEvent( aPos, rE.GetClicks(), rE.GetMode(), rE.GetButtons(), rE.GetModifier() );
}

// =======================================================================

void Window::ImplInitData( WindowType nType )
{
    meOutDevType        = OUTDEV_WINDOW;

    mpWinData           = NULL;         // Extra Window Data, that we dont need for all windows
    mpOverlapData       = NULL;         // Overlap Data
    mpFrameData         = NULL;         // Frame Data
    mpFrame             = NULL;         // Pointer to frame window
    mpSysObj            = NULL;
    mpFrameWindow       = NULL;         // window to top level parent (same as frame window)
    mpOverlapWindow     = NULL;         // first overlap parent
    mpBorderWindow      = NULL;         // Border-Window
    mpClientWindow      = NULL;         // Client-Window of a FrameWindow
    mpParent            = NULL;         // parent (inkl. BorderWindow)
    mpRealParent        = NULL;         // real parent (exkl. BorderWindow)
    mpFirstChild        = NULL;         // first child window
    mpLastChild         = NULL;         // last child window
    mpFirstOverlap      = NULL;         // first overlap window (only set in overlap windows)
    mpLastOverlap       = NULL;         // last overlap window (only set in overlap windows)
    mpPrev              = NULL;         // prev window
    mpNext              = NULL;         // next window
    mpNextOverlap       = NULL;         // next overlap window of frame
    mpLastFocusWindow   = NULL;         // window for focus restore
    mpDlgCtrlDownWindow = NULL;         // window for dialog control
    mpFirstDel          = NULL;         // Dtor notification list
    mpUserData          = NULL;         // user data
    mpCursor            = NULL;         // cursor
    mpControlFont       = NULL;         // font propertie
    mpVCLXWindow        = NULL;
    maControlForeground = Color( COL_TRANSPARENT );     // kein Foreground gesetzt
    maControlBackground = Color( COL_TRANSPARENT );     // kein Background gesetzt
    mnLeftBorder        = 0;            // left border
    mnTopBorder         = 0;            // top border
    mnRightBorder       = 0;            // right border
    mnBottomBorder      = 0;            // bottom border
    mnX                 = 0;            // X-Position to Parent
    mnY                 = 0;            // Y-Position to Parent
    mnHelpId            = 0;            // help id
    mnUniqId            = 0;            // unique id
    mpChildClipRegion   = NULL;         // Child-Clip-Region when ClipChildren
    mpPaintRegion       = NULL;         // Paint-ClipRegion
    mnStyle             = 0;            // style (init in ImplInitWindow)
    mnPrevStyle         = 0;            // prevstyle (set in SetStyle)
    mnExtendedStyle     = 0;            // extended style (init in ImplInitWindow)
    mnPrevExtendedStyle = 0;            // prevstyle (set in SetExtendedStyle)
    mnType              = nType;        // type
    mnGetFocusFlags     = 0;            // Flags fuer GetFocus()-Aufruf
    mnWaitCount         = 0;            // Wait-Count (>1 == Warte-MousePointer)
    mnPaintFlags        = 0;            // Flags for ImplCallPaint
    mnParentClipMode    = 0;            // Flags for Parent-ClipChildren-Mode
    mnActivateMode      = 0;            // Wird bei System/Overlap-Windows umgesetzt
    mnDlgCtrlFlags      = 0;            // DialogControl-Flags
    mbFrame             = FALSE;        // TRUE: Window is a frame window
    mbBorderWin         = FALSE;        // TRUE: Window is a border window
    mbOverlapWin        = FALSE;        // TRUE: Window is a overlap window
    mbSysWin            = FALSE;        // TRUE: SystemWindow is the base class
    mbDialog            = FALSE;        // TRUE: Dialog is the base class
    mbDockWin           = FALSE;        // TRUE: DockingWindow is the base class
    mbFloatWin          = FALSE;        // TRUE: FloatingWindow is the base class
    mbPushButton        = FALSE;        // TRUE: PushButton is the base class
    mbVisible           = FALSE;        // TRUE: Show( TRUE ) called
    mbOverlapVisible    = FALSE;        // TRUE: Hide called for visible window from ImplHideAllOverlapWindow()
    mbDisabled          = FALSE;        // TRUE: Enable( FALSE ) called
    mbInputDisabled     = FALSE;        // TRUE: EnableInput( FALSE ) called
    mbAlwaysEnableInput = FALSE;        // TRUE: AlwaysEnableInput( TRUE ) called
    mbDropDisabled      = FALSE;        // TRUE: Drop is enabled
    mbNoUpdate          = FALSE;        // TRUE: SetUpdateMode( FALSE ) called
    mbNoParentUpdate    = FALSE;        // TRUE: SetParentUpdateMode( FALSE ) called
    mbActive            = FALSE;        // TRUE: Window Active
    mbParentActive      = FALSE;        // TRUE: OverlapActive from Parent
    mbReallyVisible     = FALSE;        // TRUE: this and all parents to an overlaped window are visible
    mbReallyShown       = FALSE;        // TRUE: this and all parents to an overlaped window are shown
    mbInInitShow        = FALSE;        // TRUE: we are in InitShow
    mbChildNotify       = FALSE;        // TRUE: ChildNotify
    mbChildPtrOverwrite = FALSE;        // TRUE: PointerStyle overwrites Child-Pointer
    mbNoPtrVisible      = FALSE;        // TRUE: ShowPointer( FALSE ) called
    mbMouseMove         = FALSE;        // TRUE: BaseMouseMove called
    mbPaintFrame        = FALSE;        // TRUE: Paint is visible, but not painted
    mbInPaint           = FALSE;        // TRUE: Inside PaintHdl
    mbMouseButtonDown   = FALSE;        // TRUE: BaseMouseButtonDown called
    mbMouseButtonUp     = FALSE;        // TRUE: BaseMouseButtonUp called
    mbKeyInput          = FALSE;        // TRUE: BaseKeyInput called
    mbKeyUp             = FALSE;        // TRUE: BaseKeyUp called
    mbCommand           = FALSE;        // TRUE: BaseCommand called
    mbDefPos            = TRUE;         // TRUE: Position is not Set
    mbDefSize           = TRUE;         // TRUE: Size is not Set
    mbCallMove          = TRUE;         // TRUE: Move must be called by Show
    mbCallResize        = TRUE;         // TRUE: Resize must be called by Show
    mbWaitSystemResize  = TRUE;         // TRUE: Wait for System-Resize
    mbInitWinClipRegion = TRUE;         // TRUE: Calc Window Clip Region
    mbInitChildRegion   = FALSE;        // TRUE: InitChildClipRegion
    mbWinRegion         = FALSE;        // TRUE: Window Region
    mbClipChildren      = FALSE;        // TRUE: Child-Fenster muessen evt. geclippt werden
    mbClipSiblings      = FALSE;        // TRUE: Nebeneinanderliegende Child-Fenster muessen evt. geclippt werden
    mbChildTransparent  = FALSE;        // TRUE: Child-Fenster duerfen transparent einschalten (inkl. Parent-CLIPCHILDREN)
    mbPaintTransparent  = FALSE;        // TRUE: Paints muessen auf Parent ausgeloest werden
    mbMouseTransparent  = FALSE;        // TRUE: Window is transparent for Mouse
    mbDlgCtrlStart      = FALSE;        // TRUE: Ab hier eigenes Dialog-Control
    mbFocusVisible      = FALSE;        // TRUE: Focus Visible
    mbTrackVisible      = FALSE;        // TRUE: Tracking Visible
    mbControlForeground = FALSE;        // TRUE: Foreground-Property set
    mbControlBackground = FALSE;        // TRUE: Background-Property set
    mbAlwaysOnTop       = FALSE;        // TRUE: immer vor allen anderen normalen Fenstern sichtbar
    mbCompoundControl   = FALSE;        // TRUE: Zusammengesetztes Control => Listener...
    mbCompoundControlHasFocus = FALSE;  // TRUE: Zusammengesetztes Control hat irgendwo den Focus
    mbPaintDisabled     = FALSE;        // TRUE: Paint soll nicht ausgefuehrt werden
    mbAllResize         = FALSE;        // TRUE: Auch ResizeEvents mit 0,0 schicken
    mbInDtor            = FALSE;        // TRUE: Wir befinden uns im Window-Dtor
    mbExtTextInput      = FALSE;        // TRUE: ExtTextInput-Mode is active
    mbInFocusHdl        = FALSE;        // TRUE: Innerhalb vom GetFocus-Handler
    mbCreatedWithToolkit = FALSE;

#ifdef REMOTE_APPSERVER
    mpRmEvents          = NULL;

    Font aFont = maInputContext.GetFont();
    aFont.SetCharSet( gsl_getSystemTextEncoding() );
    maInputContext.SetFont( aFont );
#endif
}

// -----------------------------------------------------------------------

#ifdef REMOTE_APPSERVER
void Window::ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    static ::com::sun::star::uno::Any aVoid;
    ImplInit( pParent, nStyle, aVoid );
}
#else
void Window::ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken )
{
    ImplInit( pParent, nStyle, NULL );
}
#endif

// -----------------------------------------------------------------------

#ifndef REMOTE_APPSERVER
void Window::ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
#else
void Window::ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken )
#endif
{
    DBG_ASSERT( mbFrame || pParent, "Window::Window(): pParent == NULL" );

    ImplSVData* pSVData = ImplGetSVData();
    Window*     pRealParent = pParent;

    // 3D-Look vererben
    if ( !mbOverlapWin && (pParent->GetStyle() & WB_3DLOOK) )
        nStyle |= WB_3DLOOK;

    // Wenn wir einen Border haben, muessen wir ein BorderWindow anlegen
    if ( !mbFrame && !mbBorderWin && !mpBorderWindow && (nStyle & WB_BORDER) )
    {
        ImplBorderWindow* pBorderWin = new ImplBorderWindow( pParent, nStyle & (WB_BORDER | WB_DIALOGCONTROL | WB_NODIALOGCONTROL) );
        ((Window*)pBorderWin)->mpClientWindow = this;
        pBorderWin->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
        mpBorderWindow  = pBorderWin;
        pParent = mpBorderWindow;
    }

    // insert window in list
    ImplInsertWindow( pParent );
    mnStyle = nStyle;

    // Overlap-Window-Daten
    if ( mbOverlapWin )
    {
        mpOverlapData                   = new ImplOverlapData;
        mpOverlapData->mpSaveBackDev    = NULL;
        mpOverlapData->mpSaveBackRgn    = NULL;
        mpOverlapData->mpNextBackWin    = NULL;
        mpOverlapData->mnSaveBackSize   = 0;
        mpOverlapData->mbSaveBack       = FALSE;
        mpOverlapData->mnTopLevel       = 1;
    }

    // test for frame creation
    if ( mbFrame )
    {
        // create frame
#ifndef REMOTE_APPSERVER
        ULONG nFrameStyle = 0;

        if ( nStyle & WB_MOVEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_MOVEABLE;
        if ( nStyle & WB_SIZEABLE )
        {
            nFrameStyle |= SAL_FRAME_STYLE_SIZEABLE |
                           SAL_FRAME_STYLE_MINABLE |
                           SAL_FRAME_STYLE_MAXABLE;
        }
        if ( nStyle & WB_CLOSEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_CLOSEABLE;
        if ( nStyle & WB_APP )
            nFrameStyle |= SAL_FRAME_STYLE_DEFAULT;

        SalFrame* pParentFrame = NULL;
        if ( pParent )
            pParentFrame = pParent->mpFrame;
        SalFrame* pFrame;
        if ( pSystemParentData )
            pFrame = pSVData->mpDefInst->CreateChildFrame( pSystemParentData, nFrameStyle | SAL_FRAME_STYLE_CHILD );
        else
            pFrame = pSVData->mpDefInst->CreateFrame( pParentFrame, nFrameStyle );
        if ( !pFrame )
            GetpApp()->Exception( EXC_SYSOBJNOTCREATED );
        pFrame->SetCallback( this, ImplWindowFrameProc );

        // initialize system-Drag&Drop-interface
        DragManager::SystemEnableDrop( pFrame, TRUE );
#else
        RmFrameWindow* pParentFrame = pParent ? pParent->mpFrame : NULL;;
        RmFrameWindow* pFrame = new RmFrameWindow( this );
        if ( !pFrame->IsValid() )
        {
            delete pFrame, pFrame = NULL;
            GetpApp()->Exception( EXC_SYSOBJNOTCREATED );
        }
        else
        {
            pFrame->Create( nStyle, pFrame->GetEventHdlInterface(),
                            aSystemWorkWindowToken,
                            pParentFrame ? pParentFrame->GetFrameInterface() : REF( NMSP_CLIENT::XRmFrameWindow )() );
        }
#endif

        // set window frame data
        mpFrameData     = new ImplFrameData;
        mpFrame         = pFrame;
        mpFrameWindow   = this;
        mpOverlapWindow = this;

        // set frame data
        mpFrameData->mpNextFrame        = pSVData->maWinData.mpFirstFrame;
        pSVData->maWinData.mpFirstFrame = this;
        mpFrameData->mpFirstOverlap     = NULL;
        mpFrameData->mpFocusWin         = NULL;
        mpFrameData->mpExtTextInputWin  = NULL;
        mpFrameData->mpMouseMoveWin     = NULL;
        mpFrameData->mpMouseDownWin     = NULL;
        mpFrameData->mpFirstBackWin     = NULL;
        mpFrameData->mpFontList         = pSVData->maGDIData.mpScreenFontList;
        mpFrameData->mpFontCache        = pSVData->maGDIData.mpScreenFontCache;
        mpFrameData->mnAllSaveBackSize  = 0;
        mpFrameData->mnFocusId          = 0;
        mpFrameData->mnMouseMoveId      = 0;
        mpFrameData->mnLastMouseX       = -1;
        mpFrameData->mnLastMouseY       = -1;
        mpFrameData->mnFirstMouseX      = -1;
        mpFrameData->mnFirstMouseY      = -1;
        mpFrameData->mnLastMouseWinX    = -1;
        mpFrameData->mnLastMouseWinY    = -1;
        mpFrameData->mnMouseDownTime    = 0;
        mpFrameData->mnClickCount       = 0;
        mpFrameData->mnFirstMouseCode   = 0;
        mpFrameData->mnMouseCode        = 0;
        mpFrameData->mnMouseMode        = 0;
        mpFrameData->meMapUnit          = MAP_PIXEL;
        mpFrameData->mbHasFocus         = FALSE;
        mpFrameData->mbInMouseMove      = FALSE;
        mpFrameData->mbMouseIn          = FALSE;
        mpFrameData->mbStartDragCalled  = FALSE;
        mpFrameData->mbNeedSysWindow    = FALSE;
        mpFrameData->mbMinimized        = FALSE;
        mpFrameData->mbStartFocusState  = FALSE;
        mpFrameData->mbInSysObjFocusHdl = FALSE;
        mpFrameData->mbInSysObjToTopHdl = FALSE;
        mpFrameData->mbSysObjFocus      = FALSE;

        mpFrameData->mpDragTimer        = NULL;
        mpFrameData->maPaintTimer.SetTimeout( 30 );
        mpFrameData->maPaintTimer.SetTimeoutHdl( LINK( this, Window, ImplHandlePaintHdl ) );

#ifndef REMOTE_APPSERVER
        // Muessen Application-Settings noch upgedatet werden
        if ( !pSVData->maAppData.mbSettingsInit )
        {
            mpFrame->UpdateSettings( *pSVData->maAppData.mpSettings );
            GetpApp()->SystemSettingsChanging( *pSVData->maAppData.mpSettings, this );
#ifdef DBG_UTIL
            // Evt. AppFont auf Fett schalten, damit man feststellen kann,
            // ob fuer die Texte auf anderen Systemen genuegend Platz
            // vorhanden ist
            if ( DbgIsBoldAppFont() )
            {
                StyleSettings aStyleSettings = pSVData->maAppData.mpSettings->GetStyleSettings();
                Font aFont = aStyleSettings.GetAppFont();
                aFont.SetWeight( WEIGHT_BOLD );
                aStyleSettings.SetAppFont( aFont );
                aFont = aStyleSettings.GetGroupFont();
                aFont.SetWeight( WEIGHT_BOLD );
                aStyleSettings.SetGroupFont( aFont );
                aFont = aStyleSettings.GetLabelFont();
                aFont.SetWeight( WEIGHT_BOLD );
                aStyleSettings.SetLabelFont( aFont );
                aFont = aStyleSettings.GetRadioCheckFont();
                aFont.SetWeight( WEIGHT_BOLD );
                aStyleSettings.SetRadioCheckFont( aFont );
                aFont = aStyleSettings.GetPushButtonFont();
                aFont.SetWeight( WEIGHT_BOLD );
                aStyleSettings.SetPushButtonFont( aFont );
                aFont = aStyleSettings.GetFieldFont();
                aFont.SetWeight( WEIGHT_BOLD );
                aStyleSettings.SetFieldFont( aFont );
                aFont = aStyleSettings.GetIconFont();
                aFont.SetWeight( WEIGHT_BOLD );
                aStyleSettings.SetIconFont( aFont );
                pSVData->maAppData.mpSettings->SetStyleSettings( aStyleSettings );
            }
#endif
            OutputDevice::SetSettings( *pSVData->maAppData.mpSettings );
            pSVData->maAppData.mbSettingsInit = TRUE;
        }
#endif
    }

    // init data
    mpRealParent = pRealParent;

    if ( mbFrame )
    {
#ifndef REMOTE_APPSERVER
        if ( pParent )
        {
            mpFrameData->mnDPIX     = pParent->mpFrameData->mnDPIX;
            mpFrameData->mnDPIY     = pParent->mpFrameData->mnDPIY;
            mpFrameData->mnFontDPIX = pParent->mpFrameData->mnFontDPIX;
            mpFrameData->mnFontDPIY = pParent->mpFrameData->mnFontDPIY;
        }
        else
        {
            if ( ImplGetGraphics() )
            {
                mpGraphics->GetResolution( mpFrameData->mnDPIX, mpFrameData->mnDPIY );
                mpGraphics->GetScreenFontResolution( mpFrameData->mnFontDPIX, mpFrameData->mnFontDPIY );
                if ( !mpFrameData->mpFontList->Count() )
                {
                    mpGraphics->GetDevFontList( mpFrameData->mpFontList );
                    mpFrameData->mpFontList->InitStdFonts();
                }
            }
        }
#else
        const REF( NMSP_CLIENT::XRmFrameWindow )& rxWindow = mpFrame->GetFrameInterface();
        REF( NMSP_CLIENT::XRmOutputDevice ) xOutDev( mpFrame->GetOutdevInterface() );

        if ( rxWindow.is() && xOutDev.is() )
        {
            mpGraphics = new ImplServerGraphics;
            mpGraphics->SetInterface( xOutDev );
            if ( pParent )
            {
                mpFrameData->mnDPIX     = pParent->mpFrameData->mnDPIX;
                mpFrameData->mnDPIY     = pParent->mpFrameData->mnDPIY;
                mpFrameData->mnFontDPIX = pParent->mpFrameData->mnFontDPIX;
                mpFrameData->mnFontDPIY = pParent->mpFrameData->mnFontDPIY;
            }
            else
            {
                // We currently assume, that we have only one display
                static NMSP_CLIENT::RmFrameResolutions aResl = mpFrame->GetFrameResolutions();
                mpFrameData->mnDPIX     = aResl.DPIx;
                mpFrameData->mnDPIY     = aResl.DPIy;
                mpFrameData->mnFontDPIX = aResl.FontDPIx;
                mpFrameData->mnFontDPIY = aResl.FontDPIy;
                mpGraphics->SetWindowResolution( aResl.DPIx, aResl.DPIy, aResl.Depth );
                if ( !mpFrameData->mpFontList->Count() )
                {
                    mpGraphics->GetDevFontList( mpFrameData->mpFontList );
                    mpFrameData->mpFontList->InitStdFonts();
                }
            }
        }
#endif

        // If we create a Window with default size, query this
        // size directly, because we want resize all Controls to
        // the correct size before we display the window
        if ( nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_APP) )
            mpFrame->GetClientSize( mnOutWidth, mnOutHeight );
    }
    else
    {
        if ( pParent )
        {
            if ( !ImplIsOverlapWindow() )
            {
                mbDisabled          = pParent->mbDisabled;
                mbInputDisabled     = pParent->mbInputDisabled;
                mbAlwaysEnableInput = pParent->mbAlwaysEnableInput;
            }

            OutputDevice::SetSettings( pParent->GetSettings() );
        }
    }

    const StyleSettings& rStyleSettings = maSettings.GetStyleSettings();
    USHORT nScreenZoom = rStyleSettings.GetScreenZoom();
    mnDPIX          = (mpFrameData->mnDPIX*nScreenZoom)/100;
    mnDPIY          = (mpFrameData->mnDPIY*nScreenZoom)/100;
    mpFontList      = mpFrameData->mpFontList;
    mpFontCache     = mpFrameData->mpFontCache;
    maFont          = rStyleSettings.GetAppFont();
    ImplPointToLogic( maFont );

    if ( nStyle & WB_3DLOOK )
    {
        SetTextColor( rStyleSettings.GetButtonTextColor() );
        SetBackground( Wallpaper( rStyleSettings.GetFaceColor() ) );
    }
    else
    {
        SetTextColor( rStyleSettings.GetWindowTextColor() );
        SetBackground( Wallpaper( rStyleSettings.GetWindowColor() ) );
    }

    ImplUpdatePos();

    // AppFont-Aufloesung berechnen
    if ( mbFrame && !pSVData->maGDIData.mnAppFontX )
        ImplInitAppFontData( this );
}

// -----------------------------------------------------------------------

void Window::ImplInsertWindow( Window* pParent )
{
    mpParent            = pParent;
    mpRealParent        = pParent;

    if ( pParent && !mbFrame )
    {
        // search frame window and set window frame data
        Window* pFrameParent = pParent->mpFrameWindow;
        mpFrameData     = pFrameParent->mpFrameData;
        mpFrame         = pFrameParent->mpFrame;
        mpFrameWindow   = pFrameParent;
        mbFrame         = FALSE;
#ifdef REMOTE_APPSERVER
        mpGraphics      = mpFrameWindow->mpGraphics;
#endif

        // search overlap window and insert window in list
        if ( ImplIsOverlapWindow() )
        {
            Window* pFirstOverlapParent = pParent;
            while ( !pFirstOverlapParent->ImplIsOverlapWindow() )
                pFirstOverlapParent = pFirstOverlapParent->ImplGetParent();
            mpOverlapWindow = pFirstOverlapParent;

            mpNextOverlap = mpFrameData->mpFirstOverlap;
            mpFrameData->mpFirstOverlap = this;

            // Overlap-Windows sind per default die obersten
            mpNext = pFirstOverlapParent->mpFirstOverlap;
            pFirstOverlapParent->mpFirstOverlap = this;
            if ( !pFirstOverlapParent->mpLastOverlap )
                pFirstOverlapParent->mpLastOverlap = this;
            else
                mpNext->mpPrev = this;
        }
        else
        {
            if ( pParent->ImplIsOverlapWindow() )
                mpOverlapWindow = pParent;
            else
                mpOverlapWindow = pParent->mpOverlapWindow;
            mpPrev = pParent->mpLastChild;
            pParent->mpLastChild = this;
            if ( !pParent->mpFirstChild )
                pParent->mpFirstChild = this;
            else
                mpPrev->mpNext = this;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplRemoveWindow( BOOL bRemoveFrameData )
{
    // Fenster aus den Listen austragen
    if ( !mbFrame )
    {
        if ( ImplIsOverlapWindow() )
        {
            if ( mpFrameData->mpFirstOverlap == this )
                mpFrameData->mpFirstOverlap = mpNextOverlap;
            else
            {
                Window* pTempWin = mpFrameData->mpFirstOverlap;
                while ( pTempWin->mpNextOverlap != this )
                    pTempWin = pTempWin->mpNextOverlap;
                pTempWin->mpNextOverlap = mpNextOverlap;
            }

            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpOverlapWindow->mpFirstOverlap = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpOverlapWindow->mpLastOverlap = mpPrev;
        }
        else
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpParent->mpFirstChild = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpParent->mpLastChild = mpPrev;
        }

        mpPrev = NULL;
        mpNext = NULL;
    }

    if ( bRemoveFrameData )
    {
        // Graphic freigeben
#ifndef REMOTE_APPSERVER
        ImplReleaseGraphics();
#else
        ImplReleaseServerGraphics();
#endif
    }
}

// -----------------------------------------------------------------------

static ULONG ImplAutoHelpID()
{
    if ( !Application::IsAutoHelpIdEnabled() )
        return 0;

    ULONG nHID = 0;

    ResMgr *pResMgr = Resource::GetResManager();

    DBG_ASSERT( pResMgr,          "MM hat gesagt, dass wir immer einen haben" );
    DBG_ASSERT( pResMgr->nTopRes, "MM hat gesagt, dass der Stack nie leer ist" );
    if( !pResMgr || pResMgr->nTopRes < 1 || pResMgr->nTopRes > 2 )
        return 0;

    const ImpRCStack *pRC = pResMgr->StackTop( pResMgr->nTopRes==1 ? 0 : 1 );

    DBG_ASSERT( pRC->pResource, "MM hat gesagt, dass der immer einen hat" );
    ULONG nGID = pRC->pResource->GetId();

    if( !nGID || nGID > 32767 )
        return 0;

    // GGGg gggg::gggg gggg::ggLL LLLl::llll llll
    switch( pRC->pResource->GetRT() ) { // maximal 7
        case RSC_DOCKINGWINDOW:
            nHID += 0x20000000L;
        case RSC_WORKWIN:
            nHID += 0x20000000L;
        case RSC_MODELESSDIALOG:
            nHID += 0x20000000L;
        case RSC_FLOATINGWINDOW:
            nHID += 0x20000000L;
        case RSC_MODALDIALOG:
            nHID += 0x20000000L;
        case RSC_TABPAGE:
            nHID += 0x20000000L;

            if( pResMgr->nTopRes == 2 ) {
                pRC = pResMgr->StackTop();
                ULONG nLID = pRC->pResource->GetId();

                if( !nLID || nLID > 511 )
                    return 0;

                switch( pRC->pResource->GetRT() ) { // maximal 32
                    case RSC_TABCONTROL:        nHID |= 0x0000; break;
                    case RSC_RADIOBUTTON:       nHID |= 0x0200; break;
                    case RSC_CHECKBOX:          nHID |= 0x0400; break;
                    case RSC_TRISTATEBOX:       nHID |= 0x0600; break;
                    case RSC_EDIT:              nHID |= 0x0800; break;
                    case RSC_MULTILINEEDIT:     nHID |= 0x0A00; break;
                    case RSC_MULTILISTBOX:      nHID |= 0x0C00; break;
                    case RSC_LISTBOX:           nHID |= 0x0E00; break;
                    case RSC_COMBOBOX:          nHID |= 0x1000; break;
                    case RSC_PUSHBUTTON:        nHID |= 0x1200; break;
                    case RSC_SPINFIELD:         nHID |= 0x1400; break;
                    case RSC_PATTERNFIELD:      nHID |= 0x1600; break;
                    case RSC_NUMERICFIELD:      nHID |= 0x1800; break;
                    case RSC_METRICFIELD:       nHID |= 0x1A00; break;
                    case RSC_CURRENCYFIELD:     nHID |= 0x1C00; break;
                    case RSC_DATEFIELD:         nHID |= 0x1E00; break;
                    case RSC_TIMEFIELD:         nHID |= 0x2000; break;
                    case RSC_IMAGERADIOBUTTON:  nHID |= 0x2200; break;
                    case RSC_NUMERICBOX:        nHID |= 0x2400; break;
                    case RSC_METRICBOX:         nHID |= 0x2600; break;
                    case RSC_CURRENCYBOX:       nHID |= 0x2800; break;
                    case RSC_DATEBOX:           nHID |= 0x2A00; break;
                    case RSC_TIMEBOX:           nHID |= 0x2C00; break;
                    case RSC_IMAGEBUTTON:       nHID |= 0x2E00; break;
                    case RSC_MENUBUTTON:        nHID |= 0x3000; break;
                    case RSC_MOREBUTTON:        nHID |= 0x3200; break;
                    default:
                        return 0;
                } // of switch
                nHID |= nLID;
            } // of if
            break;
        default:
            return 0;
    } // of switch
    nHID |= nGID << 14;
    return nHID;
}

// -----------------------------------------------------------------------

WinBits Window::ImplInitRes( const ResId& rResId )
{
    GetRes( rResId );

    char* pRes = (char*)GetClassRes();
    pRes += 4;
    ULONG nStyle = GetLongRes( (void*)pRes );
    ((ResId&)rResId).aWinBits = nStyle;
    return nStyle;
}

// -----------------------------------------------------------------------

void Window::ImplLoadRes( const ResId& rResId )
{
    // newer move this line after IncrementRes
    char* pRes = (char*)GetClassRes();
    pRes += 8;
    ULONG nHelpId = (ULONG)GetLongRes( (void*)pRes );
    if ( !nHelpId )
        nHelpId = ImplAutoHelpID();
    SetHelpId( nHelpId );

    USHORT nObjMask = (USHORT)ReadShortRes();

    // ResourceStyle
    USHORT nRSStyle = ReadShortRes();
    // WinBits
    ReadLongRes();
    // HelpId
    ReadLongRes();

    BOOL  bPos  = FALSE;
    BOOL  bSize = FALSE;
    Point aPos;
    Size  aSize;

    if ( nObjMask & (WINDOW_XYMAPMODE | WINDOW_X | WINDOW_Y) )
    {
        // Groessenangabe aus der Resource verwenden
        MapUnit ePosMap = MAP_PIXEL;

        bPos = TRUE;

        if ( nObjMask & WINDOW_XYMAPMODE )
            ePosMap = (MapUnit)(USHORT)ReadShortRes();
        if ( nObjMask & WINDOW_X )
            aPos.X() = ImplLogicUnitToPixelX( ReadLongRes(), ePosMap );
        if ( nObjMask & WINDOW_Y )
            aPos.Y() = ImplLogicUnitToPixelY( ReadLongRes(), ePosMap );
    }

    if ( nObjMask & (WINDOW_WHMAPMODE | WINDOW_WIDTH | WINDOW_HEIGHT) )
    {
        // Groessenangabe aus der Resource verwenden
        MapUnit eSizeMap = MAP_PIXEL;

        bSize = TRUE;

        if ( nObjMask & WINDOW_WHMAPMODE )
            eSizeMap = (MapUnit)(USHORT)ReadShortRes();
        if ( nObjMask & WINDOW_WIDTH )
            aSize.Width() = ImplLogicUnitToPixelX( ReadLongRes(), eSizeMap );
        if ( nObjMask & WINDOW_HEIGHT )
            aSize.Height() = ImplLogicUnitToPixelY( ReadLongRes(), eSizeMap );
    }

    // Wegen Optimierung so schlimm aussehend
    if ( nRSStyle & RSWND_CLIENTSIZE )
    {
        if ( bPos )
            SetPosPixel( aPos );
        if ( bSize )
            SetOutputSizePixel( aSize );
    }
    else if ( bPos && bSize )
        SetPosSizePixel( aPos, aSize );
    else if ( bPos )
        SetPosPixel( aPos );
    else if ( bSize )
        SetSizePixel( aSize );

    if ( nRSStyle & RSWND_DISABLED )
        Enable( FALSE );

    if ( nObjMask & WINDOW_TEXT )
        SetText( ReadStringRes() );
    if ( nObjMask & WINDOW_HELPTEXT )
        SetHelpText( ReadStringRes() );
    if ( nObjMask & WINDOW_QUICKTEXT )
        SetQuickHelpText( ReadStringRes() );
    if ( nObjMask & WINDOW_EXTRALONG )
        SetData( (void*)ReadLongRes() );
    if ( nObjMask & WINDOW_UNIQUEID )
        SetUniqueId( (ULONG)ReadLongRes() );
}

// -----------------------------------------------------------------------

ImplWinData* Window::ImplGetWinData() const
{
    if ( !mpWinData )
    {
        ((Window*)this)->mpWinData = new ImplWinData;
        mpWinData->mpExtOldText     = NULL;
        mpWinData->mpExtOldAttrAry  = NULL;
        mpWinData->mpCursorRect     = 0;
        mpWinData->mnCursorExtWidth = 0;
        mpWinData->mpFocusRect      = NULL;
        mpWinData->mpTrackRect      = NULL;
        mpWinData->mnTrackFlags     = 0;
    }

    return mpWinData;
}

// -----------------------------------------------------------------------

#ifndef REMOTE_APPSERVER
SalGraphics* Window::ImplGetFrameGraphics() const
{
    if ( mpFrameWindow->mpGraphics )
        mpFrameWindow->mbInitClipRegion = TRUE;
    else
        mpFrameWindow->ImplGetGraphics();
    mpFrameWindow->mpGraphics->ResetClipRegion();
    return mpFrameWindow->mpGraphics;
}
#endif

// -----------------------------------------------------------------------

Window* Window::ImplFindWindow( const Point& rFramePos )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Window* pTempWindow;
    Window* pFindWindow;

    // Zuerst alle ueberlappenden Fenster ueberpruefen
    pTempWindow = mpFirstOverlap;
    while ( pTempWindow )
    {
        pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
        if ( pFindWindow )
            return pFindWindow;
        pTempWindow = pTempWindow->mpNext;
    }

    // dann testen wir unser Fenster
    if ( !mbVisible )
        return NULL;

    USHORT nHitTest = ImplHitTest( rFramePos );
    if ( nHitTest & WINDOW_HITTEST_INSIDE )
    {
        // und danach gehen wir noch alle Child-Fenster durch
        pTempWindow = mpFirstChild;
        while ( pTempWindow )
        {
            pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
            if ( pFindWindow )
                return pFindWindow;
            pTempWindow = pTempWindow->mpNext;
        }

        if ( nHitTest & WINDOW_HITTEST_TRANSPARENT )
            return NULL;
        else
            return this;
    }

    return NULL;
}

// -----------------------------------------------------------------------

USHORT Window::ImplHitTest( const Point& rFramePos )
{
    Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    if ( !aRect.IsInside( rFramePos ) )
        return 0;
    if ( mbWinRegion )
    {
        Point aTempPos = rFramePos;
        aTempPos.X() -= mnOutOffX;
        aTempPos.Y() -= mnOutOffY;
        if ( !maWinRegion.IsInside( aTempPos ) )
            return 0;
    }

    USHORT nHitTest = WINDOW_HITTEST_INSIDE;
    if ( mbMouseTransparent )
        nHitTest |= WINDOW_HITTEST_TRANSPARENT;
    return nHitTest;
}

// -----------------------------------------------------------------------

BOOL Window::ImplIsRealParentPath( const Window* pWindow ) const
{
    pWindow = pWindow->GetParent();
    while ( pWindow )
    {
        if ( pWindow == this )
            return TRUE;
        pWindow = pWindow->GetParent();
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Window::ImplIsChild( const Window* pWindow, BOOL bSystemWindow ) const
{
    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return TRUE;
    }
    while ( pWindow );

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Window::ImplIsWindowOrChild( const Window* pWindow, BOOL bSystemWindow ) const
{
    if ( this == pWindow )
        return TRUE;
    return ImplIsChild( pWindow, bSystemWindow );
}

// -----------------------------------------------------------------------

Window* Window::ImplGetSameParent( const Window* pWindow ) const
{
    if ( mpFrameWindow != pWindow->mpFrameWindow )
        return NULL;
    else
    {
        if ( pWindow->ImplIsChild( this ) )
            return (Window*)pWindow;
        else
        {
            Window* pTestWindow = (Window*)this;
            while ( (pTestWindow == pWindow) || pTestWindow->ImplIsChild( pWindow ) )
                pTestWindow = pTestWindow->ImplGetParent();
            return pTestWindow;
        }
    }
}

// -----------------------------------------------------------------------

int Window::ImplTestMousePointerSet()
{
    // Wenn Mouse gecaptured ist, dann soll MousePointer umgeschaltet werden
    if ( IsMouseCaptured() )
        return TRUE;

    // Wenn sich Mouse ueber dem Fenster befindet, dann soll MousePointer
    // umgeschaltet werden
    Rectangle aClientRect( Point( 0, 0 ), GetOutputSizePixel() );
    if ( aClientRect.IsInside( GetPointerPosPixel() ) )
        return TRUE;

    return FALSE;
}

// -----------------------------------------------------------------------

PointerStyle Window::ImplGetMousePointer() const
{
    PointerStyle    ePointerStyle;
    BOOL            bWait = FALSE;

    if ( IsEnabled() && IsInputEnabled() )
        ePointerStyle = GetPointer().GetStyle();
    else
        ePointerStyle = POINTER_ARROW;

    const Window* pWindow = this;
    do
    {
        // Wenn Pointer nicht sichtbar, dann wird suche abgebrochen, da
        // dieser Status nicht ueberschrieben werden darf
        if ( pWindow->mbNoPtrVisible )
            return POINTER_NULL;

        if ( !bWait )
        {
            if ( pWindow->mnWaitCount )
            {
                ePointerStyle = POINTER_WAIT;
                bWait = TRUE;
            }
            else
            {
                if ( pWindow->mbChildPtrOverwrite )
                    ePointerStyle = pWindow->GetPointer().GetStyle();
            }
        }

        if ( pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    return ePointerStyle;
}

// -----------------------------------------------------------------------

void Window::ImplResetReallyVisible()
{
    mbDevOutput     = FALSE;
    mbReallyVisible = FALSE;
    mbReallyShown   = FALSE;

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpNext;
    }

    pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplSetReallyVisible()
{
    mbDevOutput     = TRUE;
    mbReallyVisible = TRUE;
    mbReallyShown   = TRUE;

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpNext;
    }

    pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallInitShow()
{
    mbReallyShown   = TRUE;
    mbInInitShow    = TRUE;
    StateChanged( STATE_CHANGE_INITSHOW );
    mbInInitShow    = FALSE;

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpNext;
    }

    pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplAddDel( ImplDelData* pDel )
{
    pDel->mpNext = mpFirstDel;
    mpFirstDel = pDel;
}

// -----------------------------------------------------------------------

void Window::ImplRemoveDel( ImplDelData* pDel )
{
    if ( mpFirstDel == pDel )
        mpFirstDel = pDel->mpNext;
    else
    {
        ImplDelData* pData = mpFirstDel;
        while ( pData->mpNext != pDel )
            pData = pData->mpNext;
        pData->mpNext = pDel->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInitResolutionSettings()
{
    // AppFont-Aufloesung und DPI-Aufloesung neu berechnen
    if ( mbFrame )
    {
        const StyleSettings& rStyleSettings = maSettings.GetStyleSettings();
        USHORT nScreenZoom = rStyleSettings.GetScreenZoom();
        mnDPIX = (mpFrameData->mnDPIX*nScreenZoom)/100;
        mnDPIY = (mpFrameData->mnDPIY*nScreenZoom)/100;
        SetPointFont( rStyleSettings.GetAppFont() );

        if ( !ImplGetSVData()->maGDIData.mnAppFontX )
            ImplInitAppFontData( this );
    }
    else if ( mpParent )
    {
        mnDPIX  = mpParent->mnDPIX;
        mnDPIY  = mpParent->mnDPIY;
    }

    // Vorberechnete Werte fuer logische Einheiten updaten und auch
    // die entsprechenden Tools dazu
    if ( IsMapMode() )
    {
        MapMode aMapMode = GetMapMode();
        SetMapMode();
        SetMapMode( aMapMode );
    }
}

// -----------------------------------------------------------------------

void Window::ImplPointToLogic( Font& rFont ) const
{
    Size    aSize = rFont.GetSize();
    USHORT  nScreenFontZoom = maSettings.GetStyleSettings().GetScreenFontZoom();

    if ( aSize.Width() )
    {
        aSize.Width() *= mpFrameData->mnFontDPIX;
        aSize.Width() += 72/2;
        aSize.Width() /= 72;
        aSize.Width() *= nScreenFontZoom;
        aSize.Width() /= 100;
    }
    aSize.Height() *= mpFrameData->mnFontDPIY;
    aSize.Height() += 72/2;
    aSize.Height() /= 72;
    aSize.Height() *= nScreenFontZoom;
    aSize.Height() /= 100;

    if ( IsMapModeEnabled() )
        aSize = PixelToLogic( aSize );

    rFont.SetSize( aSize );
}

// -----------------------------------------------------------------------

void Window::ImplLogicToPoint( Font& rFont ) const
{
    Size    aSize = rFont.GetSize();
    USHORT  nScreenFontZoom = maSettings.GetStyleSettings().GetScreenFontZoom();

    if ( IsMapModeEnabled() )
        aSize = LogicToPixel( aSize );

    if ( aSize.Width() )
    {
        aSize.Width() *= 100;
        aSize.Width() /= nScreenFontZoom;
        aSize.Width() *= 72;
        aSize.Width() += mpFrameData->mnFontDPIX/2;
        aSize.Width() /= mpFrameData->mnFontDPIX;
    }
    aSize.Height() *= 100;
    aSize.Height() /= nScreenFontZoom;
    aSize.Height() *= 72;
    aSize.Height() += mpFrameData->mnFontDPIY/2;
    aSize.Height() /= mpFrameData->mnFontDPIY;

    rFont.SetSize( aSize );
}

// -----------------------------------------------------------------------

#ifndef REMOTE_APPSERVER
BOOL Window::ImplSysObjClip( const Region* pOldRegion )
{
    BOOL bUpdate = TRUE;

    if ( mpSysObj )
    {
        BOOL bVisibleState = mbReallyVisible;

        if ( bVisibleState )
        {
            Region* pWinChildClipRegion = ImplGetWinChildClipRegion();

            if ( !pWinChildClipRegion->IsEmpty() )
            {
                if ( pOldRegion )
                {
                    Region aNewRegion = *pWinChildClipRegion;
                    pWinChildClipRegion->Intersect( *pOldRegion );
                    bUpdate = aNewRegion == *pWinChildClipRegion;
                }

                if ( mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();

                Region      aRegion = *pWinChildClipRegion;
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Region      aWinRectRegion( aWinRect );
                USHORT      nClipFlags = mpSysObj->GetClipRegionType();

                if ( aRegion == aWinRectRegion )
                    mpSysObj->ResetClipRegion();
                else
                {
                    if ( nClipFlags & SAL_OBJECT_CLIP_EXCLUDERECTS )
                    {
                        aWinRectRegion.Exclude( aRegion );
                        aRegion = aWinRectRegion;
                    }
                    if ( !(nClipFlags & SAL_OBJECT_CLIP_ABSOLUTE) )
                        aRegion.Move( -mnOutOffX, -mnOutOffY );

                    // ClipRegion setzen/updaten
                    long                nX;
                    long                nY;
                    long                nWidth;
                    long                nHeight;
                    ULONG               nRectCount;
                    ImplRegionInfo      aInfo;
                    BOOL                bRegionRect;

                    nRectCount = aRegion.GetRectCount();
                    mpSysObj->BeginSetClipRegion( nRectCount );
                    bRegionRect = aRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
                    while ( bRegionRect )
                    {
                        mpSysObj->UnionClipRegion( nX, nY, nWidth, nHeight );
                        bRegionRect = aRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
                    }
                    mpSysObj->EndSetClipRegion();
                }
            }
            else
                bVisibleState = FALSE;
        }

        // Visible-Status updaten
        mpSysObj->Show( bVisibleState );
    }

    return bUpdate;
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjChildsClip()
{
    if ( mpSysObj && mbInitWinClipRegion )
        ImplSysObjClip( NULL );

    Window* pWindow = mpFirstChild;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjChildsClip();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjOverlapsClip()
{
    ImplUpdateSysObjChildsClip();

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjOverlapsClip();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjClip()
{
    if ( !ImplIsOverlapWindow() )
    {
        ImplUpdateSysObjChildsClip();

        // Schwestern muessen ihre ClipRegion auch neu berechnen
        if ( mbClipSiblings )
        {
            Window* pWindow = mpNext;
            while ( pWindow )
            {
                pWindow->ImplUpdateSysObjChildsClip();
                pWindow = pWindow->mpNext;
            }
        }
    }
    else
        mpFrameWindow->ImplUpdateSysObjOverlapsClip();
}

#endif

// -----------------------------------------------------------------------

BOOL Window::ImplSetClipFlagChilds( BOOL bSysObjOnlySmaller )
{
    BOOL bUpdate = TRUE;
#ifndef REMOTE_APPSERVER
    if ( mpSysObj )
    {
        Region* pOldRegion = NULL;
        if ( bSysObjOnlySmaller && !mbInitWinClipRegion )
            pOldRegion = new Region( maWinClipRegion );

        mbInitClipRegion = TRUE;
        mbInitWinClipRegion = TRUE;

        Window* pWindow = mpFirstChild;
        while ( pWindow )
        {
            if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
                bUpdate = FALSE;
            pWindow = pWindow->mpNext;
        }

        if ( !ImplSysObjClip( pOldRegion ) )
        {
            mbInitClipRegion = TRUE;
            mbInitWinClipRegion = TRUE;
            bUpdate = FALSE;
        }

        if ( pOldRegion )
            delete pOldRegion;
    }
    else
#endif
    {
    mbInitClipRegion = TRUE;
    mbInitWinClipRegion = TRUE;

    Window* pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
            bUpdate = FALSE;
        pWindow = pWindow->mpNext;
    }
    }
    return bUpdate;
}

// -----------------------------------------------------------------------

BOOL Window::ImplSetClipFlagOverlapWindows( BOOL bSysObjOnlySmaller )
{
    BOOL bUpdate = ImplSetClipFlagChilds( bSysObjOnlySmaller );

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller ) )
            bUpdate = FALSE;
        pWindow = pWindow->mpNext;
    }

    return bUpdate;
}

// -----------------------------------------------------------------------

BOOL Window::ImplSetClipFlag( BOOL bSysObjOnlySmaller )
{
    if ( !ImplIsOverlapWindow() )
    {
        BOOL bUpdate = ImplSetClipFlagChilds( bSysObjOnlySmaller );

        Window* pParent = ImplGetParent();
        if ( pParent &&
             ((pParent->GetStyle() & WB_CLIPCHILDREN) || (mnParentClipMode & PARENTCLIPMODE_CLIP)) )
        {
            pParent->mbInitClipRegion = TRUE;
            pParent->mbInitChildRegion = TRUE;
        }

        // Schwestern muessen ihre ClipRegion auch neu berechnen
        if ( mbClipSiblings )
        {
            Window* pWindow = mpNext;
            while ( pWindow )
            {
                if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
                    bUpdate = FALSE;
                pWindow = pWindow->mpNext;
            }
        }

        return bUpdate;
    }
    else
        return mpFrameWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller );
}

// -----------------------------------------------------------------------

void Window::ImplIntersectWindowClipRegion( Region& rRegion )
{
    if ( mbInitWinClipRegion )
        ImplInitWinClipRegion();

    rRegion.Intersect( maWinClipRegion );
}

// -----------------------------------------------------------------------

void Window::ImplIntersectWindowRegion( Region& rRegion )
{
    rRegion.Intersect( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                  Size( mnOutWidth, mnOutHeight ) ) );
    if ( mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
}

// -----------------------------------------------------------------------

void Window::ImplExcludeWindowRegion( Region& rRegion )
{
    if ( mbWinRegion )
    {
        Point aPoint( mnOutOffX, mnOutOffY );
        Region aRegion( Rectangle( aPoint,
                                   Size( mnOutWidth, mnOutHeight ) ) );
        aRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
        rRegion.Exclude( aRegion );
    }
    else
    {
        Point aPoint( mnOutOffX, mnOutOffY );
        rRegion.Exclude( Rectangle( aPoint,
                                    Size( mnOutWidth, mnOutHeight ) ) );
    }
}

// -----------------------------------------------------------------------

void Window::ImplExcludeOverlapWindows( Region& rRegion )
{
    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
        {
            pWindow->ImplExcludeWindowRegion( rRegion );
            pWindow->ImplExcludeOverlapWindows( rRegion );
        }

        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplExcludeOverlapWindows2( Region& rRegion )
{
    if ( mbReallyVisible )
        ImplExcludeWindowRegion( rRegion );

    ImplExcludeOverlapWindows( rRegion );
}

// -----------------------------------------------------------------------

void Window::ImplClipBoundaries( Region& rRegion, BOOL bThis, BOOL bOverlaps )
{
    if ( bThis )
        ImplIntersectWindowClipRegion( rRegion );
    else if ( ImplIsOverlapWindow() )
    {
        // Evt. noch am Frame clippen
        if ( !mbFrame )
            rRegion.Intersect( Rectangle( Point( 0, 0 ), Size( mpFrameWindow->mnOutWidth, mpFrameWindow->mnOutHeight ) ) );

        if ( bOverlaps && !rRegion.IsEmpty() )
        {
            // Clip Overlap Siblings
            Window* pStartOverlapWindow = this;
            while ( !pStartOverlapWindow->mbFrame )
            {
                Window* pOverlapWindow = pStartOverlapWindow->mpOverlapWindow->mpFirstOverlap;
                while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
                {
                    pOverlapWindow->ImplExcludeOverlapWindows2( rRegion );
                    pOverlapWindow = pOverlapWindow->mpNext;
                }
                pStartOverlapWindow = pStartOverlapWindow->mpOverlapWindow;
            }

            // Clip Child Overlap Windows
            ImplExcludeOverlapWindows( rRegion );
        }
    }
    else
        ImplGetParent()->ImplIntersectWindowClipRegion( rRegion );
}

// -----------------------------------------------------------------------

BOOL Window::ImplClipChilds( Region& rRegion )
{
    BOOL    bOtherClip = FALSE;
    Window* pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
        {
            // ParentClipMode-Flags auswerten
            USHORT nClipMode = pWindow->GetParentClipMode();
            if ( !(nClipMode & PARENTCLIPMODE_NOCLIP) &&
                 ((nClipMode & PARENTCLIPMODE_CLIP) || (GetStyle() & WB_CLIPCHILDREN)) )
                pWindow->ImplExcludeWindowRegion( rRegion );
            else
                bOtherClip = TRUE;
        }

        pWindow = pWindow->mpNext;
    }

    return bOtherClip;
}

// -----------------------------------------------------------------------

void Window::ImplClipAllChilds( Region& rRegion )
{
    Window* pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
            pWindow->ImplExcludeWindowRegion( rRegion );
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplClipSiblings( Region& rRegion )
{
    Window* pWindow = ImplGetParent()->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow == this )
            break;

        if ( pWindow->mbReallyVisible )
            pWindow->ImplExcludeWindowRegion( rRegion );

        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInitWinClipRegion()
{
    // Build Window Region
    maWinClipRegion = Rectangle( Point( mnOutOffX, mnOutOffY ),
                                 Size( mnOutWidth, mnOutHeight ) );
    if ( mbWinRegion )
        maWinClipRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );

    // ClipSiblings
    if ( mbClipSiblings && !ImplIsOverlapWindow() )
        ImplClipSiblings( maWinClipRegion );

    // Clip Parent Boundaries
    ImplClipBoundaries( maWinClipRegion, FALSE, TRUE );

    // Clip Children
    if ( (GetStyle() & WB_CLIPCHILDREN) || mbClipChildren )
        mbInitChildRegion = TRUE;

    mbInitWinClipRegion = FALSE;
}

// -----------------------------------------------------------------------

void Window::ImplInitWinChildClipRegion()
{
    if ( !mpFirstChild )
    {
        if ( mpChildClipRegion )
        {
            delete mpChildClipRegion;
            mpChildClipRegion = NULL;
        }
    }
    else
    {
        if ( !mpChildClipRegion )
            mpChildClipRegion = new Region( maWinClipRegion );
        else
            *mpChildClipRegion = maWinClipRegion;

        ImplClipChilds( *mpChildClipRegion );
    }

    mbInitChildRegion = FALSE;
}

// -----------------------------------------------------------------------

Region* Window::ImplGetWinChildClipRegion()
{
    if ( mbInitWinClipRegion )
        ImplInitWinClipRegion();
    if ( mbInitChildRegion )
        ImplInitWinChildClipRegion();
    if ( mpChildClipRegion )
        return mpChildClipRegion;
    else
        return &maWinClipRegion;
}

// -----------------------------------------------------------------------

void Window::ImplIntersectAndUnionOverlapWindows( const Region& rInterRegion, Region& rRegion )
{
    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
        {
            Region aTempRegion( rInterRegion );
            pWindow->ImplIntersectWindowRegion( aTempRegion );
            rRegion.Union( aTempRegion );
            pWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
        }

        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplIntersectAndUnionOverlapWindows2( const Region& rInterRegion, Region& rRegion )
{
    if ( mbReallyVisible )
    {
        Region aTempRegion( rInterRegion );
        ImplIntersectWindowRegion( aTempRegion );
        rRegion.Union( aTempRegion );
    }

    ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

// -----------------------------------------------------------------------

void Window::ImplCalcOverlapRegionOverlaps( const Region& rInterRegion, Region& rRegion )
{
    // Clip Overlap Siblings
    Window* pStartOverlapWindow;
    if ( !ImplIsOverlapWindow() )
        pStartOverlapWindow = mpOverlapWindow;
    else
        pStartOverlapWindow = this;
    while ( !pStartOverlapWindow->mbFrame )
    {
        Window* pOverlapWindow = pStartOverlapWindow->mpOverlapWindow->mpFirstOverlap;
        while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
        {
            pOverlapWindow->ImplIntersectAndUnionOverlapWindows2( rInterRegion, rRegion );
            pOverlapWindow = pOverlapWindow->mpNext;
        }
        pStartOverlapWindow = pStartOverlapWindow->mpOverlapWindow;
    }

    // Clip Child Overlap Windows
    if ( !ImplIsOverlapWindow() )
        mpOverlapWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
    else
        ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

// -----------------------------------------------------------------------

void Window::ImplCalcOverlapRegion( const Rectangle& rSourceRect, Region& rRegion,
                                    BOOL bChilds, BOOL bParent, BOOL bSiblings )
{
    Region  aRegion( rSourceRect );
    if ( mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
    Region  aTempRegion;
    Window* pWindow;

    ImplCalcOverlapRegionOverlaps( aRegion, rRegion );

    // Parent-Boundaries
    if ( bParent )
    {
        pWindow = this;
        if ( !ImplIsOverlapWindow() )
        {
            pWindow = ImplGetParent();
            do
            {
                aTempRegion = aRegion;
                pWindow->ImplExcludeWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
                if ( pWindow->ImplIsOverlapWindow() )
                    break;
                pWindow = pWindow->ImplGetParent();
            }
            while ( pWindow );
        }
        if ( !pWindow->mbFrame )
        {
            aTempRegion = aRegion;
            aTempRegion.Exclude( Rectangle( Point( 0, 0 ), Size( mpFrameWindow->mnOutWidth, mpFrameWindow->mnOutHeight ) ) );
            rRegion.Union( aTempRegion );
        }
    }

    // Siblings
    if ( bSiblings && !ImplIsOverlapWindow() )
    {
        pWindow = mpParent->mpFirstChild;
        do
        {
            if ( pWindow->mbReallyVisible && (pWindow != this) )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpNext;
        }
        while ( pWindow );
    }

    // Childs
    if ( bChilds )
    {
        pWindow = mpFirstChild;
        while ( pWindow )
        {
            if ( pWindow->mbReallyVisible )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallPaint( const Region* pRegion, USHORT nPaintFlags )
{
    mbPaintFrame = FALSE;

    if ( nPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
        mnPaintFlags |= IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALLCHILDS | (nPaintFlags & IMPL_PAINT_PAINTALL);
    if ( nPaintFlags & IMPL_PAINT_PAINTCHILDS )
        mnPaintFlags |= IMPL_PAINT_PAINTCHILDS;
    if ( nPaintFlags & IMPL_PAINT_ERASE )
        mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( !mpFirstChild )
        mnPaintFlags &= ~IMPL_PAINT_PAINTALLCHILDS;

    if ( mbPaintDisabled )
    {
        if ( mnPaintFlags & IMPL_PAINT_PAINTALL )
            Invalidate( INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE | INVALIDATE_NOTRANSPARENT | INVALIDATE_NOCLIPCHILDREN );
        else if ( pRegion )
            Invalidate( *pRegion, INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE | INVALIDATE_NOTRANSPARENT | INVALIDATE_NOCLIPCHILDREN );
        return;
    }

    nPaintFlags = mnPaintFlags & ~(IMPL_PAINT_PAINT);

    Region* pChildRegion = NULL;
    if ( mnPaintFlags & IMPL_PAINT_PAINT )
    {
        Region* pWinChildClipRegion = ImplGetWinChildClipRegion();
        if ( mnPaintFlags & IMPL_PAINT_PAINTALL )
            maInvalidateRegion = *pWinChildClipRegion;
        else
        {
            if ( pRegion )
                maInvalidateRegion.Union( *pRegion );
            if ( mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
                pChildRegion = new Region( maInvalidateRegion );
            maInvalidateRegion.Intersect( *pWinChildClipRegion );
        }
        mnPaintFlags = 0;
        if ( !maInvalidateRegion.IsEmpty() )
        {
            if ( mpCursor )
                mpCursor->ImplHide();

            mbInitClipRegion = TRUE;
            mbInPaint = TRUE;

            // Paint-Region zuruecksetzen
            Region      aPaintRegion( maInvalidateRegion );
            Rectangle   aPaintRect = ImplDevicePixelToLogic( aPaintRegion.GetBoundRect() );
            mpPaintRegion = &aPaintRegion;
            maInvalidateRegion.SetEmpty();

            if ( (nPaintFlags & IMPL_PAINT_ERASE) && IsBackground() )
            {
                if ( IsClipRegion() )
                {
                    Region aOldRegion = GetClipRegion();
                    SetClipRegion();
                    Erase();
                    SetClipRegion( aOldRegion );
                }
                else
                    Erase();
            }
            Paint( aPaintRect );

            if ( mpWinData )
            {
                if ( mbFocusVisible )
                    ImplInvertFocus( *(mpWinData->mpFocusRect) );
                if ( mbTrackVisible && (mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                    InvertTracking( *(mpWinData->mpTrackRect), mpWinData->mnTrackFlags );
            }
            mbInPaint = FALSE;
            mbInitClipRegion = TRUE;
            mpPaintRegion = NULL;
            if ( mpCursor )
                mpCursor->ImplShow( FALSE );
        }
    }
    else
        mnPaintFlags = 0;

    if ( nPaintFlags & (IMPL_PAINT_PAINTALLCHILDS | IMPL_PAINT_PAINTCHILDS) )
    {
        // die Childfenster ausgeben
        Window* pTempWindow = mpFirstChild;
        while ( pTempWindow )
        {
            if ( pTempWindow->mbVisible )
                pTempWindow->ImplCallPaint( pChildRegion, nPaintFlags );
            pTempWindow = pTempWindow->mpNext;
        }
    }

    if ( pChildRegion )
        delete pChildRegion;
}

// -----------------------------------------------------------------------

void Window::ImplCallOverlapPaint()
{
    // Zuerst geben wir die ueberlappenden Fenster aus
    Window* pTempWindow = mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->mbReallyVisible )
            pTempWindow->ImplCallOverlapPaint();
        pTempWindow = pTempWindow->mpNext;
    }

    // und dann erst uns selber
    if ( mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
        ImplCallPaint( NULL, mnPaintFlags );
}

// -----------------------------------------------------------------------

void Window::ImplPostPaint()
{
    if ( !mpFrameData->maPaintTimer.IsActive() )
        mpFrameData->maPaintTimer.Start();
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplHandlePaintHdl, void*, EMPTYARG )
{
    if ( mbReallyVisible )
        ImplCallOverlapPaint();
    return 0;
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateFrameRegion( const Region* pRegion, USHORT nFlags )
{
    // PAINTCHILDS bei allen Parent-Fenster bis zum ersten OverlapWindow
    // setzen
    if ( !ImplIsOverlapWindow() )
    {
        Window* pTempWindow = this;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mnPaintFlags & IMPL_PAINT_PAINTCHILDS )
                break;
            pTempWindow->mnPaintFlags |= IMPL_PAINT_PAINTCHILDS;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    // Paint-Flags setzen
    mnPaintFlags |= IMPL_PAINT_PAINT;
    if ( nFlags & INVALIDATE_CHILDREN )
        mnPaintFlags |= IMPL_PAINT_PAINTALLCHILDS;
    if ( !(nFlags & INVALIDATE_NOERASE) )
        mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( !pRegion )
        mnPaintFlags |= IMPL_PAINT_PAINTALL;

    // Wenn nicht alles neu ausgegeben werden muss, dann die Region
    // dazupacken
    if ( !(mnPaintFlags & IMPL_PAINT_PAINTALL) )
        maInvalidateRegion.Union( *pRegion );

    ImplPostPaint();
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateOverlapFrameRegion( const Region& rRegion )
{
    Region aRegion = rRegion;
    ImplClipBoundaries( aRegion, TRUE, TRUE );
    if ( !aRegion.IsEmpty() )
        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );

    // Dann invalidieren wir die ueberlappenden Fenster
    Window* pTempWindow = mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->IsVisible() )
            pTempWindow->ImplInvalidateOverlapFrameRegion( rRegion );

        pTempWindow = pTempWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateParentFrameRegion( Region& rRegion )
{
    if ( mbOverlapWin )
        mpFrameWindow->ImplInvalidateOverlapFrameRegion( rRegion );
    else
        ImplGetParent()->ImplInvalidateFrameRegion( &rRegion, INVALIDATE_CHILDREN );
}

// -----------------------------------------------------------------------

void Window::ImplInvalidate( const Region* pRegion, USHORT nFlags )
{
    // Hintergrund-Sicherung zuruecksetzen
    if ( mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // Feststellen, was neu ausgegeben werden muss
    BOOL bInvalidateAll = !pRegion;

    // Transparent-Invalidate beruecksichtigen
    Window* pWindow = this;
    if ( (mbPaintTransparent && !(nFlags & INVALIDATE_NOTRANSPARENT)) || (nFlags & INVALIDATE_TRANSPARENT) )
    {
        Window* pTempWindow = pWindow->ImplGetParent();
        while ( pTempWindow )
        {
            if ( !pTempWindow->IsPaintTransparent() )
            {
                pWindow = pTempWindow;
                nFlags |= INVALIDATE_CHILDREN;
                bInvalidateAll = FALSE;
                break;
            }

            if ( pTempWindow->ImplIsOverlapWindow() )
                break;

            pTempWindow = pTempWindow->ImplGetParent();
        }
    }

    // Region zusammenbauen
    USHORT nOrgFlags = nFlags;
    if ( !(nFlags & (INVALIDATE_CHILDREN | INVALIDATE_NOCHILDREN)) )
    {
        if ( pWindow->GetStyle() & WB_CLIPCHILDREN )
            nFlags |= INVALIDATE_NOCHILDREN;
        else
            nFlags |= INVALIDATE_CHILDREN;
    }
    if ( (nFlags & INVALIDATE_NOCHILDREN) && pWindow->mpFirstChild )
        bInvalidateAll = FALSE;
    if ( bInvalidateAll )
        pWindow->ImplInvalidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
            aRegion.Intersect( *pRegion );
        pWindow->ImplClipBoundaries( aRegion, TRUE, TRUE );
        if ( nFlags & INVALIDATE_NOCHILDREN )
        {
            nFlags &= ~INVALIDATE_CHILDREN;
            if ( !(nFlags & INVALIDATE_NOCLIPCHILDREN) )
            {
                if ( nOrgFlags & INVALIDATE_NOCHILDREN )
                    pWindow->ImplClipAllChilds( aRegion );
                else
                {
                    if ( pWindow->ImplClipChilds( aRegion ) )
                        nFlags |= INVALIDATE_CHILDREN;
                }
            }
        }
        if ( !aRegion.IsEmpty() )
            pWindow->ImplInvalidateFrameRegion( &aRegion, nFlags );
    }

    if ( nFlags & INVALIDATE_UPDATE )
        pWindow->Update();
}

// -----------------------------------------------------------------------

void Window::ImplMoveInvalidateRegion( const Rectangle& rRect,
                                       long nHorzScroll, long nVertScroll,
                                       BOOL bChilds )
{
    if ( (mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALL)) == IMPL_PAINT_PAINT )
    {
        Region aTempRegion = maInvalidateRegion;
        aTempRegion.Intersect( rRect );
        aTempRegion.Move( nHorzScroll, nVertScroll );
        maInvalidateRegion.Union( aTempRegion );
    }

    if ( bChilds && (mnPaintFlags & IMPL_PAINT_PAINTCHILDS) )
    {
        Window* pWindow = mpFirstChild;
        while ( pWindow )
        {
            pWindow->ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, TRUE );
            pWindow = pWindow->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplMoveAllInvalidateRegions( const Rectangle& rRect,
                                           long nHorzScroll, long nVertScroll,
                                           BOOL bChilds )
{
    // Paint-Region auch verschieben, wenn noch Paints anstehen
    ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, bChilds );
    // Paint-Region muss bei uns verschoben gesetzt werden, die durch
    // die Parents gezeichnet werden
    if ( !ImplIsOverlapWindow() )
    {
        Region  aPaintAllRegion;
        Window* pPaintAllWindow = this;
        do
        {
            pPaintAllWindow = pPaintAllWindow->ImplGetParent();
            if ( pPaintAllWindow->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
            {
                if ( pPaintAllWindow->mnPaintFlags & IMPL_PAINT_PAINTALL )
                {
                    aPaintAllRegion.SetEmpty();
                    break;
                }
                else
                    aPaintAllRegion.Union( pPaintAllWindow->maInvalidateRegion );
            }
        }
        while ( !pPaintAllWindow->ImplIsOverlapWindow() );
        if ( !aPaintAllRegion.IsEmpty() )
        {
            aPaintAllRegion.Move( nHorzScroll, nVertScroll );
            USHORT nPaintFlags = 0;
            if ( bChilds )
                mnPaintFlags |= INVALIDATE_CHILDREN;
            ImplInvalidateFrameRegion( &aPaintAllRegion, nPaintFlags );
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplValidateFrameRegion( const Region* pRegion, USHORT nFlags )
{
    if ( !pRegion )
        maInvalidateRegion.SetEmpty();
    else
    {
        // Wenn alle Childfenster neu ausgegeben werden muessen,
        // dann invalidieren wir diese vorher
        if ( (mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS) && mpFirstChild )
        {
            Region aChildRegion = maInvalidateRegion;
            if ( mnPaintFlags & IMPL_PAINT_PAINTALL )
            {
                Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                aChildRegion = aRect;
            }
            Window* pChild = mpFirstChild;
            while ( pChild )
            {
                pChild->Invalidate( aChildRegion, INVALIDATE_CHILDREN | INVALIDATE_NOTRANSPARENT );
                pChild = pChild->mpNext;
            }
        }
        if ( mnPaintFlags & IMPL_PAINT_PAINTALL )
        {
            Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            maInvalidateRegion = aRect;
        }
        maInvalidateRegion.Exclude( *pRegion );
    }
    mnPaintFlags &= ~IMPL_PAINT_PAINTALL;

    if ( nFlags & VALIDATE_CHILDREN )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->ImplValidateFrameRegion( pRegion, nFlags );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplValidate( const Region* pRegion, USHORT nFlags )
{
    // Region zusammenbauen
    BOOL    bValidateAll = !pRegion;
    USHORT  nOrgFlags = nFlags;
    if ( !(nFlags & (VALIDATE_CHILDREN | VALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= VALIDATE_NOCHILDREN;
        else
            nFlags |= VALIDATE_CHILDREN;
    }
    if ( (nFlags & VALIDATE_NOCHILDREN) && mpFirstChild )
        bValidateAll = FALSE;
    if ( bValidateAll )
        ImplValidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
            aRegion.Intersect( *pRegion );
        ImplClipBoundaries( aRegion, TRUE, TRUE );
        if ( nFlags & VALIDATE_NOCHILDREN )
        {
            nFlags &= ~VALIDATE_CHILDREN;
            if ( nOrgFlags & VALIDATE_NOCHILDREN )
                ImplClipAllChilds( aRegion );
            else
            {
                if ( ImplClipChilds( aRegion ) )
                    nFlags |= VALIDATE_CHILDREN;
            }
        }
        if ( !aRegion.IsEmpty() )
            ImplValidateFrameRegion( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::ImplScroll( const Rectangle& rRect,
                         long nHorzScroll, long nVertScroll, USHORT nFlags )
{
    if ( !IsDeviceOutputNecessary() )
        return;

    nHorzScroll = ImplLogicWidthToDevicePixel( nHorzScroll );
    nVertScroll = ImplLogicHeightToDevicePixel( nVertScroll );

    if ( !nHorzScroll && !nVertScroll )
        return;

    // Hintergrund-Sicherung zuruecksetzen
    if ( mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    if ( mpCursor )
        mpCursor->ImplHide();

    USHORT nOrgFlags = nFlags;
    if ( !(nFlags & (SCROLL_CHILDREN | SCROLL_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= SCROLL_NOCHILDREN;
        else
            nFlags |= SCROLL_CHILDREN;
    }

    Region  aInvalidateRegion;
    BOOL    bScrollChilds = (nFlags & SCROLL_CHILDREN) != 0;
    BOOL    bErase = (nFlags & SCROLL_NOERASE) == 0;

    if ( !mpFirstChild )
        bScrollChilds = FALSE;

    // Paint-Bereiche anpassen
    ImplMoveAllInvalidateRegions( rRect, nHorzScroll, nVertScroll, bScrollChilds );

    if ( !(nFlags & SCROLL_NOINVALIDATE) )
    {
        ImplCalcOverlapRegion( rRect, aInvalidateRegion, !bScrollChilds, TRUE, FALSE );
        if ( !aInvalidateRegion.IsEmpty() )
        {
            aInvalidateRegion.Move( nHorzScroll, nVertScroll );
            bErase = TRUE;
        }
        if ( !(nFlags & SCROLL_NOWINDOWINVALIDATE) )
        {
            Rectangle aDestRect( rRect );
            aDestRect.Move( nHorzScroll, nVertScroll );
            Region aWinInvalidateRegion( rRect );
            aWinInvalidateRegion.Exclude( aDestRect );
            aInvalidateRegion.Union( aWinInvalidateRegion );
        }
    }

    Point aPoint( mnOutOffX, mnOutOffY );
    Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
    if ( nFlags & SCROLL_CLIP )
        aRegion.Intersect( rRect );
    if ( mbWinRegion )
        aRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
    aRegion.Exclude( aInvalidateRegion );
    ImplClipBoundaries( aRegion, FALSE, TRUE );
    if ( !bScrollChilds )
    {
        if ( nOrgFlags & SCROLL_NOCHILDREN )
            ImplClipAllChilds( aRegion );
        else
            ImplClipChilds( aRegion );
    }
    if ( mbClipRegion && (nFlags & SCROLL_USECLIPREGION) )
        aRegion.Intersect( maRegion );
    if ( !aRegion.IsEmpty() )
    {
        if ( mpWinData )
        {
            if ( mbFocusVisible )
                ImplInvertFocus( *(mpWinData->mpFocusRect) );
            if ( mbTrackVisible && (mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                InvertTracking( *(mpWinData->mpTrackRect), mpWinData->mnTrackFlags );
        }

#ifndef REMOTE_APPSERVER
        SalGraphics* pGraphics = ImplGetFrameGraphics();
        if ( pGraphics )
        {
            ImplSelectClipRegion( pGraphics, aRegion );
            pGraphics->CopyArea( rRect.Left()+nHorzScroll, rRect.Top()+nVertScroll,
                                 rRect.Left(), rRect.Top(),
                                 rRect.GetWidth(), rRect.GetHeight(),
                                 SAL_COPYAREA_WINDOWINVALIDATE );
        }
#else
        ImplServerGraphics* pGraphics = ImplGetServerGraphics( TRUE );
        if ( pGraphics )
        {
            pGraphics->SetClipRegion( aRegion );
            pGraphics->CopyArea( rRect.Left()+nHorzScroll, rRect.Top()+nVertScroll,
                                 rRect.Left(), rRect.Top(),
                                 rRect.GetWidth(), rRect.GetHeight(),
                                 COPYAREA_WINDOWINVALIDATE );
        }
#endif

        if ( mpWinData )
        {
            if ( mbFocusVisible )
                ImplInvertFocus( *(mpWinData->mpFocusRect) );
            if ( mbTrackVisible && (mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                InvertTracking( *(mpWinData->mpTrackRect), mpWinData->mnTrackFlags );
        }
    }

    if ( !aInvalidateRegion.IsEmpty() )
    {
        USHORT nPaintFlags = INVALIDATE_CHILDREN;
        if ( !bErase )
            nPaintFlags |= INVALIDATE_NOERASE;
        if ( !bScrollChilds )
        {
            if ( nOrgFlags & SCROLL_NOCHILDREN )
                ImplClipAllChilds( aInvalidateRegion );
            else
                ImplClipChilds( aInvalidateRegion );
        }
        ImplInvalidateFrameRegion( &aInvalidateRegion, nPaintFlags );
    }

    if ( bScrollChilds )
    {
        Rectangle aDestRect( rRect );
        Window* pWindow = mpFirstChild;
        while ( pWindow )
        {
            Rectangle aWinRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
            if ( aDestRect.IsOver( aWinRect ) )
            {
                pWindow->mnX        += nHorzScroll;
                pWindow->maPos.X()  += nHorzScroll;
                pWindow->mnY        += nVertScroll;
                pWindow->maPos.Y()  += nVertScroll;
#ifndef REMOTE_APPSERVER
                if ( pWindow->ImplUpdatePos() )
                    pWindow->ImplUpdateSysObjPos();
#else
                pWindow->ImplUpdatePos();
#endif
                if ( pWindow->IsReallyVisible() )
                    pWindow->ImplSetClipFlag();
                if ( pWindow->mpClientWindow )
                    pWindow->mpClientWindow->maPos = pWindow->maPos;
                if ( pWindow->IsVisible() )
                {
                    pWindow->mbCallMove = FALSE;
                    pWindow->Move();
                }
                else
                    pWindow->mbCallMove = TRUE;
            }
            pWindow = pWindow->mpNext;
        }
    }

    if ( nFlags & SCROLL_UPDATE )
        Update();

    if ( mpCursor )
        mpCursor->ImplShow( FALSE );
}

// -----------------------------------------------------------------------

void Window::ImplUpdateAll( BOOL bOverlapWindows )
{
    if ( !mbReallyVisible )
        return;

    BOOL bFlush = FALSE;
    if ( mpFrameWindow->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mbFrame || (mpBorderWindow && mpBorderWindow->mbFrame) )
            bFlush = TRUE;
    }

    // Ein Update wirkt immer auf das OverlapWindow, damit bei spaeteren
    // Paints nicht zuviel gemalt wird, wenn dort ALLCHILDREN usw. gesetzt
    // ist
    Window* pWindow = ImplGetFirstOverlapWindow();
    if ( bOverlapWindows )
        pWindow->ImplCallOverlapPaint();
    else
    {
        if ( pWindow->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
            pWindow->ImplCallPaint( NULL, pWindow->mnPaintFlags );
    }

    if ( bFlush )
        Flush();
}

// -----------------------------------------------------------------------

void Window::ImplUpdateWindowPtr( Window* pWindow )
{
    if ( mpFrameWindow != pWindow->mpFrameWindow )
    {
        // Graphic freigeben
#ifndef REMOTE_APPSERVER
        ImplReleaseGraphics();
#else
        ImplReleaseServerGraphics();
        mpGraphics = pWindow->mpFrameWindow->mpGraphics;
#endif
    }

    mpFrameData     = pWindow->mpFrameData;
    mpFrame         = pWindow->mpFrame;
    mpFrameWindow   = pWindow->mpFrameWindow;
    if ( pWindow->ImplIsOverlapWindow() )
        mpOverlapWindow = pWindow;
    else
        mpOverlapWindow = pWindow->mpOverlapWindow;

    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( pWindow );
        pChild = pChild->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateWindowPtr()
{
    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( this );
        pChild = pChild->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateOverlapWindowPtr( BOOL bNewFrame )
{
    BOOL bVisible = IsVisible();
    Show( FALSE );
    ImplRemoveWindow( bNewFrame );
    Window* pRealParent = mpRealParent;
    ImplInsertWindow( ImplGetParent() );
    mpRealParent = pRealParent;
    ImplUpdateWindowPtr();
#ifndef REMOTE_APPSERVER
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();
#else
    ImplUpdatePos();
#endif

    if ( bNewFrame )
    {
        Window* pOverlapWindow = mpFirstOverlap;
        while ( pOverlapWindow )
        {
            Window* pNextOverlapWindow = pOverlapWindow->mpNext;
            pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }
    }

    if ( bVisible )
        Show( TRUE );
}

// -----------------------------------------------------------------------

BOOL Window::ImplUpdatePos()
{
    BOOL bSysChild = FALSE;

    if ( ImplIsOverlapWindow() )
    {
        mnOutOffX  = mnX;
        mnOutOffY  = mnY;
    }
    else
    {
        Window* pParent = ImplGetParent();
        mnOutOffX  = mnX+pParent->mnOutOffX;
        mnOutOffY  = mnY+pParent->mnOutOffY;
    }

    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        if ( pChild->ImplUpdatePos() )
            bSysChild = TRUE;
        pChild = pChild->mpNext;
    }

#ifndef REMOTE_APPSERVER
    if ( mpSysObj )
        bSysChild = TRUE;
#endif

    return bSysChild;
}

// -----------------------------------------------------------------------

#ifndef REMOTE_APPSERVER
void Window::ImplUpdateSysObjPos()
{
    if ( mpSysObj )
        mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );

    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateSysObjPos();
        pChild = pChild->mpNext;
    }
}
#endif

// -----------------------------------------------------------------------

/* --- RTL ---
void Window::ImplAlignChilds()
{
    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->ImplPosSizeWindow( pChild->maPos.X(), 0, 0, 0, WINDOW_POSSIZE_X );
        pChild = pChild->mpNext;
    }
}
*/

// -----------------------------------------------------------------------

void Window::ImplPosSizeWindow( long nX, long nY,
                                long nWidth, long nHeight, USHORT nFlags )
{
    BOOL    bNewPos         = FALSE;
    BOOL    bNewSize        = FALSE;
    BOOL    bNewWidth       = FALSE;
    BOOL    bCopyBits       = FALSE;
    long    nOldOutOffX     = mnOutOffX;
    long    nOldOutOffY     = mnOutOffY;
    long    nOldOutWidth    = mnOutWidth;
    long    nOldOutHeight   = mnOutHeight;
    Region* pOverlapRegion  = NULL;
    Region* pOldRegion      = NULL;

    if ( IsReallyVisible() )
    {
        if ( mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        Rectangle aOldWinRect( Point( nOldOutOffX, nOldOutOffY ),
                               Size( nOldOutWidth, nOldOutHeight ) );
        pOldRegion = new Region( aOldWinRect );
        if ( mbWinRegion )
            pOldRegion->Intersect( ImplPixelToDevicePixel( maWinRegion ) );

        if ( mnOutWidth && mnOutHeight && !mbPaintTransparent &&
             !mbInitWinClipRegion && !maWinClipRegion.IsEmpty() &&
             !HasPaintEvent() )
            bCopyBits = TRUE;
    }

    if ( nFlags & WINDOW_POSSIZE_WIDTH )
    {
        if ( nWidth < 0 )
            nWidth = 0;
        if ( nWidth != mnOutWidth )
        {
/* --- RTL ---
            if ( !ImplIsOverlapWindow() )
            {
                if ( !(nFlags & WINDOW_POSSIZE_X) )
                {
                    nFlags |= WINDOW_POSSIZE_X;
                    nX = mnX;
                }
            }
*/
            mnOutWidth = nWidth;
            bNewSize = TRUE;
            bCopyBits = FALSE;
            bNewWidth = TRUE;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_HEIGHT )
    {
        if ( nHeight < 0 )
            nHeight = 0;
        if ( nHeight != mnOutHeight )
        {
            mnOutHeight = nHeight;
            bNewSize = TRUE;
            bCopyBits = FALSE;
        }
    }

    if ( nFlags & WINDOW_POSSIZE_X )
    {
        if ( nX != mnX )
        {
            long nOrgX = nX;
/* --- RTL ---
            if ( !ImplIsOverlapWindow() )
                nX = mpParent->mnOutWidth-mnOutWidth-nX;
*/

            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, FALSE, TRUE, TRUE );
            }
            mnX = nX;
            maPos.X() = nOrgX;
            bNewPos = TRUE;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_Y )
    {
        if ( nY != mnY )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, FALSE, TRUE, TRUE );
            }
            mnY = nY;
            maPos.Y() = nY;
            bNewPos = TRUE;
        }
    }

    if ( bNewPos || bNewSize )
    {
#ifndef REMOTE_APPSERVER
        BOOL bUpdateSysObjPos = FALSE;
        if ( bNewPos )
            bUpdateSysObjPos = ImplUpdatePos();
#else
        if ( bNewPos )
            ImplUpdatePos();
#endif

        if ( mpClientWindow )
        {
            mpClientWindow->ImplPosSizeWindow( mpClientWindow->mnLeftBorder,
                                               mpClientWindow->mnTopBorder,
                                               mnOutWidth-mpClientWindow->mnLeftBorder-mpClientWindow->mnRightBorder,
                                               mnOutHeight-mpClientWindow->mnTopBorder-mpClientWindow->mnBottomBorder,
                                               WINDOW_POSSIZE_X | WINDOW_POSSIZE_Y |
                                               WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT );
            // Wenn wir ein ClientWindow haben, dann hat dieses fuer die
            // Applikation auch die Position des FloatingWindows
            mpClientWindow->maPos = maPos;
            if ( bNewPos )
            {
                if ( mpClientWindow->IsVisible() )
                {
                    mpClientWindow->mbCallMove = FALSE;
                    mpClientWindow->Move();
                }
                else
                    mpClientWindow->mbCallMove = TRUE;
            }
        }
        else
        {
            if ( mpBorderWindow )
                maPos = mpBorderWindow->maPos;
        }

/* --- RTL ---
        if ( bNewWidth && !ImplIsOverlapWindow() )
            ImplAlignChilds();
*/

        // Move()/Resize() werden erst bei Show() gerufen, damit min. eins vor
        // einem Show() kommt
        if ( IsVisible() )
        {
            if ( bNewPos )
            {
                mbCallMove = FALSE;
                Move();
            }
            if ( bNewSize )
            {
                mbCallResize = FALSE;
                Resize();
            }
        }
        else
        {
            if ( bNewPos )
                mbCallMove = TRUE;
            if ( bNewSize )
                mbCallResize = TRUE;
        }

#ifndef REMOTE_APPSERVER
        BOOL bUpdateSysObjClip = FALSE;
#endif
        if ( IsReallyVisible() )
        {
            if ( bNewPos || bNewSize )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpOverlapData && mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                // Clip-Flag neu setzen
#ifndef REMOTE_APPSERVER
                bUpdateSysObjClip = !ImplSetClipFlag( TRUE );
#else
                ImplSetClipFlag();
#endif
            }

            // Fensterinhalt invalidieren ?
            if ( bNewPos || (mnOutWidth > nOldOutWidth) || (mnOutHeight > nOldOutHeight) )
            {
                if ( bNewPos )
                {
                    BOOL bInvalidate = FALSE;
                    BOOL bParentPaint = TRUE;
                    if ( !ImplIsOverlapWindow() )
                        bParentPaint = mpParent->IsPaintEnabled();
                    if ( bCopyBits && bParentPaint && !HasPaintEvent() )
                    {
                        Point aPoint( mnOutOffX, mnOutOffY );
                        Region aRegion( Rectangle( aPoint,
                                                   Size( mnOutWidth, mnOutHeight ) ) );
                        if ( mbWinRegion )
                            aRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
                        ImplClipBoundaries( aRegion, FALSE, TRUE );
                        if ( !pOverlapRegion->IsEmpty() )
                        {
                            pOverlapRegion->Move( mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY );
                            aRegion.Exclude( *pOverlapRegion );
                        }
                        if ( !aRegion.IsEmpty() )
                        {
                            // Paint-Bereiche anpassen
                            ImplMoveAllInvalidateRegions( Rectangle( Point( nOldOutOffX, nOldOutOffY ),
                                                                     Size( nOldOutWidth, nOldOutHeight ) ),
                                                          mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY,
                                                          TRUE );
#ifndef REMOTE_APPSERVER
                            SalGraphics* pGraphics = ImplGetFrameGraphics();
                            if ( pGraphics )
                            {
                                BOOL bSelectClipRegion = ImplSelectClipRegion( pGraphics, aRegion );
                                if ( bSelectClipRegion )
                                {
                                    pGraphics->CopyArea( mnOutOffX, mnOutOffY,
                                                         nOldOutOffX, nOldOutOffY,
                                                         nOldOutWidth, nOldOutHeight,
                                                         SAL_COPYAREA_WINDOWINVALIDATE );
                                }
                                else
                                    bInvalidate = TRUE;
                            }
                            else
                                bInvalidate = TRUE;
#else
                            ImplServerGraphics* pGraphics = ImplGetServerGraphics( TRUE );
                            if ( pGraphics )
                            {
                                pGraphics->SetClipRegion( aRegion );
                                pGraphics->CopyArea( mnOutOffX, mnOutOffY,
                                                     nOldOutOffX, nOldOutOffY,
                                                     nOldOutWidth, nOldOutHeight,
                                                     COPYAREA_WINDOWINVALIDATE );
                            }
                            else
                                bInvalidate = TRUE;
#endif
                            if ( !bInvalidate )
                            {
                                if ( !pOverlapRegion->IsEmpty() )
                                    ImplInvalidateFrameRegion( pOverlapRegion, INVALIDATE_CHILDREN );
                            }
                        }
                    }
                    else
                        bInvalidate = TRUE;
                    if ( bInvalidate )
                        ImplInvalidateFrameRegion( NULL, INVALIDATE_CHILDREN );
                }
                else
                {
                    Point aPoint( mnOutOffX, mnOutOffY );
                    Region aRegion( Rectangle( aPoint,
                                               Size( mnOutWidth, mnOutHeight ) ) );
                    aRegion.Exclude( *pOldRegion );
                    if ( mbWinRegion )
                        aRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
                    ImplClipBoundaries( aRegion, FALSE, TRUE );
                    if ( !aRegion.IsEmpty() )
                        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );
                }
            }

            // Parent oder Overlaps invalidieren
            if ( bNewPos ||
                 (mnOutWidth < nOldOutWidth) || (mnOutHeight < nOldOutHeight) )
            {
                Region aRegion( *pOldRegion );
                if ( !mbPaintTransparent )
                    ImplExcludeWindowRegion( aRegion );
                ImplClipBoundaries( aRegion, FALSE, TRUE );
                if ( !aRegion.IsEmpty() && !mpBorderWindow )
                    ImplInvalidateParentFrameRegion( aRegion );
            }
        }

#ifndef REMOTE_APPSERVER
        // System-Objekte anpassen
        if ( bUpdateSysObjClip )
            ImplUpdateSysObjClip();
        if ( bUpdateSysObjPos )
            ImplUpdateSysObjPos();
        if ( bNewSize && mpSysObj )
            mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );
#endif
    }

    if ( pOverlapRegion )
        delete pOverlapRegion;
    if ( pOldRegion )
        delete pOldRegion;
}

// -----------------------------------------------------------------------

void Window::ImplToBottomChild()
{
    if ( !ImplIsOverlapWindow() && !mbReallyVisible && (mpParent->mpLastChild != this) )
    {
        // Fenster an das Ende der Liste setzen
        if ( mpPrev )
            mpPrev->mpNext = mpNext;
        else
            mpParent->mpFirstChild = mpNext;
        mpNext->mpPrev = mpPrev;
        mpPrev = mpParent->mpLastChild;
        mpParent->mpLastChild = this;
        mpPrev->mpNext = this;
        mpNext = NULL;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCalcToTop( ImplCalcToTopData* pPrevData )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplCalcToTop(): Is not a OverlapWindow" );

    if ( !mbFrame )
    {
        if ( IsReallyVisible() )
        {
            // Region berechnen, wo das Fenster mit anderen Fenstern ueberlappt
            Point aPoint( mnOutOffX, mnOutOffY );
            Region  aRegion( Rectangle( aPoint,
                                        Size( mnOutWidth, mnOutHeight ) ) );
            Region  aInvalidateRegion;
            ImplCalcOverlapRegionOverlaps( aRegion, aInvalidateRegion );

            if ( !aInvalidateRegion.IsEmpty() )
            {
                ImplCalcToTopData* pData    = new ImplCalcToTopData;
                pPrevData->mpNext           = pData;
                pData->mpNext               = NULL;
                pData->mpWindow             = this;
                pData->mpInvalidateRegion   = new Region( aInvalidateRegion );
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplCalcChildOverlapToTop( ImplCalcToTopData* pPrevData )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplCalcChildOverlapToTop(): Is not a OverlapWindow" );

    ImplCalcToTop( pPrevData );
    if ( pPrevData->mpNext )
        pPrevData = pPrevData->mpNext;

    Window* pOverlap = mpFirstOverlap;
    while ( pOverlap )
    {
        pOverlap->ImplCalcToTop( pPrevData );
        if ( pPrevData->mpNext )
            pPrevData = pPrevData->mpNext;
        pOverlap = pOverlap->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplToTop( USHORT nFlags )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplToTop(): Is not a OverlapWindow" );

    if ( mbFrame )
    {
        // Wenn in das externe Fenster geklickt wird, ist dieses
        // dafuer zustaendig dafuer zu sorgen, das unser Frame
        // nach vorne kommt
        if ( !mpFrameData->mbHasFocus &&
             !mpFrameData->mbSysObjFocus &&
             !mpFrameData->mbInSysObjFocusHdl &&
             !mpFrameData->mbInSysObjToTopHdl )
        {
#ifndef REMOTE_APPSERVER
            USHORT nSysFlags = 0;
            if ( nFlags & TOTOP_RESTOREWHENMIN )
                nSysFlags = SAL_FRAME_TOTOP_RESTOREWHENMIN;
            if ( nFlags & TOTOP_FOREGROUNDTASK )
                nSysFlags = SAL_FRAME_TOTOP_FOREGROUNDTASK;
            mpFrame->ToTop( nSysFlags );
#else
            mpFrame->ToTop( nFlags );
#endif
        }
    }
    else
    {
        if ( mpOverlapWindow->mpFirstOverlap != this )
        {
            // Fenster aus der Liste entfernen
            mpPrev->mpNext = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpOverlapWindow->mpLastOverlap = mpPrev;

            // AlwaysOnTop beruecksichtigen
            BOOL    bOnTop = IsAlwaysOnTopEnabled();
            Window* pNextWin = mpOverlapWindow->mpFirstOverlap;
            if ( !bOnTop )
            {
                while ( pNextWin )
                {
                    if ( !pNextWin->IsAlwaysOnTopEnabled() )
                        break;
                    pNextWin = pNextWin->mpNext;
                }
            }

            // TopLevel abpruefen
            BYTE nTopLevel = mpOverlapData->mnTopLevel;
            while ( pNextWin )
            {
                if ( (bOnTop != pNextWin->IsAlwaysOnTopEnabled()) ||
                     (nTopLevel <= pNextWin->mpOverlapData->mnTopLevel) )
                    break;
                pNextWin = pNextWin->mpNext;
            }

            // Fenster in die Liste wieder eintragen
            mpNext = pNextWin;
            if ( pNextWin )
            {
                mpPrev = pNextWin->mpPrev;
                pNextWin->mpPrev = this;
            }
            else
            {
                mpPrev = mpOverlapWindow->mpLastOverlap;
                mpOverlapWindow->mpLastOverlap = this;
            }
            if ( mpPrev )
                mpPrev->mpNext = this;
            else
                mpOverlapWindow->mpFirstOverlap = this;

            // ClipRegion muss von diesem Fenster und allen weiteren
            // ueberlappenden Fenstern neu berechnet werden.
            if ( IsReallyVisible() )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                mpOverlapWindow->ImplSetClipFlagOverlapWindows();
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplStartToTop( USHORT nFlags )
{
    ImplCalcToTopData   aStartData;
    ImplCalcToTopData*  pCurData;
    ImplCalcToTopData*  pNextData;
    Window* pOverlapWindow;
    if ( ImplIsOverlapWindow() )
        pOverlapWindow = this;
    else
        pOverlapWindow = mpOverlapWindow;

    // Zuerst die Paint-Bereiche berechnen
    Window* pTempOverlapWindow = pOverlapWindow;
    aStartData.mpNext = NULL;
    pCurData = &aStartData;
    do
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mbFrame );
    // Dann die Paint-Bereiche der ChildOverlap-Windows berechnen
    pTempOverlapWindow = mpFirstOverlap;
    while ( pTempOverlapWindow )
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpNext;
    }

    // Dann die Fenster-Verkettung aendern
    pTempOverlapWindow = pOverlapWindow;
    do
    {
        pTempOverlapWindow->ImplToTop( nFlags );
        pTempOverlapWindow = pTempOverlapWindow->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mbFrame );
    // Und zum Schluss invalidieren wir die ungueltigen Bereiche
    pCurData = aStartData.mpNext;
    while ( pCurData )
    {
        pCurData->mpWindow->ImplInvalidateFrameRegion( pCurData->mpInvalidateRegion, INVALIDATE_CHILDREN );
        pNextData = pCurData->mpNext;
        delete pCurData->mpInvalidateRegion;
        delete pCurData;
        pCurData = pNextData;
    }
}

// -----------------------------------------------------------------------

void Window::ImplFocusToTop( USHORT nFlags, BOOL bReallyVisible )
{
    // Soll Focus auch geholt werden?
    if ( !(nFlags & TOTOP_NOGRABFOCUS) )
    {
        // Erstes Fenster mit GrabFocus-Activate bekommt den Focus
        Window* pFocusWindow = this;
        while ( !pFocusWindow->ImplIsOverlapWindow() )
        {
            // Nur wenn Fenster kein Border-Fenster hat, da wir
            // immer das dazugehoerende BorderFenster finden wollen
            if ( !pFocusWindow->mpBorderWindow )
            {
                if ( pFocusWindow->mnActivateMode & ACTIVATE_MODE_GRABFOCUS )
                    break;
            }
            pFocusWindow = pFocusWindow->ImplGetParent();
        }
        if ( (pFocusWindow->mnActivateMode & ACTIVATE_MODE_GRABFOCUS) &&
             !pFocusWindow->HasChildPathFocus( TRUE ) )
            pFocusWindow->GrabFocus();
    }

    if ( bReallyVisible )
        ImplGenerateMouseMove();
}

// -----------------------------------------------------------------------

void Window::ImplShowAllOverlaps()
{
    Window* pOverlapWindow = mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->mbOverlapVisible )
        {
            pOverlapWindow->Show( TRUE, SHOW_NOACTIVATE );
            pOverlapWindow->mbOverlapVisible = FALSE;
        }

        pOverlapWindow = pOverlapWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplHideAllOverlaps()
{
    Window* pOverlapWindow = mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->IsVisible() )
        {
            pOverlapWindow->mbOverlapVisible = TRUE;
            pOverlapWindow->Show( FALSE );
        }

        pOverlapWindow = pOverlapWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallMouseMove( USHORT nMouseCode, BOOL bModChanged )
{
    if ( mpFrameData->mbMouseIn && mpFrameWindow->mbReallyVisible )
    {
        ULONG   nTime   = Time::GetSystemTicks();
        long    nX      = mpFrameData->mnLastMouseX;
        long    nY      = mpFrameData->mnLastMouseY;
        USHORT  nCode   = nMouseCode;
        USHORT  nMode   = mpFrameData->mnMouseMode;
        BOOL    bLeave;
        // Auf MouseLeave testen
        if ( ((nX < 0) || (nY < 0) ||
              (nX >= mpFrameWindow->mnOutWidth) ||
              (nY >= mpFrameWindow->mnOutHeight)) &&
             !ImplGetSVData()->maWinData.mpCaptureWin )
            bLeave = TRUE;
        else
            bLeave = FALSE;
        nMode |= MOUSE_SYNTHETIC;
        if ( bModChanged )
            nMode |= MOUSE_MODIFIERCHANGED;
        ImplHandleMouseEvent( mpFrameWindow, EVENT_MOUSEMOVE, bLeave, nX, nY, nTime, nCode, nMode );
    }
}

// -----------------------------------------------------------------------

void Window::ImplGenerateMouseMove()
{
    if ( !mpFrameData->mnMouseMoveId )
        Application::PostUserEvent( mpFrameData->mnMouseMoveId, LINK( mpFrameWindow, Window, ImplGenerateMouseMoveHdl ) );
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplGenerateMouseMoveHdl, void*, EMPTYARG )
{
    mpFrameData->mnMouseMoveId = 0;
    ImplCallMouseMove( mpFrameData->mnMouseCode );
    return 0;
}

// -----------------------------------------------------------------------

void Window::ImplInvertFocus( const Rectangle& rRect )
{
    InvertTracking( rRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
}

// -----------------------------------------------------------------------

void Window::ImplCallFocusChangeActivate( Window* pNewOverlapWindow,
                                          Window* pOldOverlapWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pNewRealWindow;
    Window*     pOldRealWindow;
    Window*     pLastRealWindow;
    BOOL        bCallActivate = TRUE;
    BOOL        bCallDeactivate = TRUE;

    pOldRealWindow = pOldOverlapWindow->ImplGetWindow();
    pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
    if ( (pOldRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
         pOldRealWindow->GetActivateMode() )
    {
        if ( (pNewRealWindow->GetType() == WINDOW_FLOATINGWINDOW) &&
             !pNewRealWindow->GetActivateMode() )
        {
            pSVData->maWinData.mpLastDeacWin = pOldOverlapWindow;
            bCallDeactivate = FALSE;
        }
    }
    else if ( (pNewRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
              pNewRealWindow->GetActivateMode() )
    {
        if ( pSVData->maWinData.mpLastDeacWin )
        {
            if ( pSVData->maWinData.mpLastDeacWin == pNewOverlapWindow )
                bCallActivate = FALSE;
            else
            {
                pLastRealWindow = pSVData->maWinData.mpLastDeacWin->ImplGetWindow();
                pSVData->maWinData.mpLastDeacWin->mbActive = FALSE;
                pSVData->maWinData.mpLastDeacWin->Deactivate();
                if ( pLastRealWindow != pSVData->maWinData.mpLastDeacWin )
                {
                    pLastRealWindow->mbActive = TRUE;
                    pLastRealWindow->Activate();
                }
            }
            pSVData->maWinData.mpLastDeacWin = NULL;
        }
    }

    if ( bCallDeactivate )
    {
        pOldOverlapWindow->mbActive = FALSE;
        pOldOverlapWindow->Deactivate();
        if ( pOldRealWindow != pOldOverlapWindow )
        {
            pOldRealWindow->mbActive = FALSE;
            pOldRealWindow->Deactivate();
        }
    }
    if ( bCallActivate )
    {
        pNewOverlapWindow->mbActive = TRUE;
        pNewOverlapWindow->Activate();
        if ( pNewRealWindow != pNewOverlapWindow )
        {
            pNewRealWindow->mbActive = TRUE;
            pNewRealWindow->Activate();
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplGrabFocus( USHORT nFlags )
{
    // Es soll immer das Client-Fenster den Focus bekommen. Falls
    // wir mal auch Border-Fenstern den Focus geben wollen,
    // muessten wir bei allen GrabFocus()-Aufrufen in VCL dafuer
    // sorgen, das es an der Stelle gemacht wird. Dies wuerde
    // beispielsweise bei ToTop() der Fall sein.
    if ( mpClientWindow )
    {
        // Wegen nicht ganz durchdachtem Konzept muessen wir hier
        // leider noch einen Hack einbauen, damit wenn Dialoge
        // geschlossen werden der Focus wieder auf das richtige
        // Fenster zurueckgesetzt wird
        if ( mpLastFocusWindow && (mpLastFocusWindow != this) &&
             !(mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
             mpLastFocusWindow->IsEnabled() )
            mpLastFocusWindow->GrabFocus();
        else
            mpClientWindow->GrabFocus();
        return;
    }
    else if ( mbFrame )
    {
        // Wegen nicht ganz durchdachtem Konzept muessen wir hier
        // leider noch einen Hack einbauen, damit wenn Dialoge
        // geschlossen werden der Focus wieder auf das richtige
        // Fenster zurueckgesetzt wird
        if ( mpLastFocusWindow && (mpLastFocusWindow != this) &&
             !(mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
             mpLastFocusWindow->IsEnabled() )
        {
            mpLastFocusWindow->GrabFocus();
            return;
        }
    }

    // Wir brauchen Focus nur setzen, wenn es diesen noch nicht hat
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpFocusWin != this )
    {
        // Dieses Fenster als letztes FocusWindow merken
        Window* pOverlapWindow = ImplGetFirstOverlapWindow();
        pOverlapWindow->mpLastFocusWindow = this;
        mpFrameData->mpFocusWin = this;

#ifndef REMOTE_APPSERVER
        if ( !mpSysObj && !mpFrameData->mbHasFocus )
#else
        if ( !mpFrameData->mbHasFocus )
#endif
        {
            // Hier setzen wir schon den Focus um, da ToTop() den Focus
            // nicht auf ein anderes Fenster setzen darf
            DBG_WARNING( "Window::GrabFocus() - Frame doesn't have the focus" );
            mpFrame->ToTop( 0 );
            return;
        }

        Window* pOldFocusWindow = pSVData->maWinData.mpFocusWin;
        pSVData->maWinData.mpFocusWin = this;

        if ( pOldFocusWindow )
        {
            // Cursor hiden
            if ( pOldFocusWindow->mpCursor )
                pOldFocusWindow->mpCursor->ImplHide();
        }

        // !!!!! Wegen altem SV-Office Activate/Deavtivate Handling
        // !!!!! erstmal so wie frueher
        if ( pOldFocusWindow )
        {
            // Focus merken
            Window* pOldOverlapWindow = pOldFocusWindow->ImplGetFirstOverlapWindow();
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            if ( pOldOverlapWindow != pNewOverlapWindow )
                ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
        }
        else
        {
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            Window* pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
            pNewOverlapWindow->mbActive = TRUE;
            pNewOverlapWindow->Activate();
            if ( pNewRealWindow != pNewOverlapWindow )
            {
                pNewRealWindow->mbActive = TRUE;
                pNewRealWindow->Activate();
            }
        }
/*
        // call Deactivate and Activate
        Window* pDeactivateParent;
        Window* pActivateParent;
        Window* pParent;
        Window* pLastParent;
        pDeactivateParent = pOldFocusWindow;
        while ( pDeactivateParent )
        {
            pParent = pDeactivateParent;
            if ( pParent->ImplIsChild( this ) )
                break;

            if ( pDeactivateParent->ImplIsOverlapWindow() )
            {
                if ( !pDeactivateParent->mbParentActive )
                    break;
            }

            pDeactivateParent = pDeactivateParent->ImplGetParent();
        }
        if ( pOldFocusWindow )
        {
            pActivateParent = this;
            while ( pActivateParent )
            {
                pParent = pActivateParent;
                if ( pParent->ImplIsChild( pOldFocusWindow ) )
                    break;

                if ( pActivateParent->ImplIsOverlapWindow() )
                {
                    if ( !pActivateParent->mbParentActive )
                        break;
                }

                pActivateParent = pActivateParent->ImplGetParent();
            }
        }
        else
        {
            if ( ImplIsOverlapWindow() )
                pActivateParent = this;
            else
                pActivateParent = mpOverlapWindow;
            while ( pActivateParent )
            {
                if ( pActivateParent->ImplIsOverlapWindow() )
                {
                    if ( !pActivateParent->mbParentActive )
                        break;
                }

                pActivateParent = pActivateParent->ImplGetParent();
            }
        }
        if ( pDeactivateParent )
        {
            do
            {
                pLastParent = pOldFocusWindow;
                if ( pLastParent != pDeactivateParent )
                {
                    pParent = pLastParent->ImplGetParent();
                    while ( pParent )
                    {
                        if ( pParent == pDeactivateParent )
                            break;
                        pLastParent = pParent;
                        pParent = pParent->ImplGetParent();
                    }
                }
                else
                    pParent = pLastParent;

                pParent->mbActive = FALSE;
                pParent->Deactivate();
                pDeactivateParent = pLastParent;
            }
            while ( pDeactivateParent != pOldFocusWindow );
        }
        do
        {
            pLastParent = this;
            if ( pLastParent != pActivateParent )
            {
                pParent = pLastParent->ImplGetParent();
                while ( pParent )
                {
                    if ( pParent == pActivateParent )
                        break;
                    pLastParent = pParent;
                    pParent = pParent->ImplGetParent();
                }
            }
            else
                pParent = pLastParent;

            pParent->mbActive = TRUE;
            pParent->Activate();
            pActivateParent = pLastParent;
        }
        while ( pActivateParent != this );
*/
        // call Get- and LoseFocus
        if ( pOldFocusWindow )
        {
            if ( pOldFocusWindow->IsTracking() &&
                 (pSVData->maWinData.mnTrackFlags & STARTTRACK_FOCUSCANCEL) )
                pOldFocusWindow->EndTracking( ENDTRACK_CANCEL | ENDTRACK_FOCUS );
            pOldFocusWindow->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );
            NotifyEvent aNEvt( EVENT_LOSEFOCUS, pOldFocusWindow );
            if ( !ImplCallPreNotify( aNEvt ) )
                pOldFocusWindow->LoseFocus();
            pOldFocusWindow->ImplCallDeactivateListeners( this );
        }

        if ( pSVData->maWinData.mpFocusWin == this )
        {
            // EndExtTextInput if it is not the same window
            if ( mpFrameData->mpExtTextInputWin &&
                 mpFrameData->mpExtTextInputWin != this )
                mpFrameData->mpExtTextInputWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );

#ifndef REMOTE_APPSERVER
            if ( mpSysObj )
            {
                mpFrameData->mpFocusWin = this;
                if ( !mpFrameData->mbInSysObjFocusHdl )
                    mpSysObj->GrabFocus();
            }
#endif

            if ( pSVData->maWinData.mpFocusWin == this )
            {
                if ( mpCursor )
                    mpCursor->ImplShow();
                mbInFocusHdl = TRUE;
                mnGetFocusFlags = nFlags;
                NotifyEvent aNEvt( EVENT_GETFOCUS, this );
                if ( !ImplCallPreNotify( aNEvt ) )
                    GetFocus();
                ImplCallActivateListeners( pOldFocusWindow );
                mnGetFocusFlags = 0;
                mbInFocusHdl = FALSE;
            }
        }

        GetpApp()->FocusChanged();
        ImplNewInputContext();
    }
}

// -----------------------------------------------------------------------

void Window::ImplNewInputContext()
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFocusWin = pSVData->maWinData.mpFocusWin;
    if ( !pFocusWin )
        return;

    // Is InputContext changed?
    const InputContext& rInputContext = pFocusWin->GetInputContext();
    if ( rInputContext == pFocusWin->mpFrameData->maOldInputContext )
        return;

    pFocusWin->mpFrameData->maOldInputContext = rInputContext;

#ifndef REMOTE_APPSERVER
    SalInputContext         aNewContext;
    const Font&             rFont = rInputContext.GetFont();
    const XubString&        rFontName = rFont.GetName();
    ImplFontEntry*          pFontEntry = NULL;
    aNewContext.mpFont = NULL;
    if ( rFontName.Len() )
    {
        Size aSize = pFocusWin->ImplLogicToDevicePixel( rFont.GetSize() );
        if ( !aSize.Height() )
        {
            // Nur dann Defaultgroesse setzen, wenn Fonthoehe auch in logischen
            // Koordinaaten 0 ist
            if ( rFont.GetSize().Height() )
                aSize.Height() = 1;
            else
                aSize.Height() = (12*pFocusWin->mnDPIY)/72;
        }
        pFontEntry = pFocusWin->mpFontCache->Get( pFocusWin->mpFontList, rFont, aSize );
        if ( pFontEntry )
            aNewContext.mpFont = &pFontEntry->maFontSelData;
    }
    aNewContext.meLanguage  = rFont.GetLanguage();
    aNewContext.mnOptions   = rInputContext.GetOptions();
    pFocusWin->ImplGetFrame()->SetInputContext( &aNewContext );

    if ( pFontEntry )
        pFocusWin->mpFontCache->Release( pFontEntry );
#else
    // !!!
#endif
}

// -----------------------------------------------------------------------

Window::Window( WindowType nType ) :
    maZoom( 1, 1 ),
    maWinRegion( REGION_NULL ),
    maWinClipRegion( REGION_NULL )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitData( nType );
}

// -----------------------------------------------------------------------

Window::Window( Window* pParent, WinBits nStyle ) :
    maZoom( 1, 1 ),
    maWinRegion( REGION_NULL ),
    maWinClipRegion( REGION_NULL )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

Window::Window( Window* pParent, const ResId& rResId ) :
    maZoom( 1, 1 ),
    maWinRegion( REGION_NULL ),
    maWinClipRegion( REGION_NULL )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitData( WINDOW_WINDOW );
    rResId.SetRT( RSC_WINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

Window::~Window()
{
    DBG_DTOR( Window, ImplDbgCheckWindow );

    mbInDtor = TRUE;

    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    if ( pWrapper )
        pWrapper->WindowDestroyed( this );

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mpHelpWin && (pSVData->maHelpData.mpHelpWin->GetParent() == this) )
        ImplDestroyHelpWindow( FALSE );

    DBG_ASSERT( pSVData->maWinData.mpTrackWin != this,
                "Window::~Window(): Window is in TrackingMode" );
    DBG_ASSERT( pSVData->maWinData.mpCaptureWin != this,
                "Window::~Window(): Window has the mouse captured" );
    DBG_ASSERT( pSVData->maWinData.mpDefDialogParent != this,
                "Window::~Window(): Window is DefModalDialogParent" );

    // Wegen alter kompatibilitaet
    if ( pSVData->maWinData.mpTrackWin == this )
        EndTracking();
    if ( pSVData->maWinData.mpCaptureWin == this )
        ReleaseMouse();
    if ( pSVData->maWinData.mpDefDialogParent == this )
        pSVData->maWinData.mpDefDialogParent = NULL;

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        ByteString  aErrorStr;
        BOOL        bError = FALSE;
        Window*     pTempWin = mpFrameData->mpFirstOverlap;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = TRUE;
                if ( aErrorStr.Len() )
                    aErrorStr += "; ";
                aErrorStr += ByteString( pTempWin->GetText(), RTL_TEXTENCODING_UTF8 );
            }
            pTempWin = pTempWin->mpNextOverlap;
        }
        if ( bError )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living SystemWindow(s) destroyed: ";
            aTempStr += aErrorStr;
            DBG_ERROR( aTempStr.GetBuffer() );
        }

        if ( mpFirstChild )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living Child(s) destroyed: ";
            Window* pTempWin = mpFirstChild;
            while ( pTempWin )
            {
                aTempStr += ByteString( pTempWin->GetText(), RTL_TEXTENCODING_UTF8 );
                pTempWin = pTempWin->mpNext;
                if ( pTempWin )
                    aTempStr += "; ";
            }
            DBG_ERROR( aTempStr.GetBuffer() );
        }

        if ( mpFirstOverlap )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living SystemWindow(s) destroyed: ";
            Window* pTempWin = mpFirstOverlap;
            while ( pTempWin )
            {
                aTempStr += ByteString( pTempWin->GetText(), RTL_TEXTENCODING_UTF8 );
                pTempWin = pTempWin->mpNext;
                if ( pTempWin )
                    aTempStr += "; ";
            }
            DBG_ERROR( aTempStr.GetBuffer() );
        }
    }
#endif

    // Fenster hiden, um das entsprechende Paint-Handling auszuloesen
    Hide();

    // Mitteilen, das Fenster zerstoert wird
    {
    NotifyEvent aNEvt( EVENT_DESTROY, this );
    Notify( aNEvt );
    }

    // Wenn wir den Focus haben, dann den Focus auf ein anderes Fenster setzen
    Window* pOverlapWindow = ImplGetFirstOverlapWindow();
    if ( pSVData->maWinData.mpFocusWin == this )
    {
        if ( mbFrame )
        {
            pSVData->maWinData.mpFocusWin = NULL;
            pOverlapWindow->mpLastFocusWindow = NULL;
            GetpApp()->FocusChanged();
        }
        else
        {
            Window* pParent = GetParent();
            Window* pBorderWindow = mpBorderWindow;
            // Bei ueberlappenden Fenstern wird der Focus auf den
            // Parent vom naechsten FrameWindow gesetzt
            if ( pBorderWindow )
            {
                if ( pBorderWindow->ImplIsOverlapWindow() )
                    pParent = pBorderWindow->mpOverlapWindow;
            }
            else if ( ImplIsOverlapWindow() )
                pParent = mpOverlapWindow;
            if ( pParent && pParent->IsEnabled() && pParent->IsInputEnabled() )
                pParent->GrabFocus();
            else
                mpFrameWindow->GrabFocus();
            // Falls der Focus wieder auf uns gesetzt wurde, dann wird er
            // auf nichts gesetzt
            if ( pSVData->maWinData.mpFocusWin == this )
            {
                pSVData->maWinData.mpFocusWin = NULL;
                pOverlapWindow->mpLastFocusWindow = NULL;
                GetpApp()->FocusChanged();
            }
        }
    }
    if ( pOverlapWindow->mpLastFocusWindow == this )
        pOverlapWindow->mpLastFocusWindow = NULL;

    // gemerkte Fenster zuruecksetzen
    if ( mpFrameData->mpFocusWin == this )
        mpFrameData->mpFocusWin = NULL;
    if ( mpFrameData->mpExtTextInputWin == this )
        mpFrameData->mpExtTextInputWin = NULL;
    if ( mpFrameData->mpMouseMoveWin == this )
        mpFrameData->mpMouseMoveWin = NULL;
    if ( mpFrameData->mpMouseDownWin == this )
        mpFrameData->mpMouseDownWin = NULL;

    // Deactivate-Window zuruecksetzen
    if ( pSVData->maWinData.mpLastDeacWin == this )
        pSVData->maWinData.mpLastDeacWin = NULL;

#ifdef REMOTE_APPSERVER
    {
    // Events als ungueltig markieren...
    VOS_NAMESPACE(OGuard,vos) aGuard( pSVData->mpWindowObjectMutex );
    if ( mpRmEvents )
    {
        ExtRmEvent* p = mpRmEvents;
        while ( p )
        {
            p->MarkInvalid();
            p = p->GetNextWindowEvent();
        }
    }
    }
#endif

    if ( mbFrame )
    {
#ifndef REMOTE_APPSERVER
        DragManager::SystemEnableDrop( mpFrame, FALSE);
#endif
        if ( mpFrameData->mnFocusId )
            Application::RemoveUserEvent( mpFrameData->mnFocusId );
        if ( mpFrameData->mnMouseMoveId )
            Application::RemoveUserEvent( mpFrameData->mnMouseMoveId );
    }

    // Graphic freigeben
#ifndef REMOTE_APPSERVER
    ImplReleaseGraphics();
#else
    ImplReleaseServerGraphics();
#endif

    // Evt. anderen Funktion mitteilen, das das Fenster geloescht
    // wurde
    ImplDelData* pDelData = mpFirstDel;
    while ( pDelData )
    {
        pDelData->mbDel = TRUE;
        pDelData = pDelData->mpNext;
    }

    // Fenster aus den Listen austragen
    ImplRemoveWindow( TRUE );

    // Extra Window Daten loeschen
    if ( mpWinData )
    {
        if ( mpWinData->mpExtOldText )
            delete mpWinData->mpExtOldText;
        if ( mpWinData->mpExtOldAttrAry )
            delete mpWinData->mpExtOldAttrAry;
        if ( mpWinData->mpCursorRect )
            delete mpWinData->mpCursorRect;
        if ( mpWinData->mpFocusRect )
            delete mpWinData->mpFocusRect;
        if ( mpWinData->mpTrackRect )
            delete mpWinData->mpTrackRect;
        delete mpWinData;
    }

    // Overlap-Window-Daten loeschen
    if ( mpOverlapData )
    {
        delete mpOverlapData;
    }

    // Evt. noch BorderWindow oder Frame zerstoeren
    if ( mpBorderWindow )
        delete mpBorderWindow;
    else if ( mbFrame )
    {
        if ( pSVData->maWinData.mpFirstFrame == this )
            pSVData->maWinData.mpFirstFrame = mpFrameData->mpNextFrame;
        else
        {
            Window* pSysWin = pSVData->maWinData.mpFirstFrame;
            while ( pSysWin->mpFrameData->mpNextFrame != this )
                pSysWin = pSysWin->mpFrameData->mpNextFrame;
            pSysWin->mpFrameData->mpNextFrame = mpFrameData->mpNextFrame;
        }
#ifndef REMOTE_APPSERVER
        mpFrame->SetCallback( NULL, NULL );
        pSVData->mpDefInst->DestroyFrame( mpFrame );
#else
        mpGraphics->SetInterface( REF( NMSP_CLIENT::XRmOutputDevice )() );
        delete mpFrame;
#endif
        if ( mpFrameData->mpDragTimer )
            delete mpFrameData->mpDragTimer;
        delete mpFrameData;
#ifdef REMOTE_APPSERVER
        delete mpGraphics;
#endif
    }

    if ( mpChildClipRegion )
        delete mpChildClipRegion;
}

// -----------------------------------------------------------------------

void Window::MouseMove( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEMOVE, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mbMouseMove = TRUE;
}

// -----------------------------------------------------------------------

void Window::MouseButtonDown( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEBUTTONDOWN, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mbMouseButtonDown = TRUE;
}

// -----------------------------------------------------------------------

void Window::MouseButtonUp( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEBUTTONUP, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mbMouseButtonUp = TRUE;
}

// -----------------------------------------------------------------------

void Window::KeyInput( const KeyEvent& rKEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_KEYINPUT, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mbKeyInput = TRUE;
}

// -----------------------------------------------------------------------

void Window::KeyUp( const KeyEvent& rKEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_KEYUP, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mbKeyUp = TRUE;
}

// -----------------------------------------------------------------------

void Window::Paint( const Rectangle& rRect )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    if ( mxWindowPeer.is() )
        Application::GetUnoWrapper()->WindowEvent_Paint( this, rRect );
}

// -----------------------------------------------------------------------

void Window::Draw( OutputDevice*, const Point&, const Size&, ULONG )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::Move()
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    if ( mxWindowPeer.is() )
        Application::GetUnoWrapper()->WindowEvent_Move( this );
}

// -----------------------------------------------------------------------

void Window::Resize()
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    if ( mxWindowPeer.is() )
        Application::GetUnoWrapper()->WindowEvent_Resize( this );
}

// -----------------------------------------------------------------------

void Window::Activate()
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }
}

// -----------------------------------------------------------------------

void Window::Deactivate()
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }
}

// -----------------------------------------------------------------------

void Window::GetFocus()
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    if ( HasFocus() && mpLastFocusWindow && !(mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
        mpLastFocusWindow->GrabFocus();

    NotifyEvent aNEvt( EVENT_GETFOCUS, this );
    Notify( aNEvt );

    if ( Application::GetAccessHdlCount() )
        Application::AccessNotify( AccessNotification( ACCESS_EVENT_GETFOCUS, this ) );
}

// -----------------------------------------------------------------------

void Window::LoseFocus()
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_LOSEFOCUS, this );
    Notify( aNEvt );

    if ( Application::GetAccessHdlCount() )
        Application::AccessNotify( AccessNotification( ACCESS_EVENT_LOSEFOCUS, this ) );
}

// -----------------------------------------------------------------------

void Window::RequestHelp( const HelpEvent& rHEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    // Wenn Balloon-Help angefordert wird, dann den Balloon mit dem
    // gesetzten Hilfetext anzeigen
    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
    {
        const XubString* pStr = &(GetHelpText());
        if ( !pStr->Len() )
            pStr = &(GetQuickHelpText());
        if ( !pStr->Len() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), *pStr );
    }
    else if ( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        const XubString* pStr = &(GetQuickHelpText());
        if ( !pStr->Len() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Point aPos = GetPosPixel();
            if ( ImplGetParent() && !ImplIsOverlapWindow() )
                aPos = ImplGetParent()->OutputToScreenPixel( aPos );
            Rectangle   aRect( aPos, GetSizePixel() );
            String      aHelpText;
            if ( pStr->Len() )
                aHelpText = GetHelpText();
            Help::ShowQuickHelp( this, aRect, *pStr, aHelpText, QUICKHELP_CTRLTEXT );
        }
    }
    else
    {
        ULONG nStartHelpId = GetHelpId();

        if ( !nStartHelpId && ImplGetParent() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            if ( !nStartHelpId )
                nStartHelpId = HELP_INDEX;

            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pHelp->Start( nStartHelpId );
        }
    }
}

// -----------------------------------------------------------------------

void Window::Command( const CommandEvent& rCEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    if ( mxWindowPeer.is() )
        Application::GetUnoWrapper()->WindowEvent_Command( this, rCEvt );

    NotifyEvent aNEvt( EVENT_COMMAND, this, &rCEvt );
    if ( !Notify( aNEvt ) )
        mbCommand = TRUE;
}

// -----------------------------------------------------------------------

void Window::Tracking( const TrackingEvent& )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

BOOL Window::QueryDrop( DropEvent& rDEvt )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    NotifyEvent aNEvt( EVENT_QUERYDROP, this, &rDEvt, FALSE );
    Notify( aNEvt );
    return (BOOL)aNEvt.GetReturnValue();
}

// -----------------------------------------------------------------------

BOOL Window::Drop( const DropEvent& rDEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_DROP, this, &rDEvt, FALSE );
    Notify( aNEvt );
    return (BOOL)aNEvt.GetReturnValue();
}

// -----------------------------------------------------------------------

void Window::UserEvent( ULONG, void* )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::StateChanged( StateChangedType )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::DataChanged( const DataChangedEvent& )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

long Window::PreNotify( NotifyEvent& rNEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    BOOL bDone = FALSE;
    if ( mpParent && !ImplIsOverlapWindow() )
        bDone = mpParent->PreNotify( rNEvt );

    if ( !bDone )
    {
        if( rNEvt.GetType() == EVENT_GETFOCUS )
        {
            BOOL bCompoundFocusChanged = FALSE;
            if ( mbCompoundControl && !mbCompoundControlHasFocus && HasChildPathFocus() )
            {
                mbCompoundControlHasFocus = TRUE;
                bCompoundFocusChanged = TRUE;
            }

            if ( mxWindowPeer.is() && ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) ) )
                Application::GetUnoWrapper()->WindowEvent_GetFocus( this );
        }
        else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            BOOL bCompoundFocusChanged = FALSE;
            if ( mbCompoundControl && mbCompoundControlHasFocus && !HasChildPathFocus() )
            {
                mbCompoundControlHasFocus = FALSE ;
                bCompoundFocusChanged = TRUE;
            }

            if ( mxWindowPeer.is() && ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) ) )
                Application::GetUnoWrapper()->WindowEvent_LoseFocus( this );
        }
        else if( rNEvt.GetType() == EVENT_MOUSEMOVE )
        {
            if ( mxWindowPeer.is() && ( mbCompoundControl || ( rNEvt.GetWindow() == this ) ) )
            {
                if ( rNEvt.GetWindow() == this )
                    Application::GetUnoWrapper()->WindowEvent_MouseMove( this, *rNEvt.GetMouseEvent() );
                else
                    Application::GetUnoWrapper()->WindowEvent_MouseMove( this, ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this ) );
            }
        }
        else if( rNEvt.GetType() == EVENT_MOUSEBUTTONUP )
        {
            if ( mxWindowPeer.is() && ( mbCompoundControl || ( rNEvt.GetWindow() == this ) ) )
            {
                if ( rNEvt.GetWindow() == this )
                    Application::GetUnoWrapper()->WindowEvent_MouseButtonUp( this, *rNEvt.GetMouseEvent() );
                else
                    Application::GetUnoWrapper()->WindowEvent_MouseButtonUp( this, ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this ) );
            }
        }
        else if( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            if ( mxWindowPeer.is() && ( mbCompoundControl || ( rNEvt.GetWindow() == this ) ) )
            {
                if ( rNEvt.GetWindow() == this )
                    Application::GetUnoWrapper()->WindowEvent_MouseButtonDown( this, *rNEvt.GetMouseEvent() );
                else
                    Application::GetUnoWrapper()->WindowEvent_MouseButtonDown( this, ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this ) );
            }
        }
        else if( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            if ( mxWindowPeer.is() && ( mbCompoundControl || ( rNEvt.GetWindow() == this ) ) )
                Application::GetUnoWrapper()->WindowEvent_KeyInput( this, *rNEvt.GetKeyEvent() );
        }
        else if( rNEvt.GetType() == EVENT_KEYUP )
        {
            if ( mxWindowPeer.is() && ( mbCompoundControl || ( rNEvt.GetWindow() == this ) ) )
                Application::GetUnoWrapper()->WindowEvent_KeyUp( this, *rNEvt.GetKeyEvent() );
        }
    }

    return bDone;
}

// -----------------------------------------------------------------------

long Window::Notify( NotifyEvent& rNEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    long nRet = FALSE;

    // Dialog-Steuerung
    if ( (GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
    {
        // Wenn Parent auch DialogSteuerung aktiviert hat, uebernimmt dieser die Steuerung
        if ( (rNEvt.GetType() == EVENT_KEYINPUT) || (rNEvt.GetType() == EVENT_KEYUP) )
        {
            if ( ImplIsOverlapWindow() ||
                 ((ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
            {
                nRet = ImplDlgCtrl( *rNEvt.GetKeyEvent(), rNEvt.GetType() == EVENT_KEYINPUT );
            }
        }
        else if ( (rNEvt.GetType() == EVENT_GETFOCUS) || (rNEvt.GetType() == EVENT_LOSEFOCUS) )
        {
            ImplDlgCtrlFocusChanged( rNEvt.GetWindow(), rNEvt.GetType() == EVENT_GETFOCUS );
            if ( (rNEvt.GetWindow() == this) && (rNEvt.GetType() == EVENT_GETFOCUS) &&
                 !(GetStyle() & WB_TABSTOP) && !(mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
            {
                USHORT n = 0;
                Window* pFirstChild = ImplGetDlgWindow( n, DLGWINDOW_FIRST );
                if ( pFirstChild )
                    pFirstChild->ImplControlFocus();
            }
        }
    }

    if ( !nRet )
    {
        if ( mpParent && !ImplIsOverlapWindow() )
            nRet = mpParent->Notify( rNEvt );
    }

    return nRet;
}

// -----------------------------------------------------------------------

ULONG Window::PostUserEvent( ULONG nEvent, void* pEventData )
{
    ULONG nEventId;
    PostUserEvent( nEventId, nEvent, pEventData );
    return nEventId;
}

// -----------------------------------------------------------------------

ULONG Window::PostUserEvent( const Link& rLink, void* pCaller )
{
    ULONG nEventId;
    PostUserEvent( nEventId, rLink, pCaller );
    return nEventId;
}

// -----------------------------------------------------------------------

BOOL Window::PostUserEvent( ULONG& rEventId, ULONG nEvent, void* pEventData )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = nEvent;
    pSVEvent->mpData    = pEventData;
    pSVEvent->mpLink    = NULL;
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = TRUE;
    ImplAddDel( &(pSVEvent->maDelData) );
    rEventId = (ULONG)pSVEvent;
#ifndef REMOTE_APPSERVER
    if ( mpFrame->PostEvent( pSVEvent ) )
        return TRUE;
    else
    {
        rEventId = 0;
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent;
        return FALSE;
    }
#else
    ExtRmEvent* pEvt = new ExtRmEvent( RMEVENT_USEREVENT, NULL, pSVEvent );
    ImplPostEvent( pEvt );
    return TRUE;
#endif
}

// -----------------------------------------------------------------------

BOOL Window::PostUserEvent( ULONG& rEventId, const Link& rLink, void* pCaller )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = 0;
    pSVEvent->mpData    = pCaller;
    pSVEvent->mpLink    = new Link( rLink );
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = TRUE;
    ImplAddDel( &(pSVEvent->maDelData) );
    rEventId = (ULONG)pSVEvent;
#ifndef REMOTE_APPSERVER
    if ( mpFrame->PostEvent( pSVEvent ) )
        return TRUE;
    else
    {
        rEventId = 0;
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent;
        return FALSE;
    }
#else
    ExtRmEvent* pEvt = new ExtRmEvent( RMEVENT_USEREVENT, NULL, pSVEvent );
    ImplPostEvent( pEvt );
    return TRUE;
#endif
}

// -----------------------------------------------------------------------

void Window::RemoveUserEvent( ULONG nUserEvent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVEvent* pSVEvent = (ImplSVEvent*)nUserEvent;

    DBG_ASSERT( pSVEvent->mpWindow == this,
                "Window::RemoveUserEvent(): Event doesn't send to this window or is already removed" );
    DBG_ASSERT( pSVEvent->mbCall,
                "Window::RemoveUserEvent(): Event is already removed" );

    if ( pSVEvent->mpWindow )
    {
        pSVEvent->mpWindow->ImplRemoveDel( &(pSVEvent->maDelData) );
        pSVEvent->mpWindow = NULL;
    }

    pSVEvent->mbCall = FALSE;
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplAsyncStateChangedHdl, void*, pState )
{
    StateChanged( (StateChangedType)(ULONG)pState );
    return 0;
}

// -----------------------------------------------------------------------

void Window::PostStateChanged( StateChangedType nState )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    PostUserEvent( LINK( this, Window, ImplAsyncStateChangedHdl ), (void*)(ULONG)nState );
}

// -----------------------------------------------------------------------

void Window::SetStyle( WinBits nStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mnStyle != nStyle )
    {
        mnPrevStyle = mnStyle;
        mnStyle = nStyle;
        StateChanged( STATE_CHANGE_STYLE );
    }
}

// -----------------------------------------------------------------------

void Window::SetExtendedStyle( WinBits nExtendedStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mnExtendedStyle != nExtendedStyle )
    {
        mnPrevExtendedStyle = mnExtendedStyle;
        mnExtendedStyle = nExtendedStyle;
        StateChanged( STATE_CHANGE_EXTENDEDSTYLE );
    }
}

// -----------------------------------------------------------------------

SystemWindow* Window::GetSystemWindow() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    const Window* pWin = this;
    while ( !pWin->IsSystemWindow() )
        pWin  = pWin->GetParent();
    return (SystemWindow*)pWin;
}

// -----------------------------------------------------------------------

void Window::SetBorderStyle( USHORT nBorderStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        if ( mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
             ((ImplBorderWindow*)mpBorderWindow)->SetBorderStyle( nBorderStyle );
        else
            mpBorderWindow->SetBorderStyle( nBorderStyle );
    }
}

// -----------------------------------------------------------------------

USHORT Window::GetBorderStyle() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        if ( mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpBorderWindow)->GetBorderStyle();
        else
            return mpBorderWindow->GetBorderStyle();
    }

    return 0;
}

// -----------------------------------------------------------------------

long Window::CalcTitleWidth() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        if ( mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpBorderWindow)->CalcTitleWidth();
        else
            return mpBorderWindow->CalcTitleWidth();
    }
    else if ( mbFrame && (mnStyle & WB_MOVEABLE) )
    {
        // Fuer Frame-Fenster raten wir die Breite, da wir den Border fuer
        // externe Dialoge nicht kennen
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Font aFont = GetFont();
        ((Window*)this)->SetPointFont( rStyleSettings.GetTitleFont() );
        long nTitleWidth = GetTextWidth( GetText() );
        ((Window*)this)->SetFont( aFont );
        nTitleWidth += rStyleSettings.GetTitleHeight() * 3;
        nTitleWidth += rStyleSettings.GetBorderSize() * 2;
        nTitleWidth += 10;
        return nTitleWidth;
    }

    return 0;
}

// -----------------------------------------------------------------------

void Window::EnableClipSiblings( BOOL bClipSiblings )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->EnableClipSiblings( bClipSiblings );

    mbClipSiblings = bClipSiblings;
}

// -----------------------------------------------------------------------

void Window::SetMouseTransparent( BOOL bTransparent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetMouseTransparent( bTransparent );

    mbMouseTransparent = bTransparent;
}

// -----------------------------------------------------------------------

void Window::SetPaintTransparent( BOOL bTransparent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetPaintTransparent( bTransparent );

    mbPaintTransparent = bTransparent;
}

// -----------------------------------------------------------------------

void Window::SetInputContext( const InputContext& rInputContext )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    maInputContext = rInputContext;
    if ( !mbInFocusHdl && HasFocus() )
        ImplNewInputContext();
}

// -----------------------------------------------------------------------

void Window::EndExtTextInput( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

#ifndef REMOTE_APPSERVER
    if ( mbExtTextInput )
        ImplGetFrame()->EndExtTextInput( nFlags );
#endif
}

// -----------------------------------------------------------------------

void Window::SetCursorRect( const Rectangle* pRect, long nExtTextInputWidth )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();
    if ( pWinData->mpCursorRect )
    {
        if ( pRect )
            *pWinData->mpCursorRect = *pRect;
        else
        {
            delete pWinData->mpCursorRect;
            pWinData->mpCursorRect = NULL;
        }
    }
    else
    {
        if ( pRect )
            pWinData->mpCursorRect = new Rectangle( *pRect );
    }

    pWinData->mnCursorExtWidth = nExtTextInputWidth;
}

// -----------------------------------------------------------------------

const Rectangle* Window::GetCursorRect() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mpCursorRect;
}

// -----------------------------------------------------------------------

long Window::GetCursorExtTextInputWidth() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mnCursorExtWidth;
}

// -----------------------------------------------------------------------

void Window::SetSettings( const AllSettings& rSettings, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        mpBorderWindow->SetSettings( rSettings, FALSE );
        if ( (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow->SetSettings( rSettings, TRUE );
    }

    AllSettings aOldSettings = maSettings;
    OutputDevice::SetSettings( rSettings );
    ULONG nChangeFlags = aOldSettings.GetChangeFlags( rSettings );
    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
    }

    // AppFont-Aufloesung und DPI-Aufloesung neu berechnen
    ImplInitResolutionSettings();

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->SetSettings( rSettings, bChild );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::UpdateSettings( const AllSettings& rSettings, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        mpBorderWindow->UpdateSettings( rSettings, FALSE );
        if ( (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow->UpdateSettings( rSettings, TRUE );
    }

    AllSettings aOldSettings = maSettings;
    ULONG nChangeFlags = maSettings.Update( maSettings.GetWindowUpdate(), rSettings );
    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
    }

    // AppFont-Aufloesung und DPI-Aufloesung neu berechnen
    ImplInitResolutionSettings();

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->UpdateSettings( rSettings, bChild );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::NotifyAllChilds( DataChangedEvent& rDCEvt )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    DataChanged( rDCEvt );

    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->NotifyAllChilds( rDCEvt );
        pChild = pChild->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::SetPointFont( const Font& rFont )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Font aFont = rFont;
    ImplPointToLogic( aFont );
    SetFont( aFont );
}

// -----------------------------------------------------------------------

Font Window::GetPointFont() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Font aFont = GetFont();
    ImplLogicToPoint( aFont );
    return aFont;
}

// -----------------------------------------------------------------------

void Window::GetFontResolution( long& nDPIX, long& nDPIY ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    nDPIX = mpFrameData->mnFontDPIX;
    nDPIY = mpFrameData->mnFontDPIY;
}

// -----------------------------------------------------------------------

void Window::SetParentClipMode( USHORT nMode )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetParentClipMode( nMode );
    else
    {
        if ( !ImplIsOverlapWindow() )
        {
            mnParentClipMode = nMode;
            if ( nMode & PARENTCLIPMODE_CLIP )
                mpParent->mbClipChildren = TRUE;
        }
    }
}

// -----------------------------------------------------------------------

USHORT Window::GetParentClipMode() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        return mpBorderWindow->GetParentClipMode();
    else
        return mnParentClipMode;
}

// -----------------------------------------------------------------------

void Window::SetWindowRegionPixel()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetWindowRegionPixel();
    else
    {
        if ( mbWinRegion )
        {
            maWinRegion = Region( REGION_NULL );
            mbWinRegion = FALSE;
            ImplSetClipFlag();

            if ( IsReallyVisible() )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpOverlapData && mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Region      aRegion( aRect );
                ImplInvalidateParentFrameRegion( aRegion );
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetWindowRegionPixel( const Region& rRegion )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetWindowRegionPixel( rRegion );
    else
    {
        BOOL bInvalidate = FALSE;

        if ( rRegion.GetType() == REGION_NULL )
        {
            if ( mbWinRegion )
            {
                maWinRegion = Region( REGION_NULL );
                mbWinRegion = FALSE;
                ImplSetClipFlag();
                bInvalidate = TRUE;
            }
        }
        else
        {
            maWinRegion = rRegion;
            mbWinRegion = TRUE;
            ImplSetClipFlag();
            bInvalidate = TRUE;
        }

        if ( IsReallyVisible() )
        {
            // Hintergrund-Sicherung zuruecksetzen
            if ( mpOverlapData && mpOverlapData->mpSaveBackDev )
                ImplDeleteOverlapBackground();
            if ( mpFrameData->mpFirstBackWin )
                ImplInvalidateAllOverlapBackgrounds();
            Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            Region      aRegion( aRect );
            ImplInvalidateParentFrameRegion( aRegion );
        }
    }
}

// -----------------------------------------------------------------------

const Region& Window::GetWindowRegionPixel() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        return mpBorderWindow->GetWindowRegionPixel();
    else
        return maWinRegion;
}

// -----------------------------------------------------------------------

BOOL Window::IsWindowRegionPixel() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        return mpBorderWindow->IsWindowRegionPixel();
    else
        return mbWinRegion;
}

// -----------------------------------------------------------------------

Region Window::GetWindowClipRegionPixel( USHORT nFlags ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Region aWinClipRegion;

    if ( nFlags & WINDOW_GETCLIPREGION_NOCHILDREN )
    {
        if ( mbInitWinClipRegion )
            ((Window*)this)->ImplInitWinClipRegion();
        aWinClipRegion = maWinClipRegion;
    }
    else
    {
        Region* pWinChildClipRegion = ((Window*)this)->ImplGetWinChildClipRegion();
        aWinClipRegion = *pWinChildClipRegion;
    }

    if ( nFlags & WINDOW_GETCLIPREGION_NULL )
    {
        Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aWinRegion( aWinRect );

        if ( aWinRegion == aWinClipRegion )
            aWinClipRegion.SetNull();
    }

    aWinClipRegion.Move( -mnOutOffX, -mnOutOffY );

    return aWinClipRegion;
}

// -----------------------------------------------------------------------

Region Window::GetPaintRegion() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpPaintRegion )
    {
        Region aRegion = *mpPaintRegion;
        aRegion.Move( -mnOutOffX, -mnOutOffY );
        return PixelToLogic( aRegion );
    }
    else
    {
        Region aPaintRegion( REGION_NULL );
        return aPaintRegion;
    }
}

// -----------------------------------------------------------------------

void Window::SetParent( Window* pNewParent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_ASSERT( pNewParent, "Window::SetParent(): pParent == NULL" );

    if ( mbFrame )
        return;

    if ( mpBorderWindow )
    {
        mpRealParent = pNewParent;
        mpBorderWindow->SetParent( pNewParent );
        return;
    }

    if ( mpParent == pNewParent )
        return;

    BOOL bVisible = IsVisible();
    Show( FALSE, SHOW_NOFOCUSCHANGE );

    // Testen, ob sich das Overlap-Window aendert
    Window* pOldOverlapWindow;
    Window* pNewOverlapWindow;
    if ( ImplIsOverlapWindow() )
        pOldOverlapWindow = NULL;
    else
    {
        pNewOverlapWindow = pNewParent->ImplGetFirstOverlapWindow();
        if ( mpOverlapWindow != pNewOverlapWindow )
            pOldOverlapWindow = mpOverlapWindow;
        else
            pOldOverlapWindow = NULL;
    }

    // Fenster in der Hirachie umsetzen
    BOOL bFocusOverlapWin = HasChildPathFocus( TRUE );
    BOOL bFocusWin = HasChildPathFocus();
    BOOL bNewFrame = pNewParent->mpFrameWindow != mpFrameWindow;
    if ( bNewFrame )
    {
        if ( mpFrameData->mpFocusWin )
        {
            if ( IsWindowOrChild( mpFrameData->mpFocusWin ) )
                mpFrameData->mpFocusWin = NULL;
        }
        if ( mpFrameData->mpExtTextInputWin )
        {
            if ( IsWindowOrChild( mpFrameData->mpExtTextInputWin ) )
                mpFrameData->mpExtTextInputWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );
        }
        if ( mpFrameData->mpMouseMoveWin )
        {
            if ( IsWindowOrChild( mpFrameData->mpMouseMoveWin ) )
                mpFrameData->mpMouseMoveWin = NULL;
        }
        if ( mpFrameData->mpMouseDownWin )
        {
            if ( IsWindowOrChild( mpFrameData->mpMouseDownWin ) )
                mpFrameData->mpMouseDownWin = NULL;
        }
    }
    ImplRemoveWindow( bNewFrame );
    ImplInsertWindow( pNewParent );
    if ( mnParentClipMode & PARENTCLIPMODE_CLIP )
        pNewParent->mbClipChildren = TRUE;
    ImplUpdateWindowPtr();
#ifndef REMOTE_APPSERVER
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();
#else
    ImplUpdatePos();
#endif

    // Wenn sich das Overlap-Window geaendert hat, dann muss getestet werden,
    // ob auch OverlapWindow die das Child-Fenster als Parent gehabt haben
    // in der Window-Hirachie umgesetzt werden muessen
    if ( ImplIsOverlapWindow() )
    {
        if ( bNewFrame )
        {
            Window* pOverlapWindow = mpFirstOverlap;
            while ( pOverlapWindow )
            {
                Window* pNextOverlapWindow = pOverlapWindow->mpNext;
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
                pOverlapWindow = pNextOverlapWindow;
            }
        }
    }
    else if ( pOldOverlapWindow )
    {
        // Focus-Save zuruecksetzen
        if ( bFocusWin ||
             (pOldOverlapWindow->mpLastFocusWindow &&
              IsWindowOrChild( pOldOverlapWindow->mpLastFocusWindow )) )
            pOldOverlapWindow->mpLastFocusWindow = NULL;

        Window* pOverlapWindow = pOldOverlapWindow->mpFirstOverlap;
        while ( pOverlapWindow )
        {
            Window* pNextOverlapWindow = pOverlapWindow->mpNext;
            if ( ImplIsRealParentPath( pOverlapWindow->ImplGetWindow() ) )
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }

        // Activate-Status beim naechsten Overlap-Window updaten
        if ( HasChildPathFocus( TRUE ) )
            ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
    }

    // Activate-Status mit umsetzen
    if ( bNewFrame )
    {
        if ( (GetType() == WINDOW_BORDERWINDOW) &&
             (ImplGetWindow()->GetType() == WINDOW_FLOATINGWINDOW) )
            ((ImplBorderWindow*)this)->SetDisplayActive( mpFrameData->mbHasFocus );
    }

    // Focus evtl. auf den neuen Frame umsetzen, wenn FocusWindow mit
    // SetParent() umgesetzt wird
    if ( bFocusOverlapWin )
    {
        mpFrameData->mpFocusWin = Application::GetFocusWindow();
        if ( !mpFrameData->mbHasFocus )
        {
            mpFrame->ToTop( 0 );
        }
    }

    if ( bVisible )
        Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
}

// -----------------------------------------------------------------------

void Window::Show( BOOL bVisible, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbVisible == bVisible )
        return;

    mbVisible = bVisible != 0;

    if ( !bVisible )
    {
        ImplHideAllOverlaps();

        if ( mpBorderWindow )
        {
            BOOL bOldUpdate = mpBorderWindow->mbNoParentUpdate;
            if ( mbNoParentUpdate )
                mpBorderWindow->mbNoParentUpdate = TRUE;
            mpBorderWindow->Show( FALSE, nFlags );
            mpBorderWindow->mbNoParentUpdate = bOldUpdate;
        }
        else if ( mbFrame )
            mpFrame->Show( FALSE );

        StateChanged( STATE_CHANGE_VISIBLE );

        if ( mbReallyVisible )
        {
            Region  aInvRegion( REGION_EMPTY );
            BOOL    bSaveBack = FALSE;

            if ( ImplIsOverlapWindow() && !mbFrame )
            {
                if ( ImplRestoreOverlapBackground( aInvRegion ) )
                    bSaveBack = TRUE;
            }

            if ( !bSaveBack )
            {
                if ( mbInitWinClipRegion )
                    ImplInitWinClipRegion();
                aInvRegion = maWinClipRegion;
            }

            ImplResetReallyVisible();
            ImplSetClipFlag();

            if ( ImplIsOverlapWindow() && !mbFrame )
            {
                // Focus umsetzen
                if ( !(nFlags & SHOW_NOFOCUSCHANGE) && HasChildPathFocus() )
                {
                    if ( mpOverlapWindow->IsEnabled() && mpOverlapWindow->IsInputEnabled() )
                        mpOverlapWindow->GrabFocus();
                }
            }

            if ( !mbFrame )
            {
                if ( !mbNoParentUpdate && !(nFlags & SHOW_NOPARENTUPDATE) )
                {
                    if ( !aInvRegion.IsEmpty() )
                        ImplInvalidateParentFrameRegion( aInvRegion );
                }
                ImplGenerateMouseMove();
            }
        }
    }
    else
    {
        if ( mbCallMove )
        {
            mbCallMove = FALSE;
            Move();
        }
        if ( mbCallResize )
        {
            mbCallResize = FALSE;
            Resize();
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        Window* pTestParent;
        if ( ImplIsOverlapWindow() )
            pTestParent = mpOverlapWindow;
        else
            pTestParent = ImplGetParent();
        if ( mbFrame || pTestParent->mbReallyVisible )
        {
            // Wenn ein Window gerade sichtbar wird, schicken wir allen
            // Child-Fenstern ein StateChanged, damit diese sich
            // initialisieren koennen
            ImplCallInitShow();

            // Wenn es ein SystemWindow ist, dann kommt es auch automatisch
            // nach vorne, wenn es gewuenscht ist
            if ( ImplIsOverlapWindow() && !(nFlags & SHOW_NOACTIVATE) )
            {
                ImplStartToTop( 0 );
                ImplFocusToTop( 0, FALSE );
            }

            // Hintergrund sichern
            if ( mpOverlapData && mpOverlapData->mbSaveBack )
                ImplSaveOverlapBackground();
            // Dafuer sorgen, das Clip-Rechtecke neu berechnet werden
            ImplSetReallyVisible();
            ImplSetClipFlag();

            if ( !mbFrame )
            {
                ImplInvalidate( NULL, INVALIDATE_NOTRANSPARENT | INVALIDATE_CHILDREN );
                ImplGenerateMouseMove();
            }
        }

        if ( mpBorderWindow )
            mpBorderWindow->Show( TRUE, nFlags );
        else if ( mbFrame )
        {
            mbPaintFrame = TRUE;
            mpFrame->Show( TRUE );

            // Query the correct size of the window, if we are waiting for
            // a system resize
            if ( mbWaitSystemResize )
            {
                long nOutWidth;
                long nOutHeight;
                mpFrame->GetClientSize( nOutWidth, nOutHeight );
                ImplHandleResize( this, nOutWidth, nOutHeight );
            }
        }

#ifdef DBG_UTIL
        if ( IsDialog() || (GetType() == WINDOW_TABPAGE) )
        {
            DBG_DIALOGTEST( this );
        }
#endif

        ImplShowAllOverlaps();
    }

    // Hintergrund-Sicherung zuruecksetzen
    if ( mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    if ( mxWindowPeer.is() )
        Application::GetUnoWrapper()->WindowEvent_Show( this, mbVisible );
}

// -----------------------------------------------------------------------

void Window::Enable( BOOL bEnable, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !bEnable )
    {
        // Wenn ein Fenster disablte wird, wird automatisch der Tracking-Modus
        // beendet oder der Capture geklaut
        if ( IsTracking() )
            EndTracking( ENDTRACK_CANCEL );
        if ( IsMouseCaptured() )
            ReleaseMouse();
        // Wenn Fenster den Focus hat und in der Dialog-Steuerung enthalten,
        // wird versucht, den Focus auf das naechste Control weiterzuschalten
        // mbDisabled darf erst nach Aufruf von ImplDlgCtrlNextWindow() gesetzt
        // werden. Ansonsten muss ImplDlgCtrlNextWindow() umgestellt werden
        if ( HasFocus() )
            ImplDlgCtrlNextWindow();
    }

    if ( mpBorderWindow )
    {
        mpBorderWindow->Enable( bEnable, FALSE );
        if ( (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow->Enable( bEnable, TRUE );
    }

    if ( mbDisabled != !bEnable )
    {
        mbDisabled = !bEnable;
#ifndef REMOTE_APPSERVER
        if ( mpSysObj )
            mpSysObj->Enable( bEnable && !mbInputDisabled );
#endif
//      if ( mbFrame )
//          mpFrame->Enable( bEnable && !mbInputDisabled );
        StateChanged( STATE_CHANGE_ENABLE );
    }

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->Enable( bEnable, bChild );
            pChild = pChild->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();
}

// -----------------------------------------------------------------------

void Window::EnableInput( BOOL bEnable, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        mpBorderWindow->EnableInput( bEnable, FALSE );
        if ( (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow->EnableInput( bEnable, TRUE );
    }

    if ( !mbAlwaysEnableInput || bEnable )
    {
        // Wenn ein Fenster disablte wird, wird automatisch der
        // Tracking-Modus beendet oder der Capture geklaut
        if ( !bEnable )
        {
            if ( IsTracking() )
                EndTracking( ENDTRACK_CANCEL );
            if ( IsMouseCaptured() )
                ReleaseMouse();
        }

        if ( mbInputDisabled != !bEnable )
        {
            mbInputDisabled = !bEnable;
#ifndef REMOTE_APPSERVER
            if ( mpSysObj )
                mpSysObj->Enable( !mbDisabled && bEnable );
#endif
//          if ( mbFrame )
//              mpFrame->Enable( !mbDisabled && bEnable );
        }
    }

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->EnableInput( bEnable, bChild );
            pChild = pChild->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();
}

// -----------------------------------------------------------------------

void Window::EnableInput( BOOL bEnable, BOOL bChild, BOOL bSysWin,
                          const Window* pExcludeWindow )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    EnableInput( bEnable, bChild );
    if ( bSysWin )
    {
        // pExculeWindow is the first Overlap-Frame --> if this
        // shouldn't be the case, than this must be changed in dialog.cxx
        pExcludeWindow = pExcludeWindow->ImplGetFirstOverlapWindow();
        Window* pSysWin = mpFrameWindow->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            // Is Window in the path from this window
            if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pSysWin, TRUE ) )
            {
                // Is Window not in the exclude window path or not the
                // exclude window, than change the status
                if ( !pExcludeWindow->ImplIsWindowOrChild( pSysWin, TRUE ) )
                    pSysWin->EnableInput( bEnable, bChild );
            }
            pSysWin = pSysWin->mpNextOverlap;
        }
    }
}

// -----------------------------------------------------------------------

void Window::AlwaysEnableInput( BOOL bAlways, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->AlwaysEnableInput( bAlways, FALSE );

    if ( mbAlwaysEnableInput != bAlways )
    {
        mbAlwaysEnableInput = bAlways;

        if ( bAlways )
            EnableInput( TRUE, FALSE );
    }

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->AlwaysEnableInput( bAlways, bChild );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetActivateMode( USHORT nMode )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetActivateMode( nMode );

    if ( mnActivateMode != nMode )
    {
        mnActivateMode = nMode;

        // Evtl. ein Decativate/Activate ausloesen
        if ( mnActivateMode )
        {
            if ( (mbActive || (GetType() == WINDOW_BORDERWINDOW)) &&
                 !HasChildPathFocus( TRUE ) )
            {
                mbActive = FALSE;
                Deactivate();
            }
        }
        else
        {
            if ( !mbActive || (GetType() == WINDOW_BORDERWINDOW) )
            {
                mbActive = TRUE;
                Activate();
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::ToTop( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplStartToTop( nFlags );
    ImplFocusToTop( nFlags, IsReallyVisible() );
}

// -----------------------------------------------------------------------

void Window::SetZOrder( Window* pRefWindow, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        mpBorderWindow->SetZOrder( pRefWindow, nFlags );
        return;
    }

    if ( nFlags & WINDOW_ZORDER_FIRST )
    {
        if ( ImplIsOverlapWindow() )
            pRefWindow = mpOverlapWindow->mpFirstOverlap;
        else
            pRefWindow = mpParent->mpFirstChild;
        nFlags |= WINDOW_ZORDER_BEFOR;
    }
    else if ( nFlags & WINDOW_ZORDER_LAST )
    {
        if ( ImplIsOverlapWindow() )
            pRefWindow = mpOverlapWindow->mpLastOverlap;
        else
            pRefWindow = mpParent->mpLastChild;
        nFlags |= WINDOW_ZORDER_BEHIND;
    }

    while ( pRefWindow->mpBorderWindow )
        pRefWindow = pRefWindow->mpBorderWindow;
    if ( (pRefWindow == this) || mbFrame )
        return;

    DBG_ASSERT( pRefWindow->mpParent == mpParent, "Window::SetZOrder() - pRefWindow has other parent" );
    if ( nFlags & WINDOW_ZORDER_BEFOR )
    {
        if ( pRefWindow->mpPrev == this )
            return;

        if ( ImplIsOverlapWindow() )
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpOverlapWindow->mpFirstOverlap = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpOverlapWindow->mpLastOverlap = mpPrev;
            if ( !pRefWindow->mpPrev )
                mpOverlapWindow->mpFirstOverlap = this;
        }
        else
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpParent->mpFirstChild = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpParent->mpLastChild = mpPrev;
            if ( !pRefWindow->mpPrev )
                mpParent->mpFirstChild = this;
        }

        mpPrev = pRefWindow->mpPrev;
        mpNext = pRefWindow;
        if ( mpPrev )
            mpPrev->mpNext = this;
        mpNext->mpPrev = this;
    }
    else if ( nFlags & WINDOW_ZORDER_BEHIND )
    {
        if ( pRefWindow->mpNext == this )
            return;

        if ( ImplIsOverlapWindow() )
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpOverlapWindow->mpFirstOverlap = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpOverlapWindow->mpLastOverlap = mpPrev;
            if ( !pRefWindow->mpNext )
                mpOverlapWindow->mpLastOverlap = this;
        }
        else
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpParent->mpFirstChild = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpParent->mpLastChild = mpPrev;
            if ( !pRefWindow->mpNext )
                mpParent->mpLastChild = this;
        }

        mpPrev = pRefWindow;
        mpNext = pRefWindow->mpNext;
        if ( mpNext )
            mpNext->mpPrev = this;
        mpPrev->mpNext = this;
    }

    if ( IsReallyVisible() )
    {
        // Hintergrund-Sicherung zuruecksetzen
        if ( mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        if ( mbInitWinClipRegion || !maWinClipRegion.IsEmpty() )
        {
            BOOL bInitWinClipRegion = mbInitWinClipRegion;
            ImplSetClipFlag();

            // Wenn ClipRegion noch nicht initalisiert wurde, dann
            // gehen wir davon aus, das das Fenster noch nicht
            // ausgegeben wurde und loesen somit auch keine
            // Invalidates aus. Dies ist eine Optimierung fuer
            // HTML-Dokumenten mit vielen Controls. Wenn es mal
            // Probleme mit dieser Abfrage gibt, sollte man ein
            // Flag einfuehren, ob das Fenster nach Show schon
            // einmal ausgegeben wurde.
            if ( !bInitWinClipRegion )
            {
                // Alle nebeneinanderliegen Fenster invalidieren
                // Noch nicht komplett implementiert !!!
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Window*     pWindow = NULL;
                if ( ImplIsOverlapWindow() )
                {
                    if ( mpOverlapWindow )
                        pWindow = mpOverlapWindow->mpFirstOverlap;
                }
                else
                    pWindow = ImplGetParent()->mpFirstChild;
                // Alle Fenster, die vor uns liegen und von uns verdeckt wurden,
                // invalidieren
                while ( pWindow )
                {
                    if ( pWindow == this )
                        break;
                    Rectangle aCompRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                         Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
                    if ( aWinRect.IsOver( aCompRect ) )
                        pWindow->Invalidate( INVALIDATE_CHILDREN | INVALIDATE_NOTRANSPARENT );
                    pWindow = pWindow->mpNext;
                }
                // Wenn uns ein Fenster welches im Hinterund liegt verdeckt hat,
                // dann muessen wir uns neu ausgeben
                while ( pWindow )
                {
                    if ( pWindow != this )
                    {
                        Rectangle aCompRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                             Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
                        if ( aWinRect.IsOver( aCompRect ) )
                        {
                            Invalidate( INVALIDATE_CHILDREN | INVALIDATE_NOTRANSPARENT );
                            break;
                        }
                    }
                    pWindow = pWindow->mpNext;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::EnableAlwaysOnTop( BOOL bEnable )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mbAlwaysOnTop = bEnable;

    if ( mpBorderWindow )
        mpBorderWindow->EnableAlwaysOnTop( bEnable );
    else if ( bEnable && IsReallyVisible() )
        ToTop();

    if ( mbFrame )
        mpFrame->SetAlwaysOnTop( bEnable );
}

// -----------------------------------------------------------------------

void Window::SetPosSizePixel( long nX, long nY,
                              long nWidth, long nHeight, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( nFlags & WINDOW_POSSIZE_POS )
        mbDefPos = FALSE;
    if ( nFlags & WINDOW_POSSIZE_SIZE )
        mbDefSize = FALSE;

    // Oberstes BorderWindow ist das Window, welches positioniert werden soll
    Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;

    if ( pWindow->mbFrame )
    {
        // Nur Groessenaenderungen werden beruecksichtig
        if ( (nFlags & WINDOW_POSSIZE_SIZE) == WINDOW_POSSIZE_SIZE )
        {
            if ( !(nFlags & WINDOW_POSSIZE_WIDTH) )
                nWidth = pWindow->mnOutWidth;
            if ( !(nFlags & WINDOW_POSSIZE_HEIGHT) )
                nHeight = pWindow->mnOutHeight;

            pWindow->mpFrame->SetClientSize( nWidth, nHeight );
            // Resize should be called directly. If we havn't
            // set the correct size, we get a second resize from
            // the system with the correct size. This can be happend
            // if the size is to small or to lare.
            ImplHandleResize( pWindow, nWidth, nHeight );
        }
    }
    else
    {
        pWindow->ImplPosSizeWindow( nX, nY, nWidth, nHeight, nFlags );
        if ( IsReallyVisible() )
            ImplGenerateMouseMove();
    }
}

// -----------------------------------------------------------------------

Point Window::OutputToScreenPixel( const Point& rPos ) const
{
    return Point( rPos.X()+mnOutOffX, rPos.Y()+mnOutOffY );
}

// -----------------------------------------------------------------------

Point Window::ScreenToOutputPixel( const Point& rPos ) const
{
    return Point( rPos.X()-mnOutOffX, rPos.Y()-mnOutOffY );
}

// -----------------------------------------------------------------------

void Window::Scroll( long nHorzScroll, long nVertScroll, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplScroll( Rectangle( Point( mnOutOffX, mnOutOffY ),
                           Size( mnOutWidth, mnOutHeight ) ),
                nHorzScroll, nVertScroll, nFlags & ~SCROLL_CLIP );
}

// -----------------------------------------------------------------------

void Window::Scroll( long nHorzScroll, long nVertScroll,
                     const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Rectangle aRect = ImplLogicToDevicePixel( rRect );
    aRect.Intersection( Rectangle( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) ) );
    if ( !aRect.IsEmpty() )
        ImplScroll( aRect, nHorzScroll, nVertScroll, nFlags );
}

// -----------------------------------------------------------------------

void Window::Invalidate( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplInvalidate( NULL, nFlags );
}

// -----------------------------------------------------------------------

void Window::Invalidate( const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    Rectangle aRect = ImplLogicToDevicePixel( rRect );
    if ( !aRect.IsEmpty() )
    {
        Region aRegion( aRect );
        ImplInvalidate( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::Invalidate( const Region& rRegion, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    if ( rRegion.IsNull() )
        ImplInvalidate( NULL, nFlags );
    else
    {
        Region aRegion = ImplPixelToDevicePixel( LogicToPixel( rRegion ) );
        if ( !aRegion.IsEmpty() )
            ImplInvalidate( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::Validate( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplValidate( NULL, nFlags );
}

// -----------------------------------------------------------------------

void Window::Validate( const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    Rectangle aRect = ImplLogicToDevicePixel( rRect );
    if ( !aRect.IsEmpty() )
    {
        Region aRegion( aRect );
        ImplValidate( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::Validate( const Region& rRegion, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    if ( rRegion.IsNull() )
        ImplValidate( NULL, nFlags );
    else
    {
        Region aRegion = ImplPixelToDevicePixel( LogicToPixel( rRegion ) );
        if ( !aRegion.IsEmpty() )
            ImplValidate( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

BOOL Window::HasPaintEvent() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !mbReallyVisible )
        return FALSE;

    if ( mpFrameWindow->mbPaintFrame )
        return TRUE;

    if ( mnPaintFlags & IMPL_PAINT_PAINT )
        return TRUE;

    if ( !ImplIsOverlapWindow() )
    {
        const Window* pTempWindow = this;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mnPaintFlags & (IMPL_PAINT_PAINTCHILDS | IMPL_PAINT_PAINTALLCHILDS) )
                return TRUE;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Window::Update()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        mpBorderWindow->Update();
        return;
    }

    if ( !mbReallyVisible )
        return;

    BOOL bFlush = FALSE;
    if ( mpFrameWindow->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mbFrame || (mpBorderWindow && mpBorderWindow->mbFrame) )
            bFlush = TRUE;
    }

    // Zuerst muessen wir alle Fenster ueberspringen, die Paint-Transparent
    // sind
    Window* pUpdateWindow = this;
    Window* pWindow = pUpdateWindow;
    while ( !pWindow->ImplIsOverlapWindow() )
    {
        if ( !pWindow->mbPaintTransparent )
        {
            pUpdateWindow = pWindow;
            break;
        }
        pWindow = pWindow->ImplGetParent();
    }
    // Ein Update wirkt immer auf das Window, wo PAINTALLCHILDS gesetzt
    // ist, damit nicht zuviel gemalt wird
    pWindow = pUpdateWindow;
    do
    {
        if ( pWindow->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
            pUpdateWindow = pWindow;
        if ( pWindow->ImplIsOverlapWindow() )
            break;
        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    // Wenn es etwas zu malen gibt, dann ein Paint ausloesen
    if ( pUpdateWindow->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
    {
         // und fuer alle ueber uns stehende System-Fenster auch ein Update
         // ausloesen, damit nicht die ganze Zeit luecken stehen bleiben
         Window* pUpdateOverlapWindow = ImplGetFirstOverlapWindow()->mpFirstOverlap;
         while ( pUpdateOverlapWindow )
         {
             pUpdateOverlapWindow->Update();
             pUpdateOverlapWindow = pUpdateOverlapWindow->mpNext;
         }

        pUpdateWindow->ImplCallPaint( NULL, pUpdateWindow->mnPaintFlags );
    }

    if ( bFlush )
        Flush();
}

// -----------------------------------------------------------------------

void Window::Flush()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

#ifdef REMOTE_APPSERVER
    // !!!!!
#else
    mpFrame->Flush();
#endif
}

// -----------------------------------------------------------------------

void Window::Sync()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

#ifdef REMOTE_APPSERVER
    // Wir rufen eine syncrone Funktion, um uns zu syncronisieren
    long nDummy;
    mpFrame->GetClientSize( nDummy, nDummy );
#else
    mpFrame->Sync();
#endif
}

// -----------------------------------------------------------------------

void Window::SetUpdateMode( BOOL bUpdate )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mbNoUpdate = !bUpdate;
    StateChanged( STATE_CHANGE_UPDATEMODE );
}

// -----------------------------------------------------------------------

void Window::GrabFocus()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplGrabFocus( 0 );
}

// -----------------------------------------------------------------------

BOOL Window::HasFocus() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return (this == ImplGetSVData()->maWinData.mpFocusWin);
}

// -----------------------------------------------------------------------

BOOL Window::HasChildPathFocus( BOOL bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Window* pWindow = ImplGetSVData()->maWinData.mpFocusWin;
    if ( pWindow )
        return ImplIsWindowOrChild( pWindow, bSystemWindow );
    return FALSE;
}

// -----------------------------------------------------------------------

void Window::CaptureMouse()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVData* pSVData = ImplGetSVData();

    // Tracking evt. beenden
    if ( pSVData->maWinData.mpTrackWin != this )
    {
        if ( pSVData->maWinData.mpTrackWin )
            pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    }

    if ( pSVData->maWinData.mpCaptureWin != this )
    {
        pSVData->maWinData.mpCaptureWin = this;
        mpFrame->CaptureMouse( TRUE );
    }
}

// -----------------------------------------------------------------------

void Window::ReleaseMouse()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVData* pSVData = ImplGetSVData();

    DBG_ASSERTWARNING( pSVData->maWinData.mpCaptureWin == this,
                       "Window::ReleaseMouse(): window doesn't have the mouse capture" );

    if ( pSVData->maWinData.mpCaptureWin == this )
    {
        pSVData->maWinData.mpCaptureWin = NULL;
        mpFrame->CaptureMouse( FALSE );
        ImplGenerateMouseMove();
    }
}

// -----------------------------------------------------------------------

BOOL Window::IsMouseCaptured() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return (this == ImplGetSVData()->maWinData.mpCaptureWin);
}

// -----------------------------------------------------------------------

void Window::SetPointer( const Pointer& rPointer )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( maPointer == rPointer )
        return;

    maPointer   = rPointer;

    // Pointer evt. direkt umsetzen
    if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpFrame->SetPointer( ImplGetMousePointer() );
}

// -----------------------------------------------------------------------

void Window::EnableChildPointerOverwrite( BOOL bOverwrite )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbChildPtrOverwrite == bOverwrite )
        return;

    mbChildPtrOverwrite  = bOverwrite;

    // Pointer evt. direkt umsetzen
    if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpFrame->SetPointer( ImplGetMousePointer() );
}

// -----------------------------------------------------------------------

void Window::SetPointerPosPixel( const Point& rPos )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Point aPos = ImplOutputToFrame( rPos );
    mpFrame->SetPointerPos( aPos.X(), aPos.Y() );
}

// -----------------------------------------------------------------------

Point Window::GetPointerPosPixel()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return ImplFrameToOutput( Point( mpFrameData->mnLastMouseX, mpFrameData->mnLastMouseY ) );
}

// -----------------------------------------------------------------------

void Window::ShowPointer( BOOL bVisible )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbNoPtrVisible != !bVisible )
    {
        mbNoPtrVisible = !bVisible;

        // Pointer evt. direkt umsetzen
        if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

// -----------------------------------------------------------------------

void Window::EnterWait()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mnWaitCount++;

    if ( mnWaitCount == 1 )
    {
        // Pointer evt. direkt umsetzen
        if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

// -----------------------------------------------------------------------

void Window::LeaveWait()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mnWaitCount )
    {
        mnWaitCount--;

        if ( !mnWaitCount )
        {
            // Pointer evt. direkt umsetzen
            if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
                mpFrame->SetPointer( ImplGetMousePointer() );
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetCursor( Cursor* pCursor )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpCursor != pCursor )
    {
        if ( mpCursor )
            mpCursor->ImplHide();
        mpCursor = pCursor;
        if ( pCursor )
            pCursor->ImplShow();
    }
}

// -----------------------------------------------------------------------

void Window::SetText( const XubString& rStr )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    maText = rStr;

    if ( mpBorderWindow )
        mpBorderWindow->SetText( rStr );
    else if ( mbFrame )
        mpFrame->SetTitle( rStr );

    StateChanged( STATE_CHANGE_TEXT );
}

// -----------------------------------------------------------------------

XubString Window::GetText() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return maText;
}

// -----------------------------------------------------------------------

const XubString& Window::GetHelpText() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !maHelpText.Len() && mnHelpId )
    {
        if ( !IsDialog() && (mnType != WINDOW_TABPAGE) && (mnType != WINDOW_FLOATINGWINDOW) )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                ((Window*)this)->maHelpText = pHelp->GetHelpText( GetHelpId() );
        }
    }

    return maHelpText;
}

// -----------------------------------------------------------------------

Window* Window::FindWindow( const Point& rPos ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Point aPos = OutputToScreenPixel( rPos );
    return ((Window*)this)->ImplFindWindow( aPos );
}

// -----------------------------------------------------------------------

USHORT Window::GetChildCount() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    USHORT  nChildCount = 0;
    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        nChildCount++;
        pChild = pChild->mpNext;
    }

    return nChildCount;
}

// -----------------------------------------------------------------------

Window* Window::GetChild( USHORT nChild ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    USHORT  nChildCount = 0;
    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        if ( nChild == nChildCount )
            return pChild;
        pChild = pChild->mpNext;
        nChildCount++;
    }

    return NULL;
}

// -----------------------------------------------------------------------

Window* Window::GetWindow( USHORT nType ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    switch ( nType )
    {
        case WINDOW_PARENT:
            return mpRealParent;

        case WINDOW_FIRSTCHILD:
            return mpFirstChild;

        case WINDOW_LASTCHILD:
            return mpLastChild;

        case WINDOW_PREV:
            return mpPrev;

        case WINDOW_NEXT:
            return mpNext;

        case WINDOW_FIRSTOVERLAP:
            return mpFirstOverlap;

        case WINDOW_LASTOVERLAP:
            return mpLastOverlap;

        case WINDOW_OVERLAP:
            if ( ImplIsOverlapWindow() )
                return (Window*)this;
            else
                return mpOverlapWindow;

        case WINDOW_PARENTOVERLAP:
            if ( ImplIsOverlapWindow() )
                return mpOverlapWindow;
            else
                return mpOverlapWindow->mpOverlapWindow;

        case WINDOW_CLIENT:
            return ((Window*)this)->ImplGetWindow();

        case WINDOW_REALPARENT:
            return ImplGetParent();

        case WINDOW_FRAME:
            return mpFrameWindow;

        case WINDOW_BORDER:
            if ( mpBorderWindow )
                return mpBorderWindow->GetWindow( WINDOW_BORDER );
            return (Window*)this;
    }

    return NULL;
}

// -----------------------------------------------------------------------

BOOL Window::IsChild( const Window* pWindow, BOOL bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_CHKOBJ( pWindow, Window, ImplDbgCheckWindow );

    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return TRUE;
    }
    while ( pWindow );

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Window::IsWindowOrChild( const Window* pWindow, BOOL bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_CHKOBJ( pWindow, Window, ImplDbgCheckWindow );

    if ( this == pWindow )
        return TRUE;
    return ImplIsChild( pWindow, bSystemWindow );
}

// -----------------------------------------------------------------------

const SystemEnvData* Window::GetSystemData() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

#ifndef REMOTE_APPSERVER
    return mpFrame->GetSystemData();
#else
    return NULL;
#endif
}

::com::sun::star::uno::Any Window::GetSystemDataAny() const
{
    ::com::sun::star::uno::Any aRet;
    const SystemEnvData* pSysData = GetSystemData();
    if( pSysData )
    {
        ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( (sal_Int8*)pSysData, pSysData->nSize );
        aRet <<= aSeq;
    }
    return aRet;
}

// -----------------------------------------------------------------------

void Window::SetWindowPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer, VCLXWindow* pVCLXWindow  )
{
    mxWindowPeer = xPeer;
    mpVCLXWindow = pVCLXWindow;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > Window::GetComponentInterface( BOOL bCreate )
{
    if ( !mxWindowPeer.is() && bCreate )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
        if ( pWrapper )
            mxWindowPeer = pWrapper->GetWindowInterface( this, sal_True );
    }
    return mxWindowPeer;
}

// -----------------------------------------------------------------------

void Window::SetComponentInterface( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xIFace )
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    DBG_ASSERT( pWrapper, "SetComponentInterface: No Wrapper!" );
    if ( pWrapper )
        pWrapper->SetWindowInterface( this, xIFace );
}

// -----------------------------------------------------------------------

void Window::ImplCallDeactivateListeners( Window *pNew )
{
    // Ich werde nicht deaktiviert, wenn das neu aktivierte Window ein Child von mir ist
    if ( !pNew || !ImplIsChild( pNew ) )
    {
        if ( mxWindowPeer.is() )
            Application::GetUnoWrapper()->WindowEvent_Activate( this, FALSE );
        if ( ImplGetParent() )
            ImplGetParent()->ImplCallDeactivateListeners( pNew );
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallActivateListeners( Window *pOld )
{
    // Ich werde nicht aktiviert, wenn das alte aktive Fenster ein Child von mir ist
    if ( !pOld || !ImplIsChild( pOld ) )
    {
        if ( mxWindowPeer.is() )
            // Hier muss noch irgendwie pOld mitgeschickt werden!
            Application::GetUnoWrapper()->WindowEvent_Activate( this, TRUE );
        if ( ImplGetParent() )
            ImplGetParent()->ImplCallActivateListeners( pOld );
    }
}
