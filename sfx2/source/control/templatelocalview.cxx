/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatelocalview.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/templatelocalviewitem.hxx>
#include <sfx2/templateview.hxx>
#include <sfx2/templateviewitem.hxx>
#include <svl/inettype.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

void lcl_updateThumbnails (TemplateLocalViewItem *pItem);

class FolderFilter_Application
{
public:

    FolderFilter_Application (FILTER_APPLICATION eApp)
        : meApp(eApp)
    {
    }

    bool operator () (const ThumbnailViewItem *pItem)
    {
        TemplateLocalViewItem *pFolderItem = (TemplateLocalViewItem*)pItem;

        std::vector<TemplateItemProperties> &rTemplates = pFolderItem->maTemplates;

        size_t nVisCount = 0;

        // Clear thumbnails
        pFolderItem->maPreview1.Clear();
        pFolderItem->maPreview2.Clear();

        for (size_t i = 0, n = rTemplates.size(); i < n; ++i)
        {
            if (isValid(rTemplates[i].aType))
            {
                ++nVisCount;

                // Update the thumbnails
                if (nVisCount == 1)
                    pFolderItem->maPreview1 = rTemplates[i].aThumbnail;
                else if (nVisCount == 2)
                    pFolderItem->maPreview2 = rTemplates[i].aThumbnail;
            }
        }

        return meApp != FILTER_APP_NONE ? nVisCount : true ;
    }

    bool isValid (const rtl::OUString &rType) const
    {
        bool bRet = true;

        if (meApp == FILTER_APP_WRITER)
        {
            bRet = rType == "OpenDocument Text" || rType == "OpenDocument Text Template";
        }
        else if (meApp == FILTER_APP_CALC)
        {
            bRet = rType == "OpenDocument Spreadsheet" || rType == "OpenDocument Spreadsheet Template";
        }
        else if (meApp == FILTER_APP_IMPRESS)
        {
            bRet = rType == "OpenDocument Presentation" || rType == "OpenDocument Presentation Template";
        }
        else if (meApp == FILTER_APP_DRAW)
        {
            bRet = rType == "OpenDocument Drawing" || rType == "OpenDocument Drawing Template";
        }

        return bRet;
    }

private:

    FILTER_APPLICATION meApp;
};

TemplateLocalView::TemplateLocalView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren)
    : TemplateAbstractView(pParent,rResId,bDisableTransientChildren),
      mbFilteredResults(false),
      meFilterOption(FILTER_APP_NONE),
      mpDocTemplates(new SfxDocumentTemplates)
{
    mpItemView->SetColor(Color(COL_WHITE));
    mpItemView->setChangeNameHdl(LINK(this,TemplateLocalView,ChangeNameHdl));
}

TemplateLocalView::~TemplateLocalView()
{
    delete mpDocTemplates;
}

void TemplateLocalView::Populate ()
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

        TemplateLocalViewItem* pItem = new TemplateLocalViewItem( *this, this );
        pItem->mnId = i+1;
        pItem->maTitle = aRegionName;
        pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

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

            TemplateItemProperties aProperties;;
            aProperties.nId = j+1;
            aProperties.nDocId = j;
            aProperties.nRegionId = i;
            aProperties.aName = aName;
            aProperties.aPath = aURL;
            aProperties.aType = aType;
            aProperties.aThumbnail = TemplateAbstractView::fetchThumbnail(aURL,
                                                                          TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                          TEMPLATE_THUMBNAIL_MAX_HEIGHT);

            pItem->maTemplates.push_back(aProperties);
        }

        lcl_updateThumbnails(pItem);

        mItemList.push_back(pItem);
    }

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void TemplateLocalView::reload ()
{
    mpDocTemplates->Update();

    Clear();

    Populate();

    if (mpItemView->IsVisible())
    {
        sal_uInt16 nItemId = mpItemView->getId() + 1;

        for (size_t i = 0; i < mItemList.size(); ++i)
        {
            if (mItemList[i]->mnId == nItemId)
            {
                mpItemView->Clear();
                mpItemView->InsertItems(static_cast<TemplateLocalViewItem*>(mItemList[i])->maTemplates);
                break;
            }
        }
    }
}

std::vector<rtl::OUString> TemplateLocalView::getFolderNames()
{
    size_t n = mItemList.size();
    std::vector<rtl::OUString> ret(n);

    for (size_t i = 0; i < n; ++i)
        ret[i] = mItemList[i]->maTitle;

    return ret;
}

