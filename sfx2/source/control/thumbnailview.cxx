/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/thumbnailview.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <sfx2/doctempl.hxx>

#include "orgmgr.hxx"
#include "thumbnailviewacc.hxx"

#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/help.hxx>
#include <vcl/pngread.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>


enum
{
    ITEM_OFFSET = 4,
    ITEM_OFFSET_DOUBLE = 6,
    NAME_LINE_OFF_X = 2,
    NAME_LINE_OFF_Y = 2,
    NAME_LINE_HEIGHT = 2,
    NAME_OFFSET = 2,
    SCRBAR_OFFSET = 1,
    SCROLL_OFFSET = 4
};

Image lcl_fetchThumbnail (const rtl::OUString &msURL, int width, int height)
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;

    // Load the thumbnail from a template document.
    uno::Reference<io::XInputStream> xIStream;

    uno::Reference< lang::XMultiServiceFactory > xServiceManager (
        ::comphelper::getProcessServiceFactory());
    if (xServiceManager.is())
    {
        try
        {
            uno::Reference<lang::XSingleServiceFactory> xStorageFactory(
                xServiceManager->createInstance( "com.sun.star.embed.StorageFactory"),
                uno::UNO_QUERY);

            if (xStorageFactory.is())
            {
                uno::Sequence<uno::Any> aArgs (2);
                aArgs[0] <<= msURL;
                aArgs[1] <<= embed::ElementModes::READ;
                uno::Reference<embed::XStorage> xDocStorage (
                    xStorageFactory->createInstanceWithArguments(aArgs),
                    uno::UNO_QUERY);

                try
                {
                    if (xDocStorage.is())
                    {
                        uno::Reference<embed::XStorage> xStorage (
                            xDocStorage->openStorageElement(
                                "Thumbnails",
                                embed::ElementModes::READ));
                        if (xStorage.is())
                        {
                            uno::Reference<io::XStream> xThumbnailCopy (
                                xStorage->cloneStreamElement("thumbnail.png"));
                            if (xThumbnailCopy.is())
                                xIStream = xThumbnailCopy->getInputStream();
                        }
                    }
                }
                catch (const uno::Exception& rException)
                {
                    OSL_TRACE (
                        "caught exception while trying to access Thumbnail/thumbnail.png of %s: %s",
                        ::rtl::OUStringToOString(msURL,
                            RTL_TEXTENCODING_UTF8).getStr(),
                        ::rtl::OUStringToOString(rException.Message,
                            RTL_TEXTENCODING_UTF8).getStr());
                }

                try
                {
                    // An (older) implementation had a bug - The storage
                    // name was "Thumbnail" instead of "Thumbnails".  The
                    // old name is still used as fallback but this code can
                    // be removed soon.
                    if ( ! xIStream.is())
                    {
                        uno::Reference<embed::XStorage> xStorage (
                            xDocStorage->openStorageElement( "Thumbnail",
                                embed::ElementModes::READ));
                        if (xStorage.is())
                        {
                            uno::Reference<io::XStream> xThumbnailCopy (
                                xStorage->cloneStreamElement("thumbnail.png"));
                            if (xThumbnailCopy.is())
                                xIStream = xThumbnailCopy->getInputStream();
                        }
                    }
                }
                catch (const uno::Exception& rException)
                {
                    OSL_TRACE (
                        "caught exception while trying to access Thumbnails/thumbnail.png of %s: %s",
                        ::rtl::OUStringToOString(msURL,
                            RTL_TEXTENCODING_UTF8).getStr(),
                        ::rtl::OUStringToOString(rException.Message,
                            RTL_TEXTENCODING_UTF8).getStr());
                }
            }
        }
        catch (const uno::Exception& rException)
        {
            OSL_TRACE (
                "caught exception while trying to access tuhmbnail of %s: %s",
                ::rtl::OUStringToOString(msURL,
                    RTL_TEXTENCODING_UTF8).getStr(),
                ::rtl::OUStringToOString(rException.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
        }
    }

    // Extract the image from the stream.
    BitmapEx aThumbnail;
    if (xIStream.is())
    {
        ::std::auto_ptr<SvStream> pStream (
            ::utl::UcbStreamHelper::CreateStream (xIStream));
        ::vcl::PNGReader aReader (*pStream);
        aThumbnail = aReader.Read ();
    }

    aThumbnail.Scale(Size(width,height));

    // Note that the preview is returned without scaling it to the desired
    // width.  This gives the caller the chance to take advantage of a
    // possibly larger resolution then was asked for.
    return aThumbnail;
}

ThumbnailView::ThumbnailView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : Control( pParent, nWinStyle ),
    maVirDev( *this ),
    maColor( COL_TRANSPARENT )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

ThumbnailView::ThumbnailView (Window *pParent, const ResId &rResId, bool bDisableTransientChildren)
    : Control( pParent, rResId ),
    maVirDev( *this ),
    maColor( COL_TRANSPARENT )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

ThumbnailView::~ThumbnailView()
{
    com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>
            xComponent(GetAccessible(sal_False),
                       com::sun::star::uno::UNO_QUERY);

    if (xComponent.is())
        xComponent->dispose ();

    delete mpMgr;
    delete mpScrBar;

    ImplDeleteItems();
}

void ThumbnailView::ImplInit()
{
    mpMgr = new SfxOrganizeMgr(NULL,NULL);
    mpScrBar            = NULL;
    mnItemWidth         = 0;
    mnItemHeight        = 0;
    mnVisLines          = 0;
    mnLines             = 0;
    mnUserItemWidth     = 0;
    mnUserItemHeight    = 0;
    mnFirstLine         = 0;
    mnSelItemId         = 0;
    mnHighItemId        = 0;
    mnCols              = 0;
    mnCurCol            = 0;
    mnUserCols          = 0;
    mnUserVisLines      = 0;
    mnSpacing           = 0;
    mnFrameStyle        = 0;
    mbFormat            = true;
    mbHighlight         = false;
    mbSelection         = false;
    mbNoSelection       = true;
    mbDrawSelection     = true;
    mbBlackSel          = false;
    mbDoubleSel         = false;
    mbScroll            = false;
    mbFullMode          = true;
    mbHasVisibleItems   = false;

    // #106446#, #106601# force mirroring of virtual device
    maVirDev.EnableRTL( GetParent()->IsRTLEnabled() );

    ImplInitSettings( true, true, true );
}

void ThumbnailView::ImplDeleteItems()
{
    const size_t n = mItemList.size();

    for ( size_t i = 0; i < n; ++i )
    {
        ThumbnailViewItem *const pItem = mItemList[i];
        if ( pItem->mbVisible && ImplHasAccessibleListeners() )
        {
            ::com::sun::star::uno::Any aOldAny, aNewAny;

            aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
        }

        delete pItem;
    }

    mItemList.clear();
}

void ThumbnailView::ImplInitSettings( bool bFont, bool bForeground, bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else
            aColor = rStyleSettings.GetButtonTextColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else
            aColor = rStyleSettings.GetFaceColor();
        SetBackground( aColor );
    }
}

