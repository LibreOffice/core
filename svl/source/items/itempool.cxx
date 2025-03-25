/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svl/itempool.hxx>
#include <svl/setitem.hxx>

#include <string.h>
#include <libxml/xmlwriter.h>

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/hint.hxx>
#include <svl/itemset.hxx>
#include <tools/debug.hxx>

#include <cassert>
#include <vector>

// WhichIDs that need to set SFX_ITEMINFOFLAG_SUPPORT_SURROGATE in SfxItemInfo
// to true to allow a register of all items of that type/with that WhichID
// to be accessible using SfxItemPool::GetItemSurrogates. Created by
// grepping for 'GetItemSurrogates' usages & interpreting. Some
// are double, more may be necessary. There is a SAL_INFO("svl.items", ...)
// in SfxItemPool::GetItemSurrogates that will give hints on missing flags.
//
// due to SwTable::UpdateFields
// due to SwCursorShell::GotoNxtPrvTableFormula
// due to DocumentFieldsManager::UpdateTableFields
// due to SwTable::GatherFormulas
//  RES_BOXATR_FORMULA ok
// due to SwContentTree::EditEntry
// due to SwDoc::FindINetAttr
// due to SwUndoResetAttr::RedoImpl
// due to SwContentTree::EditEntry
// due to SwContentTree::BringEntryToAttention
//  RES_TXTATR_REFMARK ok
// due to ImpEditEngine::WriteRTF
// due to ScDocument::UpdateFontCharSet()
// due to ScXMLFontAutoStylePool_Impl
// due to SdXImpressDocument::getPropertyValue
// due to Writer::AddFontItems_
//  EE_CHAR_FONTINFO ok
// due to ImpEditEngine::WriteRTF
// due to ScXMLFontAutoStylePool_Impl
// due to SdXImpressDocument::getPropertyValue
// due to Writer::AddFontItems_
//  EE_CHAR_FONTINFO_CJK ok
// due to ImpEditEngine::WriteRTF
// due to ScXMLFontAutoStylePool_Impl
// due to SdXImpressDocument::getPropertyValue
// due to Writer::AddFontItems_
//  EE_CHAR_FONTINFO_CTL ok
// due to ImpEditEngine::WriteRTF
//  EE_CHAR_COLOR ok
// due to ScDocumentPool::StyleDeleted
// due to ScDocument::UpdateFontCharSet()
// due to ScXMLFontAutoStylePool_Impl
//  ATTR_FONT ok
// due to OptimizeHasAttrib
//  ATTR_ROTATE_VALUE ok
// due to ScDocument::GetDocColors()
//  ATTR_BACKGROUND ok
//  ATTR_FONT_COLOR ok
// due to ScXMLExport::CollectUserDefinedNamespaces
//  ATTR_USERDEF ok
// due to ScXMLExport::CollectUserDefinedNamespaces
// due to SwXMLExport::exportDoc
//  EE_PARA_XMLATTRIBS ok
// due to ScXMLExport::CollectUserDefinedNamespaces
// due to SwXMLExport::exportDoc
//  EE_CHAR_XMLATTRIBS ok
// due to ScXMLExport::CollectUserDefinedNamespaces
// due to SwXMLExport::exportDoc
//  SDRATTR_XMLATTRIBUTES ok
// due to ScXMLFontAutoStylePool_Impl
//  ATTR_CJK_FONT ok
//  ATTR_CTL_FONT ok
//  ATTR_PAGE_HEADERLEFT ok
//  ATTR_PAGE_FOOTERLEFT ok
//  ATTR_PAGE_HEADERRIGHT ok
//  ATTR_PAGE_FOOTERRIGHT ok
//  ATTR_PAGE_HEADERFIRST ok
//  ATTR_PAGE_FOOTERFIRST ok
// due to ScCellShell::ExecuteEdit
// due to ScTabViewShell::CreateRefDialogController
//  SCITEM_CONDFORMATDLGDATA ok
// due to SdDrawDocument::UpdatePageRelativeURLs
//  EE_FEATURE_FIELD ok
// due to SvxUnoMarkerTable::replaceByName
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
// due to XLineStartItem::checkForUniqueItem
//  XATTR_LINESTART ok
// due to SvxUnoMarkerTable::replaceByName
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
// due to XLineStartItem::checkForUniqueItem
//  XATTR_LINEEND ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
// due to NameOrIndex::CheckNamedItem all derived from NameOrIndex
//  XATTR_FILLBITMAP ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
//  XATTR_LINEDASH ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
// due to NameOrIndex::CheckNamedItem all derived from NameOrIndex
//  XATTR_FILLGRADIENT ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
//  XATTR_FILLHATCH ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
//  XATTR_FILLFLOATTRANSPARENCE ok
// due to NamespaceIteratorImpl
//      -> needs to be evaluated
// due to SwCursorShell::GotoNxtPrvTOXMark
// due to SwDoc::GetTOIKeys
//  RES_TXTATR_TOXMARK ok
// due to SwDoc::GetRefMark
// due to SwDoc::CallEvent
// due to SwURLStateChanged::Notify
// due to SwHTMLWriter::CollectLinkTargets
// due to MSWordExportBase::CollectOutlineBookmarks
//  RES_TXTATR_INETFMT ok
// due to SwDoc::GetAllUsedDB
// due to lcl_FindInputField
// due to SwViewShell::IsAnyFieldInDoc
//  RES_TXTATR_FIELD ok
// due to SwDoc::GetAllUsedDB
// due to lcl_FindInputField
// due to SwViewShell::IsAnyFieldInDoc
//  RES_TXTATR_INPUTFIELD ok
// due to SwDoc::SetDefault
//  RES_PARATR_TABSTOP ok
// due to SwDoc::GetDocColors()
// due to RtfExport::OutColorTable
//  RES_CHRATR_COLOR ok
// due to SwDoc::GetDocColors()
//  RES_CHRATR_HIGHLIGHT ok
// due to SwDoc::GetDocColors()
//  RES_BACKGROUND ok
// due to SwNode::FindPageDesc
// due to SwPageNumberFieldType::ChangeExpansion
// due to SwFrame::GetVirtPageNum
//  RES_PAGEDESC ok
// due to SwAutoStylesEnumImpl::
//  RES_TXTATR_CJK_RUBY ok
// due to SwHTMLWriter::CollectLinkTargets
// due to MSWordExportBase::CollectOutlineBookmarks
//  RES_URL
// due to RtfExport::OutColorTable
//  RES_CHRATR_UNDERLINE ok
//  RES_CHRATR_OVERLINE ok
//  RES_CHRATR_BACKGROUND ok
//  RES_SHADOW ok
//  RES_BOX ok
//  RES_CHRATR_BOX ok
//  XATTR_FILLCOLOR ok
// due to wwFontHelper::InitFontTable
// due to SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl
//  RES_CHRATR_FONT ok
// due to wwFontHelper::InitFontTable
// due to SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl
//  RES_CHRATR_CJK_FONT ok
// due to wwFontHelper::InitFontTable
// due to SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl
//  RES_CHRATR_CTL_FONT
// due to SwXMLExport::exportDoc
//  RES_UNKNOWNATR_CONTAINER ok
//  RES_TXTATR_UNKNOWN_CONTAINER ok

