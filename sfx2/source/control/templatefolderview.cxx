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
#include <sfx2/templateviewitem.hxx>
#include <sfx2/templatefolderviewitem.hxx>
#include <svl/inettype.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#define ITEM_MAX_WIDTH 192
#define ITEM_MAX_HEIGHT 192
#define ITEM_PADDING 5
#define ITEM_SPACE 20
#define THUMBNAIL_MAX_HEIGHT 128 - 2*ITEM_PADDING
#define THUMBNAIL_MAX_WIDTH ITEM_MAX_WIDTH - 2*ITEM_PADDING

#define INIT_VIEW_COLS 3
#define INIT_VIEW_LINES 1

void lcl_updateThumbnails (TemplateFolderViewItem *pItem);

BitmapEx lcl_ScaleImg (const BitmapEx &rImg, long width, long height)
{
    BitmapEx aImg = rImg;

    int sWidth = std::min(aImg.GetSizePixel().getWidth(),width);
    int sHeight = std::min(aImg.GetSizePixel().getHeight(),height);

    aImg.Scale(Size(sWidth,sHeight),BMP_SCALE_INTERPOLATE);

    return aImg;
}

BitmapEx lcl_fetchThumbnail (const rtl::OUString &msURL, long width, long height)
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

    return lcl_ScaleImg(aThumbnail,width,height);
}

// Display template items depending on the generator application
class ViewFilter_Application
{
public:

    ViewFilter_Application (SfxDocumentTemplates *pDocTemplates, FILTER_APPLICATION App)
        : mApp(App), mpDocTemplates(pDocTemplates)
    {}

    bool operator () (const ThumbnailViewItem *pItem)
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

private:

    FILTER_APPLICATION mApp;
    SfxDocumentTemplates *mpDocTemplates;
};

class FolderFilter_Application
{
public:

    FolderFilter_Application ( SfxDocumentTemplates *pDocTemplates, FILTER_APPLICATION eApp)
        : meApp(eApp)
    {
        maFilterFunc = ViewFilter_Application(pDocTemplates,eApp);
    }

    bool operator () (const ThumbnailViewItem *pItem)
    {
        TemplateFolderViewItem *pFolderItem = (TemplateFolderViewItem*)pItem;

        std::vector<TemplateViewItem*> &rTemplates = pFolderItem->maTemplates;

        size_t nVisCount = 0;
        ThumbnailViewItem *pTemplateItem;

        // Clear thumbnails
        pFolderItem->maPreview1.Clear();
        pFolderItem->maPreview2.Clear();

        for (size_t i = 0, n = rTemplates.size(); i < n; ++i)
        {
            pTemplateItem = rTemplates[i];

            if (maFilterFunc(pTemplateItem))
            {
                ++nVisCount;

                // Update the thumbnails
                if (nVisCount == 1)
                    pFolderItem->maPreview1 = pTemplateItem->maPreview1;
                else if (nVisCount == 2)
                    pFolderItem->maPreview2 = pTemplateItem->maPreview1;
            }
        }

        return nVisCount;
    }

private:

    FILTER_APPLICATION meApp;
    boost::function<bool (const ThumbnailViewItem*)> maFilterFunc;
};

