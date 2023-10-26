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

#include <memory>
#include <markdata.hxx>
#include <markarr.hxx>
#include <markmulti.hxx>
#include <rangelst.hxx>
#include <segmenttree.hxx>
#include <sheetlimits.hxx>
#include <document.hxx>
#include <columnspanset.hxx>
#include <fstalgorithm.hxx>
#include <unordered_map>

#include <osl/diagnose.h>

#include <mdds/flat_segment_tree.hpp>
#include <cassert>


ScMarkData::ScMarkData(const ScSheetLimits& rSheetLimits) :
    aMultiSel(rSheetLimits),
    mrSheetLimits(rSheetLimits)
{
    ResetMark();
}

ScMarkData& ScMarkData::operator=(const ScMarkData& rOther)
{
    maTabMarked = rOther.maTabMarked;
    aMarkRange = rOther.aMarkRange;
    aMultiRange = rOther.aMultiRange;
    aMultiSel = rOther.aMultiSel;
    aTopEnvelope = rOther.aTopEnvelope;
    aBottomEnvelope = rOther.aBottomEnvelope;
    aLeftEnvelope = rOther.aLeftEnvelope;
    aRightEnvelope = rOther.aRightEnvelope;
    bMarked = rOther.bMarked;
    bMultiMarked = rOther.bMultiMarked;
    bMarking = rOther.bMarking;
    bMarkIsNeg = rOther.bMarkIsNeg;
    return *this;
}

ScMarkData& ScMarkData::operator=(ScMarkData&& rOther)
{
    maTabMarked = std::move(rOther.maTabMarked);
    aMarkRange = std::move(rOther.aMarkRange);
    aMultiRange = std::move(rOther.aMultiRange);
    aMultiSel = std::move(rOther.aMultiSel);
    aTopEnvelope = std::move(rOther.aTopEnvelope);
    aBottomEnvelope = std::move(rOther.aBottomEnvelope);
    aLeftEnvelope = std::move(rOther.aLeftEnvelope);
    aRightEnvelope = std::move(rOther.aRightEnvelope);
    bMarked = rOther.bMarked;
    bMultiMarked = rOther.bMultiMarked;
    bMarking = rOther.bMarking;
    bMarkIsNeg = rOther.bMarkIsNeg;
    return *this;
}


void ScMarkData::ResetMark()
{
    aMultiSel.Clear();

    bMarked = bMultiMarked = false;
    bMarking = bMarkIsNeg = false;
    aTopEnvelope.RemoveAll();
    aBottomEnvelope.RemoveAll();
    aLeftEnvelope.RemoveAll();
    aRightEnvelope.RemoveAll();
}

void ScMarkData::SetMarkArea( const ScRange& rRange )
{
    aMarkRange = rRange;
    aMarkRange.PutInOrder();
    if ( !bMarked )
    {
        // Upon creation of a document ScFormatShell GetTextAttrState
        // may query (default) attributes although no sheet is marked yet.
        // => mark that one.
        if ( !GetSelectCount() )
            maTabMarked.insert( aMarkRange.aStart.Tab() );
        bMarked = true;
    }
}

void ScMarkData::SetMultiMarkArea( const ScRange& rRange, bool bMark, bool bSetupMulti )
{
    if ( aMultiSel.IsEmpty() )
    {
        // if simple mark range is set, copy to multi marks
        if ( bMarked && !bMarkIsNeg && !bSetupMulti )
        {
            bMarked = false;
            SCCOL nStartCol = aMarkRange.aStart.Col();
            SCCOL nEndCol = aMarkRange.aEnd.Col();
            PutInOrder( nStartCol, nEndCol );
            SetMultiMarkArea( aMarkRange, true, true );
        }
    }

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartCol, nEndCol );

    aMultiSel.SetMarkArea( nStartCol, nEndCol, nStartRow, nEndRow, bMark );

    if ( bMultiMarked )                 // Update aMultiRange
    {
        if ( nStartCol < aMultiRange.aStart.Col() )
            aMultiRange.aStart.SetCol( nStartCol );
        if ( nStartRow < aMultiRange.aStart.Row() )
            aMultiRange.aStart.SetRow( nStartRow );
        if ( nEndCol > aMultiRange.aEnd.Col() )
            aMultiRange.aEnd.SetCol( nEndCol );
        if ( nEndRow > aMultiRange.aEnd.Row() )
            aMultiRange.aEnd.SetRow( nEndRow );
    }
    else
    {
        aMultiRange = rRange;           // new
        bMultiMarked = true;
    }
}

