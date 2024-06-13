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

#include <scitems.hxx>
#include <comphelper/string.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/brushitem.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <vcl/keycodes.hxx>
#include <rtl/math.hxx>
#include <unotools/charclass.hxx>
#include <tools/duration.hxx>
#include <osl/diagnose.h>

#include <attrib.hxx>
#include <patattr.hxx>
#include <formulacell.hxx>
#include <table.hxx>
#include <global.hxx>
#include <document.hxx>
#include <autoform.hxx>
#include <userlist.hxx>
#include <zforauto.hxx>
#include <subtotal.hxx>
#include <formula/errorcodes.hxx>
#include <docpool.hxx>
#include <progress.hxx>
#include <conditio.hxx>
#include <editutil.hxx>
#include <listenercontext.hxx>
#include <scopetools.hxx>
#include <o3tl/string_view.hxx>

#include <math.h>
#include <memory>
#include <list>
#include <string_view>

#define D_MAX_LONG_  double(0x7fffffff)

namespace {

short lcl_DecompValueString( OUString& rValue, sal_Int32& nVal, sal_uInt16* pMinDigits = nullptr )
{
    if ( rValue.isEmpty() )
    {
        nVal = 0;
        return 0;
    }
    const sal_Unicode* p = rValue.getStr();
    sal_Int32 nSign = 0;
    sal_Int32 nNum = 0;
    if ( p[nNum] == '-' || p[nNum] == '+' )
        nNum = nSign = 1;
    while ( p[nNum] && CharClass::isAsciiNumeric( std::u16string_view(&p[nNum], 1) ) )
        nNum++;

    sal_Unicode cNext = p[nNum];            // 0 if at the end
    sal_Unicode cLast = p[rValue.getLength()-1];

    // #i5550# If there are numbers at the beginning and the end,
    // prefer the one at the beginning only if it's followed by a space.
    // Otherwise, use the number at the end, to enable things like IP addresses.
    if ( nNum > nSign && ( cNext == 0 || cNext == ' ' || !CharClass::isAsciiNumeric(std::u16string_view(&cLast, 1)) ) )
    {   // number at the beginning
        nVal = o3tl::toInt32(rValue.subView( 0, nNum ));
        //  any number with a leading zero sets the minimum number of digits
        if ( p[nSign] == '0' && pMinDigits && ( nNum - nSign > *pMinDigits ) )
            *pMinDigits = nNum - nSign;
        rValue = rValue.copy(nNum);
        return -1;
    }
    else
    {
        nSign = 0;
        sal_Int32 nEnd = nNum = rValue.getLength() - 1;
        while ( nNum && CharClass::isAsciiNumeric( std::u16string_view(&p[nNum], 1) ) )
            nNum--;
        if ( p[nNum] == '-' || p[nNum] == '+' )
        {
            nNum--;
            nSign = 1;
        }
        if ( nNum < nEnd - nSign )
        {   // number at the end
            nVal = o3tl::toInt32(rValue.subView( nNum + 1 ));
            //  any number with a leading zero sets the minimum number of digits
            if ( p[nNum+1+nSign] == '0' && pMinDigits && ( nEnd - nNum - nSign > *pMinDigits ) )
                *pMinDigits = nEnd - nNum - nSign;
            rValue = rValue.copy(0, nNum + 1);
            if (nSign) // use the return value = 2 to put back the '+'
                return 2;
            else
                return 1;
        }
    }
    nVal = 0;
    return 0;
}

OUString lcl_ValueString( sal_Int32 nValue, sal_uInt16 nMinDigits )
{
    if ( nMinDigits <= 1 )
        return OUString::number( nValue );           // simple case...
    else
    {
        OUString aStr = OUString::number( std::abs( nValue ) );
        if ( aStr.getLength() < nMinDigits )
        {
            OUStringBuffer aZero(nMinDigits);
            comphelper::string::padToLength(aZero, nMinDigits - aStr.getLength(), '0');
            aStr = aZero.append(aStr).makeStringAndClear();
        }
        //  nMinDigits doesn't include the '-' sign -> add after inserting zeros
        if ( nValue < 0 )
            aStr = "-" + aStr;
        return aStr;
    }
}

void setSuffixCell(
    ScColumn& rColumn, SCROW nRow, sal_Int32 nValue, sal_uInt16 nDigits,
    std::u16string_view rSuffix,
    CellType eCellType, bool bIsOrdinalSuffix )
{
    ScDocument& rDoc = rColumn.GetDoc();
    OUString aValue = lcl_ValueString(nValue, nDigits);
    if (!bIsOrdinalSuffix)
    {
        aValue += rSuffix;
        rColumn.SetRawString(nRow, aValue);
        return;
    }

    OUString aOrdinalSuffix = ScGlobal::GetOrdinalSuffix(nValue);
    if (eCellType != CELLTYPE_EDIT)
    {
        aValue += aOrdinalSuffix;
        rColumn.SetRawString(nRow, aValue);
        return;
    }

    EditEngine aEngine(rDoc.GetEnginePool());
    aEngine.SetEditTextObjectPool(rDoc.GetEditPool());

    SfxItemSet aAttr = aEngine.GetEmptyItemSet();
    aAttr.Put( SvxEscapementItem( SvxEscapement::Superscript, EE_CHAR_ESCAPEMENT));
    aEngine.SetText( aValue );
    aEngine.QuickInsertText(
        aOrdinalSuffix,
        ESelection(0, aValue.getLength(), 0, aValue.getLength() + aOrdinalSuffix.getLength()));

    aEngine.QuickSetAttribs(
        aAttr,
        ESelection(0, aValue.getLength(), 0, aValue.getLength() + aOrdinalSuffix.getLength()));

    // Text object instance will be owned by the cell.
    rColumn.SetEditText(nRow, aEngine.CreateTextObject());
}

}

namespace {
/* TODO: move this to rtl::math::approxDiff() ? Though the name is funny, the
 * approx is expected to be more correct than the raw diff. */
/** Calculate a-b trying to diminish precision errors such as for 0.11-0.12
    not return -0.009999999999999995 but -0.01 instead.
 */
double approxDiff( double a, double b )
{
    if (a == b)
        return 0.0;
    if (a == 0.0)
        return -b;
    if (b == 0.0)
        return a;
    const double c = a - b;
    const double aa = fabs(a);
    const double ab = fabs(b);
    if (aa < 1e-16 || aa > 1e+16 || ab < 1e-16 || ab > 1e+16)
        // This is going nowhere, live with the result.
        return c;

    const double q = aa < ab ? b / a : a / b;
    const double d = (a * q - b * q) / q;
    if (d == c)
        // No differing error, live with the result.
        return c;

    // We now have two subtractions with a similar but not equal error. Obtain
    // the exponent of the error magnitude and round accordingly.
    const double e = fabs(d - c);
    const int nExp = static_cast<int>(floor(log10(e))) + 1;
    // tdf#129606: Limit precision to the 16th significant digit of the least precise argument.
    // Cf. mnMaxGeneralPrecision in sc/source/core/data/column3.cxx.
    const int nExpArg = static_cast<int>(floor(log10(std::max(aa, ab)))) - 15;
    return rtl::math::round(c, -std::max(nExp, nExpArg));
}

double approxTypedDiff( double a, double b, bool bTime, tools::Duration& rDuration )
{
    if (bTime)
    {
        rDuration = tools::Duration(a - b);
        return rDuration.GetInDays();
    }
    return approxDiff( a, b);
}
}

