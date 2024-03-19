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
#include <unordered_map>

#include <libxml/xmlwriter.h>

#include <sal/log.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/setitem.hxx>
#include <svl/whiter.hxx>
#include <svl/voiditem.hxx>

static bool g_bDisableItemInstanceManager(getenv("SVL_DISABLE_ITEM_INSTANCE_MANAGER"));
static bool g_bShareImmediately(getenv("SVL_SHARE_ITEMS_GLOBALLY_INSTANTLY"));
#define NUMBER_OF_UNSHARED_INSTANCES  (50)

#ifdef DBG_UTIL
static size_t nAllocatedSfxItemSetCount(0);
static size_t nUsedSfxItemSetCount(0);
static size_t nAllocatedSfxPoolItemHolderCount(0);
static size_t nUsedSfxPoolItemHolderCount(0);
size_t getAllocatedSfxItemSetCount() { return nAllocatedSfxItemSetCount; }
size_t getUsedSfxItemSetCount() { return nUsedSfxItemSetCount; }
size_t getAllocatedSfxPoolItemHolderCount() { return nAllocatedSfxPoolItemHolderCount; }
size_t getUsedSfxPoolItemHolderCount() { return nUsedSfxPoolItemHolderCount; }

typedef std::unordered_map<sal_uInt16, std::pair<sal_uInt32, const char*>> HightestUsage;
static HightestUsage aHightestUsage;

static void addUsage(const SfxPoolItem& rCandidate)
{
    HightestUsage::iterator aHit(aHightestUsage.find(rCandidate.Which()));
    if (aHit == aHightestUsage.end())
    {
        aHightestUsage.insert({rCandidate.Which(), {1, typeid(rCandidate).name()}});
        return;
    }
    aHit->second.first++;
}

void listSfxPoolItemsWithHighestUsage(sal_uInt16 nNum)
{
    struct sorted {
        sal_uInt16 nWhich;
        sal_uInt32 nUsage;
        const char* pType;
        sorted(sal_uInt16 _nWhich, sal_uInt32 _nUsage, const char* _pType)
            : nWhich(_nWhich), nUsage(_nUsage), pType(_pType) {}
        bool operator<(const sorted& rDesc) const { return nUsage > rDesc.nUsage; }
    };
    std::vector<sorted> aSorted;
    aSorted.reserve(aHightestUsage.size());
    for (const auto& rEntry : aHightestUsage)
        aSorted.emplace_back(rEntry.first, rEntry.second.first, rEntry.second.second);
    std::sort(aSorted.begin(), aSorted.end());
    sal_uInt16 a(0);
    SAL_INFO("svl.items", "ITEM: List of the " << nNum << " SfxPoolItems with highest non-RefCounted usages:");
    for (const auto& rEntry : aSorted)
    {
        SAL_INFO("svl.items", "  ITEM(" << a << "): Which: " << rEntry.nWhich << " Uses: " << rEntry.nUsage << " Type: " << rEntry.pType);
        if (++a >= nNum)
            break;
    }
}
#endif
// NOTE: Only needed for one Item in SC (see notes below for
// ScPatternAttr). Still keep it so that when errors
// come up to this change be able to quickly check using the
// fallback flag 'ITEM_CLASSIC_MODE'

// I thought about this constructor a while, but when there is no
// Item we need no cleanup at destruction (what we would need the
// Pool for), so it is OK and makes default construction easier
// when no Pool is needed. The other constructors guarantee that
// there *cannot* be a state with Item set and Pool not set. IF
// you change this class, ALWAYS ensure that this can not happen (!)
SfxPoolItemHolder::SfxPoolItemHolder()
: m_pPool(nullptr)
, m_pItem(nullptr)
#ifndef NDEBUG
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
        m_pItem = implCreateItemEntry(getPool(), m_pItem, bPassingOwnership);
    if (nullptr != m_pItem && getPool().NeedsSurrogateSupport(m_pItem->Which()))
        getPool().registerPoolItemHolder(*this);
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
        m_pItem = implCreateItemEntry(getPool(), m_pItem, false);
    if (nullptr != m_pItem && getPool().NeedsSurrogateSupport(m_pItem->Which()))
        getPool().registerPoolItemHolder(*this);
}

SfxPoolItemHolder::~SfxPoolItemHolder()
{
#ifdef DBG_UTIL
    assert(!isDeleted() && "Destructed instance used (!)");
    nAllocatedSfxPoolItemHolderCount--;
#endif
    if (nullptr != m_pItem && getPool().NeedsSurrogateSupport(m_pItem->Which()))
        getPool().unregisterPoolItemHolder(*this);
    if (nullptr != m_pItem)
        implCleanupItemEntry(m_pItem);
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

    // avoid unnecessary unregister/register actions
    const bool bWasRegistered(nullptr != m_pItem && getPool().NeedsSurrogateSupport(m_pItem->Which()));
    const bool bWillBeRegistered(nullptr != rHolder.m_pItem && rHolder.getPool().NeedsSurrogateSupport(rHolder.m_pItem->Which()));
    SfxItemPool* pOldPool(m_pPool);

    if (nullptr != m_pItem)
        implCleanupItemEntry(m_pItem);

    m_pPool = rHolder.m_pPool;
    m_pItem = rHolder.m_pItem;

    if (nullptr != m_pItem)
        m_pItem = implCreateItemEntry(getPool(), m_pItem, false);

    if (bWasRegistered != bWillBeRegistered)
    {
        // adapt registration if needed
        if (bWillBeRegistered)
            getPool().registerPoolItemHolder(*this);
        else
            pOldPool->unregisterPoolItemHolder(*this);
    }

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
 */
SfxItemSet::SfxItemSet(SfxItemPool& rPool)
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_nCount(0)
    , m_nRegister(0)
    , m_bItemsFixed(false)
    , m_ppItems(new const SfxPoolItem*[rPool.GetMergedIdRanges().TotalCount()]{})
    , m_aWhichRanges(rPool.GetMergedIdRanges())
    , m_aCallback()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    assert(m_aWhichRanges.validRanges2());
}

