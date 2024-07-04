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

#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/setitem.hxx>
#include <sal/log.hxx>
#include <unordered_map>
#include <unordered_set>
#include <memory>

// Classes that implement global SfxPoolItem sharing.

#ifdef DBG_UTIL

// <WhichID, <number of entries, typeid_name>>
typedef std::unordered_map<sal_uInt16, std::pair<sal_uInt32, const char*>> HightestUsage;
static HightestUsage aHightestUsage;

static void addUsage(const SfxPoolItem& rCandidate)
{
    HightestUsage::iterator aHit(aHightestUsage.find(rCandidate.Which()));
    if (aHit == aHightestUsage.end())
    {
        aHightestUsage.insert({ rCandidate.Which(), { 1, typeid(rCandidate).name() } });
        return;
    }
    aHit->second.first++;
}

void listSfxPoolItemsWithHighestUsage(sal_uInt16 nNum)
{
    struct sorted
    {
        sal_uInt16 nWhich;
        sal_uInt32 nUsage;
        const char* pType;
        sorted(sal_uInt16 _nWhich, sal_uInt32 _nUsage, const char* _pType)
            : nWhich(_nWhich)
            , nUsage(_nUsage)
            , pType(_pType)
        {
        }
        bool operator<(const sorted& rDesc) const { return nUsage > rDesc.nUsage; }
    };
    std::vector<sorted> aSorted;
    aSorted.reserve(aHightestUsage.size());
    for (const auto& rEntry : aHightestUsage)
        aSorted.emplace_back(rEntry.first, rEntry.second.first, rEntry.second.second);
    std::sort(aSorted.begin(), aSorted.end());
    sal_uInt16 a(0);
    SAL_INFO("svl.items",
             "ITEM: List of the " << nNum << " SfxPoolItems with highest non-RefCounted usages:");
    for (const auto& rEntry : aSorted)
    {
        SAL_INFO("svl.items", "  ITEM(" << a << "): Which: " << rEntry.nWhich
                                        << " Uses: " << rEntry.nUsage << " Type: " << rEntry.pType);
        if (++a >= nNum)
            break;
    }
}

#endif

namespace
{
// basic Interface definition
struct ItemInstanceManager
{
    virtual ~ItemInstanceManager() {}
    // standard interface, accessed exclusively
    // by implCreateItemEntry/implCleanupItemEntry
    virtual const SfxPoolItem* find(const SfxPoolItem&) const = 0;
    virtual bool add(const SfxPoolItem&) = 0;
    virtual void remove(const SfxPoolItem&) = 0;
};

// offering a default implementation that can be use for
// each SfxPoolItem (except when !isShareable()). It just
// uses an unordered_set holding ptrs to SfxPoolItems added
// and SfxPoolItem::operator== to linearly search for one.
// Thus this is not the fastest, but as fast as old 'poooled'
// stuff - better use an intelligent, pro-Item implementation
// that does e.g. hashing or whatever might be feasible for
// that specific Item (see other derivations)
struct DefaultItemInstanceManager : public ItemInstanceManager
{
    std::unordered_set<const SfxPoolItem*> maRegistered;

    virtual const SfxPoolItem* find(const SfxPoolItem& rItem) const override
    {
        for (const auto& rCandidate : maRegistered)
            if (*rCandidate == rItem)
                return rCandidate;
        return nullptr;
    }
    virtual bool add(const SfxPoolItem& rItem) override
    {
        return maRegistered.insert(&rItem).second;
    }
    virtual void remove(const SfxPoolItem& rItem) override
    {
        bool bSuccess = maRegistered.erase(&rItem);
        assert(bSuccess && "removing item but it is already gone");
        (void)bSuccess;
    }
};

struct HashableItemInstanceManager : public ItemInstanceManager
{
    struct ItemHash
    {
        size_t operator()(const SfxPoolItem* pItem) const { return pItem->hashCode(); }
    };
    struct ItemEqual
    {
        bool operator()(const SfxPoolItem* lhs, const SfxPoolItem* rhs) const
        {
            return *lhs == *rhs;
        }
    };
    std::unordered_set<const SfxPoolItem*, ItemHash, ItemEqual> maRegistered;

    virtual const SfxPoolItem* find(const SfxPoolItem& rItem) const override
    {
        auto it = maRegistered.find(&rItem);
        if (it == maRegistered.end())
            return nullptr;
        return *it;
    }
    virtual bool add(const SfxPoolItem& rItem) override
    {
        return maRegistered.insert(&rItem).second;
    }
    virtual void remove(const SfxPoolItem& rItem) override
    {
        auto it = maRegistered.find(&rItem);
        if (it != maRegistered.end())
        {
            if (&rItem != *it)
            {
                SAL_WARN("svl", "erasing wrong object, hash/operator== methods likely bad "
                                    << typeid(rItem).name());
                assert(false && "erasing wrong object, hash/operator== methods likely bad");
            }
            maRegistered.erase(it);
        }
        else
        {
            SAL_WARN("svl",
                     "removing item but its already gone, hash/operator== methods likely bad "
                         << typeid(rItem).name());
            assert(false
                   && "removing item but its already gone, hash/operator== methods likely bad");
        }
    }
};

struct PairHash
{
    size_t operator()(const std::pair<SfxItemType, sal_uInt16>& rKey) const
    {
        return (static_cast<int>(rKey.first) << 16) | rKey.second;
    }
};
}
// The single static instance that takes over that global Item sharing
// Maps SfxItemPool sub-classes to a set of shared items.
//
// Noting that the WhichId is part of the key, to simply the implementation of the hashCode() overrides
// in SfxPoolItem sub-classes.
static std::unordered_map<std::pair<SfxItemType, sal_uInt16>, std::unique_ptr<ItemInstanceManager>,
                          PairHash>
    gInstanceManagerMap;

