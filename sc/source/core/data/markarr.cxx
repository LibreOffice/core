/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "markarr.hxx"
#include "global.hxx"
#include "address.hxx"

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------------

ScMarkArray::ScMarkArray() :
    nCount( 0 ),
    nLimit( 0 ),
    pData( NULL )
{
    // special case "no marks" with pData = NULL
}

//------------------------------------------------------------------------

ScMarkArray::~ScMarkArray()
{
    delete[] pData;
}

//------------------------------------------------------------------------

void ScMarkArray::Reset( sal_Bool bMarked )
{
    // always create pData here
    // (or have separate method to ensure pData)

    delete[] pData;

    nCount = nLimit = 1;
    pData = new ScMarkEntry[1];
    pData[0].nRow = MAXROW;
    pData[0].bMarked = bMarked;
}

//------------------------------------------------------------------------

sal_Bool ScMarkArray::Search( SCROW nRow, SCSIZE& nIndex ) const
{
    long    nHi         = static_cast<long>(nCount) - 1;
    long    i           = 0;
    sal_Bool    bFound      = (nCount == 1);
    if (pData)
    {
        long    nLo         = 0;
        long    nStartRow   = 0;
        while ( !bFound && nLo <= nHi )
        {
            i = (nLo + nHi) / 2;
            if (i > 0)
                nStartRow = (long) pData[i - 1].nRow;
            else
                nStartRow = -1;
            long nEndRow = (long) pData[i].nRow;
            if (nEndRow < (long) nRow)
                nLo = ++i;
            else
                if (nStartRow >= (long) nRow)
                    nHi = --i;
                else
                    bFound = sal_True;
        }
    }
    else
        bFound = false;

    if (bFound)
        nIndex=(SCSIZE)i;
    else
        nIndex=0;
    return bFound;
}

sal_Bool ScMarkArray::GetMark( SCROW nRow ) const
{
    SCSIZE i;
    if (Search( nRow, i ))
        return pData[i].bMarked;
    else
        return false;

}

//------------------------------------------------------------------------

void ScMarkArray::SetMarkArea( SCROW nStartRow, SCROW nEndRow, sal_Bool bMarked )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        if ((nStartRow == 0) && (nEndRow == MAXROW))
        {
            Reset(bMarked);
        }
        else
        {
            if (!pData)
                Reset(false);   // create pData for further processing - could use special case handling!

            SCSIZE nNeeded = nCount + 2;
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

            SCSIZE ni;          // number of entries in beginning
            SCSIZE nInsert;     // insert position (MAXROW+1 := no insert)
            sal_Bool bCombined = false;
            sal_Bool bSplit = false;
            if ( nStartRow > 0 )
            {
                // skip beginning
                SCSIZE nIndex;
                Search( nStartRow, nIndex );
                ni = nIndex;

                nInsert = MAXROWCOUNT;
                if ( pData[ni].bMarked != bMarked )
                {
                    if ( ni == 0 || (pData[ni-1].nRow < nStartRow - 1) )
                    {   // may be a split or a simple insert or just a shrink,
                        // row adjustment is done further down
                        if ( pData[ni].nRow > nEndRow )
                            bSplit = sal_True;
                        ni++;
                        nInsert = ni;
                    }
                    else if ( ni > 0 && pData[ni-1].nRow == nStartRow - 1 )
                        nInsert = ni;
                }
                if ( ni > 0 && pData[ni-1].bMarked == bMarked )
                {   // combine
                    pData[ni-1].nRow = nEndRow;
                    nInsert = MAXROWCOUNT;
                    bCombined = sal_True;
                }
            }
            else
        {
                nInsert = 0;
                ni = 0;
        }

            SCSIZE nj = ni;     // stop position of range to replace
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
                    nInsert = MAXROWCOUNT;
                    bCombined = sal_True;
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
                    nInsert = MAXROWCOUNT;
                }
                if ( ni < nj )
                {   // remove entries
                    memmove( pData + ni, pData + nj, (nCount - nj) * sizeof(ScMarkEntry) );
                    nCount -= nj - ni;
                }
            }

            if ( nInsert < sal::static_int_cast<SCSIZE>(MAXROWCOUNT) )
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
}

sal_Bool ScMarkArray::IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const
{
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;

    if (Search( nStartRow, nStartIndex ))
        if (pData[nStartIndex].bMarked)
            if (Search( nEndRow, nEndIndex ))
                if (nEndIndex==nStartIndex)
                    return sal_True;

    return false;
}

sal_Bool ScMarkArray::HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const
{
    sal_Bool bRet = false;
    if ( nCount == 1 )
    {
        if ( pData[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = MAXROW;
            bRet = sal_True;
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
        bRet = sal_True;
    }
    else if ( nCount == 3 )
    {
        if ( pData[1].bMarked )
        {
            rStartRow = pData[0].nRow + 1;
            rEndRow = pData[1].nRow;
            bRet = sal_True;
        }
    }
    return bRet;
}

void ScMarkArray::CopyMarksTo( ScMarkArray& rDestMarkArray ) const
{
    delete[] rDestMarkArray.pData;

    if (pData)
    {
        rDestMarkArray.pData = new ScMarkEntry[nCount];
        memcpy( rDestMarkArray.pData, pData, nCount * sizeof(ScMarkEntry) );
    }
    else
        rDestMarkArray.pData = NULL;

    rDestMarkArray.nCount = rDestMarkArray.nLimit = nCount;
}

SCsROW ScMarkArray::GetNextMarked( SCsROW nRow, sal_Bool bUp ) const
{
    if (!pData)
        const_cast<ScMarkArray*>(this)->Reset(false);   // create pData for further processing

    SCsROW nRet = nRow;
    if (VALIDROW(nRow))
    {
        SCSIZE nIndex;
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

SCROW ScMarkArray::GetMarkEnd( SCROW nRow, sal_Bool bUp ) const
{
    if (!pData)
        const_cast<ScMarkArray*>(this)->Reset(false);   // create pData for further processing

    SCROW nRet;
    SCSIZE nIndex;
    Search(nRow, nIndex);
    OSL_ENSURE( pData[nIndex].bMarked, "GetMarkEnd ohne bMarked" );
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

sal_Bool ScMarkArrayIter::Next( SCROW& rTop, SCROW& rBottom )
{
    if ( nPos >= pArray->nCount )
        return false;
    while (!pArray->pData[nPos].bMarked)
    {
        ++nPos;
        if ( nPos >= pArray->nCount )
            return false;
    }
    rBottom = pArray->pData[nPos].nRow;
    if (nPos==0)
        rTop = 0;
    else
        rTop = pArray->pData[nPos-1].nRow + 1;
    ++nPos;
    return sal_True;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
