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

#include <vcl/svapp.hxx>

///////////////////////////////////////////////////////////////////////////
// NODRAW.HXX
// Erweiterte Konstanten, um CLOKs mit SVDRAW.HXX zu vermeiden
// Die u.a. Aenderungen nehmen vorgeschlagene Konstante vorweg
///////////////////////////////////////////////////////////////////////////

////////////////////// Umsetzen der Standard-Defines //////////////////////

#define _SVDPAGE_HXX        // SdrPage

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

////////////////////// Ende der SVDRAW-Modifikationen /////////////////////

#include "scitems.hxx"
#include <sfx2/viewfrm.hxx>
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
#include "appoptio.hxx"
#include "attrib.hxx"
#include "hintwin.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

#include <string>
#include <algorithm>

#include <basegfx/tools/zoomtools.hxx>

#define SPLIT_MARGIN    30
#define SPLIT_HANDLE_SIZE   5
#define SC_ICONSIZE     36

#define SC_SCROLLBAR_MIN    30
#define SC_TABBAR_MIN       6

using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------


//==================================================================

//  Corner-Button

ScCornerButton::ScCornerButton( Window* pParent, ScViewData* pData, bool bAdditional ) :
    Window( pParent, WinBits( 0 ) ),
    pViewData( pData ),
    bAdd( bAdditional )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( rStyleSettings.GetFaceColor() );
    EnableRTL( false );
}

ScCornerButton::~ScCornerButton()
{
}

void ScCornerButton::Paint( const Rectangle& rRect )
{
    Size aSize = GetOutputSizePixel();
    long nPosX = aSize.Width()-1;
    long nPosY = aSize.Height()-1;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    Window::Paint(rRect);

    bool bLayoutRTL = pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
    long nDarkX = bLayoutRTL ? 0 : nPosX;

    if ( !bAdd )
    {
        // match the shaded look of column/row headers

        Color aFace( rStyleSettings.GetFaceColor() );
        Color aWhite( COL_WHITE );
        Color aCenter( aFace );
        aCenter.Merge( aWhite, 0xd0 );          // lighten up a bit
        Color aOuter( aFace );
        aOuter.Merge( aWhite, 0xa0 );           // lighten up more

        long nCenterX = (aSize.Width() / 2) - 1;
        long nCenterY = (aSize.Height() / 2) - 1;

        SetLineColor();
        SetFillColor(aCenter);
        DrawRect( Rectangle( nCenterX, nCenterY, nCenterX, nPosY ) );
        DrawRect( Rectangle( nCenterX, nCenterY, nDarkX, nCenterY ) );
        SetFillColor(aOuter);
        DrawRect( Rectangle( 0, 0, nPosX, nCenterY-1 ) );
        if ( bLayoutRTL )
            DrawRect( Rectangle( nCenterX+1, nCenterY, nPosX, nPosY ) );
        else
            DrawRect( Rectangle( 0, nCenterY, nCenterX-1, nPosY ) );
    }

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


void ScCornerButton::Resize()
{
    Invalidate();
}

void ScCornerButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    ScModule* pScMod = SC_MOD();
    bool bDisable = pScMod->IsFormulaMode() || pScMod->IsModalMode();
    if (!bDisable)
    {
        ScTabViewShell* pViewSh = pViewData->GetViewShell();
        pViewSh->SetActive();                                   // Appear und SetViewFrame
        pViewSh->ActiveGrabFocus();

        bool bControl = rMEvt.IsMod1();
        pViewSh->SelectAll( bControl );
    }
}

//==================================================================

bool lcl_HasColOutline( const ScViewData& rViewData )
{
    const ScOutlineTable* pTable = rViewData.GetDocument()->GetOutlineTable(rViewData.GetTabNo());
    if (pTable)
    {
        const ScOutlineArray* pArray = pTable->GetColArray();
        if ( pArray->GetDepth() > 0 )
            return true;
    }
    return false;
}

bool lcl_HasRowOutline( const ScViewData& rViewData )
{
    const ScOutlineTable* pTable = rViewData.GetDocument()->GetOutlineTable(rViewData.GetTabNo());
    if (pTable)
    {
        const ScOutlineArray* pArray = pTable->GetRowArray();
        if ( pArray->GetDepth() > 0 )
            return true;
    }
    return false;
}

