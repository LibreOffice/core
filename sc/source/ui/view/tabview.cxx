/*************************************************************************
 *
 *  $RCSfile: tabview.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 10:15:15 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

//------------------------------------------------------------------

#if 0
#define _MACRODLG_HXX
#define _BIGINT_HXX
#define _SVCONTNR_HXX
#define BASIC_NODIALOGS
#define _SFXMNUITEM_HXX
#define _SVDXOUT_HXX
#define _SVDATTR_HXX
#define _SFXMNUITEM_HXX
#define _DLGCFG_HXX
#define _SFXMNUMGR_HXX
#define _SFXBASIC_HXX
#define _MODALDLG_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXMGR_HXX
#define _BASE_DLGS_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUMGR_HXX
#define _SFXSTBITEM_HXX
#define _SFXTBXCTRL_HXX
#define _PASSWD_HXX
//#define _SFXFILEDLG_HXX
//#define _SFXREQUEST_HXX
#define _SFXOBJFACE_HXX

#define _SDR_NOTRANSFORM
#define _SVDXOUT_HXX
#endif
///////////////////////////////////////////////////////////////////////////
// NODRAW.HXX
// Erweiterte Konstanten, um CLOKs mit SVDRAW.HXX zu vermeiden
// Die u.a. Aenderungen nehmen vorgeschlagene Konstante vorweg
///////////////////////////////////////////////////////////////////////////

#if 0
#define _SDR_NOTRANSFORM        // Transformationen, selten verwendet
#define _SDR_NOTOUCH            // Hit-Tests, selten verwendet

#define _SDR_NOEXTDEV           // ExtOutputDevice
#define _SDR_NOUNDO             // Undo-Objekte
#define _SDR_NOSURROGATEOBJ     // SdrObjSurrogate
#define _SDR_NOPAGEOBJ          // SdrPageObj
#define _SDR_NOVIRTOBJ          // SdrVirtObj
#define _SDR_NOGROUPOBJ         // SdrGroupObj
#define _SDR_NOTEXTOBJ          // SdrTextObj
#define _SDR_NOPATHOBJ          // SdrPathObj
#define _SDR_NOEDGEOBJ          // SdrEdgeObj
#define _SDR_NORECTOBJ          // SdrRectObj
#define _SDR_NOCAPTIONOBJ       // SdrCaptionObj
#define _SDR_NOCIRCLEOBJ        // SdrCircleObj
#define _SDR_NOGRAFOBJ          // SdrGrafObj
#define _SDR_NOOLE2OBJ          // SdrOle2Obj
#endif

// Dieses define entfernt die VCControls aus SI.HXX

#define _SI_HXX                 // VCControls

////////////////////// Umsetzen der Standard-Defines //////////////////////

#ifdef _SDR_NOEXTDEV
    #define _SDVXOUT_HXX
    #undef _SDR_NOEXTDEV
#endif

//#define _SVDDRAG_HXX      // SdrDragStat
#define _SVDPAGE_HXX        // SdrPage

#ifdef _SDR_NOSURROGATEOBJ
    #undef _SDR_NOSURROGATEOBJ
    #define _SVDSURO_HXX
#endif

#ifdef _SDR_NOPAGEOBJ
    #undef _SDR_NOPAGEOBJ
    #define _SVDOPAGE_HXX
#endif

#ifdef _SDR_NOVIRTOBJ
    #undef _SDR_NOVIRTOBJ
    #define _SVDOVIRT_HXX
#endif

#ifdef _SDR_NOGROUPOBJ
    #undef _SDR_NOGROUPOBJ
    #define _SVDOGRP_HXX
#endif

#ifdef _SDR_NOTEXTOBJ
    #undef _SDR_NOTEXTOBJ
    #define _SVDOTEXT_HXX
#endif

#ifdef _SDR_NOPATHOBJ
    #undef _SDR_NOPATHOBJ
    #define _SVDOPATH_HXX
#endif

#ifdef _SDR_NOEDGEOBJ
    #undef _SDR_NOEDGEOBJ
    #define _SVDOEDGE_HXX
#endif

#ifdef _SDR_NORECTOBJ
    #undef _SDR_NORECTOBJ
    #define _SVDORECT_HXX
#else
    #undef _SDVOTEXT_OBJ
#endif

#ifdef _SDR_NOCAPTIONOBJ
    #undef _SDR_NOCAPTIONOBJ
    #define _SVDCAPT_HXX
#endif

#ifdef _SDR_NOCIRCLEOBJ
    #undef _SDR_NOCIRCLEOBJ
    #define _SVDOCIRC_HXX
#endif

#ifdef _SDR_NOGRAFOBJ
    #undef _SDR_NOGRAFOBJ
    #define _SVDOGRAF_HXX
#else
    #undef _SVDOTEXT_HXX
    #undef _SVDORECT_HXX
#endif

#ifdef _SDR_NOOLE2OBJ
    #undef _SDR_NOOLE2OBJ
    #define _SVDOOLE2_HXX
#else
    #undef _SVDOTEXT_HXX
    #undef _SVDORECT_HXX
#endif

//#ifdef _SDR_NOVIEWS
//  #define _SVDDRAG_HXX
//#endif

////////////////////// Ende der SVDRAW-Modifikationen /////////////////////

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/ipfrm.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/help.hxx>
#include <rtl/logfile.hxx>

#include "tabview.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "gridwin.hxx"
#include "olinewin.hxx"
#include "olinetab.hxx"
#include "tabsplit.hxx"
#include "colrowba.hxx"
#include "tabcont.hxx"
#include "scmod.hxx"
#include "sc.hrc"
#include "viewutil.hxx"
#include "globstr.hrc"
#include "drawview.hxx"
#include "docsh.hxx"
#include "viewuno.hxx"
#include "AccessibilityHints.hxx"

#include <string>
#include <algorithm>

#define SPLIT_MARGIN    30
#define SC_ICONSIZE     36

#define SC_SCROLLBAR_MIN    30
#define SC_TABBAR_MIN       6

//  fuer Rad-Maus
#define SC_DELTA_ZOOM   10

// STATIC DATA -----------------------------------------------------------


//==================================================================

//  Corner-Button

ScCornerButton::ScCornerButton( Window* pParent, ScViewData* pData, BOOL bAdditional ) :
    Window( pParent, WinBits( 0 ) ),
    pViewData( pData ),
    bAdd( bAdditional )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( rStyleSettings.GetFaceColor() );
    EnableRTL( FALSE );
}

__EXPORT ScCornerButton::~ScCornerButton()
{
}

void __EXPORT ScCornerButton::Paint( const Rectangle& rRect )
{
    Size aSize = GetOutputSizePixel();
    long nPosX = aSize.Width()-1;
    long nPosY = aSize.Height()-1;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    Window::Paint(rRect);

    BOOL bLayoutRTL = pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
    long nDarkX = bLayoutRTL ? 0 : nPosX;

    //  both buttons have the same look now - only dark right/bottom lines
    SetLineColor( rStyleSettings.GetDarkShadowColor() );
    DrawLine( Point(0,nPosY), Point(nPosX,nPosY) );
    DrawLine( Point(nDarkX,0), Point(nDarkX,nPosY) );
}

void ScCornerButton::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( rStyleSettings.GetFaceColor() );
    Invalidate();
}

// -----------------------------------------------------------------------

void ScCornerButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( rStyleSettings.GetFaceColor() );
    Invalidate();
}


void __EXPORT ScCornerButton::Resize()
{
    Invalidate();
}

void __EXPORT ScCornerButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    ScModule* pScMod = SC_MOD();
    BOOL bDisable = pScMod->IsFormulaMode() || pScMod->IsModalMode();
    if (!bDisable)
    {
        ScTabViewShell* pViewSh = pViewData->GetViewShell();
        pViewSh->SetActive();                                   // Appear und SetViewFrame
        pViewSh->ActiveGrabFocus();

        BOOL bControl = rMEvt.IsMod1();
        pViewSh->SelectAll( bControl );
    }
}

//==================================================================

BOOL lcl_HasColOutline( const ScViewData& rViewData )
{
    const ScOutlineTable* pTable = rViewData.GetDocument()->GetOutlineTable(rViewData.GetTabNo());
    if (pTable)
    {
        const ScOutlineArray* pArray = pTable->GetColArray();
        if ( pArray->GetDepth() > 0 )
            return TRUE;
    }
    return FALSE;
}

BOOL lcl_HasRowOutline( const ScViewData& rViewData )
{
    const ScOutlineTable* pTable = rViewData.GetDocument()->GetOutlineTable(rViewData.GetTabNo());
    if (pTable)
    {
        const ScOutlineArray* pArray = pTable->GetRowArray();
        if ( pArray->GetDepth() > 0 )
            return TRUE;
    }
    return FALSE;
}

//==================================================================

//  Init und Konstruktoren
//  ScTabView::Init() in tabview5.cxx wegen out of keys


#define TABVIEW_INIT    \
            aFunctionSet( &aViewData ),                                     \
            aHdrFunc( &aViewData ),                                         \
            aHScrollLeft( pFrameWin, WinBits( WB_HSCROLL | WB_DRAG ) ),     \
            aHScrollRight( pFrameWin, WinBits( WB_HSCROLL | WB_DRAG ) ),    \
            aVScrollTop( pFrameWin, WinBits( WB_VSCROLL | WB_DRAG ) ),      \
            aVScrollBottom( pFrameWin, WinBits( WB_VSCROLL | WB_DRAG ) ),   \
            aCornerButton( pFrameWin, &aViewData, FALSE ),                  \
            aTopButton( pFrameWin, &aViewData, TRUE ),                      \
            aScrollBarBox( pFrameWin, WB_SIZEABLE ),                        \
            pInputHintWindow( NULL ),                                       \
            pPageBreakData( NULL ),                                         \
            pHighlightRanges( NULL ),                                       \
            pBrushDocument( NULL ),                                         \
            pDrawBrushSet( NULL ),                                          \
            bLockPaintBrush( FALSE ),                                       \
            bDragging( FALSE ),                                             \
            bIsBlockMode( FALSE ),                                          \
            bBlockNeg( FALSE ),                                             \
            bBlockCols( FALSE ),                                            \
            bBlockRows( FALSE ),                                            \
            nTipVisible( 0 ),                                               \
            pDrawView( NULL ),                                              \
            bDrawSelMode( FALSE ),                                          \
            bMinimized( FALSE ),                                            \
            pTimerWindow( NULL ),                                           \
            pSelEngine( NULL ),                                             \
            pHdrSelEng( NULL ),                                             \
            bInUpdateHeader( FALSE ),                                       \
            bInActivatePart( FALSE ),                                       \
            bInZoomUpdate( FALSE ),                                         \
            bMoveIsShift( FALSE ),                                          \
            bNewStartIfMarking( FALSE )


ScTabView::ScTabView( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
            pFrameWin( pParent ),
            aViewData( &rDocSh, pViewShell ),
            eZoomType( SVX_ZOOM_PERCENT ),
            TABVIEW_INIT
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScTabView::ScTabView" );

    Init();
}

ScTabView::ScTabView( Window* pParent, const ScTabView& rScTabView, ScTabViewShell* pViewShell ) :
            pFrameWin( pParent ),
            aViewData( rScTabView.aViewData ),
            eZoomType( rScTabView.eZoomType ),
            TABVIEW_INIT
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScTabView::ScTabView" );

    aViewData.SetViewShell( pViewShell );
    Init();

    UpdateShow();
    if ( aViewData.GetActivePart() != SC_SPLIT_BOTTOMLEFT )
        pGridWin[SC_SPLIT_BOTTOMLEFT]->Show();

    InvalidateSplit();
}

void ScTabView::InitScrollBar( ScrollBar& rScrollBar, long nMaxVal )
{
    rScrollBar.SetRange( Range( 0, nMaxVal ) );
    rScrollBar.SetLineSize( 1 );
    rScrollBar.SetPageSize( 1 );                // wird getrennt abgefragt
    rScrollBar.SetVisibleSize( 10 );            // wird bei Resize neu gesetzt

    rScrollBar.SetScrollHdl( LINK(this, ScTabView, ScrollHdl) );
    rScrollBar.SetEndScrollHdl( LINK(this, ScTabView, EndScrollHdl) );
}

//  Scroll-Timer

void ScTabView::SetTimer( ScGridWindow* pWin, const MouseEvent& rMEvt )
{
    pTimerWindow = pWin;
    aTimerMEvt   = rMEvt;
    aScrollTimer.Start();
}

void ScTabView::ResetTimer()
{
    aScrollTimer.Stop();
    pTimerWindow = NULL;
}

IMPL_LINK( ScTabView, TimerHdl, Timer*, pTimer )
{
    DBG_ASSERT( pTimer == &aScrollTimer, "Unbekannter Timer" );

//  aScrollTimer.Stop();
    if (pTimerWindow)
        pTimerWindow->MouseMove( aTimerMEvt );

    return 0;
}

// --- Resize ---------------------------------------------------------------------

void lcl_SetPosSize( Window& rWindow, const Point& rPos, const Size& rSize,
                        long nTotalWidth, BOOL bLayoutRTL )
{
    Point aNewPos = rPos;
    if ( bLayoutRTL )
    {
        aNewPos.X() = nTotalWidth - rPos.X() - rSize.Width();
        if ( aNewPos == rWindow.GetPosPixel() && rSize.Width() != rWindow.GetSizePixel().Width() )
        {
            //  Document windows are manually painted right-to-left, so they need to
            //  be repainted if the size changes.
            rWindow.Invalidate();
        }
    }
    rWindow.SetPosSizePixel( aNewPos, rSize );
}

void ScTabView::DoResize( const Point& rOffset, const Size& rSize, BOOL bInner )
{
    HideListBox();

    BOOL bHasHint = ( pInputHintWindow != NULL );
    if (bHasHint)
        RemoveHintWindow();

    BOOL bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );
    long nTotalWidth = rSize.Width();
    if ( bLayoutRTL )
        nTotalWidth += 2*rOffset.X();

    BOOL bVScroll    = aViewData.IsVScrollMode();
    BOOL bHScroll    = aViewData.IsHScrollMode();
    BOOL bTabControl = aViewData.IsTabMode();
    BOOL bHeaders    = aViewData.IsHeaderMode();
    BOOL bOutlMode   = aViewData.IsOutlineMode();
    BOOL bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    BOOL bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);

    //  Scrollbar-Einstellungen koennen vom Sfx ueberschrieben werden:
    SfxScrollingMode eMode = aViewData.GetViewShell()->GetScrollingMode();
    if ( eMode == SCROLLING_NO )
        bHScroll = bVScroll = FALSE;
    else if ( eMode == SCROLLING_YES || eMode == SCROLLING_AUTO )   //! Auto ???
        bHScroll = bVScroll = TRUE;

    if ( aViewData.GetDocShell()->IsPreview() )
        bHScroll = bVScroll = bTabControl = bHeaders = bOutlMode = bHOutline = bVOutline = FALSE;

    long nBarX = 0;
    long nBarY = 0;
    long nOutlineX;
    long nOutlineY;
    long nOutPosX;
    long nOutPosY;

    long nPosX = rOffset.X();
    long nPosY = rOffset.Y();
    long nSizeX = rSize.Width();
    long nSizeY = rSize.Height();
    long nSize1;

    bMinimized = ( nSizeX<=SC_ICONSIZE || nSizeY<=SC_ICONSIZE );
    if ( bMinimized )
        return;

    long nSplitSizeX = SPLIT_HANDLE_SIZE;
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        nSplitSizeX = 1;
    long nSplitSizeY = SPLIT_HANDLE_SIZE;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        nSplitSizeY = 1;

    const long nOverlap = 0;    // ScrollBar::GetWindowOverlapPixel();

    aBorderPos = rOffset;
    aFrameSize = rSize;

    if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
        if ( aViewData.GetHSplitPos() > nSizeX - SPLIT_MARGIN )
        {
            aViewData.SetHSplitMode( SC_SPLIT_NONE );
            if ( WhichH( aViewData.GetActivePart() ) == SC_SPLIT_RIGHT )
                ActivatePart( SC_SPLIT_BOTTOMLEFT );
            InvalidateSplit();
//          UpdateShow();
        }
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        if ( aViewData.GetVSplitPos() > nSizeY - SPLIT_MARGIN )
        {
            aViewData.SetVSplitMode( SC_SPLIT_NONE );
            if ( WhichV( aViewData.GetActivePart() ) == SC_SPLIT_TOP )
                ActivatePart( SC_SPLIT_BOTTOMLEFT );
            InvalidateSplit();
//          UpdateShow();
        }

    UpdateShow();

    if (bHScroll || bVScroll)       // Scrollbars horizontal oder vertikal
    {
        long nScrollBarSize = pFrameWin->GetSettings().GetStyleSettings().GetScrollBarSize();
        if (bVScroll)
        {
//          nBarX = aVScrollBottom.GetSizePixel().Width();
            nBarX = nScrollBarSize;
            nSizeX -= nBarX - nOverlap;
        }
        if (bHScroll)
        {
//          nBarY = aHScrollLeft.GetSizePixel().Height();
            nBarY = nScrollBarSize;
            nSizeY -= nBarY - nOverlap;
        }

        //  window at the bottom right
        lcl_SetPosSize( aScrollBarBox, Point( nPosX+nSizeX, nPosY+nSizeY ), Size( nBarX, nBarY ),
                        nTotalWidth, bLayoutRTL );

        if (bHScroll)                               // Scrollbars horizontal
        {
            long nSizeLt;           // linker Scrollbar
            long nSizeRt;           // rechter Scrollbar
            long nSizeSp;           // Splitter

            switch (aViewData.GetHSplitMode())
            {
                case SC_SPLIT_NONE:
                    nSizeSp = nSplitSizeX;
                    nSizeLt = nSizeX - nSizeSp + nOverlap;          // Ecke ueberdecken
                    break;
                case SC_SPLIT_NORMAL:
                    nSizeSp = nSplitSizeX;
                    nSizeLt = aViewData.GetHSplitPos();
                    break;
                case SC_SPLIT_FIX:
                    nSizeSp = 0;
                    nSizeLt = 0;
                    break;
            }
            nSizeRt = nSizeX - nSizeLt - nSizeSp;

            long nTabSize = 0;
            if (bTabControl)
            {
                nTabSize = pTabControl->GetSizePixel().Width()-nOverlap;

                if ( aViewData.GetHSplitMode() != SC_SPLIT_FIX )    // bei linkem Scrollbar
                {
                    if (nTabSize > nSizeLt-SC_SCROLLBAR_MIN) nTabSize = nSizeLt-SC_SCROLLBAR_MIN;
                    if (nTabSize < SC_TABBAR_MIN) nTabSize = SC_TABBAR_MIN;
                    nSizeLt -= nTabSize;
                }
                else                                                // bei rechtem Scrollbar
                {
                    if (nTabSize > nSizeRt-SC_SCROLLBAR_MIN) nTabSize = nSizeRt-SC_SCROLLBAR_MIN;
                    if (nTabSize < SC_TABBAR_MIN) nTabSize = SC_TABBAR_MIN;
                    nSizeRt -= nTabSize;
                }
            }

            lcl_SetPosSize( *pTabControl, Point(nPosX-nOverlap, nPosY+nSizeY),
                                                Size(nTabSize+nOverlap, nBarY), nTotalWidth, bLayoutRTL );
            pTabControl->SetSheetLayoutRTL( bLayoutRTL );

            lcl_SetPosSize( aHScrollLeft, Point(nPosX+nTabSize-nOverlap, nPosY+nSizeY),
                                                Size(nSizeLt+2*nOverlap, nBarY), nTotalWidth, bLayoutRTL );
            lcl_SetPosSize( *pHSplitter, Point( nPosX+nTabSize+nSizeLt, nPosY+nSizeY ),
                                            Size( nSizeSp, nBarY ), nTotalWidth, bLayoutRTL );
            lcl_SetPosSize( aHScrollRight, Point(nPosX+nTabSize+nSizeLt+nSizeSp-nOverlap,
                                                    nPosY+nSizeY),
                                            Size(nSizeRt+2*nOverlap, nBarY), nTotalWidth, bLayoutRTL );

            //  SetDragRectPixel is done below
        }

        if (bVScroll)                               // Scrollbars vertikal
        {
            long nSizeUp;           // oberer Scrollbar
            long nSizeSp;           // Splitter
            long nSizeDn;           // unterer Scrollbar

            switch (aViewData.GetVSplitMode())
            {
                case SC_SPLIT_NONE:
                    nSizeUp = 0;
                    nSizeSp = nSplitSizeY;
                    break;
                case SC_SPLIT_NORMAL:
                    nSizeUp = aViewData.GetVSplitPos();
                    nSizeSp = nSplitSizeY;
                    break;
                case SC_SPLIT_FIX:
                    nSizeUp = 0;
                    nSizeSp = 0;
                    break;
            }
            nSizeDn = nSizeY - nSizeUp - nSizeSp;

            lcl_SetPosSize( aVScrollTop, Point(nPosX+nSizeX, nPosY-nOverlap),
                                            Size(nBarX,nSizeUp+2*nOverlap), nTotalWidth, bLayoutRTL );
            lcl_SetPosSize( *pVSplitter, Point( nPosX+nSizeX, nPosY+nSizeUp ),
                                            Size( nBarX, nSizeSp ), nTotalWidth, bLayoutRTL );
            lcl_SetPosSize( aVScrollBottom, Point(nPosX+nSizeX,
                                                nPosY+nSizeUp+nSizeSp-nOverlap),
                                            Size(nBarX, nSizeDn+2*nOverlap), nTotalWidth, bLayoutRTL );

            //  SetDragRectPixel is done below
        }
    }

    //  SetDragRectPixel auch ohne Scrollbars etc., wenn schon gesplittet ist
    if ( bHScroll || aViewData.GetHSplitMode() != SC_SPLIT_NONE )
        pHSplitter->SetDragRectPixel(
            Rectangle( nPosX, nPosY, nPosX+nSizeX, nPosY+nSizeY ), pFrameWin );
    if ( bVScroll || aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        pVSplitter->SetDragRectPixel(
            Rectangle( nPosX, nPosY, nPosX+nSizeX, nPosY+nSizeY ), pFrameWin );

    if (bTabControl && ! bHScroll )
    {
        nBarY = aHScrollLeft.GetSizePixel().Height();
        nBarX = aVScrollBottom.GetSizePixel().Width();

        nSize1 = nSizeX + nOverlap;

        long nTabSize = nSize1;
        if (nTabSize < 0) nTabSize = 0;

        lcl_SetPosSize( *pTabControl, Point(nPosX-nOverlap, nPosY+nSizeY-nBarY),
                                        Size(nTabSize+nOverlap, nBarY), nTotalWidth, bLayoutRTL );
        nSizeY -= nBarY - nOverlap;
        lcl_SetPosSize( aScrollBarBox, Point( nPosX+nSizeX, nPosY+nSizeY ), Size( nBarX, nBarY ),
                                        nTotalWidth, bLayoutRTL );

        if( bVScroll )
        {
            Size aVScrSize = aVScrollBottom.GetSizePixel();
            aVScrSize.Height() -= nBarY;
            aVScrollBottom.SetSizePixel( aVScrSize );
        }
    }

    nOutPosX = nPosX;
    nOutPosY = nPosY;

    // Outline-Controls
    if (bVOutline && pRowOutline[SC_SPLIT_BOTTOM])
    {
        nOutlineX = pRowOutline[SC_SPLIT_BOTTOM]->GetDepthSize();
        nSizeX -= nOutlineX;
        nPosX += nOutlineX;
    }
    if (bHOutline && pColOutline[SC_SPLIT_LEFT])
    {
        nOutlineY = pColOutline[SC_SPLIT_LEFT]->GetDepthSize();
        nSizeY -= nOutlineY;
        nPosY += nOutlineY;
    }

    if (bHeaders)                               // Spalten/Zeilen-Header
    {
        nBarX = pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width();
        nBarY = pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height();
        nSizeX -= nBarX;
        nSizeY -= nBarY;
        nPosX += nBarX;
        nPosY += nBarY;
    }
    else
        nBarX = nBarY = 0;

        //
        //      Splitter auswerten
        //

    long nLeftSize   = nSizeX;
    long nRightSize  = 0;
    long nTopSize    = 0;
    long nBottomSize = nSizeY;
    long nSplitPosX  = nPosX;
    long nSplitPosY  = nPosY;

    if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
    {
        long nSplitHeight = rSize.Height();
        if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        {
            //  Fixier-Splitter nicht mit Scrollbar/TabBar ueberlappen lassen
            if ( bHScroll )
                nSplitHeight -= aHScrollLeft.GetSizePixel().Height();
            else if ( bTabControl && pTabControl )
                nSplitHeight -= pTabControl->GetSizePixel().Height();
        }
        nSplitPosX = aViewData.GetHSplitPos();
        lcl_SetPosSize( *pHSplitter,
            Point( nSplitPosX, nOutPosY ), Size( nSplitSizeX, nSplitHeight ), nTotalWidth, bLayoutRTL );
        nLeftSize = nSplitPosX - nPosX;
        nSplitPosX += nSplitSizeX;
        nRightSize = nSizeX - nLeftSize - nSplitSizeX;
    }
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
    {
        long nSplitWidth = rSize.Width();
        if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX && bVScroll )
            nSplitWidth -= aVScrollBottom.GetSizePixel().Width();
        nSplitPosY = aViewData.GetVSplitPos();
        lcl_SetPosSize( *pVSplitter,
            Point( nOutPosX, nSplitPosY ), Size( nSplitWidth, nSplitSizeY ), nTotalWidth, bLayoutRTL );
        nTopSize = nSplitPosY - nPosY;
        nSplitPosY += nSplitSizeY;
        nBottomSize = nSizeY - nTopSize - nSplitSizeY;
    }

    //  ShowHide fuer pColOutline / pRowOutline passiert in UpdateShow

    if (bHOutline)                              // Outline-Controls
    {
        if (pColOutline[SC_SPLIT_LEFT])
        {
            pColOutline[SC_SPLIT_LEFT]->SetHeaderSize( nBarX );
            lcl_SetPosSize( *pColOutline[SC_SPLIT_LEFT],
                    Point(nPosX-nBarX,nOutPosY), Size(nLeftSize+nBarX,nOutlineY), nTotalWidth, bLayoutRTL );
        }
        if (pColOutline[SC_SPLIT_RIGHT])
        {
            pColOutline[SC_SPLIT_RIGHT]->SetHeaderSize( 0 );    // always call to update RTL flag
            lcl_SetPosSize( *pColOutline[SC_SPLIT_RIGHT],
                    Point(nSplitPosX,nOutPosY), Size(nRightSize,nOutlineY), nTotalWidth, bLayoutRTL );
        }
    }
    if (bVOutline)
    {
        if (nTopSize)
        {
            if (pRowOutline[SC_SPLIT_TOP] && pRowOutline[SC_SPLIT_BOTTOM])
            {
                pRowOutline[SC_SPLIT_TOP]->SetHeaderSize( nBarY );
                lcl_SetPosSize( *pRowOutline[SC_SPLIT_TOP],
                        Point(nOutPosX,nPosY-nBarY), Size(nOutlineX,nTopSize+nBarY), nTotalWidth, bLayoutRTL );
                pRowOutline[SC_SPLIT_BOTTOM]->SetHeaderSize( 0 );
                lcl_SetPosSize( *pRowOutline[SC_SPLIT_BOTTOM],
                        Point(nOutPosX,nSplitPosY), Size(nOutlineX,nBottomSize), nTotalWidth, bLayoutRTL );
            }
        }
        else if (pRowOutline[SC_SPLIT_BOTTOM])
        {
            pRowOutline[SC_SPLIT_BOTTOM]->SetHeaderSize( nBarY );
            lcl_SetPosSize( *pRowOutline[SC_SPLIT_BOTTOM],
                    Point(nOutPosX,nSplitPosY-nBarY), Size(nOutlineX,nBottomSize+nBarY), nTotalWidth, bLayoutRTL );
        }
    }
    if (bHOutline && bVOutline)
    {
        lcl_SetPosSize( aTopButton, Point(nOutPosX,nOutPosY), Size(nOutlineX,nOutlineY), nTotalWidth, bLayoutRTL );
        aTopButton.Show();
    }
    else
        aTopButton.Hide();

    if (bHeaders)                               // Spalten/Zeilen-Header
    {
        lcl_SetPosSize( *pColBar[SC_SPLIT_LEFT],
            Point(nPosX,nPosY-nBarY), Size(nLeftSize,nBarY), nTotalWidth, bLayoutRTL );
        if (pColBar[SC_SPLIT_RIGHT])
            lcl_SetPosSize( *pColBar[SC_SPLIT_RIGHT],
                Point(nSplitPosX,nPosY-nBarY), Size(nRightSize,nBarY), nTotalWidth, bLayoutRTL );

        if (pRowBar[SC_SPLIT_TOP])
            lcl_SetPosSize( *pRowBar[SC_SPLIT_TOP],
                Point(nPosX-nBarX,nPosY), Size(nBarX,nTopSize), nTotalWidth, bLayoutRTL );
        lcl_SetPosSize( *pRowBar[SC_SPLIT_BOTTOM],
            Point(nPosX-nBarX,nSplitPosY), Size(nBarX,nBottomSize), nTotalWidth, bLayoutRTL );

        lcl_SetPosSize( aCornerButton, Point(nPosX-nBarX,nPosY-nBarY), Size(nBarX,nBarY), nTotalWidth, bLayoutRTL );
        aCornerButton.Show();
        pColBar[SC_SPLIT_LEFT]->Show();
        pRowBar[SC_SPLIT_BOTTOM]->Show();
    }
    else
    {
        aCornerButton.Hide();
        pColBar[SC_SPLIT_LEFT]->Hide();         // immer da
        pRowBar[SC_SPLIT_BOTTOM]->Hide();
    }


                                            // Grid-Windows

    if (bInner)
    {
        long nInnerPosX = bLayoutRTL ? ( nTotalWidth - nPosX - nLeftSize ) : nPosX;
        pGridWin[SC_SPLIT_BOTTOMLEFT]->SetPosPixel( Point(nInnerPosX,nSplitPosY) );
    }
    else
    {
        lcl_SetPosSize( *pGridWin[SC_SPLIT_BOTTOMLEFT],
            Point(nPosX,nSplitPosY), Size(nLeftSize,nBottomSize), nTotalWidth, bLayoutRTL );
        if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
            lcl_SetPosSize( *pGridWin[SC_SPLIT_BOTTOMRIGHT],
                Point(nSplitPosX,nSplitPosY), Size(nRightSize,nBottomSize), nTotalWidth, bLayoutRTL );
        if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
            lcl_SetPosSize( *pGridWin[SC_SPLIT_TOPLEFT],
                Point(nPosX,nPosY), Size(nLeftSize,nTopSize), nTotalWidth, bLayoutRTL );
        if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE && aViewData.GetVSplitMode() != SC_SPLIT_NONE )
            lcl_SetPosSize( *pGridWin[SC_SPLIT_TOPRIGHT],
                Point(nSplitPosX,nPosY), Size(nRightSize,nTopSize), nTotalWidth, bLayoutRTL );
    }

                //
                //  Scrollbars updaten
                //

    if (!bInUpdateHeader)
    {
        UpdateScrollBars();     // Scrollbars nicht beim Scrollen neu setzen
        UpdateHeaderWidth();

        InterpretVisible();     // #69343# have everything calculated before painting
    }

    if (bHasHint)
        TestHintWindow();       // neu positionieren

    UpdateVarZoom();    //  update variable zoom types (after resizing GridWindows)

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxSimpleHint(SC_HINT_ACC_WINDOWRESIZED));
}

void ScTabView::UpdateVarZoom()
{
    //  update variable zoom types

    if ( eZoomType != SVX_ZOOM_PERCENT && !bInZoomUpdate )
    {
        bInZoomUpdate = TRUE;
        const Fraction& rOldX = GetViewData()->GetZoomX();
        const Fraction& rOldY = GetViewData()->GetZoomY();
        long nOldPercent = ( rOldY.GetNumerator() * 100 ) / rOldY.GetDenominator();
        USHORT nNewZoom = CalcZoom( eZoomType, (USHORT)nOldPercent );
        Fraction aNew( nNewZoom, 100 );

        if ( aNew != rOldX || aNew != rOldY )
        {
            SetZoom( aNew, aNew );
            PaintGrid();
            PaintTop();
            PaintLeft();
            aViewData.GetViewShell()->GetViewFrame()->GetBindings().
                Invalidate( SID_ATTR_ZOOM );
        }
        bInZoomUpdate = FALSE;
    }
}

void ScTabView::UpdateFixPos()
{
    BOOL bResize = FALSE;
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        if (aViewData.UpdateFixX())
            bResize = TRUE;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        if (aViewData.UpdateFixY())
            bResize = TRUE;
    if (bResize)
        RepeatResize(FALSE);
}

void ScTabView::RepeatResize( BOOL bUpdateFix )
{
    if ( bUpdateFix )
    {
        if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
            aViewData.UpdateFixX();
        if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
            aViewData.UpdateFixY();
    }

    DoResize( aBorderPos, aFrameSize );

    //! Border muss neu gesetzt werden ???
}

void ScTabView::GetBorderSize( SvBorder& rBorder, const Size& rSize )
{
    BOOL bScrollBars = aViewData.IsVScrollMode();
    BOOL bTabControl = aViewData.IsTabMode();
    BOOL bHeaders    = aViewData.IsHeaderMode();
    BOOL bOutlMode   = aViewData.IsOutlineMode();
    BOOL bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    BOOL bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);
    BOOL bLayoutRTL  = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );

    rBorder = SvBorder();

    if (bScrollBars)                            // Scrollbars horizontal oder vertikal
    {
        rBorder.Right()  += aVScrollBottom.GetSizePixel().Width();
        rBorder.Bottom() += aHScrollLeft.GetSizePixel().Height();
    }

    // Outline-Controls
    if (bVOutline && pRowOutline[SC_SPLIT_BOTTOM])
        rBorder.Left() += pRowOutline[SC_SPLIT_BOTTOM]->GetDepthSize();
    if (bHOutline && pColOutline[SC_SPLIT_LEFT])
        rBorder.Top()  += pColOutline[SC_SPLIT_LEFT]->GetDepthSize();

    if (bHeaders)                               // Spalten/Zeilen-Header
    {
        rBorder.Left() += pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width();
        rBorder.Top()  += pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height();
    }

    if ( bLayoutRTL )
        ::std::swap( rBorder.Left(), rBorder.Right() );
}

IMPL_LINK( ScTabView, TabBarResize, void*, EMPTY_ARG )
{
    BOOL bHScrollMode = aViewData.IsHScrollMode();

    //  Scrollbar-Einstellungen koennen vom Sfx ueberschrieben werden:
    SfxScrollingMode eMode = aViewData.GetViewShell()->GetScrollingMode();
    if ( eMode == SCROLLING_NO )
        bHScrollMode = FALSE;
    else if ( eMode == SCROLLING_YES || eMode == SCROLLING_AUTO )   //! Auto ???
        bHScrollMode = TRUE;

    if( bHScrollMode )
    {
        const long nOverlap = 0;    // ScrollBar::GetWindowOverlapPixel();
        long nSize = pTabControl->GetSplitSize();

        if (aViewData.GetHSplitMode() != SC_SPLIT_FIX)
        {
            long nMax = pHSplitter->GetPosPixel().X();
            if( pTabControl->IsMirrored() != Application::GetSettings().GetLayoutRTL() )
                nMax = pFrameWin->GetSizePixel().Width() - nMax;
            --nMax;
            if (nSize>nMax) nSize = nMax;
        }

        if ( nSize != pTabControl->GetSizePixel().Width() )
        {
            pTabControl->SetSizePixel( Size( nSize+nOverlap,
                                        pTabControl->GetSizePixel().Height() ) );
            RepeatResize();
        }
    }

    return 0;
}

void ScTabView::SetTabBarWidth( long nNewWidth )
{
    Size aSize = pTabControl->GetSizePixel();

    if ( aSize.Width() != nNewWidth )
    {
        aSize.Width() = nNewWidth;
        pTabControl->SetSizePixel( aSize );
    }
}

long ScTabView::GetTabBarWidth()
{
    return pTabControl->GetSizePixel().Width();
}

Window* ScTabView::GetActiveWin()
{
    ScSplitPos ePos = aViewData.GetActivePart();
    DBG_ASSERT(pGridWin[ePos],"kein aktives Fenster");
    return pGridWin[ePos];
}

Window* ScTabView::GetWindowByPos( ScSplitPos ePos )
{
    return pGridWin[ePos];
}

void ScTabView::SetActivePointer( const Pointer& rPointer )
{
    for (USHORT i=0; i<4; i++)
        if (pGridWin[i])
            pGridWin[i]->SetPointer( rPointer );

/*  ScSplitPos ePos = aViewData.GetActivePart();
    if (pGridWin[ePos])
        pGridWin[ePos]->SetPointer( rPointer );
*/
}

