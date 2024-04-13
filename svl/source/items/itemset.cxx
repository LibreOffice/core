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


#include <string.h>

#include <algorithm>
#include <cassert>
#include <cstddef>

#include <libxml/xmlwriter.h>

#include <sal/log.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/setitem.hxx>
#include <svl/whiter.hxx>
#include <svl/voiditem.hxx>

#include <items_helper.hxx>

#ifdef DBG_UTIL
static size_t nAllocatedSfxItemSetCount(0);
static size_t nUsedSfxItemSetCount(0);
static size_t nAllocatedSfxPoolItemHolderCount(0);
static size_t nUsedSfxPoolItemHolderCount(0);
size_t getAllocatedSfxItemSetCount() { return nAllocatedSfxItemSetCount; }
size_t getUsedSfxItemSetCount() { return nUsedSfxItemSetCount; }
size_t getAllocatedSfxPoolItemHolderCount() { return nAllocatedSfxPoolItemHolderCount; }
size_t getUsedSfxPoolItemHolderCount() { return nUsedSfxPoolItemHolderCount; }
#endif
// NOTE: Only needed for one Item in SC (see notes below for
// ScPatternAttr/ATTR_PATTERN). Still keep it so that when errors
// come up to this change be able to quickly check using the
// fallback flag 'ITEM_CLASSIC_MODE'
static bool g_bItemClassicMode(getenv("ITEM_CLASSIC_MODE"));

// I thought about this constructor a while, but when there is no
// Item we need no cleanup at destruction (what we would need the
// Pool for), so it is OK and makes default construction easier
// when no Pool is needed. The other constructors guanantee that
// there *cannot* be a state with Item set and Pool not set. IF
// you change this class, ALWAYS ensure that this can not happen (!)
SfxPoolItemHolder::SfxPoolItemHolder()
: m_pPool(nullptr)
, m_pItem(nullptr)
#ifdef DBG_UTIL
, m_bDeleted(false)
#endif
{
#ifdef DBG_UTIL
    nAllocatedSfxPoolItemHolderCount++;
    nUsedSfxPoolItemHolderCount++;
#endif
}

SfxPoolItemHolder::SfxPoolItemHolder(SfxItemPool& rPool, const SfxPoolItem* pItem, bool bPassingOwnership)
: m_pPool(&rPool)
, m_pItem(pItem)
#ifndef NDEBUG
, m_bDeleted(false)
#endif
{
#ifdef DBG_UTIL
    nAllocatedSfxPoolItemHolderCount++;
    nUsedSfxPoolItemHolderCount++;
#endif
    if (nullptr != m_pItem)
        m_pItem = implCreateItemEntry(*m_pPool, m_pItem, m_pItem->Which(), bPassingOwnership);
}

SfxPoolItemHolder::SfxPoolItemHolder(const SfxPoolItemHolder& rHolder)
: m_pPool(rHolder.m_pPool)
, m_pItem(rHolder.m_pItem)
#ifndef NDEBUG
, m_bDeleted(false)
#endif
{
#ifdef DBG_UTIL
    assert(!rHolder.isDeleted() && "Destructed instance used (!)");
    nAllocatedSfxPoolItemHolderCount++;
    nUsedSfxPoolItemHolderCount++;
#endif
    if (nullptr != m_pItem)
        m_pItem = implCreateItemEntry(*m_pPool, m_pItem, m_pItem->Which(), false);
}

SfxPoolItemHolder::~SfxPoolItemHolder()
{
#ifdef DBG_UTIL
    assert(!isDeleted() && "Destructed instance used (!)");
    nAllocatedSfxPoolItemHolderCount--;
#endif
    if (nullptr != m_pItem)
        implCleanupItemEntry(*m_pPool, m_pItem);
#ifdef DBG_UTIL
    m_bDeleted = true;
#endif
}

const SfxPoolItemHolder& SfxPoolItemHolder::operator=(const SfxPoolItemHolder& rHolder)
{
    assert(!isDeleted() && "Destructed instance used (!)");
    assert(!rHolder.isDeleted() && "Destructed instance used (!)");
    if (this == &rHolder || *this == rHolder)
        return *this;

    if (nullptr != m_pItem)
        implCleanupItemEntry(*m_pPool, m_pItem);

    m_pPool = rHolder.m_pPool;
    m_pItem = rHolder.m_pItem;

    if (nullptr != m_pItem)
        m_pItem = implCreateItemEntry(*m_pPool, m_pItem, m_pItem->Which(), false);

    return *this;
}

bool SfxPoolItemHolder::operator==(const SfxPoolItemHolder &rHolder) const
{
    assert(!isDeleted() && "Destructed instance used (!)");
    assert(!rHolder.isDeleted() && "Destructed instance used (!)");
    return m_pPool == rHolder.m_pPool && areSfxPoolItemPtrsEqual(m_pItem, rHolder.m_pItem);
}

/**
 * Ctor for a SfxItemSet with exactly the Which Ranges, which are known to
 * the supplied SfxItemPool.
 *
 * For Sfx programmers: an SfxItemSet constructed in this way cannot
 * contain any Items with SlotIds as Which values.
 *
 * Don't create ItemSets with full range before FreezeIdRanges()!
 */
SfxItemSet::SfxItemSet(SfxItemPool& rPool)
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_nCount(0)
    , m_nTotalCount(svl::detail::CountRanges(rPool.GetFrozenIdRanges()))
    , m_bItemsFixed(false)
    , m_ppItems(new SfxPoolItem const *[m_nTotalCount]{})
    , m_pWhichRanges(rPool.GetFrozenIdRanges())
    , m_aCallback()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxItemSet::SfxItemSet( SfxItemPool& rPool, SfxAllItemSetFlag )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_nCount(0)
    , m_nTotalCount(0)
    , m_bItemsFixed(false)
    , m_ppItems(nullptr)
    , m_pWhichRanges()
    , m_aCallback()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
}

