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
#include <recentdocsview.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/historyoptions.hxx>
#include <vcl/event.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/svapp.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <sfx2/strings.hrc>
#include <bitmaps.hlst>
#include <vcl/virdev.hxx>
#include "recentdocsviewitem.hxx"
#include <sfx2/app.hxx>

#include <officecfg/Office/Common.hxx>

#include <map>

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

        uno::Sequence<uno::Any> aArgs{ uno::Any(rURL), uno::Any(embed::ElementModes::READ) };
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
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sfx",
            "caught exception trying to find out if doc <" << rURL << "> is encrypted:");
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

constexpr tools::Long gnTextHeight = 30;
constexpr tools::Long gnItemPadding = 5;

RecentDocsView::RecentDocsView(std::unique_ptr<weld::ScrolledWindow> xWindow, std::unique_ptr<weld::Menu> xMenu)
    : ThumbnailView(std::move(xWindow), std::move(xMenu))
    , mnFileTypes(ApplicationType::TYPE_NONE)
    , mnLastMouseDownItem(THUMBNAILVIEW_ITEM_NOTFOUND)
    , maWelcomeLine1(SfxResId(STR_WELCOME_LINE1))
    , maWelcomeLine2(SfxResId(STR_WELCOME_LINE2))
    , mpLoadRecentFile(nullptr)
    , m_nExecuteHdlId(nullptr)
{
    tools::Rectangle aScreen = Application::GetScreenPosSizePixel(Application::GetDisplayBuiltInScreen());
    mnItemMaxSize = std::min(aScreen.GetWidth(),aScreen.GetHeight()) > 800 ? 256 : 192;

    setItemMaxTextLength( 30 );
    setItemDimensions( mnItemMaxSize, mnItemMaxSize, gnTextHeight, gnItemPadding );

    maFillColor = Color(ColorTransparency, officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsBackgroundColor::get());
    maTextColor = Color(ColorTransparency, officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsTextColor::get());
    maHighlightColor = Color(ColorTransparency, officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsHighlightColor::get());
    maHighlightTextColor = Color(ColorTransparency, officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsHighlightTextColor::get());
    mfHighlightTransparence = 0.25;

    UpdateColors();
}

RecentDocsView::~RecentDocsView()
{
    Application::RemoveUserEvent(m_nExecuteHdlId);
    m_nExecuteHdlId = nullptr;
    if (mpLoadRecentFile)
    {
        mpLoadRecentFile->pView = nullptr;
        mpLoadRecentFile = nullptr;
    }
}

bool RecentDocsView::typeMatchesExtension(ApplicationType type, std::u16string_view rExt)
{
    bool bRet = false;

    if (rExt == u"odt" || rExt == u"fodt" || rExt == u"doc" || rExt == u"docx" ||
        rExt == u"rtf" || rExt == u"txt" || rExt == u"odm" || rExt == u"otm")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_WRITER);
    }
    else if (rExt == u"ods" || rExt == u"fods" || rExt == u"xls" || rExt == u"xlsx")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_CALC);
    }
    else if (rExt == u"odp" || rExt == u"fodp" || rExt == u"pps" || rExt == u"ppt" ||
            rExt == u"pptx")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_IMPRESS);
    }
    else if (rExt == u"odg" || rExt == u"fodg")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_DRAW);
    }
    else if (rExt == u"odb")
    {
        bRet = static_cast<bool>(type & ApplicationType::TYPE_DATABASE);
    }
    else if (rExt == u"odf")
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