void ScTabView::SetActivePointer( const ResId& rId )
{
    DBG_ERRORFILE( "keine Pointer mit ResId!" );
}

void ScTabView::ActiveGrabFocus()
{
    ScSplitPos ePos = aViewData.GetActivePart();
    if (pGridWin[ePos])
        pGridWin[ePos]->GrabFocus();
}

void ScTabView::ActiveCaptureMouse()
{
    ScSplitPos ePos = aViewData.GetActivePart();
    if (pGridWin[ePos])
        pGridWin[ePos]->CaptureMouse();
}

void ScTabView::ActiveReleaseMouse()
{
    ScSplitPos ePos = aViewData.GetActivePart();
    if (pGridWin[ePos])
        pGridWin[ePos]->ReleaseMouse();
}

Point ScTabView::ActivePixelToLogic( const Point& rDevicePoint )
{
    ScSplitPos ePos = aViewData.GetActivePart();
    if (pGridWin[ePos])
        return pGridWin[ePos]->PixelToLogic(rDevicePoint);
    else
        return Point();
}

ScSplitPos ScTabView::FindWindow( Window* pWindow ) const
{
    ScSplitPos eVal = SC_SPLIT_BOTTOMLEFT;      // Default
    for (USHORT i=0; i<4; i++)
        if ( pGridWin[i] == pWindow )
            eVal = (ScSplitPos) i;

    return eVal;
}

