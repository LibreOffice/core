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

#include <svl/zforlist.hxx>

#include "scitems.hxx"
#include "global.hxx"
#include "dociter.hxx"
#include "document.hxx"
#include "table.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "formulacell.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "docoptio.hxx"
#include "cellform.hxx"
#include "segmenttree.hxx"
#include "progress.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"
#include "globstr.hrc"
#include "editutil.hxx"
#include "cellvalue.hxx"

#include "tools/fract.hxx"
#include "editeng/editobj.hxx"

#include <vector>

using ::rtl::math::approxEqual;
using ::std::vector;
using ::std::set;

// STATIC DATA -----------------------------------------------------------

namespace
{
    void lcl_uppercase(OUString& rStr)
    {
        rStr = ScGlobal::pCharClass->uppercase(rStr.trim());
    }
}

ScDocumentIterator::ScDocumentIterator( ScDocument* pDocument,
                            SCTAB nStartTable, SCTAB nEndTable ) :
    pDoc( pDocument ),
    nStartTab( nStartTable ),
    nEndTab( nEndTable )
{
    SCTAB nDocMaxTab = pDoc->GetTableCount() - 1;
    PutInOrder( nStartTab, nEndTab );
    if (!ValidTab(nStartTab) || nStartTab > nDocMaxTab ) nStartTab = nDocMaxTab;
    if (!ValidTab(nEndTab) || nEndTab > nDocMaxTab ) nEndTab = nDocMaxTab;

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

bool ScDocumentIterator::GetThisCol()
{
    ScTable* pTab = NULL;
    while ( nTab < pDoc->GetTableCount() && (pTab = pDoc->maTabs[nTab]) == NULL )
    {
        if ( nTab == nEndTab )
        {
            nCol = MAXCOL;
            nRow = MAXROW;
            return false;
        }
        ++nTab;
    }
    if (pTab == NULL)
    {
        OSL_FAIL("no table in document?");
        return false;
    }
    ScColumn*       pCol = &pTab->aCol[nCol];
    const ScAttrArray* pAtt = pCol->pAttrArray;

    bool bFound = false;
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
            nColRow = (nColPos < pCol->maItems.size()) ? pCol->maItems[nColPos].nRow : MAXROW+1;
            if (nColRow < nRow)
                ++nColPos;
        }
        while (nColRow < nRow);

        if (nColRow == nRow)
        {
            bFound   = true;
            pCell    = pCol->maItems[nColPos].pCell;
            pPattern = pAtt->pData[nAttrPos].pPattern;
        }
        else if ( pAtt->pData[nAttrPos].pPattern != pDefPattern )
        {
            bFound = true;
            pCell = NULL;
            pPattern = pAtt->pData[nAttrPos].pPattern;
        }
        else
        {
            nRow = std::min( (SCROW)nColRow, (SCROW)(nAttrEnd+1) );
        }
    }
    while (!bFound && nRow <= MAXROW);

    return bFound;
}

bool ScDocumentIterator::GetThis()
{
    bool bEnd = false;
    bool bSuccess = false;

    while ( !bSuccess && !bEnd )
    {
        if ( nRow > MAXROW )
            bSuccess = false;
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
                    bEnd = true;
            }
            nRow = 0;
            nColPos = 0;
            nAttrPos = 0;
        }
    }

    return !bEnd;
}

bool ScDocumentIterator::GetFirst()
{
    nCol = 0;
    nTab = nStartTab;

    nRow = 0;
    nColPos = 0;
    nAttrPos = 0;

    return GetThis();
}

bool ScDocumentIterator::GetNext()
{
    ++nRow;

    return GetThis();
}

ScCellValue ScDocumentIterator::GetCellValue() const
{
    ScCellValue aRet;
    if (pCell)
        aRet.assign(*pCell);
    return aRet;
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
void ScAttrArray_IterGetNumberFormat( sal_uLong& nFormat, const ScAttrArray*& rpArr,
        SCROW& nAttrEndRow, const ScAttrArray* pNewArr, SCROW nRow,
        ScDocument* pDoc )
{
    if ( rpArr != pNewArr || nAttrEndRow < nRow )
    {
        SCROW nRowStart = 0;
        SCROW nRowEnd = MAXROW;
        const ScPatternAttr* pPattern;
        if( !(pPattern = pNewArr->GetPatternRange( nRowStart, nRowEnd, nRow ) ) )
        {
            pPattern = pDoc->GetDefPattern();
            nRowEnd = MAXROW;
        }

        nFormat = pPattern->GetNumberFormat( pDoc->GetFormatTable() );
        rpArr = pNewArr;
        nAttrEndRow = nRowEnd;
    }
}

ScValueIterator::ScValueIterator( ScDocument* pDocument, const ScRange& rRange,
            bool bSTotal, bool bTextZero ) :
    pDoc( pDocument ),
    nNumFmtIndex(0),
    nStartCol( rRange.aStart.Col() ),
    nStartRow( rRange.aStart.Row() ),
    nStartTab( rRange.aStart.Tab() ),
    nEndCol( rRange.aEnd.Col() ),
    nEndRow( rRange.aEnd.Row() ),
    nEndTab( rRange.aEnd.Tab() ),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    bNumValid( false ),
    bSubTotal(bSTotal),
    bNextValid( false ),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
    bTextAsZero( bTextZero )
{
    SCTAB nDocMaxTab = pDocument->GetTableCount() - 1;

    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab) || nStartTab > nDocMaxTab) nStartTab = nDocMaxTab;
    if (!ValidTab(nEndTab) || nEndTab > nDocMaxTab) nEndTab = nDocMaxTab;

    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;

    nColRow = 0; // Initialized in GetFirst
    nNumFormat = 0; // Initialized in GetNumberFormat
    pAttrArray = 0;
    nAttrEndRow = 0;
}