ItemInfoUser::ItemInfoUser(const ItemInfo& rItemInfo, const SfxItemPool& rItemPool, const SfxPoolItem& rItem, bool bPassingOwnership)
: ItemInfo(rItemInfo)
, m_pItem(implCreateItemEntry(rItemPool, &rItem, bPassingOwnership))
{
}

ItemInfoUser::~ItemInfoUser()
{
    implCleanupItemEntry(m_pItem);
}

const SlotIDToWhichIDMap& ItemInfoPackage::getSlotIDToWhichIDMap() const
{
    if (maSlotIDToWhichIDMap.empty())
    {
        // will be filled only once per office runtime
        for (size_t a(0); a < size(); a++)
        {
            const ItemInfoStatic& rCandidate(getItemInfoStatic(a));
            if (0 != rCandidate.getSlotID())
            {
#ifdef DBG_UTIL
                if (maSlotIDToWhichIDMap.contains(rCandidate.getSlotID()))
                    assert(false && "ITEM: SlotID used double in ItemInfoPackage (!)");
#endif
                maSlotIDToWhichIDMap[rCandidate.getSlotID()] = rCandidate.getWhich();
            }
        }
    }

    return maSlotIDToWhichIDMap;
}

const ItemInfo& ItemInfoPackage::getExistingItemInfo(size_t /*nIndex*/)
{
    static ItemInfoStatic EMPTY(0, nullptr, 0, 0);
    return EMPTY;
}

void SfxItemPool::registerItemInfoPackage(
    ItemInfoPackage& rPackage,
    const std::function<SfxPoolItem*(sal_uInt16)>& rCallback)
{
    assert(maItemInfos.empty() && "ITEM: registering more than one ItemInfoPackage per Pool is not allowed (!)");

    // we know the size :-)
    maItemInfos.reserve(rPackage.size());

    // loop over ItemInfoPackage and add ptrs to provided ItemInfos
    for(size_t a(0); a < rPackage.size(); a++)
    {
        // get ItemInfo entry, maybe StaticDefault or DynamicDefault
        const ItemInfo& rItemInfo(rPackage.getItemInfo(a, *this));

        if (nullptr != rItemInfo.getItem())
        {
            // if it has an item, use it, done
            maItemInfos.push_back(&rItemInfo);
            continue;
        }

        // if not, use the callback to create a DynamicDefault. This
        // *has* to be supported then by the caller
        SfxPoolItem* pDynamicItem(rCallback(rItemInfo.getWhich()));
        assert(nullptr != pDynamicItem);
        maItemInfos.push_back(new ItemInfoDynamic(rItemInfo, pDynamicItem));
    }

    // use infos to fill local variables
    mnStart = maItemInfos.front()->getWhich();
    mnEnd = maItemInfos.back()->getWhich();

    // set mapper for fast SlotIDToWhichID conversion
    mpSlotIDToWhichIDMap = &rPackage.getSlotIDToWhichIDMap();

#ifdef DBG_UTIL
    for (size_t a(1); a < maItemInfos.size(); a++)
        if (maItemInfos[a-1]->getWhich() + 1 != maItemInfos[a]->getWhich())
            assert(false && "ITEM: Order is wrong (!)");
#endif
}

