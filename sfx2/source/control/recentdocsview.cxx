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

#include <sax/tools/converter.hxx>
#include <sfx2/recentdocsview.hxx>
#include <sfx2/templateabstractview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxresid.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/pngread.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <templateview.hrc>

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;

RecentDocsView::RecentDocsView( vcl::Window* pParent )
    : ThumbnailView(pParent)
    , mnFileTypes(TYPE_NONE)
    , mnTextHeight(30)
    , mnItemPadding(5)
    , mnItemMaxTextLength(30)
    , mnLastMouseDownItem(THUMBNAILVIEW_ITEM_NOTFOUND)
    , maWelcomeImage(SfxResId(IMG_WELCOME))
    , maWelcomeLine1(SfxResId(STR_WELCOME_LINE1))
    , maWelcomeLine2(SfxResId(STR_WELCOME_LINE2))
{
    Rectangle aScreen = Application::GetScreenPosSizePixel(Application::GetDisplayBuiltInScreen());
    mnItemMaxSize = std::min(aScreen.GetWidth(),aScreen.GetHeight()) > 800 ? 256 : 192;

    SetStyle(GetStyle() | WB_VSCROLL);
    setItemMaxTextLength( mnItemMaxTextLength );
    setItemDimensions( mnItemMaxSize, mnItemMaxSize, mnTextHeight, mnItemPadding );
}

VCL_BUILDER_FACTORY(RecentDocsView)

bool RecentDocsView::typeMatchesExtension(ApplicationType type, const OUString &rExt)
{
    bool bRet = false;

    if (rExt == "odt" || rExt == "doc" || rExt == "docx" ||
        rExt == "rtf" || rExt == "txt" || rExt == "odm" || rExt == "otm")
    {
        bRet = type & TYPE_WRITER;
    }
    else if (rExt == "ods" || rExt == "xls" || rExt == "xlsx")
    {
        bRet = type & TYPE_CALC;
    }
    else if (rExt == "odp" || rExt == "pps" || rExt == "ppt" ||
            rExt == "pptx")
    {
        bRet = type & TYPE_IMPRESS;
    }
    else if (rExt == "odg")
    {
        bRet = type & TYPE_DRAW;
    }
    else if (rExt == "odb")
    {
        bRet = type & TYPE_DATABASE;
    }
    else if (rExt == "odf")
    {
        bRet = type & TYPE_MATH;
    }
    else
    {
        bRet = type & TYPE_OTHER;
    }

    return bRet;
}

bool RecentDocsView::isAcceptedFile(const OUString &rURL) const
{
    INetURLObject aUrl(rURL);
    OUString aExt = aUrl.getExtension();
    return (mnFileTypes & TYPE_WRITER   && typeMatchesExtension(TYPE_WRITER,  aExt)) ||
           (mnFileTypes & TYPE_CALC     && typeMatchesExtension(TYPE_CALC,    aExt)) ||
           (mnFileTypes & TYPE_IMPRESS  && typeMatchesExtension(TYPE_IMPRESS, aExt)) ||
           (mnFileTypes & TYPE_DRAW     && typeMatchesExtension(TYPE_DRAW,    aExt)) ||
           (mnFileTypes & TYPE_DATABASE && typeMatchesExtension(TYPE_DATABASE,aExt)) ||
           (mnFileTypes & TYPE_MATH     && typeMatchesExtension(TYPE_MATH,    aExt)) ||
           (mnFileTypes & TYPE_OTHER    && typeMatchesExtension(TYPE_OTHER,   aExt));
}

void RecentDocsView::SetMessageFont()
{
    vcl::Font aFont(GetFont());
    aFont.SetHeight(aFont.GetHeight()*1.3);
    SetFont(aFont);
}

BitmapEx RecentDocsView::getDefaultThumbnail(const OUString &rURL)
{
    BitmapEx aImg;
    INetURLObject aUrl(rURL);
    OUString aExt = aUrl.getExtension();

    if ( typeMatchesExtension( TYPE_WRITER, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_TEXT ) );
    else if ( typeMatchesExtension( TYPE_CALC, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_SHEET ) );
    else if ( typeMatchesExtension( TYPE_IMPRESS, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_PRESENTATION ) );
    else if ( typeMatchesExtension( TYPE_DRAW, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_DRAWING ) );
    else if ( typeMatchesExtension( TYPE_DATABASE, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_DATABASE ) );
    else if ( typeMatchesExtension( TYPE_MATH, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_MATH ) );
    else
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_DEFAULT ) );

    return aImg;
}

void RecentDocsView::insertItem(const OUString &rURL, const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId)
{
    RecentDocsViewItem *pChild = new RecentDocsViewItem(*this, rURL, rTitle, rThumbnail, nId, GetThumbnailSize());

    AppendItem(pChild);
}