void ThumbnailView::ImplInitScrollBar()
{
    if ( GetStyle() & WB_VSCROLL )
    {
        if ( !mpScrBar )
        {
            mpScrBar = new ScrollBar( this, WB_VSCROLL | WB_DRAG );
            mpScrBar->SetScrollHdl( LINK( this, ThumbnailView, ImplScrollHdl ) );
        }
        else
        {
            // adapt the width because of the changed settings
            long nScrBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            mpScrBar->SetPosSizePixel( 0, 0, nScrBarWidth, 0, WINDOW_POSSIZE_WIDTH );
        }
    }
}

void ThumbnailView::DrawItem (ThumbnailViewItem *pItem, const Rectangle &aRect)
{
    if ( (aRect.GetHeight() > 0) && (aRect.GetWidth() > 0) )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        if ( IsColor() )
            maVirDev.SetFillColor( maColor );
        else if ( IsEnabled() )
            maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
        else
            maVirDev.SetFillColor( rStyleSettings.GetFaceColor() );

        maVirDev.DrawRect( aRect );

        // Draw thumbnail
        Size    aImageSize = pItem->maImage.GetSizePixel();
        Size    aRectSize = aRect.GetSize();
        Point   aPos( aRect.Left(), aRect.Top() );
        aPos.X() += (aRectSize.Width()-aImageSize.Width())/2;
        aPos.Y() += (aRectSize.Height()-aImageSize.Height())/2;

        sal_uInt16  nImageStyle  = 0;
        if( !IsEnabled() )
            nImageStyle  |= IMAGE_DRAW_DISABLE;

        if ( (aImageSize.Width()  > aRectSize.Width()) ||
             (aImageSize.Height() > aRectSize.Height()) )
        {
            maVirDev.SetClipRegion( Region( aRect ) );
            maVirDev.DrawImage( aPos, pItem->maImage, nImageStyle);
            maVirDev.SetClipRegion();
        }
        else
            maVirDev.DrawImage( aPos, pItem->maImage, nImageStyle );

        // Draw centered text below thumbnail
        aPos.Y() += 5 + aImageSize.Height();
        aPos.X() = aRect.Left() + (aRectSize.Width() - maVirDev.GetTextWidth(pItem->maText))/2;

        maVirDev.DrawText(aPos,pItem->maText);
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ThumbnailView::CreateAccessible()
{
    return new ThumbnailViewAcc( this, mbIsTransientChildrenDisabled );
}

void ThumbnailView::Format()
{
    Size        aWinSize = GetOutputSizePixel();
    size_t      nItemCount = mItemList.size();
    WinBits     nStyle = GetStyle();
    long        nNoneHeight = 0;
    long        nNoneSpace = 0;
    ScrollBar*  pDelScrBar = NULL;

    // consider the scrolling
    if ( nStyle & WB_VSCROLL )
        ImplInitScrollBar();
    else
    {
        if ( mpScrBar )
        {
            // delete ScrollBar not until later, to prevent recursive calls
            pDelScrBar = mpScrBar;
            mpScrBar = NULL;
        }
    }

    // calculate ScrollBar width
    long nScrBarWidth = 0;
    if ( mpScrBar )
        nScrBarWidth = mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;

    // calculate number of columns
    if ( !mnUserCols )
    {
        if ( mnUserItemWidth )
        {
            mnCols = (sal_uInt16)((aWinSize.Width()-nScrBarWidth+mnSpacing) / (mnUserItemWidth+mnSpacing));
            if ( !mnCols )
                mnCols = 1;
        }
        else
            mnCols = 1;
    }
    else
        mnCols = mnUserCols;

    // calculate number of rows
    mbScroll = false;
    // Floor( (M+N-1)/N )==Ceiling( M/N )
    mnLines = (static_cast<long>(nItemCount)+mnCols-1) / mnCols;
    if ( !mnLines )
        mnLines = 1;

    long nCalcHeight = aWinSize.Height()-nNoneHeight;
    if ( mnUserVisLines )
        mnVisLines = mnUserVisLines;
    else if ( mnUserItemHeight )
    {
        mnVisLines = (nCalcHeight-nNoneSpace+mnSpacing) / (mnUserItemHeight+mnSpacing);
        if ( !mnVisLines )
            mnVisLines = 1;
    }
    else
        mnVisLines = mnLines;
    if ( mnLines > mnVisLines )
        mbScroll = true;
    if ( mnLines <= mnVisLines )
        mnFirstLine = 0;
    else
    {
        if ( mnFirstLine > (sal_uInt16)(mnLines-mnVisLines) )
            mnFirstLine = (sal_uInt16)(mnLines-mnVisLines);
    }

    // calculate item size
    const long nColSpace  = (mnCols-1)*mnSpacing;
    const long nLineSpace = ((mnVisLines-1)*mnSpacing)+nNoneSpace;
    if ( mnUserItemWidth && !mnUserCols )
    {
        mnItemWidth = mnUserItemWidth;
        if ( mnItemWidth > aWinSize.Width()-nScrBarWidth-nColSpace )
            mnItemWidth = aWinSize.Width()-nScrBarWidth-nColSpace;
    }
    else
        mnItemWidth = (aWinSize.Width()-nScrBarWidth-nColSpace) / mnCols;
    if ( mnUserItemHeight && !mnUserVisLines )
    {
        mnItemHeight = mnUserItemHeight;
        if ( mnItemHeight > nCalcHeight-nNoneSpace )
            mnItemHeight = nCalcHeight-nNoneSpace;
    }
    else
    {
        nCalcHeight -= nLineSpace;
        mnItemHeight = nCalcHeight / mnVisLines;
    }

    // Init VirDev
    maVirDev.SetSettings( GetSettings() );
    maVirDev.SetBackground( GetBackground() );
    maVirDev.SetOutputSizePixel( aWinSize, sal_True );

    // nothing is changed in case of too small items
    if ( (mnItemWidth <= 0) ||
         (mnItemHeight <= 2) ||
         !nItemCount )
    {
        mbHasVisibleItems = false;

        for ( size_t i = 0; i < nItemCount; i++ )
        {
            mItemList[i]->mbVisible = false;
        }

        if ( mpScrBar )
            mpScrBar->Hide();
    }
    else
    {
        mbHasVisibleItems = true;

        // determine Frame-Style
        mnFrameStyle = FRAME_DRAW_IN;

        // determine selected color and width
        // if necessary change the colors, to make the selection
        // better detectable
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Color aHighColor( rStyleSettings.GetHighlightColor() );
        if ( ((aHighColor.GetRed() > 0x80) || (aHighColor.GetGreen() > 0x80) ||
              (aHighColor.GetBlue() > 0x80)) ||
             ((aHighColor.GetRed() == 0x80) && (aHighColor.GetGreen() == 0x80) &&
              (aHighColor.GetBlue() == 0x80)) )
            mbBlackSel = true;
        else
            mbBlackSel = false;

        // draw the selection with double width if the items are bigger
        mbDoubleSel = false;

        // calculate offsets
        long nStartX;
        long nStartY;
        if ( mbFullMode )
        {
            long nAllItemWidth = (mnItemWidth*mnCols)+nColSpace;
            long nAllItemHeight = (mnItemHeight*mnVisLines)+nNoneHeight+nLineSpace;
            nStartX = (aWinSize.Width()-nScrBarWidth-nAllItemWidth)/2;
            nStartY = (aWinSize.Height()-nAllItemHeight)/2;
        }
        else
        {
            nStartX = 0;
            nStartY = 0;
        }

        // calculate and draw items
        maVirDev.SetLineColor();
        long x = nStartX;
        long y = nStartY;

        // draw items
        sal_uLong nFirstItem = mnFirstLine * mnCols;
        sal_uLong nLastItem = nFirstItem + (mnVisLines * mnCols);

        maItemListRect.Left() = x;
        maItemListRect.Top() = y;
        maItemListRect.Right() = x + mnCols*(mnItemWidth+mnSpacing) - mnSpacing - 1;
        maItemListRect.Bottom() = y + mnVisLines*(mnItemHeight+mnSpacing) - mnSpacing - 1;

        if ( !mbFullMode )
        {
            // If want also draw parts of items in the last line,
            // then we add one more line if parts of these line are
            // visible
            if ( y+(mnVisLines*(mnItemHeight+mnSpacing)) < aWinSize.Height() )
                nLastItem += mnCols;
            maItemListRect.Bottom() = aWinSize.Height() - y;
        }
        for ( size_t i = 0; i < nItemCount; i++ )
        {
            ThumbnailViewItem *const pItem = mItemList[i];

            if ( (i >= nFirstItem) && (i < nLastItem) )
            {
                if( !pItem->mbVisible && ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aNewAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->mbVisible = true;
                DrawItem( pItem, Rectangle( Point(x,y), Size(mnItemWidth, mnItemHeight) ) );

                if ( !((i+1) % mnCols) )
                {
                    x = nStartX;
                    y += mnItemHeight+mnSpacing;
                }
                else
                    x += mnItemWidth+mnSpacing;
            }
            else
            {
                if( pItem->mbVisible && ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->mbVisible = false;
            }
        }

        // arrange ScrollBar, set values and show it
        if ( mpScrBar )
        {
            Point   aPos( aWinSize.Width()-nScrBarWidth+SCRBAR_OFFSET, 0 );
            Size    aSize( nScrBarWidth-SCRBAR_OFFSET, aWinSize.Height() );

            mpScrBar->SetPosSizePixel( aPos, aSize );
            mpScrBar->SetRangeMax( mnLines );
            mpScrBar->SetVisibleSize( mnVisLines );
            mpScrBar->SetThumbPos( (long)mnFirstLine );
            long nPageSize = mnVisLines;
            if ( nPageSize < 1 )
                nPageSize = 1;
            mpScrBar->SetPageSize( nPageSize );
            mpScrBar->Show();
        }
    }

    // waiting for the next since the formatting is finished
    mbFormat = false;

    // delete ScrollBar
    delete pDelScrBar;
}

void ThumbnailView::ImplDrawSelect()
{
    if ( !IsReallyVisible() )
        return;

    const bool bFocus = HasFocus();
    const bool bDrawSel = !( (mbNoSelection && !mbHighlight) || (!mbDrawSelection && mbHighlight) );

    if ( !bFocus && !bDrawSel )
        return;

    DrawSelectedItem( mnSelItemId, bFocus, bDrawSel );
    if (mbHighlight)
    {
        DrawSelectedItem( mnHighItemId, bFocus, bDrawSel );
    }
}

void ThumbnailView::DrawSelectedItem( const sal_uInt16 nItemId, const bool bFocus, const bool bDrawSel )
{
    ThumbnailViewItem* pItem;
    Rectangle aRect;
    if ( nItemId )
    {
        const size_t nPos = GetItemPos( nItemId );
        pItem = mItemList[ nPos ];
        aRect = ImplGetItemRect( nPos );
    }
    else if ( bFocus && (pItem = ImplGetFirstItem()) )
    {
        aRect = ImplGetItemRect( 0 );
    }
    else
    {
        return;
    }

    if ( pItem->mbVisible )
    {
        // draw selection
        const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
        Control::SetFillColor();

        Color aDoubleColor( rStyleSettings.GetHighlightColor() );
        Color aSingleColor( rStyleSettings.GetHighlightTextColor() );
        if( !mbDoubleSel )
        {
            /*
            *  #99777# contrast enhancement for thin mode
            */
            const Wallpaper& rWall = GetDisplayBackground();
            if( ! rWall.IsBitmap() && ! rWall.IsGradient() )
            {
                const Color& rBack = rWall.GetColor();
                if( rBack.IsDark() && ! aDoubleColor.IsBright() )
                {
                    aDoubleColor = Color( COL_WHITE );
                    aSingleColor = Color( COL_BLACK );
                }
                else if( rBack.IsBright() && ! aDoubleColor.IsDark() )
                {
                    aDoubleColor = Color( COL_BLACK );
                    aSingleColor = Color( COL_WHITE );
                }
            }
        }

        // specify selection output
        aRect.Left()    += 4;
        aRect.Top()     += 4;
        aRect.Right()   -= 4;
        aRect.Bottom()  -= 4;

        if ( bDrawSel )
        {
            SetLineColor( aDoubleColor );
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            DrawRect( aRect );
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            DrawRect( aRect );
        }
    }
}

void ThumbnailView::ImplHideSelect( sal_uInt16 nItemId )
{
    Rectangle aRect;

    const size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos != THUMBNAILVIEW_ITEM_NOTFOUND && mItemList[nItemPos]->mbVisible )
    {
        aRect = ImplGetItemRect(nItemPos);

        HideFocus();
        const Point aPos  = aRect.TopLeft();
        const Size  aSize = aRect.GetSize();
        DrawOutDev( aPos, aSize, aPos, aSize, maVirDev );
    }
}

void ThumbnailView::ImplHighlightItem( sal_uInt16 nItemId, bool bIsSelection )
{
    if ( mnHighItemId != nItemId )
    {
        // remember the old item to delete the previous selection
        sal_uInt16 nOldItem = mnHighItemId;
        mnHighItemId = nItemId;

        // don't draw the selection if nothing is selected
        if ( !bIsSelection && mbNoSelection )
            mbDrawSelection = false;

        // remove the old selection and draw the new one
        ImplHideSelect( nOldItem );
        ImplDrawSelect();
        mbDrawSelection = true;
    }
}

void ThumbnailView::ImplDraw()
{
    if ( mbFormat )
        Format();

    HideFocus();

    Point   aDefPos;
    Size    aSize = maVirDev.GetOutputSizePixel();

    if ( mpScrBar && mpScrBar->IsVisible() )
    {
        Point   aScrPos = mpScrBar->GetPosPixel();
        Size    aScrSize = mpScrBar->GetSizePixel();
        Point   aTempPos( 0, aScrPos.Y() );
        Size    aTempSize( aSize.Width(), aScrPos.Y() );

        DrawOutDev( aDefPos, aTempSize, aDefPos, aTempSize, maVirDev );
        aTempSize.Width()   = aScrPos.X()-1;
        aTempSize.Height()  = aScrSize.Height();
        DrawOutDev( aTempPos, aTempSize, aTempPos, aTempSize, maVirDev );
        aTempPos.Y()        = aScrPos.Y()+aScrSize.Height();
        aTempSize.Width()   = aSize.Width();
        aTempSize.Height()  = aSize.Height()-aTempPos.Y();
        DrawOutDev( aTempPos, aTempSize, aTempPos, aTempSize, maVirDev );
    }
    else
        DrawOutDev( aDefPos, aSize, aDefPos, aSize, maVirDev );

    ImplDrawSelect();
}

bool ThumbnailView::ImplScroll( const Point& rPos )
{
    if ( !mbScroll || !maItemListRect.IsInside(rPos) )
        return false;

    const long nScrollOffset = (mnItemHeight <= 16) ? SCROLL_OFFSET/2 : SCROLL_OFFSET;
    bool bScroll = false;

    if ( rPos.Y() <= maItemListRect.Top()+nScrollOffset )
    {
        if ( mnFirstLine > 0 )
        {
            --mnFirstLine;
            bScroll = true;
        }
    }
    else if ( rPos.Y() >= maItemListRect.Bottom()-nScrollOffset )
    {
        if ( mnFirstLine < static_cast<sal_uInt16>(mnLines-mnVisLines) )
        {
            ++mnFirstLine;
            bScroll = true;
        }
    }

    if ( !bScroll )
        return false;

    mbFormat = true;
    ImplDraw();
    return true;
}

size_t ThumbnailView::ImplGetItem( const Point& rPos, bool bMove ) const
{
    if ( !mbHasVisibleItems )
    {
        return THUMBNAILVIEW_ITEM_NOTFOUND;
    }

    if ( maItemListRect.IsInside( rPos ) )
    {
        const int xc = rPos.X()-maItemListRect.Left();
        const int yc = rPos.Y()-maItemListRect.Top();
        // The point is inside the area of item list,
        // let's find the containing item.
        const int col = xc/(mnItemWidth+mnSpacing);
        const int x = xc%(mnItemWidth+mnSpacing);
        const int row = yc/(mnItemHeight+mnSpacing);
        const int y = yc%(mnItemHeight+mnSpacing);

        if (x<mnItemWidth && y<mnItemHeight)
        {
            // the point is inside item rect and not inside spacing
            const size_t item = (mnFirstLine+row)*mnCols+col;
            if (item < mItemList.size())
            {
                return item;
            }
        }

        // return the previously selected item if spacing is set and
        // the mouse hasn't left the window yet
        if ( bMove && mnSpacing && mnHighItemId )
        {
            return GetItemPos( mnHighItemId );
        }
    }

    return THUMBNAILVIEW_ITEM_NOTFOUND;
}

ThumbnailViewItem* ThumbnailView::ImplGetItem( size_t nPos )
{
    return ( nPos < mItemList.size() ) ? mItemList[nPos] : NULL;
}

ThumbnailViewItem* ThumbnailView::ImplGetFirstItem()
{
    return mItemList.empty() ? NULL : mItemList[0];
}

sal_uInt16 ThumbnailView::ImplGetVisibleItemCount() const
{
    sal_uInt16 nRet = 0;
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        if ( mItemList[n]->mbVisible )
            ++nRet;
    }

    return nRet;
}

ThumbnailViewItem* ThumbnailView::ImplGetVisibleItem( sal_uInt16 nVisiblePos )
{
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        ThumbnailViewItem *const pItem = mItemList[n];

        if ( pItem->mbVisible && !nVisiblePos-- )
            return pItem;
    }

    return NULL;
}

