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

/**
 * Loads surrogate from 'rStream' and returns the corresponding SfxPoolItem
 * from the rRefPool.
 * If the surrogate contained within the stream == SFX_ITEMS_DIRECT
 * (!poolable), we return 0 and the Item is to be loaded directly
 * from the stream.
 * We also return 0 for 0xfffffff0 (SFX_ITEMS_NULL) and rWhich is set to 0,
 * making the Items unavailable.
 *
 * Apart from that we also take into account whether the Pool is loaded without
 * RefCounts, if we reload from a new Pool (&rRefPool != this) or if we're
 * building from a differently constructed Pool.
 *
 * If we load from a differently constructed Pool and the 'nSlotId' cannot be
 * mapped to a WhichId of this Pool, we also return 0.
 *
 * Preconditions:   - Pool must be loaded
 *                  - LoadCompleted must not have been called yet
 *                  - 'rStream' is at the position at which a surrogate
 *                    for an Item with the SlotId 'nSlotId', the WhichId
 *                    'rWhichId' was saved with StoreSurrogate
 *
 * Postconditions:  - 'rStream' is at the same position as after StoreSurrogate
 *                    had finished saving
 *                  - If we were able to load an Item, it's now in this
 *                    SfxItemPool
 *                  - 'rWhichId' now contains the mapped WhichId
 *
 * Runtime: Depth of the traget secondary Pool * 10 + 10
 *
 * @see SfxItemPool::StoreSurrogate(SvStream&,const SfxPoolItem &)const
*/
const SfxPoolItem* SfxItemPool::LoadSurrogate
(
    SvStream&           rStream,    // Stream before a surrogate
    sal_uInt16&         rWhich,     // WhichId of the SfxPoolItem that is to be loaded
    sal_uInt16          nSlotId,    // SlotId of the SfxPoolItem that is to be loaded
    const SfxItemPool*  pRefPool    // SfxItemPool in which the surrogate is valid
)
{
    // Read the first surrogate
    sal_uInt32 nSurrogat(0);
    rStream.ReadUInt32( nSurrogat );

    // Is item stored directly?
    if ( SFX_ITEMS_DIRECT == nSurrogat )
        return nullptr;

    // Item does not exist?
    if ( SFX_ITEMS_NULL == nSurrogat )
    {
        rWhich = 0;
        return nullptr;
    }

    // If the Pool in the stream has the same structure, the surrogate
    // can be resolved in any case
    if ( !pRefPool )
        pRefPool = this;

    bool bResolvable = !pRefPool->GetName().isEmpty();
    if ( !bResolvable )
    {
        // If the pool in the stream has a different structure, the SlotId
        // from the stream must be mappable to a WhichId
        sal_uInt16 nMappedWhich = nSlotId ? GetWhich(nSlotId) : 0;
        if ( IsWhich(nMappedWhich) )
        {
            // Mapped SlotId can be taken over
            rWhich = nMappedWhich;
            bResolvable = true;
        }
    }

    // Can the surrogate be resolved?
    if ( bResolvable )
    {
        const SfxPoolItem *pItem = nullptr;
        for ( SfxItemPool *pTarget = this; pTarget; pTarget = pTarget->pImpl->mpSecondary )
        {
            // Found the right (Range-)Pool?
            if ( pTarget->IsInRange(rWhich) )
            {
                sal_uInt16 nIndex = pTarget->GetIndex_Impl(rWhich);
                // Default attribute?
                if ( SFX_ITEMS_DEFAULT == nSurrogat )
                    return (*pTarget->pImpl->mpStaticDefaults)[ nIndex ];

                SfxPoolItemArray_Impl* pItemArr = pTarget->pImpl->maPoolItems[nIndex];
                pItem = pItemArr && nSurrogat < pItemArr->size()
                            ? (*pItemArr)[nSurrogat]
                            : nullptr;
                if ( !pItem )
                {
                    OSL_FAIL( "can't resolve surrogate" );
                    rWhich = 0; // Just to be sure; for the right StreamPos
                    return nullptr;
                }

                // Reload from RefPool?
                if ( pRefPool != pImpl->mpMaster )
                    return &pTarget->Put( *pItem );

                // References have NOT been loaded together with the pool?
                if ( !pTarget->pImpl->mbPersistentRefCounts )
                    AddRef( *pItem );
                else
                    return pItem;

                return pItem;
            }
        }

        SAL_WARN("svl.items", "can't resolve Which-Id in LoadSurrogate, with ID/pos " << rWhich );
    }

    return nullptr;
}


