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

#include <string.h>
#include <libxml/xmlwriter.h>

#include <tools/solar.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/hint.hxx>
#include <poolio.hxx>

#include <algorithm>
#include <cassert>
#include <vector>


#if OSL_DEBUG_LEVEL > 0
#include <map>

static void
lcl_CheckSlots2(std::map<sal_uInt16, sal_uInt16> & rSlotMap,
        SfxItemPool const& rPool, SfxItemInfo const* pInfo)
{
    if (!pInfo)
        return; // may not be initialized yet
    if (rPool.GetName() == "EditEngineItemPool")
        return; // HACK: this one has loads of duplicates already, ignore it :(
    sal_uInt16 const nFirst(rPool.GetFirstWhich());
    sal_uInt16 const nCount(rPool.GetLastWhich() - rPool.GetFirstWhich() + 1);
    for (sal_uInt16 n = 0; n < nCount; ++n)
    {
        sal_uInt16 const nSlotId(pInfo[n]._nSID);
        if (nSlotId != 0
            && nSlotId != 10883  // preexisting duplicate SID_ATTR_GRAF_CROP
            && nSlotId != 10023  // preexisting duplicate SID_ATTR_BORDER_INNER
            && nSlotId != 10024  // preexisting duplicate SID_ATTR_BORDER_OUTER
            && nSlotId != 11013  // preexisting duplicate SID_ATTR_BORDER_DIAG_TLBR
            && nSlotId != 11014) // preexisting duplicate SID_ATTR_BORDER_DIAG_BLTR
        {   // check for duplicate slot-id mapping
            std::map<sal_uInt16, sal_uInt16>::const_iterator const iter(
                rSlotMap.find(nSlotId));
            sal_uInt16 const nWhich(nFirst + n);
            if (iter != rSlotMap.end())
            {
                SAL_WARN("svl", "SfxItemPool: duplicate SlotId " << nSlotId
                        << " mapped to " << iter->second << " and " << nWhich);
                assert(false);
            }
            rSlotMap.insert(std::make_pair(nSlotId, nWhich));
        }
    }
}

#define CHECK_SLOTS() \
do { \
    std::map<sal_uInt16, sal_uInt16> slotmap; \
    for (SfxItemPool * p = pImpl->mpMaster; p; p = p->pImpl->mpSecondary) \
    { \
        lcl_CheckSlots2(slotmap, *p, p->pItemInfos); \
    } \
} while (false)

#else
#define CHECK_SLOTS() do {} while (false)
#endif


void SfxItemPool::AddSfxItemPoolUser(SfxItemPoolUser& rNewUser)
{
    // maintain sorted to reduce cost of remove
    const auto insertIt = ::std::lower_bound(
        pImpl->maSfxItemPoolUsers.begin(), pImpl->maSfxItemPoolUsers.end(), &rNewUser);
    pImpl->maSfxItemPoolUsers.insert(insertIt, &rNewUser);
}

void SfxItemPool::RemoveSfxItemPoolUser(SfxItemPoolUser& rOldUser)
{
    const auto aFindResult = ::std::lower_bound(
        pImpl->maSfxItemPoolUsers.begin(), pImpl->maSfxItemPoolUsers.end(), &rOldUser);
    if(aFindResult != pImpl->maSfxItemPoolUsers.end() && *aFindResult == &rOldUser)
    {
        pImpl->maSfxItemPoolUsers.erase(aFindResult);
    }
}

const SfxPoolItem* SfxItemPool::GetPoolDefaultItem( sal_uInt16 nWhich ) const
{
    const SfxPoolItem* pRet;
    if( IsInRange( nWhich ) )
        pRet = pImpl->maPoolDefaults[GetIndex_Impl(nWhich)];
    else if( pImpl->mpSecondary )
        pRet = pImpl->mpSecondary->GetPoolDefaultItem( nWhich );
    else
    {
        assert(false && "unknown WhichId - cannot get pool default");
        pRet = nullptr;
    }
    return pRet;
}