void TemplateLocalView::showOverlay (bool bVisible)
{
    mpItemView->Show(bVisible);

    // Clear items is the overlay is closed.
    if (!bVisible)
    {
        // Check if the folder view needs to be filtered
        if (mbFilteredResults)
        {
            filterItems(FolderFilter_Application(meFilterOption));

            mbFilteredResults = false;
            meFilterOption = FILTER_APP_NONE;
        }

        mpItemView->Clear();

        setSelectionMode(mbSelectionMode);
    }
}

void TemplateLocalView::filterTemplatesByApp (const FILTER_APPLICATION &eApp)
{
    meFilterOption = eApp;

    if (mpItemView->IsVisible())
    {
        mbFilteredResults = true;
        mpItemView->filterItems(ViewFilter_Application(eApp));
    }
    else
    {
        filterItems(FolderFilter_Application(eApp));
    }
}

std::vector<TemplateItemProperties>
TemplateLocalView::getFilteredItems(const boost::function<bool (const TemplateItemProperties&) > &rFunc) const
{
    std::vector<TemplateItemProperties> aItems;

    for (size_t i = 0; i < mItemList.size(); ++i)
    {
        TemplateLocalViewItem *pFolderItem = static_cast<TemplateLocalViewItem*>(mItemList[i]);

        for (size_t j = 0; j < pFolderItem->maTemplates.size(); ++j)
        {
            if (rFunc(pFolderItem->maTemplates[j]))
                aItems.push_back(pFolderItem->maTemplates[j]);
        }
    }

    return aItems;
}

sal_uInt16 TemplateLocalView::createRegion(const rtl::OUString &rName)
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

    TemplateLocalViewItem* pItem = new TemplateLocalViewItem( *this, this );
    pItem->mnId = nRegionId+1;
    pItem->maTitle = aRegionName;
    pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

    mItemList.push_back(pItem);

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();

    return pItem->mnId;
}

bool TemplateLocalView::removeRegion(const sal_uInt16 nItemId)
{
    sal_uInt16 nRegionId = nItemId - 1;

    if (!mpDocTemplates->Delete(nRegionId,USHRT_MAX))
        return false;

    RemoveItem(nItemId);

    return true;
}

