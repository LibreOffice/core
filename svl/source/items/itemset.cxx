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

#include <cstdarg>
#include <libxml/xmlwriter.h>

#include <sal/log.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/nranges.hxx>
#include "whassert.hxx"

#include <tools/stream.hxx>
#include <tools/solar.h>
#include <rtl/string.hxx>

static const sal_uInt16 nInitCount = 10; // Single USHORTs => 5 pairs without '0'
#include "nranges.cxx"
#include "poolio.hxx"


#ifdef DBG_UTIL


const sal_Char *DbgCheckItemSet( const void* pVoid )
{
    const SfxItemSet *pSet = static_cast<const SfxItemSet*>(pVoid);
    SfxWhichIter aIter( *pSet );
    sal_uInt16 nCount = 0, n = 0;
    for ( sal_uInt16 nWh = aIter.FirstWhich(); nWh; nWh = aIter.NextWhich(), ++n )
    {
        const SfxPoolItem *pItem = pSet->m_pItems[n];
        if ( pItem )
        {
            ++nCount;
            assert((IsInvalidItem(pItem) ||
                        pItem->Which() == 0 || pItem->Which() == nWh
                    ) && "SfxItemSet: invalid which-id" );
            assert((IsInvalidItem(pItem) || !pItem->Which() ||
                    !SfxItemPool::IsWhich(pItem->Which()) ||
                    pSet->GetPool()->IsItemFlag(nWh, SfxItemPoolFlags::NOT_POOLABLE) ||
                    SFX_ITEMS_NULL != pSet->GetPool()->GetSurrogate(pItem)
                   ) && "SfxItemSet: item in set which is not in pool" );
        }

    }
    assert(pSet->m_nCount == nCount);

    return nullptr;
}

#endif

/**
 * Ctor for a SfxItemSet with exactly the Which Ranges, which are known to
 * the supplied SfxItemPool.
 *
 * For Sfx programmers: an SfxItemSet constructed in this way cannot
 * contain any Items with SlotIds as Which values.
 */
SfxItemSet::SfxItemSet
(
    SfxItemPool&    rPool,       /* Target Pool for the SfxPoolItems which are
                                    added to this SfxItemSet */
    bool        bTotalRanges    /* Take over complete pool ranges? */
)
    : m_pPool( &rPool )
    , m_pParent(nullptr)
    , m_nCount(0)
{
//  DBG_ASSERT( bTotalRanges || abs( &bTotalRanges - this ) < 1000,
//              "please use suitable ranges" );
#if defined DBG_UTIL && defined SFX_ITEMSET_NO_DEFAULT_CTOR
    if ( !bTotalRanges )
        *(int*)0 = 0; // GPF
#else
    (void) bTotalRanges; // avoid warnings
#endif

    m_pWhichRanges = const_cast<sal_uInt16*>(m_pPool->GetFrozenIdRanges());
    assert( m_pWhichRanges && "don't create ItemSets with full range before FreezeIdRanges()" );
    if (!m_pWhichRanges)
        m_pPool->FillItemIdRanges_Impl( m_pWhichRanges );

    const sal_uInt16 nSize = TotalCount();
    m_pItems = new const SfxPoolItem* [ nSize ];
    memset(static_cast<void*>(m_pItems), 0, nSize * sizeof(SfxPoolItem*));
}

SfxItemSet::SfxItemSet(SfxItemPool& rPool, sal_uInt16 nWhich1, sal_uInt16 nWhich2)
    : m_pPool( &rPool )
    , m_pParent(nullptr)
    , m_nCount(0)
{
    assert(nWhich1 <= nWhich2);

    InitRanges_Impl(nWhich1, nWhich2);
}

void SfxItemSet::InitRanges_Impl(sal_uInt16 nWh1, sal_uInt16 nWh2)
{
    m_pWhichRanges = new sal_uInt16[ 3 ];
    *(m_pWhichRanges+0) = nWh1;
    *(m_pWhichRanges+1) = nWh2;
    *(m_pWhichRanges+2) = 0;
    const sal_uInt16 nRg = nWh2 - nWh1 + 1;
    m_pItems = new const SfxPoolItem* [ nRg ];
    memset(static_cast<void*>(m_pItems), 0, nRg * sizeof(SfxPoolItem*));
}

void SfxItemSet::InitRanges_Impl(va_list pArgs, sal_uInt16 nWh1, sal_uInt16 nWh2, sal_uInt16 nNull)
{
    sal_uInt16 nSize = InitializeRanges_Impl(m_pWhichRanges, pArgs, nWh1, nWh2, nNull);
    m_pItems = new const SfxPoolItem* [ nSize ];
    memset(static_cast<void*>(m_pItems), 0, sizeof(SfxPoolItem*) * nSize);
}

SfxItemSet::SfxItemSet(SfxItemPool& rPool, int nWh1, int nWh2, int nNull, ...)
    : m_pPool( &rPool )
    , m_pParent(nullptr)
    , m_pWhichRanges(nullptr)
    , m_nCount(0)
{
    assert(nWh1 <= nWh2);

    if(!nNull)
        InitRanges_Impl(
            sal::static_int_cast< sal_uInt16 >(nWh1),
            sal::static_int_cast< sal_uInt16 >(nWh2));
    else {
        va_list pArgs;
        va_start( pArgs, nNull );
        InitRanges_Impl(
            pArgs, sal::static_int_cast< sal_uInt16 >(nWh1),
            sal::static_int_cast< sal_uInt16 >(nWh2),
            sal::static_int_cast< sal_uInt16 >(nNull));
        va_end(pArgs);
    }
}

void SfxItemSet::InitRanges_Impl(const sal_uInt16 *pWhichPairTable)
{
    sal_uInt16 nCnt = 0;
    const sal_uInt16* pPtr = pWhichPairTable;
    while( *pPtr )
    {
        nCnt += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }

    m_pItems = new const SfxPoolItem* [ nCnt ];
    memset(static_cast<void*>(m_pItems), 0, sizeof(SfxPoolItem*) * nCnt);

    std::ptrdiff_t cnt = pPtr - pWhichPairTable +1;
    m_pWhichRanges = new sal_uInt16[ cnt ];
    memcpy( m_pWhichRanges, pWhichPairTable, sizeof( sal_uInt16 ) * cnt );
}

SfxItemSet::SfxItemSet( SfxItemPool& rPool, const sal_uInt16* pWhichPairTable )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_pItems(nullptr)
    , m_pWhichRanges(nullptr)
    , m_nCount(0)
{
    // pWhichPairTable == 0 is for the SfxAllEnumItemSet
    if ( pWhichPairTable )
        InitRanges_Impl(pWhichPairTable);
}

