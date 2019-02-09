/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <columnspanset.hxx>
#include <column.hxx>
#include <table.hxx>
#include <document.hxx>
#include <mtvfunctions.hxx>
#include <markdata.hxx>
#include <rangelst.hxx>
#include <fstalgorithm.hxx>

#include <algorithm>
#include <memory>

namespace sc {

namespace {

class ColumnScanner
{
    ColumnSpanSet::ColumnSpansType& mrRanges;
    bool const mbVal;
public:
    ColumnScanner(ColumnSpanSet::ColumnSpansType& rRanges, bool bVal) :
        mrRanges(rRanges), mbVal(bVal) {}

    void operator() (const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        if (node.type == sc::element_type_empty)
            return;

        size_t nRow = node.position + nOffset;
        size_t nEndRow = nRow + nDataSize; // Last row of current block plus 1
        mrRanges.insert_back(nRow, nEndRow, mbVal);
    }
};

}

RowSpan::RowSpan(SCROW nRow1, SCROW nRow2) : mnRow1(nRow1), mnRow2(nRow2) {}

ColRowSpan::ColRowSpan(SCCOLROW nStart, SCCOLROW nEnd) : mnStart(nStart), mnEnd(nEnd) {}

ColumnSpanSet::ColumnType::ColumnType(SCROW nStart, SCROW nEnd, bool bInit) :
    maSpans(nStart, nEnd+1, bInit), miPos(maSpans.begin()) {}

ColumnSpanSet::Action::~Action() {}
void ColumnSpanSet::Action::startColumn(SCTAB /*nTab*/, SCCOL /*nCol*/) {}

ColumnSpanSet::ColumnAction::~ColumnAction() {}

ColumnSpanSet::ColumnSpanSet(bool bInit) : mbInit(bInit) {}

ColumnSpanSet::~ColumnSpanSet()
{
}

ColumnSpanSet::ColumnType& ColumnSpanSet::getColumn(SCTAB nTab, SCCOL nCol)
{
    if (static_cast<size_t>(nTab) >= maTables.size())
        maTables.resize(nTab+1);

    if (!maTables[nTab])
        maTables[nTab].reset(new TableType);

    TableType& rTab = *maTables[nTab];
    if (static_cast<size_t>(nCol) >= rTab.size())
        rTab.resize(nCol+1);

    if (!rTab[nCol])
        rTab[nCol].reset(new ColumnType(0, MAXROW, mbInit));

    return *rTab[nCol];
}

void ColumnSpanSet::set(SCTAB nTab, SCCOL nCol, SCROW nRow, bool bVal)
{
    if (!ValidTab(nTab) || !ValidCol(nCol) || !ValidRow(nRow))
        return;

    ColumnType& rCol = getColumn(nTab, nCol);
    rCol.miPos = rCol.maSpans.insert(rCol.miPos, nRow, nRow+1, bVal).first;
}

void ColumnSpanSet::set(SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2, bool bVal)
{
    if (!ValidTab(nTab) || !ValidCol(nCol) || !ValidRow(nRow1) || !ValidRow(nRow2))
        return;

    ColumnType& rCol = getColumn(nTab, nCol);
    rCol.miPos = rCol.maSpans.insert(rCol.miPos, nRow1, nRow2+1, bVal).first;
}

void ColumnSpanSet::set(const ScRange& rRange, bool bVal)
{
    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
        {
            ColumnType& rCol = getColumn(nTab, nCol);
            rCol.miPos = rCol.maSpans.insert(rCol.miPos, rRange.aStart.Row(), rRange.aEnd.Row()+1, bVal).first;
        }
    }
}

void ColumnSpanSet::set( SCTAB nTab, SCCOL nCol, const SingleColumnSpanSet& rSingleSet, bool bVal )
{
    SingleColumnSpanSet::SpansType aSpans;
    rSingleSet.getSpans(aSpans);
    for (const auto& rSpan : aSpans)
        set(nTab, nCol, rSpan.mnRow1, rSpan.mnRow2, bVal);
}

