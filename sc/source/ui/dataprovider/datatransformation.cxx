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
#include <tools/datetime.hxx>
#include <svl/zforlist.hxx>
namespace {

int getHour(double nDateTime)
{
    long nDays = std::trunc(nDateTime);
    double nTime = nDateTime - nDays;
    return std::trunc(nTime*24);
}

int getMinute(double nDateTime)
{
    long nDays = std::trunc(nDateTime);
    double nTime = nDateTime - nDays;
    nTime = nTime*24;
    nTime = nTime - std::trunc(nTime);
    return std::trunc(nTime*60);
}

int getSecond(double nDateTime)
{
    double nDays = std::trunc(nDateTime);
    double nTime = nDateTime - nDays;
    nTime = nTime*24;
    nTime = nTime - std::trunc(nTime);
    nTime = nTime*60;
    nTime = nTime - std::trunc(nTime);
    return std::trunc(nTime*60);
}

OUString getTwoDigitString(OUString sString)
{
    if(sString.getLength() == 1)
        sString = "0" + sString;
    return sString;
}

DateTime getDate(double nDateTime, SvNumberFormatter* pFormatter)
{
    sal_Int32 nDays = std::trunc(nDateTime);
    Date aDate =    pFormatter->GetNullDate();
    aDate.AddDays(nDays + 1);
    return aDate;
}

OUString getTimeString(double nDateTime)
{
    OUString sHour = OUString::number(getHour(nDateTime));
    sHour = getTwoDigitString(sHour);

    OUString sMinute = OUString::number(getMinute(nDateTime));
    sMinute = getTwoDigitString(sMinute);

    OUString sSecond = OUString::number(getSecond(nDateTime));
    sSecond = getTwoDigitString(sSecond);

    return sHour + ":" + sMinute + ":" + sSecond;
}
}

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

TextTransformation::TextTransformation(const std::set<SCCOL>& nCol, const TEXT_TRANSFORM_TYPE rType):
    mnCol(nCol),
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

TEXT_TRANSFORM_TYPE TextTransformation::getTextTransformationType() const
{
    return maType;
}

