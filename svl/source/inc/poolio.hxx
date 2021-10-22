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

#include <rtl/ref.hxx>
#include <svl/itempool.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <tools/debug.hxx>
#include <memory>
#include <o3tl/sorted_vector.hxx>

class SfxPoolItem;
class SfxItemPoolUser;

const sal_uInt32 SFX_ITEMS_DEFAULT = 0xfffffffe;

static bool CompareSortablePoolItems(SfxPoolItem const* lhs, SfxPoolItem const* rhs)
{
    return (*lhs) < (*rhs);
}
/**
 * This array contains a set of SfxPoolItems, if those items are
 * poolable then each item has a unique set of properties, and we
 * often search linearly to ensure uniqueness. If they are
 * non-poolable we maintain an (often large) list of pointers.
 */
struct SfxPoolItemArray_Impl
{
private:
    o3tl::sorted_vector<SfxPoolItem*> maPoolItemSet;
    // In some cases, e.g. subclasses of NameOrIndex, the parent class (NameOrIndex) is sortable,
    // but the subclasses do not define an operator<, which means that we don't get an ordering
    // strong enough to enforce uniqueness purely with operator<, which means we need to do
    // a partial scan with operator==
    std::vector<SfxPoolItem*> maSortablePoolItems;
public:
    o3tl::sorted_vector<SfxPoolItem*>::const_iterator begin() const { return maPoolItemSet.begin(); }
    o3tl::sorted_vector<SfxPoolItem*>::const_iterator end() const { return maPoolItemSet.end(); }
    /// clear array of PoolItem variants after all PoolItems are deleted
    /// or all ref counts are decreased
    void clear();
    size_t size() const {return maPoolItemSet.size();}
    bool empty() const {return maPoolItemSet.empty();}
    o3tl::sorted_vector<SfxPoolItem*>::const_iterator find(SfxPoolItem* pItem) const { return maPoolItemSet.find(pItem); }
    void insert(SfxPoolItem* pItem)
    {
        bool bInserted = maPoolItemSet.insert(pItem).second;
        assert( bInserted && "duplicate item?" );
        (void)bInserted;

        if (pItem->IsSortable())
        {
            // bail early if someone modified one of these things underneath me
            assert( std::is_sorted_until(maSortablePoolItems.begin(), maSortablePoolItems.end(), CompareSortablePoolItems) == maSortablePoolItems.end());

            auto it = std::lower_bound(maSortablePoolItems.begin(), maSortablePoolItems.end(), pItem, CompareSortablePoolItems);
            maSortablePoolItems.insert(maSortablePoolItems.begin() + (it - maSortablePoolItems.begin()), pItem);
        }
    }
    const SfxPoolItem* findByLessThan(const SfxPoolItem* pNeedle) const
    {
        // bail early if someone modified one of these things underneath me
        assert( std::is_sorted_until(maSortablePoolItems.begin(), maSortablePoolItems.end(), CompareSortablePoolItems) == maSortablePoolItems.end());
        assert( maPoolItemSet.empty() || maPoolItemSet.front()->IsSortable() );

        auto it = std::lower_bound(maSortablePoolItems.begin(), maSortablePoolItems.end(), pNeedle, CompareSortablePoolItems);
        for (;;)
        {
            if (it == maSortablePoolItems.end())
                return nullptr;
            if (**it < *pNeedle)
                return nullptr;
            if (*pNeedle == **it)
                return *it;
            ++it;
        }
    }
    std::vector<const SfxPoolItem*> findSurrogateRange(const SfxPoolItem* pNeedle) const
    {
        std::vector<const SfxPoolItem*> rv;
        if (!maSortablePoolItems.empty())
        {
            // bail early if someone modified one of these things underneath me
            assert( std::is_sorted_until(maSortablePoolItems.begin(), maSortablePoolItems.end(), CompareSortablePoolItems) == maSortablePoolItems.end());

            auto range = std::equal_range(maSortablePoolItems.begin(), maSortablePoolItems.end(), pNeedle, CompareSortablePoolItems);
            rv.reserve(std::distance(range.first, range.second));
            for (auto it = range.first; it != range.second; ++it)
                rv.push_back(*it);
        }
        else
        {
            for (const SfxPoolItem* p : maPoolItemSet)
                if (*pNeedle == *p)
                    rv.push_back(p);
        }
        return rv;
    }
    void erase(o3tl::sorted_vector<SfxPoolItem*>::const_iterator it)
    {
        SfxPoolItem* pNeedle = *it;
        if ((*it)->IsSortable())
        {
            // bail early if someone modified one of these things underneath me
            assert( std::is_sorted_until(maSortablePoolItems.begin(), maSortablePoolItems.end(), CompareSortablePoolItems) == maSortablePoolItems.end());

            auto sortIt = std::lower_bound(maSortablePoolItems.begin(), maSortablePoolItems.end(), pNeedle, CompareSortablePoolItems);
            for (;;)
            {
                if (sortIt == maSortablePoolItems.end())
                {
                    assert(false && "did not find item?");
                    break;
                }
                if (**sortIt < *pNeedle)
                {
                    assert(false && "did not find item?");
                    break;
                }
                // need to compare by pointer here, since we might have duplicates
                if (*sortIt == pNeedle)
                {
                    maSortablePoolItems.erase(sortIt);
                    break;
                }
                ++sortIt;
            }
        }
        maPoolItemSet.erase(it);
    }
};

struct SfxItemPool_Impl
{
    SfxBroadcaster                  aBC;
    std::vector<SfxPoolItemArray_Impl> maPoolItemArrays;
    OUString                        aName;
    std::vector<SfxPoolItem*>       maPoolDefaults;
    std::vector<SfxPoolItem*>*      mpStaticDefaults;
    SfxItemPool*                    mpMaster;
    rtl::Reference<SfxItemPool>     mpSecondary;
    WhichRangesContainer            mpPoolRanges;
    sal_uInt16                      mnStart;
    sal_uInt16                      mnEnd;
    MapUnit                         eDefMetric;

    SfxItemPool_Impl( SfxItemPool* pMaster, const OUString& rName, sal_uInt16 nStart, sal_uInt16 nEnd )
        : maPoolItemArrays(nEnd - nStart + 1)
        , aName(rName)
        , maPoolDefaults(nEnd - nStart + 1)
        , mpStaticDefaults(nullptr)
        , mpMaster(pMaster)
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
        maPoolItemArrays.clear();
        maPoolDefaults.clear();
        mpPoolRanges.reset();
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
