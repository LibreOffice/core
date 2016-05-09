/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatelocalview.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateviewitem.hxx>
#include <svl/inettype.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/builderfactory.hxx>
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

static void lcl_updateThumbnails (TemplateContainerItem *pItem);

TemplateLocalView::TemplateLocalView ( vcl::Window* pParent)
    : TemplateAbstractView(pParent),
      mpDocTemplates(new SfxDocumentTemplates)
{
}

VCL_BUILDER_FACTORY(TemplateLocalView)

TemplateLocalView::~TemplateLocalView()
{
    disposeOnce();
}

void TemplateLocalView::dispose()
{
    for ( TemplateContainerItem* pRegion : maRegions)
        delete pRegion;

    maRegions.clear();

    delete mpDocTemplates;
    TemplateAbstractView::dispose();
}

void TemplateLocalView::Populate ()
{
    for (TemplateContainerItem* pRegion : maRegions)
        delete pRegion;

    maRegions.clear();

    sal_uInt16 nCount = mpDocTemplates->GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString aRegionName(mpDocTemplates->GetFullRegionName(i));

        TemplateContainerItem* pItem = new TemplateContainerItem( *this, i+1 );
        pItem->mnRegionId = i;
        pItem->maTitle = aRegionName;

        sal_uInt16 nEntries = mpDocTemplates->GetCount(i);

        for (sal_uInt16 j = 0; j < nEntries; ++j)
        {
            OUString aName = mpDocTemplates->GetName(i,j);
            OUString aURL = mpDocTemplates->GetPath(i,j);

            TemplateItemProperties aProperties;
            aProperties.aIsFolder = false;          // Flat hierarchy for the local filesystem (no nested folders)
            aProperties.nId = j+1;
            aProperties.nDocId = j;
            aProperties.nRegionId = i;
            aProperties.aName = aName;
            aProperties.aPath = aURL;
            aProperties.aThumbnail = TemplateAbstractView::fetchThumbnail(aURL,
                                                                          getThumbnailWidth(),
                                                                          getThumbnailHeight());

            pItem->maTemplates.push_back(aProperties);
        }

        lcl_updateThumbnails(pItem);

        maRegions.push_back(pItem);
    }
}

void TemplateLocalView::reload ()
{
    mpDocTemplates->Update();

    Populate();

    // Check if we are currently browsing a region or root folder
    if (mnCurRegionId)
    {
        sal_uInt16 nRegionId = mnCurRegionId - 1;   //Is offset by 1

        for (TemplateContainerItem* pRegion : maRegions)
        {
            if (pRegion->mnRegionId == nRegionId)
            {
                showRegion(pRegion);
                break;
            }
        }
    }
    else
        showRootRegion();
}

void TemplateLocalView::showRootRegion()
{
    mnHeaderHeight = 0;
    mnCurRegionId = 0;
    maCurRegionName.clear();

    // Clone root region items so they don't get invalidated when we open another region
    std::vector<ThumbnailViewItem*> items(maRegions.size());
    for (int i = 0, n = maRegions.size(); i < n; ++i)
    {
        TemplateContainerItem *pCur = maRegions[i];
        TemplateContainerItem *pItem = new TemplateContainerItem(*this, pCur->mnId);
        pItem->mnRegionId = pCur->mnRegionId;
        pItem->maTitle = pCur->maTitle;
        pItem->maTemplates = pCur->maTemplates;

        items[i] = pItem;
    }

    maAllButton->Show(false);
    maFTName->Show(false);

    updateItems(items);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateLocalView::showRegion(ThumbnailViewItem *pItem)
{
    mnHeaderHeight = maAllButton->GetSizePixel().getHeight() + maAllButton->GetPosPixel().Y() * 2;

    mnCurRegionId = static_cast<TemplateContainerItem*>(pItem)->mnRegionId+1;
    maCurRegionName = pItem->maTitle;
    maAllButton->Show();
    maFTName->Show();

    insertItems(reinterpret_cast<TemplateContainerItem*>(pItem)->maTemplates);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateLocalView::showRegion(const OUString &rName)
{
    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->maTitle == rName)
        {
            maFTName->SetText(rName);
            showRegion(pRegion);
            break;
        }
    }
}

sal_uInt16 TemplateLocalView::getCurRegionItemId() const
{
    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnRegionId == mnCurRegionId-1)
            return pRegion->mnId;
    }

    return 0;
}