void ThumbnailView::ImplFireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue )
{
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

bool ThumbnailView::ImplHasAccessibleListeners()
{
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );
    return( pAcc && pAcc->HasAccessibleListeners() );
}

IMPL_LINK( ThumbnailView,ImplScrollHdl, ScrollBar*, pScrollBar )
{
    sal_uInt16 nNewFirstLine = (sal_uInt16)pScrollBar->GetThumbPos();
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;
        mbFormat = true;
        ImplDraw();
    }
    return 0;
}

IMPL_LINK_NOARG(ThumbnailView, ImplTimerHdl)
{
    ImplTracking( GetPointerPosPixel(), true );
    return 0;
}

void ThumbnailView::ImplTracking( const Point& rPos, bool bRepeat )
{
    if ( bRepeat || mbSelection )
    {
        if ( ImplScroll( rPos ) )
        {
            if ( mbSelection )
            {
                maTimer.SetTimeoutHdl( LINK( this, ThumbnailView, ImplTimerHdl ) );
                maTimer.SetTimeout( GetSettings().GetMouseSettings().GetScrollRepeat() );
                maTimer.Start();
            }
        }
    }

    ThumbnailViewItem* pItem = ImplGetItem( ImplGetItem( rPos ) );
    if ( pItem )
    {
        ImplHighlightItem( pItem->mnId );
    }
    else
    {
        ImplHighlightItem( mnSelItemId, false );
    }
}

