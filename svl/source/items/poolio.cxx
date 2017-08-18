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

#include "poolio.hxx"

#include <sal/log.hxx>
#include <tools/solar.h>
#include <svl/itempool.hxx>
#include <svl/SfxBroadcaster.hxx>

#include <algorithm>
#include <memory>

/// clear array of PoolItem variants
/// after all PoolItems are deleted
/// or all ref counts are decreased
void SfxPoolItemArray_Impl::clear()
{
    maPoolItemVector.clear();
    maFree.clear();
    maPtrToIndex.clear();
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

sal_uInt16 SfxItemPool::GetFirstWhich() const
{
    return pImpl->mnStart;
}

sal_uInt16 SfxItemPool::GetLastWhich() const
{
    return pImpl->mnEnd;
}

bool SfxItemPool::IsInRange( sal_uInt16 nWhich ) const
{
    return nWhich >= pImpl->mnStart && nWhich <= pImpl->mnEnd;
}

sal_uInt16 SfxItemPool::GetIndex_Impl(sal_uInt16 nWhich) const
{
    if (nWhich < pImpl->mnStart || nWhich > pImpl->mnEnd)
    {
        assert(false && "missing bounds check before use");
        return 0;
    }
    return nWhich - pImpl->mnStart;
}

sal_uInt16 SfxItemPool::GetSize_Impl() const
{
    return pImpl->mnEnd - pImpl->mnStart + 1;
}


bool SfxItemPool::CheckItemInPool(const SfxPoolItem *pItem) const
{
    DBG_ASSERT( pItem, "no 0-Pointer Surrogate" );
    DBG_ASSERT( !IsInvalidItem(pItem), "no Invalid-Item Surrogate" );
    DBG_ASSERT( !IsPoolDefaultItem(pItem), "no Pool-Default-Item Surrogate" );

    if ( !IsInRange(pItem->Which()) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->CheckItemInPool( pItem );
        SAL_WARN( "svl.items", "unknown Which-Id - don't ask me for surrogates, with ID/pos " << pItem->Which());
    }

    // Pointer on static or pool-default attribute?
    if( IsStaticDefaultItem(pItem) || IsPoolDefaultItem(pItem) )
        return true;

    SfxPoolItemArray_Impl* pItemArr = pImpl->maPoolItems[GetIndex_Impl(pItem->Which())];
    DBG_ASSERT(pItemArr, "ItemArr is not available");

    for ( size_t i = 0; i < pItemArr->size(); ++i )
    {
        const SfxPoolItem *p = (*pItemArr)[i];
        if ( p == pItem )
            return true;
    }
    SAL_WARN( "svl.items", "Item not in the pool, with ID/pos " << pItem->Which());
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