void ScMarkData::SetAreaTab( SCTAB nTab )
{
    aMarkRange.aStart.SetTab(nTab);
    aMarkRange.aEnd.SetTab(nTab);
    aMultiRange.aStart.SetTab(nTab);
    aMultiRange.aEnd.SetTab(nTab);
}

void ScMarkData::SelectTable( SCTAB nTab, bool bNew )
{
    if ( bNew )
    {
        maTabMarked.insert( nTab );
    }
    else
    {
        maTabMarked.erase( nTab );
    }
}

bool ScMarkData::GetTableSelect( SCTAB nTab ) const
{
    return (maTabMarked.find( nTab ) != maTabMarked.end());
}

void ScMarkData::SelectOneTable( SCTAB nTab )
{
    maTabMarked.clear();
    maTabMarked.insert( nTab );
}

SCTAB ScMarkData::GetSelectCount() const
{
    return static_cast<SCTAB> ( maTabMarked.size() );
}

SCTAB ScMarkData::GetFirstSelected() const
{
    if (!maTabMarked.empty())
        return (*maTabMarked.begin());

    OSL_FAIL("GetFirstSelected: nothing selected");
    return 0;
}

SCTAB ScMarkData::GetLastSelected() const
{
    if (!maTabMarked.empty())
        return (*maTabMarked.rbegin());

    OSL_FAIL("GetLastSelected: nothing selected");
    return 0;
}

void ScMarkData::SetSelectedTabs(const MarkedTabsType& rTabs)
{
    MarkedTabsType aTabs(rTabs.begin(), rTabs.end());
    maTabMarked.swap(aTabs);
}

void ScMarkData::MarkToMulti()
{
    if ( bMarked && !bMarking )
    {
        SetMultiMarkArea( aMarkRange, !bMarkIsNeg );
        bMarked = false;

        //  check if all multi mark ranges have been removed
        if ( bMarkIsNeg && !HasAnyMultiMarks() )
            ResetMark();
    }
}

void ScMarkData::MarkToSimple()
{
    if ( bMarking )
        return;

    if ( bMultiMarked && bMarked )
        MarkToMulti();                  // may result in bMarked and bMultiMarked reset

    if ( !bMultiMarked )
        return;

    ScRange aNew = aMultiRange;

    bool bOk = false;
    SCCOL nStartCol = aNew.aStart.Col();
    SCCOL nEndCol   = aNew.aEnd.Col();

    while ( nStartCol < nEndCol && !aMultiSel.HasMarks( nStartCol ) )
        ++nStartCol;
    while ( nStartCol < nEndCol && !aMultiSel.HasMarks( nEndCol ) )
        --nEndCol;

    // Rows are only taken from MarkArray
    SCROW nStartRow, nEndRow;
    if ( aMultiSel.HasOneMark( nStartCol, nStartRow, nEndRow ) )
    {
        bOk = true;
        SCROW nCmpStart, nCmpEnd;
        for (SCCOL nCol=nStartCol+1; nCol<=nEndCol && bOk; nCol++)
            if ( !aMultiSel.HasOneMark( nCol, nCmpStart, nCmpEnd )
                    || nCmpStart != nStartRow || nCmpEnd != nEndRow )
                bOk = false;
    }

    if (bOk)
    {
        aNew.aStart.SetCol(nStartCol);
        aNew.aStart.SetRow(nStartRow);
        aNew.aEnd.SetCol(nEndCol);
        aNew.aEnd.SetRow(nEndRow);

        ResetMark();
        aMarkRange = aNew;
        bMarked = true;
        bMarkIsNeg = false;
    }
}

bool ScMarkData::IsCellMarked( SCCOL nCol, SCROW nRow, bool bNoSimple ) const
{
    if ( bMarked && !bNoSimple && !bMarkIsNeg )
        if ( aMarkRange.aStart.Col() <= nCol && aMarkRange.aEnd.Col() >= nCol &&
             aMarkRange.aStart.Row() <= nRow && aMarkRange.aEnd.Row() >= nRow )
            return true;

    if (bMultiMarked)
    {
        //TODO: test here for negative Marking ?

        return aMultiSel.GetMark( nCol, nRow );
    }

    return false;
}

