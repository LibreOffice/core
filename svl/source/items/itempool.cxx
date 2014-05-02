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
#include <stdio.h>

#include <svl/itempool.hxx>
#include "whassert.hxx"
#include <svl/brdcst.hxx>
#include <svl/smplhint.hxx>
#include "poolio.hxx"


#if OSL_DEBUG_LEVEL > 0
#include <map>

static void
lcl_CheckSlots2(std::map<sal_uInt16, sal_uInt16> & rSlotMap,
        SfxItemPool const& rPool, SfxItemInfo const* pInfos)
{
    if (!pInfos)
        return; // may not be initialized yet
    if (rPool.GetName() == "EditEngineItemPool")
        return; // HACK: this one has loads of duplicates already, ignore it :(
    sal_uInt16 const nFirst(rPool.GetFirstWhich());
    sal_uInt16 const nCount(rPool.GetLastWhich() - rPool.GetFirstWhich() + 1);
    for (sal_uInt16 n = 0; n < nCount; ++n)
    {
        sal_uInt16 const nSlotId(pInfos[n]._nSID);
        if (nSlotId != 0
            && nSlotId != 10883  // preexisting duplicate SID_ATTR_GRAF_CROP
            && nSlotId != 10024) // preexisting duplicate SID_ATTR_BORDER_OUTER
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
        SFX_ASSERT( false, nWhich, "unknown Which-Id - cannot get pool default" );
        pRet = 0;
    }
    return pRet;
}



bool SfxItemPool::IsItemFlag_Impl( sal_uInt16 nPos, sal_uInt16 nFlag ) const
{
    sal_uInt16 nItemFlag = pItemInfos[nPos]._nFlags;
    return nFlag == (nItemFlag & nFlag);
}



bool SfxItemPool::IsItemFlag( sal_uInt16 nWhich, sal_uInt16 nFlag ) const
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




SfxItemPool::SfxItemPool
(
    const OUString& rName,              /* Name of the pool for identification
                                           in the file format */
    sal_uInt16          nStartWhich,    /* first Which-Id of the Pool */
    sal_uInt16          nEndWhich,      /* last Which-Id of the Pool*/
    const SfxItemInfo*  pInfos,         /* SID-Map and Item-Flags */
    SfxPoolItem**       pDefaults,      /* Pointer to static defaults.
                                           Are referenced directly from the
                                           Pool, but without transfer of
                                           ownership */
    bool                bLoadRefCounts  /* load Ref-Counts or set to 1 */
) :

/*  [Description]

    The constructor used normally for class SfxItemPool. An SfxItemPool
    instance is initialized which can manage Items in the Which
    range from 'nStartWhich' to 'nEndWhich'.

    For each of these Which-Ids a static default must be
    present in the 'pDefaults' array. This array must contain
    consecutive <SfxPoolItem>-s, sorted by Which-Id, the first
    of which has Which-Id 'nStartWhich'.

    'pItemInfos' is an identially arranged array of USHORTs, which
    represent Slot-Ids and flags. The Slot-Ids can be 0 if the
    corresponding Items are only used in the Core.
    The flags indicate for instance whether Value-Sharing
    (SFX_ITEM_POOLABLE) should happen.


    [Note]

    If the Pool should contain <SfxSetItem>-s then the static defaults
    cannot be given yet in the constructor. In that case they should be
    set later with <SfxItemPool::SetDefaults(SfxItemPool**)>.


    [References]

    <SfxItemPool::SetDefaults(SfxItemPool**)>
    <SfxItemPool::ReleaseDefaults(SfxPoolItem**,sal_uInt16,sal_Bool)>
    <SfxItemPool::ReldaseDefaults(sal_Bool)>
*/

    pItemInfos(pInfos),
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




SfxItemPool::SfxItemPool
(
    const SfxItemPool&  rPool,                  //  instance to copy from
    bool                bCloneStaticDefaults    /*  sal_True
                                                    clone static defaults

                                                    sal_False
                                                    reference static defaults
                                                */
) :

