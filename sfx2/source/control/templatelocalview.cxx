/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatelocalview.hxx>

#include <comphelper/string.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/inputdlg.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateviewitem.hxx>
#include <sfx2/docfac.hxx>
#include <svl/inettype.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <sfx2/strings.hrc>
#include <bitmaps.hlst>

#define MNI_OPEN               1
#define MNI_EDIT               2
#define MNI_DEFAULT_TEMPLATE   3
#define MNI_DELETE             4
#define MNI_RENAME             5

using namespace ::com::sun::star;

bool ViewFilter_Application::isFilteredExtension(FILTER_APPLICATION filter, const OUString &rExt)
{
    bool bRet = rExt == "ott" || rExt == "stw" || rExt == "oth" || rExt == "dot" || rExt == "dotx" || rExt == "otm"
          || rExt == "ots" || rExt == "stc" || rExt == "xlt" || rExt == "xltm" || rExt == "xltx"
          || rExt == "otp" || rExt == "sti" || rExt == "pot" || rExt == "potm" || rExt == "potx"
          || rExt == "otg" || rExt == "std";

    if (filter == FILTER_APPLICATION::WRITER)
    {
        bRet = rExt == "ott" || rExt == "stw" || rExt == "oth" || rExt == "dot" || rExt == "dotx" || rExt == "otm";
    }
    else if (filter == FILTER_APPLICATION::CALC)
    {
        bRet = rExt == "ots" || rExt == "stc" || rExt == "xlt" || rExt == "xltm" || rExt == "xltx";
    }
    else if (filter == FILTER_APPLICATION::IMPRESS)
    {
        bRet = rExt == "otp" || rExt == "sti" || rExt == "pot" || rExt == "potm" || rExt == "potx";
    }
    else if (filter == FILTER_APPLICATION::DRAW)
    {
        bRet = rExt == "otg" || rExt == "std";
    }

    return bRet;
}

bool ViewFilter_Application::isValid (const OUString &rPath) const
{
    INetURLObject aUrl(rPath);
    return isFilteredExtension(mApp, aUrl.getExtension());
}

bool ViewFilter_Application::operator () (const ThumbnailViewItem *pItem)
{
    const TemplateViewItem *pTempItem = dynamic_cast<const TemplateViewItem*>(pItem);
    if (pTempItem)
        return isValid(pTempItem->getPath());

    return true;
}

TemplateLocalView::TemplateLocalView ( vcl::Window* pParent, WinBits nWinStyle)
    : ThumbnailView(pParent, nWinStyle),
      mnCurRegionId(0),
      maSelectedItem(nullptr),
      mnThumbnailWidth(TEMPLATE_THUMBNAIL_MAX_WIDTH),
      mnThumbnailHeight(TEMPLATE_THUMBNAIL_MAX_HEIGHT),
      maPosition(0,0),
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
    maRegions.clear();

    maAllTemplates.clear();

    mpDocTemplates.reset();
    ThumbnailView::dispose();
}