void ScTable::FillAnalyse( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            FillCmd& rCmd, FillDateCmd& rDateCmd,
                            double& rInc, tools::Duration& rDuration, sal_uInt16& rMinDigits,
                            ScUserListData*& rListData, sal_uInt16& rListIndex,
                            bool bHasFiltered, bool& rSkipOverlappedCells,
                            std::vector<sal_Int32>& rNonOverlappedCellIdx)
{
    OSL_ENSURE( nCol1==nCol2 || nRow1==nRow2, "FillAnalyse: invalid range" );

    rInc = 0.0;
    rDuration = tools::Duration();
    rMinDigits = 0;
    rListData = nullptr;
    rCmd = FILL_SIMPLE;
    rSkipOverlappedCells = false;
    if ( nScFillModeMouseModifier & KEY_MOD1 )
        return ;        // Ctrl-key: Copy

    SCCOL nAddX;
    SCROW nAddY;
    SCSIZE nCount;
    if (nCol1 == nCol2)
    {
        nAddX = 0;
        nAddY = 1;
        nCount = static_cast<SCSIZE>(nRow2 - nRow1 + 1);
    }
    else
    {
        nAddX = 1;
        nAddY = 0;
        nCount = static_cast<SCSIZE>(nCol2 - nCol1 + 1);
    }

    // Try to analyse the merged cells only if there are no filtered rows in the destination area
    // Else fallback to the old way to avoid regression.
    // Filling merged cells into an area with filtered (hidden) rows, is a very complex task
    // that is not implemented, but not even decided how to do, even excel can't handle that well
    if (!bHasFiltered)
    {
        bool bHasOverlappedCells = false;
        bool bSkipOverlappedCells = true;
        SCCOL nColCurr = nCol1;
        SCROW nRowCurr = nRow1;

        // collect cells that are not empty or not overlapped
        rNonOverlappedCellIdx.resize(nCount);
        SCSIZE nValueCount = 0;
        for (SCSIZE i = 0; i < nCount; ++i)
        {
            const ScPatternAttr* pPattern = GetPattern(nColCurr, nRowCurr);
            bool bOverlapped
                = pPattern->GetItemSet().GetItemState(ATTR_MERGE_FLAG, false) == SfxItemState::SET
                  && pPattern->GetItem(ATTR_MERGE_FLAG).IsOverlapped();

            if (bOverlapped)
                bHasOverlappedCells = true;

            if (!bOverlapped || GetCellValue(nColCurr, nRowCurr).getType() != CELLTYPE_NONE)
            {
                rNonOverlappedCellIdx[nValueCount++] = i;
                // if there is at least 1 non empty overlapped cell, then no cell should be skipped
                if (bOverlapped)
                    bSkipOverlappedCells = false;
            }

            nColCurr += nAddX;
            nRowCurr += nAddY;
        }
        rNonOverlappedCellIdx.resize(nValueCount);

        // if all the values are overlapped CELLTYPE_NONE, then there is no need to analyse it.
        if (nValueCount == 0)
            return;

        // if there is no overlapped cells, there is nothing to skip
        if (!bHasOverlappedCells)
            bSkipOverlappedCells = false;

        if (bSkipOverlappedCells)
        {
            nColCurr = nCol1 + rNonOverlappedCellIdx[0] * nAddX;
            nRowCurr = nRow1 + rNonOverlappedCellIdx[0] * nAddY;
            ScRefCellValue aPrevCell, aCurrCell;
            aCurrCell = GetCellValue(nColCurr, nRowCurr);
            CellType eCellType = aCurrCell.getType();
            if (eCellType == CELLTYPE_VALUE)
            {
                bool bVal = true;
                double fVal;
                SvNumFormatType nCurrCellFormatType
                    = rDocument.GetFormatTable()->GetType(GetNumberFormat(nColCurr, nRowCurr));
                if (nCurrCellFormatType == SvNumFormatType::DATE)
                {
                    if (nValueCount >= 2)
                    {
                        tools::Long nCmpInc = 0;
                        FillDateCmd eType = FILL_YEAR;  // just some temporary default values
                        tools::Long nDDiff = 0, nMDiff = 0, nYDiff = 0; // to avoid warnings
                        Date aNullDate = rDocument.GetFormatTable()->GetNullDate();
                        Date aCurrDate = aNullDate, aPrevDate = aNullDate;
                        aCurrDate.AddDays(aCurrCell.getDouble());
                        for (SCSIZE i = 1; i < nValueCount && bVal; i++)
                        {
                            aPrevCell = aCurrCell;
                            aPrevDate = aCurrDate;
                            nColCurr = nCol1 + rNonOverlappedCellIdx[i] * nAddX;
                            nRowCurr = nRow1 + rNonOverlappedCellIdx[i] * nAddY;
                            aCurrCell = GetCellValue(nColCurr, nRowCurr);
                            if (aCurrCell.getType() == CELLTYPE_VALUE)
                            {
                                aCurrDate = aNullDate + static_cast<sal_Int32>(aCurrCell.getDouble());
                                if (eType != FILL_DAY) {
                                    nDDiff = aCurrDate.GetDay()
                                             - static_cast<tools::Long>(aPrevDate.GetDay());
                                    nMDiff = aCurrDate.GetMonth()
                                             - static_cast<tools::Long>(aPrevDate.GetMonth());
                                    nYDiff = aCurrDate.GetYear()
                                             - static_cast<tools::Long>(aPrevDate.GetYear());
                                }
                                if (i == 1)
                                {
                                    if (nDDiff != 0)
                                    {
                                        eType = FILL_DAY;
                                        nCmpInc = aCurrDate - aPrevDate;
                                    }
                                    else
                                    {
                                        eType = FILL_MONTH;
                                        nCmpInc = nMDiff + 12 * nYDiff;
                                    }
                                }
                                else if (eType == FILL_DAY)
                                {
                                    if (aCurrDate - aPrevDate != nCmpInc)
                                        bVal = false;
                                }
                                else
                                {
                                    if (nDDiff || (nMDiff + 12 * nYDiff != nCmpInc))
                                        bVal = false;
                                }
                            }
                            else
                                bVal = false;   // No date is also not ok
                        }
                        if (bVal)
                        {
                            if (eType == FILL_MONTH && (nCmpInc % 12 == 0))
                            {
                                eType = FILL_YEAR;
                                nCmpInc /= 12;
                            }
                            rCmd = FILL_DATE;
                            rDateCmd = eType;
                            rInc = nCmpInc;
                            rSkipOverlappedCells = true;
                            return;
                        }
                    }
                    else
                    {
                        rCmd = FILL_DATE;
                        rDateCmd = FILL_DAY;
                        rInc = 1.0;
                        rSkipOverlappedCells = true;
                        return;
                    }
                }
                else if (nCurrCellFormatType == SvNumFormatType::LOGICAL
                         && ((fVal = aCurrCell.getDouble()) == 0.0 || fVal == 1.0))
                {
                }
                else if (nValueCount >= 2)
                {
                    tools::Duration aDuration;
                    for (SCSIZE i = 1; i < nValueCount && bVal; i++)
                    {
                        aPrevCell = aCurrCell;
                        nColCurr = nCol1 + rNonOverlappedCellIdx[i] * nAddX;
                        nRowCurr = nRow1 + rNonOverlappedCellIdx[i] * nAddY;
                        aCurrCell = GetCellValue(nColCurr, nRowCurr);
                        if (aCurrCell.getType() == CELLTYPE_VALUE)
                        {
                            const bool bTime = (nCurrCellFormatType == SvNumFormatType::TIME ||
                                    nCurrCellFormatType == SvNumFormatType::DATETIME);
                            double nDiff = approxTypedDiff(aCurrCell.getDouble(), aPrevCell.getDouble(),
                                    bTime, aDuration);
                            if (i == 1)
                            {
                                rInc = nDiff;
                                if (bTime)
                                    rDuration = aDuration;
                            }
                            if (!::rtl::math::approxEqual(nDiff, rInc, 13))
                                bVal = false;
                            else if ((aCurrCell.getDouble() == 0.0 || aCurrCell.getDouble() == 1.0)
                                     && (rDocument.GetFormatTable()->GetType(
                                             GetNumberFormat(nColCurr, nRowCurr))
                                         == SvNumFormatType::LOGICAL))
                                bVal = false;
                        }
                        else
                            bVal = false;
                    }
                    if (bVal)
                    {
                        rCmd = FILL_LINEAR;
                        rSkipOverlappedCells = true;
                        return;
                    }
                }
            }
            else if (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT)
            {
                OUString aStr = GetString(nColCurr, nRowCurr );
                OUString aStr2;

                rListData = const_cast<ScUserListData*>(ScGlobal::GetUserList().GetData(aStr));
                if (rListData)
                {
                    bool bMatchCase = false;
                    (void)rListData->GetSubIndex(aStr, rListIndex, bMatchCase);
                    size_t nListStrCount = rListData->GetSubCount();
                    sal_uInt16 nPrevListIndex, nInc = 1;
                    for (SCSIZE i = 1; i < nValueCount && rListData; i++)
                    {
                        nColCurr = nCol1 + rNonOverlappedCellIdx[i] * nAddX;
                        nRowCurr = nRow1 + rNonOverlappedCellIdx[i] * nAddY;
                        aStr2 = GetString(nColCurr, nRowCurr);

                        nPrevListIndex = rListIndex;
                        if (!rListData->GetSubIndex(aStr2, rListIndex, bMatchCase))
                            rListData = nullptr;
                        else
                        {
                            sal_Int32 nIncCurr = rListIndex - nPrevListIndex;
                            if (nIncCurr < 0)
                                nIncCurr += nListStrCount;
                            if (i == 1)
                                nInc = nIncCurr;
                            else if (nInc != nIncCurr)
                                rListData = nullptr;
                        }
                    }
                    if (rListData) {
                        rInc = nInc;
                        rSkipOverlappedCells = true;
                        return;
                    }
                }
                short nFlag1, nFlag2;
                sal_Int32 nVal1, nVal2;
                nFlag1 = lcl_DecompValueString(aStr, nVal1, &rMinDigits);
                if (nFlag1)
                {
                    bool bVal = true;
                    rInc = 1;
                    for (SCSIZE i = 1; i < nValueCount && bVal; i++)
                    {
                        nColCurr = nCol1 + rNonOverlappedCellIdx[i] * nAddX;
                        nRowCurr = nRow1 + rNonOverlappedCellIdx[i] * nAddY;
                        ScRefCellValue aCell = GetCellValue(nColCurr, nRowCurr);
                        CellType eType = aCell.getType();
                        if (eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT)
                        {
                            aStr2 = aCell.getString(&rDocument);
                            nFlag2 = lcl_DecompValueString(aStr2, nVal2, &rMinDigits);
                            if (nFlag1 == nFlag2 && aStr == aStr2)
                            {
                                double nDiff = approxDiff(nVal2, nVal1);
                                if (i == 1)
                                    rInc = nDiff;
                                else if (!::rtl::math::approxEqual(nDiff, rInc, 13))
                                    bVal = false;
                                nVal1 = nVal2;
                            }
                            else
                                bVal = false;
                        }
                        else
                            bVal = false;
                    }
                    if (bVal)
                    {
                        rCmd = FILL_LINEAR;
                        rSkipOverlappedCells = true;
                        return;
                    }
                }
            }
        }
    }

    //if it is not a FILL_LINEAR - CELLTYPE_VALUE - with merged cells [without hidden values]
    //then do it in the old way

    SCCOL nCol = nCol1;
    SCROW nRow = nRow1;

    ScRefCellValue aFirstCell = GetCellValue(nCol, nRow);
    CellType eCellType = aFirstCell.getType();

    if (eCellType == CELLTYPE_VALUE)
    {
        double fVal;
        sal_uInt32 nFormat = GetAttr(nCol,nRow,ATTR_VALUE_FORMAT)->GetValue();
        const SvNumFormatType nFormatType = rDocument.GetFormatTable()->GetType(nFormat);
        bool bDate = (nFormatType == SvNumFormatType::DATE);        // date without time
        bool bTime = (nFormatType == SvNumFormatType::TIME || nFormatType == SvNumFormatType::DATETIME);
        bool bBooleanCell = (nFormatType == SvNumFormatType::LOGICAL);
        if (bDate)
        {
            if (nCount > 1)
            {
                double nVal;
                Date aNullDate = rDocument.GetFormatTable()->GetNullDate();
                Date aDate1 = aNullDate;
                nVal = aFirstCell.getDouble();
                aDate1.AddDays(nVal);
                Date aDate2 = aNullDate;
                nVal = GetValue(nCol+nAddX, nRow+nAddY);
                aDate2.AddDays(nVal);
                if ( aDate1 != aDate2 )
                {
                    tools::Long nCmpInc = 0;
                    FillDateCmd eType;
                    tools::Long nDDiff = aDate2.GetDay()   - static_cast<tools::Long>(aDate1.GetDay());
                    tools::Long nMDiff = aDate2.GetMonth() - static_cast<tools::Long>(aDate1.GetMonth());
                    tools::Long nYDiff = aDate2.GetYear()  - static_cast<tools::Long>(aDate1.GetYear());
                    if (nMDiff && aDate1.IsEndOfMonth() && aDate2.IsEndOfMonth())
                    {
                        eType = FILL_END_OF_MONTH;
                        nCmpInc = nMDiff + 12 * nYDiff;
                    }
                    else if (nDDiff)
                    {
                        eType = FILL_DAY;
                        nCmpInc = aDate2 - aDate1;
                    }
                    else
                    {
                        eType = FILL_MONTH;
                        nCmpInc = nMDiff + 12 * nYDiff;
                    }

                    nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                    nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                    bool bVal = true;
                    for (SCSIZE i=1; i<nCount && bVal; i++)
                    {
                        ScRefCellValue aCell = GetCellValue(nCol,nRow);
                        if (aCell.getType() == CELLTYPE_VALUE)
                        {
                            nVal = aCell.getDouble();
                            aDate2 = aNullDate + static_cast<sal_Int32>(nVal);
                            if ( eType == FILL_DAY )
                            {
                                if ( aDate2-aDate1 != nCmpInc )
                                    bVal = false;
                            }
                            else
                            {
                                nDDiff = aDate2.GetDay()   - static_cast<tools::Long>(aDate1.GetDay());
                                nMDiff = aDate2.GetMonth() - static_cast<tools::Long>(aDate1.GetMonth());
                                nYDiff = aDate2.GetYear()  - static_cast<tools::Long>(aDate1.GetYear());
                                if ((nDDiff && !aDate1.IsEndOfMonth() && !aDate2.IsEndOfMonth())
                                    || (nMDiff + 12 * nYDiff != nCmpInc))
                                    bVal = false;
                            }
                            aDate1 = aDate2;
                            nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                            nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                        }
                        else
                            bVal = false;   // No date is also not ok
                    }
                    if (bVal)
                    {
                        if ((eType == FILL_MONTH || eType == FILL_END_OF_MONTH)
                            && (nCmpInc % 12 == 0))
                        {
                            eType = FILL_YEAR;
                            nCmpInc /= 12;
                        }
                        rCmd = FILL_DATE;
                        rDateCmd = eType;
                        rInc = nCmpInc;
                    }
                }
                else
                {
                    // tdf#89754 - don't increment non different consecutive date cells
                    rCmd = FILL_DATE;
                    rDateCmd = FILL_DAY;
                    rInc = 0.0;
                }
            }
            else                            // single date -> increment by days
            {
                rCmd = FILL_DATE;
                rDateCmd = FILL_DAY;
                rInc = 1.0;
            }
        }
        else if (bBooleanCell && ((fVal = aFirstCell.getDouble()) == 0.0 || fVal == 1.0))
        {
            // Nothing, rInc stays 0.0, no specific fill mode.
        }
        else
        {
            if (nCount > 1)
            {
                tools::Duration aDuration;
                double nVal1 = aFirstCell.getDouble();
                double nVal2 = GetValue(nCol+nAddX, nRow+nAddY);
                rInc = approxTypedDiff( nVal2, nVal1, bTime, aDuration);
                nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                bool bVal = true;
                for (SCSIZE i=1; i<nCount && bVal; i++)
                {
                    ScRefCellValue aCell = GetCellValue(nCol,nRow);
                    if (aCell.getType() == CELLTYPE_VALUE)
                    {
                        nVal2 = aCell.getDouble();
                        double nDiff = approxTypedDiff( nVal2, nVal1, bTime, aDuration);
                        if ( !::rtl::math::approxEqual( nDiff, rInc, 13 ) )
                            bVal = false;
                        else if ((nVal2 == 0.0 || nVal2 == 1.0) &&
                                (rDocument.GetFormatTable()->GetType(GetNumberFormat(nCol,nRow)) ==
                                 SvNumFormatType::LOGICAL))
                            bVal = false;
                        nVal1 = nVal2;
                    }
                    else
                        bVal = false;
                    nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                    nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                    if (bVal && bTime)
                        rDuration = aDuration;
                }
                if (bVal)
                    rCmd = FILL_LINEAR;
            }
            else if(nFormatType == SvNumFormatType::PERCENT)
            {
                rInc = 0.01; // tdf#89998 increment by 1% at a time
            }
        }
    }
    else if (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT)
    {
        OUString aStr = GetString(nCol, nRow);

        rListData = const_cast<ScUserListData*>(ScGlobal::GetUserList().GetData(aStr));
        if (rListData)
        {
            bool bMatchCase = false;
            (void)rListData->GetSubIndex(aStr, rListIndex, bMatchCase);
            size_t nListStrCount = rListData->GetSubCount();
            sal_uInt16 nPrevListIndex, nInc = 1;
            nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
            nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
            for (SCSIZE i=1; i<nCount && rListData; i++)
            {
                nPrevListIndex = rListIndex;
                aStr = GetString(nCol, nRow);
                if (!rListData->GetSubIndex(aStr, rListIndex, bMatchCase))
                    rListData = nullptr;
                else
                {
                    sal_Int32 nIncCurr = rListIndex - nPrevListIndex;
                    if (nIncCurr < 0)
                        nIncCurr += nListStrCount;
                    if (i == 1)
                        nInc = nIncCurr;
                    else if (nInc != nIncCurr)
                        rListData = nullptr;
                }
                nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
            }
            if (rListData)
                rInc = nInc;
        }
        else if ( nCount > 1 )
        {
            //  pass rMinDigits to all DecompValueString calls
            //  -> longest number defines rMinDigits

            sal_Int32 nVal1;
            short nFlag1 = lcl_DecompValueString( aStr, nVal1, &rMinDigits );
            if ( nFlag1 )
            {
                sal_Int32 nVal2;
                aStr = GetString( nCol+nAddX, nRow+nAddY );
                short nFlag2 = lcl_DecompValueString( aStr, nVal2, &rMinDigits );
                if ( nFlag1 == nFlag2 )
                {
                    rInc = approxDiff( nVal2, nVal1);
                    nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                    nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                    bool bVal = true;
                    for (SCSIZE i=1; i<nCount && bVal; i++)
                    {
                        ScRefCellValue aCell = GetCellValue(nCol, nRow);
                        CellType eType = aCell.getType();
                        if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
                        {
                            aStr = aCell.getString(&rDocument);
                            nFlag2 = lcl_DecompValueString( aStr, nVal2, &rMinDigits );
                            if ( nFlag1 == nFlag2 )
                            {
                                double nDiff = approxDiff( nVal2, nVal1);
                                if ( !::rtl::math::approxEqual( nDiff, rInc, 13 ) )
                                    bVal = false;
                                nVal1 = nVal2;
                            }
                            else
                                bVal = false;
                        }
                        else
                            bVal = false;
                        nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                        nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                    }
                    if (bVal)
                        rCmd = FILL_LINEAR;
                }
            }
        }
        else
        {
            //  call DecompValueString to set rMinDigits
            sal_Int32 nDummy;
            lcl_DecompValueString( aStr, nDummy, &rMinDigits );
        }
    }
}

