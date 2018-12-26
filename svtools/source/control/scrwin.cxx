/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svtools/scrwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>


ScrollableWindow::ScrollableWindow( vcl::Window* pParent ) :
    Window( pParent, WB_CLIPCHILDREN ),
    aVScroll( VclPtr<ScrollBar>::Create(this, WinBits(WB_VSCROLL | WB_DRAG)) ),
    aHScroll( VclPtr<ScrollBar>::Create(this, WinBits(WB_HSCROLL | WB_DRAG)) ),
    aCornerWin( VclPtr<ScrollBarBox>::Create(this) )
{
    bScrolling = false;

    // set the handlers for the scrollbars
    aVScroll->SetScrollHdl( LINK(this, ScrollableWindow, ScrollHdl) );
    aHScroll->SetScrollHdl( LINK(this, ScrollableWindow, ScrollHdl) );
    aVScroll->SetEndScrollHdl( LINK(this, ScrollableWindow, EndScrollHdl) );
    aHScroll->SetEndScrollHdl( LINK(this, ScrollableWindow, EndScrollHdl) );

    nColumnPixW = nLinePixH = GetSettings().GetStyleSettings().GetScrollBarSize();
}


ScrollableWindow::~ScrollableWindow()
{
    disposeOnce();
}

void ScrollableWindow::dispose()
{
    aVScroll.disposeAndClear();
    aHScroll.disposeAndClear();
    aCornerWin.disposeAndClear();
    Window::dispose();
}


void ScrollableWindow::Command( const CommandEvent& rCEvt )
{
    if ( (rCEvt.GetCommand() == CommandEventId::Wheel) ||
         (rCEvt.GetCommand() == CommandEventId::StartAutoScroll) ||
         (rCEvt.GetCommand() == CommandEventId::AutoScroll) )
    {
        ScrollBar* pHScrBar;
        ScrollBar* pVScrBar;
        if ( aHScroll->IsVisible() )
            pHScrBar = aHScroll.get();
        else
            pHScrBar = nullptr;
        if ( aVScroll->IsVisible() )
            pVScrBar = aVScroll.get();
        else
            pVScrBar = nullptr;
        if ( HandleScrollCommand( rCEvt, pHScrBar, pVScrBar ) )
            return;
    }

    Window::Command( rCEvt );
}


void ScrollableWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        Resize();
        Invalidate();
    }

    Window::DataChanged( rDCEvt );
}


Size ScrollableWindow::GetOutputSizePixel() const
{
    Size aSz( Window::GetOutputSizePixel() );

    long nTmp = GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( aHScroll->IsVisible() )
        aSz.AdjustHeight( -nTmp );
    if ( aVScroll->IsVisible() )
        aSz.AdjustWidth( -nTmp );
    return aSz;
}


IMPL_LINK( ScrollableWindow, EndScrollHdl, ScrollBar *, /*pScroll*/, void )
{
    // notify the end of scrolling
    bScrolling = false;
}


IMPL_LINK( ScrollableWindow, ScrollHdl, ScrollBar *, pScroll, void )
{
    // notify the start of scrolling, if not already scrolling
    if ( !bScrolling )
        bScrolling = true;

    // get the delta in logic coordinates
    Size aDelta( PixelToLogic(
        Size( aHScroll->GetDelta(), aVScroll->GetDelta() ) ) );
    if ( pScroll == aHScroll.get() )
        Scroll( aDelta.Width(), 0 );
    else
        Scroll( 0, aDelta.Height() );
}


