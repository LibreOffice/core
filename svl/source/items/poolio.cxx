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

static sal_uInt16 convertSfxItemKindToUInt16(SfxItemKind eKind)
{
    switch (eKind)
    {
    case SfxItemKind::NONE:
        return 0;
    case SfxItemKind::DeleteOnIdle:
        return 0xfffd;
    case SfxItemKind::StaticDefault:
        return 0xfffe;
    case SfxItemKind::PoolDefault:
        return 0xffff;
    default:
        assert("unknown SfxItemKind");
        return 0;
    }
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
 * The SfxItemPool is saved to the specified Stream (together with all its
 * secondary Pools) using its Pool Defaults and pooled Items.
 * The static defaults are not saved.
 * [Fileformat]
 *
 *  ;First, a compatibility header section
 *    Start:      0x1111  SFX_ITEMPOOL_TAG_STARTPOOLS(_4/_5)
 *                sal_uInt8   MAJOR_VER                   ;SfxItemPool version
 *               sal_uInt8   MINOR_VER                   ;"
 *               0xFFFF  SFX_ITEMPOOL_TAG_TRICK4OLD      ;ex. GetVersion()
 *               sal_uInt16  0x0000                      ;Pseudo StyleSheetPool
 *               sal_uInt16  0x0000                      ;Pseudo StyleSheetPool
 *
 *   ;The whole Pool into a record
 *              record  SfxMiniRecod(SFX_ITEMPOOL_REC)
 *
 *   ;Start with a Header for each
 *   Header:     record      SfxMiniRecord(SFX_ITEMPOOL_REC_HEADER)
 *               sal_uInt16          GetVersion()        ;Which-Ranges etc.
 *               String          GetName()               ;Pool name
 *
 *  ;The version map: in order to be able to map WhichIds of new file version
 *    Versions:   record      SfxMultiRecord(SFX_ITEMPOOL_REC_VERSIONS, 0)
 *               sal_uInt16          OldVersion
 *               sal_uInt16          OldStartWhich
 *               sal_uInt16          OldEndWhich
 *               sal_uInt16[]        NewWhich (OldEndWhich-OldStartWhich+1)
 *
 *   ;Now the pooled Items (first the non-SfxSetItems)
 *   Items:      record      SfxMultiRecord(SFX_ITEMPOOL_REC_WHICHIDS, 0)
 *                content         SlotId, 0
 *               sal_uInt16          WhichId
 *               sal_uInt16          pItem->GetVersion()
 *               sal_uInt16          Array-Size
 *               record          SfxMultiRecord(SFX_, 0)
 *               content             Surrogate
 *               sal_uInt16              RefCount
 *               unknown             pItem->Store()
 *
 *   ;Now the set Pool defaults
 *   Defaults:   record      SfxMultiRecord(SFX_ITEMPOOL_REC_DEFAULTS, 0)
 *               content         SlotId, 0
 *               sal_uInt16          WhichId
 *               sal_uInt16          pPoolDef->GetVersion()
 *               unknown         pPoolDef->Store();
 *
 *   ;Hereafter the secondary follows (if present) without compatibility header section
 */
SvStream &SfxItemPool::Store(SvStream &rStream) const
{
    // Find StoreMaster
    SfxItemPool *pStoreMaster = pImpl->mpMaster != this ? pImpl->mpMaster : nullptr;
    while ( pStoreMaster && !pStoreMaster->pImpl->bStreaming )
        pStoreMaster = pStoreMaster->pImpl->mpSecondary;

    // Old header (version of the Pool and content version is 0xffff by default)
    pImpl->bStreaming = true;
    if ( !pStoreMaster )
    {
        rStream.WriteUInt16( rStream.GetVersion() >= SOFFICE_FILEFORMAT_50
                ? SFX_ITEMPOOL_TAG_STARTPOOL_5
                : SFX_ITEMPOOL_TAG_STARTPOOL_4  );
        rStream.WriteUInt8( SFX_ITEMPOOL_VER_MAJOR ).WriteUInt8( SFX_ITEMPOOL_VER_MINOR );
        rStream.WriteUInt16( SFX_ITEMPOOL_TAG_TRICK4OLD );

        // Work around SfxStyleSheet bug
        rStream.WriteUInt16( 0 ); // Version
        rStream.WriteUInt16( 0 ); // Count (or else 2nd loop breaks)
    }

    // Every Pool as a whole is a record
    SfxMiniRecordWriter aPoolRec( &rStream, SFX_ITEMPOOL_REC );
    pStoringPool_ = this;

    // Single header (content version and name)
    {
        SfxMiniRecordWriter aPoolHeaderRec( &rStream, SFX_ITEMPOOL_REC_HEADER);
        rStream.WriteUInt16( pImpl->nVersion );
        writeByteString(rStream, pImpl->aName);
    }

    // VersionMaps
    {
        SfxMultiVarRecordWriter aVerRec( &rStream, SFX_ITEMPOOL_REC_VERSIONMAP );
        for (std::shared_ptr<SfxPoolVersion_Impl>& pVer : pImpl->aVersions)
        {
            aVerRec.NewContent();
            rStream.WriteUInt16( pVer->_nVer ).WriteUInt16( pVer->_nStart ).WriteUInt16( pVer->_nEnd );
            sal_uInt16 nCount = pVer->_nEnd - pVer->_nStart + 1;
            sal_uInt16 nNewWhich = 0;
            for ( sal_uInt16 n = 0; n < nCount; ++n )
            {
                nNewWhich = pVer->_pMap[n];
                rStream.WriteUInt16( nNewWhich );
            }

            // Workaround for bug in SetVersionMap 312
            if ( SOFFICE_FILEFORMAT_31 == pImpl->mnFileFormatVersion )
                rStream.WriteUInt16( nNewWhich+1 );
        }
    }

    // Pooled Items
    {
        SfxMultiMixRecordWriter aWhichIdsRec( &rStream, SFX_ITEMPOOL_REC_WHICHIDS );

        // First write the atomic Items and then write the Sets (important when loading)
        for (int ft = 0 ; ft < 2 && !rStream.GetError(); ft++)
        {
            pImpl->bInSetItem = ft != 0;

            std::vector<SfxPoolItemArray_Impl*>::const_iterator itrArr = pImpl->maPoolItems.begin();
            std::vector<SfxPoolItem*> & rStaticDefaults = *pImpl->mpStaticDefaults;
            const sal_uInt16 nSize = GetSize_Impl();
            for ( size_t i = 0; i < nSize && !rStream.GetError(); ++i, ++itrArr )
            {
                // Get version of the Item
                sal_uInt16 nItemVersion = rStaticDefaults[i]->GetVersion( pImpl->mnFileFormatVersion );
                if ( USHRT_MAX == nItemVersion )
                    // => Was not present in the version that was supposed to be exported
                    continue;

                // ! Poolable is not even saved in the Pool
                // And itemsets/plain-items depending on the round
                if ( *itrArr && IsItemPoolable(*rStaticDefaults[i]) &&
                     pImpl->bInSetItem == (dynamic_cast< const SfxSetItem* >(rStaticDefaults[i]) != nullptr) )
                {
                    // Own signature, global WhichId and ItemVersion
                    sal_uInt16 nSlotId = GetSlotId( rStaticDefaults[i]->Which(), false );
                    aWhichIdsRec.NewContent(nSlotId, 0);
                    rStream.WriteUInt16( rStaticDefaults[i]->Which() );
                    rStream.WriteUInt16( nItemVersion );
                    const sal_uInt32 nCount = ::std::min<size_t>( (*itrArr)->size(), SAL_MAX_UINT32 );
                    DBG_ASSERT(nCount, "ItemArr is empty");
                    rStream.WriteUInt32( nCount );

                    // Write Items
                    SfxMultiMixRecordWriter aItemsRec( &rStream, SFX_ITEMPOOL_REC_ITEMS );
                    for ( size_t j = 0; j < nCount; ++j )
                    {
                        // Get Item
                        const SfxPoolItem *pItem = (*itrArr)->operator[](j);
                        if ( pItem && pItem->GetRefCount() ) //! See other MI-REF
                        {
                            aItemsRec.NewContent((sal_uInt16)j, 'X' );

                            if ( pItem->GetRefCount() == SFX_ITEMS_SPECIAL )
                                rStream.WriteUInt16( convertSfxItemKindToUInt16(pItem->GetKind()) );
                            else
                            {
                                if( pItem->GetRefCount() > SFX_ITEMS_OLD_MAXREF )
                                {
                                    assert(!"refcount does not fit into 16-bits");
                                    rStream.SetError( ERRCODE_IO_NOTSTORABLEINBINARYFORMAT );
                                }
                                else
                                    rStream.WriteUInt16( pItem->GetRefCount() );
                            }

                            if ( !rStream.GetError() )
                                pItem->Store(rStream, nItemVersion);
                            else
                                break;
                        }
                    }
                }
            }
        }

        pImpl->bInSetItem = false;
    }

    // Save the set Defaults (PoolDefaults)
    if ( !rStream.GetError() )
    {
        SfxMultiMixRecordWriter aDefsRec( &rStream, SFX_ITEMPOOL_REC_DEFAULTS );
        for (const SfxPoolItem* pDefaultItem : pImpl->maPoolDefaults)
        {
            if ( pDefaultItem )
            {
                // Get version
                sal_uInt16 nItemVersion = pDefaultItem->GetVersion( pImpl->mnFileFormatVersion );
                if ( USHRT_MAX == nItemVersion )
                    // => Was not present in the version yet
                    continue;

                // Own signature, global signature, version
                sal_uInt16 nSlotId = GetSlotId( pDefaultItem->Which(), false );
                aDefsRec.NewContent( nSlotId, 0 );
                rStream.WriteUInt16( pDefaultItem->Which() );
                rStream.WriteUInt16( nItemVersion );

                // Item
                pDefaultItem->Store( rStream, nItemVersion );
            }
        }
    }

    // Write out additional Pools
    pStoringPool_ = nullptr;
    aPoolRec.Close();
    if ( !rStream.GetError() && pImpl->mpSecondary )
        pImpl->mpSecondary->Store( rStream );

    pImpl->bStreaming = false;
    return rStream;
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
    SfxPoolItem * pDefItem, SfxPoolItemArray_Impl ** ppArr)
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

SvStream &SfxItemPool::Load(SvStream &rStream)
{
    DBG_ASSERT(pImpl->mpStaticDefaults, "No DefaultArray");

    // Protect items by increasing ref count
    if ( !pImpl->mbPersistentRefCounts )
    {

        // Iterate over all Which values
        for(auto const& rArrayPtr : pImpl->maPoolItems)
        {
            // Is there an Item with that Which value present at all?
            if (rArrayPtr)
            {
                for (auto const& rItemPtr : *rArrayPtr)
                    if (rItemPtr)
                    {
                        SAL_INFO( "svl.items", "loading non-empty ItemPool" );
                        AddRef(*rItemPtr);
                    }
            }
        }

        // During loading (until LoadCompleted()) protect all items
        pImpl->nInitRefCount = 2;
    }

    // Find LoadMaster
    SfxItemPool *pLoadMaster = pImpl->mpMaster != this ? pImpl->mpMaster : nullptr;
    while ( pLoadMaster && !pLoadMaster->pImpl->bStreaming )
        pLoadMaster = pLoadMaster->pImpl->mpSecondary;

    // Read whole Header
    pImpl->bStreaming = true;
    if ( !pLoadMaster )
    {
        // Load format version
        sal_uInt16 nFileTag;
        rStream.ReadUInt16( nFileTag );
        if ( SFX_ITEMPOOL_TAG_STARTPOOL_5 != nFileTag && SFX_ITEMPOOL_TAG_STARTPOOL_4 != nFileTag )
        {
            OSL_FAIL( "SFX_ITEMPOOL_TAG_STARTPOOL_5" ); /*! s.u. */
            /*! Set error code and evaluate! */
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            pImpl->bStreaming = false;
            return rStream;
        }

        rStream.ReadUChar( pImpl->nMajorVer ).ReadUChar( pImpl->nMinorVer );

        // Take over format version to MasterPool
        pImpl->mpMaster->pImpl->nMajorVer = pImpl->nMajorVer;
        pImpl->mpMaster->pImpl->nMinorVer = pImpl->nMinorVer;

        // Unknown Format
        if (pImpl->nMajorVer < 2 || pImpl->nMajorVer > SFX_ITEMPOOL_VER_MAJOR)
        {
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            pImpl->bStreaming = false;
            return rStream;
        }

        // Trick for version 1.2: skip data
        sal_uInt16 nFileTag2;
        rStream.ReadUInt16( nFileTag2 );
        if ( SFX_ITEMPOOL_TAG_TRICK4OLD != nFileTag2 )
        {
            OSL_FAIL( "SFX_ITEMPOOL_TAG_TRICK4OLD" ); /*! s.u. */
            /*! Set error code and evaluate! */
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            pImpl->bStreaming = false;
            return rStream;
        }

        rStream.SeekRel( 4 ); // Hack: Skip data due to SfxStyleSheetPool bug
    }

    // New record-oriented format
    SfxMiniRecordReader aPoolRec( &rStream, SFX_ITEMPOOL_REC );
    if ( rStream.GetError() )
    {
        pImpl->bStreaming = false;
        return rStream;
    }

    // Single header
    OUString aExternName;
    {
        // Find HeaderRecord
        SfxMiniRecordReader aPoolHeaderRec( &rStream, SFX_ITEMPOOL_REC_HEADER );
        if ( rStream.GetError() )
        {
            pImpl->bStreaming = false;
            return rStream;
        }

        // Read Header
        rStream.ReadUInt16( pImpl->nLoadingVersion );
        aExternName = readByteString(rStream);
        bool bOwnPool = aExternName == pImpl->aName;

        //! As long as we cannot read foreign Pools
        if ( !bOwnPool )
        {
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            aPoolRec.Skip();
            pImpl->bStreaming = false;
            return rStream;
        }
    }

    // Version maps
    {
        SfxMultiRecordReader aVerRec( &rStream, SFX_ITEMPOOL_REC_VERSIONMAP );
        if ( rStream.GetError() )
        {
            pImpl->bStreaming = false;
            return rStream;
        }

        // Version maps einlesen
        sal_uInt16 nOwnVersion = pImpl->nVersion;
        for ( sal_uInt16 nVerNo = 0; aVerRec.GetContent(); ++nVerNo )
        {
            // Read header for single versions
            sal_uInt16 nVersion(0), nHStart(0), nHEnd(0);
            rStream.ReadUInt16( nVersion ).ReadUInt16( nHStart ).ReadUInt16( nHEnd );
            sal_uInt16 nCount = nHEnd - nHStart + 1;

            // Is new version is known?
            if ( nVerNo >= pImpl->aVersions.size() )
            {
                // Add new Version
                const size_t nMaxRecords = rStream.remainingSize() / sizeof(sal_uInt16);
                if (nCount > nMaxRecords)
                {
                    SAL_WARN("svl.items", "Parsing error: " << nMaxRecords <<
                             " max possible entries, but " << nCount << " claimed, truncating");
                    nCount = nMaxRecords;
                }
                sal_uInt16 *pMap = new sal_uInt16[nCount]{};
                for ( sal_uInt16 n = 0; n < nCount; ++n )
                    rStream.ReadUInt16( pMap[n] );
                SetVersionMap( nVersion, nHStart, nHEnd, pMap );
            }
        }
        pImpl->nVersion = nOwnVersion;
    }

    // Load Items
    bool bSecondaryLoaded = false;
    long nSecondaryEnd = 0;
    {
        SfxMultiRecordReader aWhichIdsRec( &rStream, SFX_ITEMPOOL_REC_WHICHIDS);
        while ( aWhichIdsRec.GetContent() )
        {
            // Get SlotId, WhichId and Item version
            sal_uInt32 nCount(0);
            sal_uInt16 nVersion(0), nWhich(0);
            //!sal_uInt16 nSlotId = aWhichIdsRec.GetContentTag();
            rStream.ReadUInt16( nWhich );
            if ( pImpl->nLoadingVersion != pImpl->nVersion )
                // Move WhichId from file version to Pool version
                nWhich = GetNewWhich( nWhich );

            // Unknown Item from newer version
            if ( !IsInRange(nWhich) )
                continue;

            rStream.ReadUInt16( nVersion );
            rStream.ReadUInt32( nCount );
            //!SFX_ASSERTWARNING( !nSlotId || !HasMap() ||
            //!         ( nSlotId == GetSlotId( nWhich, sal_False ) ) ||
            //!         !GetSlotId( nWhich, sal_False ),
            //!         nWhich, "Slot/Which mismatch" );

            sal_uInt16 nIndex = GetIndex_Impl(nWhich);
            SfxPoolItemArray_Impl **ppArr = &pImpl->maPoolItems[0] + nIndex;

            // SfxSetItems could contain Items from secondary Pools
            SfxPoolItem *pDefItem = (*pImpl->mpStaticDefaults)[nIndex];
            pImpl->bInSetItem = dynamic_cast<const SfxSetItem*>( pDefItem ) !=  nullptr;
            if ( !bSecondaryLoaded && pImpl->mpSecondary && pImpl->bInSetItem )
            {
                // Seek to end of own Pool
                sal_uLong nLastPos = rStream.Tell();
                aPoolRec.Skip();

                // Read secondary Pool
                pImpl->mpSecondary->Load( rStream );
                bSecondaryLoaded = true;
                nSecondaryEnd = rStream.Tell();

                // Back to our own Items
                rStream.Seek(nLastPos);
            }

            // Read Items
            pImpl->readTheItems(rStream, nCount, nVersion, pDefItem, ppArr);

            pImpl->bInSetItem = false;
        }
    }

    // Read Pool defaults
    {
        SfxMultiRecordReader aDefsRec( &rStream, SFX_ITEMPOOL_REC_DEFAULTS );

        while ( aDefsRec.GetContent() )
        {
            // Get SlotId, WhichId and Item versions
            sal_uInt16 nVersion(0), nWhich(0);
            //!sal_uInt16 nSlotId = aDefsRec.GetContentTag();
            rStream.ReadUInt16( nWhich );
            if ( pImpl->nLoadingVersion != pImpl->nVersion )
                // Move WhichId from file version to Pool version
                nWhich = GetNewWhich( nWhich );

            // Unknown Item from newer version
            if ( !IsInRange(nWhich) )
                continue;

            rStream.ReadUInt16( nVersion );
            //!SFX_ASSERTWARNING( !HasMap() || ( nSlotId == GetSlotId( nWhich, sal_False ) ),
            //!         nWhich, "Slot/Which mismatch" );

            // Load PoolDefaultItem
            sal_uInt16 nIndex = GetIndex_Impl(nWhich);
            SfxPoolItem *pItem = (*pImpl->mpStaticDefaults)[ nIndex ]->Create( rStream, nVersion );
            pItem->SetKind( SfxItemKind::PoolDefault );
            pImpl->maPoolDefaults[nIndex] = pItem;
        }
    }

    // Load secondary Pool if needed
    aPoolRec.Skip();
    if ( pImpl->mpSecondary )
    {
        if ( !bSecondaryLoaded )
            pImpl->mpSecondary->Load( rStream );
        else
            rStream.Seek( nSecondaryEnd );
    }

    // If not own Pool, then no name
    if ( aExternName != pImpl->aName )
        (pImpl->aName).clear();

    pImpl->bStreaming = false;
    return rStream;
};

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
    pImpl->aVersions.push_back( pVerMap );

    DBG_ASSERT( nVer > pImpl->nVersion, "Versions not sorted" );
    pImpl->nVersion = nVer;

    // Adapt version range
    for ( sal_uInt16 n = 0; n < nOldEnd-nOldStart+1; ++n )
    {
        sal_uInt16 nWhich = pOldWhichIdTab[n];
        if ( nWhich < pImpl->nVerStart )
        {
            if ( !nWhich )
                nWhich = 0;
            pImpl->nVerStart = nWhich;
        }
        else if ( nWhich > pImpl->nVerEnd )
            pImpl->nVerEnd = nWhich;
    }
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
    // Determine (secondary) Pool
    if ( !IsInVersionsRange(nFileWhich) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetNewWhich( nFileWhich );
        SAL_WARN( "svl.items", "unknown which in GetNewWhich(), with ID/pos " << nFileWhich);
    }

    // Newer/the same/older version?
    short nDiff = (short)pImpl->nLoadingVersion - (short)pImpl->nVersion;

    // WhichId of a newer version?
    if ( nDiff > 0 )
    {
        // Map step by step from the top version down to the file version
        for ( size_t nMap = pImpl->aVersions.size(); nMap > 0; --nMap )
        {
            SfxPoolVersion_ImplPtr pVerInfo = pImpl->aVersions[nMap-1];
            if ( pVerInfo->_nVer > pImpl->nVersion )
            {   sal_uInt16 nOfs;
                sal_uInt16 nCount = pVerInfo->_nEnd - pVerInfo->_nStart + 1;
                for ( nOfs = 0;
                      nOfs <= nCount &&
                        pVerInfo->_pMap[nOfs] != nFileWhich;
                      ++nOfs )
                    continue;

                if ( pVerInfo->_pMap[nOfs] == nFileWhich )
                    nFileWhich = pVerInfo->_nStart + nOfs;
                else
                    return 0;
            }
            else
                break;
        }
    }

    // WhichId of a newer version?
    else if ( nDiff < 0 )
    {
        // Map step by step from the top version down to the file version
        for (std::shared_ptr<SfxPoolVersion_Impl>& pVerInfo : pImpl->aVersions)
        {
            if ( pVerInfo->_nVer > pImpl->nLoadingVersion )
            {
                if (nFileWhich >= pVerInfo->_nStart &&
                            nFileWhich <= pVerInfo->_nEnd)
                {
                    nFileWhich = pVerInfo->_pMap[nFileWhich - pVerInfo->_nStart];
                }
                else
                {
                    SAL_WARN("svl.items", "which-id unknown in version");
                }
            }
        }
    }

    // Return original (nDiff==0) or mapped (nDiff!=0) Id
    return nFileWhich;
}


