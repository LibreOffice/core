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
#include "precompiled_sc.hxx"


#include <string.h>
#include <tools/debug.hxx>


#include "pagedata.hxx"

//============================================================================

ScPrintRangeData::ScPrintRangeData()
{
    nPagesX = nPagesY = 0;
    pPageEndX = NULL;
    pPageEndY = NULL;
    bTopDown = bAutomatic = sal_True;
    nFirstPage = 1;
}

ScPrintRangeData::~ScPrintRangeData()
{
    delete[] pPageEndX;
    delete[] pPageEndY;
}

void ScPrintRangeData::SetPagesX( size_t nCount, const SCCOL* pData )
{
    delete[] pPageEndX;
    if ( nCount )
    {
        pPageEndX = new SCCOL[nCount];
        memcpy( pPageEndX, pData, nCount * sizeof(SCCOL) );
    }
    else
        pPageEndX = NULL;
    nPagesX = nCount;
}

void ScPrintRangeData::SetPagesY( size_t nCount, const SCROW* pData )
{
    delete[] pPageEndY;
    if ( nCount )
    {
        pPageEndY = new SCROW[nCount];
        memcpy( pPageEndY, pData, nCount * sizeof(SCROW) );
    }
    else
        pPageEndY = NULL;
    nPagesY = nCount;
}

//============================================================================

ScPageBreakData::ScPageBreakData(size_t nMax)
{
    nUsed = 0;
    if (nMax)
        pData = new ScPrintRangeData[nMax];
    else
        pData = NULL;
    nAlloc = nMax;
}

ScPageBreakData::~ScPageBreakData()
{
    delete[] pData;
}

ScPrintRangeData& ScPageBreakData::GetData(size_t nPos)
{
    DBG_ASSERT(nPos < nAlloc, "ScPageBreakData::GetData bumm");

    if ( nPos >= nUsed )
    {
        DBG_ASSERT(nPos == nUsed, "ScPageBreakData::GetData falsche Reihenfolge");
        nUsed = nPos+1;
    }

    return pData[nPos];
}

sal_Bool ScPageBreakData::IsEqual( const ScPageBreakData& rOther ) const
{
    if ( nUsed != rOther.nUsed )
        return sal_False;

    for (sal_uInt16 i=0; i<nUsed; i++)
        if ( pData[i].GetPrintRange() != rOther.pData[i].GetPrintRange() )
            return sal_False;

    //! ScPrintRangeData komplett vergleichen ??

    return sal_True;
}

void ScPageBreakData::AddPages()
{
    if ( nUsed > 1 )
    {
        long nPage = pData[0].GetFirstPage();
        for (sal_uInt16 i=0; sal::static_int_cast<size_t>(i+1)<nUsed; i++)
        {
            nPage += ((long)pData[i].GetPagesX())*pData[i].GetPagesY();
            pData[i+1].SetFirstPage( nPage );
        }
    }
}



