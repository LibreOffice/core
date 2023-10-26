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

#include <markmulti.hxx>
#include <markarr.hxx>
#include <rangelst.hxx>
#include <segmenttree.hxx>
#include <sheetlimits.hxx>

#include <o3tl/safeint.hxx>

#include <algorithm>

ScMultiSel::ScMultiSel(const ScSheetLimits& rSheetLimits)
    : aRowSel(rSheetLimits), mrSheetLimits(rSheetLimits)
{
}

ScMultiSel& ScMultiSel::operator=(const ScMultiSel& rOther)
{
    aMultiSelContainer = rOther.aMultiSelContainer;
    aRowSel = rOther.aRowSel;
    return *this;
}

ScMultiSel& ScMultiSel::operator=(ScMultiSel&& rOther)
{
    aMultiSelContainer = std::move(rOther.aMultiSelContainer);
    aRowSel = std::move(rOther.aRowSel);
    return *this;
}


void ScMultiSel::Clear()
{
    aMultiSelContainer.clear();
    aRowSel.Reset();
}

SCCOL ScMultiSel::GetMultiSelectionCount() const
{
    SCCOL nCount = 0;
    for (const auto & i : aMultiSelContainer)
        if (i.HasMarks())
            ++nCount;
    return nCount;
}

bool ScMultiSel::HasMarks( SCCOL nCol ) const
{
    if ( aRowSel.HasMarks() )
        return true;
    return nCol < static_cast<SCCOL>(aMultiSelContainer.size()) && aMultiSelContainer[nCol].HasMarks();
}

bool ScMultiSel::HasOneMark( SCCOL nCol, SCROW& rStartRow, SCROW& rEndRow ) const
{
    SCROW nRow1 = -1, nRow2 = -1, nRow3 = -1, nRow4 = -1;
    bool aResult1 = aRowSel.HasOneMark( nRow1, nRow2 );
    bool aResult2 = nCol < static_cast<SCCOL>(aMultiSelContainer.size())
                    && aMultiSelContainer[nCol].HasOneMark( nRow3, nRow4 );

    if ( aResult1 || aResult2 )
    {
        if ( aResult1 && aResult2 )
        {
            if ( ( nRow2 + 1 ) < nRow3 )
                return false;
            if ( ( nRow4 + 1 ) < nRow1 )
                return false;

            auto aRows = std::minmax( { nRow1, nRow2, nRow3, nRow4 } );
            rStartRow = aRows.first;
            rEndRow = aRows.second;
            return true;
        }
        if ( aResult1 )
        {
            rStartRow = nRow1;
            rEndRow = nRow2;
            return true;
        }

        rStartRow = nRow3;
        rEndRow = nRow4;
        return true;
    }

    return false;
}

bool ScMultiSel::GetMark( SCCOL nCol, SCROW nRow ) const
{
    if ( aRowSel.GetMark( nRow ) )
        return true;
    return nCol < static_cast<SCCOL>(aMultiSelContainer.size()) && aMultiSelContainer[nCol].GetMark(nRow);
}

