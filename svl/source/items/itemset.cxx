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
#include <svl/whiter.hxx>

#include <tools/stream.hxx>
#include <tools/solar.h>
#include <rtl/string.hxx>

#include <poolio.hxx>

static const sal_uInt16 nInitCount = 10; // Single USHORTs => 5 pairs without '0'

namespace
{

/**
 * Determines the number of sal_uInt16s in a 0-terminated array of pairs of
 * sal_uInt16s.
 * The terminating 0 is not included in the count.
 */
sal_uInt16 Count_Impl( const sal_uInt16 *pRanges )
{
    sal_uInt16 nCount = 0;
    while ( *pRanges )
    {
        nCount += 2;
        pRanges += 2;
    }
    return nCount;
}

/**
 * Determines the total number of sal_uInt16s described in a 0-terminated
 * array of pairs of sal_uInt16s, each representing an range of sal_uInt16s.
 */
sal_uInt16 Capacity_Impl( const sal_uInt16 *pRanges )
{
    sal_uInt16 nCount = 0;

    if ( pRanges )
    {
        while ( *pRanges )
        {
            nCount += pRanges[1] - pRanges[0] + 1;
            pRanges += 2;
        }
    }
    return nCount;
}

}

/**
 * Ctor for a SfxItemSet with exactly the Which Ranges, which are known to
 * the supplied SfxItemPool.
 *
 * For Sfx programmers: an SfxItemSet constructed in this way cannot
 * contain any Items with SlotIds as Which values.
 */
SfxItemSet::SfxItemSet(SfxItemPool& rPool)
    : m_pPool( &rPool )
    , m_pParent(nullptr)
    , m_nCount(0)
{
    m_pWhichRanges = const_cast<sal_uInt16*>(m_pPool->GetFrozenIdRanges());
    assert( m_pWhichRanges && "don't create ItemSets with full range before FreezeIdRanges()" );
    if (!m_pWhichRanges)
    {
        std::unique_ptr<sal_uInt16[]> tmp;
        m_pPool->FillItemIdRanges_Impl(tmp);
        m_pWhichRanges = tmp.release();
    }

    const sal_uInt16 nSize = TotalCount();
    m_pItems.reset(new const SfxPoolItem*[nSize]{});
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

    m_pItems.reset( new const SfxPoolItem*[nCnt]{} );

    std::ptrdiff_t cnt = pPtr - pWhichPairTable +1;
    m_pWhichRanges = new sal_uInt16[ cnt ];
    memcpy( m_pWhichRanges, pWhichPairTable, sizeof( sal_uInt16 ) * cnt );
}

SfxItemSet::SfxItemSet(
    SfxItemPool & pool, std::initializer_list<sal_uInt16> wids,
    std::size_t items):
    m_pPool(&pool), m_pParent(nullptr),
    m_pItems(new SfxPoolItem const *[items]{}),
    m_pWhichRanges(new sal_uInt16[wids.size() + 1]),
        // cannot overflow, assuming std::size_t is no smaller than sal_uInt16,
        // as wids.size() must be substantially smaller than
        // std::numeric_limits<sal_uInt16>::max() by construction in
        // SfxItemSet::create
    m_nCount(0)
{
    assert(wids.size() != 0);
    assert(wids.size() % 2 == 0);
    std::copy(wids.begin(), wids.end(), m_pWhichRanges);
    m_pWhichRanges[wids.size()] = 0;
}