void ScTable::FillFormula(
    const ScFormulaCell* pSrcCell, SCCOL nDestCol, SCROW nDestRow, bool bLast )
{

    rDocument.SetNoListening( true );  // still the wrong reference
    ScAddress aAddr( nDestCol, nDestRow, nTab );
    ScFormulaCell* pDestCell = new ScFormulaCell( *pSrcCell, rDocument, aAddr );
    aCol[nDestCol].SetFormulaCell(nDestRow, pDestCell);

    if ( bLast && pDestCell->GetMatrixFlag() != ScMatrixMode::NONE )
    {
        ScAddress aOrg;
        if ( pDestCell->GetMatrixOrigin( GetDoc(), aOrg ) )
        {
            if ( nDestCol >= aOrg.Col() && nDestRow >= aOrg.Row() )
            {
                ScFormulaCell* pOrgCell = rDocument.GetFormulaCell(aOrg);
                if (pOrgCell && pOrgCell->GetMatrixFlag() == ScMatrixMode::Formula)
                {
                    pOrgCell->SetMatColsRows(
                        nDestCol - aOrg.Col() + 1,
                        nDestRow - aOrg.Row() + 1 );
                }
                else
                {
                    OSL_FAIL( "FillFormula: MatrixOrigin no formula cell with ScMatrixMode::Formula" );
                }
            }
            else
            {
                OSL_FAIL( "FillFormula: MatrixOrigin bottom right" );
            }
        }
        else
        {
            OSL_FAIL( "FillFormula: no MatrixOrigin" );
        }
    }
    rDocument.SetNoListening( false );
    pDestCell->StartListeningTo( rDocument );
}

void ScTable::FillAuto( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        sal_uInt64 nFillCount, FillDir eFillDir, ScProgress* pProgress )
{
    if ( (nFillCount == 0) || !ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2) )
        return;

    //  Detect direction

    bool bVertical = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP);
    bool bPositive = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_RIGHT);

    SCCOLROW nCol = 0;
    SCCOLROW nRow = 0;
    SCCOLROW& rInner = bVertical ? nRow : nCol;        // loop variables
    SCCOLROW& rOuter = bVertical ? nCol : nRow;
    SCCOLROW nOStart;
    SCCOLROW nOEnd;
    SCCOLROW nIStart;
    SCCOLROW nIEnd;
    SCCOLROW nISrcStart;
    SCCOLROW nISrcEnd;
    ScRange aFillRange;

    if (bVertical)
    {
        nOStart = nCol1;
        nOEnd = nCol2;
        if (bPositive)
        {
            nISrcStart = nRow1;
            nISrcEnd = nRow2;
            nIStart = nRow2 + 1;
            nIEnd = nRow2 + nFillCount;
            aFillRange = ScRange(nCol1, nRow2+1, 0, nCol2, nRow2 + nFillCount, 0);
        }
        else
        {
            nISrcStart = nRow2;
            nISrcEnd = nRow1;
            nIStart = nRow1 - 1;
            nIEnd = nRow1 - nFillCount;
            aFillRange = ScRange(nCol1, nRow1-1, 0, nCol2, nRow2 - nFillCount, 0);
        }
    }
    else
    {
        nOStart = nRow1;
        nOEnd = nRow2;
        if (bPositive)
        {
            nISrcStart = nCol1;
            nISrcEnd = nCol2;
            nIStart = nCol2 + 1;
            nIEnd = nCol2 + nFillCount;
            aFillRange = ScRange(nCol2 + 1, nRow1, 0, nCol2 + nFillCount, nRow2, 0);
        }
        else
        {
            nISrcStart = nCol2;
            nISrcEnd = nCol1;
            nIStart = nCol1 - 1;
            nIEnd = nCol1 - nFillCount;
            aFillRange = ScRange(nCol1 - 1, nRow1, 0, nCol1 - nFillCount, nRow2, 0);
        }
    }
    sal_uInt64 nIMin = nIStart;
    sal_uInt64 nIMax = nIEnd;
    PutInOrder(nIMin,nIMax);
    bool bHasFiltered = IsDataFiltered(aFillRange);

    if (!bHasFiltered)
    {
        if (bVertical)
            DeleteArea(nCol1, static_cast<SCROW>(nIMin), nCol2, static_cast<SCROW>(nIMax), InsertDeleteFlags::AUTOFILL);
        else
            DeleteArea(static_cast<SCCOL>(nIMin), nRow1, static_cast<SCCOL>(nIMax), nRow2, InsertDeleteFlags::AUTOFILL);
    }

    sal_uInt64 nProgress = 0;
    if (pProgress)
        nProgress = pProgress->GetState();

    // Avoid possible repeated calls to StartListeningFormulaCells() (tdf#132165).
    std::list< sc::DelayStartListeningFormulaCells > delayStartListening;
    SCCOL delayStartColumn, delayEndColumn;
    if(bVertical)
    {
        delayStartColumn = std::min( nOStart, nOEnd );
        delayEndColumn = std::max( nOStart, nOEnd );
    }
    else
    {
        delayStartColumn = std::min( nIStart, nIEnd );
        delayEndColumn = std::max( nIStart, nIEnd );
    }
    for( SCROW col = delayStartColumn; col <= delayEndColumn; ++col )
    {
        if( ScColumn* column = FetchColumn( col ))
            delayStartListening.emplace_back( *column, true );
    }

    //  execute

    sal_uInt64 nActFormCnt = 0;
    for (rOuter = nOStart; rOuter <= nOEnd; rOuter++)
    {
        sal_uInt64 nMaxFormCnt = 0;                      // for formulas

        //  transfer attributes

        const ScPatternAttr* pSrcPattern = nullptr;
        const ScStyleSheet* pStyleSheet = nullptr;
        SCCOLROW nAtSrc = nISrcStart;
        std::unique_ptr<ScPatternAttr> pNewPattern;
        bool bGetPattern = true;
        rInner = nIStart;
        while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
        {
            if (!ColHidden(nCol) && !RowHidden(nRow))
            {
                if ( bGetPattern )
                {
                    if (bVertical)      // rInner&:=nRow, rOuter&:=nCol
                        pSrcPattern = GetColumnData(nCol).GetPattern(static_cast<SCROW>(nAtSrc));
                    else                // rInner&:=nCol, rOuter&:=nRow
                        pSrcPattern = GetColumnData(nAtSrc).GetPattern(static_cast<SCROW>(nRow));
                    bGetPattern = false;
                    pStyleSheet = pSrcPattern->GetStyleSheet();
                    // do transfer ATTR_MERGE / ATTR_MERGE_FLAG
                    //
                    // Note: ATTR_MERGE is an attribute of the top left cell of a merged area
                    // containing the size of the area. ATTR_MERGE_FLAGs are attributes of the
                    // other cells of a merged area, containing the information about also
                    // overlapping, i.e. visibility of their content.
                    //
                    // TODO: extend the similar incomplete selections to a bounding rectangle to
                    // avoid incomplete fill, where not all AUTO_MERGE_FLAGs are synchronized with
                    // the copied ATTR_MERGE, resulting broken grid and visibility during run-time.
                    //
                    //  +--+        +--+--+
                    //  |  |        |  |  |
                    //  +--+--+     +--+--+
                    //  |     | ->  |     |
                    //  +--+--+     +--+--+
                    //  |  |        |  |  |
                    //  +--+        +--+--+
                    //
                    // TODO: protect incompatible merged cells of the destination area, for example
                    // by skipping the fill operation.
                    //
                    // TODO: by dragging the fill handle select only the multiples of the height
                    // of the originally selected area which is merged vertically to avoid of
                    // incomplete fill.
                    //
                    //  +--+     +--+
                    //  |XX|     |XX|
                    //  +XX+     +XX+
                    //  |XX| ->  |XX|
                    //  +--+     +--+
                    //  |  |     |  |
                    //  +--+     +--+
                    //           |  |
                    //           +--+
                    //
                    // Other things stored in ATTR_MERGE_FLAG, like autofilter button, will be
                    // deleted now, but may need to be repaired later, like at ScDocument::Fill.
                    const SfxItemSet& rSet = pSrcPattern->GetItemSet();
                    if ( rSet.GetItemState(ATTR_MERGE_FLAG, false) == SfxItemState::SET )
                    {
                        ScMF nOldValue = pSrcPattern->GetItem(ATTR_MERGE_FLAG).GetValue();
                        ScMF nOldValueMerge = nOldValue & (ScMF::Hor | ScMF::Ver);
                        // keep only the merge flags
                        if ( nOldValue != nOldValueMerge )
                        {
                            pNewPattern.reset(new ScPatternAttr(*pSrcPattern));
                            SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                            if ( nOldValueMerge == ScMF::NONE )
                                rNewSet.ClearItem(ATTR_MERGE_FLAG);
                            else
                                rNewSet.Put(ScMergeFlagAttr(nOldValueMerge));
                        }
                        else
                            pNewPattern.reset();
                    }
                    else
                        pNewPattern.reset();
                }

                const ScCondFormatItem& rCondFormatItem = pSrcPattern->GetItem(ATTR_CONDITIONAL);
                const ScCondFormatIndexes& rCondFormatIndex = rCondFormatItem.GetCondFormatData();

                if ( bVertical && nISrcStart == nISrcEnd && !bHasFiltered )
                {
                    //  set all attributes at once (en bloc)
                    if (pNewPattern || !pSrcPattern->isDefault())
                    {
                        //  Default is already present (DeleteArea)
                        SCROW nY1 = static_cast<SCROW>(std::min( nIStart, nIEnd ));
                        SCROW nY2 = static_cast<SCROW>(std::max( nIStart, nIEnd ));
                        if ( pStyleSheet )
                            aCol[nCol].ApplyStyleArea( nY1, nY2, *pStyleSheet );
                        if ( pNewPattern )
                            aCol[nCol].ApplyPatternArea( nY1, nY2, *pNewPattern );
                        else
                            aCol[nCol].ApplyPatternArea( nY1, nY2, *pSrcPattern );

                        for(const auto& rIndex : rCondFormatIndex)
                        {
                            ScConditionalFormat* pCondFormat = mpCondFormatList->GetFormat(rIndex);
                            if (pCondFormat)
                            {
                                ScRangeList aRange = pCondFormat->GetRange();
                                aRange.Join(ScRange(nCol, nY1, nTab, nCol, nY2, nTab));
                                pCondFormat->SetRange(aRange);
                            }
                        }
                    }

                    break;
                }

                if ( bHasFiltered )
                    DeleteArea(static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow),
                            static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), InsertDeleteFlags::AUTOFILL);

                if ( !ScPatternAttr::areSame(pSrcPattern, aCol[nCol].GetPattern( static_cast<SCROW>(nRow) ) ) )
                {
                    // Transfer template too
                    //TODO: Merge ApplyPattern to AttrArray ??
                    if ( pStyleSheet )
                        aCol[nCol].ApplyStyle( static_cast<SCROW>(nRow), pStyleSheet );

                    //  Use ApplyPattern instead of SetPattern to keep old MergeFlags
                    if ( pNewPattern )
                        aCol[nCol].ApplyPattern( static_cast<SCROW>(nRow), *pNewPattern );
                    else
                        aCol[nCol].ApplyPattern( static_cast<SCROW>(nRow), *pSrcPattern );

                    for(const auto& rIndex : rCondFormatIndex)
                    {
                        ScConditionalFormat* pCondFormat = mpCondFormatList->GetFormat(rIndex);
                        if (pCondFormat)
                        {
                            ScRangeList aRange = pCondFormat->GetRange();
                            aRange.Join(ScRange(nCol, nRow, nTab, nCol, nRow, nTab));
                            pCondFormat->SetRange(aRange);
                        }
                    }
                }

                if (nAtSrc==nISrcEnd)
                {
                    if ( nAtSrc != nISrcStart )
                    {    // More than one source cell
                        nAtSrc = nISrcStart;
                        bGetPattern = true;
                    }
                }
                else if (bPositive)
                {
                    ++nAtSrc;
                    bGetPattern = true;
                }
                else
                {
                    --nAtSrc;
                    bGetPattern = true;
                }
            }

            if (rInner == nIEnd) break;
            if (bPositive) ++rInner; else --rInner;
        }
        pNewPattern.reset();

        //  Analyse

        FillCmd eFillCmd;
        FillDateCmd eDateCmd = {};
        double nInc;
        tools::Duration aDurationInc;
        sal_uInt16 nMinDigits;
        ScUserListData* pListData = nullptr;
        sal_uInt16 nListIndex;
        bool bSkipOverlappedCells;
        std::vector<sal_Int32> aNonOverlappedCellIdx;
        if (bVertical)
            FillAnalyse(static_cast<SCCOL>(nCol),nRow1,
                    static_cast<SCCOL>(nCol),nRow2, eFillCmd,eDateCmd,
                    nInc, aDurationInc, nMinDigits, pListData, nListIndex,
                    bHasFiltered, bSkipOverlappedCells, aNonOverlappedCellIdx);
        else
            FillAnalyse(nCol1,static_cast<SCROW>(nRow),
                    nCol2,static_cast<SCROW>(nRow), eFillCmd,eDateCmd,
                    nInc, aDurationInc, nMinDigits, pListData, nListIndex,
                    bHasFiltered, bSkipOverlappedCells, aNonOverlappedCellIdx);

        if (pListData)
        {
            sal_uInt16 nListCount = pListData->GetSubCount();
            if (bSkipOverlappedCells)
            {
                int nFillerCount = 1 + ( nISrcEnd - nISrcStart ) * (bPositive ? 1 : -1);
                std::vector<bool> aIsNonEmptyCell(nFillerCount, false);
                SCCOLROW nLastValueIdx;
                if (bPositive)
                {
                    nLastValueIdx = nISrcEnd - (nFillerCount - 1 - aNonOverlappedCellIdx.back());
                    for (auto i : aNonOverlappedCellIdx)
                        aIsNonEmptyCell[i] = true;
                }
                else
                {
                    nLastValueIdx = nISrcEnd + aNonOverlappedCellIdx[0];
                    for (auto i : aNonOverlappedCellIdx)
                        aIsNonEmptyCell[nFillerCount - 1 - i] = true;
                }

                OUString aStr;
                if (bVertical)
                    aStr = GetString(rOuter, nLastValueIdx);
                else
                    aStr = GetString(nLastValueIdx, rOuter);

                bool bMatchCase = false;
                (void)pListData->GetSubIndex(aStr, nListIndex, bMatchCase);

                sal_Int32 nFillerIdx = 0;
                rInner = nIStart;
                while (true)
                {
                    if (aIsNonEmptyCell[nFillerIdx])
                    {
                        if (bPositive)
                        {
                            nListIndex += nInc;
                            if (nListIndex >= nListCount) nListIndex -= nListCount;
                        }
                        else
                        {
                            if (nListIndex < nInc) nListIndex += nListCount;
                            nListIndex -= nInc;
                        }
                        aCol[nCol].SetRawString(static_cast<SCROW>(nRow), pListData->GetSubStr(nListIndex));

                    }
                    if (rInner == nIEnd) break;
                    nFillerIdx = (nFillerIdx + 1) % nFillerCount;
                    if (bPositive)
                        ++rInner;
                    else
                        --rInner;
                }
            }
            else
            {
                if (!bPositive)
                {
                    //  nListIndex of FillAnalyse points to the last entry -> adjust
                    sal_Int64 nAdjust = nListIndex - (nISrcStart - nISrcEnd) * nInc;
                    nAdjust = nAdjust % nListCount;
                    if (nAdjust < 0)
                        nAdjust += nListCount;
                    nListIndex = nAdjust;
                }

                rInner = nIStart;
                while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
                {
                    if (!ColHidden(nCol) && !RowHidden(nRow))
                    {
                        if (bPositive)
                        {
                            nListIndex += nInc;
                            if (nListIndex >= nListCount) nListIndex -= nListCount;
                        }
                        else
                        {
                            if (nListIndex < nInc) nListIndex += nListCount;
                            nListIndex -= nInc;
                        }
                        aCol[nCol].SetRawString(static_cast<SCROW>(nRow), pListData->GetSubStr(nListIndex));
                    }

                    if (rInner == nIEnd) break;
                    if (bPositive) ++rInner; else --rInner;
                }
            }
            if(pProgress)
            {
                nProgress += nIMax - nIMin + 1;
                pProgress->SetStateOnPercent( nProgress );
            }
        }
        else if (eFillCmd == FILL_SIMPLE)           // fill with pattern/sample
        {
            FillAutoSimple(
                nISrcStart, nISrcEnd, nIStart, nIEnd, rInner, nCol, nRow,
                nActFormCnt, nMaxFormCnt, bHasFiltered, bVertical, bPositive, pProgress, nProgress);
        }
        else
        {
            if (!bPositive)
            {
                nInc = -nInc;
                aDurationInc = -aDurationInc;
            }
            double nEndVal = (nInc>=0.0) ? MAXDOUBLE : -MAXDOUBLE;
            if (bVertical)
                FillSeries( static_cast<SCCOL>(nCol), nRow1,
                        static_cast<SCCOL>(nCol), nRow2, nFillCount, eFillDir,
                        eFillCmd, eDateCmd, nInc, aDurationInc, nEndVal, nMinDigits, false,
                        pProgress, bSkipOverlappedCells, &aNonOverlappedCellIdx);
            else
                FillSeries( nCol1, static_cast<SCROW>(nRow), nCol2,
                        static_cast<SCROW>(nRow), nFillCount, eFillDir,
                        eFillCmd, eDateCmd, nInc, aDurationInc, nEndVal, nMinDigits, false,
                        pProgress, bSkipOverlappedCells, &aNonOverlappedCellIdx);
            if (pProgress)
                nProgress = pProgress->GetState();
        }

        if (bVertical)
            FillSparkline(bVertical, nCol, nRow1, nRow2, nIStart, nIEnd);
        else
            FillSparkline(bVertical, nRow, nCol1, nCol2, nIStart, nIEnd);

        nActFormCnt += nMaxFormCnt;
    }
}