ScTabView::ScTabView( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    pFrameWin( pParent ),
    aViewData( &rDocSh, pViewShell ),
    pSelEngine( NULL ),
    aFunctionSet( &aViewData ),
    pHdrSelEng( NULL ),
    aHdrFunc( &aViewData ),
    pDrawView( NULL ),
    aVScrollTop( pFrameWin, WinBits( WB_VSCROLL | WB_DRAG ) ),
    aVScrollBottom( pFrameWin, WinBits( WB_VSCROLL | WB_DRAG ) ),
    aHScrollLeft( pFrameWin, WinBits( WB_HSCROLL | WB_DRAG ) ),
    aHScrollRight( pFrameWin, WinBits( WB_HSCROLL | WB_DRAG ) ),
    aCornerButton( pFrameWin, &aViewData, false ),
    aTopButton( pFrameWin, &aViewData, sal_True ),
    aScrollBarBox( pFrameWin, WB_SIZEABLE ),
    mpInputHintWindow( NULL ),
    pPageBreakData( NULL ),
    pBrushDocument( NULL ),
    pDrawBrushSet( NULL ),
    pTimerWindow( NULL ),
    nTipVisible( 0 ),
    meBlockMode(None),
    mfPendingTabBarWidth( -1.0 ),
    bMinimized( false ),
    bInUpdateHeader( false ),
    bInActivatePart( false ),
    bInZoomUpdate( false ),
    bMoveIsShift( false ),
    bDrawSelMode( false ),
    bLockPaintBrush( false ),
    bDragging( false ),
    bBlockNeg( false ),
    bBlockCols( false ),
    bBlockRows( false )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScTabView::ScTabView" );

    Init();
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

IMPL_LINK_NOARG(ScTabView, TimerHdl)
{
    if (pTimerWindow)
        pTimerWindow->MouseMove( aTimerMEvt );

    return 0;
}

// --- Resize ---------------------------------------------------------------------

void lcl_SetPosSize( Window& rWindow, const Point& rPos, const Size& rSize,
                        long nTotalWidth, bool bLayoutRTL )
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

void ScTabView::DoResize( const Point& rOffset, const Size& rSize, bool bInner )
{
    HideListBox();

    bool bHasHint = HasHintWindow();
    if (bHasHint)
        RemoveHintWindow();

    bool bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );
    long nTotalWidth = rSize.Width();
    if ( bLayoutRTL )
        nTotalWidth += 2*rOffset.X();

    bool bVScroll    = aViewData.IsVScrollMode();
    bool bHScroll    = aViewData.IsHScrollMode();
    bool bTabControl = aViewData.IsTabMode();
    bool bHeaders    = aViewData.IsHeaderMode();
    bool bOutlMode   = aViewData.IsOutlineMode();
    bool bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    bool bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);

    if ( aViewData.GetDocShell()->IsPreview() )
        bHScroll = bVScroll = bTabControl = bHeaders = bOutlMode = bHOutline = bVOutline = false;

    long nBarX = 0;
    long nBarY = 0;
    long nOutlineX = 0;
    long nOutlineY = 0;
    long nOutPosX;
    long nOutPosY;

    long nPosX = rOffset.X();
    long nPosY = rOffset.Y();
    long nSizeX = rSize.Width();
    long nSizeY = rSize.Height();

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
        }
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        if ( aViewData.GetVSplitPos() > nSizeY - SPLIT_MARGIN )
        {
            aViewData.SetVSplitMode( SC_SPLIT_NONE );
            if ( WhichV( aViewData.GetActivePart() ) == SC_SPLIT_TOP )
                ActivatePart( SC_SPLIT_BOTTOMLEFT );
            InvalidateSplit();
        }

    UpdateShow();

    if (bHScroll || bVScroll)       // Scrollbars horizontal oder vertikal
    {
        long nScrollBarSize = pFrameWin->GetSettings().GetStyleSettings().GetScrollBarSize();
        if (bVScroll)
        {
            nBarX = nScrollBarSize;
            nSizeX -= nBarX - nOverlap;
        }
        if (bHScroll)
        {
            nBarY = nScrollBarSize;
            nSizeY -= nBarY - nOverlap;
        }

        //  window at the bottom right
        lcl_SetPosSize( aScrollBarBox, Point( nPosX+nSizeX, nPosY+nSizeY ), Size( nBarX, nBarY ),
                        nTotalWidth, bLayoutRTL );

        if (bHScroll)                               // Scrollbars horizontal
        {
            long nSizeLt = 0;       // left scroll bar
            long nSizeRt = 0;       // right scroll bar
            long nSizeSp = 0;       // splitter

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
                // pending relative tab bar width from extended document options
                if( mfPendingTabBarWidth >= 0.0 )
                {
                    SetRelTabBarWidth( mfPendingTabBarWidth );
                    mfPendingTabBarWidth = -1.0;
                }

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
            long nSizeUp = 0;       // upper scroll bar
            long nSizeSp = 0;       // splitter
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

        long nSize1 = nSizeX + nOverlap;

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

        InterpretVisible();     // have everything calculated before painting
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

    SvxZoomType eZoomType = GetZoomType();
    if ( eZoomType != SVX_ZOOM_PERCENT && !bInZoomUpdate )
    {
        bInZoomUpdate = true;
        const Fraction& rOldX = GetViewData()->GetZoomX();
        const Fraction& rOldY = GetViewData()->GetZoomY();
        long nOldPercent = ( rOldY.GetNumerator() * 100 ) / rOldY.GetDenominator();
        sal_uInt16 nNewZoom = CalcZoom( eZoomType, (sal_uInt16)nOldPercent );
        Fraction aNew( nNewZoom, 100 );

        if ( aNew != rOldX || aNew != rOldY )
        {
            SetZoom( aNew, aNew, false );   // always separately per sheet
            PaintGrid();
            PaintTop();
            PaintLeft();
            aViewData.GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
            aViewData.GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
        }
        bInZoomUpdate = false;
    }
}

void ScTabView::UpdateFixPos()
{
    bool bResize = false;
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        if (aViewData.UpdateFixX())
            bResize = true;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        if (aViewData.UpdateFixY())
            bResize = true;
    if (bResize)
        RepeatResize(false);
}

void ScTabView::RepeatResize( bool bUpdateFix )
{
    if ( bUpdateFix )
    {
        ScSplitMode eHSplit = aViewData.GetHSplitMode();
        ScSplitMode eVSplit = aViewData.GetVSplitMode();

        // #i46796# UpdateFixX / UpdateFixY uses GetGridOffset, which requires the
        // outline windows to be available. So UpdateShow has to be called before
        // (also called from DoResize).
        if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )
            UpdateShow();

        if ( eHSplit == SC_SPLIT_FIX )
            aViewData.UpdateFixX();
        if ( eVSplit == SC_SPLIT_FIX )
            aViewData.UpdateFixY();
    }

    DoResize( aBorderPos, aFrameSize );

    //! Border muss neu gesetzt werden ???
}

