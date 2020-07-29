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

#include <markarr.hxx>
#include <address.hxx>
#include <rangelst.hxx>
#include <sheetlimits.hxx>
#include <vector>

#include <osl/diagnose.h>

ScMarkArray::ScMarkArray(const ScSheetLimits& rLimits) :
    mrSheetLimits(rLimits)
{
    Reset(false);
}

// Move constructor
ScMarkArray::ScMarkArray( ScMarkArray&& rOther ) noexcept
    : mrSheetLimits(rOther.mrSheetLimits)
{
    operator=(std::move(rOther));
}

// Copy constructor
ScMarkArray::ScMarkArray( const ScMarkArray & rOther )
    : mrSheetLimits(rOther.mrSheetLimits)
{
    operator=(rOther);
}

ScMarkArray::~ScMarkArray()
{
}

void ScMarkArray::Reset( bool bMarked, SCSIZE nNeeded )
{
    // always create pData here
    // (or have separate method to ensure pData)

    assert(nNeeded);
    mvData.resize(1);
    mvData.reserve(nNeeded);
    mvData[0].nRow = mrSheetLimits.mnMaxRow;
    mvData[0].bMarked = bMarked;
}

// Iterative implementation of Binary Search
bool ScMarkArray::Search( SCROW nRow, SCSIZE& nIndex ) const
{
    assert(mvData.size() > 0);
    SCSIZE    nHi         = mvData.size() - 1;
    SCSIZE    nLo         = 0;

    while ( nLo <= nHi )
    {
        SCSIZE i = (nLo + nHi) / 2;

        if (mvData[i].nRow < nRow)
        {
            // If [nRow] greater, ignore left half
            nLo = i + 1;
        }
        else if ((i > 0) && (mvData[i - 1].nRow >= nRow))
        {
            // If [nRow] is smaller, ignore right half
            nHi = i - 1;
        }
        else
        {
            // found
            nIndex=i;
            return true;
        }
    }

    // not found
    nIndex=0;
    return false;
}

bool ScMarkArray::GetMark( SCROW nRow ) const
{
    SCSIZE i;
    if (Search( nRow, i ))
        return mvData[i].bMarked;
    else
        return false;

}

void ScMarkArray::SetMarkArea( SCROW nStartRow, SCROW nEndRow, bool bMarked )
{
    if (!(mrSheetLimits.ValidRow(nStartRow) && mrSheetLimits.ValidRow(nEndRow)))
        return;

    if ((nStartRow == 0) && (nEndRow == mrSheetLimits.mnMaxRow))
    {
        Reset(bMarked);
    }
    else
    {
        SCSIZE ni;          // number of entries in beginning
        SCSIZE nInsert;     // insert position (mnMaxRow+1 := no insert)
        bool bCombined = false;
        bool bSplit = false;
        if ( nStartRow > 0 )
        {
            // skip beginning
            SCSIZE nIndex;
            Search( nStartRow, nIndex );
            ni = nIndex;

            nInsert = mrSheetLimits.GetMaxRowCount();
            if ( mvData[ni].bMarked != bMarked )
            {
                if ( ni == 0 || (mvData[ni-1].nRow < nStartRow - 1) )
                {   // may be a split or a simple insert or just a shrink,
                    // row adjustment is done further down
                    if ( mvData[ni].nRow > nEndRow )
                        bSplit = true;
                    ni++;
                    nInsert = ni;
                }
                else if ( ni > 0 && mvData[ni-1].nRow == nStartRow - 1 )
                    nInsert = ni;
            }
            if ( ni > 0 && mvData[ni-1].bMarked == bMarked )
            {   // combine
                mvData[ni-1].nRow = nEndRow;
                nInsert = mrSheetLimits.GetMaxRowCount();
                bCombined = true;
            }
        }
        else
        {
            nInsert = 0;
            ni = 0;
        }

        SCSIZE nj = ni;     // stop position of range to replace
        while ( nj < mvData.size() && mvData[nj].nRow <= nEndRow )
            nj++;
        if ( !bSplit )
        {
            if ( nj < mvData.size() && mvData[nj].bMarked == bMarked )
            {   // combine
                if ( ni > 0 )
                {
                    if ( mvData[ni-1].bMarked == bMarked )
                    {   // adjacent entries
                        mvData[ni-1].nRow = mvData[nj].nRow;
                        nj++;
                    }
                    else if ( ni == nInsert )
                        mvData[ni-1].nRow = nStartRow - 1;   // shrink
                }
                nInsert = mrSheetLimits.GetMaxRowCount();
                bCombined = true;
            }
            else if ( ni > 0 && ni == nInsert )
                mvData[ni-1].nRow = nStartRow - 1;   // shrink
        }
        if ( ni < nj )
        {   // remove middle entries
            if ( !bCombined )
            {   // replace one entry
                mvData[ni].nRow = nEndRow;
                mvData[ni].bMarked = bMarked;
                ni++;
                nInsert = mrSheetLimits.GetMaxRowCount();
            }
            if ( ni < nj )
            {   // remove entries
                mvData.erase(mvData.begin() + ni, mvData.begin() + nj);
            }
        }

        if ( nInsert < sal::static_int_cast<SCSIZE>(mrSheetLimits.GetMaxRowCount()) )
        {   // insert or append new entry
            if ( nInsert <= mvData.size() )
            {
                if ( !bSplit )
                    mvData.insert(mvData.begin() + nInsert, { nEndRow, bMarked });
                else
                {
                    mvData.insert(mvData.begin() + nInsert, 2, { nEndRow, bMarked });
                    mvData[nInsert+1] = mvData[nInsert-1];
                }
            }
            else
                mvData.push_back(ScMarkEntry{ nEndRow, bMarked });
            if ( nInsert )
                mvData[nInsert-1].nRow = nStartRow - 1;
        }
    }
}