void TemplateLocalView::Populate ()
{
    maRegions.clear();
    maAllTemplates.clear();

    sal_uInt16 nCount = mpDocTemplates->GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString aRegionName(mpDocTemplates->GetFullRegionName(i));

        std::unique_ptr<TemplateContainerItem> pItem(new TemplateContainerItem( i+1 ));
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
            aProperties.aThumbnail = TemplateLocalView::fetchThumbnail(aURL,
                                                                          mnThumbnailWidth,
                                                                          mnThumbnailHeight);

            pItem->maTemplates.push_back(aProperties);
            maAllTemplates.push_back(aProperties);
        }

        maRegions.push_back(std::move(pItem));
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

        for (auto const & pRegion : maRegions)
        {
            if (pRegion->mnRegionId == nRegionId)
            {
                showRegion(pRegion.get());
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

    insertItems(maAllTemplates, false, true);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateLocalView::showRegion(TemplateContainerItem const *pItem)
{
    mnCurRegionId = pItem->mnRegionId+1;

    insertItems(pItem->maTemplates);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateLocalView::showRegion(const OUString &rName)
{
    for (auto const & pRegion : maRegions)
    {
        if (pRegion->maTitle == rName)
        {
            showRegion(pRegion.get());
            break;
        }
    }
}

TemplateContainerItem* TemplateLocalView::getRegion(OUString const & rName)
{
    for (auto const & pRegion : maRegions)
        if (pRegion->maTitle == rName)
            return pRegion.get();

    return nullptr;
}

void TemplateLocalView::createContextMenu(const bool bIsDefault)
{
    ScopedVclPtrInstance<PopupMenu> pItemMenu;
    pItemMenu->InsertItem(MNI_OPEN,SfxResId(STR_OPEN));
    pItemMenu->InsertItem(MNI_EDIT,SfxResId(STR_EDIT_TEMPLATE));

    if(!bIsDefault)
        pItemMenu->InsertItem(MNI_DEFAULT_TEMPLATE,SfxResId(STR_DEFAULT_TEMPLATE));
    else
        pItemMenu->InsertItem(MNI_DEFAULT_TEMPLATE,SfxResId(STR_RESET_DEFAULT));

    pItemMenu->InsertSeparator();
    pItemMenu->InsertItem(MNI_RENAME,SfxResId(STR_RENAME));
    pItemMenu->InsertItem(MNI_DELETE,SfxResId(STR_DELETE));
    pItemMenu->InsertSeparator();
    deselectItems();
    maSelectedItem->setSelection(true);
    maItemStateHdl.Call(maSelectedItem);
    pItemMenu->SetSelectHdl(LINK(this, TemplateLocalView, ContextMenuSelectHdl));
    pItemMenu->Execute(this, tools::Rectangle(maPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
    Invalidate();
}

IMPL_LINK(TemplateLocalView, ContextMenuSelectHdl, Menu*, pMenu, bool)
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
        InputDialog aTitleEditDlg(GetFrameWeld(), SfxResId(STR_RENAME_TEMPLATE));
        OUString sOldTitle = maSelectedItem->getTitle();
        aTitleEditDlg.SetEntryText(sOldTitle);
        aTitleEditDlg.HideHelpBtn();

        if (!aTitleEditDlg.run())
            break;
        OUString sNewTitle = comphelper::string::strip(aTitleEditDlg.GetEntryText(), ' ');

        if ( !sNewTitle.isEmpty() && sNewTitle != sOldTitle )
        {
            maSelectedItem->setTitle(sNewTitle);
        }
    }
        break;
    case MNI_DELETE:
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
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

sal_uInt16 TemplateLocalView::getRegionId(size_t pos) const
{
    assert(pos < maRegions.size());

    return maRegions[pos]->mnId;
}

sal_uInt16 TemplateLocalView::getRegionId(OUString const & sRegion) const
{
    for (auto const & pRegion : maRegions)
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
    for (auto const & pRegion : maRegions)
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
        TemplateContainerItem *pFolderItem = maRegions[mnCurRegionId-1].get();

        for (TemplateItemProperties & rItemProps : pFolderItem->maTemplates)
        {
            if (rFunc(rItemProps))
                aItems.push_back(rItemProps);
        }
    }
    else
    {
        for (auto const & pFolderItem : maRegions)
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
    std::unique_ptr<TemplateContainerItem> pItem(new TemplateContainerItem( nItemId ));
    pItem->mnRegionId = nRegionId;
    pItem->maTitle = rName;

    maRegions.push_back(std::move(pItem));

    return nItemId;
}

bool TemplateLocalView::renameRegion(const OUString &rTitle, const OUString &rNewTitle)
{
    TemplateContainerItem *pRegion = getRegion(rTitle);

    if(pRegion)
    {
        sal_uInt16 nRegionId = pRegion->mnRegionId;
        return mpDocTemplates->SetName( rNewTitle, nRegionId, USHRT_MAX/*nDocId*/ );
    }
    return false;
}

bool TemplateLocalView::removeRegion(const sal_uInt16 nItemId)
{
    sal_uInt16 nRegionId = USHRT_MAX;

    // Remove from the region cache list
    for (auto pRegionIt = maRegions.begin(); pRegionIt != maRegions.end();)
    {
        if ( (*pRegionIt)->mnId == nItemId )
        {
            if (!mpDocTemplates->Delete((*pRegionIt)->mnRegionId,USHRT_MAX))
                return false;

            nRegionId = (*pRegionIt)->mnRegionId;

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
    for (auto const& region : maRegions)
    {
        if (region->mnRegionId > nRegionId)
            --region->mnRegionId;
    }

    return true;
}

bool TemplateLocalView::removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId)
{
    for (auto const & pRegion : maRegions)
    {
        if (pRegion->mnId == nSrcItemId)
        {
            TemplateContainerItem *pItem = pRegion.get();
            auto pIter = std::find_if(pItem->maTemplates.begin(), pItem->maTemplates.end(),
                [nItemId](const TemplateItemProperties& rTemplate) { return rTemplate.nId == nItemId; });
            if (pIter != pItem->maTemplates.end())
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

    for (auto const & pRegion : maRegions)
    {
        if (pRegion->mnId == nTargetItem)
            pTarget = pRegion.get();
        else if (pRegion->mnId == nSrcItem)
            pSrc = pRegion.get();
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
            OUString sQuery = SfxResId(STR_MSG_QUERY_COPY).replaceFirst("$1", pViewItem->maTitle).replaceFirst("$2",
                getRegionName(nTargetRegion));

            std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo, sQuery));
            if (xQueryDlg->run() != RET_YES)
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
            for (auto const& item : mItemList)
            {
                auto pTemplateViewItem = static_cast<TemplateViewItem*>(item.get());
                if (pTemplateViewItem->mnDocId > pViewItem->mnDocId)
                    --pTemplateViewItem->mnDocId;
            }
        }

        CalculateItemPositions();
        Invalidate();

        return true;
    }

    return false;
}

void TemplateLocalView::moveTemplates(const std::set<const ThumbnailViewItem*, selection_cmp_fn> &rItems,
                                      const sal_uInt16 nTargetItem)
{
    TemplateContainerItem *pTarget = nullptr;
    TemplateContainerItem *pSrc = nullptr;

    for (auto const & pRegion : maRegions)
    {
        if (pRegion->mnId == nTargetItem)
            pTarget = pRegion.get();
    }

    if (!pTarget)
        return;

    bool refresh = false;

    sal_uInt16 nTargetRegion = pTarget->mnRegionId;
    sal_uInt16 nTargetIdx = mpDocTemplates->GetCount(nTargetRegion);    // Next Idx
    std::vector<sal_uInt16> aItemIds;    // List of moved items ids (also prevents the invalidation of rItems iterators when we remove them as we go)

    std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator aSelIter;
    for ( aSelIter = rItems.begin(); aSelIter != rItems.end(); ++aSelIter, ++nTargetIdx )
    {
        const TemplateViewItem *pViewItem = static_cast<const TemplateViewItem*>(*aSelIter);
        sal_uInt16 nSrcRegionId = pViewItem->mnRegionId;

        for (auto const & pRegion : maRegions)
        {
            if (pRegion->mnRegionId == nSrcRegionId)
                pSrc = pRegion.get();
        }

        if(pSrc)
        {
            bool bCopy = !mpDocTemplates->Move(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId);

            if (bCopy)
            {
                OUString sQuery = SfxResId(STR_MSG_QUERY_COPY).replaceFirst("$1", pViewItem->maTitle).replaceFirst("$2",
                    getRegionName(nTargetRegion));
                std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo, sQuery));
                if (xQueryDlg->run() != RET_YES)
                {
                    OUString sMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE));
                    sMsg = sMsg.replaceFirst("$1",getRegionName(nTargetRegion));
                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Warning, VclButtonsType::Ok, sMsg.replaceFirst( "$2",pViewItem->maTitle)));
                    xBox->run();

                    return; //return if any single move operation fails
                }

                if (!mpDocTemplates->Copy(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId))
                {
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
                for (auto const& item : mItemList)
                {
                    auto pTemplateViewItem = static_cast<TemplateViewItem*>(item.get());
                    if (pTemplateViewItem->mnDocId > pViewItem->mnDocId)
                        --pTemplateViewItem->mnDocId;
                }
            }
        }

        refresh = true;
    }

    // Remove items from the current view
    for (auto const& itemId : aItemIds)
        RemoveItem(itemId);

    if (refresh)
    {
        CalculateItemPositions();
        Invalidate();
    }
}

