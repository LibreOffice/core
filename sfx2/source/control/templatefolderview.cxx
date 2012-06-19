/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatefolderview.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/templateview.hxx>
#include <sfx2/templatefolderviewitem.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

BitmapEx lcl_fetchThumbnail (const rtl::OUString &msURL, int width, int height)
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

TemplateFolderView::TemplateFolderView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren)
    : ThumbnailView(pParent,rResId,bDisableTransientChildren),
      mpDocTemplates(new SfxDocumentTemplates),
      mpItemView(new TemplateView(this,mpDocTemplates))
{
    mpItemView->SetColor(Color(COL_WHITE));
    mpItemView->SetPosPixel(Point(0,0));
    mpItemView->SetSizePixel(GetOutputSizePixel());
    mpItemView->SetColCount(3);
    mpItemView->SetLineCount(2);
}

TemplateFolderView::~TemplateFolderView()
{
    delete mpItemView;
    delete mpDocTemplates;
}

void TemplateFolderView::Populate ()
{
    sal_uInt16 nCount = mpDocTemplates->GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        rtl::OUString aRegionName(mpDocTemplates->GetFullRegionName(i));

        sal_uInt16 nEntries = mpDocTemplates->GetCount(i);

        if (nEntries)
        {
            TemplateFolderViewItem* pItem = new TemplateFolderViewItem( *this, this );
            pItem->mnId = i+1;
            pItem->maText = aRegionName;
            pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnFolderSelected));

            /// Preview first 2 thumbnails for folder
            pItem->maPreview1 = lcl_fetchThumbnail(mpDocTemplates->GetPath(i,0),128,128);

            if ( nEntries > 2 )
                pItem->maPreview2 = lcl_fetchThumbnail(mpDocTemplates->GetPath(i,1),128,128);

            mItemList.push_back(pItem);
        }
    }

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

bool TemplateFolderView::isOverlayVisible () const
{
    return mpItemView->IsVisible();
}

void TemplateFolderView::showOverlay (bool bVisible)
{
    mpItemView->Show(bVisible);

    // Clear items is the overlay is closed.
    if (!bVisible)
        mpItemView->Clear();
}

void TemplateFolderView::OnSelectionMode (bool bMode)
{
    if (mpItemView->IsVisible())
    {
        mpItemView->setSelectionMode(bMode);

        for (size_t i = 0, n = mItemList.size(); i < n; ++i)
            mItemList[i]->setSelectionMode(bMode);
    }
    else
        ThumbnailView::OnSelectionMode(bMode);
}

void TemplateFolderView::OnItemDblClicked (ThumbnailViewItem *pRegionItem)
{
    // Fill templates
    sal_uInt16 nRegionId = pRegionItem->mnId-1;

    mpItemView->setRegionId(nRegionId);

    sal_uInt16 nEntries = mpDocTemplates->GetCount(nRegionId);
    for (sal_uInt16 i = 0; i < nEntries; ++i)
    {
        mpItemView->InsertItem(i+1,lcl_fetchThumbnail(mpDocTemplates->GetPath(nRegionId,i),128,128),
                               mpDocTemplates->GetName(nRegionId,i));
    }

    if (mbSelectionMode)
        mpItemView->setSelectionMode(true);

    mpItemView->Show();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