void  ScTable::FillSparkline(bool bVertical, SCCOLROW nFixed,
                             SCCOLROW nStart, SCCOLROW nEnd,
                             SCCOLROW nFillStart, SCCOLROW nFillEnd)
{
    bool bHasSparklines = false;
    std::vector<std::shared_ptr<sc::Sparkline>> aSparklineSeries;

    for (SCROW nCurrent = nStart; nCurrent <= nEnd; nCurrent++)
    {
        auto pSparkline = bVertical ? GetSparkline(nFixed, nCurrent) : GetSparkline(nCurrent, nFixed);
        bHasSparklines = bHasSparklines || pSparkline;
        aSparklineSeries.push_back(pSparkline);
    }

    if (bHasSparklines)
    {
        for (SCCOLROW nCurrent = nFillStart; nCurrent <= nFillEnd; nCurrent++)
        {
            size_t nIndex = size_t(nFillStart - nCurrent) % aSparklineSeries.size();
            if (auto& rpSparkline = aSparklineSeries[nIndex])
            {
                auto pGroup = rpSparkline->getSparklineGroup();

                auto* pNewSparkline = bVertical ? CreateSparkline(nFixed, nCurrent, pGroup)
                                                : CreateSparkline(nCurrent, nFixed, pGroup);
                if (pNewSparkline)
                {
                    SCCOLROW nPosition = bVertical ? rpSparkline->getRow()
                                                   : rpSparkline->getColumn();
                    SCCOLROW nDelta = nCurrent - nPosition;
                    ScRangeList aRangeList(rpSparkline->getInputRange());
                    for (ScRange& rRange : aRangeList)
                    {
                        if (bVertical)
                        {
                            rRange.aStart.IncRow(nDelta);
                            rRange.aEnd.IncRow(nDelta);
                        }
                        else
                        {
                            rRange.aStart.IncCol(nDelta);
                            rRange.aEnd.IncCol(nDelta);
                        }
                    }
                    pNewSparkline->setInputRange(aRangeList);
                }
            }
        }
    }
}

void ScTable::GetBackColorArea(SCCOL& rStartCol, SCROW& /*rStartRow*/,
                               SCCOL& rEndCol, SCROW& rEndRow ) const
{
    bool bExtend;
    const SvxBrushItem* pDefBackground = &rDocument.GetPool()->GetUserOrPoolDefaultItem(ATTR_BACKGROUND);

    rStartCol = std::min<SCCOL>(rStartCol, aCol.size() - 1);
    rEndCol = std::min<SCCOL>(rEndCol, aCol.size() - 1);

    do
    {
        bExtend = false;

        if (rEndRow < rDocument.MaxRow())
        {
            for (SCCOL nCol = rStartCol; nCol <= rEndCol; ++nCol)
            {
                const ScPatternAttr* pPattern = GetColumnData(nCol).GetPattern(rEndRow + 1);
                const SvxBrushItem* pBackground = &pPattern->GetItem(ATTR_BACKGROUND);
                if (!pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty() ||
                    (pBackground->GetColor() != COL_TRANSPARENT && pBackground != pDefBackground))
                {
                    bExtend = true;
                    break;
                }
            }

            if (bExtend)
                ++rEndRow;
        }
    } while (bExtend);
}

