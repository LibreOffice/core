/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateabstractview.hxx>
#include <sfx2/templatecontaineritem.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/templateview.hxx>
#include <sfx2/templateviewitem.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

bool ViewFilter_Application::isValid (const OUString &rPath) const
{
    bool bRet = true;

    INetURLObject aUrl(rPath);
    OUString aExt = aUrl.getExtension();
    if (mApp == FILTER_APP_WRITER)
    {
        bRet = aExt == "ott" || aExt == "stw" || aExt == "oth" || aExt == "dot" || aExt == "dotx";
    }
    else if (mApp == FILTER_APP_CALC)
    {
        bRet = aExt == "ots" || aExt == "stc" || aExt == "xlt" || aExt == "xltm" || aExt == "xltx";
    }
    else if (mApp == FILTER_APP_IMPRESS)
    {
        bRet = aExt == "otp" || aExt == "sti" || aExt == "pot" || aExt == "potm" || aExt == "potx";
    }
    else if (mApp == FILTER_APP_DRAW)
    {
        bRet = aExt == "otg" || aExt == "std";
    }

    return bRet;
}

bool ViewFilter_Application::operator () (const ThumbnailViewItem *pItem)
{
    const TemplateViewItem *pTempItem = dynamic_cast<const TemplateViewItem*>(pItem);
    if (pTempItem)
        return isValid(pTempItem->getPath());

    TemplateContainerItem *pContainerItem = const_cast<TemplateContainerItem*>(dynamic_cast<const TemplateContainerItem*>(pItem));
    if (pContainerItem)
    {
        std::vector<TemplateItemProperties> &rTemplates = pContainerItem->maTemplates;

        size_t nVisCount = 0;

        // Clear thumbnails
        pContainerItem->maPreview1.Clear();
        pContainerItem->maPreview2.Clear();
        pContainerItem->maPreview3.Clear();
        pContainerItem->maPreview4.Clear();

        for (size_t i = 0, n = rTemplates.size(); i < n && pContainerItem->HasMissingPreview(); ++i)
        {
            if (isValid(rTemplates[i].aPath))
            {
                ++nVisCount;
                if ( pContainerItem->maPreview1.IsEmpty( ) )
                {
                    pContainerItem->maPreview1 = TemplateAbstractView::scaleImg(rTemplates[i].aThumbnail,
                                                                       TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                                       TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
                }
                else if ( pContainerItem->maPreview2.IsEmpty() )
                {
                    pContainerItem->maPreview2 = TemplateAbstractView::scaleImg(rTemplates[i].aThumbnail,
                                                                       TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                                       TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
                }
                else if ( pContainerItem->maPreview3.IsEmpty() )
                {
                    pContainerItem->maPreview3 = TemplateAbstractView::scaleImg(rTemplates[i].aThumbnail,
                                                                       TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                                       TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
                }
                else if ( pContainerItem->maPreview4.IsEmpty() )
                {
                    pContainerItem->maPreview4 = TemplateAbstractView::scaleImg(rTemplates[i].aThumbnail,
                                                                       TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                                       TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
                }
            }
        }

        return mApp != FILTER_APP_NONE ? nVisCount : true ;
    }
    return true;
}

bool ViewFilter_Keyword::operator ()(const ThumbnailViewItem *pItem)
{
    assert(pItem);

    return pItem->maTitle.matchIgnoreAsciiCase(maKeyword);
}

TemplateAbstractView::TemplateAbstractView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : ThumbnailView(pParent,nWinStyle,bDisableTransientChildren),
      mpItemView(new TemplateView(pParent)),
      mbFilteredResults(false),
      meFilterOption(FILTER_APP_WRITER)
{
    mpItemView->setItemStateHdl(LINK(this,TemplateAbstractView,OverlayItemStateHdl));
}

TemplateAbstractView::TemplateAbstractView(Window *pParent, const ResId &rResId, bool bDisableTransientChildren)
    : ThumbnailView(pParent,rResId,bDisableTransientChildren),
      mpItemView(new TemplateView(pParent)),
      mbFilteredResults(false),
      meFilterOption(FILTER_APP_WRITER)
{
    mpItemView->setItemStateHdl(LINK(this,TemplateAbstractView,OverlayItemStateHdl));
}

TemplateAbstractView::~TemplateAbstractView ()
{
    delete mpItemView;
}

void TemplateAbstractView::setItemDimensions(long ItemWidth, long ThumbnailHeight, long DisplayHeight, int itemPadding)
{
    ThumbnailView::setItemDimensions(ItemWidth,ThumbnailHeight,DisplayHeight,itemPadding);

    mpItemView->setItemDimensions(ItemWidth,ThumbnailHeight,DisplayHeight,itemPadding);
}

sal_uInt16 TemplateAbstractView::getOverlayRegionId() const
{
    return mpItemView->getId();
}

const OUString &TemplateAbstractView::getOverlayName() const
{
    return mpItemView->getName();
}

bool TemplateAbstractView::isOverlayVisible () const
{
    return mpItemView->IsVisible();
}

void TemplateAbstractView::deselectOverlayItems()
{
    mpItemView->deselectItems();
}

void TemplateAbstractView::sortOverlayItems(const boost::function<bool (const ThumbnailViewItem*,
                                                                        const ThumbnailViewItem*) > &func)
{
    mpItemView->sortItems(func);
}

void TemplateAbstractView::filterTemplatesByApp (const FILTER_APPLICATION &eApp)
{
    meFilterOption = eApp;

    if (mpItemView->IsVisible())
    {
        mbFilteredResults = true;
        mpItemView->filterItems(ViewFilter_Application(eApp));
    }
    else
    {
        filterItems(ViewFilter_Application(eApp));
    }
}

void TemplateAbstractView::showOverlay (bool bVisible)
{
    Show(!bVisible);
    mpItemView->Show(bVisible);

    mpItemView->SetPosSizePixel(GetPosPixel(), GetSizePixel());
    mpItemView->SetStyle(GetStyle());

    mpItemView->GrabFocus();

    // Clear items is the overlay is closed.
    if (!bVisible)
    {
        // Check if the folder view needs to be filtered
        if (mbFilteredResults)
        {
            filterItems(ViewFilter_Application(meFilterOption));
        }

        mpItemView->Clear();
    }
}

void TemplateAbstractView::filterTemplatesByKeyword(const OUString &rKeyword)
{
    if (mpItemView->IsVisible())
        mpItemView->filterItems(ViewFilter_Keyword(rKeyword));
}

void TemplateAbstractView::setOpenHdl(const Link &rLink)
{
    maOpenHdl = rLink;
    mpItemView->setOpenHdl(rLink);
}

void TemplateAbstractView::setOverlayCloseHdl(const Link &rLink)
{
    mpItemView->setCloseHdl(rLink);
}

BitmapEx TemplateAbstractView::scaleImg (const BitmapEx &rImg, long width, long height)
{
    BitmapEx aImg = rImg;

    if ( !rImg.IsEmpty() )
    {

        const Size& aImgSize = aImg.GetSizePixel();
        double nRatio = double(aImgSize.getWidth()) / double(aImgSize.getHeight());

        long nDestWidth = aImgSize.getWidth();
        long nDestHeight = aImgSize.getHeight();

        // Which one side is the overflowing most?
        long nDistW = aImgSize.getWidth() - width;
        long nDistH = aImgSize.getHeight() - height;

        // Use the biggest overflow side to make it fit the destination
        if ( nDistW >= nDistH && nDistW > 0 )
        {
            nDestWidth = width;
            nDestHeight = width / nRatio;
        }
        else if ( nDistW < nDistH && nDistH > 0 )
        {
            nDestHeight = height;
            nDestWidth = height * nRatio;
        }

        aImg.Scale(Size(nDestWidth,nDestHeight));
    }

    return aImg;
}

BitmapEx TemplateAbstractView::fetchThumbnail (const OUString &msURL, long width, long height)
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;

    // Load the thumbnail from a template document.
    uno::Reference<io::XInputStream> xIStream;

    uno::Reference< uno::XComponentContext > xContext (comphelper::getProcessComponentContext());

    try
    {
        uno::Reference<lang::XSingleServiceFactory> xStorageFactory = embed::StorageFactory::create( xContext );

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
    catch (const uno::Exception& rException)
    {
        OSL_TRACE (
            "caught exception while trying to access tuhmbnail of %s: %s",
            ::rtl::OUStringToOString(msURL,
                RTL_TEXTENCODING_UTF8).getStr(),
            ::rtl::OUStringToOString(rException.Message,
                RTL_TEXTENCODING_UTF8).getStr());
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

    return TemplateAbstractView::scaleImg(aThumbnail,width,height);
}

void TemplateAbstractView::Resize()
{
    mpItemView->SetSizePixel(GetSizePixel());
    ThumbnailView::Resize();
}

void TemplateAbstractView::Paint(const Rectangle &rRect)
{
    if (!mpItemView->IsVisible())
        ThumbnailView::Paint(rRect);
}

void TemplateAbstractView::DrawItem(ThumbnailViewItem *pItem)
{
    if (!mpItemView->IsVisible())
        ThumbnailView::DrawItem(pItem);
}

IMPL_LINK(TemplateAbstractView, OverlayItemStateHdl, const ThumbnailViewItem*, pItem)
{
    maOverlayItemStateHdl.Call((void*)pItem);
    return 0;
}

void TemplateAbstractView::OnItemDblClicked (ThumbnailViewItem *pItem)
{
    TemplateContainerItem* pContainerItem = dynamic_cast<TemplateContainerItem*>(pItem);
    if ( pContainerItem )
    {
        // Fill templates
        sal_uInt16 nRegionId = pContainerItem->mnId-1;

        mpItemView->setId(nRegionId);
        mpItemView->setName(pContainerItem->maTitle);
        mpItemView->InsertItems(pContainerItem->maTemplates);

        mpItemView->filterItems(ViewFilter_Application(meFilterOption));

        showOverlay(true);
    }
    else
    {
        maOpenHdl.Call(pItem);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
