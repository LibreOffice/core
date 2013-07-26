/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "cellmergeoption.hxx"
#include "address.hxx"

ScCellMergeOption::ScCellMergeOption(const ScRange& rRange) :
    mnStartCol(rRange.aStart.Col()),
    mnStartRow(rRange.aStart.Row()),
    mnEndCol(rRange.aEnd.Col()),
    mnEndRow(rRange.aEnd.Row()),
    mbCenter(false)
{
    SCTAB nTab1 = rRange.aStart.Tab();
    SCTAB nTab2 = rRange.aEnd.Tab();
    for (SCTAB i = nTab1; i <= nTab2; ++i)
        maTabs.insert(i);
}

ScCellMergeOption::ScCellMergeOption(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, bool bCenter) :
    mnStartCol(nStartCol),
    mnStartRow(nStartRow),
    mnEndCol(nEndCol),
    mnEndRow(nEndRow),
    mbCenter(bCenter)
{
}

ScCellMergeOption::ScCellMergeOption(const ScCellMergeOption& r) :
    maTabs(r.maTabs),
    mnStartCol(r.mnStartCol),
    mnStartRow(r.mnStartRow),
    mnEndCol(r.mnEndCol),
    mnEndRow(r.mnEndRow),
    mbCenter(r.mbCenter)
{
}

ScRange ScCellMergeOption::getSingleRange(SCTAB nTab) const
{
    return ScRange(mnStartCol, mnStartRow, nTab, mnEndCol, mnEndRow, nTab);
}

ScRange ScCellMergeOption::getFirstSingleRange() const
{
    SCTAB nTab = 0;
    if (!maTabs.empty())
        nTab = *maTabs.begin();

    return getSingleRange(nTab);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