/** special constructor for SfxItemSetFixed */
SfxItemSet::SfxItemSet( SfxItemPool& rPool, WhichRangesContainer&& ranges, SfxPoolItem const ** ppItems, sal_uInt16 nTotalCount )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_nCount(0)
    , m_nTotalCount(nTotalCount)
    , m_bItemsFixed(true)
    , m_ppItems(ppItems)
    , m_pWhichRanges(std::move(ranges))
    , m_aCallback()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    assert(ppItems);
    assert(m_pWhichRanges.size() > 0);
    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxItemSet::SfxItemSet(SfxItemPool& pool, WhichRangesContainer wids)
    : m_pPool(&pool)
    , m_pParent(nullptr)
    , m_nCount(0)
    , m_nTotalCount(svl::detail::CountRanges(wids))
    , m_bItemsFixed(false)
    , m_ppItems(new SfxPoolItem const *[m_nTotalCount]{})
    , m_pWhichRanges(std::move(wids))
    , m_aCallback()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    assert(svl::detail::CountRanges(m_pWhichRanges) != 0);
    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxPoolItem const* implCreateItemEntry(SfxItemPool& rPool, SfxPoolItem const* pSource, sal_uInt16 nWhich, bool bPassingOwnership)
{
    if (nullptr == pSource)
        // SfxItemState::UNKNOWN aka current default (nullptr)
        // just use/return nullptr
        return nullptr;

    if (IsInvalidItem(pSource))
        // SfxItemState::DONTCARE aka invalid item
        // just use pSource which equals INVALID_POOL_ITEM
        return pSource;

    if (pSource->isExceptionalSCItem() && rPool.GetMasterPool()->newItem_UseDirect(*pSource))
        // exceptional handling for *some* items, see SC
        // (do not copy item: use directly, it is a pool default)
        return pSource;

    // CAUTION: static default items are not *that* static as it seems
    // (or: should be). If they are freed with the Pool (see
    // ::ReleaseDefaults) they will be deleted. Same is true for
    // dynamic defaults. Thus currently no default can be shared
    // at all since these may be deleted with the pool owning them.
    // That these are not shared but cloned is ensured by those
    // having a default RefCount of zero, so these are used merely as
    // templates.
    // if (IsStaticDefaultItem(pSource))
    //     return pSource;

    if (0 == pSource->Which())
        // These *should* be SfxVoidItem(0) the only Items with 0 == WhichID,
        // these need to be cloned (currently...)
        return pSource->Clone();

    // get correct target WhichID
    if (0 == nWhich)
        nWhich = pSource->Which();

    if (SfxItemPool::IsSlot(nWhich))
    {
        // SlotItems were always cloned in original (even when bPassingOwnership),
        // so do that here, too (but without bPassingOwnership).
        // They do not need to be registered at pool (actually impossible, pools
        // do not have entries for SlotItems) so handle here early
        if (!bPassingOwnership)
            pSource = pSource->Clone(rPool.GetMasterPool());

        if (pSource->Which() != nWhich)
            const_cast<SfxPoolItem*>(pSource)->SetWhich(nWhich);

        pSource->AddRef();
        return pSource;
    }

    // get the pool with which ItemSets have to work, plus get the
    // pool at which the WhichID is defined, so calls to it do not
    // have to do this repeatedly
    SfxItemPool* pMasterPool(rPool.GetMasterPool());
    assert(nullptr != pMasterPool);
    SfxItemPool* pTargetPool(pMasterPool);
    while (!pTargetPool->IsInRange(nWhich))
        pTargetPool = pTargetPool->GetSecondaryPool();

    // if this goes wrong, an Item with invalid ID for this pool is
    // processed. This is not allowed (and should not happen, e.g.
    // ItemSets already have WhichRanges that are checked against
    // their Pool)
    if (nullptr == pTargetPool)
    {
        assert(false);
        return pSource;
    }

    // CAUTION: Shareable_Impl and NeedsPoolRegistration_Impl
    // use index, not WhichID (one more reason to change the Pools)
    const sal_uInt16 nIndex(pTargetPool->GetIndex_Impl(nWhich));

    // the Item itself is shareable when it already is used somewhere
    // which is equivalent to be referenced already. IsPooledItem also
    // checked for SFX_ITEMS_MAXREF, that is not needed here. Use a
    // fake 'while' loop and 'break' to make this better readable

    // only try to share items that are already shared somehow, else
    // these items are probably not (heap) item-ptr's (but on the
    // stack or else)
    while(pSource->GetRefCount() > 0)
    {
        if (pSource->Which() != nWhich)
            // If the target WhichID differs from the WhichID of a shared Item
            // the item needs to be cloned. This happens, e.g. in
            // ScPatternAttr::GetFromEditItemSet existing items with WhichIDs
            // from EditEngine get set at a SetItem's ItemSet with different
            // target ranges (e.g. look for ATTR_FONT_UNDERLINE)
            break;

        if (!pTargetPool->Shareable_Impl(nIndex))
            // not shareable, done
            break;

        // SfxSetItems cannot be shared if they are in/use another pool
        if (pSource->isSetItem() && static_cast<const SfxSetItem*>(pSource)->GetItemSet().GetPool() != pMasterPool)
            break;

        // If the Item is registered it is pool-dependent, so do not share when
        // it is registered but not at this pool
        if (pSource->isRegisteredAtPool() && !pTargetPool->isSfxPoolItemRegisteredAtThisPool(*pSource))
            break;

        // If we get here we can share the Item
        pSource->AddRef();
        return pSource;
    }

    // g_bItemClassicMode: try finding already existing item
    // NOTE: the UnitTest testIteratorsDefPattern claims that that Item "can be
    //   edited by the user" which explains why it breaks so many rules for Items,
    //   it behaves like an alien. That Item in the SC Pool claims to be a
    //   'StaticDefault' and gets changed (..?)

    // only do this if classic mode or required (calls from Pool::Direct*)
    while(g_bItemClassicMode || pSource->isExceptionalSCItem())
    {
        if (!pTargetPool->Shareable_Impl(nIndex))
            // not shareable, so no need to search for identical item
            break;

        // try to get equal Item. This is the expensive part...
        const SfxPoolItem* pExisting(pTargetPool->tryToGetEqualItem(*pSource, nWhich));

        if (nullptr == pExisting)
            // none found, done
            break;

        if (0 == pExisting->GetRefCount())
            // do not share not-yet shared Items (should not happen)
            break;

        if (bPassingOwnership)
            // need to cleanup if we are offered to own pSource
            delete pSource;

        // If we get here we can share the found Item
        pExisting->AddRef();
        return pExisting;
    }

    // check if the handed over and to be directly used item is a
    // SfxSetItem, that would make it pool-dependent. It then must have
    // the same target-pool, ensure that by the cost of cloning it
    // (should not happen)
    if (bPassingOwnership
        && pSource->isSetItem()
        && static_cast<const SfxSetItem*>(pSource)->GetItemSet().GetPool() != pMasterPool)
    {
        const SfxPoolItem* pOld(pSource);
        pSource = pSource->Clone(pMasterPool);
        delete pOld;
    }

    // when we reach this line we know that we have to add/create a new item. If
    // bPassingOwnership is given just use the item, else clone it
    if (!bPassingOwnership)
        pSource = pSource->Clone(pMasterPool);

    // ensure WhichID @Item
    if (pSource->Which() != nWhich)
        const_cast<SfxPoolItem*>(pSource)->SetWhich(nWhich);

    // increase RefCnt 0->1
    pSource->AddRef();

    // Unfortunately e,g, SC does 'special' things for some new Items,
    // so we need to give the opportunity for this. To limit this to
    // the needed cases, use m_bExceptionalSCItem flag at item
    if (pSource->isExceptionalSCItem())
        pMasterPool->newItem_Callback(*pSource);

    // try to register @Pool (only needed if not yet registered)
    if (!pSource->isRegisteredAtPool())
    {
        bool bRegisterAtPool(pSource->isExceptionalSCItem());

        if (!bRegisterAtPool)
        {
            if (g_bItemClassicMode)
            {
                // in classic mode register only/all shareable items
                bRegisterAtPool = pTargetPool->Shareable_Impl(nIndex);
            }
            else
            {
                // in new mode register only/all items marked as need to be registered
                bRegisterAtPool = pTargetPool->NeedsPoolRegistration_Impl(nIndex);
            }
        }

        if (bRegisterAtPool)
            pTargetPool->registerSfxPoolItem(*pSource);
    }

    return pSource;
}

void implCleanupItemEntry(SfxItemPool& rPool, SfxPoolItem const* pSource)
{
    if (nullptr == pSource)
        // no entry, done
        return;

    if (IsInvalidItem(pSource))
        // nothing to do for invalid item entries
        return;

    if (pSource->isExceptionalSCItem() && rPool.GetMasterPool()->newItem_UseDirect(*pSource))
        // exceptional handling for *some* items, see SC
        // do not delete Item, it is a pool default
        return;

    if (0 == pSource->Which())
    {
        // de-register when registered @pool
        if (pSource->isRegisteredAtPool())
            rPool.unregisterSfxPoolItem(*pSource);

        // These *should* be SfxVoidItem(0) the only Items with 0 == WhichID
        // and need to be deleted
        delete pSource;
        return;
    }

    if (1 < pSource->GetRefCount())
    {
        // Still multiple references present, so just alter the RefCount
        pSource->ReleaseRef();
        return;
    }

    if (IsDefaultItem(pSource))
        // default items (static and dynamic) are owned by the pool, do not delete
        return;

    // decrease RefCnt before deleting (destructor asserts for it and that's
    // good to find other errors)
    pSource->ReleaseRef();

    // de-register before deletion when registered @pool
    if (pSource->isRegisteredAtPool())
        rPool.unregisterSfxPoolItem(*pSource);

    // delete Item
    delete pSource;
}

SfxItemSet::SfxItemSet( const SfxItemSet& rASet )
    : m_pPool( rASet.m_pPool )
    , m_pParent( rASet.m_pParent )
    , m_nCount( rASet.m_nCount )
    , m_nTotalCount( rASet.m_nTotalCount )
    , m_bItemsFixed(false)
    , m_ppItems(nullptr)
    , m_pWhichRanges( rASet.m_pWhichRanges )
    , m_aCallback(rASet.m_aCallback)
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    if (rASet.GetRanges().empty())
    {
        return;
    }

    if (0 == rASet.Count())
    {
        // no Items set in source ItemSet, allocate new array
        // *plus* init to nullptr
        m_ppItems = new const SfxPoolItem* [TotalCount()]{};
        return;
    }

    // allocate new array (no need to initialize, will be done below)
    m_ppItems = new const SfxPoolItem* [TotalCount()];

    // Copy attributes
    SfxPoolItem const** ppDst(m_ppItems);

    for (const auto& rSource : rASet)
    {
        *ppDst = implCreateItemEntry(*GetPool(), rSource, 0, false);
        ppDst++;
    }

    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxItemSet::SfxItemSet(SfxItemSet&& rASet) noexcept
    : m_pPool( rASet.m_pPool )
    , m_pParent( rASet.m_pParent )
    , m_nCount( rASet.m_nCount )
    , m_nTotalCount( rASet.m_nTotalCount )
    , m_bItemsFixed(false)
    , m_ppItems( rASet.m_ppItems )
    , m_pWhichRanges( std::move(rASet.m_pWhichRanges) )
    , m_aCallback(rASet.m_aCallback)
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    if (rASet.m_bItemsFixed)
    {
        // have to make a copy
        m_ppItems = new const SfxPoolItem* [TotalCount()];

        // can just copy the pointers, the ones in the original m_ppItems
        // array will no longer be used/referenced (unused mem, but not lost,
        // it's part of the ItemSet-derived object)
        std::copy(rASet.m_ppItems, rASet.m_ppItems + TotalCount(), m_ppItems);
    }
    else
    {
        // taking over ownership
        rASet.m_nTotalCount = 0;
        rASet.m_ppItems = nullptr;
    }
    rASet.m_pPool = nullptr;
    rASet.m_pParent = nullptr;
    rASet.m_nCount = 0;

    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxItemSet::~SfxItemSet()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount--;