const ItemInfo* SfxItemPool::impCheckItemInfoForClone(const ItemInfo* pInfo)
{
    const SfxPoolItem* pItem(pInfo->getItem());
    assert(nullptr != pItem && "ITEM: Missing Item in ItemInfo (!)");

    if (pItem->isStaticDefault())
        // noting to do, not ref-counted
        return pInfo;

    if (pItem->isDynamicDefault())
    {
        // need to clone to new Pool as DynamicDefault, owned by the Pool
        // and not shared. Mainly SfxSetItems. Not RefCounted
        return new ItemInfoDynamic(*pInfo, pItem->Clone(this));
    }

    // all Items else that can be in the Pool are UserDefaults. These
    // are RefCounted, so use implCreateItemEntry to increase reference
    return new ItemInfoUser(*pInfo, *this, *pItem);
}

void SfxItemPool::impClearUserDefault(const userItemInfos::iterator& rHit)
{
    if (rHit == maUserItemInfos.end())
        // does not exist
        return;

    // get ItemInfo and Item, HAS to be a UserDefault
    const sal_uInt16 nIndex(GetIndex_Impl(rHit->first));
    const ItemInfo* pInfo(maItemInfos[nIndex]);
    assert(nullptr != pInfo && "ITEM: access error to Defaults in Pool (!)");

    // restore original entry using the remembered one
    maItemInfos[nIndex] = rHit->second;

    // free Item, delete ItemInfo
    delete pInfo;
}

void SfxItemPool::impCreateUserDefault(const SfxPoolItem& rItem)
{
    const sal_uInt16 nWhich(rItem.Which());

    // make sure by an assert check that none exists
    assert(maUserItemInfos.end() == maUserItemInfos.find(nWhich));

    const sal_uInt16 nIndex(GetIndex_Impl(nWhich));
    const ItemInfo* pInfo(maItemInfos[nIndex]);
    assert(nullptr != pInfo && "ITEM: access error to Defaults in Pool (!)");

    // safe original ItemInfo in UserItemInfos
    maUserItemInfos.insert({nWhich, pInfo});

    // create new Item by using implCreateItemEntry and new ItemInfo
    maItemInfos[nIndex] = new ItemInfoUser(*pInfo, *this, rItem);
}

void SfxItemPool::cleanupItemInfos()
{
    // reset all UserDefaultItems & restore original maItemInfos
    while (!maUserItemInfos.empty())
    {
        // get next candidate, cleanup UseDefault and remove data
        userItemInfos::iterator aHit(maUserItemInfos.begin());
        impClearUserDefault(aHit);
        maUserItemInfos.erase(aHit);
    }

    // delete DynamicDefaults in maItemInfos, these only exist
    // for Pool lifetime since they are Pool-dependent. There should
    // be NO MORE UserDefaults after cleanup above
    for (auto& rInfo : maItemInfos)
    {
        if (rInfo->getItem()->isDynamicDefault())
        {
            // the whole ItemInfo is owned by the pool, so
            // delete the Item and the ItemInfo (in that order :-)
            delete rInfo;
        }
#ifdef DBG_UTIL
        // since there should be NO MORE UserDefaults the item
        // then *has* to be StaticDefault - check that
        else if (!rInfo->getItem()->isStaticDefault())
            assert(false && "ITEM: Error in UserDefault handling (!)");
#endif
    }
}

void SfxItemPool::registerItemSet(SfxItemSet& rSet)
{
    registeredSfxItemSets& rTarget(GetMasterPool()->maRegisteredSfxItemSets);
#ifdef DBG_UTIL
    const size_t nBefore(rTarget.size());
#endif
    rTarget.insert(&rSet);
#ifdef DBG_UTIL
    const size_t nAfter(rTarget.size());
    if (nBefore + 1 != nAfter)
    {
        SAL_WARN("svl.items", "SfxItemPool::registerItemSet: ItemSet was already registered (!)");
    }
#endif
}