TemplateFolderView::TemplateFolderView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren)
    : ThumbnailView(pParent,rResId,bDisableTransientChildren),
      mbFilteredResults(false),
      meFilterOption(FILTER_APP_NONE),
      mpDocTemplates(new SfxDocumentTemplates),
      mpItemView(new TemplateView(this,mpDocTemplates))
{
    Size aViewSize = mpItemView->CalcWindowSizePixel(INIT_VIEW_COLS,INIT_VIEW_LINES,
                                                     ITEM_MAX_WIDTH,ITEM_MAX_HEIGHT,ITEM_SPACE);

    mpItemView->SetColor(Color(COL_WHITE));
    mpItemView->SetSizePixel(aViewSize);
    mpItemView->setItemDimensions(ITEM_MAX_WIDTH,THUMBNAIL_MAX_HEIGHT,
                                  ITEM_MAX_HEIGHT-THUMBNAIL_MAX_HEIGHT,
                                  ITEM_PADDING);

    mpItemView->setItemStateHdl(LINK(this,TemplateFolderView,TVTemplateStateHdl));
    mpItemView->setChangeNameHdl(LINK(this,TemplateFolderView,ChangeNameHdl));
    mpItemView->setCloseHdl(LINK(this,TemplateFolderView,OverlayCloseHdl));
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

        if ((sal_uInt32)aRegionName.getLength() > mpItemAttrs->nMaxTextLenght)
        {
            aRegionName = aRegionName.copy(0,mpItemAttrs->nMaxTextLenght-3);
            aRegionName += "...";
        }

        TemplateFolderViewItem* pItem = new TemplateFolderViewItem( *this, this );
        pItem->mnId = i+1;
        pItem->maText = aRegionName;
        pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnFolderSelected));

        sal_uInt16 nEntries = mpDocTemplates->GetCount(i);

        for (sal_uInt16 j = 0; j < nEntries; ++j)
        {
            rtl::OUString aName = mpDocTemplates->GetName(i,j);
            rtl::OUString aURL = mpDocTemplates->GetPath(i,j);
            rtl::OUString aType = SvFileInformationManager::GetDescription(INetURLObject(aURL));

            if ((sal_uInt32)aName.getLength() > mpItemAttrs->nMaxTextLenght)
            {
                aName = aName.copy(0,mpItemAttrs->nMaxTextLenght-3);
                aName += "...";
            }

            TemplateViewItem *pTemplateItem = new TemplateViewItem(*mpItemView,mpItemView);
            pTemplateItem->mnId = j+1;
            pTemplateItem->maText = aName;
            pTemplateItem->setPath(aURL);
            pTemplateItem->setFileType(aType);
            pTemplateItem->maPreview1 = lcl_fetchThumbnail(aURL,THUMBNAIL_MAX_WIDTH,THUMBNAIL_MAX_HEIGHT);

            pItem->maTemplates.push_back(pTemplateItem);
        }

        lcl_updateThumbnails(pItem);

        mItemList.push_back(pItem);
    }

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

std::vector<rtl::OUString> TemplateFolderView::getFolderNames()
{
    size_t n = mItemList.size();
    std::vector<rtl::OUString> ret(n);

    for (size_t i = 0; i < n; ++i)
        ret[i] = mItemList[i]->maText;

    return ret;
}

bool TemplateFolderView::isOverlayVisible () const
{
    return mpItemView->IsVisible();
}

void TemplateFolderView::showOverlay (bool bVisible)
{
    mbActive = !bVisible;
    mpItemView->Show(bVisible);

    // Clear items is the overlay is closed.
    if (!bVisible)
    {
        // Check if the folder view needs to be filtered
        if (mbFilteredResults)
        {
            filterItems(FolderFilter_Application(mpDocTemplates,meFilterOption));

            mbFilteredResults = false;
            meFilterOption = FILTER_APP_NONE;
        }

        mpItemView->Clear();

        setSelectionMode(mbSelectionMode);
    }
}

void TemplateFolderView::setOverlayDblClickHdl(const Link &rLink)
{
    mpItemView->setDblClickHdl(rLink);
}

void TemplateFolderView::filterTemplatesByApp (const FILTER_APPLICATION &eApp)
{
    meFilterOption = eApp;

    if (mpItemView->IsVisible())
    {
        mbFilteredResults = true;
        mpItemView->filterItems(ViewFilter_Application(mpDocTemplates,eApp));
    }
    else
    {
        filterItems(FolderFilter_Application(mpDocTemplates,eApp));
    }
}

std::vector<std::pair<sal_uInt16,std::vector<ThumbnailViewItem*> > >
TemplateFolderView::getFilteredItems(const boost::function<bool (const ThumbnailViewItem*) > &rFunc) const
{
    std::vector<ThumbnailViewItem*> aRegionItems;
    std::vector<std::pair<sal_uInt16,std::vector<ThumbnailViewItem*> > > aItems;

    for (size_t i = 0; i < mItemList.size(); ++i)
    {
        TemplateFolderViewItem *pFolderItem = static_cast<TemplateFolderViewItem*>(mItemList[i]);

        sal_uInt16 nRegionId = pFolderItem->mnId-1;

        for (size_t j = 0; j < pFolderItem->maTemplates.size(); ++j)
        {
            if (rFunc(pFolderItem->maTemplates[j]))
                aRegionItems.push_back(pFolderItem->maTemplates[j]);
        }

        aItems.push_back(std::make_pair(nRegionId,aRegionItems));
        aRegionItems.clear();
    }

    return aItems;
}

void TemplateFolderView::sortOverlayItems(const boost::function<bool (const ThumbnailViewItem*,
                                                                      const ThumbnailViewItem*) > &func)
{
    mpItemView->sortItems(func);
}

