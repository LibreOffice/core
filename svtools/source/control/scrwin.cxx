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
#include "precompiled_svtools.hxx"

#define _SVT_SCRWIN_CXX
#include <svtools/scrwin.hxx>

//===================================================================

void ScrollableWindow::ImpInitialize( ScrollableWindowFlags nFlags )
{
    bHandleDragging = (sal_Bool) ( nFlags & SCRWIN_THUMBDRAGGING );
    bVCenter = (nFlags & SCRWIN_VCENTER) == SCRWIN_VCENTER;
    bHCenter = (nFlags & SCRWIN_HCENTER) == SCRWIN_HCENTER;
    bScrolling = sal_False;

    // set the handlers for the scrollbars
    aVScroll.SetScrollHdl( LINK(this, ScrollableWindow, ScrollHdl) );
    aHScroll.SetScrollHdl( LINK(this, ScrollableWindow, ScrollHdl) );
    aVScroll.SetEndScrollHdl( LINK(this, ScrollableWindow, EndScrollHdl) );
    aHScroll.SetEndScrollHdl( LINK(this, ScrollableWindow, EndScrollHdl) );

    nColumnPixW = nLinePixH = GetSettings().GetStyleSettings().GetScrollBarSize();
}

//-------------------------------------------------------------------

ScrollableWindow::ScrollableWindow( Window* pParent, WinBits nBits,
                                    ScrollableWindowFlags nFlags ) :
    Window( pParent, WinBits(nBits|WB_CLIPCHILDREN) ),
    aVScroll( this, WinBits(WB_VSCROLL | WB_DRAG) ),
    aHScroll( this, WinBits(WB_HSCROLL | WB_DRAG) ),
    aCornerWin( this )
{
    ImpInitialize( nFlags );
}

//-------------------------------------------------------------------

ScrollableWindow::ScrollableWindow( Window* pParent, const ResId& rId,
                                    ScrollableWindowFlags nFlags ) :
    Window( pParent, rId ),
    aVScroll( this, WinBits(WB_VSCROLL | WB_DRAG) ),
    aHScroll( this, WinBits(WB_HSCROLL | WB_DRAG) ),
    aCornerWin( this )
{
    ImpInitialize( nFlags );
}

// -----------------------------------------------------------------------

void ScrollableWindow::Command( const CommandEvent& rCEvt )
{
    if ( (rCEvt.GetCommand() == COMMAND_WHEEL) ||
         (rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL) ||
         (rCEvt.GetCommand() == COMMAND_AUTOSCROLL) )
    {
        ScrollBar* pHScrBar;
        ScrollBar* pVScrBar;
        if ( aHScroll.IsVisible() )
            pHScrBar = &aHScroll;
        else
            pHScrBar = NULL;
        if ( aVScroll.IsVisible() )
            pVScrBar = &aVScroll;
        else
            pVScrBar = NULL;
        if ( HandleScrollCommand( rCEvt, pHScrBar, pVScrBar ) )
            return;
    }

    Window::Command( rCEvt );
}

//-------------------------------------------------------------------

void ScrollableWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Resize();
        Invalidate();
    }

    Window::DataChanged( rDCEvt );
}

//-------------------------------------------------------------------

Size __EXPORT ScrollableWindow::GetOutputSizePixel() const
{
    Size aSz( Window::GetOutputSizePixel() );

    long nTmp = GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( aHScroll.IsVisible() )
        aSz.Height() -= nTmp;
    if ( aVScroll.IsVisible() )
        aSz.Width() -= nTmp;
    return aSz;
}

//-------------------------------------------------------------------

Size ScrollableWindow::GetOutputSize() const
{
    return PixelToLogic( GetOutputSizePixel() );
}

//-------------------------------------------------------------------

