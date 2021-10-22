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
 * Determines the number of sal_uInt16s in a container of pairs of
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

inline bool validRanges2(const WhichRangesContainer& pRanges)
{
    for (sal_Int32 i = 0; i < pRanges.size(); ++i)
    {
        std::pair<sal_uInt16, sal_uInt16> p = pRanges[i];
        if (!validRange(p.first, p.second))
            return false;
        // ranges must be sorted
        if (i < pRanges.size() - 1 && !validGap(p.second, pRanges[i + 1].first))
            return false;
    }
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
