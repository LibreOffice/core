/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <toolkit/awt/scrollabledialog.hxx>
namespace toolkit
{

ScrollableDialog::ScrollableDialog( Window* pParent, WinBits nStyle ) : Dialog( pParent, nStyle | ~( WB_HSCROLL | WB_VSCROLL ) ), maHScrollBar( this, WB_HSCROLL | WB_DRAG), maVScrollBar( this, WB_VSCROLL | WB_DRAG ), mbHasHoriBar( false ), mbHasVertBar( false ), maScrollVis( None )
{
    Link aLink( LINK( this, ScrollableDialog, ScrollBarHdl ) );
    maVScrollBar.SetScrollHdl( aLink );
    maHScrollBar.SetScrollHdl( aLink );

    Size aOutSz = GetOutputSizePixel();
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
    mnScrWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
}

void ScrollableDialog::setScrollVisibility( ScrollBarVisibility rVisState )
{
    maScrollVis = rVisState;
    if (  maScrollVis == Hori || maScrollVis == Both )
        mbHasHoriBar = true;
    if ( maScrollVis == Vert || maScrollVis == Both )
        mbHasVertBar = true;
    if ( mbHasVertBar )
        maVScrollBar.Show();
    if ( mbHasHoriBar )
        maHScrollBar.Show();
    if ( mbHasHoriBar || mbHasVertBar )
        SetStyle( GetStyle() | WB_CLIPCHILDREN | SCROLL_UPDATE );
}

ScrollableDialog::~ScrollableDialog()
{
}

Window* ScrollableDialog::getContentWindow()
{
    return this;
}

void ScrollableDialog::lcl_Scroll( long nX, long nY )
{
    long nXScroll = mnScrollPos.X() - nX;
    long nYScroll = mnScrollPos.Y() - nY;
    printf( "ScrollableDialog::lcl_Scroll %d, %d nXScroll %d, nYScroll %d\n", nX, nY, nXScroll, nYScroll );
    mnScrollPos = Point( nX, nY );

    Rectangle aScrollableArea( 0, 0, maScrollArea.Width(), maScrollArea.Height() );
    Window::Scroll(nXScroll, nYScroll, aScrollableArea );

    // Manually scroll all children ( except the scrollbars )
    for ( int index = 0; index < GetChildCount(); ++index )
    {
        Window* pChild = GetChild( index );
        if ( pChild && pChild != &maVScrollBar && pChild != &maHScrollBar )
        {
            Point aPos = pChild->GetPosPixel();
            aPos += Point( nXScroll, nYScroll );
            pChild->SetPosPixel( aPos );
        }
    }
}

IMPL_LINK( ScrollableDialog, ScrollBarHdl, ScrollBar*, pSB )
{
    sal_uInt16 nPos = (sal_uInt16) pSB->GetThumbPos();
    if( pSB == &maVScrollBar )
        lcl_Scroll(mnScrollPos.X(), nPos );
    else if( pSB == &maHScrollBar )
        lcl_Scroll(nPos, mnScrollPos.Y() );
    return 1;
}

void ScrollableDialog::SetScrollTop( long nTop )
{
    printf("ScrollableDialog::SetScrollTop(%d)\n", nTop );
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() , nTop );
    printf("about to set thumb\n");
    maHScrollBar.SetThumbPos( 0 );
    mnScrollPos = aOld;
}
void ScrollableDialog::SetScrollLeft( long nLeft )
{
    printf("ScrollableDialog::SetScrollLeft(%d)\n", nLeft );
    Point aOld = mnScrollPos;
    lcl_Scroll( nLeft , mnScrollPos.Y() );
    printf("about to set thumb\n");
    maVScrollBar.SetThumbPos( 0 );
    mnScrollPos = aOld;
}
void ScrollableDialog::SetScrollWidth( long nWidth )
{
    printf("ScrollableDialog::SetScrollWidth(%d)\n", nWidth );
    maScrollArea.Width() = nWidth;
    ResetScrollBars();
}

void ScrollableDialog::SetScrollHeight( long nHeight )
{
    printf("ScrollableDialog::SetScrollHeight(%d)\n", nHeight );
    maScrollArea.Height() = nHeight;
    ResetScrollBars();
}

void ScrollableDialog::Resize()
{
    ResetScrollBars();
}

void ScrollableDialog::ResetScrollBars()
{
    Size aOutSz = GetOutputSizePixel();
    printf("ScrollableDialog::ResetScrollbars() - size is width %d height %d\n", GetSizePixel().Width(),  GetSizePixel().Height() );

    Point aVPos( aOutSz.Width() - mnScrWidth, 0 );
    Point aHPos( 0, aOutSz.Height() - mnScrWidth );

    maVScrollBar.SetPosSizePixel( aVPos, Size( mnScrWidth,  GetSizePixel().Height() - mnScrWidth ) );
    maHScrollBar.SetPosSizePixel( aHPos, Size(  GetSizePixel().Width() - mnScrWidth, mnScrWidth ) );

    maHScrollBar.SetRangeMax( maScrollArea.Width() + mnScrWidth  );
    maHScrollBar.SetVisibleSize( GetSizePixel().Width() );
//    maHScrollBar.SetPageSize( maScrollArea.Height() );

    maVScrollBar.SetRangeMax( maScrollArea.Height() + mnScrWidth );
    maVScrollBar.SetVisibleSize( GetSizePixel().Height() );
//    maVScrollBar.SetPageSize( maScrollArea.Width() );
}

} // toolkit
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
