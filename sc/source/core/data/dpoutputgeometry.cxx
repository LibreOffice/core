/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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




// INCLUDE ---------------------------------------------------------------

#include "dpoutputgeometry.hxx"
#include "address.hxx"

#include <vector>

using ::std::vector;

ScDPOutputGeometry::ScDPOutputGeometry(const ScRange& rOutRange, bool bShowFilter) :
    maOutRange(rOutRange),
    mnRowFields(0),
    mnColumnFields(0),
    mnPageFields(0),
    mnDataFields(0),
    mbShowFilter(bShowFilter)
{
}

ScDPOutputGeometry::~ScDPOutputGeometry()
{
}

void ScDPOutputGeometry::setRowFieldCount(sal_uInt32 nCount)
{
    mnRowFields = nCount;
}

void ScDPOutputGeometry::setColumnFieldCount(sal_uInt32 nCount)
{
    mnColumnFields = nCount;
}

void ScDPOutputGeometry::setPageFieldCount(sal_uInt32 nCount)
{
    mnPageFields = nCount;
}

void ScDPOutputGeometry::setDataFieldCount(sal_uInt32 nCount)
{
    mnDataFields = nCount;
}

void ScDPOutputGeometry::getColumnFieldPositions(vector<ScAddress>& rAddrs) const
{
    vector<ScAddress> aAddrs;
    if (!mnColumnFields)
    {
        rAddrs.swap(aAddrs);
        return;
    }

    SCROW nCurRow = maOutRange.aStart.Row();

    if (mnPageFields)
    {
        SCROW nRowStart = maOutRange.aStart.Row() + mbShowFilter;
        SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);
        nCurRow = nRowEnd + 2;
    }
    else if (mbShowFilter)
        nCurRow += 2;

    SCROW nRow = nCurRow;
    SCTAB nTab = maOutRange.aStart.Tab();
    SCCOL nColStart = static_cast<SCCOL>(maOutRange.aStart.Col() + mnRowFields);
    SCCOL nColEnd = nColStart + static_cast<SCCOL>(mnColumnFields-1);

    for (SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        aAddrs.push_back(ScAddress(nCol, nRow, nTab));
    rAddrs.swap(aAddrs);
}

void ScDPOutputGeometry::getRowFieldPositions(vector<ScAddress>& rAddrs) const
{
    vector<ScAddress> aAddrs;
    if (!mnRowFields)
    {
        rAddrs.swap(aAddrs);
        return;
    }

    SCROW nRow = getRowFieldHeaderRow();
    SCTAB nTab = maOutRange.aStart.Tab();
    SCCOL nColStart = maOutRange.aStart.Col();
    SCCOL nColEnd = nColStart + static_cast<SCCOL>(mnRowFields-1);

    for (SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        aAddrs.push_back(ScAddress(nCol, nRow, nTab));
    rAddrs.swap(aAddrs);
}

void ScDPOutputGeometry::getPageFieldPositions(vector<ScAddress>& rAddrs) const
{
    vector<ScAddress> aAddrs;
    if (!mnPageFields)
    {
        rAddrs.swap(aAddrs);
        return;
    }

    SCTAB nTab = maOutRange.aStart.Tab();
    SCCOL nCol = maOutRange.aStart.Col();

    SCROW nRowStart = maOutRange.aStart.Row() + mbShowFilter;
    SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);

    for (SCROW nRow = nRowStart; nRow <= nRowEnd; ++nRow)
        aAddrs.push_back(ScAddress(nCol, nRow, nTab));
    rAddrs.swap(aAddrs);
}

SCROW ScDPOutputGeometry::getRowFieldHeaderRow() const
{
    SCROW nCurRow = maOutRange.aStart.Row();

    if (mnPageFields)
    {
        SCROW nRowStart = maOutRange.aStart.Row() + mbShowFilter;
        SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);
        nCurRow = nRowEnd + 2;
    }
    else if (mbShowFilter)
        nCurRow += 2;

    if (mnColumnFields)
        nCurRow += static_cast<SCROW>(mnColumnFields);
    else if (mnRowFields)
        ++nCurRow;

    return nCurRow;
}

std::pair<ScDPOutputGeometry::FieldType, size_t>
ScDPOutputGeometry::getFieldButtonType(const ScAddress& rPos) const
{
    // We will ignore the table position for now.

    SCROW nCurRow = maOutRange.aStart.Row();

    if (mnPageFields)
    {
        SCCOL nCol = maOutRange.aStart.Col();
        SCROW nRowStart = maOutRange.aStart.Row() + mbShowFilter;
        SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);
        if (rPos.Col() == nCol && nRowStart <= rPos.Row() && rPos.Row() <= nRowEnd)
        {
            size_t nPos = static_cast<size_t>(rPos.Row() - nRowStart);
            return std::pair<FieldType, size_t>(Page, nPos);
        }

        nCurRow = nRowEnd + 2;
    }
    else if (mbShowFilter)
        nCurRow += 2;

    if (mnColumnFields)
    {
        SCROW nRow = nCurRow;
        SCCOL nColStart = static_cast<SCCOL>(maOutRange.aStart.Col() + mnRowFields);
        SCCOL nColEnd = nColStart + static_cast<SCCOL>(mnColumnFields-1);
        if (rPos.Row() == nRow && nColStart <= rPos.Col() && rPos.Col() <= nColEnd)
        {
            size_t nPos = static_cast<size_t>(rPos.Col() - nColStart);
            return std::pair<FieldType, size_t>(Column, nPos);
        }

        nCurRow += static_cast<SCROW>(mnColumnFields);
    }
    else
        ++nCurRow;

    if (mnRowFields)
    {
        SCCOL nColStart = maOutRange.aStart.Col();
        SCCOL nColEnd = nColStart + static_cast<SCCOL>(mnRowFields-1);
        if (rPos.Row() == nCurRow && nColStart <= rPos.Col() && rPos.Col() <= nColEnd)
        {
            size_t nPos = static_cast<size_t>(rPos.Col() - nColStart);
            return std::pair<FieldType, size_t>(Row, nPos);
        }
    }

    return std::pair<FieldType, size_t>(None, 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