void SfxItemPool::unregisterItemSet(SfxItemSet& rSet)
{
    registeredSfxItemSets& rTarget(GetMasterPool()->maRegisteredSfxItemSets);
#ifdef DBG_UTIL
    const size_t nBefore(rTarget.size());
#endif
    rTarget.erase(&rSet);
#ifdef DBG_UTIL
    const size_t nAfter(rTarget.size());
    if (nBefore != nAfter + 1)
    {
        SAL_WARN("svl.items", "SfxItemPool::unregisterItemSet: ItemSet was not registered (!)");
    }
#endif
}

void SfxItemPool::registerPoolItemHolder(SfxPoolItemHolder& rHolder)
{
    registeredSfxPoolItemHolders& rTarget(GetMasterPool()->maRegisteredSfxPoolItemHolders);
#ifdef DBG_UTIL
    const size_t nBefore(rTarget.size());
#endif
    rTarget.insert(&rHolder);
#ifdef DBG_UTIL
    const size_t nAfter(rTarget.size());
    if (nBefore + 1 != nAfter)
    {
        SAL_WARN("svl.items", "SfxItemPool::registerPoolItemHolder: SfxPoolItemHolder was already registered (!)");
    }
#endif
    if (rHolder.is() && rHolder.getItem()->isNameOrIndex())
        registerNameOrIndex(*rHolder.getItem());
}

void SfxItemPool::unregisterPoolItemHolder(SfxPoolItemHolder& rHolder)
{
    registeredSfxPoolItemHolders& rTarget(GetMasterPool()->maRegisteredSfxPoolItemHolders);
#ifdef DBG_UTIL
    const size_t nBefore(rTarget.size());
#endif
    rTarget.erase(&rHolder);
#ifdef DBG_UTIL
    const size_t nAfter(rTarget.size());
    if (nBefore != nAfter + 1)
    {
        SAL_WARN("svl.items", "SfxItemPool::unregisterPoolItemHolder: SfxPoolItemHolder was not registered (!)");
    }
#endif
    if (rHolder.is() && rHolder.getItem()->isNameOrIndex())
        unregisterNameOrIndex(*rHolder.getItem());
}

void SfxItemPool::registerNameOrIndex(const SfxPoolItem& rItem)
{
    assert(rItem.isNameOrIndex() && "ITEM: only Items derived from NameOrIndex supported for this mechanism (!)");
    NameOrIndexContent& rTarget(GetMasterPool()->maRegisteredNameOrIndex[rItem.ItemType()]);
    NameOrIndexContent::iterator aHit(rTarget.find(&rItem));
    if (aHit == rTarget.end())
        rTarget.insert(std::pair<const SfxPoolItem*, sal_uInt32>(&rItem, 0));
    else
        aHit->second++;
}

void SfxItemPool::unregisterNameOrIndex(const SfxPoolItem& rItem)
{
    assert(rItem.isNameOrIndex() && "ITEM: only Items derived from NameOrIndex supported for this mechanism (!)");
    NameOrIndexContent& rTarget(GetMasterPool()->maRegisteredNameOrIndex[rItem.ItemType()]);
    NameOrIndexContent::iterator aHit(rTarget.find(&rItem));
    assert(aHit != rTarget.end() && "ITEM: malformed order of buffered NameOrIndex Items, entry *expected* (!)");
    if (0 == aHit->second)
        rTarget.erase(aHit);
    else
        aHit->second--;
}

SfxItemPool* SfxItemPool::getTargetPool(sal_uInt16 nWhich) const
{
    if (IsInRange(nWhich))
        return const_cast<SfxItemPool*>(this);
    if (mpSecondary)
        return mpSecondary->getTargetPool(nWhich);
    return nullptr;
}

bool SfxItemPool::CheckItemInfoFlag(sal_uInt16 nWhich, sal_uInt16 nMask) const
{
    SfxItemPool* pTarget(getTargetPool(nWhich));
    if (nullptr == pTarget)
        return false;

    if (!pTarget->maItemInfos.empty())
    {
        const sal_uInt16 nIndex(pTarget->GetIndex_Impl(nWhich));
        const ItemInfo* pInfo(pTarget->maItemInfos[nIndex]);
        assert(nullptr != pInfo);
        return pInfo->getItemInfoFlags() & nMask;
    }

    return pTarget->CheckItemInfoFlag_Impl(pTarget->GetIndex_Impl(nWhich), nMask);
}

SfxBroadcaster& SfxItemPool::BC()
{
    return aBC;
}

/**
 * This is the regular ctor to be used for this class.
 * An SfxItemPool instance is initialized, which can manage Items in the
 * range from 'nStartWhich' to 'nEndWhich'.
 *
 * For every one of these WhichIds a static Default must be present in the
 * 'pDefaults' array. They start with an SfxPoolItem (with the WhichId
 * 'nStartWhich'), are sorted by WhichId and consecutively stored.
 *
 * 'pItemInfos' is a USHORT array arranged in the same way, which holds
 *  SlotIds and Flags. These SlotIds can be 0, if the affected Items are
 *  exclusively used in the Core.
 *  The flags allow for e.g. enabling value sharing (poolable).
 *
 *  If the Pool is supposed to hold SfxSetItems, the ctor cannot yet contain
 *  static Defaults. This needs to be done afterwards, using
 *  @see SfxItemPool::SetPoolDefaults(std::vector<SfxPoolItem*>*).
 *
 *  @see SfxItemPool::SetPoolDefaults(std::vector<SfxPoolItem*>*)
 *  @see SfxItemPool::ReleasePoolDefaults(std::vector<SfxPoolItem*>*,bool)
 *  @see SfxItemPool::ReleasePoolDefaults(bool)
 */
