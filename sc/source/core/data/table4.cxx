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

#include "scitems.hxx"
#include <comphelper/string.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/escapementitem.hxx>
#include <svl/zforlist.hxx>
#include <vcl/keycodes.hxx>
#include <rtl/math.hxx>
#include <unotools/charclass.hxx>

#include "attrib.hxx"
#include "patattr.hxx"
#include "formulacell.hxx"
#include "table.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "document.hxx"
#include "autoform.hxx"
#include "userlist.hxx"
#include "zforauto.hxx"
#include "subtotal.hxx"
#include "formula/errorcodes.hxx"
#include "rangenam.hxx"
#include "docpool.hxx"
#include "progress.hxx"
#include "segmenttree.hxx"
#include "conditio.hxx"
#include "editutil.hxx"

#include <math.h>
#include <boost/scoped_ptr.hpp>

// STATIC DATA -----------------------------------------------------------

#define _D_MAX_LONG_  (double) 0x7fffffff

extern sal_uInt16 nScFillModeMouseModifier;     // global.cxx

namespace {

short lcl_DecompValueString( String& aValue, sal_Int32& nVal, sal_uInt16* pMinDigits = NULL )
{
    if ( !aValue.Len() )
    {
        nVal = 0;
        return 0;
    }
    const sal_Unicode* p = aValue.GetBuffer();
    xub_StrLen nNeg = 0;
    xub_StrLen nNum = 0;
    if ( p[nNum] == '-' )
        nNum = nNeg = 1;
    while ( p[nNum] && CharClass::isAsciiNumeric( OUString(p[nNum]) ) )
        nNum++;

    sal_Unicode cNext = p[nNum];            // 0 if at the end
    sal_Unicode cLast = p[aValue.Len()-1];

    // #i5550# If there are numbers at the beginning and the end,
    // prefer the one at the beginning only if it's followed by a space.
    // Otherwise, use the number at the end, to enable things like IP addresses.
    if ( nNum > nNeg && ( cNext == 0 || cNext == ' ' || !CharClass::isAsciiNumeric(OUString(cLast)) ) )
    {   // number at the beginning
        nVal = aValue.Copy( 0, nNum ).ToInt32();
        //  any number with a leading zero sets the minimum number of digits
        if ( p[nNeg] == '0' && pMinDigits && ( nNum - nNeg > *pMinDigits ) )
            *pMinDigits = nNum - nNeg;
        aValue.Erase( 0, nNum );
        return -1;
    }
    else
    {
        nNeg = 0;
        xub_StrLen nEnd = nNum = aValue.Len() - 1;
        while ( nNum && CharClass::isAsciiNumeric( OUString(p[nNum]) ) )
            nNum--;
        if ( p[nNum] == '-' )
        {
            nNum--;
            nNeg = 1;
        }
        if ( nNum < nEnd - nNeg )
        {   // number at the end
            nVal = aValue.Copy( nNum + 1 ).ToInt32();
            //  any number with a leading zero sets the minimum number of digits
            if ( p[nNum+1+nNeg] == '0' && pMinDigits && ( nEnd - nNum - nNeg > *pMinDigits ) )
                *pMinDigits = nEnd - nNum - nNeg;
            aValue.Erase( nNum + 1 );
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
            OUStringBuffer aZero;
            comphelper::string::padToLength(aZero, nMinDigits - aStr.getLength(), '0');
            aStr = aZero.makeStringAndClear() + aStr;
        }
        //  nMinDigits doesn't include the '-' sign -> add after inserting zeros
        if ( nValue < 0 )
            aStr = "-" + aStr;
        return aStr;
    }
}

void setSuffixCell(
    ScColumn& rColumn, SCROW nRow, sal_Int32 nValue, sal_uInt16 nDigits, const OUString& rSuffix,
    CellType eCellType, bool bIsOrdinalSuffix )
{
    ScDocument& rDoc = rColumn.GetDoc();
    OUString aValue = lcl_ValueString(nValue, nDigits);
    if (!bIsOrdinalSuffix)
    {
        rColumn.SetRawString(nRow, aValue += rSuffix);
        return;
    }

    OUString aOrdinalSuffix = ScGlobal::GetOrdinalSuffix(nValue);
    if (eCellType != CELLTYPE_EDIT)
    {
        rColumn.SetRawString(nRow, aValue += aOrdinalSuffix);
        return;
    }

    EditEngine aEngine(rDoc.GetEnginePool());
    aEngine.SetEditTextObjectPool(rDoc.GetEditPool());

    SfxItemSet aAttr = aEngine.GetEmptyItemSet();
    aAttr.Put( SvxEscapementItem( SVX_ESCAPEMENT_SUPERSCRIPT, EE_CHAR_ESCAPEMENT));
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

void ScTable::FillAnalyse( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            FillCmd& rCmd, FillDateCmd& rDateCmd,
                            double& rInc, sal_uInt16& rMinDigits,
                            ScUserListData*& rListData, sal_uInt16& rListIndex)
{
    OSL_ENSURE( nCol1==nCol2 || nRow1==nRow2, "FillAnalyse: invalid range" );

    rInc = 0.0;
    rMinDigits = 0;
    rListData = NULL;
    rCmd = FILL_SIMPLE;
    if ( (nScFillModeMouseModifier & KEY_MOD1) )
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

    SCCOL nCol = nCol1;
    SCROW nRow = nRow1;

    ScRefCellValue aFirstCell = GetCellValue(nCol, nRow);
    CellType eCellType = aFirstCell.meType;

    if (eCellType == CELLTYPE_VALUE)
    {
        sal_uInt32 nFormat = ((const SfxUInt32Item*)GetAttr(nCol,nRow,ATTR_VALUE_FORMAT))->GetValue();
        bool bDate = ( pDocument->GetFormatTable()->GetType(nFormat) == NUMBERFORMAT_DATE );
        if (bDate)
        {
            if (nCount > 1)
            {
                double nVal;
                Date aNullDate = *pDocument->GetFormatTable()->GetNullDate();
                Date aDate1 = aNullDate;
                nVal = aFirstCell.mfValue;
                aDate1 += (long)nVal;
                Date aDate2 = aNullDate;
                nVal = GetValue(nCol+nAddX, nRow+nAddY);
                aDate2 += (long)nVal;
                if ( aDate1 != aDate2 )
                {
                    long nCmpInc = 0;
                    FillDateCmd eType;
                    long nDDiff = aDate2.GetDay()   - (long) aDate1.GetDay();
                    long nMDiff = aDate2.GetMonth() - (long) aDate1.GetMonth();
                    long nYDiff = aDate2.GetYear()  - (long) aDate1.GetYear();
                    if ( nDDiff )
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
                    for (sal_uInt16 i=1; i<nCount && bVal; i++)
                    {
                        ScRefCellValue aCell = GetCellValue(nCol,nRow);
                        if (aCell.meType == CELLTYPE_VALUE)
                        {
                            nVal = aCell.mfValue;
                            aDate2 = aNullDate + (long) nVal;
                            if ( eType == FILL_DAY )
                            {
                                if ( aDate2-aDate1 != nCmpInc )
                                    bVal = false;
                            }
                            else
                            {
                                nDDiff = aDate2.GetDay()   - (long) aDate1.GetDay();
                                nMDiff = aDate2.GetMonth() - (long) aDate1.GetMonth();
                                nYDiff = aDate2.GetYear()  - (long) aDate1.GetYear();
                                if (nDDiff || ( nMDiff + 12 * nYDiff != nCmpInc ))
                                    bVal = false;
                            }
                            aDate1 = aDate2;
                            nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                            nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                        }
                        else
                            bVal = false;   // kein Datum passt auch nicht
                    }
                    if (bVal)
                    {
                        if ( eType == FILL_MONTH && ( nCmpInc % 12 == 0 ) )
                        {
                            eType = FILL_YEAR;
                            nCmpInc /= 12;
                        }
                        rCmd = FILL_DATE;
                        rDateCmd = eType;
                        rInc = nCmpInc;
                    }
                }
            }
            else                            // single date -> increment by days
            {
                rCmd = FILL_DATE;
                rDateCmd = FILL_DAY;
                rInc = 1.0;
            }
        }
        else
        {
            if (nCount > 1)
            {
                double nVal1 = aFirstCell.mfValue;
                double nVal2 = GetValue(nCol+nAddX, nRow+nAddY);
                rInc = nVal2 - nVal1;
                nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                bool bVal = true;
                for (sal_uInt16 i=1; i<nCount && bVal; i++)
                {
                    ScRefCellValue aCell = GetCellValue(nCol,nRow);
                    if (aCell.meType == CELLTYPE_VALUE)
                    {
                        nVal2 = aCell.mfValue;
                        double nDiff = nVal2 - nVal1;
                        if ( !::rtl::math::approxEqual( nDiff, rInc, 13 ) )
                            bVal = false;
                        nVal1 = nVal2;
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
    else if (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT)
    {
        OUString aStr;
        GetString(nCol, nRow, aStr);

        // fdo#39500 don't deduce increment from multiple equal list entries
        bool bAllSame = true;
        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            OUString aTestStr;
            GetString(static_cast<SCCOL>(nCol + i* nAddX), static_cast<SCROW>(nRow + i * nAddY), aTestStr);
            if(aStr != aTestStr)
            {
                bAllSame = false;
                break;
            }
        }
        if(bAllSame && nCount > 1)
            return;

        rListData = (ScUserListData*)(ScGlobal::GetUserList()->GetData(aStr));
        if (rListData)
        {
            rListData->GetSubIndex(aStr, rListIndex);
            nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
            nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
            for (sal_uInt16 i=1; i<nCount && rListData; i++)
            {
                GetString(nCol, nRow, aStr);
                if (!rListData->GetSubIndex(aStr, rListIndex))
                    rListData = NULL;
                nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
            }
        }
        else if ( nCount > 1 )
        {
            //  pass rMinDigits to all DecompValueString calls
            //  -> longest number defines rMinDigits

            sal_Int32 nVal1;
            String aString = aStr;
            short nFlag1 = lcl_DecompValueString( aString, nVal1, &rMinDigits );
            aStr = aString;
            if ( nFlag1 )
            {
                sal_Int32 nVal2;
                GetString( nCol+nAddX, nRow+nAddY, aStr );
                aString = aStr;
                short nFlag2 = lcl_DecompValueString( aString, nVal2, &rMinDigits );
                aStr = aString;
                if ( nFlag1 == nFlag2 )
                {
                    rInc = (double)nVal2 - (double)nVal1;
                    nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                    nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                    bool bVal = true;
                    for (sal_uInt16 i=1; i<nCount && bVal; i++)
                    {
                        ScRefCellValue aCell = GetCellValue(nCol, nRow);
                        CellType eType = aCell.meType;
                        if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
                        {
                            aStr = aCell.getString(pDocument);
                            aString = aStr;
                            nFlag2 = lcl_DecompValueString( aString, nVal2, &rMinDigits );
                            aStr = aString;
                            if ( nFlag1 == nFlag2 )
                            {
                                double nDiff = (double)nVal2 - (double)nVal1;
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
            String aString = aStr;
            lcl_DecompValueString( aString, nDummy, &rMinDigits );
        }
    }
}

void ScTable::FillFormula(sal_uLong& /* nFormulaCounter */, bool /* bFirst */, ScFormulaCell* pSrcCell,
                          SCCOL nDestCol, SCROW nDestRow, bool bLast )
{

    pDocument->SetNoListening( true );  // still the wrong reference
    ScAddress aAddr( nDestCol, nDestRow, nTab );
    ScFormulaCell* pDestCell = new ScFormulaCell( *pSrcCell, *pDocument, aAddr );
    aCol[nDestCol].SetFormulaCell(nDestRow, pDestCell);

    if ( bLast && pDestCell->GetMatrixFlag() )
    {
        ScAddress aOrg;
        if ( pDestCell->GetMatrixOrigin( aOrg ) )
        {
            if ( nDestCol >= aOrg.Col() && nDestRow >= aOrg.Row() )
            {
                ScFormulaCell* pOrgCell = pDocument->GetFormulaCell(aOrg);
                if (pOrgCell && pOrgCell->GetMatrixFlag() == MM_FORMULA)
                {
                    ((ScFormulaCell*)pOrgCell)->SetMatColsRows(
                        nDestCol - aOrg.Col() + 1,
                        nDestRow - aOrg.Row() + 1 );
                }
                else
                {
                    OSL_FAIL( "FillFormula: MatrixOrigin no forumla cell with MM_FORMULA" );
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
    pDocument->SetNoListening( false );
    pDestCell->StartListeningTo( pDocument );

}

void ScTable::FillAuto( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        sal_uLong nFillCount, FillDir eFillDir, ScProgress* pProgress )
{
    if ( (nFillCount == 0) || !ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2) )
        return;

    //
    //  Detect direction
    //

    bool bVertical = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP);
    bool bPositive = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_RIGHT);

    sal_uLong nCol = 0;
    sal_uLong nRow = 0;
    sal_uLong& rInner = bVertical ? nRow : nCol;        // loop variables
    sal_uLong& rOuter = bVertical ? nCol : nRow;
    sal_uLong nOStart;
    sal_uLong nOEnd;
    sal_uLong nIStart;
    sal_uLong nIEnd;
    sal_uLong nISrcStart;
    sal_uLong nISrcEnd;
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
    sal_uLong nIMin = nIStart;
    sal_uLong nIMax = nIEnd;
    PutInOrder(nIMin,nIMax);
    bool bHasFiltered = IsDataFiltered(aFillRange);

    if (!bHasFiltered)
    {
        if (bVertical)
            DeleteArea(nCol1, static_cast<SCROW>(nIMin), nCol2, static_cast<SCROW>(nIMax), IDF_AUTOFILL);
        else
            DeleteArea(static_cast<SCCOL>(nIMin), nRow1, static_cast<SCCOL>(nIMax), nRow2, IDF_AUTOFILL);
    }

    sal_uLong nProgress = 0;
    if (pProgress)
        nProgress = pProgress->GetState();

    //
    //  execute
    //

    sal_uLong nActFormCnt = 0;
    for (rOuter = nOStart; rOuter <= nOEnd; rOuter++)
    {
        sal_uLong nMaxFormCnt = 0;                      // for formulas

        //  transfer attributes

        const ScPatternAttr* pSrcPattern = NULL;
        const ScStyleSheet* pStyleSheet = NULL;
        sal_uLong nAtSrc = nISrcStart;
        ScPatternAttr* pNewPattern = NULL;
        bool bGetPattern = true;
        rInner = nIStart;
        while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
        {
            if (!ColHidden(nCol) && !RowHidden(nRow))
            {
                if ( bGetPattern )
                {
                    delete pNewPattern;
                    if (bVertical)      // rInner&:=nRow, rOuter&:=nCol
                        pSrcPattern = aCol[nCol].GetPattern(static_cast<SCROW>(nAtSrc));
                    else                // rInner&:=nCol, rOuter&:=nRow
                        pSrcPattern = aCol[nAtSrc].GetPattern(static_cast<SCROW>(nRow));
                    bGetPattern = false;
                    pStyleSheet = pSrcPattern->GetStyleSheet();
                    //  do not transfer ATTR_MERGE / ATTR_MERGE_FLAG
                    const SfxItemSet& rSet = pSrcPattern->GetItemSet();
                    if ( rSet.GetItemState(ATTR_MERGE, false) == SFX_ITEM_SET
                            || rSet.GetItemState(ATTR_MERGE_FLAG, false) == SFX_ITEM_SET )
                    {
                        pNewPattern = new ScPatternAttr( *pSrcPattern );
                        SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                        rNewSet.ClearItem(ATTR_MERGE);
                        rNewSet.ClearItem(ATTR_MERGE_FLAG);
                    }
                    else
                        pNewPattern = NULL;
                }

                const ScCondFormatItem& rCondFormatItem = static_cast<const ScCondFormatItem&>(pSrcPattern->GetItem(ATTR_CONDITIONAL));
                const std::vector<sal_uInt32>& rCondFormatIndex = rCondFormatItem.GetCondFormatData();

                if ( bVertical && nISrcStart == nISrcEnd && !bHasFiltered )
                {
                    //  set all attributes at once (en bloc)
                    if (pNewPattern || pSrcPattern != pDocument->GetDefPattern())
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

                        for(std::vector<sal_uInt32>::const_iterator itr = rCondFormatIndex.begin(), itrEnd = rCondFormatIndex.end();
                                                        itr != itrEnd; ++itr)
                        {
                            ScConditionalFormat* pCondFormat = mpCondFormatList->GetFormat(*itr);
                            ScRangeList aRange = pCondFormat->GetRange();
                            aRange.Join(ScRange(nCol, nY1, nTab, nCol, nY2, nTab));
                            pCondFormat->AddRange(aRange);
                        }
                    }


                    break;
                }

                if ( bHasFiltered )
                    DeleteArea(static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow),
                            static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), IDF_AUTOFILL);

                if ( pSrcPattern != aCol[nCol].GetPattern( static_cast<SCROW>(nRow) ) )
                {
                    // Transfer template too
                    //! Merge ApplyPattern to AttrArray ??
                    if ( pStyleSheet )
                        aCol[nCol].ApplyStyle( static_cast<SCROW>(nRow), *pStyleSheet );

                    //  Use ApplyPattern instead of SetPattern to keep old MergeFlags
                    if ( pNewPattern )
                        aCol[nCol].ApplyPattern( static_cast<SCROW>(nRow), *pNewPattern );
                    else
                        aCol[nCol].ApplyPattern( static_cast<SCROW>(nRow), *pSrcPattern );

                    for(std::vector<sal_uInt32>::const_iterator itr = rCondFormatIndex.begin(), itrEnd = rCondFormatIndex.end();
                            itr != itrEnd; ++itr)
                    {
                        ScConditionalFormat* pCondFormat = mpCondFormatList->GetFormat(*itr);
                        ScRangeList aRange = pCondFormat->GetRange();
                        aRange.Join(ScRange(nCol, nRow, nTab, nCol, nRow, nTab));
                        pCondFormat->AddRange(aRange);
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
        if ( pNewPattern )
            delete pNewPattern;

        //  Analyse

        FillCmd eFillCmd;
        FillDateCmd eDateCmd;
        double nInc;
        sal_uInt16 nMinDigits;
        ScUserListData* pListData = NULL;
        sal_uInt16 nListIndex;
        if (bVertical)
            FillAnalyse(static_cast<SCCOL>(nCol),nRow1,
                    static_cast<SCCOL>(nCol),nRow2, eFillCmd,eDateCmd,
                    nInc,nMinDigits, pListData,nListIndex);
        else
            FillAnalyse(nCol1,static_cast<SCROW>(nRow),
                    nCol2,static_cast<SCROW>(nRow), eFillCmd,eDateCmd,
                    nInc,nMinDigits, pListData,nListIndex);

        if (pListData)
        {
            sal_uInt16 nListCount = pListData->GetSubCount();
            if ( !bPositive )
            {
                //  nListIndex of FillAnalyse points to the last entry -> adjust
                sal_uLong nSub = nISrcStart - nISrcEnd;
                for (sal_uLong i=0; i<nSub; i++)
                {
                    if (nListIndex == 0) nListIndex = nListCount;
                    --nListIndex;
                }
            }

            rInner = nIStart;
            while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
            {
                if(!ColHidden(nCol) && !RowHidden(nRow))
                {
                    if (bPositive)
                    {
                        ++nListIndex;
                        if (nListIndex >= nListCount) nListIndex = 0;
                    }
                    else
                    {
                        if (nListIndex == 0) nListIndex = nListCount;
                        --nListIndex;
                    }
                    aCol[nCol].SetRawString(static_cast<SCROW>(nRow), pListData->GetSubStr(nListIndex));
                }

                if (rInner == nIEnd) break;
                if (bPositive) ++rInner; else --rInner;
            }
            if(pProgress)
            {
                nProgress += nIMax - nIMin + 1;
                pProgress->SetStateOnPercent( nProgress );
            }
        }
        else if (eFillCmd == FILL_SIMPLE)           // fill with pattern/sample
        {
            sal_uLong nSource = nISrcStart;
            double nDelta;
            if ( (nScFillModeMouseModifier & KEY_MOD1) )
                nDelta = 0.0;
            else if ( bPositive )
                nDelta = 1.0;
            else
                nDelta = -1.0;
            double nVal = 0.0;
            sal_uLong nFormulaCounter = nActFormCnt;
            bool bFirst = true;
            bool bGetCell = true;
            sal_uInt16 nCellDigits = 0;
            short nHeadNoneTail = 0;
            sal_Int32 nStringValue = 0;
            String aValue;
            ScCellValue aSrcCell;
            CellType eCellType = CELLTYPE_NONE;
            bool bIsOrdinalSuffix = false;

            rInner = nIStart;
            while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
            {
                if(!ColHidden(nCol) && !RowHidden(nRow))
                {
                    if ( bGetCell )
                    {
                        if (bVertical)      // rInner&:=nRow, rOuter&:=nCol
                            aSrcCell = aCol[nCol].GetCellValue(static_cast<SCROW>(nSource));
                        else                // rInner&:=nCol, rOuter&:=nRow
                            aSrcCell = aCol[nSource].GetCellValue(static_cast<SCROW>(nRow));

                        bGetCell = false;
                        if (!aSrcCell.isEmpty())
                        {
                            eCellType = aSrcCell.meType;
                            switch (eCellType)
                            {
                                case CELLTYPE_VALUE:
                                    nVal = aSrcCell.mfValue;
                                    break;
                                case CELLTYPE_STRING:
                                case CELLTYPE_EDIT:
                                    if ( eCellType == CELLTYPE_STRING )
                                        aValue = *aSrcCell.mpString;
                                    else
                                        aValue = ScEditUtil::GetString(*aSrcCell.mpEditText, pDocument);
                                    if ( !(nScFillModeMouseModifier & KEY_MOD1) && !bHasFiltered )
                                    {
                                        nCellDigits = 0;    // look at each source cell individually
                                        nHeadNoneTail = lcl_DecompValueString(
                                                aValue, nStringValue, &nCellDigits );

                                        bIsOrdinalSuffix = aValue.Equals(
                                                ScGlobal::GetOrdinalSuffix( nStringValue));
                                    }
                                    break;
                                default:
                                    {
                                        // added to avoid warnings
                                    }
                            }
                        }
                        else
                            eCellType = CELLTYPE_NONE;
                    }

                    switch (eCellType)
                    {
                        case CELLTYPE_VALUE:
                            aCol[nCol].SetValue(static_cast<SCROW>(nRow), nVal + nDelta);
                            break;
                        case CELLTYPE_STRING:
                        case CELLTYPE_EDIT:
                            if ( nHeadNoneTail )
                            {
                                // #i48009# with the "nStringValue+(long)nDelta" expression within the
                                // lcl_ValueString calls, gcc 3.4.1 makes wrong optimizations (ok in 3.4.3),
                                // so nNextValue is now calculated ahead.
                                sal_Int32 nNextValue = nStringValue+(sal_Int32)nDelta;

                                OUString aStr;
                                if ( nHeadNoneTail < 0 )
                                {
                                    setSuffixCell(
                                        aCol[nCol], static_cast<SCROW>(nRow),
                                        nNextValue, nCellDigits, aValue,
                                        eCellType, bIsOrdinalSuffix);
                                }
                                else
                                {
                                    aStr = aValue + lcl_ValueString( nNextValue, nCellDigits );
                                    aCol[nCol].SetRawString(static_cast<SCROW>(nRow), aStr);
                                }
                            }
                            else
                                aSrcCell.commit(aCol[nCol], nRow);

                            break;
                        case CELLTYPE_FORMULA :
                            FillFormula( nFormulaCounter, bFirst, aSrcCell.mpFormula,
                                    static_cast<SCCOL>(nCol),
                                    static_cast<SCROW>(nRow), (rInner == nIEnd) );
                            if (nFormulaCounter - nActFormCnt > nMaxFormCnt)
                                nMaxFormCnt = nFormulaCounter - nActFormCnt;
                            break;
                        default:
                            {
                                // added to avoid warnings
                            }
                    }

                    if (nSource==nISrcEnd)
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
                        bFirst = false;
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

                if (rInner == nIEnd) break;
                if (bPositive) ++rInner; else --rInner;

                //  Progress in inner loop only for expensive cells,
                //  and even then not individually for each one

                ++nProgress;
                if ( pProgress && (eCellType == CELLTYPE_FORMULA || eCellType == CELLTYPE_EDIT) )
                    pProgress->SetStateOnPercent( nProgress );

            }
            if (pProgress)
                pProgress->SetStateOnPercent( nProgress );
        }
        else
        {
            if (!bPositive)
                nInc = -nInc;
            double nEndVal = (nInc>=0.0) ? MAXDOUBLE : -MAXDOUBLE;
            if (bVertical)
                FillSeries( static_cast<SCCOL>(nCol), nRow1,
                        static_cast<SCCOL>(nCol), nRow2, nFillCount, eFillDir,
                        eFillCmd, eDateCmd, nInc, nEndVal, nMinDigits, false,
                        pProgress );
            else
                FillSeries( nCol1, static_cast<SCROW>(nRow), nCol2,
                        static_cast<SCROW>(nRow), nFillCount, eFillDir,
                        eFillCmd, eDateCmd, nInc, nEndVal, nMinDigits, false,
                        pProgress );
            if (pProgress)
                nProgress = pProgress->GetState();
        }

        nActFormCnt += nMaxFormCnt;
    }
}

String ScTable::GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY )
{
    String aValue;

    SCCOL nCol1 = rSource.aStart.Col();
    SCROW nRow1 = rSource.aStart.Row();
    SCCOL nCol2 = rSource.aEnd.Col();
    SCROW nRow2 = rSource.aEnd.Row();
    bool bOk = true;
    long nIndex = 0;
    sal_uLong nSrcCount = 0;
    FillDir eFillDir = FILL_TO_BOTTOM;
    if ( nEndX == nCol2 && nEndY == nRow2 )     // empty
        bOk = false;
    else if ( nEndX == nCol2 )                  // to up / down
    {
        nEndX = nCol2 = nCol1;                  // use only first column
        nSrcCount = nRow2 - nRow1 + 1;
        nIndex = ((long)nEndY) - nRow1;         // can be negative
        if ( nEndY >= nRow1 )
            eFillDir = FILL_TO_BOTTOM;
        else
            eFillDir = FILL_TO_TOP;
    }
    else if ( nEndY == nRow2 )                  // to left / right
    {
        nEndY = nRow2 = nRow1;                  // use only first row
        nSrcCount = nCol2 - nCol1 + 1;
        nIndex = ((long)nEndX) - nCol1;         // can be negative
        if ( nEndX >= nCol1 )
            eFillDir = FILL_TO_RIGHT;
        else
            eFillDir = FILL_TO_LEFT;
    }
    else                                        // direction not clear
        bOk = false;

    if ( bOk )
    {
        FillCmd eFillCmd;
        FillDateCmd eDateCmd;
        double nInc;
        sal_uInt16 nMinDigits;
        ScUserListData* pListData = NULL;
        sal_uInt16 nListIndex;

        FillAnalyse(nCol1,nRow1, nCol2,nRow2, eFillCmd,eDateCmd, nInc,nMinDigits, pListData,nListIndex);

        if ( pListData )                            // user defined list
        {
            sal_uInt16 nListCount = pListData->GetSubCount();
            if ( nListCount )
            {
                sal_uLong nSub = nSrcCount - 1; //  nListIndex is from last source entry
                while ( nIndex < sal::static_int_cast<long>(nSub) )
                    nIndex += nListCount;
                sal_uLong nPos = ( nListIndex + nIndex - nSub ) % nListCount;
                aValue = pListData->GetSubStr(sal::static_int_cast<sal_uInt16>(nPos));
            }
        }
        else if ( eFillCmd == FILL_SIMPLE )         // fill with pattern/sample
        {
            if ((eFillDir == FILL_TO_BOTTOM)||(eFillDir == FILL_TO_TOP))
            {
                long nBegin = 0;
                long nEnd = 0;
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

                long nNonFiltered = CountNonFilteredRows(nBegin, nEnd);
                long nFiltered = nEnd + 1 - nBegin - nNonFiltered;

                if (nIndex > 0)
                    nIndex = nIndex - nFiltered;
                else
                    nIndex = nIndex + nFiltered;
            }

            long nPosIndex = nIndex;
            while ( nPosIndex < 0 )
                nPosIndex += nSrcCount;
            sal_uLong nPos = nPosIndex % nSrcCount;
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

                CellType eType = aCell.meType;
                switch ( eType )
                {
                    case CELLTYPE_STRING:
                    case CELLTYPE_EDIT:
                    {
                        aValue = aCell.getString(pDocument);

                        if ( !(nScFillModeMouseModifier & KEY_MOD1) )
                        {
                            sal_Int32 nVal;
                            sal_uInt16 nCellDigits = 0; // look at each source cell individually
                            short nFlag = lcl_DecompValueString( aValue, nVal, &nCellDigits );
                            if ( nFlag < 0 )
                            {
                                if (aValue.Equals( ScGlobal::GetOrdinalSuffix( nVal)))
                                    aValue = ScGlobal::GetOrdinalSuffix( nVal + nDelta);

                                aValue.Insert( lcl_ValueString( nVal + nDelta, nCellDigits ), 0 );
                            }
                            else if ( nFlag > 0 )
                                aValue += lcl_ValueString( nVal + nDelta, nCellDigits );
                        }
                    }
                    break;
                    case CELLTYPE_VALUE:
                    {
                        //  overflow is possible...
                        double nVal = aCell.mfValue;
                        if ( !(nScFillModeMouseModifier & KEY_MOD1) )
                            nVal += (double) nDelta;

                        Color* pColor;
                        sal_uLong nNumFmt = GetNumberFormat( nSrcX, nSrcY );
                        OUString sTmp(aValue);
                        pDocument->GetFormatTable()->GetOutputString( nVal, nNumFmt, sTmp, &pColor );
                        aValue = sTmp;
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
                CellType eType = aCell.meType;
                switch ( eType )
                {
                    case CELLTYPE_STRING:
                    case CELLTYPE_EDIT:
                    {
                        aValue = aCell.getString(pDocument);
                        nHeadNoneTail = lcl_DecompValueString( aValue, nVal );
                        if ( nHeadNoneTail )
                            nStart = (double)nVal;
                        else
                            nStart = 0.0;
                    }
                    break;
                    case CELLTYPE_VALUE:
                        nStart = aCell.mfValue;
                    break;
                    case CELLTYPE_FORMULA:
                        nStart = aCell.mpFormula->GetValue();
                    break;
                    default:
                        nStart = 0.0;
                }
            }
            else
                nStart = 0.0;
            if ( eFillCmd == FILL_LINEAR )
            {
                double nAdd = nInc;
                bValueOk = ( SubTotal::SafeMult( nAdd, (double) nIndex ) &&
                             SubTotal::SafePlus( nStart, nAdd ) );
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
                for (long i=0; i<nIndex; i++)
                    IncDate( nStart, nDayOfMonth, nInc, eDateCmd );
            }

            if (bValueOk)
            {
                if ( nHeadNoneTail )
                {
                    if ( nHeadNoneTail < 0 )
                    {
                        if (aValue.Equals( ScGlobal::GetOrdinalSuffix( nVal)))
                            aValue = ScGlobal::GetOrdinalSuffix( (sal_Int32)nStart );

                        aValue.Insert( lcl_ValueString( (sal_Int32)nStart, nMinDigits ), 0 );
                    }
                    else
                        aValue += lcl_ValueString( (sal_Int32)nStart, nMinDigits );
                }
                else
                {
                    //! Zahlformat je nach Index holen?
                    Color* pColor;
                    sal_uLong nNumFmt = GetNumberFormat( nCol1, nRow1 );
                    OUString sTmp(aValue);
                    pDocument->GetFormatTable()->GetOutputString( nStart, nNumFmt, sTmp, &pColor );
                    aValue = sTmp;
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

    long nInc = (long) nStep;       // upper/lower limits ?
    Date aNullDate = *pDocument->GetFormatTable()->GetNullDate();
    Date aDate = aNullDate;
    aDate += (long)rVal;
    switch (eCmd)
    {
        case FILL_WEEKDAY:
            {
                aDate += nInc;
                DayOfWeek eWeekDay = aDate.GetDayOfWeek();
                if (nInc >= 0)
                {
                    if (eWeekDay == SATURDAY)
                        aDate += 2;
                    else if (eWeekDay == SUNDAY)
                        aDate += 1;
                }
                else
                {
                    if (eWeekDay == SATURDAY)
                        aDate -= 1;
                    else if (eWeekDay == SUNDAY)
                        aDate -= 2;
                }
            }
            break;
        case FILL_MONTH:
            {
                if ( nDayOfMonth == 0 )
                    nDayOfMonth = aDate.GetDay();       // init
                long nMonth = aDate.GetMonth();
                long nYear = aDate.GetYear();

                nMonth += nInc;

                if (nInc >= 0)
                {
                    if (nMonth > 12)
                    {
                        long nYAdd = (nMonth-1) / 12;
                        nMonth -= nYAdd * 12;
                        nYear += nYAdd;
                    }
                }
                else
                {
                    if (nMonth < 1)
                    {
                        long nYAdd = 1 - nMonth / 12;       // positive
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
                    aDate.SetMonth((sal_uInt16) nMonth);
                    aDate.SetYear((sal_uInt16) nYear);
                    aDate.SetDay( std::min( Date::GetDaysInMonth( nMonth, nYear), nDayOfMonth ) );
                }
            }
            break;
        case FILL_YEAR:
            {
                long nYear = aDate.GetYear();
                nYear += nInc;
                if ( nYear < nMinYear )
                    aDate = Date( 1,1, nMinYear );
                else if ( nYear > nMaxYear )
                    aDate = Date( 31,12, nMaxYear );
                else
                    aDate.SetYear((sal_uInt16) nYear);
            }
            break;
        default:
        {
            // added to avoid warnings
        }
    }

    rVal = aDate - aNullDate;
}

namespace
{

bool HiddenRowColumn(sal_uLong nRowColumn, bool bVertical, ScTable* pTable)
{
    if(bVertical)
    {
        return pTable->RowHidden(static_cast<SCROW>(nRowColumn));
    }
    else
    {
        return pTable->ColHidden(static_cast<SCCOL>(nRowColumn));
    }
}

}

void ScTable::FillSeries( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                    double nStepValue, double nMaxValue, sal_uInt16 nArgMinDigits,
                    bool bAttribs, ScProgress* pProgress )
{
    //
    //  Detect direction
    //

    bool bVertical = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP);
    bool bPositive = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_RIGHT);

    sal_uLong nCol = 0;
    sal_uLong nRow = 0;
    sal_uLong& rInner = bVertical ? nRow : nCol;        // loop variables
    sal_uLong& rOuter = bVertical ? nCol : nRow;
    sal_uLong nOStart;
    sal_uLong nOEnd;
    sal_uLong nIStart;
    sal_uLong nIEnd;
    sal_uLong nISource;
    ScRange aFillRange;

    if (bVertical)
    {
        nFillCount += (nRow2 - nRow1);
        if (nFillCount == 0)
            return;
        nOStart = nCol1;
        nOEnd = nCol2;
        if (bPositive)
        {
            nISource = nRow1;
            nIStart = nRow1 + 1;
            nIEnd = nRow1 + nFillCount;
            aFillRange = ScRange(nCol1, nRow1 + 1, nTab, nCol2, nRow1 + nFillCount, nTab);
        }
        else
        {
            nISource = nRow2;
            nIStart = nRow2 - 1;
            nIEnd = nRow2 - nFillCount;
            aFillRange = ScRange(nCol1, nRow2 -1, nTab, nCol2, nRow2 - nFillCount, nTab);
        }
    }
    else
    {
        nFillCount += (nCol2 - nCol1);
        if (nFillCount == 0)
            return;
        nOStart = nRow1;
        nOEnd = nRow2;
        if (bPositive)
        {
            nISource = nCol1;
            nIStart = nCol1 + 1;
            nIEnd = nCol1 + nFillCount;
            aFillRange = ScRange(nCol1 + 1, nRow1, nTab, nCol1 + nFillCount, nRow2, nTab);
        }
        else
        {
            nISource = nCol2;
            nIStart = nCol2 - 1;
            nIEnd = nCol2 - nFillCount;
            aFillRange = ScRange(nCol2 - 1, nRow1, nTab, nCol2 - nFillCount, nRow2, nTab);
        }
    }

    sal_uLong nIMin = nIStart;
    sal_uLong nIMax = nIEnd;
    PutInOrder(nIMin,nIMax);
    sal_uInt16 nDel = bAttribs ? IDF_AUTOFILL : (IDF_AUTOFILL & IDF_CONTENTS);

    bool bIsFiltered = IsDataFiltered(aFillRange);
    if (!bIsFiltered)
    {
        if (bVertical)
            DeleteArea(nCol1, static_cast<SCROW>(nIMin), nCol2, static_cast<SCROW>(nIMax), nDel);
        else
            DeleteArea(static_cast<SCCOL>(nIMin), nRow1, static_cast<SCCOL>(nIMax), nRow2, nDel);
    }

    sal_uLong nProgress = 0;
    if (pProgress)
        nProgress = pProgress->GetState();

    //
    //  execute
    //

    sal_uLong nActFormCnt = 0;
    for (rOuter = nOStart; rOuter <= nOEnd; rOuter++)
    {
        rInner = nISource;

        // Source cell value. We need to clone the value since it may be inserted repeatedly.
        ScCellValue aSrcCell = aCol[nCol].GetCellValue(static_cast<SCROW>(nRow));

        if (bAttribs)
        {
            const ScPatternAttr* pSrcPattern = aCol[nCol].GetPattern(static_cast<SCROW>(nRow));

            const ScCondFormatItem& rCondFormatItem = static_cast<const ScCondFormatItem&>(pSrcPattern->GetItem(ATTR_CONDITIONAL));
            const std::vector<sal_uInt32>& rCondFormatIndex = rCondFormatItem.GetCondFormatData();

            if (bVertical)
            {
                // if not filtered use the faster method
                // hidden cols/rows should be skiped
                if(!bIsFiltered)
                {
                    aCol[nCol].SetPatternArea( static_cast<SCROW>(nIMin),
                            static_cast<SCROW>(nIMax), *pSrcPattern, true );

                    for(std::vector<sal_uInt32>::const_iterator itr = rCondFormatIndex.begin(), itrEnd = rCondFormatIndex.end();
                            itr != itrEnd; ++itr)
                    {
                        ScConditionalFormat* pCondFormat = mpCondFormatList->GetFormat(*itr);
                        ScRangeList aRange = pCondFormat->GetRange();
                        aRange.Join(ScRange(nCol, nIMin, nTab, nCol, nIMax, nTab));
                        pCondFormat->AddRange(aRange);
                    }
                }
                else
                {
                    for(SCROW nAtRow = static_cast<SCROW>(nIMin); nAtRow <= static_cast<SCROW>(nIMax); ++nAtRow)
                    {
                        if(!RowHidden(nAtRow))
                        {
                            aCol[nCol].SetPatternArea( static_cast<SCROW>(nAtRow),
                                    static_cast<SCROW>(nAtRow), *pSrcPattern, true);
                            for(std::vector<sal_uInt32>::const_iterator itr = rCondFormatIndex.begin(), itrEnd = rCondFormatIndex.end();
                                    itr != itrEnd; ++itr)
                            {
                                ScConditionalFormat* pCondFormat = mpCondFormatList->GetFormat(*itr);
                                ScRangeList aRange = pCondFormat->GetRange();
                                aRange.Join(ScRange(nCol, nAtRow, nTab, nCol, nAtRow, nTab));
                                pCondFormat->AddRange(aRange);
                            }
                        }
                    }

                }
            }
            else
                for (SCCOL nAtCol = static_cast<SCCOL>(nIMin); nAtCol <= sal::static_int_cast<SCCOL>(nIMax); nAtCol++)
                    if(!ColHidden(nAtCol))
                    {
                        aCol[nAtCol].SetPattern(static_cast<SCROW>(nRow), *pSrcPattern, true);
                        for(std::vector<sal_uInt32>::const_iterator itr = rCondFormatIndex.begin(), itrEnd = rCondFormatIndex.end();
                                itr != itrEnd; ++itr)
                        {
                            ScConditionalFormat* pCondFormat = mpCondFormatList->GetFormat(*itr);
                            ScRangeList aRange = pCondFormat->GetRange();
                            aRange.Join(ScRange(nAtCol, static_cast<SCROW>(nRow), nTab, nAtCol, static_cast<SCROW>(nRow), nTab));
                            pCondFormat->AddRange(aRange);
                        }
                    }
        }

        if (!aSrcCell.isEmpty())
        {
            CellType eCellType = aSrcCell.meType;

            if (eFillCmd == FILL_SIMPLE)                // copy
            {
                if (eCellType == CELLTYPE_FORMULA)
                {
                    bool bFirst = true;
                    for (rInner = nIMin; rInner <= nIMax; rInner++)
                    {
                        if(HiddenRowColumn(rInner, bVertical, this))
                            continue;
                        sal_uLong nInd = nActFormCnt;
                        FillFormula(nInd, bFirst, aSrcCell.mpFormula,
                            static_cast<SCCOL>(nCol), nRow, (rInner == nIEnd) );
                        bFirst = false;
                        if(pProgress)
                            pProgress->SetStateOnPercent( ++nProgress );
                    }
                }
                else
                {
                    for (rInner = nIMin; rInner <= nIMax; rInner++)
                    {
                        if(HiddenRowColumn(rInner, bVertical, this))
                            continue;
                        ScAddress aDestPos( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), nTab );
                        aSrcCell.commit(aCol[nCol], aDestPos.Row());
                    }
                    nProgress += nIMax - nIMin + 1;
                    if(pProgress)
                        pProgress->SetStateOnPercent( nProgress );
                }
            }
            else if (eCellType == CELLTYPE_VALUE || eCellType == CELLTYPE_FORMULA)
            {
                double nStartVal;
                if (eCellType == CELLTYPE_VALUE)
                    nStartVal = aSrcCell.mfValue;
                else
                    nStartVal = aSrcCell.mpFormula->GetValue();
                double nVal = nStartVal;
                long nIndex = 0;

                bool bError = false;
                bool bOverflow = false;

                sal_uInt16 nDayOfMonth = 0;
                rInner = nIStart;
                while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
                {
                    if(!ColHidden(nCol) && !RowHidden(nRow))
                    {
                        if (!bError && !bOverflow)
                        {
                            switch (eFillCmd)
                            {
                                case FILL_LINEAR:
                                    {
                                        //  use multiplication instead of repeated addition
                                        //  to avoid accumulating rounding errors
                                        nVal = nStartVal;
                                        double nAdd = nStepValue;
                                        if ( !SubTotal::SafeMult( nAdd, (double) ++nIndex ) ||
                                                !SubTotal::SafePlus( nVal, nAdd ) )
                                            bError = true;
                                    }
                                    break;
                                case FILL_GROWTH:
                                    if (!SubTotal::SafeMult(nVal, nStepValue))
                                        bError = true;
                                    break;
                                case FILL_DATE:
                                    if (fabs(nVal) > _D_MAX_LONG_)
                                        bError = true;
                                    else
                                        IncDate(nVal, nDayOfMonth, nStepValue, eFillDateCmd);
                                    break;
                                default:
                                    {
                                        // added to avoid warnings
                                    }
                            }

                            if (nStepValue >= 0)
                            {
                                if (nVal > nMaxValue)           // target value reached ?
                                {
                                    nVal = nMaxValue;
                                    bOverflow = true;
                                }
                            }
                            else
                            {
                                if (nVal < nMaxValue)
                                {
                                    nVal = nMaxValue;
                                    bOverflow = true;
                                }
                            }
                        }

                        if (bError)
                            aCol[nCol].SetError(static_cast<SCROW>(nRow), errNoValue);
                        else if (bOverflow)
                            aCol[nCol].SetError(static_cast<SCROW>(nRow), errIllegalFPOperation);
                        else
                            aCol[nCol].SetValue(static_cast<SCROW>(nRow), nVal);
                    }

                    if (rInner == nIEnd)
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
                    if ( nMaxValue >= (double)LONG_MAX )
                        nMaxValue = (double)LONG_MAX - 1;
                }
                else
                {
                    if ( nMaxValue <= (double)LONG_MIN )
                        nMaxValue = (double)LONG_MIN + 1;
                }
                String aValue;
                if (eCellType == CELLTYPE_STRING)
                    aValue = *aSrcCell.mpString;
                else
                    aValue = ScEditUtil::GetString(*aSrcCell.mpEditText, pDocument);
                sal_Int32 nStringValue;
                sal_uInt16 nMinDigits = nArgMinDigits;
                short nHeadNoneTail = lcl_DecompValueString( aValue, nStringValue, &nMinDigits );
                if ( nHeadNoneTail )
                {
                    double nStartVal = (double)nStringValue;
                    double nVal = nStartVal;
                    long nIndex = 0;
                    bool bError = false;
                    bool bOverflow = false;

                    bool bIsOrdinalSuffix = aValue.Equals( ScGlobal::GetOrdinalSuffix(
                                (sal_Int32)nStartVal));

                    rInner = nIStart;
                    while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
                    {
                        if(!ColHidden(nCol) && !RowHidden(nRow))
                        {
                            if (!bError && !bOverflow)
                            {
                                switch (eFillCmd)
                                {
                                    case FILL_LINEAR:
                                        {
                                            //  use multiplication instead of repeated addition
                                            //  to avoid accumulating rounding errors
                                            nVal = nStartVal;
                                            double nAdd = nStepValue;
                                            if ( !SubTotal::SafeMult( nAdd, (double) ++nIndex ) ||
                                                    !SubTotal::SafePlus( nVal, nAdd ) )
                                                bError = true;
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

                                if (nStepValue >= 0)
                                {
                                    if (nVal > nMaxValue)           // target value reached ?
                                    {
                                        nVal = nMaxValue;
                                        bOverflow = true;
                                    }
                                }
                                else
                                {
                                    if (nVal < nMaxValue)
                                    {
                                        nVal = nMaxValue;
                                        bOverflow = true;
                                    }
                                }
                            }

                            if (bError)
                                aCol[nCol].SetError(static_cast<SCROW>(nRow), errNoValue);
                            else if (bOverflow)
                                aCol[nCol].SetError(static_cast<SCROW>(nRow), errIllegalFPOperation);
                            else
                            {
                                nStringValue = (sal_Int32)nVal;
                                String aStr;
                                if ( nHeadNoneTail < 0 )
                                {
                                    setSuffixCell(
                                        aCol[nCol], static_cast<SCROW>(nRow),
                                        nStringValue, nMinDigits, aValue,
                                        eCellType, bIsOrdinalSuffix);
                                }
                                else
                                {
                                    aStr = aValue;
                                    aStr += lcl_ValueString( nStringValue, nMinDigits );
                                    aCol[nCol].SetRawString(static_cast<SCROW>(nRow), aStr);
                                }
                            }
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
        }
        else if(pProgress)
        {
            nProgress += nIMax - nIMin + 1;
            pProgress->SetStateOnPercent( nProgress );
        }
        ++nActFormCnt;
    }
}

void ScTable::Fill( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                    double nStepValue, double nMaxValue, ScProgress* pProgress)
{
    if (eFillCmd == FILL_AUTO)
        FillAuto(nCol1, nRow1, nCol2, nRow2, nFillCount, eFillDir, pProgress);
    else
        FillSeries(nCol1, nRow1, nCol2, nRow2, nFillCount, eFillDir,
                    eFillCmd, eFillDateCmd, nStepValue, nMaxValue, 0, true, pProgress);
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
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        ScAutoFormat& rFormat = *ScGlobal::GetOrCreateAutoFormat();
        ScAutoFormatData* pData = rFormat.findByIndex(nFormatNo);
        if (pData)
        {
            ScPatternAttr* pPatternAttrs[16];
            for (sal_uInt8 i = 0; i < 16; ++i)
            {
                pPatternAttrs[i] = new ScPatternAttr(pDocument->GetPool());
                pData->FillToItemSet(i, pPatternAttrs[i]->GetItemSet(), *pDocument);
            }

            SCCOL nCol = nStartCol;
            SCROW nRow = nStartRow;
            sal_uInt16 nIndex = 0;
            // Left top corner
            AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
            // Left column
            if (pData->IsEqualData(4, 8))
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
            if (pData->IsEqualData(7, 11))
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
            // Boddy
            if ((pData->IsEqualData(5, 6)) && (pData->IsEqualData(9, 10)) && (pData->IsEqualData(5, 9)))
                AutoFormatArea(nStartCol + 1, nStartRow + 1, nEndCol-1, nEndRow - 1, *pPatternAttrs[5], nFormatNo);
            else
            {
                if ((pData->IsEqualData(5, 9)) && (pData->IsEqualData(6, 10)))
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

            for (sal_uInt8 j = 0; j < 16; ++j)
                delete pPatternAttrs[j];
        } // if AutoFormatData != NULL
    } // if ValidColRow
}

void ScTable::GetAutoFormatAttr(SCCOL nCol, SCROW nRow, sal_uInt16 nIndex, ScAutoFormatData& rData)
{
    sal_uInt32 nFormatIndex = GetNumberFormat( nCol, nRow );
    ScNumFormatAbbrev   aNumFormat( nFormatIndex, *pDocument->GetFormatTable() );
    rData.GetFromItemSet( nIndex, GetPattern( nCol, nRow )->GetItemSet(), aNumFormat );
}

#define LF_LEFT         1
#define LF_TOP          2
#define LF_RIGHT        4
#define LF_BOTTOM       8
#define LF_ALL          (LF_LEFT | LF_TOP | LF_RIGHT | LF_BOTTOM)

void ScTable::GetAutoFormatFrame(SCCOL nCol, SCROW nRow, sal_uInt16 nFlags, sal_uInt16 nIndex, ScAutoFormatData& rData)
{
    const SvxBoxItem* pTheBox = (SvxBoxItem*)GetAttr(nCol, nRow, ATTR_BORDER);
    const SvxBoxItem* pLeftBox = (SvxBoxItem*)GetAttr(nCol - 1, nRow, ATTR_BORDER);
    const SvxBoxItem* pTopBox = (SvxBoxItem*)GetAttr(nCol, nRow - 1, ATTR_BORDER);
    const SvxBoxItem* pRightBox = (SvxBoxItem*)GetAttr(nCol + 1, nRow, ATTR_BORDER);
    const SvxBoxItem* pBottomBox = (SvxBoxItem*)GetAttr(nCol, nRow + 1, ATTR_BORDER);

    SvxBoxItem aBox( ATTR_BORDER );
    if (nFlags & LF_LEFT)
    {
        if (pLeftBox)
        {
            if (ScHasPriority(pTheBox->GetLeft(), pLeftBox->GetRight()))
                aBox.SetLine(pTheBox->GetLeft(), BOX_LINE_LEFT);
            else
                aBox.SetLine(pLeftBox->GetRight(), BOX_LINE_LEFT);
        }
        else
            aBox.SetLine(pTheBox->GetLeft(), BOX_LINE_LEFT);
    }
    if (nFlags & LF_TOP)
    {
        if (pTopBox)
        {
            if (ScHasPriority(pTheBox->GetTop(), pTopBox->GetBottom()))
                aBox.SetLine(pTheBox->GetTop(), BOX_LINE_TOP);
            else
                aBox.SetLine(pTopBox->GetBottom(), BOX_LINE_TOP);
        }
        else
            aBox.SetLine(pTheBox->GetTop(), BOX_LINE_TOP);
    }
    if (nFlags & LF_RIGHT)
    {
        if (pRightBox)
        {
            if (ScHasPriority(pTheBox->GetRight(), pRightBox->GetLeft()))
                aBox.SetLine(pTheBox->GetRight(), BOX_LINE_RIGHT);
            else
                aBox.SetLine(pRightBox->GetLeft(), BOX_LINE_RIGHT);
        }
        else
            aBox.SetLine(pTheBox->GetRight(), BOX_LINE_RIGHT);
    }
    if (nFlags & LF_BOTTOM)
    {
        if (pBottomBox)
        {
            if (ScHasPriority(pTheBox->GetBottom(), pBottomBox->GetTop()))
                aBox.SetLine(pTheBox->GetBottom(), BOX_LINE_BOTTOM);
            else
                aBox.SetLine(pBottomBox->GetTop(), BOX_LINE_BOTTOM);
        }
        else
            aBox.SetLine(pTheBox->GetBottom(), BOX_LINE_BOTTOM);
    }
    rData.PutItem( nIndex, aBox );
}

void ScTable::GetAutoFormatData(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, ScAutoFormatData& rData)
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        if ((nEndCol - nStartCol >= 3) && (nEndRow - nStartRow >= 3))
        {
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
    }
}

void ScTable::SetError( SCCOL nCol, SCROW nRow, sal_uInt16 nError)
{
    if (ValidColRow(nCol, nRow))
        aCol[nCol].SetError( nRow, nError );
}

void ScTable::UpdateInsertTabAbs(SCTAB nTable)
{
    for (SCCOL i=0; i <= MAXCOL; i++)
        aCol[i].UpdateInsertTabAbs(nTable);
}

bool ScTable::GetNextSpellingCell(SCCOL& rCol, SCROW& rRow, bool bInSel,
                                    const ScMarkData& rMark) const
{
    if (rRow == MAXROW+2)                       // end of table
    {
        rRow = 0;
        rCol = 0;
    }
    else
    {
        rRow++;
        if (rRow == MAXROW+1)
        {
            rCol++;
            rRow = 0;
        }
    }
    if (rCol == MAXCOL+1)
        return true;
    else
    {
        bool bStop = false;
        while (!bStop)
        {
            if (ValidCol(rCol))
            {
                bStop = aCol[rCol].GetNextSpellingCell(rRow, bInSel, rMark);
                if (bStop)
                    return true;
                else /*if (rRow == MAXROW+1) */
                {
                    rCol++;
                    rRow = 0;
                }
            }
            else
                return true;
        }
    }
    return false;
}

void ScTable::RemoveAutoSpellObj()
{
    for (SCCOL i=0; i <= MAXCOL; i++)
        aCol[i].RemoveAutoSpellObj();
}

bool ScTable::TestTabRefAbs(SCTAB nTable) const
{
    for (SCCOL i=0; i <= MAXCOL; i++)
        if (aCol[i].TestTabRefAbs(nTable))
            return true;
    return false;
}

void ScTable::CompileDBFormula()
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CompileDBFormula();
}

void ScTable::CompileDBFormula( bool bCreateFormulaString )
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CompileDBFormula( bCreateFormulaString );
}

void ScTable::CompileNameFormula( bool bCreateFormulaString )
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CompileNameFormula( bCreateFormulaString );
}

void ScTable::CompileColRowNameFormula()
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CompileColRowNameFormula();
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