sal_uInt16 TemplateFolderView::createRegion(const rtl::OUString &rName)
{
    sal_uInt16 nRegionId = mpDocTemplates->GetRegionCount();    // Next regionId

    if (!mpDocTemplates->InsertDir(rName,nRegionId))
        return false;

    rtl::OUString aRegionName = rName;

    if ((sal_uInt32)aRegionName.getLength() > mpItemAttrs->nMaxTextLenght)
    {
        aRegionName = aRegionName.copy(0,mpItemAttrs->nMaxTextLenght-3);
        aRegionName += "...";
    }

    TemplateFolderViewItem* pItem = new TemplateFolderViewItem( *this, this );
    pItem->mnId = nRegionId+1;
    pItem->maText = aRegionName;
    pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnFolderSelected));

    mItemList.push_back(pItem);

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();

    return true;
}

bool TemplateFolderView::removeTemplate (const sal_uInt16 nItemId)
{
    sal_uInt16 nRegionId = mpItemView->getRegionId();
    sal_uInt16 nItemRegionId = nRegionId + 1;
    sal_uInt16 nTemplateId = nItemId - 1;

    if (!mpDocTemplates->Delete(nRegionId,nTemplateId))
        return false;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->mnId == nItemRegionId)
        {

            TemplateFolderViewItem *pItem = static_cast<TemplateFolderViewItem*>(mItemList[i]);
            std::vector<TemplateViewItem*>::iterator pIter;
            for (pIter = pItem->maTemplates.begin(); pIter != pItem->maTemplates.end(); ++pIter)
            {
                if ((*pIter)->mnId == nItemId)
                {
                    delete *pIter;

                    pItem->maTemplates.erase(pIter);

                    mpItemView->RemoveItem(nItemId);

                    break;
                }
            }

            lcl_updateThumbnails(pItem);

            CalculateItemPositions();

            break;
        }
    }

    return true;
}

bool TemplateFolderView::moveTemplates(std::set<const ThumbnailViewItem *> &rItems,
                                       const sal_uInt16 nTargetItem, bool bCopy)
{
    bool ret = true;
    bool refresh = false;

    sal_uInt16 nSrcRegionId = mpItemView->getRegionId();
    sal_uInt16 nSrcRegionItemId = nSrcRegionId + 1;

    TemplateFolderViewItem *pTarget = NULL;
    TemplateFolderViewItem *pSrc = NULL;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->mnId == nTargetItem)
            pTarget = static_cast<TemplateFolderViewItem*>(mItemList[i]);
        else if (mItemList[i]->mnId == nSrcRegionItemId)
            pSrc = static_cast<TemplateFolderViewItem*>(mItemList[i]);
    }

    if (pTarget && pSrc)
    {
        sal_uInt16 nTargetRegion = pTarget->mnId-1;
        sal_uInt16 nTargetIdx = mpDocTemplates->GetCount(nTargetRegion);    // Next Idx

        std::set<const ThumbnailViewItem*>::iterator aSelIter;
        for ( aSelIter = rItems.begin(); aSelIter != rItems.end(); ++aSelIter, ++nTargetIdx )
        {
            const TemplateViewItem *pViewItem = static_cast<const TemplateViewItem*>(*aSelIter);

            bool bOK;

            if (bCopy)
                bOK = mpDocTemplates->Copy(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnId-1);
            else
                bOK = mpDocTemplates->Move(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnId-1);

            if (!bOK)
            {
                ret = false;
                continue;
            }

            // move template to destination

            TemplateViewItem *pTemplateItem = new TemplateViewItem(*mpItemView,mpItemView);
            pTemplateItem->mnId = nTargetIdx + 1;
            pTemplateItem->maText = pViewItem->maText;
            pTemplateItem->setPath(pViewItem->getPath());
            pTemplateItem->setFileType(pViewItem->getFileType());
            pTemplateItem->maPreview1 = pViewItem->maPreview1;

            pTarget->maTemplates.push_back(pTemplateItem);

            if (!bCopy)
            {
                // remove template for overlay and from cached data

                std::vector<TemplateViewItem*>::iterator pIter;
                for (pIter = pSrc->maTemplates.begin(); pIter != pSrc->maTemplates.end(); ++pIter)
                {
                    if ((*pIter)->mnId == pViewItem->mnId)
                    {
                        delete *pIter;

                        pSrc->maTemplates.erase(pIter);

                        mpItemView->RemoveItem(pViewItem->mnId);
                        break;
                    }
                }
            }

            refresh = true;
        }
    }
    else
        ret = false;

    if (refresh)
    {
        lcl_updateThumbnails(pSrc);
        lcl_updateThumbnails(pTarget);

        CalculateItemPositions();

        Invalidate();
        mpItemView->Invalidate();
    }

    return ret;
}

