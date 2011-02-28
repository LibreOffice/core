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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <svl/zforlist.hxx>

#include "scitems.hxx"
#include "global.hxx"
#include "dociter.hxx"
#include "document.hxx"
#include "table.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "docoptio.hxx"
#include "cellform.hxx"

#include <vector>

using ::rtl::math::approxEqual;
using ::std::vector;
using ::rtl::OUString;
using ::std::set;

// STATIC DATA -----------------------------------------------------------

namespace {

void lcl_toUpper(OUString& rStr)
{
    rStr = ScGlobal::pCharClass->toUpper(rStr.trim(), 0, static_cast<sal_uInt16>(rStr.getLength()));
}

}

ScDocumentIterator::ScDocumentIterator( ScDocument* pDocument,
                            SCTAB nStartTable, SCTAB nEndTable ) :
    pDoc( pDocument ),
    nStartTab( nStartTable ),
    nEndTab( nEndTable )
{
    PutInOrder( nStartTab, nEndTab );
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

    pDefPattern = pDoc->GetDefPattern();

    nCol = 0;
    nRow = 0;
    nTab = nStartTab;

    nColPos = 0;
    nAttrPos = 0;
}

ScDocumentIterator::~ScDocumentIterator()
{
}

sal_Bool ScDocumentIterator::GetThisCol()
{
    ScTable*        pTab;
    while ( (pTab = pDoc->pTab[nTab]) == NULL )
    {
        if ( nTab == nEndTab )
        {
            nCol = MAXCOL;
            nRow = MAXROW;
            return sal_False;
        }
        ++nTab;
    }
    ScColumn*       pCol = &pTab->aCol[nCol];
    ScAttrArray*    pAtt = pCol->pAttrArray;

    sal_Bool bFound = sal_False;
    do
    {
        SCROW nColRow;
        SCROW nAttrEnd;

        do
        {
            nAttrEnd = pAtt->pData[nAttrPos].nRow;
            if (nAttrEnd < nRow)
                ++nAttrPos;
        }
        while (nAttrEnd < nRow);

        do
        {
            nColRow = (nColPos < pCol->nCount) ? pCol->pItems[nColPos].nRow : MAXROW+1;
            if (nColRow < nRow)
                ++nColPos;
        }
        while (nColRow < nRow);

        if (nColRow == nRow)
        {
            bFound   = sal_True;
            pCell    = pCol->pItems[nColPos].pCell;
            pPattern = pAtt->pData[nAttrPos].pPattern;
        }
        else if ( pAtt->pData[nAttrPos].pPattern != pDefPattern )
        {
            bFound = sal_True;
            pCell = NULL;
            pPattern = pAtt->pData[nAttrPos].pPattern;
        }
        else
        {
            nRow = Min( (SCROW)nColRow, (SCROW)(nAttrEnd+1) );
        }
    }
    while (!bFound && nRow <= MAXROW);

    return bFound;
}

sal_Bool ScDocumentIterator::GetThis()
{
    sal_Bool bEnd = sal_False;
    sal_Bool bSuccess = sal_False;

    while ( !bSuccess && !bEnd )
    {
        if ( nRow > MAXROW )
            bSuccess = sal_False;
        else
            bSuccess = GetThisCol();

        if ( !bSuccess )
        {
            ++nCol;
            if (nCol > MAXCOL)
            {
                nCol = 0;
                ++nTab;
                if (nTab > nEndTab)
                    bEnd = sal_True;
            }
            nRow = 0;
            nColPos = 0;
            nAttrPos = 0;
        }
    }

    return !bEnd;
}

sal_Bool ScDocumentIterator::GetFirst()
{
    nCol = 0;
    nTab = nStartTab;

    nRow = 0;
    nColPos = 0;
    nAttrPos = 0;

    return GetThis();
}

sal_Bool ScDocumentIterator::GetNext()
{
    ++nRow;

    return GetThis();
}

//------------------------------------------------------------------------

ScBaseCell* ScDocumentIterator::GetCell()
{
    return pCell;
}

const ScPatternAttr* ScDocumentIterator::GetPattern()
{
    return pPattern;
}

void ScDocumentIterator::GetPos( SCCOL& rCol, SCROW& rRow, SCTAB& rTab )
{
    rCol = nCol;
    rRow = nRow;
    rTab = nTab;
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------
void lcl_IterGetNumberFormat( sal_uLong& nFormat, const ScAttrArray*& rpArr,
        SCROW& nAttrEndRow, const ScAttrArray* pNewArr, SCROW nRow,
        ScDocument* pDoc )
{
    if ( rpArr != pNewArr || nAttrEndRow < nRow )
    {
        SCSIZE nPos;
        pNewArr->Search( nRow, nPos );  // nPos 0 gueltig wenn nicht gefunden
        const ScPatternAttr* pPattern = pNewArr->pData[nPos].pPattern;
        nFormat = pPattern->GetNumberFormat( pDoc->GetFormatTable() );
        rpArr = pNewArr;
        nAttrEndRow = pNewArr->pData[nPos].nRow;
    }
}

//UNUSED2008-05  ScValueIterator::ScValueIterator( ScDocument* pDocument,
//UNUSED2008-05                                    SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
//UNUSED2008-05                                    SCCOL nECol, SCROW nERow, SCTAB nETab,
//UNUSED2008-05                                    sal_Bool bSTotal, sal_Bool bTextZero ) :
//UNUSED2008-05  pDoc( pDocument ),
//UNUSED2008-05  nNumFmtIndex(0),
//UNUSED2008-05  nStartCol( nSCol),
//UNUSED2008-05  nStartRow( nSRow),
//UNUSED2008-05  nStartTab( nSTab ),
//UNUSED2008-05  nEndCol( nECol ),
//UNUSED2008-05  nEndRow( nERow),
//UNUSED2008-05  nEndTab( nETab ),
//UNUSED2008-05  nNumFmtType( NUMBERFORMAT_UNDEFINED ),
//UNUSED2008-05  bNumValid( sal_False ),
//UNUSED2008-05  bSubTotal(bSTotal),
//UNUSED2008-05  bNextValid( sal_False ),
//UNUSED2008-05  bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
//UNUSED2008-05  bTextAsZero( bTextZero )
//UNUSED2008-05  {
//UNUSED2008-05      PutInOrder( nStartCol, nEndCol);
//UNUSED2008-05      PutInOrder( nStartRow, nEndRow);
//UNUSED2008-05      PutInOrder( nStartTab, nEndTab );
//UNUSED2008-05
//UNUSED2008-05      if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
//UNUSED2008-05      if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
//UNUSED2008-05      if (!ValidRow(nStartRow)) nStartRow = MAXROW;
//UNUSED2008-05      if (!ValidRow(nEndRow)) nEndRow = MAXROW;
//UNUSED2008-05      if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
//UNUSED2008-05      if (!ValidTab(nEndTab)) nEndTab = MAXTAB;
//UNUSED2008-05
//UNUSED2008-05      nCol = nStartCol;
//UNUSED2008-05      nRow = nStartRow;
//UNUSED2008-05      nTab = nStartTab;
//UNUSED2008-05
//UNUSED2008-05      nColRow = 0;                    // wird bei GetFirst initialisiert
//UNUSED2008-05
//UNUSED2008-05      nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
//UNUSED2008-05      pAttrArray = 0;
//UNUSED2008-05      nAttrEndRow = 0;
//UNUSED2008-05  }

ScValueIterator::ScValueIterator( ScDocument* pDocument, const ScRange& rRange,
            sal_Bool bSTotal, sal_Bool bTextZero ) :
    pDoc( pDocument ),
    nNumFmtIndex(0),
    nStartCol( rRange.aStart.Col() ),
    nStartRow( rRange.aStart.Row() ),
    nStartTab( rRange.aStart.Tab() ),
    nEndCol( rRange.aEnd.Col() ),
    nEndRow( rRange.aEnd.Row() ),
    nEndTab( rRange.aEnd.Tab() ),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    bNumValid( sal_False ),
    bSubTotal(bSTotal),
    bNextValid( sal_False ),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
    bTextAsZero( bTextZero )
{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;

    nColRow = 0;                    // wird bei GetFirst initialisiert

    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;
}

sal_Bool ScValueIterator::GetThis(double& rValue, sal_uInt16& rErr)
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    for (;;)
    {
        if ( nRow > nEndRow )
        {
            nRow = nStartRow;
            do
            {
                nCol++;
                if ( nCol > nEndCol )
                {
                    nCol = nStartCol;
                    nTab++;
                    if ( nTab > nEndTab )
                    {
                        // rValue = 0.0;    //! do not change caller's value!
                        rErr = 0;
                        return sal_False;               // Ende und Aus
                    }
                }
                pCol = &(pDoc->pTab[nTab])->aCol[nCol];
            } while ( pCol->nCount == 0 );
            pCol->Search( nRow, nColRow );
        }

        while (( nColRow < pCol->nCount ) && ( pCol->pItems[nColRow].nRow < nRow ))
            nColRow++;

        if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= nEndRow )
        {
            nRow = pCol->pItems[nColRow].nRow + 1;
            if ( !bSubTotal || !pDoc->pTab[nTab]->RowFiltered( nRow-1 ) )
            {
                ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
                ++nColRow;
                switch (pCell->GetCellType())
                {
                    case CELLTYPE_VALUE:
                    {
                        bNumValid = sal_False;
                        rValue = ((ScValueCell*)pCell)->GetValue();
                        rErr = 0;
                        --nRow;
                        if ( bCalcAsShown )
                        {
                            lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
                                nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
                            rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
                        }
                        //
                        //  wenn in der selben Spalte gleich noch eine Value-Cell folgt, die
                        //  auch noch im Block liegt, den Wert jetzt schon holen
                        //
                        if ( nColRow < pCol->nCount &&
                             pCol->pItems[nColRow].nRow <= nEndRow &&
                             pCol->pItems[nColRow].pCell->GetCellType() == CELLTYPE_VALUE &&
                             !bSubTotal )
                        {
                            fNextValue = ((ScValueCell*)pCol->pItems[nColRow].pCell)->GetValue();
                            nNextRow = pCol->pItems[nColRow].nRow;
                            bNextValid = sal_True;
                            if ( bCalcAsShown )
                            {
                                lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
                                    nAttrEndRow, pCol->pAttrArray, nNextRow, pDoc );
                                fNextValue = pDoc->RoundValueAsShown( fNextValue, nNumFormat );
                            }
                        }

                        return sal_True;                                    // gefunden
                    }
//                    break;
                    case CELLTYPE_FORMULA:
                    {
                        if (!bSubTotal || !((ScFormulaCell*)pCell)->IsSubTotal())
                        {
                            rErr = ((ScFormulaCell*)pCell)->GetErrCode();
                            if ( rErr || ((ScFormulaCell*)pCell)->IsValue() )
                            {
                                rValue = ((ScFormulaCell*)pCell)->GetValue();
                                nRow--;
                                bNumValid = sal_False;
                                return sal_True;                            // gefunden
                            }
                            else if ( bTextAsZero )
                            {
                                rValue = 0.0;
                                nRow--;
                                bNumValid = sal_False;
                                return sal_True;
                            }
                        }
                    }
                    break;
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                    {
                        if ( bTextAsZero )
                        {
                            rErr = 0;
                            rValue = 0.0;
                            nNumFmtType = NUMBERFORMAT_NUMBER;
                            nNumFmtIndex = 0;
                            bNumValid = sal_True;
                            --nRow;
                            return sal_True;
                        }
                    }
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
        }
        else
            nRow = nEndRow + 1;         // naechste Spalte
    }
}

