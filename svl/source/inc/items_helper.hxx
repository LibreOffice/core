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
 * Determines the number of sal_uInt16s in a 0-terminated array of pairs of
 * sal_uInt16s, each representing a range of sal_uInt16s, and total capacity of the ranges.
 * The terminating 0 is included in the count.
 */
inline std::pair<sal_uInt16, sal_uInt16> CountRanges(const sal_uInt16* pRanges)
{
    sal_uInt16 nCount = 0;
    sal_uInt16 nCapacity = 0;
    if (pRanges)
    {
        nCount = 1;
        while (*pRanges)
        {
            nCount += 2;
            nCapacity += rangeSize(pRanges[0], pRanges[1]);
            pRanges += 2;
        }
    }
    return { nCount, nCapacity };
}

// Adds a range to which ranges, keeping the ranges in valid state (sorted, non-overlapping)
inline std::unique_ptr<sal_uInt16[]> MergeRange(const sal_uInt16* pWhichRanges, sal_uInt16 nFrom,
                                                sal_uInt16 nTo)
{
    assert(validRange(nFrom, nTo));

    if (!pWhichRanges)
    {
        auto pNewRanges = std::make_unique<sal_uInt16[]>(3);
        pNewRanges[0] = nFrom;
        pNewRanges[1] = nTo;
        pNewRanges[2] = 0;
        return pNewRanges;
    }

    assert(validRanges(pWhichRanges));

    // create vector of ranges (sal_uInt16 pairs of lower and upper bound)
    const size_t nOldCount = CountRanges(pWhichRanges).first;
    std::vector<std::pair<sal_uInt16, sal_uInt16>> aRangesTable;
    aRangesTable.reserve(nOldCount / 2 + 1);
    bool bAdded = false;
    for (size_t i = 0; i + 1 < nOldCount; i += 2)
    {
        if (!bAdded && pWhichRanges[i] >= nFrom)
        { // insert new range, keep ranges sorted
            aRangesTable.emplace_back(std::pair<sal_uInt16, sal_uInt16>(nFrom, nTo));
            bAdded = true;
        }
        // insert current range
        aRangesTable.emplace_back(
            std::pair<sal_uInt16, sal_uInt16>(pWhichRanges[i], pWhichRanges[i + 1]));
    }
    if (!bAdded)
        aRangesTable.emplace_back(std::pair<sal_uInt16, sal_uInt16>(nFrom, nTo));

    // true if ranges overlap or adjoin, false if ranges are separate
    auto needMerge
        = [](std::pair<sal_uInt16, sal_uInt16> lhs, std::pair<sal_uInt16, sal_uInt16> rhs) {
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

    // construct range array
    const size_t nNewSize = 2 * aRangesTable.size() + 1;
    auto pNewRanges = std::make_unique<sal_uInt16[]>(nNewSize);
    for (size_t i = 0; i < (nNewSize - 1); i += 2)
        std::tie(pNewRanges[i], pNewRanges[i + 1]) = aRangesTable[i / 2];

    pNewRanges[nNewSize - 1] = 0;
    return pNewRanges;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
