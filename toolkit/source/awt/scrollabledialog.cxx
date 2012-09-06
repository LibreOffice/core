/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <toolkit/awt/scrollabledialog.hxx>
#include <vcl/group.hxx>

namespace toolkit
{

template< class T>
ScrollableDialog<T>::ScrollableDialog( Window* pParent, WinBits nStyle ) : T( pParent, nStyle | ~( WB_HSCROLL | WB_VSCROLL ) ), maHScrollBar( this, WB_HSCROLL | WB_DRAG), maVScrollBar( this, WB_VSCROLL | WB_DRAG ), mbHasHoriBar( false ), mbHasVertBar( false ), maScrollVis( None )
{
    Link aLink( LINK( this, ScrollableDialog, ScrollBarHdl ) );
    maVScrollBar.SetScrollHdl( aLink );
    maHScrollBar.SetScrollHdl( aLink );

    Size aOutSz = T::GetOutputSizePixel();
    ScrollBarVisibility aVis = None;

    if ( nStyle & ( WB_HSCROLL | WB_VSCROLL ) )
    {
        if ( nStyle & WB_HSCROLL )
            aVis = Hori;
        if ( nStyle &  WB_VSCROLL )
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
void ScrollableDialog<T>::setScrollVisibility( ScrollBarVisibility rVisState )
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
        SetStyle( T::GetStyle() | WB_CLIPCHILDREN | SCROLL_UPDATE );
}

template< class T>
ScrollableDialog<T>::~ScrollableDialog()
{
}

template< class T>
Window* ScrollableDialog<T>::getContentWindow()
{
    return this;
}

template< class T>
void ScrollableDialog<T>::lcl_Scroll( long nX, long nY )
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
//IMPL_LINK( ScrollableDialog, ScrollBarHdl, ScrollBar*, pSB )

template< class T>
long ScrollableDialog<T>::LinkStubScrollBarHdl( void* pThis, void* pCaller)
{
    return ((ScrollableDialog<T>*)pThis )->ScrollBarHdl( (ScrollBar*)pCaller );
}

template< class T>
long ScrollableDialog<T>::ScrollBarHdl( ScrollBar* pSB )
{
    sal_uInt16 nPos = (sal_uInt16) pSB->GetThumbPos();
    if( pSB == &maVScrollBar )
        lcl_Scroll(mnScrollPos.X(), nPos );
    else if( pSB == &maHScrollBar )
        lcl_Scroll(nPos, mnScrollPos.Y() );
    return 1;
}

template< class T>
void ScrollableDialog<T>::SetScrollTop( long nTop )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() , mnScrollPos.Y() - nTop );
    maHScrollBar.SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}
template< class T>
void ScrollableDialog<T>::SetScrollLeft( long nLeft )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() - nLeft , mnScrollPos.Y() );
    maVScrollBar.SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}
template< class T>
void ScrollableDialog<T>::SetScrollWidth( long nWidth )
{
    maScrollArea.Width() = nWidth;
    ResetScrollBars();
}

template< class T>
void ScrollableDialog<T>::SetScrollHeight( long nHeight )
{
    maScrollArea.Height() = nHeight;
    ResetScrollBars();
}

template< class T>
void ScrollableDialog<T>::Resize()
{
    ResetScrollBars();
}

template< class T>
void ScrollableDialog<T>::ResetScrollBars()
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

template class ScrollableDialog< Dialog >;
template class ScrollableDialog< GroupBox >;

} // toolkit
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
