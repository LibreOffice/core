/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <datatransformation.hxx>
#include <limits>
#include <document.hxx>
#include <rtl/math.hxx>
#include <cmath>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <unotools/charclass.hxx>
#include <utility>

namespace {

Date getDate(double nDateTime, const SvNumberFormatter* pFormatter)
{
    Date aDate = pFormatter->GetNullDate();
    aDate.AddDays(static_cast<sal_Int32>(::rtl::math::approxFloor(nDateTime)));
    return aDate;
}
}

namespace sc {
DataTransformation::~DataTransformation()
{
}

SCROW DataTransformation::getLastRow(const ScDocument& rDoc, SCCOL nCol)
{
    SCROW nEndRow = rDoc.MaxRow();

    return rDoc.GetLastDataRow(0, nCol, nCol, nEndRow);
}

ColumnRemoveTransformation::ColumnRemoveTransformation(std::set<SCCOL>&& rColumns):
    maColumns(std::move(rColumns))
{
}

ColumnRemoveTransformation::~ColumnRemoveTransformation()
{
}

void ColumnRemoveTransformation::Transform(ScDocument& rDoc) const
{
    sal_Int32 nIncrementIndex = 0;
    for (auto& rCol : maColumns)
    {
        rDoc.DeleteCol(0, 0, rDoc.MaxRow(), 0, rCol - nIncrementIndex, 1);
        nIncrementIndex++;
    }
}

TransformationType ColumnRemoveTransformation::getTransformationType() const
{
    return TransformationType::DELETE_TRANSFORMATION;
}

const std::set<SCCOL> & ColumnRemoveTransformation::getColumns() const
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
    if (mnCol == -1)
        return;

    rDoc.InsertCol(0, 0, rDoc.MaxRow(), 0, mnCol + 1, 1);