SfxItemSet::SfxItemSet( const SfxItemSet& rASet )
    : m_pPool( rASet.m_pPool )
    , m_pParent( rASet.m_pParent )
    , m_nCount( rASet.m_nCount )
{
    // Calculate the attribute count
    sal_uInt16 nCnt = 0;
    sal_uInt16* pPtr = rASet.m_pWhichRanges;
    while( *pPtr )
    {
        nCnt += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }

    m_pItems = new const SfxPoolItem* [ nCnt ];

    // Copy attributes
    SfxItemArray ppDst = m_pItems, ppSrc = rASet.m_pItems;
    for( sal_uInt16 n = nCnt; n; --n, ++ppDst, ++ppSrc )
        if ( nullptr == *ppSrc ||                 // Current Default?
             IsInvalidItem(*ppSrc) ||       // DontCare?
             IsStaticDefaultItem(*ppSrc) )  // Defaults that are not to be pooled?
            // Just copy the pointer
            *ppDst = *ppSrc;
        else if (m_pPool->IsItemFlag( **ppSrc, SfxItemPoolFlags::POOLABLE ))
        {
            // Just copy the pointer and increase RefCount
            *ppDst = *ppSrc;
            (*ppDst)->AddRef();
        }
        else if ( !(*ppSrc)->Which() )
            *ppDst = (*ppSrc)->Clone();
        else
            // !IsPoolable() => assign via Pool
            *ppDst = &m_pPool->Put( **ppSrc );

    // Copy the WhichRanges
    std::ptrdiff_t cnt = pPtr - rASet.m_pWhichRanges+1;
    m_pWhichRanges = new sal_uInt16[ cnt ];
    memcpy( m_pWhichRanges, rASet.m_pWhichRanges, sizeof( sal_uInt16 ) * cnt);
}

SfxItemSet::~SfxItemSet()
{
    sal_uInt16 nCount = TotalCount();
    if( Count() )
    {
        SfxItemArray ppFnd = m_pItems;
        for( sal_uInt16 nCnt = nCount; nCnt; --nCnt, ++ppFnd )
            if( *ppFnd && !IsInvalidItem(*ppFnd) )
            {
                if( !(*ppFnd)->Which() )
                    delete *ppFnd;
                else {
                    // Still multiple references present, so just alter the RefCount
                    if ( 1 < (*ppFnd)->GetRefCount() && !IsDefaultItem(*ppFnd) )
                        (*ppFnd)->ReleaseRef();
                    else
                        if ( !IsDefaultItem(*ppFnd) )
                            // Delete from Pool
                            m_pPool->Remove( **ppFnd );
                }
            }
    }

    delete[] m_pItems;
    if (m_pWhichRanges != m_pPool->GetFrozenIdRanges())
        delete[] m_pWhichRanges;
    m_pWhichRanges = nullptr; // for invariant-testing
}

/**
 * Delete single Items or all Items (nWhich == 0)
 */