BitmapEx RecentDocsView::getDefaultThumbnail(const OUString &rURL, bool bCheckEncrypted)
{
    BitmapEx aImg;
    INetURLObject aUrl(rURL);
    OUString aExt = aUrl.getExtension();

    // tdf#131850: avoid reading the file to check if it's encrypted,
    // if we only need its generic "module" thumbnail
    const std::map<ApplicationType,OUString>& rWhichMap = bCheckEncrypted && IsDocEncrypted(rURL) ?
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

    std::vector< SvtHistoryOptions::HistoryItem > aHistoryList = SvtHistoryOptions::GetList( EHistoryType::PickList );
    for ( size_t i = 0; i < aHistoryList.size(); i++ )
    {
        const SvtHistoryOptions::HistoryItem& rRecentEntry = aHistoryList[i];

        OUString aURL = rRecentEntry.sURL;
        OUString aTitle;
        BitmapEx aThumbnail;
        BitmapEx aModule;

        //fdo#74834: only load thumbnail if the corresponding option is not disabled in the configuration
        if (officecfg::Office::Common::History::RecentDocsThumbnail::get())
        {
            OUString aBase64 = rRecentEntry.sThumbnail;
            if (!aBase64.isEmpty())
            {
                Sequence<sal_Int8> aDecoded;
                comphelper::Base64::decode(aDecoded, aBase64);

                SvMemoryStream aStream(aDecoded.getArray(), aDecoded.getLength(), StreamMode::READ);
                vcl::PngImageReader aReader(aStream);
                aThumbnail = aReader.read();
            } else
            {
                INetURLObject aUrl(aURL);
                if (mnFileTypes & ApplicationType::TYPE_DATABASE && typeMatchesExtension(ApplicationType::TYPE_DATABASE, aUrl.getExtension()))
                {
                    aThumbnail = BitmapEx(ThumbnailView::ItemHeight() > 192 ? SFX_THUMBNAIL_BASE_256 : SFX_THUMBNAIL_BASE_192);
                }
            }
        }

        aModule = getDefaultThumbnail(aURL, false); // We don't need an "encrypted" icon here
        if (!aModule.IsEmpty() && !aThumbnail.IsEmpty()) {
            ScopedVclPtr<VirtualDevice> m_pVirDev(VclPtr<VirtualDevice>::Create());
            Size aSize(aThumbnail.GetSizePixel());
            m_pVirDev->SetOutputSizePixel(aSize);
            m_pVirDev->DrawBitmapEx(Point(), aThumbnail);
            m_pVirDev->DrawBitmapEx(Point(aSize.Width()-53,aSize.Height()-53), Size(48, 48), aModule);
            aThumbnail = m_pVirDev->GetBitmapEx(Point(), aSize);
            m_pVirDev.disposeAndClear();
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

bool RecentDocsView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (rMEvt.IsLeft())
    {
        mnLastMouseDownItem = ImplGetItem(rMEvt.GetPosPixel());

        // ignore to avoid stuff done in ThumbnailView; we don't do selections etc.
        return true;
    }

    return ThumbnailView::MouseButtonDown(rMEvt);
}

bool RecentDocsView::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if( rMEvt.GetClicks() > 1 )
            return true;

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
            return true;
    }
    return ThumbnailView::MouseButtonUp(rMEvt);
}

void RecentDocsView::OnItemDblClicked(ThumbnailViewItem *pItem)
{
    RecentDocsViewItem* pRecentItem = dynamic_cast< RecentDocsViewItem* >(pItem);
    if (pRecentItem)
        pRecentItem->OpenDocument();
}

void RecentDocsView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle &aRect)
{
    ThumbnailView::Paint(rRenderContext, aRect);

    if (!mItemList.empty())
        return;

    if (maWelcomeImage.IsEmpty())
    {
        const tools::Long aWidth(aRect.GetWidth() > aRect.getHeight() ? aRect.GetHeight()/2 : aRect.GetWidth()/2);
        maWelcomeImage = SfxApplication::GetApplicationLogo(aWidth);
    }

    // No recent files to be shown yet. Show a welcome screen.
    rRenderContext.Push(vcl::PushFlags::FONT | vcl::PushFlags::TEXTCOLOR);
    SetMessageFont(rRenderContext);
    rRenderContext.SetTextColor(maTextColor);

    tools::Long nTextHeight = rRenderContext.GetTextHeight();

    const Size& rImgSize = maWelcomeImage.GetSizePixel();
    const Size& rSize = GetOutputSizePixel();

    const int nX = (rSize.Width() - rImgSize.Width())/2;
    int nY = (rSize.Height() - 3 * nTextHeight - rImgSize.Height())/2;
    Point aImgPoint(nX, nY);
    rRenderContext.DrawBitmapEx(aImgPoint, rImgSize, maWelcomeImage);

    nY = nY + rImgSize.Height();
    rRenderContext.DrawText(tools::Rectangle(0, nY + 1 * nTextHeight, rSize.Width(), nY + nTextHeight),
                            maWelcomeLine1,
                            DrawTextFlags::Center);
    rRenderContext.DrawText(tools::Rectangle(0, nY + 2 * nTextHeight, rSize.Width(), rSize.Height()),
                            maWelcomeLine2,
                            DrawTextFlags::MultiLine | DrawTextFlags::WordBreak | DrawTextFlags::Center);

    rRenderContext.Pop();
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

void RecentDocsView::PostLoadRecentUsedFile(LoadRecentFile* pLoadRecentFile)
{
    assert(!mpLoadRecentFile);
    mpLoadRecentFile = pLoadRecentFile;
    m_nExecuteHdlId = Application::PostUserEvent(LINK(this, RecentDocsView, ExecuteHdl_Impl), pLoadRecentFile);
}

void RecentDocsView::DispatchedLoadRecentUsedFile()
{
    mpLoadRecentFile = nullptr;
}

IMPL_LINK( RecentDocsView, ExecuteHdl_Impl, void*, p, void )
{
    m_nExecuteHdlId = nullptr;
    LoadRecentFile* pLoadRecentFile = static_cast<LoadRecentFile*>(p);
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

    if (pLoadRecentFile->pView)
    {
        pLoadRecentFile->pView->DispatchedLoadRecentUsedFile();
        pLoadRecentFile->pView->SetPointer(PointerStyle::Arrow);
        pLoadRecentFile->pView->Enable();
    }

    delete pLoadRecentFile;
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
