/*************************************************************************
 *
 *  $RCSfile: pagedata.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#include <string.h>

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#pragma hdrstop

#ifndef PCH
#include "segmentc.hxx"
#endif

#include "pagedata.hxx"

//============================================================================

ScPrintRangeData::ScPrintRangeData()
{
    nPagesX = nPagesY = 0;
    pPageEndX = pPageEndY = NULL;
    bTopDown = bAutomatic = TRUE;
    nFirstPage = 1;
}

ScPrintRangeData::~ScPrintRangeData()
{
    delete[] pPageEndX;
    delete[] pPageEndY;
}

void ScPrintRangeData::SetPagesX( USHORT nCount, const USHORT* pData )
{
    delete[] pPageEndX;
    if ( nCount )
    {
        pPageEndX = new USHORT[nCount];
        memcpy( pPageEndX, pData, nCount * sizeof(USHORT) );
    }
    else
        pPageEndX = NULL;
    nPagesX = nCount;
}

void ScPrintRangeData::SetPagesY( USHORT nCount, const USHORT* pData )
{
    delete[] pPageEndY;
    if ( nCount )
    {
        pPageEndY = new USHORT[nCount];
        memcpy( pPageEndY, pData, nCount * sizeof(USHORT) );
    }
    else
        pPageEndY = NULL;
    nPagesY = nCount;
}

//============================================================================

ScPageBreakData::ScPageBreakData(USHORT nMax)
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

ScPrintRangeData& ScPageBreakData::GetData(USHORT nPos)
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