bool SfxItemPool::IsItemPoolable_Impl( sal_uInt16 nPos ) const
{
    return pItemInfos[nPos]._bPoolable;
}


bool SfxItemPool::IsItemPoolable( sal_uInt16 nWhich ) const
{
    for ( const SfxItemPool *pPool = this; pPool; pPool = pPool->pImpl->mpSecondary )
    {
        if ( pPool->IsInRange(nWhich) )
            return pPool->IsItemPoolable_Impl( pPool->GetIndex_Impl(nWhich));
    }
    DBG_ASSERT( !IsWhich(nWhich), "unknown which-id" );
    return false;
}


SfxBroadcaster& SfxItemPool::BC()
{
    return pImpl->aBC;
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
 *  @see SfxItemPool::SetDefaults(std::vector<SfxPoolItem*>*).
 *
 *  @see SfxItemPool::SetDefaults(std::vector<SfxPoolItem*>*)
 *  @see SfxItemPool::ReleaseDefaults(std::vector<SfxPoolItem*>*,sal_uInt16,sal_Bool)
 *  @see SfxItemPool::ReldaseDefaults(sal_Bool)
 */
SfxItemPool::SfxItemPool
(
    const OUString&     rName,          /* Pool name to identify in the file format */
    sal_uInt16          nStartWhich,    /* First WhichId of the Pool (must be > 0) */
    sal_uInt16          nEndWhich,      /* Last WhichId of the Pool */
    const SfxItemInfo*  pInfo,          /* SID Map and Item flags */
    std::vector<SfxPoolItem*>*
                        pDefaults       /* Pointer to static Defaults;
                                           is directly referenced by the Pool,
                                           but no transfer of ownership */
) :
    pItemInfos(pInfo),
    pImpl( new SfxItemPool_Impl( this, rName, nStartWhich, nEndWhich ) )
{
    pImpl->eDefMetric = MapUnit::MapTwip;

    if ( pDefaults )
        SetDefaults(pDefaults);
}


/**
 * Copy ctor
 *
 * @see SfxItemPool::Clone() const
*/
SfxItemPool::SfxItemPool
(
    const SfxItemPool&  rPool,                  //  Copy from this instance
    bool                bCloneStaticDefaults    /*  true
                                                    Copy static Defaults

                                                    false
                                                    Take over static Defaults */
) :
    pItemInfos(rPool.pItemInfos),
    pImpl( new SfxItemPool_Impl( this, rPool.pImpl->aName, rPool.pImpl->mnStart, rPool.pImpl->mnEnd ) )
{
    pImpl->eDefMetric = rPool.pImpl->eDefMetric;

    // Take over static Defaults
    if ( bCloneStaticDefaults )
    {
        std::vector<SfxPoolItem *>* ppDefaults = new std::vector<SfxPoolItem*>(pImpl->mnEnd-pImpl->mnStart+1);
        for ( sal_uInt16 n = 0; n <= pImpl->mnEnd - pImpl->mnStart; ++n )
        {
            (*ppDefaults)[n] = (*rPool.pImpl->mpStaticDefaults)[n]->Clone(this);
            (*ppDefaults)[n]->SetKind(SfxItemKind::StaticDefault);
        }

        SetDefaults( ppDefaults );
    }
    else
        SetDefaults( rPool.pImpl->mpStaticDefaults );

    // Copy Pool Defaults
    for (size_t n = 0; n < pImpl->maPoolDefaults.size(); ++n )
        if (rPool.pImpl->maPoolDefaults[n])
        {
            pImpl->maPoolDefaults[n] = rPool.pImpl->maPoolDefaults[n]->Clone(this); //resets kind
            pImpl->maPoolDefaults[n]->SetKind(SfxItemKind::PoolDefault);
        }

    // Repair linkage
    if ( rPool.pImpl->mpSecondary )
        SetSecondaryPool( rPool.pImpl->mpSecondary->Clone() );
}

void SfxItemPool::SetDefaults( std::vector<SfxPoolItem*>* pDefaults )
{
    DBG_ASSERT( pDefaults, "first we ask for it, and then we don't give back..." );
    DBG_ASSERT( !pImpl->mpStaticDefaults, "already have Defaults" );

    pImpl->mpStaticDefaults = pDefaults;
    //! if ((*mpStaticDefaults)->GetKind() != SfxItemKind::StaticDefault)
    //! FIXME: Probably doesn't work with SetItems at the end
    {
        DBG_ASSERT( (*pImpl->mpStaticDefaults)[0]->GetRefCount() == 0 ||
                    IsDefaultItem( (*pImpl->mpStaticDefaults)[0] ),
                    "these are not static" );
        for ( sal_uInt16 n = 0; n <= pImpl->mnEnd - pImpl->mnStart; ++n )
        {
            assert(  ((*pImpl->mpStaticDefaults)[n]->Which() == n + pImpl->mnStart)
                        && "static defaults not sorted" );
            (*pImpl->mpStaticDefaults)[n]->SetKind(SfxItemKind::StaticDefault);
            DBG_ASSERT( !(pImpl->maPoolItems[n]), "defaults with setitems with items?!" );
        }
    }
}

void SfxItemPool::ClearDefaults()
{
    pImpl->mpStaticDefaults = nullptr;
}

/**
 * Frees the static Defaults of the corresponding SfxItemPool instance
 * and deletes them if specified.
 *
 * The SfxItemPool instance MUST NOT BE USED after this function has
 * been called; only the dtor must be called.
 */
void SfxItemPool::ReleaseDefaults
(
    bool    bDelete     /*  true
                            Deletes the array as well as the single static Defaults

                            false
                            Neither deletes the array not the single static Defaults */
)


{
    DBG_ASSERT( pImpl->mpStaticDefaults, "requirements not met" );
    ReleaseDefaults( pImpl->mpStaticDefaults, bDelete );

    // mpStaticDefaults points to deleted memory if bDelete == true.
    if ( bDelete )
        pImpl->mpStaticDefaults = nullptr;
}


/**
 * Frees the specified static Defaults and also deletes them, if so
 * specified.
 *
 * This method MUST be called AFTER all SfxItemPool instances (which
 * use the specified static Defaults 'pDefault') have been destroyed.
 */
void SfxItemPool::ReleaseDefaults
(
    std::vector<SfxPoolItem*>*
                    pDefaults,  /*  Static Defaults that are to be freed */

    bool            bDelete     /*  true
                                    Deletes the array as well as the specified
                                    static Defaults

                                    false
                                    Neither deletes the array nor the single
                                    static Defaults */
)
{
    DBG_ASSERT( pDefaults, "we first ask for it and the return nothing ..." );

    for ( auto & rpItem : *pDefaults )
    {
        assert(IsStaticDefaultItem(rpItem));
        rpItem->SetRefCount(0);
        if ( bDelete )
        {
            delete rpItem;
            rpItem = nullptr;
        }
    }

    if ( bDelete )
    {
        delete pDefaults;
        pDefaults = nullptr;
    }
}


SfxItemPool::~SfxItemPool()
{
    if ( !pImpl->maPoolItems.empty() && !pImpl->maPoolDefaults.empty() )
        Delete();

    if (pImpl->mpMaster != nullptr && pImpl->mpMaster != this)
    {
        // This condition indicates an error.
        // A pImpl->mpMaster->SetSecondaryPool(...) call should have been made
        // earlier to prevent this. At this point we can only try to
        // prevent a crash later on.
        DBG_ASSERT( pImpl->mpMaster == this, "destroying active Secondary-Pool" );
        if (pImpl->mpMaster->pImpl->mpSecondary == this)
            pImpl->mpMaster->pImpl->mpSecondary = nullptr;
    }
}

void SfxItemPool::Free(SfxItemPool* pPool)
{
    if(!pPool)
        return;

    // tell all the registered SfxItemPoolUsers that the pool is in destruction
    std::vector<SfxItemPoolUser*> aListCopy(pPool->pImpl->maSfxItemPoolUsers);
    for(SfxItemPoolUser* pSfxItemPoolUser : aListCopy)
    {
        DBG_ASSERT(pSfxItemPoolUser, "corrupt SfxItemPoolUser list (!)");
        pSfxItemPoolUser->ObjectInDestruction(*pPool);
    }

    // Clear the vector. This means that user do not need to call RemoveSfxItemPoolUser()
    // when they get called from ObjectInDestruction().
    pPool->pImpl->maSfxItemPoolUsers.clear();

    // delete pool
    delete pPool;
}


void SfxItemPool::SetSecondaryPool( SfxItemPool *pPool )
{
    // Reset Master in attached Pools
    if ( pImpl->mpSecondary )
    {
#ifdef DBG_UTIL
        if (pImpl->mpStaticDefaults != nullptr && !pImpl->maPoolItems.empty()
            && !pImpl->mpSecondary->pImpl->maPoolItems.empty())
            // Delete() did not yet run?
        {
                // Does the Master have SetItems?
            bool bHasSetItems = false;
            for ( sal_uInt16 i = 0; !bHasSetItems && i < pImpl->mnEnd - pImpl->mnStart; ++i )
                bHasSetItems = dynamic_cast<const SfxSetItem *>((*pImpl->mpStaticDefaults)[i]) != nullptr;

            // Detached Pools must be empty
            bool bOK = bHasSetItems;
            for (auto const& rSecArrayPtr : pImpl->mpSecondary->pImpl->maPoolItems)
            {
                if (!bOK)
                    break;
                if (rSecArrayPtr)
                {
                    for (const SfxPoolItem* pItem : *rSecArrayPtr)
                        if (pItem)
                        {
                            SAL_WARN("svl.items", "old secondary pool: " << pImpl->mpSecondary->pImpl->aName
                                            << " of pool: " << pImpl->aName << " must be empty.");
                            bOK = false;
                            break;
                        }
                }
            }
        }
#endif

        pImpl->mpSecondary->pImpl->mpMaster = pImpl->mpSecondary;
        for ( SfxItemPool *p = pImpl->mpSecondary->pImpl->mpSecondary; p; p = p->pImpl->mpSecondary )
            p->pImpl->mpMaster = pImpl->mpSecondary;
    }

    // Set Master of new Secondary Pools
    DBG_ASSERT( !pPool || pPool->pImpl->mpMaster == pPool, "Secondary is present in two Pools" );
    SfxItemPool *pNewMaster = GetMasterPool() ? pImpl->mpMaster : this;
    for ( SfxItemPool *p = pPool; p; p = p->pImpl->mpSecondary )
        p->pImpl->mpMaster = pNewMaster;

    // Remember new Secondary Pool
    pImpl->mpSecondary = pPool;

    CHECK_SLOTS();
}

void SfxItemPool::SetItemInfos(SfxItemInfo const*const pInfo)
{
    pItemInfos = pInfo;
    CHECK_SLOTS();
}


MapUnit SfxItemPool::GetMetric( sal_uInt16 ) const
{
    return pImpl->eDefMetric;
}


void SfxItemPool::SetDefaultMetric( MapUnit eNewMetric )
{
    pImpl->eDefMetric = eNewMetric;
}

const OUString& SfxItemPool::GetName() const
{
    return pImpl->aName;
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


SfxItemPool* SfxItemPool::Clone() const
{
    SfxItemPool *pPool = new SfxItemPool( *this );
    return pPool;
}


void SfxItemPool::Delete()
{
    // Already deleted?
    if (pImpl->maPoolItems.empty() || pImpl->maPoolDefaults.empty())
        return;

    // Inform e.g. running Requests
    pImpl->aBC.Broadcast( SfxHint( SfxHintId::Dying ) );

    // Iterate through twice: first for the SetItems.
    if (pImpl->mpStaticDefaults != nullptr) {
        for (size_t n = 0; n < GetSize_Impl(); ++n)
        {
            // *mpStaticDefaultItem could've already been deleted in a class derived
            // from SfxItemPool
            // This causes chaos in Itempool!
            const SfxPoolItem* pStaticDefaultItem = (*pImpl->mpStaticDefaults)[n];
            if (dynamic_cast<const SfxSetItem*>(pStaticDefaultItem))
            {
                // SfxSetItem found, remove PoolItems (and defaults) with same ID
                auto& rArrayPtr = pImpl->maPoolItems[n];
                if (rArrayPtr)
                {
                    for (auto& rItemPtr : *rArrayPtr)
                        if (rItemPtr)
                        {
                            ReleaseRef(*rItemPtr, rItemPtr->GetRefCount()); // for RefCount check in dtor
                            delete rItemPtr;
                        }
                    rArrayPtr->clear();
                    // let pImpl->DeleteItems() delete item arrays in maPoolItems
                }
                auto& rItemPtr = pImpl->maPoolDefaults[n];
                if (rItemPtr)
                {
#ifdef DBG_UTIL
                    ClearRefCount(*rItemPtr);
#endif
                    delete rItemPtr;
                    rItemPtr = nullptr;
                }
            }
        }
    }

    // now remove remaining PoolItems (and defaults) who didn't have SetItems
    for (auto& rArrayPtr : pImpl->maPoolItems)
    {
        if (rArrayPtr)
        {
            for (auto& rItemPtr : *rArrayPtr)
                if (rItemPtr)
                {
                    ReleaseRef(*rItemPtr, rItemPtr->GetRefCount()); // for RefCount check in dtor
                    delete rItemPtr;
                }
            rArrayPtr->clear();
            // let pImpl->DeleteItems() delete item arrays in maPoolItems
        }
    }
    // default items
    for (auto rItemPtr : pImpl->maPoolDefaults)
    {
        if (rItemPtr)
        {
#ifdef DBG_UTIL
            ClearRefCount(*rItemPtr);
#endif
            delete rItemPtr;
            rItemPtr = nullptr;
        }
    }

    pImpl->DeleteItems();
}


void SfxItemPool::SetPoolDefaultItem(const SfxPoolItem &rItem)
{
    if ( IsInRange(rItem.Which()) )
    {
        auto& rOldDefault =
            pImpl->maPoolDefaults[GetIndex_Impl(rItem.Which())];
        SfxPoolItem *pNewDefault = rItem.Clone(this);
        pNewDefault->SetKind(SfxItemKind::PoolDefault);
        if (rOldDefault)
        {
            rOldDefault->SetRefCount(0);
            DELETEZ(rOldDefault);
        }
        rOldDefault = pNewDefault;
    }
    else if ( pImpl->mpSecondary )
        pImpl->mpSecondary->SetPoolDefaultItem(rItem);
    else
    {
        assert(false && "unknown WhichId - cannot set pool default");
    }
}

/**
 * Resets the default of the given WhichId back to the static Default.
 * If a pool default exists, it is removed.
 */
void SfxItemPool::ResetPoolDefaultItem( sal_uInt16 nWhichId )
{
    if ( IsInRange(nWhichId) )
    {
        auto& rOldDefault =
            pImpl->maPoolDefaults[GetIndex_Impl(nWhichId)];
        if (rOldDefault)
        {
            rOldDefault->SetRefCount(0);
            DELETEZ(rOldDefault);
        }
    }
    else if ( pImpl->mpSecondary )
        pImpl->mpSecondary->ResetPoolDefaultItem(nWhichId);
    else
    {
        assert(false && "unknown WhichId - cannot reset pool default");
    }
}


const SfxPoolItem& SfxItemPool::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    if ( 0 == nWhich )
        nWhich = rItem.Which();

    // Find correct Secondary Pool
    bool bSID = IsSlot(nWhich);
    if ( !bSID && !IsInRange(nWhich) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->Put( rItem, nWhich );
        OSL_FAIL( "unknown WhichId - cannot put item" );
    }

    // SID ?
    if (bSID)
    {
        assert((rItem.Which() != nWhich ||
            !IsDefaultItem(&rItem) || rItem.GetKind() == SfxItemKind::DeleteOnIdle)
                && "a non Pool Item is Default?!");
        SfxPoolItem *pPoolItem = rItem.Clone(pImpl->mpMaster);
        pPoolItem->SetWhich(nWhich);
        AddRef( *pPoolItem );
        return *pPoolItem;
    }

    assert(!pImpl->mpStaticDefaults ||
            typeid(rItem) == typeid(GetDefaultItem(nWhich)));

    const sal_uInt16 nIndex = GetIndex_Impl(nWhich);
    SfxPoolItemArray_Impl* pItemArr = pImpl->maPoolItems[nIndex].get();
    if (!pItemArr)
    {
        pImpl->maPoolItems[nIndex].reset(new SfxPoolItemArray_Impl);
        pItemArr = pImpl->maPoolItems[nIndex].get();
    }

    std::vector<SfxPoolItem*>::iterator ppFree;
    bool ppFreeIsSet = false;

    // Is this a 'poolable' item - ie. should we re-use and return
    // the same underlying item for equivalent (==) SfxPoolItems?
    if ( IsItemPoolable_Impl( nIndex ) )
    {
        // if is already in a pool, then it is worth checking if it is in this one.
        if ( IsPooledItem(&rItem) )
        {
            SfxPoolItemArray_Impl::PoolItemPtrToIndexMap::const_iterator it;
            it = pItemArr->maPtrToIndex.find(const_cast<SfxPoolItem *>(&rItem));

            // 1. search for an identical pointer in the pool
            if (it != pItemArr->maPtrToIndex.cend())
            {
                AddRef(rItem);
                return rItem;
            }
        }

        // 2. search for an item with matching attributes.
        for (auto itr = pItemArr->begin(); itr != pItemArr->end(); ++itr)
        {
            if (*itr)
            {
                if (**itr == rItem)
                {
                    AddRef(**itr);
                    return **itr;
                }
            }
            else
            {
                if (!ppFreeIsSet)
                {
                    ppFree = itr;
                    ppFreeIsSet = true;
                }
            }
        }
    }
    else
    {
        // Unconditionally insert; check for a recently freed place
        if (!pItemArr->maFree.empty())
        {
            auto itr = pItemArr->begin();
            sal_uInt32 nIdx = pItemArr->maFree.back();
            pItemArr->maFree.pop_back();

            assert(nIdx < pItemArr->size());
            std::advance(itr, nIdx);
            ppFreeIsSet = true;
            ppFree = itr;
        }
    }

    // 3. not found, so clone to insert into the pointer array.
    SfxPoolItem* pNewItem = rItem.Clone(pImpl->mpMaster);
    pNewItem->SetWhich(nWhich);
    assert(typeid(rItem) == typeid(*pNewItem) && "SfxItemPool::Put(): unequal types, no Clone() override?");
    if (dynamic_cast<const SfxSetItem*>(&rItem) == nullptr)
    {
        assert((!IsItemPoolable(nWhich) || rItem == *pNewItem)
            && "SfxItemPool::Put(): unequal items: no operator== override?");
        assert((!IsItemPoolable(*pNewItem) || *pNewItem == rItem)
            && "SfxItemPool::Put(): unequal items: no operator== override?");
    }
    AddRef( *pNewItem );

    // 4. finally insert into the pointer array
    assert( pItemArr->maPtrToIndex.find(pNewItem) == pItemArr->maPtrToIndex.end() );
    if ( !ppFreeIsSet )
    {
        sal_uInt32 nOffset = pItemArr->size();
        pItemArr->maPtrToIndex.insert(std::make_pair(pNewItem, nOffset));
        pItemArr->push_back( pNewItem );
    }
    else
    {
        sal_uInt32 nOffset = std::distance(pItemArr->begin(), ppFree);
        pItemArr->maPtrToIndex.insert(std::make_pair(pNewItem, nOffset));
        assert(*ppFree == nullptr);
        *ppFree = pNewItem;
    }
    return *pNewItem;
}

void SfxItemPool::Remove( const SfxPoolItem& rItem )
{
    assert(!IsPoolDefaultItem(&rItem) && "cannot remove Pool Default");

    // Find correct Secondary Pool
    const sal_uInt16 nWhich = rItem.Which();
    bool bSID = IsSlot(nWhich);
    if ( !bSID && !IsInRange(nWhich) )
    {
        if ( pImpl->mpSecondary )
        {
            pImpl->mpSecondary->Remove( rItem );
            return;
        }
        OSL_FAIL( "unknown WhichId - cannot remove item" );
    }

    // SID ?
    if ( bSID )
    {
        assert(!IsDefaultItem(&rItem) && "a non Pool Item is Default?!");
        if ( 0 == ReleaseRef(rItem) )
        {
            delete &rItem;
        }
        return;
    }

    assert(rItem.GetRefCount() && "RefCount == 0, Remove impossible");

    const sal_uInt16 nIndex = GetIndex_Impl(nWhich);
    // Static Defaults are just there
    if ( IsStaticDefaultItem(&rItem) &&
         &rItem == (*pImpl->mpStaticDefaults)[nIndex])
        return;

    // Find Item in own Pool
    SfxPoolItemArray_Impl* pItemArr = pImpl->maPoolItems[nIndex].get();
    assert(pItemArr && "removing Item not in Pool");

    SfxPoolItemArray_Impl::PoolItemPtrToIndexMap::const_iterator it
        = pItemArr->maPtrToIndex.find(const_cast<SfxPoolItem *>(&rItem));
    if (it != pItemArr->maPtrToIndex.end())
    {
        sal_uInt32 nIdx = it->second;
        assert(nIdx < pItemArr->size());
        SfxPoolItem*& p = (*pItemArr)[nIdx];
        assert(p == &rItem);

        if ( p->GetRefCount() ) //!
            ReleaseRef( *p );
        else
        {
            assert(false && "removing Item without ref");
        }

        // FIXME: Hack, for as long as we have problems with the Outliner
        // See other MI-REF
        if ( 0 == p->GetRefCount() && nWhich < 4000 )
        {
            DELETEZ(p);

            // remove ourselves from the hash
            pItemArr->maPtrToIndex.erase(it);

            // record that this slot is free
            pItemArr->maFree.push_back( nIdx );
        }

        return;
    }

    // not found
    assert(false && "removing Item not in Pool");
}


const SfxPoolItem& SfxItemPool::GetDefaultItem( sal_uInt16 nWhich ) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetDefaultItem( nWhich );
        assert(!"unknown which - don't ask me for defaults");
    }

    DBG_ASSERT( pImpl->mpStaticDefaults, "no defaults known - don't ask me for defaults" );
    sal_uInt16 nPos = GetIndex_Impl(nWhich);
    SfxPoolItem* pDefault = pImpl->maPoolDefaults[nPos];
    if ( pDefault )
        return *pDefault;
    return *(*pImpl->mpStaticDefaults)[nPos];
}

