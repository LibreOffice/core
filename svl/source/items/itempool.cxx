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

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include "whassert.hxx"
#include <svl/SfxBroadcaster.hxx>
#include <svl/smplhint.hxx>
#include "poolio.hxx"

#include <algorithm>
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
    for (SfxItemPool * p = pImp->mpMaster; p; p = p->pImp->mpSecondary) \
    { \
        lcl_CheckSlots2(slotmap, *p, p->pItemInfos); \
    } \
} while (false)

#else
#define CHECK_SLOTS() do {} while (false)
#endif


void SfxItemPool::AddSfxItemPoolUser(SfxItemPoolUser& rNewUser)
{
    pImp->maSfxItemPoolUsers.push_back(&rNewUser);
}

void SfxItemPool::RemoveSfxItemPoolUser(SfxItemPoolUser& rOldUser)
{
    const std::vector<SfxItemPoolUser*>::iterator aFindResult = ::std::find(
        pImp->maSfxItemPoolUsers.begin(), pImp->maSfxItemPoolUsers.end(), &rOldUser);
    if(aFindResult != pImp->maSfxItemPoolUsers.end())
    {
        pImp->maSfxItemPoolUsers.erase(aFindResult);
    }
}

const SfxPoolItem* SfxItemPool::GetPoolDefaultItem( sal_uInt16 nWhich ) const
{
    const SfxPoolItem* pRet;
    if( IsInRange( nWhich ) )
        pRet = *(pImp->ppPoolDefaults + GetIndex_Impl( nWhich ));
    else if( pImp->mpSecondary )
        pRet = pImp->mpSecondary->GetPoolDefaultItem( nWhich );
    else
    {
        assert(false && "unknown WhichId - cannot get pool default");
        pRet = nullptr;
    }
    return pRet;
}


bool SfxItemPool::IsItemFlag_Impl( sal_uInt16 nPos, SfxItemPoolFlags nFlag ) const
{
    return bool(pItemInfos[nPos]._nFlags & nFlag);
}


bool SfxItemPool::IsItemFlag( sal_uInt16 nWhich, SfxItemPoolFlags nFlag ) const
{
    for ( const SfxItemPool *pPool = this; pPool; pPool = pPool->pImp->mpSecondary )
    {
        if ( pPool->IsInRange(nWhich) )
            return pPool->IsItemFlag_Impl( pPool->GetIndex_Impl(nWhich), nFlag);
    }
    DBG_ASSERT( !IsWhich(nWhich), "unknown which-id" );
    return false;
}