void ScValueIterator::GetCurNumFmtInfo( short& nType, sal_uLong& nIndex )
{
    if (!bNumValid)
    {
        const ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
        nNumFmtIndex = pCol->GetNumberFormat( nRow );
        if ( (nNumFmtIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
        {
            const ScBaseCell* pCell;
            SCSIZE nIdx = nColRow - 1;
            // there might be rearranged something, so be on the safe side
            if ( nIdx < pCol->nCount && pCol->pItems[nIdx].nRow == nRow )
                pCell = pCol->pItems[nIdx].pCell;
            else
            {
                if ( pCol->Search( nRow, nIdx ) )
                    pCell = pCol->pItems[nIdx].pCell;
                else
                    pCell = NULL;
            }
            if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
                ((const ScFormulaCell*)pCell)->GetFormatInfo( nNumFmtType, nNumFmtIndex );
            else
                nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
        }
        else
            nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
        bNumValid = sal_True;
    }
    nType = nNumFmtType;
    nIndex = nNumFmtIndex;
}

sal_Bool ScValueIterator::GetFirst(double& rValue, sal_uInt16& rErr)
{
    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;

//  nColRow = 0;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    pCol->Search( nRow, nColRow );

    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;

    return GetThis(rValue, rErr);
}

/*  ist inline:
sal_Bool ScValueIterator::GetNext(double& rValue, sal_uInt16& rErr)
{
    ++nRow;
    return GetThis(rValue, rErr);
}
*/

// ============================================================================

ScDBQueryDataIterator::DataAccess::DataAccess(const ScDBQueryDataIterator* pParent) :
    mpParent(pParent)
{
}

ScDBQueryDataIterator::DataAccess::~DataAccess()
{
}

SCROW ScDBQueryDataIterator::GetRowByColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCCOL nCol, SCSIZE nColRow)
{
    ScColumn* pCol = &rDoc.pTab[nTab]->aCol[nCol];
    return pCol->pItems[nColRow].nRow;
}

ScBaseCell* ScDBQueryDataIterator::GetCellByColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCCOL nCol, SCSIZE nColRow)
{
    ScColumn* pCol = &rDoc.pTab[nTab]->aCol[nCol];
    return pCol->pItems[nColRow].pCell;
}

ScAttrArray* ScDBQueryDataIterator::GetAttrArrayByCol(ScDocument& rDoc, SCTAB nTab, SCCOL nCol)
{
    ScColumn* pCol = &rDoc.pTab[nTab]->aCol[nCol];
    return pCol->pAttrArray;
}

bool ScDBQueryDataIterator::IsQueryValid(ScDocument& rDoc, const ScQueryParam& rParam, SCTAB nTab, SCROW nRow, ScBaseCell* pCell)
{
    return rDoc.pTab[nTab]->ValidQuery(nRow, rParam, NULL, pCell);
}

SCSIZE ScDBQueryDataIterator::SearchColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCROW nRow, SCCOL nCol)
{
    ScColumn* pCol = &rDoc.pTab[nTab]->aCol[nCol];
    SCSIZE nColRow;
    pCol->Search(nRow, nColRow);
    return nColRow;
}

// ----------------------------------------------------------------------------

ScDBQueryDataIterator::DataAccessInternal::DataAccessInternal(const ScDBQueryDataIterator* pParent, ScDBQueryParamInternal* pParam, ScDocument* pDoc) :
    DataAccess(pParent),
    mpParam(pParam),
    mpDoc(pDoc),
    bCalcAsShown( pDoc->GetDocOptions().IsCalcAsShown() )
{
    nCol = mpParam->mnField;
    nRow = mpParam->nRow1;
    nTab = mpParam->nTab;

    nColRow = 0;                    // wird bei GetFirst initialisiert
    SCSIZE i;
    SCSIZE nCount = mpParam->GetEntryCount();
    for (i=0; (i<nCount) && (mpParam->GetEntry(i).bDoQuery); i++)
    {
        ScQueryEntry& rEntry = mpParam->GetEntry(i);
        sal_uInt32 nIndex = 0;
        rEntry.bQueryByString =
            !(mpDoc->GetFormatTable()->IsNumberFormat(*rEntry.pStr, nIndex, rEntry.nVal));
    }
    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;
}

ScDBQueryDataIterator::DataAccessInternal::~DataAccessInternal()
{
}

