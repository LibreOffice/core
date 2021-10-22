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

#include <items_helper.hxx>

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
    : SfxItemSet(rPool, rPool.GetFrozenIdRanges())
{
}

SfxItemSet::SfxItemSet( SfxItemPool& rPool, SfxAllItemSetFlag )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_ppItems(nullptr)
    , m_nCount(0)
    , m_bItemsFixed(false)
{
}

/** special constructor for SfxItemSetFixed */
SfxItemSet::SfxItemSet( SfxItemPool& rPool, WhichRangesContainer&& ranges, SfxPoolItem const ** ppItems )
    : m_pPool(&rPool)
    , m_pParent(nullptr)
    , m_ppItems(ppItems)
    , m_pWhichRanges(std::move(ranges))
    , m_nCount(0)
    , m_bItemsFixed(true)
{
    assert(ppItems);
    assert(m_pWhichRanges.size() > 0);
    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxItemSet::SfxItemSet(
    SfxItemPool & pool,
    const WhichRangesContainer& wids,
    std::size_t items):
    m_pPool(&pool), m_pParent(nullptr),
    m_ppItems(new SfxPoolItem const *[items]{}),
    m_pWhichRanges(wids),
    m_nCount(0),
    m_bItemsFixed(false)
{
    assert(wids.size() != 0);
    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxItemSet::SfxItemSet(SfxItemPool& pool, const WhichRangesContainer& wids)
    : SfxItemSet(pool, wids, svl::detail::CountRanges(wids))
{
}

SfxItemSet::SfxItemSet(SfxItemPool& pool, WhichRangesContainer&& wids)
    : SfxItemSet(pool, wids, svl::detail::CountRanges(wids))
{
}

SfxItemSet::SfxItemSet( const SfxItemSet& rASet )
    : m_pPool( rASet.m_pPool )
    , m_pParent( rASet.m_pParent )
    , m_pWhichRanges( rASet.m_pWhichRanges )
    , m_nCount( rASet.m_nCount )
    , m_bItemsFixed(false)
{
    if (rASet.m_pWhichRanges.empty())
    {
        m_ppItems = nullptr;
        return;
    }

    sal_uInt16 nCnt = svl::detail::CountRanges(m_pWhichRanges);
    m_ppItems = new const SfxPoolItem* [nCnt] {};

    // Copy attributes
    SfxPoolItem const** ppDst = m_ppItems;
    SfxPoolItem const** ppSrc = rASet.m_ppItems;
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

    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxItemSet::SfxItemSet(SfxItemSet&& rASet) noexcept
    : m_pPool( rASet.m_pPool )
    , m_pParent( rASet.m_pParent )
    , m_ppItems( rASet.m_ppItems )
    , m_pWhichRanges( std::move(rASet.m_pWhichRanges) )
    , m_nCount( rASet.m_nCount )
    , m_bItemsFixed(false)
{
    if (rASet.m_bItemsFixed)
    {
        // have to make a copy
        int noItems = svl::detail::CountRanges(m_pWhichRanges);
        m_ppItems = new const SfxPoolItem* [noItems];
        std::copy(rASet.m_ppItems, rASet.m_ppItems + noItems, m_ppItems);
    }
    else
        // taking over ownership
        rASet.m_ppItems = nullptr;
    rASet.m_pPool = nullptr;
    rASet.m_pParent = nullptr;
    rASet.m_nCount = 0;
    assert(svl::detail::validRanges2(m_pWhichRanges));
}

SfxItemSet::~SfxItemSet()
{
    if (!m_pWhichRanges.empty()) // might be nullptr if we have been moved-from
    {
        sal_uInt16 nCount = TotalCount();
        if( Count() )
        {
            SfxPoolItem const** ppFnd = m_ppItems;
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
    }

    if (!m_bItemsFixed)
        delete[] m_ppItems;
    m_pWhichRanges.reset(); // for invariant-testing
}

/**
 * Delete single Items or all Items (nWhich == 0)
 */
sal_uInt16 SfxItemSet::ClearItem( sal_uInt16 nWhich )
{
    if( !Count() )
        return 0;

    sal_uInt16 nDel = 0;
    SfxPoolItem const** ppFnd = m_ppItems;

    if( nWhich )
    {
        for (const WhichPair& rPair : m_pWhichRanges)
        {
            // Within this range?
            if( rPair.first <= nWhich && nWhich <= rPair.second )
            {
                // Actually set?
                ppFnd += nWhich - rPair.first;
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
            ppFnd += rPair.second - rPair.first + 1;
        }
    }
    else
    {
        nDel = m_nCount;

        for (const WhichPair& rPair : m_pWhichRanges)
        {
            for( nWhich = rPair.first; nWhich <= rPair.second; ++nWhich, ++ppFnd )
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
        }
    }
    return nDel;
}

void SfxItemSet::ClearInvalidItems()
{
    SfxPoolItem const** ppFnd = m_ppItems;
    for (const WhichPair& rPair : m_pWhichRanges)
    {
        for( sal_uInt16 nWhich = rPair.first; nWhich <= rPair.second; ++nWhich, ++ppFnd )
            if( IsInvalidItem(*ppFnd) )
            {
                *ppFnd = nullptr;
                --m_nCount;
            }
    }
}

void SfxItemSet::InvalidateAllItems()
{
    assert( !m_nCount && "There are still Items set" );
    m_nCount = TotalCount();
    memset(static_cast<void*>(m_ppItems), -1, m_nCount * sizeof(SfxPoolItem*));
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
        SfxPoolItem const** ppFnd = pCurrentSet->m_ppItems;
        for (const WhichPair& rPair : pCurrentSet->m_pWhichRanges)
        {
            if ( rPair.first <= nWhich && nWhich <= rPair.second )
            {
                // Within this range
                ppFnd += nWhich - rPair.first;
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
            ppFnd += rPair.second - rPair.first + 1;
        }
        if (!bSrchInParent)
            break;
        pCurrentSet = pCurrentSet->m_pParent;
    } while (nullptr != pCurrentSet);
    return eRet;
}

bool SfxItemSet::HasItem(sal_uInt16 nWhich, const SfxPoolItem** ppItem) const
{
    bool bRet = SfxItemState::SET == GetItemState(nWhich, true, ppItem);
    if (!bRet && ppItem)
        *ppItem = nullptr;
    return bRet;
}

const SfxPoolItem* SfxItemSet::PutImpl( const SfxPoolItem& rItem, sal_uInt16 nWhich, bool bPassingOwnership )
{
    if ( !nWhich )
    {
        assert(!bPassingOwnership);
        return nullptr; //FIXME: Only because of Outliner bug
    }

    SfxPoolItem const** ppFnd = m_ppItems;
    for (const WhichPair& rPair : m_pWhichRanges)
    {
        if( rPair.first <= nWhich && nWhich <= rPair.second )
        {
            // Within this range
            ppFnd += nWhich - rPair.first;
            if( *ppFnd ) // Already one present
            {
                // Same Item already present?
                if ( *ppFnd == &rItem )
                {
                    assert(!bPassingOwnership);
                    return nullptr;
                }

                // Will 'dontcare' or 'disabled' be overwritten with some real value?
                if ( rItem.Which() && ( IsInvalidItem(*ppFnd) || !(*ppFnd)->Which() ) )
                {
                    const SfxPoolItem* const old = *ppFnd;
                    *ppFnd = &m_pPool->PutImpl( rItem, nWhich, bPassingOwnership );
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
                    if (bPassingOwnership)
                        delete &rItem;
                    return nullptr;
                }
                else
                {
                    // Same value already present?
                    if ( rItem == **ppFnd )
                    {
                        if (bPassingOwnership)
                            delete &rItem;
                        return nullptr;
                    }

                    // Add the new one, remove the old one
                    const SfxPoolItem& rNew = m_pPool->PutImpl( rItem, nWhich, bPassingOwnership );
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
                {
                    *ppFnd = rItem.Clone(m_pPool);
                    if (bPassingOwnership)
                        delete &rItem;
                }
                else
                {
                    const SfxPoolItem& rNew = m_pPool->PutImpl( rItem, nWhich, bPassingOwnership );
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
            SAL_WARN_IF(!bPassingOwnership && m_pPool->IsItemPoolable(nWhich) &&
                        dynamic_cast<const SfxSetItem*>( &rItem ) == nullptr &&
                        **ppFnd != rItem,
                        "svl.items", "putted Item unequal, with ID/pos " << nWhich );
            return *ppFnd;
        }
        ppFnd += rPair.second - rPair.first + 1;
    }
    if (bPassingOwnership)
        delete &rItem;
    return nullptr;
}

bool SfxItemSet::Put( const SfxItemSet& rSet, bool bInvalidAsDefault )
{
    bool bRet = false;
    if( rSet.Count() )
    {
        SfxPoolItem const** ppFnd = rSet.m_ppItems;
        for (const WhichPair& rPair : rSet.m_pWhichRanges)
        {
            for ( sal_uInt16 nWhich = rPair.first; nWhich <= rPair.second; ++nWhich, ++ppFnd )
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
    SfxPoolItem const** ppFnd = rSet.m_ppItems;
    for (const WhichPair& rPair : rSet.m_pWhichRanges)
    {
        for ( sal_uInt16 nWhich = rPair.first; nWhich <= rPair.second; ++nWhich, ++ppFnd )
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
    }
}

/**
 * Expands the ranges of settable items by 'nFrom' to 'nTo'. Keeps state of
 * items which are new ranges too.
 */
void SfxItemSet::MergeRange( sal_uInt16 nFrom, sal_uInt16 nTo )
{
    // special case: exactly one sal_uInt16 which is already included?
    if (nFrom == nTo)
        if (SfxItemState eItemState = GetItemState(nFrom, false);
            eItemState == SfxItemState::DEFAULT || eItemState == SfxItemState::SET)
            return;

    WhichRangesContainer pNewRanges = m_pWhichRanges.MergeRange(nFrom, nTo);
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
    if (m_pWhichRanges == pNewRanges)
        return;
    assert(svl::detail::validRanges2(pNewRanges));
    RecreateRanges_Impl(pNewRanges);
    m_pWhichRanges = pNewRanges;
}

void SfxItemSet::SetRanges( WhichRangesContainer&& pNewRanges )
{
    // Identical Ranges?
    if (m_pWhichRanges == pNewRanges)
        return;
    assert(svl::detail::validRanges2(pNewRanges));
    RecreateRanges_Impl(pNewRanges);
    m_pWhichRanges = std::move(pNewRanges);
}

void SfxItemSet::RecreateRanges_Impl(const WhichRangesContainer& pNewRanges)
{
    // create new item-array (by iterating through all new ranges)
    const sal_uInt16 nSize = svl::detail::CountRanges(pNewRanges);
    SfxPoolItem const** aNewItems = new const SfxPoolItem* [ nSize ];
    sal_uInt16 nNewCount = 0;
    if (m_nCount == 0)
        memset( aNewItems, 0, nSize * sizeof( SfxPoolItem* ) );
    else
    {
        sal_uInt16 n = 0;
        for ( auto const & pRange : pNewRanges )
        {
            // iterate through all ids in the range
            for ( sal_uInt16 nWID = pRange.first; nWID <= pRange.second; ++nWID, ++n )
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
            const SfxPoolItem *pItem = m_ppItems[nItem];
            if ( pItem && !IsInvalidItem(pItem) && pItem->Which() )
                m_pPool->Remove(*pItem);
        }
    }

    // replace old items-array and ranges
    if (m_bItemsFixed)
        m_bItemsFixed = false;
    else
        delete[] m_ppItems;
    m_ppItems = aNewItems;
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
            SfxPoolItem const** ppFnd = pCurrentSet->m_ppItems;
            for (auto const & pPtr : pCurrentSet->m_pWhichRanges)
            {
                if( pPtr.first <= nWhich && nWhich <= pPtr.second )
                {
                    // In this Range
                    ppFnd += nWhich - pPtr.first;
                    if( *ppFnd )
                    {
                        if( IsInvalidItem(*ppFnd) ) {
                            //FIXME: The following code is duplicated further down
                            assert(m_pPool);
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
                ppFnd += pPtr.second - pPtr.first + 1;
            }
        }
//TODO: Search until end of Range: What are we supposed to do now? To the Parent or Default??
//      if( !*pPtr )            // Until the end of the search Range?
//      break;
        if (!bSrchInParent)
            break;
        pCurrentSet = pCurrentSet->m_pParent;
    } while (nullptr != pCurrentSet);

    // Get the Default from the Pool and return
    assert(m_pPool);
    return m_pPool->GetDefaultItem( nWhich );
}

/**
 * Notification callback
 */
void SfxItemSet::Changed( const SfxPoolItem&, const SfxPoolItem& )
{
}

sal_uInt16 SfxItemSet::TotalCount() const
{
    return svl::detail::CountRanges(m_pWhichRanges);
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

    // If the Ranges are identical, we can easily process it
    if( m_pWhichRanges == rSet.m_pWhichRanges )
    {
        sal_uInt16 nSize = TotalCount();
        SfxPoolItem const** ppFnd1 = m_ppItems;
        SfxPoolItem const** ppFnd2 = rSet.m_ppItems;

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
        do
        {
            sal_uInt16 nWhich = IsInvalidItem( pItem )
                                ? GetWhichByPos( aIter.GetCurPos() )
                                : pItem->Which();
            if( SfxItemState::UNKNOWN == rSet.GetItemState( nWhich, false ) )
                ClearItem( nWhich );        // Delete
            pItem = aIter.NextItem();
        } while (pItem);
    }
}

void SfxItemSet::Differentiate( const SfxItemSet& rSet )
{
    if( !Count() || !rSet.Count() )// None set?
        return;

    // If the Ranges are identical, we can easily process it
    if( m_pWhichRanges == rSet.m_pWhichRanges )
    {
        sal_uInt16 nSize = TotalCount();
        SfxPoolItem const** ppFnd1 = m_ppItems;
        SfxPoolItem const** ppFnd2 = rSet.m_ppItems;

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
        do
        {
            sal_uInt16 nWhich = IsInvalidItem( pItem )
                                ? GetWhichByPos( aIter.GetCurPos() )
                                : pItem->Which();
            if( SfxItemState::SET == rSet.GetItemState( nWhich, false ) )
                ClearItem( nWhich ); // Delete
            pItem = aIter.NextItem();
        } while (pItem);

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

    // If the Ranges match, they are easier to process!
    if( m_pWhichRanges == rSet.m_pWhichRanges )
    {
        sal_uInt16 nSize = TotalCount();
        SfxPoolItem const** ppFnd1 = m_ppItems;
        SfxPoolItem const** ppFnd2 = rSet.m_ppItems;

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
    SfxPoolItem const** ppFnd = m_ppItems;
    const sal_uInt16 nWhich = rAttr.Which();
    for( auto const & pPtr : m_pWhichRanges )
    {
        // In this Range??
        if( pPtr.first <= nWhich && nWhich <= pPtr.second )
        {
            ppFnd += nWhich - pPtr.first;
            MergeItem_Impl(m_pPool, m_nCount, ppFnd, &rAttr, bIgnoreDefaults);
            break;
        }
        ppFnd += pPtr.second - pPtr.first + 1;
    }
}

void SfxItemSet::InvalidateItem( sal_uInt16 nWhich )
{
    SfxPoolItem const** ppFnd = m_ppItems;
    for( auto const & pPtr : m_pWhichRanges )
    {
        if( pPtr.first <= nWhich && nWhich <= pPtr.second )
        {
            // In this Range?
            ppFnd += nWhich - pPtr.first;

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
        ppFnd += pPtr.second - pPtr.first + 1;
    }
}

sal_uInt16 SfxItemSet::GetWhichByPos( sal_uInt16 nPos ) const
{
    sal_uInt16 n = 0;
    for( auto const & pPtr : m_pWhichRanges )
    {
        n = ( pPtr.second - pPtr.first ) + 1;
        if( nPos < n )
            return pPtr.first + nPos;
        nPos = nPos - n;
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
    for (sal_Int32 i = 0; i < m_pWhichRanges.size(); ++i)
    {
        if (m_pWhichRanges[i] != rCmp.m_pWhichRanges[i])
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
    if (0 == memcmp( m_ppItems, rCmp.m_ppItems, nCount1 * sizeof(m_ppItems[0]) ))
        return true;

    // We need to compare each one separately then
    const SfxPoolItem **ppItem1 = m_ppItems;
    const SfxPoolItem **ppItem2 = rCmp.m_ppItems;
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

SfxItemSet SfxItemSet::CloneAsValue(bool bItems, SfxItemPool *pToPool ) const
{
    if (pToPool && pToPool != m_pPool)
    {
        SfxItemSet aNewSet(*pToPool, m_pWhichRanges);
        if ( bItems )
        {
            SfxWhichIter aIter(aNewSet);
            sal_uInt16 nWhich = aIter.FirstWhich();
            while ( nWhich )
            {
                const SfxPoolItem* pItem;
                if ( SfxItemState::SET == GetItemState( nWhich, false, &pItem ) )
                    aNewSet.Put( *pItem, pItem->Which() );
                nWhich = aIter.NextWhich();
            }
        }
        return aNewSet;
    }
    else
        return bItems
                ? *this
                : SfxItemSet(*m_pPool, m_pWhichRanges);
}

void SfxItemSet::PutDirect(const SfxPoolItem &rItem)
{
    SfxPoolItem const** ppFnd = m_ppItems;
    const sal_uInt16 nWhich = rItem.Which();
#ifdef DBG_UTIL
    IsPoolDefaultItem(&rItem) || m_pPool->CheckItemInPool(&rItem);
        // Only cause assertion in the callees
#endif
    for( auto const & pPtr : m_pWhichRanges)
    {
        if( pPtr.first <= nWhich && nWhich <= pPtr.second )
        {
            // In this Range?
            ppFnd += nWhich - pPtr.first;
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
        ppFnd += pPtr.second - pPtr.first + 1;
    }
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
    if (pToPool && pToPool != m_pPool)
    {
        std::unique_ptr<SfxAllItemSet> pNewSet(new SfxAllItemSet( *pToPool ));
        if ( bItems )
            pNewSet->Set( *this );
        return pNewSet;
    }
    else
        return std::unique_ptr<SfxItemSet>(bItems ? new SfxAllItemSet(*this) : new SfxAllItemSet(*m_pPool));
}

SfxItemSet SfxAllItemSet::CloneAsValue(bool , SfxItemPool * ) const
{
    // if you are trying to clone, then the thing you are cloning is polymorphic, which means
    // it cannot be cloned as a value
    throw std::logic_error("cannot do this");
}




WhichRangesContainer::WhichRangesContainer( const WhichPair* wids, sal_Int32 nSize )
{
    auto p = new WhichPair[nSize];
    for (int i=0; i<nSize; ++i)
        p[i] = wids[i];
    m_pairs = p;
    m_size = nSize;
    m_bOwnRanges = true;
}

WhichRangesContainer::WhichRangesContainer(sal_uInt16 nWhichStart, sal_uInt16 nWhichEnd)
    : m_size(1), m_bOwnRanges(true)
{
    auto p = new WhichPair[1];
    p[0] = { nWhichStart, nWhichEnd };
    m_pairs = p;
}

WhichRangesContainer::WhichRangesContainer(WhichRangesContainer && other)
{
    std::swap(m_pairs, other.m_pairs);
    std::swap(m_size, other.m_size);
    std::swap(m_bOwnRanges, other.m_bOwnRanges);
}

WhichRangesContainer& WhichRangesContainer::operator=(WhichRangesContainer && other)
{
    std::swap(m_pairs, other.m_pairs);
    std::swap(m_size, other.m_size);
    std::swap(m_bOwnRanges, other.m_bOwnRanges);
    return *this;
}

WhichRangesContainer& WhichRangesContainer::operator=(WhichRangesContainer const & other)
{
    reset();
    m_size = other.m_size;
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
}

// Adds a range to which ranges, keeping the ranges in valid state (sorted, non-overlapping)
WhichRangesContainer WhichRangesContainer::MergeRange(sal_uInt16 nFrom,
                                       sal_uInt16 nTo) const
{
    assert(svl::detail::validRange(nFrom, nTo));

    if (empty())
        return WhichRangesContainer(nFrom, nTo);

    // create vector of ranges (sal_uInt16 pairs of lower and upper bound)
    const size_t nOldCount = size();
    std::vector<WhichPair> aRangesTable;
    aRangesTable.reserve(nOldCount);
    bool bAdded = false;
    for (const auto& rPair : *this)
    {
        if (!bAdded && rPair.first >= nFrom)
        { // insert new range, keep ranges sorted
            aRangesTable.push_back({ nFrom, nTo });
            bAdded = true;
        }
        // insert current range
        aRangesTable.emplace_back(rPair);
    }
    if (!bAdded)
        aRangesTable.push_back({ nFrom, nTo });

    // true if ranges overlap or adjoin, false if ranges are separate
    auto needMerge = [](WhichPair lhs, WhichPair rhs) {
        return (lhs.first - 1) <= rhs.second && (rhs.first - 1) <= lhs.second;
    };

    auto it = aRangesTable.begin();
    // we got at least one range
    for (;;)
    {
        auto itNext = std::next(it);
        if (itNext == aRangesTable.end())
            break;
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

    return WhichRangesContainer(aRangesTable.data(), aRangesTable.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