SfxBroadcaster& SfxItemPool::BC()
{
    return pImp->aBC;
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
 *  The flags allow for e.g. enabling value sharing (SfxItemPoolFlags::POOLABLE).
 *
 *  If the Pool is supposed to hold SfxSetItems, the ctor cannot yet contain
 *  static Defaults. This needs to be done afterwards, using
 *  @see SfxItemPool::SetDefaults(SfxItemPool**).
 *
 *  @see SfxItemPool::SetDefaults(SfxItemPool**)
 *  @see SfxItemPool::ReleaseDefaults(SfxPoolItem**,sal_uInt16,sal_Bool)
 *  @see SfxItemPool::ReldaseDefaults(sal_Bool)
 */
SfxItemPool::SfxItemPool
(
    const OUString&     rName,          /* Pool name to identify in the file format */
    sal_uInt16          nStartWhich,    /* First WhichId of the Pool */
    sal_uInt16          nEndWhich,      /* Last WhichId of the Pool */
    const SfxItemInfo*  pInfo,         /* SID Map and Item flags */
    SfxPoolItem**       pDefaults,      /* Pointer to static Defaults;
                                           is directly referenced by the Pool,
                                           but no transfer of ownership */
    bool                bLoadRefCounts  /* Load RefCounts or set to 1? */
) :
    pItemInfos(pInfo),
    pImp( new SfxItemPool_Impl( this, rName, nStartWhich, nEndWhich ) )
{
    pImp->eDefMetric = SFX_MAPUNIT_TWIP;
    pImp->nVersion = 0;
    pImp->bStreaming = false;
    pImp->nLoadingVersion = 0;
    pImp->nInitRefCount = 1;
    pImp->nVerStart = pImp->mnStart;
    pImp->nVerEnd = pImp->mnEnd;
    pImp->bInSetItem = false;
    pImp->nStoringStart = nStartWhich;
    pImp->nStoringEnd = nEndWhich;
    pImp->mbPersistentRefCounts = bLoadRefCounts;

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
    pImp( new SfxItemPool_Impl( this, rPool.pImp->aName, rPool.pImp->mnStart, rPool.pImp->mnEnd ) )
{
    pImp->eDefMetric = rPool.pImp->eDefMetric;
    pImp->nVersion = rPool.pImp->nVersion;
    pImp->bStreaming = false;
    pImp->nLoadingVersion = 0;
    pImp->nInitRefCount = 1;
    pImp->nVerStart = rPool.pImp->nVerStart;
    pImp->nVerEnd = rPool.pImp->nVerEnd;
    pImp->bInSetItem = false;
    pImp->nStoringStart = pImp->mnStart;
    pImp->nStoringEnd = pImp->mnEnd;
    pImp->mbPersistentRefCounts = rPool.pImp->mbPersistentRefCounts;

    // Take over static Defaults
    if ( bCloneStaticDefaults )
    {
        SfxPoolItem **ppDefaults = new SfxPoolItem*[pImp->mnEnd-pImp->mnStart+1];
        for ( sal_uInt16 n = 0; n <= pImp->mnEnd - pImp->mnStart; ++n )
        {
            (*( ppDefaults + n )) = (*( rPool.pImp->ppStaticDefaults + n ))->Clone(this);
            (*( ppDefaults + n ))->SetKind( SFX_ITEMS_STATICDEFAULT );
        }

        SetDefaults( ppDefaults );
    }
    else
        SetDefaults( rPool.pImp->ppStaticDefaults );

    // Copy Pool Defaults
    for ( sal_uInt16 n = 0; n <= pImp->mnEnd - pImp->mnStart; ++n )
        if ( (*( rPool.pImp->ppPoolDefaults + n )) )
        {
            (*( pImp->ppPoolDefaults + n )) = (*( rPool.pImp->ppPoolDefaults + n ))->Clone(this);
            (*( pImp->ppPoolDefaults + n ))->SetKind( SFX_ITEMS_POOLDEFAULT );
        }

    // Copy Version map
    for ( size_t nVer = 0; nVer < rPool.pImp->aVersions.size(); ++nVer )
    {
        const SfxPoolVersion_ImplPtr pOld = rPool.pImp->aVersions[nVer];
        SfxPoolVersion_ImplPtr pNew = SfxPoolVersion_ImplPtr( new SfxPoolVersion_Impl( *pOld ) );
        pImp->aVersions.push_back( pNew );
    }

    // Repair linkage
    if ( rPool.pImp->mpSecondary )
        SetSecondaryPool( rPool.pImp->mpSecondary->Clone() );
}


void SfxItemPool::SetDefaults( SfxPoolItem **pDefaults )
{
    DBG_ASSERT( pDefaults, "first we ask for it, and then we don't give back..." );
    DBG_ASSERT( !pImp->ppStaticDefaults, "already have Defaults" );

    pImp->ppStaticDefaults = pDefaults;
    //! if ( (*ppStaticDefaults)->GetKind() != SFX_ITEMS_STATICDEFAULT )
    //! FIXME: Probably doesn't work with SetItems at the end
    {
        DBG_ASSERT( (*pImp->ppStaticDefaults)->GetRefCount() == 0 ||
                    IsDefaultItem( (*pImp->ppStaticDefaults) ),
                    "these are not static" );
        for ( sal_uInt16 n = 0; n <= pImp->mnEnd - pImp->mnStart; ++n )
        {
            assert(((*(pImp->ppStaticDefaults + n))->Which() == n + pImp->mnStart)
                        && "static defaults not sorted" );
            (*( pImp->ppStaticDefaults + n ))->SetKind( SFX_ITEMS_STATICDEFAULT );
            DBG_ASSERT( !(pImp->maPoolItems[n]), "defaults with setitems with items?!" );
        }
    }
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
    DBG_ASSERT( pImp->ppStaticDefaults, "requirements not met" );
    ReleaseDefaults( pImp->ppStaticDefaults, pImp->mnEnd - pImp->mnStart + 1, bDelete );

    // ppStaticDefaults points to deleted memory if bDelete == true.
    if ( bDelete )
        pImp->ppStaticDefaults = nullptr;
}


/**
 * Frees the specified static Defaults und also deletes them, if so
 * specified.
 *
 * This method MUST be called AFTER all SfxItemPool instances (which
 * use the specified static Defaults 'pDefault') have been destroyed.
 */
void SfxItemPool::ReleaseDefaults
(
    SfxPoolItem**   pDefaults,  /*  Static Defaults that are to be freed */

    sal_uInt16      nCount,     /*  Count of static Defaults */

    bool            bDelete     /*  true
                                    Deletes the array as well as the specified
                                    static Defaults

                                    false
                                    Neither deletes the array nor the single
                                    static Defaults */
)
{
    DBG_ASSERT( pDefaults, "we first ask for it and the return nothing ..." );

    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        assert(IsStaticDefaultItem(*(pDefaults+n)));
        (*( pDefaults + n ))->SetRefCount( 0 );
        if ( bDelete )
            { delete *( pDefaults + n ); *(pDefaults + n) = nullptr; }
    }

    if ( bDelete )
        { delete[] pDefaults; pDefaults = nullptr; }
}


SfxItemPool::~SfxItemPool()
{
    if ( !pImp->maPoolItems.empty() && pImp->ppPoolDefaults )
        Delete();

    if (pImp->mpMaster != nullptr && pImp->mpMaster != this)
    {
        // This condition indicates an error.
        // A pImp->mpMaster->SetSecondaryPool(...) call should have been made
        // earlier to prevent this. At this point we can only try to
        // prevent a crash later on.
        DBG_ASSERT( pImp->mpMaster == this, "destroying active Secondary-Pool" );
        if (pImp->mpMaster->pImp->mpSecondary == this)
            pImp->mpMaster->pImp->mpSecondary = nullptr;
    }

    delete pImp;
}

void SfxItemPool::Free(SfxItemPool* pPool)
{
    if(pPool)
    {
        // tell all the registered SfxItemPoolUsers that the pool is in destruction
        std::vector<SfxItemPoolUser*> aListCopy(pPool->pImp->maSfxItemPoolUsers.begin(), pPool->pImp->maSfxItemPoolUsers.end());
        for(std::vector<SfxItemPoolUser*>::iterator aIterator = aListCopy.begin(); aIterator != aListCopy.end(); ++aIterator)
        {
            SfxItemPoolUser* pSfxItemPoolUser = *aIterator;
            DBG_ASSERT(pSfxItemPoolUser, "corrupt SfxItemPoolUser list (!)");
            pSfxItemPoolUser->ObjectInDestruction(*pPool);
        }

        // Clear the vector. This means that user do not need to call RemoveSfxItemPoolUser()
        // when they get called from ObjectInDestruction().
        pPool->pImp->maSfxItemPoolUsers.clear();

        // delete pool
        delete pPool;
    }
}


void SfxItemPool::SetSecondaryPool( SfxItemPool *pPool )
{
    // Reset Master in attached Pools
    if ( pImp->mpSecondary )
    {
#ifdef DBG_UTIL
        SAL_INFO( "svl.items", "for Image: there are not statics; this is a bug" );
        if ( pImp->ppStaticDefaults )
        {
            // Delete() did not yet run?
            if ( !pImp->maPoolItems.empty() && !pImp->mpSecondary->pImp->maPoolItems.empty() )
            {
                // Does the Master have SetItems?
                bool bHasSetItems = false;
                for ( sal_uInt16 i = 0; !bHasSetItems && i < pImp->mnEnd - pImp->mnStart; ++i )
                    bHasSetItems = dynamic_cast<const SfxSetItem *>(pImp->ppStaticDefaults[i]) != nullptr;

                // Detached Pools must be empty
                bool bOK = bHasSetItems;
                for ( sal_uInt16 n = 0;
                      bOK && n <= pImp->mpSecondary->pImp->mnEnd - pImp->mpSecondary->pImp->mnStart;
                      ++n )
                {
                    SfxPoolItemArray_Impl* pItemArr = pImp->mpSecondary->pImp->maPoolItems[n];
                    if ( pItemArr )
                    {
                        SfxPoolItemArrayBase_Impl::iterator ppHtArr =   pItemArr->begin();
                        for( size_t i = pItemArr->size(); i; ++ppHtArr, --i )
                            if ( !(*ppHtArr) )
                            {
                                OSL_FAIL( "old secondary pool must be empty" );
                                bOK = false;
                                break;
                            }
                    }
                }
            }
        }
#endif

        pImp->mpSecondary->pImp->mpMaster = pImp->mpSecondary;
        for ( SfxItemPool *p = pImp->mpSecondary->pImp->mpSecondary; p; p = p->pImp->mpSecondary )
            p->pImp->mpMaster = pImp->mpSecondary;
    }

    // Set Master of new Secondary Pools
    DBG_ASSERT( !pPool || pPool->pImp->mpMaster == pPool, "Secondary is present in two Pools" );
    SfxItemPool *pNewMaster = GetMasterPool() ? pImp->mpMaster : this;
    for ( SfxItemPool *p = pPool; p; p = p->pImp->mpSecondary )
        p->pImp->mpMaster = pNewMaster;

    // Remember new Secondary Pool
    pImp->mpSecondary = pPool;

    CHECK_SLOTS();
}

void SfxItemPool::SetItemInfos(SfxItemInfo const*const pInfo)
{
    pItemInfos = pInfo;
    CHECK_SLOTS();
}


SfxMapUnit SfxItemPool::GetMetric( sal_uInt16 ) const
{
    return pImp->eDefMetric;
}


void SfxItemPool::SetDefaultMetric( SfxMapUnit eNewMetric )
{
    pImp->eDefMetric = eNewMetric;
}

const OUString& SfxItemPool::GetName() const
{
    return pImp->aName;
}


bool SfxItemPool::GetPresentation
(
    const SfxPoolItem&  rItem,
    SfxMapUnit          eMetric,
    OUString&           rText,
    const IntlWrapper * pIntlWrapper
)   const
{
    return rItem.GetPresentation(
        SFX_ITEM_PRESENTATION_COMPLETE, GetMetric(rItem.Which()), eMetric, rText, pIntlWrapper );
}


SfxItemPool* SfxItemPool::Clone() const
{
    SfxItemPool *pPool = new SfxItemPool( *this );
    return pPool;
}


void SfxItemPool::Delete()
{
    // Already deleted?
    if ( pImp->maPoolItems.empty() || !pImp->ppPoolDefaults )
        return;

    // Inform e.g. running Requests
    pImp->aBC.Broadcast( SfxSimpleHint( SFX_HINT_DYING ) );

    // Iterate through twice: first for the SetItems.
    // We separate this into two loops (for clarity's sake)
    std::vector<SfxPoolItemArray_Impl*>::iterator itrItemArr = pImp->maPoolItems.begin();
    SfxPoolItem** ppDefaultItem = pImp->ppPoolDefaults;
    SfxPoolItem** ppStaticDefaultItem = pImp->ppStaticDefaults;
    sal_uInt16 nArrCnt;

    // Collect the SetItems first
    SAL_INFO( "svl.items", "for Image: there are not statics there yet; this is a bug" );
    if ( pImp->ppStaticDefaults )
    {
        for ( nArrCnt = GetSize_Impl();
                nArrCnt;
                --nArrCnt, ++itrItemArr, ++ppDefaultItem, ++ppStaticDefaultItem )
        {
            // *ppStaticDefaultItem could've already been deleted in a class derived
            // from SfxItemPool
            // This causes chaos in Itempool!
            if ( *ppStaticDefaultItem && dynamic_cast< const SfxSetItem* >(*ppStaticDefaultItem) !=  nullptr )
            {
                if ( *itrItemArr )
                {
                    SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*itrItemArr)->begin();
                    for ( size_t n = (*itrItemArr)->size(); n; --n, ++ppHtArr )
                        if (*ppHtArr)
                        {
#ifdef DBG_UTIL
                            ReleaseRef( **ppHtArr, (*ppHtArr)->GetRefCount() );
#endif
                            delete *ppHtArr;
                        }
                    DELETEZ( *itrItemArr );
                }
                if ( *ppDefaultItem )
                {
#ifdef DBG_UTIL
                    SetRefCount( **ppDefaultItem, 0 );
#endif
                    DELETEZ( *ppDefaultItem );
                }
            }
        }
    }

    itrItemArr = pImp->maPoolItems.begin();
    ppDefaultItem = pImp->ppPoolDefaults;

    // Now for the easy Items
    for ( nArrCnt = GetSize_Impl();
            nArrCnt;
            --nArrCnt, ++itrItemArr, ++ppDefaultItem )
    {
        if ( *itrItemArr )
        {
            SfxPoolItemArrayBase_Impl::iterator ppHtArr = (*itrItemArr)->begin();
            for ( size_t n = (*itrItemArr)->size(); n; --n, ++ppHtArr )
                if (*ppHtArr)
                {
#ifdef DBG_UTIL
                    ReleaseRef( **ppHtArr, (*ppHtArr)->GetRefCount() );
#endif
                    delete *ppHtArr;
                }
            DELETEZ( *itrItemArr );
        }
        if ( *ppDefaultItem )
        {
#ifdef DBG_UTIL
            SetRefCount( **ppDefaultItem, 0 );
#endif
            delete *ppDefaultItem;
        }
    }

    pImp->DeleteItems();
}


