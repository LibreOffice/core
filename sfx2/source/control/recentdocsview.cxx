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

#include <sal/log.hxx>
#include <comphelper/base64.hxx>
#include <sax/tools/converter.hxx>
#include <sfx2/recentdocsview.hxx>
#include <sfx2/templatelocalview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/event.hxx>
#include <vcl/pngread.hxx>
#include <vcl/ptrstyle.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <sfx2/strings.hrc>
#include <bitmaps.hlst>

#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace {

/// Set (larger) font for the Welcome message.
void SetMessageFont(vcl::RenderContext& rRenderContext)
{
    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetFontHeight(aFont.GetFontHeight() * 1.3);
    rRenderContext.SetFont(aFont);
}

bool IsDocEncrypted(const OUString& rURL)
{
    uno::Reference< uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
    bool bIsEncrypted = false;

    try
    {
        uno::Reference<lang::XSingleServiceFactory> xStorageFactory = embed::StorageFactory::create(xContext);

        uno::Sequence<uno::Any> aArgs (2);
        aArgs[0] <<= rURL;
        aArgs[1] <<= embed::ElementModes::READ;
        uno::Reference<embed::XStorage> xDocStorage (
            xStorageFactory->createInstanceWithArguments(aArgs),
            uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > xStorageProps( xDocStorage, uno::UNO_QUERY );
        if ( xStorageProps.is() )
        {
            try
            {
                xStorageProps->getPropertyValue("HasEncryptedEntries")
                    >>= bIsEncrypted;
            } catch( uno::Exception& ) {}
        }
    }
    catch (const uno::Exception& rException)
    {
        SAL_WARN("sfx",
            "caught exception trying to find out if doc is encrypted"
            << rURL << ": " << rException);
    }

    return bIsEncrypted;
}

}

namespace sfx2
{

static std::map<ApplicationType,OUString> BitmapForExtension =
{
    { ApplicationType::TYPE_WRITER, SFX_FILE_THUMBNAIL_TEXT },
    { ApplicationType::TYPE_CALC, SFX_FILE_THUMBNAIL_SHEET },
    { ApplicationType::TYPE_IMPRESS, SFX_FILE_THUMBNAIL_PRESENTATION },
    { ApplicationType::TYPE_DRAW, SFX_FILE_THUMBNAIL_DRAWING },
    { ApplicationType::TYPE_DATABASE, SFX_FILE_THUMBNAIL_DATABASE },
    { ApplicationType::TYPE_MATH, SFX_FILE_THUMBNAIL_MATH }
};

static std::map<ApplicationType,OUString> EncryptedBitmapForExtension =
{
    { ApplicationType::TYPE_WRITER, BMP_128X128_WRITER_DOC },
    { ApplicationType::TYPE_CALC, BMP_128X128_CALC_DOC },
    { ApplicationType::TYPE_IMPRESS, BMP_128X128_IMPRESS_DOC },
    { ApplicationType::TYPE_DRAW, BMP_128X128_DRAW_DOC },
    // FIXME: icon for encrypted db doc doesn't exist
    { ApplicationType::TYPE_DATABASE, BMP_128X128_CALC_DOC },
    { ApplicationType::TYPE_MATH, BMP_128X128_MATH_DOC }
};

static constexpr long gnTextHeight = 30;
static constexpr long gnItemPadding = 5;

RecentDocsView::RecentDocsView( vcl::Window* pParent )
    : ThumbnailView(pParent)
    , mnFileTypes(ApplicationType::TYPE_NONE)
    , mnLastMouseDownItem(THUMBNAILVIEW_ITEM_NOTFOUND)
    , maWelcomeImage(StockImage::Yes, BMP_WELCOME)
    , maWelcomeLine1(SfxResId(STR_WELCOME_LINE1))
    , maWelcomeLine2(SfxResId(STR_WELCOME_LINE2))
{
    tools::Rectangle aScreen = Application::GetScreenPosSizePixel(Application::GetDisplayBuiltInScreen());
    mnItemMaxSize = std::min(aScreen.GetWidth(),aScreen.GetHeight()) > 800 ? 256 : 192;

    SetStyle(GetStyle() | WB_VSCROLL);
    setItemMaxTextLength( 30 );
    setItemDimensions( mnItemMaxSize, mnItemMaxSize, gnTextHeight, gnItemPadding );

    maFillColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsBackgroundColor::get());
    maTextColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsTextColor::get());
    maHighlightColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsHighlightColor::get());
    maHighlightTextColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsHighlightTextColor::get());
    mfHighlightTransparence = 0.25;
}