bool TemplateLocalView::copyFrom (TemplateContainerItem *pItem, const OUString &rPath)
{
    sal_uInt16 nId = 1;
    sal_uInt16 nDocId = 0;
    sal_uInt16 nRegionId = pItem->mnRegionId;
    OUString aPath(rPath);

    if (!pItem->maTemplates.empty())
    {
        nId = pItem->maTemplates.back().nId+1;
        nDocId = pItem->maTemplates.back().nDocId+1;
    }

    if (mpDocTemplates->CopyFrom(nRegionId,nDocId,aPath))
    {
        TemplateItemProperties aTemplate;
        aTemplate.nId = nId;
        aTemplate.nDocId = nDocId;
        aTemplate.nRegionId = nRegionId;
        aTemplate.aName = aPath;
        aTemplate.aThumbnail = TemplateLocalView::fetchThumbnail(rPath,
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
    for (auto const & pRegItem : maRegions)
    {
        if (pRegItem->mnId == nRegionItemId)
        {
            for (auto const& elem : pRegItem->maTemplates)
            {
                if (elem.nId == nItemId)
                {
                    return mpDocTemplates->CopyTo(pRegItem->mnRegionId,elem.nDocId,rName);
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

void TemplateLocalView::insertItems(const std::vector<TemplateItemProperties> &rTemplates, bool isRegionSelected, bool bShowCategoryInTooltip)
{
    std::vector<std::unique_ptr<ThumbnailViewItem>> aItems(rTemplates.size());
    for (size_t i = 0, n = rTemplates.size(); i < n; ++i )
    {
        const TemplateItemProperties *pCur = &rTemplates[i];

        std::unique_ptr<TemplateViewItem> pChild;
        if(isRegionSelected)
            pChild.reset(new TemplateViewItem(*this, pCur->nId));
        else
            pChild.reset(new TemplateViewItem(*this, i+1));

        pChild->mnDocId = pCur->nDocId;
        pChild->mnRegionId = pCur->nRegionId;
        pChild->maTitle = pCur->aName;
        pChild->setPath(pCur->aPath);

        if(!bShowCategoryInTooltip)
            pChild->setHelpText(pCur->aName);
        else
        {
            OUString sHelpText = SfxResId(STR_TEMPLATE_TOOLTIP);
            sHelpText = (sHelpText.replaceFirst("$1", pCur->aName)).replaceFirst("$2", pCur->aRegionName);
            pChild->setHelpText(sHelpText);
        }

        pChild->maPreview1 = pCur->aThumbnail;

        if(IsDefaultTemplate(pCur->aPath))
            pChild->showDefaultIcon(true);

        if ( pCur->aThumbnail.IsEmpty() )
        {
            // Use the default thumbnail if we have nothing else
            pChild->maPreview1 = TemplateLocalView::getDefaultThumbnail(pCur->aPath);
        }

        aItems[i] = std::move(pChild);
    }

    updateItems(std::move(aItems));
}

void TemplateLocalView::updateThumbnailDimensions(long itemMaxSize)
{
    mnThumbnailWidth = itemMaxSize;
    mnThumbnailHeight = itemMaxSize;
}


void TemplateLocalView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    ThumbnailView::MouseButtonDown(rMEvt);
}

void TemplateLocalView::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        tools::Rectangle aRect( OutputToScreenPixel( GetPosPixel() ), GetSizePixel() );
        Help::ShowQuickHelp( this, aRect, GetQuickHelpText(),
                             QuickHelpFlags::CtrlText | QuickHelpFlags::TipStyleBalloon );
        return;
    }

    ThumbnailView::RequestHelp( rHEvt );
}

void TemplateLocalView::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        if(rCEvt.IsMouseEvent())
        {
            deselectItems();
            size_t nPos = ImplGetItem(rCEvt.GetMousePosPixel());
            Point aPosition (rCEvt.GetMousePosPixel());
            maPosition = aPosition;
            ThumbnailViewItem* pItem = ImplGetItem(nPos);
            const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

            if(pViewItem)
            {
                maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
                maCreateContextMenuHdl.Call(pItem);
            }
        }
        else
        {
            for (ThumbnailViewItem* pItem : mFilteredItemList)
            {
                //create context menu for the first selected item
                if (pItem->isSelected())
                {
                    deselectItems();
                    pItem->setSelection(true);
                    maItemStateHdl.Call(pItem);
                    tools::Rectangle aRect = pItem->getDrawArea();
                    maPosition = aRect.Center();
                    maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
                    maCreateContextMenuHdl.Call(pItem);
                    break;
                }
            }
        }
    }

    ThumbnailView::Command(rCEvt);
}

void TemplateLocalView::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if(aKeyCode == ( KEY_MOD1 | KEY_A ) )
    {
        for (ThumbnailViewItem* pItem : mFilteredItemList)
        {
            if (!pItem->isSelected())
            {
                pItem->setSelection(true);
                maItemStateHdl.Call(pItem);
            }
        }

        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
        return;
    }
    else if( aKeyCode == KEY_DELETE && !mFilteredItemList.empty())
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return;

        //copy to avoid changing filtered item list during deletion
        ThumbnailValueItemList mFilteredItemListCopy = mFilteredItemList;

        for (ThumbnailViewItem* pItem : mFilteredItemListCopy)
        {
            if (pItem->isSelected())
            {
                maDeleteTemplateHdl.Call(pItem);
            }
        }
        reload();
    }

    ThumbnailView::KeyInput(rKEvt);
}


