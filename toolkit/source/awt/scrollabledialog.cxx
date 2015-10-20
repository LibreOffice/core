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
#include <vcl/settings.hxx>

namespace toolkit
{

// Using  WB_AUTOHSCROLL, WB_AUTOVSCROLL here sucks big time, there is a
// problem in the toolkit class where there are some clashing IDs
// ( ::com::sun::star::awt::VclWindowPeerAttribute::VSCROLL has the same value
// as ::com::sun::star::awt::WindowAttribute::NODECORATION and they are used
// in the same bitmap :-( WB_VSCROLL & WB_HSCROLL apparently are only for
// child classes ( whole thing is a mess if you ask me )
template< class T>
ScrollableWrapper<T>::ScrollableWrapper( vcl::Window* pParent, WinBits nStyle, Dialog::InitFlag eFlag )
    : T( pParent, nStyle & ~( WB_AUTOHSCROLL | WB_AUTOVSCROLL ), eFlag ),
      maHScrollBar( VclPtr<ScrollBar>::Create(this, WB_HSCROLL | WB_DRAG) ),
      maVScrollBar( VclPtr<ScrollBar>::Create(this, WB_VSCROLL | WB_DRAG) ),
      mbHasHoriBar( false ),
      mbHasVertBar( false ),
      maScrollVis( None )
{
    Link<ScrollBar*,void> aLink( LINK( this, ScrollableWrapper, ScrollBarHdl ) );
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
    mnScrWidth = T::GetSettings().GetStyleSettings().GetScrollBarSize();
}

template< class T>
void ScrollableWrapper<T>::setScrollVisibility( ScrollBarVisibility rVisState )
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
        this->SetStyle( T::GetStyle() | WB_CLIPCHILDREN | WB_AUTOSIZE );
}

template< class T>
ScrollableWrapper<T>::~ScrollableWrapper()
{
    T::disposeOnce();
}

template< class T>
void ScrollableWrapper<T>::dispose()
{
    maHScrollBar.disposeAndClear();
    maVScrollBar.disposeAndClear();
    T::dispose();
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
        vcl::Window* pChild = T::GetChild( index );
        if ( pChild && pChild != maVScrollBar.get() && pChild != maHScrollBar.get() )
        {
            Point aPos = pChild->GetPosPixel();
            aPos += Point( nXScroll, nYScroll );
            pChild->SetPosPixel( aPos );
        }
    }
}

//Can't use IMPL_LINK_TYPED with the template
//IMPL_LINK_TYPED( ScrollableWrapper, ScrollBarHdl, ScrollBar*, pSB, void )

template< class T>
void ScrollableWrapper<T>::LinkStubScrollBarHdl( void* pThis, ScrollBar* pCaller)
{
    static_cast<ScrollableWrapper<T>*>(pThis)->ScrollBarHdl( static_cast<ScrollBar*>(pCaller) );
}

template< class T>
void ScrollableWrapper<T>::ScrollBarHdl( ScrollBar* pSB )
{
    sal_uInt16 nPos = (sal_uInt16) pSB->GetThumbPos();
    if( pSB == maVScrollBar.get() )
        lcl_Scroll(mnScrollPos.X(), nPos );
    else if( pSB == maHScrollBar.get() )
        lcl_Scroll(nPos, mnScrollPos.Y() );
}

template< class T>
void ScrollableWrapper<T>::SetScrollTop( long nTop )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() , mnScrollPos.Y() - nTop );
    maHScrollBar->SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}
template< class T>
void ScrollableWrapper<T>::SetScrollLeft( long nLeft )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() - nLeft , mnScrollPos.Y() );
    maVScrollBar->SetThumbPos( 0 );
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

    maVScrollBar->SetPosSizePixel( aVPos, Size( mnScrWidth,  T::GetSizePixel().Height() - mnScrWidth ) );
    maHScrollBar->SetPosSizePixel( aHPos, Size(  T::GetSizePixel().Width() - mnScrWidth, mnScrWidth ) );

    maHScrollBar->SetRangeMax( maScrollArea.Width() + mnScrWidth  );
    maHScrollBar->SetVisibleSize( T::GetSizePixel().Width() );

    maVScrollBar->SetRangeMax( maScrollArea.Height() + mnScrWidth );
    maVScrollBar->SetVisibleSize( T::GetSizePixel().Height() );
}

template class ScrollableWrapper< Dialog >;

} // toolkit
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