bool ScMarkData::IsColumnMarked( SCCOL nCol ) const
{
    //  bMarkIsNeg meanwhile also for columns heads
    //TODO: GetMarkColumnRanges for completely marked column

    if ( bMarked && !bMarkIsNeg &&
                    aMarkRange.aStart.Col() <= nCol && aMarkRange.aEnd.Col() >= nCol &&
                    aMarkRange.aStart.Row() == 0    && aMarkRange.aEnd.Row() == mrSheetLimits.mnMaxRow )
        return true;

    if ( bMultiMarked && aMultiSel.IsAllMarked( nCol, 0, mrSheetLimits.mnMaxRow ) )
        return true;

    return false;
}

bool ScMarkData::IsRowMarked( SCROW nRow ) const
{
    //  bMarkIsNeg meanwhile also for row heads
    //TODO: GetMarkRowRanges for completely marked rows

    if ( bMarked && !bMarkIsNeg &&
                    aMarkRange.aStart.Col() == 0    && aMarkRange.aEnd.Col() == mrSheetLimits.mnMaxCol &&
                    aMarkRange.aStart.Row() <= nRow && aMarkRange.aEnd.Row() >= nRow )
        return true;

    if ( bMultiMarked )
        return aMultiSel.IsRowMarked( nRow );

    return false;
}

void ScMarkData::MarkFromRangeList( const ScRangeList& rList, bool bReset )
{
    if (bReset)
    {
        maTabMarked.clear();
        ResetMark();
    }

    size_t nCount = rList.size();
    if ( nCount == 1 && !bMarked && !bMultiMarked )
    {
        const ScRange& rRange = rList[ 0 ];
        SetMarkArea( rRange );
        SelectTable( rRange.aStart.Tab(), true );
    }
    else
    {
        for (size_t i=0; i < nCount; i++)
        {
            const ScRange& rRange = rList[ i ];
            SetMultiMarkArea( rRange );
            SelectTable( rRange.aStart.Tab(), true );
        }
    }
}

/**
  Optimise the case of constructing from a range list, speeds up import.
*/
ScMarkData::ScMarkData(const ScSheetLimits& rLimits, const ScRangeList& rList)
    : aMultiSel(rLimits),
      mrSheetLimits(rLimits)
{
    ResetMark();

    for (const ScRange& rRange : rList)
        maTabMarked.insert( rRange.aStart.Tab() );

    if (rList.size() > 1)
    {
        bMultiMarked = true;
        aMultiRange = rList.Combine();

        aMultiSel.Set( rList );
    }
    else if (rList.size() == 1)
    {
        const ScRange& rRange = rList[ 0 ];
        SetMarkArea( rRange );
    }
}


void ScMarkData::FillRangeListWithMarks( ScRangeList* pList, bool bClear, SCTAB nForTab ) const
{
    if (!pList)
        return;

    if (bClear)
        pList->RemoveAll();

    //TODO: for multiple selected tables enter multiple ranges !!!

    if ( bMultiMarked )
    {
        SCTAB nTab = (nForTab < 0 ? aMultiRange.aStart.Tab() : nForTab);

        SCCOL nStartCol = aMultiRange.aStart.Col();
        SCCOL nEndCol = aMultiRange.aEnd.Col();
        for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
        {
            if (aMultiSel.HasMarks( nCol ))
            {
                // Feeding column-wise fragments to ScRangeList::Join() is a
                // huge bottleneck, speed this up for multiple columns
                // consisting of identical row sets by building a column span
                // first. This is usually the case for filtered data, for
                // example.
                SCCOL nToCol = nCol+1;
                for ( ; nToCol <= nEndCol; ++nToCol)
                {
                    if (!aMultiSel.HasEqualRowsMarked(nCol, nToCol))
                        break;
                }
                --nToCol;
                ScRange aRange( nCol, 0, nTab, nToCol, 0, nTab );
                SCROW nTop, nBottom;
                ScMultiSelIter aMultiIter( aMultiSel, nCol );
                while ( aMultiIter.Next( nTop, nBottom ) )
                {
                    aRange.aStart.SetRow( nTop );
                    aRange.aEnd.SetRow( nBottom );
                    pList->Join( aRange );
                }
                nCol = nToCol;
            }
        }
    }

    if ( bMarked )
    {
        if (nForTab < 0)
            pList->push_back( aMarkRange );
        else
        {
            ScRange aRange( aMarkRange );
            aRange.aStart.SetTab( nForTab );
            aRange.aEnd.SetTab( nForTab );
            pList->push_back( aRange );
        }
    }
}