#endif
    // cleanup items. No std::fill needed, we are done with this ItemSet.
    // the callback is not set in destructor, so no worries about that
    ClearAllItemsImpl();

    if (!m_bItemsFixed)
    {
        // free SfxPoolItem array
        delete[] m_ppItems;
    }

    // for invariant-testing
    m_pWhichRanges.reset();
}

// Delete single Items or all Items (nWhich == 0)
sal_uInt16 SfxItemSet::ClearItem( sal_uInt16 nWhich )
{
    if( !Count() )
        return 0;

    if( nWhich )
        return ClearSingleItem_ForWhichID(nWhich);

    // clear all & reset to nullptr
    const sal_uInt16 nRetval(ClearAllItemsImpl());
    std::fill(begin(), begin() + TotalCount(), nullptr);
    return nRetval;
}

sal_uInt16 SfxItemSet::ClearSingleItem_ForWhichID( sal_uInt16 nWhich )
{
    const sal_uInt16 nOffset(GetRanges().getOffsetFromWhich(nWhich));

    if (INVALID_WHICHPAIR_OFFSET != nOffset)
    {
        // found, continue with offset
        return ClearSingleItem_ForOffset(nOffset);
    }

    // not found, return sal_uInt16 nDel = 0;
    return 0;
}

sal_uInt16 SfxItemSet::ClearSingleItem_ForOffset( sal_uInt16 nOffset )
{
    assert(nOffset < TotalCount());
    const_iterator aEntry(begin() + nOffset);
    assert(nullptr == *aEntry || IsInvalidItem(*aEntry) || (*aEntry)->isVoidItem() || 0 != (*aEntry)->Which());

    if (nullptr == *aEntry)
        // no entry, done
        return 0;

    // we reset an entry to nullptr -> decrement count
    --m_nCount;

    // Notification-Callback
    if(m_aCallback)
    {
        m_aCallback(*aEntry, nullptr);
    }

    // cleanup item & reset ptr
    implCleanupItemEntry(*GetPool(), *aEntry);
    *aEntry = nullptr;

    return 1;
}

sal_uInt16 SfxItemSet::ClearAllItemsImpl()
{
    if (0 == Count())
        // no items set, done
        return 0;

    // loop & cleanup items
    for (auto& rCandidate : *this)
    {
        if (nullptr != rCandidate && m_aCallback)
        {
            m_aCallback(rCandidate, nullptr);
        }

        implCleanupItemEntry(*GetPool(), rCandidate);
    }

    // remember count before resetting it, that is the retval
    const sal_uInt16 nRetval(Count());
    m_nCount = 0;
    return nRetval;
}

void SfxItemSet::ClearInvalidItems()
{
    if (0 == Count())
        // no items set, done
        return;

    // loop, here using const_iterator due to need to set ptr in m_ppItems array
    for (const_iterator candidate(begin()); candidate != end(); candidate++)
    {
        if (IsInvalidItem(*candidate))
        {
            *candidate = nullptr;
            --m_nCount;
        }
    }
}

void SfxItemSet::InvalidateAllItems()
{
    // instead of just asserting, do the change. Clear all items
    ClearAllItemsImpl();

    // set all to invalid
    std::fill(begin(), begin() + TotalCount(), INVALID_POOL_ITEM);

    // ...and correct the count - invalid items get counted
    m_nCount = m_nTotalCount;
}

SfxItemState SfxItemSet::GetItemState_ForWhichID( SfxItemState eState, sal_uInt16 nWhich, bool bSrchInParent, const SfxPoolItem **ppItem) const
{
    const sal_uInt16 nOffset(GetRanges().getOffsetFromWhich(nWhich));

    if (INVALID_WHICHPAIR_OFFSET != nOffset)
    {
        // found, continue with offset
        eState = GetItemState_ForOffset(nOffset, ppItem);
    }

    // search in parent?
    if (bSrchInParent && nullptr != GetParent() && (SfxItemState::UNKNOWN == eState || SfxItemState::DEFAULT == eState))
    {
        // nOffset was only valid for *local* SfxItemSet, need to continue with WhichID
        // Use the *highest* SfxItemState as result
        return GetParent()->GetItemState_ForWhichID( eState, nWhich, true, ppItem);
    }

    return eState;
}

SfxItemState SfxItemSet::GetItemState_ForOffset( sal_uInt16 nOffset, const SfxPoolItem **ppItem) const
{
    // check and assert from invalid offset. The caller is responsible for
    // ensuring a valid offset (see callers, all checked & safe)
    assert(nOffset < TotalCount());
    SfxPoolItem const* pCandidate(*(begin() + nOffset));

    if (nullptr == pCandidate)
        // set to Default
        return SfxItemState::DEFAULT;

    if (IsInvalidItem(pCandidate))
        // Different ones are present
        return SfxItemState::DONTCARE;

    if (pCandidate->isVoidItem())
        // Item is Disabled
        return SfxItemState::DISABLED;

    if (nullptr != ppItem)
        // if we have the Item, add it to output an hand back
        *ppItem = pCandidate;

    // Item is set
    return SfxItemState::SET;
}

bool SfxItemSet::HasItem(sal_uInt16 nWhich, const SfxPoolItem** ppItem) const
{
    const bool bRet(SfxItemState::SET == GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, true, ppItem));

    // we need to reset ppItem when it was *not* set by GetItemState_ForWhichID
    // since many usages of that return parameter re-use it, so it might still
    // be set to 'something'
    if (!bRet && nullptr != ppItem)
    {
        *ppItem = nullptr;
    }

    return bRet;
}

const SfxPoolItem* SfxItemSet::PutImpl(const SfxPoolItem& rItem, sal_uInt16 nWhich, bool bPassingOwnership)
{
    bool bActionNeeded(0 != nWhich);
    sal_uInt16 nOffset(INVALID_WHICHPAIR_OFFSET);

#ifdef _WIN32
    // Win build *insists* for initialization, triggers warning C4701:
    // "potentially uninitialized local variable 'aEntry' used" for
    // lines below. This is not the case here, but I know of no way
    // to silence the warning in another way
    const_iterator aEntry(begin());
#else
    const_iterator aEntry;
#endif

    if (bActionNeeded)
    {
        nOffset = GetRanges().getOffsetFromWhich(nWhich);
        bActionNeeded = (INVALID_WHICHPAIR_OFFSET != nOffset);
    }

    if (bActionNeeded)
    {
        aEntry = begin() + nOffset;

        if (nullptr == *aEntry)
        {
            // increase count if there was no entry before
            ++m_nCount;
        }
        else
        {
            // compare items, evtl. containing content compare
            bActionNeeded = !SfxPoolItem::areSame(**aEntry, rItem);
        }
    }

    if (!bActionNeeded)
    {
        if (bPassingOwnership)
        {
            delete &rItem;
        }

        return nullptr;
    }

    // prepare new entry
    SfxPoolItem const* pNew(implCreateItemEntry(*GetPool(), &rItem, nWhich, bPassingOwnership));

    // Notification-Callback
    if(m_aCallback)
    {
        m_aCallback(*aEntry, pNew);
    }

    // cleanup old entry & set entry at m_ppItems array
    implCleanupItemEntry(*GetPool(), *aEntry);
    *aEntry = pNew;

    return pNew;
}