sal_uInt16 SfxItemSet::ClearItem( sal_uInt16 nWhich )
{
    if( !Count() )
        return 0;

    sal_uInt16 nDel = 0;
    SfxItemArray ppFnd = m_pItems;

    if( nWhich )
    {
        const sal_uInt16* pPtr = m_pWhichRanges;
        while( *pPtr )
        {
            // Within this range?
            if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
            {
                // Actually set?
                ppFnd += nWhich - *pPtr;
                if( *ppFnd )
                {
                    // Due to the assertions in the sub calls, we need to do the following
                    --m_nCount;
                    const SfxPoolItem *pItemToClear = *ppFnd;
                    *ppFnd = nullptr;

                    if ( !IsInvalidItem(pItemToClear) )
                    {
                        if ( nWhich <= SFX_WHICH_MAX )
                        {
                            const SfxPoolItem& rNew = m_pParent
                                    ? m_pParent->Get( nWhich )
                                    : m_pPool->GetDefaultItem( nWhich );

                            Changed( *pItemToClear, rNew );
                        }
                        if ( pItemToClear->Which() )
                            m_pPool->Remove( *pItemToClear );
                    }
                    ++nDel;
                }

                // found => break
                break;
            }
            ppFnd += *(pPtr+1) - *pPtr + 1;
            pPtr += 2;
        }
    }
    else
    {
        nDel = m_nCount;

        sal_uInt16* pPtr = m_pWhichRanges;
        while( *pPtr )
        {
            for( nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if( *ppFnd )
                {
                    // Due to the assertions in the sub calls, we need to do this
                    --m_nCount;
                    const SfxPoolItem *pItemToClear = *ppFnd;
                    *ppFnd = nullptr;

                    if ( !IsInvalidItem(pItemToClear) )
                    {
                        if ( nWhich <= SFX_WHICH_MAX )
                        {
                            const SfxPoolItem& rNew = m_pParent
                                    ? m_pParent->Get( nWhich )
                                    : m_pPool->GetDefaultItem( nWhich );

                            Changed( *pItemToClear, rNew );
                        }

                        // #i32448#
                        // Take care of disabled items, too.
                        if (!pItemToClear->m_nWhich)
                        {
                            // item is disabled, delete it
                            delete pItemToClear;
                        }
                        else
                        {
                            // remove item from pool
                            m_pPool->Remove( *pItemToClear );
                        }
                    }
                }
            pPtr += 2;
        }
    }
    return nDel;
}

void SfxItemSet::ClearInvalidItems( bool bHardDefault )
{
    sal_uInt16* pPtr = m_pWhichRanges;
    SfxItemArray ppFnd = m_pItems;
    if ( bHardDefault )
        while( *pPtr )
        {
            for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if ( IsInvalidItem(*ppFnd) )
                     *ppFnd = &m_pPool->Put( m_pPool->GetDefaultItem(nWhich) );
            pPtr += 2;
        }
    else
        while( *pPtr )
        {
            for( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if( IsInvalidItem(*ppFnd) )
                {
                    *ppFnd = nullptr;
                    --m_nCount;
                }
            pPtr += 2;
        }
}

void SfxItemSet::InvalidateDefaultItems()
{
    sal_uInt16* pPtr = m_pWhichRanges;
    SfxItemArray ppFnd = m_pItems;

    while( *pPtr )
    {
        for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
            if (*ppFnd && *ppFnd != reinterpret_cast<SfxPoolItem *>(-1)
                && **ppFnd == m_pPool->GetDefaultItem(nWhich))
            {
                m_pPool->Remove( **ppFnd );
                *ppFnd = reinterpret_cast<SfxPoolItem*>(-1);
            }
        pPtr += 2;
    }
}

void SfxItemSet::InvalidateAllItems()
{
    assert( !m_nCount && "There are still Items set" );
    m_nCount = TotalCount();
    memset(static_cast<void*>(m_pItems), -1, m_nCount * sizeof(SfxPoolItem*));
}

SfxItemState SfxItemSet::GetItemState( sal_uInt16 nWhich,
                                        bool bSrchInParent,
                                        const SfxPoolItem **ppItem ) const
{
    // Find the range in which the Which is located
    const SfxItemSet* pAktSet = this;
    SfxItemState eRet = SfxItemState::UNKNOWN;
    do
    {
        SfxItemArray ppFnd = pAktSet->m_pItems;
        const sal_uInt16* pPtr = pAktSet->m_pWhichRanges;
        if (pPtr)
        {
            while ( *pPtr )
            {
                if ( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    // Within this range
                    ppFnd += nWhich - *pPtr;
                    if ( !*ppFnd )
                    {
                        eRet = SfxItemState::DEFAULT;
                        if( !bSrchInParent )
                            return eRet; // Not present
                        break; // Keep searching in the parents!
                    }

                    if ( reinterpret_cast<SfxPoolItem*>(-1) == *ppFnd )
                        // Different ones are present
                        return SfxItemState::DONTCARE;

                    if ( dynamic_cast<const SfxVoidItem *>(*ppFnd) != nullptr )
                        return SfxItemState::DISABLED;

                    if (ppItem)
                    {
                        *ppItem = *ppFnd;
                    }
                    return SfxItemState::SET;
                }
                ppFnd += *(pPtr+1) - *pPtr + 1;
                pPtr += 2;
            }
        }
    } while (bSrchInParent && nullptr != (pAktSet = pAktSet->m_pParent));
    return eRet;
}

bool SfxItemSet::HasItem(sal_uInt16 nWhich, const SfxPoolItem** ppItem) const
{
    bool bRet = SfxItemState::SET == GetItemState(nWhich, true, ppItem);
    if (!bRet && ppItem)
        *ppItem = nullptr;
    return bRet;
}

const SfxPoolItem* SfxItemSet::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    if ( !nWhich )
        return nullptr; //FIXME: Only because of Outliner bug

    SfxItemArray ppFnd = m_pItems;
    const sal_uInt16* pPtr = m_pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // Within this range
            ppFnd += nWhich - *pPtr;
            if( *ppFnd ) // Already one present
            {
                // Same Item already present?
                if ( *ppFnd == &rItem )
                    return nullptr;

                // Will 'dontcare' or 'disabled' be overwritten with some real value?
                if ( rItem.Which() && ( IsInvalidItem(*ppFnd) || !(*ppFnd)->Which() ) )
                {
                    *ppFnd = &m_pPool->Put( rItem, nWhich );
                    return *ppFnd;
                }

                // Turns into disabled?
                if( !rItem.Which() )
                {
                    *ppFnd = rItem.Clone(m_pPool);
                    return nullptr;
                }
                else
                {
                    // Same value already present?
                    if ( rItem == **ppFnd )
                        return nullptr;

                    // Add the new one, remove the old one
                    const SfxPoolItem& rNew = m_pPool->Put( rItem, nWhich );
                    const SfxPoolItem* pOld = *ppFnd;
                    *ppFnd = &rNew;
                    if(nWhich <= SFX_WHICH_MAX)
                        Changed( *pOld, rNew );
                    m_pPool->Remove( *pOld );
                }
            }
            else
            {
                ++m_nCount;
                if( !rItem.Which() )
                    *ppFnd = rItem.Clone(m_pPool);
                else {
                    const SfxPoolItem& rNew = m_pPool->Put( rItem, nWhich );
                    *ppFnd = &rNew;
                    if (nWhich <= SFX_WHICH_MAX )
                    {
                        const SfxPoolItem& rOld = m_pParent
                            ? m_pParent->Get( nWhich )
                            : m_pPool->GetDefaultItem( nWhich );
                        Changed( rOld, rNew );
                    }
                }
            }
            SFX_ASSERT( !m_pPool->IsItemFlag(nWhich, SfxItemPoolFlags::POOLABLE) ||
                        dynamic_cast<const SfxSetItem*>( &rItem ) !=  nullptr || **ppFnd == rItem,
                        nWhich, "putted Item unequal" );
            return *ppFnd;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
    return nullptr;
}

bool SfxItemSet::Put( const SfxItemSet& rSet, bool bInvalidAsDefault )
{
    bool bRet = false;
    if( rSet.Count() )
    {
        SfxItemArray ppFnd = rSet.m_pItems;
        const sal_uInt16* pPtr = rSet.m_pWhichRanges;
        while ( *pPtr )
        {
            for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
                if( *ppFnd )
                {
                    if ( IsInvalidItem( *ppFnd ) )
                    {
                        if ( bInvalidAsDefault )
                            bRet |= 0 != ClearItem( nWhich );
                            // FIXME: Caused a SEGFAULT on non Windows-platforms:
                            // bRet |= 0 != Put( rSet.GetPool()->GetDefaultItem(nWhich), nWhich );
                        else
                            InvalidateItem( nWhich );
                    }
                    else
                        bRet |= nullptr != Put( **ppFnd, nWhich );
                }
            pPtr += 2;
        }
    }
    return bRet;
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
    const SfxItemSet&   rSet,           // Source of the Items to be put
    SfxItemState        eDontCareAs,    // What will happen to the DontCare Items
    SfxItemState        eDefaultAs      // What will happen to the Default Items
)
{
    // don't "optimize" with "if( rSet.Count()" because of dont-care + defaults
    SfxItemArray ppFnd = rSet.m_pItems;
    const sal_uInt16* pPtr = rSet.m_pWhichRanges;
    while ( *pPtr )
    {
        for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
            if( *ppFnd )
            {
                if ( IsInvalidItem( *ppFnd ) )
                {
                    // Item ist DontCare:
                    switch ( eDontCareAs )
                    {
                        case SfxItemState::SET:
                            Put( rSet.GetPool()->GetDefaultItem(nWhich), nWhich );
                            break;

                        case SfxItemState::DEFAULT:
                            ClearItem( nWhich );
                            break;

                        case SfxItemState::DONTCARE:
                            InvalidateItem( nWhich );
                            break;

                        default:
                            assert(!"invalid Argument for eDontCareAs");
                    }
                }
                else
                    // Item is set:
                    Put( **ppFnd, nWhich );
            }
            else
            {
                // Item is default:
                switch ( eDefaultAs )
                {
                    case SfxItemState::SET:
                        Put( rSet.GetPool()->GetDefaultItem(nWhich), nWhich );
                        break;

                    case SfxItemState::DEFAULT:
                        ClearItem( nWhich );
                        break;

                    case SfxItemState::DONTCARE:
                        InvalidateItem( nWhich );
                        break;

                    default:
                        assert(!"invalid Argument for eDefaultAs");
                }
            }
        pPtr += 2;
    }
}

/**
 * Expands the ranges of settable items by 'nFrom' to 'nTo'. Keeps state of
 * items which are new ranges too.
 */
void SfxItemSet::MergeRange( sal_uInt16 nFrom, sal_uInt16 nTo )
{
    // special case: exactly one sal_uInt16 which is already included?
    SfxItemState eItemState = GetItemState(nFrom, false);
    if ( nFrom == nTo && ( eItemState == SfxItemState::DEFAULT || eItemState == SfxItemState::SET ) )
        return;

    // merge new range
    SfxUShortRanges aRanges( m_pWhichRanges );
    aRanges += SfxUShortRanges( nFrom, nTo );
    SetRanges( aRanges );
}

/**
 * Modifies the ranges of settable items. Keeps state of items which
 * are new ranges too.
 */
void SfxItemSet::SetRanges( const sal_uInt16 *pNewRanges )
{
    // Identical Ranges?
    if (m_pWhichRanges == pNewRanges)
        return;
    const sal_uInt16* pOld = m_pWhichRanges;
    const sal_uInt16* pNew = pNewRanges;
    while ( *pOld == *pNew )
    {
        if ( !*pOld && !*pNew )
            return;
        ++pOld, ++pNew;
    }

    // create new item-array (by iterating through all new ranges)
    sal_uLong        nSize = Capacity_Impl(pNewRanges);
    SfxItemArray aNewItems = new const SfxPoolItem* [ nSize ];
    sal_uInt16 nNewCount = 0;
    if (m_nCount == 0)
        memset( aNewItems, 0, nSize * sizeof( SfxPoolItem* ) );
    else
    {
        sal_uInt16 n = 0;
        for ( const sal_uInt16 *pRange = pNewRanges; *pRange; pRange += 2 )
        {
            // iterate through all ids in the range
            for ( sal_uInt16 nWID = *pRange; nWID <= pRange[1]; ++nWID, ++n )
            {
                // direct move of pointer (not via pool)
                SfxItemState eState = GetItemState( nWID, false, aNewItems+n );
                if ( SfxItemState::SET == eState )
                {
                    // increment new item count and possibly increment ref count
                    ++nNewCount;
                    aNewItems[n]->AddRef();
                }
                else if ( SfxItemState::DISABLED == eState )
                {
                    // put "disabled" item
                    ++nNewCount;
                    aNewItems[n] = new SfxVoidItem(0);
                }
                else if ( SfxItemState::DONTCARE == eState )
                {
                    ++nNewCount;
                    aNewItems[n] = reinterpret_cast<SfxPoolItem*>(-1);
                }
                else
                {
                    // default
                    aNewItems[n] = nullptr;
                }
            }
        }
        // free old items
        sal_uInt16 nOldTotalCount = TotalCount();
        for ( sal_uInt16 nItem = 0; nItem < nOldTotalCount; ++nItem )
        {
            const SfxPoolItem *pItem = m_pItems[nItem];
            if ( pItem && !IsInvalidItem(pItem) && pItem->Which() )
                m_pPool->Remove(*pItem);
        }
    }

    // replace old items-array and ranges
    delete[] m_pItems;
    m_pItems = aNewItems;
    m_nCount = nNewCount;

    if( pNewRanges == GetPool()->GetFrozenIdRanges() )
    {
        delete[] m_pWhichRanges;
        m_pWhichRanges = const_cast<sal_uInt16*>(pNewRanges);
    }
    else
    {
        sal_uInt16 nCount = Count_Impl(pNewRanges) + 1;
        if (m_pWhichRanges != m_pPool->GetFrozenIdRanges())
            delete[] m_pWhichRanges;
        m_pWhichRanges = new sal_uInt16[ nCount ];
        memcpy( m_pWhichRanges, pNewRanges, sizeof( sal_uInt16 ) * nCount );
    }
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
    if (m_nCount)
        ClearItem();
    if ( bDeep )
    {
        SfxWhichIter aIter(*this);
        sal_uInt16 nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            const SfxPoolItem* pItem;
            if( SfxItemState::SET == rSet.GetItemState( nWhich, true, &pItem ) )
                bRet |= nullptr != Put( *pItem, pItem->Which() );
            nWhich = aIter.NextWhich();
        }
    }
    else
        bRet = Put(rSet, false);

    return bRet;
}