void TemplateFolderView::copyFrom (TemplateFolderViewItem *pItem, const rtl::OUString &rPath)
{
    sal_uInt16 nId = 0;
    sal_uInt16 nRegionId = pItem->mnId - 1;
    String aPath(rPath);

    if (!mItemList.empty())
        nId = (mItemList.back())->mnId+1;

    if (mpDocTemplates->CopyFrom(nRegionId,nId,aPath))
    {
        TemplateViewItem *pTemplate = new TemplateViewItem(*mpItemView,mpItemView);
        pTemplate->mnId = nId;
        pTemplate->maText = mpDocTemplates->GetName(nRegionId,nId);
        pTemplate->maPreview1 = lcl_fetchThumbnail(rPath,128,128);
        pTemplate->setPath(rPath);
        pTemplate->setFileType(SvFileInformationManager::GetDescription(INetURLObject(rPath)));

        pItem->maTemplates.push_back(pTemplate);

        lcl_updateThumbnails(pItem);

        CalculateItemPositions();
    }
}

void TemplateFolderView::Resize()
{
    Size aWinSize = GetOutputSize();
    Size aViewSize = mpItemView->GetSizePixel();

    Point aPos;
    aPos.setX((aWinSize.getWidth() - aViewSize.getWidth())/2);

    mpItemView->SetPosPixel(aPos);
}

void TemplateFolderView::OnSelectionMode (bool bMode)
{
    if (mpItemView->IsVisible())
    {
        mbSelectionMode = bMode;
        mpItemView->setSelectionMode(bMode);
    }
    else
        ThumbnailView::OnSelectionMode(bMode);
}

void TemplateFolderView::OnItemDblClicked (ThumbnailViewItem *pRegionItem)
{
    // Fill templates
    sal_uInt16 nRegionId = pRegionItem->mnId-1;

    mpItemView->setRegionId(nRegionId);
    mpItemView->InsertItems(static_cast<TemplateFolderViewItem*>(pRegionItem)->maTemplates);

    if (mbSelectionMode)
        mpItemView->setSelectionMode(true);

    if (meFilterOption != FILTER_APP_NONE)
        mpItemView->filterItems(ViewFilter_Application(mpDocTemplates,meFilterOption));

    mbActive = false;
    mpItemView->Show();
}

IMPL_LINK(TemplateFolderView, TVTemplateStateHdl, const ThumbnailViewItem*, pItem)
{
    maTemplateStateHdl.Call((void*)pItem);
    return 0;
}

IMPL_LINK(TemplateFolderView, ChangeNameHdl, TemplateView*, pView)
{
    sal_uInt16 nRegionId = pView->getRegionId();
    sal_uInt16 nItemId = nRegionId + 1;

    if (!mpDocTemplates->SetName(pView->getRegionName(),nRegionId,USHRT_MAX))
        return false;

    for (size_t i = 0; i < mItemList.size(); ++i)
    {
        if (mItemList[i]->mnId == nItemId)
        {
            mItemList[i]->maText = pView->getRegionName();
            mItemList[i]->calculateItemsPosition(mpItemAttrs->nMaxTextLenght);
            Invalidate();
            break;
        }
    }

    return true;
}

IMPL_LINK_NOARG(TemplateFolderView, OverlayCloseHdl)
{
    showOverlay(false);
    return 0;
}

void lcl_updateThumbnails (TemplateFolderViewItem *pItem)
{
    // Update folder thumbnails
    for (size_t i = 0, n = pItem->maTemplates.size(); i < 2 && i < n; ++i)
    {
        if (i == 0)
        {
            pItem->maPreview1 = lcl_ScaleImg(pItem->maTemplates[i]->maPreview1,
                                            THUMBNAIL_MAX_WIDTH*0.75,
                                            THUMBNAIL_MAX_HEIGHT*0.75);
        }
        else
        {
            pItem->maPreview2 = lcl_ScaleImg(pItem->maTemplates[i]->maPreview1,
                                            THUMBNAIL_MAX_WIDTH*0.75,
                                            THUMBNAIL_MAX_HEIGHT*0.75);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
