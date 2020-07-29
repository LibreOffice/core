/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <dpoutputgeometry.hxx>
#include <address.hxx>

#include <vector>

using ::std::vector;

ScDPOutputGeometry::ScDPOutputGeometry(const ScRange& rOutRange, bool bShowFilter) :
    maOutRange(rOutRange),
    mnRowFields(0),
    mnColumnFields(0),
    mnPageFields(0),
    mnDataFields(0),
    meDataLayoutType(None),
    mbShowFilter(bShowFilter),
    mbHeaderLayout (false),
    mbCompactMode (false)
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

void ScDPOutputGeometry::setDataLayoutType(FieldType eType)
{
    meDataLayoutType = eType;
}

void ScDPOutputGeometry::setHeaderLayout(bool bHeaderLayout)
{
    mbHeaderLayout = bHeaderLayout;
}

void ScDPOutputGeometry::setCompactMode(bool bCompactMode)
{
    mbCompactMode = bCompactMode;
}

void ScDPOutputGeometry::getColumnFieldPositions(vector<ScAddress>& rAddrs) const
{
    sal_uInt32 nColumnFields, nRowFields;
    adjustFieldsForDataLayout(nColumnFields, nRowFields);

    vector<ScAddress> aAddrs;
    if (!nColumnFields)
    {
        rAddrs.swap(aAddrs);
        return;
    }

    SCROW nCurRow = maOutRange.aStart.Row();

    if (mnPageFields)
    {
        SCROW nRowStart = maOutRange.aStart.Row() + int(mbShowFilter);
        SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);
        nCurRow = nRowEnd + 2;
    }
    else if (mbShowFilter)
        nCurRow += 2;

    SCROW nRow = nCurRow;
    SCTAB nTab = maOutRange.aStart.Tab();
    SCCOL nColStart = static_cast<SCCOL>(maOutRange.aStart.Col() + nRowFields);
    if(mbCompactMode)
        nColStart = static_cast<SCCOL>(maOutRange.aStart.Col() + 1); // We have only one row in compact mode
    SCCOL nColEnd = nColStart + static_cast<SCCOL>(nColumnFields-1);

    for (SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        aAddrs.emplace_back(nCol, nRow, nTab);
    rAddrs.swap(aAddrs);
}

void ScDPOutputGeometry::getRowFieldPositions(vector<ScAddress>& rAddrs) const
{
    sal_uInt32 nColumnFields, nRowFields;
    adjustFieldsForDataLayout(nColumnFields, nRowFields);

    vector<ScAddress> aAddrs;
    if (!nRowFields)
    {
        rAddrs.swap(aAddrs);
        return;
    }

    SCROW nRow = getRowFieldHeaderRow();
    SCTAB nTab = maOutRange.aStart.Tab();
    SCCOL nColStart = maOutRange.aStart.Col();
    SCCOL nColEnd = nColStart + static_cast<SCCOL>(nRowFields-1);

    if(mbCompactMode)
        nColEnd = nColStart; // We have only one row in compact mode

    for (SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        aAddrs.emplace_back(nCol, nRow, nTab);
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

    SCROW nRowStart = maOutRange.aStart.Row() + int(mbShowFilter);
    SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);

    for (SCROW nRow = nRowStart; nRow <= nRowEnd; ++nRow)
        aAddrs.emplace_back(nCol, nRow, nTab);
    rAddrs.swap(aAddrs);
}

SCROW ScDPOutputGeometry::getRowFieldHeaderRow() const
{
    SCROW nCurRow = maOutRange.aStart.Row();
    sal_uInt32 nColumnFields, nRowFields;
    adjustFieldsForDataLayout(nColumnFields, nRowFields);

    if (mnPageFields)
    {
        SCROW nRowStart = maOutRange.aStart.Row() + int(mbShowFilter);
        SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);
        nCurRow = nRowEnd + 2;
    }
    else if (mbShowFilter)
        nCurRow += 2;

    if (nColumnFields)
        nCurRow += static_cast<SCROW>(nColumnFields);
    else if (nRowFields && mbHeaderLayout)
        ++nCurRow;

    return nCurRow;
}

void ScDPOutputGeometry::adjustFieldsForDataLayout(sal_uInt32& rColumnFields, sal_uInt32& rRowFields) const
{
    rRowFields = mnRowFields;
    rColumnFields = mnColumnFields;

    if (mnDataFields >= 2)
        return;

    // Data layout field can be either row or column field, never page field.
    switch (meDataLayoutType)
    {
        case Column:
            if (rColumnFields > 0)
                rColumnFields -= 1;
        break;
        case Row:
            if (rRowFields > 0)
                rRowFields -= 1;
        break;
        default:
            ;
    }
}

std::pair<ScDPOutputGeometry::FieldType, size_t>
ScDPOutputGeometry::getFieldButtonType(const ScAddress& rPos) const
{
    SCROW nCurRow = maOutRange.aStart.Row();
    sal_uInt32 nColumnFields, nRowFields;
    adjustFieldsForDataLayout(nColumnFields, nRowFields);

    if (mnPageFields)
    {
        SCCOL nCol = maOutRange.aStart.Col();
        SCROW nRowStart = maOutRange.aStart.Row() + int(mbShowFilter);
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

    if (nColumnFields)
    {
        SCROW nRow = nCurRow;
        SCCOL nColStart = static_cast<SCCOL>(maOutRange.aStart.Col() + nRowFields);
        SCCOL nColEnd = nColStart + static_cast<SCCOL>(nColumnFields-1);
        if (rPos.Row() == nRow && nColStart <= rPos.Col() && rPos.Col() <= nColEnd)
        {
            size_t nPos = static_cast<size_t>(rPos.Col() - nColStart);
            return std::pair<FieldType, size_t>(Column, nPos);
        }

        nCurRow += static_cast<SCROW>(nColumnFields);
    }
    else if (mbHeaderLayout)
        ++nCurRow;

    if (nRowFields)
    {
        SCCOL nColStart = maOutRange.aStart.Col();
        SCCOL nColEnd = nColStart + static_cast<SCCOL>(nRowFields-1);
        if (rPos.Row() == nCurRow && nColStart <= rPos.Col() && rPos.Col() <= nColEnd)
        {
            size_t nPos = static_cast<size_t>(rPos.Col() - nColStart);
            return std::pair<FieldType, size_t>(Row, nPos);
        }
    }

    return std::pair<FieldType, size_t>(None, 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