    SCROW nEndRow = getLastRow(rDoc, mnCol);
    for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
    {
        CellType eType = rDoc.GetCellType(mnCol, nRow, 0);
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

MergeColumnTransformation::MergeColumnTransformation( std::set<SCCOL>&& rColumns, OUString aMergeString):
    maColumns(std::move(rColumns)),
    maMergeString(std::move(aMergeString))
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
    assert(nMaxRow != -1);

    SCCOL nTargetCol = *maColumns.begin();


    for (SCROW nRow = 0; nRow <= nMaxRow; ++nRow)
    {
        OUStringBuffer aStr(rDoc.GetString(nTargetCol, nRow, 0));
        for (auto& itr : maColumns)
        {
            if (itr != nTargetCol)
            {
                aStr.append(maMergeString + rDoc.GetString(itr, nRow, 0));
            }
        }
        rDoc.SetString(nTargetCol, nRow, 0, aStr.makeStringAndClear());
    }

    for (auto& itr : maColumns)
    {
        if (itr == nTargetCol)
            continue;

        rDoc.DeleteCol(0, 0, rDoc.MaxRow(), 0, itr, 1);
    }
}

TransformationType MergeColumnTransformation::getTransformationType() const
{
    return TransformationType::MERGE_TRANSFORMATION;
}

const OUString & MergeColumnTransformation::getMergeString() const
{
    return maMergeString;
}

const std::set<SCCOL> & MergeColumnTransformation::getColumns() const
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

const ScSortParam & SortTransformation::getSortParam() const
{
    return maSortParam;
}

TextTransformation::TextTransformation( std::set<SCCOL>&& nCol, const TEXT_TRANSFORM_TYPE rType):
    mnCol(std::move(nCol)),
    maType(rType)
{
}

void TextTransformation::Transform(ScDocument& rDoc) const
{
    SCROW nEndRow = 0;
    for(auto& rCol : mnCol)
    {
        nEndRow = getLastRow(rDoc, rCol);
    }
    assert(nEndRow != -1);

    for(auto& rCol : mnCol)
    {
        switch (maType)
        {
            case TEXT_TRANSFORM_TYPE::TO_LOWER:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_STRING)
                    {
                        OUString aStr = rDoc.GetString(rCol, nRow, 0);
                        rDoc.SetString(rCol, nRow, 0, ScGlobal::getCharClass().lowercase(aStr));
                    }
                }
            }
            break;
            case TEXT_TRANSFORM_TYPE::TO_UPPER:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_STRING)
                    {
                        OUString aStr = rDoc.GetString(rCol, nRow, 0);
                        rDoc.SetString(rCol, nRow, 0, ScGlobal::getCharClass().uppercase(aStr));
                    }
                }
            }
            break;
            case TEXT_TRANSFORM_TYPE::CAPITALIZE:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_STRING)
                    {
                        OUString aStr = rDoc.GetString(rCol, nRow, 0);

                        sal_Int32 length = aStr.getLength();

                        if(length != 0)
                            aStr = aStr.replaceAt(0, 1, ScGlobal::getCharClass().uppercase(OUString(aStr[0])));

                        for (sal_Int32 i = 1; i < length; i++){
                            if (aStr[i-1] == sal_Unicode(U' '))
                            {
                                aStr = aStr.replaceAt(i, 1, ScGlobal::getCharClass().uppercase(OUString(aStr[i])));
                            }
                            else
                            {
                                aStr = aStr.replaceAt(i, 1, ScGlobal::getCharClass().lowercase(OUString(aStr[i])));
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
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
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

TEXT_TRANSFORM_TYPE TextTransformation::getTextTransformationType() const
{
    return maType;
}

const std::set<SCCOL>& TextTransformation::getColumns() const
{
    return mnCol;
}

AggregateFunction::AggregateFunction(std::set<SCCOL>&& rColumns, const AGGREGATE_FUNCTION rType):
    maColumns(std::move(rColumns)),
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
    assert(nEndRow != -1);

    for (auto& rCol : maColumns)
    {
        switch (maType)
        {
            case AGGREGATE_FUNCTION::SUM:
            {
                double nSum = 0;
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
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
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
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
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
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
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
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

AGGREGATE_FUNCTION AggregateFunction::getAggregateType() const
{
     return maType;
}

const std::set<SCCOL>& AggregateFunction::getColumns() const
{
     return maColumns;
}

NumberTransformation::NumberTransformation(std::set<SCCOL>&& nCol,
                                           const NUMBER_TRANSFORM_TYPE rType)
    : mnCol(std::move(nCol))
    , maType(rType)
    , maPrecision(-1)
{
}

NumberTransformation::NumberTransformation(std::set<SCCOL>&& nCol,
                                           const NUMBER_TRANSFORM_TYPE rType, int nPrecision)
    : mnCol(std::move(nCol))
    , maType(rType)
    , maPrecision(nPrecision)
{
}

void NumberTransformation::Transform(ScDocument& rDoc) const
{
    SCROW nEndRow = 0;
    for(auto& rCol : mnCol)
    {
        nEndRow = getLastRow(rDoc, rCol);
    }
    assert(nEndRow != -1);

    for(auto& rCol : mnCol)
    {
        switch (maType)
        {
            case NUMBER_TRANSFORM_TYPE::ROUND:
            {
                if(maPrecision > -1)
                {
                    for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                    {
                        CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                        if (eType == CELLTYPE_VALUE)
                        {
                            double nVal = rDoc.GetValue(rCol, nRow, 0);
                            rDoc.SetValue(rCol, nRow, 0, rtl::math::round(nVal, maPrecision));
                        }
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::ROUND_UP:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, rtl::math::approxCeil(nVal));
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::ROUND_DOWN:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, rtl::math::approxFloor(nVal));
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::ABSOLUTE:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if(std::signbit(nVal))
                            rDoc.SetValue(rCol, nRow, 0, -1 * nVal);
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::LOG_E:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if (nVal > 0)
                        {
                            rDoc.SetValue(rCol, nRow, 0, std::log1p(nVal-1));
                        }
                        else
                        {
                            rDoc.SetString(rCol, nRow, 0, OUString());
                        }
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::LOG_10:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if (nVal > 0)
                        {
                            rDoc.SetValue(rCol, nRow, 0, log10(nVal));
                        }
                        else
                        {
                            rDoc.SetString(rCol, nRow, 0, OUString());
                        }
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::CUBE:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, nVal * nVal * nVal);
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::SQUARE:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, nVal * nVal);
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::SQUARE_ROOT:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if (!std::signbit(nVal))
                        {
                            rDoc.SetValue(rCol, nRow, 0, sqrt(nVal));
                        }
                        else
                        {
                            rDoc.SetString(rCol, nRow, 0, OUString());
                        }
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::IS_EVEN:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if (fmod(nVal, 1) == 0 && fmod(nVal, 2) == 0)
                            rDoc.SetValue(rCol, nRow, 0, 1);
                        else
                            rDoc.SetValue(rCol, nRow, 0, 0);
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::IS_ODD:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if (fmod(nVal, 1) == 0 && fmod(nVal, 2) != 0)
                            rDoc.SetValue(rCol, nRow, 0, 1);
                        else
                            rDoc.SetValue(rCol, nRow, 0, 0);
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::SIGN:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if (nVal > 0)
                            rDoc.SetValue(rCol, nRow, 0, 1);
                        else if (nVal < 0)
                            rDoc.SetValue(rCol, nRow, 0, -1);
                        else
                            rDoc.SetValue(rCol, nRow, 0, 0);
                    }
                }
            }
            break;
            default:
            break;
        }
    }
}

TransformationType NumberTransformation::getTransformationType() const
{
    return TransformationType::NUMBER_TRANSFORMATION;
}

NUMBER_TRANSFORM_TYPE NumberTransformation::getNumberTransformationType() const
{
    return maType;
}

int NumberTransformation::getPrecision() const
{
    return maPrecision;
}

const std::set<SCCOL>& NumberTransformation::getColumn() const
{
    return mnCol;
}

ReplaceNullTransformation::ReplaceNullTransformation(std::set<SCCOL>&& nCol,
                                                     OUString sReplaceWith)
    : mnCol(std::move(nCol))
    , msReplaceWith(std::move(sReplaceWith))
{
}

void ReplaceNullTransformation::Transform(ScDocument& rDoc) const
{
    if (mnCol.empty())
        return;

    for(auto& rCol : mnCol)
    {
        SCROW nEndRow = getLastRow(rDoc, rCol);
        for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
        {
            CellType eType = rDoc.GetCellType(rCol, nRow, 0);
            if (eType == CELLTYPE_NONE)
            {
               // OUString aStr = rDoc.GetString(rCol, nRow, 0);
               // if (aStr == "" || aStr.isEmpty())
                    rDoc.SetString(rCol, nRow, 0, msReplaceWith);
            }
        }
    }

}

const std::set<SCCOL>& ReplaceNullTransformation::getColumn() const
{
    return mnCol;
}

const OUString& ReplaceNullTransformation::getReplaceString() const
{
    return msReplaceWith;
}

TransformationType ReplaceNullTransformation::getTransformationType() const
{
     return TransformationType::REMOVE_NULL_TRANSFORMATION;
}

DateTimeTransformation::DateTimeTransformation(std::set<SCCOL>&& nCol,
                                               const DATETIME_TRANSFORMATION_TYPE rType)
    : mnCol(std::move(nCol))
    , maType(rType)
{
}

void DateTimeTransformation::Transform(ScDocument& rDoc) const
{
    SCROW nEndRow = 0;
    for(auto& rCol : mnCol)
    {
        nEndRow = getLastRow(rDoc, rCol);
    }
    assert(nEndRow != -1);

    for(auto& rCol : mnCol)
    {
        switch (maType)
        {
            case DATETIME_TRANSFORMATION_TYPE::DATE_STRING:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType        eLanguage = ScGlobal::eLnge;
                sal_uInt32 nFormat = pFormatter->GetStandardFormat( SvNumFormatType::DATE, eLanguage );
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        ScAddress aAddress(rCol, nRow, 0);
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::YEAR:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetYear());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::START_OF_YEAR:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType        eLanguage = ScGlobal::eLnge;
                sal_uInt32 nFormat = pFormatter->GetStandardFormat( SvNumFormatType::DATE, eLanguage );
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        aDate.SetDay(1);
                        aDate.SetMonth(1);
                        nVal = aDate - pFormatter->GetNullDate();
                        ScAddress aAddress(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, nVal);
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::END_OF_YEAR:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType        eLanguage = ScGlobal::eLnge;
                sal_uInt32 nFormat = pFormatter->GetStandardFormat( SvNumFormatType::DATE, eLanguage );

                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        aDate.SetMonth(12);
                        aDate.SetDay(31);
                        nVal = aDate - pFormatter->GetNullDate();
                        ScAddress aAddress(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, nVal);
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::MONTH:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetMonth());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::MONTH_NAME:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType eLanguage = ScGlobal::eLnge;
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        const Color* pColor = nullptr;
                        OUString aResult;
                        pFormatter->GetPreviewStringGuess(u"MMMM"_ustr, nVal, aResult, &pColor, eLanguage);
                        rDoc.SetString(rCol, nRow, 0, aResult);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::START_OF_MONTH:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType eLanguage = ScGlobal::eLnge;
                sal_uInt32 nFormat = pFormatter->GetStandardFormat( SvNumFormatType::DATE, eLanguage );
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        ScAddress aAddress(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        aDate.SetDay(1);
                        nVal = aDate - pFormatter->GetNullDate();
                        rDoc.SetValue(rCol, nRow, 0, nVal);
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::END_OF_MONTH:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType eLanguage = ScGlobal::eLnge;
                sal_uInt32 nFormat = pFormatter->GetStandardFormat( SvNumFormatType::DATE, eLanguage );
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        ScAddress aAddress(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        aDate.SetDay(aDate.GetDaysInMonth());
                        nVal = aDate - pFormatter->GetNullDate();
                        rDoc.SetValue(rCol, nRow, 0, nVal);
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::DAY:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetDay());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::DAY_OF_WEEK:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetDayOfWeek());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::DAY_OF_YEAR:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetDayOfYear());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::QUARTER:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);

                        int nMonth = aDate.GetMonth();

                        if(nMonth >= 1 && nMonth <=3)
                            rDoc.SetValue(rCol, nRow, 0, 1);

                        else if(nMonth >= 4 && nMonth <=6)
                            rDoc.SetValue(rCol, nRow, 0, 2);

                        else if(nMonth >= 7 && nMonth <=9)
                            rDoc.SetValue(rCol, nRow, 0, 3);

                        else if(nMonth >= 10 && nMonth <=12)
                            rDoc.SetValue(rCol, nRow, 0, 4);
                        else
                            rDoc.SetValue(rCol, nRow, 0, -1);

                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::START_OF_QUARTER:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType eLanguage = ScGlobal::eLnge;
                sal_uInt32 nFormat = pFormatter->GetStandardFormat( SvNumFormatType::DATE, eLanguage );
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        ScAddress aAddress(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);

                        int nMonth = aDate.GetMonth();

                        if(nMonth >= 1 && nMonth <=3)
                        {
                            aDate.SetDay(1);
                            aDate.SetMonth(1);
                            nVal = aDate - pFormatter->GetNullDate();
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }
                        else if(nMonth >= 4 && nMonth <=6)
                        {
                            aDate.SetDay(1);
                            aDate.SetMonth(4);
                            nVal = aDate - pFormatter->GetNullDate();
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }
                        else if(nMonth >= 7 && nMonth <=9)
                        {
                            aDate.SetDay(1);
                            aDate.SetMonth(7);
                            nVal = aDate - pFormatter->GetNullDate();
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }
                        else if(nMonth >= 10 && nMonth <=12)
                        {
                            aDate.SetDay(1);
                            aDate.SetMonth(10);
                            nVal = aDate - pFormatter->GetNullDate();
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }
                        else
                            rDoc.SetValue(rCol, nRow, 0, -1);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::END_OF_QUARTER:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType        eLanguage = ScGlobal::eLnge;
                sal_uInt32 nFormat = pFormatter->GetStandardFormat( SvNumFormatType::DATE, eLanguage );
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    ScAddress aAddress(rCol, nRow, 0);
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        Date aDate = getDate(nVal, pFormatter);
                        int nMonth = aDate.GetMonth();

                        if(nMonth >= 1 && nMonth <=3)
                        {
                            aDate.SetDay(31);
                            aDate.SetMonth(3);
                            nVal = aDate - pFormatter->GetNullDate();
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }

                        else if(nMonth >= 4 && nMonth <=6)
                        {
                            aDate.SetDay(30);
                            aDate.SetMonth(6);
                            nVal = aDate - pFormatter->GetNullDate();
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }

                        else if(nMonth >= 7 && nMonth <=9)
                        {
                            aDate.SetDay(30);
                            aDate.SetMonth(9);
                            nVal = aDate - pFormatter->GetNullDate();
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }

                        else if(nMonth >= 10 && nMonth <=12)
                        {
                            aDate.SetDay(31);
                            aDate.SetMonth(12);
                            nVal = aDate - pFormatter->GetNullDate();
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }
                        else
                            rDoc.SetValue(rCol, nRow, 0, -1);

                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::TIME:
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                LanguageType eLanguage = ScGlobal::eLnge;
                sal_uInt32 nFormat = pFormatter->GetStandardFormat(SvNumFormatType::TIME, eLanguage);
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    ScAddress aAddress(rCol, nRow, 0);
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::HOUR:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        sal_uInt16 nHour, nMinute, nSecond;
                        double fFractionOfSecond;
                        tools::Time::GetClock( nVal, nHour, nMinute, nSecond, fFractionOfSecond, 0);
                        rDoc.SetValue(rCol, nRow, 0, nHour);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::MINUTE:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        sal_uInt16 nHour, nMinute, nSecond;
                        double fFractionOfSecond;
                        tools::Time::GetClock( nVal, nHour, nMinute, nSecond, fFractionOfSecond, 0);
                        rDoc.SetValue(rCol, nRow, 0, nMinute);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::SECOND:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType = rDoc.GetCellType(rCol, nRow, 0);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        sal_uInt16 nHour, nMinute, nSecond;
                        double fFractionOfSecond;
                        tools::Time::GetClock( nVal, nHour, nMinute, nSecond, fFractionOfSecond, 0);
                        rDoc.SetValue(rCol, nRow, 0, nSecond);
                    }
                }
            }
            break;
            default:
            break;
        }
    }
}