void ThumbnailView::ImplEndTracking( const Point& rPos, bool bCancel )
{
    ThumbnailViewItem* pItem;

    // restore the old status in case of termination
    if ( bCancel )
        pItem = NULL;
    else
        pItem = ImplGetItem( ImplGetItem( rPos ) );

    if ( pItem )
    {
        SelectItem( pItem->mnId );
        if ( !mbSelection )
            GrabFocus();
        mbHighlight = false;
        mbSelection = false;
        Select();
    }
    else
    {
        ImplHighlightItem( mnSelItemId, false );
        mbHighlight = false;
        mbSelection = false;
    }
}

void ThumbnailView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        ThumbnailViewItem* pItem = ImplGetItem( ImplGetItem( rMEvt.GetPosPixel() ) );
        if ( mbSelection )
        {
            mbHighlight = true;
            if ( pItem )
            {
                mnHighItemId = mnSelItemId;
                ImplHighlightItem( pItem->mnId );
            }

            return;
        }
        else
        {
            if ( pItem && !rMEvt.IsMod2() )
            {
                if ( rMEvt.GetClicks() == 1 )
                {
                    mbHighlight  = true;
                    mnHighItemId = mnSelItemId;
                    ImplHighlightItem( pItem->mnId );
                    StartTracking( STARTTRACK_SCROLLREPEAT );
                }
                else if ( rMEvt.GetClicks() == 2 )
                    DoubleClick();

                return;
            }
        }
    }

    Control::MouseButtonDown( rMEvt );
}