void TemplateLocalView::setOpenRegionHdl(const Link<void*,void> &rLink)
{
    maOpenRegionHdl = rLink;
}

void TemplateLocalView::setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maCreateContextMenuHdl = rLink;
}

void TemplateLocalView::setOpenTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maOpenTemplateHdl = rLink;
}

void TemplateLocalView::setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maEditTemplateHdl = rLink;
}

void TemplateLocalView::setDeleteTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maDeleteTemplateHdl = rLink;
}

void TemplateLocalView::setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maDefaultTemplateHdl = rLink;
}

BitmapEx TemplateLocalView::scaleImg (const BitmapEx &rImg, long width, long height)
{
    BitmapEx aImg = rImg;

    if (!rImg.IsEmpty())
    {
        Size aSize = rImg.GetSizePixel();

        if (aSize.Width() == 0)
            aSize.setWidth( 1 );

        if (aSize.Height() == 0)
            aSize.setHeight( 1 );

        // make the picture fit the given width/height constraints
        double nRatio = std::min(double(width)/double(aSize.Width()), double(height)/double(aSize.Height()));

        aImg.Scale(Size(aSize.Width() * nRatio, aSize.Height() * nRatio));
    }

    return aImg;
}

bool TemplateLocalView::IsDefaultTemplate(const OUString& rPath)
{
    SvtModuleOptions aModOpt;
    const css::uno::Sequence<OUString> &aServiceNames = aModOpt.GetAllServiceNames();

    for( sal_Int32 i=0, nCount = aServiceNames.getLength(); i < nCount; ++i )
    {
        const OUString defaultPath = SfxObjectFactory::GetStandardTemplate( aServiceNames[i] );
        if(defaultPath.match(rPath))
            return true;
    }

    return false;
}

