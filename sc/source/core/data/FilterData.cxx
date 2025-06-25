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

#include <sal/types.h>

#include <FilterData.hxx>
#include <document.hxx>
#include <table.hxx>
#include <segmenttree.hxx>

#include <vector>
#include <memory>

FilterData::FilterData(ScTable& rTable)
    : mrTable(rTable)
    , mpFilteredCols(new ScFlatBoolColSegments(getDocument().MaxCol()))
    , mpFilteredRows(new ScFlatBoolRowSegments(getDocument().MaxRow()))
    , mpHiddenCols(new ScFlatBoolColSegments(getDocument().MaxCol()))
    , mpHiddenRows(new ScFlatBoolRowSegments(getDocument().MaxRow()))
{
}

ScDocument& FilterData::getDocument() { return mrTable.GetDoc(); }
ScDocument const& FilterData::getDocument() const { return mrTable.GetDoc(); }

void FilterData::makeReady()
{
    mpFilteredCols->makeReady();
    mpFilteredRows->makeReady();
    mpHiddenCols->makeReady();
    mpHiddenRows->makeReady();
}

OString FilterData::dumpColsAsString(OString const& rDefault)
{
    return mpFilteredCols ? mpFilteredCols->dumpAsString() : rDefault;
}

OString FilterData::dumpRowsAsString(OString const& rDefault)
{
    return mpFilteredRows ? mpFilteredRows->dumpAsString() : rDefault;
}

OString FilterData::dumpHiddenColsAsString(OString const& rDefault)
{
    return mpHiddenCols ? mpHiddenCols->dumpAsString() : rDefault;
}

OString FilterData::dumpHiddenRowsAsString(OString const& rDefault)
{
    return mpHiddenRows ? mpHiddenRows->dumpAsString() : rDefault;
}

bool FilterData::setColFiltered(SCCOL nStartCol, SCCOL nEndCol, bool bFiltered)
{
    if (bFiltered)
        return mpFilteredCols->setTrue(nStartCol, nEndCol);
    else
        return mpFilteredCols->setFalse(nStartCol, nEndCol);
}

bool FilterData::setRowFiltered(SCROW nStartRow, SCROW nEndRow, bool bFiltered)
{
    if (bFiltered)
        return mpFilteredRows->setTrue(nStartRow, nEndRow);
    else
        return mpFilteredRows->setFalse(nStartRow, nEndRow);
}

bool FilterData::rowFiltered(SCROW nRow, SCROW* pFirstRow, SCROW* pLastRow) const
{
    if (!getDocument().ValidRow(nRow))
        return false;

    ScFlatBoolRowSegments::RangeData aData;
    if (!mpFilteredRows->getRangeData(nRow, aData))
        // search failed.
        return false;

    if (pFirstRow)
        *pFirstRow = aData.mnRow1;
    if (pLastRow)
        *pLastRow = aData.mnRow2;

    return aData.mbValue;
}

bool FilterData::columnFiltered(SCCOL nCol, SCCOL* pFirstCol, SCCOL* pLastCol) const
{
    if (!getDocument().ValidCol(nCol))
        return false;

    ScFlatBoolColSegments::RangeData aData;
    if (!mpFilteredCols->getRangeData(nCol, aData))
        // search failed.
        return false;

    if (pFirstCol)
        *pFirstCol = aData.mnCol1;
    if (pLastCol)
        *pLastCol = aData.mnCol2;

    return aData.mbValue;
}

bool FilterData::hasFilteredRows(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nLastRow = nRow;
        bool bFiltered = rowFiltered(nRow, nullptr, &nLastRow);
        if (bFiltered)
            return true;

        nRow = nLastRow + 1;
    }
    return false;
}

void FilterData::copyColFiltered(FilterData const& rFilter, SCCOL nStartCol, SCCOL nEndCol)
{
    SCCOL nCol = nStartCol;
    while (nCol <= nEndCol)
    {
        SCCOL nLastCol = -1;
        bool bFiltered = rFilter.columnFiltered(nCol, nullptr, &nLastCol);
        if (nLastCol > nEndCol)
            nLastCol = nEndCol;

        setColFiltered(nCol, nLastCol, bFiltered);
        nCol = nLastCol + 1;
    }
}