/*  [Description]

    copy constructor of class SfxItemPool.


    [References]

    <SfxItemPool::Clone()const>
*/

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

    // copy or clonse Static Defaults
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

    // copy Pool Defaults
    for ( sal_uInt16 n = 0; n <= pImp->mnEnd - pImp->mnStart; ++n )
        if ( (*( rPool.pImp->ppPoolDefaults + n )) )
        {
            (*( pImp->ppPoolDefaults + n )) = (*( rPool.pImp->ppPoolDefaults + n ))->Clone(this);
            (*( pImp->ppPoolDefaults + n ))->SetKind( SFX_ITEMS_POOLDEFAULT );
        }

    // Copy Version-Map
    for ( size_t nVer = 0; nVer < rPool.pImp->aVersions.size(); ++nVer )
    {
        const SfxPoolVersion_ImplPtr pOld = rPool.pImp->aVersions[nVer];
        SfxPoolVersion_ImplPtr pNew = SfxPoolVersion_ImplPtr( new SfxPoolVersion_Impl( *pOld ) );
        pImp->aVersions.push_back( pNew );
    }

    // restore link to secondary pool
    if ( rPool.pImp->mpSecondary )
        SetSecondaryPool( rPool.pImp->mpSecondary->Clone() );
}



void SfxItemPool::SetDefaults( SfxPoolItem **pDefaults )
{
    DBG_ASSERT( pDefaults, "promise but not provide..." );
    DBG_ASSERT( !pImp->ppStaticDefaults, "defaults exist already" );

    pImp->ppStaticDefaults = pDefaults;
    //! if ( (*ppStaticDefaults)->GetKind() != SFX_ITEMS_STATICDEFAULT )
    //! doesn't work with SetItems that come later
    {
        DBG_ASSERT( (*pImp->ppStaticDefaults)->GetRefCount() == 0 ||
                    IsDefaultItem( (*pImp->ppStaticDefaults) ),
                    "those are note statics" );
        for ( sal_uInt16 n = 0; n <= pImp->mnEnd - pImp->mnStart; ++n )
        {
            SFX_ASSERT( (*( pImp->ppStaticDefaults + n ))->Which() == n + pImp->mnStart,
                        n + pImp->mnStart, "static defaults not sorted" );
            (*( pImp->ppStaticDefaults + n ))->SetKind( SFX_ITEMS_STATICDEFAULT );
            DBG_ASSERT( !(pImp->maPoolItems[n]), "defaults with setitems with items?!" );
        }
    }
}



void SfxItemPool::ReleaseDefaults
(
    bool    bDelete     /*  sal_True
                            deletes both the array and the individual
                            Static Defaults

                            sal_False
                            deletes neither the array nor the individual
                            Static Defaults */
)

/*  [Description]

    Releases the Static Defaults of the given SfxItemPool instance
    and deletes the static defaults if requested.

    After calling the method use of the SfxItemPool instance is not
    allowed any more, only calling the destructor is still possible.
*/

{
    DBG_ASSERT( pImp->ppStaticDefaults, "no Static Defaults left");
    ReleaseDefaults( pImp->ppStaticDefaults, pImp->mnEnd - pImp->mnStart + 1, bDelete );

    // KSO (22.10.98): ppStaticDefaults points to deleted memory
    // if bDelete == sal_True.
    if ( bDelete )
        pImp->ppStaticDefaults = 0;
}



void SfxItemPool::ReleaseDefaults
(
    SfxPoolItem**   pDefaults,  /*  Static Defaults to release */

    sal_uInt16      nCount,     /*  number of Static Defaults */

    bool            bDelete     /*  sal_True
                                    deletes both the array and the
                                    individual Static Defaults

                                    sal_False
                                    deletes neither the array nor the
                                    individual Static Defaults
)

/*  [Description]

    Releases the given static defaults and deletes them if requested.

    This method can only be called after all SfxItemPool instances
    that use it as the 'pDefault' static defaults have been deleted.
*/

{
    DBG_ASSERT( pDefaults, "release what?");

    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        SFX_ASSERT( IsStaticDefaultItem( *(pDefaults+n) ),
                    n, "that is not a static default" );
        (*( pDefaults + n ))->SetRefCount( 0 );
        if ( bDelete )
            { delete *( pDefaults + n ); *(pDefaults + n) = 0; }
    }

    if ( bDelete )
        { delete[] pDefaults; pDefaults = 0; }
}