bool ScDBQueryDataIterator::DataAccessInternal::getCurrent(Value& rValue)
{
    SCCOLROW nFirstQueryField = mpParam->GetEntry(0).nField;
    for ( ;; )
    {
        if (nRow > mpParam->nRow2)
        {
            // Bottom of the range reached.  Bail out.
            rValue.mnError = 0;
            return false;
        }

        SCSIZE nCellCount = mpDoc->GetCellCount(nTab, nCol);
        SCROW nThisRow = ScDBQueryDataIterator::GetRowByColEntryIndex(*mpDoc, nTab, nCol, nColRow);
        while ( (nColRow < nCellCount) && (nThisRow < nRow) )
            nThisRow = ScDBQueryDataIterator::GetRowByColEntryIndex(*mpDoc, nTab, nCol, ++nColRow);

        if ( nColRow < nCellCount && nThisRow <= mpParam->nRow2 )
        {
            nRow = nThisRow;
            ScBaseCell* pCell = NULL;
            if (nCol == static_cast<SCCOL>(nFirstQueryField))
                pCell = ScDBQueryDataIterator::GetCellByColEntryIndex(*mpDoc, nTab, nCol, nColRow);

            if (ScDBQueryDataIterator::IsQueryValid(*mpDoc, *mpParam, nTab, nRow, pCell))
            {
                // #i109812# get cell here if it wasn't done above
                if (nCol != static_cast<SCCOL>(nFirstQueryField))
                    pCell = ScDBQueryDataIterator::GetCellByColEntryIndex(*mpDoc, nTab, nCol, nColRow);

                switch (pCell ? pCell->GetCellType() : CELLTYPE_NONE)
                {
                    case CELLTYPE_VALUE:
                        {
                            rValue.mfValue = ((ScValueCell*)pCell)->GetValue();
                            rValue.mbIsNumber = true;
                            if ( bCalcAsShown )
                            {
                                const ScAttrArray* pNewAttrArray =
                                    ScDBQueryDataIterator::GetAttrArrayByCol(*mpDoc, nTab, nCol);
                                lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
                                    nAttrEndRow, pNewAttrArray, nRow, mpDoc );
                                rValue.mfValue = mpDoc->RoundValueAsShown( rValue.mfValue, nNumFormat );
                            }
                            nNumFmtType = NUMBERFORMAT_NUMBER;
                            nNumFmtIndex = 0;
                            rValue.mnError = 0;
                            return sal_True;        // gefunden
                        }
//                        break;
                    case CELLTYPE_FORMULA:
                        {
                            if (((ScFormulaCell*)pCell)->IsValue())
                            {
                                rValue.mfValue = ((ScFormulaCell*)pCell)->GetValue();
                                rValue.mbIsNumber = true;
                                mpDoc->GetNumberFormatInfo( nNumFmtType,
                                    nNumFmtIndex, ScAddress( nCol, nRow, nTab ),
                                    pCell );
                                rValue.mnError = ((ScFormulaCell*)pCell)->GetErrCode();
                                return sal_True;    // gefunden
                            }
                            else
                                nRow++;
                        }
                        break;
                    case CELLTYPE_STRING:
                    case CELLTYPE_EDIT:
                        if (mpParam->mbSkipString)
                            ++nRow;
                        else
                        {
                            rValue.maString = pCell->GetStringData();
                            rValue.mfValue = 0.0;
                            rValue.mnError = 0;
                            rValue.mbIsNumber = false;
                            return true;
                        }
                        break;
                    default:
                        nRow++;
                        break;
                }
            }
            else
                nRow++;
        }
        else
            nRow = mpParam->nRow2 + 1; // Naechste Spalte
    }
// statement unreachable
//    return false;
}

bool ScDBQueryDataIterator::DataAccessInternal::getFirst(Value& rValue)
{
    if (mpParam->bHasHeader)
        nRow++;

    nColRow = ScDBQueryDataIterator::SearchColEntryIndex(*mpDoc, nTab, nRow, nCol);
    return getCurrent(rValue);
}

bool ScDBQueryDataIterator::DataAccessInternal::getNext(Value& rValue)
{
    ++nRow;
    return getCurrent(rValue);
}

// ----------------------------------------------------------------------------

ScDBQueryDataIterator::DataAccessMatrix::DataAccessMatrix(const ScDBQueryDataIterator* pParent, ScDBQueryParamMatrix* pParam) :
    DataAccess(pParent),
    mpParam(pParam)
{
    SCSIZE nC, nR;
    mpParam->mpMatrix->GetDimensions(nC, nR);
    mnRows = static_cast<SCROW>(nR);
    mnCols = static_cast<SCCOL>(nC);
}

ScDBQueryDataIterator::DataAccessMatrix::~DataAccessMatrix()
{
}

bool ScDBQueryDataIterator::DataAccessMatrix::getCurrent(Value& rValue)
{
    // Starting from row == mnCurRow, get the first row that satisfies all the
    // query parameters.
    for ( ;mnCurRow < mnRows; ++mnCurRow)
    {
        const ScMatrix& rMat = *mpParam->mpMatrix;
        if (rMat.IsEmpty(mpParam->mnField, mnCurRow))
            // Don't take empty values into account.
            continue;

        bool bIsStrVal = rMat.IsString(mpParam->mnField, mnCurRow);
        if (bIsStrVal && mpParam->mbSkipString)
            continue;

        if (isValidQuery(mnCurRow, rMat))
        {
            rValue.maString = rMat.GetString(mpParam->mnField, mnCurRow);
            rValue.mfValue = rMat.GetDouble(mpParam->mnField, mnCurRow);
            rValue.mbIsNumber = !bIsStrVal;
            rValue.mnError = 0;
            return true;
        }
    }
    return false;
}

bool ScDBQueryDataIterator::DataAccessMatrix::getFirst(Value& rValue)
{
    mnCurRow = mpParam->bHasHeader ? 1 : 0;
    return getCurrent(rValue);
}

bool ScDBQueryDataIterator::DataAccessMatrix::getNext(Value& rValue)
{
    ++mnCurRow;
    return getCurrent(rValue);
}

namespace {

bool lcl_isQueryByValue(const ScQueryEntry& rEntry, const ScMatrix& rMat, SCSIZE nCol, SCSIZE nRow)
{
    if (rEntry.bQueryByString)
        return false;

    if (!rMat.IsValueOrEmpty(nCol, nRow))
        return false;

    return true;
}

bool lcl_isQueryByString(const ScQueryEntry& rEntry, const ScMatrix& rMat, SCSIZE nCol, SCSIZE nRow)
{
    switch (rEntry.eOp)
    {
        case SC_EQUAL:
        case SC_NOT_EQUAL:
        case SC_CONTAINS:
        case SC_DOES_NOT_CONTAIN:
        case SC_BEGINS_WITH:
        case SC_ENDS_WITH:
        case SC_DOES_NOT_BEGIN_WITH:
        case SC_DOES_NOT_END_WITH:
            return true;
        default:
            ;
    }

    if (rEntry.bQueryByString && rMat.IsString(nCol, nRow))
        return true;

    return false;
}

}

bool ScDBQueryDataIterator::DataAccessMatrix::isValidQuery(SCROW nRow, const ScMatrix& rMat) const
{
    SCSIZE nEntryCount = mpParam->GetEntryCount();
    vector<bool> aResults;
    aResults.reserve(nEntryCount);

    const CollatorWrapper& rCollator =
        mpParam->bCaseSens ? *ScGlobal::GetCaseCollator() : *ScGlobal::GetCollator();

    for (SCSIZE i = 0; i < nEntryCount; ++i)
    {
        const ScQueryEntry& rEntry = mpParam->GetEntry(i);
        if (!rEntry.bDoQuery)
            continue;

        switch (rEntry.eOp)
        {
            case SC_EQUAL:
            case SC_LESS:
            case SC_GREATER:
            case SC_LESS_EQUAL:
            case SC_GREATER_EQUAL:
            case SC_NOT_EQUAL:
                break;
            default:
                // Only the above operators are supported.
                continue;
        }

        bool bValid = false;

        SCSIZE nField = static_cast<SCSIZE>(rEntry.nField);
        if (lcl_isQueryByValue(rEntry, rMat, nField, nRow))
        {
            // By value
            double fMatVal = rMat.GetDouble(nField, nRow);
            bool bEqual = approxEqual(fMatVal, rEntry.nVal);
            switch (rEntry.eOp)
            {
                case SC_EQUAL:
                    bValid = bEqual;
                break;
                case SC_LESS:
                    bValid = (fMatVal < rEntry.nVal) && !bEqual;
                break;
                case SC_GREATER:
                    bValid = (fMatVal > rEntry.nVal) && !bEqual;
                break;
                case SC_LESS_EQUAL:
                    bValid = (fMatVal < rEntry.nVal) || bEqual;
                break;
                case SC_GREATER_EQUAL:
                    bValid = (fMatVal > rEntry.nVal) || bEqual;
                break;
                case SC_NOT_EQUAL:
                    bValid = !bEqual;
                break;
                default:
                    ;
            }
        }
        else if (lcl_isQueryByString(rEntry, rMat, nField, nRow))
        {
            // By string
            do
            {
                if (!rEntry.pStr)
                    break;

                // Equality check first.

                OUString aMatStr = rMat.GetString(nField, nRow);
                lcl_toUpper(aMatStr);
                OUString aQueryStr = *rEntry.pStr;
                lcl_toUpper(aQueryStr);
                bool bDone = false;
                switch (rEntry.eOp)
                {
                    case SC_EQUAL:
                        bValid = aMatStr.equals(aQueryStr);
                        bDone = true;
                    break;
                    case SC_NOT_EQUAL:
                        bValid = !aMatStr.equals(aQueryStr);
                        bDone = true;
                    break;
                    default:
                        ;
                }

                if (bDone)
                    break;

                // Unequality check using collator.

                sal_Int32 nCompare = rCollator.compareString(aMatStr, aQueryStr);
                switch (rEntry.eOp)
                {
                    case SC_LESS :
                        bValid = (nCompare < 0);
                    break;
                    case SC_GREATER :
                        bValid = (nCompare > 0);
                    break;
                    case SC_LESS_EQUAL :
                        bValid = (nCompare <= 0);
                    break;
                    case SC_GREATER_EQUAL :
                        bValid = (nCompare >= 0);
                    break;
                    default:
                        ;
                }
            }
            while (false);
        }
        else if (mpParam->bMixedComparison)
        {
            // Not used at the moment.
        }

        if (aResults.empty())
            // First query entry.
            aResults.push_back(bValid);
        else if (rEntry.eConnect == SC_AND)
        {
            // For AND op, tuck the result into the last result value.
            size_t n = aResults.size();
            aResults[n-1] = aResults[n-1] && bValid;
        }
        else
            // For OR op, store its own result.
            aResults.push_back(bValid);
    }

    // Row is valid as long as there is at least one result being true.
    vector<bool>::const_iterator itr = aResults.begin(), itrEnd = aResults.end();
    for (; itr != itrEnd; ++itr)
        if (*itr)
            return true;

    return false;
}