bool ScValueIterator::GetThis(double& rValue, sal_uInt16& rErr)
{
    if (nTab >= pDoc->GetTableCount())
    {
        OSL_FAIL("try to access out of index, FIX IT");
    }
    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
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
                        rErr = 0;
                        return false; // Over and out
                    }
                }
                pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
            } while ( pCol->maItems.empty() );
            pCol->Search( nRow, nColRow );
        }

        while (( nColRow < pCol->maItems.size() ) && ( pCol->maItems[nColRow].nRow < nRow ))
            nColRow++;

        if ( nColRow < pCol->maItems.size() && pCol->maItems[nColRow].nRow <= nEndRow )
        {
            nRow = pCol->maItems[nColRow].nRow + 1;
            if ( !bSubTotal || !pDoc->maTabs[nTab]->RowFiltered( nRow-1 ) )
            {
                ScBaseCell* pCell = pCol->maItems[nColRow].pCell;
                ++nColRow;
                switch (pCell->GetCellType())
                {
                    case CELLTYPE_VALUE:
                    {
                        bNumValid = false;
                        rValue = ((ScValueCell*)pCell)->GetValue();
                        rErr = 0;
                        --nRow;
                        if ( bCalcAsShown )
                        {
                            ScAttrArray_IterGetNumberFormat( nNumFormat, pAttrArray,
                                nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
                            rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
                        }
                        /*
                         * If there's another Value Cell in the same column,
                         * which is also within the Area, we retrieve it's value
                         */
                        if ( nColRow < pCol->maItems.size() &&
                             pCol->maItems[nColRow].nRow <= nEndRow &&
                             pCol->maItems[nColRow].pCell->GetCellType() == CELLTYPE_VALUE &&
                             !bSubTotal )
                        {
                            fNextValue = ((ScValueCell*)pCol->maItems[nColRow].pCell)->GetValue();
                            nNextRow = pCol->maItems[nColRow].nRow;
                            bNextValid = true;
                            if ( bCalcAsShown )
                            {
                                ScAttrArray_IterGetNumberFormat( nNumFormat, pAttrArray,
                                    nAttrEndRow, pCol->pAttrArray, nNextRow, pDoc );
                                fNextValue = pDoc->RoundValueAsShown( fNextValue, nNumFormat );
                            }
                        }

                        return true; // Found it!
                    }

                    case CELLTYPE_FORMULA:
                    {
                        if (!bSubTotal || !((ScFormulaCell*)pCell)->IsSubTotal())
                        {
                            rErr = ((ScFormulaCell*)pCell)->GetErrCode();
                            if ( rErr || ((ScFormulaCell*)pCell)->IsValue() )
                            {
                                rValue = ((ScFormulaCell*)pCell)->GetValue();
                                nRow--;
                                bNumValid = false;
                                return true; // Found it!
                            }
                            else if ( bTextAsZero )
                            {
                                rValue = 0.0;
                                nRow--;
                                bNumValid = false;
                                return true;
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
                            --nRow;
                            return true;
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
            nRow = nEndRow + 1; // Next column
    }
}

void ScValueIterator::GetCurNumFmtInfo( short& nType, sal_uLong& nIndex )
{
    if (!bNumValid && nTab < pDoc->GetTableCount())
    {
        const ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
        nNumFmtIndex = pCol->GetNumberFormat( nRow );
        nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
        bNumValid = true;
    }
    nType = nNumFmtType;
    nIndex = nNumFmtIndex;
}

bool ScValueIterator::GetFirst(double& rValue, sal_uInt16& rErr)
{
    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;

    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
    pCol->Search( nRow, nColRow );

    nNumFormat = 0; // Initialized in GetNumberFormat
    pAttrArray = 0;
    nAttrEndRow = 0;

    return GetThis(rValue, rErr);
}

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
    if (nTab >= rDoc.GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    ScColumn* pCol = &rDoc.maTabs[nTab]->aCol[nCol];
    return pCol->maItems[nColRow].nRow;
}

ScBaseCell* ScDBQueryDataIterator::GetCellByColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCCOL nCol, SCSIZE nColRow)
{
    if (nTab >= rDoc.GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    ScColumn* pCol = &rDoc.maTabs[nTab]->aCol[nCol];
    return pCol->maItems[nColRow].pCell;
}

const ScAttrArray* ScDBQueryDataIterator::GetAttrArrayByCol(ScDocument& rDoc, SCTAB nTab, SCCOL nCol)
{
    if (nTab >= rDoc.GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    ScColumn* pCol = &rDoc.maTabs[nTab]->aCol[nCol];
    return pCol->pAttrArray;
}

bool ScDBQueryDataIterator::IsQueryValid(ScDocument& rDoc, const ScQueryParam& rParam, SCTAB nTab, SCROW nRow, ScBaseCell* pCell)
{
    if (nTab >= rDoc.GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    return rDoc.maTabs[nTab]->ValidQuery(nRow, rParam, pCell);
}

SCSIZE ScDBQueryDataIterator::SearchColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCROW nRow, SCCOL nCol)
{
    if (nTab >= rDoc.GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    ScColumn* pCol = &rDoc.maTabs[nTab]->aCol[nCol];
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

    nColRow = 0; // Initialized in GetFirst
    SCSIZE i;
    SCSIZE nCount = mpParam->GetEntryCount();
    for (i=0; (i<nCount) && (mpParam->GetEntry(i).bDoQuery); i++)
    {
        ScQueryEntry& rEntry = mpParam->GetEntry(i);
        ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        rItems.resize(1);
        ScQueryEntry::Item& rItem = rItems.front();
        sal_uInt32 nIndex = 0;
        bool bNumber = mpDoc->GetFormatTable()->IsNumberFormat(
            rItem.maString, nIndex, rItem.mfVal);
        rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
    }
    nNumFormat = 0; // Initialized in GetNumberFormat
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
            // Bottom of the range reached. Bail out.
            rValue.mnError = 0;
            return false;
        }

        SCSIZE nCellCount = mpDoc->GetCellCount(nTab, nCol);
        if (!nCellCount)
            // No cells found in this column. Bail out.
            return false;

        SCROW nThisRow = ScDBQueryDataIterator::GetRowByColEntryIndex(*mpDoc, nTab, nCol, nColRow);
        while ( (nColRow < nCellCount) && (nThisRow < nRow) )
        {
            ++nColRow;
            if(nColRow < nCellCount)
                nThisRow = ScDBQueryDataIterator::GetRowByColEntryIndex(*mpDoc, nTab, nCol, nColRow);
        }

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
                                ScAttrArray_IterGetNumberFormat( nNumFormat, pAttrArray,
                                    nAttrEndRow, pNewAttrArray, nRow, mpDoc );
                                rValue.mfValue = mpDoc->RoundValueAsShown( rValue.mfValue, nNumFormat );
                            }
                            nNumFmtType = NUMBERFORMAT_NUMBER;
                            nNumFmtIndex = 0;
                            rValue.mnError = 0;
                            return true; // Found it!
                        }

                    case CELLTYPE_FORMULA:
                        {
                            if (((ScFormulaCell*)pCell)->IsValue())
                            {
                                rValue.mfValue = ((ScFormulaCell*)pCell)->GetValue();
                                rValue.mbIsNumber = true;
                                mpDoc->GetNumberFormatInfo( nNumFmtType,
                                    nNumFmtIndex, ScAddress(nCol, nRow, nTab));
                                rValue.mnError = ((ScFormulaCell*)pCell)->GetErrCode();
                                return true; // Found it!
                            }
                            else if(mpParam->mbSkipString)
                                nRow++;
                            else
                            {
                                rValue.maString = static_cast<ScFormulaCell*>(pCell)->GetString();
                                rValue.mfValue = 0.0;
                                rValue.mnError = static_cast<ScFormulaCell*>(pCell)->GetErrCode();
                                rValue.mbIsNumber = false;
                                return true;
                            }
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
            nRow = mpParam->nRow2 + 1; // Next column
    }
// statement unreachable
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

bool isQueryByValue(const ScQueryEntry::Item& rItem, const ScMatrix& rMat, SCSIZE nCol, SCSIZE nRow)
{
    if (rItem.meType == ScQueryEntry::ByString)
        return false;

    if (!rMat.IsValueOrEmpty(nCol, nRow))
        return false;

    return true;
}

bool isQueryByString(const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem, const ScMatrix& rMat, SCSIZE nCol, SCSIZE nRow)
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

    if (rItem.meType == ScQueryEntry::ByString && rMat.IsString(nCol, nRow))
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
        const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
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
        if (isQueryByValue(rItem, rMat, nField, nRow))
        {
            // By value
            double fMatVal = rMat.GetDouble(nField, nRow);
            bool bEqual = approxEqual(fMatVal, rItem.mfVal);
            switch (rEntry.eOp)
            {
                case SC_EQUAL:
                    bValid = bEqual;
                break;
                case SC_LESS:
                    bValid = (fMatVal < rItem.mfVal) && !bEqual;
                break;
                case SC_GREATER:
                    bValid = (fMatVal > rItem.mfVal) && !bEqual;
                break;
                case SC_LESS_EQUAL:
                    bValid = (fMatVal < rItem.mfVal) || bEqual;
                break;
                case SC_GREATER_EQUAL:
                    bValid = (fMatVal > rItem.mfVal) || bEqual;
                break;
                case SC_NOT_EQUAL:
                    bValid = !bEqual;
                break;
                default:
                    ;
            }
        }
        else if (isQueryByString(rEntry, rItem, rMat, nField, nRow))
        {
            // By string
            do
            {
                // Equality check first.
                OUString aMatStr = rMat.GetString(nField, nRow);
                lcl_uppercase(aMatStr);
                OUString aQueryStr = rEntry.GetQueryItem().maString;
                lcl_uppercase(aQueryStr);
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

ScCellIterator::ScCellIterator( ScDocument* pDoc, const ScRange& rRange, bool bSTotal ) :
    mpDoc(pDoc),
    maStartPos(rRange.aStart),
    maEndPos(rRange.aEnd),
    mnIndex(0),
    mbSubTotal(bSTotal)
{
    init();
}

void ScCellIterator::init()
{
    SCTAB nDocMaxTab = mpDoc->GetTableCount() - 1;

    PutInOrder(maStartPos, maEndPos);

    if (!ValidCol(maStartPos.Col())) maStartPos.SetCol(MAXCOL);
    if (!ValidCol(maEndPos.Col())) maEndPos.SetCol(MAXCOL);
    if (!ValidRow(maStartPos.Row())) maStartPos.SetRow(MAXROW);
    if (!ValidRow(maEndPos.Row())) maEndPos.SetRow(MAXROW);
    if (!ValidTab(maStartPos.Tab(), nDocMaxTab)) maStartPos.SetTab(nDocMaxTab);
    if (!ValidTab(maEndPos.Tab(), nDocMaxTab)) maEndPos.SetTab(nDocMaxTab);

    while (maEndPos.Tab() > 0 && !mpDoc->maTabs[maEndPos.Tab()])
        maEndPos.IncTab(-1); // Only the tables in use

    if (maStartPos.Tab() > maEndPos.Tab())
        maStartPos.SetTab(maEndPos.Tab());

    maCurPos = maStartPos;

    if (!mpDoc->maTabs[maCurPos.Tab()])
    {
        OSL_FAIL("Table not found");
        maStartPos = ScAddress(MAXCOL+1, MAXROW+1, MAXTAB+1); // -> Abort on GetFirst.
        maCurPos = maStartPos;
    }
}

bool ScCellIterator::getCurrent()
{
    ScColumn* pCol = &(mpDoc->maTabs[maCurPos.Tab()])->aCol[maCurPos.Col()];
    while (true)
    {
        if (maCurPos.Row() > maEndPos.Row())
        {
            maCurPos.SetRow(maStartPos.Row());
            do
            {
                maCurPos.IncCol();
                if (maCurPos.Col() > maEndPos.Col())
                {
                    maCurPos.SetCol(maStartPos.Col());
                    maCurPos.IncTab();
                    if (maCurPos.Tab() > maEndPos.Tab())
                    {
                        maCurCell.clear();
                        return false; // Over and out
                    }
                }
                pCol = &(mpDoc->maTabs[maCurPos.Tab()])->aCol[maCurPos.Col()];
            } while ( pCol->maItems.empty() );
            pCol->Search(maCurPos.Row(), mnIndex);
        }

        while ( (mnIndex < pCol->maItems.size()) && (pCol->maItems[mnIndex].nRow < maCurPos.Row()) )
            ++mnIndex;

        if (mnIndex < pCol->maItems.size() && pCol->maItems[mnIndex].nRow <= maEndPos.Row())
        {
            maCurPos.SetRow(pCol->maItems[mnIndex].nRow);
            if (!mbSubTotal || !mpDoc->maTabs[maCurPos.Tab()]->RowFiltered(maCurPos.Row()))
            {
                ScBaseCell* pCell = pCol->maItems[mnIndex].pCell;

                if ( mbSubTotal && pCell->GetCellType() == CELLTYPE_FORMULA
                                && ((ScFormulaCell*)pCell)->IsSubTotal() )
                    maCurPos.IncRow(); // Don't subtotal rows
                else
                {
                    // Found it!
                    maCurCell.meType = pCell->GetCellType();
                    switch (maCurCell.meType)
                    {
                        case CELLTYPE_VALUE:
                            maCurCell.mfValue = static_cast<const ScValueCell*>(pCell)->GetValue();
                        break;
                        case CELLTYPE_STRING:
                            maCurCell.mpString = static_cast<const ScStringCell*>(pCell)->GetStringPtr();
                        break;
                        case CELLTYPE_EDIT:
                            maCurCell.mpEditText = static_cast<const ScEditCell*>(pCell)->GetData();
                        break;
                        case CELLTYPE_FORMULA:
                            maCurCell.mpFormula = static_cast<ScFormulaCell*>(pCell);
                        break;
                        default:
                            maCurCell.meType = CELLTYPE_NONE;
                    }

                    if (maCurCell.meType != CELLTYPE_NONE)
                        return true;

                    maCurPos.IncRow();
                }
            }
            else
                maCurPos.IncRow();
        }
        else
            maCurPos.SetRow(maEndPos.Row() + 1); // Next column
    }
    return false;
}

CellType ScCellIterator::getType() const
{
    return maCurCell.meType;
}

OUString ScCellIterator::getString()
{
    return maCurCell.getString(mpDoc);
}

const EditTextObject* ScCellIterator::getEditText() const
{
    return maCurCell.mpEditText;
}

ScFormulaCell* ScCellIterator::getFormulaCell()
{
    return maCurCell.mpFormula;
}

const ScFormulaCell* ScCellIterator::getFormulaCell() const
{
    return maCurCell.mpFormula;
}

double ScCellIterator::getValue()
{
    return maCurCell.getValue();
}

ScCellValue ScCellIterator::getCellValue() const
{
    ScCellValue aRet;
    aRet.meType = maCurCell.meType;

    switch (maCurCell.meType)
    {
        case CELLTYPE_STRING:
            aRet.mpString = new OUString(*maCurCell.mpString);
        break;
        case CELLTYPE_EDIT:
            aRet.mpEditText = maCurCell.mpEditText->Clone();
        break;
        case CELLTYPE_VALUE:
            aRet.mfValue = maCurCell.mfValue;
        break;
        case CELLTYPE_FORMULA:
            aRet.mpFormula = maCurCell.mpFormula->Clone();
        break;
        default:
            ;
    }

    return aRet;
}

const ScRefCellValue& ScCellIterator::getRefCellValue() const
{
    return maCurCell;
}

bool ScCellIterator::hasString() const
{
    return maCurCell.hasString();
}

bool ScCellIterator::hasNumeric() const
{
    return maCurCell.hasNumeric();
}

bool ScCellIterator::hasEmptyData() const
{
    if (maCurCell.isEmpty())
        return true;

    if (maCurCell.meType == CELLTYPE_FORMULA)
        return maCurCell.mpFormula->IsEmpty();

    return false;
}

bool ScCellIterator::isEmpty() const
{
    return maCurCell.isEmpty();
}

bool ScCellIterator::equalsWithoutFormat( const ScAddress& rPos ) const
{
    ScRefCellValue aOther;
    aOther.assign(*mpDoc, rPos);
    return maCurCell.equalsWithoutFormat(aOther);
}

bool ScCellIterator::first()
{
    if (!ValidTab(maCurPos.Tab()))
        return false;

    maCurPos = maStartPos;
    ScColumn* pCol = &(mpDoc->maTabs[maCurPos.Tab()])->aCol[maCurPos.Col()];
    pCol->Search(maCurPos.Row(), mnIndex);
    return getCurrent();
}

bool ScCellIterator::next()
{
    maCurPos.IncRow();
    return getCurrent();
}

//-------------------------------------------------------------------------------

ScQueryCellIterator::ScQueryCellIterator(ScDocument* pDocument, SCTAB nTable,
             const ScQueryParam& rParam, bool bMod ) :
    mpParam(new ScQueryParam(rParam)),
    pDoc( pDocument ),
    nTab( nTable),
    nStopOnMismatch( nStopOnMismatchDisabled ),
    nTestEqualCondition( nTestEqualConditionDisabled ),
    bAdvanceQuery( false ),
    bIgnoreMismatchOnLeadingStrings( false )
{
    nCol = mpParam->nCol1;
    nRow = mpParam->nRow1;
    nColRow = 0; // Initialized in GetFirst
    SCSIZE i;
    if (bMod) // Or else it's already inserted
    {
        SCSIZE nCount = mpParam->GetEntryCount();
        for (i = 0; (i < nCount) && (mpParam->GetEntry(i).bDoQuery); ++i)
        {
            ScQueryEntry& rEntry = mpParam->GetEntry(i);
            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            sal_uInt32 nIndex = 0;
            bool bNumber = pDoc->GetFormatTable()->IsNumberFormat(
                rItem.maString, nIndex, rItem.mfVal);
            rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
        }
    }
    nNumFormat = 0; // Initialized in GetNumberFormat
    pAttrArray = 0;
    nAttrEndRow = 0;
}

ScBaseCell* ScQueryCellIterator::GetThis()
{
    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
    const ScQueryEntry& rEntry = mpParam->GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

    SCCOLROW nFirstQueryField = rEntry.nField;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        rItem.meType != ScQueryEntry::ByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !mpParam->bHasHeader && rItem.meType == ScQueryEntry::ByString &&
        ((mpParam->bByRow && nRow == mpParam->nRow1) ||
         (!mpParam->bByRow && nCol == mpParam->nCol1));
    for ( ;; )
    {
        if ( nRow > mpParam->nRow2 )
        {
            nRow = mpParam->nRow1;
            if (mpParam->bHasHeader && mpParam->bByRow)
                nRow++;
            do
            {
                if ( ++nCol > mpParam->nCol2 )
                    return NULL; // Over and out
                if ( bAdvanceQuery )
                {
                    AdvanceQueryParamEntryField();
                    nFirstQueryField = rEntry.nField;
                }
                pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
            } while ( pCol->maItems.empty() );
            pCol->Search( nRow, nColRow );
            bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
                !mpParam->bHasHeader && rItem.meType == ScQueryEntry::ByString &&
                mpParam->bByRow;
        }

        while ( nColRow < pCol->maItems.size() && pCol->maItems[nColRow].nRow < nRow )
            nColRow++;

        if ( nColRow < pCol->maItems.size() &&
                (nRow = pCol->maItems[nColRow].nRow) <= mpParam->nRow2 )
        {
            ScBaseCell* pCell = pCol->maItems[nColRow].pCell;
            if (bAllStringIgnore && pCell->HasStringData())
                ++nRow;
            else
            {
                bool bTestEqualCondition = false;
                if ( (pDoc->maTabs[nTab])->ValidQuery( nRow, *mpParam,
                        (nCol == static_cast<SCCOL>(nFirstQueryField) ? pCell : NULL),
                        (nTestEqualCondition ? &bTestEqualCondition : NULL) ) )
                {
                    if ( nTestEqualCondition && bTestEqualCondition )
                        nTestEqualCondition |= nTestEqualConditionMatched;
                    return pCell; // Found it!
                }
                else if ( nStopOnMismatch )
                {
                    // Yes, even a mismatch may have a fulfilled equal
                    // condition if regular expressions were involved and
                    // SC_LESS_EQUAL or SC_GREATER_EQUAL were queried.
                    if ( nTestEqualCondition && bTestEqualCondition )
                    {
                        nTestEqualCondition |= nTestEqualConditionMatched;
                        nStopOnMismatch |= nStopOnMismatchOccurred;
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
                        nStopOnMismatch |= nStopOnMismatchOccurred;
                        return NULL;
                    }
                }
                else
                    nRow++;
            }
        }
        else
            nRow = mpParam->nRow2 + 1; // Next column
        bFirstStringIgnore = false;
    }
}

ScBaseCell* ScQueryCellIterator::GetFirst()
{
    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    nCol = mpParam->nCol1;
    nRow = mpParam->nRow1;
    if (mpParam->bHasHeader)
        nRow++;
    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
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
    SCSIZE nEntries = mpParam->GetEntryCount();
    for ( SCSIZE j = 0; j < nEntries; j++  )
    {
        ScQueryEntry& rEntry = mpParam->GetEntry( j );
        if ( rEntry.bDoQuery )
        {
            if ( rEntry.nField < MAXCOL )
                rEntry.nField++;
            else
            {
                OSL_FAIL( "AdvanceQueryParamEntryField: ++rEntry.nField > MAXCOL" );
            }
        }
        else
            break;  // for
    }
}


bool ScQueryCellIterator::FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
        SCROW& nFoundRow, bool bSearchForEqualAfterMismatch,
        bool bIgnoreMismatchOnLeadingStringsP )
{
    // Set and automatically reset mpParam->mbRangeLookup when returning. We
    // could use comphelper::FlagRestorationGuard, but really, that one is
    // overengineered for this simple purpose here.
    struct BoolResetter
    {
        bool& mr;
        bool  mb;
        BoolResetter( bool& r, bool b ) : mr(r), mb(r) { r = b; }
        ~BoolResetter() { mr = mb; }
    } aRangeLookupResetter( mpParam->mbRangeLookup, true);

    nFoundCol = MAXCOL+1;
    nFoundRow = MAXROW+1;
    SetStopOnMismatch( true ); // assume sorted keys
    SetTestEqualCondition( true );
    bIgnoreMismatchOnLeadingStrings = bIgnoreMismatchOnLeadingStringsP;
    bool bRegExp = mpParam->bRegExp && mpParam->GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByString;
    bool bBinary = !bRegExp && mpParam->bByRow && (mpParam->GetEntry(0).eOp ==
            SC_LESS_EQUAL || mpParam->GetEntry(0).eOp == SC_GREATER_EQUAL);
    bool bFound = false;
    if (bBinary)
    {
        if (BinarySearch())
        {
            // BinarySearch() already positions correctly and only needs real
            // query comparisons afterwards, skip the verification check below.
            mpParam->mbRangeLookup = false;
            bFound = GetThis();
        }
    }
    else
    {
        bFound = GetFirst();
    }
    if (bFound)
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
            SCCOL nColDiff = nCol - nFoundCol;
            nCol = nFoundCol;
            nRow = nFoundRow;
            nColRow = nColRowSave;
            if (mpParam->mbRangeLookup)
            {
                // Verify that the found entry does not only fulfill the range
                // lookup but also the real query, i.e. not numeric was found
                // if query is ByString and vice versa.
                mpParam->mbRangeLookup = false;
                // Step back the last field advance if GetNext() did one.
                if (bAdvanceQuery && nColDiff)
                {
                    SCSIZE nEntries = mpParam->GetEntryCount();
                    for (SCSIZE j=0; j < nEntries; ++j)
                    {
                        ScQueryEntry& rEntry = mpParam->GetEntry( j );
                        if (rEntry.bDoQuery)
                        {
                            if (rEntry.nField - nColDiff >= 0)
                                rEntry.nField -= nColDiff;
                            else
                            {
                                assert(!"FindEqualOrSortedLastInRange: rEntry.nField -= nColDiff < 0");
                            }
                        }
                        else
                            break;  // for
                    }
                }
                // Check it.
                if (!GetThis())
                {
                    nFoundCol = MAXCOL+1;
                    nFoundRow = MAXROW+1;
                }
            }
        }
    }
    if ( IsEqualConditionFulfilled() )
    {
        // Position on last equal entry.
        SCSIZE nEntries = mpParam->GetEntryCount();
        for ( SCSIZE j = 0; j < nEntries; j++  )
        {
            ScQueryEntry& rEntry = mpParam->GetEntry( j );
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
        bIgnoreMismatchOnLeadingStrings = false;
        SetTestEqualCondition( false );
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            nColRowSave = nColRow;
        } while (GetNext());

        // Step back conditions are the same as above
        nCol = nFoundCol;
        nRow = nFoundRow;
        nColRow = nColRowSave;
        return true;
    }
    if ( (bSearchForEqualAfterMismatch || mpParam->bRegExp) &&
            StoppedOnMismatch() )
    {
        // Assume found entry to be the last value less than respectively
        // greater than the query. But keep on searching for an equal match.
        SCSIZE nEntries = mpParam->GetEntryCount();
        for ( SCSIZE j = 0; j < nEntries; j++  )
        {
            ScQueryEntry& rEntry = mpParam->GetEntry( j );
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
        SetStopOnMismatch( false );
        SetTestEqualCondition( false );
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
                SetStopOnMismatch( true );
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
    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    nCol = mpParam->nCol1;
    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
    if (!pCol->maItems.size())
        return 0;

    ScRefCellValue aCell;
    ScBaseCell* pCell;
    SCSIZE nHi, nLo;
    CollatorWrapper* pCollator = (mpParam->bCaseSens ? ScGlobal::GetCaseCollator() :
        ScGlobal::GetCollator());
    SvNumberFormatter& rFormatter = *(pDoc->GetFormatTable());
    const ScQueryEntry& rEntry = mpParam->GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    bool bLessEqual = rEntry.eOp == SC_LESS_EQUAL;
    bool bByString = rItem.meType == ScQueryEntry::ByString;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings && !bByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !mpParam->bHasHeader && bByString;

    nRow = mpParam->nRow1;
    if (mpParam->bHasHeader)
        nRow++;
    if (pCol->Search( nRow, nLo ) && bFirstStringIgnore &&
            pCol->maItems[nLo].pCell->HasStringData())
    {
        OUString aCellStr;
        sal_uLong nFormat = pCol->GetNumberFormat( pCol->maItems[nLo].nRow);
        aCell.assign(*pCol->maItems[nLo].pCell);
        ScCellFormat::GetInputString(aCell, nFormat, aCellStr, rFormatter, pDoc);
        sal_Int32 nTmp = pCollator->compareString(aCellStr, rEntry.GetQueryItem().maString);
        if ((rEntry.eOp == SC_LESS_EQUAL && nTmp > 0) ||
                (rEntry.eOp == SC_GREATER_EQUAL && nTmp < 0) ||
                (rEntry.eOp == SC_EQUAL && nTmp != 0))
            ++nLo;
    }
    if (!pCol->Search( mpParam->nRow2, nHi ) && nHi>0)
        --nHi;
    while (bAllStringIgnore && nLo <= nHi && nLo < pCol->maItems.size() &&
            pCol->maItems[nLo].pCell->HasStringData())
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
    if (nLastInRange < pCol->maItems.size())
    {
        aCell.assign(*pCol->maItems[nLastInRange].pCell);
        if (aCell.hasString())
        {
            sal_uLong nFormat = pCol->GetNumberFormat( pCol->maItems[nLastInRange].nRow);
            OUString aStr;
            ScCellFormat::GetInputString(aCell, nFormat, aStr, rFormatter, pDoc);
            aLastInRangeString = aStr;
        }
        else
        {
            switch (aCell.meType)
            {
                case CELLTYPE_VALUE :
                    fLastInRangeValue = aCell.mfValue;
                break;
                case CELLTYPE_FORMULA :
                    fLastInRangeValue = aCell.mpFormula->GetValue();
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
        if (i > nHi)
        {
            if (nMid > 0)
                nHi = nMid - 1;
            else
                bDone = true;
            continue; // while
        }

        bool bStr = pCol->maItems[i].pCell->HasStringData();
        nRes = 0;

        // compares are content<query:-1, content>query:1
        // Cell value comparison similar to ScTable::ValidQuery()
        if (!bStr && !bByString)
        {
            double nCellVal;
            pCell = pCol->maItems[i].pCell;
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
            if ((nCellVal < rItem.mfVal) && !::rtl::math::approxEqual(
                        nCellVal, rItem.mfVal))
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
            else if ((nCellVal > rItem.mfVal) && !::rtl::math::approxEqual(
                        nCellVal, rItem.mfVal))
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
            OUString aCellStr;
            sal_uLong nFormat = pCol->GetNumberFormat( pCol->maItems[i].nRow);
            aCell.assign(*pCol->maItems[i].pCell);
            ScCellFormat::GetInputString(aCell, nFormat, aCellStr, rFormatter, pDoc);

            nRes = pCollator->compareString(aCellStr, rEntry.GetQueryItem().maString);
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
            nRes = -1; // numeric < string
            if (bLessEqual)
                nLastInRange = i;
        }
        else // if (bStr && !bByString)
        {
            nRes = 1; // string > numeric
            if (!bLessEqual)
                nLastInRange = i;
        }
        if (nRes < 0)
        {
            if (bLessEqual)
                nLo = nMid + 1;
            else // assumed to be SC_GREATER_EQUAL
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
            else // assumed to be SC_GREATER_EQUAL
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
    if (nLo < pCol->maItems.size() && pCol->maItems[nLo].nRow <= mpParam->nRow2)
    {
        nRow = pCol->maItems[nLo].nRow;
        pCell = pCol->maItems[nLo].pCell;
        nColRow = nLo;
    }
    else
    {
        nRow = mpParam->nRow2 + 1;
        pCell = 0;
        nColRow = pCol->maItems.size() - 1;
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
    bMore( true )
{
    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");

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
    bMore = true;

    for (SCCOL i=nStartCol; i<=nEndCol; i++)
    {
        ScColumn* pCol = &pDoc->maTabs[nTab]->aCol[i];

        SCSIZE nIndex;
        pCol->Search( nStartRow, nIndex );
        if ( nIndex < pCol->maItems.size() )
        {
            pNextRows[i-nStartCol] = pCol->maItems[nIndex].nRow;
            pNextIndices[i-nStartCol] = nIndex;
        }
        else
        {
            pNextRows[i-nStartCol] = MAXROWCOUNT; // Nothing found
            pNextIndices[i-nStartCol] = MAXROWCOUNT;
        }
    }

   if (pNextRows[0] != nStartRow)
        Advance();
}

ScRefCellValue* ScHorizontalCellIterator::GetNext( SCCOL& rCol, SCROW& rRow )
{
    if ( bMore )
    {
        rCol = nCol;
        rRow = nRow;

        ScColumn* pCol = &pDoc->maTabs[nTab]->aCol[nCol];
        SCSIZE nIndex = pNextIndices[nCol-nStartCol];
        OSL_ENSURE( nIndex < pCol->maItems.size(), "ScHorizontalCellIterator::GetNext: nIndex out of range" );
        ScBaseCell* pCell = pCol->maItems[nIndex].pCell;
        if ( ++nIndex < pCol->maItems.size() )
        {
            pNextRows[nCol-nStartCol] = pCol->maItems[nIndex].nRow;
            pNextIndices[nCol-nStartCol] = nIndex;
        }
        else
        {
            pNextRows[nCol-nStartCol] = MAXROWCOUNT; // Nothing found
            pNextIndices[nCol-nStartCol] = MAXROWCOUNT;
        }

        Advance();
        maCurCell.assign(*pCell);
        return &maCurCell;
    }
    else
        return NULL;
}

bool ScHorizontalCellIterator::ReturnNext( SCCOL& rCol, SCROW& rRow )
{
    rCol = nCol;
    rRow = nRow;
    return bMore;
}

void ScHorizontalCellIterator::Advance()
{
    bool bFound = false;
    SCCOL i;

    for (i=nCol+1; i<=nEndCol && !bFound; i++)
        if (pNextRows[i-nStartCol] == nRow)
        {
            nCol = i;
            bFound = true;
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
            bFound = true;
        }
    }

    if ( !bFound )
        bMore = false;
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

    nNumFormat = 0; // Will be initialized in GetNumberFormat()
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
        ScRefCellValue* pCell = pCellIter->GetNext( nCurCol, nCurRow );
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
        if ( !bSubTotal || !pDoc->maTabs[nCurTab]->RowFiltered( nCurRow ) )
        {
            switch (pCell->meType)
            {
                case CELLTYPE_VALUE:
                    {
                        bNumValid = false;
                        rValue = pCell->mfValue;
                        rErr = 0;
                        if ( bCalcAsShown )
                        {
                            ScColumn* pCol = &pDoc->maTabs[nCurTab]->aCol[nCurCol];
                            ScAttrArray_IterGetNumberFormat( nNumFormat, pAttrArray,
                                    nAttrEndRow, pCol->pAttrArray, nCurRow, pDoc );
                            rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
                        }
                        bFound = true;
                    }
                    break;
                case CELLTYPE_FORMULA:
                    {
                        if (!bSubTotal || !pCell->mpFormula->IsSubTotal())
                        {
                            rErr = pCell->mpFormula->GetErrCode();
                            if (rErr || pCell->mpFormula->IsValue())
                            {
                                rValue = pCell->mpFormula->GetValue();
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
                default: ;   // nothing
            }
        }
    }
    return bFound;
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
    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    OSL_ENSURE( pDoc->maTabs[nTab], "Table does not exist" );

    SCCOL i;

    nRow = nStartRow;
    nCol = nStartCol;
    bRowEmpty = false;

    pIndices    = new SCSIZE[nEndCol-nStartCol+1];
    pNextEnd    = new SCROW[nEndCol-nStartCol+1];
    ppPatterns  = new const ScPatternAttr*[nEndCol-nStartCol+1];

    SCROW nSkipTo = MAXROW;
    bool bEmpty = true;
    for (i=nStartCol; i<=nEndCol; i++)
    {
        SCCOL nPos = i - nStartCol;
        const ScAttrArray* pArray = pDoc->maTabs[nTab]->aCol[i].pAttrArray;
        OSL_ENSURE( pArray, "pArray == 0" );

        SCSIZE nIndex;
        pArray->Search( nStartRow, nIndex );

        const ScPatternAttr* pPattern = pArray->pData[nIndex].pPattern;
        SCROW nThisEnd = pArray->pData[nIndex].nRow;
        if ( IsDefaultItem( pPattern ) )
        {
            pPattern = NULL;
            if ( nThisEnd < nSkipTo )
                nSkipTo = nThisEnd; // nSkipTo can be set here already
        }
        else
            bEmpty = false; // Found attributes

        pIndices[nPos] = nIndex;
        pNextEnd[nPos] = nThisEnd;
        ppPatterns[nPos] = pPattern;
    }

    if (bEmpty)
        nRow = nSkipTo; // Skip until end of next section

    bRowEmpty = bEmpty;
}

ScHorizontalAttrIterator::~ScHorizontalAttrIterator()
{
    delete[] ppPatterns;
    delete[] pNextEnd;
    delete[] pIndices;
}

const ScPatternAttr* ScHorizontalAttrIterator::GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow )
{
    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    for (;;)
    {
        if (!bRowEmpty)
        {
            // Search in this row
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
                ++nCol; // Count up for next call
                return pPat; // Found it!
            }
        }

        // Next row
        ++nRow;
        if ( nRow > nEndRow ) // Already at the end?
            return NULL; // Found nothing

        bool bEmpty = true;
        SCCOL i;

        for ( i = nStartCol; i <= nEndCol; i++)
        {
            SCCOL nPos = i-nStartCol;
            if ( pNextEnd[nPos] < nRow )
            {
                const ScAttrArray* pArray = pDoc->maTabs[nTab]->aCol[i].pAttrArray;

                SCSIZE nIndex = ++pIndices[nPos];
                if ( nIndex < pArray->nCount )
                {
                    const ScPatternAttr* pPattern = pArray->pData[nIndex].pPattern;
                    SCROW nThisEnd = pArray->pData[nIndex].nRow;
                    if ( IsDefaultItem( pPattern ) )
                        pPattern = NULL;
                    else
                        bEmpty = false; // Found attributes

                    pNextEnd[nPos] = nThisEnd;
                    ppPatterns[nPos] = pPattern;

                    OSL_ENSURE( pNextEnd[nPos] >= nRow, "Sequence out of order" );
                }
                else
                {
                    OSL_FAIL("AttrArray does not range to MAXROW");
                    pNextEnd[nPos] = MAXROW;
                    ppPatterns[nPos] = NULL;
                }
            }
            else if ( ppPatterns[nPos] )
                bEmpty = false; // Area not at the end yet
        }

        if (bEmpty)
        {
            SCCOL nCount = nEndCol-nStartCol+1;
            SCROW nSkipTo = pNextEnd[0]; // Search next end of area
            for (i=1; i<nCount; i++)
                if ( pNextEnd[i] < nSkipTo )
                    nSkipTo = pNextEnd[i];
            nRow = nSkipTo; // Skip empty rows
        }
        bRowEmpty = bEmpty;
        nCol = nStartCol; // Start at the left again
    }
}

//-------------------------------------------------------------------------------

inline bool IsGreater( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
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

bool ScUsedAreaIterator::GetNext()
{
    //  Forward iterators
    if ( pCell && IsGreater( nNextCol, nNextRow, nCellCol, nCellRow ) )
        pCell = aCellIter.GetNext( nCellCol, nCellRow );

    while (pCell && pCell->isEmpty())
        pCell = aCellIter.GetNext( nCellCol, nCellRow );

    if ( pPattern && IsGreater( nNextCol, nNextRow, nAttrCol2, nAttrRow ) )
        pPattern = aAttrIter.GetNext( nAttrCol1, nAttrCol2, nAttrRow );

    if ( pPattern && nAttrRow == nNextRow && nAttrCol1 < nNextCol )
        nAttrCol1 = nNextCol;

    // Find next area
    bool bFound = true;
    bool bUseCell = false;

    if ( pCell && pPattern )
    {
        if ( IsGreater( nCellCol, nCellRow, nAttrCol1, nAttrRow ) ) // Only attributes at the beginning?
        {
            maFoundCell.clear();
            pFoundPattern = pPattern;
            nFoundRow = nAttrRow;
            nFoundStartCol = nAttrCol1;
            if ( nCellRow == nAttrRow && nCellCol <= nAttrCol2 ) // Area also contains cell?
                nFoundEndCol = nCellCol - 1; // Only until right before the cell
            else
                nFoundEndCol = nAttrCol2; // Everything
        }
        else
        {
            bUseCell = true;
            if ( nAttrRow == nCellRow && nAttrCol1 == nCellCol ) // Attributes on the cell?
                pFoundPattern = pPattern;
            else
                pFoundPattern = NULL;
        }
    }
    else if ( pCell ) // Just a cell -> take over right away
    {
        pFoundPattern = NULL;
        bUseCell = true; // Cell position
    }
    else if ( pPattern ) // Just attributes -> take over right away
    {
        maFoundCell.clear();
        pFoundPattern = pPattern;
        nFoundRow = nAttrRow;
        nFoundStartCol = nAttrCol1;
        nFoundEndCol = nAttrCol2;
    }
    else // Nothing
        bFound = false;

    if ( bUseCell ) // Cell position
    {
        if (pCell)
            maFoundCell = *pCell;
        else
            maFoundCell.clear();

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

const ScRefCellValue& ScUsedAreaIterator::GetCell() const
{
    return maFoundCell;
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
    if ( ValidTab(nTab) && nTab < pDoc->GetTableCount() && pDoc->maTabs[nTab] )
        pColIter = pDoc->maTabs[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
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
            pColIter = pDoc->maTabs[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
        else
            pColIter = NULL;
    }
    return NULL;  // Nothing anymore
}

// ============================================================================

ScDocRowHeightUpdater::TabRanges::TabRanges(SCTAB nTab) :
    mnTab(nTab), mpRanges(new ScFlatBoolRowSegments)
{
}

ScDocRowHeightUpdater::ScDocRowHeightUpdater(ScDocument& rDoc, OutputDevice* pOutDev, double fPPTX, double fPPTY, const vector<TabRanges>* pTabRangesArray) :
    mrDoc(rDoc), mpOutDev(pOutDev), mfPPTX(fPPTX), mfPPTY(fPPTY), mpTabRangesArray(pTabRangesArray)
{
}

void ScDocRowHeightUpdater::update()
{
    if (!mpTabRangesArray || mpTabRangesArray->empty())
    {
        // No ranges defined. Update all rows in all tables.
        updateAll();
        return;
    }

    sal_uInt32 nCellCount = 0;
    vector<TabRanges>::const_iterator itr = mpTabRangesArray->begin(), itrEnd = mpTabRangesArray->end();
    for (; itr != itrEnd; ++itr)
    {
        ScFlatBoolRowSegments::RangeData aData;
        ScFlatBoolRowSegments::RangeIterator aRangeItr(*itr->mpRanges);
        for (bool bFound = aRangeItr.getFirst(aData); bFound; bFound = aRangeItr.getNext(aData))
        {
            if (!aData.mbValue)
                continue;

            nCellCount += aData.mnRow2 - aData.mnRow1 + 1;
        }
    }

    ScProgress aProgress(mrDoc.GetDocumentShell(), ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), nCellCount);

    Fraction aZoom(1, 1);
    itr = mpTabRangesArray->begin();
    sal_uInt32 nProgressStart = 0;
    for (; itr != itrEnd; ++itr)
    {
        SCTAB nTab = itr->mnTab;
        if (!ValidTab(nTab) || nTab >= mrDoc.GetTableCount() || !mrDoc.maTabs[nTab])
            continue;

        ScFlatBoolRowSegments::RangeData aData;
        ScFlatBoolRowSegments::RangeIterator aRangeItr(*itr->mpRanges);
        for (bool bFound = aRangeItr.getFirst(aData); bFound; bFound = aRangeItr.getNext(aData))
        {
            if (!aData.mbValue)
                continue;

            mrDoc.maTabs[nTab]->SetOptimalHeight(
                aData.mnRow1, aData.mnRow2, 0, mpOutDev, mfPPTX, mfPPTY, aZoom, aZoom, false, &aProgress, nProgressStart);

            nProgressStart += aData.mnRow2 - aData.mnRow1 + 1;
        }
    }
}

void ScDocRowHeightUpdater::updateAll()
{
    sal_uInt32 nCellCount = 0;
    for (SCTAB nTab = 0; nTab < mrDoc.GetTableCount(); ++nTab)
    {
        if (!ValidTab(nTab) || !mrDoc.maTabs[nTab])
            continue;

        nCellCount += mrDoc.maTabs[nTab]->GetWeightedCount();
    }

    ScProgress aProgress(mrDoc.GetDocumentShell(), ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), nCellCount);

    Fraction aZoom(1, 1);
    sal_uLong nProgressStart = 0;
    for (SCTAB nTab = 0; nTab < mrDoc.GetTableCount(); ++nTab)
    {
        if (!ValidTab(nTab) || !mrDoc.maTabs[nTab])
            continue;

        mrDoc.maTabs[nTab]->SetOptimalHeight(
            0, MAXROW, 0, mpOutDev, mfPPTX, mfPPTY, aZoom, aZoom, false, &aProgress, nProgressStart);

        nProgressStart += mrDoc.maTabs[nTab]->GetWeightedCount();
    }
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
    if ( ValidTab(nTab) && nTab < pDoc->GetTableCount() && pDoc->maTabs[nTab] )
    {
        pColIter = pDoc->maTabs[nTab]->aCol[nIterStartCol].CreateAttrIterator( nStartRow, nEndRow );
        while ( nIterEndCol < nEndCol &&
                pDoc->maTabs[nTab]->aCol[nIterEndCol].IsAllAttrEqual(
                    pDoc->maTabs[nTab]->aCol[nIterEndCol+1], nStartRow, nEndRow ) )
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
        pColIter = pDoc->maTabs[nTab]->aCol[nIterStartCol].CreateAttrIterator( nNextRow, nEndRow );
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
            pColIter = pDoc->maTabs[nTab]->aCol[nIterStartCol].CreateAttrIterator( nStartRow, nEndRow );
            while ( nIterEndCol < nEndCol &&
                    pDoc->maTabs[nTab]->aCol[nIterEndCol].IsAllAttrEqual(
                        pDoc->maTabs[nTab]->aCol[nIterEndCol+1], nStartRow, nEndRow ) )
                ++nIterEndCol;
        }
        else
            pColIter = NULL;
    }
    return NULL; // Nothing anymore
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
