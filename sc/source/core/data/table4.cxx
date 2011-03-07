/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2011 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// System - Includes -----------------------------------------------------



#ifdef _MSC_VER
#pragma optimize("",off)
                                        // sonst Absturz Win beim Fuellen
#endif

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
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
#include <editeng/escpitem.hxx>
#include <svl/zforlist.hxx>
#include <vcl/keycodes.hxx>
#include <rtl/math.hxx>
#include <unotools/charclass.hxx>

#include "attrib.hxx"
#include "patattr.hxx"
#include "cell.hxx"
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

#include <math.h>

// STATIC DATA -----------------------------------------------------------

#define _D_MAX_LONG_  (double) 0x7fffffff

extern sal_uInt16 nScFillModeMouseModifier;     // global.cxx

// -----------------------------------------------------------------------

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
    while ( p[nNum] && CharClass::isAsciiNumeric( p[nNum] ) )
        nNum++;

    sal_Unicode cNext = p[nNum];            // 0 if at the end
    sal_Unicode cLast = p[aValue.Len()-1];

    // #i5550# If there are numbers at the beginning and the end,
    // prefer the one at the beginning only if it's followed by a space.
    // Otherwise, use the number at the end, to enable things like IP addresses.
    if ( nNum > nNeg && ( cNext == 0 || cNext == ' ' || !CharClass::isAsciiNumeric(cLast) ) )
    {   // number at the beginning
        nVal = aValue.Copy( 0, nNum ).ToInt32();
        //  #60893# any number with a leading zero sets the minimum number of digits
        if ( p[nNeg] == '0' && pMinDigits && ( nNum - nNeg > *pMinDigits ) )
            *pMinDigits = nNum - nNeg;
        aValue.Erase( 0, nNum );
        return -1;
    }
    else
    {
        nNeg = 0;
        xub_StrLen nEnd = nNum = aValue.Len() - 1;
        while ( nNum && CharClass::isAsciiNumeric( p[nNum] ) )
            nNum--;
        if ( p[nNum] == '-' )
        {
            nNum--;
            nNeg = 1;
        }
        if ( nNum < nEnd - nNeg )
        {   // number at the end
            nVal = aValue.Copy( nNum + 1 ).ToInt32();
            //  #60893# any number with a leading zero sets the minimum number of digits
            if ( p[nNum+1+nNeg] == '0' && pMinDigits && ( nEnd - nNum - nNeg > *pMinDigits ) )
                *pMinDigits = nEnd - nNum - nNeg;
            aValue.Erase( nNum + 1 );
            return 1;
        }
    }
    nVal = 0;
    return 0;
}

String lcl_ValueString( sal_Int32 nValue, sal_uInt16 nMinDigits )
{
    if ( nMinDigits <= 1 )
        return String::CreateFromInt32( nValue );           // simple case...
    else
    {
        String aStr = String::CreateFromInt32( Abs( nValue ) );
        if ( aStr.Len() < nMinDigits )
        {
            String aZero;
            aZero.Fill( nMinDigits - aStr.Len(), '0' );
            aStr.Insert( aZero, 0 );
        }
        //  nMinDigits doesn't include the '-' sign -> add after inserting zeros
        if ( nValue < 0 )
            aStr.Insert( '-', 0 );
        return aStr;
    }
}

static ScBaseCell * lcl_getSuffixCell( ScDocument* pDocument, sal_Int32 nValue,
        sal_uInt16 nDigits, const String& rSuffix, CellType eCellType,
        sal_Bool bIsOrdinalSuffix )
{
    String aValue( lcl_ValueString( nValue, nDigits ));
    if (!bIsOrdinalSuffix)
        return new ScStringCell( aValue += rSuffix);

    String aOrdinalSuffix( ScGlobal::GetOrdinalSuffix( nValue));
    if (eCellType != CELLTYPE_EDIT)
        return new ScStringCell( aValue += aOrdinalSuffix);

    EditEngine aEngine( pDocument->GetEnginePool() );
    SfxItemSet aAttr = aEngine.GetEmptyItemSet();
    aAttr.Put( SvxEscapementItem( SVX_ESCAPEMENT_SUPERSCRIPT, EE_CHAR_ESCAPEMENT));
    aEngine.SetText( aValue );
    aEngine.QuickInsertText( aOrdinalSuffix, ESelection( 0, aValue.Len(), 0,
                aValue.Len() + aOrdinalSuffix.Len()));
    aEngine.QuickSetAttribs( aAttr, ESelection( 0, aValue.Len(), 0, aValue.Len() +
                aOrdinalSuffix.Len()));
    return new ScEditCell( aEngine.CreateTextObject(), pDocument, NULL );
}

