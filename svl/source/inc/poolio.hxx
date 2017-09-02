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

#include <svl/itempool.hxx>
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

/**
 * This array contains a set of SfxPoolItems, if those items are
 * poolable then each item has a unique set of properties, and we
 * often search linearly to ensure uniqueness. If they are
 * non-poolable we maintain an (often large) list of pointers.
 */
struct SfxPoolItemArray_Impl
{
    typedef std::vector<sal_uInt32> FreeList;
    typedef std::unordered_map<SfxPoolItem*,sal_uInt32> PoolItemPtrToIndexMap;
private:
    std::vector<SfxPoolItem*> maPoolItemVector;
public:
    /// Track list of indices into our array that contain an empty slot
    FreeList maFree;
    /// Hash of SfxPoolItem pointer to index into our array that contains that slot
    PoolItemPtrToIndexMap     maPtrToIndex;

    SfxPoolItemArray_Impl () {}
    SfxPoolItem*& operator[](size_t n) {return maPoolItemVector[n];}
    std::vector<SfxPoolItem*>::iterator begin() {return maPoolItemVector.begin();}
    std::vector<SfxPoolItem*>::iterator end() {return maPoolItemVector.end();}
    /// clear array of PoolItem variants after all PoolItems are deleted
    /// or all ref counts are decreased
    void clear();
    size_t size() const {return maPoolItemVector.size();}
    void push_back(SfxPoolItem* pItem) {maPoolItemVector.push_back(pItem);}

    /// re-build the list of free slots and hash from clean
    void SVL_DLLPUBLIC ReHash();
};

struct SfxItemPool_Impl
{
    SfxBroadcaster                  aBC;
    std::vector<SfxPoolItemArray_Impl*> maPoolItems;
    std::vector<SfxItemPoolUser*>   maSfxItemPoolUsers; /// ObjectUser section
    OUString                        aName;
    std::vector<SfxPoolItem*>       maPoolDefaults;
    std::vector<SfxPoolItem*>*      mpStaticDefaults;
    SfxItemPool*                    mpMaster;
    SfxItemPool*                    mpSecondary;
    sal_uInt16*                     mpPoolRanges;
    sal_uInt16                      mnStart;
    sal_uInt16                      mnEnd;
    MapUnit                         eDefMetric;

    SfxItemPool_Impl( SfxItemPool* pMaster, const OUString& rName, sal_uInt16 nStart, sal_uInt16 nEnd )
        : maPoolItems(nEnd - nStart + 1)
        , aName(rName)
        , maPoolDefaults(nEnd - nStart + 1)
        , mpStaticDefaults(nullptr)
        , mpMaster(pMaster)
        , mpSecondary(nullptr)
        , mpPoolRanges(nullptr)
        , mnStart(nStart)
        , mnEnd(nEnd)
        , eDefMetric(MapUnit::MapCM)
    {
        DBG_ASSERT(mnStart, "Start-Which-Id must be greater 0" );
    }

    ~SfxItemPool_Impl()
    {
        DeleteItems();
    }

    void DeleteItems()
    {
        for (auto pPoolItemArray : maPoolItems)
            delete pPoolItemArray;
        maPoolItems.clear();
        maPoolDefaults.clear();

        delete[] mpPoolRanges;
        mpPoolRanges = nullptr;
    }

    // unit testing
    friend class PoolItemTest;
    static SfxItemPool_Impl *GetImpl(SfxItemPool const *pPool) { return pPool->pImpl.get(); }
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

#endif // INCLUDED_SVL_SOURCE_INC_POOLIO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