bool SfxItemSet::Put(const SfxItemSet& rSource, bool bInvalidAsDefault)
{
    if (0 == rSource.Count())
        // no items in source, done
        return false;

    const_iterator aSource(rSource.begin());
    sal_uInt16 nNumberToGo(rSource.Count());
    bool bRetval(false);

    // iterate based on WhichIDs to have it available for evtl. PutImpl calls
    for (const WhichPair& rPair : rSource.GetRanges())
    {
        for (sal_uInt16 nWhich(rPair.first); nWhich <= rPair.second; nWhich++, aSource++)
        {
            if (nullptr != *aSource)
            {
                nNumberToGo--;

                if (IsInvalidItem(*aSource))
                {
                    if (bInvalidAsDefault)
                    {
                        bRetval |= 0 != ClearSingleItem_ForWhichID(nWhich);
                    }
                    else
                    {
                        InvalidateItem_ForWhichID(nWhich);
                    }
                }
                else
                {
                    bRetval |= nullptr != PutImpl(**aSource, nWhich, false);
                }
            }

            if (0 == nNumberToGo)
            {
                // we can return early if all set Items are handled
                return bRetval;
            }
        }
    }

    return bRetval;
}

/**
 * This method takes the Items from the 'rSet' and adds to '*this'.
 * Which ranges in '*this' that are non-existent in 'rSet' will not
 * be altered. The Which range of '*this' is also not changed.
 *
 * Items set in 'rSet' are also set in '*this'.
 * Default (0 pointer) and Invalid (-1 pointer) Items are processed
 * according to their parameter 'eDontCareAs' and 'eDefaultAs':
 *
 * SfxItemState::SET:       Hard set to the default of the Pool
 * SfxItemState::DEFAULT:   Deleted (0 pointer)
 * SfxItemState::DONTCARE:  Invalid (-1 pointer)
 *
 * NB: All other values for 'eDontCareAs' and 'eDefaultAs' are invalid
 */
void SfxItemSet::PutExtended
(
    const SfxItemSet&   rSource,        // Source of the Items to be put
    SfxItemState        eDontCareAs,    // What will happen to the DontCare Items
    SfxItemState        eDefaultAs      // What will happen to the Default Items
)
{
    // don't "optimize" with "if( rSource.Count()" because of dontcare + defaults
    const_iterator aSource(rSource.begin());

    for (const WhichPair& rPair : rSource.GetRanges())
    {
        for (sal_uInt16 nWhich = rPair.first; nWhich <= rPair.second; nWhich++, aSource++)
        {
            if (nullptr != *aSource)
            {
                if (IsInvalidItem(*aSource))
                {
                    // Item is DontCare:
                    switch (eDontCareAs)
                    {
                        case SfxItemState::SET:
                            PutImpl(rSource.GetPool()->GetDefaultItem(nWhich), nWhich, false);
                            break;

                        case SfxItemState::DEFAULT:
                            ClearSingleItem_ForWhichID(nWhich);
                            break;

                        case SfxItemState::DONTCARE:
                            InvalidateItem_ForWhichID(nWhich);
                            break;

                        default:
                            assert(!"invalid Argument for eDontCareAs");
                    }
                }
                else
                {
                    // Item is set:
                    PutImpl(**aSource, nWhich, false);
                }
            }
            else
            {
                // Item is default:
                switch (eDefaultAs)
                {
                    case SfxItemState::SET:
                        PutImpl(rSource.GetPool()->GetDefaultItem(nWhich), nWhich, false);
                        break;

                    case SfxItemState::DEFAULT:
                        ClearSingleItem_ForWhichID(nWhich);
                        break;

                    case SfxItemState::DONTCARE:
                        InvalidateItem_ForWhichID(nWhich);
                        break;

                    default:
                        assert(!"invalid Argument for eDefaultAs");
                }
            }
        }
    }
}

/**
 * Expands the ranges of settable items by 'nFrom' to 'nTo'. Keeps state of
 * items which are new ranges too.
 */
void SfxItemSet::MergeRange( sal_uInt16 nFrom, sal_uInt16 nTo )
{
    // check if all from new range are already included. This will
    // use the cache in WhichRangesContainer since we check linearly.
    // Start with assuming all are included, but only if not empty.
    // If empty all included is wrong (and GetRanges().MergeRange
    // will do the right thing/shortcut)
    bool bAllIncluded(!GetRanges().empty());

    for (sal_uInt16 a(nFrom); bAllIncluded && a <= nTo; a++)
        if (INVALID_WHICHPAIR_OFFSET == GetRanges().getOffsetFromWhich(a))
            bAllIncluded = false;

    // if yes, we are done
    if (bAllIncluded)
        return;

    // need to create new WhichRanges
    auto pNewRanges = m_pWhichRanges.MergeRange(nFrom, nTo);
    RecreateRanges_Impl(pNewRanges);
    m_pWhichRanges = std::move(pNewRanges);
}

/**
 * Modifies the ranges of settable items. Keeps state of items which
 * are new ranges too.
 */
void SfxItemSet::SetRanges( const WhichRangesContainer& pNewRanges )
{
    // Identical Ranges?
    if (GetRanges() == pNewRanges)
        return;

    assert(svl::detail::validRanges2(pNewRanges));
    RecreateRanges_Impl(pNewRanges);
    m_pWhichRanges = pNewRanges;
}

void SfxItemSet::SetRanges( WhichRangesContainer&& pNewRanges )
{
    // Identical Ranges?
    if (GetRanges() == pNewRanges)
        return;

    assert(svl::detail::validRanges2(pNewRanges));
    RecreateRanges_Impl(pNewRanges);
    m_pWhichRanges = std::move(pNewRanges);
}

void SfxItemSet::RecreateRanges_Impl(const WhichRangesContainer& rNewRanges)
{
    // create new item-array (by iterating through all new ranges)
    const sal_uInt16 nNewTotalCount(svl::detail::CountRanges(rNewRanges));
    SfxPoolItem const** aNewItemArray(new const SfxPoolItem* [nNewTotalCount]);
    sal_uInt16 nNewCount(0);

    if (0 == Count())
    {
        // no Items set, just reset to nullptr (default)
        std::fill(aNewItemArray, aNewItemArray + nNewTotalCount, nullptr);
    }
    else
    {
        // iterate over target - all entries there need to be set/initialized. Use
        // WhichIDs, we need to access two different WhichRanges
        const_iterator aNewTarget(aNewItemArray);

        for (auto const & rNewRange : rNewRanges)
        {
            for (sal_uInt16 nWhich(rNewRange.first); nWhich <= rNewRange.second; nWhich++, aNewTarget++)
            {
                // check if we have that WhichID in source ranges
                const sal_uInt16 nSourceOffset(GetRanges().getOffsetFromWhich(nWhich));

                if (INVALID_WHICHPAIR_OFFSET == nSourceOffset)
                {
                    // no entry in source, init to nullptr
                    *aNewTarget = nullptr;
                }
                else
                {
                    // we have entry in source, transfer entry - whatever it may be,
                    // also for nullptr (for initialization)
                    const_iterator aSourceEntry(begin() + nSourceOffset);
                    *aNewTarget = *aSourceEntry;

                    // take care of new Count
                    if (nullptr != *aNewTarget)
                    {
                        nNewCount++;
                    }

                    // reset entry, since it got transferred it should not be cleaned-up
                    *aSourceEntry = nullptr;
                }
            }
        }

        // free old items: only necessary when not all Items were transferred
        if (nNewCount != Count())
        {
            ClearAllItemsImpl();
        }
    }

    // replace old items-array and ranges
    if (m_bItemsFixed)
    {
        m_bItemsFixed = false;
    }
    else
    {
        delete[] m_ppItems;
    }

    m_nTotalCount = nNewTotalCount;
    m_ppItems = aNewItemArray;
    m_nCount = nNewCount;
}

/**
 * The SfxItemSet takes over exactly those SfxPoolItems that are
 * set in rSet and are in their own Which range. All others are removed.
 * The SfxItemPool is retained, such that SfxPoolItems that have been
 * taken over, are moved from the rSet's SfxItemPool to the SfxItemPool
 * of *this.
 *
 * SfxPoolItems in rSet, for which holds 'IsInvalidItem() == true' are
 * taken over as invalid items.
 *
 * @return bool true
 *              SfxPoolItems have been taken over
 *
 *              false
 *              No SfxPoolItems have been taken over, because
 *              e.g. the Which ranges of SfxItemSets are not intersecting
 *              or the intersection does not contain SfxPoolItems that are
 *              set in rSet
 */