void ColumnSpanSet::scan(
    const ScDocument& rDoc, SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bVal)
{
    if (!ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2))
        return;

    if (nCol1 > nCol2 || nRow1 > nRow2)
        return;

    const ScTable* pTab = rDoc.FetchTable(nTab);
    if (!pTab)
        return;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        ColumnType& rCol = getColumn(nTab, nCol);

        const CellStoreType& rSrcCells = pTab->aCol[nCol].maCells;

        ColumnScanner aScanner(rCol.maSpans, bVal);
        ParseBlock(rSrcCells.begin(), rSrcCells, aScanner, nRow1, nRow2);
    }
}

void ColumnSpanSet::executeAction(Action& ac) const
{
    for (size_t nTab = 0; nTab < maTables.size(); ++nTab)
    {
        if (!maTables[nTab])
            continue;

        const TableType& rTab = *maTables[nTab];
        for (size_t nCol = 0; nCol < rTab.size(); ++nCol)
        {
            if (!rTab[nCol])
                continue;

            ac.startColumn(nTab, nCol);
            ColumnType& rCol = *rTab[nCol];
            ColumnSpansType::const_iterator it = rCol.maSpans.begin(), itEnd = rCol.maSpans.end();
            SCROW nRow1, nRow2;
            nRow1 = it->first;
            bool bVal = it->second;
            for (++it; it != itEnd; ++it)
            {
                nRow2 = it->first-1;
                ac.execute(ScAddress(nCol, nRow1, nTab), nRow2-nRow1+1, bVal);

                nRow1 = nRow2+1; // for the next iteration.
                bVal = it->second;
            }
        }
    }
}

void ColumnSpanSet::executeColumnAction(ScDocument& rDoc, ColumnAction& ac) const
{
    for (size_t nTab = 0; nTab < maTables.size(); ++nTab)
    {
        if (!maTables[nTab])
            continue;

        const TableType& rTab = *maTables[nTab];
        for (size_t nCol = 0; nCol < rTab.size(); ++nCol)
        {
            if (!rTab[nCol])
                continue;

            ScTable* pTab = rDoc.FetchTable(nTab);
            if (!pTab)
                continue;

            if (!ValidCol(nCol))
            {
                // End the loop.
                nCol = rTab.size();
                continue;
            }

            ScColumn& rColumn = pTab->aCol[nCol];
            ac.startColumn(&rColumn);
            ColumnType& rCol = *rTab[nCol];
            ColumnSpansType::const_iterator it = rCol.maSpans.begin(), itEnd = rCol.maSpans.end();
            SCROW nRow1, nRow2;
            nRow1 = it->first;
            bool bVal = it->second;
            for (++it; it != itEnd; ++it)
            {
                nRow2 = it->first-1;
                ac.execute(nRow1, nRow2, bVal);

                nRow1 = nRow2+1; // for the next iteration.
                bVal = it->second;
            }
        }
    }
}

namespace {

class Scanner
{
    SingleColumnSpanSet::ColumnSpansType& mrRanges;
public:
    explicit Scanner(SingleColumnSpanSet::ColumnSpansType& rRanges) : mrRanges(rRanges) {}

    void operator() (const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        if (node.type == sc::element_type_empty)
            return;

        size_t nRow = node.position + nOffset;
        size_t nEndRow = nRow + nDataSize; // Last row of current block plus 1
        mrRanges.insert_back(nRow, nEndRow, true);
    }
};

}

SingleColumnSpanSet::SingleColumnSpanSet() : maSpans(0, MAXROWCOUNT, false) {}

void SingleColumnSpanSet::scan(const ScColumn& rColumn)
{
    const CellStoreType& rCells = rColumn.maCells;
    SCROW nCurRow = 0;
    for (const auto& rCell : rCells)
    {
        SCROW nEndRow = nCurRow + rCell.size; // Last row of current block plus 1.
        if (rCell.type != sc::element_type_empty)
            maSpans.insert_back(nCurRow, nEndRow, true);

        nCurRow = nEndRow;
    }
}

void SingleColumnSpanSet::scan(const ScColumn& rColumn, SCROW nStart, SCROW nEnd)
{
    const CellStoreType& rCells = rColumn.maCells;
    Scanner aScanner(maSpans);
    sc::ParseBlock(rCells.begin(), rCells, aScanner, nStart, nEnd);
}