SfxItemSet::SfxItemSet(
    SfxItemPool & pool, std::initializer_list<Pair> wids):
    m_pPool(&pool), m_pParent(nullptr),
    m_pWhichRanges(new sal_uInt16[2 * wids.size() + 1]), //TODO: overflow
    m_nCount(0)
{
    assert(wids.size() != 0);
    std::size_t i = 0;
    std::size_t size = 0;
#if !defined NDEBUG
    //TODO: sal_uInt16 prev = 0;
#endif
    for (auto const & p: wids) {
        assert(svl::detail::validRange(p.wid1, p.wid2));
        //TODO: assert(prev == 0 || svl::detail::validGap(prev, p.wid1));
        m_pWhichRanges[i++] = p.wid1;
        m_pWhichRanges[i++] = p.wid2;
        size += svl::detail::rangeSize(p.wid1, p.wid2);
            // cannot overflow, assuming std::size_t is no smaller than
            // sal_uInt16
#if !defined NDEBUG
        //TODO: prev = p.wid2;
#endif
    }
    m_pWhichRanges[i] = 0;
    m_pItems.reset( new SfxPoolItem const *[size]{} );
}

SfxItemSet::SfxItemSet( SfxItemPool& rPool, const sal_uInt16* pWhichPairTable )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
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

    m_pItems.reset( new const SfxPoolItem* [ nCnt ] );

    // Copy attributes
    SfxPoolItem const** ppDst = m_pItems.get();
    SfxPoolItem const** ppSrc = rASet.m_pItems.get();
    for( sal_uInt16 n = nCnt; n; --n, ++ppDst, ++ppSrc )
        if ( nullptr == *ppSrc ||                 // Current Default?
             IsInvalidItem(*ppSrc) ||       // DontCare?
             IsStaticDefaultItem(*ppSrc) )  // Defaults that are not to be pooled?
            // Just copy the pointer
            *ppDst = *ppSrc;
        else if (m_pPool->IsItemPoolable( **ppSrc ))
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
        SfxPoolItem const** ppFnd = m_pItems.get();
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

    m_pItems.reset();
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
    SfxPoolItem const** ppFnd = m_pItems.get();

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
                        if (SfxItemPool::IsWhich(nWhich))
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
                        if (SfxItemPool::IsWhich(nWhich))
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