IMPL_LINK( ScrollableWindow, EndScrollHdl, ScrollBar *, pScroll )
{
    // notify the start of scrolling, if not already scrolling
    if ( !bScrolling )
        StartScroll(), bScrolling = sal_True;

    // get the delta in logic coordinates
    Size aDelta( PixelToLogic( Size( aHScroll.GetDelta(), aVScroll.GetDelta() ) ) );

    // scroll the window, if this is not already done
    if ( !bHandleDragging )
    {
        if ( pScroll == &aHScroll )
            Scroll( aDelta.Width(), 0 );
        else
            Scroll( 0, aDelta.Height() );
    }

    // notify the end of scrolling
    bScrolling = sal_False;
    EndScroll( aDelta.Width(), aDelta.Height() );
    return 0;
}

//-------------------------------------------------------------------

IMPL_LINK( ScrollableWindow, ScrollHdl, ScrollBar *, pScroll )
{
    // notify the start of scrolling, if not already scrolling
    if ( !bScrolling )
        StartScroll(), bScrolling = sal_True;

    if ( bHandleDragging )
    {
        // get the delta in logic coordinates
        Size aDelta( PixelToLogic(
            Size( aHScroll.GetDelta(), aVScroll.GetDelta() ) ) );
        if ( pScroll == &aHScroll )
            Scroll( aDelta.Width(), 0 );
        else
            Scroll( 0, aDelta.Height() );
    }
    return 0;
}

//-------------------------------------------------------------------

void __EXPORT ScrollableWindow::Resize()
{
    // get the new output-size in pixel
    Size aOutPixSz = Window::GetOutputSizePixel();

    // determine the size of the output-area and if we need scrollbars
    const long nScrSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    sal_Bool bVVisible = sal_False; // by default no vertical-ScrollBar
    sal_Bool bHVisible = sal_False; // by default no horizontal-ScrollBar
    sal_Bool bChanged;          // determines if a visiblility was changed
    do
    {
        bChanged = sal_False;

        // does we need a vertical ScrollBar
        if ( aOutPixSz.Width() < aTotPixSz.Width() && !bHVisible )
        {   bHVisible = sal_True;
            aOutPixSz.Height() -= nScrSize;
            bChanged = sal_True;
        }

        // does we need a horizontal ScrollBar
        if ( aOutPixSz.Height() < aTotPixSz.Height() && !bVVisible )
        {   bVVisible = sal_True;
            aOutPixSz.Width() -= nScrSize;
            bChanged = sal_True;
        }

    }
    while ( bChanged );   // until no visibility has changed

    // store the old offset and map-mode
    MapMode aMap( GetMapMode() );
    Point aOldPixOffset( aPixOffset );

    // justify (right/bottom borders should never exceed the virtual window)
    Size aPixDelta;
    if ( aPixOffset.X() < 0 &&
         aPixOffset.X() + aTotPixSz.Width() < aOutPixSz.Width() )
        aPixDelta.Width() =
            aOutPixSz.Width() - ( aPixOffset.X() + aTotPixSz.Width() );
    if ( aPixOffset.Y() < 0 &&
         aPixOffset.Y() + aTotPixSz.Height() < aOutPixSz.Height() )
        aPixDelta.Height() =
            aOutPixSz.Height() - ( aPixOffset.Y() + aTotPixSz.Height() );
    if ( aPixDelta.Width() || aPixDelta.Height() )
    {
        aPixOffset.X() += aPixDelta.Width();
        aPixOffset.Y() += aPixDelta.Height();
    }

    // for axis without scrollbar restore the origin
    if ( !bVVisible || !bHVisible )
    {
        aPixOffset = Point(
                     bHVisible
                     ? aPixOffset.X()
                     : ( bHCenter
                            ? (aOutPixSz.Width()-aTotPixSz.Width()) / 2
                            : 0 ),
                     bVVisible
                     ? aPixOffset.Y()
                     : ( bVCenter
                            ? (aOutPixSz.Height()-aTotPixSz.Height()) / 2
                            : 0 ) );
    }
    if ( bHVisible && !aHScroll.IsVisible() )
        aPixOffset.X() = 0;
    if ( bVVisible && !aVScroll.IsVisible() )
        aPixOffset.Y() = 0;

    // select the shifted map-mode
    if ( aPixOffset != aOldPixOffset )
    {
        Window::SetMapMode( MapMode( MAP_PIXEL ) );
        Window::Scroll(
            aPixOffset.X() - aOldPixOffset.X(),
            aPixOffset.Y() - aOldPixOffset.Y() );
        SetMapMode( aMap );
    }

    // show or hide scrollbars
    aVScroll.Show( bVVisible );
    aHScroll.Show( bHVisible );

    // disable painting in the corner between the scrollbars
    if ( bVVisible && bHVisible )
    {
        aCornerWin.SetPosSizePixel(Point(aOutPixSz.Width(), aOutPixSz.Height()),
            Size(nScrSize, nScrSize) );
        aCornerWin.Show();
    }
    else
        aCornerWin.Hide();

    // resize scrollbars and set their ranges
    if ( bHVisible )
    {
        aHScroll.SetPosSizePixel(
            Point( 0, aOutPixSz.Height() ),
            Size( aOutPixSz.Width(), nScrSize ) );
        aHScroll.SetRange( Range( 0, aTotPixSz.Width() ) );
        aHScroll.SetPageSize( aOutPixSz.Width() );
        aHScroll.SetVisibleSize( aOutPixSz.Width() );
        aHScroll.SetLineSize( nColumnPixW );
        aHScroll.SetThumbPos( -aPixOffset.X() );
    }
    if ( bVVisible )
    {
        aVScroll.SetPosSizePixel(
            Point( aOutPixSz.Width(), 0 ),
            Size( nScrSize,aOutPixSz.Height() ) );
        aVScroll.SetRange( Range( 0, aTotPixSz.Height() ) );
        aVScroll.SetPageSize( aOutPixSz.Height() );
        aVScroll.SetVisibleSize( aOutPixSz.Height() );
        aVScroll.SetLineSize( nLinePixH );
        aVScroll.SetThumbPos( -aPixOffset.Y() );
    }
}