Point ScTabView::GetGridOffset() const
{
    Point aPos;

        // Groessen hier wie in DoResize

    BOOL bHeaders    = aViewData.IsHeaderMode();
    BOOL bOutlMode   = aViewData.IsOutlineMode();
    BOOL bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    BOOL bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);

    // Outline-Controls
    if (bVOutline && pRowOutline[SC_SPLIT_BOTTOM])
        aPos.X() += pRowOutline[SC_SPLIT_BOTTOM]->GetDepthSize();
    if (bHOutline && pColOutline[SC_SPLIT_LEFT])
        aPos.Y() += pColOutline[SC_SPLIT_LEFT]->GetDepthSize();

    if (bHeaders)                               // Spalten/Zeilen-Header
    {
        if (pRowBar[SC_SPLIT_BOTTOM])
            aPos.X() += pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width();
        if (pColBar[SC_SPLIT_LEFT])
            aPos.Y() += pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height();
    }

    return aPos;
}

// ---  Scroll-Bars  --------------------------------------------------------

BOOL ScTabView::ScrollCommand( const CommandEvent& rCEvt, ScSplitPos ePos )
{
    HideNoteMarker();

    BOOL bDone = FALSE;
    const CommandWheelData* pData = rCEvt.GetWheelData();
    if ( pData && pData->GetMode() == COMMAND_WHEEL_ZOOM )
    {
        if ( !aViewData.GetViewShell()->GetViewFrame()->ISA(SfxInPlaceFrame) )
        {
            //  for ole inplace editing, the scale is defined by the visarea and client size
            //  and can't be changed directly

            const Fraction& rOldY = aViewData.GetZoomY();
            long nOld = (long)(( rOldY.GetNumerator() * 100 ) / rOldY.GetDenominator());
            long nNew = nOld;
            if ( pData->GetDelta() < 0 )
                nNew = Max( (long) MINZOOM, (long)( nOld - SC_DELTA_ZOOM ) );
            else
                nNew = Min( (long) MAXZOOM, (long)( nOld + SC_DELTA_ZOOM ) );

            if ( nNew != nOld )
            {
                //! Zoom an AppOptions merken ???

                SetZoomType( SVX_ZOOM_PERCENT );
                Fraction aFract( nNew, 100 );
                SetZoom( aFract, aFract );
                PaintGrid();
                PaintTop();
                PaintLeft();
                aViewData.GetBindings().Invalidate( SID_ATTR_ZOOM );
            }

            bDone = TRUE;
        }
    }
    else
    {
        ScHSplitPos eHPos = WhichH(ePos);
        ScVSplitPos eVPos = WhichV(ePos);
        ScrollBar* pHScroll = ( eHPos == SC_SPLIT_LEFT ) ? &aHScrollLeft : &aHScrollRight;
        ScrollBar* pVScroll = ( eVPos == SC_SPLIT_TOP )  ? &aVScrollTop  : &aVScrollBottom;
        if ( pGridWin[ePos] )
            bDone = pGridWin[ePos]->HandleScrollCommand( rCEvt, pHScroll, pVScroll );
    }
    return bDone;
}