void ScrollableWindow::Resize()
{
    // get the new output-size in pixel
    Size aOutPixSz = Window::GetOutputSizePixel();

    // determine the size of the output-area and if we need scrollbars
    const long nScrSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    bool bVVisible = false; // by default no vertical-ScrollBar
    bool bHVisible = false; // by default no horizontal-ScrollBar
    bool bChanged;          // determines if a visiblility was changed
    do
    {
        bChanged = false;

        // does we need a vertical ScrollBar
        if ( aOutPixSz.Width() < aTotPixSz.Width() && !bHVisible )
        {
            bHVisible = true;
            aOutPixSz.AdjustHeight( -nScrSize );
            bChanged = true;
        }

        // does we need a horizontal ScrollBar
        if ( aOutPixSz.Height() < aTotPixSz.Height() && !bVVisible )
        {
            bVVisible = true;
            aOutPixSz.AdjustWidth( -nScrSize );
            bChanged = true;
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
        aPixDelta.setWidth(
            aOutPixSz.Width() - ( aPixOffset.X() + aTotPixSz.Width() ) );
    if ( aPixOffset.Y() < 0 &&
         aPixOffset.Y() + aTotPixSz.Height() < aOutPixSz.Height() )
        aPixDelta.setHeight(
            aOutPixSz.Height() - ( aPixOffset.Y() + aTotPixSz.Height() ) );
    if ( aPixDelta.Width() || aPixDelta.Height() )
    {
        aPixOffset.AdjustX(aPixDelta.Width() );
        aPixOffset.AdjustY(aPixDelta.Height() );
    }

    // for axis without scrollbar restore the origin
    if ( !bVVisible || !bHVisible )
    {
        aPixOffset = Point(
                     bHVisible
                     ? aPixOffset.X()
                     : (aOutPixSz.Width()-aTotPixSz.Width()) / 2,
                     bVVisible
                     ? aPixOffset.Y()
                     : (aOutPixSz.Height()-aTotPixSz.Height()) / 2 );
    }
    if ( bHVisible && !aHScroll->IsVisible() )
        aPixOffset.setX( 0 );
    if ( bVVisible && !aVScroll->IsVisible() )
        aPixOffset.setY( 0 );

    // select the shifted map-mode
    if ( aPixOffset != aOldPixOffset )
    {
        Window::SetMapMode( MapMode( MapUnit::MapPixel ) );
        Window::Scroll(
            aPixOffset.X() - aOldPixOffset.X(),
            aPixOffset.Y() - aOldPixOffset.Y() );
        SetMapMode( aMap );
    }

    // show or hide scrollbars
    aVScroll->Show( bVVisible );
    aHScroll->Show( bHVisible );

    // disable painting in the corner between the scrollbars
    if ( bVVisible && bHVisible )
    {
        aCornerWin->SetPosSizePixel(Point(aOutPixSz.Width(), aOutPixSz.Height()),
            Size(nScrSize, nScrSize) );
        aCornerWin->Show();
    }
    else
        aCornerWin->Hide();

    // resize scrollbars and set their ranges
    if ( bHVisible )
    {
        aHScroll->SetPosSizePixel(
            Point( 0, aOutPixSz.Height() ),
            Size( aOutPixSz.Width(), nScrSize ) );
        aHScroll->SetRange( Range( 0, aTotPixSz.Width() ) );
        aHScroll->SetPageSize( aOutPixSz.Width() );
        aHScroll->SetVisibleSize( aOutPixSz.Width() );
        aHScroll->SetLineSize( nColumnPixW );
        aHScroll->SetThumbPos( -aPixOffset.X() );
    }
    if ( bVVisible )
    {
        aVScroll->SetPosSizePixel(
            Point( aOutPixSz.Width(), 0 ),
            Size( nScrSize,aOutPixSz.Height() ) );
        aVScroll->SetRange( Range( 0, aTotPixSz.Height() ) );
        aVScroll->SetPageSize( aOutPixSz.Height() );
        aVScroll->SetVisibleSize( aOutPixSz.Height() );
        aVScroll->SetLineSize( nLinePixH );
        aVScroll->SetThumbPos( -aPixOffset.Y() );
    }
}


void ScrollableWindow::SetMapMode( const MapMode& rNewMapMode )
{
    MapMode aMap( rNewMapMode );
    aMap.SetOrigin( aMap.GetOrigin() + PixelToLogic( aPixOffset, aMap ) );
    Window::SetMapMode( aMap );
}


MapMode ScrollableWindow::GetMapMode() const
{
    MapMode aMap( Window::GetMapMode() );
    aMap.SetOrigin( aMap.GetOrigin() - PixelToLogic( aPixOffset ) );
    return aMap;
}


void ScrollableWindow::SetTotalSize( const Size& rNewSize )
{
    aTotPixSz = LogicToPixel( rNewSize );
    ScrollableWindow::Resize();
}


void ScrollableWindow::Scroll( long nDeltaX, long nDeltaY, ScrollFlags )
{
    // get the delta in pixel
    Size aDeltaPix( LogicToPixel( Size(nDeltaX, nDeltaY) ) );
    Size aOutPixSz( GetOutputSizePixel() );
    MapMode aMap( GetMapMode() );
    Point aNewPixOffset( aPixOffset );

    // scrolling horizontally?
    if ( nDeltaX != 0 )
    {
        aNewPixOffset.AdjustX( -(aDeltaPix.Width()) );
        if ( ( aOutPixSz.Width() - aNewPixOffset.X() ) > aTotPixSz.Width() )
            aNewPixOffset.setX( - ( aTotPixSz.Width() - aOutPixSz.Width() ) );
        else if ( aNewPixOffset.X() > 0 )
            aNewPixOffset.setX( 0 );
    }

    // scrolling vertically?
    if ( nDeltaY != 0 )
    {
        aNewPixOffset.AdjustY( -(aDeltaPix.Height()) );
        if ( ( aOutPixSz.Height() - aNewPixOffset.Y() ) > aTotPixSz.Height() )
            aNewPixOffset.setY( - ( aTotPixSz.Height() - aOutPixSz.Height() ) );
        else if ( aNewPixOffset.Y() > 0 )
            aNewPixOffset.setY( 0 );
    }

    // recompute the logical scroll units
    aDeltaPix.setWidth( aPixOffset.X() - aNewPixOffset.X() );
    aDeltaPix.setHeight( aPixOffset.Y() - aNewPixOffset.Y() );
    Size aDelta( PixelToLogic(aDeltaPix) );
    nDeltaX = aDelta.Width();
    nDeltaY = aDelta.Height();
    aPixOffset = aNewPixOffset;

    // scrolling?
    if ( nDeltaX != 0 || nDeltaY != 0 )
    {
        Update();

        // does the new area overlap the old one?
        if ( std::abs( static_cast<int>(aDeltaPix.Height()) ) < aOutPixSz.Height() ||
             std::abs( static_cast<int>(aDeltaPix.Width()) ) < aOutPixSz.Width() )
        {
            // scroll the overlapping area
            SetMapMode( aMap );

            // never scroll the scrollbars itself!
            Window::Scroll(-nDeltaX, -nDeltaY,
                PixelToLogic( tools::Rectangle( Point(0, 0), aOutPixSz ) ) );
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
        if ( nDeltaX )
            aHScroll->SetThumbPos( -aPixOffset.X() );
        if ( nDeltaY )
            aVScroll->SetThumbPos( -aPixOffset.Y() );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