//-------------------------------------------------------------------

void __EXPORT ScrollableWindow::StartScroll()
{
}

//-------------------------------------------------------------------

void __EXPORT ScrollableWindow::EndScroll( long, long )
{
}

//-------------------------------------------------------------------

void ScrollableWindow::SetMapMode( const MapMode& rNewMapMode )
{
    MapMode aMap( rNewMapMode );
    aMap.SetOrigin( aMap.GetOrigin() + PixelToLogic( aPixOffset, aMap ) );
    Window::SetMapMode( aMap );
}

//-------------------------------------------------------------------

MapMode ScrollableWindow::GetMapMode() const
{
    MapMode aMap( Window::GetMapMode() );
    aMap.SetOrigin( aMap.GetOrigin() - PixelToLogic( aPixOffset ) );
    return aMap;
}

//-------------------------------------------------------------------

void ScrollableWindow::SetTotalSize( const Size& rNewSize )
{
    aTotPixSz = LogicToPixel( rNewSize );
    ScrollableWindow::Resize();
}

//-------------------------------------------------------------------

void ScrollableWindow::SetVisibleSize( const Size& rNewSize )
{
    // get the rectangle, we wish to view
    Rectangle aWish( Point(0, 0), LogicToPixel(rNewSize) );

    // get maximum rectangle for us from our parent-window (subst our border!)
    Rectangle aMax( Point(0, 0), GetParent()->GetOutputSizePixel() );
    aMax.Left() -=  ( Window::GetSizePixel().Width() -
                    Window::GetOutputSizePixel().Width() );
    aMax.Bottom() -= (Window::GetSizePixel().Height() -
                     Window::GetOutputSizePixel().Height());

    Size aWill( aWish.GetIntersection(aMax).GetSize() );
    sal_Bool bHScroll = sal_False;
    const long nScrSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( aWill.Width() < aWish.GetSize().Width() )
    {   bHScroll = sal_True;
        aWill.Height() =
            Min( aWill.Height()+nScrSize, aMax.GetSize().Height() );
    }
    if ( aWill.Height() < aWish.GetSize().Height() )
        aWill.Width() =
            Min( aWill.Width()+nScrSize, aMax.GetSize().Width() );
    if ( !bHScroll && (aWill.Width() < aWish.GetSize().Width()) )
        aWill.Height() =
            Min( aWill.Height()+nScrSize, aMax.GetSize().Height() );
    Window::SetOutputSizePixel( aWill );
}

