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

void ColumnSpanSet::set(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bVal)
{
    if (!ValidTab(nTab) || !ValidCol(nCol) || !ValidRow(nRow))
        return;

    if (static_cast<size_t>(nTab) >= maDoc.size())
        maDoc.resize(nTab+1, NULL);

    if (!maDoc[nTab])
        maDoc[nTab] = new TableType;

    TableType& rTab = *maDoc[nTab];
    if (static_cast<size_t>(nCol) >= rTab.size())
        rTab.resize(nCol+1, NULL);

    if (!rTab[nCol])
        rTab[nCol] = new ColumnSpansType(0, MAXROW+1, false);

    ColumnSpansType& rCol = *rTab[nCol];
    rCol.insert_back(nRow, nRow+1, bVal);
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

            ColumnSpansType& rCol = *rTab[nCol];
            ColumnSpansType::const_iterator it = rCol.begin(), itEnd = rCol.end();
            SCROW nRow1, nRow2;
            nRow1 = it->first;
            for (++it; it != itEnd; ++it)
            {
                nRow2 = it->first-1;
                bool bVal = it->second;
                ac.execute(ScAddress(nCol, nRow1, nTab), nRow2-nRow1+1, bVal);

                nRow1 = nRow2+1; // for the next iteration.
            }
        }
    }
}

void ColumnSpanSet::executeFromBottom(Action& ac) const
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

            ColumnSpansType& rCol = *rTab[nCol];
            ColumnSpansType::const_reverse_iterator it = rCol.rbegin(), itEnd = rCol.rend();
            SCROW nRow1, nRow2;
            nRow2 = it->first-1;
            for (++it; it != itEnd; ++it)
            {
                nRow1 = it->first;
                bool bVal = it->second;
                ac.execute(ScAddress(nCol, nRow1, nTab), nRow2-nRow1+1, bVal);

                nRow2 = nRow1-1; // for the next iteration.
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