void SfxItemSet::ClearInvalidItems()
{
    sal_uInt16* pPtr = m_pWhichRanges;
    SfxPoolItem const** ppFnd = m_pItems.get();
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

void SfxItemSet::InvalidateAllItems()
{
    assert( !m_nCount && "There are still Items set" );
    m_nCount = TotalCount();
    memset(static_cast<void*>(m_pItems.get()), -1, m_nCount * sizeof(SfxPoolItem*));
}

SfxItemState SfxItemSet::GetItemState( sal_uInt16 nWhich,
                                        bool bSrchInParent,
                                        const SfxPoolItem **ppItem ) const
{
    // Find the range in which the Which is located
    const SfxItemSet* pCurrentSet = this;
    SfxItemState eRet = SfxItemState::UNKNOWN;
    do
    {
        SfxPoolItem const** ppFnd = pCurrentSet->m_pItems.get();
        const sal_uInt16* pPtr = pCurrentSet->m_pWhichRanges;
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

                    if ( IsInvalidItem(*ppFnd) )
                        // Different ones are present
                        return SfxItemState::DONTCARE;

                    if ( (*ppFnd)->IsVoidItem() )
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
    } while (bSrchInParent && nullptr != (pCurrentSet = pCurrentSet->m_pParent));
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

    SfxPoolItem const** ppFnd = m_pItems.get();
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
                    auto const old = *ppFnd;
                    *ppFnd = &m_pPool->Put( rItem, nWhich );
                    if (!IsInvalidItem(old)) {
                        assert(old->Which() == 0);
                        delete old;
                    }
                    return *ppFnd;
                }

                // Turns into disabled?
                if( !rItem.Which() )
                {
                    if (IsInvalidItem(*ppFnd) || (*ppFnd)->Which() != 0) {
                        *ppFnd = rItem.Clone(m_pPool);
                    }
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
                    if (SfxItemPool::IsWhich(nWhich))
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
                    if (SfxItemPool::IsWhich(nWhich))
                    {
                        const SfxPoolItem& rOld = m_pParent
                            ? m_pParent->Get( nWhich )
                            : m_pPool->GetDefaultItem( nWhich );
                        Changed( rOld, rNew );
                    }
                }
            }
            SAL_WARN_IF(m_pPool->IsItemPoolable(nWhich) &&
                        dynamic_cast<const SfxSetItem*>( &rItem ) == nullptr &&
                        **ppFnd != rItem,
                        "svl.items", "putted Item unequal, with ID/pos " << nWhich );
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
        SfxPoolItem const** ppFnd = rSet.m_pItems.get();
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
    SfxPoolItem const** ppFnd = rSet.m_pItems.get();
    const sal_uInt16* pPtr = rSet.m_pWhichRanges;
    while ( *pPtr )
    {
        for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich, ++ppFnd )
            if( *ppFnd )
            {
                if ( IsInvalidItem( *ppFnd ) )
                {
                    // Item is DontCare:
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

#ifdef DBG_UTIL
    assert(nFrom <= nTo);
    for (const sal_uInt16 *pRange = m_pWhichRanges; *pRange; pRange += 2)
    {
        assert(pRange[0] <= pRange[1]);
        // ranges must be sorted and discrete
        assert(
            !pRange[2] || (pRange[2] > pRange[1] && pRange[2] - pRange[1] > 1));
    }
#endif

    // create vector of ranges (sal_uInt16 pairs of lower and upper bound)
    const size_t nOldCount = Count_Impl(m_pWhichRanges);
    std::vector<std::pair<sal_uInt16, sal_uInt16>> aRangesTable;
    aRangesTable.reserve(nOldCount/2 + 1);
    bool bAdded = false;
    for (size_t i = 0; i < nOldCount; i += 2)
    {
        if (!bAdded && m_pWhichRanges[i] >= nFrom)
        {   // insert new range, keep ranges sorted
            aRangesTable.emplace_back(std::pair<sal_uInt16, sal_uInt16>(nFrom, nTo));
            bAdded = true;
        }
        // insert current range
        aRangesTable.emplace_back(std::pair<sal_uInt16, sal_uInt16>(m_pWhichRanges[i], m_pWhichRanges[i+1]));
    }
    if (!bAdded)
        aRangesTable.emplace_back(std::pair<sal_uInt16, sal_uInt16>(nFrom, nTo));

    // true if ranges overlap or adjoin, false if ranges are separate
    auto needMerge = [](std::pair<sal_uInt16, sal_uInt16> lhs, std::pair<sal_uInt16, sal_uInt16> rhs)
                     {return (lhs.first-1) <= rhs.second && (rhs.first-1) <= lhs.second;};

    std::vector<std::pair<sal_uInt16, sal_uInt16> >::iterator it = aRangesTable.begin();
    std::vector<std::pair<sal_uInt16, sal_uInt16> >::iterator itNext;
    // we got at least one range
    while ((itNext = std::next(it)) != aRangesTable.end())
    {
        // check neighbouring ranges, find first range which overlaps or adjoins a previous range
        if (needMerge(*it, *itNext))
        {
            // lower bounds are sorted, implies: it->first = min(it[0].first, it[1].first)
            it->second = std::max(it->second, itNext->second);
            aRangesTable.erase(itNext);
        }
        else
            ++it;
    }

    // construct range array
    const size_t nNewSize = 2 * aRangesTable.size() + 1;
    std::vector<sal_uInt16> aRanges(nNewSize);
    for (size_t i = 0; i < (nNewSize - 1); i +=2)
        std::tie(aRanges[i], aRanges[i+1]) = aRangesTable[i/2];

    // null terminate to be compatible with sal_uInt16* array pointers
    aRanges.back() = 0;

    SetRanges( aRanges.data() );
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
        ++pOld;
        ++pNew;
    }

    // create new item-array (by iterating through all new ranges)
    sal_uInt16   nSize = Capacity_Impl(pNewRanges);
    SfxPoolItem const** aNewItems = new const SfxPoolItem* [ nSize ];
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
                    aNewItems[n] = INVALID_POOL_ITEM;
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
    m_pItems.reset( aNewItems );
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
    if (bSearchInParent && SfxItemState::DEFAULT == eState && SfxItemPool::IsWhich(nWhich))
    {
        pItem = &m_pPool->GetDefaultItem(nWhich);
    }

    return pItem;
}