//-------------------------------------------------------------------

sal_Bool ScrollableWindow::MakeVisible( const Rectangle& rTarget, sal_Bool bSloppy )
{
    Rectangle aTarget;
    Rectangle aTotRect( Point(0, 0), PixelToLogic( aTotPixSz ) );

    if ( bSloppy )
    {
        aTarget = rTarget;

        // at maximum to right border
        if ( aTarget.Right() > aTotRect.Right() )
        {
            long nDelta = aTarget.Right() - aTotRect.Right();
            aTarget.Left() -= nDelta;
            aTarget.Right() -= nDelta;

            // too wide?
            if ( aTarget.Left() < aTotRect.Left() )
                aTarget.Left() = aTotRect.Left();
        }

        // at maximum to bottom border
        if ( aTarget.Bottom() > aTotRect.Bottom() )
        {
            long nDelta = aTarget.Bottom() - aTotRect.Bottom();
            aTarget.Top() -= nDelta;
            aTarget.Bottom() -= nDelta;

            // too high?
            if ( aTarget.Top() < aTotRect.Top() )
                aTarget.Top() = aTotRect.Top();
        }

        // at maximum to left border
        if ( aTarget.Left() < aTotRect.Left() )
        {
            long nDelta = aTarget.Left() - aTotRect.Left();
            aTarget.Right() -= nDelta;
            aTarget.Left() -= nDelta;

            // too wide?
            if ( aTarget.Right() > aTotRect.Right() )
                aTarget.Right() = aTotRect.Right();
        }

        // at maximum to top border
        if ( aTarget.Top() < aTotRect.Top() )
        {
            long nDelta = aTarget.Top() - aTotRect.Top();
            aTarget.Bottom() -= nDelta;
            aTarget.Top() -= nDelta;

            // too high?
            if ( aTarget.Bottom() > aTotRect.Bottom() )
                aTarget.Bottom() = aTotRect.Bottom();
        }
    }
    else
        aTarget = rTarget.GetIntersection( aTotRect );

    // is the area already visible?
    Rectangle aVisArea( GetVisibleArea() );
    if ( aVisArea.IsInside(rTarget) )
        return sal_True;

    // is there somewhat to scroll?
    if ( aVisArea.TopLeft() != aTarget.TopLeft() )
    {
        Rectangle aBox( aTarget.GetUnion(aVisArea) );
        long  nDeltaX = ( aBox.Right() - aVisArea.Right() ) +
                        ( aBox.Left() - aVisArea.Left() );
        long  nDeltaY = ( aBox.Top() - aVisArea.Top() ) +
                        ( aBox.Bottom() - aVisArea.Bottom() );
        Scroll( nDeltaX, nDeltaY );
    }

    // determine if the target is completely visible
    return aVisArea.GetWidth() >= aTarget.GetWidth() &&
           aVisArea.GetHeight() >= aTarget.GetHeight();
}

//-------------------------------------------------------------------

Rectangle ScrollableWindow::GetVisibleArea() const
{
    Point aTopLeft( PixelToLogic( Point() ) );
    Size aSz( GetOutputSize() );
    return Rectangle( aTopLeft, aSz );
}

//-------------------------------------------------------------------