bool TemplateLocalView::removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId)
{
    sal_uInt16 nRegionId = nSrcItemId - 1;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->mnId == nSrcItemId)
        {
            TemplateLocalViewItem *pItem = static_cast<TemplateLocalViewItem*>(mItemList[i]);
            std::vector<TemplateItemProperties>::iterator pIter;
            for (pIter = pItem->maTemplates.begin(); pIter != pItem->maTemplates.end(); ++pIter)
            {
                if (pIter->nId == nItemId)
                {
                    if (!mpDocTemplates->Delete(nRegionId,pIter->nDocId))
                        return false;

                    pIter = pItem->maTemplates.erase(pIter);

                    mpItemView->RemoveItem(nItemId);

                    // Update Doc Idx for all templates that follow
                    for (; pIter != pItem->maTemplates.end(); ++pIter)
                        pIter->nDocId = pIter->nDocId - 1;

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

bool TemplateLocalView::moveTemplate (const ThumbnailViewItem *pItem, const sal_uInt16 nSrcItem,
                                       const sal_uInt16 nTargetItem, bool bCopy)
{
    bool bRet = true;
    bool bRefresh = false;

    TemplateLocalViewItem *pTarget = NULL;
    TemplateLocalViewItem *pSrc = NULL;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->mnId == nTargetItem)
            pTarget = static_cast<TemplateLocalViewItem*>(mItemList[i]);
        else if (mItemList[i]->mnId == nSrcItem)
            pSrc = static_cast<TemplateLocalViewItem*>(mItemList[i]);
    }

    if (pTarget && pSrc)
    {
        sal_uInt16 nSrcRegionId = nSrcItem-1;
        sal_uInt16 nTargetRegion = pTarget->mnId-1;
        sal_uInt16 nTargetIdx = mpDocTemplates->GetCount(nTargetRegion);    // Next Idx

        const TemplateViewItem *pViewItem = static_cast<const TemplateViewItem*>(pItem);

        bool bOK;

        if (bCopy)
            bOK = mpDocTemplates->Copy(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnId-1);
        else
            bOK = mpDocTemplates->Move(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnId-1);

        if (!bOK)
            return false;

        // move template to destination

        TemplateItemProperties aTemplateItem;
        aTemplateItem.nId = nTargetIdx + 1;
        aTemplateItem.nDocId = nTargetIdx;
        aTemplateItem.nRegionId = nTargetRegion;
        aTemplateItem.aName = pViewItem->maTitle;
        aTemplateItem.aPath = pViewItem->getPath();
        aTemplateItem.aType = pViewItem->getFileType();
        aTemplateItem.aThumbnail = pViewItem->maPreview1;

        pTarget->maTemplates.push_back(aTemplateItem);

        if (!bCopy)
        {
            // remove template from overlay and from cached data

            std::vector<TemplateItemProperties>::iterator aIter;
            for (aIter = pSrc->maTemplates.begin(); aIter != pSrc->maTemplates.end(); ++aIter)
            {
                if (aIter->nId == pViewItem->mnId)
                {
                    pSrc->maTemplates.erase(aIter);

                    mpItemView->RemoveItem(pViewItem->mnId);
                    break;
                }
            }
        }

        bRefresh = true;
    }
    else
        bRet = false;

    if (bRefresh)
    {
        lcl_updateThumbnails(pSrc);
        lcl_updateThumbnails(pTarget);

        CalculateItemPositions();

        if (IsReallyVisible() && IsUpdateMode())
        {
            Invalidate();
            mpItemView->Invalidate();
        }
    }

    return bRet;
}

bool TemplateLocalView::moveTemplates(std::set<const ThumbnailViewItem *> &rItems,
                                       const sal_uInt16 nTargetItem, bool bCopy)
{
    bool ret = true;
    bool refresh = false;

    sal_uInt16 nSrcRegionId = mpItemView->getId();
    sal_uInt16 nSrcRegionItemId = nSrcRegionId + 1;

    TemplateLocalViewItem *pTarget = NULL;
    TemplateLocalViewItem *pSrc = NULL;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->mnId == nTargetItem)
            pTarget = static_cast<TemplateLocalViewItem*>(mItemList[i]);
        else if (mItemList[i]->mnId == nSrcRegionItemId)
            pSrc = static_cast<TemplateLocalViewItem*>(mItemList[i]);
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

            TemplateItemProperties aTemplateItem;
            aTemplateItem.nId = nTargetIdx + 1;
            aTemplateItem.nDocId = nTargetIdx;
            aTemplateItem.nRegionId = nTargetRegion;
            aTemplateItem.aName = pViewItem->maTitle;
            aTemplateItem.aPath = pViewItem->getPath();
            aTemplateItem.aType = pViewItem->getFileType();
            aTemplateItem.aThumbnail = pViewItem->maPreview1;

            pTarget->maTemplates.push_back(aTemplateItem);

            if (!bCopy)
            {
                // remove template from overlay and from cached data

                std::vector<TemplateItemProperties>::iterator pIter;
                for (pIter = pSrc->maTemplates.begin(); pIter != pSrc->maTemplates.end(); ++pIter)
                {
                    if (pIter->nId == pViewItem->mnId)
                    {
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

bool TemplateLocalView::copyFrom(const sal_uInt16 nRegionItemId, const BitmapEx &rThumbnail,
                                  const OUString &rPath)
{
    sal_uInt16 nRegionId = nRegionItemId - 1;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->mnId == nRegionItemId)
        {
            sal_uInt16 nId = 0;
            sal_uInt16 nDocId = 0;

            TemplateLocalViewItem *pRegionItem =
                    static_cast<TemplateLocalViewItem*>(mItemList[i]);

            if (!pRegionItem->maTemplates.empty())
            {
                nId = (pRegionItem->maTemplates.back()).nId+1;
                nDocId = (pRegionItem->maTemplates.back()).nDocId+1;
            }

            String aPath(rPath);

            if (mpDocTemplates->CopyFrom(nRegionId,nDocId,aPath))
            {
                TemplateItemProperties aTemplate;
                aTemplate.nId = nId;
                aTemplate.nDocId = nDocId;
                aTemplate.nRegionId = nRegionId;
                aTemplate.aName = aPath;
                aTemplate.aThumbnail = rThumbnail;
                aTemplate.aPath = mpDocTemplates->GetPath(nRegionId,nDocId);
                aTemplate.aType = SvFileInformationManager::GetDescription(INetURLObject(aTemplate.aPath));

                TemplateLocalViewItem *pItem =
                        static_cast<TemplateLocalViewItem*>(mItemList[i]);

                pItem->maTemplates.push_back(aTemplate);

                lcl_updateThumbnails(pItem);

                return true;
            }

            break;
        }
    }

    return false;
}

bool TemplateLocalView::copyFrom (TemplateLocalViewItem *pItem, const rtl::OUString &rPath)
{
    sal_uInt16 nId = 0;
    sal_uInt16 nDocId = 0;
    sal_uInt16 nRegionId = pItem->mnId - 1;
    String aPath(rPath);

    if (!pItem->maTemplates.empty())
    {
        nId = (pItem->maTemplates.back()).nId+1;
        nDocId = (pItem->maTemplates.back()).nDocId+1;
    }

    if (mpDocTemplates->CopyFrom(nRegionId,nDocId,aPath))
    {
        TemplateItemProperties aTemplate;
        aTemplate.nId = nId;
        aTemplate.nDocId = nDocId;
        aTemplate.nRegionId = nRegionId;
        aTemplate.aName = aPath;
        aTemplate.aThumbnail = TemplateAbstractView::fetchThumbnail(rPath,
                                                                    TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                    TEMPLATE_THUMBNAIL_MAX_HEIGHT);
        aTemplate.aPath = rPath;
        aTemplate.aType = SvFileInformationManager::GetDescription(INetURLObject(rPath));

        pItem->maTemplates.push_back(aTemplate);

        lcl_updateThumbnails(pItem);

        CalculateItemPositions();

        return true;
    }

    return false;
}

bool TemplateLocalView::exportTo(const sal_uInt16 nItemId, const sal_uInt16 nRegionItemId, const OUString &rName)
{
    sal_uInt16 nRegionId = nRegionItemId - 1;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->mnId == nRegionItemId)
        {
            TemplateLocalViewItem *pRegItem =
                    static_cast<TemplateLocalViewItem*>(mItemList[i]);

            std::vector<TemplateItemProperties>::iterator aIter;
            for (aIter = pRegItem->maTemplates.begin(); aIter != pRegItem->maTemplates.end(); ++aIter)
            {
                if (aIter->nId == nItemId)
                {
                    if (!mpDocTemplates->CopyTo(nRegionId,aIter->nDocId,rName))
                        return false;

                    return true;
                }
            }

            break;
        }
    }

    return false;
}