bool ScMultiSel::IsAllMarked( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const
{
    bool bHasMarks1 = aRowSel.HasMarks();
    bool bHasMarks2 = nCol < static_cast<SCCOL>(aMultiSelContainer.size()) && aMultiSelContainer[nCol].HasMarks();

    if ( !bHasMarks1 && !bHasMarks2 )
        return false;

    if ( bHasMarks1 && bHasMarks2 )
    {
        if ( aRowSel.IsAllMarked( nStartRow, nEndRow ) ||
             aMultiSelContainer[nCol].IsAllMarked( nStartRow, nEndRow ) )
            return true;
        ScMultiSelIter aMultiIter( *this, nCol );
        ScFlatBoolRowSegments::RangeData aRowRange;
        bool bRet = aMultiIter.GetRangeData( nStartRow, aRowRange );
        return bRet && aRowRange.mbValue && aRowRange.mnRow2 >= nEndRow;
    }

    if ( bHasMarks1 )
        return aRowSel.IsAllMarked( nStartRow, nEndRow );

    return aMultiSelContainer[nCol].IsAllMarked( nStartRow, nEndRow );
}

bool ScMultiSel::HasEqualRowsMarked( SCCOL nCol1, SCCOL nCol2 ) const
{
    bool bCol1Exists = nCol1 < static_cast<SCCOL>(aMultiSelContainer.size());
    bool bCol2Exists = nCol2 < static_cast<SCCOL>(aMultiSelContainer.size());
    if ( bCol1Exists || bCol2Exists )
    {
        if ( bCol1Exists && bCol2Exists )
            return aMultiSelContainer[nCol1] == aMultiSelContainer[nCol2];
        else if ( bCol1Exists )
            return !aMultiSelContainer[nCol1].HasMarks();
        else
            return !aMultiSelContainer[nCol2].HasMarks();
    }

    return true;
}

SCCOL ScMultiSel::GetStartOfEqualColumns( SCCOL nLastCol, SCCOL nMinCol ) const
{
    if( nMinCol > nLastCol )
        return nMinCol;
    if( nLastCol >= static_cast<SCCOL>(aMultiSelContainer.size()))
    {
        if( nMinCol >= static_cast<SCCOL>(aMultiSelContainer.size()))
            return nMinCol;
        SCCOL nCol = static_cast<SCCOL>(aMultiSelContainer.size()) - 1;
        while( nCol >= nMinCol && aMultiSelContainer[nCol] == aRowSel )
            --nCol;
        return nCol + 1;
    }
    SCCOL nCol = nLastCol - 1;
    while( nCol >= nMinCol && aMultiSelContainer[nCol] == aMultiSelContainer[nLastCol] )
        --nCol;
    return nCol + 1;
}

SCROW ScMultiSel::GetNextMarked( SCCOL nCol, SCROW nRow, bool bUp ) const
{
    if ( nCol >= static_cast<SCCOL>(aMultiSelContainer.size()) || !aMultiSelContainer[nCol].HasMarks() )
        return aRowSel.GetNextMarked( nRow, bUp );

    SCROW nRow1, nRow2;
    nRow1 = aRowSel.GetNextMarked( nRow, bUp );
    nRow2 = aMultiSelContainer[nCol].GetNextMarked( nRow, bUp );
    if ( nRow1 == nRow2 )
        return nRow1;
    if ( nRow1 == -1 )
        return nRow2;
    if ( nRow2 == -1 )
        return nRow1;

    PutInOrder( nRow1, nRow2 );
    return ( bUp ? nRow2 : nRow1 );
}

void ScMultiSel::MarkAllCols( SCROW nStartRow, SCROW nEndRow )
{
    aMultiSelContainer.resize(mrSheetLimits.mnMaxCol+1, ScMarkArray(mrSheetLimits));
    for ( SCCOL nCol = mrSheetLimits.mnMaxCol; nCol >= 0; --nCol )
    {
        aMultiSelContainer[nCol].SetMarkArea( nStartRow, nEndRow, true );
    }
}

void ScMultiSel::SetMarkArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow, bool bMark )
{
    if ( nStartCol == 0 && nEndCol == mrSheetLimits.mnMaxCol )
    {
        aRowSel.SetMarkArea( nStartRow, nEndRow, bMark );
        if ( !bMark )
        {
            // Remove any per column marks for the row range.
            for ( auto& aIter : aMultiSelContainer )
                if ( aIter.HasMarks() )
                    aIter.SetMarkArea( nStartRow, nEndRow, false );
        }
        return;
    }

    // Bad case - we need to extend aMultiSelContainer size to MAXCOL
    // and move row marks from aRowSel to aMultiSelContainer
    if ( !bMark && aRowSel.HasMarks() )
    {
        SCROW nBeg, nLast = nEndRow;
        if ( aRowSel.GetMark( nStartRow ) )
        {
            nBeg = nStartRow;
            nLast = aRowSel.GetMarkEnd( nStartRow, false );
        }
        else
        {
            nBeg = aRowSel.GetNextMarked( nStartRow, false );
            if ( nBeg != mrSheetLimits.GetMaxRowCount() )
                nLast = aRowSel.GetMarkEnd( nBeg, false );
        }

        if ( nBeg != mrSheetLimits.GetMaxRowCount() && nLast >= nEndRow && nBeg <= nEndRow )
            MarkAllCols( nBeg, nEndRow );
        else
        {
            while ( nBeg != mrSheetLimits.GetMaxRowCount() && nLast < nEndRow )
            {
                MarkAllCols( nBeg, nLast );
                nBeg = aRowSel.GetNextMarked( nLast + 1, false );
                if ( nBeg != mrSheetLimits.GetMaxRowCount() )
                    nLast = aRowSel.GetMarkEnd( nBeg, false );
            }
            if ( nBeg != mrSheetLimits.GetMaxRowCount() && nLast >= nEndRow && nBeg <= nEndRow )
                MarkAllCols( nBeg, nEndRow );
        }

        aRowSel.SetMarkArea( nStartRow, nEndRow, false );
    }

    if (nEndCol >= static_cast<SCCOL>(aMultiSelContainer.size()))
        aMultiSelContainer.resize(nEndCol+1, ScMarkArray(mrSheetLimits));
    for ( SCCOL nColIter = nEndCol; nColIter >= nStartCol; --nColIter )
        aMultiSelContainer[nColIter].SetMarkArea( nStartRow, nEndRow, bMark );
}