IMPL_LINK( ScTabView, EndScrollHdl, ScrollBar*, pScroll )
{
    BOOL bOnlineScroll = TRUE;      //! Optionen

    if ( bDragging )
    {
        if ( bOnlineScroll )                // nur Ranges aktualisieren
            UpdateScrollBars();
        else
        {
            long nScrollMin = 0;        // RangeMin simulieren
            if ( aViewData.GetHSplitMode()==SC_SPLIT_FIX && pScroll == &aHScrollRight )
                nScrollMin = aViewData.GetFixPosX();
            if ( aViewData.GetVSplitMode()==SC_SPLIT_FIX && pScroll == &aVScrollBottom )
                nScrollMin = aViewData.GetFixPosY();

            if ( pScroll == &aHScrollLeft || pScroll == &aHScrollRight )
            {
                BOOL bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );
                ScHSplitPos eWhich = (pScroll == &aHScrollLeft) ? SC_SPLIT_LEFT : SC_SPLIT_RIGHT;
                long nDelta = GetScrollBarPos( *pScroll, bLayoutRTL ) + nScrollMin - aViewData.GetPosX(eWhich);
                if (nDelta) ScrollX( nDelta, eWhich );
            }
            else                            // VScroll...
            {
                ScVSplitPos eWhich = (pScroll == &aVScrollTop) ? SC_SPLIT_TOP : SC_SPLIT_BOTTOM;
                long nDelta = GetScrollBarPos( *pScroll, FALSE ) + nScrollMin - aViewData.GetPosY(eWhich);
                if (nDelta) ScrollY( nDelta, eWhich );
            }
        }
        bDragging = FALSE;
    }
    return 0;
}

