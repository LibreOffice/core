/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagedata.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:45:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include <string.h>

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif


#ifndef PCH
#include "segmentc.hxx"
#endif

#include "pagedata.hxx"

//============================================================================

ScPrintRangeData::ScPrintRangeData()
{
    nPagesX = nPagesY = 0;
    pPageEndX = NULL;
    pPageEndY = NULL;
    bTopDown = bAutomatic = TRUE;
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

BOOL ScPageBreakData::IsEqual( const ScPageBreakData& rOther ) const
{
    if ( nUsed != rOther.nUsed )
        return FALSE;

    for (USHORT i=0; i<nUsed; i++)
        if ( pData[i].GetPrintRange() != rOther.pData[i].GetPrintRange() )
            return FALSE;

    //! ScPrintRangeData komplett vergleichen ??

    return TRUE;
}

void ScPageBreakData::AddPages()
{
    if ( nUsed > 1 )
    {
        long nPage = pData[0].GetFirstPage();
        for (USHORT i=0; i+1<nUsed; i++)
        {
            nPage += ((long)pData[i].GetPagesX())*pData[i].GetPagesY();
            pData[i+1].SetFirstPage( nPage );
        }
    }
}