SfxItemPool::SfxItemPool(const OUString& rName) /* Pool name to identify in the file format */
: salhelper::SimpleReferenceObject()
, aBC()
, aName(rName)
, mpMaster(this)
, mpSecondary()
, mnStart(0)
, mnEnd(0)
, eDefMetric(MapUnit::MapCM)
, maRegisteredSfxItemSets()
, maRegisteredSfxPoolItemHolders()
, maRegisteredNameOrIndex()
, mbShutdownHintSent(false)
, maItemInfos()
, maUserItemInfos()
, mpSlotIDToWhichIDMap(nullptr)
{
    eDefMetric = MapUnit::MapTwip;
}

/**
 * Copy ctor
 *
 * @see SfxItemPool::Clone() const
*/
SfxItemPool::SfxItemPool(const SfxItemPool& rPool) //  Copy from this instance
: salhelper::SimpleReferenceObject()
, aBC()
, aName(rPool.aName)
, mpMaster(this)
, mpSecondary()
, maPoolRanges()
, mnStart(rPool.mnStart)
, mnEnd(rPool.mnEnd)
, eDefMetric(MapUnit::MapCM)
, maRegisteredSfxItemSets()
, maRegisteredSfxPoolItemHolders()
, maRegisteredNameOrIndex()
, mbShutdownHintSent(false)
, maItemInfos(rPool.maItemInfos)
, maUserItemInfos(rPool.maUserItemInfos)
, mpSlotIDToWhichIDMap(rPool.mpSlotIDToWhichIDMap)
{
    // DynamicDefaults and UserDefaults need to be cloned for the new Pool
    for (itemInfoVector::iterator aInfo(maItemInfos.begin()); aInfo != maItemInfos.end(); aInfo++)
        *aInfo = impCheckItemInfoForClone(*aInfo);

    // DynamicDefaults need to be cloned for the new Pool (no UserDefaults in UserItemInfos)
    for (auto& rUserItem : maUserItemInfos)
        rUserItem.second = impCheckItemInfoForClone(rUserItem.second);

    eDefMetric = rPool.eDefMetric;

    // Repair linkage
    if ( rPool.mpSecondary )
        SetSecondaryPool( rPool.mpSecondary->Clone().get() );
}

SfxItemPool::~SfxItemPool()
{
    // cleanup UserDefaults & delete owned DynamicDefaults
    cleanupItemInfos();

    // Need to send ShutdownHint?
    sendShutdownHint();

    if (mpMaster != nullptr && mpMaster != this)
    {
        // This condition indicates an error.
        // A mpMaster->SetSecondaryPool(...) call should have been made
        // earlier to prevent this. At this point we can only try to
        // prevent a crash later on.
        DBG_ASSERT( mpMaster == this, "destroying active Secondary-Pool" );
        if (mpMaster->mpSecondary == this)
            mpMaster->mpSecondary = nullptr;
    }
}

void SfxItemPool::SetSecondaryPool( SfxItemPool *pPool )
{
    // Reset Master in attached Pools
    if ( mpSecondary )
    {
        mpSecondary->mpMaster = mpSecondary.get();
        for ( SfxItemPool *p = mpSecondary->mpSecondary.get(); p; p = p->mpSecondary.get() )
            p->mpMaster = mpSecondary.get();
    }

    // Set Master of new Secondary Pools
    DBG_ASSERT( !pPool || pPool->mpMaster == pPool, "Secondary is present in two Pools" );
    SfxItemPool *pNewMaster = GetMasterPool() ? mpMaster : this;
    for ( SfxItemPool *p = pPool; p; p = p->mpSecondary.get() )
        p->mpMaster = pNewMaster;

    // Remember new Secondary Pool
    mpSecondary = pPool;
}

MapUnit SfxItemPool::GetMetric( sal_uInt16 ) const
{
    return eDefMetric;
}

void SfxItemPool::SetDefaultMetric( MapUnit eNewMetric )
{
//    assert((pImpl->eDefMetric == eNewMetric || !pImpl->maPoolRanges) && "pool already frozen, cannot change metric");
    eDefMetric = eNewMetric;
}

bool SfxItemPool::GetPresentation
(
    const SfxPoolItem&  rItem,
    MapUnit             eMetric,
    OUString&           rText,
    const IntlWrapper&  rIntlWrapper
)   const
{
    return rItem.GetPresentation(
        SfxItemPresentation::Complete, GetMetric(rItem.Which()), eMetric, rText, rIntlWrapper );
}