SfxItemSet::SfxItemSet( SfxItemPool& rPool, SfxAllItemSetFlag )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_nCount(0)
    , m_nRegister(0)
    , m_bItemsFixed(false)
    , m_ppItems(nullptr)
    , m_aWhichRanges()
    , m_aCallback()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
}

/** special constructor for SfxItemSetFixed */
SfxItemSet::SfxItemSet( SfxItemPool& rPool, WhichRangesContainer&& ranges, SfxPoolItem const ** ppItems)//, sal_uInt16 nTotalCount )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_nCount(0)
    , m_nRegister(0)
    , m_bItemsFixed(true)
    , m_ppItems(ppItems)
    , m_aWhichRanges(std::move(ranges))
    , m_aCallback()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    assert(ppItems);
    assert(m_aWhichRanges.size() > 0);
    assert(m_aWhichRanges.validRanges2());
}

/** special constructor for SfxItemSetFixed copy constructor */
SfxItemSet::SfxItemSet( const SfxItemSet& rOther,
                        SfxPoolItem const ** ppMyItems )
    : m_pPool(rOther.m_pPool)
    , m_pParent(rOther.m_pParent)
    , m_nCount(rOther.m_nCount)
    , m_nRegister(rOther.m_nRegister)
    , m_bItemsFixed(true)
    , m_ppItems(ppMyItems)
    , m_aWhichRanges(rOther.m_aWhichRanges)
    , m_aCallback(rOther.m_aCallback)
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    assert(ppMyItems);
    assert(m_aWhichRanges.size() > 0);
    assert(m_aWhichRanges.validRanges2());

    if (0 == rOther.Count())
        return;

    // Copy attributes
    SfxPoolItem const** ppDst(m_ppItems);

    for (const auto& rSource : rOther)
    {
        *ppDst = implCreateItemEntry(*GetPool(), rSource, false);
        ppDst++;
    }

    if (0 != m_nRegister)
        GetPool()->registerItemSet(*this);
}

SfxItemSet::SfxItemSet(SfxItemPool& pool, WhichRangesContainer wids)
    : m_pPool(&pool)
    , m_pParent(nullptr)
    , m_nCount(0)
    , m_nRegister(0)
    , m_bItemsFixed(false)
    , m_ppItems(new const SfxPoolItem*[wids.TotalCount()]{})
    , m_aWhichRanges(std::move(wids))
    , m_aCallback()
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    assert(m_aWhichRanges.TotalCount() != 0);
    assert(m_aWhichRanges.validRanges2());
}

// Class that implements global Item sharing. It uses rtti to
// associate every Item-derivation with a possible incarnation
// of a DefaultItemInstanceManager. This is the default, it will
// give direct implementations at the Items that overload
// getItemInstanceManager() preference. These are expected to
// return static instances of a derived implementation of a
// ItemInstanceManager.
// All in all there are now the following possibilities to support
// this for individual Item derivations:
// (1) Do nothing:
//     In that case, if the Item is shareable, the new mechanism
//     will kick in: It will start sharing the Item globally,
//     but not immediately: After a defined amount of allowed
//     non-shared occurrences (look for NUMBER_OF_UNSHARED_INSTANCES)
//     an instance of the default ItemInstanceManager, a
//     DefaultItemInstanceManager, will be incarnated and used.
//     NOTE: Mixing shared/unshared instances is not a problem (we
//     might even implement a kind of 're-hash' when this kicks in,
//     but is not really needed).
// (2) Overload getItemInstanceManager for SfxPoolItem in a class
//     derived from SfxPoolItem and...
// (2a) Return a static incarnation of DefaultItemInstanceManager to
//      immediately start global sharing of that Item derivation.
// (2b) Implement and return your own implementation and static
//      incarnation of ItemInstanceManager to do something better/
//      faster that the default implementation can do. Example:
//      SvxFontItem, uses hashing now.
// There are two supported ENVVARs to use:
// (a) SVL_DISABLE_ITEM_INSTANCE_MANAGER:
//     This disables the mechanism of global Item sharing completely.
//     This can be used to test/check speed/memory needs compared with
//     using it, but also may come in handy to check if evtl. errors/
//     regressions have to do with it.
// (b) SVL_SHARE_ITEMS_GLOBALLY_INSTANTLY:
//     This internally forces the NUMBER_OF_UNSHARED_INSTANCES to be
//     ignored and start sharing ALL Item derivations instantly.
class InstanceManagerHelper
{
    typedef std::unordered_map<std::size_t, std::pair<sal_uInt16, DefaultItemInstanceManager*>> managerTypeMap;
    managerTypeMap  maManagerPerType;

public:
    InstanceManagerHelper() {}
    ~InstanceManagerHelper()
    {
        for (auto& rCandidate : maManagerPerType)
            if (nullptr != rCandidate.second.second)
                delete rCandidate.second.second;
    }

