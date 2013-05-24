/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "columnspanset.hxx"
#include "stlalgorithm.hxx"
#include "column.hxx"
#include "mtvfunctions.hxx"
#include "markdata.hxx"
#include "rangelst.hxx"

#include <algorithm>

namespace sc {

ColumnSpanSet::Action::~Action() {}
void ColumnSpanSet::Action::startColumn(SCTAB /*nTab*/, SCCOL /*nCol*/) {}

ColumnSpanSet::~ColumnSpanSet()
{
    DocType::iterator itTab = maDoc.begin(), itTabEnd = maDoc.end();
    for (; itTab != itTabEnd; ++itTab)
    {
        TableType* pTab = *itTab;
        if (!pTab)
            continue;

        std::for_each(pTab->begin(), pTab->end(), ScDeleteObjectByPtr<ColumnSpansType>());
        delete pTab;
    }
}

ColumnSpanSet::ColumnSpansType& ColumnSpanSet::getColumnSpans(SCTAB nTab, SCCOL nCol)
{
    if (static_cast<size_t>(nTab) >= maDoc.size())
        maDoc.resize(nTab+1, NULL);

    if (!maDoc[nTab])
        maDoc[nTab] = new TableType;

    TableType& rTab = *maDoc[nTab];
    if (static_cast<size_t>(nCol) >= rTab.size())
        rTab.resize(nCol+1, NULL);

    if (!rTab[nCol])
        rTab[nCol] = new ColumnSpansType(0, MAXROW+1, false);

    return *rTab[nCol];
}

void ColumnSpanSet::set(SCTAB nTab, SCCOL nCol, SCROW nRow, bool bVal)
{
    if (!ValidTab(nTab) || !ValidCol(nCol) || !ValidRow(nRow))
        return;

    ColumnSpansType& rCol = getColumnSpans(nTab, nCol);
    rCol.insert_back(nRow, nRow+1, bVal);
}

void ColumnSpanSet::set(SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2, bool bVal)
{
    if (!ValidTab(nTab) || !ValidCol(nCol) || !ValidRow(nRow1) || !ValidRow(nRow2))
        return;

    ColumnSpansType& rCol = getColumnSpans(nTab, nCol);
    rCol.insert_back(nRow1, nRow2+1, bVal);
}

void ColumnSpanSet::executeFromTop(Action& ac) const
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
            ColumnSpansType& rCol = *rTab[nCol];
            ColumnSpansType::const_iterator it = rCol.begin(), itEnd = rCol.end();
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

namespace {

class Scanner
{
    SingleColumnSpanSet::ColumnSpansType& mrRanges;
public:
    Scanner(SingleColumnSpanSet::ColumnSpansType& rRanges) : mrRanges(rRanges) {}

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
    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
    {
        const ScRange* p = aRanges[i];
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
    SpansType aSpans;
    ColumnSpansType::const_iterator it = maSpans.begin(), itEnd = maSpans.end();
    SCROW nLastRow = it->first;
    bool bLastVal = it->second;
    for (++it; it != itEnd; ++it)
    {
        SCROW nThisRow = it->first;
        bool bThisVal = it->second;

        if (bLastVal)
            aSpans.push_back(Span(nLastRow, nThisRow-1));

        nLastRow = nThisRow;
        bLastVal = bThisVal;
    }

    rSpans.swap(aSpans);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