// ----------------------------------------------------------------------------

ScDBQueryDataIterator::Value::Value() :
    mnError(0), mbIsNumber(true)
{
    ::rtl::math::setNan(&mfValue);
}

// ----------------------------------------------------------------------------

ScDBQueryDataIterator::ScDBQueryDataIterator(ScDocument* pDocument, ScDBQueryParamBase* pParam) :
    mpParam (pParam)
{
    switch (mpParam->GetType())
    {
        case ScDBQueryParamBase::INTERNAL:
        {
            ScDBQueryParamInternal* p = static_cast<ScDBQueryParamInternal*>(pParam);
            mpData.reset(new DataAccessInternal(this, p, pDocument));
        }
        break;
        case ScDBQueryParamBase::MATRIX:
        {
            ScDBQueryParamMatrix* p = static_cast<ScDBQueryParamMatrix*>(pParam);
            mpData.reset(new DataAccessMatrix(this, p));
        }
    }
}

bool ScDBQueryDataIterator::GetFirst(Value& rValue)
{
    return mpData->getFirst(rValue);
}

bool ScDBQueryDataIterator::GetNext(Value& rValue)
{
    return mpData->getNext(rValue);
}

// ============================================================================

ScCellIterator::ScCellIterator( ScDocument* pDocument,
                                SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
                                SCCOL nECol, SCROW nERow, SCTAB nETab, sal_Bool bSTotal ) :
    pDoc( pDocument ),
    nStartCol( nSCol),
    nStartRow( nSRow),
    nStartTab( nSTab ),
    nEndCol( nECol ),
    nEndRow( nERow),
    nEndTab( nETab ),
    bSubTotal(bSTotal)

{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

    while (nEndTab>0 && !pDoc->pTab[nEndTab])
        --nEndTab;                                      // nur benutzte Tabellen
    if (nStartTab>nEndTab)
        nStartTab = nEndTab;

    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;
    nColRow = 0;                    // wird bei GetFirst initialisiert

    if (!pDoc->pTab[nTab])
    {
        DBG_ERROR("Tabelle nicht gefunden");
        nStartCol = nCol = MAXCOL+1;
        nStartRow = nRow = MAXROW+1;
        nStartTab = nTab = MAXTAB+1;    // -> Abbruch bei GetFirst
    }
}

ScCellIterator::ScCellIterator
    ( ScDocument* pDocument, const ScRange& rRange, sal_Bool bSTotal ) :
    pDoc( pDocument ),
    nStartCol( rRange.aStart.Col() ),
    nStartRow( rRange.aStart.Row() ),
    nStartTab( rRange.aStart.Tab() ),
    nEndCol( rRange.aEnd.Col() ),
    nEndRow( rRange.aEnd.Row() ),
    nEndTab( rRange.aEnd.Tab() ),
    bSubTotal(bSTotal)

{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

    while (nEndTab>0 && !pDoc->pTab[nEndTab])
        --nEndTab;                                      // nur benutzte Tabellen
    if (nStartTab>nEndTab)
        nStartTab = nEndTab;

    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;
    nColRow = 0;                    // wird bei GetFirst initialisiert

    if (!pDoc->pTab[nTab])
    {
        DBG_ERROR("Tabelle nicht gefunden");
        nStartCol = nCol = MAXCOL+1;
        nStartRow = nRow = MAXROW+1;
        nStartTab = nTab = MAXTAB+1;    // -> Abbruch bei GetFirst
    }
}

ScBaseCell* ScCellIterator::GetThis()
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    for ( ;; )
    {
        if ( nRow > nEndRow )
        {
            nRow = nStartRow;
            do
            {
                nCol++;
                if ( nCol > nEndCol )
                {
                    nCol = nStartCol;
                    nTab++;
                    if ( nTab > nEndTab )
                        return NULL;                // Ende und Aus
                }
                pCol = &(pDoc->pTab[nTab])->aCol[nCol];
            } while ( pCol->nCount == 0 );
            pCol->Search( nRow, nColRow );
        }

        while ( (nColRow < pCol->nCount) && (pCol->pItems[nColRow].nRow < nRow) )
            nColRow++;

        if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= nEndRow )
        {
            nRow = pCol->pItems[nColRow].nRow;
            if ( !bSubTotal || !pDoc->pTab[nTab]->RowFiltered( nRow ) )
            {
                ScBaseCell* pCell = pCol->pItems[nColRow].pCell;

                if ( bSubTotal && pCell->GetCellType() == CELLTYPE_FORMULA
                                && ((ScFormulaCell*)pCell)->IsSubTotal() )
                    nRow++;             // Sub-Total-Zeilen nicht
                else
                    return pCell;       // gefunden
            }
            else
                nRow++;
        }
        else
            nRow = nEndRow + 1; // Naechste Spalte
    }
}

ScBaseCell* ScCellIterator::GetFirst()
{
    if ( !ValidTab(nTab) )
        return NULL;
    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;
//  nColRow = 0;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    pCol->Search( nRow, nColRow );
    return GetThis();
}

ScBaseCell* ScCellIterator::GetNext()
{
    ++nRow;
    return GetThis();
}

//-------------------------------------------------------------------------------

ScQueryCellIterator::ScQueryCellIterator(ScDocument* pDocument, SCTAB nTable,
             const ScQueryParam& rParam, sal_Bool bMod ) :
    aParam (rParam),
    pDoc( pDocument ),
    nTab( nTable),
    nStopOnMismatch( nStopOnMismatchDisabled ),
    nTestEqualCondition( nTestEqualConditionDisabled ),
    bAdvanceQuery( sal_False ),
    bIgnoreMismatchOnLeadingStrings( sal_False )
{
    nCol = aParam.nCol1;
    nRow = aParam.nRow1;
    nColRow = 0;                    // wird bei GetFirst initialisiert
    SCSIZE i;
    if (bMod)                               // sonst schon eingetragen
    {
        for (i=0; (i<MAXQUERY) && (aParam.GetEntry(i).bDoQuery); i++)
        {
            ScQueryEntry& rEntry = aParam.GetEntry(i);
            sal_uInt32 nIndex = 0;
            rEntry.bQueryByString =
                     !(pDoc->GetFormatTable()->IsNumberFormat(*rEntry.pStr,
                                                              nIndex, rEntry.nVal));
        }
    }
    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;
}