const SfxPoolItem* SfxItemSet::GetItem(sal_uInt16 nId, bool bSearchInParent) const
{
    // Convert to WhichId
    sal_uInt16 nWhich = GetPool()->GetWhich(nId);

    // Is the Item set or 'bDeep == true' available?
    const SfxPoolItem *pItem = nullptr;
    SfxItemState eState = GetItemState(nWhich, bSearchInParent, &pItem);
    if (bSearchInParent && SfxItemState::DEFAULT == eState && nWhich <= SFX_WHICH_MAX)
    {
        pItem = &m_pPool->GetDefaultItem(nWhich);
    }

    return pItem;
}

const SfxPoolItem& SfxItemSet::Get( sal_uInt16 nWhich, bool bSrchInParent) const
{
    // Search the Range in which the Which is located in:
    const SfxItemSet* pAktSet = this;
    do
    {
        if( pAktSet->Count() )
        {
            SfxItemArray ppFnd = pAktSet->m_pItems;
            const sal_uInt16* pPtr = pAktSet->m_pWhichRanges;
            while( *pPtr )
            {
                if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    // In this Range
                    ppFnd += nWhich - *pPtr;
                    if( *ppFnd )
                    {
                        if( reinterpret_cast<SfxPoolItem*>(-1) == *ppFnd ) {
                            //FIXME: The following code is duplicated further down
                            SFX_ASSERT(m_pPool, nWhich, "no Pool, but status is ambiguous");
                            //!((SfxAllItemSet *)this)->aDefault.SetWhich(nWhich);
                            //!return aDefault;
                            return m_pPool->GetDefaultItem( nWhich );
                        }
#ifdef DBG_UTIL
                        const SfxPoolItem *pItem = *ppFnd;
                        if ( dynamic_cast<const SfxVoidItem *>(pItem) != nullptr || !pItem->Which() )
                            SAL_INFO("svl.items", "SFX_WARNING: Getting disabled Item");
#endif
                        return **ppFnd;
                    }
                    break; // Continue with Parent
                }
                ppFnd += *(pPtr+1) - *pPtr + 1;
                pPtr += 2;
            }
        }
//TODO: Search until end of Range: What are we supposed to do now? To the Parent or Default??
//      if( !*pPtr )            // Until the end of the search Range?
//      break;
    } while (bSrchInParent && nullptr != (pAktSet = pAktSet->m_pParent));

    // Get the Default from the Pool and return
    SFX_ASSERT(m_pPool, nWhich, "no Pool, but status is ambiguous");
    const SfxPoolItem *pItem = &m_pPool->GetDefaultItem( nWhich );
    return *pItem;
}