void SfxItemPool::SetPoolDefaultItem(const SfxPoolItem &rItem)
{
    if ( IsInRange(rItem.Which()) )
    {
        SfxPoolItem **ppOldDefault =
            pImp->ppPoolDefaults + GetIndex_Impl(rItem.Which());
        SfxPoolItem *pNewDefault = rItem.Clone(this);
        pNewDefault->SetKind(SFX_ITEMS_POOLDEFAULT);
        if ( *ppOldDefault )
        {
            (*ppOldDefault)->SetRefCount(0);
            DELETEZ( *ppOldDefault );
        }
        *ppOldDefault = pNewDefault;
    }
    else if ( pImp->mpSecondary )
        pImp->mpSecondary->SetPoolDefaultItem(rItem);
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
        SfxPoolItem **ppOldDefault =
            pImp->ppPoolDefaults + GetIndex_Impl( nWhichId );
        if ( *ppOldDefault )
        {
            (*ppOldDefault)->SetRefCount(0);
            DELETEZ( *ppOldDefault );
        }
    }
    else if ( pImp->mpSecondary )
        pImp->mpSecondary->ResetPoolDefaultItem(nWhichId);
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
    bool bSID = nWhich > SFX_WHICH_MAX;
    if ( !bSID && !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->Put( rItem, nWhich );
        OSL_FAIL( "unknown WhichId - cannot put item" );
    }

    // SID or not poolable (new definition)?
    sal_uInt16 nIndex = bSID ? USHRT_MAX : GetIndex_Impl(nWhich);
    if ( USHRT_MAX == nIndex ||
         IsItemFlag_Impl( nIndex, SfxItemPoolFlags::NOT_POOLABLE ) )
    {
        assert((USHRT_MAX != nIndex || rItem.Which() != nWhich ||
            !IsDefaultItem(&rItem) || rItem.GetKind() == SFX_ITEMS_DELETEONIDLE)
                && "a non Pool Item is Default?!");
        SfxPoolItem *pPoolItem = rItem.Clone(pImp->mpMaster);
        pPoolItem->SetWhich(nWhich);
        AddRef( *pPoolItem );
        return *pPoolItem;
    }

    assert(!pImp->ppStaticDefaults ||
            typeid(rItem) == typeid(GetDefaultItem(nWhich)));

    SfxPoolItemArray_Impl* pItemArr = pImp->maPoolItems[nIndex];
    if (!pItemArr)
    {
        pImp->maPoolItems[nIndex] = new SfxPoolItemArray_Impl;
        pItemArr = pImp->maPoolItems[nIndex];
    }

    SfxPoolItemArrayBase_Impl::iterator ppFree;
    bool ppFreeIsSet = false;

    // Is this a 'poolable' item - ie. should we re-use and return
    // the same underlying item for equivalent (==) SfxPoolItems?
    if ( IsItemFlag_Impl( nIndex, SfxItemPoolFlags::POOLABLE ) )
    {
        // if is already in a pool, then it is worth checking if it is in this one.
        if ( IsPooledItem(&rItem) )
        {
            SfxPoolItemArray_Impl::PoolItemPtrToIndexMap::const_iterator it;
            it = pItemArr->maPtrToIndex.find(const_cast<SfxPoolItem *>(&rItem));

            // 1. search for an identical pointer in the pool
            if (it != pItemArr->maPtrToIndex.end())
            {
                AddRef(rItem);
                return rItem;
            }
        }

        // 2. search for an item with matching attributes.
        SfxPoolItemArrayBase_Impl::iterator itr = pItemArr->begin();
        for (; itr != pItemArr->end(); ++itr)
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
        if (pItemArr->maFree.size() > 0)
        {
            SfxPoolItemArrayBase_Impl::iterator itr = pItemArr->begin();
            sal_uInt32 nIdx = pItemArr->maFree.back();
            pItemArr->maFree.pop_back();

            assert(nIdx < pItemArr->size());
            std::advance(itr, nIdx);
            ppFreeIsSet = true;
            ppFree = itr;
        }
    }

    // 3. not found, so clone to insert into the pointer array.
    SfxPoolItem* pNewItem = rItem.Clone(pImp->mpMaster);
    pNewItem->SetWhich(nWhich);
    assert(typeid(rItem) == typeid(*pNewItem) && "SfxItemPool::Put(): unequal types, no Clone() override?");
    if (dynamic_cast<const SfxSetItem*>(&rItem) == nullptr)
    {
        assert((!IsItemFlag(nWhich, SfxItemPoolFlags::POOLABLE) || rItem == *pNewItem)
            && "SfxItemPool::Put(): unequal items: no operator== override?");
        assert((!IsItemFlag(*pNewItem, SfxItemPoolFlags::POOLABLE) || *pNewItem == rItem)
            && "SfxItemPool::Put(): unequal items: no operator== override?");
    }
    AddRef( *pNewItem, pImp->nInitRefCount );

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

