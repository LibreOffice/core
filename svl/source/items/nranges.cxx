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

#include <cassert>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