sal_uInt16 TemplateLocalView::getRegionId(size_t pos) const
{
    assert(pos < maRegions.size());

    return maRegions[pos]->mnId;
}

OUString TemplateLocalView::getRegionName(const sal_uInt16 nRegionId) const
{
    return mpDocTemplates->GetRegionName(nRegionId);
}

OUString TemplateLocalView::getRegionItemName(const sal_uInt16 nItemId) const
{
    for (const TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnId == nItemId)
            return pRegion->maTitle;
    }

    return OUString();
}

std::vector<OUString> TemplateLocalView::getFolderNames()
{
    size_t n = maRegions.size();
    std::vector<OUString> ret(n);

    for (size_t i = 0; i < n; ++i)
        ret[i] = maRegions[i]->maTitle;

    return ret;
}

std::vector<TemplateItemProperties>
TemplateLocalView::getFilteredItems(const std::function<bool (const TemplateItemProperties&)> &rFunc) const
{
    std::vector<TemplateItemProperties> aItems;

    if (mnCurRegionId)
    {
        TemplateContainerItem *pFolderItem = maRegions[mnCurRegionId-1];

        for (TemplateItemProperties & rItemProps : pFolderItem->maTemplates)
        {
            if (rFunc(rItemProps))
                aItems.push_back(rItemProps);
        }
    }
    else
    {
        for (const TemplateContainerItem* pFolderItem : maRegions)
        {
            for (const TemplateItemProperties & rItemProps : pFolderItem->maTemplates)
            {
                if (rFunc(rItemProps))
                    aItems.push_back(rItemProps);
            }
        }
    }

    return aItems;
}

sal_uInt16 TemplateLocalView::createRegion(const OUString &rName)
{
    sal_uInt16 nRegionId = mpDocTemplates->GetRegionCount();    // Next regionId
    sal_uInt16 nItemId = getNextItemId();

    if (!mpDocTemplates->InsertDir(rName,nRegionId))
        return 0;

    // Insert to the region cache list and to the thumbnail item list
    TemplateContainerItem* pItem = new TemplateContainerItem( *this, nItemId );
    pItem->mnRegionId = nRegionId;
    pItem->maTitle = rName;

    maRegions.push_back(pItem);

    pItem = new TemplateContainerItem(*this, nItemId);
    pItem->mnRegionId = nRegionId;
    pItem->maTitle = rName;

    AppendItem(pItem);

    CalculateItemPositions();
    Invalidate();

    return pItem->mnId;
}

bool TemplateLocalView::isNestedRegionAllowed() const
{
    return !mnCurRegionId;
}

bool TemplateLocalView::isImportAllowed() const
{
    return mnCurRegionId;
}

bool TemplateLocalView::removeRegion(const sal_uInt16 nItemId)
{
    sal_uInt16 nRegionId = USHRT_MAX;

    // Remove from the region cache list
    std::vector<TemplateContainerItem*>::iterator pRegionIt;
    for ( pRegionIt = maRegions.begin(); pRegionIt != maRegions.end();)
    {
        if ( (*pRegionIt)->mnId == nItemId )
        {
            if (!mpDocTemplates->Delete((*pRegionIt)->mnRegionId,USHRT_MAX))
                return false;

            nRegionId = (*pRegionIt)->mnRegionId;

            delete *pRegionIt;
            pRegionIt = maRegions.erase(pRegionIt);
        }
        else
        {
            // Synchronize regions cache ids with SfxDocumentTemplates
            if (nRegionId != USHRT_MAX && (*pRegionIt)->mnRegionId > nRegionId)
                --(*pRegionIt)->mnRegionId;

            ++pRegionIt;
        }
    }

    if (nRegionId == USHRT_MAX)
        return false;

    // Synchronize view regions ids with SfxDocumentTemplates
    std::vector<ThumbnailViewItem*>::iterator pViewIt = mItemList.begin();
    for ( pViewIt = mItemList.begin(); pViewIt != mItemList.end(); ++pViewIt)
    {
        if (static_cast<TemplateContainerItem*>(*pViewIt)->mnRegionId > nRegionId)
            --static_cast<TemplateContainerItem*>(*pViewIt)->mnRegionId;
    }

    RemoveItem(nItemId);

    return true;
}