void ScMarkData::ExtendRangeListTables( ScRangeList* pList ) const
{
    if (!pList)
        return;

    ScRangeList aOldList(*pList);
    pList->RemoveAll();                 //TODO: or skip the existing below

    for (const auto& rTab : maTabMarked)
        for ( size_t i=0, nCount = aOldList.size(); i<nCount; i++)
        {
            ScRange aRange = aOldList[ i ];
            aRange.aStart.SetTab(rTab);
            aRange.aEnd.SetTab(rTab);
            pList->push_back( aRange );
        }
}

ScRangeList ScMarkData::GetMarkedRanges() const
{
    ScRangeList aRet;
    FillRangeListWithMarks(&aRet, false);
    return aRet;
}

ScRangeList ScMarkData::GetMarkedRangesForTab( SCTAB nTab ) const
{
    ScRangeList aRet;
    FillRangeListWithMarks(&aRet, false, nTab);
    return aRet;
}

std::vector<sc::ColRowSpan> ScMarkData::GetMarkedRowSpans() const
{
    typedef mdds::flat_segment_tree<SCCOLROW, bool> SpansType;

    ScRangeList aRanges = GetMarkedRanges();
    SpansType aSpans(0, mrSheetLimits.mnMaxRow+1, false);
    SpansType::const_iterator itPos = aSpans.begin();

    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
    {
        const ScRange& r = aRanges[i];
        itPos = aSpans.insert(itPos, r.aStart.Row(), r.aEnd.Row()+1, true).first;
    }

    return sc::toSpanArray<SCCOLROW,sc::ColRowSpan>(aSpans);
}

std::vector<sc::ColRowSpan> ScMarkData::GetMarkedColSpans() const
{

    if (bMultiMarked)
    {
        SCCOL nStartCol = aMultiRange.aStart.Col();
        SCCOL nEndCol = aMultiRange.aEnd.Col();
        if (bMarked)
        {
            // Use segment tree to merge marked with multi marked.
            typedef mdds::flat_segment_tree<SCCOLROW, bool> SpansType;
            SpansType aSpans(0, mrSheetLimits.mnMaxCol+1, false);
            SpansType::const_iterator itPos = aSpans.begin();
            do
            {
                if (aMultiSel.GetRowSelArray().HasMarks())
                {
                    itPos = aSpans.insert(itPos, nStartCol, nEndCol+1, true).first;
                    break;  // do; all columns marked
                }

                /* XXX if it turns out that span insert is too slow for lots of
                 * subsequent columns we could gather each span first and then
                 * insert. */
                for (SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol)
                {
                    const ScMarkArray* pMultiArray = aMultiSel.GetMultiSelArray( nCol );
                    if (pMultiArray && pMultiArray->HasMarks())
                        itPos = aSpans.insert(itPos, nCol, nCol+1, true).first;
                }
            }
            while(false);

            // Merge marked.
            aSpans.insert(itPos, aMarkRange.aStart.Col(), aMarkRange.aEnd.Col()+1, true);

            return sc::toSpanArray<SCCOLROW,sc::ColRowSpan>(aSpans);
        }
        else
        {
            // A plain vector is sufficient, avoid segment tree and conversion
            // to vector overhead.
            std::vector<sc::ColRowSpan> aVec;
            if (aMultiSel.GetRowSelArray().HasMarks())
            {
                aVec.emplace_back( nStartCol, nEndCol);
                return aVec;    // all columns marked
            }
            sc::ColRowSpan aSpan( -1, -1);
            for (SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol)
            {
                const ScMarkArray* pMultiArray = aMultiSel.GetMultiSelArray( nCol );
                if (pMultiArray && pMultiArray->HasMarks())
                {
                    if (aSpan.mnStart == -1)
                        aSpan.mnStart = nCol;
                    aSpan.mnEnd = nCol;
                }
                else
                {
                    // Add span gathered so far, if any.
                    if (aSpan.mnStart != -1)
                    {
                        aVec.push_back( aSpan);
                        aSpan.mnStart = -1;
                    }
                }
            }
            // Add last span, if any.
            if (aSpan.mnStart != -1)
                aVec.push_back( aSpan);
            return aVec;
        }
    }

    // Only reached if not multi marked.
    std::vector<sc::ColRowSpan> aVec;
    if (bMarked)
    {
        aVec.emplace_back( aMarkRange.aStart.Col(), aMarkRange.aEnd.Col());
    }
    return aVec;
}