void ThumbnailView::MouseButtonUp( const MouseEvent& rMEvt )
{
    // because of SelectionMode
    if ( rMEvt.IsLeft() && mbSelection )
        ImplEndTracking( rMEvt.GetPosPixel(), false );
    else
        Control::MouseButtonUp( rMEvt );
}

void ThumbnailView::MouseMove( const MouseEvent& rMEvt )
{
    // because of SelectionMode
    if ( mbSelection )
        ImplTracking( rMEvt.GetPosPixel(), false );
    Control::MouseMove( rMEvt );
}

void ThumbnailView::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

    if ( rTEvt.IsTrackingEnded() )
        ImplEndTracking( aMousePos, rTEvt.IsTrackingCanceled() );
    else
        ImplTracking( aMousePos, rTEvt.IsTrackingRepeat() );
}

void ThumbnailView::KeyInput( const KeyEvent& rKEvt )
{
    size_t nLastItem = mItemList.size();

    if ( !nLastItem || !ImplGetFirstItem() )
    {
        Control::KeyInput( rKEvt );
        return;
    }

    --nLastItem;
    const size_t nCurPos = mnSelItemId ? GetItemPos( mnSelItemId ) : 0;
    size_t nItemPos = THUMBNAILVIEW_ITEM_NOTFOUND;
    size_t nVStep = mnCols;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_HOME:
            nItemPos = 0;
            break;

        case KEY_END:
            nItemPos = nLastItem;
            break;

        case KEY_LEFT:
            if (nCurPos != THUMBNAILVIEW_ITEM_NONEITEM)
            {
                if (nCurPos)
                {
                    nItemPos = nCurPos-1;
                }
            }
            break;

        case KEY_RIGHT:
            if (nCurPos < nLastItem)
                nItemPos = nCurPos+1;

            break;

        case KEY_PAGEUP:
            if (rKEvt.GetKeyCode().IsShift() || rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                Control::KeyInput( rKEvt );
                return;
            }
            nVStep *= mnVisLines;
            // intentional fall-through
        case KEY_UP:
            if (nCurPos != THUMBNAILVIEW_ITEM_NONEITEM)
            {
                if (nCurPos == nLastItem)
                {
                    const size_t nCol = nLastItem % mnCols;
                    if (nCol < mnCurCol)
                    {
                        // Move to previous row/page, keeping the old column
                        nVStep -= mnCurCol - nCol;
                    }
                }
                if (nCurPos >= nVStep)
                {
                    // Go up of a whole page
                    nItemPos = nCurPos-nVStep;
                }
                else if (nCurPos > mnCols)
                {
                    // Go to same column in first row
                    nItemPos = nCurPos % mnCols;
                }
            }
            break;

        case KEY_PAGEDOWN:
            if (rKEvt.GetKeyCode().IsShift() || rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                Control::KeyInput( rKEvt );
                return;
            }
            nVStep *= mnVisLines;
            // intentional fall-through
        case KEY_DOWN:
            if (nCurPos != nLastItem)
            {
                if (nCurPos == THUMBNAILVIEW_ITEM_NONEITEM)
                {
                    nItemPos = nVStep-mnCols+mnCurCol;
                }
                else
                {
                    nItemPos = nCurPos+nVStep;
                }
                if (nItemPos > nLastItem)
                {
                    nItemPos = nLastItem;
                }
            }
            break;
            // intentional fall-through
        default:
            Control::KeyInput( rKEvt );
            return;
    }

    // This point is reached only if key travelling was used,
    // in which case selection mode should be switched off
    EndSelection();

    if ( nItemPos != THUMBNAILVIEW_ITEM_NOTFOUND )
    {
        if ( nItemPos!=THUMBNAILVIEW_ITEM_NONEITEM && nItemPos<nLastItem )
        {
            // update current column only in case of a new position
            // which is also not a "specially" handled one.
            mnCurCol = nItemPos % mnCols;
        }
        const sal_uInt16 nItemId = (nItemPos != THUMBNAILVIEW_ITEM_NONEITEM) ? GetItemId( nItemPos ) : 0;
        if ( nItemId != mnSelItemId )
        {
            SelectItem( nItemId );
            Select();
        }
    }
}