OUString ScTable::GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY )
{
    OUString aValue;

    SCCOL nCol1 = rSource.aStart.Col();
    SCROW nRow1 = rSource.aStart.Row();
    SCCOL nCol2 = rSource.aEnd.Col();
    SCROW nRow2 = rSource.aEnd.Row();
    bool bOk = true;
    tools::Long nIndex = 0;
    sal_uInt64 nSrcCount = 0;
    FillDir eFillDir = FILL_TO_BOTTOM;
    if ( nEndX == nCol2 && nEndY == nRow2 )     // empty
        bOk = false;
    else if ( nEndX == nCol2 )                  // to up / down
    {
        nCol2 = nCol1;                          // use only first column
        nSrcCount = nRow2 - nRow1 + 1;
        nIndex = static_cast<tools::Long>(nEndY) - nRow1;         // can be negative
        if ( nEndY >= nRow1 )
            eFillDir = FILL_TO_BOTTOM;
        else
            eFillDir = FILL_TO_TOP;
    }
    else if ( nEndY == nRow2 )                  // to left / right
    {
        nEndY = nRow2 = nRow1;                  // use only first row
        nSrcCount = nCol2 - nCol1 + 1;
        nIndex = static_cast<tools::Long>(nEndX) - nCol1;         // can be negative
        if ( nEndX >= nCol1 )
            eFillDir = FILL_TO_RIGHT;
        else
            eFillDir = FILL_TO_LEFT;
    }
    else                                        // direction not clear
        bOk = false;

    if ( bOk )
    {
        tools::Long nBegin = 0;
        tools::Long nEnd = 0;
        tools::Long nHidden = 0;
        if (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP)
        {
            if (nEndY > nRow1)
            {
                nBegin = nRow2+1;
                nEnd = nEndY;
            }
            else
            {
                nBegin = nEndY;
                nEnd = nRow1 -1;
            }

            tools::Long nVisible = CountVisibleRows(nBegin, nEnd);
            nHidden = nEnd + 1 - nBegin - nVisible;
        }
        else
        {
            if (nEndX > nCol1)
            {
                nBegin = nCol2+1;
                nEnd = nEndX;
            }
            else
            {
                nBegin = nEndX;
                nEnd = nCol1 -1;
            }

            tools::Long nVisible = CountVisibleCols(nBegin, nEnd);
            nHidden = nEnd + 1 - nBegin - nVisible;
        }
        if (nHidden)
        {
            if (nIndex > 0)
                nIndex = nIndex - nHidden;
            else
                nIndex = nIndex + nHidden;
        }

        FillCmd eFillCmd;
        FillDateCmd eDateCmd;
        double nInc;
        tools::Duration aDurationInc;
        sal_uInt16 nMinDigits;
        ScUserListData* pListData = nullptr;
        sal_uInt16 nListIndex;
        bool bSkipOverlappedCells;
        std::vector<sal_Int32> aNonOverlappedCellIdx;

        // Todo: update this function to calculate with merged cell fills,
        //       after FillAnalyse / FillSeries fully handle them.
        // Now FillAnalyse called as if there are filtered rows, so it will work in the old way.
        FillAnalyse(nCol1, nRow1, nCol2, nRow2, eFillCmd, eDateCmd,
                    nInc, aDurationInc, nMinDigits, pListData, nListIndex,
                    true, bSkipOverlappedCells, aNonOverlappedCellIdx);

        if ( pListData )                            // user defined list
        {
            sal_uInt16 nListCount = pListData->GetSubCount();
            if ( nListCount )
            {
                sal_uInt64 nSub = nSrcCount - 1; //  nListIndex is from last source entry
                while ( nIndex < sal::static_int_cast<tools::Long>(nSub) )
                    nIndex += nListCount;
                sal_uInt64 nPos = ( nListIndex + nIndex - nSub ) % nListCount;
                aValue = pListData->GetSubStr(sal::static_int_cast<sal_uInt16>(nPos));
            }
        }
        else if ( eFillCmd == FILL_SIMPLE )         // fill with pattern/sample
        {
            tools::Long nPosIndex = nIndex;
            while ( nPosIndex < 0 )
                nPosIndex += nSrcCount;
            sal_uInt64 nPos = nPosIndex % nSrcCount;
            SCCOL nSrcX = nCol1;
            SCROW nSrcY = nRow1;
            if ( eFillDir == FILL_TO_TOP || eFillDir == FILL_TO_BOTTOM )
                nSrcY = sal::static_int_cast<SCROW>( nSrcY + static_cast<SCROW>(nPos) );
            else
                nSrcX = sal::static_int_cast<SCCOL>( nSrcX + static_cast<SCCOL>(nPos) );

            ScRefCellValue aCell = GetCellValue(nSrcX, nSrcY);
            if (!aCell.isEmpty())
            {
                sal_Int32 nDelta;
                if (nIndex >= 0)
                    nDelta = nIndex / nSrcCount;
                else
                    nDelta = ( nIndex - nSrcCount + 1 ) / nSrcCount;    // -1 -> -1

                CellType eType = aCell.getType();
                switch ( eType )
                {
                    case CELLTYPE_STRING:
                    case CELLTYPE_EDIT:
                    {
                        aValue = aCell.getString(&rDocument);

                        if ( !(nScFillModeMouseModifier & KEY_MOD1) )
                        {
                            sal_Int32 nVal;
                            sal_uInt16 nCellDigits = 0; // look at each source cell individually
                            short nFlag = lcl_DecompValueString( aValue, nVal, &nCellDigits );
                            if ( nFlag < 0 )
                            {
                                if (aValue == ScGlobal::GetOrdinalSuffix( nVal))
                                    aValue = ScGlobal::GetOrdinalSuffix( nVal + nDelta);
                                aValue = lcl_ValueString( nVal + nDelta, nCellDigits ) + aValue;
                            }
                            else if ( nFlag > 0 )
                            {
                                sal_Int32 nNextValue;
                                if ( nVal < 0 )
                                    nNextValue = nVal - nDelta;
                                else
                                    nNextValue = nVal + nDelta;
                                if ( nFlag == 2 && nNextValue >= 0 ) // Put back the '+'
                                    aValue += "+";
                                aValue += lcl_ValueString( nNextValue, nCellDigits );
                            }
                        }
                    }
                    break;
                    case CELLTYPE_VALUE:
                    {
                        sal_uInt32 nNumFmt = GetNumberFormat( nSrcX, nSrcY );
                        //  overflow is possible...
                        double nVal = aCell.getDouble();
                        if ( !(nScFillModeMouseModifier & KEY_MOD1) )
                        {
                            const SvNumFormatType nFormatType = rDocument.GetFormatTable()->GetType(nNumFmt);
                            bool bPercentCell = (nFormatType == SvNumFormatType::PERCENT);
                            if (bPercentCell)
                            {
                                // tdf#89998 increment by 1% at a time
                                nVal += static_cast<double>(nDelta) * 0.01;
                            }
                            else if (nVal == 0.0 || nVal == 1.0)
                            {
                                bool bBooleanCell = (nFormatType == SvNumFormatType::LOGICAL);
                                if (!bBooleanCell)
                                    nVal += static_cast<double>(nDelta);
                            }
                            else
                            {
                                nVal += static_cast<double>(nDelta);
                            }
                        }

                        const Color* pColor;
                        rDocument.GetFormatTable()->GetOutputString( nVal, nNumFmt, aValue, &pColor );
                    }
                    break;
                    //  not for formulas
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
        }
        else if ( eFillCmd == FILL_LINEAR || eFillCmd == FILL_DATE )        // values
        {
            bool bValueOk;
            double nStart;
            sal_Int32 nVal = 0;
            short nHeadNoneTail = 0;
            ScRefCellValue aCell = GetCellValue(nCol1, nRow1);
            if (!aCell.isEmpty())
            {
                CellType eType = aCell.getType();
                switch ( eType )
                {
                    case CELLTYPE_STRING:
                    case CELLTYPE_EDIT:
                    {
                        aValue = aCell.getString(&rDocument);
                        nHeadNoneTail = lcl_DecompValueString( aValue, nVal );
                        if ( nHeadNoneTail )
                            nStart = static_cast<double>(nVal);
                        else
                            nStart = 0.0;
                    }
                    break;
                    case CELLTYPE_VALUE:
                        nStart = aCell.getDouble();
                    break;
                    case CELLTYPE_FORMULA:
                        nStart = aCell.getFormula()->GetValue();
                    break;
                    default:
                        nStart = 0.0;
                }
            }
            else
                nStart = 0.0;
            if ( eFillCmd == FILL_LINEAR )
            {
                if (aDurationInc)
                {
                    bool bOverflow;
                    tools::Duration aDuration( aDurationInc.Mult( nIndex, bOverflow));
                    bValueOk = SubTotal::SafePlus( nStart, aDuration.GetInDays()) && !bOverflow;
                }
                else
                {
                    double nAdd = nInc;
                    bValueOk = ( SubTotal::SafeMult( nAdd, static_cast<double>(nIndex) ) &&
                                 SubTotal::SafePlus( nStart, nAdd ) );
                }
            }
            else        // date
            {
                bValueOk = true;
                sal_uInt16 nDayOfMonth = 0;
                if ( nIndex < 0 )
                {
                    nIndex = -nIndex;
                    nInc = -nInc;
                }
                for (tools::Long i=0; i<nIndex; i++)
                    IncDate( nStart, nDayOfMonth, nInc, eDateCmd );
            }

            if (bValueOk)
            {
                if ( nHeadNoneTail )
                {
                    if ( nHeadNoneTail < 0 )
                    {
                        if (aValue == ScGlobal::GetOrdinalSuffix( nVal))
                            aValue = ScGlobal::GetOrdinalSuffix( static_cast<sal_Int32>(nStart) );

                        aValue = lcl_ValueString( static_cast<sal_Int32>(nStart), nMinDigits ) + aValue;
                    }
                    else
                    {
                        if ( nHeadNoneTail == 2 && nStart >= 0 ) // Put back the '+'
                            aValue += "+";
                        aValue += lcl_ValueString( static_cast<sal_Int32>(nStart), nMinDigits );
                    }
                }
                else
                {
                    //TODO: get number format according to Index?
                    const Color* pColor;
                    sal_uInt32 nNumFmt = GetNumberFormat( nCol1, nRow1 );
                    rDocument.GetFormatTable()->GetOutputString( nStart, nNumFmt, aValue, &pColor );
                }
            }
        }
        else
        {
            OSL_FAIL("GetAutoFillPreview: invalid mode");
        }
    }

    return aValue;
}

void ScTable::IncDate(double& rVal, sal_uInt16& nDayOfMonth, double nStep, FillDateCmd eCmd)
{
    if (eCmd == FILL_DAY)
    {
        rVal += nStep;
        return;
    }

    // class Date limits
    const sal_uInt16 nMinYear = 1583;
    const sal_uInt16 nMaxYear = 9956;

    tools::Long nInc = static_cast<tools::Long>(nStep);       // upper/lower limits ?
    Date aNullDate = rDocument.GetFormatTable()->GetNullDate();
    Date aDate = aNullDate;
    aDate.AddDays(rVal);
    switch (eCmd)
    {
        case FILL_WEEKDAY:
            {
                aDate.AddDays(nInc);
                DayOfWeek eWeekDay = aDate.GetDayOfWeek();
                if (nInc >= 0)
                {
                    if (eWeekDay == SATURDAY)
                        aDate.AddDays(2);
                    else if (eWeekDay == SUNDAY)
                        aDate.AddDays(1);
                }
                else
                {
                    if (eWeekDay == SATURDAY)
                        aDate.AddDays(-1);
                    else if (eWeekDay == SUNDAY)
                        aDate.AddDays(-2);
                }
            }
            break;
        case FILL_MONTH:
        case FILL_END_OF_MONTH:
            {
                if ( nDayOfMonth == 0 )
                    nDayOfMonth = aDate.GetDay();       // init
                tools::Long nMonth = aDate.GetMonth();
                tools::Long nYear = aDate.GetYear();

                nMonth += nInc;

                if (nInc >= 0)
                {
                    if (nMonth > 12)
                    {
                        tools::Long nYAdd = (nMonth-1) / 12;
                        nMonth -= nYAdd * 12;
                        nYear += nYAdd;
                    }
                }
                else
                {
                    if (nMonth < 1)
                    {
                        tools::Long nYAdd = 1 - nMonth / 12;       // positive
                        nMonth += nYAdd * 12;
                        nYear -= nYAdd;
                    }
                }

                if ( nYear < nMinYear )
                    aDate = Date( 1,1, nMinYear );
                else if ( nYear > nMaxYear )
                    aDate = Date( 31,12, nMaxYear );
                else
                {
                    aDate.SetMonth(static_cast<sal_uInt16>(nMonth));
                    aDate.SetYear(static_cast<sal_uInt16>(nYear));
                    if (eCmd == FILL_END_OF_MONTH)
                    {
                        aDate.SetDay(Date::GetDaysInMonth(nMonth, nYear));
                    }
                    else
                    {
                        aDate.SetDay(std::min(Date::GetDaysInMonth(nMonth, nYear), nDayOfMonth));
                    }
                }
            }
            break;
        case FILL_YEAR:
            {
                tools::Long nYear = aDate.GetYear();
                nYear += nInc;
                if ( nYear < nMinYear )
                    aDate = Date( 1,1, nMinYear );
                else if ( nYear > nMaxYear )
                    aDate = Date( 31,12, nMaxYear );
                else
                    aDate.SetYear(static_cast<sal_uInt16>(nYear));
            }
            break;
        default:
        {
            // added to avoid warnings
        }
    }

    rVal = aDate - aNullDate;
}

namespace {

bool HiddenRowColumn(const ScTable* pTable, SCCOLROW nRowColumn, bool bVertical, SCCOLROW& rLastPos)
{
    bool bHidden = false;
    if(bVertical)
    {
        SCROW nLast;
        bHidden = pTable->RowHidden(nRowColumn, nullptr, &nLast);
        rLastPos = nLast;
    }
    else
    {
        SCCOL nLast;
        bHidden = pTable->ColHidden(static_cast<SCCOL>(nRowColumn), nullptr, &nLast);
        rLastPos = nLast;
    }
    return bHidden;
}

}

void ScTable::FillFormulaVertical(
    const ScFormulaCell& rSrcCell,
    SCCOLROW& rInner, SCCOL nCol, SCROW nRow1, SCROW nRow2,
    ScProgress* pProgress, sal_uInt64& rProgress )
{
    // rInner is the row position when filling vertically.  Also, when filling
    // across hidden regions, it may create multiple dis-jointed spans of
    // formula cells.

    bool bHidden = false;
    SCCOLROW nHiddenLast = -1;

    SCCOLROW nRowStart = -1, nRowEnd = -1;
    std::vector<sc::RowSpan> aSpans;
    PutInOrder(nRow1, nRow2);
    for (rInner = nRow1; rInner <= nRow2; ++rInner)
    {
        if (rInner > nHiddenLast)
            bHidden = HiddenRowColumn(this, rInner, true, nHiddenLast);

        if (bHidden)
        {
            if (nRowStart >= 0)
            {
                nRowEnd = rInner - 1;
                aSpans.emplace_back(nRowStart, nRowEnd);
                nRowStart = -1;
            }
            rInner = nHiddenLast;
            continue;
        }

        if (nRowStart < 0)
            nRowStart = rInner;
    }

    if (nRowStart >= 0)
    {
        nRowEnd = rInner - 1;
        aSpans.emplace_back(nRowStart, nRowEnd);
    }

    if (aSpans.empty())
        return;

    aCol[nCol].DeleteRanges(aSpans, InsertDeleteFlags::VALUE | InsertDeleteFlags::DATETIME | InsertDeleteFlags::STRING | InsertDeleteFlags::FORMULA | InsertDeleteFlags::OUTLINE);
    aCol[nCol].CloneFormulaCell(rSrcCell, sc::CellTextAttr(), aSpans);

    auto pSet = std::make_shared<sc::ColumnBlockPositionSet>(rDocument);
    sc::StartListeningContext aStartCxt(rDocument, pSet);
    sc::EndListeningContext aEndCxt(rDocument, pSet);

    SCROW nStartRow = aSpans.front().mnRow1;
    SCROW nEndRow = aSpans.back().mnRow2;
    aCol[nCol].EndListeningFormulaCells(aEndCxt, nStartRow, nEndRow, &nStartRow, &nEndRow);
    aCol[nCol].StartListeningFormulaCells(aStartCxt, aEndCxt, nStartRow, nEndRow);

    for (const auto& rSpan : aSpans)
        aCol[nCol].SetDirty(rSpan.mnRow1, rSpan.mnRow2, ScColumn::BROADCAST_NONE);

    rProgress += nRow2 - nRow1 + 1;
    if (pProgress)
        pProgress->SetStateOnPercent(rProgress);
}

void ScTable::FillSeriesSimple(
    const ScCellValue& rSrcCell, SCCOLROW& rInner, SCCOLROW nIMin, SCCOLROW nIMax,
    const SCCOLROW& rCol, const SCCOLROW& rRow, bool bVertical, ScProgress* pProgress, sal_uInt64& rProgress )
{
    bool bHidden = false;
    SCCOLROW nHiddenLast = -1;

    if (bVertical)
    {
        switch (rSrcCell.getType())
        {
            case CELLTYPE_FORMULA:
            {
                FillFormulaVertical(
                    *rSrcCell.getFormula(), rInner, rCol, nIMin, nIMax, pProgress, rProgress);
            }
            break;
            default:
            {
                for (rInner = nIMin; rInner <= nIMax; ++rInner)
                {
                    if (rInner > nHiddenLast)
                        bHidden = HiddenRowColumn(this, rInner, bVertical, nHiddenLast);

                    if (bHidden)
                    {
                        rInner = nHiddenLast;
                        continue;
                    }

                    ScAddress aDestPos(rCol, rRow, nTab);
                    rSrcCell.commit(aCol[rCol], aDestPos.Row());
                }
                rProgress += nIMax - nIMin + 1;
                if (pProgress)
                    pProgress->SetStateOnPercent(rProgress);
            }
        }
    }
    else
    {
        switch (rSrcCell.getType())
        {
            case CELLTYPE_FORMULA:
            {
                for (rInner = nIMin; rInner <= nIMax; ++rInner)
                {
                    if (rInner > nHiddenLast)
                        bHidden = HiddenRowColumn(this, rInner, bVertical, nHiddenLast);

                    if (bHidden)
                        continue;

                    FillFormula(rSrcCell.getFormula(), rCol, rRow, (rInner == nIMax));
                    if (pProgress)
                        pProgress->SetStateOnPercent(++rProgress);
                }
            }
            break;
            default:
            {
                for (rInner = nIMin; rInner <= nIMax; ++rInner)
                {
                    if (rInner > nHiddenLast)
                        bHidden = HiddenRowColumn(this, rInner, bVertical, nHiddenLast);

                    if (bHidden)
                        continue;

                    ScAddress aDestPos(rCol, rRow, nTab);
                    rSrcCell.commit(aCol[rCol], aDestPos.Row());
                }
                rProgress += nIMax - nIMin + 1;
                if (pProgress)
                    pProgress->SetStateOnPercent(rProgress);
            }
        }
    }
}

void ScTable::FillAutoSimple(
    SCCOLROW nISrcStart, SCCOLROW nISrcEnd, SCCOLROW nIStart, SCCOLROW nIEnd,
    SCCOLROW& rInner, const SCCOLROW& rCol, const SCCOLROW& rRow, sal_uInt64 nActFormCnt,
    sal_uInt64 nMaxFormCnt, bool bHasFiltered, bool bVertical, bool bPositive,
    ScProgress* pProgress, sal_uInt64& rProgress )
{
    SCCOLROW nSource = nISrcStart;
    double nDelta;
    if ( nScFillModeMouseModifier & KEY_MOD1 )
        nDelta = 0.0;
    else if ( bPositive )
        nDelta = 1.0;
    else
        nDelta = -1.0;
    sal_uInt64 nFormulaCounter = nActFormCnt;
    bool bGetCell = true;
    bool bBooleanCell = false;
    bool bPercentCell = false;
    sal_uInt16 nCellDigits = 0;
    short nHeadNoneTail = 0;
    sal_Int32 nStringValue = 0;
    OUString aValue;
    ScCellValue aSrcCell;
    bool bIsOrdinalSuffix = false;

    bool bColHidden = false, bRowHidden = false;
    SCCOL nColHiddenFirst = rDocument.MaxCol();
    SCCOL nColHiddenLast = -1;
    SCROW nRowHiddenFirst = rDocument.MaxRow();
    SCROW nRowHiddenLast = -1;

    rInner = nIStart;
    while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
    {
        if (bPositive)
        {
            if (rCol > nColHiddenLast)
                bColHidden = ColHidden(rCol, nullptr, &nColHiddenLast);
            if (rRow > nRowHiddenLast)
                bRowHidden = RowHidden(rRow, nullptr, &nRowHiddenLast);
        }
        else
        {
            if (rCol < nColHiddenFirst)
                bColHidden = ColHidden(rCol, &nColHiddenFirst);
            if (rRow < nRowHiddenFirst)
                bRowHidden = RowHidden(rRow, &nRowHiddenFirst);
        }

        if (!bColHidden && !bRowHidden)
        {
            if ( bGetCell )
            {
                if (bVertical)      // rInner&:=nRow, rOuter&:=nCol
                {
                    aSrcCell = GetCellValue(rCol, nSource);
                    if (nISrcStart == nISrcEnd && aSrcCell.getType() == CELLTYPE_FORMULA)
                    {
                        FillFormulaVertical(*aSrcCell.getFormula(), rInner, rCol, nIStart, nIEnd, pProgress, rProgress);
                        return;
                    }
                    const SvNumFormatType nFormatType = rDocument.GetFormatTable()->GetType(
                                GetColumnData(rCol).GetNumberFormat( rDocument.GetNonThreadedContext(), nSource));
                    bBooleanCell = (nFormatType == SvNumFormatType::LOGICAL);
                    bPercentCell = (nFormatType == SvNumFormatType::PERCENT);

                }
                else                // rInner&:=nCol, rOuter&:=nRow
                {
                    aSrcCell = GetCellValue(nSource, rRow);
                    const SvNumFormatType nFormatType = rDocument.GetFormatTable()->GetType(
                                GetColumnData(nSource).GetNumberFormat( rDocument.GetNonThreadedContext(), rRow));
                    bBooleanCell = (nFormatType == SvNumFormatType::LOGICAL);
                    bPercentCell = (nFormatType == SvNumFormatType::PERCENT);
                }

                bGetCell = false;
                if (!aSrcCell.isEmpty())
                {
                    switch (aSrcCell.getType())
                    {
                        case CELLTYPE_STRING:
                        case CELLTYPE_EDIT:
                            if (aSrcCell.getType() == CELLTYPE_STRING)
                                aValue = aSrcCell.getSharedString()->getString();
                            else
                                aValue = ScEditUtil::GetString(*aSrcCell.getEditText(), &rDocument);
                            if ( !(nScFillModeMouseModifier & KEY_MOD1) && !bHasFiltered )
                            {
                                nCellDigits = 0;    // look at each source cell individually
                                nHeadNoneTail = lcl_DecompValueString(
                                        aValue, nStringValue, &nCellDigits );

                                bIsOrdinalSuffix = aValue ==
                                        ScGlobal::GetOrdinalSuffix(nStringValue);
                            }
                            break;
                        default:
                            {
                                // added to avoid warnings
                            }
                    }
                }
            }

            switch (aSrcCell.getType())
            {
                case CELLTYPE_VALUE:
                    {
                        double fVal;
                        if (bBooleanCell && ((fVal = aSrcCell.getDouble()) == 0.0 || fVal == 1.0))
                            aCol[rCol].SetValue(rRow, aSrcCell.getDouble());
                        else if(bPercentCell)
                            aCol[rCol].SetValue(rRow, aSrcCell.getDouble() + nDelta * 0.01); // tdf#89998 increment by 1% at a time
                        else
                            aCol[rCol].SetValue(rRow, aSrcCell.getDouble() + nDelta);
                    }
                    break;
                case CELLTYPE_STRING:
                case CELLTYPE_EDIT:
                    if ( nHeadNoneTail )
                    {
                        sal_Int32 nNextValue;
                        if (nStringValue < 0)
                            nNextValue = nStringValue - static_cast<sal_Int32>(nDelta);
                        else
                            nNextValue = nStringValue + static_cast<sal_Int32>(nDelta);

                        if ( nHeadNoneTail < 0 )
                        {
                            setSuffixCell(
                                aCol[rCol], rRow,
                                nNextValue, nCellDigits, aValue,
                                aSrcCell.getType(), bIsOrdinalSuffix);
                        }
                        else
                        {
                            OUString aStr;
                            if (nHeadNoneTail == 2 && nNextValue >= 0) // Put back the '+'
                                aStr = aValue + "+" + lcl_ValueString(nNextValue, nCellDigits);
                            else
                                aStr = aValue + lcl_ValueString(nNextValue, nCellDigits);

                            aCol[rCol].SetRawString(rRow, aStr);
                        }
                    }
                    else
                      aSrcCell.commit(aCol[rCol], rRow);

                    break;
                case CELLTYPE_FORMULA :
                    FillFormula(
                        aSrcCell.getFormula(), rCol, rRow, (rInner == nIEnd));
                    if (nFormulaCounter - nActFormCnt > nMaxFormCnt)
                        nMaxFormCnt = nFormulaCounter - nActFormCnt;
                    break;
                default:
                    {
                        // added to avoid warnings
                    }
            }

            if (nSource == nISrcEnd)
            {
                if ( nSource != nISrcStart )
                {   // More than one source cell
                    nSource = nISrcStart;
                    bGetCell = true;
                }
                if ( !(nScFillModeMouseModifier & KEY_MOD1) )
                {
                    if ( bPositive )
                        nDelta += 1.0;
                    else
                        nDelta -= 1.0;
                }
                nFormulaCounter = nActFormCnt;
            }
            else if (bPositive)
            {
                ++nSource;
                bGetCell = true;
            }
            else
            {
                --nSource;
                bGetCell = true;
            }
        }

        if (rInner == nIEnd)
            break;
        if (bPositive)
            ++rInner;
        else
            --rInner;

        //  Progress in inner loop only for expensive cells,
        //  and even then not individually for each one

        ++rProgress;
        if ( pProgress && (aSrcCell.getType() == CELLTYPE_FORMULA || aSrcCell.getType() == CELLTYPE_EDIT) )
            pProgress->SetStateOnPercent( rProgress );

    }
    if (pProgress)
        pProgress->SetStateOnPercent( rProgress );
}

namespace
{
// Target value exceeded?
inline bool isOverflow( const double& rVal, const double& rMax, const double& rStep,
        const double& rStartVal, FillCmd eFillCmd )
{
    switch (eFillCmd)
    {
        case FILL_LINEAR:
        case FILL_DATE:
            if (rStep >= 0.0)
                return rVal > rMax;
            else
                return rVal < rMax;
        case FILL_GROWTH:
            if (rStep > 0.0)
            {
                if (rStep >= 1.0)
                {
                    // Growing away from zero, including zero growth (1.0).
                    if (rVal >= 0.0)
                        return rVal > rMax;
                    else
                        return rVal < rMax;
                }
                else
                {
                    // Shrinking towards zero.
                    if (rVal >= 0.0)
                        return rVal < rMax;
                    else
                        return rVal > rMax;
                }
            }
            else if (rStep < 0.0)
            {
                // Alternating positive and negative values.
                if (rStep <= -1.0)
                {
                    // Growing away from zero, including zero growth (-1.0).
                    if (rVal >= 0.0)
                    {
                        if (rMax >= 0.0)
                            return rVal > rMax;
                        else
                            // Regard negative rMax as lower limit, which will
                            // be reached only by a negative rVal.
                            return false;
                    }
                    else
                    {
                        if (rMax <= 0.0)
                            return rVal < rMax;
                        else
                            // Regard positive rMax as upper limit, which will
                            // be reached only by a positive rVal.
                            return false;
                    }
                }
                else
                {
                    // Shrinking towards zero.
                    if (rVal >= 0.0)
                        return rVal < rMax;
                    else
                        return rVal > rMax;
                }
            }
            else // if (rStep == 0.0)
            {
                // All values become zero.
                // Corresponds with bEntireArea in FillSeries().
                if (rMax > 0.0)
                    return rMax < rStartVal;
                else if (rMax < 0.0)
                    return rStartVal < rMax;
            }
        break;
        default:
            assert(!"eFillCmd");
    }
    return false;
}
}

void ScTable::FillSeries( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uInt64 nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                    double nStepValue, const tools::Duration& rDurationStep,
                    double nMaxValue, sal_uInt16 nArgMinDigits,
                    bool bAttribs, ScProgress* pProgress,
                    bool bSkipOverlappedCells, std::vector<sal_Int32>* pNonOverlappedCellIdx )
{
    // The term 'inner' here refers to the loop in the filling direction i.e.
    // when filling vertically, the inner position is the row position whereas
    // when filling horizontally the column position becomes the inner
    // position. The term 'outer' refers to the column position when filling
    // vertically, or the row position when filling horizontally. The fill is
    // performed once in each 'outer' position e.g. when filling vertically,
    // we perform the fill once in each column.

    //  Detect direction

    bool bVertical = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP);
    bool bPositive = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_RIGHT);

    SCCOLROW nCol = 0;
    SCCOLROW nRow = 0;
    SCCOLROW& rInner = bVertical ? nRow : nCol;        // loop variables
    SCCOLROW& rOuter = bVertical ? nCol : nRow;
    SCCOLROW nOStart;
    SCCOLROW nOEnd;
    SCCOLROW nIStart;
    SCCOLROW nIEnd;
    SCCOLROW nISource;
    ScRange aFillRange;
    sal_uInt64 nFillerCount;
    std::vector<bool> aIsNonEmptyCell;

    if (bVertical)
    {
        nFillerCount = (nRow2 - nRow1) + 1;
        nFillCount += (nRow2 - nRow1);
        if (nFillCount == 0)
            return;
        nOStart = nCol1;
        nOEnd = nCol2;
        if (bPositive)
        {
            // downward fill
            nISource = nRow1; // top row of the source range.
            nIStart = nRow1 + 1; // first row where we start filling.
            nIEnd = nRow1 + nFillCount;
            aFillRange = ScRange(nCol1, nRow1 + 1, nTab, nCol2, nRow1 + nFillCount, nTab);
        }
        else
        {
            // upward fill
            nISource = nRow2;
            nIStart = nRow2 - 1;
            nIEnd = nRow2 - nFillCount;
            aFillRange = ScRange(nCol1, nRow2 -1, nTab, nCol2, nRow2 - nFillCount, nTab);
        }
    }
    else
    {
        nFillerCount = (nCol2 - nCol1) + 1;
        nFillCount += (nCol2 - nCol1);
        if (nFillCount == 0)
            return;
        nOStart = nRow1;
        nOEnd = nRow2;
        if (bPositive)
        {
            // to the right
            nISource = nCol1;
            nIStart = nCol1 + 1;
            nIEnd = nCol1 + nFillCount;
            aFillRange = ScRange(nCol1 + 1, nRow1, nTab, nCol1 + nFillCount, nRow2, nTab);
        }
        else
        {
            // to the left
            nISource = nCol2;
            nIStart = nCol2 - 1;
            nIEnd = nCol2 - nFillCount;
            aFillRange = ScRange(nCol2 - 1, nRow1, nTab, nCol2 - nFillCount, nRow2, nTab);
        }
    }

    SCCOLROW nIMin = nIStart;
    SCCOLROW nIMax = nIEnd;
    PutInOrder(nIMin,nIMax);

    const bool bIsFiltered = IsDataFiltered(aFillRange);
    bool bEntireArea = (!bIsFiltered && eFillCmd == FILL_SIMPLE);
    if (!bIsFiltered && !bEntireArea && (eFillCmd == FILL_LINEAR || eFillCmd == FILL_GROWTH)
            && (nOEnd - nOStart == 0))
    {
        // For the usual case of one col/row determine if a numeric series is
        // at least as long as the area to be filled and does not end earlier,
        // so we can treat it as entire area for performance reasons at least
        // in the vertical case.
        // This is not exact in case of merged cell fills with skipping overlapped parts, but
        // it is still a good upper estimation.
        ScCellValue aSrcCell;
        if (bVertical)
            aSrcCell = GetCellValue(static_cast<SCCOL>(nOStart), static_cast<SCROW>(nISource));
        else
            aSrcCell = GetCellValue(static_cast<SCCOL>(nISource), static_cast<SCROW>(nOStart));
        // Same logic as for the actual series.
        if (!aSrcCell.isEmpty() && (aSrcCell.getType() == CELLTYPE_VALUE || aSrcCell.getType() == CELLTYPE_FORMULA))
        {
            double nStartVal;
            if (aSrcCell.getType() == CELLTYPE_VALUE)
                nStartVal = aSrcCell.getDouble();
            else
                nStartVal = aSrcCell.getFormula()->GetValue();
            if (eFillCmd == FILL_LINEAR)
            {
                if (nStepValue == 0.0)
                    bEntireArea = (nStartVal <= nMaxValue); // fill with same value
                else if (((nMaxValue - nStartVal) / nStepValue) >= nFillCount)
                    bEntireArea = true;
            }
            else if (eFillCmd == FILL_GROWTH)
            {
                if (nStepValue == 1.0)
                    bEntireArea = (nStartVal <= nMaxValue); // fill with same value
                else if (nStepValue == -1.0)
                    bEntireArea = (fabs(nStartVal) <= fabs(nMaxValue)); // fill with alternating value
                else if (nStepValue == 0.0)
                    bEntireArea = (nStartVal == 0.0
                            || (nStartVal < 0.0 && nMaxValue >= 0.0)
                            || (nStartVal > 0.0 && nMaxValue <= 0.0));  // fill with 0.0
            }
        }
    }
    if (bEntireArea)
    {
        InsertDeleteFlags nDel = (bAttribs ? InsertDeleteFlags::AUTOFILL :
                (InsertDeleteFlags::AUTOFILL & InsertDeleteFlags::CONTENTS));
        if (bVertical)
            DeleteArea(nCol1, static_cast<SCROW>(nIMin), nCol2, static_cast<SCROW>(nIMax), nDel);
        else
            DeleteArea(static_cast<SCCOL>(nIMin), nRow1, static_cast<SCCOL>(nIMax), nRow2, nDel);
    }

    sal_uInt64 nProgress = 0;
    if (pProgress)
        nProgress = pProgress->GetState();

    // Perform the fill once per each 'outer' position i.e. one per column
    // when filling vertically.

    for (rOuter = nOStart; rOuter <= nOEnd; rOuter++)
    {
        rInner = nISource;

        CreateColumnIfNotExists(nCol);

        // Source cell value. We need to clone the value since it may be inserted repeatedly.
        ScCellValue aSrcCell = GetCellValue(nCol, static_cast<SCROW>(nRow));

        // Maybe another source cell need to be searched, if the fill is going through merged cells,
        // where overlapped parts does not contain any information, so they can be skipped.
        if (bSkipOverlappedCells)
        {
            // create a vector to make it easier to decide if a cell need to be filled, or skipped.
            aIsNonEmptyCell.resize(nFillerCount, false);

            SCCOLROW nFirstValueIdx;
            if (bPositive)
            {
                nFirstValueIdx = nISource + (*pNonOverlappedCellIdx)[0];
                for (auto i : (*pNonOverlappedCellIdx))
                    aIsNonEmptyCell[i] = true;
            }
            else
            {
                nFirstValueIdx = nISource - (nFillerCount - 1 - (*pNonOverlappedCellIdx).back());
                for (auto i : (*pNonOverlappedCellIdx))
                    aIsNonEmptyCell[nFillerCount - 1 - i] = true;
            }

            //Set the real source cell
            if (bVertical)
                aSrcCell = GetCellValue(nOStart, static_cast<SCROW>(nFirstValueIdx));
            else
                aSrcCell = GetCellValue(nFirstValueIdx, static_cast<SCROW>(nOStart));
        }

        const ScPatternAttr* pSrcPattern = aCol[nCol].GetPattern(static_cast<SCROW>(nRow));
        const ScCondFormatItem& rCondFormatItem = pSrcPattern->GetItem(ATTR_CONDITIONAL);
        const ScCondFormatIndexes& rCondFormatIndex = rCondFormatItem.GetCondFormatData();

        if (bAttribs)
        {
            if (bVertical)
            {
                // If entire area (not filtered and simple fill) use the faster
                // method, else hidden cols/rows should be skipped and series
                // fill needs to determine the end row dynamically.
                if (bEntireArea)
                {
                    SetPatternAreaCondFormat( nCol, static_cast<SCROW>(nIMin),
                            static_cast<SCROW>(nIMax), *pSrcPattern, rCondFormatIndex);
                }
                else if (eFillCmd == FILL_SIMPLE)
                {
                    assert(bIsFiltered);
                    for(SCROW nAtRow = static_cast<SCROW>(nIMin); nAtRow <= static_cast<SCROW>(nIMax); ++nAtRow)
                    {
                        if(!RowHidden(nAtRow))
                        {
                            SetPatternAreaCondFormat( nCol, nAtRow, nAtRow, *pSrcPattern, rCondFormatIndex);
                        }
                    }

                }
            }
            else if (bEntireArea || eFillCmd == FILL_SIMPLE)
            {
                for (SCCOL nAtCol = static_cast<SCCOL>(nIMin); nAtCol <= sal::static_int_cast<SCCOL>(nIMax); nAtCol++)
                {
                    if(!ColHidden(nAtCol))
                    {
                        SetPatternAreaCondFormat( nAtCol, nRow, nRow, *pSrcPattern, rCondFormatIndex);
                    }
                }
            }
        }

        if (!aSrcCell.isEmpty())
        {
            CellType eCellType = aSrcCell.getType();

            if (eFillCmd == FILL_SIMPLE)                // copy
            {
                FillSeriesSimple(aSrcCell, rInner, nIMin, nIMax, nCol, nRow, bVertical, pProgress, nProgress);
            }
            else if (eCellType == CELLTYPE_VALUE || eCellType == CELLTYPE_FORMULA)
            {
                const double nStartVal = (eCellType == CELLTYPE_VALUE ? aSrcCell.getDouble() :
                        aSrcCell.getFormula()->GetValue());
                double nVal = nStartVal;
                tools::Long nIndex = 0;

                bool bError = false;
                bool bOverflow = false;
                bool bNonEmpty = true;

                sal_uInt16 nDayOfMonth = 0;
                sal_Int32 nFillerIdx = 0;
                if (bSkipOverlappedCells && !aIsNonEmptyCell[0])
                    --nIndex;
                rInner = nIStart;
                while (true)
                {
                    if (bSkipOverlappedCells)
                    {
                        nFillerIdx = (nFillerIdx + 1) % nFillerCount;
                        bNonEmpty = aIsNonEmptyCell[nFillerIdx];
                    }

                    if(!ColHidden(nCol) && !RowHidden(nRow))
                    {
                        if (!bError && bNonEmpty)
                        {
                            switch (eFillCmd)
                            {
                                case FILL_LINEAR:
                                    {
                                        //  use multiplication instead of repeated addition
                                        //  to avoid accumulating rounding errors
                                        nVal = nStartVal;
                                        if (rDurationStep)
                                        {
                                            tools::Duration aDuration( rDurationStep.Mult( ++nIndex, bError));
                                            bError |= !SubTotal::SafePlus( nVal, aDuration.GetInDays());
                                        }
                                        else
                                        {
                                            double nAdd = nStepValue;
                                            if ( !SubTotal::SafeMult( nAdd, static_cast<double>(++nIndex) ) ||
                                                    !SubTotal::SafePlus( nVal, nAdd ) )
                                                bError = true;
                                        }
                                    }
                                    break;
                                case FILL_GROWTH:
                                    if (!SubTotal::SafeMult(nVal, nStepValue))
                                        bError = true;
                                    break;
                                case FILL_DATE:
                                    if (fabs(nVal) > D_MAX_LONG_)
                                        bError = true;
                                    else
                                        IncDate(nVal, nDayOfMonth, nStepValue, eFillDateCmd);
                                    break;
                                default:
                                    {
                                        // added to avoid warnings
                                    }
                            }

                            if (!bError)
                                bOverflow = isOverflow( nVal, nMaxValue, nStepValue, nStartVal, eFillCmd);
                        }

                        CreateColumnIfNotExists(nCol);
                        if (bError)
                            aCol[nCol].SetError(static_cast<SCROW>(nRow), FormulaError::NoValue);
                        else if (!bOverflow && bNonEmpty)
                            aCol[nCol].SetValue(static_cast<SCROW>(nRow), nVal);

                        if (bAttribs && !bEntireArea && !bOverflow)
                            SetPatternAreaCondFormat( nCol, nRow, nRow, *pSrcPattern, rCondFormatIndex);
                    }

                    if (rInner == nIEnd || bOverflow)
                        break;
                    if (bPositive)
                    {
                        ++rInner;
                    }
                    else
                    {
                        --rInner;
                    }
                }
                nProgress += nIMax - nIMin + 1;
                if(pProgress)
                    pProgress->SetStateOnPercent( nProgress );
            }
            else if (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT)
            {
                if ( nStepValue >= 0 )
                {
                    if ( nMaxValue >= double(LONG_MAX) )
                        nMaxValue = double(LONG_MAX) - 1;
                }
                else
                {
                    if ( nMaxValue <= double(LONG_MIN) )
                        nMaxValue = double(LONG_MIN) + 1;
                }
                OUString aValue;
                if (eCellType == CELLTYPE_STRING)
                    aValue = aSrcCell.getSharedString()->getString();
                else
                    aValue = ScEditUtil::GetString(*aSrcCell.getEditText(), &rDocument);
                sal_Int32 nStringValue;
                sal_uInt16 nMinDigits = nArgMinDigits;
                short nHeadNoneTail = lcl_DecompValueString( aValue, nStringValue, &nMinDigits );
                if ( nHeadNoneTail )
                {
                    const double nStartVal = static_cast<double>(nStringValue);
                    double nVal = nStartVal;
                    tools::Long nIndex = 0;
                    bool bError = false;
                    bool bOverflow = false;
                    bool bNonEmpty = true;

                    bool bIsOrdinalSuffix = aValue == ScGlobal::GetOrdinalSuffix(
                                static_cast<sal_Int32>(nStartVal));

                    sal_Int32 nFillerIdx = 0;
                    if (bSkipOverlappedCells && !aIsNonEmptyCell[0])
                        --nIndex;
                    rInner = nIStart;
                    while (true)
                    {
                        if (bSkipOverlappedCells)
                        {
                            nFillerIdx = (nFillerIdx + 1) % nFillerCount;
                            bNonEmpty = aIsNonEmptyCell[nFillerIdx];
                        }
                        if(!ColHidden(nCol) && !RowHidden(nRow))
                        {
                            if (!bError && bNonEmpty)
                            {
                                switch (eFillCmd)
                                {
                                    case FILL_LINEAR:
                                        {
                                            //  use multiplication instead of repeated addition
                                            //  to avoid accumulating rounding errors
                                            nVal = nStartVal;
                                            if (rDurationStep)
                                            {
                                                tools::Duration aDuration( rDurationStep.Mult( ++nIndex, bError));
                                                bError |= !SubTotal::SafePlus( nVal, aDuration.GetInDays());
                                            }
                                            else
                                            {
                                                double nAdd = nStepValue;
                                                if ( !SubTotal::SafeMult( nAdd, static_cast<double>(++nIndex) ) ||
                                                        !SubTotal::SafePlus( nVal, nAdd ) )
                                                    bError = true;
                                            }
                                        }
                                        break;
                                    case FILL_GROWTH:
                                        if (!SubTotal::SafeMult(nVal, nStepValue))
                                            bError = true;
                                        break;
                                    default:
                                        {
                                            // added to avoid warnings
                                        }
                                }

                                if (!bError)
                                    bOverflow = isOverflow( nVal, nMaxValue, nStepValue, nStartVal, eFillCmd);
                            }

                            if (bError)
                                aCol[nCol].SetError(static_cast<SCROW>(nRow), FormulaError::NoValue);
                            else if (!bOverflow && bNonEmpty)
                            {
                                nStringValue = static_cast<sal_Int32>(nVal);
                                if ( nHeadNoneTail < 0 )
                                {
                                    setSuffixCell(
                                        aCol[nCol], static_cast<SCROW>(nRow),
                                        nStringValue, nMinDigits, aValue,
                                        eCellType, bIsOrdinalSuffix);
                                }
                                else
                                {
                                    OUString aStr;
                                    if (nHeadNoneTail == 2 && nStringValue >= 0) // Put back the '+'
                                        aStr = aValue + "+";
                                    else
                                        aStr = aValue;
                                    aStr += lcl_ValueString( nStringValue, nMinDigits );
                                    aCol[nCol].SetRawString(static_cast<SCROW>(nRow), aStr);
                                }
                            }

                            if (bAttribs && !bEntireArea && !bOverflow)
                                SetPatternAreaCondFormat( nCol, nRow, nRow, *pSrcPattern, rCondFormatIndex);
                        }

                        if (rInner == nIEnd || bOverflow)
                            break;
                        if (bPositive)
                            ++rInner;
                        else
                            --rInner;
                    }
                }
                if(pProgress)
                {
                    nProgress += nIMax - nIMin + 1;
                    pProgress->SetStateOnPercent( nProgress );
                }
            }
        }
        else if(pProgress)
        {
            nProgress += nIMax - nIMin + 1;
            pProgress->SetStateOnPercent( nProgress );
        }
    }
}

