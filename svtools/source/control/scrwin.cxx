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

#include <svtools/scrwin.hxx>
#include <vcl/settings.hxx>



void ScrollableWindow::ImpInitialize( ScrollableWindowFlags nFlags )
{
    bHandleDragging = (sal_Bool) ( nFlags & SCRWIN_THUMBDRAGGING );
    bVCenter = (nFlags & SCRWIN_VCENTER) == SCRWIN_VCENTER;
    bHCenter = (nFlags & SCRWIN_HCENTER) == SCRWIN_HCENTER;
    bScrolling = sal_False;

    
    aVScroll.SetScrollHdl( LINK(this, ScrollableWindow, ScrollHdl) );
    aHScroll.SetScrollHdl( LINK(this, ScrollableWindow, ScrollHdl) );
    aVScroll.SetEndScrollHdl( LINK(this, ScrollableWindow, EndScrollHdl) );
    aHScroll.SetEndScrollHdl( LINK(this, ScrollableWindow, EndScrollHdl) );

    nColumnPixW = nLinePixH = GetSettings().GetStyleSettings().GetScrollBarSize();
}



ScrollableWindow::ScrollableWindow( Window* pParent, WinBits nBits,
                                    ScrollableWindowFlags nFlags ) :
    Window( pParent, WinBits(nBits|WB_CLIPCHILDREN) ),
    aVScroll( this, WinBits(WB_VSCROLL | WB_DRAG) ),
    aHScroll( this, WinBits(WB_HSCROLL | WB_DRAG) ),
    aCornerWin( this )
{
    ImpInitialize( nFlags );
}



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



Size ScrollableWindow::GetOutputSizePixel() const
{
    Size aSz( Window::GetOutputSizePixel() );

    long nTmp = GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( aHScroll.IsVisible() )
        aSz.Height() -= nTmp;
    if ( aVScroll.IsVisible() )
        aSz.Width() -= nTmp;
    return aSz;
}



IMPL_LINK( ScrollableWindow, EndScrollHdl, ScrollBar *, pScroll )
{
    
    if ( !bScrolling )
        StartScroll(), bScrolling = sal_True;

    
    Size aDelta( PixelToLogic( Size( aHScroll.GetDelta(), aVScroll.GetDelta() ) ) );

    
    if ( !bHandleDragging )
    {
        if ( pScroll == &aHScroll )
            Scroll( aDelta.Width(), 0 );
        else
            Scroll( 0, aDelta.Height() );
    }

    
    bScrolling = sal_False;
    EndScroll( aDelta.Width(), aDelta.Height() );
    return 0;
}



IMPL_LINK( ScrollableWindow, ScrollHdl, ScrollBar *, pScroll )
{
    
    if ( !bScrolling )
        StartScroll(), bScrolling = sal_True;

    if ( bHandleDragging )
    {
        
        Size aDelta( PixelToLogic(
            Size( aHScroll.GetDelta(), aVScroll.GetDelta() ) ) );
        if ( pScroll == &aHScroll )
            Scroll( aDelta.Width(), 0 );
        else
            Scroll( 0, aDelta.Height() );
    }
    return 0;
}



void ScrollableWindow::Resize()
{
    
    Size aOutPixSz = Window::GetOutputSizePixel();

    
    const long nScrSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    sal_Bool bVVisible = sal_False; 
    sal_Bool bHVisible = sal_False; 
    sal_Bool bChanged;          
    do
    {
        bChanged = sal_False;

        
        if ( aOutPixSz.Width() < aTotPixSz.Width() && !bHVisible )
        {   bHVisible = sal_True;
            aOutPixSz.Height() -= nScrSize;
            bChanged = sal_True;
        }

        
        if ( aOutPixSz.Height() < aTotPixSz.Height() && !bVVisible )
        {   bVVisible = sal_True;
            aOutPixSz.Width() -= nScrSize;
            bChanged = sal_True;
        }

    }
    while ( bChanged );   

    
    MapMode aMap( GetMapMode() );
    Point aOldPixOffset( aPixOffset );

    
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

    
    if ( aPixOffset != aOldPixOffset )
    {
        Window::SetMapMode( MapMode( MAP_PIXEL ) );
        Window::Scroll(
            aPixOffset.X() - aOldPixOffset.X(),
            aPixOffset.Y() - aOldPixOffset.Y() );
        SetMapMode( aMap );
    }

    
    aVScroll.Show( bVVisible );
    aHScroll.Show( bHVisible );

    
    if ( bVVisible && bHVisible )
    {
        aCornerWin.SetPosSizePixel(Point(aOutPixSz.Width(), aOutPixSz.Height()),
            Size(nScrSize, nScrSize) );
        aCornerWin.Show();
    }
    else
        aCornerWin.Hide();

    
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



void ScrollableWindow::StartScroll()
{
}



void ScrollableWindow::EndScroll( long, long )
{
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



void ScrollableWindow::Scroll( long nDeltaX, long nDeltaY, sal_uInt16 )
{
    if ( !bScrolling )
        StartScroll();

    
    Size aDeltaPix( LogicToPixel( Size(nDeltaX, nDeltaY) ) );
    Size aOutPixSz( GetOutputSizePixel() );
    MapMode aMap( GetMapMode() );
    Point aNewPixOffset( aPixOffset );

    
    if ( nDeltaX != 0 )
    {
        aNewPixOffset.X() -= aDeltaPix.Width();
        if ( ( aOutPixSz.Width() - aNewPixOffset.X() ) > aTotPixSz.Width() )
            aNewPixOffset.X() = - ( aTotPixSz.Width() - aOutPixSz.Width() );
        else if ( aNewPixOffset.X() > 0 )
            aNewPixOffset.X() = 0;
    }

    
    if ( nDeltaY != 0 )
    {
        aNewPixOffset.Y() -= aDeltaPix.Height();
        if ( ( aOutPixSz.Height() - aNewPixOffset.Y() ) > aTotPixSz.Height() )
            aNewPixOffset.Y() = - ( aTotPixSz.Height() - aOutPixSz.Height() );
        else if ( aNewPixOffset.Y() > 0 )
            aNewPixOffset.Y() = 0;
    }

    
    aDeltaPix.Width() = aPixOffset.X() - aNewPixOffset.X();
    aDeltaPix.Height() = aPixOffset.Y() - aNewPixOffset.Y();
    Size aDelta( PixelToLogic(aDeltaPix) );
    nDeltaX = aDelta.Width();
    nDeltaY = aDelta.Height();
    aPixOffset = aNewPixOffset;

    
    if ( nDeltaX != 0 || nDeltaY != 0 )
    {
        Update();

        
        if ( std::abs( (int)aDeltaPix.Height() ) < aOutPixSz.Height() ||
             std::abs( (int)aDeltaPix.Width() ) < aOutPixSz.Width() )
        {
            
            SetMapMode( aMap );

            
            Window::Scroll(-nDeltaX, -nDeltaY,
                PixelToLogic( Rectangle( Point(0, 0), aOutPixSz ) ) );
        }
        else
        {
            
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