const SfxPoolItem& SfxItemSet::Get( sal_uInt16 nWhich, bool bSrchInParent) const
{
    // Search the Range in which the Which is located in:
    const SfxItemSet* pCurrentSet = this;
    do
    {
        if( pCurrentSet->Count() )
        {
            SfxPoolItem const** ppFnd = pCurrentSet->m_pItems.get();
            const sal_uInt16* pPtr = pCurrentSet->m_pWhichRanges;
            while( *pPtr )
            {
                if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    // In this Range
                    ppFnd += nWhich - *pPtr;
                    if( *ppFnd )
                    {
                        if( IsInvalidItem(*ppFnd) ) {
                            //FIXME: The following code is duplicated further down
                            SAL_WARN_IF(!m_pPool, "svl.items", "no Pool, but status is ambiguous, with ID/pos " << nWhich);
                            //!((SfxAllItemSet *)this)->aDefault.SetWhich(nWhich);
                            //!return aDefault;
                            return m_pPool->GetDefaultItem( nWhich );
                        }
#ifdef DBG_UTIL
                        const SfxPoolItem *pItem = *ppFnd;
                        if ( pItem->IsVoidItem() || !pItem->Which() )
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
    } while (bSrchInParent && nullptr != (pCurrentSet = pCurrentSet->m_pParent));

    // Get the Default from the Pool and return
    SAL_WARN_IF(!m_pPool, "svl.items", "no Pool, but status is ambiguous, with ID/pos " << nWhich);
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
            nSize += ( *pWh1 - *(pWh1-1) ) + 1;
    }
    bool bEqual = *pWh1 == *pWh2; // Also check for 0

    // If the Ranges are identical, we can easily process it
    if( bEqual )
    {
        SfxPoolItem const** ppFnd1 = m_pItems.get();
        SfxPoolItem const** ppFnd2 = rSet.m_pItems.get();

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            if( *ppFnd1 && !*ppFnd2 )
            {
                // Delete from Pool
                if( !IsInvalidItem( *ppFnd1 ) )
                {
                    sal_uInt16 nWhich = (*ppFnd1)->Which();
                    if (SfxItemPool::IsWhich(nWhich))
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
            nSize += ( *pWh1 - *(pWh1-1) ) + 1;
    }
    bool bEqual = *pWh1 == *pWh2; // Also test for 0

    // If the Ranges are identical, we can easily process it
    if( bEqual )
    {
        SfxPoolItem const** ppFnd1 = m_pItems.get();
        SfxPoolItem const** ppFnd2 = rSet.m_pItems.get();

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            if( *ppFnd1 && *ppFnd2 )
            {
                // Delete from Pool
                if( !IsInvalidItem( *ppFnd1 ) )
                {
                    sal_uInt16 nWhich = (*ppFnd1)->Which();
                    if (SfxItemPool::IsWhich(nWhich))
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
            *ppFnd1 = INVALID_POOL_ITEM;

        else if ( pFnd2 && !bIgnoreDefaults &&
                  _pPool->GetDefaultItem(pFnd2->Which()) != *pFnd2 )
            // Decision table: default, set, !=, sal_False
            *ppFnd1 = INVALID_POOL_ITEM;

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
                *ppFnd1 = INVALID_POOL_ITEM;
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
                *ppFnd1 = INVALID_POOL_ITEM;
            }
        }
        else
        {
            // 2nd Item is set
            if ( **ppFnd1 != *pFnd2 )
            {
                // Decision table: set, set, !=, doesn't matter
                _pPool->Remove( **ppFnd1 );
                *ppFnd1 = INVALID_POOL_ITEM;
            }
        }
    }
}

void SfxItemSet::MergeValues( const SfxItemSet& rSet )
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
            nSize += ( *pWh1 - *(pWh1-1) ) + 1;
    }
    bool bEqual = *pWh1 == *pWh2; // Also check for 0

    // If the Ranges match, they are easier to process!
    if( bEqual )
    {
        SfxPoolItem const** ppFnd1 = m_pItems.get();
        SfxPoolItem const** ppFnd2 = rSet.m_pItems.get();

        for( ; nSize; --nSize, ++ppFnd1, ++ppFnd2 )
            MergeItem_Impl(m_pPool, m_nCount, ppFnd1, *ppFnd2, false/*bIgnoreDefaults*/);
    }
    else
    {
        SfxWhichIter aIter( rSet );
        sal_uInt16 nWhich;
        while( 0 != ( nWhich = aIter.NextWhich() ) )
        {
            const SfxPoolItem* pItem = nullptr;
            (void)rSet.GetItemState( nWhich, true, &pItem );
            if( !pItem )
            {
                // Not set, so default
                MergeValue( rSet.GetPool()->GetDefaultItem( nWhich ) );
            }
            else if( IsInvalidItem( pItem ) )
                // don't care
                InvalidateItem( nWhich );
            else
                MergeValue( *pItem );
        }
    }
}

void SfxItemSet::MergeValue( const SfxPoolItem& rAttr, bool bIgnoreDefaults )
{
    SfxPoolItem const** ppFnd = m_pItems.get();
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
    SfxPoolItem const** ppFnd = m_pItems.get();
    const sal_uInt16* pPtr = m_pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            // In this Range?
            ppFnd += nWhich - *pPtr;

            if( *ppFnd ) // Set for me
            {
                if( !IsInvalidItem(*ppFnd) )
                {
                    m_pPool->Remove( **ppFnd );
                    *ppFnd = INVALID_POOL_ITEM;
                }
            }
            else
            {
                *ppFnd = INVALID_POOL_ITEM;
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
            return *pPtr + nPos;
        nPos = nPos - n;
        pPtr += 2;
    }
    assert(false);
    return 0;
}

bool SfxItemSet::operator==(const SfxItemSet &rCmp) const
{
    return Equals( rCmp, true);
}

bool SfxItemSet::Equals(const SfxItemSet &rCmp, bool bComparePool) const
{
    // Values we can get quickly need to be the same
    const bool bDifferentPools = (m_pPool != rCmp.m_pPool);
    if ( (bComparePool && m_pParent != rCmp.m_pParent) ||
         (bComparePool && bDifferentPools) ||
         Count() != rCmp.Count() )
        return false;

    // If we reach here and bDifferentPools==true that means bComparePool==false.

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
                          (m_pPool->IsItemPoolable(*pItem1) &&
                            *pItem1 != *pItem2 ) ) ) )
                    return false;
            }

            return true;
        }
    }

    // Are all pointers the same?
    if (0 == memcmp( m_pItems.get(), rCmp.m_pItems.get(), nCount1 * sizeof(m_pItems[0]) ))
        return true;

    // We need to compare each one separately then
    const SfxPoolItem **ppItem1 = m_pItems.get();
    const SfxPoolItem **ppItem2 = rCmp.m_pItems.get();
    for ( sal_uInt16 nPos = 0; nPos < nCount1; ++nPos )
    {
        // If the pointers of the poolable Items are not the same, the Items
        // must match
        if ( *ppItem1 != *ppItem2 &&
             ( ( !*ppItem1 || !*ppItem2 ) ||
               ( IsInvalidItem(*ppItem1) || IsInvalidItem(*ppItem2) ) ||
               (!bDifferentPools && m_pPool->IsItemPoolable(**ppItem1)) ||
                 **ppItem1 != **ppItem2 ) )
            return false;

        ++ppItem1;
        ++ppItem2;
    }

    return true;
}

