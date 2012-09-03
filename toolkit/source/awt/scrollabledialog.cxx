/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <toolkit/awt/scrollabledialog.hxx>
namespace toolkit
{

ScrollableDialog::ScrollableDialog( Window* pParent, WinBits nStyle ) : Dialog( pParent, nStyle ), maContents( this, nStyle ), maHScrollBar( this, WB_HSCROLL | WB_DRAG), maVScrollBar( this, WB_VSCROLL | WB_DRAG ), mbHasHoriBar( true ), mbHasVertBar( true )
{
    Link aLink( LINK( this, ScrollableDialog, ScrollBarHdl ) );
    maVScrollBar.SetScrollHdl( aLink );
    maHScrollBar.SetScrollHdl( aLink );
    maContents.Show();
    maVScrollBar.Show();
    maHScrollBar.Show();
}

ScrollableDialog::~ScrollableDialog()
{
}

Window* ScrollableDialog::getContentWindow()
{
    return &maContents;
}

IMPL_LINK( ScrollableDialog, ScrollBarHdl, ScrollBar*, pSB )
{
    sal_uInt16 nPos = (sal_uInt16) pSB->GetThumbPos();
    Rectangle aScrollableArea( 0, 0, maContents.GetSizePixel().Width(), maContents.GetSizePixel().Height() );

    if( pSB == &maVScrollBar )
    {
        printf("vertical scroll %d\n", nPos );
        printf("vertical scroll %d\n", nPos );
        long nScroll = mnScrollPos.Y() - nPos;
        maContents.Scroll(0, nScroll, aScrollableArea );
        mnScrollPos.Y() = nPos;
    }
    else if( pSB == &maHScrollBar )
    {
        printf("horizontal scroll %d\n", nPos );
        long nScroll =  mnScrollPos.X() - nPos;
        maContents.Scroll( nScroll, 0, aScrollableArea);
        mnScrollPos.X() = nPos;
    }
    return 1;
}

void ScrollableDialog::Resize()
{
    printf("ScrollableDialog::Resize() - size is width %d height %d\n", GetSizePixel().Width(),  GetSizePixel().Height() );
    maContents.SetSizePixel( GetSizePixel() );
    // find the output area for the window
    long nMaxX = GetSizePixel().Width();
    long nMaxY = GetSizePixel().Height();
    for ( int index = 0, count = maContents.GetChildCount(); index < count; ++index )
    {
        Window* pChild = maContents.GetChild( index );
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
        }
    }

    Size aOutSz = GetOutputSizePixel();
    long nScrWidth = GetSettings().GetStyleSettings().GetScrollBarSize();

    // assume for the moment that we have both hori & vert scroll bars
    Size aContentsSize( aOutSz );
    if ( mbHasVertBar )
    {
        aContentsSize.Width() -= nScrWidth;
        nMaxX += nScrWidth;
    }
    if ( mbHasHoriBar )
    {
        aContentsSize.Height() -= nScrWidth;
        nMaxY += nScrWidth;
    }
    maContents.SetSizePixel( aContentsSize );

    Point aVPos( aOutSz.Width() - nScrWidth, 0 );
    Point aHPos( 0, aOutSz.Height() - nScrWidth );

    maVScrollBar.SetPosSizePixel( aVPos, Size( nScrWidth, aContentsSize.Height() ) );
    maHScrollBar.SetPosSizePixel( aHPos, Size( aContentsSize.Width(), nScrWidth ) );
    maHScrollBar.SetRangeMax( nMaxX );
    maHScrollBar.SetVisibleSize( GetSizePixel().Width() );
    maHScrollBar.SetPageSize( GetSizePixel().Width() );
    maVScrollBar.SetRangeMax( nMaxY );
    maVScrollBar.SetVisibleSize( GetSizePixel().Height() );
    maVScrollBar.SetPageSize( GetSizePixel().Height() );
}

} // toolkit
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