/// Re-build our free list and pointer hash.
void SfxPoolItemArray_Impl::ReHash()
{
    maFree.clear();
    maPtrToIndex.clear();

    for (size_t nIdx = 0; nIdx < size(); ++nIdx)
    {
        SfxPoolItem *pItem = (*this)[nIdx];
        if (!pItem)
            maFree.push_back(nIdx);
        else
        {
            maPtrToIndex.insert(std::make_pair(pItem,nIdx));
            assert(maPtrToIndex.find(pItem) != maPtrToIndex.end());
        }
    }
}

void SfxItemPool::Remove( const SfxPoolItem& rItem )
{
    assert(!IsPoolDefaultItem(&rItem) && "cannot remove Pool Default");

    // Find correct Secondary Pool
    const sal_uInt16 nWhich = rItem.Which();
    bool bSID = nWhich > SFX_WHICH_MAX;
    if ( !bSID && !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
        {
            pImp->mpSecondary->Remove( rItem );
            return;
        }
        OSL_FAIL( "unknown WhichId - cannot remove item" );
    }

    // SID or not poolable (new definition)?
    sal_uInt16 nIndex = bSID ? USHRT_MAX : GetIndex_Impl(nWhich);
    if ( bSID || IsItemFlag_Impl( nIndex, SfxItemPoolFlags::NOT_POOLABLE ) )
    {
        assert((USHRT_MAX != nIndex || !IsDefaultItem(&rItem)) &&
                    "a non Pool Item is Default?!");
        if ( 0 == ReleaseRef(rItem) )
        {
            SfxPoolItem *pItem = &(SfxPoolItem &)rItem;
            delete pItem;
        }
        return;
    }

    assert(rItem.GetRefCount() && "RefCount == 0, Remove impossible");

    // Static Defaults are just there
    if ( rItem.GetKind() == SFX_ITEMS_STATICDEFAULT &&
         &rItem == *( pImp->ppStaticDefaults + GetIndex_Impl(nWhich) ) )
        return;

    // Find Item in own Pool
    SfxPoolItemArray_Impl* pItemArr = pImp->maPoolItems[nIndex];
    assert(pItemArr && "removing Item not in Pool");

    SfxPoolItemArray_Impl::PoolItemPtrToIndexMap::iterator it;
    it = pItemArr->maPtrToIndex.find(const_cast<SfxPoolItem *>(&rItem));
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
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetDefaultItem( nWhich );
        assert(!"unknown which - don't ask me for defaults");
    }

    DBG_ASSERT( pImp->ppStaticDefaults, "no defaults known - don't ask me for defaults" );
    sal_uInt16 nPos = GetIndex_Impl(nWhich);
    SfxPoolItem *pDefault = *(pImp->ppPoolDefaults + nPos);
    if ( pDefault )
        return *pDefault;
    return **(pImp->ppStaticDefaults + nPos);
}