/**
 * Notification callback
 */
void SfxItemSet::Changed( const SfxPoolItem&, const SfxPoolItem& )
{
}

sal_uInt16 SfxItemSet::TotalCount() const
{
    sal_uInt16 nRet = 0;
    sal_uInt16* pPtr = m_pWhichRanges;
    while( *pPtr )
    {
        nRet += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }
    return nRet;
}

/**
 * Only retain the Items that are also present in rSet
 * (nevermind their value).
 */
void SfxItemSet::Intersect( const SfxItemSet& rSet )
{
    assert(m_pPool && "Not implemented without Pool");
    if( !Count() ) // None set?
        return;

    // Delete all Items not contained in rSet
    if( !rSet.Count() )
    {
        ClearItem(); // Delete everything
        return;
    }

    // Test whether the Which Ranges are different
    sal_uInt16* pWh1 = m_pWhichRanges;
    sal_uInt16* pWh2 = rSet.m_pWhichRanges;
    sal_uInt16 nSize = 0;

    for( sal_uInt16 n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            break;
        }
        if( n & 1 )
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }
    bool bEqual = *pWh1 == *pWh2; // Also check for 0

    // If the Ranges are identical, we can easily process it
    if( bEqual )
    {
        SfxItemArray ppFnd1 = m_pItems;
        SfxItemArray ppFnd2 = rSet.m_pItems;

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            if( *ppFnd1 && !*ppFnd2 )
            {
                // Delete from Pool
                if( !IsInvalidItem( *ppFnd1 ) )
                {
                    sal_uInt16 nWhich = (*ppFnd1)->Which();
                    if(nWhich <= SFX_WHICH_MAX)
                    {
                        const SfxPoolItem& rNew = m_pParent
                            ? m_pParent->Get( nWhich )
                            : m_pPool->GetDefaultItem( nWhich );

                        Changed( **ppFnd1, rNew );
                    }
                    m_pPool->Remove( **ppFnd1 );
                }
                *ppFnd1 = nullptr;
                --m_nCount;
            }
    }
    else
    {
        SfxItemIter aIter( *this );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( true )
        {
            sal_uInt16 nWhich = IsInvalidItem( pItem )
                                ? GetWhichByPos( aIter.GetCurPos() )
                                : pItem->Which();
            if( SfxItemState::UNKNOWN == rSet.GetItemState( nWhich, false ) )
                ClearItem( nWhich );        // Delete
            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }
    }
}

