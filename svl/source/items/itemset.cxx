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

#include <cassert>
#include <cstdarg>
#include <libxml/xmlwriter.h>

#include <sal/log.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>

#include <tools/stream.hxx>
#include <tools/solar.h>
#include <rtl/string.hxx>

#include "poolio.hxx"

static const sal_uInt16 nInitCount = 10; // Single USHORTs => 5 pairs without '0'

namespace
{

/**
 * Creates a sal_uInt16-ranges-array in 'rpRanges' using 'nWh1' and 'nWh2' as
 * first range, 'nNull' as terminator or start of 2nd range and 'pArgs' as
 * remainder.
 *
 * It returns the number of sal_uInt16s which are contained in the described
 * set of sal_uInt16s.
 */
sal_uInt16 InitializeRanges_Impl( sal_uInt16 *&rpRanges, va_list pArgs,
                               sal_uInt16 nWh1, sal_uInt16 nWh2, sal_uInt16 nNull )
{
    sal_uInt16 nSize = 0, nIns = 0;
    std::vector<sal_uInt16> aNumArr;
    aNumArr.push_back( nWh1 );
    aNumArr.push_back( nWh2 );
    DBG_ASSERT( nWh1 <= nWh2, "Invalid range" );
    nSize += nWh2 - nWh1 + 1;
    aNumArr.push_back( nNull );
    bool bEndOfRange = false;
    while ( 0 !=
            ( nIns =
              sal::static_int_cast< sal_uInt16 >(
                  va_arg( pArgs, int ) ) ) )
    {
        bEndOfRange = !bEndOfRange;
        if ( bEndOfRange )
        {
            const sal_uInt16 nPrev(*aNumArr.rbegin());
            DBG_ASSERT( nPrev <= nIns, "Invalid range" );
            nSize += nIns - nPrev + 1;
        }
        aNumArr.push_back( nIns );
    }

    assert( bEndOfRange ); // odd number of WhichIds

    // Now all ranges are present
    rpRanges = new sal_uInt16[ aNumArr.size() + 1 ];
    std::copy( aNumArr.begin(), aNumArr.end(), rpRanges);
    *(rpRanges + aNumArr.size()) = 0;

    return nSize;
}

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
{
    m_pWhichRanges = const_cast<sal_uInt16*>(m_pPool->GetFrozenIdRanges());
    assert( m_pWhichRanges && "don't create ItemSets with full range before FreezeIdRanges()" );
    if (!m_pWhichRanges)
        m_pPool->FillItemIdRanges_Impl( m_pWhichRanges );
}

SfxItemSet::SfxItemSet(SfxItemPool& rPool, sal_uInt16 nWhich1, sal_uInt16 nWhich2)
    : m_pPool( &rPool )
    , m_pParent(nullptr)
{
    assert(nWhich1 <= nWhich2);

    InitRanges_Impl(nWhich1, nWhich2);
}

void SfxItemSet::InitRanges_Impl(sal_uInt16 nWh1, sal_uInt16 nWh2)
{
    m_pWhichRanges = new sal_uInt16[3]{nWh1, nWh2, 0};
}

void SfxItemSet::InitRanges_Impl(va_list pArgs, sal_uInt16 nWh1, sal_uInt16 nWh2, sal_uInt16 nNull)
{
    InitializeRanges_Impl(m_pWhichRanges, pArgs, nWh1, nWh2, nNull);
}

SfxItemSet::SfxItemSet(SfxItemPool& rPool, int nWh1, int nWh2, int nNull, ...)
    : m_pPool( &rPool )
    , m_pParent(nullptr)
    , m_pWhichRanges(nullptr)
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

    std::ptrdiff_t cnt = pPtr - pWhichPairTable +1;
    m_pWhichRanges = new sal_uInt16[ cnt ];
    memcpy( m_pWhichRanges, pWhichPairTable, sizeof( sal_uInt16 ) * cnt );
}

SfxItemSet::SfxItemSet( SfxItemPool& rPool, const sal_uInt16* pWhichPairTable )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_pWhichRanges(nullptr)
{
    // pWhichPairTable == 0 is for the SfxAllEnumItemSet
    if ( pWhichPairTable )
        InitRanges_Impl(pWhichPairTable);
}