void ThumbnailView::Command( const CommandEvent& rCEvt )
{
    if ( (rCEvt.GetCommand() == COMMAND_WHEEL) ||
         (rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL) ||
         (rCEvt.GetCommand() == COMMAND_AUTOSCROLL) )
    {
        if ( HandleScrollCommand( rCEvt, NULL, mpScrBar ) )
            return;
    }

    Control::Command( rCEvt );
}

void ThumbnailView::Paint( const Rectangle& )
{
    ImplDraw();
}

void ThumbnailView::GetFocus()
{
    ImplDrawSelect();
    Control::GetFocus();

    // Tell the accessible object that we got the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->GetFocus();
}

void ThumbnailView::LoseFocus()
{
    if ( mbNoSelection && mnSelItemId )
        ImplHideSelect( mnSelItemId );
    else
        HideFocus();
    Control::LoseFocus();

    // Tell the accessible object that we lost the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->LoseFocus();
}

void ThumbnailView::Resize()
{
    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    Control::Resize();
}

void ThumbnailView::RequestHelp( const HelpEvent& rHEvt )
{
    if ( (rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON)) == HELPMODE_QUICK )
    {
        Point aPos = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        size_t nItemPos = ImplGetItem( aPos );
        if ( nItemPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        {
            Rectangle aItemRect = ImplGetItemRect( nItemPos );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();
            Help::ShowQuickHelp( this, aItemRect, GetItemText( ImplGetItem( nItemPos )->mnId ) );
            return;
        }
    }

    Control::RequestHelp( rHEvt );
}

void ThumbnailView::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( mbFormat )
            Format();
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_TEXT )
    {
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( true, false, false );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( false, true, false );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( false, false, true );
        Invalidate();
    }
    else if ( (nType == STATE_CHANGE_STYLE) || (nType == STATE_CHANGE_ENABLE) )
    {
        mbFormat = true;
        ImplInitSettings( false, false, true );
        Invalidate();
    }
}

void ThumbnailView::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        mbFormat = true;
        ImplInitSettings( true, true, true );
        Invalidate();
    }
}

void ThumbnailView::Select()
{
    maSelectHdl.Call( this );
}

void ThumbnailView::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

void ThumbnailView::Populate ()
{
    const SfxDocumentTemplates* pTemplates = mpMgr->GetTemplates();

    sal_uInt16 nCount = pTemplates->GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        rtl::OUString aRegionName(pTemplates->GetFullRegionName(i));

        sal_uInt16 nEntries = pTemplates->GetCount(i);

        if (nEntries)
        {
            /// Preview first 2 thumbnails for folder
            Image aImg = lcl_fetchThumbnail(pTemplates->GetPath(i,0),128,128);

            if ( nEntries > 2 )
            {
                Color aWhite(COL_WHITE);
                BitmapEx aBmp = lcl_fetchThumbnail(pTemplates->GetPath(i,1),128,128).GetBitmapEx();
                BitmapEx aResult = aBmp;

                aResult.Erase(aWhite);
                aBmp.Scale(Size(128,128));
                aResult.CopyPixel(Rectangle(32,4,128,100),Rectangle(0,0,96,96),&aBmp);

                aBmp = aImg.GetBitmapEx();
                aBmp.Scale(Size(96,96));
                aResult.CopyPixel(Rectangle(10,22,106,118), Rectangle(0,0,96,96),&aBmp);

                aImg = aResult;

            }

            InsertItem(i+1,aImg,aRegionName);
        }
    }
}

void ThumbnailView::InsertItem( sal_uInt16 nItemId, const Image& rImage,
                           const rtl::OUString& rText, size_t nPos )
{
    ThumbnailViewItem* pItem = new ThumbnailViewItem( *this );
    pItem->mnId     = nItemId;
    pItem->maImage  = rImage;
    pItem->maText   = rText;
    ImplInsertItem( pItem, nPos );
}