    ItemInstanceManager* getOrCreateItemInstanceManager(const SfxPoolItem& rItem)
    {
        // deactivated?
        if (g_bDisableItemInstanceManager)
            return nullptr;

        // Item cannot be shared?
        if (!rItem.isShareable())
            return nullptr;

        // Prefer getting an ItemInstanceManager directly from
        // the Item: These are the extra implemented (and thus
        // hopefully fastest) incarnations
        ItemInstanceManager* pManager(rItem.getItemInstanceManager());

        // Check for correct hash, there may be derivations of that class.
        // Note that Managers from the Items are *not* added to local list,
        // they are expected to be static instances at the Items
        const std::size_t aHash(typeid(rItem).hash_code());
        if (nullptr != pManager && pManager->getClassHash() == aHash)
            return pManager;

        // check local memory for existing entry
        managerTypeMap::iterator aHit(maManagerPerType.find(aHash));

        // no instance yet
        if (aHit == maManagerPerType.end())
        {
            // create a default one to start usage-counting
            if (g_bShareImmediately)
            {
                // create, insert locally and immediately start sharing
                DefaultItemInstanceManager* pNew(new DefaultItemInstanceManager(aHash));
                maManagerPerType.insert({aHash, std::make_pair(0, pNew)});
                return pNew;
            }

            // start countdown from NUMBER_OF_UNSHARED_INSTANCES until zero is reached
            maManagerPerType.insert({aHash, std::make_pair(NUMBER_OF_UNSHARED_INSTANCES, nullptr)});
            return nullptr;
        }

        // if there is already an ItemInstanceManager incarnated, return it
        if (nullptr != aHit->second.second)
            return aHit->second.second;

        if (aHit->second.first > 0)
        {
            // still not the needed number of hits, countdown & return nullptr
            aHit->second.first--;
            return nullptr;
        }

        // here the countdown is zero and there is not yet a ItemInstanceManager
        // incarnated. Do so, register and return it
        assert(nullptr == aHit->second.second);
        DefaultItemInstanceManager* pNew(new DefaultItemInstanceManager(aHash));
        aHit->second.second = pNew;

        return pNew;
    }

    ItemInstanceManager* getExistingItemInstanceManager(const SfxPoolItem& rItem)
    {
        // deactivated?
        if (g_bDisableItemInstanceManager)
            return nullptr;

        // Item cannot be shared?
        if (!rItem.isShareable())
            return nullptr;

        // Prefer getting an ItemInstanceManager directly from
        // the Item: These are the extra implemented (and thus
        // hopefully fastest) incarnations
        ItemInstanceManager* pManager(rItem.getItemInstanceManager());

        // Check for correct hash, there may be derivations of that class.
        // Note that Managers from the Items are *not* added to local list,
        // they are expected to be static instances at the Items
        const std::size_t aHash(typeid(rItem).hash_code());
        if (nullptr != pManager && pManager->getClassHash() == aHash)
            return pManager;

        // check local memory for existing entry
        managerTypeMap::iterator aHit(maManagerPerType.find(aHash));

        if (aHit == maManagerPerType.end())
            // no instance yet, return nullptr
            return nullptr;

        // if there is already a ItemInstanceManager incarnated, return it
        if (nullptr != aHit->second.second)
            return aHit->second.second;

        // count-up needed number of hits again if item is released
        if (aHit->second.first < NUMBER_OF_UNSHARED_INSTANCES)
            aHit->second.first++;

        return nullptr;
    }
};

// the single static instance that takes over that global Item sharing
static InstanceManagerHelper aInstanceManagerHelper;