/**
  optimised init-from-range-list. Specifically this is optimised for cases
  where we have very large data columns with lots and lots of ranges.
*/
void ScMultiSel::Set( ScRangeList const & rList )
{
    Clear();
    if (rList.empty())
        return;

    // sort by row to make the combining/merging faster
    auto aNewList = rList;
    std::sort(aNewList.begin(), aNewList.end(),
        [](const ScRange& lhs, const ScRange& rhs)
        {
            return lhs.aStart.Row() < rhs.aStart.Row();
        });

    std::vector<std::vector<ScMarkEntry>> aMarkEntriesPerCol(mrSheetLimits.mnMaxCol+1);

    SCCOL nMaxCol = -1;
    for (const ScRange& rRange : aNewList)
    {
        SCCOL nStartCol = rRange.aStart.Col();
        SCROW nStartRow = rRange.aStart.Row();
        SCCOL nEndCol = rRange.aEnd.Col();
        SCROW nEndRow = rRange.aEnd.Row();
        assert( nEndRow >= nStartRow && "this method assumes the input data has ranges with endrow>=startrow");
        assert( nEndCol >= nStartCol && "this method assumes the input data has ranges with endcol>=startcol");
        if ( nStartCol == 0 && nEndCol == mrSheetLimits.mnMaxCol )
            aRowSel.SetMarkArea( nStartRow, nEndRow, /*bMark*/true );
        else
        {
            for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
            {
                auto & rMarkEntries = aMarkEntriesPerCol[nCol];
                int nEntries = rMarkEntries.size();
                if (nEntries > 1 && nStartRow >= rMarkEntries[nEntries-2].nRow+1
                   && nStartRow <= rMarkEntries[nEntries-1].nRow+1)
                {
                    // overlaps or directly adjacent previous range
                    rMarkEntries.back().nRow = std::max(nEndRow, rMarkEntries.back().nRow);
                }
                else
                {
                    // new range
                    if (nStartRow > 0)
	                    rMarkEntries.emplace_back(ScMarkEntry{nStartRow-1, false});
                    rMarkEntries.emplace_back(ScMarkEntry{nEndRow, true});
                }
            }
            nMaxCol = std::max(nMaxCol, nEndCol);
        }
    }

    aMultiSelContainer.resize(nMaxCol+1, ScMarkArray(mrSheetLimits));
    for (SCCOL nCol = 0; nCol<=nMaxCol; ++nCol)
        if (!aMarkEntriesPerCol[nCol].empty())
            aMultiSelContainer[nCol].Set( std::move(aMarkEntriesPerCol[nCol]) );
}

bool ScMultiSel::IsRowMarked( SCROW nRow ) const
{
    return aRowSel.GetMark( nRow );
}

bool ScMultiSel::IsRowRangeMarked( SCROW nStartRow, SCROW nEndRow ) const
{
    if ( !aRowSel.GetMark( nStartRow ) )
        return false;
    SCROW nLast = aRowSel.GetMarkEnd( nStartRow, false );
    return ( nLast >= nEndRow );
}

ScMarkArray ScMultiSel::GetMarkArray( SCCOL nCol ) const
{
    ScMultiSelIter aMultiIter( *this, nCol );
    ScMarkArray aMarkArray(mrSheetLimits);
    SCROW nTop, nBottom;
    while( aMultiIter.Next( nTop, nBottom ) )
        aMarkArray.SetMarkArea( nTop, nBottom, true );
    return aMarkArray;
}

