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

#include <swtypes.hxx>
#include <swrect.hxx>
#include <scroll.hxx>

#define SCROLL_LINE_SIZE 250

SwScrollbar::SwScrollbar( vcl::Window *pWin, bool bHoriz ) :
    ScrollBar( pWin,
    WinBits( WB_3DLOOK | WB_HIDE | ( bHoriz ? WB_HSCROLL : WB_VSCROLL)  ) ),
    bHori( bHoriz ),
    bAuto( false ),
    bVisible(false),
    bSizeSet(false)
{
    // No mirroring for horizontal scrollbars
    if( bHoriz )
        EnableRTL( false );
}

 SwScrollbar::~SwScrollbar() {}

// Will be called after a change of the document size
// to refresh the range of the scrollbars.

void SwScrollbar::DocSzChgd( const Size &rSize )
{
    aDocSz = rSize;
    SetRange( Range( 0, bHori ? rSize.Width() : rSize.Height()) );
    const sal_uLong nVisSize = GetVisibleSize();
    SetLineSize( SCROLL_LINE_SIZE );
    SetPageSize( nVisSize * 77 / 100 );
}

// Will be called after a change of the visible view section.

void SwScrollbar::ViewPortChgd( const tools::Rectangle &rRect )
{
    long nThumb, nVisible;
    if( bHori )
    {
        nThumb = rRect.Left();
        nVisible = rRect.GetWidth();
    }
    else
    {
        nThumb = rRect.Top();
        nVisible = rRect.GetHeight();
    }

    SetVisibleSize( nVisible );
    DocSzChgd(aDocSz);
    SetThumbPos( nThumb );
    if(bAuto)
        AutoShow();
}

void SwScrollbar::ExtendedShow( bool bSet )
{
    bVisible = bSet;
    if( (!bSet ||  !bAuto) && IsUpdateMode() && bSizeSet)
        ScrollBar::Show(bSet);
}

void SwScrollbar::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    ScrollBar::SetPosSizePixel(rNewPos, rNewSize);
    bSizeSet = true;
    if(bVisible)
        ExtendedShow();

}

void SwScrollbar::SetAuto(bool bSet)
{
    if(bAuto != bSet)
    {
        bAuto = bSet;

        // hide automatically - then show
        if(!bAuto && bVisible && !ScrollBar::IsVisible())
            ExtendedShow();
        else if(bAuto)
            AutoShow(); // or hide automatically
    }
}

void SwScrollbar::AutoShow()
{
    long nVis = GetVisibleSize();
    long nLen = GetRange().Len();
    if( nVis >= nLen - 1)
    {
        if(ScrollBar::IsVisible())
            ScrollBar::Show(false);
    }
    else if ( !ScrollBar::IsVisible() )
    {
        ScrollBar::Show();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