SfxItemPool* SfxItemPool::GetSecondaryPool() const
{
    return pImp->mpSecondary;
}

SfxItemPool* SfxItemPool::GetMasterPool() const
{
    return pImp->mpMaster;
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
    FillItemIdRanges_Impl( pImp->mpPoolRanges );
}


void SfxItemPool::FillItemIdRanges_Impl( sal_uInt16*& pWhichRanges ) const
{
    DBG_ASSERT( !pImp->mpPoolRanges, "GetFrozenRanges() would be faster!" );

    const SfxItemPool *pPool;
    sal_uInt16 nLevel = 0;
    for( pPool = this; pPool; pPool = pPool->pImp->mpSecondary )
        ++nLevel;

    pWhichRanges = new sal_uInt16[ 2*nLevel + 1 ];

    nLevel = 0;
    for( pPool = this; pPool; pPool = pPool->pImp->mpSecondary )
    {
        *(pWhichRanges+(nLevel++)) = pPool->pImp->mnStart;
        *(pWhichRanges+(nLevel++)) = pPool->pImp->mnEnd;
        *(pWhichRanges+nLevel) = 0;
    }
}

const sal_uInt16* SfxItemPool::GetFrozenIdRanges() const
{
    return pImp->mpPoolRanges;
}

const SfxPoolItem *SfxItemPool::GetItem2Default(sal_uInt16 nWhich) const
{
    return GetItem2(nWhich, SFX_ITEMS_DEFAULT);
}

