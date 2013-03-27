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
#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateviewitem.hxx>
#include <svl/inettype.hxx>
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

static void lcl_updateThumbnails (TemplateContainerItem *pItem);

TemplateLocalView::TemplateLocalView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren)
    : TemplateAbstractView(pParent,rResId,bDisableTransientChildren),
      mpDocTemplates(new SfxDocumentTemplates)
{
}

TemplateLocalView::~TemplateLocalView()
{
    for (size_t i = 0; i < maRegions.size(); ++i)
        delete maRegions[i];

    maRegions.clear();

    delete mpDocTemplates;
}

void TemplateLocalView::Populate ()
{
    for (size_t i = 0; i < maRegions.size(); ++i)
        delete maRegions[i];

    maRegions.clear();

    sal_uInt16 nCount = mpDocTemplates->GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString aRegionName(mpDocTemplates->GetFullRegionName(i));

        TemplateContainerItem* pItem = new TemplateContainerItem( *this );
        pItem->mnId = i+1;
        pItem->maTitle = aRegionName;
        pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

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
                                                                          TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                          TEMPLATE_THUMBNAIL_MAX_HEIGHT);

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
        sal_uInt16 nItemId = mnCurRegionId + 1;

        for (size_t i = 0; i < maRegions.size(); ++i)
        {
            if (maRegions[i]->mnId == nItemId)
            {
                showRegion(maRegions[i]);
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
    maCurRegionName = OUString();

    // Clone root region items so they dont get invalidated when we open another region
    std::vector<ThumbnailViewItem*> items(maRegions.size());
    for (int i = 0, n = maRegions.size(); i < n; ++i)
    {
        TemplateContainerItem *pCur = maRegions[i];
        TemplateContainerItem *pItem = new TemplateContainerItem(*this);
        pItem->mnId = pCur->mnId;
        pItem->maTitle = pCur->maTitle;
        pItem->maTemplates = pCur->maTemplates;
        pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

        items[i] = pItem;
    }

    maAllButton.Show(false);
    maFTName.Show(false);

    updateItems(items);

    maOpenRegionHdl.Call(NULL);
}

void TemplateLocalView::showRegion(ThumbnailViewItem *pItem)
{
    mnHeaderHeight = maAllButton.GetSizePixel().getHeight() + maAllButton.GetPosPixel().Y() * 2;

    mnCurRegionId = pItem->mnId-1;
    maCurRegionName = pItem->maTitle;
    maAllButton.Show(true);
    maFTName.Show(true);

    insertItems(reinterpret_cast<TemplateContainerItem*>(pItem)->maTemplates);

    maOpenRegionHdl.Call(NULL);
}

sal_uInt16 TemplateLocalView::getRegionId(size_t pos) const
{
    assert(pos < maRegions.size());

    return maRegions[pos]->mnId;
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
TemplateLocalView::getFilteredItems(const boost::function<bool (const TemplateItemProperties&) > &rFunc) const
{
    std::vector<TemplateItemProperties> aItems;

    for (size_t i = 0; i < maRegions.size(); ++i)
    {
        TemplateContainerItem *pFolderItem = static_cast<TemplateContainerItem*>(maRegions[i]);

        for (size_t j = 0; j < pFolderItem->maTemplates.size(); ++j)
        {
            if (rFunc(pFolderItem->maTemplates[j]))
                aItems.push_back(pFolderItem->maTemplates[j]);
        }
    }

    return aItems;
}

sal_uInt16 TemplateLocalView::createRegion(const OUString &rName)
{
    sal_uInt16 nRegionId = mpDocTemplates->GetRegionCount();    // Next regionId

    if (!mpDocTemplates->InsertDir(rName,nRegionId))
        return false;

    OUString aRegionName = rName;

    // Insert to the region cache list and to the thumbnail item list
    TemplateContainerItem* pItem = new TemplateContainerItem( *this );
    pItem->mnId = nRegionId+1;
    pItem->maTitle = aRegionName;
    pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

    maRegions.push_back(pItem);

    pItem = new TemplateContainerItem(*this);
    pItem->mnId = nRegionId + 1;
    pItem->maTitle = aRegionName;
    pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

    mItemList.push_back(pItem);

    CalculateItemPositions();
    Invalidate();

    return pItem->mnId;
}

bool TemplateLocalView::removeRegion(const sal_uInt16 nItemId)
{
    sal_uInt16 nRegionId = nItemId - 1;

    if (!mpDocTemplates->Delete(nRegionId,USHRT_MAX))
        return false;

    RemoveItem(nItemId);

    // Remove from the region cache list
    std::vector<TemplateContainerItem*>::iterator it;
    for ( it = maRegions.begin(); it != maRegions.end(); ++it )
    {
        if ( (*it)->mnId == nItemId )
        {
            delete *it;
            maRegions.erase(it);
            break;
        }
    }

    return true;
}

bool TemplateLocalView::removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId)
{
    sal_uInt16 nRegionId = nSrcItemId - 1;

    for (size_t i = 0, n = maRegions.size(); i < n; ++i)
    {
        if (maRegions[i]->mnId == nSrcItemId)
        {
            TemplateContainerItem *pItem = static_cast<TemplateContainerItem*>(maRegions[i]);
            std::vector<TemplateItemProperties>::iterator pIter;
            for (pIter = pItem->maTemplates.begin(); pIter != pItem->maTemplates.end(); ++pIter)
            {
                if (pIter->nId == nItemId)
                {
                    if (!mpDocTemplates->Delete(nRegionId,pIter->nDocId))
                        return false;

                    pIter = pItem->maTemplates.erase(pIter);

                    RemoveItem(nItemId);

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
    TemplateContainerItem *pTarget = NULL;
    TemplateContainerItem *pSrc = NULL;

    for (size_t i = 0, n = maRegions.size(); i < n; ++i)
    {
        if (maRegions[i]->mnId == nTargetItem)
            pTarget = static_cast<TemplateContainerItem*>(maRegions[i]);
        else if (maRegions[i]->mnId == nSrcItem)
            pSrc = static_cast<TemplateContainerItem*>(maRegions[i]);
    }

    if (pTarget && pSrc)
    {
        sal_uInt16 nSrcRegionId = nSrcItem-1;
        sal_uInt16 nTargetRegion = pTarget->mnId-1;
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
        aTemplateItem.aPath = pViewItem->getPath();
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
                    // Keep region document id syncronized with SfxDocumentTemplates
                    if (aIter->nDocId > pViewItem->mnDocId)
                        --aIter->nDocId;

                    ++aIter;
                }
            }

            // Keep view document id syncronized with SfxDocumentTemplates
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
    bool ret = true;
    bool refresh = false;

    sal_uInt16 nSrcRegionId = mnCurRegionId;
    sal_uInt16 nSrcRegionItemId = nSrcRegionId + 1;

    TemplateContainerItem *pTarget = NULL;
    TemplateContainerItem *pSrc = NULL;

    for (size_t i = 0, n = maRegions.size(); i < n; ++i)
    {
        if (maRegions[i]->mnId == nTargetItem)
            pTarget = static_cast<TemplateContainerItem*>(maRegions[i]);
        else if (maRegions[i]->mnId == nSrcRegionItemId)
            pSrc = static_cast<TemplateContainerItem*>(maRegions[i]);
    }

    if (pTarget && pSrc)
    {
        sal_uInt16 nTargetRegion = pTarget->mnId-1;
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
            aTemplateItem.aPath = pViewItem->getPath();
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
                        // Keep region document id syncronized with SfxDocumentTemplates
                        if (pPropIter->nDocId > pViewItem->mnDocId)
                            --pPropIter->nDocId;

                        ++pPropIter;
                    }
                }

                // Keep view document id syncronized with SfxDocumentTemplates
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
    sal_uInt16 nRegionId = nRegionItemId - 1;

    for (size_t i = 0, n = maRegions.size(); i < n; ++i)
    {
        if (maRegions[i]->mnId == nRegionItemId)
        {
            sal_uInt16 nId = 0;
            sal_uInt16 nDocId = 0;

            TemplateContainerItem *pRegionItem =
                    static_cast<TemplateContainerItem*>(maRegions[i]);

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

                TemplateContainerItem *pItem =
                        static_cast<TemplateContainerItem*>(maRegions[i]);

                pItem->maTemplates.push_back(aTemplate);

                lcl_updateThumbnails(pItem);

                return true;
            }

            break;
        }
    }

    return false;
}

bool TemplateLocalView::copyFrom (TemplateContainerItem *pItem, const OUString &rPath)
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

    for (size_t i = 0, n = maRegions.size(); i < n; ++i)
    {
        if (maRegions[i]->mnId == nRegionItemId)
        {
            TemplateContainerItem *pRegItem =
                    static_cast<TemplateContainerItem*>(maRegions[i]);

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

    for (size_t i = 0, n = maRegions.size(); i < n; ++i)
    {
        if (maRegions[i]->mnId == nItemId)
        {
            bRet = saveTemplateAs((const TemplateContainerItem*)maRegions[i],rModel,rName);
            break;
        }
    }

    return bRet;
}

bool TemplateLocalView::saveTemplateAs(const TemplateContainerItem *pDstItem,
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

bool TemplateLocalView::isTemplateNameUnique(const sal_uInt16 nRegionItemId, const OUString &rName) const
{
    for (size_t i = 0, n = maRegions.size(); i < n; ++i)
    {
        if (maRegions[i]->mnId == nRegionItemId)
        {
            TemplateContainerItem *pRegItem =
                    static_cast<TemplateContainerItem*>(maRegions[i]);

            std::vector<TemplateItemProperties>::iterator aIter;
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

bool TemplateLocalView::renameItem(ThumbnailViewItem* pItem, rtl::OUString sNewTitle)
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
        nRegionId = pContainerItem->mnId - 1;
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