void ScTable::Fill( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uInt64 nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                    double nStepValue, const tools::Duration& rDurationStep,
                    double nMaxValue, ScProgress* pProgress)
{
    if (eFillCmd == FILL_AUTO)
        FillAuto(nCol1, nRow1, nCol2, nRow2, nFillCount, eFillDir, pProgress);
    else
        FillSeries(nCol1, nRow1, nCol2, nRow2, nFillCount, eFillDir,
                    eFillCmd, eFillDateCmd, nStepValue, rDurationStep, nMaxValue, 0, true, pProgress);
}

void ScTable::AutoFormatArea(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                const ScPatternAttr& rAttr, sal_uInt16 nFormatNo)
{
    ScAutoFormat& rFormat = *ScGlobal::GetOrCreateAutoFormat();
    ScAutoFormatData* pData = rFormat.findByIndex(nFormatNo);
    if (pData)
    {
        ApplyPatternArea(nStartCol, nStartRow, nEndCol, nEndRow, rAttr);
    }
}

void ScTable::AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            sal_uInt16 nFormatNo )
{
    if (!(ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow)))
        return;

    ScAutoFormat& rFormat = *ScGlobal::GetOrCreateAutoFormat();
    ScAutoFormatData* pData = rFormat.findByIndex(nFormatNo);
    if (!pData)
        return;

    std::unique_ptr<ScPatternAttr> pPatternAttrs[16];
    for (sal_uInt8 i = 0; i < 16; ++i)
    {
        pPatternAttrs[i].reset(new ScPatternAttr(rDocument.getCellAttributeHelper()));
        pData->FillToItemSet(i, pPatternAttrs[i]->GetItemSet(), rDocument);
    }

    SCCOL nCol = nStartCol;
    SCROW nRow = nStartRow;
    sal_uInt16 nIndex = 0;
    // Left top corner
    AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
    // Left column
    if (pData->HasSameData(4, 8))
        AutoFormatArea(nStartCol, nStartRow + 1, nStartCol, nEndRow - 1, *pPatternAttrs[4], nFormatNo);
    else
    {
        nIndex = 4;
        for (nRow = nStartRow + 1; nRow < nEndRow; nRow++)
        {
            AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
            if (nIndex == 4)
                nIndex = 8;
            else
                nIndex = 4;
        }
    }
    // Left bottom corner
    nRow = nEndRow;
    nIndex = 12;
    AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
    // Right top corner
    nCol = nEndCol;
    nRow = nStartRow;
    nIndex = 3;
    AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
    // Right column
    if (pData->HasSameData(7, 11))
        AutoFormatArea(nEndCol, nStartRow + 1, nEndCol, nEndRow - 1, *pPatternAttrs[7], nFormatNo);
    else
    {
        nIndex = 7;
        for (nRow = nStartRow + 1; nRow < nEndRow; nRow++)
        {
            AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
            if (nIndex == 7)
                nIndex = 11;
            else
                nIndex = 7;
        }
    }
    // Right bottom corner
    nRow = nEndRow;
    nIndex = 15;
    AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
    nRow = nStartRow;
    nIndex = 1;
    for (nCol = nStartCol + 1; nCol < nEndCol; nCol++)
    {
        AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
        if (nIndex == 1)
            nIndex = 2;
        else
            nIndex = 1;
    }
    // Bottom row
    nRow = nEndRow;
    nIndex = 13;
    for (nCol = nStartCol + 1; nCol < nEndCol; nCol++)
    {
        AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
        if (nIndex == 13)
            nIndex = 14;
        else
            nIndex = 13;
    }
    // Body
    if ((pData->HasSameData(5, 6)) && (pData->HasSameData(9, 10)) && (pData->HasSameData(5, 9)))
        AutoFormatArea(nStartCol + 1, nStartRow + 1, nEndCol-1, nEndRow - 1, *pPatternAttrs[5], nFormatNo);
    else
    {
        if ((pData->HasSameData(5, 9)) && (pData->HasSameData(6, 10)))
        {
            nIndex = 5;
            for (nCol = nStartCol + 1; nCol < nEndCol; nCol++)
            {
                AutoFormatArea(nCol, nStartRow + 1, nCol, nEndRow - 1, *pPatternAttrs[nIndex], nFormatNo);
                if (nIndex == 5)
                    nIndex = 6;
                else
                    nIndex = 5;
            }
        }
        else
        {
            nIndex = 5;
            for (nCol = nStartCol + 1; nCol < nEndCol; nCol++)
            {
                for (nRow = nStartRow + 1; nRow < nEndRow; nRow++)
                {
                    AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
                    if ((nIndex == 5) || (nIndex == 9))
                    {
                        if (nIndex == 5)
                            nIndex = 9;
                        else
                            nIndex = 5;
                    }
                    else
                    {
                        if (nIndex == 6)
                            nIndex = 10;
                        else
                            nIndex = 6;
                    }
                } // for nRow
                if ((nIndex == 5) || (nIndex == 9))
                    nIndex = 6;
                else
                    nIndex = 5;
            } // for nCol
        } // if not equal Column
    } // if not all equal
}