void ScTabView::GetBorderSize( SvBorder& rBorder, const Size& /* rSize */ )
{
    sal_Bool bScrollBars = aViewData.IsVScrollMode();
    sal_Bool bHeaders    = aViewData.IsHeaderMode();
    sal_Bool bOutlMode   = aViewData.IsOutlineMode();
    sal_Bool bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    sal_Bool bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);
    sal_Bool bLayoutRTL  = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );

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

IMPL_LINK_NOARG(ScTabView, TabBarResize)
{
    if (aViewData.IsHScrollMode())
    {
        const long nOverlap = 0;    // ScrollBar::GetWindowOverlapPixel();
        long nSize = pTabControl->GetSplitSize();

        if (aViewData.GetHSplitMode() != SC_SPLIT_FIX)
        {
            long nMax = pHSplitter->GetPosPixel().X();
            if( pTabControl->IsEffectiveRTL() )
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

void ScTabView::SetRelTabBarWidth( double fRelTabBarWidth )
{
    if( (0.0 <= fRelTabBarWidth) && (fRelTabBarWidth <= 1.0) )
        if( long nFrameWidth = pFrameWin->GetSizePixel().Width() )
            SetTabBarWidth( static_cast< long >( fRelTabBarWidth * nFrameWidth + 0.5 ) );
}

void ScTabView::SetPendingRelTabBarWidth( double fRelTabBarWidth )
{
    mfPendingTabBarWidth = fRelTabBarWidth;
    SetRelTabBarWidth( fRelTabBarWidth );
}

long ScTabView::GetTabBarWidth() const
{
    return pTabControl->GetSizePixel().Width();
}

double ScTabView::GetRelTabBarWidth() const
{
    if( long nFrameWidth = pFrameWin->GetSizePixel().Width() )
        return static_cast< double >( GetTabBarWidth() ) / nFrameWidth;
    return 0.0;
}

double ScTabView::GetPendingRelTabBarWidth() const
{
    return mfPendingTabBarWidth;
}

Window* ScTabView::GetActiveWin()
{
    ScSplitPos ePos = aViewData.GetActivePart();
    OSL_ENSURE(pGridWin[ePos],"kein aktives Fenster");
    return pGridWin[ePos];
}

Window* ScTabView::GetWindowByPos( ScSplitPos ePos )
{
    return pGridWin[ePos];
}

void ScTabView::SetActivePointer( const Pointer& rPointer )
{
    for (sal_uInt16 i=0; i<4; i++)
        if (pGridWin[i])
            pGridWin[i]->SetPointer( rPointer );
}


void ScTabView::ActiveGrabFocus()
{
    ScSplitPos ePos = aViewData.GetActivePart();
    if (pGridWin[ePos])
        pGridWin[ePos]->GrabFocus();
}


ScSplitPos ScTabView::FindWindow( Window* pWindow ) const
{
    ScSplitPos eVal = SC_SPLIT_BOTTOMLEFT;      // Default
    for (sal_uInt16 i=0; i<4; i++)
        if ( pGridWin[i] == pWindow )
            eVal = (ScSplitPos) i;

    return eVal;
}

Point ScTabView::GetGridOffset() const
{
    Point aPos;

        // Groessen hier wie in DoResize

    sal_Bool bHeaders    = aViewData.IsHeaderMode();
    sal_Bool bOutlMode   = aViewData.IsOutlineMode();
    sal_Bool bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    sal_Bool bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);

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

bool ScTabView::ScrollCommand( const CommandEvent& rCEvt, ScSplitPos ePos )
{
    HideNoteMarker();

    bool bDone = false;
    const CommandWheelData* pData = rCEvt.GetWheelData();
    if ( pData && pData->GetMode() == COMMAND_WHEEL_ZOOM )
    {
        if ( !aViewData.GetViewShell()->GetViewFrame()->GetFrame().IsInPlace() )
        {
            //  for ole inplace editing, the scale is defined by the visarea and client size
            //  and can't be changed directly

            const Fraction& rOldY = aViewData.GetZoomY();
            long nOld = (long)(( rOldY.GetNumerator() * 100 ) / rOldY.GetDenominator());
            long nNew = nOld;
            if ( pData->GetDelta() < 0 )
                nNew = Max( (long) MINZOOM, basegfx::zoomtools::zoomOut( nOld ));
            else
                nNew = Min( (long) MAXZOOM, basegfx::zoomtools::zoomIn( nOld ));

            if ( nNew != nOld )
            {
                // scroll wheel doesn't set the AppOptions default

                sal_Bool bSyncZoom = SC_MOD()->GetAppOptions().GetSynchronizeZoom();
                SetZoomType( SVX_ZOOM_PERCENT, bSyncZoom );
                Fraction aFract( nNew, 100 );
                SetZoom( aFract, aFract, bSyncZoom );
                PaintGrid();
                PaintTop();
                PaintLeft();
                aViewData.GetBindings().Invalidate( SID_ATTR_ZOOM );
                aViewData.GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
            }

            bDone = true;
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
    sal_Bool bOnlineScroll = sal_True;      //! Optionen

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
                bool bMirror = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() ) != Application::GetSettings().GetLayoutRTL();
                ScHSplitPos eWhich = (pScroll == &aHScrollLeft) ? SC_SPLIT_LEFT : SC_SPLIT_RIGHT;
                long nDelta = GetScrollBarPos( *pScroll, bMirror ) + nScrollMin - aViewData.GetPosX(eWhich);
                if (nDelta) ScrollX( nDelta, eWhich );
            }
            else                            // VScroll...
            {
                ScVSplitPos eWhich = (pScroll == &aVScrollTop) ? SC_SPLIT_TOP : SC_SPLIT_BOTTOM;
                long nDelta = GetScrollBarPos( *pScroll, false ) + nScrollMin - aViewData.GetPosY(eWhich);
                if (nDelta) ScrollY( nDelta, eWhich );
            }
        }
        bDragging = false;
    }
    return 0;
}

IMPL_LINK( ScTabView, ScrollHdl, ScrollBar*, pScroll )
{
    sal_Bool bOnlineScroll = sal_True;      //! Optionen

    bool bHoriz = ( pScroll == &aHScrollLeft || pScroll == &aHScrollRight );
    long nViewPos;
    if ( bHoriz )
        nViewPos = aViewData.GetPosX( (pScroll == &aHScrollLeft) ?
                                        SC_SPLIT_LEFT : SC_SPLIT_RIGHT );
    else
        nViewPos = aViewData.GetPosY( (pScroll == &aVScrollTop) ?
                                        SC_SPLIT_TOP : SC_SPLIT_BOTTOM );

    bool bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );
    bool bMirror = bHoriz && (bLayoutRTL != Application::GetSettings().GetLayoutRTL());

    ScrollType eType = pScroll->GetType();
    if ( eType == SCROLL_DRAG )
    {
        if (!bDragging)
        {
            bDragging = true;
            nPrevDragPos = nViewPos;
        }

        //  Scroll-Position anzeigen
        //  (nur QuickHelp, in der Statuszeile gibt es keinen Eintrag dafuer)

        if (Help::IsQuickHelpEnabled())
        {
            Size aSize = pScroll->GetSizePixel();

            /*  Convert scrollbar mouse position to screen position. If RTL
                mode of scrollbar differs from RTL mode of its parent, then the
                direct call to Window::OutputToNormalizedScreenPixel() will
                give unusable results, because calcualtion of screen position
                is based on parent orientation and expects equal orientation of
                the child position. Need to mirror mouse position before. */
            Point aMousePos = pScroll->GetPointerPosPixel();
            if( pScroll->IsRTLEnabled() != pScroll->GetParent()->IsRTLEnabled() )
                aMousePos.X() = aSize.Width() - aMousePos.X() - 1;
            aMousePos = pScroll->OutputToNormalizedScreenPixel( aMousePos );

            // convert top-left position of scrollbar to screen position
            Point aPos = pScroll->OutputToNormalizedScreenPixel( Point() );

            // get scrollbar scroll position for help text (row number/column name)
            long nScrollMin = 0;        // RangeMin simulieren
            if ( aViewData.GetHSplitMode()==SC_SPLIT_FIX && pScroll == &aHScrollRight )
                nScrollMin = aViewData.GetFixPosX();
            if ( aViewData.GetVSplitMode()==SC_SPLIT_FIX && pScroll == &aVScrollBottom )
                nScrollMin = aViewData.GetFixPosY();
            long nScrollPos = GetScrollBarPos( *pScroll, bMirror ) + nScrollMin;

            String aHelpStr;
            Rectangle aRect;
            sal_uInt16 nAlign;
            if (bHoriz)
            {
                aHelpStr = ScGlobal::GetRscString(STR_COLUMN);
                aHelpStr += ' ';
                aHelpStr += ScColToAlpha((SCCOL) nScrollPos);

                aRect.Left() = aMousePos.X();
                aRect.Top()  = aPos.Y() - 4;
                nAlign       = QUICKHELP_BOTTOM|QUICKHELP_CENTER;
            }
            else
            {
                aHelpStr = ScGlobal::GetRscString(STR_ROW);
                aHelpStr += ' ';
                aHelpStr += String::CreateFromInt32(nScrollPos + 1);

                // show quicktext always inside sheet area
                aRect.Left() = bLayoutRTL ? (aPos.X() + aSize.Width() + 8) : (aPos.X() - 8);
                aRect.Top()  = aMousePos.Y();
                nAlign       = (bLayoutRTL ? QUICKHELP_LEFT : QUICKHELP_RIGHT) | QUICKHELP_VCENTER;
            }
            aRect.Right()   = aRect.Left();
            aRect.Bottom()  = aRect.Top();

            Help::ShowQuickHelp(pScroll->GetParent(), aRect, aHelpStr, nAlign);
        }
    }

    if ( bOnlineScroll || eType != SCROLL_DRAG )
    {
        if ( bMirror )
        {
            // change scroll type so visible/previous cells calculation below remains the same
            switch ( eType )
            {
                case SCROLL_LINEUP:     eType = SCROLL_LINEDOWN;    break;
                case SCROLL_LINEDOWN:   eType = SCROLL_LINEUP;      break;
                case SCROLL_PAGEUP:     eType = SCROLL_PAGEDOWN;    break;
                case SCROLL_PAGEDOWN:   eType = SCROLL_PAGEUP;      break;
                default:
                {
                    // added to avoid warnings
                }
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

                    long nScrollPos = GetScrollBarPos( *pScroll, bMirror ) + nScrollMin;
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
            default:
            {
                // added to avoid warnings
            }
        }

        if (nDelta)
        {
            sal_Bool bUpdate = ( eType != SCROLL_DRAG );    // bei Drag die Ranges nicht aendern
            if ( bHoriz )
                ScrollX( nDelta, (pScroll == &aHScrollLeft) ? SC_SPLIT_LEFT : SC_SPLIT_RIGHT, bUpdate );
            else
                ScrollY( nDelta, (pScroll == &aVScrollTop) ? SC_SPLIT_TOP : SC_SPLIT_BOTTOM, bUpdate );
        }
    }

    return 0;
}

void ScTabView::ScrollX( long nDeltaX, ScHSplitPos eWhich, bool bUpdBars )
{
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
    while ( pDoc->ColHidden(nNewX, nTab) &&
            nNewX+nDir >= 0 && nNewX+nDir <= MAXCOL )
        nNewX = sal::static_int_cast<SCsCOL>( nNewX + nDir );

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

    TestHintWindow();
}

void ScTabView::ScrollY( long nDeltaY, ScVSplitPos eWhich, bool bUpdBars )
{
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
    while ( pDoc->RowHidden(nNewY, nTab) &&
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

    TestHintWindow();
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
    while ( nVis > 0 && pDoc->GetRowHeight( nVis, nTab ) == 0 )
        --nVis;
    return nVis;
}

void ScTabView::UpdateHeaderWidth( const ScVSplitPos* pWhich, const SCROW* pPosY )
{
    if ( !pRowBar[SC_SPLIT_BOTTOM] || MAXROW < 10000 )
        return;

    SCROW nEndPos = MAXROW;
    if ( !aViewData.GetViewShell()->GetViewFrame()->GetFrame().IsInPlace() )
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
        bInUpdateHeader = true;

        pRowBar[SC_SPLIT_BOTTOM]->SetWidth( nWidth );
        if (pRowBar[SC_SPLIT_TOP])
            pRowBar[SC_SPLIT_TOP]->SetWidth( nWidth );

        RepeatResize();

        // auf VCL gibt's Update ohne Ende (jedes Update gilt fuer alle Fenster)
        //aCornerButton.Update();       // der bekommt sonst nie ein Update

        bInUpdateHeader = false;
    }
}

inline void ShowHide( Window* pWin, sal_Bool bShow )
{
    OSL_ENSURE(pWin || !bShow, "Fenster ist nicht da");
    if (pWin)
        pWin->Show(bShow);
}

void ScTabView::UpdateShow()
{
    sal_Bool bHScrollMode = aViewData.IsHScrollMode();
    sal_Bool bVScrollMode = aViewData.IsVScrollMode();
    sal_Bool bTabMode     = aViewData.IsTabMode();
    sal_Bool bOutlMode    = aViewData.IsOutlineMode();
    sal_Bool bHOutline    = bOutlMode && lcl_HasColOutline(aViewData);
    sal_Bool bVOutline    = bOutlMode && lcl_HasRowOutline(aViewData);
    sal_Bool bHeader      = aViewData.IsHeaderMode();

    sal_Bool bShowH = ( aViewData.GetHSplitMode() != SC_SPLIT_NONE );
    sal_Bool bShowV = ( aViewData.GetVSplitMode() != SC_SPLIT_NONE );

    if ( aViewData.GetDocShell()->IsPreview() )
        bHScrollMode = bVScrollMode = bTabMode = bHeader = bOutlMode = bHOutline = bVOutline = false;

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

void ScTabView::UpdateVisibleRange()
{
    for (int i = 0; i < 4; ++i)
    {
        if (!pGridWin[i] || !pGridWin[i]->IsVisible())
            continue;

        pGridWin[i]->UpdateVisibleRange();
    }
}

// ---  Splitter  --------------------------------------------------------

IMPL_LINK( ScTabView, SplitHdl, Splitter*, pSplitter )
{
    if ( pSplitter == pHSplitter )
        DoHSplit( pHSplitter->GetSplitPosPixel() );
    else
        DoVSplit( pVSplitter->GetSplitPosPixel() );

    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX || aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        FreezeSplitters( sal_True );

    DoResize( aBorderPos, aFrameSize );

    return 0;
}

void ScTabView::DoHSplit(long nSplitPos)
{
    //  nSplitPos is the real pixel position on the frame window,
    //  mirroring for RTL has to be done here.

    sal_Bool bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );
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
            long nLeftWidth = nSplitPos - pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width();
            if ( nLeftWidth < 0 ) nLeftWidth = 0;
            nNewDelta = nOldDelta + aViewData.CellsAtX( nOldDelta, 1, SC_SPLIT_LEFT,
                            (sal_uInt16) nLeftWidth );
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

        //  Form-Layer muss den sichtbaren Ausschnitt aller Fenster kennen
        //  dafuer muss hier schon der MapMode stimmen
        for (sal_uInt16 i=0; i<4; i++)
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
                            (sal_uInt16) nTopHeight );
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

        //  Form-Layer muss den sichtbaren Ausschnitt aller Fenster kennen
        //  dafuer muss hier schon der MapMode stimmen
        for (sal_uInt16 i=0; i<4; i++)
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
    long nPosY = (long) pDoc->GetRowHeight( 0, nRow-1, nTab);
    nPosY = (long)(nPosY * HMM_PER_TWIPS);
    return Point(nPosX,nPosY);
}