std::unique_ptr<SfxItemSet> SfxItemSet::Clone(bool bItems, SfxItemPool *pToPool ) const
{
    if (pToPool && pToPool != m_pPool)
    {
        std::unique_ptr<SfxItemSet> pNewSet(new SfxItemSet(*pToPool, m_pWhichRanges));
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
        return std::unique_ptr<SfxItemSet>(bItems
                ? new SfxItemSet(*this)
                : new SfxItemSet(*m_pPool, m_pWhichRanges));
}

void SfxItemSet::PutDirect(const SfxPoolItem &rItem)
{
    SfxPoolItem const** ppFnd = m_pItems.get();
    const sal_uInt16* pPtr = m_pWhichRanges;
    const sal_uInt16 nWhich = rItem.Which();
#ifdef DBG_UTIL
    IsPoolDefaultItem(&rItem) || m_pPool->CheckItemInPool(&rItem);
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

void SfxItemSet::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SfxItemSet"));
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
    m_pWhichRanges = new sal_uInt16[nInitCount + 1]{};
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
static void AddItem_Impl(std::unique_ptr<SfxPoolItem const*[]> & rpItems, sal_uInt16 nOldSize, sal_uInt16 nPos)
{
    // Create new ItemArray
    SfxPoolItem const** pNew = new const SfxPoolItem*[nOldSize+1];

    // Was there one before?
    if ( rpItems )
    {
        // Copy all Items before nPos
        if ( nPos )
            memcpy( static_cast<void*>(pNew), rpItems.get(), nPos * sizeof(SfxPoolItem *) );

        // Copy all Items after nPos
        if ( nPos < nOldSize )
            memcpy( static_cast<void*>(pNew + nPos + 1), rpItems.get() + nPos,
                    (nOldSize-nPos) * sizeof(SfxPoolItem *) );
    }

    // Initialize new Item
    *(pNew + nPos) = nullptr;

    rpItems.reset(pNew);
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
                AddItem_Impl(m_pItems, nItemCount, nPos);
                break;
            }

            // WhichId is right after this Range?
            else if ( (nWhich-1) == *(pPtr+1) )
            {
                // Range grows upwards?
                (*(pPtr+1))++;

                // Make room after last Item of this Range
                nPos += nWhich - *pPtr;
                AddItem_Impl(m_pItems, nItemCount, nPos);
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
        AddItem_Impl(m_pItems, nItemCount, nPos);
    }

    // Add new Item to Pool
    const SfxPoolItem& rNew = m_pPool->Put( rItem, nWhich );

    // Remember old Item
    bool bIncrementCount = false;
    const SfxPoolItem* pOld = m_pItems[nPos];
    if ( IsInvalidItem(pOld) ) // state "dontcare"
        pOld = nullptr;
    if ( !pOld )
    {
        bIncrementCount = true;
        pOld = m_pParent
            ? &m_pParent->Get( nWhich )
            : (SfxItemPool::IsWhich(nWhich)
                    ? &m_pPool->GetDefaultItem(nWhich)
                    : nullptr);
    }

    // Add new Item to ItemSet
    m_pItems[nPos] = &rNew;

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

std::unique_ptr<SfxItemSet> SfxAllItemSet::Clone(bool bItems, SfxItemPool *pToPool ) const
{
    if (pToPool && pToPool != m_pPool)
    {
        std::unique_ptr<SfxAllItemSet> pNewSet(new SfxAllItemSet( *pToPool ));
        if ( bItems )
            pNewSet->Set( *this );
        return std::unique_ptr<SfxItemSet>(pNewSet.release()); // clang3.8 does not seem to be able to upcast std::unique_ptr
    }
    else
        return std::unique_ptr<SfxItemSet>(bItems ? new SfxAllItemSet(*this) : new SfxAllItemSet(*m_pPool));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