void FilterData::copyRowFiltered(FilterData const& rFilter, SCROW nStartRow, SCROW nEndRow)
{
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nLastRow = -1;
        bool bFiltered = rFilter.rowFiltered(nRow, nullptr, &nLastRow);
        if (nLastRow > nEndRow)
            nLastRow = nEndRow;
        setRowFiltered(nRow, nLastRow, bFiltered);
        nRow = nLastRow + 1;
    }
}

SCROW FilterData::firstNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!getDocument().ValidRow(nRow))
            break;

        if (!mpFilteredRows->getRangeData(nRow, aData))
            // failed to get range data.
            break;

        if (!aData.mbValue)
            // non-filtered row found
            return nRow;

        nRow = aData.mnRow2 + 1;
    }

    return ::std::numeric_limits<SCROW>::max();
}

SCROW FilterData::lastNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nEndRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow >= nStartRow)
    {
        if (!getDocument().ValidRow(nRow))
            break;

        if (!mpFilteredRows->getRangeData(nRow, aData))
            // failed to get range data.
            break;

        if (!aData.mbValue)
            // non-filtered row found
            return nRow;

        nRow = aData.mnRow1 - 1;
    }

    return ::std::numeric_limits<SCROW>::max();
}

SCROW FilterData::countNonFilteredRows(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nCount = 0;
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!mpFilteredRows->getRangeData(nRow, aData))
            break;

        if (aData.mnRow2 > nEndRow)
            aData.mnRow2 = nEndRow;

        if (!aData.mbValue)
            nCount += aData.mnRow2 - nRow + 1;

        nRow = aData.mnRow2 + 1;
    }
    return nCount;
}

bool FilterData::rowHidden(SCROW nRow, SCROW* pFirstRow, SCROW* pLastRow) const
{
    if (!getDocument().ValidRow(nRow))
    {
        if (pFirstRow)
            *pFirstRow = nRow;
        if (pLastRow)
            *pLastRow = nRow;
        return true;
    }

    ScFlatBoolRowSegments::RangeData aData;
    if (!mpHiddenRows->getRangeData(nRow, aData))
    {
        // search failed.
        if (pFirstRow)
            *pFirstRow = nRow;
        if (pLastRow)
            *pLastRow = nRow;
        return true;
    }

    if (pFirstRow)
        *pFirstRow = aData.mnRow1;
    if (pLastRow)
        *pLastRow = aData.mnRow2;

    return aData.mbValue;
}

bool FilterData::rowHiddenLeaf(SCROW nRow, SCROW* pFirstRow, SCROW* pLastRow) const
{
    if (!getDocument().ValidRow(nRow))
    {
        if (pFirstRow)
            *pFirstRow = nRow;
        if (pLastRow)
            *pLastRow = nRow;
        return true;
    }

    ScFlatBoolRowSegments::RangeData aData;
    if (!mpHiddenRows->getRangeDataLeaf(nRow, aData))
    {
        // search failed.
        if (pFirstRow)
            *pFirstRow = nRow;
        if (pLastRow)
            *pLastRow = nRow;
        return true;
    }

    if (pFirstRow)
        *pFirstRow = aData.mnRow1;
    if (pLastRow)
        *pLastRow = aData.mnRow2;

    return aData.mbValue;
}

bool FilterData::hasHiddenRows(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nLastRow = -1;
        bool bHidden = rowHidden(nRow, nullptr, &nLastRow);
        if (bHidden)
            return true;

        nRow = nLastRow + 1;
    }
    return false;
}

bool FilterData::colHidden(SCCOL nCol, SCCOL* pFirstCol, SCCOL* pLastCol) const
{
    if (!getDocument().ValidCol(nCol))
        return true;

    ScFlatBoolColSegments::RangeData aData;
    if (!mpHiddenCols->getRangeData(nCol, aData))
        return true;

    if (pFirstCol)
        *pFirstCol = aData.mnCol1;
    if (pLastCol)
        *pLastCol = aData.mnCol2;

    return aData.mbValue;
}

bool FilterData::setRowHidden(SCROW nStartRow, SCROW nEndRow, bool bHidden)
{
    bool bChanged = false;
    if (bHidden)
        bChanged = mpHiddenRows->setTrue(nStartRow, nEndRow);
    else
        bChanged = mpHiddenRows->setFalse(nStartRow, nEndRow);

    mrTable.updateObjectsForRowsChanged(nStartRow, nEndRow, bHidden, bChanged);

    return bChanged;
}