Point ScTabView::GetChartInsertPos( const Size& rSize, const ScRange& rCellRange )
{
    Point aInsertPos;
    const long nBorder = 100;   // leave 1mm for border
    long nNeededWidth = rSize.Width() + 2 * nBorder;
    long nNeededHeight = rSize.Height() + 2 * nBorder;

    // use the active window, or lower/right if frozen (as in CalcZoom)
    ScSplitPos eUsedPart = aViewData.GetActivePart();
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        eUsedPart = (WhichV(eUsedPart)==SC_SPLIT_TOP) ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        eUsedPart = (WhichH(eUsedPart)==SC_SPLIT_LEFT) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;

    ScGridWindow* pWin = pGridWin[eUsedPart];
    OSL_ENSURE( pWin, "Window not found" );
    if (pWin)
    {
        ActivatePart( eUsedPart );

        //  get the visible rectangle in logic units

        MapMode aDrawMode = pWin->GetDrawMapMode();
        Rectangle aVisible( pWin->PixelToLogic( Rectangle( Point(0,0), pWin->GetOutputSizePixel() ), aDrawMode ) );

        ScDocument* pDoc = aViewData.GetDocument();
        SCTAB nTab = aViewData.GetTabNo();
        sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
        long nLayoutSign = bLayoutRTL ? -1 : 1;

        long nDocX = (long)( (double) pDoc->GetColOffset( MAXCOL + 1, nTab ) * HMM_PER_TWIPS ) * nLayoutSign;
        long nDocY = (long)( (double) pDoc->GetRowOffset( MAXROW + 1, nTab ) * HMM_PER_TWIPS );

        if ( aVisible.Left() * nLayoutSign > nDocX * nLayoutSign )
            aVisible.Left() = nDocX;
        if ( aVisible.Right() * nLayoutSign > nDocX * nLayoutSign )
            aVisible.Right() = nDocX;
        if ( aVisible.Top() > nDocY )
            aVisible.Top() = nDocY;
        if ( aVisible.Bottom() > nDocY )
            aVisible.Bottom() = nDocY;

        //  get the logic position of the selection

        Rectangle aSelection = pDoc->GetMMRect( rCellRange.aStart.Col(), rCellRange.aStart.Row(),
                                                rCellRange.aEnd.Col(), rCellRange.aEnd.Row(), nTab );

        long nLeftSpace = aSelection.Left() - aVisible.Left();
        long nRightSpace = aVisible.Right() - aSelection.Right();
        long nTopSpace = aSelection.Top() - aVisible.Top();
        long nBottomSpace = aVisible.Bottom() - aSelection.Bottom();

        bool bFitLeft = ( nLeftSpace >= nNeededWidth );
        bool bFitRight = ( nRightSpace >= nNeededWidth );

        if ( bFitLeft || bFitRight )
        {
            // first preference: completely left or right of the selection

            // if both fit, prefer left in RTL mode, right otherwise
            bool bPutLeft = bFitLeft && ( bLayoutRTL || !bFitRight );

            if ( bPutLeft )
                aInsertPos.X() = aSelection.Left() - nNeededWidth;
            else
                aInsertPos.X() = aSelection.Right() + 1;

            // align with top of selection (is moved again if it doesn't fit)
            aInsertPos.Y() = std::max( aSelection.Top(), aVisible.Top() );
        }
        else if ( nTopSpace >= nNeededHeight || nBottomSpace >= nNeededHeight )
        {
            // second preference: completely above or below the selection

            if ( nBottomSpace > nNeededHeight )             // bottom is preferred
                aInsertPos.Y() = aSelection.Bottom() + 1;
            else
                aInsertPos.Y() = aSelection.Top() - nNeededHeight;

            // align with (logic) left edge of selection (moved again if it doesn't fit)
            if ( bLayoutRTL )
                aInsertPos.X() = std::min( aSelection.Right(), aVisible.Right() ) - nNeededWidth + 1;
            else
                aInsertPos.X() = std::max( aSelection.Left(), aVisible.Left() );
        }
        else
        {
            // place to the (logic) right of the selection and move so it fits

            if ( bLayoutRTL )
                aInsertPos.X() = aSelection.Left() - nNeededWidth;
            else
                aInsertPos.X() = aSelection.Right() + 1;
            aInsertPos.Y() = std::max( aSelection.Top(), aVisible.Top() );
        }

        // move the position if the object doesn't fit in the screen

        Rectangle aCompareRect( aInsertPos, Size( nNeededWidth, nNeededHeight ) );
        if ( aCompareRect.Right() > aVisible.Right() )
            aInsertPos.X() -= aCompareRect.Right() - aVisible.Right();
        if ( aCompareRect.Bottom() > aVisible.Bottom() )
            aInsertPos.Y() -= aCompareRect.Bottom() - aVisible.Bottom();

        if ( aInsertPos.X() < aVisible.Left() )
            aInsertPos.X() = aVisible.Left();
        if ( aInsertPos.Y() < aVisible.Top() )
            aInsertPos.Y() = aVisible.Top();

        // nNeededWidth / nNeededHeight includes all borders - move aInsertPos to the
        // object position, inside the border

        aInsertPos.X() += nBorder;
        aInsertPos.Y() += nBorder;
    }
    return aInsertPos;
}