ScBaseCell* ScQueryCellIterator::GetThis()
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    const ScQueryEntry& rEntry = aParam.GetEntry(0);
    SCCOLROW nFirstQueryField = rEntry.nField;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !rEntry.bQueryByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !aParam.bHasHeader && rEntry.bQueryByString &&
        ((aParam.bByRow && nRow == aParam.nRow1) ||
         (!aParam.bByRow && nCol == aParam.nCol1));
    for ( ;; )
    {
        if ( nRow > aParam.nRow2 )
        {
            nRow = aParam.nRow1;
            if (aParam.bHasHeader && aParam.bByRow)
                nRow++;
            do
            {
                if ( ++nCol > aParam.nCol2 )
                    return NULL;                // Ende und Aus
                if ( bAdvanceQuery )
                {
                    AdvanceQueryParamEntryField();
                    nFirstQueryField = rEntry.nField;
                }
                pCol = &(pDoc->pTab[nTab])->aCol[nCol];
            } while ( pCol->nCount == 0 );
            pCol->Search( nRow, nColRow );
            bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
                !aParam.bHasHeader && rEntry.bQueryByString &&
                aParam.bByRow;
        }

        while ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow < nRow )
            nColRow++;

        if ( nColRow < pCol->nCount &&
                (nRow = pCol->pItems[nColRow].nRow) <= aParam.nRow2 )
        {
            ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
            if ( pCell->GetCellType() == CELLTYPE_NOTE )
                ++nRow;
            else if (bAllStringIgnore && pCell->HasStringData())
                ++nRow;
            else
            {
                sal_Bool bTestEqualCondition;
                if ( (pDoc->pTab[nTab])->ValidQuery( nRow, aParam, NULL,
                        (nCol == static_cast<SCCOL>(nFirstQueryField) ? pCell : NULL),
                        (nTestEqualCondition ? &bTestEqualCondition : NULL) ) )
                {
                    if ( nTestEqualCondition && bTestEqualCondition )
                        nTestEqualCondition |= nTestEqualConditionMatched;
                    return pCell;     // found
                }
                else if ( nStopOnMismatch )
                {
                    // Yes, even a mismatch may have a fulfilled equal
                    // condition if regular expressions were involved and
                    // SC_LESS_EQUAL or SC_GREATER_EQUAL were queried.
                    if ( nTestEqualCondition && bTestEqualCondition )
                    {
                        nTestEqualCondition |= nTestEqualConditionMatched;
                        nStopOnMismatch |= nStopOnMismatchOccured;
                        return NULL;
                    }
                    bool bStop;
                    if (bFirstStringIgnore)
                    {
                        if (pCell->HasStringData())
                        {
                            ++nRow;
                            bStop = false;
                        }
                        else
                            bStop = true;
                    }
                    else
                        bStop = true;
                    if (bStop)
                    {
                        nStopOnMismatch |= nStopOnMismatchOccured;
                        return NULL;
                    }
                }
                else
                    nRow++;
            }
        }
        else
            nRow = aParam.nRow2 + 1; // Naechste Spalte
        bFirstStringIgnore = false;
    }
}

ScBaseCell* ScQueryCellIterator::GetFirst()
{
    nCol = aParam.nCol1;
    nRow = aParam.nRow1;
    if (aParam.bHasHeader)
        nRow++;
//  nColRow = 0;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    pCol->Search( nRow, nColRow );
    return GetThis();
}

ScBaseCell* ScQueryCellIterator::GetNext()
{
    ++nRow;
    if ( nStopOnMismatch )
        nStopOnMismatch = nStopOnMismatchEnabled;
    if ( nTestEqualCondition )
        nTestEqualCondition = nTestEqualConditionEnabled;
    return GetThis();
}

void ScQueryCellIterator::AdvanceQueryParamEntryField()
{
    SCSIZE nEntries = aParam.GetEntryCount();
    for ( SCSIZE j = 0; j < nEntries; j++  )
    {
        ScQueryEntry& rEntry = aParam.GetEntry( j );
        if ( rEntry.bDoQuery )
        {
            if ( rEntry.nField < MAXCOL )
                rEntry.nField++;
            else
            {
                DBG_ERRORFILE( "AdvanceQueryParamEntryField: ++rEntry.nField > MAXCOL" );
            }
        }
        else
            break;  // for
    }
}