bool TemplateLocalView::saveTemplateAs (sal_uInt16 nItemId,
                                        com::sun::star::uno::Reference<com::sun::star::frame::XModel> &rModel,
                                        const OUString &rName)
{
    bool bRet = false;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->mnId == nItemId)
        {
            bRet = saveTemplateAs((const TemplateLocalViewItem*)mItemList[i],rModel,rName);
            break;
        }
    }

    return bRet;
}

bool TemplateLocalView::saveTemplateAs(const TemplateLocalViewItem *pDstItem,
                                       com::sun::star::uno::Reference<com::sun::star::frame::XModel> &rModel,
                                       const OUString &rName)
{
    uno::Reference< frame::XStorable > xStorable(rModel, uno::UNO_QUERY_THROW );

    uno::Reference< frame::XDocumentTemplates > xTemplates(
                    frame::DocumentTemplates::create(comphelper::getProcessComponentContext()) );

    sal_uInt16 nRegionId = pDstItem->mnId-1;

    if (!xTemplates->storeTemplate(mpDocTemplates->GetRegionName(nRegionId),rName, xStorable ))
        return false;

    return true;
}

void TemplateLocalView::OnItemDblClicked (ThumbnailViewItem *pRegionItem)
{
    // Fill templates
    sal_uInt16 nRegionId = pRegionItem->mnId-1;

    mpItemView->setId(nRegionId);
    mpItemView->setName(mpDocTemplates->GetRegionName(nRegionId));
    mpItemView->InsertItems(static_cast<TemplateLocalViewItem*>(pRegionItem)->maTemplates);

    if (mbSelectionMode)
        mpItemView->setSelectionMode(true);

    mpItemView->filterItems(ViewFilter_Application(meFilterOption));

    mpItemView->Show();
}

IMPL_LINK(TemplateLocalView, ChangeNameHdl, TemplateView*, pView)
{
    sal_uInt16 nRegionId = pView->getId();
    sal_uInt16 nItemId = nRegionId + 1;

    if (!mpDocTemplates->SetName(pView->getName(),nRegionId,USHRT_MAX))
        return false;

    for (size_t i = 0; i < mItemList.size(); ++i)
    {
        if (mItemList[i]->mnId == nItemId)
        {
            mItemList[i]->maTitle = pView->getName();
            mItemList[i]->calculateItemsPosition(mnThumbnailHeight,mnDisplayHeight,
                                                 mnItemPadding,mpItemAttrs->nMaxTextLenght);
            Invalidate();
            break;
        }
    }

    return true;
}

void lcl_updateThumbnails (TemplateLocalViewItem *pItem)
{
    pItem->maPreview1.Clear();
    pItem->maPreview2.Clear();

    // Update folder thumbnails
    for (size_t i = 0, n = pItem->maTemplates.size(); i < 2 && i < n; ++i)
    {
        if (i == 0)
        {
            pItem->maPreview1 = TemplateAbstractView::scaleImg(pItem->maTemplates[i].aThumbnail,
                                                               TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                               TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
        }
        else
        {
            pItem->maPreview2 = TemplateAbstractView::scaleImg(pItem->maTemplates[i].aThumbnail,
                                                               TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                               TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