void ScTable::GetAutoFormatAttr(SCCOL nCol, SCROW nRow, sal_uInt16 nIndex, ScAutoFormatData& rData)
{
    sal_uInt32 nFormatIndex = GetNumberFormat( nCol, nRow );
    ScNumFormatAbbrev   aNumFormat( nFormatIndex, *rDocument.GetFormatTable() );
    rData.GetFromItemSet( nIndex, GetPattern( nCol, nRow )->GetItemSet(), aNumFormat );
}

#define LF_LEFT         1
#define LF_TOP          2
#define LF_RIGHT        4
#define LF_BOTTOM       8
#define LF_ALL          (LF_LEFT | LF_TOP | LF_RIGHT | LF_BOTTOM)

void ScTable::GetAutoFormatFrame(SCCOL nCol, SCROW nRow, sal_uInt16 nFlags, sal_uInt16 nIndex, ScAutoFormatData& rData)
{
    const SvxBoxItem* pTheBox = GetAttr(nCol, nRow, ATTR_BORDER);
    const SvxBoxItem* pLeftBox = GetAttr(nCol - 1, nRow, ATTR_BORDER);
    const SvxBoxItem* pTopBox = GetAttr(nCol, nRow - 1, ATTR_BORDER);
    const SvxBoxItem* pRightBox = GetAttr(nCol + 1, nRow, ATTR_BORDER);
    const SvxBoxItem* pBottomBox = GetAttr(nCol, nRow + 1, ATTR_BORDER);

    SvxBoxItem aBox( ATTR_BORDER );
    if (nFlags & LF_LEFT)
    {
        if (pLeftBox)
        {
            if (ScHasPriority(pTheBox->GetLeft(), pLeftBox->GetRight()))
                aBox.SetLine(pTheBox->GetLeft(), SvxBoxItemLine::LEFT);
            else
                aBox.SetLine(pLeftBox->GetRight(), SvxBoxItemLine::LEFT);
        }
        else
            aBox.SetLine(pTheBox->GetLeft(), SvxBoxItemLine::LEFT);
    }
    if (nFlags & LF_TOP)
    {
        if (pTopBox)
        {
            if (ScHasPriority(pTheBox->GetTop(), pTopBox->GetBottom()))
                aBox.SetLine(pTheBox->GetTop(), SvxBoxItemLine::TOP);
            else
                aBox.SetLine(pTopBox->GetBottom(), SvxBoxItemLine::TOP);
        }
        else
            aBox.SetLine(pTheBox->GetTop(), SvxBoxItemLine::TOP);
    }
    if (nFlags & LF_RIGHT)
    {
        if (pRightBox)
        {
            if (ScHasPriority(pTheBox->GetRight(), pRightBox->GetLeft()))
                aBox.SetLine(pTheBox->GetRight(), SvxBoxItemLine::RIGHT);
            else
                aBox.SetLine(pRightBox->GetLeft(), SvxBoxItemLine::RIGHT);
        }
        else
            aBox.SetLine(pTheBox->GetRight(), SvxBoxItemLine::RIGHT);
    }
    if (nFlags & LF_BOTTOM)
    {
        if (pBottomBox)
        {
            if (ScHasPriority(pTheBox->GetBottom(), pBottomBox->GetTop()))
                aBox.SetLine(pTheBox->GetBottom(), SvxBoxItemLine::BOTTOM);
            else
                aBox.SetLine(pBottomBox->GetTop(), SvxBoxItemLine::BOTTOM);
        }
        else
            aBox.SetLine(pTheBox->GetBottom(), SvxBoxItemLine::BOTTOM);
    }
    rData.PutItem( nIndex, aBox );
}

