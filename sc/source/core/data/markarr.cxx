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

#include "markarr.hxx"
#include "global.hxx"
#include "address.hxx"

#include <osl/diagnose.h>

// STATIC DATA -----------------------------------------------------------

ScMarkArray::ScMarkArray() :
    nCount( 0 ),
    nLimit( 0 ),
    pData( nullptr )
{
    // special case "no marks" with pData = NULL
}

ScMarkArray::~ScMarkArray()
{
    delete[] pData;
}

void ScMarkArray::Reset( bool bMarked )
{
    // always create pData here
    // (or have separate method to ensure pData)

    delete[] pData;

    nCount = nLimit = 1;
    pData = new ScMarkEntry[1];
    pData[0].nRow = MAXROW;
    pData[0].bMarked = bMarked;
}

bool ScMarkArray::Search( SCROW nRow, SCSIZE& nIndex ) const
{
    long    nHi         = static_cast<long>(nCount) - 1;
    long    i           = 0;
    bool    bFound      = (nCount == 1);
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
                    bFound = true;
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

bool ScMarkArray::GetMark( SCROW nRow ) const
{
    SCSIZE i;
    if (Search( nRow, i ))
        return pData[i].bMarked;
    else
        return false;

}

void ScMarkArray::SetMarkArea( SCROW nStartRow, SCROW nEndRow, bool bMarked )
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
                Reset();   // create pData for further processing - could use special case handling!

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
            bool bCombined = false;
            bool bSplit = false;
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
                            bSplit = true;
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
                    bCombined = true;
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
                    bCombined = true;
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

bool ScMarkArray::IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const
{
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;

    if (Search( nStartRow, nStartIndex ))
        if (pData[nStartIndex].bMarked)
            if (Search( nEndRow, nEndIndex ))
                if (nEndIndex==nStartIndex)
                    return true;

    return false;
}

bool ScMarkArray::HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const
{
    bool bRet = false;
    if ( nCount == 1 )
    {
        if ( pData[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = MAXROW;
            bRet = true;
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
        bRet = true;
    }
    else if ( nCount == 3 )
    {
        if ( pData[1].bMarked )
        {
            rStartRow = pData[0].nRow + 1;
            rEndRow = pData[1].nRow;
            bRet = true;
        }
    }
    return bRet;
}

bool ScMarkArray::HasEqualRowsMarked( const ScMarkArray& rOther ) const
{
    if (nCount != rOther.nCount)
        return false;

    for (size_t i=0; i < nCount; ++i)
    {
        if (pData[i].bMarked != rOther.pData[i].bMarked ||
                pData[i].nRow != rOther.pData[i].nRow)
            return false;
    }

    return true;
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
        rDestMarkArray.pData = nullptr;

    rDestMarkArray.nCount = rDestMarkArray.nLimit = nCount;
}

SCsROW ScMarkArray::GetNextMarked( SCsROW nRow, bool bUp ) const
{
    if (!pData)
        const_cast<ScMarkArray*>(this)->Reset();   // create pData for further processing

    SCsROW nRet = nRow;
    if (ValidRow(nRow))
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

SCROW ScMarkArray::GetMarkEnd( SCROW nRow, bool bUp ) const
{
    if (!pData)
        const_cast<ScMarkArray*>(this)->Reset();   // create pData for further processing

    SCROW nRet;
    SCSIZE nIndex;
    Search(nRow, nIndex);
    OSL_ENSURE( pData[nIndex].bMarked, "GetMarkEnd without bMarked" );
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

//  -------------- Iterator ----------------------------------------------

ScMarkArrayIter::ScMarkArrayIter( const ScMarkArray* pNewArray ) :
    pArray( pNewArray ),
    nPos( 0 )
{
}

ScMarkArrayIter::~ScMarkArrayIter()
{
}

bool ScMarkArrayIter::Next( SCROW& rTop, SCROW& rBottom )
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
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