bool ScMarkData::IsAllMarked( const ScRange& rRange ) const
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();

    if ( !bMultiMarked )
    {
        if ( bMarked && !bMarkIsNeg &&
             aMarkRange.aStart.Col() <= nStartCol && aMarkRange.aEnd.Col() >= nEndCol &&
             aMarkRange.aStart.Row() <= nStartRow && aMarkRange.aEnd.Row() >= nEndRow )
            return true;
        return false;
    }

    bool bOk = true;

    if ( nStartCol == 0 && nEndCol == mrSheetLimits.mnMaxCol )
        return aMultiSel.IsRowRangeMarked( nStartRow, nEndRow );

    for (SCCOL nCol=nStartCol; nCol<=nEndCol && bOk; nCol++)
        if ( !aMultiSel.IsAllMarked( nCol, nStartRow, nEndRow ) )
            bOk = false;

    return bOk;
}

SCCOL ScMarkData::GetStartOfEqualColumns( SCCOL nLastCol, SCCOL nMinCol ) const
{
    if( !bMultiMarked )
    {
        if ( bMarked && !bMarkIsNeg )
        {
            if( aMarkRange.aEnd.Col() >= nMinCol && aMarkRange.aStart.Col() < nLastCol )
                return aMarkRange.aEnd.Col() + 1;
            if( aMarkRange.aEnd.Col() >= nLastCol && aMarkRange.aStart.Col() <= nMinCol )
                return aMarkRange.aStart.Col();
        }
        return nMinCol;
    }
    return aMultiSel.GetStartOfEqualColumns( nLastCol, nMinCol );
}

SCROW ScMarkData::GetNextMarked( SCCOL nCol, SCROW nRow, bool bUp ) const
{
    if ( !bMultiMarked )
        return nRow;

    return aMultiSel.GetNextMarked( nCol, nRow, bUp );
}

bool ScMarkData::HasMultiMarks( SCCOL nCol ) const
{
    if ( !bMultiMarked )
        return false;

    return aMultiSel.HasMarks( nCol );
}

bool ScMarkData::HasAnyMultiMarks() const
{
    if ( !bMultiMarked )
        return false;

    return aMultiSel.HasAnyMarks();
}

void ScMarkData::InsertTab( SCTAB nTab )
{
    std::set<SCTAB> tabMarked;
    for (const auto& rTab : maTabMarked)
    {
        if (rTab < nTab)
            tabMarked.insert(rTab);
        else
            tabMarked.insert(rTab + 1);
    }
    maTabMarked.swap(tabMarked);
}

void ScMarkData::DeleteTab( SCTAB nTab )
{
    std::set<SCTAB> tabMarked;
    for (const auto& rTab : maTabMarked)
    {
        if (rTab < nTab)
            tabMarked.insert(rTab);
        else if (rTab > nTab)
            tabMarked.insert(rTab - 1);
    }
    maTabMarked.swap(tabMarked);
}

void ScMarkData::ShiftCols(const ScDocument& rDoc, SCCOL nStartCol, sal_Int32 nColOffset)
{
    if (bMarked)
        aMarkRange.IncColIfNotLessThan(rDoc, nStartCol, nColOffset);
    if (bMultiMarked)
    {
        aMultiRange.IncColIfNotLessThan(rDoc, nStartCol, nColOffset);
        aMultiSel.ShiftCols(nStartCol, nColOffset);
    }
}

void ScMarkData::ShiftRows(const ScDocument& rDoc, SCROW nStartRow, sal_Int32 nRowOffset)
{
    if (bMarked)
    {
        aMarkRange.IncRowIfNotLessThan(rDoc, nStartRow, nRowOffset);
    }
    else if (bMultiMarked)
    {
        aMultiSel.ShiftRows(nStartRow, nRowOffset);
        aMultiRange.IncRowIfNotLessThan(rDoc, nStartRow, nRowOffset);
    }
}