bool TemplateLocalView::removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId)
{
    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnId == nSrcItemId)
        {
            TemplateContainerItem *pItem = static_cast<TemplateContainerItem*>(pRegion);
            std::vector<TemplateItemProperties>::iterator pIter;
            for (pIter = pItem->maTemplates.begin(); pIter != pItem->maTemplates.end(); ++pIter)
            {
                if (pIter->nId == nItemId)
                {
                    if (!mpDocTemplates->Delete(pItem->mnRegionId,pIter->nDocId))
                        return false;

                    pIter = pItem->maTemplates.erase(pIter);

                    if (pRegion->mnRegionId == mnCurRegionId-1)
                    {
                        RemoveItem(nItemId);
                        Invalidate();
                    }

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
                                       const sal_uInt16 nTargetItem)
{
    TemplateContainerItem *pTarget = nullptr;
    TemplateContainerItem *pSrc = nullptr;

    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnId == nTargetItem)
            pTarget = static_cast<TemplateContainerItem*>(pRegion);
        else if (pRegion->mnId == nSrcItem)
            pSrc = static_cast<TemplateContainerItem*>(pRegion);
    }

    if (pTarget && pSrc)
    {
        sal_uInt16 nSrcRegionId = pSrc->mnRegionId;
        sal_uInt16 nTargetRegion = pTarget->mnRegionId;
        sal_uInt16 nTargetIdx = mpDocTemplates->GetCount(nTargetRegion);    // Next Idx

        const TemplateViewItem *pViewItem = static_cast<const TemplateViewItem*>(pItem);

        bool bCopy = !mpDocTemplates->Move(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId);

        if (bCopy)
        {
            if (!mpDocTemplates->Copy(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId))
                return false;
        }
        // move template to destination

        TemplateItemProperties aTemplateItem;
        aTemplateItem.nId = nTargetIdx + 1;
        aTemplateItem.nDocId = nTargetIdx;
        aTemplateItem.nRegionId = nTargetRegion;
        aTemplateItem.aName = pViewItem->maTitle;
        aTemplateItem.aPath = mpDocTemplates->GetPath(nTargetRegion,nTargetIdx);
        aTemplateItem.aThumbnail = pViewItem->maPreview1;

        pTarget->maTemplates.push_back(aTemplateItem);

        if (!bCopy)
        {
            // remove template from region cached data

            std::vector<TemplateItemProperties>::iterator aIter;
            for (aIter = pSrc->maTemplates.begin(); aIter != pSrc->maTemplates.end();)
            {
                if (aIter->nDocId == pViewItem->mnDocId)
                {
                    aIter = pSrc->maTemplates.erase(aIter);
                }
                else
                {
                    // Keep region document id synchronized with SfxDocumentTemplates
                    if (aIter->nDocId > pViewItem->mnDocId)
                        --aIter->nDocId;

                    ++aIter;
                }
            }

            // Keep view document id synchronized with SfxDocumentTemplates
            std::vector<ThumbnailViewItem*>::iterator pItemIter = mItemList.begin();
            for (; pItemIter != mItemList.end(); ++pItemIter)
            {
                if (static_cast<TemplateViewItem*>(*pItemIter)->mnDocId > pViewItem->mnDocId)
                    --static_cast<TemplateViewItem*>(*pItemIter)->mnDocId;
            }
        }

        lcl_updateThumbnails(pSrc);
        lcl_updateThumbnails(pTarget);

        CalculateItemPositions();
        Invalidate();

        return true;
    }

    return false;
}

