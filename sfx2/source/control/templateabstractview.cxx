/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateabstractview.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/templateview.hxx>
#include <sfx2/templateviewitem.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

bool ViewFilter_Application::operator () (const ThumbnailViewItem *pItem)
{
    const TemplateViewItem *pTempItem = static_cast<const TemplateViewItem*>(pItem);

    if (mApp == FILTER_APP_WRITER)
    {
        return pTempItem->getFileType() == "OpenDocument Text" ||
                pTempItem->getFileType() == "OpenDocument Text Template";
    }
    else if (mApp == FILTER_APP_CALC)
    {
        return pTempItem->getFileType() == "OpenDocument Spreadsheet" ||
                pTempItem->getFileType() == "OpenDocument Spreadsheet Template";
    }
    else if (mApp == FILTER_APP_IMPRESS)
    {
        return pTempItem->getFileType() == "OpenDocument Presentation" ||
                pTempItem->getFileType() == "OpenDocument Presentation Template";
    }
    else if (mApp == FILTER_APP_DRAW)
    {
        return pTempItem->getFileType() == "OpenDocument Drawing" ||
                pTempItem->getFileType() == "OpenDocument Drawing Template";
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
      mpItemView(new TemplateView(this))
{
    mpItemView->setItemStateHdl(LINK(this,TemplateAbstractView,OverlayItemStateHdl));
}

TemplateAbstractView::TemplateAbstractView(Window *pParent, const ResId &rResId, bool bDisableTransientChildren)
    : ThumbnailView(pParent,rResId,bDisableTransientChildren),
      mpItemView(new TemplateView(this))
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

void TemplateAbstractView::deselectOverlayItem(const sal_uInt16 nItemId)
{
    mpItemView->deselectItem(nItemId);
}

void TemplateAbstractView::sortOverlayItems(const boost::function<bool (const ThumbnailViewItem*,
                                                                        const ThumbnailViewItem*) > &func)
{
    mpItemView->sortItems(func);
}

void TemplateAbstractView::filterTemplatesByKeyword(const OUString &rKeyword)
{
    if (mpItemView->IsVisible())
        mpItemView->filterItems(ViewFilter_Keyword(rKeyword));
}

void TemplateAbstractView::setOverlayDblClickHdl(const Link &rLink)
{
    mpItemView->setDblClickHdl(rLink);
}

void TemplateAbstractView::setOverlayCloseHdl(const Link &rLink)
{
    mpItemView->setCloseHdl(rLink);
}

BitmapEx TemplateAbstractView::scaleImg (const BitmapEx &rImg, long width, long height)
{
    BitmapEx aImg = rImg;

    int sWidth = std::min(aImg.GetSizePixel().getWidth(),width);
    int sHeight = std::min(aImg.GetSizePixel().getHeight(),height);

    aImg.Scale(Size(sWidth,sHeight),BMP_SCALE_INTERPOLATE);

    return aImg;
}

BitmapEx TemplateAbstractView::fetchThumbnail (const rtl::OUString &msURL, long width, long height)
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;

    // Load the thumbnail from a template document.
    uno::Reference<io::XInputStream> xIStream;

    uno::Reference< lang::XMultiServiceFactory > xServiceManager (comphelper::getProcessServiceFactory());

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

    return TemplateAbstractView::scaleImg(aThumbnail,width,height);
}

void TemplateAbstractView::Resize()
{
    mpItemView->SetSizePixel(GetSizePixel());
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

void TemplateAbstractView::OnSelectionMode (bool bMode)
{
    if (mpItemView->IsVisible())
    {
        mbSelectionMode = bMode;
        mpItemView->setSelectionMode(bMode);
    }
    else
        ThumbnailView::OnSelectionMode(bMode);
}

IMPL_LINK(TemplateAbstractView, OverlayItemStateHdl, const ThumbnailViewItem*, pItem)
{
    maOverlayItemStateHdl.Call((void*)pItem);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