bool SfxItemPool::IsInVersionsRange( sal_uInt16 nWhich ) const
{
    return nWhich >= pImpl->nVerStart && nWhich <= pImpl->nVerEnd;
}


/**
 * This method determines whether the loaded Pool version corresponds to the
 * currently loaded Pool structure.
 *
 * Precondition:   Pool is loaded
 * Postcondition:  Unchanged
 * Runtime:        linear(Count of secondary pools)
 *
 * @see SfxItemPool::SetVersionMap(sal_uInt16,sal_uInt16,sal_uInt16,sal_uInt16*)
 * @see SfxItemPool::GetNewWhich(sal_uInt16) const
 * @see SfxItemPool::GetVersion() const
 */
bool SfxItemPool::IsCurrentVersionLoading() const
{
    return ( pImpl->nVersion == pImpl->nLoadingVersion ) &&
           ( !pImpl->mpSecondary || pImpl->mpSecondary->IsCurrentVersionLoading() );
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

    // Find right secondary Pool
    while ( !pRefPool->IsInVersionsRange(nWhich) )
    {
        if ( pRefPool->pImpl->mpSecondary )
            pRefPool = pRefPool->pImpl->mpSecondary;
        else
        {
            // WID not present in this version => skip
            sal_uInt32 nSurro(0);
            sal_uInt16 nVersion(0), nLen(0);
            rStream.ReadUInt32( nSurro );
            if ( SFX_ITEMS_DIRECT == nSurro )
            {
                rStream.ReadUInt16( nVersion ).ReadUInt16( nLen );
                rStream.SeekRel( nLen );
            }
            return nullptr;
        }
    }

    // Are we loading a different version?
    bool bCurVersion = pRefPool->IsCurrentVersionLoading();
    if ( !bCurVersion )
        nWhich = pRefPool->GetNewWhich( nWhich ); // Map WhichId to new version

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