/**
  optimised init-from-range-list. Specifically this is optimised for cases
  where we have very large data columns with lots and lots of ranges.
*/
void ScMarkArray::Set( const std::vector<ScMarkEntry> & rMarkEntries )
{
    mvData = rMarkEntries;
}

bool ScMarkArray::IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const
{
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;

    if (Search( nStartRow, nStartIndex ))
        if (mvData[nStartIndex].bMarked)
            if (Search( nEndRow, nEndIndex ))
                if (nEndIndex==nStartIndex)
                    return true;

    return false;
}

bool ScMarkArray::HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const
{
    bool bRet = false;
    if ( mvData.size() == 1 )
    {
        if ( mvData[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = mrSheetLimits.mnMaxRow;
            bRet = true;
        }
    }
    else if ( mvData.size() == 2 )
    {
        if ( mvData[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = mvData[0].nRow;
        }
        else
        {
            rStartRow = mvData[0].nRow + 1;
            rEndRow = mrSheetLimits.mnMaxRow;
        }
        bRet = true;
    }
    else if ( mvData.size() == 3 )
    {
        if ( mvData[1].bMarked )
        {
            rStartRow = mvData[0].nRow + 1;
            rEndRow = mvData[1].nRow;
            bRet = true;
        }
    }
    return bRet;
}

bool ScMarkArray::operator==( const ScMarkArray& rOther ) const
{
    return mvData == rOther.mvData;
}

ScMarkArray& ScMarkArray::operator=( const ScMarkArray& rOther )
{
    mvData = rOther.mvData;
    return *this;
}

ScMarkArray& ScMarkArray::operator=(ScMarkArray&& rOther) noexcept
{
    mvData = std::move(rOther.mvData);
    return *this;
}

SCROW ScMarkArray::GetNextMarked( SCROW nRow, bool bUp ) const
{
    SCROW nRet = nRow;
    if (mrSheetLimits.ValidRow(nRow))
    {
        SCSIZE nIndex;
        Search(nRow, nIndex);
        if (!mvData[nIndex].bMarked)
        {
            if (bUp)
            {
                if (nIndex>0)
                    nRet = mvData[nIndex-1].nRow;
                else
                    nRet = -1;
            }
            else
                nRet = mvData[nIndex].nRow + 1;
        }
    }
    return nRet;
}

SCROW ScMarkArray::GetMarkEnd( SCROW nRow, bool bUp ) const
{
    SCROW nRet;
    SCSIZE nIndex;
    Search(nRow, nIndex);
    assert( mvData[nIndex].bMarked && "GetMarkEnd without bMarked" );
    if (bUp)
    {
        if (nIndex>0)
            nRet = mvData[nIndex-1].nRow + 1;
        else
            nRet = 0;
    }
    else
        nRet = mvData[nIndex].nRow;

    return nRet;
}

void ScMarkArray::Shift(SCROW nStartRow, long nOffset)
{
    if (nOffset == 0 || nStartRow > mrSheetLimits.mnMaxRow)
        return;

    for (size_t i=0; i < mvData.size(); ++i)
    {
        auto& rEntry = mvData[i];

        if (rEntry.nRow < nStartRow)
            continue;
        rEntry.nRow += nOffset;
        if (rEntry.nRow < 0)
        {
            rEntry.nRow = 0;
        }
        else if (rEntry.nRow > mrSheetLimits.mnMaxRow)
        {
            rEntry.nRow = mrSheetLimits.mnMaxRow;
        }
    }
}

void ScMarkArray::Intersect(const ScMarkArray& rOther)
{
    size_t i = 0;
    size_t j = 0;

    std::vector<ScMarkEntry> aEntryArray;
    aEntryArray.reserve(std::max(mvData.size(), rOther.mvData.size()));

    while (i < mvData.size() && j < rOther.mvData.size())
    {
        const auto& rEntry = mvData[i];
        const auto& rOtherEntry = rOther.mvData[j];

        if (rEntry.bMarked != rOtherEntry.bMarked)
        {
            if (!rOtherEntry.bMarked)
            {
                aEntryArray.push_back(rOther.mvData[j++]);
                while (i < mvData.size() && mvData[i].nRow <= rOtherEntry.nRow)
                    ++i;
            }
            else // rEntry not marked
            {
                aEntryArray.push_back(mvData[i++]);
                while (j < rOther.mvData.size() && rOther.mvData[j].nRow <= rEntry.nRow)
                    ++j;
            }
        }
        else // rEntry.bMarked == rOtherEntry.bMarked
        {
            if (rEntry.bMarked) // both marked
            {
                if (rEntry.nRow <= rOtherEntry.nRow)
                {
                    aEntryArray.push_back(mvData[i++]); // upper row
                    if (rEntry.nRow == rOtherEntry.nRow)
                        ++j;
                }
                else
                {
                    aEntryArray.push_back(rOther.mvData[j++]); // upper row
                }
            }
            else // both not marked
            {
                if (rEntry.nRow <= rOtherEntry.nRow)
                {
                    aEntryArray.push_back(rOther.mvData[j++]); // lower row
                    while (i < mvData.size() && mvData[i].nRow <= rOtherEntry.nRow)
                        ++i;
                }
                else
                {
                    aEntryArray.push_back(mvData[i++]); // lower row
                    while (j < rOther.mvData.size() && rOther.mvData[j].nRow <= rEntry.nRow)
                        ++j;
                }
            }
        }
    }

    assert((i == mvData.size() || j == rOther.mvData.size()) && "Unexpected case.");

    if (i == mvData.size())
    {
        aEntryArray.insert(aEntryArray.end(), rOther.mvData.begin() + j, rOther.mvData.end());
    }
    else // j == rOther.nCount
    {
        aEntryArray.insert(aEntryArray.end(), mvData.begin() + i, mvData.end());
    }

    mvData = std::move(aEntryArray);
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

void ScMarkArrayIter::reset( const ScMarkArray* pNewArray )
{
    pArray = pNewArray;
    nPos = 0;
}

bool ScMarkArrayIter::Next( SCROW& rTop, SCROW& rBottom )
{
    if (!pArray)
        return false;
    if ( nPos >= pArray->mvData.size() )
        return false;
    while (!pArray->mvData[nPos].bMarked)
    {
        ++nPos;
        if ( nPos >= pArray->mvData.size() )
            return false;
    }
    rBottom = pArray->mvData[nPos].nRow;
    if (nPos==0)
        rTop = 0;
    else
        rTop = pArray->mvData[nPos-1].nRow + 1;
    ++nPos;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