void SfxItemSet::Differentiate( const SfxItemSet& rSet )
{
    if( !Count() || !rSet.Count() )// None set?
        return;

   // Test whether the Which Ranges are different
    sal_uInt16* pWh1 = m_pWhichRanges;
    sal_uInt16* pWh2 = rSet.m_pWhichRanges;
    sal_uInt16 nSize = 0;

    for( sal_uInt16 n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            break;
        }
        if( n & 1 )
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }
    bool bEqual = *pWh1 == *pWh2; // Also test for 0

    // If the Ranges are identical, we can easily process it
    if( bEqual )
    {
        SfxItemArray ppFnd1 = m_pItems;
        SfxItemArray ppFnd2 = rSet.m_pItems;

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            if( *ppFnd1 && *ppFnd2 )
            {
                // Delete from Pool
                if( !IsInvalidItem( *ppFnd1 ) )
                {
                    sal_uInt16 nWhich = (*ppFnd1)->Which();
                    if(nWhich <= SFX_WHICH_MAX)
                    {
                        const SfxPoolItem& rNew = m_pParent
                            ? m_pParent->Get( nWhich )
                            : m_pPool->GetDefaultItem( nWhich );

                        Changed( **ppFnd1, rNew );
                    }
                    m_pPool->Remove( **ppFnd1 );
                }
                *ppFnd1 = nullptr;
                --m_nCount;
            }
    }
    else
    {
        SfxItemIter aIter( *this );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( true )
        {
            sal_uInt16 nWhich = IsInvalidItem( pItem )
                                ? GetWhichByPos( aIter.GetCurPos() )
                                : pItem->Which();
            if( SfxItemState::SET == rSet.GetItemState( nWhich, false ) )
                ClearItem( nWhich ); // Delete
            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
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
static void MergeItem_Impl( SfxItemPool *_pPool, sal_uInt16 &rCount,
                            const SfxPoolItem **ppFnd1, const SfxPoolItem *pFnd2,
                            bool bIgnoreDefaults )
{
    assert(ppFnd1 != nullptr && "Merging to 0-Item");

    // 1st Item is Default?
    if ( !*ppFnd1 )
    {
        if ( IsInvalidItem(pFnd2) )
            // Decision table: default, dontcare, doesn't matter, doesn't matter
            *ppFnd1 = reinterpret_cast<SfxPoolItem*>(-1);

        else if ( pFnd2 && !bIgnoreDefaults &&
                  _pPool->GetDefaultItem(pFnd2->Which()) != *pFnd2 )
            // Decision table: default, set, !=, sal_False
            *ppFnd1 = reinterpret_cast<SfxPoolItem*>(-1);

        else if ( pFnd2 && bIgnoreDefaults )
            // Decision table: default, set, doesn't matter, sal_True
            *ppFnd1 = &_pPool->Put( *pFnd2 );

        if ( *ppFnd1 )
            ++rCount;
    }

    // 1st Item set?
    else if ( !IsInvalidItem(*ppFnd1) )
    {
        if ( !pFnd2 )
        {
            // 2nd Item is Default
            if ( !bIgnoreDefaults &&
                 **ppFnd1 != _pPool->GetDefaultItem((*ppFnd1)->Which()) )
            {
                // Decision table: set, default, !=, sal_False
                _pPool->Remove( **ppFnd1 );
                *ppFnd1 = reinterpret_cast<SfxPoolItem*>(-1);
            }
        }
        else if ( IsInvalidItem(pFnd2) )
        {
            // 2nd Item is dontcare
            if ( !bIgnoreDefaults ||
                 **ppFnd1 != _pPool->GetDefaultItem( (*ppFnd1)->Which()) )
            {
                // Decision table: set, dontcare, doesn't matter, sal_False
                // or:             set, dontcare, !=, sal_True
                _pPool->Remove( **ppFnd1 );
                *ppFnd1 = reinterpret_cast<SfxPoolItem*>(-1);
            }
        }
        else
        {
            // 2nd Item is set
            if ( **ppFnd1 != *pFnd2 )
            {
                // Decision table: set, set, !=, doesn't matter
                _pPool->Remove( **ppFnd1 );
                *ppFnd1 = reinterpret_cast<SfxPoolItem*>(-1);
            }
        }
    }
}

void SfxItemSet::MergeValues( const SfxItemSet& rSet, bool bIgnoreDefaults )
{
    // WARNING! When making changes/fixing bugs, always update the table above!!
    assert( GetPool() == rSet.GetPool() && "MergeValues with different Pools" );

    // Test if the which Ranges are different
    sal_uInt16* pWh1 = m_pWhichRanges;
    sal_uInt16* pWh2 = rSet.m_pWhichRanges;
    sal_uInt16 nSize = 0;

    for( sal_uInt16 n = 0; *pWh1 && *pWh2; ++pWh1, ++pWh2, ++n )
    {
        if( *pWh1 != *pWh2 )
        {
            break;
        }
        if( n & 1 )
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }
    bool bEqual = *pWh1 == *pWh2; // Also check for 0

    // If the Ranges match, they are easier to process!
    if( bEqual )
    {
        SfxItemArray ppFnd1 = m_pItems;
        SfxItemArray ppFnd2 = rSet.m_pItems;

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            MergeItem_Impl(m_pPool, m_nCount, ppFnd1, *ppFnd2, bIgnoreDefaults);
    }
    else
    {
        SfxWhichIter aIter( rSet );
        sal_uInt16 nWhich;
        while( 0 != ( nWhich = aIter.NextWhich() ) )
        {
            const SfxPoolItem* pItem = nullptr;
            rSet.GetItemState( nWhich, true, &pItem );
            if( !pItem )
            {
                // Not set, so default
                if ( !bIgnoreDefaults )
                    MergeValue( rSet.GetPool()->GetDefaultItem( nWhich ), bIgnoreDefaults );
            }
            else if( IsInvalidItem( pItem ) )
                // don't care
                InvalidateItem( nWhich );
            else
                MergeValue( *pItem, bIgnoreDefaults );
        }
    }
}

void SfxItemSet::MergeValue( const SfxPoolItem& rAttr, bool bIgnoreDefaults )
{
    SfxItemArray ppFnd = m_pItems;
    const sal_uInt16* pPtr = m_pWhichRanges;
    const sal_uInt16 nWhich = rAttr.Which();
    while( *pPtr )
    {
        // In this Range??
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            ppFnd += nWhich - *pPtr;
            MergeItem_Impl(m_pPool, m_nCount, ppFnd, &rAttr, bIgnoreDefaults);
            break;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
}

void SfxItemSet::InvalidateItem( sal_uInt16 nWhich )
{
    SfxItemArray ppFnd = m_pItems;
    const sal_uInt16* pPtr = m_pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // In this Range?
            ppFnd += nWhich - *pPtr;

            if( *ppFnd ) // Set for me
            {
                if( reinterpret_cast<SfxPoolItem*>(-1) != *ppFnd ) // Not yet dontcare!
                {
                    m_pPool->Remove( **ppFnd );
                    *ppFnd = reinterpret_cast<SfxPoolItem*>(-1);
                }
            }
            else
            {
                *ppFnd = reinterpret_cast<SfxPoolItem*>(-1);
                ++m_nCount;
            }
            break;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
}

sal_uInt16 SfxItemSet::GetWhichByPos( sal_uInt16 nPos ) const
{
    sal_uInt16 n = 0;
    sal_uInt16* pPtr = m_pWhichRanges;
    while( *pPtr )
    {
        n = ( *(pPtr+1) - *pPtr ) + 1;
        if( nPos < n )
            return *(pPtr)+nPos;
        nPos = nPos - n;
        pPtr += 2;
    }
    assert(false);
    return 0;
}

/**
 * Saves the SfxItemSet instance to the supplied Stream.
 * The surrogates as well as the ones with 'bDirect == true' are saved
 * to the stream in the following way:
 *
 *  sal_uInt16  ... Count of the set Items
 *  Count*  m_pPool->StoreItem()
 *
 *  @see SfxItemPool::StoreItem() const
 *  @see SfxItemSet::Load(SvStream&,bool,const SfxItemPool*)
 */
SvStream &SfxItemSet::Store
(
    SvStream&   rStream,        // Target stream for normal Items
    bool        bDirect         /* true: Save Items directly
                                   false: Surrogates */
)   const
{
    assert(m_pPool);

    // Remember position of the count (to be able to correct it, if need be)
    sal_uLong nCountPos = rStream.Tell();
    rStream.WriteUInt16( m_nCount );

    // If there's nothing to save, don't construct an ItemIter
    if (m_nCount)
    {
        // Keep record of how many Items are really saved
        sal_uInt16 nWrittenCount = 0; // Count of Items streamed in 'rStream'

        // Iterate over all set Items
        SfxItemIter aIter(*this);
        for ( const SfxPoolItem *pItem = aIter.FirstItem();
              pItem;
              pItem = aIter.NextItem() )
        {
            // Let Items (if need be as a Surrogate) be saved via Pool
            SAL_WARN_IF(IsInvalidItem(pItem), "svl.items", "can't store invalid items");
            if ( !IsInvalidItem(pItem) &&
                 m_pPool->StoreItem( rStream, *pItem, bDirect ) )
                // Item was streamed in 'rStream'
                ++nWrittenCount;
        }

        // Fewer written than read (e.g. old format)
        if (nWrittenCount != m_nCount)
        {
            // Store real count in the stream
            sal_uLong nPos = rStream.Tell();
            rStream.Seek( nCountPos );
            rStream.WriteUInt16( nWrittenCount );
            rStream.Seek( nPos );
        }
    }

    return rStream;
}

/**
 * This method loads an SfxItemSet from a stream.
 * If the SfxItemPool was loaded without RefCounts the loaded Item
 * references are counted, else we assume the they were accounted for
 * when loading the SfxItemPool.
 *
 * @see SfxItemSet::Store(Stream&,bool) const
 */
SvStream &SfxItemSet::Load
(
    SvStream&           rStream,    //  Stream we're loading from

    bool                bDirect,    /*  true
                                        Items are directly read form the stream
                                        and not via Surrogates

                                        false (default)
                                        Items are read via Surrogates */

    const SfxItemPool*  pRefPool    /*  Pool that can resolve the Surrogates
                                        (e.g. when inserting documents) */
)
{
    assert(m_pPool);

    // No RefPool => Resolve Surrogates with ItemSet's Pool
    if ( !pRefPool )
        pRefPool = m_pPool;

    // Load Item count and as many Items
    sal_uInt16 nCount = 0;
    rStream.ReadUInt16( nCount );

    const size_t nMinRecordSize = sizeof(sal_uInt16) * 2;
    const size_t nMaxRecords = rStream.remainingSize() / nMinRecordSize;
    if (nCount > nMaxRecords)
    {
        SAL_WARN("svl", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nCount << " claimed, truncating");
        nCount = nMaxRecords;
    }

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        // Load Surrogate/Item and resolve Surrogate
        const SfxPoolItem *pItem =
                m_pPool->LoadItem( rStream, bDirect, pRefPool );

        // Did we load an Item or resolve a Surrogate?
        if ( pItem )
        {
            // Find position for Item pointer in the set
            sal_uInt16 nWhich = pItem->Which();
            SfxItemArray ppFnd = m_pItems;
            const sal_uInt16* pPtr = m_pWhichRanges;
            while ( *pPtr )
            {
                // In this Range??
                if ( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    // Remember Item pointer in the set
                    ppFnd += nWhich - *pPtr;
                    SFX_ASSERT( !*ppFnd, nWhich, "Item is present twice");
                    *ppFnd = pItem;
                    ++m_nCount;
                    break;
                }

                // In the range array and Item array to the next Which range
                ppFnd += *(pPtr+1) - *pPtr + 1;
                pPtr += 2;
            }
        }
    }

    return rStream;
}

bool SfxItemSet::operator==(const SfxItemSet &rCmp) const
{
    // Values we can get quickly need to be the same
    if ( m_pParent != rCmp.m_pParent ||
         m_pPool != rCmp.m_pPool ||
         Count() != rCmp.Count() )
        return false;

    // Counting Ranges takes longer; they also need to be the same, however
    sal_uInt16 nCount1 = TotalCount();
    sal_uInt16 nCount2 = rCmp.TotalCount();
    if ( nCount1 != nCount2 )
        return false;

    // Are the Ranges themselves unequal?
    for (sal_uInt16 nRange = 0; m_pWhichRanges[nRange]; nRange += 2)
    {
        if (m_pWhichRanges[nRange] != rCmp.m_pWhichRanges[nRange] ||
            m_pWhichRanges[nRange+1] != rCmp.m_pWhichRanges[nRange+1])
        {
            // We must use the slow method then
            SfxWhichIter aIter( *this );
            for ( sal_uInt16 nWh = aIter.FirstWhich();
                  nWh;
                  nWh = aIter.NextWhich() )
            {
                // If the pointer of the poolable Items are unequal, the Items must match
                const SfxPoolItem *pItem1 = nullptr, *pItem2 = nullptr;
                if ( GetItemState( nWh, false, &pItem1 ) !=
                        rCmp.GetItemState( nWh, false, &pItem2 ) ||
                     ( pItem1 != pItem2 &&
                        ( !pItem1 || IsInvalidItem(pItem1) ||
                          (m_pPool->IsItemFlag(*pItem1, SfxItemPoolFlags::POOLABLE) &&
                            *pItem1 != *pItem2 ) ) ) )
                    return false;
            }

            return true;
        }
    }

    // Are all pointers the same?
    if (0 == memcmp( m_pItems, rCmp.m_pItems, nCount1 * sizeof(m_pItems[0]) ))
        return true;

    // We need to compare each one separately then
    const SfxPoolItem **ppItem1 = m_pItems;
    const SfxPoolItem **ppItem2 = rCmp.m_pItems;
    for ( sal_uInt16 nPos = 0; nPos < nCount1; ++nPos )
    {
        // If the pointers of the poolable Items are not the same, the Items
        // must match
        if ( *ppItem1 != *ppItem2 &&
             ( ( !*ppItem1 || !*ppItem2 ) ||
               ( IsInvalidItem(*ppItem1) || IsInvalidItem(*ppItem2) ) ||
               (m_pPool->IsItemFlag(**ppItem1, SfxItemPoolFlags::POOLABLE)) ||
                 **ppItem1 != **ppItem2 ) )
            return false;

        ++ppItem1;
        ++ppItem2;
    }

    return true;
}

SfxItemSet *SfxItemSet::Clone(bool bItems, SfxItemPool *pToPool ) const
{
    if (pToPool && pToPool != m_pPool)
    {
        SfxItemSet *pNewSet = new SfxItemSet(*pToPool, m_pWhichRanges);
        if ( bItems )
        {
            SfxWhichIter aIter(*pNewSet);
            sal_uInt16 nWhich = aIter.FirstWhich();
            while ( nWhich )
            {
                const SfxPoolItem* pItem;
                if ( SfxItemState::SET == GetItemState( nWhich, false, &pItem ) )
                    pNewSet->Put( *pItem, pItem->Which() );
                nWhich = aIter.NextWhich();
            }
        }
        return pNewSet;
    }
    else
        return bItems
                ? new SfxItemSet(*this)
                : new SfxItemSet(*m_pPool, m_pWhichRanges);
}

void SfxItemSet::PutDirect(const SfxPoolItem &rItem)
{
    SfxItemArray ppFnd = m_pItems;
    const sal_uInt16* pPtr = m_pWhichRanges;
    const sal_uInt16 nWhich = rItem.Which();
#ifdef DBG_UTIL
    IsPoolDefaultItem(&rItem) || m_pPool->GetSurrogate(&rItem);
        // Only cause assertion in the callees
#endif
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // In this Range?
            ppFnd += nWhich - *pPtr;
            const SfxPoolItem* pOld = *ppFnd;
            if( pOld ) // One already present
            {
                if( rItem == **ppFnd )
                    return; // Already present!
                m_pPool->Remove( *pOld );
            }
            else
                ++m_nCount;

            // Add the new one
            if( IsPoolDefaultItem(&rItem) )
                *ppFnd = &m_pPool->Put( rItem );
            else
            {
                *ppFnd = &rItem;
                if( !IsStaticDefaultItem( &rItem ) )
                    rItem.AddRef();
            }

            return;
        }
        ppFnd += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
}

sal_Int32 SfxItemSet::getHash() const
{
    return stringify().hashCode();
}

OString SfxItemSet::stringify() const
{
    SvMemoryStream aStream;
    Store(aStream, true);
    aStream.Flush();
    return OString(
        static_cast<char const *>(aStream.GetData()), aStream.GetEndOfData());
}

void SfxItemSet::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("sfxItemSet"));
    SfxItemIter aIter(*this);
    for (const SfxPoolItem* pItem = aIter.FirstItem(); pItem; pItem = aIter.NextItem())
         pItem->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}