IMPL_LINK( ScTabView, ScrollHdl, ScrollBar*, pScroll )
{
    BOOL bOnlineScroll = TRUE;      //! Optionen

    BOOL bHoriz = ( pScroll == &aHScrollLeft || pScroll == &aHScrollRight );
    long nViewPos;
    if ( bHoriz )
        nViewPos = aViewData.GetPosX( (pScroll == &aHScrollLeft) ?
                                        SC_SPLIT_LEFT : SC_SPLIT_RIGHT );
    else
        nViewPos = aViewData.GetPosY( (pScroll == &aVScrollTop) ?
                                        SC_SPLIT_TOP : SC_SPLIT_BOTTOM );

    BOOL bLayoutRTL = bHoriz && aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );

    ScrollType eType = pScroll->GetType();
    if ( eType == SCROLL_DRAG )
    {
        if (!bDragging)
        {
            bDragging = TRUE;
            nPrevDragPos = nViewPos;
        }

        //  Scroll-Position anzeigen
        //  (nur QuickHelp, in der Statuszeile gibt es keinen Eintrag dafuer)

        if (Help::IsQuickHelpEnabled())
        {
            Point aMousePos = pScroll->OutputToNormalizedScreenPixel(pScroll->GetPointerPosPixel());
            long nScrollMin = 0;        // RangeMin simulieren
            if ( aViewData.GetHSplitMode()==SC_SPLIT_FIX && pScroll == &aHScrollRight )
                nScrollMin = aViewData.GetFixPosX();
            if ( aViewData.GetVSplitMode()==SC_SPLIT_FIX && pScroll == &aVScrollBottom )
                nScrollMin = aViewData.GetFixPosY();

            String aHelpStr;
            long nScrollPos = GetScrollBarPos( *pScroll, bLayoutRTL ) + nScrollMin;
            Point aPos = pScroll->GetParent()->OutputToNormalizedScreenPixel(pScroll->GetPosPixel());
            Size aSize = pScroll->GetSizePixel();
            Rectangle aRect;
            USHORT nAlign;
            if (bHoriz)
            {
                aHelpStr = ScGlobal::GetRscString(STR_COLUMN);
                aHelpStr += ' ';
                aHelpStr += ColToAlpha((SCCOL) nScrollPos);

                aRect.Left() = aMousePos.X();
                aRect.Top()  = aPos.Y() - 4;
                nAlign       = QUICKHELP_BOTTOM|QUICKHELP_CENTER;
            }
            else
            {
                aHelpStr = ScGlobal::GetRscString(STR_ROW);
                aHelpStr += ' ';
                aHelpStr += String::CreateFromInt32(nScrollPos + 1);

                aRect.Left() = aPos.X() - 8;
                aRect.Top()  = aMousePos.Y();
                nAlign       = QUICKHELP_RIGHT|QUICKHELP_VCENTER;
            }
            aRect.Right()   = aRect.Left();
            aRect.Bottom()  = aRect.Top();

            Help::ShowQuickHelp(pScroll->GetParent(), aRect, aHelpStr, nAlign);
        }
    }

    if ( bOnlineScroll || eType != SCROLL_DRAG )
    {
        if ( bLayoutRTL )
        {
            // change scroll type so visible/previous cells calculation below remains the same
            switch ( eType )
            {
                case SCROLL_LINEUP:     eType = SCROLL_LINEDOWN;    break;
                case SCROLL_LINEDOWN:   eType = SCROLL_LINEUP;      break;
                case SCROLL_PAGEUP:     eType = SCROLL_PAGEDOWN;    break;
                case SCROLL_PAGEDOWN:   eType = SCROLL_PAGEUP;      break;
            }
        }
        long nDelta = pScroll->GetDelta();
        switch ( eType )
        {
            case SCROLL_LINEUP:
                nDelta = -1;
                break;
            case SCROLL_LINEDOWN:
                nDelta = 1;
                break;
            case SCROLL_PAGEUP:
                if ( pScroll == &aHScrollLeft ) nDelta = -(long) aViewData.PrevCellsX( SC_SPLIT_LEFT );
                if ( pScroll == &aHScrollRight ) nDelta = -(long) aViewData.PrevCellsX( SC_SPLIT_RIGHT );
                if ( pScroll == &aVScrollTop ) nDelta = -(long) aViewData.PrevCellsY( SC_SPLIT_TOP );
                if ( pScroll == &aVScrollBottom ) nDelta = -(long) aViewData.PrevCellsY( SC_SPLIT_BOTTOM );
                if (nDelta==0) nDelta=-1;
                break;
            case SCROLL_PAGEDOWN:
                if ( pScroll == &aHScrollLeft ) nDelta = aViewData.VisibleCellsX( SC_SPLIT_LEFT );
                if ( pScroll == &aHScrollRight ) nDelta = aViewData.VisibleCellsX( SC_SPLIT_RIGHT );
                if ( pScroll == &aVScrollTop ) nDelta = aViewData.VisibleCellsY( SC_SPLIT_TOP );
                if ( pScroll == &aVScrollBottom ) nDelta = aViewData.VisibleCellsY( SC_SPLIT_BOTTOM );
                if (nDelta==0) nDelta=1;
                break;
            case SCROLL_DRAG:
                {
                    //  nur in die richtige Richtung scrollen, nicht um ausgeblendete
                    //  Bereiche herumzittern

                    long nScrollMin = 0;        // RangeMin simulieren
                    if ( aViewData.GetHSplitMode()==SC_SPLIT_FIX && pScroll == &aHScrollRight )
                        nScrollMin = aViewData.GetFixPosX();
                    if ( aViewData.GetVSplitMode()==SC_SPLIT_FIX && pScroll == &aVScrollBottom )
                        nScrollMin = aViewData.GetFixPosY();

                    long nScrollPos = GetScrollBarPos( *pScroll, bLayoutRTL ) + nScrollMin;
                    nDelta = nScrollPos - nViewPos;
                    if ( nScrollPos > nPrevDragPos )
                    {
                        if (nDelta<0) nDelta=0;
                    }
                    else if ( nScrollPos < nPrevDragPos )
                    {
                        if (nDelta>0) nDelta=0;
                    }
                    else
                        nDelta = 0;
                    nPrevDragPos = nScrollPos;
                }
                break;
        }

        if (nDelta)
        {
            BOOL bUpdate = ( eType != SCROLL_DRAG );    // bei Drag die Ranges nicht aendern
            if ( bHoriz )
                ScrollX( nDelta, (pScroll == &aHScrollLeft) ? SC_SPLIT_LEFT : SC_SPLIT_RIGHT, bUpdate );
            else
                ScrollY( nDelta, (pScroll == &aVScrollTop) ? SC_SPLIT_TOP : SC_SPLIT_BOTTOM, bUpdate );
        }
    }

    return 0;
}

void ScTabView::ScrollX( long nDeltaX, ScHSplitPos eWhich, BOOL bUpdBars )
{
    BOOL bHasHint = ( pInputHintWindow != NULL );
    if (bHasHint)
        RemoveHintWindow();

    SCCOL nOldX = aViewData.GetPosX(eWhich);
    SCsCOL nNewX = static_cast<SCsCOL>(nOldX) + static_cast<SCsCOL>(nDeltaX);
    if ( nNewX < 0 )
    {
        nDeltaX -= nNewX;
        nNewX = 0;
    }
    if ( nNewX > MAXCOL )
    {
        nDeltaX -= nNewX - MAXCOL;
        nNewX = MAXCOL;
    }

    SCsCOL nDir = ( nDeltaX > 0 ) ? 1 : -1;
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    while ( ( pDoc->GetColFlags( nNewX, nTab ) & CR_HIDDEN ) &&
            nNewX+nDir >= 0 && nNewX+nDir <= MAXCOL )
        nNewX += nDir;

    //  Fixierung

    if (aViewData.GetHSplitMode() == SC_SPLIT_FIX)
    {
        if (eWhich == SC_SPLIT_LEFT)
            nNewX = static_cast<SCsCOL>(nOldX);                             // links immer stehenlassen
        else
        {
            SCsCOL nFixX = static_cast<SCsCOL>(aViewData.GetFixPosX());
            if (nNewX < nFixX)
                nNewX = nFixX;
        }
    }
    if (nNewX == static_cast<SCsCOL>(nOldX))
        return;

    HideAllCursors();

    if ( nNewX >= 0 && nNewX <= MAXCOL && nDeltaX )
    {
        SCCOL nTrackX = std::max( nOldX, static_cast<SCCOL>(nNewX) );

            //  Auf dem Mac wird in Window::Scroll vor dem Scrollen Update gerufen,
            //  so dass mit den neuen Variablen an die alte Position ausgegeben wuerde.
            //  Deshalb ein Update vorneweg (vor SetPosX), auf welches Fenster ist beim Mac egal.
#ifdef MAC
        pGridWin[SC_SPLIT_BOTTOMLEFT]->Update();
#endif
            //  Mit VCL wirkt Update() im Moment immer auf alle Fenster, beim Update
            //  nach dem Scrollen des GridWindow's wuerde darum der Col-/RowBar evtl.
            //  mit schon geaenderter Pos. gepainted werden -
            //  darum vorher einmal Update am Col-/RowBar

        if (pColBar[eWhich])
            pColBar[eWhich]->Update();

        long nOldPos = aViewData.GetScrPos( nTrackX, 0, eWhich ).X();
        aViewData.SetPosX( eWhich, static_cast<SCCOL>(nNewX) );
        long nDiff = aViewData.GetScrPos( nTrackX, 0, eWhich ).X() - nOldPos;

        if ( eWhich==SC_SPLIT_LEFT )
        {
            pGridWin[SC_SPLIT_BOTTOMLEFT]->ScrollPixel( nDiff, 0 );
            if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
                pGridWin[SC_SPLIT_TOPLEFT]->ScrollPixel( nDiff, 0 );
        }
        else
        {
            pGridWin[SC_SPLIT_BOTTOMRIGHT]->ScrollPixel( nDiff, 0 );
            if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
                pGridWin[SC_SPLIT_TOPRIGHT]->ScrollPixel( nDiff, 0 );
        }
        if (pColBar[eWhich])     { pColBar[eWhich]->Scroll( nDiff,0 ); pColBar[eWhich]->Update(); }
        if (pColOutline[eWhich]) pColOutline[eWhich]->ScrollPixel( nDiff );
        if (bUpdBars)
            UpdateScrollBars();
    }

    if (nDeltaX==1 || nDeltaX==-1)
        pGridWin[aViewData.GetActivePart()]->Update();

    ShowAllCursors();

    SetNewVisArea();            // MapMode muss schon gesetzt sein

    if (bHasHint)
        TestHintWindow();       // neu positionieren
}

