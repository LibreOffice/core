/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
// INCLUDE ---------------------------------------------------------------

#include <svl/whiter.hxx>
#include <svl/itemset.hxx>

DBG_NAME(SfxWhichIter)

// -----------------------------------------------------------------------

SfxWhichIter::SfxWhichIter( const SfxItemSet& rSet, sal_uInt16 nFromWh, sal_uInt16 nToWh ):
    pRanges(rSet.GetRanges()),
    pStart(rSet.GetRanges()),
    nOfst(0), nFrom(nFromWh), nTo(nToWh)
{
    DBG_CTOR(SfxWhichIter, 0);
    if ( nFrom > 0 )
        FirstWhich();
}

// -----------------------------------------------------------------------

SfxWhichIter::~SfxWhichIter()
{
    DBG_DTOR(SfxWhichIter, 0);
}

// -----------------------------------------------------------------------

sal_uInt16 SfxWhichIter::NextWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    while ( 0 != *pRanges )
    {
        const sal_uInt16 nLastWhich = *pRanges + nOfst;
        ++nOfst;
        if (*(pRanges+1) == nLastWhich)
        {
            pRanges += 2;
            nOfst = 0;
        }
        sal_uInt16 nWhich = *pRanges + nOfst;
        if ( 0 == nWhich || ( nWhich >= nFrom && nWhich <= nTo ) )
            return nWhich;
    }
    return 0;
}

// -----------------------------------------------------------------------

sal_uInt16  SfxWhichIter::PrevWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    while ( pRanges != pStart || 0 != nOfst )
    {
        if(nOfst)
            --nOfst;
        else {
            pRanges -= 2;
            nOfst = *(pRanges+1) - (*pRanges);
        }
        sal_uInt16 nWhich = *pRanges + nOfst;
        if ( nWhich >= nFrom && nWhich <= nTo )
            return nWhich;
    }
    return 0;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxWhichIter::FirstWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    pRanges = pStart;
    nOfst = 0;
    if ( *pRanges >= nFrom && *pRanges <= nTo )
        return *pRanges;
    return NextWhich();
}

// -----------------------------------------------------------------------

sal_uInt16 SfxWhichIter::LastWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    while(*pRanges)
        ++pRanges;
    nOfst = 0;
    sal_uInt16 nWhich = *(pRanges-1);
    if ( nWhich >= nFrom && nWhich <= nTo )
        return nWhich;
    return PrevWhich();
}