void RecentDocsView::Reload()
{
    Clear();

    Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
    for ( int i = 0; i < aHistoryList.getLength(); i++ )
    {
        Sequence< PropertyValue >& rRecentEntry = aHistoryList[i];

        OUString aURL;
        OUString aTitle;
        BitmapEx aThumbnail;

        for ( int j = 0; j < rRecentEntry.getLength(); j++ )
        {
            Any a = rRecentEntry[j].Value;

            if (rRecentEntry[j].Name == "URL")
                a >>= aURL;
            else if (rRecentEntry[j].Name == "Title")
                a >>= aTitle;
            else if (rRecentEntry[j].Name == "Thumbnail")
            {
                OUString aBase64;
                a >>= aBase64;
                if (!aBase64.isEmpty())
                {
                    Sequence<sal_Int8> aDecoded;
                    sax::Converter::decodeBase64(aDecoded, aBase64);

                    SvMemoryStream aStream(aDecoded.getArray(), aDecoded.getLength(), StreamMode::READ);
                    vcl::PNGReader aReader(aStream);
                    aThumbnail = aReader.Read();
                }
            }
        }

        if (isAcceptedFile(aURL))
        {
            insertItem(aURL, aTitle, aThumbnail, i+1);
        }
    }

    CalculateItemPositions();
    Invalidate();

    // Set preferred width
    if (mFilteredItemList.empty())
    {
        vcl::Font aOldFont(GetFont());
        SetMessageFont();
        set_width_request(std::max(GetTextWidth(maWelcomeLine1), GetTextWidth(maWelcomeLine2)));
        SetFont(aOldFont);
    }
    else
    {
        set_width_request(mnTextHeight + mnItemMaxSize + 2*mnItemPadding);
    }
}

void RecentDocsView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (rMEvt.IsLeft())
    {
        mnLastMouseDownItem = ImplGetItem(rMEvt.GetPosPixel());

        // ignore to avoid stuff done in ThumbnailView; we don't do selections etc.
        return;
    }

    ThumbnailView::MouseButtonDown(rMEvt);
}

void RecentDocsView::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if( rMEvt.GetClicks() > 1 )
            return;

        size_t nPos = ImplGetItem(rMEvt.GetPosPixel());
        ThumbnailViewItem* pItem = ImplGetItem(nPos);

        if (pItem && nPos == mnLastMouseDownItem)
            pItem->MouseButtonUp(rMEvt);

        mnLastMouseDownItem = THUMBNAILVIEW_ITEM_NOTFOUND;

        if (pItem)
            return;
    }
    ThumbnailView::MouseButtonUp(rMEvt);
}

void RecentDocsView::OnItemDblClicked(ThumbnailViewItem *pItem)
{
    RecentDocsViewItem* pRecentItem = dynamic_cast< RecentDocsViewItem* >(pItem);
    if (pRecentItem)
        pRecentItem->OpenDocument();
}

void RecentDocsView::Paint(vcl::RenderContext& rRenderContext, const Rectangle &aRect)
{
    if ( mItemList.size() == 0 )
    {
        // No recent files to be shown yet. Show a welcome screen.
        vcl::Font aOldFont(GetFont());
        SetMessageFont();

        long nTextHeight = GetTextHeight();

        long nTextWidth1 = GetTextWidth(maWelcomeLine1);
        long nTextWidth2 = GetTextWidth(maWelcomeLine2);

        const Size & rImgSize = maWelcomeImage.GetSizePixel();
        const Size & rSize = GetSizePixel();

        const int nX = (rSize.Width() - rImgSize.Width())/2;
        const int nY = (rSize.Height() - 3 * nTextHeight - rImgSize.Height())/2;

        Point aImgPoint(nX, nY);
        Point aStr1Point((rSize.Width() - nTextWidth1)/2, nY + rImgSize.Height() + 0.7 * nTextHeight);
        Point aStr2Point((rSize.Width() - nTextWidth2)/2, nY + rImgSize.Height() + 1.7 * nTextHeight);

        DrawImage(aImgPoint, rImgSize, maWelcomeImage, IMAGE_DRAW_SEMITRANSPARENT);
        DrawText(aStr1Point, maWelcomeLine1);
        DrawText(aStr2Point, maWelcomeLine2);

        SetFont(aOldFont);
    }
    else
        ThumbnailView::Paint(rRenderContext, aRect);
}

void RecentDocsView::LoseFocus()
{
    deselectItems();

    ThumbnailView::LoseFocus();
}


void RecentDocsView::Clear()
{
    vcl::Font aOldFont(GetFont());
    SetMessageFont();
    set_width_request(std::max(GetTextWidth(maWelcomeLine1), GetTextWidth(maWelcomeLine2)));
    SetFont(aOldFont);

    ThumbnailView::Clear();
}

IMPL_STATIC_LINK( RecentDocsView, ExecuteHdl_Impl, LoadRecentFile*, pLoadRecentFile )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pLoadRecentFile->xDispatch->dispatch( pLoadRecentFile->aTargetURL, pLoadRecentFile->aArgSeq );
    }
    catch ( const Exception& )
    {
    }

    delete pLoadRecentFile;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