SfxPoolItem const* implCreateItemEntry(SfxItemPool& rPool, SfxPoolItem const* pSource, bool bPassingOwnership)
{
    if (nullptr == pSource)
        // SfxItemState::UNKNOWN aka current default (nullptr)
        // just use/return nullptr
        return nullptr;

    // if (IsInvalidItem(pSource))
    //     // SfxItemState::INVALID aka INVALID_POOL_ITEM
    //     // just use pSource which equals INVALID_POOL_ITEM
    //     return pSource;

    // if (IsDisabledItem(pSource))
    //     // SfxItemState::DISABLED aka DISABLED_POOL_ITEM
    //     // just use pSource which equals DISABLED_POOL_ITEM
    //     return pSource;

    if (pSource->isStaticDefault())
        // static default Items can just be used without RefCounting
        // NOTE: This now includes IsInvalidItem/IsDisabledItem
        return pSource;

    if (0 == pSource->Which())
    {
        // There should be no Items with 0 == WhichID, but there are some
        // constructed for dialog return values AKA result (look for SetReturnValue)
        // these need to be cloned (currently...)
        if (bPassingOwnership)
            return pSource;
        return pSource->Clone();
    }

    if (pSource->isDynamicDefault() && rPool.GetPoolDefaultItem(pSource->Which()) == pSource)
        // dynamic defaults are not allowed to 'leave' the Pool they are
        // defined for. We can check by comparing the PoolDefault (the
        // PoolDefaultItem) to pSource by ptr compare (instance). When
        // same Item we can use without RefCount. Else it will be cloned
        // below the standard way.
        return pSource;

#ifdef DBG_UTIL
    // remember WhichID due to being able to assert Clone() error(s)
    const sal_uInt16 nWhich(pSource->Which());
#endif

    if (SfxItemPool::IsSlot(pSource->Which()))
    {
        // SlotItems were always cloned in original (even when bPassingOwnership),
        // so do that here, too (but without bPassingOwnership).
        // They do not need to be registered at pool (actually impossible, pools
        // do not have entries for SlotItems) so handle here early
        if (!bPassingOwnership)
        {
            pSource = pSource->Clone(rPool.GetMasterPool());
            // ARGH! Found out that *some* ::Clone implementations fail to also clone the
            // WhichID set at the original Item, e.g. SfxFrameItem. Assert, this is an error
#ifdef DBG_UTIL
            assert(pSource->Which() == nWhich && "ITEM: Clone of Item did NOT copy/set WhichID (!)");
#endif
        }

        return pSource;
    }

    // get the pool with which ItemSets have to work, plus get the
    // pool at which the WhichID is defined, so calls to it do not
    // have to do this repeatedly
    SfxItemPool* pMasterPool(rPool.GetMasterPool());
    assert(nullptr != pMasterPool);

    // // currently need to check if pSource is a default Item and
    // // avoid it to leave the Pool. This is necessary since Pools
    // // currently delete their default items hard, resetting RefCnt
    // // and killing them. This might change after a Pool refinement.
    // // For now, replace them with the local Pool default. It *might*
    // // be even necessary to replace with a cloned non-default instance
    // // if the defaults differ.
    // // NOTE: Currently even some Pools have no 'real' StaticDefaults,
    // // but these also get deleted (sigh)
    // if (IsStaticDefaultItem(pSource))
    // {
    //     assert(!bPassingOwnership && "ITEM: PassingOwnership not possible combined with StaticDefault (!)");
    //     const SfxPoolItem* pStatic(pTargetPool->GetPoolDefaultItem(pSource->Which()));
    //     if (nullptr != pStatic)
    //     {
    //         if (SfxPoolItem::areSame(pSource, pStatic))
    //             pSource = pStatic;
    //         else
    //         {
    //             pSource = pSource->Clone(pMasterPool);
    //             bPassingOwnership = true;
    //         }
    //     }
    // }
    // else if (IsDefaultItem(pSource))
    // {
    //     assert(!bPassingOwnership && "ITEM: PassingOwnership not possible combined with DynamicDefault (!)");
    //     const SfxPoolItem* pDynamic(pTargetPool->GetUserDefaultItem(pSource->Which()));
    //     if (nullptr != pDynamic)
    //     {
    //         if (SfxPoolItem::areSame(pSource, pDynamic))
    //             pSource = pDynamic;
    //         else
    //         {
    //             pSource = pSource->Clone(pMasterPool);
    //             bPassingOwnership = true;
    //         }
    //     }
    // }

    // The Item itself is shareable when it is used/added at an instance
    // that RefCounts the Item, SfxItemPool or SfxPoolItemHolder. Try
    // to share items that are already shared
    while(pSource->GetRefCount() > 0)
    {
        if (!pSource->isShareable())
            // not shareable, done
            break;

        // SfxSetItems cannot be shared if they are in/use another pool
        if (pSource->isSetItem() && static_cast<const SfxSetItem*>(pSource)->GetItemSet().GetPool() != pMasterPool)
            break;

        // If we get here we can share the Item
        pSource->AddRef();
        return pSource;
    }

    // try to get an ItemInstanceManager for global Item instance sharing
    ItemInstanceManager* pManager(aInstanceManagerHelper.getOrCreateItemInstanceManager(*pSource));

    // check if we can globally share the Item using an ItemInstanceManager
    while (nullptr != pManager)
    {
        const SfxPoolItem* pAlternative(pManager->find(*pSource));
        if(nullptr == pAlternative)
            // no already globally shared one found, done
            break;

        // Here we do *not* need to check if it is an SfxSetItem
        // and cannot be shared if they are in/use another pool:
        // The SfxItemSet::operator== will check for SfxItemPools
        // being equal, thus when found in global share the Pool
        // cannot be equal

        // need to delete evtl. handed over ownership change Item
        if (bPassingOwnership)
            delete pSource;

        // If we get here we can share the Item
        pAlternative->AddRef();
        return pAlternative;
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
#ifdef DBG_UTIL
        assert(pSource->Which() == nWhich && "ITEM: Clone of Item did NOT copy/set WhichID (!)");
#endif
        delete pOld;
    }

#ifdef DBG_UTIL
    // create statistics for listSfxPoolItemsWithHighestUsage
    addUsage(*pSource);
#endif

    // when we reach this line we know that we have to add/create a new item. If
    // bPassingOwnership is given just use the item, else clone it
    if (!bPassingOwnership)
    {
        pSource = pSource->Clone(pMasterPool);
#ifdef DBG_UTIL
        assert(pSource->Which() == nWhich && "ITEM: Clone of Item did NOT copy/set WhichID (!)");
#endif
    }

    // increase RefCnt 0->1
    pSource->AddRef();

    // check if we should register this Item for the global
    // ItemInstanceManager mechanism (only for shareable Items)
    if (nullptr != pManager)
        pManager->add(*pSource);

    return pSource;
}