void ScTabView::ScrollY( long nDeltaY, ScVSplitPos eWhich, BOOL bUpdBars )
{
    BOOL bHasHint = ( pInputHintWindow != NULL );
    if (bHasHint)
        RemoveHintWindow();

    SCROW nOldY = aViewData.GetPosY(eWhich);
    SCsROW nNewY = static_cast<SCsROW>(nOldY) + static_cast<SCsROW>(nDeltaY);
    if ( nNewY < 0 )
    {
        nDeltaY -= nNewY;
        nNewY = 0;
    }
    if ( nNewY > MAXROW )
    {
        nDeltaY -= nNewY - MAXROW;
        nNewY = MAXROW;
    }

    SCsROW nDir = ( nDeltaY > 0 ) ? 1 : -1;
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    while ( ( pDoc->GetRowFlags( nNewY, nTab ) & CR_HIDDEN ) &&
            nNewY+nDir >= 0 && nNewY+nDir <= MAXROW )
        nNewY += nDir;

    //  Fixierung

    if (aViewData.GetVSplitMode() == SC_SPLIT_FIX)
    {
        if (eWhich == SC_SPLIT_TOP)
            nNewY = static_cast<SCsROW>(nOldY);                             // oben immer stehenlassen
        else
        {
            SCsROW nFixY = static_cast<SCsROW>(aViewData.GetFixPosY());
            if (nNewY < nFixY)
                nNewY = nFixY;
        }
    }
    if (nNewY == static_cast<SCsROW>(nOldY))
        return;

    HideAllCursors();

    if ( nNewY >= 0 && nNewY <= MAXROW && nDeltaY )
    {
        SCROW nTrackY = std::max( nOldY, static_cast<SCROW>(nNewY) );

        //  Zeilenkoepfe anpassen vor dem eigentlichen Scrolling, damit nicht
        //  doppelt gepainted werden muss
        //  PosY darf dann auch noch nicht umgesetzt sein, neuen Wert uebergeben
        SCROW nUNew = static_cast<SCROW>(nNewY);
        UpdateHeaderWidth( &eWhich, &nUNew );               // Zeilenkoepfe anpassen

#ifdef MAC
        pGridWin[SC_SPLIT_BOTTOMLEFT]->Update();
#endif

        if (pRowBar[eWhich])
            pRowBar[eWhich]->Update();

        long nOldPos = aViewData.GetScrPos( 0, nTrackY, eWhich ).Y();
        aViewData.SetPosY( eWhich, static_cast<SCROW>(nNewY) );
        long nDiff = aViewData.GetScrPos( 0, nTrackY, eWhich ).Y() - nOldPos;

        if ( eWhich==SC_SPLIT_TOP )
        {
            pGridWin[SC_SPLIT_TOPLEFT]->ScrollPixel( 0, nDiff );
            if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
                pGridWin[SC_SPLIT_TOPRIGHT]->ScrollPixel( 0, nDiff );
        }
        else
        {
            pGridWin[SC_SPLIT_BOTTOMLEFT]->ScrollPixel( 0, nDiff );
            if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
                pGridWin[SC_SPLIT_BOTTOMRIGHT]->ScrollPixel( 0, nDiff );
        }
        if (pRowBar[eWhich])     { pRowBar[eWhich]->Scroll( 0,nDiff ); pRowBar[eWhich]->Update(); }
        if (pRowOutline[eWhich]) pRowOutline[eWhich]->ScrollPixel( nDiff );
        if (bUpdBars)
            UpdateScrollBars();
    }

    if (nDeltaY==1 || nDeltaY==-1)
        pGridWin[aViewData.GetActivePart()]->Update();

    ShowAllCursors();

    SetNewVisArea();            // MapMode muss schon gesetzt sein

    if (bHasHint)
        TestHintWindow();       // neu positionieren
}

void ScTabView::ScrollLines( long nDeltaX, long nDeltaY )
{
    ScSplitPos eWhich = aViewData.GetActivePart();
    if (nDeltaX)
        ScrollX(nDeltaX,WhichH(eWhich));
    if (nDeltaY)
        ScrollY(nDeltaY,WhichV(eWhich));
}

SCROW lcl_LastVisible( ScViewData& rViewData )
{
    //  wenn am Dokumentende viele Zeilen ausgeblendet sind (welcher Trottel macht sowas?),
    //  soll dadurch nicht auf breite Zeilenkoepfe geschaltet werden
    //! als Member ans Dokument ???

    ScDocument* pDoc = rViewData.GetDocument();
    SCTAB nTab = rViewData.GetTabNo();

    SCROW nVis = MAXROW;
    while ( nVis > 0 && pDoc->FastGetRowHeight( nVis, nTab ) == 0 )
        --nVis;
    return nVis;
}

void ScTabView::UpdateHeaderWidth( const ScVSplitPos* pWhich, const SCROW* pPosY )
{
    if ( !pRowBar[SC_SPLIT_BOTTOM] || MAXROW < 10000 )
        return;

    SCROW nEndPos = MAXROW;
    if ( !aViewData.GetViewShell()->GetViewFrame()->ISA(SfxInPlaceFrame) )
    {
        //  fuer OLE Inplace immer MAXROW

        if ( pWhich && *pWhich == SC_SPLIT_BOTTOM && pPosY )
            nEndPos = *pPosY;
        else
            nEndPos = aViewData.GetPosY( SC_SPLIT_BOTTOM );
        nEndPos += aViewData.CellsAtY( nEndPos, 1, SC_SPLIT_BOTTOM, SC_SIZE_NONE ); // VisibleCellsY
        if (nEndPos > MAXROW)
            nEndPos = lcl_LastVisible( aViewData );

        if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        {
            SCROW nTopEnd;
            if ( pWhich && *pWhich == SC_SPLIT_TOP && pPosY )
                nTopEnd = *pPosY;
            else
                nTopEnd = aViewData.GetPosY( SC_SPLIT_TOP );
            nTopEnd += aViewData.CellsAtY( nTopEnd, 1, SC_SPLIT_TOP, SC_SIZE_NONE );// VisibleCellsY
            if (nTopEnd > MAXROW)
                nTopEnd = lcl_LastVisible( aViewData );

            if ( nTopEnd > nEndPos )
                nEndPos = nTopEnd;
        }
    }

    long nSmall = pRowBar[SC_SPLIT_BOTTOM]->GetSmallWidth();
    long nBig   = pRowBar[SC_SPLIT_BOTTOM]->GetBigWidth();
    long nDiff  = nBig - nSmall;

    if (nEndPos>10000)
        nEndPos = 10000;
    else if (nEndPos<1)     // avoid extra step at 0 (when only one row is visible)
        nEndPos = 1;
    long nWidth = nBig - ( 10000 - nEndPos ) * nDiff / 10000;

    if ( nWidth != pRowBar[SC_SPLIT_BOTTOM]->GetWidth() && !bInUpdateHeader )
    {
        bInUpdateHeader = TRUE;

        pRowBar[SC_SPLIT_BOTTOM]->SetWidth( nWidth );
        if (pRowBar[SC_SPLIT_TOP])
            pRowBar[SC_SPLIT_TOP]->SetWidth( nWidth );

        RepeatResize();

        // auf VCL gibt's Update ohne Ende (jedes Update gilt fuer alle Fenster)
        //aCornerButton.Update();       // der bekommt sonst nie ein Update

        bInUpdateHeader = FALSE;
    }
}

inline void ShowHide( Window* pWin, BOOL bShow )
{
    DBG_ASSERT(pWin || !bShow, "Fenster ist nicht da");
    if (pWin)
        pWin->Show(bShow);
}

void ScTabView::UpdateShow()
{
    BOOL bHScrollMode = aViewData.IsHScrollMode();
    BOOL bVScrollMode = aViewData.IsVScrollMode();
    BOOL bTabMode     = aViewData.IsTabMode();
    BOOL bOutlMode    = aViewData.IsOutlineMode();
    BOOL bHOutline    = bOutlMode && lcl_HasColOutline(aViewData);
    BOOL bVOutline    = bOutlMode && lcl_HasRowOutline(aViewData);
    BOOL bHeader      = aViewData.IsHeaderMode();

    BOOL bShowH = ( aViewData.GetHSplitMode() != SC_SPLIT_NONE );
    BOOL bShowV = ( aViewData.GetVSplitMode() != SC_SPLIT_NONE );

    //  Scrollbar-Einstellungen koennen vom Sfx ueberschrieben werden:
    SfxScrollingMode eMode = aViewData.GetViewShell()->GetScrollingMode();
    if ( eMode == SCROLLING_NO )
        bHScrollMode = bVScrollMode = FALSE;
    else if ( eMode == SCROLLING_YES || eMode == SCROLLING_AUTO )   //! Auto ???
        bHScrollMode = bVScrollMode = TRUE;

    if ( aViewData.GetDocShell()->IsPreview() )
        bHScrollMode = bVScrollMode = bTabMode = bHeader = bOutlMode = bHOutline = bVOutline = FALSE;

        //
        //  Windows anlegen
        //

    if (bShowH && !pGridWin[SC_SPLIT_BOTTOMRIGHT])
    {
        pGridWin[SC_SPLIT_BOTTOMRIGHT] = new ScGridWindow( pFrameWin, &aViewData, SC_SPLIT_BOTTOMRIGHT );
        DoAddWin( pGridWin[SC_SPLIT_BOTTOMRIGHT] );
    }
    if (bShowV && !pGridWin[SC_SPLIT_TOPLEFT])
    {
        pGridWin[SC_SPLIT_TOPLEFT] = new ScGridWindow( pFrameWin, &aViewData, SC_SPLIT_TOPLEFT );
        DoAddWin( pGridWin[SC_SPLIT_TOPLEFT] );
    }
    if (bShowH && bShowV && !pGridWin[SC_SPLIT_TOPRIGHT])
    {
        pGridWin[SC_SPLIT_TOPRIGHT] = new ScGridWindow( pFrameWin, &aViewData, SC_SPLIT_TOPRIGHT );
        DoAddWin( pGridWin[SC_SPLIT_TOPRIGHT] );
    }

    if (bHOutline && !pColOutline[SC_SPLIT_LEFT])
        pColOutline[SC_SPLIT_LEFT] = new ScOutlineWindow( pFrameWin, SC_OUTLINE_HOR, &aViewData, SC_SPLIT_BOTTOMLEFT );
    if (bShowH && bHOutline && !pColOutline[SC_SPLIT_RIGHT])
        pColOutline[SC_SPLIT_RIGHT] = new ScOutlineWindow( pFrameWin, SC_OUTLINE_HOR, &aViewData, SC_SPLIT_BOTTOMRIGHT );

    if (bVOutline && !pRowOutline[SC_SPLIT_BOTTOM])
        pRowOutline[SC_SPLIT_BOTTOM] = new ScOutlineWindow( pFrameWin, SC_OUTLINE_VER, &aViewData, SC_SPLIT_BOTTOMLEFT );
    if (bShowV && bVOutline && !pRowOutline[SC_SPLIT_TOP])
        pRowOutline[SC_SPLIT_TOP] = new ScOutlineWindow( pFrameWin, SC_OUTLINE_VER, &aViewData, SC_SPLIT_TOPLEFT );

    if (bShowH && bHeader && !pColBar[SC_SPLIT_RIGHT])
        pColBar[SC_SPLIT_RIGHT] = new ScColBar( pFrameWin, &aViewData, SC_SPLIT_RIGHT,
                                                &aHdrFunc, pHdrSelEng );
    if (bShowV && bHeader && !pRowBar[SC_SPLIT_TOP])
        pRowBar[SC_SPLIT_TOP] = new ScRowBar( pFrameWin, &aViewData, SC_SPLIT_TOP,
                                                &aHdrFunc, pHdrSelEng );

        //
        //  Windows anzeigen
        //

    ShowHide( &aHScrollLeft, bHScrollMode );
    ShowHide( &aHScrollRight, bShowH && bHScrollMode );
    ShowHide( &aVScrollBottom, bVScrollMode );
    ShowHide( &aVScrollTop, bShowV && bVScrollMode );
    ShowHide( &aScrollBarBox, bVScrollMode || bHScrollMode );

    ShowHide( pHSplitter, bHScrollMode || bShowH );         // immer angelegt
    ShowHide( pVSplitter, bVScrollMode || bShowV );
    ShowHide( pTabControl, bTabMode );

                                                    // ab hier dynamisch angelegte

    ShowHide( pGridWin[SC_SPLIT_BOTTOMRIGHT], bShowH );
    ShowHide( pGridWin[SC_SPLIT_TOPLEFT], bShowV );
    ShowHide( pGridWin[SC_SPLIT_TOPRIGHT], bShowH && bShowV );

    ShowHide( pColOutline[SC_SPLIT_LEFT], bHOutline );
    ShowHide( pColOutline[SC_SPLIT_RIGHT], bShowH && bHOutline );

    ShowHide( pRowOutline[SC_SPLIT_BOTTOM], bVOutline );
    ShowHide( pRowOutline[SC_SPLIT_TOP], bShowV && bVOutline );

    ShowHide( pColBar[SC_SPLIT_RIGHT], bShowH && bHeader );
    ShowHide( pRowBar[SC_SPLIT_TOP], bShowV && bHeader );


    //! neue Gridwindows eintragen
}