bool FilterData::setColHidden(SCROW nStartCol, SCROW nEndCol, bool bHidden)
{
    bool bChanged = false;
    if (bHidden)
        bChanged = mpHiddenCols->setTrue(nStartCol, nEndCol);
    else
        bChanged = mpHiddenCols->setFalse(nStartCol, nEndCol);

    mrTable.updateObjectsForColsChanged(nStartCol, nEndCol, bHidden, bChanged);

    return bChanged;
}

void FilterData::copyColHidden(FilterData const& rFilter, SCCOL nStartCol, SCCOL nEndCol)
{
    SCCOL nCol = nStartCol;
    while (nCol <= nEndCol)
    {
        SCCOL nLastCol = -1;
        bool bHidden = rFilter.colHidden(nCol, nullptr, &nLastCol);
        if (nLastCol > nEndCol)
            nLastCol = nEndCol;

        mrTable.SetColHidden(nCol, nLastCol, bHidden);
        nCol = nLastCol + 1;
    }
}

void FilterData::copyRowHidden(FilterData const& rFilter, SCROW nStartRow, SCROW nEndRow)
{
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nLastRow = -1;
        bool bHidden = rFilter.rowHidden(nRow, nullptr, &nLastRow);
        if (nLastRow > nEndRow)
            nLastRow = nEndRow;
        mrTable.SetRowHidden(nRow, nLastRow, bHidden);
        nRow = nLastRow + 1;
    }
}

SCROW FilterData::firstVisibleRow(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!getDocument().ValidRow(nRow))
            break;

        if (!mpHiddenRows->getRangeData(nRow, aData))
            // failed to get range data.
            break;

        if (!aData.mbValue)
            // visible row found
            return nRow;

        nRow = aData.mnRow2 + 1;
    }

    return ::std::numeric_limits<SCROW>::max();
}

SCROW FilterData::lastVisibleRow(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nEndRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow >= nStartRow)
    {
        if (!getDocument().ValidRow(nRow))
            break;

        if (!mpHiddenRows->getRangeData(nRow, aData))
            // failed to get range data.
            break;

        if (!aData.mbValue)
            // visible row found
            return nRow;

        nRow = aData.mnRow1 - 1;
    }

    return ::std::numeric_limits<SCROW>::max();
}

SCROW FilterData::countVisibleRows(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nCount = 0;
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!mpHiddenRows->getRangeData(nRow, aData))
            break;

        if (aData.mnRow2 > nEndRow)
            aData.mnRow2 = nEndRow;

        if (!aData.mbValue)
            nCount += aData.mnRow2 - nRow + 1;

        nRow = aData.mnRow2 + 1;
    }
    return nCount;
}

SCCOL FilterData::countVisibleCols(SCCOL nStartCol, SCCOL nEndCol) const
{
    assert(nStartCol <= nEndCol);
    SCCOL nCount = 0;
    SCCOL nCol = nStartCol;
    ScFlatBoolColSegments::RangeData aData;
    while (nCol <= nEndCol)
    {
        if (!mpHiddenCols->getRangeData(nCol, aData))
            break;

        if (aData.mnCol2 > nEndCol)
            aData.mnCol2 = nEndCol;

        if (!aData.mbValue)
            nCount += aData.mnCol2 - nCol + 1;

        nCol = aData.mnCol2 + 1;
    }
    return nCount;
}

SCCOLROW FilterData::lastHiddenColRow(SCCOLROW nPos, bool bCol) const
{
    if (bCol)
    {
        SCCOL nCol = static_cast<SCCOL>(nPos);
        if (colHidden(nCol))
        {
            for (SCCOL i = nCol + 1; i <= getDocument().MaxCol(); ++i)
            {
                if (!colHidden(i))
                    return i - 1;
            }
        }
    }
    else
    {
        SCROW nRow = static_cast<SCROW>(nPos);
        SCROW nLastRow;
        if (rowHidden(nRow, nullptr, &nLastRow))
            return static_cast<SCCOLROW>(nLastRow);
    }
    return ::std::numeric_limits<SCCOLROW>::max();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
