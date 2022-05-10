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

#pragma once

#include "address.hxx"
#include <svl/listener.hxx>

#include <memory>
#include <unordered_map>

class ScDocument;
struct ScSortedRangeCacheMap;

/** Sorted cache for one range used with interpreter functions such as VLOOKUP
    and MATCH. Caches sorted order for cells in the given range, which must
    be one column. This allows faster lookups when cells are not sorted.

    The class has a vector of SCROW items, which is sorted according to values
    of those cells. Therefore e.g. binary search of those cells can be done
    by doing binary search of the vector while mapping the indexes to rows.
 */

class ScSortedRangeCache final : public SvtListener
{
public:
    /// MUST be new'd because Notify() deletes.
    ScSortedRangeCache(ScDocument* pDoc, const ScRange& rRange, bool bDescending,
                       ScSortedRangeCacheMap& cacheMap);

    /// Remove from document structure and delete (!) cache on modify hint.
    virtual void Notify(const SfxHint& rHint) override;

    const ScRange& getRange() const { return maRange; }
    bool isDescending() const { return mDescending; }

    ScSortedRangeCacheMap& getCacheMap() const { return mCacheMap; }

    struct HashKey
    {
        ScRange range;
        bool descending;
        bool operator==(const HashKey& other) const
        {
            return range == other.range && descending == other.descending;
        }
    };
    HashKey getHashKey() const { return { maRange, mDescending }; }

    struct Hash
    {
        size_t operator()(const HashKey& key) const
        {
            // Range should be just one column.
            size_t hash = key.range.hashStartColumn();
            if (key.descending)
                hash = ~hash;
            return hash;
        }
    };

    const std::vector<SCROW>& sortedRows() const { return mSortedRows; }
    size_t size() const { return mSortedRows.size(); }
    size_t indexForRow(SCROW row) const
    {
        std::vector<SCROW>::const_iterator pos
            = std::find(mSortedRows.begin(), mSortedRows.end(), row);
        assert(pos != mSortedRows.end());
        return pos - mSortedRows.begin();
    }
    SCROW rowForIndex(size_t index) const { return mSortedRows[index]; }

private:
    // Rows sorted by their value.
    std::vector<SCROW> mSortedRows;
    ScRange maRange;
    ScDocument* mpDoc;
    ScSortedRangeCacheMap& mCacheMap;
    bool mDescending;

    ScSortedRangeCache(const ScSortedRangeCache&) = delete;
    ScSortedRangeCache& operator=(const ScSortedRangeCache&) = delete;
};

// Struct because including lookupcache.hxx in document.hxx isn't wanted.
struct ScSortedRangeCacheMap
{
    std::unordered_map<ScSortedRangeCache::HashKey, std::unique_ptr<ScSortedRangeCache>,
                       ScSortedRangeCache::Hash>
        aCacheMap;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
