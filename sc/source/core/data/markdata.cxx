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

#include "markdata.hxx"
#include "markarr.hxx"
#include "rangelst.hxx"
#include <columnspanset.hxx>
#include <fstalgorithm.hxx>

#include <osl/diagnose.h>

#include <mdds/flat_segment_tree.hpp>

// STATIC DATA -----------------------------------------------------------

ScMarkData::ScMarkData() :
    maTabMarked(),
    pMultiSel( nullptr )
{
    ResetMark();
}

ScMarkData::ScMarkData(const ScMarkData& rData) :
    maTabMarked( rData.maTabMarked ),
    aMarkRange( rData.aMarkRange ),
    aMultiRange( rData.aMultiRange ),
    pMultiSel( nullptr )
{
    bMarked      = rData.bMarked;
    bMultiMarked = rData.bMultiMarked;
    bMarking     = rData.bMarking;
    bMarkIsNeg   = rData.bMarkIsNeg;

    if (rData.pMultiSel)
    {
        pMultiSel = new ScMarkArray[MAXCOLCOUNT];
        for (SCCOL j=0; j<MAXCOLCOUNT; j++)
            rData.pMultiSel[j].CopyMarksTo( pMultiSel[j] );
    }
}

ScMarkData& ScMarkData::operator=(const ScMarkData& rData)
{
    if ( &rData == this )
        return *this;

    delete[] pMultiSel;
    pMultiSel = nullptr;

    aMarkRange   = rData.aMarkRange;
    aMultiRange  = rData.aMultiRange;
    bMarked      = rData.bMarked;
    bMultiMarked = rData.bMultiMarked;
    bMarking     = rData.bMarking;
    bMarkIsNeg   = rData.bMarkIsNeg;

    maTabMarked = rData.maTabMarked;

    if (rData.pMultiSel)
    {
        pMultiSel = new ScMarkArray[MAXCOLCOUNT];
        for (SCCOL j=0; j<MAXCOLCOUNT; j++)
            rData.pMultiSel[j].CopyMarksTo( pMultiSel[j] );
    }

    return *this;
}

ScMarkData::~ScMarkData()
{
    delete[] pMultiSel;
}

void ScMarkData::ResetMark()
{
    delete[] pMultiSel;
    pMultiSel = nullptr;

    bMarked = bMultiMarked = false;
    bMarking = bMarkIsNeg = false;
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

void ScMarkData::GetMarkArea( ScRange& rRange ) const
{
    rRange = aMarkRange;        //TODO: inline ?
}

void ScMarkData::GetMultiMarkArea( ScRange& rRange ) const
{
    rRange = aMultiRange;
}

void ScMarkData::SetMultiMarkArea( const ScRange& rRange, bool bMark )
{
    if (!pMultiSel)
    {
        pMultiSel = new ScMarkArray[MAXCOL+1];

        // if simple mark range is set, copy to multi marks
        if ( bMarked && !bMarkIsNeg )
        {
            bMarked = false;
            SetMultiMarkArea( aMarkRange );
        }
    }

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartCol, nEndCol );

    SCCOL nCol;
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        pMultiSel[nCol].SetMarkArea( nStartRow, nEndRow, bMark );

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
    if (maTabMarked.size() > 0)
        return (*maTabMarked.begin());

    OSL_FAIL("GetFirstSelected: nothing selected");
    return 0;
}