Point ScTabView::GetChartDialogPos( const Size& rDialogSize, const Rectangle& rLogicChart )
{
    // rDialogSize must be in pixels, rLogicChart in 1/100 mm. Return value is in pixels.

    Point aRet;

    // use the active window, or lower/right if frozen (as in CalcZoom)
    ScSplitPos eUsedPart = aViewData.GetActivePart();
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        eUsedPart = (WhichV(eUsedPart)==SC_SPLIT_TOP) ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        eUsedPart = (WhichH(eUsedPart)==SC_SPLIT_LEFT) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;

    ScGridWindow* pWin = pGridWin[eUsedPart];
    OSL_ENSURE( pWin, "Window not found" );
    if (pWin)
    {
        MapMode aDrawMode = pWin->GetDrawMapMode();
        Rectangle aObjPixel = pWin->LogicToPixel( rLogicChart, aDrawMode );
        Rectangle aObjAbs( pWin->OutputToAbsoluteScreenPixel( aObjPixel.TopLeft() ),
                           pWin->OutputToAbsoluteScreenPixel( aObjPixel.BottomRight() ) );

        Rectangle aDesktop = pWin->GetDesktopRectPixel();
        Size aSpace = pWin->LogicToPixel( Size( 8, 12 ), MAP_APPFONT );

        ScDocument* pDoc = aViewData.GetDocument();
        SCTAB nTab = aViewData.GetTabNo();
        sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );

        bool bCenterHor = false;

        if ( aDesktop.Bottom() - aObjAbs.Bottom() >= rDialogSize.Height() + aSpace.Height() )
        {
            // first preference: below the chart

            aRet.Y() = aObjAbs.Bottom() + aSpace.Height();
            bCenterHor = true;
        }
        else if ( aObjAbs.Top() - aDesktop.Top() >= rDialogSize.Height() + aSpace.Height() )
        {
            // second preference: above the chart

            aRet.Y() = aObjAbs.Top() - rDialogSize.Height() - aSpace.Height();
            bCenterHor = true;
        }
        else
        {
            bool bFitLeft = ( aObjAbs.Left() - aDesktop.Left() >= rDialogSize.Width() + aSpace.Width() );
            bool bFitRight = ( aDesktop.Right() - aObjAbs.Right() >= rDialogSize.Width() + aSpace.Width() );

            if ( bFitLeft || bFitRight )
            {
                // if both fit, prefer right in RTL mode, left otherwise
                bool bPutRight = bFitRight && ( bLayoutRTL || !bFitLeft );
                if ( bPutRight )
                    aRet.X() = aObjAbs.Right() + aSpace.Width();
                else
                    aRet.X() = aObjAbs.Left() - rDialogSize.Width() - aSpace.Width();

                // center vertically
                aRet.Y() = aObjAbs.Top() + ( aObjAbs.GetHeight() - rDialogSize.Height() ) / 2;
            }
            else
            {
                // doesn't fit on any edge - put at the bottom of the screen
                aRet.Y() = aDesktop.Bottom() - rDialogSize.Height();
                bCenterHor = true;
            }
        }
        if ( bCenterHor )
            aRet.X() = aObjAbs.Left() + ( aObjAbs.GetWidth() - rDialogSize.Width() ) / 2;

        // limit to screen (centering might lead to invalid positions)
        if ( aRet.X() + rDialogSize.Width() - 1 > aDesktop.Right() )
            aRet.X() = aDesktop.Right() - rDialogSize.Width() + 1;
        if ( aRet.X() < aDesktop.Left() )
            aRet.X() = aDesktop.Left();
        if ( aRet.Y() + rDialogSize.Height() - 1 > aDesktop.Bottom() )
            aRet.Y() = aDesktop.Bottom() - rDialogSize.Height() + 1;
        if ( aRet.Y() < aDesktop.Top() )
            aRet.Y() = aDesktop.Top();
    }

    return aRet;
}

