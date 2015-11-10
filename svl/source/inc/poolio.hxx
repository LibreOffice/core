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

#ifndef INCLUDED_SVL_SOURCE_INC_POOLIO_HXX
#define INCLUDED_SVL_SOURCE_INC_POOLIO_HXX

#include <svl/SfxBroadcaster.hxx>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

class SfxPoolItem;
class SfxItemPoolUser;

#ifndef DELETEZ
#define DELETEZ(pPtr) { delete pPtr; pPtr = 0; }
#endif

static const sal_uInt32 SFX_ITEMS_DIRECT  = 0xffffffff;
static const sal_uInt32 SFX_ITEMS_NULL    = 0xfffffff0;  // instead StoreSurrogate
static const sal_uInt32 SFX_ITEMS_DEFAULT = 0xfffffffe;

struct SfxPoolVersion_Impl
{
    sal_uInt16          _nVer;
    sal_uInt16          _nStart, _nEnd;
    const sal_uInt16*         _pMap;

                    SfxPoolVersion_Impl( sal_uInt16 nVer, sal_uInt16 nStart, sal_uInt16 nEnd,
                                         const sal_uInt16 *pMap )
                    :   _nVer( nVer ),
                        _nStart( nStart ),
                        _nEnd( nEnd ),
                        _pMap( pMap )
                    {}
                    SfxPoolVersion_Impl( const SfxPoolVersion_Impl &rOrig )
                    :   _nVer( rOrig._nVer ),
                        _nStart( rOrig._nStart ),
                        _nEnd( rOrig._nEnd ),
                        _pMap( rOrig._pMap )
                    {}
};

typedef std::vector<SfxPoolItem*> SfxPoolItemArrayBase_Impl;

typedef std::shared_ptr< SfxPoolVersion_Impl > SfxPoolVersion_ImplPtr;

/**
 * This array contains a set of SfxPoolItems, if those items are
 * poolable then each item has a unique set of properties, and we
 * often search linearly to ensure uniqueness. If they are
 * non-poolable we maintain an (often large) list of pointers.
 */
struct SfxPoolItemArray_Impl: public SfxPoolItemArrayBase_Impl
{
    typedef std::vector<sal_uInt32> FreeList;
    typedef std::unordered_map<SfxPoolItem*,sal_uInt32> PoolItemPtrToIndexMap;

public:
    /// Track list of indices into our array that contain an empty slot
    FreeList maFree;
    /// Hash of SfxPoolItem pointer to index into our array that contains that slot
    PoolItemPtrToIndexMap     maPtrToIndex;

    SfxPoolItemArray_Impl () {}

    /// re-build the list of free slots and hash from clean
    void SVL_DLLPUBLIC ReHash();
};

struct SfxItemPool_Impl
{
    SfxBroadcaster                  aBC;
    std::vector<SfxPoolItemArray_Impl*> maPoolItems;
    std::vector<SfxItemPoolUser*>   maSfxItemPoolUsers; /// ObjectUser section
    OUString                        aName;
    SfxPoolItem**                   ppPoolDefaults;
    SfxPoolItem**                   ppStaticDefaults;
    SfxItemPool*                    mpMaster;
    SfxItemPool*                    mpSecondary;
    sal_uInt16*                     mpPoolRanges;
    std::deque< SfxPoolVersion_ImplPtr > aVersions;
    sal_uInt16                      mnStart;
    sal_uInt16                      mnEnd;
    sal_uInt16                      mnFileFormatVersion;
    sal_uInt16                      nVersion;
    sal_uInt16                      nLoadingVersion;
    sal_uInt16                      nInitRefCount; // 1, during load, may be 2
    sal_uInt16                      nVerStart, nVerEnd; // WhichRange in versions
    sal_uInt16                      nStoringStart, nStoringEnd; // Range to be saved
    sal_uInt8                       nMajorVer, nMinorVer; // The Pool itself
    SfxMapUnit                      eDefMetric;
    bool                            bInSetItem;
    bool                            bStreaming; // in Load() or Store()
    bool                            mbPersistentRefCounts;