void SingleColumnSpanSet::scan(
    ColumnBlockConstPosition& rBlockPos, const ScColumn& rColumn, SCROW nStart, SCROW nEnd)
{
    const CellStoreType& rCells = rColumn.maCells;
    Scanner aScanner(maSpans);
    rBlockPos.miCellPos = sc::ParseBlock(rBlockPos.miCellPos, rCells, aScanner, nStart, nEnd);
}

void SingleColumnSpanSet::scan(const ScMarkData& rMark, SCTAB nTab, SCCOL nCol)
{
    if (!rMark.GetTableSelect(nTab))
        // This table is not selected. Nothing to scan.
        return;

    ScRangeList aRanges = rMark.GetMarkedRangesForTab(nTab);
    scan(aRanges, nTab, nCol);
}

void SingleColumnSpanSet::scan(const ScRangeList& rRanges, SCTAB nTab, SCCOL nCol)
{
    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange & rRange = rRanges[i];
        if (nTab < rRange.aStart.Tab() || rRange.aEnd.Tab() < nTab)
            continue;

        if (nCol < rRange.aStart.Col() || rRange.aEnd.Col() < nCol)
            // This column is not in this range. Skip it.
            continue;

        maSpans.insert_back(rRange.aStart.Row(), rRange.aEnd.Row()+1, true);
    }
}

void SingleColumnSpanSet::set(SCROW nRow1, SCROW nRow2, bool bVal)
{
    maSpans.insert_back(nRow1, nRow2+1, bVal);
}

void SingleColumnSpanSet::getRows(std::vector<SCROW> &rRows) const
{
    std::vector<SCROW> aRows;

    SpansType aRanges;
    getSpans(aRanges);
    for (const auto& rRange : aRanges)
    {
        for (SCROW nRow = rRange.mnRow1; nRow <= rRange.mnRow2; ++nRow)
            aRows.push_back(nRow);
    }

    rRows.swap(aRows);
}

void SingleColumnSpanSet::getSpans(SpansType& rSpans) const
{
    SpansType aSpans = toSpanArray<SCROW,RowSpan>(maSpans);
    rSpans.swap(aSpans);
}

void SingleColumnSpanSet::swap( SingleColumnSpanSet& r )
{
    maSpans.swap(r.maSpans);
}

bool SingleColumnSpanSet::empty() const
{
    // Empty if there's only the 0..MAXROW span with false.
    ColumnSpansType::const_iterator it = maSpans.begin();
    return (it->first == 0) && !(it->second) && (++it != maSpans.end()) && (it->first == MAXROWCOUNT);
}


void RangeColumnSpanSet::executeColumnAction(ScDocument& rDoc, sc::ColumnSpanSet::ColumnAction& ac) const
{
    for (SCTAB nTab = range.aStart.Tab(); nTab <= range.aEnd.Tab(); ++nTab)
    {
        for (SCCOL nCol = range.aStart.Col(); nCol <= range.aEnd.Col(); ++nCol)
        {
            ScTable* pTab = rDoc.FetchTable(nTab);
            if (!pTab)
                continue;

            if (!ValidCol(nCol))
                break;

            ScColumn& rColumn = pTab->aCol[nCol];
            ac.startColumn(&rColumn);
            ac.execute( range.aStart.Row(), range.aEnd.Row(), true );
        }
    }
}

void RangeColumnSpanSet::executeColumnAction(ScDocument& rDoc, sc::ColumnSpanSet::ColumnAction& ac, double& fMem) const
{
    for (SCTAB nTab = range.aStart.Tab(); nTab <= range.aEnd.Tab(); ++nTab)
    {
        for (SCCOL nCol = range.aStart.Col(); nCol <= range.aEnd.Col(); ++nCol)
        {
            ScTable* pTab = rDoc.FetchTable(nTab);
            if (!pTab)
                continue;

            if (!ValidCol(nCol))
                break;

            ScColumn& rColumn = pTab->aCol[nCol];
            ac.startColumn(&rColumn);
            ac.executeSum( range.aStart.Row(), range.aEnd.Row(), true, fMem );
        }
    }
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