SfxItemPool::~SfxItemPool()
{
    if ( !pImp->maPoolItems.empty() && pImp->ppPoolDefaults )
        Delete();

    if (pImp->mpMaster != NULL && pImp->mpMaster != this)
    {
        // This condition indicates an error.  A
        // pImp->mpMaster->SetSecondaryPool(...) call should have been made
        // earlier to prevent this.  At this point we can only try to
        // prevent a crash later on.
        DBG_ASSERT( pImp->mpMaster == this, "destroying active Secondary-Pool" );
        if (pImp->mpMaster->pImp->mpSecondary == this)
            pImp->mpMaster->pImp->mpSecondary = NULL;
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
    // restore unlinked Pool to master if necessary
    if ( pImp->mpSecondary )
    {
#ifdef DBG_UTIL
        SAL_INFO( "svl.items", "for Image: nu Statics currently - Bug" );
        if ( pImp->ppStaticDefaults )
        {
            // delete() has not been called yet?
            if ( !pImp->maPoolItems.empty() && !pImp->mpSecondary->pImp->maPoolItems.empty() )
            {
                // does the master have SetItems?
                bool bHasSetItems = false;
                for ( sal_uInt16 i = 0; !bHasSetItems && i < pImp->mnEnd - pImp->mnStart; ++i )
                    bHasSetItems = pImp->ppStaticDefaults[i]->ISA(SfxSetItem);

                // unlinked Pool must be empty
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

    // set Master on Secondary-Pools if present
    DBG_ASSERT( !pPool || pPool->pImp->mpMaster == pPool, "Secondary serves 2 masters " );
    SfxItemPool *pNewMaster = pImp->mpMaster ? pImp->mpMaster : this;
    for ( SfxItemPool *p = pPool; p; p = p->pImp->mpSecondary )
        p->pImp->mpMaster = pNewMaster;

    // mark new Secondary-Pool
    pImp->mpSecondary = pPool;

    CHECK_SLOTS();
}

void SfxItemPool::SetItemInfos(SfxItemInfo const*const pInfos)
{
    pItemInfos = pInfos;
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



SfxItemPresentation SfxItemPool::GetPresentation
(
    const SfxPoolItem&  rItem,
    SfxItemPresentation ePresent,
    SfxMapUnit          eMetric,
    OUString&           rText,
    const IntlWrapper * pIntlWrapper
)   const
{
    return rItem.GetPresentation(
        ePresent, GetMetric(rItem.Which()), eMetric, rText, pIntlWrapper );
}




SfxItemPool* SfxItemPool::Clone() const
{
    SfxItemPool *pPool = new SfxItemPool( *this );
    return pPool;
}



void SfxItemPool::Delete()
{
    // deleted already?
    if ( pImp->maPoolItems.empty() || !pImp->ppPoolDefaults )
        return;

    // notify current Requests, for instance
    pImp->aBC.Broadcast( SfxSimpleHint( SFX_HINT_DYING ) );

    //MA 16. Apr. 97: Do tw twice, in the first round for the SetItems.
    //Split up into two more readable loops for clarity.

    std::vector<SfxPoolItemArray_Impl*>::iterator itrItemArr = pImp->maPoolItems.begin();
    SfxPoolItem** ppDefaultItem = pImp->ppPoolDefaults;
    SfxPoolItem** ppStaticDefaultItem = pImp->ppStaticDefaults;
    sal_uInt16 nArrCnt;

    //First clean up the SetItems
    SAL_INFO( "svl.items", "for Image: nu Statics currently - Bug" );
    if ( pImp->ppStaticDefaults )
    {
        for ( nArrCnt = GetSize_Impl();
                nArrCnt;
                --nArrCnt, ++itrItemArr, ++ppDefaultItem, ++ppStaticDefaultItem )
        {
            // KSO (22.10.98): *ppStaticDefaultItem can already have been
            // deleted in the dtor of classes derived from SfxItemPool!
            // -> CHAOS Itempool
            if ( *ppStaticDefaultItem && (*ppStaticDefaultItem)->ISA(SfxSetItem) )
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

    // Now the 'simple' Items
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
        SFX_ASSERT( false, rItem.Which(), "unknown Which-Id - cannot set pool default" );
    }
}

/*
 * Resets the default of the given <Which-Id> back to the static default.
 * If a pool default exists it is removed.
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
        SFX_ASSERT( false, nWhichId, "unknown Which-Id - cannot set pool default" );
    }
}



const SfxPoolItem& SfxItemPool::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
                0 != &((const SfxSetItem&)rItem).GetItemSet(),
                "SetItem without ItemSet" );

    if ( 0 == nWhich )
        nWhich = rItem.Which();

    // find the right Secondary-Pool
    bool bSID = nWhich > SFX_WHICH_MAX;
    if ( !bSID && !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->Put( rItem, nWhich );
        OSL_FAIL( "unknown Which-Id - cannot put item" );
    }

    // SID or not poolable (new definition)?
    sal_uInt16 nIndex = bSID ? USHRT_MAX : GetIndex_Impl(nWhich);
    if ( USHRT_MAX == nIndex ||
         IsItemFlag_Impl( nIndex, SFX_ITEM_NOT_POOLABLE ) )
    {
        SFX_ASSERT( USHRT_MAX != nIndex || rItem.Which() != nWhich ||
                    !IsDefaultItem(&rItem) || rItem.GetKind() == SFX_ITEMS_DELETEONIDLE,
                    nWhich, "a non-Pool-Item is Default?!" );
        SfxPoolItem *pPoolItem = rItem.Clone(pImp->mpMaster);
        pPoolItem->SetWhich(nWhich);
        AddRef( *pPoolItem );
        return *pPoolItem;
    }

    SFX_ASSERT( rItem.IsA(GetDefaultItem(nWhich).Type()), nWhich,
                "SFxItemPool: wrong item type in Put" );

    SfxPoolItemArray_Impl* pItemArr = pImp->maPoolItems[nIndex];
    if (!pItemArr)
    {
        pImp->maPoolItems[nIndex] = new SfxPoolItemArray_Impl;
        pItemArr = pImp->maPoolItems[nIndex];
    }

    SfxPoolItemArrayBase_Impl::iterator ppFree;
    bool ppFreeIsSet = false;
    if ( IsItemFlag_Impl( nIndex, SFX_ITEM_POOLABLE ) )
    {
        // if it is in a Pool already wenn it could be in the currentone
        if ( IsPooledItem(&rItem) )
        {
            // 1st loop: test whether the pointer is present.
            SfxPoolItemArrayBase_Impl::iterator itr =
                std::find(pItemArr->begin(), pItemArr->end(), &rItem);
            if (itr != pItemArr->end())
            {
                AddRef(**itr);
                return **itr;
            }
        }

        // 2nd loop: then also compare the attributes
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
        // look for a free slot
        SfxPoolItemArrayBase_Impl::iterator itr = pItemArr->begin();
        std::advance(itr, pItemArr->nFirstFree);
        for (; itr != pItemArr->end(); ++itr)
        {
            if (!*itr)
            {
                ppFree = itr;
                ppFreeIsSet = true;
                break;
            }
        }
        // mark the next free slot
        pItemArr->nFirstFree = std::distance(pItemArr->begin(), itr);
    }

    // not present, so add to the PtrArray
    SfxPoolItem* pNewItem = rItem.Clone(pImp->mpMaster);
    pNewItem->SetWhich(nWhich);
#ifdef DBG_UTIL
    SFX_ASSERT( rItem.Type() == pNewItem->Type(), nWhich, "unequal types in Put(): no Clone()?" )
    if ( !rItem.ISA(SfxSetItem) )
    {
        SFX_ASSERT( !IsItemFlag(nWhich, SFX_ITEM_POOLABLE) ||
                    rItem == *pNewItem,
                    nWhich, "unequal items in Put(): no operator==?" );
        SFX_ASSERT( !IsItemFlag(*pNewItem, SFX_ITEM_POOLABLE) ||
                    *pNewItem == rItem,
                    nWhich, "unequal items in Put(): no operator==?" );
    }
#endif
    AddRef( *pNewItem, pImp->nInitRefCount );

    if ( ppFreeIsSet == false )
        pItemArr->push_back( pNewItem );
    else
    {
        DBG_ASSERT( *ppFree == 0, "using surrogate in use" );
        *ppFree = pNewItem;
    }
    return *pNewItem;
}



void SfxItemPool::Remove( const SfxPoolItem& rItem )
{
    DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
                0 != &((const SfxSetItem&)rItem).GetItemSet(),
                "SetItem without ItemSet" );

    SFX_ASSERT( !IsPoolDefaultItem(&rItem), rItem.Which(),
                "That's not a PoolDefaultItem" );

    // find the right Secondary-Pool
    const sal_uInt16 nWhich = rItem.Which();
    bool bSID = nWhich > SFX_WHICH_MAX;
    if ( !bSID && !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
        {
            pImp->mpSecondary->Remove( rItem );
            return;
        }
        OSL_FAIL( "unknown Which-Id - cannot remove item" );
    }

    // SID or not poolable (new definition)?
    sal_uInt16 nIndex = bSID ? USHRT_MAX : GetIndex_Impl(nWhich);
    if ( bSID || IsItemFlag_Impl( nIndex, SFX_ITEM_NOT_POOLABLE ) )
    {
        SFX_ASSERT( USHRT_MAX != nIndex ||
                    !IsDefaultItem(&rItem), rItem.Which(),
                    "a non-Pool-Item is Default?!" );
        if ( 0 == ReleaseRef(rItem) )
        {
            SfxPoolItem *pItem = &(SfxPoolItem &)rItem;
            delete pItem;
        }
        return;
    }

    SFX_ASSERT( rItem.GetRefCount(), rItem.Which(), "RefCount == 0, impossible to remove" );

    // statische Default just exist
    if ( rItem.GetKind() == SFX_ITEMS_STATICDEFAULT &&
         &rItem == *( pImp->ppStaticDefaults + GetIndex_Impl(nWhich) ) )
        return;

    // look for Item in the own Pool
    SfxPoolItemArray_Impl* pItemArr = pImp->maPoolItems[nIndex];
    SFX_ASSERT( pItemArr, rItem.Which(), "removing Item not in Pool" );
    SfxPoolItemArrayBase_Impl::iterator ppHtArrBeg = pItemArr->begin(), ppHtArrEnd = pItemArr->end();
    for (SfxPoolItemArrayBase_Impl::iterator ppHtArr = ppHtArrBeg; ppHtArr != ppHtArrEnd; ++ppHtArr)
    {
        SfxPoolItem*& p = *ppHtArr;
        if (p == &rItem)
        {
            if ( p->GetRefCount() ) //!
                ReleaseRef( *p );
            else
            {
                SFX_ASSERT( false, rItem.Which(), "removing Item without ref" );
            }

            // mark first free Position if applicable
            size_t nPos = std::distance(ppHtArrBeg, ppHtArr);
            if ( pItemArr->nFirstFree > nPos )
                pItemArr->nFirstFree = nPos;

            //! MI: Hack, as long as there are problems with the Outliner.
            //! otherwise see MI-REF
            if ( 0 == p->GetRefCount() && nWhich < 4000 )
                DELETEZ(p);
            return;
        }
    }

    // not present
    SFX_ASSERT( false, rItem.Which(), "removing Item not in Pool" );
}



const SfxPoolItem& SfxItemPool::GetDefaultItem( sal_uInt16 nWhich ) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetDefaultItem( nWhich );
        SFX_ASSERT( false, nWhich, "unknown which - dont ask me for defaults" );
    }

    DBG_ASSERT( pImp->ppStaticDefaults, "no defaults known - dont ask me for defaults" );
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

#ifdef DBG_UTIL
SfxItemPool* SfxItemPool::GetMasterPool() const
{
    return pImp->mpMaster;
}
#endif

void SfxItemPool::FreezeIdRanges()

/*  [Description]

    This method should be called at the master pool, when all secondary
    pools are appended to it.

    It calculates the ranges of 'which-ids' for fast construction of
    item-sets, which contains all 'which-ids'.
*/

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

const SfxPoolItem *SfxItemPool::GetItem2(sal_uInt16 nWhich, sal_uInt32 nOfst) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetItem2( nWhich, nOfst );
        SFX_ASSERT( false, nWhich, "unknown Which-Id - cannot resolve surrogate" );
        return 0;
    }

    // dflt-Attribut?
    if ( nOfst == SFX_ITEMS_DEFAULT )
        return *(pImp->ppStaticDefaults + GetIndex_Impl(nWhich));

    SfxPoolItemArray_Impl* pItemArr = pImp->maPoolItems[GetIndex_Impl(nWhich)];
    if( pItemArr && nOfst < pItemArr->size() )
        return (*pItemArr)[nOfst];

    return 0;
}



sal_uInt32 SfxItemPool::GetItemCount2(sal_uInt16 nWhich) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImp->mpSecondary )
            return pImp->mpSecondary->GetItemCount2( nWhich );
        SFX_ASSERT( false, nWhich, "unknown Which-Id - cannot resolve surrogate" );
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
        SFX_ASSERT( false, nWhich, "unknown Which-Id - cannot get slot-id" );
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
        SFX_ASSERT( false, nWhich, "unknown Which-Id - cannot get slot-id" );
        return 0;
    }
    return pItemInfos[nWhich - pImp->mnStart]._nSID;
}

void SfxItemPool::SetFileFormatVersion( sal_uInt16 nFileFormatVersion )

/*  [Description]

    You must call this function to set the file format version after
    concatenating your secondary-pools but before you store any
    pool, itemset or item. Only set the version at the master pool,
    never at any secondary pool.
*/

{
    DBG_ASSERT( this == pImp->mpMaster,
                "SfxItemPool::SetFileFormatVersion() but not a master pool" );
    for ( SfxItemPool *pPool = this; pPool; pPool = pPool->pImp->mpSecondary )
        pPool->pImp->mnFileFormatVersion = nFileFormatVersion;
}

const SfxItemPool* SfxItemPool::pStoringPool_ = 0;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