void implCleanupItemEntry(SfxPoolItem const* pSource)
{
    if (nullptr == pSource)
        // no entry, done
        return;

    // if (IsInvalidItem(pSource))
    //     // SfxItemState::INVALID aka INVALID_POOL_ITEM
    //     // nothing to do for invalid item entries
    //     return;

    // if (IsDisabledItem(pSource))
    //     // SfxItemState::INVALID aka DISABLED_POOL_ITEM
    //     // nothing to do for disabled item entries
    //     return;

    if (pSource->isStaticDefault())
        // static default Items can just be used without RefCounting
        // NOTE: This now includes IsInvalidItem/IsDisabledItem
        return;

    if (0 == pSource->Which())
    {
        // There should be no Items with 0 == WhichID, but there are some
        // constructed for dialog return values AKA result (look for SetReturnValue)
        // and need to be deleted
        delete pSource;
        return;
    }

    if (pSource->isDynamicDefault())
        // dynamic default Items can only be used without RefCounting
        // when same pool. this is already checked at implCreateItemEntry,
        // so it would have been cloned (and would no longer have this
        // flag). So we can just return here
        return;

    if (SfxItemPool::IsSlot(pSource->Which()))
    {
        // SlotItems are cloned, so delete
        delete pSource;
        return;
    }

    if (1 < pSource->GetRefCount())
    {
        // Still multiple references present, so just alter the RefCount
        pSource->ReleaseRef();
        return;
    }

    // try to get an ItemInstanceManager for global Item instance sharing
    ItemInstanceManager* pManager(aInstanceManagerHelper.getExistingItemInstanceManager(*pSource));

    // check if we should/can remove this Item from the global
    // ItemInstanceManager mechanism
    if (nullptr != pManager)
        pManager->remove(*pSource);

    // decrease RefCnt before deleting (destructor asserts for it and that's
    // good to find other errors)
    pSource->ReleaseRef();

    // delete Item
    delete pSource;
}

SfxItemSet::SfxItemSet( const SfxItemSet& rASet )
    : m_pPool( rASet.m_pPool )
    , m_pParent( rASet.m_pParent )
    , m_nCount( rASet.m_nCount )
    , m_nRegister( rASet.m_nRegister )
    , m_bItemsFixed(false)
    , m_ppItems(nullptr)
    , m_aWhichRanges( rASet.m_aWhichRanges )
    , m_aCallback(rASet.m_aCallback)
{
#ifdef DBG_UTIL
    nAllocatedSfxItemSetCount++;
    nUsedSfxItemSetCount++;
#endif
    if (rASet.GetRanges().empty())
        return;

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
        *ppDst = implCreateItemEntry(*GetPool(), rSource, false);
        ppDst++;
    }

    assert(m_aWhichRanges.validRanges2());
    if (0 != m_nRegister)
        GetPool()->registerItemSet(*this);
}

SfxItemSet::SfxItemSet(SfxItemSet&& rASet) noexcept
    : m_pPool( rASet.m_pPool )
    , m_pParent( rASet.m_pParent )
    , m_nCount( rASet.m_nCount )
    , m_nRegister( rASet.m_nRegister )
    , m_bItemsFixed(false)
    , m_ppItems( rASet.m_ppItems )
    , m_aWhichRanges( std::move(rASet.m_aWhichRanges) )
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
        // it's part of the ItemSet-derived object).
        std::copy(rASet.m_ppItems, rASet.m_ppItems + TotalCount(), m_ppItems);
    }

    // deregister if rASet is registered before ptrs vanish
    if (0 != rASet.m_nRegister)
        rASet.GetPool()->unregisterItemSet(rASet);

    // register if new set needs that
    if (0 != m_nRegister)
        GetPool()->registerItemSet(*this);

    // taking over ownership
    rASet.m_pPool = nullptr;
    rASet.m_pParent = nullptr;
    rASet.m_nCount = 0;
    rASet.m_nRegister = 0;
    rASet.m_ppItems = nullptr;
    rASet.m_aWhichRanges.reset();
    rASet.m_aCallback = nullptr;

    assert(m_aWhichRanges.validRanges2());
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
    m_aWhichRanges.reset();
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

void SfxItemSet::checkRemovePoolRegistration(const SfxPoolItem* pItem)
{
    if (nullptr == pItem)
        // no Item, done
        return;

    if (IsInvalidItem(pItem) || IsDisabledItem(pItem))
        // checks IsInvalidItem/IsDisabledItem
        return;

    if (SfxItemPool::IsSlot(pItem->Which()))
        // no slots, these do not support NeedsSurrogateSupport
        return;

    if(!GetPool()->NeedsSurrogateSupport(pItem->Which()))
        // not needed for this item, done
        return;

    // there must be a registered one
    assert(0 != m_nRegister);

    // decrement counter
    m_nRegister--;

    // deregister when no more Items that NeedsSurrogateSupport exist
    if (0 == m_nRegister)
        GetPool()->unregisterItemSet(*this);
}