sal_Bool ScQueryCellIterator::FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
        SCROW& nFoundRow, sal_Bool bSearchForEqualAfterMismatch,
        sal_Bool bIgnoreMismatchOnLeadingStringsP )
{
    nFoundCol = MAXCOL+1;
    nFoundRow = MAXROW+1;
    SetStopOnMismatch( sal_True );      // assume sorted keys
    SetTestEqualCondition( sal_True );
    bIgnoreMismatchOnLeadingStrings = bIgnoreMismatchOnLeadingStringsP;
    bool bRegExp = aParam.bRegExp && aParam.GetEntry(0).bQueryByString;
    bool bBinary = !bRegExp && aParam.bByRow && (aParam.GetEntry(0).eOp ==
            SC_LESS_EQUAL || aParam.GetEntry(0).eOp == SC_GREATER_EQUAL);
    if (bBinary ? (BinarySearch() ? GetThis() : 0) : GetFirst())
    {
        // First equal entry or last smaller than (greater than) entry.
        SCSIZE nColRowSave;
        ScBaseCell* pNext = 0;
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            nColRowSave = nColRow;
        } while ( !IsEqualConditionFulfilled() && (pNext = GetNext()) != NULL );
        // There may be no pNext but equal condition fulfilled if regular
        // expressions are involved. Keep the found entry and proceed.
        if (!pNext && !IsEqualConditionFulfilled())
        {
            // Step back to last in range and adjust position markers for
            // GetNumberFormat() or similar.
            nCol = nFoundCol;
            nRow = nFoundRow;
            nColRow = nColRowSave;
        }
    }
    if ( IsEqualConditionFulfilled() )
    {
        // Position on last equal entry.
        SCSIZE nEntries = aParam.GetEntryCount();
        for ( SCSIZE j = 0; j < nEntries; j++  )
        {
            ScQueryEntry& rEntry = aParam.GetEntry( j );
            if ( rEntry.bDoQuery )
            {
                switch ( rEntry.eOp )
                {
                    case SC_LESS_EQUAL :
                    case SC_GREATER_EQUAL :
                        rEntry.eOp = SC_EQUAL;
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
            else
                break;  // for
        }
        SCSIZE nColRowSave;
        bIgnoreMismatchOnLeadingStrings = sal_False;
        SetTestEqualCondition( sal_False );
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            nColRowSave = nColRow;
        } while (GetNext());
        // Step back conditions same as above
        nCol = nFoundCol;
        nRow = nFoundRow;
        nColRow = nColRowSave;
        return sal_True;
    }
    if ( (bSearchForEqualAfterMismatch || aParam.bRegExp) &&
            StoppedOnMismatch() )
    {
        // Assume found entry to be the last value less than respectively
        // greater than the query. But keep on searching for an equal match.
        SCSIZE nEntries = aParam.GetEntryCount();
        for ( SCSIZE j = 0; j < nEntries; j++  )
        {
            ScQueryEntry& rEntry = aParam.GetEntry( j );
            if ( rEntry.bDoQuery )
            {
                switch ( rEntry.eOp )
                {
                    case SC_LESS_EQUAL :
                    case SC_GREATER_EQUAL :
                        rEntry.eOp = SC_EQUAL;
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
            else
                break;  // for
        }
        SetStopOnMismatch( sal_False );
        SetTestEqualCondition( sal_False );
        if (GetNext())
        {
            // Last of a consecutive area, avoid searching the entire parameter
            // range as it is a real performance bottleneck in case of regular
            // expressions.
            SCSIZE nColRowSave;
            do
            {
                nFoundCol = GetCol();
                nFoundRow = GetRow();
                nColRowSave = nColRow;
                SetStopOnMismatch( sal_True );
            } while (GetNext());
            nCol = nFoundCol;
            nRow = nFoundRow;
            nColRow = nColRowSave;
        }
    }
    return (nFoundCol <= MAXCOL) && (nFoundRow <= MAXROW);
}


ScBaseCell* ScQueryCellIterator::BinarySearch()
{
    nCol = aParam.nCol1;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    if (!pCol->nCount)
        return 0;

    ScBaseCell* pCell;
    SCSIZE nHi, nLo;
    CollatorWrapper* pCollator = (aParam.bCaseSens ? ScGlobal::GetCaseCollator() :
        ScGlobal::GetCollator());
    SvNumberFormatter& rFormatter = *(pDoc->GetFormatTable());
    const ScQueryEntry& rEntry = aParam.GetEntry(0);
    bool bLessEqual = rEntry.eOp == SC_LESS_EQUAL;
    bool bByString = rEntry.bQueryByString;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings && !bByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !aParam.bHasHeader && bByString;

    nRow = aParam.nRow1;
    if (aParam.bHasHeader)
        nRow++;
    const ColEntry* pItems = pCol->pItems;
    if (pCol->Search( nRow, nLo ) && bFirstStringIgnore &&
            pItems[nLo].pCell->HasStringData())
    {
        String aCellStr;
        sal_uLong nFormat = pCol->GetNumberFormat( pItems[nLo].nRow);
        ScCellFormat::GetInputString( pItems[nLo].pCell, nFormat, aCellStr,
                rFormatter);
        sal_Int32 nTmp = pCollator->compareString( aCellStr, *rEntry.pStr);
        if ((rEntry.eOp == SC_LESS_EQUAL && nTmp > 0) ||
                (rEntry.eOp == SC_GREATER_EQUAL && nTmp < 0) ||
                (rEntry.eOp == SC_EQUAL && nTmp != 0))
            ++nLo;
    }
    if (!pCol->Search( aParam.nRow2, nHi ) && nHi>0)
        --nHi;
    while (bAllStringIgnore && nLo <= nHi && nLo < pCol->nCount &&
            pItems[nLo].pCell->HasStringData())
        ++nLo;

    // Bookkeeping values for breaking up the binary search in case the data
    // range isn't strictly sorted.
    SCSIZE nLastInRange = nLo;
    SCSIZE nFirstLastInRange = nLastInRange;
    double fLastInRangeValue = bLessEqual ?
        -(::std::numeric_limits<double>::max()) :
            ::std::numeric_limits<double>::max();
    String aLastInRangeString;
    if (!bLessEqual)
        aLastInRangeString.Assign( sal_Unicode(0xFFFF));
    if (nLastInRange < pCol->nCount)
    {
        pCell = pItems[nLastInRange].pCell;
        if (pCell->HasStringData())
        {
            sal_uLong nFormat = pCol->GetNumberFormat( pItems[nLastInRange].nRow);
            ScCellFormat::GetInputString( pCell, nFormat, aLastInRangeString,
                    rFormatter);
        }
        else
        {
            switch ( pCell->GetCellType() )
            {
                case CELLTYPE_VALUE :
                    fLastInRangeValue =
                        static_cast<ScValueCell*>(pCell)->GetValue();
                    break;
                case CELLTYPE_FORMULA :
                    fLastInRangeValue =
                        static_cast<ScFormulaCell*>(pCell)->GetValue();
                    break;
                default:
                {
                    // added to avoid warnings
                }
            }
        }
    }

    sal_Int32 nRes = 0;
    bool bFound = false;
    bool bDone = false;
    while (nLo <= nHi && !bDone)
    {
        SCSIZE nMid = (nLo+nHi)/2;
        SCSIZE i = nMid;
        while (i <= nHi && pItems[i].pCell->GetCellType() == CELLTYPE_NOTE)
            ++i;
        if (i > nHi)
        {
            if (nMid > 0)
                nHi = nMid - 1;
            else
                bDone = true;
            continue;   // while
        }
        sal_Bool bStr = pItems[i].pCell->HasStringData();
        nRes = 0;
        // compares are content<query:-1, content>query:1
        // Cell value comparison similar to ScTable::ValidQuery()
        if (!bStr && !bByString)
        {
            double nCellVal;
            pCell = pItems[i].pCell;
            switch ( pCell->GetCellType() )
            {
                case CELLTYPE_VALUE :
                    nCellVal = static_cast<ScValueCell*>(pCell)->GetValue();
                    break;
                case CELLTYPE_FORMULA :
                    nCellVal = static_cast<ScFormulaCell*>(pCell)->GetValue();
                    break;
                default:
                    nCellVal = 0.0;
            }
            if ((nCellVal < rEntry.nVal) && !::rtl::math::approxEqual(
                        nCellVal, rEntry.nVal))
            {
                nRes = -1;
                if (bLessEqual)
                {
                    if (fLastInRangeValue < nCellVal)
                    {
                        fLastInRangeValue = nCellVal;
                        nLastInRange = i;
                    }
                    else if (fLastInRangeValue > nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        nLastInRange = nFirstLastInRange;
                        bDone = true;
                    }
                }
            }
            else if ((nCellVal > rEntry.nVal) && !::rtl::math::approxEqual(
                        nCellVal, rEntry.nVal))
            {
                nRes = 1;
                if (!bLessEqual)
                {
                    if (fLastInRangeValue > nCellVal)
                    {
                        fLastInRangeValue = nCellVal;
                        nLastInRange = i;
                    }
                    else if (fLastInRangeValue < nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        nLastInRange = nFirstLastInRange;
                        bDone = true;
                    }
                }
            }
        }
        else if (bStr && bByString)
        {
            String aCellStr;
            sal_uLong nFormat = pCol->GetNumberFormat( pItems[i].nRow);
            ScCellFormat::GetInputString( pItems[i].pCell, nFormat, aCellStr,
                    rFormatter);
            nRes = pCollator->compareString( aCellStr, *rEntry.pStr);
            if (nRes < 0 && bLessEqual)
            {
                sal_Int32 nTmp = pCollator->compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp < 0)
                {
                    aLastInRangeString = aCellStr;
                    nLastInRange = i;
                }
                else if (nTmp > 0)
                {
                    // not strictly sorted, continue with GetThis()
                    nLastInRange = nFirstLastInRange;
                    bDone = true;
                }
            }
            else if (nRes > 0 && !bLessEqual)
            {
                sal_Int32 nTmp = pCollator->compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp > 0)
                {
                    aLastInRangeString = aCellStr;
                    nLastInRange = i;
                }
                else if (nTmp < 0)
                {
                    // not strictly sorted, continue with GetThis()
                    nLastInRange = nFirstLastInRange;
                    bDone = true;
                }
            }
        }
        else if (!bStr && bByString)
        {
            nRes = -1;  // numeric < string
            if (bLessEqual)
                nLastInRange = i;
        }
        else // if (bStr && !bByString)
        {
            nRes = 1;   // string > numeric
            if (!bLessEqual)
                nLastInRange = i;
        }
        if (nRes < 0)
        {
            if (bLessEqual)
                nLo = nMid + 1;
            else    // assumed to be SC_GREATER_EQUAL
            {
                if (nMid > 0)
                    nHi = nMid - 1;
                else
                    bDone = true;
            }
        }
        else if (nRes > 0)
        {
            if (bLessEqual)
            {
                if (nMid > 0)
                    nHi = nMid - 1;
                else
                    bDone = true;
            }
            else    // assumed to be SC_GREATER_EQUAL
                nLo = nMid + 1;
        }
        else
        {
            nLo = i;
            bDone = bFound = true;
        }
    }
    if (!bFound)
    {
        // If all hits didn't result in a moving limit there's something
        // strange, e.g. data range not properly sorted, or only identical
        // values encountered, which doesn't mean there aren't any others in
        // between.. leave it to GetThis(). The condition for this would be
        // if (nLastInRange == nFirstLastInRange) nLo = nFirstLastInRange;
        // Else, in case no exact match was found, we step back for a
        // subsequent GetThis() to find the last in range. Effectively this is
        // --nLo with nLastInRange == nLo-1. Both conditions combined yield:
        nLo = nLastInRange;
    }
    if (nLo < pCol->nCount && pCol->pItems[nLo].nRow <= aParam.nRow2)
    {
        nRow = pItems[nLo].nRow;
        pCell = pItems[nLo].pCell;
        nColRow = nLo;
    }
    else
    {
        nRow = aParam.nRow2 + 1;
        pCell = 0;
        nColRow = pCol->nCount - 1;
    }
    return pCell;
}


//-------------------------------------------------------------------------------

ScHorizontalCellIterator::ScHorizontalCellIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) :
    pDoc( pDocument ),
    nTab( nTable ),
    nStartCol( nCol1 ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 ),
    nCol( nCol1 ),
    nRow( nRow1 ),
    bMore( sal_True )
{

    pNextRows = new SCROW[ nCol2-nCol1+1 ];
    pNextIndices = new SCSIZE[ nCol2-nCol1+1 ];

    SetTab( nTab );
}

ScHorizontalCellIterator::~ScHorizontalCellIterator()
{
    delete [] pNextRows;
    delete [] pNextIndices;
}

void ScHorizontalCellIterator::SetTab( SCTAB nTabP )
{
    nTab = nTabP;
    nRow = nStartRow;
    nCol = nStartCol;
    bMore = sal_True;

    for (SCCOL i=nStartCol; i<=nEndCol; i++)
    {
        ScColumn* pCol = &pDoc->pTab[nTab]->aCol[i];

        SCSIZE nIndex;
        pCol->Search( nStartRow, nIndex );
        if ( nIndex < pCol->nCount )
        {
            pNextRows[i-nStartCol] = pCol->pItems[nIndex].nRow;
            pNextIndices[i-nStartCol] = nIndex;
        }
        else
        {
            pNextRows[i-nStartCol] = MAXROWCOUNT;       // nichts gefunden
            pNextIndices[i-nStartCol] = MAXROWCOUNT;
        }
    }

    if (pNextRows[0] != nStartRow)
        Advance();
}

ScBaseCell* ScHorizontalCellIterator::GetNext( SCCOL& rCol, SCROW& rRow )
{
    if ( bMore )
    {
        rCol = nCol;
        rRow = nRow;

        ScColumn* pCol = &pDoc->pTab[nTab]->aCol[nCol];
        SCSIZE nIndex = pNextIndices[nCol-nStartCol];
        DBG_ASSERT( nIndex < pCol->nCount, "ScHorizontalCellIterator::GetNext: nIndex out of range" );
        ScBaseCell* pCell = pCol->pItems[nIndex].pCell;
        if ( ++nIndex < pCol->nCount )
        {
            pNextRows[nCol-nStartCol] = pCol->pItems[nIndex].nRow;
            pNextIndices[nCol-nStartCol] = nIndex;
        }
        else
        {
            pNextRows[nCol-nStartCol] = MAXROWCOUNT;        // nichts gefunden
            pNextIndices[nCol-nStartCol] = MAXROWCOUNT;
        }

        Advance();
        return pCell;
    }
    else
        return NULL;
}