VCL_BUILDER_FACTORY(RecentDocsView)

bool RecentDocsView::typeMatchesExtension(ApplicationType type, const OUString &rExt)
{
    bool bRet = false;

    if (rExt == "odt" || rExt == "doc" || rExt == "docx" ||
        rExt == "rtf" || rExt == "txt" || rExt == "odm" || rExt == "otm")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_WRITER);
    }
    else if (rExt == "ods" || rExt == "xls" || rExt == "xlsx")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_CALC);
    }
    else if (rExt == "odp" || rExt == "pps" || rExt == "ppt" ||
            rExt == "pptx")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_IMPRESS);
    }
    else if (rExt == "odg")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_DRAW);
    }
    else if (rExt == "odb")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_DATABASE);
    }
    else if (rExt == "odf")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_MATH);
    }
    else
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_OTHER);
    }

    return bRet;
}

bool RecentDocsView::isAcceptedFile(const OUString &rURL) const
{
    INetURLObject aUrl(rURL);
    OUString aExt = aUrl.getExtension();
    return (mnFileTypes & ApplicationType::TYPE_WRITER   && typeMatchesExtension(ApplicationType::TYPE_WRITER,  aExt)) ||
           (mnFileTypes & ApplicationType::TYPE_CALC     && typeMatchesExtension(ApplicationType::TYPE_CALC,    aExt)) ||
           (mnFileTypes & ApplicationType::TYPE_IMPRESS  && typeMatchesExtension(ApplicationType::TYPE_IMPRESS, aExt)) ||
           (mnFileTypes & ApplicationType::TYPE_DRAW     && typeMatchesExtension(ApplicationType::TYPE_DRAW,    aExt)) ||
           (mnFileTypes & ApplicationType::TYPE_DATABASE && typeMatchesExtension(ApplicationType::TYPE_DATABASE,aExt)) ||
           (mnFileTypes & ApplicationType::TYPE_MATH     && typeMatchesExtension(ApplicationType::TYPE_MATH,    aExt)) ||
           (mnFileTypes & ApplicationType::TYPE_OTHER    && typeMatchesExtension(ApplicationType::TYPE_OTHER,   aExt));
}

BitmapEx RecentDocsView::getDefaultThumbnail(const OUString &rURL)
{
    BitmapEx aImg;
    INetURLObject aUrl(rURL);
    OUString aExt = aUrl.getExtension();

    const std::map<ApplicationType,OUString>& rWhichMap = IsDocEncrypted( rURL) ?
        EncryptedBitmapForExtension : BitmapForExtension;

    std::map<ApplicationType,OUString>::const_iterator mIt =
        std::find_if( rWhichMap.begin(), rWhichMap.end(),
              [aExt] ( const std::pair<ApplicationType,OUString>& aEntry )
              { return typeMatchesExtension( aEntry.first, aExt); } );

    if (mIt != rWhichMap.end())
        aImg = BitmapEx(mIt->second);
    else
        aImg = BitmapEx(SFX_FILE_THUMBNAIL_DEFAULT);

    return aImg;
}