bool SfxItemSet::Set
(
    const SfxItemSet&   rSet,   /*  The SfxItemSet, whose SfxPoolItems are
                                    to been taken over */

    bool                bDeep   /*  true (default)

                                    The SfxPoolItems from the parents that may
                                    be present in rSet, are also taken over into
                                    this SfxPoolItemSet

                                    false
                                    The SfxPoolItems from the parents of
                                    rSet are not taken into account */
)
{
    bool bRet = false;
    if (Count())
        ClearItem();
    if ( bDeep )
    {
        SfxWhichIter aIter1(*this);
        SfxWhichIter aIter2(rSet);
        sal_uInt16 nWhich1 = aIter1.FirstWhich();
        sal_uInt16 nWhich2 = aIter2.FirstWhich();
        for (;;)
        {
            if (!nWhich1 || !nWhich2)
                break;
            if (nWhich1 > nWhich2)
            {
                nWhich2 = aIter2.NextWhich();
                continue;
            }
            if (nWhich1 < nWhich2)
            {
                nWhich1 = aIter1.NextWhich();
                continue;
            }
            const SfxPoolItem* pItem;
            if( SfxItemState::SET == aIter2.GetItemState( true, &pItem ) )
                bRet |= nullptr != Put( *pItem, pItem->Which() );
            nWhich1 = aIter1.NextWhich();
            nWhich2 = aIter2.NextWhich();
        }
    }
    else
        bRet = Put(rSet, false);

    return bRet;
}

const SfxPoolItem* SfxItemSet::GetItem(sal_uInt16 nId, bool bSearchInParent) const
{
    // evtl. Convert from SlotID to WhichId
    const sal_uInt16 nWhich(GetPool()->GetWhich(nId));

    // Is the Item set or 'bDeep == true' available?
    const SfxPoolItem *pItem(nullptr);
    const SfxItemState eState(GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, bSearchInParent, &pItem));

    if (bSearchInParent && SfxItemState::DEFAULT == eState && SfxItemPool::IsWhich(nWhich))
    {
        pItem = &GetPool()->GetDefaultItem(nWhich);
    }

    return pItem;
}

const SfxPoolItem& SfxItemSet::Get( sal_uInt16 nWhich, bool bSrchInParent) const
{
    // Search the Range in which the Which is located in:
    const sal_uInt16 nOffset(GetRanges().getOffsetFromWhich(nWhich));

    if (INVALID_WHICHPAIR_OFFSET != nOffset)
    {
        const_iterator aFoundOne(begin() + nOffset);

        if (nullptr != *aFoundOne)
        {
            if (IsInvalidItem(*aFoundOne))
            {
                //FIXME: The following code is duplicated further down
                assert(m_pPool);
                //!((SfxAllItemSet *)this)->aDefault.SetWhich(nWhich);
                //!return aDefault;
                return GetPool()->GetDefaultItem(nWhich);
            }
#ifdef DBG_UTIL
            const SfxPoolItem *pItem = *aFoundOne;
            if ( pItem->isVoidItem() || !pItem->Which() )
                SAL_INFO("svl.items", "SFX_WARNING: Getting disabled Item");
#endif
            return **aFoundOne;
        }
    }

    if (bSrchInParent && nullptr != GetParent())
    {
        return GetParent()->Get(nWhich, bSrchInParent);
    }

    // Get the Default from the Pool and return
    assert(m_pPool);
    return GetPool()->GetDefaultItem(nWhich);
}

/**
 * Only retain the Items that are also present in rSet
 * (nevermind their value).
 */
void SfxItemSet::Intersect( const SfxItemSet& rSet )
{
    // Delete all Items not contained in rSet
    assert(m_pPool && "Not implemented without Pool");

    if (!Count() || this == &rSet)
        // none set -> none to delete
        // same ItemSet? -> no Items not contained
        return;

    if (!rSet.Count())
    {
        // no Items contained in rSet -> Delete everything
        ClearItem();
        return;
    }

    // CAUTION: In the former impl, the
    // - version for different ranges checked for SfxItemState::UNKNOWN
    //   in rSet -> this means that the WhichID is *not* defined in
    //   the ranges of rSet *at all* > definitely an *error*
    // - version for same ranges checked for
    //   nullptr != local && nullptr == rSet.
    // All together I think also using the text
    // "Delete all Items not contained in rSet" leads to
    // locally delete all Items that are *not* set in rSet
    // -> != SfxItemState::SET

    // same ranges?
    if (GetRanges() == rSet.GetRanges())
    {
        for (sal_uInt16 nOffset(0); nOffset < TotalCount(); nOffset++)
        {
            if (SfxItemState::SET != rSet.GetItemState_ForOffset(nOffset, nullptr))
            {
                // can use same offset
                ClearSingleItem_ForOffset(nOffset);
            }
        }
    }
    else
    {
        sal_uInt16 nOffset(0);

        for (auto const & rRange : GetRanges())
        {
            for (sal_uInt16 nWhich(rRange.first); nWhich <= rRange.second; nWhich++, nOffset++)
            {
                if (SfxItemState::SET != rSet.GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, false, nullptr))
                {
                    // can use offset
                    ClearSingleItem_ForOffset(nOffset);
                }
            }
        }
    }
}

void SfxItemSet::Differentiate(const SfxItemSet& rSet)
{
    assert(m_pPool && "Not implemented without Pool");

    // Delete all Items contained in rSet
    if (!Count() || !rSet.Count())
        // None set?
        return;

    if (this == &rSet)
    {
        // same ItemSet, all Items are contained -> Delete everything
        ClearItem();
        return;
    }

    // CAUTION: In the former impl, the
    // - version for different ranges checked for SfxItemState::SET
    //   in rSet
    // - version for same ranges checked for
    //   nullptr != local && nullptr != rSet.
    // All together I think also using the text
    // "Delete all Items contained in rSet" leads to
    // locally delete all Items that *are *not* set in rSet
    // -> ==SfxItemState::SET

    // same ranges?
    if (GetRanges() == rSet.GetRanges())
    {
        for (sal_uInt16 nOffset(0); nOffset < TotalCount(); nOffset++)
        {
            if (SfxItemState::SET == rSet.GetItemState_ForOffset(nOffset, nullptr))
            {
                // can use same offset
                ClearSingleItem_ForOffset(nOffset);
            }
        }
    }
    else
    {
        sal_uInt16 nOffset(0);

        for (auto const & rRange : GetRanges())
        {
            for (sal_uInt16 nWhich(rRange.first); nWhich <= rRange.second; nWhich++, nOffset++)
            {
                if (SfxItemState::SET == rSet.GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, false, nullptr))
                {
                    // can use offset
                    ClearSingleItem_ForOffset(nOffset);
                }
            }
        }
    }
}

/**
 * Decision table for MergeValue(s)
 *
 * Principles:
 * 1. If the Which value in the 1st set is "unknown", there's never any action
 * 2. If the Which value in the 2nd set is "unknown", it's made the "default"
 * 3. For comparisons the values of the "default" Items are take into account
 *
 * 1st Item    2nd Item    Values  bIgnoreDefs       Remove      Assign       Add
 *
 * set         set         ==      sal_False           -           -           -
 * default     set         ==      sal_False           -           -           -
 * dontcare    set         ==      sal_False           -           -           -
 * unknown     set         ==      sal_False           -           -           -
 * set         default     ==      sal_False           -           -           -
 * default     default     ==      sal_False           -           -           -
 * dontcare    default     ==      sal_False           -           -           -
 * unknown     default     ==      sal_False           -           -           -
 * set         dontcare    ==      sal_False        1st Item       -1          -
 * default     dontcare    ==      sal_False           -           -1          -
 * dontcare    dontcare    ==      sal_False           -           -           -
 * unknown     dontcare    ==      sal_False           -           -           -
 * set         unknown     ==      sal_False        1st Item       -1          -
 * default     unknown     ==      sal_False           -           -           -
 * dontcare    unknown     ==      sal_False           -           -           -
 * unknown     unknown     ==      sal_False           -           -           -
 *
 * set         set         !=      sal_False        1st Item       -1          -
 * default     set         !=      sal_False           -           -1          -
 * dontcare    set         !=      sal_False           -           -           -
 * unknown     set         !=      sal_False           -           -           -
 * set         default     !=      sal_False        1st Item       -1          -
 * default     default     !=      sal_False           -           -           -
 * dontcare    default     !=      sal_False           -           -           -
 * unknown     default     !=      sal_False           -           -           -
 * set         dontcare    !=      sal_False        1st Item       -1          -
 * default     dontcare    !=      sal_False           -           -1          -
 * dontcare    dontcare    !=      sal_False           -           -           -
 * unknown     dontcare    !=      sal_False           -           -           -
 * set         unknown     !=      sal_False        1st Item       -1          -
 * default     unknown     !=      sal_False           -           -           -
 * dontcare    unknown     !=      sal_False           -           -           -
 * unknown     unknown     !=      sal_False           -           -           -
 *
 * set         set         ==      sal_True            -           -           -
 * default     set         ==      sal_True            -       2nd Item     2nd Item
 * dontcare    set         ==      sal_True            -           -           -
 * unknown     set         ==      sal_True            -           -           -
 * set         default     ==      sal_True            -           -           -
 * default     default     ==      sal_True            -           -           -
 * dontcare    default     ==      sal_True            -           -           -
 * unknown     default     ==      sal_True            -           -           -
 * set         dontcare    ==      sal_True            -           -           -
 * default     dontcare    ==      sal_True            -           -1          -
 * dontcare    dontcare    ==      sal_True            -           -           -
 * unknown     dontcare    ==      sal_True            -           -           -
 * set         unknown     ==      sal_True            -           -           -
 * default     unknown     ==      sal_True            -           -           -
 * dontcare    unknown     ==      sal_True            -           -           -
 * unknown     unknown     ==      sal_True            -           -           -
 *
 * set         set         !=      sal_True         1st Item       -1          -
 * default     set         !=      sal_True            -        2nd Item    2nd Item
 * dontcare    set         !=      sal_True            -           -           -
 * unknown     set         !=      sal_True            -           -           -
 * set         default     !=      sal_True            -           -           -
 * default     default     !=      sal_True            -           -           -
 * dontcare    default     !=      sal_True            -           -           -
 * unknown     default     !=      sal_True            -           -           -
 * set         dontcare    !=      sal_True         1st Item       -1          -
 * default     dontcare    !=      sal_True            -           -1          -
 * dontcare    dontcare    !=      sal_True            -           -           -
 * unknown     dontcare    !=      sal_True            -           -           -
 * set         unknown     !=      sal_True            -           -           -
 * default     unknown     !=      sal_True            -           -           -
 * dontcare    unknown     !=      sal_True            -           -           -
 * unknown     unknown     !=      sal_True            -           -           -
 */