sal_Bool ScHorizontalCellIterator::ReturnNext( SCCOL& rCol, SCROW& rRow )
{
    rCol = nCol;
    rRow = nRow;
    return bMore;
}

void ScHorizontalCellIterator::Advance()
{
    sal_Bool bFound = sal_False;
    SCCOL i;

    for (i=nCol+1; i<=nEndCol && !bFound; i++)
        if (pNextRows[i-nStartCol] == nRow)
        {
            nCol = i;
            bFound = sal_True;
        }

    if (!bFound)
    {
        SCROW nMinRow = MAXROW+1;
        for (i=nStartCol; i<=nEndCol; i++)
            if (pNextRows[i-nStartCol] < nMinRow)
            {
                nCol = i;
                nMinRow = pNextRows[i-nStartCol];
            }

        if (nMinRow <= nEndRow)
        {
            nRow = nMinRow;
            bFound = sal_True;
        }
    }

    if ( !bFound )
        bMore = sal_False;
}

//------------------------------------------------------------------------

ScHorizontalValueIterator::ScHorizontalValueIterator( ScDocument* pDocument,
        const ScRange& rRange, bool bSTotal, bool bTextZero ) :
    pDoc( pDocument ),
    nNumFmtIndex(0),
    nEndTab( rRange.aEnd.Tab() ),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    bNumValid( false ),
    bSubTotal( bSTotal ),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
    bTextAsZero( bTextZero )
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

    nCurCol = nStartCol;
    nCurRow = nStartRow;
    nCurTab = nStartTab;

    nNumFormat = 0;                 // will be initialized in GetNumberFormat()
    pAttrArray = 0;
    nAttrEndRow = 0;

    pCellIter = new ScHorizontalCellIterator( pDoc, nStartTab, nStartCol,
            nStartRow, nEndCol, nEndRow );
}

ScHorizontalValueIterator::~ScHorizontalValueIterator()
{
    delete pCellIter;
}

bool ScHorizontalValueIterator::GetNext( double& rValue, sal_uInt16& rErr )
{
    bool bFound = false;
    while ( !bFound )
    {
        ScBaseCell* pCell = pCellIter->GetNext( nCurCol, nCurRow );
        while ( !pCell )
        {
            if ( nCurTab < nEndTab )
            {
                pCellIter->SetTab( ++nCurTab);
                pCell = pCellIter->GetNext( nCurCol, nCurRow );
            }
            else
                return false;
        }
        if ( !bSubTotal || !pDoc->pTab[nCurTab]->RowFiltered( nCurRow ) )
        {
            switch (pCell->GetCellType())
            {
                case CELLTYPE_VALUE:
                    {
                        bNumValid = false;
                        rValue = ((ScValueCell*)pCell)->GetValue();
                        rErr = 0;
                        if ( bCalcAsShown )
                        {
                            ScColumn* pCol = &pDoc->pTab[nCurTab]->aCol[nCurCol];
                            lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
                                    nAttrEndRow, pCol->pAttrArray, nCurRow, pDoc );
                            rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
                        }
                        bFound = true;
                    }
                    break;
                case CELLTYPE_FORMULA:
                    {
                        if (!bSubTotal || !((ScFormulaCell*)pCell)->IsSubTotal())
                        {
                            rErr = ((ScFormulaCell*)pCell)->GetErrCode();
                            if ( rErr || ((ScFormulaCell*)pCell)->IsValue() )
                            {
                                rValue = ((ScFormulaCell*)pCell)->GetValue();
                                bNumValid = false;
                                bFound = true;
                            }
                            else if ( bTextAsZero )
                            {
                                rValue = 0.0;
                                bNumValid = false;
                                bFound = true;
                            }
                        }
                    }
                    break;
                case CELLTYPE_STRING :
                case CELLTYPE_EDIT :
                    {
                        if ( bTextAsZero )
                        {
                            rErr = 0;
                            rValue = 0.0;
                            nNumFmtType = NUMBERFORMAT_NUMBER;
                            nNumFmtIndex = 0;
                            bNumValid = true;
                            bFound = true;
                        }
                    }
                    break;
                default:
                    ;   // nothing
            }
        }
    }
    return bFound;
}