void RecentDocsView::insertItem(const OUString &rURL, const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId)
{
    AppendItem( std::make_unique<RecentDocsViewItem>(*this, rURL, rTitle, rThumbnail, nId, mnItemMaxSize) );
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
            //fdo#74834: only load thumbnail if the corresponding option is not disabled in the configuration
            else if (rRecentEntry[j].Name == "Thumbnail" && officecfg::Office::Common::History::RecentDocsThumbnail::get())
            {
                OUString aBase64;
                a >>= aBase64;
                if (!aBase64.isEmpty())
                {
                    Sequence<sal_Int8> aDecoded;
                    comphelper::Base64::decode(aDecoded, aBase64);

                    SvMemoryStream aStream(aDecoded.getArray(), aDecoded.getLength(), StreamMode::READ);
                    vcl::PNGReader aReader(aStream);
                    aThumbnail = aReader.Read();
                }
            }
        }

        if(!aURL.isEmpty())
        {
            INetURLObject  aURLObj( aURL );
            //Remove extension from url's last segment and use it as title
            aTitle = aURLObj.GetBase(); //DecodeMechanism::WithCharset
        }

        if (isAcceptedFile(aURL))
        {
            insertItem(aURL, aTitle, aThumbnail, i+1);
        }
    }

    CalculateItemPositions();
    Invalidate();
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
        {
            pItem->MouseButtonUp(rMEvt);

            ThumbnailViewItem* pNewItem = ImplGetItem(nPos);
            if(pNewItem)
                pNewItem->setHighlight(true);
        }

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

void RecentDocsView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle &aRect)
{
    // Set preferred width
    if (mFilteredItemList.empty())
    {
        rRenderContext.Push(PushFlags::FONT);
        SetMessageFont(rRenderContext);
        set_width_request(std::max(rRenderContext.GetTextWidth(maWelcomeLine1),
                                   rRenderContext.GetTextWidth(maWelcomeLine2)));
        rRenderContext.Pop();
    }
    else
    {
        set_width_request(gnTextHeight + mnItemMaxSize + 2 * gnItemPadding);
    }

    if (mItemList.empty())
    {
        // No recent files to be shown yet. Show a welcome screen.
        rRenderContext.Push(PushFlags::FONT | PushFlags::TEXTCOLOR);
        SetMessageFont(rRenderContext);
        SetTextColor(maTextColor);

        long nTextHeight = rRenderContext.GetTextHeight();

        const Size& rImgSize = maWelcomeImage.GetSizePixel();
        const Size& rSize = GetSizePixel();

        const int nX = (rSize.Width() - rImgSize.Width())/2;
        int nY = (rSize.Height() - 3 * nTextHeight - rImgSize.Height())/2;
        Point aImgPoint(nX, nY);
        rRenderContext.DrawImage(aImgPoint, rImgSize, maWelcomeImage);

        nY = nY + rImgSize.Height();
        rRenderContext.DrawText(tools::Rectangle(0, nY, rSize.Width(), nY + nTextHeight),
                                maWelcomeLine1,
                                DrawTextFlags::Center);
        rRenderContext.DrawText(tools::Rectangle(0, nY + 1.5 * nTextHeight, rSize.Width(), rSize.Height()),
                                maWelcomeLine2,
                                DrawTextFlags::MultiLine | DrawTextFlags::WordBreak | DrawTextFlags::Center);

        rRenderContext.Pop();
    }
    else
    {
        ThumbnailView::Paint(rRenderContext, aRect);
    }
}

void RecentDocsView::LoseFocus()
{
    deselectItems();

    ThumbnailView::LoseFocus();
}

void RecentDocsView::Clear()
{
    Invalidate();
    ThumbnailView::Clear();
}

IMPL_STATIC_LINK( RecentDocsView, ExecuteHdl_Impl, void*, p, void )
{
    LoadRecentFile* pLoadRecentFile = static_cast< LoadRecentFile*>(p);
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

    if ( !pLoadRecentFile->pView->IsDisposed() )
        pLoadRecentFile->pView->SetPointer( PointerStyle::Arrow );

    delete pLoadRecentFile;
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
