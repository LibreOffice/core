/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Kohei Yoshida <kyoshida@novell.com> ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "cellmergeoption.hxx"
#include "address.hxx"

ScCellMergeOption::ScCellMergeOption() :
    mnStartCol(0),
    mnStartRow(0),
    mnEndCol(0),
    mnEndRow(0),
    mbCenter(false)
{
}

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