std::set<SCCOL> TextTransformation::getColumns() const
{
    return mnCol;
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

AGGREGATE_FUNCTION AggregateFunction::getAggregateType() const
{
     return maType;
}

std::set<SCCOL> AggregateFunction::getColumns() const
{
     return maColumns;
}

NumberTransformation::NumberTransformation(const std::set<SCCOL>& nCol,
                                           const NUMBER_TRANSFORM_TYPE rType)
    : mnCol(nCol)
    , maType(rType)
    , maPrecision(-1)
{
}

NumberTransformation::NumberTransformation(const std::set<SCCOL>& nCol,
                                           const NUMBER_TRANSFORM_TYPE rType, int nPrecision)
    : mnCol(nCol)
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
                        CellType eType;
                        rDoc.GetCellType(rCol, nRow, 0, eType);
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if(rtl::math::isSignBitSet(nVal))
                        rDoc.SetValue(rCol, nRow, 0, -1 * nVal);
                    }
                }
            }
            break;
            case NUMBER_TRANSFORM_TYPE::LOG_E:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if (nVal > 0)
                        {
                            rDoc.SetValue(rCol, nRow, 0, rtl::math::log1p(nVal-1));
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        if (!rtl::math::isSignBitSet(nVal))
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
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
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
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

NUMBER_TRANSFORM_TYPE NumberTransformation::getNumberTransfromationType() const
{
    return maType;
}

int NumberTransformation::getPrecision() const
{
    return maPrecision;
}

std::set<SCCOL> NumberTransformation::getColumn() const
{
    return mnCol;
}

ReplaceNullTransformation::ReplaceNullTransformation(const std::set<SCCOL>& nCol,
                                                     const OUString& sReplaceWith)
    : mnCol(nCol)
    , msReplaceWith(sReplaceWith)
{
}

void ReplaceNullTransformation::Transform(ScDocument& rDoc) const
{
    if (mnCol.empty())
        return;

    SCROW nEndRow = 0;
    for(auto& rCol : mnCol)
    {
        nEndRow = getLastRow(rDoc, rCol);
    }

    for(auto& rCol : mnCol)
    {
        for (SCROW nRow = 0; nRow < nEndRow; ++nRow)
        {
            CellType eType;
            rDoc.GetCellType(rCol, nRow, 0, eType);
            if (eType == CELLTYPE_NONE)
            {
               // OUString aStr = rDoc.GetString(rCol, nRow, 0);
               // if (aStr == "" || aStr.isEmpty())
                    rDoc.SetString(rCol, nRow, 0, msReplaceWith);
            }
        }
    }

}

std::set<SCCOL> ReplaceNullTransformation::getColumn() const
{
    return mnCol;
}

OUString ReplaceNullTransformation::getReplaceString() const
{
    return msReplaceWith;
}

TransformationType ReplaceNullTransformation::getTransformationType() const
{
     return TransformationType::REMOVE_NULL_TRANSFORMATION;
}

DateTimeTransformation::DateTimeTransformation(const std::set<SCCOL>& nCol,
                                               const DATETIME_TRANSFORMATION_TYPE rType)
    : mnCol(nCol)
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

    for(auto& rCol : mnCol)
    {
        switch (maType)
        {
            case DATETIME_TRANSFORMATION_TYPE::DATE_STRING:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);

                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        SvNumFormatType nFormatType = SvNumFormatType::DATE;
                        LanguageType        eLanguage = ScGlobal::eLnge;
                        ScAddress aAddress(rCol, nRow, 0);
                        sal_uLong nFormat = pFormatter->GetStandardFormat( nFormatType, eLanguage );
                        rDoc.SetValue(rCol, nRow, 0, nVal);
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::YEAR:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetYear());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::START_OF_YEAR:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);

                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);
                        nVal -= aDate.GetDayOfYear() - 2;
                        nVal = std::trunc(nVal);
                        SvNumFormatType nFormatType = SvNumFormatType::DATE;
                        LanguageType        eLanguage = ScGlobal::eLnge;
                         ScAddress aAddress(rCol, nRow, 0);
                        sal_uLong nFormat = pFormatter->GetStandardFormat( nFormatType, eLanguage );
                        rDoc.SetValue(rCol, nRow, 0, nVal);

                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::END_OF_YEAR:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);

                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);
                        nVal += ( aDate.GetDaysInYear() - aDate.GetDayOfYear() + 1);
                        nVal = std::trunc(nVal);
                        SvNumFormatType nFormatType = SvNumFormatType::DATE;
                        LanguageType        eLanguage = ScGlobal::eLnge;
                        ScAddress aAddress(rCol, nRow, 0);
                        sal_uLong nFormat = pFormatter->GetStandardFormat( nFormatType, eLanguage );
                        rDoc.SetValue(rCol, nRow, 0, nVal);

                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::MONTH:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetMonth());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::MONTH_NAME:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        OUString aMonths[] = {"January", "February", "March", "April", "May",
                       "June", "July", "August", "September", "October", "November", "December"};

                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetString(rCol, nRow, 0, aMonths[aDate.GetMonth() - 1]);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::START_OF_MONTH:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        SvNumFormatType nFormatType = SvNumFormatType::DATE;
                        LanguageType eLanguage = ScGlobal::eLnge;
                        ScAddress aAddress(rCol, nRow, 0);
                        sal_uLong nFormat = pFormatter->GetStandardFormat( nFormatType, eLanguage );

                        Date aDate = getDate(nVal, pFormatter);
                        Date aStart(1,aDate.GetMonth(), aDate.GetYear());
                        int nDays = aDate.GetDayOfYear() - aStart.GetDayOfYear() - 1;
                        rDoc.SetValue(rCol, nRow, 0, nVal - nDays);
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::END_OF_MONTH:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        SvNumFormatType nFormatType = SvNumFormatType::DATE;
                        LanguageType eLanguage = ScGlobal::eLnge;
                        ScAddress aAddress(rCol, nRow, 0);
                        sal_uLong nFormat = pFormatter->GetStandardFormat( nFormatType, eLanguage );

                        Date aDate = getDate(nVal, pFormatter);
                        Date aEnd(aDate.GetDaysInMonth(),aDate.GetMonth(), aDate.GetYear());

                        int nDays = aEnd.GetDayOfYear() - aDate.GetDayOfYear() + 1;
                        rDoc.SetValue(rCol, nRow, 0, nVal + nDays);
                        rDoc.SetNumberFormat(aAddress, nFormat);
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::DAY:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetDay());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::DAY_OF_WEEK:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetDayOfWeek());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::DAY_OF_YEAR:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);
                        rDoc.SetValue(rCol, nRow, 0, aDate.GetDayOfYear());
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::QUARTER:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        Date aDate = getDate(nVal, pFormatter);

                        int nMonth = 1 + aDate.GetMonth();

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
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                        SvNumFormatType nFormatType = SvNumFormatType::DATE;
                        LanguageType eLanguage = ScGlobal::eLnge;
                        ScAddress aAddress(rCol, nRow, 0);
                        sal_uLong nFormat = pFormatter->GetStandardFormat( nFormatType, eLanguage );
                        Date aDate = getDate(nVal, pFormatter);

                        int nMonth = aDate.GetMonth();

                        if(nMonth >= 1 && nMonth <=3)
                        {
                            Date aQuarterDate(1,1,aDate.GetYear());
                            int days = aDate.GetDayOfYear() - aQuarterDate.GetDayOfYear() - 1;
                            nVal -= days;
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }
                        else if(nMonth >= 4 && nMonth <=6)
                        {
                            Date aQuarterDate(1,4,aDate.GetYear());
                            int days = aDate.GetDayOfYear() - aQuarterDate.GetDayOfYear() - 1;
                            nVal -= days;
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }
                        else if(nMonth >= 7 && nMonth <=9)
                        {
                            Date aQuarterDate(1,7,aDate.GetYear());
                            int days = aDate.GetDayOfYear() - aQuarterDate.GetDayOfYear() - 1;
                            nVal -= days;
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }
                        else if(nMonth >= 10 && nMonth <=12)
                        {
                            Date aQuarterDate(1,10,aDate.GetYear());
                            int days = aDate.GetDayOfYear() - aQuarterDate.GetDayOfYear() - 1;
                            nVal -= days;
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
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                    SvNumFormatType nFormatType = SvNumFormatType::DATE;
                    LanguageType        eLanguage = ScGlobal::eLnge;
                    ScAddress aAddress(rCol, nRow, 0);
                    sal_uLong nFormat = pFormatter->GetStandardFormat( nFormatType, eLanguage );
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        nVal = std::trunc(nVal);
                        Date aDate = getDate(nVal, pFormatter);

                        int nMonth = aDate.GetMonth();

                        if(nMonth >= 1 && nMonth <=3)
                        {
                            Date aQuarterDate(31,3,aDate.GetYear());
                            int days = aQuarterDate.GetDayOfYear() - aDate.GetDayOfYear() + 1;
                            nVal += days;
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }

                        else if(nMonth >= 4 && nMonth <=6)
                        {
                            Date aQuarterDate(30,6,aDate.GetYear());
                            int days = aQuarterDate.GetDayOfYear() - aDate.GetDayOfYear() + 1;
                            nVal += days;
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }

                        else if(nMonth >= 7 && nMonth <=9)
                        {
                            Date aQuarterDate(30,9,aDate.GetYear());
                            int days = aQuarterDate.GetDayOfYear() - aDate.GetDayOfYear() + 1;
                            nVal += days;
                            rDoc.SetValue(rCol, nRow, 0, nVal);
                            rDoc.SetNumberFormat(aAddress, nFormat);
                        }

                        else if(nMonth >= 10 && nMonth <=12)
                        {
                            Date aQuarterDate(31,12,aDate.GetYear());
                            int days = aQuarterDate.GetDayOfYear() - aDate.GetDayOfYear() + 1;
                            nVal += days;
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
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        rDoc.SetString(rCol, nRow, 0, getTimeString(nVal));
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::HOUR:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, getHour(nVal));
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::MINUTE:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, getMinute(nVal));
                    }
                }
            }
            break;
            case DATETIME_TRANSFORMATION_TYPE::SECOND:
            {
                for (SCROW nRow = 0; nRow <= nEndRow; ++nRow)
                {
                    CellType eType;
                    rDoc.GetCellType(rCol, nRow, 0, eType);
                    if (eType == CELLTYPE_VALUE)
                    {
                        double nVal = rDoc.GetValue(rCol, nRow, 0);
                        rDoc.SetValue(rCol, nRow, 0, getSecond(nVal));
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

DATETIME_TRANSFORMATION_TYPE DateTimeTransformation::getDateTimeTransfromationType() const
{
    return maType;
}

std::set<SCCOL>DateTimeTransformation::getColumn() const
{
    return mnCol;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