void ScrollableWindow::SetLineSize( sal_uLong nHorz, sal_uLong nVert )
{
    Size aPixSz( LogicToPixel( Size(nHorz, nVert) ) );
    nColumnPixW = aPixSz.Width();
    nLinePixH = aPixSz.Height();
    aVScroll.SetLineSize( nLinePixH );
    aHScroll.SetLineSize( nColumnPixW );
}

//-------------------------------------------------------------------

void ScrollableWindow::Scroll( long nDeltaX, long nDeltaY, sal_uInt16 )
{
    if ( !bScrolling )
        StartScroll();

    // get the delta in pixel
    Size aDeltaPix( LogicToPixel( Size(nDeltaX, nDeltaY) ) );
    Size aOutPixSz( GetOutputSizePixel() );
    MapMode aMap( GetMapMode() );
    Point aNewPixOffset( aPixOffset );

    // scrolling horizontally?
    if ( nDeltaX != 0 )
    {
        aNewPixOffset.X() -= aDeltaPix.Width();
        if ( ( aOutPixSz.Width() - aNewPixOffset.X() ) > aTotPixSz.Width() )
            aNewPixOffset.X() = - ( aTotPixSz.Width() - aOutPixSz.Width() );
        else if ( aNewPixOffset.X() > 0 )
            aNewPixOffset.X() = 0;
    }

    // scrolling vertically?
    if ( nDeltaY != 0 )
    {
        aNewPixOffset.Y() -= aDeltaPix.Height();
        if ( ( aOutPixSz.Height() - aNewPixOffset.Y() ) > aTotPixSz.Height() )
            aNewPixOffset.Y() = - ( aTotPixSz.Height() - aOutPixSz.Height() );
        else if ( aNewPixOffset.Y() > 0 )
            aNewPixOffset.Y() = 0;
    }

    // recompute the logical scroll units
    aDeltaPix.Width() = aPixOffset.X() - aNewPixOffset.X();
    aDeltaPix.Height() = aPixOffset.Y() - aNewPixOffset.Y();
    Size aDelta( PixelToLogic(aDeltaPix) );
    nDeltaX = aDelta.Width();
    nDeltaY = aDelta.Height();
    aPixOffset = aNewPixOffset;

    // scrolling?
    if ( nDeltaX != 0 || nDeltaY != 0 )
    {
        Update();

        // does the new area overlap the old one?
        if ( Abs( (int)aDeltaPix.Height() ) < aOutPixSz.Height() ||
             Abs( (int)aDeltaPix.Width() ) < aOutPixSz.Width() )
        {
            // scroll the overlapping area
            SetMapMode( aMap );

            // never scroll the scrollbars itself!
            Window::Scroll(-nDeltaX, -nDeltaY,
                PixelToLogic( Rectangle( Point(0, 0), aOutPixSz ) ) );
        }
        else
        {
            // repaint all
            SetMapMode( aMap );
            Invalidate();
        }

        Update();
    }

    if ( !bScrolling )
    {
        EndScroll( nDeltaX, nDeltaY );
        if ( nDeltaX )
            aHScroll.SetThumbPos( -aPixOffset.X() );
        if ( nDeltaY )
            aVScroll.SetThumbPos( -aPixOffset.Y() );
    }
}

//-------------------------------------------------------------------

void ScrollableWindow::ScrollLines( long nLinesX, long nLinesY )
{
    Size aDelta( PixelToLogic( Size( nColumnPixW, nLinePixH ) ) );
    Scroll( aDelta.Width()*nLinesX, aDelta.Height()*nLinesY );
}

//-------------------------------------------------------------------

void ScrollableWindow::ScrollPages( long nPagesX, sal_uLong nOverlapX,
                                    long nPagesY, sal_uLong nOverlapY )
{
    Size aOutSz( GetVisibleArea().GetSize() );
    Scroll( nPagesX * aOutSz.Width() + (nPagesX>0 ? 1 : -1) * nOverlapX,
            nPagesY * aOutSz.Height() + (nPagesY>0 ? 1 : -1) * nOverlapY );
}


