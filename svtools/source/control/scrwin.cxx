/*************************************************************************
 *
 *  $RCSfile: scrwin.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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

#define _SVT_SCRWIN_CXX
#include <scrwin.hxx>

//===================================================================

void ScrollableWindow::ImpInitialize( ScrollableWindowFlags nFlags )
{
    bHandleDragging = (BOOL) ( nFlags & SCRWIN_THUMBDRAGGING );
    bVCenter = (nFlags & SCRWIN_VCENTER) == SCRWIN_VCENTER;
    bHCenter = (nFlags & SCRWIN_HCENTER) == SCRWIN_HCENTER;
    bScrolling = FALSE;

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
        StartScroll(), bScrolling = TRUE;

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
    bScrolling = FALSE;
    EndScroll( aDelta.Width(), aDelta.Height() );
    return 0;
}

//-------------------------------------------------------------------

IMPL_LINK( ScrollableWindow, ScrollHdl, ScrollBar *, pScroll )
{
    // notify the start of scrolling, if not already scrolling
    if ( !bScrolling )
        StartScroll(), bScrolling = TRUE;

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
    BOOL bVVisible = FALSE; // by default no vertical-ScrollBar
    BOOL bHVisible = FALSE; // by default no horizontal-ScrollBar
    BOOL bChanged;          // determines if a visiblility was changed
    do
    {
        bChanged = FALSE;

        // does we need a vertical ScrollBar
        if ( aOutPixSz.Width() < aTotPixSz.Width() && !bHVisible )
        {   bHVisible = TRUE;
            aOutPixSz.Height() -= nScrSize;
            bChanged = TRUE;
        }

        // does we need a horizontal ScrollBar
        if ( aOutPixSz.Height() < aTotPixSz.Height() && !bVVisible )
        {   bVVisible = TRUE;
            aOutPixSz.Width() -= nScrSize;
            bChanged = TRUE;
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
        aCornerWin.SetPosSizePixel(
            *((Point*) &aOutPixSz), Size(nScrSize, nScrSize) );
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

void __EXPORT ScrollableWindow::EndScroll( long nDeltaX, long nDeltaY )
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
    BOOL bHScroll = FALSE;
    const long nScrSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( aWill.Width() < aWish.GetSize().Width() )
    {   bHScroll = TRUE;
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

BOOL ScrollableWindow::MakeVisible( const Rectangle& rTarget, BOOL bSloppy )
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
        return TRUE;

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

void ScrollableWindow::SetLineSize( ULONG nHorz, ULONG nVert )
{
    Size aPixSz( LogicToPixel( Size(nHorz, nVert) ) );
    nColumnPixW = aPixSz.Width();
    nLinePixH = aPixSz.Height();
    aVScroll.SetLineSize( nLinePixH );
    aHScroll.SetLineSize( nColumnPixW );
}

//-------------------------------------------------------------------

void ScrollableWindow::Scroll( long nDeltaX, long nDeltaY )
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

void ScrollableWindow::ScrollPages( long nPagesX, ULONG nOverlapX,
                                    long nPagesY, ULONG nOverlapY )
{
    Size aOutSz( GetVisibleArea().GetSize() );
    Scroll( nPagesX * aOutSz.Width() + (nPagesX>0 ? 1 : -1) * nOverlapX,
            nPagesY * aOutSz.Height() + (nPagesY>0 ? 1 : -1) * nOverlapY );
}