void ScHorizontalValueIterator::GetCurNumFmtInfo( short& nType, sal_uInt32& nIndex )
{
    if (!bNumValid)
    {
        const ScColumn* pCol = &(pDoc->pTab[nCurTab])->aCol[nCurCol];
        nNumFmtIndex = pCol->GetNumberFormat( nCurRow );
        if ( (nNumFmtIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
        {
            const ScBaseCell* pCell;
            SCSIZE nCurIndex;
            if ( pCol->Search( nCurRow, nCurIndex ) )
                pCell = pCol->pItems[nCurIndex].pCell;
            else
                pCell = NULL;
            if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
                ((const ScFormulaCell*)pCell)->GetFormatInfo( nNumFmtType, nNumFmtIndex );
            else
                nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
        }
        else
            nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
        bNumValid = true;
    }
    nType = nNumFmtType;
    nIndex = nNumFmtIndex;
}

//-------------------------------------------------------------------------------

ScHorizontalAttrIterator::ScHorizontalAttrIterator( ScDocument* pDocument, SCTAB nTable,
                            SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) :
    pDoc( pDocument ),
    nTab( nTable ),
    nStartCol( nCol1 ),
    nStartRow( nRow1 ),
    nEndCol( nCol2 ),
    nEndRow( nRow2 )
{
    DBG_ASSERT( pDoc->pTab[nTab], "Tabelle nicht da" );

    SCCOL i;

    nRow = nStartRow;
    nCol = nStartCol;
    bRowEmpty = sal_False;

    pIndices    = new SCSIZE[nEndCol-nStartCol+1];
    pNextEnd    = new SCROW[nEndCol-nStartCol+1];
    ppPatterns  = new const ScPatternAttr*[nEndCol-nStartCol+1];

    SCROW nSkipTo = MAXROW;
    sal_Bool bEmpty = sal_True;
    for (i=nStartCol; i<=nEndCol; i++)
    {
        SCCOL nPos = i - nStartCol;
        ScAttrArray* pArray = pDoc->pTab[nTab]->aCol[i].pAttrArray;
        DBG_ASSERT( pArray, "pArray == 0" );

        SCSIZE nIndex;
        pArray->Search( nStartRow, nIndex );

        const ScPatternAttr* pPattern = pArray->pData[nIndex].pPattern;
        SCROW nThisEnd = pArray->pData[nIndex].nRow;
        if ( IsDefaultItem( pPattern ) )
        {
            pPattern = NULL;
            if ( nThisEnd < nSkipTo )
                nSkipTo = nThisEnd;         // nSkipTo kann gleich hier gesetzt werden
        }
        else
            bEmpty = sal_False;                 // Attribute gefunden

        pIndices[nPos] = nIndex;
        pNextEnd[nPos] = nThisEnd;
        ppPatterns[nPos] = pPattern;
    }

    if (bEmpty)
        nRow = nSkipTo;                     // bis zum naechsten Bereichsende ueberspringen
    bRowEmpty = bEmpty;
}

ScHorizontalAttrIterator::~ScHorizontalAttrIterator()
{
    delete[] (ScPatternAttr**)ppPatterns;
    delete[] pNextEnd;
    delete[] pIndices;
}

const ScPatternAttr* ScHorizontalAttrIterator::GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow )
{
    for (;;)
    {
        if (!bRowEmpty)
        {
            // in dieser Zeile suchen

            while ( nCol <= nEndCol && !ppPatterns[nCol-nStartCol] )
                ++nCol;

            if ( nCol <= nEndCol )
            {
                const ScPatternAttr* pPat = ppPatterns[nCol-nStartCol];
                rRow = nRow;
                rCol1 = nCol;
                while ( nCol < nEndCol && ppPatterns[nCol+1-nStartCol] == pPat )
                    ++nCol;
                rCol2 = nCol;
                ++nCol;                 // hochzaehlen fuer naechsten Aufruf
                return pPat;            // gefunden
            }
        }

        // naechste Zeile

        ++nRow;
        if ( nRow > nEndRow )       // schon am Ende?
            return NULL;            // nichts gefunden

        sal_Bool bEmpty = sal_True;
        SCCOL i;

        for ( i = nStartCol; i <= nEndCol; i++)
        {
            SCCOL nPos = i-nStartCol;
            if ( pNextEnd[nPos] < nRow )
            {
                ScAttrArray* pArray = pDoc->pTab[nTab]->aCol[i].pAttrArray;

                SCSIZE nIndex = ++pIndices[nPos];
                if ( nIndex < pArray->nCount )
                {
                    const ScPatternAttr* pPattern = pArray->pData[nIndex].pPattern;
                    SCROW nThisEnd = pArray->pData[nIndex].nRow;
                    if ( IsDefaultItem( pPattern ) )
                        pPattern = NULL;
                    else
                        bEmpty = sal_False;                 // Attribute gefunden

                    pNextEnd[nPos] = nThisEnd;
                    ppPatterns[nPos] = pPattern;

                    DBG_ASSERT( pNextEnd[nPos] >= nRow, "Reihenfolge durcheinander" );
                }
                else
                {
                    DBG_ERROR("AttrArray reicht nicht bis MAXROW");
                    pNextEnd[nPos] = MAXROW;
                    ppPatterns[nPos] = NULL;
                }
            }
            else if ( ppPatterns[nPos] )
                bEmpty = sal_False;                         // Bereich noch nicht zuende
        }

        if (bEmpty)
        {
            SCCOL nCount = nEndCol-nStartCol+1;
            SCROW nSkipTo = pNextEnd[0];                // naechstes Bereichsende suchen
            for (i=1; i<nCount; i++)
                if ( pNextEnd[i] < nSkipTo )
                    nSkipTo = pNextEnd[i];
            nRow = nSkipTo;                             // leere Zeilen ueberspringen
        }
        bRowEmpty = bEmpty;
        nCol = nStartCol;           // wieder links anfangen
    }

//    return NULL;
}

//-------------------------------------------------------------------------------

inline sal_Bool IsGreater( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    return ( nRow1 > nRow2 ) || ( nRow1 == nRow2 && nCol1 > nCol2 );
}

ScUsedAreaIterator::ScUsedAreaIterator( ScDocument* pDocument, SCTAB nTable,
                            SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) :
    aCellIter( pDocument, nTable, nCol1, nRow1, nCol2, nRow2 ),
    aAttrIter( pDocument, nTable, nCol1, nRow1, nCol2, nRow2 ),
    nNextCol( nCol1 ),
    nNextRow( nRow1 )
{
    pCell    = aCellIter.GetNext( nCellCol, nCellRow );
    pPattern = aAttrIter.GetNext( nAttrCol1, nAttrCol2, nAttrRow );
}

ScUsedAreaIterator::~ScUsedAreaIterator()
{
}

sal_Bool ScUsedAreaIterator::GetNext()
{
    //  Iteratoren weiterzaehlen

    if ( pCell && IsGreater( nNextCol, nNextRow, nCellCol, nCellRow ) )
        pCell = aCellIter.GetNext( nCellCol, nCellRow );

    while ( pCell && pCell->IsBlank() )
        pCell = aCellIter.GetNext( nCellCol, nCellRow );

    if ( pPattern && IsGreater( nNextCol, nNextRow, nAttrCol2, nAttrRow ) )
        pPattern = aAttrIter.GetNext( nAttrCol1, nAttrCol2, nAttrRow );

    if ( pPattern && nAttrRow == nNextRow && nAttrCol1 < nNextCol )
        nAttrCol1 = nNextCol;

    //  naechsten Abschnitt heraussuchen

    sal_Bool bFound = sal_True;
    sal_Bool bUseCell = sal_False;

    if ( pCell && pPattern )
    {
        if ( IsGreater( nCellCol, nCellRow, nAttrCol1, nAttrRow ) )     // vorne nur Attribute ?
        {
            pFoundCell = NULL;
            pFoundPattern = pPattern;
            nFoundRow = nAttrRow;
            nFoundStartCol = nAttrCol1;
            if ( nCellRow == nAttrRow && nCellCol <= nAttrCol2 )        // auch Zelle im Bereich ?
                nFoundEndCol = nCellCol - 1;                            // nur bis vor der Zelle
            else
                nFoundEndCol = nAttrCol2;                               // alles
        }
        else
        {
            bUseCell = sal_True;
            if ( nAttrRow == nCellRow && nAttrCol1 == nCellCol )        // Attribute auf der Zelle ?
                pFoundPattern = pPattern;
            else
                pFoundPattern = NULL;
        }
    }
    else if ( pCell )                   // nur Zelle -> direkt uebernehmen
    {
        pFoundPattern = NULL;
        bUseCell = sal_True;                // Position von Zelle
    }
    else if ( pPattern )                // nur Attribute -> direkt uebernehmen
    {
        pFoundCell = NULL;
        pFoundPattern = pPattern;
        nFoundRow = nAttrRow;
        nFoundStartCol = nAttrCol1;
        nFoundEndCol = nAttrCol2;
    }
    else                                // gar nichts
        bFound = sal_False;

    if ( bUseCell )                     // Position von Zelle
    {
        pFoundCell = pCell;
        nFoundRow = nCellRow;
        nFoundStartCol = nFoundEndCol = nCellCol;
    }

    if (bFound)
    {
        nNextRow = nFoundRow;
        nNextCol = nFoundEndCol + 1;
    }

    return bFound;
}

//-------------------------------------------------------------------------------

ScDocAttrIterator::ScDocAttrIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2) :
    pDoc( pDocument ),
    nTab( nTable ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 ),
    nCol( nCol1 )
{
    if ( ValidTab(nTab) && pDoc->pTab[nTab] )
        pColIter = pDoc->pTab[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
    else
        pColIter = NULL;
}

ScDocAttrIterator::~ScDocAttrIterator()
{
    delete pColIter;
}

const ScPatternAttr* ScDocAttrIterator::GetNext( SCCOL& rCol, SCROW& rRow1, SCROW& rRow2 )
{
    while ( pColIter )
    {
        const ScPatternAttr* pPattern = pColIter->Next( rRow1, rRow2 );
        if ( pPattern )
        {
            rCol = nCol;
            return pPattern;
        }

        delete pColIter;
        ++nCol;
        if ( nCol <= nEndCol )
            pColIter = pDoc->pTab[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
        else
            pColIter = NULL;
    }
    return NULL;        // is nix mehr
}

//-------------------------------------------------------------------------------

ScAttrRectIterator::ScAttrRectIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2) :
    pDoc( pDocument ),
    nTab( nTable ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 ),
    nIterStartCol( nCol1 ),
    nIterEndCol( nCol1 )
{
    if ( ValidTab(nTab) && pDoc->pTab[nTab] )
    {
        pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nStartRow, nEndRow );
        while ( nIterEndCol < nEndCol &&
                pDoc->pTab[nTab]->aCol[nIterEndCol].IsAllAttrEqual(
                    pDoc->pTab[nTab]->aCol[nIterEndCol+1], nStartRow, nEndRow ) )
            ++nIterEndCol;
    }
    else
        pColIter = NULL;
}

ScAttrRectIterator::~ScAttrRectIterator()
{
    delete pColIter;
}

void ScAttrRectIterator::DataChanged()
{
    if (pColIter)
    {
        SCROW nNextRow = pColIter->GetNextRow();
        delete pColIter;
        pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nNextRow, nEndRow );
    }
}

const ScPatternAttr* ScAttrRectIterator::GetNext( SCCOL& rCol1, SCCOL& rCol2,
                                                    SCROW& rRow1, SCROW& rRow2 )
{
    while ( pColIter )
    {
        const ScPatternAttr* pPattern = pColIter->Next( rRow1, rRow2 );
        if ( pPattern )
        {
            rCol1 = nIterStartCol;
            rCol2 = nIterEndCol;
            return pPattern;
        }

        delete pColIter;
        nIterStartCol = nIterEndCol+1;
        if ( nIterStartCol <= nEndCol )
        {
            nIterEndCol = nIterStartCol;
            pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nStartRow, nEndRow );
            while ( nIterEndCol < nEndCol &&
                    pDoc->pTab[nTab]->aCol[nIterEndCol].IsAllAttrEqual(
                        pDoc->pTab[nTab]->aCol[nIterEndCol+1], nStartRow, nEndRow ) )
                ++nIterEndCol;
        }
        else
            pColIter = NULL;
    }
    return NULL;        // is nix mehr
}

// ============================================================================

SCROW ScRowBreakIterator::NOT_FOUND = -1;

ScRowBreakIterator::ScRowBreakIterator(set<SCROW>& rBreaks) :
    mrBreaks(rBreaks),
    maItr(rBreaks.begin()), maEnd(rBreaks.end())
{
}

SCROW ScRowBreakIterator::first()
{
    maItr = mrBreaks.begin();
    return maItr == maEnd ? NOT_FOUND : *maItr;
}

SCROW ScRowBreakIterator::next()
{
    ++maItr;
    return maItr == maEnd ? NOT_FOUND : *maItr;
}
