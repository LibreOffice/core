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
#include <svl/setitem.hxx>
#include <sal/log.hxx>

static bool g_bDisableItemInstanceManager(getenv("SVL_DISABLE_ITEM_INSTANCE_MANAGER"));
static bool g_bShareImmediately(getenv("SVL_SHARE_ITEMS_GLOBALLY_INSTANTLY"));
#define NUMBER_OF_UNSHARED_INSTANCES (50)

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

void DefaultItemInstanceManager::add(const SfxPoolItem& rItem)
{
    maRegistered[rItem.Which()].insert(&rItem);
}

void DefaultItemInstanceManager::remove(const SfxPoolItem& rItem)
{
    maRegistered[rItem.Which()].erase(&rItem);
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
    typedef std::unordered_map<SfxItemType, std::pair<sal_uInt16, ItemInstanceManager*>>
        managerTypeMap;
    managerTypeMap maManagerPerType;

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

        // Check for correct SfxItemType, there may be derivations of that class.
        // Note that Managers from the Items are *not* added to local list,
        // they are expected to be static instances at the Items for fastest access
        if (nullptr != pManager && pManager->ItemType() == rItem.ItemType())
            return pManager;

        // check local memory for existing entry
        managerTypeMap::iterator aHit(maManagerPerType.find(rItem.ItemType()));

        // no instance yet
        if (aHit == maManagerPerType.end())
        {
            // create a default one to start usage-counting
            if (g_bShareImmediately)
            {
                // create, insert locally and immediately start sharing
                ItemInstanceManager* pNew;
                if (rItem.supportsHashCode())
                    pNew = new HashedItemInstanceManager(rItem.ItemType());
                else
                    pNew = new DefaultItemInstanceManager(rItem.ItemType());
                maManagerPerType.insert({ rItem.ItemType(), std::make_pair(0, pNew) });
                return pNew;
            }

            // start countdown from NUMBER_OF_UNSHARED_INSTANCES until zero is reached
            maManagerPerType.insert(
                { rItem.ItemType(), std::make_pair(NUMBER_OF_UNSHARED_INSTANCES, nullptr) });
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
        ItemInstanceManager* pNew;
        if (rItem.supportsHashCode())
            pNew = new HashedItemInstanceManager(rItem.ItemType());
        else
            pNew = new DefaultItemInstanceManager(rItem.ItemType());
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

        // Check for correct SfxItemType, there may be derivations of that class.
        // Note that Managers from the Items are *not* added to local list,
        // they are expected to be static instances at the Items
        if (nullptr != pManager && pManager->ItemType() == rItem.ItemType())
            return pManager;

        // check local memory for existing entry
        managerTypeMap::iterator aHit(maManagerPerType.find(rItem.ItemType()));

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

SfxPoolItem const* implCreateItemEntry(const SfxItemPool& rPool, SfxPoolItem const* pSource,
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
    if (pSource->GetRefCount() > 0)
    {
        if (pSource->isShareable())
        {
            // SfxSetItems cannot be shared if they are in/use another pool
            if (!pSource->isSetItem()
                || static_cast<const SfxSetItem*>(pSource)->GetItemSet().GetPool() == pMasterPool)
            {
                // If we get here we can share the Item
                pSource->AddRef();
                return pSource;
            }
        }
    }

    // try to get an ItemInstanceManager for global Item instance sharing
    ItemInstanceManager* pManager(aInstanceManagerHelper.getOrCreateItemInstanceManager(*pSource));

    // check if we can globally share the Item using an ItemInstanceManager
    if (pManager)
    {
        const SfxPoolItem* pAlternative(pManager->find(*pSource));
        if (pAlternative)
        {
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
    if (nullptr != pManager)
        pManager->add(*pSource);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