/**
 * Saves a surrogate for '*pItem' in 'rStream'
 *
 * @returns sal_True: a real surrogates has been saved
 *                    SFX_ITEMS_NULL for 'pItem==0', SfxItemKind::StaticDefault
 *                    and SfxItemKind::PoolDefault are 'real' surrogates
 *
 * @returns sal_False: a dummy surrogate (SFX_ITEMS_DIRECT) has been saved;
 *                     the actual Item needs to be saved right after it on
 *                     its own
*/
bool SfxItemPool::StoreSurrogate ( SvStream& rStream, const SfxPoolItem*  pItem) const
{
    if ( pItem )
    {
        bool bRealSurrogate = IsItemPoolable(*pItem);
        rStream.WriteUInt32( bRealSurrogate
                        ? GetSurrogate( pItem )
                        : SFX_ITEMS_DIRECT  );
        return bRealSurrogate;
    }

    rStream.WriteUInt32( SFX_ITEMS_NULL );
    return true;
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
 * This method allows for the creation of new and incompatible WhichId
 * Ranges or distributions. Pools that were saved with old versions
 * are mapped using the provided conversion table until the current
 * version has been reached. Newer pools can be loaded, but will lose
 * newer attributes, because the map is saved in conjunction with the pool.
 *
 *  Precondition:   Pool must not be loaded yet
 *  Postcondition:  WhichIds from older versions can be mapped to version 'nVer'
 *  Runtime:        1.5 * new + 10
 *
 *  For newer WhichRanges (nStart,nEnd) it must hold that older WhichRanges
 *  (nOldStart,nOldEnd) are completely contained in the newer WhichRange.
 *  It is valid to extend the WhichRange to both sides; also by inserting
 *  WhichIds. Moving WhichIds is not permitted.
 *  This method should only be called in or right after the ctor.
 *
 *  The array must be static, because it's not copied and reused in the
 *  copy-ctor of the SfxItemPool
 *
 *  Example usage:
 *  Originally (version 0) the pool had the following WhichIds:
 *
 *     1:A, 2:B, 3:C, 4:D
 *
 *  A newer version (version 1) is now supposed to contain two new Ids
 *  X and Y between B and C, looking like this:
 *
 *      1:A, 2:B, 3:X, 4:Y, 5:C, 6:D
 *
 *  We see that the Ids 3 and 4 have changed. For the new version, we
 *  would need to set the following in the new Pool:
 *
 *      static sal_uInt16 nVersion1Map = { 1, 2, 5, 6 };
 *      pPool->SetVersionMap( 1, 1, 4, &nVersion1Map );
 *
 *  @see SfxItemPool::IsLoadingVersionCurrent() const
 *  @see SfxItemPool::GetNewWhich(sal_uInt16)
 *  @see SfxItemPool::GetVersion() const
 */
void SfxItemPool::SetVersionMap
(
    sal_uInt16  nVer,               // New version number
    sal_uInt16  nOldStart,          // Old first WhichId
    sal_uInt16  nOldEnd,            // Old last WhichId
    const sal_uInt16* pOldWhichIdTab /* Array containing the structure of the WhichIds
                                        of the previous version, in which the new
                                        corresponding new WhichId is located */
)
{
    // Create new map entry to insert
    const SfxPoolVersion_ImplPtr pVerMap = std::make_shared<SfxPoolVersion_Impl>(
                nVer, nOldStart, nOldEnd, pOldWhichIdTab );

    DBG_ASSERT( nVer > pImpl->nVersion, "Versions not sorted" );
    pImpl->nVersion = nVer;
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
 * @see SfxItemPool::SetVersionMap(sal_uInt16,sal_uInt16,sal_uInt16,sal_uInt16*)
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


/**
 * Saves the SfxPoolItem 'rItem' to the SvStream 'rStream':
 * either as a surrogate ('bDirect == false') or directly with
 * 'rItem.Store()'.
 * Non-poolable Items are always saved directly. Items without WhichId and
 * SID-Items as well as Items that were not yet present in the file format
 * version (return false) are not saved.
 *
 * The Item is saved to the Stream in the following manner:
 *   sal_uInt16  rItem.Which()
 *   sal_uInt16  GetSlotId( rItem.Which() ) or 0 if not available
 *   sal_uInt16  GetSurrogate( &rItem ) or SFX_ITEM_DIRECT for '!SFX_ITEM_POOLABLE'
 *
 * Optionally (if 'bDirect == true' or '!rItem.IsPoolable()':
 *   sal_uInt16  rItem.GetVersion()
 *   sal_uLong   Size
 *   Size        rItem.Store()
 *
 *  @see SfxItemPool::LoadItem(SvStream&,bool) const
 */
bool SfxItemPool::StoreItem( SvStream &rStream, const SfxPoolItem &rItem,
                                 bool bDirect ) const
{
    DBG_ASSERT( !IsInvalidItem(&rItem), "cannot store invalid items" );

    if ( IsSlot( rItem.Which() ) )
        return false;

    const SfxItemPool *pPool = this;
    while ( !pPool->IsInStoringRange(rItem.Which()) )
        if ( nullptr == ( pPool = pPool->pImpl->mpSecondary ) )
            return false;

    DBG_ASSERT( !pImpl->bInSetItem || dynamic_cast<const SfxSetItem*>( &rItem ) ==  nullptr,
                "SetItem contains ItemSet with SetItem" );

    sal_uInt16 nSlotId = pPool->GetSlotId( rItem.Which() );
    sal_uInt16 nItemVersion = rItem.GetVersion(pImpl->mnFileFormatVersion);
    if ( USHRT_MAX == nItemVersion )
        return false;

    rStream.WriteUInt16( rItem.Which() ).WriteUInt16( nSlotId );
    if ( bDirect || !pPool->StoreSurrogate( rStream, &rItem ) )
    {
        rStream.WriteUInt16( nItemVersion );
        rStream.WriteUInt32( 0L ); // Room for length in bytes
        sal_uLong nIStart = rStream.Tell();
        rItem.Store(rStream, nItemVersion);
        sal_uLong nIEnd = rStream.Tell();
        rStream.Seek( nIStart-4 );
        rStream.WriteInt32( nIEnd-nIStart );
        rStream.Seek( nIEnd );
    }

    return true;
}


/**
 * If pRefPool==-1 => do not put!
 */
const SfxPoolItem* SfxItemPool::LoadItem( SvStream &rStream,
                                          const SfxItemPool *pRefPool )
{
    sal_uInt16 nWhich(0), nSlot(0); // nSurrogate;
    rStream.ReadUInt16( nWhich ).ReadUInt16( nSlot );

    bool bDontPut = reinterpret_cast<SfxItemPool*>(-1) == pRefPool;
    if ( bDontPut || !pRefPool )
        pRefPool = this;

    DBG_ASSERT( !nWhich || !pImpl->bInSetItem ||
                dynamic_cast<const SfxSetItem*>( (*pRefPool->pImpl->mpStaticDefaults)[pRefPool->GetIndex_Impl(nWhich)] ) == nullptr,
                "loading SetItem in ItemSet of SetItem" );

    // Are we loading via surrogate?
    const SfxPoolItem *pItem = nullptr;
    // WhichId known in this version?
    if ( nWhich )
        // Load surrogate and react if none present
        pItem = LoadSurrogate( rStream, nWhich, nSlot, pRefPool );
    else
        // Else skip it
        rStream.SeekRel( sizeof(sal_uInt16) );

    // Is loaded directly (not via surrogate)?
    if ( nWhich && !pItem )
    {
        // bDirekt or not IsPoolable() => Load Item directly
        sal_uInt16 nVersion(0);
        sal_uInt32 nLen(0);
        rStream.ReadUInt16( nVersion ).ReadUInt32( nLen );
        sal_uLong nIStart = rStream.Tell();

        // Load Item directly
        SfxPoolItem *pNewItem =
                pRefPool->GetDefaultItem(nWhich).Create(rStream, nVersion);
        if ( bDontPut )
            pItem = pNewItem;
        else
            if ( pNewItem )
            {
                pItem = &Put(*pNewItem);
                delete pNewItem;
            }
            else
                pItem = nullptr;
        sal_uLong nIEnd = rStream.Tell();
        DBG_ASSERT( nIEnd <= (nIStart+nLen), "read past end of item" );
        if ( (nIStart+nLen) != nIEnd )
            rStream.Seek( nIStart+nLen );
    }

    return pItem;
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