bool TemplateLocalView::moveTemplates(const std::set<const ThumbnailViewItem*, selection_cmp_fn> &rItems,
                                      const sal_uInt16 nTargetItem)
{
    assert(mnCurRegionId);  // Only allowed in non root regions

    bool ret = true;

    sal_uInt16 nSrcRegionId = mnCurRegionId-1;

    TemplateContainerItem *pTarget = nullptr;
    TemplateContainerItem *pSrc = nullptr;

    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnId == nTargetItem)
            pTarget = static_cast<TemplateContainerItem*>(pRegion);
        else if (pRegion->mnRegionId == nSrcRegionId)
            pSrc = static_cast<TemplateContainerItem*>(pRegion);
    }

    if (pTarget && pSrc)
    {
        bool refresh = false;

        sal_uInt16 nTargetRegion = pTarget->mnRegionId;
        sal_uInt16 nTargetIdx = mpDocTemplates->GetCount(nTargetRegion);    // Next Idx
        std::vector<sal_uInt16> aItemIds;    // List of moved items ids (also prevents the invalidation of rItems iterators when we remove them as we go)

        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator aSelIter;
        for ( aSelIter = rItems.begin(); aSelIter != rItems.end(); ++aSelIter, ++nTargetIdx )
        {
            const TemplateViewItem *pViewItem = static_cast<const TemplateViewItem*>(*aSelIter);

            bool bCopy = !mpDocTemplates->Move(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId);

            if (bCopy)
            {
                if (!mpDocTemplates->Copy(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId))
                {
                    ret = false;
                    continue;
                }
            }

            // move template to destination

            TemplateItemProperties aTemplateItem;
            aTemplateItem.nId = nTargetIdx + 1;
            aTemplateItem.nDocId = nTargetIdx;
            aTemplateItem.nRegionId = nTargetRegion;
            aTemplateItem.aName = pViewItem->maTitle;
            aTemplateItem.aPath = mpDocTemplates->GetPath(nTargetRegion,nTargetIdx);
            aTemplateItem.aThumbnail = pViewItem->maPreview1;

            pTarget->maTemplates.push_back(aTemplateItem);

            if (!bCopy)
            {
                // remove template from region cached data

                std::vector<TemplateItemProperties>::iterator pPropIter;
                for (pPropIter = pSrc->maTemplates.begin(); pPropIter != pSrc->maTemplates.end();)
                {
                    if (pPropIter->nDocId == pViewItem->mnDocId)
                    {
                        pPropIter = pSrc->maTemplates.erase(pPropIter);
                        aItemIds.push_back(pViewItem->mnId);
                    }
                    else
                    {
                        // Keep region document id synchronized with SfxDocumentTemplates
                        if (pPropIter->nDocId > pViewItem->mnDocId)
                            --pPropIter->nDocId;

                        ++pPropIter;
                    }
                }

                // Keep view document id synchronized with SfxDocumentTemplates
                std::vector<ThumbnailViewItem*>::iterator pItemIter = mItemList.begin();
                for (; pItemIter != mItemList.end(); ++pItemIter)
                {
                    if (static_cast<TemplateViewItem*>(*pItemIter)->mnDocId > pViewItem->mnDocId)
                        --static_cast<TemplateViewItem*>(*pItemIter)->mnDocId;
                }
            }

            refresh = true;
        }

        // Remove items from the current view
        for (std::vector<sal_uInt16>::iterator it = aItemIds.begin(); it != aItemIds.end(); ++it)
            RemoveItem(*it);

        if (refresh)
        {
            lcl_updateThumbnails(pSrc);
            lcl_updateThumbnails(pTarget);

            CalculateItemPositions();
            Invalidate();
        }
    }
    else
        ret = false;

    return ret;
}

bool TemplateLocalView::copyFrom(const sal_uInt16 nRegionItemId, const BitmapEx &rThumbnail,
                                  const OUString &rPath)
{
    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnId == nRegionItemId)
        {
            sal_uInt16 nId = 0;
            sal_uInt16 nDocId = 0;

            TemplateContainerItem *pRegionItem =
                    static_cast<TemplateContainerItem*>(pRegion);

            if (!pRegionItem->maTemplates.empty())
            {
                nId = (pRegionItem->maTemplates.back()).nId+1;
                nDocId = (pRegionItem->maTemplates.back()).nDocId+1;
            }

            OUString aPath(rPath);
            sal_uInt16 nRegionId = pRegion->mnRegionId;

            if (mpDocTemplates->CopyFrom(nRegionId,nDocId,aPath))
            {
                TemplateItemProperties aTemplate;
                aTemplate.nId = nId;
                aTemplate.nDocId = nDocId;
                aTemplate.nRegionId = nRegionId;
                aTemplate.aName = aPath;
                aTemplate.aThumbnail = rThumbnail;
                aTemplate.aPath = mpDocTemplates->GetPath(nRegionId,nDocId);

                TemplateContainerItem *pItem =
                        static_cast<TemplateContainerItem*>(pRegion);

                pItem->maTemplates.push_back(aTemplate);

                lcl_updateThumbnails(pItem);

                return true;
            }

            break;
        }
    }

    return false;
}