void SfxItemSet::checkAddPoolRegistration(const SfxPoolItem* pItem)
{
    if (nullptr == pItem)
        // no Item, done
        return;

    if (IsInvalidItem(pItem) || IsDisabledItem(pItem))
        // checks IsInvalidItem/IsDisabledItem
        return;

    if (SfxItemPool::IsSlot(pItem->Which()))
        // no slots, these do not support NeedsSurrogateSupport
        return;

    if(!GetPool()->NeedsSurrogateSupport(pItem->Which()))
        // not needed for this item, done
        return;

    // there cannot be more than m_nCount, *but* use one more to
    // allow paired Remove/Add calls (see SfxItemSet::PutImpl)
    assert(m_nRegister <= m_nCount);

    // register when first Item that NeedsSurrogateSupport exist
    if (0 == m_nRegister)
        GetPool()->registerItemSet(*this);

    // increment counter
    m_nRegister++;
}

sal_uInt16 SfxItemSet::ClearSingleItem_ForOffset( sal_uInt16 nOffset )
{
    assert(nOffset < TotalCount());
    const_iterator aEntry(begin() + nOffset);
    assert(nullptr == *aEntry || IsInvalidItem(*aEntry) || IsDisabledItem(*aEntry) || 0 != (*aEntry)->Which());

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

    // check register for remove
    checkRemovePoolRegistration(*aEntry);

    // cleanup item & reset ptr
    implCleanupItemEntry(*aEntry);
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

        implCleanupItemEntry(rCandidate);
    }

    // remember count before resetting it, that is the retval
    const sal_uInt16 nRetval(Count());
    m_nCount = 0;

    if (0 != m_nRegister)
    {
        GetPool()->unregisterItemSet(*this);
        m_nRegister = 0;
    }

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
        return SfxItemState::INVALID;

    if (IsDisabledItem(pCandidate))
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

const SfxPoolItem* SfxItemSet::PutImplAsTargetWhich(const SfxPoolItem& rItem, sal_uInt16 nTargetWhich, bool bPassingOwnership)
{
    if (0 == nTargetWhich || nTargetWhich == rItem.Which())
        // nTargetWhich not different or not given, use default
        return PutImpl(rItem, bPassingOwnership);

    if (bPassingOwnership && 0 == rItem.GetRefCount())
    {
        // we *can* use rItem when it's not pooled AKA has no RefCount
        const_cast<SfxPoolItem&>(rItem).SetWhich(nTargetWhich);
        return PutImpl(rItem, true);
    }

    // else we have to create a clone, set WhichID at it and
    // delete rItem when bPassingOwnership was intended
    SfxPoolItem* pClone(rItem.Clone(GetPool()));
    pClone->SetWhich(nTargetWhich);
    if (bPassingOwnership)
        delete &rItem;
    return PutImpl(*pClone, true);
}

