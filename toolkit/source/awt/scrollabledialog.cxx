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

#include <toolkit/awt/scrollabledialog.hxx>
#include <vcl/group.hxx>

namespace toolkit
{

// Using  WB_AUTOHSCROLL, WB_AUTOVSCROLL here sucks big time, there is a
// problem in the toolkit class where there are some clashing IDs
// ( ::com::sun::star::awt::VclWindowPeerAttribute::VSCROLL has the same value
// as ::com::sun::star::awt::WindowAttribute::NODECORATION and they are used
// in the same bitmap :-( WB_VSCROLL & WB_HSCROLL apparently are only for
// child classes ( whole thing is a mess if you ask me )
template< class T>
ScrollableWrapper<T>::ScrollableWrapper( Window* pParent, WinBits nStyle ) : T( pParent, nStyle & ~( WB_AUTOHSCROLL | WB_AUTOVSCROLL ) ), maHScrollBar( this, WB_HSCROLL | WB_DRAG), maVScrollBar( this, WB_VSCROLL | WB_DRAG ), mbHasHoriBar( false ), mbHasVertBar( false ), maScrollVis( None )
{
    Link aLink( LINK( this, ScrollableWrapper, ScrollBarHdl ) );
    maVScrollBar.SetScrollHdl( aLink );
    maHScrollBar.SetScrollHdl( aLink );

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
    mnScrWidth = T::GetSettings().GetStyleSettings().GetScrollBarSize();
}

template< class T>
void ScrollableWrapper<T>::setScrollVisibility( ScrollBarVisibility rVisState )
{
    maScrollVis = rVisState;
    if (  maScrollVis == Hori || maScrollVis == Both )
    {
        mbHasHoriBar = true;
        maHScrollBar.Show();
    }
    if ( maScrollVis == Vert || maScrollVis == Both )
    {
        mbHasVertBar = true;
        maVScrollBar.Show();
    }
    if ( mbHasHoriBar || mbHasVertBar )
        this->SetStyle( T::GetStyle() | WB_CLIPCHILDREN | SCROLL_UPDATE );
}

template< class T>
ScrollableWrapper<T>::~ScrollableWrapper()
{
}

template< class T>
void ScrollableWrapper<T>::lcl_Scroll( long nX, long nY )
{
    long nXScroll = mnScrollPos.X() - nX;
    long nYScroll = mnScrollPos.Y() - nY;
    mnScrollPos = Point( nX, nY );

    Rectangle aScrollableArea( 0, 0, maScrollArea.Width(), maScrollArea.Height() );
    T::Scroll(nXScroll, nYScroll, aScrollableArea );
    // Manually scroll all children ( except the scrollbars )
    for ( int index = 0; index < T::GetChildCount(); ++index )
    {
        Window* pChild = T::GetChild( index );
        if ( pChild && pChild != &maVScrollBar && pChild != &maHScrollBar )
        {
            Point aPos = pChild->GetPosPixel();
            aPos += Point( nXScroll, nYScroll );
            pChild->SetPosPixel( aPos );
        }
    }
}

//Can't use IMPL_LINK with the template
//IMPL_LINK( ScrollableWrapper, ScrollBarHdl, ScrollBar*, pSB )

template< class T>
long ScrollableWrapper<T>::LinkStubScrollBarHdl( void* pThis, void* pCaller)
{
    return ((ScrollableWrapper<T>*)pThis )->ScrollBarHdl( (ScrollBar*)pCaller );
}

template< class T>
long ScrollableWrapper<T>::ScrollBarHdl( ScrollBar* pSB )
{
    sal_uInt16 nPos = (sal_uInt16) pSB->GetThumbPos();
    if( pSB == &maVScrollBar )
        lcl_Scroll(mnScrollPos.X(), nPos );
    else if( pSB == &maHScrollBar )
        lcl_Scroll(nPos, mnScrollPos.Y() );
    return 1;
}

template< class T>
void ScrollableWrapper<T>::SetScrollTop( long nTop )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() , mnScrollPos.Y() - nTop );
    maHScrollBar.SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}
template< class T>
void ScrollableWrapper<T>::SetScrollLeft( long nLeft )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() - nLeft , mnScrollPos.Y() );
    maVScrollBar.SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}
template< class T>
void ScrollableWrapper<T>::SetScrollWidth( long nWidth )
{
    maScrollArea.Width() = nWidth;
    ResetScrollBars();
}

template< class T>
void ScrollableWrapper<T>::SetScrollHeight( long nHeight )
{
    maScrollArea.Height() = nHeight;
    ResetScrollBars();
}

template< class T>
void ScrollableWrapper<T>::Resize()
{
    ResetScrollBars();
}

template< class T>
void ScrollableWrapper<T>::ResetScrollBars()
{
    Size aOutSz = T::GetOutputSizePixel();

    Point aVPos( aOutSz.Width() - mnScrWidth, 0 );
    Point aHPos( 0, aOutSz.Height() - mnScrWidth );

    maVScrollBar.SetPosSizePixel( aVPos, Size( mnScrWidth,  T::GetSizePixel().Height() - mnScrWidth ) );
    maHScrollBar.SetPosSizePixel( aHPos, Size(  T::GetSizePixel().Width() - mnScrWidth, mnScrWidth ) );

    maHScrollBar.SetRangeMax( maScrollArea.Width() + mnScrWidth  );
    maHScrollBar.SetVisibleSize( T::GetSizePixel().Width() );

    maVScrollBar.SetRangeMax( maScrollArea.Height() + mnScrWidth );
    maVScrollBar.SetVisibleSize( T::GetSizePixel().Height() );
}

template class ScrollableWrapper< Dialog >;
#if SCROLLABLEFRAME
template class ScrollableWrapper< GroupBox >;
#endif

} // toolkit
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
