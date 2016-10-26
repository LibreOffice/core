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
#include <svl/nranges.hxx>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

#include <tools/debug.hxx>

/**
 * Creates a sal_uInt16-ranges-array in 'rpRanges' using 'nWh1' and 'nWh2' as
 * first range, 'nNull' as terminator or start of 2nd range and 'pArgs' as
 * remainder.
 *
 * It returns the number of sal_uInt16s which are contained in the described
 * set of sal_uInt16s.
 */
sal_uInt16 InitializeRanges_Impl( sal_uInt16 *&rpRanges, va_list pArgs,
                               sal_uInt16 nWh1, sal_uInt16 nWh2, sal_uInt16 nNull )
{
    sal_uInt16 nSize = 0, nIns = 0;
    std::vector<sal_uInt16> aNumArr;
    aNumArr.push_back( nWh1 );
    aNumArr.push_back( nWh2 );
    DBG_ASSERT( nWh1 <= nWh2, "Invalid range" );
    nSize += nWh2 - nWh1 + 1;
    aNumArr.push_back( nNull );
    bool bEndOfRange = false;
    while ( 0 !=
            ( nIns =
              sal::static_int_cast< sal_uInt16 >(
                  va_arg( pArgs, int ) ) ) )
    {
        bEndOfRange = !bEndOfRange;
        if ( bEndOfRange )
        {
            const sal_uInt16 nPrev(*aNumArr.rbegin());
            DBG_ASSERT( nPrev <= nIns, "Invalid range" );
            nSize += nIns - nPrev + 1;
        }
        aNumArr.push_back( nIns );
    }

    assert( bEndOfRange ); // odd number of WhichIds

    // Now all ranges are present
    rpRanges = new sal_uInt16[ aNumArr.size() + 1 ];
    std::copy( aNumArr.begin(), aNumArr.end(), rpRanges);
    *(rpRanges + aNumArr.size()) = 0;

    return nSize;
}

/**
 * Determines the number of sal_uInt16s in a 0-terminated array of pairs of
 * sal_uInt16s.
 * The terminating 0 is not included in the count.
 */
sal_uInt16 Count_Impl( const sal_uInt16 *pRanges )
{
    sal_uInt16 nCount = 0;
    while ( *pRanges )
    {
        nCount += 2;
        pRanges += 2;
    }
    return nCount;
}

/**
 * Determines the total number of sal_uInt16s described in a 0-terminated
 * array of pairs of sal_uInt16s, each representing an range of sal_uInt16s.
 */
sal_uInt16 Capacity_Impl( const sal_uInt16 *pRanges )
{
    sal_uInt16 nCount = 0;

    if ( pRanges )
    {
        while ( *pRanges )
        {
            nCount += pRanges[1] - pRanges[0] + 1;
            pRanges += 2;
        }
    }
    return nCount;
}

/**
 * Constructs a SfxUShortRanges instance from one range of sal_uInt16s.
 *
 * Precondition: nWhich1 <= nWhich2
 */
SfxUShortRanges::SfxUShortRanges( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
    :   m_aRanges({std::pair<sal_uInt16, sal_uInt16>(nWhich1, nWhich2)})
{
    assert(m_aRanges.front().first <= m_aRanges.front().second);
}

/**
 * Constructs an SfxUShortRanges-instance from an sorted ranges of sal_uInt16s,
 * terminates with on 0.
 *
 * Precondition: for each n >= 0 && n < (sizeof(pArr)-1)
 *     pArr[2n] <= pArr[2n+1] && ( pArr[2n+2]-pArr[2n+1] ) > 1
 */
SfxUShortRanges::SfxUShortRanges( const sal_uInt16* pArr )
{
#ifdef DBG_UTIL
    for ( const sal_uInt16 *pRange = pArr; *pRange; pRange += 2 )
    {
        assert(pRange[0] <= pRange[1]);
        DBG_ASSERT( !pRange[2] || ( pRange[2] - pRange[1] ) > 1,
                    "ranges must be sorted and discrete" );
    }
#endif
    sal_uInt16 nCount = Count_Impl(pArr);
    for (size_t i = 0; i < nCount; i += 2)
        m_aRanges.emplace_back(std::pair<sal_uInt16, sal_uInt16>(pArr[i],pArr[i+1]));
}


const std::vector<sal_uInt16> SfxUShortRanges::getVector() const
{
    // construct flat range array
    const size_t nSize = 2 * m_aRanges.size() + 1;
    std::vector<sal_uInt16> aRanges(nSize);
    for (size_t i = 0; i < (nSize - 1); i +=2)
    {
        aRanges[i]   = m_aRanges[i/2].first;
        aRanges[i+1] = m_aRanges[i/2].second;
    }
    // null terminate to be compatible with sal_uInt16* array pointers
    aRanges[nSize-1] = 0;
    return aRanges;
}

/**
 * Merges *this with 'rRanges'.
 *  for each sal_uInt16 n:
 *    this->Contains( n ) || rRanges.Contains( n ) => this'->Contains( n )
 *    !this->Contains( n ) && !rRanges.Contains( n ) => !this'->Contains( n )
 */
SfxUShortRanges& SfxUShortRanges::operator +=
(
    const SfxUShortRanges &rRanges
)
{
    // special cases: one is empty
    if ( rRanges.IsEmpty() )
        return *this;
    if ( IsEmpty() )
        return *this = rRanges;

    // simple comparator for pairs, only care about lower bound of range
    auto isLess = [](std::pair<sal_uInt16, sal_uInt16> lhs, std::pair<sal_uInt16, sal_uInt16> rhs)
                        {return lhs.first < rhs.first;};
    // true if ranges overlap or adjoin, false if ranges are separate (!only works on sorted pairs)
    auto needMerge = [](std::pair<sal_uInt16, sal_uInt16> lhs, std::pair<sal_uInt16, sal_uInt16> rhs)
                        {return (lhs.first-1) <= rhs.second && (rhs.first-1) <= lhs.second;};

    std::vector<std::pair<sal_uInt16, sal_uInt16>> aMerged(m_aRanges.size()+rRanges.m_aRanges.size());
    // merge and sort range pairs
    std::merge(m_aRanges.begin(), m_aRanges.end(),
               rRanges.m_aRanges.begin(), rRanges.m_aRanges.end(),
               aMerged.begin(), isLess);
    std::vector<std::pair<sal_uInt16, sal_uInt16> >::iterator it;
    // check neighbouring ranges, find first range which overlaps or adjoins the previous range
    while ((it = std::is_sorted_until(aMerged.begin(), aMerged.end(), needMerge)) != aMerged.end())
    {
        --it; // merge with previous range
        // lower bounds are sorted, implies: it->first = min(it[0].first, it[1].first)
        it->second = std::max(it[0].second, it[1].second);
        aMerged.erase(it+1);
    }
    m_aRanges = aMerged;

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
