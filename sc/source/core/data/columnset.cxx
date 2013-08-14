/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "columnset.hxx"

namespace sc {

void ColumnSet::set(SCTAB nTab, SCCOL nCol)
{
    TabsType::iterator itTab = maTabs.find(nTab);
    if (itTab == maTabs.end())
    {
        std::pair<TabsType::iterator,bool> r =
            maTabs.insert(TabsType::value_type(nTab, ColsType()));

        if (!r.second)
            // insertion failed.
            return;

        itTab = r.first;
    }

    ColsType& rCols = itTab->second;
    rCols.insert(nCol);
}

bool ColumnSet::has(SCTAB nTab, SCCOL nCol) const
{
    TabsType::const_iterator itTab = maTabs.find(nTab);
    if (itTab == maTabs.end())
        return false;

    const ColsType& rCols = itTab->second;
    return rCols.count(nCol) > 0;
}

void ColumnSet::getColumns(SCTAB nTab, std::vector<SCCOL>& rCols) const
{
    std::vector<SCCOL> aCols;
    TabsType::const_iterator itTab = maTabs.find(nTab);
    if (itTab == maTabs.end())
    {
        rCols.swap(aCols); // empty it.
        return;
    }

    const ColsType& rTabCols = itTab->second;
    aCols.assign(rTabCols.begin(), rTabCols.end());

    // Sort and remove duplicates.
    std::sort(aCols.begin(), aCols.end());
    std::vector<SCCOL>::iterator itCol = std::unique(aCols.begin(), aCols.end());
    aCols.erase(itCol, aCols.end());

    rCols.swap(aCols);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