void ThumbnailView::ImplInsertItem( ThumbnailViewItem *const pItem, const size_t nPos )
{
    assert(pItem->mnId); // "ItemId == 0"
    assert(GetItemPos( pItem->mnId ) == THUMBNAILVIEW_ITEM_NOTFOUND); // ItemId already exists

    if ( nPos < mItemList.size() ) {
        ValueItemList::iterator it = mItemList.begin();
        ::std::advance( it, nPos );
        mItemList.insert( it, pItem );
    } else {
        mItemList.push_back( pItem );
    }

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

Rectangle ThumbnailView::ImplGetItemRect( size_t nPos ) const
{
    const size_t nVisibleBegin = static_cast<size_t>(mnFirstLine)*mnCols;
    const size_t nVisibleEnd = nVisibleBegin + static_cast<size_t>(mnVisLines)*mnCols;

    // Check if the item is inside the range of the displayed ones,
    // taking into account that last row could be incomplete
    if ( nPos<nVisibleBegin || nPos>=nVisibleEnd || nPos>=mItemList.size() )
        return Rectangle();

    nPos -= nVisibleBegin;

    const size_t row = nPos/mnCols;
    const size_t col = nPos%mnCols;
    const long x = maItemListRect.Left()+col*(mnItemWidth+mnSpacing);
    const long y = maItemListRect.Top()+row*(mnItemHeight+mnSpacing);

    return Rectangle( Point(x, y), Size(mnItemWidth, mnItemHeight) );
}

void ThumbnailView::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;

    if ( nPos < mItemList.size() ) {
        ValueItemList::iterator it = mItemList.begin();
        ::std::advance( it, nPos );
        delete *it;
        mItemList.erase( it );
    }

    // reset variables
    if ( (mnHighItemId == nItemId) || (mnSelItemId == nItemId) )
    {
        mnCurCol        = 0;
        mnHighItemId    = 0;
        mnSelItemId     = 0;
        mbNoSelection   = true;
    }

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ThumbnailView::Clear()
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;
    mnCurCol        = 0;
    mnHighItemId    = 0;
    mnSelItemId     = 0;
    mbNoSelection   = true;

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

size_t ThumbnailView::GetItemCount() const
{
    return mItemList.size();
}

size_t ThumbnailView::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mItemList.size(); i < n; ++i ) {
        if ( mItemList[i]->mnId == nItemId ) {
            return i;
        }
    }
    return THUMBNAILVIEW_ITEM_NOTFOUND;
}

sal_uInt16 ThumbnailView::GetItemId( size_t nPos ) const
{
    return ( nPos < mItemList.size() ) ? mItemList[nPos]->mnId : 0 ;
}