void SfxItemSet::MergeItem_Impl(const SfxPoolItem **ppFnd1, const SfxPoolItem *pFnd2, bool bIgnoreDefaults)
{
    assert(ppFnd1 != nullptr && "Merging to 0-Item");

    // 1st Item is Default?
    if ( !*ppFnd1 )
    {
        if ( IsInvalidItem(pFnd2) )
            // Decision table: default, dontcare, doesn't matter, doesn't matter
            *ppFnd1 = INVALID_POOL_ITEM;

        else if ( pFnd2 && !bIgnoreDefaults &&
                  GetPool()->GetDefaultItem(pFnd2->Which()) != *pFnd2 )
            // Decision table: default, set, !=, sal_False
            *ppFnd1 = INVALID_POOL_ITEM;

        else if ( pFnd2 && bIgnoreDefaults )
            // Decision table: default, set, doesn't matter, sal_True
            *ppFnd1 = implCreateItemEntry(*GetPool(), pFnd2, 0, false);
            // *ppFnd1 = &GetPool()->Put( *pFnd2 );

        if ( *ppFnd1 )
            ++m_nCount;
    }

    // 1st Item set?
    else if ( !IsInvalidItem(*ppFnd1) )
    {
        if ( !pFnd2 )
        {
            // 2nd Item is Default
            if ( !bIgnoreDefaults &&
                 **ppFnd1 != GetPool()->GetDefaultItem((*ppFnd1)->Which()) )
            {
                // Decision table: set, default, !=, sal_False
                implCleanupItemEntry(*GetPool(), *ppFnd1);
                // GetPool()->Remove( **ppFnd1 );
                *ppFnd1 = INVALID_POOL_ITEM;
            }
        }
        else if ( IsInvalidItem(pFnd2) )
        {
            // 2nd Item is dontcare
            if ( !bIgnoreDefaults ||
                 **ppFnd1 != GetPool()->GetDefaultItem( (*ppFnd1)->Which()) )
            {
                // Decision table: set, dontcare, doesn't matter, sal_False
                // or:             set, dontcare, !=, sal_True
                implCleanupItemEntry(*GetPool(), *ppFnd1);
                // GetPool()->Remove( **ppFnd1 );
                *ppFnd1 = INVALID_POOL_ITEM;
            }
        }
        else
        {
            // 2nd Item is set
            if ( **ppFnd1 != *pFnd2 )
            {
                // Decision table: set, set, !=, doesn't matter
                implCleanupItemEntry(*GetPool(), *ppFnd1);
                // GetPool()->Remove( **ppFnd1 );
                *ppFnd1 = INVALID_POOL_ITEM;
            }
        }
    }
}

void SfxItemSet::MergeValues( const SfxItemSet& rSet )
{
    // WARNING! When making changes/fixing bugs, always update the table above!!
    assert( GetPool() == rSet.GetPool() && "MergeValues with different Pools" );

    // CAUTION: Old version did *different* things when the WhichRanges
    // were the same (true) or different (false) (which is an error/
    // false optimization):
    // true:  MergeItem_Impl was directly fed with SfxItem*'s
    //        for entry @this & @rSet
    // false: Looped over rSet WhichID's, fetched defaults from pool,
    //        fed all that to SfxItemSet::MergeValue which then
    //        evtl. could not find that WhichID in local WhichRanges
    // Better to loop over local WhichRanges (these get changed) and look
    // for Item with same WhichID in rSet, this is done now.
    if (GetRanges() == rSet.GetRanges())
    {

        // loop over both & merge, WhichIDs are identical
        for (const_iterator dst(begin()), src(rSet.begin()); dst != end(); dst++, src++)
        {
            MergeItem_Impl(dst, *src, false/*bIgnoreDefaults*/);
        }
    }
    else
    {
        // loop over local which-ranges - the local Items need to be changed
        const_iterator dst(begin());

        for (auto const & rRange : GetRanges())
        {
            for (sal_uInt16 nWhich(rRange.first); nWhich <= rRange.second; nWhich++, dst++)
            {
                // try to get offset in rSet for same WhichID
                const sal_uInt16 nOffset(rSet.GetRanges().getOffsetFromWhich(nWhich));

                if (INVALID_WHICHPAIR_OFFSET != nOffset)
                {
                    // if entry with same WhichID exists in rSet, merge with local entry
                    MergeItem_Impl(dst, *(rSet.begin() + nOffset), false/*bIgnoreDefaults*/);
                }
            }
        }
    }
}

void SfxItemSet::MergeValue(const SfxPoolItem& rAttr, bool bIgnoreDefaults)
{
    if (0 == rAttr.Which())
        // seems to be SfxVoidItem(0), nothing to do
        return;

    const sal_uInt16 nOffset(GetRanges().getOffsetFromWhich(rAttr.Which()));

    if (INVALID_WHICHPAIR_OFFSET != nOffset)
    {
        MergeItem_Impl(begin() + nOffset, &rAttr, bIgnoreDefaults);
    }
}

void SfxItemSet::InvalidateItem_ForWhichID(sal_uInt16 nWhich)
{
    const sal_uInt16 nOffset(GetRanges().getOffsetFromWhich(nWhich));

    if (INVALID_WHICHPAIR_OFFSET != nOffset)
    {
        InvalidateItem_ForOffset(nOffset);
    }
}

void SfxItemSet::InvalidateItem_ForOffset(sal_uInt16 nOffset)
{
    // check and assert from invalid offset. The caller is responsible for
    // ensuring a valid offset (see callers, all checked & safe)
    assert(nOffset < TotalCount());
    const_iterator aFoundOne(begin() + nOffset);

    if (nullptr == *aFoundOne)
    {
        // entry goes from nullptr -> invalid
        ++m_nCount;
    }
    else
    {
        // entry is set
        if (IsInvalidItem(*aFoundOne))
            // already invalid item, done
            return;

        // cleanup entry
        implCleanupItemEntry(*GetPool(), *aFoundOne);
    }

    // set new entry
    *aFoundOne = INVALID_POOL_ITEM;
}

sal_uInt16 SfxItemSet::GetWhichByOffset( sal_uInt16 nOffset ) const
{
    assert(nOffset < TotalCount());

    // 1st try to get a set SfxPoolItem and fetch the WhichID from there.
    const SfxPoolItem* pItem(nullptr);
    (void)GetItemState_ForOffset(nOffset, &pItem);

    if (nullptr != pItem && 0 != pItem->Which())
        return pItem->Which();

    // 2nd have to get from WhichRangesContainer. That might use
    // the buffering, too. We might assert a return value of zero
    // (which means invalid WhichID), but we already assert for
    // a valid offset at the start of this method
    return GetRanges().getWhichFromOffset(nOffset);
}