bool TemplateLocalView::copyFrom(const OUString &rPath)
{
    assert(mnCurRegionId);

    TemplateContainerItem *pRegItem = maRegions[mnCurRegionId-1];

    sal_uInt16 nId = getNextItemId();
    sal_uInt16 nDocId = 0;
    sal_uInt16 nRegionId = pRegItem->mnRegionId;

    OUString aPath(rPath);

    if (!pRegItem->maTemplates.empty())
        nDocId = (pRegItem->maTemplates.back()).nDocId+1;

    if (!mpDocTemplates->CopyFrom(nRegionId,nDocId,aPath))
        return false;

    TemplateItemProperties aTemplate;
    aTemplate.aIsFolder = false;
    aTemplate.nId = nId;
    aTemplate.nDocId = nDocId;
    aTemplate.nRegionId = nRegionId;
    aTemplate.aName = aPath;
    aTemplate.aThumbnail = TemplateAbstractView::fetchThumbnail(rPath,
                                                                TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                TEMPLATE_THUMBNAIL_MAX_HEIGHT);
    aTemplate.aPath = rPath;

    pRegItem->maTemplates.push_back(aTemplate);

    insertItem(aTemplate);

    return true;
}

bool TemplateLocalView::copyFrom (TemplateContainerItem *pItem, const OUString &rPath)
{
    sal_uInt16 nId = 1;
    sal_uInt16 nDocId = 0;
    sal_uInt16 nRegionId = pItem->mnRegionId;
    OUString aPath(rPath);

    if (!pItem->maTemplates.empty())
    {
        nId = (pItem->maTemplates.back()).nId+1;
        nDocId = (pItem->maTemplates.back()).nDocId+1;
    }

    if (mpDocTemplates->CopyFrom(nRegionId,nDocId,aPath))
    {
        TemplateItemProperties aTemplate;
        aTemplate.aIsFolder = false;
        aTemplate.nId = nId;
        aTemplate.nDocId = nDocId;
        aTemplate.nRegionId = nRegionId;
        aTemplate.aName = aPath;
        aTemplate.aThumbnail = TemplateAbstractView::fetchThumbnail(rPath,
                                                                    TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                    TEMPLATE_THUMBNAIL_MAX_HEIGHT);
        aTemplate.aPath = rPath;

        pItem->maTemplates.push_back(aTemplate);

        lcl_updateThumbnails(pItem);

        CalculateItemPositions();

        return true;
    }

    return false;
}