void ScTabView::LockModifiers( sal_uInt16 nModifiers )
{
    pSelEngine->LockModifiers( nModifiers );
    pHdrSelEng->LockModifiers( nModifiers );
}

sal_uInt16 ScTabView::GetLockedModifiers() const
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

sal_Bool lcl_MouseIsOverWin( const Point& rScreenPosPixel, Window* pWin )
{
    if (pWin)
    {
        //  SPLIT_HANDLE_SIZE draufaddieren, damit das Einrasten genau
        //  auf dem Splitter nicht aussetzt

        Point aRel = pWin->NormalizedScreenToOutputPixel( rScreenPosPixel );
        Size aWinSize = pWin->GetOutputSizePixel();
        if ( aRel.X() >= 0 && aRel.X() < aWinSize.Width() + SPLIT_HANDLE_SIZE &&
                aRel.Y() >= 0 && aRel.Y() < aWinSize.Height() + SPLIT_HANDLE_SIZE )
            return sal_True;
    }
    return false;
}

void ScTabView::SnapSplitPos( Point& rScreenPosPixel )
{
    sal_Bool bOverWin = false;
    sal_uInt16 i;
    for (i=0; i<4; i++)
        if (lcl_MouseIsOverWin(rScreenPosPixel,pGridWin[i]))
            bOverWin = sal_True;

    if (!bOverWin)
        return;

    //  don't snap to cells if the scale will be modified afterwards
    if ( GetZoomType() != SVX_ZOOM_PERCENT )
        return;

    ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        ePos = SC_SPLIT_TOPLEFT;

    Window* pWin = pGridWin[ePos];
    if (!pWin)
    {
        OSL_FAIL("Window NULL");
        return;
    }

    Point aMouse = pWin->NormalizedScreenToOutputPixel( rScreenPosPixel );
    SCsCOL nPosX;
    SCsROW nPosY;
    //  bNextIfLarge=FALSE: nicht auf naechste Zelle, wenn ausserhalb des Fensters
    aViewData.GetPosFromPixel( aMouse.X(), aMouse.Y(), ePos, nPosX, nPosY, true, false, false );
    sal_Bool bLeft;
    sal_Bool bTop;
    aViewData.GetMouseQuadrant( aMouse, ePos, nPosX, nPosY, bLeft, bTop );
    if (!bLeft)
        ++nPosX;
    if (!bTop)
        ++nPosY;
    aMouse = aViewData.GetScrPos( static_cast<SCCOL>(nPosX), static_cast<SCROW>(nPosY), ePos, sal_True );
    rScreenPosPixel = pWin->OutputToNormalizedScreenPixel( aMouse );
}