SfxPoolItem const* implCreateItemEntry(SfxItemPool& rPool, SfxPoolItem const* pSource,
                                       bool bPassingOwnership)
{
    if (nullptr == pSource)
        // SfxItemState::UNKNOWN aka current default (nullptr)
        // just use/return nullptr
        return nullptr;

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
            assert(pSource->Which() == nWhich
                   && "ITEM: Clone of Item did NOT copy/set WhichID (!)");
#endif
        }

        return pSource;
    }

    // get the pool with which ItemSets have to work, plus get the
    // pool at which the WhichID is defined, so calls to it do not
    // have to do this repeatedly
    SfxItemPool* pMasterPool(rPool.GetMasterPool());
    assert(nullptr != pMasterPool);

    // The Item itself is shareable when it is used/added at an instance
    // that RefCounts the Item, SfxItemPool or SfxPoolItemHolder. Try
    // to share items that are already shared
    while (pSource->GetRefCount() > 0)
    {
        if (!pSource->isShareable())
            // not shareable, done
            break;

        // SfxSetItems cannot be shared if they are in/use another pool
        if (pSource->isSetItem()
            && static_cast<const SfxSetItem*>(pSource)->GetItemSet().GetPool() != pMasterPool)
            break;

        // If we get here we can share the Item
        pSource->AddRef();
        return pSource;
    }

    // Item cannot be shared?
    SfxItemType nSourceItemType(pSource->ItemType());
    if (pSource->isShareable())
    {
        // check if we can globally share the Item using the ItemInstanceManager
        auto itemsetIt = gInstanceManagerMap.find({ nSourceItemType, pSource->Which() });
        if (itemsetIt != gInstanceManagerMap.end())
        {
            ItemInstanceManager& rItemManager = *(itemsetIt->second);
            const SfxPoolItem* pAlternative = rItemManager.find(*pSource);
            if (pAlternative)
            {
                SAL_WARN_IF(typeid(*pAlternative) != typeid(*pSource), "svl",
                            "wrong item from pool, expected " << typeid(*pSource).name()
                                                              << " but got "
                                                              << typeid(*pAlternative).name());
                assert(typeid(*pAlternative) == typeid(*pSource) && "wrong item from pool");
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
        }
    }

    // check if the handed over and to be directly used item is a
    // SfxSetItem, that would make it pool-dependent. It then must have
    // the same target-pool, ensure that by the cost of cloning it
    // (should not happen)
    if (bPassingOwnership && pSource->isSetItem()
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
        auto pPreviousSource = pSource;
        pSource = pSource->Clone(pMasterPool);
#ifdef DBG_UTIL
        assert(pSource->Which() == nWhich && "ITEM: Clone of Item did NOT copy/set WhichID (!)");
#endif
        SAL_WARN_IF(typeid(*pPreviousSource) != typeid(*pSource), "svl",
                    "wrong item from Clone(), expected " << typeid(*pPreviousSource).name()
                                                         << " but got " << typeid(*pSource).name());
        assert(typeid(*pPreviousSource) == typeid(*pSource) && "wrong item from Clone()");
    }

    // increase RefCnt 0->1
    pSource->AddRef();

    // check if we should register this Item for the global
    // ItemInstanceManager mechanism (only for shareable Items)
    if (pSource->isShareable())
    {
        ItemInstanceManager* pManager;
        std::pair<SfxItemType, sal_uInt16> aManagerKey{ nSourceItemType, pSource->Which() };
        auto it1 = gInstanceManagerMap.find(aManagerKey);
        if (it1 != gInstanceManagerMap.end())
            pManager = it1->second.get();
        else
        {
            if (pSource->isHashable())
                gInstanceManagerMap.insert(
                    { aManagerKey, std::make_unique<HashableItemInstanceManager>() });
            else
                gInstanceManagerMap.insert(
                    { aManagerKey, std::make_unique<DefaultItemInstanceManager>() });
            pManager = gInstanceManagerMap.find(aManagerKey)->second.get();
        }

        bool bSuccess = pManager->add(*pSource);
        assert(bSuccess && "failed to add item to pool");
        (void)bSuccess;
    }

    return pSource;
}

void implCleanupItemEntry(const SfxPoolItem* pSource)
{
    if (nullptr == pSource)
        // no entry, done
        return;

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

    // check if we should/can remove this Item from the global
    // ItemInstanceManager mechanism
    auto itemsetIt = gInstanceManagerMap.find({ pSource->ItemType(), pSource->Which() });
    if (itemsetIt != gInstanceManagerMap.end())
    {
        auto& rInstanceManager = *(itemsetIt->second);
        rInstanceManager.remove(*pSource);
    }

    // decrease RefCnt before deleting (destructor asserts for it and that's
    // good to find other errors)
    pSource->ReleaseRef();

    // delete Item
    delete pSource;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
