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
#include "queryentry.hxx"
#include <o3tl/hash_combine.hxx>
#include <svl/listener.hxx>

#include <memory>
#include <unordered_map>

class ScDocument;
struct ScInterpreterContext;
struct ScQueryParam;
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
    ScSortedRangeCache(ScDocument* pDoc, const ScRange& rRange, const ScQueryParam& param,
                       ScInterpreterContext* context, bool invalid = false,
                       sal_uInt8 nSortedBinarySearch = 0x00);

    /// Returns if the cache is usable.
    bool isValid() const { return mValid; }

    /// Remove from document structure and delete (!) cache on modify hint.
    virtual void Notify(const SfxHint& rHint) override;

    const ScRange& getRange() const { return maRange; }

    enum class ValueType
    {
        Values,
        StringsCaseSensitive,
        StringsCaseInsensitive
    };
    struct HashKey
    {
        ScRange range;
        ValueType valueType;
        ScQueryOp queryOp;
        ScQueryEntry::QueryType queryType;
        bool operator==(const HashKey& other) const
        {
            return range == other.range && valueType == other.valueType && queryOp == other.queryOp
                   && queryType == other.queryType;
        }
    };
    HashKey getHashKey() const { return { maRange, mValueType, mQueryOp, mQueryType }; }
    static HashKey makeHashKey(const ScRange& range, const ScQueryParam& param);

    struct Hash
    {
        size_t operator()(const HashKey& key) const
        {
            // Range should be just one column.
            size_t hash = key.range.hashStartColumn();
            o3tl::hash_combine(hash, key.valueType);
            o3tl::hash_combine(hash, key.queryOp);
            o3tl::hash_combine(hash, key.queryType);
            return hash;
        }
    };

    /// Returns if the cache values in rows.
    bool isRowSearch() const { return mRowSearch; }

    const std::vector<SCROW>& sortedRows() const { return mSortedRows; }
    size_t indexForRow(SCROW row) const
    {
        assert(row >= maRange.aStart.Row() && row <= maRange.aEnd.Row());
        assert(mRowToIndex[row - maRange.aStart.Row()] != mSortedRows.max_size());
        return mRowToIndex[row - maRange.aStart.Row()];
    }
    SCROW rowForIndex(size_t index) const { return mSortedRows[index]; }

    const std::vector<SCCOLROW>& sortedCols() const { return mSortedCols; }
    size_t indexForCol(SCCOL col) const
    {
        assert(col >= maRange.aStart.Col() && col <= maRange.aEnd.Col());
        assert(mColToIndex[col - maRange.aStart.Col()] != mSortedCols.max_size());
        return mColToIndex[col - maRange.aStart.Col()];
    }
    SCCOL colForIndex(size_t index) const { return mSortedCols[index]; }

private:
    std::vector<SCROW> mSortedRows; // Rows sorted by their value.
    std::vector<SCCOLROW> mSortedCols; // Cols sorted by their value.
    std::vector<size_t> mRowToIndex; // indexed by 'SCROW - maRange.aStart.Row()'
    std::vector<size_t> mColToIndex; // indexed by 'SCCOL - maRange.aStart.Col()'
    ScRange maRange;
    ScDocument* mpDoc;
    bool mValid;
    bool mRowSearch;
    ValueType mValueType;
    ScQueryOp mQueryOp;
    ScQueryEntry::QueryType mQueryType;

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