void ScTabView::FreezeSplitters( bool bFreeze )
{
    ScSplitMode eOldH = aViewData.GetHSplitMode();
    ScSplitMode eOldV = aViewData.GetVSplitMode();

    ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
    if ( eOldV != SC_SPLIT_NONE )
        ePos = SC_SPLIT_TOPLEFT;
    Window* pWin = pGridWin[ePos];

    bool bLayoutRTL = aViewData.GetDocument()->IsLayoutRTL( aViewData.GetTabNo() );

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
            sal_Bool bLeft;
            sal_Bool bTop;
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

        aSplit = aViewData.GetScrPos( static_cast<SCCOL>(nPosX), static_cast<SCROW>(nPosY), ePos, true );
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

    //  Form-Layer muss den sichtbaren Ausschnitt aller Fenster kennen
    //  dafuer muss hier schon der MapMode stimmen
    for (sal_uInt16 i=0; i<4; i++)
        if (pGridWin[i])
            pGridWin[i]->SetMapMode( pGridWin[i]->GetDrawMapMode() );
    SetNewVisArea();

    RepeatResize(false);

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
    Point aSplit = aViewData.GetScrPos( nPosX, nPosY, ePos, sal_True );
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

void ScTabView::SplitAtPixel( const Point& rPixel, bool bHor, bool bVer )
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
    //  fuer die Controls muss bei VisAreaChanged der Draw-MapMode eingestellt sein
    //  (auch wenn ansonsten der Edit-MapMode gesetzt ist)
    MapMode aOldMode[4];
    MapMode aDrawMode[4];
    sal_uInt16 i;
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

    UpdateAllOverlays();                // #i79909# with drawing MapMode set

    for (i=0; i<4; i++)
        if (pGridWin[i] && aDrawMode[i] != aOldMode[i])
        {
            pGridWin[i]->flushOverlayManager();     // #i79909# flush overlays before switching to edit MapMode
            pGridWin[i]->SetMapMode(aOldMode[i]);
        }

    SfxViewFrame* pViewFrame = aViewData.GetViewShell()->GetViewFrame();
    if (pViewFrame)
    {
        SfxFrame& rFrame = pViewFrame->GetFrame();
        com::sun::star::uno::Reference<com::sun::star::frame::XController> xController = rFrame.GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = ScTabViewObj::getImplementation( xController );
            if (pImp)
                pImp->VisAreaChanged();
        }
    }
    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxSimpleHint(SC_HINT_ACC_VISAREACHANGED));
}