SfxItemPool* SfxItemPool::GetSecondaryPool() const
{
    return pImpl->mpSecondary;
}

SfxItemPool* SfxItemPool::GetMasterPool() const
{
    return pImpl->mpMaster;
}

/**
 * This method should be called at the master pool, when all secondary
 * pools are appended to it.
 *
 * It calculates the ranges of 'which-ids' for fast construction of
 * item-sets, which contains all 'which-ids'.
 */
void SfxItemPool::FreezeIdRanges()
{
    FillItemIdRanges_Impl( pImpl->mpPoolRanges );
}


void SfxItemPool::FillItemIdRanges_Impl( std::unique_ptr<sal_uInt16[]>& pWhichRanges ) const
{
    DBG_ASSERT( !pImpl->mpPoolRanges, "GetFrozenRanges() would be faster!" );

    const SfxItemPool *pPool;
    sal_uInt16 nLevel = 0;
    for( pPool = this; pPool; pPool = pPool->pImpl->mpSecondary )
        ++nLevel;

    pWhichRanges.reset(new sal_uInt16[ 2*nLevel + 1 ]);

    nLevel = 0;
    for( pPool = this; pPool; pPool = pPool->pImpl->mpSecondary )
    {
        pWhichRanges[nLevel++] = pPool->pImpl->mnStart;
        pWhichRanges[nLevel++] = pPool->pImpl->mnEnd;
        pWhichRanges[nLevel] = 0;
    }
}