rtl::Reference<SfxItemPool> SfxItemPool::Clone() const
{
    return new SfxItemPool( *this );
}

void SfxItemPool::sendShutdownHint()
{
    // Already sent?
    if (mbShutdownHintSent)
        return;

    mbShutdownHintSent = true;

    // Inform e.g. running Requests
    aBC.Broadcast( SfxHint( SfxHintId::Dying ) );
    maPoolRanges.reset();
}

void SfxItemPool::SetUserDefaultItem(const SfxPoolItem& rItem)
{
    SfxItemPool* pTarget(getTargetPool(rItem.Which()));
    if (nullptr == pTarget)
        assert(false && "unknown WhichId - cannot set pool default");

    const sal_uInt16 nWhich(rItem.Which());
    userItemInfos::iterator aHit(pTarget->maUserItemInfos.find(nWhich));

    if (aHit == pTarget->maUserItemInfos.end())
    {
        // UserDefault does not exist, create needed entries to safe
        // original ItemInfo in UserItemInfos and set new, owned
        // ItemInfo containing an owned clone of the Item in ItemInfos
        pTarget->impCreateUserDefault(rItem);
        return;
    }

    // UserDefault does exist, check and evtl. replace
    const sal_uInt16 nIndex(pTarget->GetIndex_Impl(nWhich));
    const ItemInfo* pInfo(pTarget->maItemInfos[nIndex]);
    assert(nullptr != pInfo && "ITEM: access error to Defaults in Pool (!)");
    const SfxPoolItem* pItem(pInfo->getItem());
    assert(nullptr != pItem && "ITEM: access error to Defaults in Pool (!)");

    // nothing to do if equal, so check
    if (SfxPoolItem::areSame(pItem, &rItem))
        return;

    // need to exchange existing instance and free current one
    pTarget->maItemInfos[nIndex] = new ItemInfoUser(*pInfo, *pTarget, rItem);
    delete pInfo;
}

const SfxPoolItem* SfxItemPool::GetUserDefaultItem( sal_uInt16 nWhich ) const
{
    SfxItemPool* pTarget(getTargetPool(nWhich));
    if (nullptr == pTarget)
    {
        assert(false && "unknown WhichId - cannot get pool default");
        return nullptr;
    }

    userItemInfos::iterator aHit(pTarget->maUserItemInfos.find(nWhich));

    if (aHit == pTarget->maUserItemInfos.end())
        // no default item
        return nullptr;

    const sal_uInt16 nIndex(pTarget->GetIndex_Impl(nWhich));
    const ItemInfo* pInfo(pTarget->maItemInfos[nIndex]);
    assert(nullptr != pInfo && "ITEM: access error to Defaults in Pool (!)");
    const SfxPoolItem* pItem(pInfo->getItem());
    assert(nullptr != pItem && "ITEM: access error to Defaults in Pool (!)");
    return pItem;
}

/**
 * Resets the default of the given WhichId back to the static Default.
 * If a pool default exists, it is removed.
 */
void SfxItemPool::ResetUserDefaultItem( sal_uInt16 nWhich )
{
    SfxItemPool* pTarget(getTargetPool(nWhich));
    if (nullptr == pTarget)
        assert(false && "unknown WhichId - cannot reset pool default");

    userItemInfos::iterator aHit(pTarget->maUserItemInfos.find(nWhich));

    if (aHit != pTarget->maUserItemInfos.end())
    {
        // clear entry, cleanup, restore previous data
        pTarget->impClearUserDefault(aHit);

        // remove remembered data
        pTarget->maUserItemInfos.erase(aHit);
    }
}

const SfxPoolItem& SfxItemPool::GetUserOrPoolDefaultItem( sal_uInt16 nWhich ) const
{
    SfxItemPool* pTarget(getTargetPool(nWhich));
    if (nullptr == pTarget)
        assert(!"unknown which - don't ask me for defaults");

    const sal_uInt16 nIndex(pTarget->GetIndex_Impl(nWhich));
    const ItemInfo* pInfo(pTarget->maItemInfos[nIndex]);
    assert(nullptr != pInfo && "ITEM: access error to Defaults in Pool (!)");
    const SfxPoolItem* pItem(pInfo->getItem());
    assert(nullptr != pItem && "ITEM: access error to Defaults in Pool (!)");
    return *pItem;
}

/* get the last pool by following the GetSecondaryPool chain */
SfxItemPool* SfxItemPool::GetLastPoolInChain()
{
    SfxItemPool* pLast(this);

    while(pLast->GetSecondaryPool())
        pLast = pLast->GetSecondaryPool();

    return pLast;
}

