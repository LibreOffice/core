/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <sal/types.h>
#include <svl/itemset.hxx>

#include <memory>
#include <utility>
#include <vector>

namespace svl::detail
{
/**
 * Determines the number of sal_uInt16s in a span pairs of
 * sal_uInt16s, each representing a range of sal_uInt16s, and total capacity of the ranges.
 */
inline sal_uInt16 CountRanges(const WhichRangesContainer& pRanges)
{
    sal_uInt16 nCapacity = 0;
    for (const auto& rPair : pRanges)
    {
        nCapacity += rangeSize(rPair.first, rPair.second);
    }
    return nCapacity;
}

// Adds a range to which ranges, keeping the ranges in valid state (sorted, non-overlapping)
inline WhichRangesContainer MergeRange(const WhichRangesContainer& pWhichRanges, sal_uInt16 nFrom,
                                       sal_uInt16 nTo)
{
    assert(validRange(nFrom, nTo));

    if (pWhichRanges.empty())
    {
        return WhichRangesContainer(nFrom, nTo);
    }

    //    assert(validRanges(pWhichRanges));

    // create vector of ranges (sal_uInt16 pairs of lower and upper bound)
    const size_t nOldCount = pWhichRanges.size();
    std::vector<WhichPair> aRangesTable;
    aRangesTable.reserve(nOldCount);
    bool bAdded = false;
    for (const auto& rPair : pWhichRanges)
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