TransformationType DateTimeTransformation::getTransformationType() const
{
    return TransformationType::DATETIME_TRANSFORMATION;
}

DATETIME_TRANSFORMATION_TYPE DateTimeTransformation::getDateTimeTransformationType() const
{
    return maType;
}

const std::set<SCCOL>& DateTimeTransformation::getColumn() const
{
    return mnCol;
}

FindReplaceTransformation::FindReplaceTransformation(SCCOL nCol, OUString aFindString, OUString aReplaceString)
    : mnCol(nCol)
    , maFindString(std::move(aFindString))
    , maReplaceString(std::move(aReplaceString))
{
}

void FindReplaceTransformation::Transform(ScDocument& rDoc) const
{
    if (mnCol == -1)
        return;

    SCROW nEndRow = getLastRow(rDoc, mnCol);
    for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
    {
        CellType eType = rDoc.GetCellType(mnCol, nRow, 0);
        if (eType != CELLTYPE_NONE)
        {
            OUString aStr = rDoc.GetString(mnCol, nRow, 0);
            if (aStr == maFindString)
                rDoc.SetString(mnCol, nRow, 0, maReplaceString);
        }
    }
}

TransformationType FindReplaceTransformation::getTransformationType() const
{
    return TransformationType::FINDREPLACE_TRANSFORMATION;
}

