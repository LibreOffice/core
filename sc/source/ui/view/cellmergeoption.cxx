/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 by Novell, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docfunc.hxx,v $
 * $Revision: 1.18.30.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
