/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "datatransformation.hxx"

#include "document.hxx"

namespace sc {

DataTransformation::~DataTransformation()
{
}

SCROW DataTransformation::getLastRow(const ScDocument& rDoc, SCCOL nCol)
{
    SCROW nStartRow = 0;
    SCROW nEndRow = MAXROW;
    rDoc.ShrinkToDataArea(0, nCol, nStartRow, nCol, nEndRow);

    return nEndRow;
}

ColumnRemoveTransformation::ColumnRemoveTransformation(SCCOL nCol):
    mnCol(nCol)
{
}

ColumnRemoveTransformation::~ColumnRemoveTransformation()
{
}

void ColumnRemoveTransformation::Transform(ScDocument& rDoc) const
{
    rDoc.DeleteCol(0, 0, MAXROW, 0, mnCol, 1);
}

SplitColumnTransformation::SplitColumnTransformation(SCCOL nCol, sal_Unicode cSeparator):
    mnCol(nCol),
    mcSeparator(cSeparator)
{
}

void SplitColumnTransformation::Transform(ScDocument& rDoc) const
{
    rDoc.InsertCol(0, 0, MAXROW, 0, mnCol + 1, 1);

    SCROW nEndRow = getLastRow(rDoc, mnCol);
    for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
    {
        CellType eType;
        rDoc.GetCellType(mnCol, nRow, 0, eType);
        if (eType == CELLTYPE_STRING)
        {
            OUString aStr = rDoc.GetString(mnCol, nRow, 0);
            sal_Int32 nIndex = aStr.indexOf(mcSeparator);
            if (nIndex != -1)
            {
                rDoc.SetString(mnCol + 1, nRow, 0, aStr.copy(nIndex + 1));
                rDoc.SetString(mnCol, nRow, 0, aStr.copy(0, nIndex));
            }
        }
    }
}

MergeColumnTransformation::MergeColumnTransformation(SCCOL nCol1, SCCOL nCol2, const OUString& rMergeString):
    mnCol1(nCol1),
    mnCol2(nCol2),
    maMergeString(rMergeString)
{
}

void MergeColumnTransformation::Transform(ScDocument& rDoc) const
{
    SCROW nEndRow1 = getLastRow(rDoc, mnCol1);
    SCROW nEndRow2 = getLastRow(rDoc, mnCol2);
    SCROW nEndRow = std::max(nEndRow1, nEndRow2);

    for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
    {
        OUString aStr1 = rDoc.GetString(mnCol1, nRow, 0);
        OUString aStr2 = rDoc.GetString(mnCol2, nRow, 0);
        rDoc.SetString(mnCol1, nRow, 0, aStr1 + maMergeString + aStr2);
    }

    rDoc.DeleteCol(0, 0, MAXROW, 0, mnCol2, 1);
}

SortTransformation::SortTransformation(const ScSortParam& rSortParam):
    maSortParam(rSortParam)
{
}

void SortTransformation::Transform(ScDocument& rDoc) const
{
    rDoc.Sort(0, maSortParam, false, false, nullptr, nullptr);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