// ---  Splitter  --------------------------------------------------------

IMPL_LINK( ScTabView, SplitHdl, Splitter*, pSplitter )
{
    if ( pSplitter == pHSplitter )
        DoHSplit( pHSplitter->GetSplitPosPixel() );
    else
        DoVSplit( pVSplitter->GetSplitPosPixel() );

    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX || aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        FreezeSplitters( TRUE );

    DoResize( aBorderPos, aFrameSize );

    return 0;
}

void ScTabView::DoHSplit(long nSplitPos)
{
    //  nSplitPos is the real pixel position on the frame window,
    //  mirroring for RTL has to be done here.

    BOOL bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );
    if ( bLayoutRTL )
        nSplitPos = pFrameWin->GetOutputSizePixel().Width() - nSplitPos - 1;

    long nMinPos;
    long nMaxPos;
    SCCOL nOldDelta;
    SCCOL nNewDelta;

    nMinPos = SPLIT_MARGIN;
    if ( pRowBar[SC_SPLIT_BOTTOM] && pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width() >= nMinPos )
        nMinPos = pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width() + 1;
    nMaxPos = aFrameSize.Width() - SPLIT_MARGIN;

    ScSplitMode aOldMode = aViewData.GetHSplitMode();
    ScSplitMode aNewMode = SC_SPLIT_NORMAL;

    aViewData.SetHSplitPos( nSplitPos );
    if ( nSplitPos < nMinPos || nSplitPos > nMaxPos )
        aNewMode = SC_SPLIT_NONE;

    aViewData.SetHSplitMode( aNewMode );

    if ( aNewMode != aOldMode )
    {
        UpdateShow();       // vor ActivatePart !!

        if ( aNewMode == SC_SPLIT_NONE )
        {
            if (aViewData.GetActivePart() == SC_SPLIT_TOPRIGHT)
                ActivatePart( SC_SPLIT_TOPLEFT );
            if (aViewData.GetActivePart() == SC_SPLIT_BOTTOMRIGHT)
                ActivatePart( SC_SPLIT_BOTTOMLEFT );
        }
        else
        {
            nOldDelta = aViewData.GetPosX( SC_SPLIT_LEFT );
//          aViewData.SetPosX( SC_SPLIT_LEFT, nOldDelta );
            long nLeftWidth = nSplitPos - pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width();
            if ( nLeftWidth < 0 ) nLeftWidth = 0;
            nNewDelta = nOldDelta + aViewData.CellsAtX( nOldDelta, 1, SC_SPLIT_LEFT,
                            (USHORT) nLeftWidth );
            if ( nNewDelta > MAXCOL )
                nNewDelta = MAXCOL;
            aViewData.SetPosX( SC_SPLIT_RIGHT, nNewDelta );
            if ( nNewDelta > aViewData.GetCurX() )
                ActivatePart( (WhichV(aViewData.GetActivePart()) == SC_SPLIT_BOTTOM) ?
                    SC_SPLIT_BOTTOMLEFT : SC_SPLIT_TOPLEFT );
            else
                ActivatePart( (WhichV(aViewData.GetActivePart()) == SC_SPLIT_BOTTOM) ?
                    SC_SPLIT_BOTTOMRIGHT : SC_SPLIT_TOPRIGHT );
        }

        //  #61410# Form-Layer muss den sichtbaren Ausschnitt aller Fenster kennen
        //  dafuer muss hier schon der MapMode stimmen
        for (USHORT i=0; i<4; i++)
            if (pGridWin[i])
                pGridWin[i]->SetMapMode( pGridWin[i]->GetDrawMapMode() );
        SetNewVisArea();

        PaintGrid();
        PaintTop();

        InvalidateSplit();
    }
}

void ScTabView::DoVSplit(long nSplitPos)
{
    long nMinPos;
    long nMaxPos;
    SCROW nOldDelta;
    SCROW nNewDelta;

    nMinPos = SPLIT_MARGIN;
    if ( pColBar[SC_SPLIT_LEFT] && pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height() >= nMinPos )
        nMinPos = pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height() + 1;
    nMaxPos = aFrameSize.Height() - SPLIT_MARGIN;

    ScSplitMode aOldMode = aViewData.GetVSplitMode();
    ScSplitMode aNewMode = SC_SPLIT_NORMAL;

    aViewData.SetVSplitPos( nSplitPos );
    if ( nSplitPos < nMinPos || nSplitPos > nMaxPos )
        aNewMode = SC_SPLIT_NONE;

    aViewData.SetVSplitMode( aNewMode );

    if ( aNewMode != aOldMode )
    {
        UpdateShow();       // vor ActivatePart !!

        if ( aNewMode == SC_SPLIT_NONE )
        {
            nOldDelta = aViewData.GetPosY( SC_SPLIT_TOP );
            aViewData.SetPosY( SC_SPLIT_BOTTOM, nOldDelta );

            if (aViewData.GetActivePart() == SC_SPLIT_TOPLEFT)
                ActivatePart( SC_SPLIT_BOTTOMLEFT );
            if (aViewData.GetActivePart() == SC_SPLIT_TOPRIGHT)
                ActivatePart( SC_SPLIT_BOTTOMRIGHT );
        }
        else
        {
            if ( aOldMode == SC_SPLIT_NONE )
                nOldDelta = aViewData.GetPosY( SC_SPLIT_BOTTOM );
            else
                nOldDelta = aViewData.GetPosY( SC_SPLIT_TOP );

            aViewData.SetPosY( SC_SPLIT_TOP, nOldDelta );
            long nTopHeight = nSplitPos - pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height();
            if ( nTopHeight < 0 ) nTopHeight = 0;
            nNewDelta = nOldDelta + aViewData.CellsAtY( nOldDelta, 1, SC_SPLIT_TOP,
                            (USHORT) nTopHeight );
            if ( nNewDelta > MAXROW )
                nNewDelta = MAXROW;
            aViewData.SetPosY( SC_SPLIT_BOTTOM, nNewDelta );
            if ( nNewDelta > aViewData.GetCurY() )
                ActivatePart( (WhichH(aViewData.GetActivePart()) == SC_SPLIT_LEFT) ?
                    SC_SPLIT_TOPLEFT : SC_SPLIT_TOPRIGHT );
            else
                ActivatePart( (WhichH(aViewData.GetActivePart()) == SC_SPLIT_LEFT) ?
                    SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT );
        }

        //  #61410# Form-Layer muss den sichtbaren Ausschnitt aller Fenster kennen
        //  dafuer muss hier schon der MapMode stimmen
        for (USHORT i=0; i<4; i++)
            if (pGridWin[i])
                pGridWin[i]->SetMapMode( pGridWin[i]->GetDrawMapMode() );
        SetNewVisArea();

        PaintGrid();
        PaintLeft();

        InvalidateSplit();
    }
}

Point ScTabView::GetInsertPos()
{
    ScDocument* pDoc = aViewData.GetDocument();
    SCCOL nCol = aViewData.GetCurX();
    SCROW nRow = aViewData.GetCurY();
    SCTAB nTab = aViewData.GetTabNo();
    long nPosX = 0;
    for (SCCOL i=0; i<nCol; i++)
        nPosX += pDoc->GetColWidth(i,nTab);
    nPosX = (long)(nPosX * HMM_PER_TWIPS);
    if ( pDoc->IsNegativePage( nTab ) )
        nPosX = -nPosX;
    long nPosY = 0;
    for (SCROW j=0; j<nRow; j++)
        nPosY += pDoc->GetRowHeight(j,nTab);
    nPosY = (long)(nPosY * HMM_PER_TWIPS);
    return Point(nPosX,nPosY);
}

void ScTabView::LockModifiers( USHORT nModifiers )
{
    pSelEngine->LockModifiers( nModifiers );
    pHdrSelEng->LockModifiers( nModifiers );
}

USHORT ScTabView::GetLockedModifiers() const
{
    return pSelEngine->GetLockedModifiers();
}

Point ScTabView::GetMousePosPixel()
{
    Point aPos;
    ScGridWindow* pWin = (ScGridWindow*)GetActiveWin();

    if ( pWin )
        aPos = pWin->GetMousePosPixel();

    return aPos;
}

BOOL lcl_MouseIsOverWin( const Point& rScreenPosPixel, Window* pWin )
{
    if (pWin)
    {
        //  SPLIT_HANDLE_SIZE draufaddieren, damit das Einrasten genau
        //  auf dem Splitter nicht aussetzt

        Point aRel = pWin->NormalizedScreenToOutputPixel( rScreenPosPixel );
        Size aWinSize = pWin->GetOutputSizePixel();
        if ( aRel.X() >= 0 && aRel.X() < aWinSize.Width() + SPLIT_HANDLE_SIZE &&
                aRel.Y() >= 0 && aRel.Y() < aWinSize.Height() + SPLIT_HANDLE_SIZE )
            return TRUE;
    }
    return FALSE;
}

void ScTabView::SnapSplitPos( Point& rScreenPosPixel )
{
    BOOL bOverWin = FALSE;
    USHORT i;
    for (i=0; i<4; i++)
        if (lcl_MouseIsOverWin(rScreenPosPixel,pGridWin[i]))
            bOverWin = TRUE;

    if (!bOverWin)
        return;

    //  #74761# don't snap to cells if the scale will be modified afterwards
    if ( eZoomType != SVX_ZOOM_PERCENT )
        return;

    ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        ePos = SC_SPLIT_TOPLEFT;

    Window* pWin = pGridWin[ePos];
    if (!pWin)
    {
        DBG_ERROR("Window NULL");
        return;
    }

    Point aMouse = pWin->NormalizedScreenToOutputPixel( rScreenPosPixel );
    SCsCOL nPosX;
    SCsROW nPosY;
    //  #52949# bNextIfLarge=FALSE: nicht auf naechste Zelle, wenn ausserhalb des Fensters
    aViewData.GetPosFromPixel( aMouse.X(), aMouse.Y(), ePos, nPosX, nPosY, TRUE, FALSE, FALSE );
    BOOL bLeft;
    BOOL bTop;
    aViewData.GetMouseQuadrant( aMouse, ePos, nPosX, nPosY, bLeft, bTop );
    if (!bLeft)
        ++nPosX;
    if (!bTop)
        ++nPosY;
    aMouse = aViewData.GetScrPos( static_cast<SCCOL>(nPosX), static_cast<SCROW>(nPosY), ePos, TRUE );
    rScreenPosPixel = pWin->OutputToNormalizedScreenPixel( aMouse );
}

