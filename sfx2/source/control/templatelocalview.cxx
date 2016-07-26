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
#include <comphelper/string.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/inputdlg.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateviewitem.hxx>
#include <svl/inettype.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/pngread.hxx>
#include <vcl/layout.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "../doc/doc.hrc"

#define MNI_OPEN               1
#define MNI_EDIT               2
#define MNI_DEFAULT_TEMPLATE   3
#define MNI_DELETE             4
#define MNI_RENAME             5

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

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

    maAllTemplates.clear();

    delete mpDocTemplates;
    TemplateAbstractView::dispose();
}

void TemplateLocalView::Populate ()
{
    for (TemplateContainerItem* pRegion : maRegions)
        delete pRegion;

    maRegions.clear();

    maAllTemplates.clear();

    sal_uInt16 nCount = mpDocTemplates->GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString aRegionName(mpDocTemplates->GetFullRegionName(i));

        TemplateContainerItem* pItem = new TemplateContainerItem( i+1 );
        pItem->mnRegionId = i;
        pItem->maTitle = aRegionName;

        sal_uInt16 nEntries = mpDocTemplates->GetCount(i);

        for (sal_uInt16 j = 0; j < nEntries; ++j)
        {
            OUString aName = mpDocTemplates->GetName(i,j);
            OUString aURL = mpDocTemplates->GetPath(i,j);

            TemplateItemProperties aProperties;
            aProperties.nId = j+1;
            aProperties.nDocId = j;
            aProperties.nRegionId = i;
            aProperties.aName = aName;
            aProperties.aPath = aURL;
            aProperties.aRegionName = aRegionName;
            aProperties.aThumbnail = TemplateAbstractView::fetchThumbnail(aURL,
                                                                          getThumbnailWidth(),
                                                                          getThumbnailHeight());

            pItem->maTemplates.push_back(aProperties);
            maAllTemplates.push_back(aProperties);
        }

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
        showAllTemplates();

    //No items should be selected by default
    deselectItems();
}

void TemplateLocalView::showAllTemplates()
{
    mnCurRegionId = 0;
    maCurRegionName.clear();

    insertItems(maAllTemplates, false, true);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateLocalView::showRegion(TemplateContainerItem *pItem)
{
    mnCurRegionId = pItem->mnRegionId+1;
    maCurRegionName = pItem->maTitle;

    insertItems((pItem)->maTemplates);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateLocalView::showRegion(const OUString &rName)
{
    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->maTitle == rName)
        {
            showRegion(pRegion);
            break;
        }
    }
}

TemplateContainerItem* TemplateLocalView::getRegion(OUString const & rName)
{
    for (TemplateContainerItem* pRegion : maRegions)
        if (pRegion->maTitle == rName)
            return pRegion;

    return nullptr;
}