const SfxPoolItem *SfxItemPool::GetItem2(sal_uInt16 nWhich, sal_uInt32 nOfst) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetItem2( nWhich, nOfst );
        assert(false && "unknown WhichId - cannot resolve surrogate");
        return nullptr;
    }

    // default attribute?
    if ( nOfst == SFX_ITEMS_DEFAULT )
        return *(pImp->ppStaticDefaults + GetIndex_Impl(nWhich));

    SfxPoolItemArray_Impl* pItemArr = pImp->maPoolItems[GetIndex_Impl(nWhich)];
    if( pItemArr && nOfst < pItemArr->size() )
        return (*pItemArr)[nOfst];

    return nullptr;
}

sal_uInt32 SfxItemPool::GetItemCount2(sal_uInt16 nWhich) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetItemCount2( nWhich );
        assert(false && "unknown WhichId - cannot resolve surrogate");
        return 0;
    }

    SfxPoolItemArray_Impl* pItemArr = pImp->maPoolItems[GetIndex_Impl(nWhich)];
    if  ( pItemArr )
        return pItemArr->size();
    return 0;
}


sal_uInt16 SfxItemPool::GetWhich( sal_uInt16 nSlotId, bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return nSlotId;

    sal_uInt16 nCount = pImp->mnEnd - pImp->mnStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nSID == nSlotId )
            return nOfs + pImp->mnStart;
    if ( pImp->mpSecondary && bDeep )
        return pImp->mpSecondary->GetWhich(nSlotId);
    return nSlotId;
}