SCCOL FindReplaceTransformation::getColumn() const
{
    return mnCol;
}

const OUString& FindReplaceTransformation::getFindString() const
{
    return maFindString;
}

const OUString& FindReplaceTransformation::getReplaceString() const
{
    return maReplaceString;
}

DeleteRowTransformation::DeleteRowTransformation(SCCOL nCol, OUString aFindString)
    : mnCol(nCol)
    , maFindString(std::move(aFindString))
{
}

void DeleteRowTransformation::Transform(ScDocument& rDoc) const
{
    sal_Int32 nIncrementIndex = 0;
    if (mnCol == -1)
        return;

    SCROW nEndRow = getLastRow(rDoc, mnCol);
    for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
    {
        CellType eType = rDoc.GetCellType(mnCol, nRow - nIncrementIndex, 0);
        if (eType != CELLTYPE_NONE)
        {
            OUString aStr = rDoc.GetString(mnCol, nRow - nIncrementIndex, 0);
            if (aStr == maFindString)
            {
                rDoc.DeleteRow(0, 0, rDoc.MaxCol(), 0, nRow - nIncrementIndex, 1);
                nIncrementIndex++;
            }
        }
    }
}

TransformationType DeleteRowTransformation::getTransformationType() const
{
    return TransformationType::DELETEROW_TRANSFORMATION;
}