const SfxPoolItem* SfxItemSet::PutImpl(const SfxPoolItem& rItem, bool bPassingOwnership)
{
    if (IsDisabledItem(&rItem))
    {
        // no action needed: IsDisabledItem
        if (bPassingOwnership)
            delete &rItem;
        return nullptr;
    }

#ifdef _WIN32
    // Win build *insists* for initialization, triggers warning C4701:
    // "potentially uninitialized local variable 'aEntry' used" for
    // lines below. This is not the case here, but I know of no way
    // to silence the warning in another way
    const_iterator aEntry(begin());
#else
    const_iterator aEntry;
#endif

    sal_uInt16 nOffset(GetRanges().getOffsetFromWhich(rItem.Which()));
    bool bActionNeeded(INVALID_WHICHPAIR_OFFSET != nOffset);

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
            delete &rItem;
        return nullptr;
    }

    // prepare new entry
    const SfxPoolItem* pNew(implCreateItemEntry(*GetPool(), &rItem, bPassingOwnership));

    // Notification-Callback
    if(m_aCallback)
    {
        m_aCallback(*aEntry, pNew);
    }

    // check register for add/remove. add first so that unregister/register
    // is avoided when an Item is replaced (increase, decrease, do not reach 0)
    checkAddPoolRegistration(pNew);
    checkRemovePoolRegistration(*aEntry);

    // cleanup old entry & set entry at m_ppItems array
    implCleanupItemEntry(*aEntry);
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
                        DisableOrInvalidateItem_ForWhichID(false, nWhich);
                    }
                }
                else
                {
                    bRetval |= nullptr != PutImpl(**aSource, false);
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
 * SfxItemState::INVALID:  Invalid (-1 pointer)
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
                            PutImpl(rSource.GetPool()->GetUserOrPoolDefaultItem(nWhich), false);
                            break;

                        case SfxItemState::DEFAULT:
                            ClearSingleItem_ForWhichID(nWhich);
                            break;

                        case SfxItemState::INVALID:
                            DisableOrInvalidateItem_ForWhichID(false, nWhich);
                            break;

                        default:
                            assert(!"invalid Argument for eDontCareAs");
                    }
                }
                else
                {
                    // Item is set:
                    PutImpl(**aSource, false);
                }
            }
            else
            {
                // Item is default:
                switch (eDefaultAs)
                {
                    case SfxItemState::SET:
                        PutImpl(rSource.GetPool()->GetUserOrPoolDefaultItem(nWhich), false);
                        break;

                    case SfxItemState::DEFAULT:
                        ClearSingleItem_ForWhichID(nWhich);
                        break;

                    case SfxItemState::INVALID:
                        DisableOrInvalidateItem_ForWhichID(false, nWhich);
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
    auto aNewRanges = m_aWhichRanges.MergeRange(nFrom, nTo);
    RecreateRanges_Impl(aNewRanges);
    m_aWhichRanges = std::move(aNewRanges);
}

/**
 * Modifies the ranges of settable items. Keeps state of items which
 * are new ranges too.
 */
void SfxItemSet::SetRanges( const WhichRangesContainer& aNewRanges )
{
    // Identical Ranges?
    if (GetRanges() == aNewRanges)
        return;

    assert(aNewRanges.validRanges2());
    RecreateRanges_Impl(aNewRanges);
    m_aWhichRanges = aNewRanges;
}

void SfxItemSet::SetRanges( WhichRangesContainer&& aNewRanges )
{
    // Identical Ranges?
    if (GetRanges() == aNewRanges)
        return;

    assert(aNewRanges.validRanges2());
    RecreateRanges_Impl(aNewRanges);
    m_aWhichRanges = std::move(aNewRanges);
}

void SfxItemSet::RecreateRanges_Impl(const WhichRangesContainer& rNewRanges)
{
    // create new item-array (by iterating through all new ranges)
    const sal_uInt16 nNewTotalCount(rNewRanges.TotalCount());
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
                bRet |= nullptr != Put( *pItem );
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
    const sal_uInt16 nWhich(GetPool()->GetWhichIDFromSlotID(nId));

    // Is the Item set or 'bDeep == true' available?
    const SfxPoolItem *pItem(nullptr);
    const SfxItemState eState(GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, bSearchInParent, &pItem));

    if (bSearchInParent && SfxItemState::DEFAULT == eState && SfxItemPool::IsWhich(nWhich))
    {
        pItem = &GetPool()->GetUserOrPoolDefaultItem(nWhich);
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
                return GetPool()->GetUserOrPoolDefaultItem(nWhich);
            }
#ifdef DBG_UTIL
            if (IsDisabledItem(*aFoundOne))
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
    return GetPool()->GetUserOrPoolDefaultItem(nWhich);
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
        ClearAllItemsImpl();
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
        ClearAllItemsImpl();
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
                  GetPool()->GetUserOrPoolDefaultItem(pFnd2->Which()) != *pFnd2 )
            // Decision table: default, set, !=, sal_False
            *ppFnd1 = INVALID_POOL_ITEM;

        else if ( pFnd2 && bIgnoreDefaults )
            // Decision table: default, set, doesn't matter, sal_True
            *ppFnd1 = implCreateItemEntry(*GetPool(), pFnd2, false);
            // *ppFnd1 = &GetPool()->Put( *pFnd2 );

        if ( *ppFnd1 )
        {
            ++m_nCount;
            checkAddPoolRegistration(*ppFnd1);
        }
    }

    // 1st Item set?
    else if ( !IsInvalidItem(*ppFnd1) )
    {
        if ( !pFnd2 )
        {
            // 2nd Item is Default
            if ( !bIgnoreDefaults &&
                 **ppFnd1 != GetPool()->GetUserOrPoolDefaultItem((*ppFnd1)->Which()) )
            {
                // Decision table: set, default, !=, sal_False
                checkRemovePoolRegistration(*ppFnd1);
                implCleanupItemEntry(*ppFnd1);
                // GetPool()->Remove( **ppFnd1 );
                *ppFnd1 = INVALID_POOL_ITEM;
            }
        }
        else if ( IsInvalidItem(pFnd2) )
        {
            // 2nd Item is dontcare
            if ( !bIgnoreDefaults ||
                 **ppFnd1 != GetPool()->GetUserOrPoolDefaultItem( (*ppFnd1)->Which()) )
            {
                // Decision table: set, dontcare, doesn't matter, sal_False
                // or:             set, dontcare, !=, sal_True
                checkRemovePoolRegistration(*ppFnd1);
                implCleanupItemEntry(*ppFnd1);
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
                checkRemovePoolRegistration(*ppFnd1);
                implCleanupItemEntry(*ppFnd1);
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
    if (IsDisabledItem(&rAttr))
        // is IsDisabledItem, nothing to do
        return;

    const sal_uInt16 nOffset(GetRanges().getOffsetFromWhich(rAttr.Which()));

    if (INVALID_WHICHPAIR_OFFSET != nOffset)
    {
        MergeItem_Impl(begin() + nOffset, &rAttr, bIgnoreDefaults);
    }
}

void SfxItemSet::DisableOrInvalidateItem_ForWhichID(bool bDisable, sal_uInt16 nWhich)
{
    const sal_uInt16 nOffset(GetRanges().getOffsetFromWhich(nWhich));

    if (INVALID_WHICHPAIR_OFFSET != nOffset)
    {
        DisableOrInvalidateItem_ForOffset(bDisable, nOffset);
    }
}

void SfxItemSet::DisableOrInvalidateItem_ForOffset(bool bDisable, sal_uInt16 nOffset)
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
        if (bDisable && IsDisabledItem(*aFoundOne))
            // already disabled item, done
            return;

        if (!bDisable && IsInvalidItem(*aFoundOne))
            // already invalid item, done
            return;


        // cleanup entry
        checkRemovePoolRegistration(*aFoundOne);
        implCleanupItemEntry(*aFoundOne);
    }

    // set new entry
    *aFoundOne = bDisable ? DISABLED_POOL_ITEM : INVALID_POOL_ITEM;
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
                    pNewSet->Put( *pItem );
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
                    aNewSet.Put( *pItem );
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
const SfxPoolItem* SfxAllItemSet::PutImpl( const SfxPoolItem& rItem, bool bPassingOwnership )
{
    MergeRange(rItem.Which(), rItem.Which());
    return SfxItemSet::PutImpl(rItem, bPassingOwnership);
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


void WhichRangesContainer::CountRanges() const
{
    m_TotalCount = 0;
    for (const auto& rPair : *this)
        m_TotalCount += svl::detail::rangeSize(rPair.first, rPair.second);
}

WhichRangesContainer::WhichRangesContainer( const WhichPair* wids, sal_Int32 nSize )
: m_pairs(nullptr)
, m_size(nSize)
, m_TotalCount(0)
, m_aLastWhichPairOffset(INVALID_WHICHPAIR_OFFSET)
, m_aLastWhichPairFirst(0)
, m_aLastWhichPairSecond(0)
, m_bOwnRanges(true)
{
    auto p = new WhichPair[nSize];
    for (int i=0; i<nSize; ++i)
        p[i] = wids[i];
    m_pairs = p;
    CountRanges();
}

WhichRangesContainer::WhichRangesContainer(sal_uInt16 nWhichStart, sal_uInt16 nWhichEnd)
: m_pairs(nullptr)
, m_size(1)
, m_TotalCount(0)
, m_aLastWhichPairOffset(INVALID_WHICHPAIR_OFFSET)
, m_aLastWhichPairFirst(0)
, m_aLastWhichPairSecond(0)
, m_bOwnRanges(true)
{
    auto p = new WhichPair[1];
    p[0] = { nWhichStart, nWhichEnd };
    m_pairs = p;
    CountRanges();
}

WhichRangesContainer::WhichRangesContainer(WhichRangesContainer && other)
{
    std::swap(m_pairs, other.m_pairs);
    std::swap(m_size, other.m_size);
    std::swap(m_TotalCount, other.m_TotalCount);
    std::swap(m_aLastWhichPairOffset, other.m_aLastWhichPairOffset);
    std::swap(m_aLastWhichPairFirst, other.m_aLastWhichPairFirst);
    std::swap(m_aLastWhichPairSecond, other.m_aLastWhichPairSecond);
    std::swap(m_bOwnRanges, other.m_bOwnRanges);
}

WhichRangesContainer& WhichRangesContainer::operator=(WhichRangesContainer && other)
{
    std::swap(m_pairs, other.m_pairs);
    std::swap(m_size, other.m_size);
    std::swap(m_TotalCount, other.m_TotalCount);
    std::swap(m_aLastWhichPairOffset, other.m_aLastWhichPairOffset);
    std::swap(m_aLastWhichPairFirst, other.m_aLastWhichPairFirst);
    std::swap(m_aLastWhichPairSecond, other.m_aLastWhichPairSecond);
    std::swap(m_bOwnRanges, other.m_bOwnRanges);
    return *this;
}

WhichRangesContainer& WhichRangesContainer::operator=(WhichRangesContainer const & other)
{
    reset();

    m_size = other.m_size;
    m_TotalCount = other.m_TotalCount;
    m_aLastWhichPairOffset = other.m_aLastWhichPairOffset;
    m_aLastWhichPairFirst = other.m_aLastWhichPairFirst;
    m_aLastWhichPairSecond = other.m_aLastWhichPairSecond;
    m_bOwnRanges = other.m_bOwnRanges;

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
    if (m_TotalCount != other.m_TotalCount)
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
    m_TotalCount = 0;
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
    if (empty())
        return INVALID_WHICHPAIR_OFFSET;

    // special case for single entry - happens often e.g. UI stuff
    if (1 == m_size)
    {
        if( m_pairs->first <= nWhich && nWhich <= m_pairs->second )
            return nWhich - m_pairs->first;

        // we have only one WhichPair entry and it's not contained -> failed
        return INVALID_WHICHPAIR_OFFSET;
    }

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
    // check for empty, if yes, return null which is an invalid WhichID
    if (empty())
        return 0;

    // special case for single entry - happens often e.g. UI stuff
    if (1 == m_size)
    {
        if (nOffset <= m_pairs->second - m_pairs->first)
            return m_pairs->first + nOffset;

        // we have only one WhichPair entry and it's not contained -> failed
        return 0;
    }

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