sal_uInt16 SfxItemPool::GetSlotId( sal_uInt16 nWhich, bool bDeep ) const
{
    if ( !IsWhich(nWhich) )
        return nWhich;

    if ( !IsInRange( nWhich ) )
    {
        if ( pImp->mpSecondary && bDeep )
            return pImp->mpSecondary->GetSlotId(nWhich);
        assert(false && "unknown WhichId - cannot get slot-id");
        return 0;
    }

    sal_uInt16 nSID = pItemInfos[nWhich - pImp->mnStart]._nSID;
    return nSID ? nSID : nWhich;
}


sal_uInt16 SfxItemPool::GetTrueWhich( sal_uInt16 nSlotId, bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return 0;

    sal_uInt16 nCount = pImp->mnEnd - pImp->mnStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nSID == nSlotId )
            return nOfs + pImp->mnStart;
    if ( pImp->mpSecondary && bDeep )
        return pImp->mpSecondary->GetTrueWhich(nSlotId);
    return 0;
}


sal_uInt16 SfxItemPool::GetTrueSlotId( sal_uInt16 nWhich, bool bDeep ) const
{
    if ( !IsWhich(nWhich) )
        return 0;

    if ( !IsInRange( nWhich ) )
    {
        if ( pImp->mpSecondary && bDeep )
            return pImp->mpSecondary->GetTrueSlotId(nWhich);
        assert(false && "unknown WhichId - cannot get slot-id");
        return 0;
    }
    return pItemInfos[nWhich - pImp->mnStart]._nSID;
}

/**
 * You must call this function to set the file format version after
 * concatenating your secondary-pools but before you store any
 * pool, itemset or item. Only set the version at the master pool,
 * never at any secondary pool.
 */
void SfxItemPool::SetFileFormatVersion( sal_uInt16 nFileFormatVersion )
{
    DBG_ASSERT( this == pImp->mpMaster,
                "SfxItemPool::SetFileFormatVersion() but not a master pool" );
    for ( SfxItemPool *pPool = this; pPool; pPool = pPool->pImp->mpSecondary )
        pPool->pImp->mnFileFormatVersion = nFileFormatVersion;
}

const SfxItemPool* SfxItemPool::pStoringPool_ = nullptr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