void ScTable::FillAnalyse( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            FillCmd& rCmd, FillDateCmd& rDateCmd,
                            double& rInc, sal_uInt16& rMinDigits,
                            ScUserListData*& rListData, sal_uInt16& rListIndex)
{
    DBG_ASSERT( nCol1==nCol2 || nRow1==nRow2, "FillAnalyse: falscher Bereich" );

    rInc = 0.0;
    rMinDigits = 0;
    rListData = NULL;
    rCmd = FILL_SIMPLE;
    if (( nScFillModeMouseModifier & KEY_MOD1 )||IsDataFiltered())  //i89232
        return ;        // Ctrl-Taste: Copy

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

    ScBaseCell* pFirstCell = GetCell( nCol, nRow );
    CellType eCellType = pFirstCell ? pFirstCell->GetCellType() : CELLTYPE_NONE;

    if (eCellType == CELLTYPE_VALUE)
    {
        sal_uInt32 nFormat = ((const SfxUInt32Item*)GetAttr(nCol,nRow,ATTR_VALUE_FORMAT))->GetValue();
        sal_Bool bDate = ( pDocument->GetFormatTable()->GetType(nFormat) == NUMBERFORMAT_DATE );
        if (bDate)
        {
            if (nCount > 1)
            {
                long nCmpInc = 0;
                double nVal;
                Date aNullDate = *pDocument->GetFormatTable()->GetNullDate();
                Date aDate1 = aNullDate;
                nVal = ((ScValueCell*)pFirstCell)->GetValue();
                aDate1 += (long)nVal;
                Date aDate2 = aNullDate;
                nVal = GetValue(nCol+nAddX, nRow+nAddY);
                aDate2 += (long)nVal;
                if ( aDate1 != aDate2 )
                {
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
                    sal_Bool bVal = sal_True;
                    for (sal_uInt16 i=1; i<nCount && bVal; i++)
                    {
                        ScBaseCell* pCell = GetCell(nCol,nRow);
                        if (pCell && pCell->GetCellType() == CELLTYPE_VALUE)
                        {
                            nVal = ((ScValueCell*)pCell)->GetValue();
                            aDate2 = aNullDate + (long) nVal;
                            if ( eType == FILL_DAY )
                            {
                                if ( aDate2-aDate1 != nCmpInc )
                                    bVal = sal_False;
                            }
                            else
                            {
                                nDDiff = aDate2.GetDay()   - (long) aDate1.GetDay();
                                nMDiff = aDate2.GetMonth() - (long) aDate1.GetMonth();
                                nYDiff = aDate2.GetYear()  - (long) aDate1.GetYear();
                                if (nDDiff || ( nMDiff + 12 * nYDiff != nCmpInc ))
                                    bVal = sal_False;
                            }
                            aDate1 = aDate2;
                            nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                            nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                        }
                        else
                            bVal = sal_False;   // #50965# kein Datum passt auch nicht
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
            else                            // einzelnes Datum -> Tage hochzaehlen
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
                double nVal1 = ((ScValueCell*)pFirstCell)->GetValue();
                double nVal2 = GetValue(nCol+nAddX, nRow+nAddY);
                rInc = nVal2 - nVal1;
                nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                sal_Bool bVal = sal_True;
                for (sal_uInt16 i=1; i<nCount && bVal; i++)
                {
                    ScBaseCell* pCell = GetCell(nCol,nRow);
                    if (pCell && pCell->GetCellType() == CELLTYPE_VALUE)
                    {
                        nVal2 = ((ScValueCell*)pCell)->GetValue();
                        double nDiff = nVal2 - nVal1;
                        if ( !::rtl::math::approxEqual( nDiff, rInc ) )
                            bVal = sal_False;
                        nVal1 = nVal2;
                    }
                    else
                        bVal = sal_False;
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
        String aStr;
        GetString(nCol, nRow, aStr);
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
            short nFlag1 = lcl_DecompValueString( aStr, nVal1, &rMinDigits );
            if ( nFlag1 )
            {
                sal_Int32 nVal2;
                GetString( nCol+nAddX, nRow+nAddY, aStr );
                short nFlag2 = lcl_DecompValueString( aStr, nVal2, &rMinDigits );
                if ( nFlag1 == nFlag2 )
                {
                    rInc = (double)nVal2 - (double)nVal1;
                    nCol = sal::static_int_cast<SCCOL>( nCol + nAddX );
                    nRow = sal::static_int_cast<SCROW>( nRow + nAddY );
                    sal_Bool bVal = sal_True;
                    for (sal_uInt16 i=1; i<nCount && bVal; i++)
                    {
                        ScBaseCell* pCell = GetCell(nCol,nRow);
                        CellType eType = pCell ? pCell->GetCellType() : CELLTYPE_NONE;
                        if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
                        {
                            if ( eType == CELLTYPE_STRING )
                                ((ScStringCell*)pCell)->GetString( aStr );
                            else
                                ((ScEditCell*)pCell)->GetString( aStr );
                            nFlag2 = lcl_DecompValueString( aStr, nVal2, &rMinDigits );
                            if ( nFlag1 == nFlag2 )
                            {
                                double nDiff = (double)nVal2 - (double)nVal1;
                                if ( !::rtl::math::approxEqual( nDiff, rInc ) )
                                    bVal = sal_False;
                                nVal1 = nVal2;
                            }
                            else
                                bVal = sal_False;
                        }
                        else
                            bVal = sal_False;
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

void ScTable::FillFormula(sal_uLong& /* nFormulaCounter */, sal_Bool /* bFirst */, ScFormulaCell* pSrcCell,
                          SCCOL nDestCol, SCROW nDestRow, sal_Bool bLast )
{
/*  sal_uInt16 nTokArrLen = pSrcCell->GetTokenArrayLen();
    if ( nTokArrLen > 15 )                          // mehr als =A1 oder =67
    {
        ScRangeName* pRangeName = pDocument->GetRangeName();
        String aName("___SC_");                     // Wird dieser String veraendert,
                                                    // auch in document2 EraseNonUsed...
                                                    // mitaendern!!
        aName += pRangeName->GetSharedMaxIndex() + 1;
        aName += '_';
        aName += nFormulaCounter;
        nFormulaCounter++;
        if (bFirst)
        {
            ScRangeData *pAktRange = new ScRangeData(
                            pDocument, aName, pSrcCell->GetTokenArray(), nTokArrLen,
                            pSrcCell->GetCol(), pSrcCell->GetRow(), nTab ,RT_SHARED);
            if (!pRangeName->Insert( pAktRange ))
                delete pAktRange;
            else
                bSharedNameInserted = sal_True;
        }
        sal_uInt16 nIndex;
        pRangeName->SearchName(aName, nIndex);
        if (!pRangeName)
        {
            DBG_ERROR("ScTable::FillFormula: Falscher Name");
            return;
        }
        nIndex = ((ScRangeData*) ((*pRangeName)[nIndex]))->GetIndex();
        ScTokenArray aArr;
        aArr.AddName(nIndex);
        aArr.AddOpCode(ocStop);
        ScFormulaCell* pDestCell = new ScFormulaCell
            (pDocument, ScAddress( nDestCol, nDestRow, nTab ), aArr );
        aCol[nDestCol].Insert(nDestRow, pDestCell);
    }
    else
*/  {
        pDocument->SetNoListening( sal_True );  // noch falsche Referenzen
        ScAddress aAddr( nDestCol, nDestRow, nTab );
        ScFormulaCell* pDestCell = new ScFormulaCell( *pSrcCell, *pDocument, aAddr );
        aCol[nDestCol].Insert(nDestRow, pDestCell);
#if 0
// mit RelRefs unnoetig
        pDestCell->UpdateReference(URM_COPY,
                         ScRange( aAddr, aAddr ),
                         nDestCol - pSrcCell->aPos.Col(),
                         nDestRow - pSrcCell->aPos.Row(), 0);
#endif
        if ( bLast && pDestCell->GetMatrixFlag() )
        {
            ScAddress aOrg;
            if ( pDestCell->GetMatrixOrigin( aOrg ) )
            {
                if ( nDestCol >= aOrg.Col() && nDestRow >= aOrg.Row() )
                {
                    ScBaseCell* pOrgCell = pDocument->GetCell( aOrg );
                    if ( pOrgCell && pOrgCell->GetCellType() == CELLTYPE_FORMULA
                      && ((ScFormulaCell*)pOrgCell)->GetMatrixFlag() == MM_FORMULA )
                    {
                        ((ScFormulaCell*)pOrgCell)->SetMatColsRows(
                            nDestCol - aOrg.Col() + 1,
                            nDestRow - aOrg.Row() + 1 );
                    }
                    else
                    {
                        DBG_ERRORFILE( "FillFormula: MatrixOrigin keine Formelzelle mit MM_FORMULA" );
                    }
                }
                else
                {
                    DBG_ERRORFILE( "FillFormula: MatrixOrigin rechts unten" );
                }
            }
            else
            {
                DBG_ERRORFILE( "FillFormula: kein MatrixOrigin" );
            }
        }
        pDocument->SetNoListening( sal_False );
        pDestCell->StartListeningTo( pDocument );
    }
}

void ScTable::FillAuto( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        sal_uLong nFillCount, FillDir eFillDir, ScProgress& rProgress )
{
    if ( (nFillCount == 0) || !ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2) )
        return;

    //
    //  Richtung auswerten
    //

    sal_Bool bVertical = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP);
    sal_Bool bPositive = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_RIGHT);

    sal_uLong nCol = 0;
    sal_uLong nRow = 0;
    sal_uLong& rInner = bVertical ? nRow : nCol;        // Schleifenvariablen
    sal_uLong& rOuter = bVertical ? nCol : nRow;
    sal_uLong nOStart;
    sal_uLong nOEnd;
    sal_uLong nIStart;
    sal_uLong nIEnd;
    sal_uLong nISrcStart;
    sal_uLong nISrcEnd;

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
        }
        else
        {
            nISrcStart = nRow2;
            nISrcEnd = nRow1;
            nIStart = nRow1 - 1;
            nIEnd = nRow1 - nFillCount;
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
        }
        else
        {
            nISrcStart = nCol2;
            nISrcEnd = nCol1;
            nIStart = nCol1 - 1;
            nIEnd = nCol1 - nFillCount;
        }
    }
    sal_uLong nIMin = nIStart;
    sal_uLong nIMax = nIEnd;
    PutInOrder(nIMin,nIMax);
    if (!IsDataFiltered())  //modify for i89232
    {
        if (bVertical)
            DeleteArea(nCol1, static_cast<SCROW>(nIMin), nCol2, static_cast<SCROW>(nIMax), IDF_AUTOFILL);
        else
            DeleteArea(static_cast<SCCOL>(nIMin), nRow1, static_cast<SCCOL>(nIMax), nRow2, IDF_AUTOFILL);
    }
    sal_uLong nProgress = rProgress.GetState();

    //
    //  ausfuehren
    //

    sal_uLong nActFormCnt = 0;
    for (rOuter = nOStart; rOuter <= nOEnd; rOuter++)
    {
        sal_uLong nMaxFormCnt = 0;                      // fuer Formeln

        //  Attributierung uebertragen

        const ScPatternAttr* pSrcPattern = NULL;
        const ScStyleSheet* pStyleSheet = NULL;
        sal_uLong nAtSrc = nISrcStart;
        ScPatternAttr* pNewPattern = NULL;
        sal_Bool bGetPattern = sal_True;
        rInner = nIStart;
        while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
        {
            if ( bGetPattern )
            {
                if ( pNewPattern )
                    delete pNewPattern;
                if (bVertical)      // rInner&:=nRow, rOuter&:=nCol
                    pSrcPattern = aCol[nCol].GetPattern(static_cast<SCROW>(nAtSrc));
                else                // rInner&:=nCol, rOuter&:=nRow
                    pSrcPattern = aCol[nAtSrc].GetPattern(static_cast<SCROW>(nRow));
                bGetPattern = sal_False;
                pStyleSheet = pSrcPattern->GetStyleSheet();
                //  Merge/Mergeflag nicht uebernehmen,
                const SfxItemSet& rSet = pSrcPattern->GetItemSet();
                if ( rSet.GetItemState(ATTR_MERGE, sal_False) == SFX_ITEM_SET
                  || rSet.GetItemState(ATTR_MERGE_FLAG, sal_False) == SFX_ITEM_SET )
                {
                    pNewPattern = new ScPatternAttr( *pSrcPattern );
                    SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                    rNewSet.ClearItem(ATTR_MERGE);
                    rNewSet.ClearItem(ATTR_MERGE_FLAG);
                }
                else
                    pNewPattern = NULL;
            }

            if ( bVertical && nISrcStart == nISrcEnd )
            {
                //  Attribute komplett am Stueck setzen
                if (pNewPattern || pSrcPattern != pDocument->GetDefPattern())
                {
                    //  Default steht schon da (DeleteArea)
                    SCROW nY1 = static_cast<SCROW>(Min( nIStart, nIEnd ));
                    SCROW nY2 = static_cast<SCROW>(Max( nIStart, nIEnd ));
                    if ( pStyleSheet )
                        aCol[nCol].ApplyStyleArea( nY1, nY2, *pStyleSheet );
                    if ( pNewPattern )
                        aCol[nCol].ApplyPatternArea( nY1, nY2, *pNewPattern );
                    else
                        aCol[nCol].ApplyPatternArea( nY1, nY2, *pSrcPattern );
                }
                break;      // Schleife abbrechen
            }

            if ( pSrcPattern != aCol[nCol].GetPattern( static_cast<SCROW>(nRow) ) )
            {
                //  Vorlage auch uebernehmen
                //! am AttrArray mit ApplyPattern zusammenfassen ??
                if ( pStyleSheet )
                    aCol[nCol].ApplyStyle( static_cast<SCROW>(nRow), *pStyleSheet );

                //  ApplyPattern statt SetPattern um alte MergeFlags stehenzulassen
                if ( pNewPattern )
                    aCol[nCol].ApplyPattern( static_cast<SCROW>(nRow), *pNewPattern );
                else
                    aCol[nCol].ApplyPattern( static_cast<SCROW>(nRow), *pSrcPattern );
            }

            if (nAtSrc==nISrcEnd)
            {
                if ( nAtSrc != nISrcStart )
                {   // mehr als eine Source-Zelle
                    nAtSrc = nISrcStart;
                    bGetPattern = sal_True;
                }
            }
            else if (bPositive)
            {
                ++nAtSrc;
                bGetPattern = sal_True;
            }
            else
            {
                --nAtSrc;
                bGetPattern = sal_True;
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

        if (bVertical)
            aCol[nCol].Resize( aCol[nCol].GetCellCount() + nFillCount );

        if (pListData)
        {
            sal_uInt16 nListCount = pListData->GetSubCount();
            if ( !bPositive )
            {
                //  nListIndex auf FillAnalyse zeigt auf den letzten Eintrag -> anpassen
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
                aCol[nCol].Insert(static_cast<SCROW>(nRow), new ScStringCell(pListData->GetSubStr(nListIndex)));

                if (rInner == nIEnd) break;
                if (bPositive) ++rInner; else --rInner;
            }
            nProgress += nIMax - nIMin + 1;
            rProgress.SetStateOnPercent( nProgress );
        }
        else if (eFillCmd == FILL_SIMPLE)           // Auffuellen mit Muster
        {
            sal_uLong nSource = nISrcStart;
            double nDelta;
            if (( nScFillModeMouseModifier & KEY_MOD1 )||IsDataFiltered()) //i89232
                nDelta = 0.0;
            else if ( bPositive )
                nDelta = 1.0;
            else
                nDelta = -1.0;
            double nVal = 0.0;
            sal_uLong nFormulaCounter = nActFormCnt;
            sal_Bool bFirst = sal_True;
            sal_Bool bGetCell = sal_True;
            sal_uInt16 nCellDigits = 0;
            short nHeadNoneTail = 0;
            sal_Int32 nStringValue = 0;
            String aValue;
            ScBaseCell* pSrcCell = NULL;
            CellType eCellType = CELLTYPE_NONE;
            sal_Bool bIsOrdinalSuffix = sal_False;
            sal_Bool bRowFiltered = sal_False; //i89232

            rInner = nIStart;
            while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
            {
                if ( bGetCell )
                {
                    if (bVertical)      // rInner&:=nRow, rOuter&:=nCol
                        pSrcCell = aCol[nCol].GetCell( static_cast<SCROW>(nSource) );
                    else                // rInner&:=nCol, rOuter&:=nRow
                        pSrcCell = aCol[nSource].GetCell( static_cast<SCROW>(nRow) );
                    bGetCell = sal_False;
                    if ( pSrcCell )
                    {
                        eCellType = pSrcCell->GetCellType();
                        switch ( eCellType )
                        {
                            case CELLTYPE_VALUE:
                                nVal = ((ScValueCell*)pSrcCell)->GetValue();
                            break;
                            case CELLTYPE_STRING:
                            case CELLTYPE_EDIT:
                                if ( eCellType == CELLTYPE_STRING )
                                    ((ScStringCell*)pSrcCell)->GetString( aValue );
                                else
                                    ((ScEditCell*)pSrcCell)->GetString( aValue );
                                if ( !(nScFillModeMouseModifier & KEY_MOD1) && !IsDataFiltered())   //i89232
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

                //Modify for i89232
                bRowFiltered = mpFilteredRows->getValue(nRow);

                if (!bRowFiltered)
                {
                    if (IsDataFiltered())
                        DeleteArea(nCol, nRow, nCol, nRow, IDF_AUTOFILL);
                //End of i89232

                switch (eCellType)
                {
                    case CELLTYPE_VALUE:
                        aCol[nCol].Insert(static_cast<SCROW>(nRow), new ScValueCell(nVal + nDelta));
                        break;
                    case CELLTYPE_STRING:
                    case CELLTYPE_EDIT:
                        if ( nHeadNoneTail )
                        {
                            // #i48009# with the "nStringValue+(long)nDelta" expression within the
                            // lcl_ValueString calls, gcc 3.4.1 makes wrong optimizations (ok in 3.4.3),
                            // so nNextValue is now calculated ahead.
                            sal_Int32 nNextValue = nStringValue+(sal_Int32)nDelta;

                            String aStr;
                            if ( nHeadNoneTail < 0 )
                            {
                                aCol[nCol].Insert( static_cast<SCROW>(nRow),
                                        lcl_getSuffixCell( pDocument,
                                            nNextValue, nCellDigits, aValue,
                                            eCellType, bIsOrdinalSuffix));
                            }
                            else
                            {
                                aStr = aValue;
                                aStr += lcl_ValueString( nNextValue, nCellDigits );
                                aCol[nCol].Insert( static_cast<SCROW>(nRow),
                                        new ScStringCell( aStr));
                            }
                        }
                        else
                        {
                            ScAddress aDestPos( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), nTab );
                            switch ( eCellType )
                            {
                                case CELLTYPE_STRING:
                                case CELLTYPE_EDIT:
                                    aCol[nCol].Insert( aDestPos.Row(), pSrcCell->CloneWithoutNote( *pDocument ) );
                                break;
                                default:
                                {
                                    // added to avoid warnings
                                }
                            }
                        }
                        break;
                    case CELLTYPE_FORMULA :
                        FillFormula( nFormulaCounter, bFirst,
                                (ScFormulaCell*) pSrcCell,
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
                    {   // mehr als eine Source-Zelle
                        nSource = nISrcStart;
                        bGetCell = sal_True;
                    }
                        if ( !(nScFillModeMouseModifier & KEY_MOD1) && !IsDataFiltered() ) //i89232
                    {
                        if ( bPositive )
                            nDelta += 1.0;
                        else
                            nDelta -= 1.0;
                    }
                    nFormulaCounter = nActFormCnt;
                    bFirst = sal_False;
                }
                else if (bPositive)
                {
                    ++nSource;
                    bGetCell = sal_True;
                }
                else
                {
                    --nSource;
                    bGetCell = sal_True;
                    }
                }

                //  Progress in der inneren Schleife nur bei teuren Zellen,
                //  und auch dann nicht fuer jede einzelne

                ++nProgress;
                if ( eCellType == CELLTYPE_FORMULA || eCellType == CELLTYPE_EDIT )
                    rProgress.SetStateOnPercent( nProgress );

                if (rInner == nIEnd) break;
                if (bPositive) ++rInner; else --rInner;
            }
            rProgress.SetStateOnPercent( nProgress );
        }
        else
        {
            if (!bPositive)
                nInc = -nInc;
            double nEndVal = (nInc>=0.0) ? MAXDOUBLE : -MAXDOUBLE;
            if (bVertical)
                FillSeries( static_cast<SCCOL>(nCol), nRow1,
                        static_cast<SCCOL>(nCol), nRow2, nFillCount, eFillDir,
                        eFillCmd, eDateCmd, nInc, nEndVal, nMinDigits, sal_False,
                        rProgress );
            else
                FillSeries( nCol1, static_cast<SCROW>(nRow), nCol2,
                        static_cast<SCROW>(nRow), nFillCount, eFillDir,
                        eFillCmd, eDateCmd, nInc, nEndVal, nMinDigits, sal_False,
                        rProgress );
            nProgress = rProgress.GetState();
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
    sal_Bool bOk = sal_True;
    long nIndex = 0;
    sal_uLong nSrcCount = 0;
    FillDir eFillDir = FILL_TO_BOTTOM;
    if ( nEndX == nCol2 && nEndY == nRow2 )     // leer
        bOk = sal_False;
    else if ( nEndX == nCol2 )                  // nach oben/unten
    {
        nEndX = nCol2 = nCol1;                  // nur erste Spalte ansehen
        nSrcCount = nRow2 - nRow1 + 1;
        nIndex = ((long)nEndY) - nRow1;         // kann negativ werden
        if ( nEndY >= nRow1 )
            eFillDir = FILL_TO_BOTTOM;
        else
            eFillDir = FILL_TO_TOP;
    }
    else if ( nEndY == nRow2 )                  // nach links/rechts
    {
        nEndY = nRow2 = nRow1;                  // nur erste Zeile ansehen
        nSrcCount = nCol2 - nCol1 + 1;
        nIndex = ((long)nEndX) - nCol1;         // kann negativ werden
        if ( nEndX >= nCol1 )
            eFillDir = FILL_TO_RIGHT;
        else
            eFillDir = FILL_TO_LEFT;
    }
    else                                        // Richtung nicht eindeutig
        bOk = sal_False;

    if ( bOk )
    {
        FillCmd eFillCmd;
        FillDateCmd eDateCmd;
        double nInc;
        sal_uInt16 nMinDigits;
        ScUserListData* pListData = NULL;
        sal_uInt16 nListIndex;

        FillAnalyse(nCol1,nRow1, nCol2,nRow2, eFillCmd,eDateCmd, nInc,nMinDigits, pListData,nListIndex);

        if ( pListData )                            // benutzerdefinierte Liste
        {
            sal_uInt16 nListCount = pListData->GetSubCount();
            if ( nListCount )
            {
                sal_uLong nSub = nSrcCount - 1; //  nListIndex ist vom letzten Source-Eintrag
                while ( nIndex < sal::static_int_cast<long>(nSub) )
                    nIndex += nListCount;
                sal_uLong nPos = ( nListIndex + nIndex - nSub ) % nListCount;
                aValue = pListData->GetSubStr(sal::static_int_cast<sal_uInt16>(nPos));
            }
        }
        else if ( eFillCmd == FILL_SIMPLE )         // Auffuellen mit Muster
        {
            //Add for i89232
            if ((eFillDir == FILL_TO_BOTTOM)||(eFillDir == FILL_TO_TOP))
            {
                long nfilteredrow = 0;
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
                for (long nRowIndex = nBegin; nRowIndex <= nEnd; nRowIndex++)
                {
                    sal_uInt8 nFlags = pRowFlags->GetValue(nRowIndex);
                    if (nFlags & CR_FILTERED)
                        nfilteredrow++;
                }
                if (nIndex >0)
                    nIndex = nIndex - nfilteredrow;
                else
                    nIndex = nIndex + nfilteredrow;
            }
            //End of i89232

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

            ScBaseCell* pCell = GetCell( nSrcX, nSrcY );
            if ( pCell )
            {
                sal_Int32 nDelta;
                if (nIndex >= 0)
                    nDelta = nIndex / nSrcCount;
                else
                    nDelta = ( nIndex - nSrcCount + 1 ) / nSrcCount;    // -1 -> -1

                CellType eType = pCell->GetCellType();
                switch ( eType )
                {
                    case CELLTYPE_STRING:
                    case CELLTYPE_EDIT:
                    {
                        if ( eType == CELLTYPE_STRING )
                            ((ScStringCell*)pCell)->GetString( aValue );
                        else
                            ((ScEditCell*)pCell)->GetString( aValue );
                        if ( !(nScFillModeMouseModifier & KEY_MOD1) && !IsDataFiltered() )  //i89232
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
                        //  dabei kann's keinen Ueberlauf geben...
                        double nVal = ((ScValueCell*)pCell)->GetValue();
                        if ( !(nScFillModeMouseModifier & KEY_MOD1) && !IsDataFiltered() )  //i89232
                            nVal += (double) nDelta;

                        Color* pColor;
                        sal_uLong nNumFmt = GetNumberFormat( nSrcX, nSrcY );
                        pDocument->GetFormatTable()->
                            GetOutputString( nVal, nNumFmt, aValue, &pColor );
                    }
                    break;
                    //  Formeln nicht
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
        }
        else if ( eFillCmd == FILL_LINEAR || eFillCmd == FILL_DATE )        // Werte
        {
            sal_Bool bValueOk;
            double nStart;
            sal_Int32 nVal = 0;
            short nHeadNoneTail = 0;
            ScBaseCell* pCell = GetCell( nCol1, nRow1 );
            if ( pCell )
            {
                CellType eType = pCell->GetCellType();
                switch ( eType )
                {
                    case CELLTYPE_STRING:
                    case CELLTYPE_EDIT:
                    {
                        if ( eType == CELLTYPE_STRING )
                            ((ScStringCell*)pCell)->GetString( aValue );
                        else
                            ((ScEditCell*)pCell)->GetString( aValue );
                        nHeadNoneTail = lcl_DecompValueString( aValue, nVal );
                        if ( nHeadNoneTail )
                            nStart = (double)nVal;
                        else
                            nStart = 0.0;
                    }
                    break;
                    case CELLTYPE_VALUE:
                        nStart = ((ScValueCell*)pCell)->GetValue();
                    break;
                    case CELLTYPE_FORMULA:
                        nStart = ((ScFormulaCell*)pCell)->GetValue();
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
            else        // Datum
            {
                bValueOk = sal_True;
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
                    pDocument->GetFormatTable()->
                        GetOutputString( nStart, nNumFmt, aValue, &pColor );
                }
            }
        }
        else
        {
            DBG_ERROR("GetAutoFillPreview: falscher Modus");
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

    // class Date Grenzen
    const sal_uInt16 nMinYear = 1583;
    const sal_uInt16 nMaxYear = 9956;

    long nInc = (long) nStep;       // nach oben/unten begrenzen ?
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
                        long nYAdd = 1 - nMonth / 12;       // positiv
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
                    if ( nDayOfMonth > 28 )
                        aDate.SetDay( Min( aDate.GetDaysInMonth(), nDayOfMonth ) );
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

void ScTable::FillSeries( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                    double nStepValue, double nMaxValue, sal_uInt16 nArgMinDigits,
                    sal_Bool bAttribs, ScProgress& rProgress )
{
    //
    //  Richtung auswerten
    //

    sal_Bool bVertical = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP);
    sal_Bool bPositive = (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_RIGHT);

    sal_uLong nCol = 0;
    sal_uLong nRow = 0;
    sal_uLong& rInner = bVertical ? nRow : nCol;        // Schleifenvariablen
    sal_uLong& rOuter = bVertical ? nCol : nRow;
    sal_uLong nOStart;
    sal_uLong nOEnd;
    sal_uLong nIStart;
    sal_uLong nIEnd;
    sal_uLong nISource;

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
        }
        else
        {
            nISource = nRow2;
            nIStart = nRow2 - 1;
            nIEnd = nRow2 - nFillCount;
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
        }
        else
        {
            nISource = nCol2;
            nIStart = nCol2 - 1;
            nIEnd = nCol2 - nFillCount;
        }
    }

    sal_uLong nIMin = nIStart;
    sal_uLong nIMax = nIEnd;
    PutInOrder(nIMin,nIMax);
    sal_uInt16 nDel = bAttribs ? IDF_AUTOFILL : (IDF_AUTOFILL & IDF_CONTENTS);
    if (bVertical)
        DeleteArea(nCol1, static_cast<SCROW>(nIMin), nCol2, static_cast<SCROW>(nIMax), nDel);
    else
        DeleteArea(static_cast<SCCOL>(nIMin), nRow1, static_cast<SCCOL>(nIMax), nRow2, nDel);

    sal_uLong nProgress = rProgress.GetState();

    //
    //  ausfuehren
    //

    sal_uLong nActFormCnt = 0;
    for (rOuter = nOStart; rOuter <= nOEnd; rOuter++)
    {
        sal_Bool bFirst = sal_True;
        rInner = nISource;
        ScBaseCell* pSrcCell = aCol[nCol].GetCell(static_cast<SCROW>(nRow));

        if (bVertical && bAttribs)
            aCol[nCol].Resize( aCol[nCol].GetCellCount() + nFillCount );

        if (bAttribs)
        {
            const ScPatternAttr* pSrcPattern = aCol[nCol].GetPattern(static_cast<SCROW>(nRow));
            if (bVertical)
                aCol[nCol].SetPatternArea( static_cast<SCROW>(nIMin),
                        static_cast<SCROW>(nIMax), *pSrcPattern, sal_True );
            else
                for (SCCOL nAtCol = static_cast<SCCOL>(nIMin); nAtCol <= sal::static_int_cast<SCCOL>(nIMax); nAtCol++)
                    aCol[nAtCol].SetPattern(static_cast<SCROW>(nRow), *pSrcPattern, sal_True);
        }

        if (pSrcCell)
        {
            CellType eCellType = pSrcCell->GetCellType();

            if (eFillCmd == FILL_SIMPLE)                // kopieren
            {
                if (eCellType == CELLTYPE_FORMULA)
                {
                    for (rInner = nIMin; rInner <= nIMax; rInner++)
                    {
                        sal_uLong nInd = nActFormCnt;
                        FillFormula(nInd, bFirst, (ScFormulaCell*)pSrcCell,
                            static_cast<SCCOL>(nCol), nRow, (rInner == nIEnd) );
                        bFirst = sal_False;
                        rProgress.SetStateOnPercent( ++nProgress );
                    }
                }
                else if (eCellType != CELLTYPE_NOTE)
                {
                    for (rInner = nIMin; rInner <= nIMax; rInner++)
                    {
                        ScAddress aDestPos( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), nTab );
                        aCol[nCol].Insert( aDestPos.Row(), pSrcCell->CloneWithoutNote( *pDocument ) );
                    }
                    nProgress += nIMax - nIMin + 1;
                    rProgress.SetStateOnPercent( nProgress );
                }
            }
            else if (eCellType == CELLTYPE_VALUE || eCellType == CELLTYPE_FORMULA)
            {
                double nStartVal;
                if (eCellType == CELLTYPE_VALUE)
                    nStartVal = ((ScValueCell*)pSrcCell)->GetValue();
                else
                    nStartVal = ((ScFormulaCell*)pSrcCell)->GetValue();
                double nVal = nStartVal;
                long nIndex = 0;

                sal_Bool bError = sal_False;
                sal_Bool bOverflow = sal_False;

                sal_uInt16 nDayOfMonth = 0;
                rInner = nIStart;
                while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
                {
                    if (!bError && !bOverflow)
                    {
                        switch (eFillCmd)
                        {
                            case FILL_LINEAR:
                                {
                                    //  #86365# use multiplication instead of repeated addition
                                    //  to avoid accumulating rounding errors
                                    nVal = nStartVal;
                                    double nAdd = nStepValue;
                                    if ( !SubTotal::SafeMult( nAdd, (double) ++nIndex ) ||
                                         !SubTotal::SafePlus( nVal, nAdd ) )
                                        bError = sal_True;
                                }
                                break;
                            case FILL_GROWTH:
                                if (!SubTotal::SafeMult(nVal, nStepValue))
                                    bError = sal_True;
                                break;
                            case FILL_DATE:
                                if (fabs(nVal) > _D_MAX_LONG_)
                                    bError = sal_True;
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
                            if (nVal > nMaxValue)           // Zielwert erreicht?
                            {
                                nVal = nMaxValue;
                                bOverflow = sal_True;
                            }
                        }
                        else
                        {
                            if (nVal < nMaxValue)
                            {
                                nVal = nMaxValue;
                                bOverflow = sal_True;
                            }
                        }
                    }

                    if (bError)
                        aCol[nCol].SetError(static_cast<SCROW>(nRow), errNoValue);
                    else if (!bOverflow)
                        aCol[nCol].SetValue(static_cast<SCROW>(nRow), nVal);

                    if (rInner == nIEnd) break;
                    if (bPositive) ++rInner; else --rInner;
                }
                nProgress += nIMax - nIMin + 1;
                rProgress.SetStateOnPercent( nProgress );
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
                    ((ScStringCell*)pSrcCell)->GetString( aValue );
                else
                    ((ScEditCell*)pSrcCell)->GetString( aValue );
                sal_Int32 nStringValue;
                sal_uInt16 nMinDigits = nArgMinDigits;
                short nHeadNoneTail = lcl_DecompValueString( aValue, nStringValue, &nMinDigits );
                if ( nHeadNoneTail )
                {
                    double nStartVal = (double)nStringValue;
                    double nVal = nStartVal;
                    long nIndex = 0;
                    sal_Bool bError = sal_False;
                    sal_Bool bOverflow = sal_False;

                    sal_Bool bIsOrdinalSuffix = aValue.Equals( ScGlobal::GetOrdinalSuffix(
                                (sal_Int32)nStartVal));

                    rInner = nIStart;
                    while (true)        // #i53728# with "for (;;)" old solaris/x86 compiler mis-optimizes
                    {
                        if (!bError && !bOverflow)
                        {
                            switch (eFillCmd)
                            {
                                case FILL_LINEAR:
                                    {
                                        //  #86365# use multiplication instead of repeated addition
                                        //  to avoid accumulating rounding errors
                                        nVal = nStartVal;
                                        double nAdd = nStepValue;
                                        if ( !SubTotal::SafeMult( nAdd, (double) ++nIndex ) ||
                                             !SubTotal::SafePlus( nVal, nAdd ) )
                                            bError = sal_True;
                                    }
                                    break;
                                case FILL_GROWTH:
                                    if (!SubTotal::SafeMult(nVal, nStepValue))
                                        bError = sal_True;
                                    break;
                                default:
                                {
                                    // added to avoid warnings
                                }
                            }

                            if (nStepValue >= 0)
                            {
                                if (nVal > nMaxValue)           // Zielwert erreicht?
                                {
                                    nVal = nMaxValue;
                                    bOverflow = sal_True;
                                }
                            }
                            else
                            {
                                if (nVal < nMaxValue)
                                {
                                    nVal = nMaxValue;
                                    bOverflow = sal_True;
                                }
                            }
                        }

                        if (bError)
                            aCol[nCol].SetError(static_cast<SCROW>(nRow), errNoValue);
                        else if (!bOverflow)
                        {
                            nStringValue = (sal_Int32)nVal;
                            String aStr;
                            if ( nHeadNoneTail < 0 )
                            {
                                aCol[nCol].Insert( static_cast<SCROW>(nRow),
                                        lcl_getSuffixCell( pDocument,
                                            nStringValue, nMinDigits, aValue,
                                            eCellType, bIsOrdinalSuffix ));
                            }
                            else
                            {
                                aStr = aValue;
                                aStr += lcl_ValueString( nStringValue, nMinDigits );
                                ScStringCell* pCell = new ScStringCell( aStr );
                                aCol[nCol].Insert( static_cast<SCROW>(nRow), pCell );
                            }
                        }

                        if (rInner == nIEnd) break;
                        if (bPositive) ++rInner; else --rInner;
                    }
                }
                nProgress += nIMax - nIMin + 1;
                rProgress.SetStateOnPercent( nProgress );
            }
        }
        else
        {
            nProgress += nIMax - nIMin + 1;
            rProgress.SetStateOnPercent( nProgress );
        }
        ++nActFormCnt;
    }
}

void ScTable::Fill( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                    double nStepValue, double nMaxValue)
{
    sal_uLong nProgCount;
    if (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP)
        nProgCount = nCol2 - nCol1 + 1;
    else
        nProgCount = nRow2 - nRow1 + 1;
    nProgCount *= nFillCount;
    ScProgress aProgress( pDocument->GetDocumentShell(),
                            ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

    bSharedNameInserted = sal_False;

    if (eFillCmd == FILL_AUTO)
        FillAuto(nCol1, nRow1, nCol2, nRow2, nFillCount, eFillDir, aProgress);
    else
        FillSeries(nCol1, nRow1, nCol2, nRow2, nFillCount, eFillDir,
                    eFillCmd, eFillDateCmd, nStepValue, nMaxValue, 0, sal_True, aProgress);

    if (bSharedNameInserted)                        // Wurde Shared-Name eingefuegt?
        pDocument->GetRangeName()->SetSharedMaxIndex(
            pDocument->GetRangeName()->GetSharedMaxIndex()+1);  // dann hochzaehlen
}


void ScTable::AutoFormatArea(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                const ScPatternAttr& rAttr, sal_uInt16 nFormatNo)
{
    ScAutoFormat* pAutoFormat = ScGlobal::GetAutoFormat();
    if (pAutoFormat)
    {
        ScAutoFormatData* pData = (*pAutoFormat)[nFormatNo];
        if (pData)
        {
//          ScPatternAttr aPattern(pDocument->GetPool());
//            pData->FillToItemSet(nIndex, aPattern.GetItemSet(), *pDocument);
            ApplyPatternArea(nStartCol, nStartRow, nEndCol, nEndRow, rAttr);
        }
    }
}

void ScTable::AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            sal_uInt16 nFormatNo )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        ScAutoFormat* pAutoFormat = ScGlobal::GetAutoFormat();
        if (pAutoFormat)
        {
            ScAutoFormatData* pData = (*pAutoFormat)[nFormatNo];
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
                // Linke obere Ecke
                AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
                // Linke Spalte
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
                // Linke untere Ecke
                nRow = nEndRow;
                nIndex = 12;
                AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
                // Rechte obere Ecke
                nCol = nEndCol;
                nRow = nStartRow;
                nIndex = 3;
                AutoFormatArea(nCol, nRow, nCol, nRow, *pPatternAttrs[nIndex], nFormatNo);
                // Rechte Spalte
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
                // Rechte untere Ecke
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
                // Untere Zeile
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
        } // if AutoFormat != NULL
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
            // Linke obere Ecke
            GetAutoFormatAttr(nStartCol, nStartRow, 0, rData);
            GetAutoFormatFrame(nStartCol, nStartRow, LF_ALL, 0, rData);
            // Linke Spalte
            GetAutoFormatAttr(nStartCol, nStartRow + 1, 4, rData);
            GetAutoFormatAttr(nStartCol, nStartRow + 2, 8, rData);
            GetAutoFormatFrame(nStartCol, nStartRow + 1, LF_LEFT | LF_RIGHT | LF_BOTTOM, 4, rData);
            if (nEndRow - nStartRow >= 4)
                GetAutoFormatFrame(nStartCol, nStartRow + 2, LF_LEFT | LF_RIGHT | LF_BOTTOM, 8, rData);
            else
                rData.CopyItem( 8, 4, ATTR_BORDER );
            // Linke untere Ecke
            GetAutoFormatAttr(nStartCol, nEndRow, 12, rData);
            GetAutoFormatFrame(nStartCol, nEndRow, LF_ALL, 12, rData);
            // Rechte obere Ecke
            GetAutoFormatAttr(nEndCol, nStartRow, 3, rData);
            GetAutoFormatFrame(nEndCol, nStartRow, LF_ALL, 3, rData);
            // Rechte Spalte
            GetAutoFormatAttr(nEndCol, nStartRow + 1, 7, rData);
            GetAutoFormatAttr(nEndCol, nStartRow + 2, 11, rData);
            GetAutoFormatFrame(nEndCol, nStartRow + 1, LF_LEFT | LF_RIGHT | LF_BOTTOM, 7, rData);
            if (nEndRow - nStartRow >= 4)
                GetAutoFormatFrame(nEndCol, nStartRow + 2, LF_LEFT | LF_RIGHT | LF_BOTTOM, 11, rData);
            else
                rData.CopyItem( 11, 7, ATTR_BORDER );
            // Rechte untere Ecke
            GetAutoFormatAttr(nEndCol, nEndRow, 15, rData);
            GetAutoFormatFrame(nEndCol, nEndRow, LF_ALL, 15, rData);
            // Ober Zeile
            GetAutoFormatAttr(nStartCol + 1, nStartRow, 1, rData);
            GetAutoFormatAttr(nStartCol + 2, nStartRow, 2, rData);
            GetAutoFormatFrame(nStartCol + 1, nStartRow, LF_TOP | LF_BOTTOM | LF_RIGHT, 1, rData);
            if (nEndCol - nStartCol >= 4)
                GetAutoFormatFrame(nStartCol + 2, nStartRow, LF_TOP | LF_BOTTOM | LF_RIGHT, 2, rData);
            else
                rData.CopyItem( 2, 1, ATTR_BORDER );
            // Untere Zeile
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

//UNUSED2008-05  sal_uInt16 ScTable::GetErrorData( SCCOL nCol, SCROW nRow ) const
//UNUSED2008-05  {
//UNUSED2008-05      if (ValidColRow(nCol,nRow))
//UNUSED2008-05          return aCol[nCol].GetErrorData( nRow );
//UNUSED2008-05      else
//UNUSED2008-05          return 0;
//UNUSED2008-05  }

sal_Bool ScTable::GetNextSpellingCell(SCCOL& rCol, SCROW& rRow, sal_Bool bInSel,
                                    const ScMarkData& rMark) const
{
    if (rRow == MAXROW+2)                       // Tabellenende
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
        return sal_True;
    else
    {
        sal_Bool bStop = sal_False;
        while (!bStop)
        {
            if (ValidCol(rCol))
            {
                bStop = aCol[rCol].GetNextSpellingCell(rRow, bInSel, rMark);
                if (bStop)
                    return sal_True;
                else /*if (rRow == MAXROW+1) */
                {
                    rCol++;
                    rRow = 0;
                }
            }
            else
                return sal_True;
        }
    }
    return sal_False;
}

void ScTable::RemoveAutoSpellObj()
{
    for (SCCOL i=0; i <= MAXCOL; i++)
        aCol[i].RemoveAutoSpellObj();
}

sal_Bool ScTable::TestTabRefAbs(SCTAB nTable)
{
    sal_Bool bRet = sal_False;
    for (SCCOL i=0; i <= MAXCOL; i++)
        if (aCol[i].TestTabRefAbs(nTable))
            bRet = sal_True;
    return bRet;
}

void ScTable::CompileDBFormula()
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CompileDBFormula();
}

void ScTable::CompileDBFormula( sal_Bool bCreateFormulaString )
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CompileDBFormula( bCreateFormulaString );
}

void ScTable::CompileNameFormula( sal_Bool bCreateFormulaString )
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CompileNameFormula( bCreateFormulaString );
}

void ScTable::CompileColRowNameFormula()
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CompileColRowNameFormula();
}