bool SfxItemSet::operator==(const SfxItemSet &rCmp) const
{
    return Equals( rCmp, true);
}

bool SfxItemSet::Equals(const SfxItemSet &rCmp, bool bComparePool) const
{
    // check if same incarnation
    if (this == &rCmp)
        return true;

    // check parents (if requested, also bComparePool)
    if (bComparePool && GetParent() != rCmp.GetParent())
        return false;

    // check pools (if requested)
    if (bComparePool && GetPool() != rCmp.GetPool())
        return false;

    // check count of set items
    if (Count() != rCmp.Count())
        return false;

    // both have no items, done
    if (0 == Count())
        return true;

    // check if ranges are equal
    if (GetRanges() == rCmp.GetRanges())
    {
        // if yes, we can simplify: are all pointers the same?
        if (0 == memcmp( m_ppItems, rCmp.m_ppItems, TotalCount() * sizeof(m_ppItems[0]) ))
            return true;

        // compare each one separately
        const SfxPoolItem **ppItem1(m_ppItems);
        const SfxPoolItem **ppItem2(rCmp.m_ppItems);

        for (sal_uInt16 nPos(0); nPos < TotalCount(); nPos++)
        {
            // do full SfxPoolItem compare
            if (!SfxPoolItem::areSame(*ppItem1, *ppItem2))
                return false;
            ++ppItem1;
            ++ppItem2;
        }

        return true;
    }

    // Not same ranges, need to compare content. Only need to check if
    // the content of one is inside the other due to already having
    // compared Count() above.
    // Iterate over local SfxItemSet by using locval ranges and offset,
    // so we can access needed data at least for one SfxItemSet more
    // direct. For the 2nd one we need the WhichID which we have by
    // iterating over the ranges.
    sal_uInt16 nOffset(0);
    sal_uInt16 nNumberToGo(Count());

    for (auto const & rRange : GetRanges())
    {
        for (sal_uInt16 nWhich(rRange.first); nWhich <= rRange.second; nWhich++, nOffset++)
        {
            const SfxPoolItem *pItem1(nullptr);
            const SfxPoolItem *pItem2(nullptr);
            const SfxItemState aStateA(GetItemState_ForOffset(nOffset, &pItem1));
            const SfxItemState aStateB(rCmp.GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, false, &pItem2));

            if (aStateA != aStateB)
                return false;

            // only compare items if SfxItemState::SET, else the item ptrs are not set
            if (SfxItemState::SET == aStateA && !SfxPoolItem::areSame(pItem1, pItem2))
                return false;

            if (SfxItemState::DEFAULT != aStateA)
                // if local item is not-nullptr we have compared one more, reduce NumberToGo
                // NOTE: we could also use 'nullptr != *(begin() + nOffset)' here, but the
                //       entry was already checked by GetItemState_ForOffset above
                nNumberToGo--;

            if (0 == nNumberToGo)
                // if we have compared Count() number of items and are still here
                // (all were equal), we can exit early
                return true;
        }
    }

    return true;
}

std::unique_ptr<SfxItemSet> SfxItemSet::Clone(bool bItems, SfxItemPool *pToPool ) const
{
    if (pToPool && pToPool != GetPool())
    {
        std::unique_ptr<SfxItemSet> pNewSet(new SfxItemSet(*pToPool, GetRanges()));
        if ( bItems )
        {
            SfxWhichIter aIter(*pNewSet);
            sal_uInt16 nWhich = aIter.FirstWhich();
            while ( nWhich )
            {
                const SfxPoolItem* pItem;
                if ( SfxItemState::SET == GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, false, &pItem ) )
                    pNewSet->Put( *pItem, pItem->Which() );
                nWhich = aIter.NextWhich();
            }
        }
        return pNewSet;
    }
    else
        return std::unique_ptr<SfxItemSet>(bItems
                ? new SfxItemSet(*this)
                : new SfxItemSet(*GetPool(), GetRanges()));
}

SfxItemSet SfxItemSet::CloneAsValue(bool bItems, SfxItemPool *pToPool ) const
{
    // if you are trying to clone, then the thing you are cloning is polymorphic, which means
    // it cannot be cloned as a value
    assert((typeid(*this) == typeid(SfxItemSet)) && "cannot call this on a subclass of SfxItemSet");

    if (pToPool && pToPool != GetPool())
    {
        SfxItemSet aNewSet(*pToPool, GetRanges());
        if ( bItems )
        {
            SfxWhichIter aIter(aNewSet);
            sal_uInt16 nWhich = aIter.FirstWhich();
            while ( nWhich )
            {
                const SfxPoolItem* pItem;
                if ( SfxItemState::SET == GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, false, &pItem ) )
                    aNewSet.Put( *pItem, pItem->Which() );
                nWhich = aIter.NextWhich();
            }
        }
        return aNewSet;
    }
    else
        return bItems
                ? *this
                : SfxItemSet(*GetPool(), GetRanges());
}

void SfxItemSet::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxItemSet"));
    SfxItemIter aIter(*this);
    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        if (IsInvalidItem(pItem))
        {
            (void)xmlTextWriterStartElement(pWriter, BAD_CAST("invalid"));
            (void)xmlTextWriterEndElement(pWriter);
        }
        else
        {
            pItem->dumpAsXml(pWriter);
        }
    }
    (void)xmlTextWriterEndElement(pWriter);
}


// ----------------------------------------------- class SfxAllItemSet

SfxAllItemSet::SfxAllItemSet( SfxItemPool &rPool )
:   SfxItemSet(rPool, SfxAllItemSetFlag::Flag)
{
}

SfxAllItemSet::SfxAllItemSet(const SfxItemSet &rCopy)
:   SfxItemSet(rCopy)
{
}

/**
 * Explicitly define this ctor to avoid auto-generation by the compiler.
 * The compiler does not take the ctor with the 'const SfxItemSet&'!
 */
SfxAllItemSet::SfxAllItemSet(const SfxAllItemSet &rCopy)
:   SfxItemSet(rCopy)
{
}

/**
 * Putting with automatic extension of the WhichId with the ID of the Item.
 */
const SfxPoolItem* SfxAllItemSet::PutImpl( const SfxPoolItem& rItem, sal_uInt16 nWhich, bool bPassingOwnership )
{
    MergeRange(nWhich, nWhich);
    return SfxItemSet::PutImpl(rItem, nWhich, bPassingOwnership);
}

/**
 * Disable Item
 * Using a VoidItem with Which value 0
 */
void SfxItemSet::DisableItem(sal_uInt16 nWhich)
{
    Put( SfxVoidItem(0), nWhich );
}

std::unique_ptr<SfxItemSet> SfxAllItemSet::Clone(bool bItems, SfxItemPool *pToPool ) const
{
    if (pToPool && pToPool != GetPool())
    {
        std::unique_ptr<SfxAllItemSet> pNewSet(new SfxAllItemSet( *pToPool ));
        if ( bItems )
            pNewSet->Set( *this );
        return pNewSet;
    }
    else
        return std::unique_ptr<SfxItemSet>(bItems ? new SfxAllItemSet(*this) : new SfxAllItemSet(*GetPool()));
}


WhichRangesContainer::WhichRangesContainer( const WhichPair* wids, sal_Int32 nSize )
{
    auto p = new WhichPair[nSize];
    for (int i=0; i<nSize; ++i)
        p[i] = wids[i];
    m_pairs = p;
    m_size = nSize;
    m_bOwnRanges = true;
    m_aLastWhichPairOffset = INVALID_WHICHPAIR_OFFSET;
    m_aLastWhichPairFirst = 0;
    m_aLastWhichPairSecond = 0;
}

WhichRangesContainer::WhichRangesContainer(sal_uInt16 nWhichStart, sal_uInt16 nWhichEnd)
    : m_size(1), m_bOwnRanges(true)
{
    auto p = new WhichPair[1];
    p[0] = { nWhichStart, nWhichEnd };
    m_pairs = p;
    m_aLastWhichPairOffset = INVALID_WHICHPAIR_OFFSET;
    m_aLastWhichPairFirst = 0;
    m_aLastWhichPairSecond = 0;
}

WhichRangesContainer::WhichRangesContainer(WhichRangesContainer && other)
{
    std::swap(m_pairs, other.m_pairs);
    std::swap(m_size, other.m_size);
    std::swap(m_bOwnRanges, other.m_bOwnRanges);
    m_aLastWhichPairOffset = INVALID_WHICHPAIR_OFFSET;
    m_aLastWhichPairFirst = 0;
    m_aLastWhichPairSecond = 0;
}