    SfxItemPool_Impl( SfxItemPool* pMaster, const OUString& rName, sal_uInt16 nStart, sal_uInt16 nEnd )
        : maPoolItems(nEnd - nStart + 1, static_cast<SfxPoolItemArray_Impl*>(nullptr))
        , aName(rName)
        , ppPoolDefaults(new SfxPoolItem* [nEnd - nStart + 1])
        , ppStaticDefaults(nullptr)
        , mpMaster(pMaster)
        , mpSecondary(nullptr)
        , mpPoolRanges(nullptr)
        , mnStart(nStart)
        , mnEnd(nEnd)
        , mnFileFormatVersion(0)
        , nVersion(0)
        , nLoadingVersion(0)
        , nInitRefCount(0)
        , nVerStart(0)
        , nVerEnd(0)
        , nStoringStart(0)
        , nStoringEnd(0)
        , nMajorVer(0)
        , nMinorVer(0)
        , eDefMetric(SFX_MAPUNIT_CM)
        , bInSetItem(false)
        , bStreaming(false)
        , mbPersistentRefCounts(false)
    {
        DBG_ASSERT(mnStart, "Start-Which-Id must be greater 0" );
        memset( ppPoolDefaults, 0, sizeof( SfxPoolItem* ) * (nEnd - nStart + 1));
    }

    ~SfxItemPool_Impl()
    {
        DeleteItems();
    }

    void DeleteItems()
    {
        std::vector<SfxPoolItemArray_Impl*>::iterator itr = maPoolItems.begin(), itrEnd = maPoolItems.end();
        for (; itr != itrEnd; ++itr)
            delete *itr;
        maPoolItems.clear();

        delete[] mpPoolRanges;
        mpPoolRanges = nullptr;
        delete[] ppPoolDefaults;
        ppPoolDefaults = nullptr;
    }

    void readTheItems(SvStream & rStream, sal_uInt32 nCount, sal_uInt16 nVersion,
                      SfxPoolItem * pDefItem, SfxPoolItemArray_Impl ** pArr);

    // unit testing
    friend class PoolItemTest;
    static SfxItemPool_Impl *GetImpl(SfxItemPool *pPool) { return pPool->pImp; }
};


#define SFX_ITEMPOOL_VER_MAJOR          sal_uInt8(2)
#define SFX_ITEMPOOL_VER_MINOR          sal_uInt8(0)

#define SFX_ITEMPOOL_TAG_STARTPOOL_4    sal_uInt16(0x1111)
#define SFX_ITEMPOOL_TAG_STARTPOOL_5    sal_uInt16(0xBBBB)
#define SFX_ITEMPOOL_TAG_TRICK4OLD      sal_uInt16(0xFFFF)

#define SFX_ITEMPOOL_REC                sal_uInt8(0x01)
#define SFX_ITEMPOOL_REC_HEADER         sal_uInt8(0x10)
#define SFX_ITEMPOOL_REC_VERSIONMAP     sal_uInt16(0x0020)
#define SFX_ITEMPOOL_REC_WHICHIDS       sal_uInt16(0x0030)
#define SFX_ITEMPOOL_REC_ITEMS          sal_uInt16(0x0040)
#define SFX_ITEMPOOL_REC_DEFAULTS       sal_uInt16(0x0050)



/** Read in a Unicode string from a streamed byte string representation.

    @param rStream  Some (input) stream.  Its Stream/TargetCharSets must
    be set to correct values!

    @return  On success, returns the reconstructed Unicode string.
 */
OUString readByteString(SvStream & rStream);

/** Write a byte string representation of a Unicode string into a stream.

    @param rStream  Some (output) stream.  Its Stream/TargetCharSets must
    be set to correct values!

    @param rString  Some Unicode string.
 */
void writeByteString(SvStream & rStream, const OUString& rString);

/** Read in a Unicode string from either a streamed Unicode or byte string
    representation.

    @param rStream  Some (input) stream.  If bUnicode is false, its
    Stream/TargetCharSets must be set to correct values!

    @param bUnicode  Whether to read in a stream Unicode (true) or byte
    string (false) representation.

    @return          On success, returns the reconstructed Unicode string.
 */
OUString readUnicodeString(SvStream & rStream, bool bUnicode);

/** Write a Unicode string representation of a Unicode string into a
    stream.

    @param rStream  Some (output) stream.

    @param rString  Some Unicode string.
 */
void writeUnicodeString(SvStream & rStream, const OUString& rString);


#endif // INCLUDED_SVL_SOURCE_INC_POOLIO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