SfxItemSet::SfxItemSet( const SfxItemSet& rASet )
    : m_pPool( rASet.m_pPool )
    , m_pParent( rASet.m_pParent )
{
    // Calculate the attribute count
    sal_uInt16 nCnt = 0;
    sal_uInt16* pPtr = rASet.m_pWhichRanges;
    while( *pPtr )
    {
        nCnt += ( *(pPtr+1) - *pPtr ) + 1;
        pPtr += 2;
    }

    // Copy attributes
    SfxItemMap ppDst = m_aItems, ppSrc = rASet.m_aItems;
    for (auto & rSrcPair : rASet.m_aItems)
    {
        if ( IsInvalidItem(rSrcPair.second) ||       // DontCare?
             IsStaticDefaultItem(rSrcPair.second) )  // Defaults that are not to be pooled?
            // Just copy the pointer
            m_aItems.insert(rSrcPair);
        else if (m_pPool->IsItemPoolable( *rSrcPair.second ))
        {
            // Just copy the pointer and increase RefCount
            m_aItems.insert(rSrcPair);
            rSrcPair.second->AddRef();
        }
        else if ( !rSrcPair.second->Which() )
            m_aItems[rSrcPair.first] = rSrcPair.second->Clone();
        else
            // !IsPoolable() => assign via Pool
            m_aItems[rSrcPair.first] = &m_pPool->Put( *rSrcPair.second );
    }

    // Copy the WhichRanges
    std::ptrdiff_t cnt = pPtr - rASet.m_pWhichRanges + 1;
    m_pWhichRanges = new sal_uInt16[ cnt ];
    memcpy( m_pWhichRanges, rASet.m_pWhichRanges, sizeof( sal_uInt16 ) * cnt);
}