void ScTable::GetAutoFormatData(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, ScAutoFormatData& rData)
{
    if (!(ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow)))
        return;

    if ((nEndCol - nStartCol < 3) || (nEndRow - nStartRow < 3))
        return;

    // Left top corner
    GetAutoFormatAttr(nStartCol, nStartRow, 0, rData);
    GetAutoFormatFrame(nStartCol, nStartRow, LF_ALL, 0, rData);
    // Left column
    GetAutoFormatAttr(nStartCol, nStartRow + 1, 4, rData);
    GetAutoFormatAttr(nStartCol, nStartRow + 2, 8, rData);
    GetAutoFormatFrame(nStartCol, nStartRow + 1, LF_LEFT | LF_RIGHT | LF_BOTTOM, 4, rData);
    if (nEndRow - nStartRow >= 4)
        GetAutoFormatFrame(nStartCol, nStartRow + 2, LF_LEFT | LF_RIGHT | LF_BOTTOM, 8, rData);
    else
        rData.CopyItem( 8, 4, ATTR_BORDER );
    // Left bottom corner
    GetAutoFormatAttr(nStartCol, nEndRow, 12, rData);
    GetAutoFormatFrame(nStartCol, nEndRow, LF_ALL, 12, rData);
    // Right top corner
    GetAutoFormatAttr(nEndCol, nStartRow, 3, rData);
    GetAutoFormatFrame(nEndCol, nStartRow, LF_ALL, 3, rData);
    // Right column
    GetAutoFormatAttr(nEndCol, nStartRow + 1, 7, rData);
    GetAutoFormatAttr(nEndCol, nStartRow + 2, 11, rData);
    GetAutoFormatFrame(nEndCol, nStartRow + 1, LF_LEFT | LF_RIGHT | LF_BOTTOM, 7, rData);
    if (nEndRow - nStartRow >= 4)
        GetAutoFormatFrame(nEndCol, nStartRow + 2, LF_LEFT | LF_RIGHT | LF_BOTTOM, 11, rData);
    else
        rData.CopyItem( 11, 7, ATTR_BORDER );
    // Right bottom corner
    GetAutoFormatAttr(nEndCol, nEndRow, 15, rData);
    GetAutoFormatFrame(nEndCol, nEndRow, LF_ALL, 15, rData);
    // Top row
    GetAutoFormatAttr(nStartCol + 1, nStartRow, 1, rData);
    GetAutoFormatAttr(nStartCol + 2, nStartRow, 2, rData);
    GetAutoFormatFrame(nStartCol + 1, nStartRow, LF_TOP | LF_BOTTOM | LF_RIGHT, 1, rData);
    if (nEndCol - nStartCol >= 4)
        GetAutoFormatFrame(nStartCol + 2, nStartRow, LF_TOP | LF_BOTTOM | LF_RIGHT, 2, rData);
    else
        rData.CopyItem( 2, 1, ATTR_BORDER );
    // Bottom row
    GetAutoFormatAttr(nStartCol + 1, nEndRow, 13, rData);
    GetAutoFormatAttr(nStartCol + 2, nEndRow, 14, rData);
    GetAutoFormatFrame(nStartCol + 1, nEndRow, LF_TOP | LF_BOTTOM | LF_RIGHT, 13, rData);
    if (nEndCol - nStartCol >= 4)
        GetAutoFormatFrame(nStartCol + 2, nEndRow, LF_TOP | LF_BOTTOM | LF_RIGHT, 14, rData);
    else
        rData.CopyItem( 14, 13, ATTR_BORDER );
    // Body
    GetAutoFormatAttr(nStartCol + 1, nStartRow + 1, 5, rData);
    GetAutoFormatAttr(nStartCol + 2, nStartRow + 1, 6, rData);
    GetAutoFormatAttr(nStartCol + 1, nStartRow + 2, 9, rData);
    GetAutoFormatAttr(nStartCol + 2, nStartRow + 2, 10, rData);
    GetAutoFormatFrame(nStartCol + 1, nStartRow + 1, LF_RIGHT | LF_BOTTOM, 5, rData);
    if ((nEndCol - nStartCol >= 4) && (nEndRow - nStartRow >= 4))
    {
        GetAutoFormatFrame(nStartCol + 2, nStartRow + 1, LF_RIGHT | LF_BOTTOM, 6, rData);
        GetAutoFormatFrame(nStartCol + 1, nStartRow + 2, LF_RIGHT | LF_BOTTOM, 9, rData);
        GetAutoFormatFrame(nStartCol + 2, nStartRow + 2, LF_RIGHT | LF_BOTTOM, 10, rData);
    }
    else
    {
        rData.CopyItem( 6, 5, ATTR_BORDER );
        rData.CopyItem( 9, 5, ATTR_BORDER );
        rData.CopyItem( 10, 5, ATTR_BORDER );
    }
}

void ScTable::SetError( SCCOL nCol, SCROW nRow, FormulaError nError)
{
    if (ValidColRow(nCol, nRow))
        CreateColumnIfNotExists(nCol).SetError(nRow, nError);
}

void ScTable::UpdateInsertTabAbs(SCTAB nTable)
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].UpdateInsertTabAbs(nTable);
}

bool ScTable::GetNextSpellingCell(SCCOL& rCol, SCROW& rRow, bool bInSel,
                                    const ScMarkData& rMark) const
{
    if (rRow == rDocument.MaxRow()+2)                       // end of table
    {
        rRow = 0;
        rCol = 0;
    }
    else
    {
        rRow++;
        if (rRow == rDocument.MaxRow()+1)
        {
            rCol++;
            rRow = 0;
        }
    }
    if (rCol == rDocument.MaxCol()+1)
        return true;
    for (;;)
    {
        if (!ValidCol(rCol))
            return true;
        if (rCol >= GetAllocatedColumnsCount())
            return true;
        if (aCol[rCol].GetNextSpellingCell(rRow, bInSel, rMark))
            return true;
         /*else (rRow == rDocument.MaxRow()+1) */
        rCol++;
        rRow = 0;
    }
}

void ScTable::TestTabRefAbs(SCTAB nTable) const
{
    for (SCCOL i=0; i < aCol.size(); i++)
        if (aCol[i].TestTabRefAbs(nTable))
            return;
}

void ScTable::CompileDBFormula( sc::CompileFormulaContext& rCxt )
{
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].CompileDBFormula(rCxt);
}

void ScTable::CompileColRowNameFormula( sc::CompileFormulaContext& rCxt )
{
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].CompileColRowNameFormula(rCxt);
}

SCSIZE ScTable::GetPatternCount( SCCOL nCol ) const
{
    if( ValidCol( nCol ) )
        return aCol[nCol].GetPatternCount();
    else
        return 0;
}

SCSIZE ScTable::GetPatternCount( SCCOL nCol, SCROW nRow1, SCROW nRow2 ) const
{
    if( ValidCol( nCol ) && ValidRow( nRow1 ) && ValidRow( nRow2 ) )
        return aCol[nCol].GetPatternCount( nRow1, nRow2 );
    else
        return 0;
}

bool ScTable::ReservePatternCount( SCCOL nCol, SCSIZE nReserve )
{
    if( ValidCol( nCol ) )
        return aCol[nCol].ReservePatternCount( nReserve );
    else
        return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
