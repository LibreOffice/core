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

#include <cassert>
#include <vector>
// compiled via include from itemset.cxx only!
#include <memory>

#include <osl/diagnose.h>

#ifdef DBG_UTIL

#define DBG_CHECK_RANGES(sal_uInt16, pArr)                                 \
    for ( const sal_uInt16 *pRange = pArr; *pRange; pRange += 2 )          \
    {                                                                   \
        DBG_ASSERT( pRange[0] <= pRange[1], "ranges must be sorted" );  \
        DBG_ASSERT( !pRange[2] || ( pRange[2] - pRange[1] ) > 1,        \
                    "ranges must be sorted and discrete" );             \
    }

#else

#define DBG_CHECK_RANGES(sal_uInt16,pArr)

#endif

inline void Swap_Impl(const sal_uInt16 *& rp1, const sal_uInt16 *& rp2)
{
    const sal_uInt16 * pTemp = rp1;
    rp1 = rp2;
    rp2 = pTemp;
}

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
 * Copy ctor
 */
SfxUShortRanges::SfxUShortRanges( const SfxUShortRanges &rOrig )
{
    if ( rOrig._pRanges )
    {
        sal_uInt16 nCount = Count_Impl( rOrig._pRanges ) + 1;
        _pRanges = new sal_uInt16[nCount];
        memcpy( _pRanges, rOrig._pRanges, sizeof(sal_uInt16) * nCount );
    }
    else
        _pRanges = nullptr;
}

/**
 * Constructs a SfxUShortRanges instance from one range of sal_uInt16s.
 *
 * Precondition: nWhich1 <= nWhich2
 */
SfxUShortRanges::SfxUShortRanges( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
:   _pRanges( new sal_uInt16[3] )
{
    _pRanges[0] = nWhich1;
    _pRanges[1] = nWhich2;
    _pRanges[2] = 0;
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
    DBG_CHECK_RANGES(sal_uInt16, pArr);
    sal_uInt16 nCount = Count_Impl(pArr) + 1;
    _pRanges = new sal_uInt16[ nCount ];
    memcpy( _pRanges, pArr, sizeof(sal_uInt16) * nCount );
}


/**
 * Assigns ranges from 'rRanges' to '*this'.
 */
SfxUShortRanges& SfxUShortRanges::operator =
(
    const SfxUShortRanges &rRanges
)
{
    // special case: assign itself
    if ( &rRanges == this )
        return *this;

    delete[] _pRanges;

    // special case: 'rRanges' is empty
    if ( rRanges.IsEmpty() )
        _pRanges = nullptr;
    else
    {
        // copy ranges
        sal_uInt16 nCount = Count_Impl( rRanges._pRanges ) + 1;
        _pRanges = new sal_uInt16[ nCount ];
        memcpy( _pRanges, rRanges._pRanges, sizeof(sal_uInt16) * nCount );
    }
    return *this;
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

    // First, run through _pRanges and rRanges._pRanges and determine the size of
    // the new, merged ranges:
    sal_uInt16 nCount = 0;
    const sal_uInt16 * pRA = _pRanges;
    const sal_uInt16 * pRB = rRanges._pRanges;

    for (;;)
    {
        // The first pair of pRA has a lower lower bound than the first pair
        // of pRB:
        if (pRA[0] > pRB[0])
            Swap_Impl(pRA, pRB);

        // We are done with the merging if at least pRA is exhausted:
        if (!pRA[0])
            break;

        for (;;)
        {
            // Skip those pairs in pRB that completely lie in the first pair
            // of pRA:
            while (pRB[1] <= pRA[1])
            {
                pRB += 2;

                // Watch out for exhaustion of pRB:
                if (!pRB[0])
                {
                    Swap_Impl(pRA, pRB);
                    goto count_rest;
                }
            }

            // If the next pair of pRA does not at least touch the current new
            // pair, we are done with the current new pair:
            if (pRB[0] > pRA[1] + 1)
                break;

            // The next pair of pRB extends the current new pair; first,
            // extend the current new pair (we are done if pRB is then
            // exhausted); second, switch the roles of pRA and pRB in order to
            // merge in those following pairs of the original pRA that will
            // lie in the (now larger) current new pair or will even extend it
            // further:
            pRA += 2;
            if (!pRA[0])
                goto count_rest;
            Swap_Impl(pRA, pRB);
        }

        // Done with the current new pair:
        pRA += 2;
        nCount += 2;
    }

    // Only pRB has more pairs available, pRA is already exhausted:
count_rest:
    for (; pRB[0]; pRB += 2)
        nCount += 2;

    // Now, create new ranges of the correct size and, on a second run through
    // _pRanges and rRanges._pRanges, copy the merged pairs into the new
    // ranges:
    sal_uInt16 * pNew = new sal_uInt16[nCount + 1];
    pRA = _pRanges;
    pRB = rRanges._pRanges;
    sal_uInt16 * pRN = pNew;

    for (;;)
    {
        // The first pair of pRA has a lower lower bound than the first pair
        // of pRB:
        if (pRA[0] > pRB[0])
            Swap_Impl(pRA, pRB);

        // We are done with the merging if at least pRA is exhausted:
        if (!pRA[0])
            break;

        // Lower bound of current new pair is already known:
        *pRN++ = pRA[0];

        for (;;)
        {
            // Skip those pairs in pRB that completely lie in the first pair
            // of pRA:
            while (pRB[1] <= pRA[1])
            {
                pRB += 2;

                // Watch out for exhaustion of pRB:
                if (!pRB[0])
                {
                    Swap_Impl(pRA, pRB);
                    ++pRB;
                    goto copy_rest;
                }
            }

            // If the next pair of pRA does not at least touch the current new
            // pair, we are done with the current new pair:
            if (pRB[0] > pRA[1] + 1)
                break;

            // The next pair of pRB extends the current new pair; first,
            // extend the current new pair (we are done if pRB is then
            // exhausted); second, switch the roles of pRA and pRB in order to
            // merge in those following pairs of the original pRA that will
            // lie in the (now larger) current new pair or will even extend it
            // further:
            pRA += 2;
            if (!pRA[0])
            {
                ++pRB;
                goto copy_rest;
            }
            Swap_Impl(pRA, pRB);
        }

        // Done with the current new pair, now upper bound is also known:
        *pRN++ = pRA[1];
        pRA += 2;
    }

    // Only pRB has more pairs available (which are copied to the new ranges
    // unchanged), pRA is already exhausted:
copy_rest:
    for (; *pRB;)
        *pRN++ = *pRB++;
    *pRN = 0;

    delete[] _pRanges;
    _pRanges = pNew;

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