SfxItemSet::~SfxItemSet()
{
    for (auto & rPair : m_aItems)
        if( !IsInvalidItem(rPair.second) )
        {
            if( !rPair.second->Which() )
                delete rPair.second;
            else {
                // Still multiple references present, so just alter the RefCount
                if ( 1 < rPair.second->GetRefCount() && !IsDefaultItem(rPair.second) )
                    rPair.second->ReleaseRef();
                else
                    if ( !IsDefaultItem(rPair.second) )
                        // Delete from Pool
                        m_pPool->Remove( *rPair.second );
            }
        }

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

    if( nWhich )
    {
        auto it = m_aItems.find(nWhich);
        if (it != m_aItems.end())
        {
            const SfxPoolItem *pItemToClear = it->second;
            m_aItems.erase(it);
            // Due to the assertions in the sub calls, we need to do the following

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
    }
    else
    {
        nDel = m_aItems.size();

        SfxItemMap aTmp;
        aTmp.swap(m_aItems);
        for (auto & rPair : aTmp)
        {
            const SfxPoolItem *pItemToClear = rPair.second;
            nWhich = rPair.first;

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
    }
    return nDel;
}

void SfxItemSet::ClearInvalidItems()
{
    for (auto it = m_aItems.begin(); it != m_aItems.end(); )
    {
        if( IsInvalidItem(it->second) )
        {
            it = m_aItems.erase(it);
        }
        else
            ++it;
    }
}

void SfxItemSet::InvalidateAllItems()
{
    assert( m_aItems.empty() && "There are still Items set" );
    m_aItems.clear();
    const sal_uInt16* pPtr = m_pWhichRanges;
    while ( *pPtr )
    {
        for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich )
        {
            m_aItems[nWhich] = reinterpret_cast<const SfxPoolItem *>(-1);
        }
        pPtr += 2;
    }
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
        const sal_uInt16* pPtr = pAktSet->m_pWhichRanges;
        bool bFound = false;
        if (pPtr)
        {
            while ( *pPtr )
            {
                if ( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    bFound = true;
                    break;
                }
                pPtr += 2;
            }
        }
        if (bFound)
        {
            auto it = pAktSet->m_aItems.find(nWhich);
            if ( it == pAktSet->m_aItems.end())
            {
                eRet = SfxItemState::DEFAULT;
                if( !bSrchInParent )
                    return eRet; // Not present
                // Keep searching in the parents!
            }
            else
            {
                if ( reinterpret_cast<SfxPoolItem*>(-1) == it->second )
                    // Different ones are present
                    return SfxItemState::DONTCARE;

                if ( dynamic_cast<const SfxVoidItem *>(it->second) != nullptr )
                    return SfxItemState::DISABLED;

                if (ppItem)
                {
                    *ppItem = it->second;
                }
                return SfxItemState::SET;
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

    bool bFound = false;
    const sal_uInt16* pPtr = m_pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            bFound = true;
            break;
        }
        pPtr += 2;
    }
    if (!bFound)
        return nullptr;

    auto it = m_aItems.find(nWhich);
    if (it != m_aItems.end()) // Already one present
    {
        // Same Item already present?
        if ( it->second == &rItem )
            return nullptr;

        // Will 'dontcare' or 'disabled' be overwritten with some real value?
        if ( rItem.Which() && ( IsInvalidItem(it->second) || !it->second->Which() ) )
        {
            auto const old = it->second;
            it->second = &m_pPool->Put( rItem, nWhich );
            if (!IsInvalidItem(old)) {
                assert(old->Which() == 0);
                delete old;
            }
            return it->second;
        }

        // Turns into disabled?
        if( !rItem.Which() )
        {
            if (IsInvalidItem(it->second) || it->second->Which() != 0) {
                it->second = rItem.Clone(m_pPool);
            }
            return nullptr;
        }
        else
        {
            // Same value already present?
            if ( rItem == *it->second )
                return nullptr;

            // Add the new one, remove the old one
            const SfxPoolItem& rNew = m_pPool->Put( rItem, nWhich );
            const SfxPoolItem* pOld = it->second;
            it->second = &rNew;
            if (SfxItemPool::IsWhich(nWhich))
                Changed( *pOld, rNew );
            m_pPool->Remove( *pOld );
        }
    }
    else
    {
        if( !rItem.Which() )
            it = m_aItems.insert({ nWhich, rItem.Clone(m_pPool)}).first;
        else {
            const SfxPoolItem& rNew = m_pPool->Put( rItem, nWhich );
            it = m_aItems.insert({nWhich, &rNew}).first;
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
                *it->second != rItem,
                "svl.items", "putted Item unequal, with ID/pos " << nWhich );
    return it->second;
}

bool SfxItemSet::Put( const SfxItemSet& rSet, bool bInvalidAsDefault )
{
    bool bRet = false;
    if( rSet.Count() )
    {
        for (auto const & rPair : rSet.m_aItems)
        {
            if ( IsInvalidItem( rPair.second ) )
            {
                if ( bInvalidAsDefault )
                    bRet |= 0 != ClearItem( rPair.first );
                    // FIXME: Caused a SEGFAULT on non Windows-platforms:
                    // bRet |= 0 != Put( rSet.GetPool()->GetDefaultItem(nWhich), nWhich );
                else
                    InvalidateItem( rPair.first );
            }
            else
                bRet |= nullptr != Put( *rPair.second, rPair.first );
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
    const sal_uInt16* pPtr = rSet.m_pWhichRanges;
    while ( *pPtr )
    {
        for ( sal_uInt16 nWhich = *pPtr; nWhich <= *(pPtr+1); ++nWhich )
        {
            auto it = rSet.m_aItems.find(nWhich);
            if( it != rSet.m_aItems.end() )
            {
                if ( IsInvalidItem( it->second ) )
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
                    Put( *it->second, nWhich );
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
        assert(!pRange[2] || (pRange[2] - pRange[1]) > 1);
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
    SfxItemMap aNewItems;
    if (!m_aItems.empty())
    {
        sal_uInt16 n = 0;
        for ( const sal_uInt16 *pRange = pNewRanges; *pRange; pRange += 2 )
        {
            // iterate through all ids in the range
            for ( sal_uInt16 nWID = *pRange; nWID <= pRange[1]; ++nWID, ++n )
            {
                // direct move of pointer (not via pool)
                auto it = m_aItems.find(nWID);
                if ( it == m_aItems.end())
                {
                    // default
                }
                else if ( reinterpret_cast<SfxPoolItem*>(-1) == it->second )
                {
                    // don't care
                    aNewItems[nWID] = reinterpret_cast<SfxPoolItem*>(-1);
                }
                else if ( dynamic_cast<const SfxVoidItem *>(it->second) != nullptr )
                {
                    // put "disabled" item
                    aNewItems[nWID] = new SfxVoidItem(0);
                }
                else
                {
                    aNewItems[nWID] = it->second;
                    // increment new item count and possibly increment ref count
                    aNewItems[nWID]->AddRef();
                }
            }
        }
        // free old items
        for ( auto & rPair : m_aItems )
        {
            const SfxPoolItem *pItem = rPair.second;
            if ( !IsInvalidItem(pItem) && pItem->Which() )
                m_pPool->Remove(*pItem);
        }
    }

    // replace old items-array and ranges
    m_aItems.swap(aNewItems);

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
    if (!m_aItems.empty())
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
    const SfxItemSet* pAktSet = this;
    do
    {
        if( pAktSet->Count() )
        {
            auto it = pAktSet->m_aItems.find(nWhich);
            if( it != pAktSet->m_aItems.end() )
            {
                if( reinterpret_cast<SfxPoolItem*>(-1) == it->second ) {
                    //FIXME: The following code is duplicated further down
                    SAL_WARN_IF(!m_pPool, "svl.items", "no Pool, but status is ambiguous, with ID/pos " << nWhich);
                    //!((SfxAllItemSet *)this)->aDefault.SetWhich(nWhich);
                    //!return aDefault;
                    return m_pPool->GetDefaultItem( nWhich );
                }
#ifdef DBG_UTIL
                const SfxPoolItem *pItem = it->second;
                if ( dynamic_cast<const SfxVoidItem *>(pItem) != nullptr || !pItem->Which() )
                    SAL_INFO("svl.items", "SFX_WARNING: Getting disabled Item");
#endif
                return *it->second;
            }
        }
    } while (bSrchInParent && nullptr != (pAktSet = pAktSet->m_pParent));

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
        nRet += *(pPtr+1) - *pPtr + 1;
        pPtr += 2;
    }
    return nRet;
}

/**
 * Only retain the Items that are also present in rSet
 * (never mind their value).
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

    // get the set of keys since we are going to be modifying
    // the map while we iterate over it
    std::vector<sal_uInt16> keys(m_aItems.size());
    size_t idx = 0;
    for (auto const & rPair : m_aItems) {
        keys[idx++] = rPair.first;
    }
    for (sal_uInt16 nWhich : keys)
    {
        if( SfxItemState::UNKNOWN == rSet.GetItemState( nWhich, false ) )
            ClearItem( nWhich );        // Delete
    }
}

void SfxItemSet::Differentiate( const SfxItemSet& rSet )
{
    if( !Count() || !rSet.Count() )// None set?
        return;

    // get the set of keys since we are going to be modifying
    // the map while we iterate over it
    std::vector<sal_uInt16> keys(m_aItems.size());
    size_t idx = 0;
    for (auto const & rPair : m_aItems) {
        keys[idx++] = rPair.first;
    }
    for (sal_uInt16 nWhich : keys)
    {
        if( SfxItemState::SET == rSet.GetItemState( nWhich, false ) )
            ClearItem( nWhich ); // Delete
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
static void MergeItem_Impl( SfxItemPool *_pPool,
                            SfxItemMap& rItems, sal_uInt16 nWhich, const SfxPoolItem *pFnd2,
                            bool bIgnoreDefaults )
{
    auto ppFnd1 = rItems.find(nWhich);

    // 1st Item is Default?
    if ( ppFnd1 == rItems.end() )
    {
        if ( IsInvalidItem(pFnd2) )
            // Decision table: default, dontcare, doesn't matter, doesn't matter
            rItems[nWhich] = reinterpret_cast<SfxPoolItem*>(-1);

        else if ( pFnd2 && !bIgnoreDefaults &&
                  _pPool->GetDefaultItem(pFnd2->Which()) != *pFnd2 )
            // Decision table: default, set, !=, sal_False
            rItems[nWhich] = reinterpret_cast<SfxPoolItem*>(-1);

        else if ( pFnd2 && bIgnoreDefaults )
            // Decision table: default, set, doesn't matter, sal_True
            rItems[nWhich] = &_pPool->Put( *pFnd2 );
    }

    // 1st Item set?
    else if ( !IsInvalidItem(ppFnd1->second) )
    {
        if ( !pFnd2 )
        {
            // 2nd Item is Default
            if ( !bIgnoreDefaults &&
                 *ppFnd1->second != _pPool->GetDefaultItem(ppFnd1->second->Which()) )
            {
                // Decision table: set, default, !=, sal_False
                _pPool->Remove( *ppFnd1->second );
                ppFnd1->second = reinterpret_cast<SfxPoolItem*>(-1);
            }
        }
        else if ( IsInvalidItem(pFnd2) )
        {
            // 2nd Item is dontcare
            if ( !bIgnoreDefaults ||
                 *ppFnd1->second != _pPool->GetDefaultItem( ppFnd1->second->Which()) )
            {
                // Decision table: set, dontcare, doesn't matter, sal_False
                // or:             set, dontcare, !=, sal_True
                _pPool->Remove( *ppFnd1->second );
                ppFnd1->second = reinterpret_cast<SfxPoolItem*>(-1);
            }
        }
        else
        {
            // 2nd Item is set
            if ( *ppFnd1->second != *pFnd2 )
            {
                // Decision table: set, set, !=, doesn't matter
                _pPool->Remove( *ppFnd1->second );
                ppFnd1->second = reinterpret_cast<SfxPoolItem*>(-1);
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
            nSize += ( *(pWh1) - *(pWh1-1) ) + 1;
    }

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

void SfxItemSet::MergeValue( const SfxPoolItem& rAttr, bool bIgnoreDefaults )
{
    const sal_uInt16* pPtr = m_pWhichRanges;
    const sal_uInt16 nWhich = rAttr.Which();
    bool bFound = false;
    while( *pPtr )
    {
        // In this Range??
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            bFound = true;
            break;
        }
        pPtr += 2;
    }
    if (bFound)
        MergeItem_Impl(m_pPool, m_aItems, nWhich, &rAttr, bIgnoreDefaults);
}

void SfxItemSet::InvalidateItem( sal_uInt16 nWhich )
{
    bool bFound = false;
    const sal_uInt16* pPtr = m_pWhichRanges;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            bFound = true;
            break;
        }
        pPtr += 2;
    }
    if (!bFound)
        return;

    auto it = m_aItems.find(nWhich);
    if (it != m_aItems.end())
    {
        if( reinterpret_cast<SfxPoolItem*>(-1) != it->second ) // Not yet dontcare!
        {
            m_pPool->Remove( *it->second );
            it->second = reinterpret_cast<SfxPoolItem*>(-1);
        }
    }
    else
    {
        m_aItems[nWhich] = reinterpret_cast<SfxPoolItem*>(-1);
    }
}

sal_uInt16 SfxItemSet::GetWhichByPos( sal_uInt16 nPos ) const
{
    sal_uInt16* pPtr = m_pWhichRanges;
    while( *pPtr )
    {
        sal_uInt16 n = *(pPtr+1) - *pPtr + 1;
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
void SfxItemSet::Store
(
    SvStream&   rStream,        // Target stream for normal Items
    bool        bDirect         /* true: Save Items directly
                                   false: Surrogates */
)   const
{
    assert(m_pPool);

    // Remember position of the count (to be able to correct it, if need be)
    sal_uLong nCountPos = rStream.Tell();
    rStream.WriteUInt16( m_aItems.size() );

    // If there's nothing to save, don't construct an ItemIter
    if (!m_aItems.empty())
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
        if (nWrittenCount != m_aItems.size())
        {
            // Store real count in the stream
            sal_uLong nPos = rStream.Tell();
            rStream.Seek( nCountPos );
            rStream.WriteUInt16( nWrittenCount );
            rStream.Seek( nPos );
        }
    }
}

/**
 * This method loads an SfxItemSet from a stream.
 * If the SfxItemPool was loaded without RefCounts the loaded Item
 * references are counted, else we assume the they were accounted for
 * when loading the SfxItemPool.
 *
 * @see SfxItemSet::Store(Stream&,bool) const
 */
void SfxItemSet::Load
(
    SvStream&           rStream    //  Stream we're loading from
)
{
    assert(m_pPool);

    // Resolve Surrogates with ItemSet's Pool
    const SfxItemPool *pRefPool = m_pPool;

    // Load Item count and as many Items
    sal_uInt16 nCount = 0;
    rStream.ReadUInt16( nCount );

    const size_t nMinRecordSize = sizeof(sal_uInt16) * 2;
    const size_t nMaxRecords = rStream.remainingSize() / nMinRecordSize;
    if (nCount > nMaxRecords)
    {
        SAL_WARN("svl.items", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nCount << " claimed, truncating");
        nCount = nMaxRecords;
    }

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        // Load Surrogate/Item and resolve Surrogate
        const SfxPoolItem *pItem =
                m_pPool->LoadItem( rStream, pRefPool );

        // Did we load an Item or resolve a Surrogate?
        if ( pItem )
        {
            // Find position for Item pointer in the set
            sal_uInt16 nWhich = pItem->Which();
            const sal_uInt16* pPtr = m_pWhichRanges;
            bool bFound = false;
            while ( *pPtr )
            {
                // In this Range??
                if ( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
                {
                    bFound = true;
                    break;
                }
                pPtr += 2;
            }
            if (bFound)
            {
                SAL_WARN_IF( m_aItems.find(nWhich) != m_aItems.end(), "svl.items", "Item is present twice, with ID/pos " << nWhich);
                m_aItems[nWhich] = pItem;
            }
        }
    }
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
                          (m_pPool->IsItemPoolable(*pItem1) &&
                            *pItem1 != *pItem2 ) ) ) )
                    return false;
            }

            return true;
        }
    }

    // Are all pointers the same?
    if ( m_aItems == rCmp.m_aItems )
        return true;

    // We need to compare each one separately then
    auto it1 = m_aItems.begin();
    auto it2 = rCmp.m_aItems.begin();
    for (;; ++it1, ++it2)
    {
        if (it1 == m_aItems.end() && it2 == rCmp.m_aItems.end())
            break;
        if (it1 == m_aItems.end() || it2 == rCmp.m_aItems.end())
            return false;
        if (it1->first != it2->first)
            return false;
        if (it1->second == it2->second)
            continue;
        if (IsInvalidItem(it1->second) || IsInvalidItem(it2->second))
            return false;
        if (m_pPool->IsItemPoolable(*it1->second))
            return false;
        if (*it1->second != *it2->second)
            return false;
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
    const sal_uInt16* pPtr = m_pWhichRanges;
    const sal_uInt16 nWhich = rItem.Which();
#ifdef DBG_UTIL
    IsPoolDefaultItem(&rItem) || m_pPool->GetSurrogate(&rItem);
        // Only cause assertion in the callees
#endif

    bool bFound = false;
    while( *pPtr )
    {
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
        {
            bFound = true;
            break;
        }
        pPtr += 2;
    }
    if (!bFound)
        return;

    auto it = m_aItems.find(nWhich);
    if (it != m_aItems.end()) // One already present
    {
        if( rItem == *it->second )
            return; // Already present!
        m_pPool->Remove( *it->second );
    }

    // Add the new one
    if( IsPoolDefaultItem(&rItem) )
        m_aItems[nWhich] = &m_pPool->Put( rItem );
    else
    {
        m_aItems[nWhich] = &rItem;
        if( !IsStaticDefaultItem( &rItem ) )
            rItem.AddRef();
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
 * Putting with automatic extension of the WhichId with the ID of the Item.
 */
const SfxPoolItem* SfxAllItemSet::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    // Let's see first whether there's a suitable Range already
    sal_uInt16 *pPtr = m_pWhichRanges;
    while ( *pPtr )
    {
        // WhichId is within this Range?
        if( *pPtr <= nWhich && nWhich <= *(pPtr+1) )
            break;
        // To the next Range
        pPtr += 2;
    }

    // WhichId not yet present?
    if ( !*pPtr )
    {
        // Let's see if we can attach it somewhere
        pPtr = m_pWhichRanges;
        while ( *pPtr )
        {
            // WhichId is right before this Range?
            if ( (nWhich+1) == *pPtr )
            {
                // Grow range downwards
                (*pPtr)--;
                break;
            }

            // WhichId is right after this Range?
            else if ( (nWhich-1) == *(pPtr+1) )
            {
                // Grow range upwards
                (*(pPtr+1))++;
                break;
            }

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
    }

    // Add new Item to Pool
    const SfxPoolItem& rNew = m_pPool->Put( rItem, nWhich );

    // Remember old Item
    auto it = m_aItems.find(nWhich);
    const SfxPoolItem* pOld = nullptr;
    if (it != m_aItems.end())
        pOld = it->second;
    if ( reinterpret_cast< SfxPoolItem* >( -1 ) == pOld ) // state "dontcare"
        pOld = nullptr;
    if ( !pOld )
    {
        if (m_pParent)
            pOld = &m_pParent->Get( nWhich );
        else if (SfxItemPool::IsWhich(nWhich))
            pOld = &m_pPool->GetDefaultItem(nWhich);
    }

    // Add new Item to ItemSet
    m_aItems[nWhich] = &rNew;

    // Send Changed Notification
    if ( pOld )
    {
        Changed( *pOld, rNew );
        if ( !IsDefaultItem(pOld) )
            m_pPool->Remove( *pOld );
    }

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