WhichRangesContainer& WhichRangesContainer::operator=(WhichRangesContainer && other)
{
    std::swap(m_pairs, other.m_pairs);
    std::swap(m_size, other.m_size);
    std::swap(m_bOwnRanges, other.m_bOwnRanges);
    m_aLastWhichPairOffset = INVALID_WHICHPAIR_OFFSET;
    m_aLastWhichPairFirst = 0;
    m_aLastWhichPairSecond = 0;
    return *this;
}

WhichRangesContainer& WhichRangesContainer::operator=(WhichRangesContainer const & other)
{
    reset();
    m_size = other.m_size;
    m_bOwnRanges = other.m_bOwnRanges;
    m_aLastWhichPairOffset = INVALID_WHICHPAIR_OFFSET;
    m_aLastWhichPairFirst = 0;
    m_aLastWhichPairSecond = 0;
    if (m_bOwnRanges)
    {
        auto p = new WhichPair[m_size];
        for (int i=0; i<m_size; ++i)
            p[i] = other.m_pairs[i];
        m_pairs = p;
    }
    else
        m_pairs = other.m_pairs;
    return *this;
}

WhichRangesContainer::~WhichRangesContainer()
{
    reset();
}

bool WhichRangesContainer::operator==(WhichRangesContainer const & other) const
{
    if (m_size != other.m_size)
        return false;
    if (m_pairs == other.m_pairs)
        return true;
    return std::equal(m_pairs, m_pairs + m_size, other.m_pairs, other.m_pairs + m_size);
}


void WhichRangesContainer::reset()
{
    if (m_bOwnRanges)
    {
        delete [] m_pairs;
        m_bOwnRanges = false;
    }
    m_pairs = nullptr;
    m_size = 0;
    m_aLastWhichPairOffset = INVALID_WHICHPAIR_OFFSET;
    m_aLastWhichPairFirst = 0;
    m_aLastWhichPairSecond = 0;
}

#ifdef DBG_UTIL
static size_t g_nHit(0);
static size_t g_nMiss(1);
static bool g_bShowWhichRangesHitRate(getenv("SVL_SHOW_WHICHRANGES_HITRATE"));
static void isHit() { g_nHit++; }
static void isMiss()
{
    g_nMiss++;
    const double fHitRate(double(g_nHit) /double(g_nMiss));
    if (0 == g_nMiss % 1000 && g_bShowWhichRangesHitRate)
        SAL_WARN("svl", "ITEM: hits: " << g_nHit << " misses: " << g_nMiss << " hits/misses(rate): " << fHitRate);
}
#endif

sal_uInt16 WhichRangesContainer::getOffsetFromWhich(sal_uInt16 nWhich) const
{
    // special case for single entry - happens often e.g. UI stuff
    if (m_size == 1)
    {
        if( m_pairs->first <= nWhich && nWhich <= m_pairs->second )
            return nWhich - m_pairs->first;

        // we have only one WhichPair entry and it's not contained -> failed
        return INVALID_WHICHPAIR_OFFSET;
    }

    if (m_size == 0)
        return INVALID_WHICHPAIR_OFFSET;

    // check if nWhich is inside last successfully used WhichPair
    if (INVALID_WHICHPAIR_OFFSET != m_aLastWhichPairOffset
        && m_aLastWhichPairFirst <= nWhich
        && nWhich <= m_aLastWhichPairSecond)
    {
#ifdef DBG_UTIL
        isHit();
#endif
        // we can re-use the last found WhichPair
        return m_aLastWhichPairOffset + (nWhich - m_aLastWhichPairFirst);
    }

#ifdef DBG_UTIL
    isMiss();
#endif

    // we have to find the correct WhichPair, iterate linear. This
    // also directly updates the buffered m_aLastWhichPair* values
    m_aLastWhichPairOffset = 0;

    for (const WhichPair& rPair : *this)
    {
        // Within this range?
        if( rPair.first <= nWhich && nWhich <= rPair.second )
        {
            // found, remember parameters for buffered hits
            m_aLastWhichPairFirst = rPair.first;
            m_aLastWhichPairSecond = rPair.second;

            // ...and return
            return m_aLastWhichPairOffset + (nWhich - m_aLastWhichPairFirst);
        }

        m_aLastWhichPairOffset += rPair.second - rPair.first + 1;
    }

    // *need* to reset: if 1st WhichPair only one entry it could be 1
    // what could wrongly trigger re-use above for next search
    m_aLastWhichPairOffset = INVALID_WHICHPAIR_OFFSET;

    return m_aLastWhichPairOffset;
}

sal_uInt16 WhichRangesContainer::getWhichFromOffset(sal_uInt16 nOffset) const
{
    // special case for single entry - happens often e.g. UI stuff
    if (m_size == 1)
    {
        if (nOffset <= m_pairs->second - m_pairs->first)
            return m_pairs->first + nOffset;

        // we have only one WhichPair entry and it's not contained -> failed
        return 0;
    }

    // check for empty, if yes, return null which is an invalid WhichID
    if (m_size == 0)
        return 0;

    // check if nWhich is inside last successfully used WhichPair
    if (INVALID_WHICHPAIR_OFFSET != m_aLastWhichPairOffset)
    {
        // only try if we are beyond or at m_aLastWhichPairOffset to
        // not get numerically negative
        if (nOffset >= m_aLastWhichPairOffset)
        {
            const sal_uInt16 nAdaptedOffset(nOffset - m_aLastWhichPairOffset);

            if (nAdaptedOffset <= m_aLastWhichPairSecond - m_aLastWhichPairFirst)
            {
#ifdef DBG_UTIL
                isHit();
#endif
                return m_aLastWhichPairFirst + nAdaptedOffset;
            }
        }
    }

#ifdef DBG_UTIL
    isMiss();
#endif

    // Iterate over WhichPairs in WhichRangesContainer
    // Do not update buffered last hit (m_aLastWhichPair*), these calls
    // are potentially more rare than getOffsetFromWhich calls. Still,
    // it could also be done here
    for( auto const & pPtr : *this )
    {
        const sal_uInt16 nWhichPairRange(pPtr.second - pPtr.first);
        if( nOffset <= nWhichPairRange )
            return pPtr.first + nOffset;
        nOffset -= nWhichPairRange + 1;
    }

    // no WhichID found, return invalid one
    return 0;
}

// Adds a range to which ranges, keeping the ranges in valid state (sorted, non-overlapping)
WhichRangesContainer WhichRangesContainer::MergeRange(sal_uInt16 nFrom,
                                       sal_uInt16 nTo) const
{
    assert(svl::detail::validRange(nFrom, nTo));

    if (empty())
        return WhichRangesContainer(nFrom, nTo);

    // reset buffer
    m_aLastWhichPairOffset = INVALID_WHICHPAIR_OFFSET;

    // create vector of ranges (sal_uInt16 pairs of lower and upper bound)
    const size_t nOldCount = size();
    // Allocate one item more than we already have.
    // In the worst case scenario we waste a little bit
    // of memory, but we avoid another allocation, which is more important.
    std::unique_ptr<WhichPair[]> aRangesTable(new WhichPair[nOldCount+1]);
    int aRangesTableSize = 0;
    bool bAdded = false;
    for (const auto& rPair : *this)
    {
        if (!bAdded && rPair.first >= nFrom)
        { // insert new range, keep ranges sorted
            aRangesTable[aRangesTableSize++] = { nFrom, nTo };
            bAdded = true;
        }
        // insert current range
        aRangesTable[aRangesTableSize++] = rPair;
    }
    if (!bAdded)
        aRangesTable[aRangesTableSize++] = { nFrom, nTo };

    // true if ranges overlap or adjoin, false if ranges are separate
    auto needMerge = [](WhichPair lhs, WhichPair rhs) {
        return (lhs.first - 1) <= rhs.second && (rhs.first - 1) <= lhs.second;
    };

    auto it = aRangesTable.get();
    auto endIt = aRangesTable.get() + aRangesTableSize;
    // we have at least one range at this point
    for (;;)
    {
        auto itNext = std::next(it);
        if (itNext == endIt)
            break;
        // check if neighbouring ranges overlap or adjoin
        if (needMerge(*it, *itNext))
        {
            // lower bounds are sorted, implies: it->first = min(it[0].first, it[1].first)
            it->second = std::max(it->second, itNext->second);
            // remove next element
            std::move(std::next(itNext), endIt, itNext);
            --aRangesTableSize;
            endIt = aRangesTable.get() + aRangesTableSize;
        }
        else
            ++it;
    }

    return WhichRangesContainer(std::move(aRangesTable), aRangesTableSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