sal_uInt16 ThumbnailView::GetItemId( const Point& rPos ) const
{
    size_t nItemPos = ImplGetItem( rPos );
    if ( nItemPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

Rectangle ThumbnailView::GetItemRect( sal_uInt16 nItemId ) const
{
    const size_t nPos = GetItemPos( nItemId );

    if ( nPos!=THUMBNAILVIEW_ITEM_NOTFOUND && mItemList[nPos]->mbVisible )
        return ImplGetItemRect( nPos );

    return Rectangle();
}

void ThumbnailView::EnableFullItemMode( bool bFullMode )
{
    mbFullMode = bFullMode;
}

void ThumbnailView::SetColCount( sal_uInt16 nNewCols )
{
    if ( mnUserCols != nNewCols )
    {
        mnUserCols = nNewCols;
        mbFormat = true;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::SetLineCount( sal_uInt16 nNewLines )
{
    if ( mnUserVisLines != nNewLines )
    {
        mnUserVisLines = nNewLines;
        mbFormat = true;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::SetItemWidth( long nNewItemWidth )
{
    if ( mnUserItemWidth != nNewItemWidth )
    {
        mnUserItemWidth = nNewItemWidth;
        mbFormat = true;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::SetItemHeight( long nNewItemHeight )
{
    if ( mnUserItemHeight != nNewItemHeight )
    {
        mnUserItemHeight = nNewItemHeight;
        mbFormat = true;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::SelectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = 0;

    if ( nItemId )
    {
        nItemPos = GetItemPos( nItemId );
        if ( nItemPos == THUMBNAILVIEW_ITEM_NOTFOUND )
            return;
    }

    if ( (mnSelItemId != nItemId) || mbNoSelection )
    {
        sal_uInt16 nOldItem = mnSelItemId ? mnSelItemId : 1;
        mnSelItemId = nItemId;
        mbNoSelection = false;

        bool bNewOut = !mbFormat && IsReallyVisible() && IsUpdateMode();
        bool bNewLine = false;

        // if necessary scroll to the visible area
        if ( mbScroll && nItemId )
        {
            sal_uInt16 nNewLine = (sal_uInt16)(nItemPos / mnCols);
            if ( nNewLine < mnFirstLine )
            {
                mnFirstLine = nNewLine;
                bNewLine = true;
            }
            else if ( nNewLine > (sal_uInt16)(mnFirstLine+mnVisLines-1) )
            {
                mnFirstLine = (sal_uInt16)(nNewLine-mnVisLines+1);
                bNewLine = true;
            }
        }

        if ( bNewOut )
        {
            if ( bNewLine )
            {
                // redraw everything if the visible area has changed
                mbFormat = true;
                ImplDraw();
            }
            else
            {
                // remove old selection and draw the new one
                ImplHideSelect( nOldItem );
                ImplDrawSelect();
            }
        }

        if( ImplHasAccessibleListeners() )
        {
            // focus event (deselect)
            if( nOldItem )
            {
                const size_t nPos = GetItemPos( nItemId );

                if( nPos != THUMBNAILVIEW_ITEM_NOTFOUND )
                {
                    ThumbnailViewAcc* pItemAcc = ThumbnailViewAcc::getImplementation(
                        mItemList[nPos]->GetAccessible( mbIsTransientChildrenDisabled ) );

                    if( pItemAcc )
                    {
                        ::com::sun::star::uno::Any aOldAny, aNewAny;
                        if( !mbIsTransientChildrenDisabled )
                        {
                            aOldAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                                static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                            ImplFireAccessibleEvent (::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                        }
                        else
                        {
                            aOldAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                            pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                        }
                    }
                }
            }

            // focus event (select)
            const size_t nPos = GetItemPos( mnSelItemId );

            ThumbnailViewItem* pItem = NULL;
            if( nPos != THUMBNAILVIEW_ITEM_NOTFOUND )
                pItem = mItemList[nPos];

            ThumbnailViewAcc* pItemAcc = NULL;
            if (pItem != NULL)
                pItemAcc = ThumbnailViewAcc::getImplementation( pItem->GetAccessible( mbIsTransientChildrenDisabled ) );

            if( pItemAcc )
            {
                ::com::sun::star::uno::Any aOldAny, aNewAny;
                if( !mbIsTransientChildrenDisabled )
                {
                    aNewAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                        static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                }
                else
                {
                    aNewAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                    pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                }
            }

            // selection event
            ::com::sun::star::uno::Any aOldAny, aNewAny;
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny );
        }
        maHighlightHdl.Call(this);
    }
}

void ThumbnailView::SetNoSelection()
{
    mbNoSelection   = true;
    mbHighlight     = false;
    mbSelection     = false;

    if ( IsReallyVisible() && IsUpdateMode() )
        ImplDraw();
}

void ThumbnailView::SetItemImage( sal_uInt16 nItemId, const Image& rImage )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;

    ThumbnailViewItem* pItem = mItemList[nPos];
    pItem->maImage = rImage;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        const Rectangle aRect = ImplGetItemRect(nPos);
        DrawItem( pItem, aRect );
        Invalidate( aRect );
    }
    else
        mbFormat = true;
}

Image ThumbnailView::GetItemImage( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        return mItemList[nPos]->maImage;
    else
        return Image();
}

void ThumbnailView::SetItemText( sal_uInt16 nItemId, const rtl::OUString& rText )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;


    ThumbnailViewItem* pItem = mItemList[nPos];

    // Remember old and new name for accessibility event.
    ::com::sun::star::uno::Any aOldName, aNewName;
    ::rtl::OUString sString (pItem->maText);
    aOldName <<= sString;
    sString = rText;
    aNewName <<= sString;

    pItem->maText = rText;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        sal_uInt16 nTempId = mnSelItemId;

        if ( mbHighlight )
            nTempId = mnHighItemId;
    }

    if (ImplHasAccessibleListeners())
    {
        ::com::sun::star::uno::Reference<
              ::com::sun::star::accessibility::XAccessible> xAccessible (
                  pItem->GetAccessible( mbIsTransientChildrenDisabled ) );
        static_cast<ThumbnailViewAcc*>(xAccessible.get())->FireAccessibleEvent (
            ::com::sun::star::accessibility::AccessibleEventId::NAME_CHANGED,
            aOldName, aNewName);
    }
}

rtl::OUString ThumbnailView::GetItemText( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        return mItemList[nPos]->maText;

    return rtl::OUString();
}

void ThumbnailView::SetColor( const Color& rColor )
{
    maColor     = rColor;
    mbFormat    = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        ImplDraw();
}

void ThumbnailView::StartSelection()
{
    mbHighlight     = true;
    mbSelection     = true;
    mnHighItemId    = mnSelItemId;
}

void ThumbnailView::EndSelection()
{
    if ( mbHighlight )
    {
        if ( IsTracking() )
            EndTracking( ENDTRACK_CANCEL );

        ImplHighlightItem( mnSelItemId );
        mbHighlight = false;
    }
    mbSelection = false;
}

bool ThumbnailView::StartDrag( const CommandEvent& rCEvt, Region& rRegion )
{
    if ( rCEvt.GetCommand() != COMMAND_STARTDRAG )
        return false;

    // if necessary abort an existing action
    EndSelection();

    // Check out if the the clicked on page is selected. If this is not the
    // case set it as the current item. We only check mouse actions since
    // drag-and-drop can also be triggered by the keyboard
    sal_uInt16 nSelId;
    if ( rCEvt.IsMouseEvent() )
        nSelId = GetItemId( rCEvt.GetMousePosPixel() );
    else
        nSelId = mnSelItemId;

    // don't activate dragging if no item was clicked on
    if ( !nSelId )
        return false;

    // Check out if the page was selected. If not set as current page and
    // call select.
    if ( nSelId != mnSelItemId )
    {
        SelectItem( nSelId );
        Update();
        Select();
    }

    Region aRegion;

    // assign region
    rRegion = aRegion;

    return true;
}

Size ThumbnailView::CalcWindowSizePixel( const Size& rItemSize, sal_uInt16 nDesireCols,
                                    sal_uInt16 nDesireLines )
{
    size_t nCalcCols = nDesireCols;
    size_t nCalcLines = nDesireLines;

    if ( !nCalcCols )
    {
        if ( mnUserCols )
            nCalcCols = mnUserCols;
        else
            nCalcCols = 1;
    }

    if ( !nCalcLines )
    {
        nCalcLines = mnVisLines;

        if ( mbFormat )
        {
            if ( mnUserVisLines )
                nCalcLines = mnUserVisLines;
            else
            {
                // Floor( (M+N-1)/N )==Ceiling( M/N )
                nCalcLines = (mItemList.size()+nCalcCols-1) / nCalcCols;
                if ( !nCalcLines )
                    nCalcLines = 1;
            }
        }
    }

    Size        aSize( rItemSize.Width()*nCalcCols, rItemSize.Height()*nCalcLines );

    if ( mnSpacing )
    {
        aSize.Width()  += mnSpacing*(nCalcCols-1);
        aSize.Height() += mnSpacing*(nCalcLines-1);
    }

    // sum possible ScrollBar width
    aSize.Width() += GetScrollWidth();

    return aSize;
}

long ThumbnailView::GetScrollWidth() const
{
    if ( GetStyle() & WB_VSCROLL )
    {
        ((ThumbnailView*)this)->ImplInitScrollBar();
        return mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;
    }
    else
        return 0;
}

void ThumbnailView::SetHighlightHdl( const Link& rLink )
{
    maHighlightHdl = rLink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


