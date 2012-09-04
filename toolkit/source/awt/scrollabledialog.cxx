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
#if 0
    sal_uInt16 nPos = (sal_uInt16) pSB->GetThumbPos();
    Rectangle aScrollableArea( 0, 0, maScrollArea.Width(), maScrollArea.Height() );
    Point aScroll;
    if( pSB == &maVScrollBar )
    {
        printf("vertical scroll %d\n", nPos );
        Size aTmpScroll( nPos, nPos );
        long nScroll = mnScrollPos.Y() - aTmpScroll.Width();
        // I'm guessing I need to call scroll for ( stuff ) to happen
        Scroll(0, nScroll, aScrollableArea );
        mnScrollPos.Y() = nPos;
        aScroll.Y() = nScroll;
    }
    else if( pSB == &maHScrollBar )
    {
        printf("horizontal scroll %d\n", nPos );
        Size aTmpScroll( nPos, nPos );
        long nScroll =  mnScrollPos.X() - aTmpScroll.Width();
       Scroll( nScroll, 0, aScrollableArea);
        mnScrollPos.X() = nPos;
        aScroll.X() = nScroll;
    }

    // Manually scroll all children ( except the scrollbars )
    for ( int index = 0; index < GetChildCount(); ++index )
    {
        Window* pChild = GetChild( index );
        if ( pChild && pChild != &maVScrollBar && pChild != &maHScrollBar )
        {
            Point aPos = pChild->GetPosPixel();
            aPos += Point( aScroll.X(), aScroll.Y() );
            pChild->SetPosPixel( aPos );
        }
    }
#endif
    return 1;
}

void ScrollableDialog::SetScrollWidth( long nWidth )
{
    maScrollArea.Width() = nWidth;
}

void ScrollableDialog::SetScrollHeight( long nHeight )
{
    maScrollArea.Height() = nHeight;
}

void  ScrollableDialog::Paint( const Rectangle& rRect )
{
//    printf("ScrollableDialog::Paint( %d, %d, %d, %d width %d height %d\n", rRect.Top(), rRect.Left(), rRect.Right(), rRect.Bottom(),GetSizePixel().Width(),  GetSizePixel().Height() );
    Dialog::Paint( rRect );
}

void ScrollableDialog::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
//    printf("ScrollableDialog::Draw( ( %d, %d ) h %d, w %d \n", rPos.X(), rPos.Y(), rSize.Height(), rSize.Width() );
    Dialog::Draw( pDev, rPos, rSize, nFlags );
}

void ScrollableDialog::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    printf("ScrollableDialog::Resize() - size is width %d height %d\n", GetSizePixel().Width(),  GetSizePixel().Height() );
    // find the output area for the window
    long nMaxX = GetSizePixel().Width();
    long nMaxY = GetSizePixel().Height();
    for ( int index = 0, count = GetChildCount(); index < count; ++index )
    {
        Window* pChild = GetChild( index );
        if ( pChild )
        {
            Point aPos = pChild->GetPosPixel();
            Size aSize = pChild->GetSizePixel();
            long nX = aPos.X() + aSize.Width();
            long nY = aPos.Y() + aSize.Height();
            if ( nX > nMaxX )
                nMaxX = nX;
            if ( nY > nMaxY )
                nMaxY = nY;
            printf("%s child[%d] pos (%d,%d)  height %d, width %d\n", rtl::OUStringToOString( pChild->GetText(), RTL_TEXTENCODING_UTF8 ).getStr(), index, aPos.X(),  aPos.Y(), aSize.Height(), aSize.Width() );
        }
    }

#if 1
    // assume for the moment that we have both hori & vert scroll bars
    Size aContentsSize( aOutSz );
    if ( mbHasVertBar )
    {
        aContentsSize.Width() -= mnScrWidth;
        nMaxX += mnScrWidth;
    }
    if ( mbHasHoriBar )
    {
        aContentsSize.Height() -= mnScrWidth;
        nMaxY += mnScrWidth;
    }
    maScrollArea = aContentsSize;
#endif

    Point aVPos( aOutSz.Width() - mnScrWidth, 0 );
    Point aHPos( 0, aOutSz.Height() - mnScrWidth );

    maVScrollBar.SetPosSizePixel( aVPos, Size( mnScrWidth,  GetSizePixel().Height() - mnScrWidth ) );
    maHScrollBar.SetPosSizePixel( aHPos, Size(  GetSizePixel().Width() - mnScrWidth, mnScrWidth ) );

    printf("nMaxX is %d nMaxY is %d, scrollarea height %d, width %d\n", nMaxX, nMaxY, maScrollArea.Height(),  maScrollArea.Width() );
    printf("Width %d Height %d outsize width %d height %d\n", GetSizePixel().Width(),  GetSizePixel().Height(), aOutSz.Width(), aOutSz.Height() );
    maHScrollBar.SetRangeMax( nMaxX  );
    maHScrollBar.SetVisibleSize( GetSizePixel().Width() );
//    maHScrollBar.SetPageSize( maScrollArea.Height() );

    maVScrollBar.SetRangeMax( nMaxY );
    maVScrollBar.SetVisibleSize( GetSizePixel().Height() );
//    maVScrollBar.SetPageSize( maScrollArea.Width() );
}

} // toolkit
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
