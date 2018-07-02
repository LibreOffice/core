/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <datatransformation.hxx>

#include <document.hxx>
#include <limits>

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

ColumnRemoveTransformation::ColumnRemoveTransformation(const std::set<SCCOL>& rColumns):
    maColumns(rColumns)
{
}

ColumnRemoveTransformation::~ColumnRemoveTransformation()
{
}

void ColumnRemoveTransformation::Transform(ScDocument& rDoc) const
{
    for (auto& rCol : maColumns)
    {
        rDoc.DeleteCol(0, 0, MAXROW, 0, rCol, 1);
    }
}

TransformationType ColumnRemoveTransformation::getTransformationType() const
{
    return TransformationType::DELETE_TRANSFORMATION;
}

std::set<SCCOL> ColumnRemoveTransformation::getColumns() const
{
    return maColumns;
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

TransformationType SplitColumnTransformation::getTransformationType() const
{
    return TransformationType::SPLIT_TRANSFORMATION;
}

SCCOL SplitColumnTransformation::getColumn() const
{
    return mnCol;
}

sal_Unicode SplitColumnTransformation::getSeparator() const
{
    return mcSeparator;
}

MergeColumnTransformation::MergeColumnTransformation(const std::set<SCCOL>& rColumns, const OUString& rMergeString):
    maColumns(rColumns),
    maMergeString(rMergeString)
{
}

void MergeColumnTransformation::Transform(ScDocument& rDoc) const
{
    if (maColumns.empty())
        return;

    SCROW nMaxRow = 0;
    for (auto& itr : maColumns)
    {
        nMaxRow = getLastRow(rDoc, itr);
    }

    SCCOL nTargetCol = *maColumns.begin();


    for (SCROW nRow = 0; nRow <= nMaxRow; ++nRow)
    {
        OUStringBuffer aStr = rDoc.GetString(nTargetCol, nRow, 0);
        for (auto& itr : maColumns)
        {
            if (itr != nTargetCol)
            {
                aStr.append(maMergeString).append(rDoc.GetString(itr, nRow, 0));
            }
        }
        rDoc.SetString(nTargetCol, nRow, 0, aStr.makeStringAndClear());
    }

    for (auto& itr : maColumns)
    {
        if (itr == nTargetCol)
            continue;

        rDoc.DeleteCol(0, 0, MAXROW, 0, itr, 1);
    }
}

TransformationType MergeColumnTransformation::getTransformationType() const
{
    return TransformationType::MERGE_TRANSFORMATION;
}

OUString MergeColumnTransformation::getMergeString() const
{
    return maMergeString;
}

std::set<SCCOL> MergeColumnTransformation::getColumns() const
{
    return maColumns;
}

SortTransformation::SortTransformation(const ScSortParam& rSortParam):
    maSortParam(rSortParam)
{
}

void SortTransformation::Transform(ScDocument& rDoc) const
{
    rDoc.Sort(0, maSortParam, false, false, nullptr, nullptr);
}

TransformationType SortTransformation::getTransformationType() const
{
    return TransformationType::SORT_TRANSFORMATION;
}

ScSortParam SortTransformation::getSortParam() const
{
    return maSortParam;
}

TextTransformation::TextTransformation(const std::set<SCCOL> nCol, const TEXT_TRANSFORM_TYPE rType):
    mnCol(nCol),
    maType(rType)
{
}

void TextTransformation::Transform(ScDocument& rDoc) const
{   
    SCROW nEndRow = 0;
    for(auto& rCol : mnCol)
    {
        nEndRow = getLastRow(rDoc, mnCol);
    }
    for(auto& rCol : mnCol)
    {
        switch (maType)
        {
            case TEXT_TRANSFORM_TYPE::TO_LOWER:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_STRING)
                    {
                        OUString aStr = rDoc.GetString(rCol, nRow, 0);
                        rDoc.SetString(rCol, nRow, 0, ScGlobal::pCharClass->lowercase(aStr));
                    }
                }
            }
            break;
            case TEXT_TRANSFORM_TYPE::TO_UPPER:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_STRING)
                    {
                        OUString aStr = rDoc.GetString(rCol, nRow, 0);
                        rDoc.SetString(rCol, nRow, 0, ScGlobal::pCharClass->uppercase(aStr));
                    }
                }
            }
            break;
            case TEXT_TRANSFORM_TYPE::CAPITALIZE:
            {
                sal_Unicode separator = sal_Unicode(U' ');
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_STRING)
                    {
                        OUString aStr = rDoc.GetString(rCol, nRow, 0);

                        sal_Int32 length = aStr.getLength();

                        if(length != 0)
                            aStr = aStr.replaceAt(0, 1, ScGlobal::pCharClass->uppercase(OUString(aStr[0])));

                        for (sal_Int32 i = 1; i < length; i++){
                            if (aStr[i-1] == separator)
                            {
                                aStr = aStr.replaceAt(i, 1, ScGlobal::pCharClass->uppercase(OUString(aStr[i])));
                            }
                            else
                            {
                                aStr = aStr.replaceAt(i, 1, ScGlobal::pCharClass->lowercase(OUString(aStr[i])));
                            }
                        }
                        rDoc.SetString(rCol, nRow, 0, aStr);
                    }
                }
            }
            break;
            case TEXT_TRANSFORM_TYPE::TRIM:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_STRING)
                    {
                        OUString aStr = rDoc.GetString(rCol, nRow, 0);
                        rDoc.SetString(rCol, nRow, 0, aStr.trim());
                    }
                }
            }
            break;
            default:
            break;
        }
    }
}

TransformationType TextTransformation::getTransformationType() const
{
    return TransformationType::TEXT_TRANSFORMATION;
}

AggregateFunction::AggregateFunction(const std::set<SCCOL>& rColumns, const AGGREGATE_FUNCTION rType):
    maColumns(rColumns),
    maType(rType)
{
}

void AggregateFunction::Transform(ScDocument& rDoc) const
{
    SCROW nEndRow = 0;
    for (auto& itr : maColumns)
    {
        nEndRow = getLastRow(rDoc, itr);
    }

    for (auto& rCol : maColumns)
    {
        switch (maType)
        {
            case AGGREGATE_FUNCTION::SUM:
            {
                double nSum = 0;
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        nSum += nVal;
                    }
                }
                rDoc.SetValue(rCol, nEndRow + 1, 0, nSum);
            }
            break;
            case AGGREGATE_FUNCTION::AVERAGE:
            {
                double nSum = 0;
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        nSum += nVal;
                    }
                }

                double nAvg = nSum / (nEndRow + 1);
                rDoc.SetValue(rCol, nEndRow + 1, 0, nAvg);
            }
            break;
            case AGGREGATE_FUNCTION::MIN:
            {
                double nMin = std::numeric_limits<double>::max();
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if(nVal < nMin)
                            nMin = nVal;
                    }
                }
                rDoc.SetValue(rCol, nEndRow + 1, 0, nMin);
            }
            break;
            case AGGREGATE_FUNCTION::MAX:
            {
                double nMax = std::numeric_limits<double>::lowest();
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if(nMax < nVal)
                            nMax = nVal;
                    }
                }
                rDoc.SetValue(rCol, nEndRow + 1, 0, nMax);
            }
            break;
            default:
            break;
        }
    }
}

TransformationType AggregateFunction::getTransformationType() const
{
    return TransformationType::AGGREGATE_FUNCTION;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
