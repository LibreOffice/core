/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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