const WhichRangesContainer& SfxItemPool::GetMergedIdRanges() const
{
    if (maPoolRanges.empty())
    {
        // Merge all ranges, keeping them sorted
        for (const SfxItemPool* pPool = this; pPool; pPool = pPool->mpSecondary.get())
            maPoolRanges = maPoolRanges.MergeRange(pPool->mnStart, pPool->mnEnd);
    }

    return maPoolRanges;
}

const SfxPoolItem* SfxItemPool::GetPoolDefaultItem(sal_uInt16 nWhich) const
{
    SfxItemPool* pTarget(getTargetPool(nWhich));
    if (nullptr == pTarget)
        assert(false && "unknown WhichId - cannot resolve surrogate");

    const sal_uInt16 nIndex(pTarget->GetIndex_Impl(nWhich));
    userItemInfos::iterator aHit(pTarget->maUserItemInfos.find(nWhich));

    if (aHit != pTarget->maUserItemInfos.end())
    {
        // If it is a UserDefault Item, check saved ItemInfo and use
        // Item from there
        assert(aHit != pTarget->maUserItemInfos.end() && "ITEM: Error in UserDefault handling (!)");
        return aHit->second->getItem();
    }

    const ItemInfo* pInfo(pTarget->maItemInfos[nIndex]);
    assert(nullptr != pInfo && "ITEM: access error to Defaults in Pool (!)");
    const SfxPoolItem* pItem(pInfo->getItem());
    assert(nullptr != pItem && "ITEM: access error to Defaults in Pool (!)");
    return pItem;
}

namespace
{
    class SurrogateData_ItemSet : public SfxItemPool::SurrogateData
    {
        const SfxPoolItem*  mpItem;
        SfxItemSet*         mpSet;

    public:
        SurrogateData_ItemSet(const SfxPoolItem& rItem, SfxItemSet& rSet)
        : SfxItemPool::SurrogateData()
        , mpItem(&rItem)
        , mpSet(&rSet)
        {
        }

        SurrogateData_ItemSet(const SurrogateData_ItemSet&) = default;

        virtual const SfxPoolItem& getItem() const override
        {
            return *mpItem;
        }

        virtual const SfxPoolItem* setItem(std::unique_ptr<SfxPoolItem> aNew) override
        {
            return mpSet->Put(std::unique_ptr<SfxPoolItem>(aNew.release()));
        }
    };

    class SurrogateData_ItemHolder : public SfxItemPool::SurrogateData
    {
        SfxPoolItemHolder*  mpHolder;

    public:
        SurrogateData_ItemHolder(SfxPoolItemHolder& rHolder)
        : SfxItemPool::SurrogateData()
        , mpHolder(&rHolder)
        {
        }

        SurrogateData_ItemHolder(const SurrogateData_ItemHolder&) = default;

        virtual const SfxPoolItem& getItem() const override
        {
            return *mpHolder->getItem();
        }

        virtual const SfxPoolItem* setItem(std::unique_ptr<SfxPoolItem> aNew) override
        {
            *mpHolder = SfxPoolItemHolder(mpHolder->getPool(), aNew.release(), true);
            return mpHolder->getItem();
        }
    };
}

void SfxItemPool::iterateItemSurrogates(
    sal_uInt16 nWhich,
    const std::function<bool(SurrogateData& rCand)>& rItemCallback) const
{
    // 1st source for surrogates
    const registeredSfxItemSets& rSets(GetMasterPool()->maRegisteredSfxItemSets);

    if(!rSets.empty())
    {
        const SfxPoolItem* pItem(nullptr);
        std::vector<SurrogateData_ItemSet> aEntries;

        // NOTE: this collects the callback data in a preparing run. This
        //   is by purpose, else any write change may change the iterators
        //   used at registeredSfxItemSets. I tied with direct feed and
        //   that worked most of the time, but failed for ItemHolders due
        //   to these being changed and being re-registered. I have avoided
        //   this in SfxPoolItemHolder::operator=, but it's just a question
        //   that in some scenario someone replaces an Item even with a
        //   different type/WhichID that this will then break/crash
        for (const auto& rCand : rSets)
            if (SfxItemState::SET == rCand->GetItemState(nWhich, false, &pItem))
                aEntries.emplace_back(*pItem, *rCand);

        if (!aEntries.empty())
            for (auto& rCand : aEntries)
                if (!rItemCallback(rCand))
                    return;
    }

    // 2nd source for surrogates
    const registeredSfxPoolItemHolders& rHolders(GetMasterPool()->maRegisteredSfxPoolItemHolders);

    if (!rHolders.empty())
    {
        std::vector<SurrogateData_ItemHolder> aEntries;

        // NOTE: same as above, look there
        for (auto& rCand : rHolders)
            if (rCand->Which() == nWhich && nullptr != rCand->getItem())
                aEntries.emplace_back(*rCand);

        if (!aEntries.empty())
            for (auto& rCand : aEntries)
                if (!rItemCallback(rCand))
                    return;
    }
}