// ----------------------------------------------- class SfxAllItemSet

SfxAllItemSet::SfxAllItemSet( SfxItemPool &rPool )
:   SfxItemSet(rPool, nullptr),
    nFree(nInitCount)
{
    // Initially no Items
    m_pItems = nullptr;

    // Allocate nInitCount pairs at USHORTs for Ranges
    m_pWhichRanges = new sal_uInt16[ nInitCount + 1 ];
    memset( m_pWhichRanges, 0, (nInitCount + 1) * sizeof(sal_uInt16) );
}

SfxAllItemSet::SfxAllItemSet(const SfxItemSet &rCopy)
:   SfxItemSet(rCopy),
    nFree(0)
{
}

/**
 * Explicitly define this ctor to avoid auto-generation by the compiler.
 * The compiler does not take the ctor with the 'const SfxItemSet&'!
 */
SfxAllItemSet::SfxAllItemSet(const SfxAllItemSet &rCopy)
:   SfxItemSet(rCopy),
    nFree(0)
{
}

/**
 * This internal function creates a new WhichRanges array, which is copied
 * from the 'nOldSize'-USHORTs long 'pUS'. It has new USHORTs at the end instead
 * of 'nIncr'.
 * The terminating sal_uInt16 with the '0' is neither accounted for in 'nOldSize'
 * nor in 'nIncr', but always explicitly added.
 *
 * @returns the new WhichRanges array (the old 'pUS' is freed)
 */