bool ScMultiSel::HasAnyMarks() const
{
    if ( aRowSel.HasMarks() )
        return true;
    for ( const auto& aPair : aMultiSelContainer )
        if ( aPair.HasMarks() )
            return true;
    return false;
}

void ScMultiSel::ShiftCols(SCCOL nStartCol, sal_Int32 nColOffset)
{
    if (nStartCol > mrSheetLimits.mnMaxCol || nStartCol >= static_cast<SCCOL>(aMultiSelContainer.size()))
        return;

    if (nColOffset > 0)
    {
        aMultiSelContainer.insert(aMultiSelContainer.begin() + nStartCol, nColOffset, ScMarkArray(mrSheetLimits));
    }
    else
    {
        sal_Int32 tempOffset = nStartCol - nColOffset >= static_cast<SCCOL>(aMultiSelContainer.size()) ? static_cast<SCCOL>(aMultiSelContainer.size()) - nStartCol -1: -1 * nColOffset;
        aMultiSelContainer.erase(aMultiSelContainer.begin() + nStartCol, aMultiSelContainer.begin() + nStartCol + tempOffset);
    }
}

void ScMultiSel::ShiftRows(SCROW nStartRow, sal_Int32 nRowOffset)
{
    for (auto& aPair: aMultiSelContainer)
        aPair.Shift(nStartRow, nRowOffset);
    aRowSel.Shift(nStartRow, nRowOffset);
}

const ScMarkArray* ScMultiSel::GetMultiSelArray( SCCOL nCol ) const
{
    if (nCol >= static_cast<SCCOL>(aMultiSelContainer.size()))
        return nullptr;
    return &aMultiSelContainer[nCol];
}

ScMultiSelIter::ScMultiSelIter( const ScMultiSel& rMultiSel, SCCOL nCol ) :
    aMarkArrayIter(nullptr),
    nNextSegmentStart(0)
{
    bool bHasMarks1 = rMultiSel.aRowSel.HasMarks();
    bool bHasMarks2 = nCol < static_cast<SCCOL>(rMultiSel.aMultiSelContainer.size())
                    && rMultiSel.aMultiSelContainer[nCol].HasMarks();

    if (bHasMarks1 && bHasMarks2)
    {
        pRowSegs.reset( new ScFlatBoolRowSegments(rMultiSel.mrSheetLimits.mnMaxRow) );
        pRowSegs->setFalse( 0, rMultiSel.mrSheetLimits.mnMaxRow );
        {
            ScMarkArrayIter aMarkIter( &rMultiSel.aRowSel );
            SCROW nTop, nBottom;
            while ( aMarkIter.Next( nTop, nBottom ) )
                pRowSegs->setTrue( nTop, nBottom );
        }

        {
            ScMarkArrayIter aMarkIter( &rMultiSel.aMultiSelContainer[nCol] );
            SCROW nTop, nBottom;
            while ( aMarkIter.Next( nTop, nBottom ) )
                pRowSegs->setTrue( nTop, nBottom );
        }
    }
    else if (bHasMarks1)
    {
        aMarkArrayIter.reset( &rMultiSel.aRowSel);
    }
    else if (bHasMarks2)
    {
        aMarkArrayIter.reset( &rMultiSel.aMultiSelContainer[nCol]);
    }
}

bool ScMultiSelIter::Next( SCROW& rTop, SCROW& rBottom )
{
    if (pRowSegs)
    {
        ScFlatBoolRowSegments::RangeData aRowRange;
        bool bRet = pRowSegs->getRangeData( nNextSegmentStart, aRowRange );
        if ( bRet && !aRowRange.mbValue )
        {
            nNextSegmentStart = aRowRange.mnRow2 + 1;
            bRet = pRowSegs->getRangeData( nNextSegmentStart, aRowRange );
        }
        if ( bRet )
        {
            rTop = aRowRange.mnRow1;
            rBottom = aRowRange.mnRow2;
            nNextSegmentStart = rBottom + 1;
        }
        return bRet;
    }

    return aMarkArrayIter.Next( rTop, rBottom);
}

bool ScMultiSelIter::GetRangeData( SCROW nRow, ScFlatBoolRowSegments::RangeData& rRowRange ) const
{
    assert(pRowSegs);
    return pRowSegs->getRangeData( nRow, rRowRange);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