SCCOL DeleteRowTransformation::getColumn() const
{
    return mnCol;
}

const OUString& DeleteRowTransformation::getFindString() const
{
    return maFindString;
}

SwapRowsTransformation::SwapRowsTransformation(SCROW mRow, SCROW nRow)
    : mxRow(mRow)
    , nxRow(nRow)
{
}

void SwapRowsTransformation::Transform(ScDocument& rDoc) const
{
    if (mxRow == -1 || nxRow == -1)
        return;

    for (SCCOL nCol = 0; nCol <= rDoc.MaxCol(); ++nCol)
    {
        CellType aType = rDoc.GetCellType(nCol, mxRow, 0);
        if (aType == CELLTYPE_STRING)
        {
            OUString aStr = rDoc.GetString(nCol, mxRow, 0);
            OUString bStr = rDoc.GetString(nCol, nxRow, 0);
            rDoc.SetString(nCol, mxRow, 0, bStr);
            rDoc.SetString(nCol, nxRow, 0, aStr);
        }
        else if (aType == CELLTYPE_VALUE)
        {
            double aVal = rDoc.GetValue(nCol, mxRow, 0);
            double bVal = rDoc.GetValue(nCol, nxRow, 0);
            rDoc.SetValue(nCol, mxRow, 0, bVal);
            rDoc.SetValue(nCol, nxRow, 0, aVal);
        }
    }
}

TransformationType SwapRowsTransformation::getTransformationType() const
{
    return TransformationType::SWAPROWS_TRANSFORMATION;
}

SCROW SwapRowsTransformation::getFirstRow() const
{
    return mxRow;
}

SCROW SwapRowsTransformation::getSecondRow() const
{
    return nxRow;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