static sal_uInt16 *AddRanges_Impl(
    sal_uInt16 *pUS, std::ptrdiff_t nOldSize, sal_uInt16 nIncr)
{
    // Create new WhichRanges array
    sal_uInt16 *pNew = new sal_uInt16[ nOldSize + nIncr + 1 ];

    // Take over the old Ranges
    memcpy( pNew, pUS, nOldSize * sizeof(sal_uInt16) );

    // Initialize the new one to 0
    memset( pNew + nOldSize, 0, ( nIncr + 1 ) * sizeof(sal_uInt16) );

    // Free the old array
    delete[] pUS;

    return pNew;
}

/**
 * This internal function creates a new ItemArray, which is copied from 'pItems',
 * but has room for a new ItemPointer at 'nPos'.
 *
 * @returns the new ItemArray (the old 'pItems' is freed)
 */
static SfxItemArray AddItem_Impl(SfxItemArray pItems, sal_uInt16 nOldSize, sal_uInt16 nPos)
{
    // Create new ItemArray
    SfxItemArray pNew = new const SfxPoolItem*[nOldSize+1];

    // Was there one before?
    if ( pItems )
    {
        // Copy all Items before nPos
        if ( nPos )
            memcpy( static_cast<void*>(pNew), pItems, nPos * sizeof(SfxPoolItem *) );

        // Copy all Items after nPos
        if ( nPos < nOldSize )
            memcpy( static_cast<void*>(pNew + nPos + 1), pItems + nPos,
                    (nOldSize-nPos) * sizeof(SfxPoolItem *) );
    }

    // Initialize new Item
    *(pNew + nPos) = nullptr;

    // Free old ItemArray
    delete[] pItems;

    return pNew;
}

/**
 * Putting with automatic extension of the WhichId with the ID of the Item.
 */
const SfxPoolItem* SfxAllItemSet::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    sal_uInt16 nPos = 0; // Position for 'rItem' in 'm_pItems'
    const sal_uInt16 nItemCount = TotalCount();

    // Let's see first whether there's a suitable Range already
    sal_uInt16 *pPtr = m_pWhichRanges;
    while ( *pPtr )
    {
        // WhichId is within this Range?
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // Insert
            nPos += nWhich - *pPtr;
            break;
        }

        // Carry over the position of the Item in m_pItems
        nPos += *(pPtr+1) - *pPtr + 1;

        // To the next Range
        pPtr += 2;
    }

    // WhichId not yet present?
    if ( !*pPtr )
    {
        // Let's see if we can attach it somewhere
        pPtr = m_pWhichRanges;
        nPos = 0;
        while ( *pPtr )
        {
            // WhichId is right before this Range?
            if ( (nWhich+1) == *pPtr )
            {
                // Range grows downwards
                (*pPtr)--;

                // Make room before first Item of this Range
                m_pItems = AddItem_Impl(m_pItems, nItemCount, nPos);
                break;
            }

            // WhichId is right after this Range?
            else if ( (nWhich-1) == *(pPtr+1) )
            {
                // Range grows upwards?
                (*(pPtr+1))++;

                // Make room after last Item of this Range
                nPos += nWhich - *pPtr;
                m_pItems = AddItem_Impl(m_pItems, nItemCount, nPos);
                break;
            }

            // Carry over position of the Item in m_pItems
            nPos += *(pPtr+1) - *pPtr + 1;

            // To the next Range
            pPtr += 2;
        }
    }

    // No extensible Range found?
    if ( !*pPtr )
    {
        // No room left in m_pWhichRanges? => Expand!
        std::ptrdiff_t nSize = pPtr - m_pWhichRanges;
        if( !nFree )
        {
            m_pWhichRanges = AddRanges_Impl(m_pWhichRanges, nSize, nInitCount);
            nFree += nInitCount;
        }

        // Attach new WhichRange
        pPtr = m_pWhichRanges + nSize;
        *pPtr++ = nWhich;
        *pPtr = nWhich;
        nFree -= 2;

        // Expand ItemArray
        nPos = nItemCount;
        m_pItems = AddItem_Impl(m_pItems, nItemCount, nPos);
    }

    // Add new Item to Pool
    const SfxPoolItem& rNew = m_pPool->Put( rItem, nWhich );

    // Remember old Item
    bool bIncrementCount = false;
    const SfxPoolItem* pOld = *( m_pItems + nPos );
    if ( reinterpret_cast< SfxPoolItem* >( -1 ) == pOld ) // state "dontcare"
        pOld = nullptr;
    if ( !pOld )
    {
        bIncrementCount = true;
        pOld = (m_pParent)
            ? &m_pParent->Get( nWhich )
            : ((nWhich <= SFX_WHICH_MAX)
                    ? &m_pPool->GetDefaultItem(nWhich)
                    : nullptr);
    }

    // Add new Item to ItemSet
    *(m_pItems + nPos) = &rNew;

    // Send Changed Notification
    if ( pOld )
    {
        Changed( *pOld, rNew );
        if ( !IsDefaultItem(pOld) )
            m_pPool->Remove( *pOld );
    }

    if ( bIncrementCount )
        ++m_nCount;

    return &rNew;
}

/**
 * Disable Item
 * Using a VoidItem with Which value 0
 */
void SfxItemSet::DisableItem(sal_uInt16 nWhich)
{
    Put( SfxVoidItem(0), nWhich );
}

SfxItemSet *SfxAllItemSet::Clone(bool bItems, SfxItemPool *pToPool ) const
{
    if (pToPool && pToPool != m_pPool)
    {
        SfxAllItemSet *pNewSet = new SfxAllItemSet( *pToPool );
        if ( bItems )
            pNewSet->Set( *this );
        return pNewSet;
    }
    else
        return bItems ? new SfxAllItemSet(*this) : new SfxAllItemSet(*m_pPool);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
