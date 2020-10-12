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

#include <helper/scrollabledialog.hxx>
#include <vcl/settings.hxx>

namespace toolkit
{

// Using  WB_AUTOHSCROLL, WB_AUTOVSCROLL here sucks big time, there is a
// problem in the toolkit class where there are some clashing IDs
// ( css::awt::VclWindowPeerAttribute::VSCROLL has the same value
// as css::awt::WindowAttribute::NODECORATION and they are used
// in the same bitmap :-( WB_VSCROLL & WB_HSCROLL apparently are only for
// child classes ( whole thing is a mess if you ask me )
ScrollableDialog::ScrollableDialog( vcl::Window* pParent, WinBits nStyle, Dialog::InitFlag eFlag )
    : Dialog( pParent, nStyle & ~( WB_AUTOHSCROLL | WB_AUTOVSCROLL ), eFlag ),
      maHScrollBar( VclPtr<ScrollBar>::Create(this, WB_HSCROLL | WB_DRAG) ),
      maVScrollBar( VclPtr<ScrollBar>::Create(this, WB_VSCROLL | WB_DRAG) ),
      mbHasHoriBar( false ),
      mbHasVertBar( false ),
      maScrollVis( None )
{
    Link<ScrollBar*,void> aLink( LINK( this, ScrollableDialog, ScrollBarHdl ) );
    maVScrollBar->SetScrollHdl( aLink );
    maHScrollBar->SetScrollHdl( aLink );

    ScrollBarVisibility aVis = None;

    if ( nStyle & ( WB_AUTOHSCROLL | WB_AUTOVSCROLL ) )
    {
        if ( nStyle & WB_AUTOHSCROLL )
            aVis = Hori;
        if ( nStyle &  WB_AUTOVSCROLL )
        {
            if ( aVis == Hori )
                aVis = Both;
            else
                aVis = Vert;
        }
    }
    setScrollVisibility( aVis );
    mnScrWidth = Dialog::GetSettings().GetStyleSettings().GetScrollBarSize();
}

void ScrollableDialog::setScrollVisibility( ScrollBarVisibility rVisState )
{
    maScrollVis = rVisState;
    if (  maScrollVis == Hori || maScrollVis == Both )
    {
        mbHasHoriBar = true;
        maHScrollBar->Show();
    }
    if ( maScrollVis == Vert || maScrollVis == Both )
    {
        mbHasVertBar = true;
        maVScrollBar->Show();
    }
    if ( mbHasHoriBar || mbHasVertBar )
        SetStyle( Dialog::GetStyle() | WB_CLIPCHILDREN );
}

ScrollableDialog::~ScrollableDialog()
{
    disposeOnce();
}

void ScrollableDialog::dispose()
{
    maHScrollBar.disposeAndClear();
    maVScrollBar.disposeAndClear();
    Dialog::dispose();
}

void ScrollableDialog::lcl_Scroll( tools::Long nX, tools::Long nY )
{
    tools::Long nXScroll = mnScrollPos.X() - nX;
    tools::Long nYScroll = mnScrollPos.Y() - nY;
    mnScrollPos = Point( nX, nY );

    tools::Rectangle aScrollableArea( 0, 0, maScrollArea.Width(), maScrollArea.Height() );
    Scroll(nXScroll, nYScroll, aScrollableArea );
    // Manually scroll all children ( except the scrollbars )
    for ( int index = 0; index < GetChildCount(); ++index )
    {
        vcl::Window* pChild = GetChild( index );
        if ( pChild && pChild != maVScrollBar.get() && pChild != maHScrollBar.get() )
        {
            Point aPos = pChild->GetPosPixel();
            aPos += Point( nXScroll, nYScroll );
            pChild->SetPosPixel( aPos );
        }
    }
}

IMPL_LINK( ScrollableDialog, ScrollBarHdl, ScrollBar*, pSB, void )
{
    sal_uInt16 nPos = static_cast<sal_uInt16>(pSB->GetThumbPos());
    if( pSB == maVScrollBar.get() )
        lcl_Scroll(mnScrollPos.X(), nPos );
    else if( pSB == maHScrollBar.get() )
        lcl_Scroll(nPos, mnScrollPos.Y() );
}

void ScrollableDialog::SetScrollTop( tools::Long nTop )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() , mnScrollPos.Y() - nTop );
    maHScrollBar->SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}
void ScrollableDialog::SetScrollLeft( tools::Long nLeft )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() - nLeft , mnScrollPos.Y() );
    maVScrollBar->SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}

void ScrollableDialog::SetScrollWidth( tools::Long nWidth )
{
    maScrollArea.setWidth( nWidth );
    ResetScrollBars();
}

void ScrollableDialog::SetScrollHeight( tools::Long nHeight )
{
    maScrollArea.setHeight( nHeight );
    ResetScrollBars();
}

void ScrollableDialog::Resize()
{
    ResetScrollBars();
}

void ScrollableDialog::ResetScrollBars()
{
    Size aOutSz = GetOutputSizePixel();

    Point aVPos( aOutSz.Width() - mnScrWidth, 0 );
    Point aHPos( 0, aOutSz.Height() - mnScrWidth );

    maVScrollBar->SetPosSizePixel( aVPos, Size( mnScrWidth,  GetSizePixel().Height() - mnScrWidth ) );
    maHScrollBar->SetPosSizePixel( aHPos, Size(  GetSizePixel().Width() - mnScrWidth, mnScrWidth ) );

    maHScrollBar->SetRangeMax( maScrollArea.Width() + mnScrWidth  );
    maHScrollBar->SetVisibleSize( GetSizePixel().Width() );

    maVScrollBar->SetRangeMax( maScrollArea.Height() + mnScrWidth );
    maVScrollBar->SetVisibleSize( GetSizePixel().Height() );
}

} // toolkit
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