static void lcl_AddRanges(ScRange& rRangeDest, const ScRange& rNewRange )
{
    SCCOL nStartCol = rNewRange.aStart.Col();
    SCROW nStartRow = rNewRange.aStart.Row();
    SCCOL nEndCol = rNewRange.aEnd.Col();
    SCROW nEndRow = rNewRange.aEnd.Row();
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartCol, nEndCol );
    if ( nStartCol < rRangeDest.aStart.Col() )
        rRangeDest.aStart.SetCol( nStartCol );
    if ( nStartRow < rRangeDest.aStart.Row() )
        rRangeDest.aStart.SetRow( nStartRow );
    if ( nEndCol > rRangeDest.aEnd.Col() )
        rRangeDest.aEnd.SetCol( nEndCol );
    if ( nEndRow > rRangeDest.aEnd.Row() )
        rRangeDest.aEnd.SetRow( nEndRow );
}

void ScMarkData::GetSelectionCover( ScRange& rRange )
{
    if( bMultiMarked )
    {
        rRange = aMultiRange;
        SCCOL nStartCol = aMultiRange.aStart.Col(), nEndCol = aMultiRange.aEnd.Col();
        PutInOrder( nStartCol, nEndCol );
        nStartCol = ( nStartCol == 0 ) ? nStartCol : nStartCol - 1;
        nEndCol = ( nEndCol == mrSheetLimits.mnMaxCol ) ? nEndCol : nEndCol + 1;
        std::unique_ptr<ScFlatBoolRowSegments> pPrevColMarkedRows;
        std::unique_ptr<ScFlatBoolRowSegments> pCurColMarkedRows;
        std::unordered_map<SCROW,ScFlatBoolColSegments> aRowToColSegmentsInTopEnvelope;
        std::unordered_map<SCROW,ScFlatBoolColSegments> aRowToColSegmentsInBottomEnvelope;
        ScFlatBoolRowSegments aNoRowsMarked(mrSheetLimits.mnMaxRow);
        aNoRowsMarked.setFalse( 0, mrSheetLimits.mnMaxRow );

        bool bPrevColUnMarked = false;

        for ( SCCOL nCol=nStartCol; nCol <= nEndCol; nCol++ )
        {
            SCROW nTop, nBottom;
            bool bCurColUnMarked = !aMultiSel.HasMarks( nCol );
            if ( !bCurColUnMarked )
            {
                pCurColMarkedRows.reset( new ScFlatBoolRowSegments(mrSheetLimits.mnMaxRow) );
                pCurColMarkedRows->setFalse( 0, mrSheetLimits.mnMaxRow );
                ScMultiSelIter aMultiIter( aMultiSel, nCol );
                ScFlatBoolRowSegments::ForwardIterator aPrevItr(
                    pPrevColMarkedRows ? *pPrevColMarkedRows
                                       : aNoRowsMarked); // For finding left envelope
                ScFlatBoolRowSegments::ForwardIterator aPrevItr1(
                    pPrevColMarkedRows ? *pPrevColMarkedRows
                                       : aNoRowsMarked); // For finding right envelope
                SCROW nTopPrev = 0, nBottomPrev = 0; // For right envelope
                while ( aMultiIter.Next( nTop, nBottom ) )
                {
                    pCurColMarkedRows->setTrue( nTop, nBottom );
                    if( bPrevColUnMarked && ( nCol > nStartCol ))
                    {
                        ScRange aAddRange(nCol - 1, nTop, aMultiRange.aStart.Tab(),
                                          nCol - 1, nBottom, aMultiRange.aStart.Tab());
                        lcl_AddRanges( rRange, aAddRange ); // Left envelope
                        aLeftEnvelope.push_back( aAddRange );
                    }
                    else if( nCol > nStartCol )
                    {
                        SCROW nTop1 = nTop, nBottom1 = nTop;
                        while( nTop1 <= nBottom && nBottom1 <= nBottom )
                        {
                            bool bRangeMarked = false;
                            const bool bHasValue = aPrevItr.getValue( nTop1, bRangeMarked );
                            assert(bHasValue); (void)bHasValue;
                            if( bRangeMarked )
                            {
                                nTop1 = aPrevItr.getLastPos() + 1;
                                nBottom1 = nTop1;
                            }
                            else
                            {
                                nBottom1 = aPrevItr.getLastPos();
                                if( nBottom1 > nBottom )
                                    nBottom1 = nBottom;
                                ScRange aAddRange( nCol - 1, nTop1, aMultiRange.aStart.Tab(),
                                                   nCol - 1, nBottom1, aMultiRange.aStart.Tab() );
                                lcl_AddRanges( rRange, aAddRange ); // Left envelope
                                aLeftEnvelope.push_back( aAddRange );
                                nTop1 = ++nBottom1;
                            }
                        }
                        while( nTopPrev <= nBottom && nBottomPrev <= nBottom )
                        {
                            bool bRangeMarked;
                            const bool bHasValue = aPrevItr1.getValue( nTopPrev, bRangeMarked );
                            assert(bHasValue); (void)bHasValue;
                            if( bRangeMarked )
                            {
                                nBottomPrev = aPrevItr1.getLastPos();
                                if( nTopPrev < nTop )
                                {
                                    if( nBottomPrev >= nTop )
                                    {
                                        nBottomPrev = nTop - 1;
                                        ScRange aAddRange( nCol, nTopPrev, aMultiRange.aStart.Tab(),
                                                           nCol, nBottomPrev, aMultiRange.aStart.Tab());
                                        lcl_AddRanges( rRange, aAddRange ); // Right envelope
                                        aRightEnvelope.push_back( aAddRange );
                                        nTopPrev = nBottomPrev = (nBottom + 1);
                                    }
                                    else
                                    {
                                        ScRange aAddRange( nCol, nTopPrev, aMultiRange.aStart.Tab(),
                                                           nCol, nBottomPrev, aMultiRange.aStart.Tab());
                                        lcl_AddRanges( rRange, aAddRange ); // Right envelope
                                        aRightEnvelope.push_back( aAddRange );
                                        nTopPrev = ++nBottomPrev;
                                    }
                                }
                                else
                                    nTopPrev = nBottomPrev = ( nBottom + 1 );
                            }
                            else
                            {
                                nBottomPrev = aPrevItr1.getLastPos();
                                nTopPrev = ++nBottomPrev;
                            }
                        }
                    }
                    if( nTop )
                    {
                        ScRange aAddRange( nCol, nTop - 1, aMultiRange.aStart.Tab(),
                                           nCol, nTop - 1, aMultiRange.aStart.Tab());
                        lcl_AddRanges( rRange, aAddRange ); // Top envelope
                        auto it = aRowToColSegmentsInTopEnvelope.find(nTop - 1);
                        if (it == aRowToColSegmentsInTopEnvelope.end())
                            it = aRowToColSegmentsInTopEnvelope.emplace(nTop - 1, ScFlatBoolColSegments(mrSheetLimits.mnMaxCol)).first;
                        it->second.setTrue( nCol, nCol );
                    }
                    if( nBottom < mrSheetLimits.mnMaxRow )
                    {
                        ScRange aAddRange(nCol, nBottom + 1, aMultiRange.aStart.Tab(),
                                          nCol, nBottom + 1, aMultiRange.aStart.Tab());
                        lcl_AddRanges( rRange, aAddRange ); // Bottom envelope
                        auto it = aRowToColSegmentsInBottomEnvelope.find(nBottom + 1);
                        if (it == aRowToColSegmentsInBottomEnvelope.end())
                            it = aRowToColSegmentsInBottomEnvelope.emplace(nBottom + 1, ScFlatBoolColSegments(mrSheetLimits.mnMaxCol)).first;
                        it->second.setTrue( nCol, nCol );
                    }
                }

                while( nTopPrev <= mrSheetLimits.mnMaxRow && nBottomPrev <= mrSheetLimits.mnMaxRow && ( nCol > nStartCol ) )
                {
                    bool bRangeMarked;
                    const bool bHasValue = aPrevItr1.getValue( nTopPrev, bRangeMarked );
                    assert(bHasValue); (void)bHasValue;
                    if( bRangeMarked )
                    {
                        nBottomPrev = aPrevItr1.getLastPos();
                        ScRange aAddRange(nCol, nTopPrev, aMultiRange.aStart.Tab(),
                                          nCol, nBottomPrev, aMultiRange.aStart.Tab());
                        lcl_AddRanges( rRange, aAddRange ); // Right envelope
                        aRightEnvelope.push_back( aAddRange );
                        nTopPrev = ++nBottomPrev;
                    }
                    else
                    {
                        nBottomPrev = aPrevItr1.getLastPos();
                        nTopPrev = ++nBottomPrev;
                    }
                }
            }
            else if( nCol > nStartCol )
            {
                bPrevColUnMarked = true;
                SCROW nTopPrev = 0, nBottomPrev = 0;
                bool bRangeMarked = false;
                ScFlatBoolRowSegments::ForwardIterator aPrevItr(
                    pPrevColMarkedRows ? *pPrevColMarkedRows : aNoRowsMarked);
                while( nTopPrev <= mrSheetLimits.mnMaxRow && nBottomPrev <= mrSheetLimits.mnMaxRow )
                {
                    const bool bHasValue = aPrevItr.getValue(nTopPrev, bRangeMarked);
                    assert(bHasValue); (void)bHasValue;
                    if( bRangeMarked )
                    {
                        nBottomPrev = aPrevItr.getLastPos();
                        ScRange aAddRange(nCol, nTopPrev, aMultiRange.aStart.Tab(),
                                          nCol, nBottomPrev, aMultiRange.aStart.Tab());
                        lcl_AddRanges( rRange, aAddRange ); // Right envelope
                        aRightEnvelope.push_back( aAddRange );
                        nTopPrev = ++nBottomPrev;
                    }
                    else
                    {
                        nBottomPrev = aPrevItr.getLastPos();
                        nTopPrev = ++nBottomPrev;
                    }
                }
            }
            if ( bCurColUnMarked )
                pPrevColMarkedRows.reset();
            else
                pPrevColMarkedRows = std::move( pCurColMarkedRows );
        }
        for( auto& rKV : aRowToColSegmentsInTopEnvelope )
        {
            SCCOL nStart = nStartCol;
            ScFlatBoolColSegments::RangeData aRange;
            while( nStart <= nEndCol )
            {
                if( !rKV.second.getRangeData( nStart, aRange ) )
                    break;
                if( aRange.mbValue ) // is marked
                    aTopEnvelope.push_back( ScRange( aRange.mnCol1, rKV.first, aMultiRange.aStart.Tab(),
                                                  aRange.mnCol2, rKV.first, aMultiRange.aStart.Tab() ) );
                nStart = aRange.mnCol2 + 1;
            }
        }
        for( auto& rKV : aRowToColSegmentsInBottomEnvelope )
        {
            SCCOL nStart = nStartCol;
            ScFlatBoolColSegments::RangeData aRange;
            while( nStart <= nEndCol )
            {
                if( !rKV.second.getRangeData( nStart, aRange ) )
                    break;
                if( aRange.mbValue ) // is marked
                    aBottomEnvelope.push_back( ScRange( aRange.mnCol1, rKV.first, aMultiRange.aStart.Tab(),
                                                     aRange.mnCol2, rKV.first, aMultiRange.aStart.Tab() ) );
                nStart = aRange.mnCol2 + 1;
            }
        }
    }
    else if( bMarked )
    {
        aMarkRange.PutInOrder();
        SCROW nRow1, nRow2, nRow1New, nRow2New;
        SCCOL nCol1, nCol2, nCol1New, nCol2New;
        SCTAB nTab1, nTab2;
        aMarkRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        nCol1New = nCol1;
        nCol2New = nCol2;
        nRow1New = nRow1;
        nRow2New = nRow2;
        // Each envelope will have zero or more ranges for single rectangle selection.
        if( nCol1 > 0 )
        {
            aLeftEnvelope.push_back( ScRange( nCol1 - 1, nRow1, nTab1, nCol1 - 1, nRow2, nTab2 ) );
            --nCol1New;
        }
        if( nRow1 > 0 )
        {
            aTopEnvelope.push_back( ScRange( nCol1, nRow1 - 1, nTab1, nCol2, nRow1 - 1, nTab2 ) );
            --nRow1New;
        }
        if( nCol2 < mrSheetLimits.mnMaxCol )
        {
            aRightEnvelope.push_back( ScRange( nCol2 + 1, nRow1, nTab1, nCol2 + 1, nRow2, nTab2 ) );
            ++nCol2New;
        }
        if( nRow2 < mrSheetLimits.mnMaxRow )
        {
            aBottomEnvelope.push_back( ScRange( nCol1, nRow2 + 1, nTab1, nCol2, nRow2 + 1, nTab2 ) );
            ++nRow2New;
        }
        rRange = ScRange( nCol1New, nRow1New, nTab1, nCol2New, nRow2New, nTab2 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