const sal_uInt16* SfxItemPool::GetFrozenIdRanges() const
{
    return pImpl->mpPoolRanges.get();
}

const SfxPoolItem *SfxItemPool::GetItem2Default(sal_uInt16 nWhich) const
{
    return GetItem2(nWhich, SFX_ITEMS_DEFAULT);
}

const SfxPoolItem *SfxItemPool::GetItem2(sal_uInt16 nWhich, sal_uInt32 nOfst) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetItem2( nWhich, nOfst );
        assert(false && "unknown WhichId - cannot resolve surrogate");
        return nullptr;
    }

    // default attribute?
    if ( nOfst == SFX_ITEMS_DEFAULT )
        return (*pImpl->mpStaticDefaults)[ GetIndex_Impl(nWhich) ];

    SfxPoolItemArray_Impl* pItemArr = pImpl->maPoolItems[GetIndex_Impl(nWhich)].get();
    if( pItemArr && nOfst < pItemArr->size() )
        return (*pItemArr)[nOfst];

    return nullptr;
}

sal_uInt32 SfxItemPool::GetItemCount2(sal_uInt16 nWhich) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetItemCount2( nWhich );
        assert(false && "unknown WhichId - cannot resolve surrogate");
        return 0;
    }

    SfxPoolItemArray_Impl* pItemArr = pImpl->maPoolItems[GetIndex_Impl(nWhich)].get();
    if  ( pItemArr )
        return pItemArr->size();
    return 0;
}