void ScTabView::FreezeSplitters( BOOL bFreeze )
{
    ScSplitMode eOldH = aViewData.GetHSplitMode();
    ScSplitMode eOldV = aViewData.GetVSplitMode();

    ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
    if ( eOldV != SC_SPLIT_NONE )
        ePos = SC_SPLIT_TOPLEFT;
    Window* pWin = pGridWin[ePos];

    BOOL bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );

    if ( bFreeze )
    {
        Point aWinStart = pWin->GetPosPixel();

        Point aSplit;
        SCsCOL nPosX;
        SCsROW nPosY;
        if (eOldH != SC_SPLIT_NONE || eOldV != SC_SPLIT_NONE)
        {
            if (eOldH != SC_SPLIT_NONE)
            {
                long nSplitPos = aViewData.GetHSplitPos();
                if ( bLayoutRTL )
                    nSplitPos = pFrameWin->GetOutputSizePixel().Width() - nSplitPos - 1;
                aSplit.X() = nSplitPos - aWinStart.X();
            }
            if (eOldV != SC_SPLIT_NONE)
                aSplit.Y() = aViewData.GetVSplitPos() - aWinStart.Y();

            aViewData.GetPosFromPixel( aSplit.X(), aSplit.Y(), ePos, nPosX, nPosY );
            BOOL bLeft;
            BOOL bTop;
            aViewData.GetMouseQuadrant( aSplit, ePos, nPosX, nPosY, bLeft, bTop );
            if (!bLeft)
                ++nPosX;
            if (!bTop)
                ++nPosY;
        }
        else
        {
            nPosX = static_cast<SCsCOL>( aViewData.GetCurX());
            nPosY = static_cast<SCsROW>( aViewData.GetCurY());
        }

        SCCOL nLeftPos = aViewData.GetPosX(SC_SPLIT_LEFT);
        SCROW nTopPos = aViewData.GetPosY(SC_SPLIT_BOTTOM);
        SCCOL nRightPos = static_cast<SCCOL>(nPosX);
        SCROW nBottomPos = static_cast<SCROW>(nPosY);
        if (eOldH != SC_SPLIT_NONE)
            if (aViewData.GetPosX(SC_SPLIT_RIGHT) > nRightPos)
                nRightPos = aViewData.GetPosX(SC_SPLIT_RIGHT);
        if (eOldV != SC_SPLIT_NONE)
        {
            nTopPos = aViewData.GetPosY(SC_SPLIT_TOP);
            if (aViewData.GetPosY(SC_SPLIT_BOTTOM) > nBottomPos)
                nBottomPos = aViewData.GetPosY(SC_SPLIT_BOTTOM);
        }

        aSplit = aViewData.GetScrPos( static_cast<SCCOL>(nPosX), static_cast<SCROW>(nPosY), ePos, TRUE );
        if (nPosX > aViewData.GetPosX(SC_SPLIT_LEFT))       // (aSplit.X() > 0) doesn't work for RTL
        {
            long nSplitPos = aSplit.X() + aWinStart.X();
            if ( bLayoutRTL )
                nSplitPos = pFrameWin->GetOutputSizePixel().Width() - nSplitPos - 1;

            aViewData.SetHSplitMode( SC_SPLIT_FIX );
            aViewData.SetHSplitPos( nSplitPos );
            aViewData.SetFixPosX( nPosX );

            aViewData.SetPosX(SC_SPLIT_LEFT, nLeftPos);
            aViewData.SetPosX(SC_SPLIT_RIGHT, nRightPos);
        }
        else
            aViewData.SetHSplitMode( SC_SPLIT_NONE );
        if (aSplit.Y() > 0)
        {
            aViewData.SetVSplitMode( SC_SPLIT_FIX );
            aViewData.SetVSplitPos( aSplit.Y() + aWinStart.Y() );
            aViewData.SetFixPosY( nPosY );

            aViewData.SetPosY(SC_SPLIT_TOP, nTopPos);
            aViewData.SetPosY(SC_SPLIT_BOTTOM, nBottomPos);
        }
        else
            aViewData.SetVSplitMode( SC_SPLIT_NONE );
    }
    else                        // Fixierung aufheben
    {
        if ( eOldH == SC_SPLIT_FIX )
            aViewData.SetHSplitMode( SC_SPLIT_NORMAL );
        if ( eOldV == SC_SPLIT_FIX )
            aViewData.SetVSplitMode( SC_SPLIT_NORMAL );
    }

    //  #61410# Form-Layer muss den sichtbaren Ausschnitt aller Fenster kennen
    //  dafuer muss hier schon der MapMode stimmen
    for (USHORT i=0; i<4; i++)
        if (pGridWin[i])
            pGridWin[i]->SetMapMode( pGridWin[i]->GetDrawMapMode() );
    SetNewVisArea();

    RepeatResize(FALSE);

    UpdateShow();
    PaintLeft();
    PaintTop();
    PaintGrid();

    //  SC_FOLLOW_NONE: only update active part
    AlignToCursor( aViewData.GetCurX(), aViewData.GetCurY(), SC_FOLLOW_NONE );
    UpdateAutoFillMark();

    InvalidateSplit();
}

void ScTabView::RemoveSplit()
{
    DoHSplit( 0 );
    DoVSplit( 0 );
    RepeatResize();
}

void ScTabView::SplitAtCursor()
{
    ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        ePos = SC_SPLIT_TOPLEFT;
    Window* pWin = pGridWin[ePos];
    Point aWinStart = pWin->GetPosPixel();

    SCCOL nPosX = aViewData.GetCurX();
    SCROW nPosY = aViewData.GetCurY();
    Point aSplit = aViewData.GetScrPos( nPosX, nPosY, ePos, TRUE );
    if ( nPosX > 0 )
        DoHSplit( aSplit.X() + aWinStart.X() );
    else
        DoHSplit( 0 );
    if ( nPosY > 0 )
        DoVSplit( aSplit.Y() + aWinStart.Y() );
    else
        DoVSplit( 0 );
    RepeatResize();
}

void ScTabView::SplitAtPixel( const Point& rPixel, BOOL bHor, BOOL bVer )       // fuer API
{
    //  Pixel ist auf die ganze View bezogen, nicht auf das erste GridWin

    if (bHor)
    {
        if ( rPixel.X() > 0 )
            DoHSplit( rPixel.X() );
        else
            DoHSplit( 0 );
    }
    if (bVer)
    {
        if ( rPixel.Y() > 0 )
            DoVSplit( rPixel.Y() );
        else
            DoVSplit( 0 );
    }
    RepeatResize();
}

void ScTabView::InvalidateSplit()
{
    SfxBindings& rBindings = aViewData.GetBindings();
    rBindings.Invalidate( SID_WINDOW_SPLIT );
    rBindings.Invalidate( SID_WINDOW_FIX );

    pHSplitter->SetFixed( aViewData.GetHSplitMode() == SC_SPLIT_FIX );
    pVSplitter->SetFixed( aViewData.GetVSplitMode() == SC_SPLIT_FIX );
}

void ScTabView::SetNewVisArea()
{
    //  #63854# fuer die Controls muss bei VisAreaChanged der Draw-MapMode eingestellt sein
    //  (auch wenn ansonsten der Edit-MapMode gesetzt ist)
    MapMode aOldMode[4];
    MapMode aDrawMode[4];
    USHORT i;
    for (i=0; i<4; i++)
        if (pGridWin[i])
        {
            aOldMode[i] = pGridWin[i]->GetMapMode();
            aDrawMode[i] = pGridWin[i]->GetDrawMapMode();
            if (aDrawMode[i] != aOldMode[i])
                pGridWin[i]->SetMapMode(aDrawMode[i]);
        }

    Window* pActive = pGridWin[aViewData.GetActivePart()];
    if (pActive)
        aViewData.GetViewShell()->VisAreaChanged(
            pActive->PixelToLogic(Rectangle(Point(),pActive->GetOutputSizePixel())) );
    if (pDrawView)
        pDrawView->VisAreaChanged();    // kein Window uebergeben -> alle Fenster

    for (i=0; i<4; i++)
        if (pGridWin[i] && aDrawMode[i] != aOldMode[i])
            pGridWin[i]->SetMapMode(aOldMode[i]);

    SfxViewFrame* pViewFrame = aViewData.GetViewShell()->GetViewFrame();
    if (pViewFrame)
    {
        SfxFrame* pFrame = pViewFrame->GetFrame();
        if (pFrame)
        {
            com::sun::star::uno::Reference<com::sun::star::frame::XController> xController = pFrame->GetController();
            if (xController.is())
            {
                ScTabViewObj* pImp = ScTabViewObj::getImplementation( xController );
                if (pImp)
                    pImp->VisAreaChanged();
            }
        }
    }
    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxSimpleHint(SC_HINT_ACC_VISAREACHANGED));
}

void ScTabView::StartDataSelect()
{
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()];
    SCCOL nCol = aViewData.GetCurX();
    SCROW nRow = aViewData.GetCurY();
    if (pWin)
        pWin->DoAutoFilterMenue( nCol, nRow, TRUE );
}

void ScTabView::EnableRefInput(BOOL bFlag)
{
    aHScrollLeft.EnableInput(bFlag);
    aHScrollRight.EnableInput(bFlag);
    aVScrollBottom.EnableInput(bFlag);
    aVScrollTop.EnableInput(bFlag);
    aScrollBarBox.EnableInput(bFlag);

    // ab hier dynamisch angelegte

    if(pTabControl!=NULL) pTabControl->EnableInput(bFlag,TRUE);

    if(pGridWin[SC_SPLIT_BOTTOMLEFT]!=NULL)
        pGridWin[SC_SPLIT_BOTTOMLEFT]->EnableInput(bFlag,FALSE);
    if(pGridWin[SC_SPLIT_BOTTOMRIGHT]!=NULL)
        pGridWin[SC_SPLIT_BOTTOMRIGHT]->EnableInput(bFlag,FALSE);
    if(pGridWin[SC_SPLIT_TOPLEFT]!=NULL)
        pGridWin[SC_SPLIT_TOPLEFT]->EnableInput(bFlag,FALSE);
    if(pGridWin[SC_SPLIT_TOPRIGHT]!=NULL)
        pGridWin[SC_SPLIT_TOPRIGHT]->EnableInput(bFlag,FALSE);
    if(pColBar[SC_SPLIT_RIGHT]!=NULL)
        pColBar[SC_SPLIT_RIGHT]->EnableInput(bFlag,FALSE);
    if(pRowBar[SC_SPLIT_TOP]!=NULL)
        pRowBar[SC_SPLIT_TOP]->EnableInput(bFlag,FALSE);
}