void TemplateLocalView::RemoveDefaultTemplateIcon(const OUString& rPath)
{
    for (std::unique_ptr<ThumbnailViewItem>& pItem : mItemList)
    {
        TemplateViewItem* pViewItem = dynamic_cast<TemplateViewItem*>(pItem.get());
        if (pViewItem && pViewItem->getPath().match(rPath))
        {
            pViewItem->showDefaultIcon(false);
            Invalidate();
            return;
        }
    }
}

BitmapEx TemplateLocalView::getDefaultThumbnail( const OUString& rPath )
{
    BitmapEx aImg;
    INetURLObject aUrl(rPath);
    OUString aExt = aUrl.getExtension();

    if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::WRITER, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_TEXT);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::CALC, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_SHEET);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::IMPRESS, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_PRESENTATION);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::DRAW, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_DRAWING);

    return aImg;
}

BitmapEx TemplateLocalView::fetchThumbnail (const OUString &msURL, long width, long height)
{
    return TemplateLocalView::scaleImg(ThumbnailView::readThumbnail(msURL), width, height);
}

void TemplateLocalView::OnItemDblClicked (ThumbnailViewItem *pItem)
{
    TemplateViewItem* pViewItem = dynamic_cast<TemplateViewItem*>(pItem);

    if( pViewItem )
        maOpenTemplateHdl.Call(pViewItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