void TemplateLocalView::createContextMenu(const bool bIsDefault)
{
    std::unique_ptr<PopupMenu> pItemMenu(new PopupMenu);
    pItemMenu->InsertItem(MNI_OPEN,SfxResId(STR_OPEN).toString());
    pItemMenu->InsertItem(MNI_EDIT,SfxResId(STR_EDIT_TEMPLATE).toString());

    if(!bIsDefault)
        pItemMenu->InsertItem(MNI_DEFAULT_TEMPLATE,SfxResId(STR_DEFAULT_TEMPLATE).toString());
    else
        pItemMenu->InsertItem(MNI_DEFAULT_TEMPLATE,SfxResId(STR_RESET_DEFAULT).toString());

    pItemMenu->InsertSeparator();
    pItemMenu->InsertItem(MNI_RENAME,SfxResId(STR_RENAME).toString());
    pItemMenu->InsertItem(MNI_DELETE,SfxResId(STR_DELETE).toString());
    pItemMenu->InsertSeparator();
    deselectItems();
    maSelectedItem->setSelection(true);
    maItemStateHdl.Call(maSelectedItem);
    pItemMenu->SetSelectHdl(LINK(this, TemplateLocalView, ContextMenuSelectHdl));
    pItemMenu->Execute(this, Rectangle(maPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
    Invalidate();
}

IMPL_LINK_TYPED(TemplateLocalView, ContextMenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case MNI_OPEN:
        maOpenTemplateHdl.Call(maSelectedItem);
        break;
    case MNI_EDIT:
        maEditTemplateHdl.Call(maSelectedItem);
        break;
    case MNI_RENAME:
    {
        ScopedVclPtrInstance< InputDialog > m_pTitleEditDlg( SfxResId(STR_RENAME_TEMPLATE).toString(), this);
        OUString sOldTitle = maSelectedItem->getTitle();
        m_pTitleEditDlg->SetEntryText( sOldTitle );
        m_pTitleEditDlg->HideHelpBtn();

        if(!m_pTitleEditDlg->Execute())
            break;
        OUString sNewTitle = comphelper::string::strip( m_pTitleEditDlg->GetEntryText(), ' ');

        if ( !sNewTitle.isEmpty() && sNewTitle != sOldTitle )
        {
            maSelectedItem->setTitle(sNewTitle);
        }
    }
        break;
    case MNI_DELETE:
    {
        ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE), VclMessageType::VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
        if ( aQueryDlg->Execute() != RET_YES )
            break;

        maDeleteTemplateHdl.Call(maSelectedItem);
        reload();
    }
        break;
    case MNI_DEFAULT_TEMPLATE:
        maDefaultTemplateHdl.Call(maSelectedItem);
        break;
    default:
        break;
    }

    return false;
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

sal_uInt16 TemplateLocalView::getRegionId(OUString const & sRegion) const
{
    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->maTitle == sRegion)
            return pRegion->mnId;
    }

    return 0;
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
    TemplateContainerItem* pItem = new TemplateContainerItem( nItemId );
    pItem->mnRegionId = nRegionId;
    pItem->maTitle = rName;

    maRegions.push_back(pItem);

    return pItem->mnId;
}

bool TemplateLocalView::renameRegion(const OUString &rTitle, const OUString &rNewTitle)
{
    sal_uInt16 nDocId = USHRT_MAX;
    TemplateContainerItem *pRegion = getRegion(rTitle);

    if(pRegion)
    {
        sal_uInt16 nRegionId = pRegion->mnRegionId;
        return mpDocTemplates->SetName( rNewTitle, nRegionId, nDocId );
    }
    return false;
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
    std::vector<TemplateContainerItem*>::iterator pRegionIter = maRegions.begin();
    for ( ; pRegionIter != maRegions.end(); ++pRegionIter)
    {
        if ((*pRegionIter)->mnRegionId > nRegionId)
            --(*pRegionIter)->mnRegionId;
    }

    return true;
}