SCTAB ScMarkData::GetLastSelected() const
{
    if (maTabMarked.size() > 0)
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

    if ( bMultiMarked )
    {
        OSL_ENSURE(pMultiSel, "bMultiMarked, but pMultiSel == 0");

        ScRange aNew = aMultiRange;

        bool bOk = false;
        SCCOL nStartCol = aNew.aStart.Col();
        SCCOL nEndCol   = aNew.aEnd.Col();

        while ( nStartCol < nEndCol && !pMultiSel[nStartCol].HasMarks() )
            ++nStartCol;
        while ( nStartCol < nEndCol && !pMultiSel[nEndCol].HasMarks() )
            --nEndCol;

        // Rows are only taken from MarkArray
        SCROW nStartRow, nEndRow;
        if ( pMultiSel[nStartCol].HasOneMark( nStartRow, nEndRow ) )
        {
            bOk = true;
            SCROW nCmpStart, nCmpEnd;
            for (SCCOL nCol=nStartCol+1; nCol<=nEndCol && bOk; nCol++)
                if ( !pMultiSel[nCol].HasOneMark( nCmpStart, nCmpEnd )
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

        OSL_ENSURE(pMultiSel, "bMultiMarked, but pMultiSel == 0");
        return pMultiSel[nCol].GetMark( nRow );
    }

    return false;
}

bool ScMarkData::IsColumnMarked( SCCOL nCol ) const
{
    //  bMarkIsNeg meanwhile also for columns heads
    //TODO: GetMarkColumnRanges for completely marked column

    if ( bMarked && !bMarkIsNeg &&
                    aMarkRange.aStart.Col() <= nCol && aMarkRange.aEnd.Col() >= nCol &&
                    aMarkRange.aStart.Row() == 0    && aMarkRange.aEnd.Row() == MAXROW )
        return true;

    if ( bMultiMarked && pMultiSel[nCol].IsAllMarked(0,MAXROW) )
        return true;

    return false;
}

bool ScMarkData::IsRowMarked( SCROW nRow ) const
{
    //  bMarkIsNeg meanwhile also for row heads
    //TODO: GetMarkRowRanges for completely marked rows

    if ( bMarked && !bMarkIsNeg &&
                    aMarkRange.aStart.Col() == 0    && aMarkRange.aEnd.Col() == MAXCOL &&
                    aMarkRange.aStart.Row() <= nRow && aMarkRange.aEnd.Row() >= nRow )
        return true;

    if ( bMultiMarked )
    {
        OSL_ENSURE(pMultiSel, "bMultiMarked, but pMultiSel == 0");
        for (SCCOL nCol=0; nCol<=MAXCOL; nCol++)
            if (!pMultiSel[nCol].GetMark(nRow))
                return false;
        return true;
    }

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
        const ScRange& rRange = *rList[ 0 ];
        SetMarkArea( rRange );
        SelectTable( rRange.aStart.Tab(), true );
    }
    else
    {
        for (size_t i=0; i < nCount; i++)
        {
            const ScRange& rRange = *rList[ i ];
            SetMultiMarkArea( rRange );
            SelectTable( rRange.aStart.Tab(), true );
        }
    }
}

void ScMarkData::FillRangeListWithMarks( ScRangeList* pList, bool bClear ) const
{
    if (!pList)
        return;

    if (bClear)
        pList->RemoveAll();

    //TODO: for multiple selected tables enter multiple ranges !!!

    if ( bMultiMarked )
    {
        OSL_ENSURE(pMultiSel, "bMultiMarked, but pMultiSel == 0");

        SCTAB nTab = aMultiRange.aStart.Tab();

        SCCOL nStartCol = aMultiRange.aStart.Col();
        SCCOL nEndCol = aMultiRange.aEnd.Col();
        for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
        {
            if (pMultiSel[nCol].HasMarks())
            {
                // Feeding column-wise fragments to ScRangeList::Join() is a
                // huge bottleneck, speed this up for multiple columns
                // consisting of identical row sets by building a column span
                // first. This is usually the case for filtered data, for
                // example.
                SCCOL nToCol = nCol+1;
                for ( ; nToCol <= nEndCol; ++nToCol)
                {
                    if (!pMultiSel[nCol].HasEqualRowsMarked( pMultiSel[nToCol]))
                        break;
                }
                --nToCol;
                ScRange aRange( nCol, 0, nTab, nToCol, 0, nTab );
                SCROW nTop, nBottom;
                ScMarkArrayIter aMarkIter( &pMultiSel[nCol] );
                while ( aMarkIter.Next( nTop, nBottom ) )
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
        pList->Append( aMarkRange );
}

void ScMarkData::ExtendRangeListTables( ScRangeList* pList ) const
{
    if (!pList)
        return;

    ScRangeList aOldList(*pList);
    pList->RemoveAll();                 //TODO: or skip the existing below

    std::set<SCTAB>::const_iterator it = maTabMarked.begin();
    for (; it != maTabMarked.end(); ++it)
        for ( size_t i=0, nCount = aOldList.size(); i<nCount; i++)
        {
            ScRange aRange = *aOldList[ i ];
            aRange.aStart.SetTab(*it);
            aRange.aEnd.SetTab(*it);
            pList->Append( aRange );
        }
}

ScRangeList ScMarkData::GetMarkedRanges() const
{
    ScRangeList aRet;
    FillRangeListWithMarks(&aRet, false);
    return aRet;
}

std::vector<sc::ColRowSpan> ScMarkData::GetMarkedRowSpans() const
{
    typedef mdds::flat_segment_tree<SCCOLROW, bool> SpansType;

    ScRangeList aRanges = GetMarkedRanges();
    SpansType aSpans(0, MAXROW+1, false);
    SpansType::const_iterator itPos = aSpans.begin();

    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
    {
        const ScRange& r = *aRanges[i];
        itPos = aSpans.insert(itPos, r.aStart.Row(), r.aEnd.Row()+1, true).first;
    }

    return sc::toSpanArray<SCCOLROW,sc::ColRowSpan>(aSpans);
}

std::vector<sc::ColRowSpan> ScMarkData::GetMarkedColSpans() const
{
    typedef mdds::flat_segment_tree<SCCOLROW, bool> SpansType;

    ScRangeList aRanges = GetMarkedRanges();
    SpansType aSpans(0, MAXCOL+1, false);
    SpansType::const_iterator itPos = aSpans.begin();

    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
    {
        const ScRange& r = *aRanges[i];
        itPos = aSpans.insert(itPos, r.aStart.Col(), r.aEnd.Col()+1, true).first;
    }

    return sc::toSpanArray<SCCOLROW,sc::ColRowSpan>(aSpans);
}

bool ScMarkData::IsAllMarked( const ScRange& rRange ) const
{
    if ( !bMultiMarked )
        return false;

    OSL_ENSURE(pMultiSel, "bMultiMarked, but pMultiSel == 0");

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    bool bOk = true;
    for (SCCOL nCol=nStartCol; nCol<=nEndCol && bOk; nCol++)
        if ( !pMultiSel[nCol].IsAllMarked( nStartRow, nEndRow ) )
            bOk = false;

    return bOk;
}

SCsROW ScMarkData::GetNextMarked( SCCOL nCol, SCsROW nRow, bool bUp ) const
{
    if ( !bMultiMarked )
        return nRow;

    OSL_ENSURE(pMultiSel, "bMultiMarked, but pMultiSel == 0");

    return pMultiSel[nCol].GetNextMarked( nRow, bUp );
}

bool ScMarkData::HasMultiMarks( SCCOL nCol ) const
{
    if ( !bMultiMarked )
        return false;

    OSL_ENSURE(pMultiSel, "bMultiMarked, but pMultiSel == 0");

    return pMultiSel[nCol].HasMarks();
}

bool ScMarkData::HasAnyMultiMarks() const
{
    if ( !bMultiMarked )
        return false;

    OSL_ENSURE(pMultiSel, "bMultiMarked, but pMultiSel == 0");

    for (SCCOL nCol=0; nCol<=MAXCOL; nCol++)
        if ( pMultiSel[nCol].HasMarks() )
            return true;

    return false;       // no
}

void ScMarkData::InsertTab( SCTAB nTab )
{
    std::set<SCTAB> tabMarked(maTabMarked.begin(), maTabMarked.upper_bound(nTab));
    std::set<SCTAB>::iterator it = maTabMarked.upper_bound(nTab);
    for (; it != maTabMarked.end(); ++it)
        tabMarked.insert(*it + 1);
    maTabMarked.swap(tabMarked);
}

void ScMarkData::DeleteTab( SCTAB nTab )
{
    std::set<SCTAB> tabMarked(maTabMarked.begin(), maTabMarked.find(nTab));
    tabMarked.erase( nTab );
    std::set<SCTAB>::iterator it = maTabMarked.find(nTab);
    for (; it != maTabMarked.end(); ++it)
        tabMarked.insert(*it + 1);
    maTabMarked.swap(tabMarked);
}

//iterators
ScMarkData::iterator ScMarkData::begin()
{
    return maTabMarked.begin();
}

ScMarkData::iterator ScMarkData::end()
{
    return maTabMarked.end();
}

ScMarkData::const_iterator ScMarkData::begin() const
{
    return maTabMarked.begin();
}

ScMarkData::const_iterator ScMarkData::end() const
{
    return maTabMarked.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