sal_uInt16 SfxItemPool::GetWhich( sal_uInt16 nSlotId, bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return nSlotId;

    sal_uInt16 nCount = pImpl->mnEnd - pImpl->mnStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nSID == nSlotId )
            return nOfs + pImpl->mnStart;
    if ( pImpl->mpSecondary && bDeep )
        return pImpl->mpSecondary->GetWhich(nSlotId);
    return nSlotId;
}


sal_uInt16 SfxItemPool::GetSlotId( sal_uInt16 nWhich ) const
{
    if ( !IsWhich(nWhich) )
        return nWhich;

    if ( !IsInRange( nWhich ) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetSlotId(nWhich);
        assert(false && "unknown WhichId - cannot get slot-id");
        return 0;
    }

    sal_uInt16 nSID = pItemInfos[nWhich - pImpl->mnStart]._nSID;
    return nSID ? nSID : nWhich;
}


sal_uInt16 SfxItemPool::GetTrueWhich( sal_uInt16 nSlotId, bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return 0;

    sal_uInt16 nCount = pImpl->mnEnd - pImpl->mnStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nSID == nSlotId )
            return nOfs + pImpl->mnStart;
    if ( pImpl->mpSecondary && bDeep )
        return pImpl->mpSecondary->GetTrueWhich(nSlotId);
    return 0;
}


sal_uInt16 SfxItemPool::GetTrueSlotId( sal_uInt16 nWhich ) const
{
    if ( !IsWhich(nWhich) )
        return 0;

    if ( !IsInRange( nWhich ) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetTrueSlotId(nWhich);
        assert(false && "unknown WhichId - cannot get slot-id");
        return 0;
    }
    return pItemInfos[nWhich - pImpl->mnStart]._nSID;
}

void SfxItemPool::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SfxItemPool"));
    for (auto const & rArrayPtr : pImpl->maPoolItems)
        if (rArrayPtr)
            for (auto const & rItem : *rArrayPtr)
                if (rItem)
                    rItem->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