bool TemplateLocalView::removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId)
{
    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnId == nSrcItemId)
        {
            TemplateContainerItem *pItem = pRegion;
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
            pTarget = pRegion;
        else if (pRegion->mnId == nSrcItem)
            pSrc = pRegion;
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
            OUString sQuery = (OUString(SfxResId(STR_MSG_QUERY_COPY).toString()).replaceFirst("$1", pViewItem->maTitle)).replaceFirst("$2",
                getRegionName(nTargetRegion));
            ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, sQuery, VclMessageType::VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
            if ( aQueryDlg->Execute() != RET_YES )
                return false;

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
        aTemplateItem.aRegionName = pViewItem->maHelpText;
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

    TemplateContainerItem *pTarget = nullptr;
    TemplateContainerItem *pSrc = nullptr;

    for (TemplateContainerItem* pRegion : maRegions)
    {
        if (pRegion->mnId == nTargetItem)
            pTarget = pRegion;
    }

    if (pTarget)
    {
        bool refresh = false;

        sal_uInt16 nTargetRegion = pTarget->mnRegionId;
        sal_uInt16 nTargetIdx = mpDocTemplates->GetCount(nTargetRegion);    // Next Idx
        std::vector<sal_uInt16> aItemIds;    // List of moved items ids (also prevents the invalidation of rItems iterators when we remove them as we go)

        std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator aSelIter;
        for ( aSelIter = rItems.begin(); aSelIter != rItems.end(); ++aSelIter, ++nTargetIdx )
        {
            const TemplateViewItem *pViewItem = static_cast<const TemplateViewItem*>(*aSelIter);
            sal_uInt16 nSrcRegionId = pViewItem->mnRegionId;

            for (TemplateContainerItem* pRegion : maRegions)
            {
                if (pRegion->mnRegionId == nSrcRegionId)
                    pSrc = pRegion;
            }

            if(pSrc)
            {
                bool bCopy = !mpDocTemplates->Move(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId);

                if (bCopy)
                {
                    OUString sQuery = (OUString(SfxResId(STR_MSG_QUERY_COPY).toString()).replaceFirst("$1", pViewItem->maTitle)).replaceFirst("$2",
                        getRegionName(nTargetRegion));
                    ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, sQuery, VclMessageType::VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

                    if ( aQueryDlg->Execute() != RET_YES )
                    {
                        OUString sMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE).toString());
                        sMsg = sMsg.replaceFirst("$1",getRegionName(nTargetRegion));
                        ScopedVclPtrInstance<MessageDialog>(this, sMsg.replaceFirst( "$2",pViewItem->maTitle))->Execute();

                        return false; //return if any single move operation fails
                    }

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
                aTemplateItem.aRegionName = pViewItem->maHelpText;
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
                            aItemIds.push_back(pViewItem->mnDocId + 1);//mnid
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
            }

            refresh = true;
        }

        // Remove items from the current view
        for (std::vector<sal_uInt16>::iterator it = aItemIds.begin(); it != aItemIds.end(); ++it)
            RemoveItem(*it);

        if (refresh)
        {
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
                    pRegion;

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
                aTemplate.aRegionName = getRegionName(nRegionId);
                aTemplate.aThumbnail = rThumbnail;
                aTemplate.aPath = mpDocTemplates->GetPath(nRegionId,nDocId);

                TemplateContainerItem *pItem = pRegion;

                pItem->maTemplates.push_back(aTemplate);

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
    aTemplate.nId = nId;
    aTemplate.nDocId = nDocId;
    aTemplate.nRegionId = nRegionId;
    aTemplate.aName = aPath;
    aTemplate.aThumbnail = TemplateAbstractView::fetchThumbnail(rPath,
                                                                TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                TEMPLATE_THUMBNAIL_MAX_HEIGHT);
    aTemplate.aPath = rPath;
    aTemplate.aRegionName = getRegionName(nRegionId);

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
        aTemplate.nId = nId;
        aTemplate.nDocId = nDocId;
        aTemplate.nRegionId = nRegionId;
        aTemplate.aName = aPath;
        aTemplate.aThumbnail = TemplateAbstractView::fetchThumbnail(rPath,
                                                                    TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                    TEMPLATE_THUMBNAIL_MAX_HEIGHT);
        aTemplate.aPath = rPath;
        aTemplate.aRegionName = getRegionName(nRegionId);

        pItem->maTemplates.push_back(aTemplate);

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

bool TemplateLocalView::renameItem(ThumbnailViewItem* pItem, const OUString& sNewTitle)
{
    sal_uInt16 nRegionId = 0;
    sal_uInt16 nDocId = USHRT_MAX;
    TemplateViewItem* pDocItem = dynamic_cast<TemplateViewItem*>( pItem );

    if ( pDocItem )
    {
        nRegionId = pDocItem->mnRegionId;
        nDocId = pDocItem->mnDocId;
    }

    return mpDocTemplates->SetName( sNewTitle, nRegionId, nDocId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