bool ScTabView::HasPageFieldDataAtCursor() const
{
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()];
    SCCOL nCol = aViewData.GetCurX();
    SCROW nRow = aViewData.GetCurY();
    if (pWin)
        return pWin->GetDPFieldOrientation( nCol, nRow ) == sheet::DataPilotFieldOrientation_PAGE;

    return false;
}

void ScTabView::StartDataSelect()
{
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()];
    SCCOL nCol = aViewData.GetCurX();
    SCROW nRow = aViewData.GetCurY();

    if (!pWin)
        return;

    switch (pWin->GetDPFieldOrientation(nCol, nRow))
    {
        case sheet::DataPilotFieldOrientation_PAGE:
            //  #i36598# If the cursor is on a page field's data cell,
            //  no meaningful input is possible anyway, so this function
            //  can be used to select a page field entry.
            pWin->LaunchPageFieldMenu( nCol, nRow );
            return;
        case sheet::DataPilotFieldOrientation_COLUMN:
        case sheet::DataPilotFieldOrientation_ROW:
            pWin->LaunchDPFieldMenu( nCol, nRow );
            return;
        default:
            ;
    }

    // Do autofilter if the current cell has autofilter button.  Otherwise do
    // a normal data select popup.
    const ScMergeFlagAttr* pAttr = static_cast<const ScMergeFlagAttr*>(
        aViewData.GetDocument()->GetAttr(
            nCol, nRow, aViewData.GetTabNo(), ATTR_MERGE_FLAG));

    if (pAttr->HasAutoFilter())
        pWin->LaunchAutoFilterMenu(nCol, nRow);
    else
        pWin->LaunchDataSelectMenu(nCol, nRow, true);
}

void ScTabView::EnableRefInput(bool bFlag)
{
    aHScrollLeft.EnableInput(bFlag);
    aHScrollRight.EnableInput(bFlag);
    aVScrollBottom.EnableInput(bFlag);
    aVScrollTop.EnableInput(bFlag);
    aScrollBarBox.EnableInput(bFlag);

    // ab hier dynamisch angelegte

    if(pTabControl!=NULL) pTabControl->EnableInput(bFlag,true);

    if(pGridWin[SC_SPLIT_BOTTOMLEFT]!=NULL)
        pGridWin[SC_SPLIT_BOTTOMLEFT]->EnableInput(bFlag,false);
    if(pGridWin[SC_SPLIT_BOTTOMRIGHT]!=NULL)
        pGridWin[SC_SPLIT_BOTTOMRIGHT]->EnableInput(bFlag,false);
    if(pGridWin[SC_SPLIT_TOPLEFT]!=NULL)
        pGridWin[SC_SPLIT_TOPLEFT]->EnableInput(bFlag,false);
    if(pGridWin[SC_SPLIT_TOPRIGHT]!=NULL)
        pGridWin[SC_SPLIT_TOPRIGHT]->EnableInput(bFlag,false);
    if(pColBar[SC_SPLIT_RIGHT]!=NULL)
        pColBar[SC_SPLIT_RIGHT]->EnableInput(bFlag,false);
    if(pRowBar[SC_SPLIT_TOP]!=NULL)
        pRowBar[SC_SPLIT_TOP]->EnableInput(bFlag,false);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
