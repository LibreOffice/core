/*************************************************************************
 *
 *  $RCSfile: markarr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:03:59 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "markarr.hxx"
#include "global.hxx"

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------------

ScMarkArray::ScMarkArray()
{
    nCount = nLimit = 1;
    pData = new ScMarkEntry[1];
    if (pData)
    {
        pData[0].nRow = MAXROW;
        pData[0].bMarked = FALSE;
    }
}

//------------------------------------------------------------------------

ScMarkArray::~ScMarkArray()
{
    if (pData)
        delete[] pData;
}

//------------------------------------------------------------------------

void ScMarkArray::Reset( BOOL bMarked )
{
    if (pData)
    {
        delete[] pData;

        nCount = nLimit = 1;
        pData = new ScMarkEntry[1];
        if (pData)
        {
            pData[0].nRow = MAXROW;
            pData[0].bMarked = bMarked;
        }
    }
}

//------------------------------------------------------------------------

BOOL ScMarkArray::Search( USHORT nRow, short& nIndex ) const
{
    short   nLo         = 0;
    short   nHi         = ((short) nCount) - 1;
    short   nStartRow   = 0;
    short   nEndRow     = 0;
    short   i           = 0;
    BOOL    bFound      = (nCount == 1);
    if (pData)
    {
        while ( !bFound && nLo <= nHi )
        {
            i = (nLo + nHi) / 2;
            if (i > 0)
                nStartRow = (short) pData[i - 1].nRow;
            else
                nStartRow = -1;
            nEndRow = (short) pData[i].nRow;
            if (nEndRow < (short) nRow)
                nLo = ++i;
            else
                if (nStartRow >= (short) nRow)
                    nHi = --i;
                else
                    bFound = TRUE;
        }
    }
    else
        bFound = FALSE;

    if (bFound)
        nIndex=i;
    else
        nIndex=0;
    return bFound;
}

BOOL ScMarkArray::GetMark( USHORT nRow ) const
{
    short i;
    if (Search( nRow, i ))
        return pData[i].bMarked;
    else
        return FALSE;

}

//------------------------------------------------------------------------

void ScMarkArray::SetMark( USHORT nRow, BOOL bMarked )
{
    SetMarkArea( nRow, nRow, bMarked );
}

void ScMarkArray::SetMarkArea( USHORT nStartRow, USHORT nEndRow, BOOL bMarked )
{
    if ((nStartRow >= 0 && nStartRow <= MAXROW) && (nEndRow >= 0 && nEndRow <= MAXROW))
    {
        if ((nStartRow == 0) && (nEndRow == MAXROW))
        {
            Reset(bMarked);
        }
        else
        {
            USHORT nNeeded = nCount + 2;
            if ( nLimit < nNeeded )
            {
                nLimit += SC_MARKARRAY_DELTA;
                if ( nLimit < nNeeded )
                    nLimit = nNeeded;
                ScMarkEntry* pNewData = new ScMarkEntry[nLimit];
                memcpy( pNewData, pData, nCount*sizeof(ScMarkEntry) );
                delete[] pData;
                pData = pNewData;
            }

            USHORT ni;          // number of entries in beginning
            USHORT nInsert;     // insert position (MAXROW+1 := no insert)
            BOOL bCombined = FALSE;
            BOOL bSplit = FALSE;
            if ( nStartRow > 0 )
            {
                // skip beginning
                short nIndex;
                Search( nStartRow, nIndex );
                ni = nIndex;

                nInsert = MAXROW+1;
                if ( pData[ni].bMarked != bMarked )
                {
                    if ( ni == 0 || (pData[ni-1].nRow < nStartRow - 1) )
                    {   // may be a split or a simple insert or just a shrink,
                        // row adjustment is done further down
                        if ( pData[ni].nRow > nEndRow )
                            bSplit = TRUE;
                        ni++;
                        nInsert = ni;
                    }
                    else if ( ni > 0 && pData[ni-1].nRow == nStartRow - 1 )
                        nInsert = ni;
                }
                if ( ni > 0 && pData[ni-1].bMarked == bMarked )
                {   // combine
                    pData[ni-1].nRow = nEndRow;
                    nInsert = MAXROW+1;
                    bCombined = TRUE;
                }
            }
            else
                nInsert = ni = 0;

            USHORT nj = ni;     // stop position of range to replace
            while ( nj < nCount && pData[nj].nRow <= nEndRow )
                nj++;
            if ( !bSplit )
            {
                if ( nj < nCount && pData[nj].bMarked == bMarked )
                {   // combine
                    if ( ni > 0 )
                    {
                        if ( pData[ni-1].bMarked == bMarked )
                        {   // adjacent entries
                            pData[ni-1].nRow = pData[nj].nRow;
                            nj++;
                        }
                        else if ( ni == nInsert )
                            pData[ni-1].nRow = nStartRow - 1;   // shrink
                    }
                    nInsert = MAXROW+1;
                    bCombined = TRUE;
                }
                else if ( ni > 0 && ni == nInsert )
                    pData[ni-1].nRow = nStartRow - 1;   // shrink
            }
            if ( ni < nj )
            {   // remove middle entries
                if ( !bCombined )
                {   // replace one entry
                    pData[ni].nRow = nEndRow;
                    pData[ni].bMarked = bMarked;
                    ni++;
                    nInsert = MAXROW+1;
                }
                if ( ni < nj )
                {   // remove entries
                    memmove( pData + ni, pData + nj, (nCount - nj) * sizeof(ScMarkEntry) );
                    nCount -= nj - ni;
                }
            }

            if ( nInsert <= MAXROW )
            {   // insert or append new entry
                if ( nInsert <= nCount )
                {
                    if ( !bSplit )
                        memmove( pData + nInsert + 1, pData + nInsert,
                            (nCount - nInsert) * sizeof(ScMarkEntry) );
                    else
                    {
                        memmove( pData + nInsert + 2, pData + nInsert,
                            (nCount - nInsert) * sizeof(ScMarkEntry) );
                        pData[nInsert+1] = pData[nInsert-1];
                        nCount++;
                    }
                }
                if ( nInsert )
                    pData[nInsert-1].nRow = nStartRow - 1;
                pData[nInsert].nRow = nEndRow;
                pData[nInsert].bMarked = bMarked;
                nCount++;
            }
        }
    }
//  InfoBox(0, String(nCount) + String(" Eintraege") ).Execute();
}

void ScMarkArray::DeleteArea(USHORT nStartRow, USHORT nEndRow)
{
    SetMarkArea(nStartRow, nEndRow, FALSE);
}

BOOL ScMarkArray::IsAllMarked( USHORT nStartRow, USHORT nEndRow ) const
{
    short nStartIndex;
    short nEndIndex;

    if (Search( nStartRow, nStartIndex ))
        if (pData[nStartIndex].bMarked)
            if (Search( nEndRow, nEndIndex ))
                if (nEndIndex==nStartIndex)
                    return TRUE;

    return FALSE;
}

BOOL ScMarkArray::HasOneMark( USHORT& rStartRow, USHORT& rEndRow ) const
{
    BOOL bRet = FALSE;
    if ( nCount == 1 )
    {
        if ( pData[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = MAXROW;
            bRet = TRUE;
        }
    }
    else if ( nCount == 2 )
    {
        if ( pData[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = pData[0].nRow;
        }
        else
        {
            rStartRow = pData[0].nRow + 1;
            rEndRow = MAXROW;
        }
        bRet = TRUE;
    }
    else if ( nCount == 3 )
    {
        if ( pData[1].bMarked )
        {
            rStartRow = pData[0].nRow + 1;
            rEndRow = pData[1].nRow;
            bRet = TRUE;
        }
    }
    return bRet;
}

BOOL ScMarkArray::HasMarks() const
{
    return ( nCount > 1 || pData[0].bMarked );
}

void ScMarkArray::SwapCol(ScMarkArray& rMarkArray)
{
    USHORT nTemp = rMarkArray.nCount;
    rMarkArray.nCount = nCount;
    nCount = nTemp;

    nTemp = rMarkArray.nLimit;
    rMarkArray.nLimit = nLimit;
    nLimit = nTemp;

    ScMarkEntry* pTemp = rMarkArray.pData;
    rMarkArray.pData = pData;
    pData = pTemp;
}

void ScMarkArray::MoveTo(USHORT nStartRow, USHORT nEndRow, ScMarkArray& rMarkArray)
{
    USHORT nStart = nStartRow;
    for (USHORT i = 0; i < nCount; i++)
    {
        if ((pData[i].nRow >= nStartRow) && ((i==0) ? TRUE : pData[i-1].nRow < nEndRow))
        {
            rMarkArray.SetMarkArea(nStart, Min(pData[i].nRow,nEndRow), pData[i].bMarked);
        }
        nStart = Max((USHORT)nStart, (USHORT)(pData[i].nRow + 1) );
    }
    DeleteArea(nStartRow, nEndRow);
}

void ScMarkArray::CopyMarksTo( ScMarkArray& rDestMarkArray ) const
{
    delete[] rDestMarkArray.pData;

    if (pData)
    {
        rDestMarkArray.pData = new ScMarkEntry[nCount];
        memmove( rDestMarkArray.pData, pData, nCount * sizeof(ScMarkEntry) );
    }
    else
        rDestMarkArray.pData = NULL;

    rDestMarkArray.nCount = rDestMarkArray.nLimit = nCount;
}

short ScMarkArray::GetNextMarked( short nRow, BOOL bUp ) const
{
    short nRet = nRow;
    if (VALIDROW(nRow))
    {
        short nIndex;
        Search(nRow, nIndex);
        if (!pData[nIndex].bMarked)
        {
            if (bUp)
            {
                if (nIndex>0)
                    nRet = pData[nIndex-1].nRow;
                else
                    nRet = -1;
            }
            else
                nRet = pData[nIndex].nRow + 1;
        }
    }
    return nRet;
}

USHORT ScMarkArray::GetMarkEnd( USHORT nRow, BOOL bUp ) const
{
    USHORT nRet;
    short nIndex;
    Search(nRow, nIndex);
    DBG_ASSERT( pData[nIndex].bMarked, "GetMarkEnd ohne bMarked" );
    if (bUp)
    {
        if (nIndex>0)
            nRet = pData[nIndex-1].nRow + 1;
        else
            nRet = 0;
    }
    else
        nRet = pData[nIndex].nRow;

    return nRet;
}

//
//  -------------- Iterator ----------------------------------------------
//

ScMarkArrayIter::ScMarkArrayIter( const ScMarkArray* pNewArray ) :
    pArray( pNewArray ),
    nPos( 0 )
{
}

ScMarkArrayIter::~ScMarkArrayIter()
{
}

BOOL ScMarkArrayIter::Next( USHORT& rTop, USHORT& rBottom )
{
    if ( nPos >= pArray->nCount )
        return FALSE;
    while (!pArray->pData[nPos].bMarked)
    {
        ++nPos;
        if ( nPos >= pArray->nCount )
            return FALSE;
    }
    rBottom = pArray->pData[nPos].nRow;
    if (nPos==0)
        rTop = 0;
    else
        rTop = pArray->pData[nPos-1].nRow + 1;
    ++nPos;
    return TRUE;
}





