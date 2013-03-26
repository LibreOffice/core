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
#include <sfx2/sfxresid.hxx>
#include <sfx2/templatecontaineritem.hxx>
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

#include "../doc/doc.hrc"
#include "templateview.hrc"

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
      mnCurRegionId(0),
      maAllButton(this, SfxResId(BTN_ALL_TEMPLATES)),
      maFTName(this, SfxResId(FT_NAME))
{
    maAllButton.Hide();
    maAllButton.SetStyle(maAllButton.GetStyle() | WB_FLATBUTTON);
    maAllButton.SetClickHdl(LINK(this,TemplateAbstractView,ShowRootRegionHdl));
    maFTName.Hide();
}

TemplateAbstractView::TemplateAbstractView(Window *pParent, const ResId &rResId, bool bDisableTransientChildren)
    : ThumbnailView(pParent,rResId,bDisableTransientChildren),
      mnCurRegionId(0),
      maAllButton(this, SfxResId(BTN_ALL_TEMPLATES)),
      maFTName(this, SfxResId(FT_NAME))
{
    maAllButton.Hide();
    maAllButton.SetStyle(maAllButton.GetStyle() | WB_FLATBUTTON);
    maAllButton.SetClickHdl(LINK(this,TemplateAbstractView,ShowRootRegionHdl));
    maFTName.Hide();
}

TemplateAbstractView::~TemplateAbstractView ()
{
}

void TemplateAbstractView::insertItems(const std::vector<TemplateItemProperties> &rTemplates)
{
    std::vector<ThumbnailViewItem*> aItems(rTemplates.size());
    for (size_t i = 0, n = rTemplates.size(); i < n; ++i )
    {
        //TODO: CHECK IF THE ITEM IS A FOLDER OR NOT
        TemplateViewItem *pChild = new TemplateViewItem(*this);
        const TemplateItemProperties *pCur = &rTemplates[i];

        pChild->mnId = pCur->nId;
        pChild->mnDocId = pCur->nDocId;
        pChild->mnRegionId = pCur->nRegionId;
        pChild->maTitle = pCur->aName;
        pChild->setPath(pCur->aPath);
        pChild->maPreview1 = pCur->aThumbnail;

        if ( pCur->aThumbnail.IsEmpty() )
        {
            // Use the default thumbnail if we have nothing else
            pChild->maPreview1 = TemplateAbstractView::getDefaultThumbnail(pCur->aPath);
        }

        pChild->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

        aItems[i] = pChild;
    }

    updateItems(aItems);
}

sal_uInt16 TemplateAbstractView::getCurRegionId() const
{
    return mnCurRegionId;
}

const OUString &TemplateAbstractView::getCurRegionName() const
{
    return maCurRegionName;
}

bool TemplateAbstractView::isNonRootRegionVisible () const
{
    return mnCurRegionId;
}

void TemplateAbstractView::setOpenRegionHdl(const Link &rLink)
{
    maOpenRegionHdl = rLink;
}

void TemplateAbstractView::setOpenTemplateHdl(const Link &rLink)
{
    maOpenTemplateHdl = rLink;
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

BitmapEx TemplateAbstractView::getDefaultThumbnail( const OUString& rPath )
{
    INetURLObject aUrl(rPath);
    OUString aExt = aUrl.getExtension();

    BitmapEx aImg;
    if ( aExt == "ott" || aExt == "stw" || aExt == "oth" || aExt == "dot" || aExt == "dotx" )
    {
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_TEXT ) );
    }
    else if ( aExt == "ots" || aExt == "stc" || aExt == "xlt" || aExt == "xltm" || aExt == "xltx" )
    {
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_SHEET ) );
    }
    else if ( aExt == "otp" || aExt == "sti" || aExt == "pot" || aExt == "potm" || aExt == "potx" )
    {
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_PRESENTATION ) );
    }
    else if ( aExt == "otg" || aExt == "std" )
    {
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_DRAWING ) );
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

IMPL_LINK_NOARG(TemplateAbstractView, ShowRootRegionHdl)
{
    showRootRegion();
    return 0;
}

void TemplateAbstractView::OnItemDblClicked (ThumbnailViewItem *pItem)
{
    //Check if the item is a TemplateContainerItem (Folder) or a TemplateViewItem (File)

    TemplateContainerItem* pContainerItem = dynamic_cast<TemplateContainerItem*>(pItem);
    if ( pContainerItem )
    {
        // Fill templates

        mnCurRegionId = pContainerItem->mnRegionId+1;
        maCurRegionName = pContainerItem->maTitle;
        maFTName.SetText(maCurRegionName);
        showRegion(pItem);
    }
    else
    {
        maOpenTemplateHdl.Call(pItem);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
