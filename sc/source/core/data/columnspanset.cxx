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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