bool TemplateLocalView::exportTo(const sal_uInt16 nItemId, const sal_uInt16 nRegionItemId, const OUString &rName)
{
    for (TemplateContainerItem* pRegItem : maRegions)
    {
        if (pRegItem->mnId == nRegionItemId)
        {
            std::vector<TemplateItemProperties>::iterator aIter;
            for (aIter = pRegItem->maTemplates.begin(); aIter != pRegItem->maTemplates.end(); ++aIter)
            {
                if (aIter->nId == nItemId)
                {
                    if (!mpDocTemplates->CopyTo(pRegItem->mnRegionId,aIter->nDocId,rName))
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
                                        css::uno::Reference<css::frame::XModel> &rModel,
                                        const OUString &rName)
{

    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnId == nItemId)
        {
            uno::Reference< frame::XStorable > xStorable(rModel, uno::UNO_QUERY_THROW );

            uno::Reference< frame::XDocumentTemplates > xTemplates(
                            frame::DocumentTemplates::create(comphelper::getProcessComponentContext()) );

            if (!xTemplates->storeTemplate(mpDocTemplates->GetRegionName(pRegion->mnRegionId),rName, xStorable ))
                return false;

            sal_uInt16 nDocId = pRegion->maTemplates.size();

            OUString aURL = mpDocTemplates->GetTemplateTargetURLFromComponent(mpDocTemplates->GetRegionName(pRegion->mnRegionId),rName);

            if(!mpDocTemplates->InsertTemplate(pRegion->mnRegionId,nDocId,rName,aURL))
                return false;


            TemplateItemProperties aTemplate;
            aTemplate.aIsFolder = false;
            aTemplate.nId = getNextItemId();
            aTemplate.nDocId = nDocId;
            aTemplate.nRegionId = pRegion->mnRegionId;
            aTemplate.aName = rName;
            aTemplate.aThumbnail = TemplateAbstractView::fetchThumbnail(aURL,
                                                                        TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                        TEMPLATE_THUMBNAIL_MAX_HEIGHT);
            aTemplate.aPath = aURL;

            pRegion->maTemplates.push_back(aTemplate);

            insertItem(aTemplate);

            return true;
        }
    }

    return false;
}

bool TemplateLocalView::saveTemplateAs(TemplateContainerItem *pDstItem,
                                       css::uno::Reference<css::frame::XModel> &rModel,
                                       const OUString &rName)
{
    uno::Reference< frame::XStorable > xStorable(rModel, uno::UNO_QUERY_THROW );

    uno::Reference< frame::XDocumentTemplates > xTemplates(
                    frame::DocumentTemplates::create(comphelper::getProcessComponentContext()) );

    if (!xTemplates->storeTemplate(mpDocTemplates->GetRegionName(pDstItem->mnRegionId),rName, xStorable ))
        return false;

    sal_uInt16 nDocId = pDstItem->maTemplates.size();
    OUString aURL = mpDocTemplates->GetTemplateTargetURLFromComponent(mpDocTemplates->GetRegionName(pDstItem->mnRegionId),rName);

    if(!mpDocTemplates->InsertTemplate(pDstItem->mnRegionId,nDocId,rName,aURL))
        return false;

    TemplateItemProperties aTemplate;
    aTemplate.aIsFolder = false;
    aTemplate.nId = pDstItem->maTemplates.empty() ? 1 : pDstItem->maTemplates.back().nId+1;
    aTemplate.nDocId = nDocId;
    aTemplate.nRegionId = pDstItem->mnRegionId;
    aTemplate.aName = rName;
    aTemplate.aThumbnail = TemplateAbstractView::fetchThumbnail(aURL,
                                                                TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                TEMPLATE_THUMBNAIL_MAX_HEIGHT);
    aTemplate.aPath = aURL;

    pDstItem->maTemplates.push_back(aTemplate);

    return true;
}

bool TemplateLocalView::isTemplateNameUnique(const sal_uInt16 nRegionItemId, const OUString &rName) const
{
    for (const TemplateContainerItem* pRegItem : maRegions)
    {
        if (pRegItem->mnId == nRegionItemId)
        {
            std::vector<TemplateItemProperties>::const_iterator aIter;
            for (aIter = pRegItem->maTemplates.begin(); aIter != pRegItem->maTemplates.end(); ++aIter)
            {
                if (aIter->aName == rName)
                    return false;
            }

            break;
        }
    }

    return true;
}

bool TemplateLocalView::renameItem(ThumbnailViewItem* pItem, const OUString& sNewTitle)
{
    sal_uInt16 nRegionId = 0;
    sal_uInt16 nDocId = USHRT_MAX;
    TemplateViewItem* pDocItem = dynamic_cast<TemplateViewItem*>( pItem );
    TemplateContainerItem* pContainerItem = dynamic_cast<TemplateContainerItem*>( pItem );
    if ( pDocItem )
    {
        nRegionId = pDocItem->mnRegionId;
        nDocId = pDocItem->mnDocId;
    }
    else if ( pContainerItem )
    {
        nRegionId = pContainerItem->mnRegionId;
    }
    return mpDocTemplates->SetName( sNewTitle, nRegionId, nDocId );
}

static void lcl_updateThumbnails (TemplateContainerItem *pItem)
{
    pItem->maPreview1.Clear();
    pItem->maPreview2.Clear();
    pItem->maPreview3.Clear();
    pItem->maPreview4.Clear();

    // Update folder thumbnails
    for (size_t i = 0, n = pItem->maTemplates.size(); i < n && pItem->HasMissingPreview(); ++i)
    {
        if ( pItem->maPreview1.IsEmpty( ) )
        {
            pItem->maPreview1 = TemplateAbstractView::scaleImg(pItem->maTemplates[i].aThumbnail,
                                                               TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                               TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
        }
        else if ( pItem->maPreview2.IsEmpty() )
        {
            pItem->maPreview2 = TemplateAbstractView::scaleImg(pItem->maTemplates[i].aThumbnail,
                                                               TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                               TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
        }
        else if ( pItem->maPreview3.IsEmpty() )
        {
            pItem->maPreview3 = TemplateAbstractView::scaleImg(pItem->maTemplates[i].aThumbnail,
                                                               TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                               TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
        }
        else if ( pItem->maPreview4.IsEmpty() )
        {
            pItem->maPreview4 = TemplateAbstractView::scaleImg(pItem->maTemplates[i].aThumbnail,
                                                               TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                               TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
