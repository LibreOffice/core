/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "columnspanset.hxx"
#include "column.hxx"
#include "table.hxx"
#include "document.hxx"
#include "mtvfunctions.hxx"
#include "markdata.hxx"
#include "rangelst.hxx"
#include <fstalgorithm.hxx>
#include <boost/checked_delete.hpp>

#include <algorithm>

namespace sc {

namespace {

class ColumnScanner
{
    ColumnSpanSet::ColumnSpansType& mrRanges;
    bool mbVal;
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
    DocType::iterator itTab = maDoc.begin(), itTabEnd = maDoc.end();
    for (; itTab != itTabEnd; ++itTab)
    {
        TableType* pTab = *itTab;
        if (!pTab)
            continue;

        std::for_each(pTab->begin(), pTab->end(), boost::checked_deleter<ColumnType>());
        delete pTab;
    }
}

ColumnSpanSet::ColumnType& ColumnSpanSet::getColumn(SCTAB nTab, SCCOL nCol)
{
    if (static_cast<size_t>(nTab) >= maDoc.size())
        maDoc.resize(nTab+1, nullptr);

    if (!maDoc[nTab])
        maDoc[nTab] = new TableType;

    TableType& rTab = *maDoc[nTab];
    if (static_cast<size_t>(nCol) >= rTab.size())
        rTab.resize(nCol+1, nullptr);

    if (!rTab[nCol])
        rTab[nCol] = new ColumnType(0, MAXROW, mbInit);

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
    SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
    for (; it != itEnd; ++it)
        set(nTab, nCol, it->mnRow1, it->mnRow2, bVal);
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
    for (size_t nTab = 0; nTab < maDoc.size(); ++nTab)
    {
        if (!maDoc[nTab])
            continue;

        const TableType& rTab = *maDoc[nTab];
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
    for (size_t nTab = 0; nTab < maDoc.size(); ++nTab)
    {
        if (!maDoc[nTab])
            continue;

        const TableType& rTab = *maDoc[nTab];
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

void ColumnSpanSet::executeColumnAction(ScDocument& rDoc, ColumnAction& ac, double& fMem) const
{
    for (size_t nTab = 0; nTab < maDoc.size(); ++nTab)
    {
        if (!maDoc[nTab])
            continue;

        const TableType& rTab = *maDoc[nTab];
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
                ac.executeSum( nRow1, nRow2, bVal, fMem );

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
    sc::CellStoreType::const_iterator it = rCells.begin(), itEnd = rCells.end();
    SCROW nCurRow = 0;
    for (;it != itEnd; ++it)
    {
        SCROW nEndRow = nCurRow + it->size; // Last row of current block plus 1.
        if (it->type != sc::element_type_empty)
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

    ScRangeList aRanges = rMark.GetMarkedRanges();
    scan(aRanges, nTab, nCol);
}

void SingleColumnSpanSet::scan(const ScRangeList& rRanges, SCTAB nTab, SCCOL nCol)
{
    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange* p = rRanges[i];
        if (nTab < p->aStart.Tab() || p->aEnd.Tab() < nTab)
            continue;

        if (nCol < p->aStart.Col() || p->aEnd.Col() < nCol)
            // This column is not in this range. Skip it.
            continue;

        maSpans.insert_back(p->aStart.Row(), p->aEnd.Row()+1, true);
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
    SpansType::const_iterator it = aRanges.begin(), itEnd = aRanges.end();
    for (; it != itEnd; ++it)
    {
        for (SCROW nRow = it->mnRow1; nRow <= it->mnRow2; ++nRow)
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
