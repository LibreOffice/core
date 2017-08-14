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

#include <sal/log.hxx>
#include <tools/solar.h>
#include <svl/itempool.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/filerec.hxx>
#include "poolio.hxx"
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

/**
 * Returns the <SfxItemPool> that is being saved.
 * This should only be used in very exceptional cases e.g.
 * when guaranteeing file format compatibility:
 * When overriding a <SfxPoolItem::Store()> getting additional data from the Pool
 */
const SfxItemPool* SfxItemPool::GetStoringPool()
{
    return pStoringPool_;
}

static SfxItemKind convertUInt16ToSfxItemKind(sal_uInt16 nRefCnt)
{
    switch (nRefCnt)
    {
    case 0:
        return SfxItemKind::NONE;
    case 0xfffd:
        return SfxItemKind::DeleteOnIdle;
    case 0xfffe:
        return SfxItemKind::StaticDefault;
    case 0xffff:
        return SfxItemKind::PoolDefault;
    default:
        assert(false);
        abort();
    }
}

/**
 * If the SfxItemPool was loaded with 'bRefCounts' == sal_False, we need
 * to finish the loading of the document contents with a call of this method.
 * In any other case calling this function has no meaning.
 *
 * When loading without RefCounts, they are actually set to 1 so that
 * SfxPoolItems that are needed during and after loading are not deleted.
 * This method resets the RefCount and also removes all items that are not
 * needed anymore.
 *
 * @see SfxItemPool::Load()
*/
void SfxItemPool::LoadCompleted()
{
    // Did we load without RefCounts?
    if ( pImpl->nInitRefCount > 1 )
    {
        // Iterate over all Which values
        for (auto& rPoolItemArrayPtr : pImpl->maPoolItems)
        {
            // Is there an item with the Which value present at all?
            if (rPoolItemArrayPtr)
            {
                // Iterate over all items with this WhichId
                for (auto& rItemPtr : *rPoolItemArrayPtr)
                {
                    if (rItemPtr)
                    {
                        if (!ReleaseRef(*rItemPtr))
                            DELETEZ(rItemPtr);
                    }
                }
                // don't clear array, fill free list and clear pointer map
                rPoolItemArrayPtr->ReHash();
            }
        }

        // from now on normal initial ref count
        pImpl->nInitRefCount = 1;
    }

    // notify secondary pool
    if ( pImpl->mpSecondary )
        pImpl->mpSecondary->LoadCompleted();
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

// This had to be moved to a method of its own to keep Solaris GCC happy:
void SfxItemPool_Impl::readTheItems (
    SvStream & rStream, sal_uInt32 nItemCount, sal_uInt16 nVer,
    SfxPoolItem const * pDefItem, SfxPoolItemArray_Impl ** ppArr)
{
    SfxMultiRecordReader aItemsRec( &rStream, SFX_ITEMPOOL_REC_ITEMS );

    SfxPoolItemArray_Impl *pNewArr = new SfxPoolItemArray_Impl();
    SfxPoolItem *pItem = nullptr;

    sal_uLong n, nLastSurrogate = sal_uLong(-1);
    while (aItemsRec.GetContent())
    {
        // Get next surrogate
        sal_uInt16 nSurrogate = aItemsRec.GetContentTag();
        DBG_ASSERT( aItemsRec.GetContentVersion() == 'X',
                    "not an item content" );

        // Fill up missing ones
        // coverity[tainted_data] - ignore this, though we should finally kill off this format
        for ( pItem = nullptr, n = nLastSurrogate+1; n < nSurrogate; ++n )
            pNewArr->push_back( pItem );
        nLastSurrogate = nSurrogate;

        // Load RefCount and Item
        sal_uInt16 nRef(0);
        rStream.ReadUInt16( nRef );

        pItem = pDefItem->Create(rStream, nVer);
        pNewArr->push_back( pItem );

        if ( !mbPersistentRefCounts )
            // Hold onto it until SfxItemPool::LoadCompleted()
            SfxItemPool::AddRef(*pItem);
        else
        {
            if ( nRef > SFX_ITEMS_OLD_MAXREF )
                SfxItemPool::SetKind(*pItem, convertUInt16ToSfxItemKind(nRef));
            else
                SfxItemPool::AddRef(*pItem, nRef);
        }
    }

    // Fill up missing ones
    for ( pItem = nullptr, n = nLastSurrogate+1; n < nItemCount; ++n )
        pNewArr->push_back( pItem );

    SfxPoolItemArray_Impl *pOldArr = *ppArr;
    *ppArr = pNewArr;

    // Remember items that are already in the pool
    bool bEmpty = true;
    if ( nullptr != pOldArr )
        for ( n = 0; bEmpty && n < pOldArr->size(); ++n )
            bEmpty = pOldArr->operator[](n) == nullptr;
    SAL_WARN_IF( !bEmpty, "svl.items", "loading non-empty pool" );
    if ( !bEmpty )
    {
        // See if there's a new one for all old ones
        for ( size_t nOld = 0; nOld < pOldArr->size(); ++nOld )
        {
            SfxPoolItem *pOldItem = (*pOldArr)[nOld];
            if ( pOldItem )
            {
                sal_uInt32 nFree = SAL_MAX_UINT32;
                bool bFound = false;
                for ( size_t nNew = (*ppArr)->size(); nNew--; )
                {
                    // Loaded Item
                    SfxPoolItem *&rpNewItem =
                        (*ppArr)->operator[](nNew);

                    // Unused surrogate?
                    if ( !rpNewItem )
                        nFree = nNew;

                    // Found it?
                    else if ( *rpNewItem == *pOldItem )
                    {
                        // Reuse
                        SfxItemPool::AddRef( *pOldItem, rpNewItem->GetRefCount() );
                        SfxItemPool::ClearRefCount( *rpNewItem );
                        delete rpNewItem;
                        rpNewItem = pOldItem;
                        bFound = true;
                        break;
                    }
                }

                // Take over the ones that were previously present, but had not been loaded
                if ( !bFound )
                {
                    if ( nFree != SAL_MAX_UINT32 )
                        (*ppArr)->operator[](nFree) = pOldItem;
                    else
                        (*ppArr)->push_back( pOldItem );
                }
            }
        }
    }
    delete pOldArr;

    (*ppArr)->ReHash(); // paranoid
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


sal_uInt32 SfxItemPool::GetSurrogate(const SfxPoolItem *pItem) const
{
    DBG_ASSERT( pItem, "no 0-Pointer Surrogate" );
    DBG_ASSERT( !IsInvalidItem(pItem), "no Invalid-Item Surrogate" );
    DBG_ASSERT( !IsPoolDefaultItem(pItem), "no Pool-Default-Item Surrogate" );

    if ( !IsInRange(pItem->Which()) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetSurrogate( pItem );
        SAL_WARN( "svl.items", "unknown Which-Id - don't ask me for surrogates, with ID/pos " << pItem->Which());
    }

    // Pointer on static or pool-default attribute?
    if( IsStaticDefaultItem(pItem) || IsPoolDefaultItem(pItem) )
        return SFX_ITEMS_DEFAULT;

    SfxPoolItemArray_Impl* pItemArr = pImpl->maPoolItems[GetIndex_Impl(pItem->Which())];
    DBG_ASSERT(pItemArr, "ItemArr is not available");

    for ( size_t i = 0; i < pItemArr->size(); ++i )
    {
        const SfxPoolItem *p = (*pItemArr)[i];
        if ( p == pItem )
            return i;
    }
    SAL_WARN( "svl.items", "Item not in the pool, with ID/pos " << pItem->Which());
    return SFX_ITEMS_NULL;
}


bool SfxItemPool::IsInStoringRange( sal_uInt16 nWhich ) const
{
    return nWhich >= pImpl->nStoringStart &&
           nWhich <= pImpl->nStoringEnd;
}

/**
 * This method allows for restricting the WhichRange, which is saved
 * by ItemSets of this Pool (and the Pool itself).
 * The method must be called before SfxItemPool::Store() and the values
 * must also be still set when the actual document (the ItemSets) is
 * being saved.
 *
 * Resetting it is not necessary, if this range is set correctly before
 * _every_ save, because its only accounted for when saving.
 *
 * We need to do this for the 3.1 format, because there's a bug in the
 * Pool loading method.
*/
void SfxItemPool::SetStoringRange( sal_uInt16 nFrom, sal_uInt16 nTo )
{
    pImpl->nStoringStart = nFrom;
    pImpl->nStoringEnd = nTo;
}

/**
 * This method converts WhichIds from a file format to the version of the
 * current pool.
 * If the file format is older, the conversion tables (set by the pool developer
 * using SetVersion()) are used. If the file format is newer the conversion tables
 * loaded from the file format are used. In this case, not every WhichId can be
 * mapped in which case we return 0.
 *
 * The calculation is only defined for WhichIds supported by the corresponding
 * file version, which is guarded by an assertion.
 *
 * Precondition:   Pool must be loaded
 * Postcondition:  Unchanged
 * Runtime:        linear(Count of the secondary pools) +
 *                 linear(Difference of the old and newer version)
 *
 * @see SfxItemPool::IsLoadingVersionCurrent() const
 * @see SfxItemPool::GetVersion() const
 */
sal_uInt16 SfxItemPool::GetNewWhich
(
    sal_uInt16  nFileWhich // The WhichId loaded from the stream
)   const
{
    // Return original (nDiff==0) or mapped (nDiff!=0) Id
    return nFileWhich;
}


OUString readByteString(SvStream& rStream)
{
    return rStream.ReadUniOrByteString(rStream.GetStreamCharSet());
}

void writeByteString(SvStream & rStream, const OUString& rString)
{
    rStream.WriteUniOrByteString(rString, rStream.GetStreamCharSet());
}

OUString readUnicodeString(SvStream & rStream, bool bUnicode)
{
    return rStream.ReadUniOrByteString(bUnicode ? RTL_TEXTENCODING_UCS2 :
                                      rStream.GetStreamCharSet());
}

void writeUnicodeString(SvStream & rStream, const OUString& rString)
{
    rStream.WriteUniOrByteString(rString, RTL_TEXTENCODING_UCS2);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