void SfxItemPool::GetItemSurrogatesForItem(ItemSurrogates& rTarget, SfxItemType eItemType) const
{
    rTarget.clear();
    const registeredNameOrIndex& rRegistered(GetMasterPool()->maRegisteredNameOrIndex);
    registeredNameOrIndex::const_iterator aHit(rRegistered.find(eItemType));
    if (aHit != rRegistered.end())
    {
        rTarget.reserve(aHit->second.size());
        for (const auto& entry : aHit->second)
            rTarget.push_back(entry.first);
    }
}

void SfxItemPool::GetItemSurrogatesForItem(ItemSurrogates& rTarget, const SfxPoolItem& rItem) const
{
    assert(rItem.isNameOrIndex() && "ITEM: only Items derived from NameOrIndex supported for this mechanism (!)");
    GetItemSurrogatesForItem(rTarget, rItem.ItemType());
}

void SfxItemPool::GetItemSurrogates(ItemSurrogates& rTarget, sal_uInt16 nWhich) const
{
    rTarget.clear();

    if (0 == nWhich)
        return;

    // NOTE: This is pre-collected, in this case mainly to
    //   remove all double listings of SfxPoolItems which can
    //   of course be referenced multiple times in multiple
    //   ItemSets/ItemHolders. It comes handy that
    //   std::unordered_set does this by definition
    std::unordered_set<const SfxPoolItem*> aNewSurrogates;

    // 1st source for surrogates
    const registeredSfxItemSets& rSets(GetMasterPool()->maRegisteredSfxItemSets);
    const SfxPoolItem* pItem(nullptr);
    for (const auto& rCand : rSets)
        if (SfxItemState::SET == rCand->GetItemState(nWhich, false, &pItem))
            aNewSurrogates.insert(pItem);

    // 2nd source for surrogates
    const registeredSfxPoolItemHolders& rHolders(GetMasterPool()->maRegisteredSfxPoolItemHolders);
    for (const auto& rCand : rHolders)
        if (rCand->Which() == nWhich && nullptr != rCand->getItem())
            aNewSurrogates.insert(rCand->getItem());

    rTarget = ItemSurrogates(aNewSurrogates.begin(), aNewSurrogates.end());
}

sal_uInt16 SfxItemPool::GetWhichIDFromSlotID(sal_uInt16 nSlotId, bool bDeep) const
{
    if (!IsSlot(nSlotId))
        return nSlotId;

    if (nullptr != mpSlotIDToWhichIDMap)
    {
        // use the static global translation table -> near linear access time
        SlotIDToWhichIDMap::const_iterator aHit(mpSlotIDToWhichIDMap->find(nSlotId));
        if (aHit != mpSlotIDToWhichIDMap->end())
            return aHit->second;
    }

    if (mpSecondary && bDeep)
        return mpSecondary->GetWhichIDFromSlotID(nSlotId);

    return nSlotId;
}


sal_uInt16 SfxItemPool::GetSlotId(sal_uInt16 nWhich) const
{
    if (!IsWhich(nWhich))
        return nWhich;

    SfxItemPool* pTarget(getTargetPool(nWhich));
    if (nullptr == pTarget)
        assert(false && "unknown WhichId - cannot get slot-id");

    const sal_uInt16 nIndex(pTarget->GetIndex_Impl(nWhich));
    const ItemInfo* pInfo(pTarget->maItemInfos[nIndex]);
    assert(nullptr != pInfo && "ITEM: access error to Defaults in Pool (!)");
    const sal_uInt16 nSID(pInfo->getSlotID());
    return (0 != nSID) ? nSID : nWhich;
}


sal_uInt16 SfxItemPool::GetTrueWhichIDFromSlotID( sal_uInt16 nSlotId, bool bDeep ) const
{
    if (!IsSlot(nSlotId))
        return 0;

    if (nullptr != mpSlotIDToWhichIDMap)
    {
        // use the static global translation table -> near linear access time
        SlotIDToWhichIDMap::const_iterator aHit(mpSlotIDToWhichIDMap->find(nSlotId));
        if (aHit != mpSlotIDToWhichIDMap->end())
            return aHit->second;
    }

    if (mpSecondary && bDeep)
        return mpSecondary->GetTrueWhichIDFromSlotID(nSlotId);

    return 0;
}


sal_uInt16 SfxItemPool::GetTrueSlotId( sal_uInt16 nWhich ) const
{
    if (!IsWhich(nWhich))
        return 0;

    SfxItemPool* pTarget(getTargetPool(nWhich));
    if (nullptr == pTarget)
        assert(false && "unknown WhichId - cannot get slot-id");

    const sal_uInt16 nIndex(pTarget->GetIndex_Impl(nWhich));
    const ItemInfo* pInfo(pTarget->maItemInfos[nIndex]);
    assert(nullptr != pInfo && "ITEM: access error to Defaults in Pool (!)");
    return pInfo->getSlotID();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
