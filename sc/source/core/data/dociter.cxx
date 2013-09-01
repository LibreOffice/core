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
#include "scmatrix.hxx"

#include "tools/fract.hxx"
#include "editeng/editobj.hxx"

#include <vector>

using ::rtl::math::approxEqual;
using ::std::vector;
using ::std::set;

// STATIC DATA -----------------------------------------------------------

namespace {

void upperCase(OUString& rStr)
{
    rStr = ScGlobal::pCharClass->uppercase(rStr.trim());
}

template<typename _Iter>
void incBlock(std::pair<_Iter, size_t>& rPos)
{
    // Move to the next block.
    ++rPos.first;
    rPos.second = 0;
}

template<typename _Iter>
void incPos(std::pair<_Iter, size_t>& rPos)
{
    if (rPos.second + 1 < rPos.first->size)
        // Increment within the block.
        ++rPos.second;
    else
        incBlock(rPos);
}

template<typename _Iter>
size_t toLogicalPos(const std::pair<_Iter, size_t>& rPos)
{
    return rPos.first->position + rPos.second;
}

}

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
    maStartPos(rRange.aStart),
    maEndPos(rRange.aEnd),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    bNumValid( false ),
    bSubTotal(bSTotal),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
    bTextAsZero( bTextZero )
{
    SCTAB nDocMaxTab = pDocument->GetTableCount() - 1;

    if (!ValidCol(maStartPos.Col())) maStartPos.SetCol(MAXCOL);
    if (!ValidCol(maEndPos.Col())) maEndPos.SetCol(MAXCOL);
    if (!ValidRow(maStartPos.Row())) maStartPos.SetRow(MAXROW);
    if (!ValidRow(maEndPos.Row())) maEndPos.SetRow(MAXROW);
    if (!ValidTab(maStartPos.Tab()) || maStartPos.Tab() > nDocMaxTab) maStartPos.SetTab(nDocMaxTab);
    if (!ValidTab(maEndPos.Tab()) || maEndPos.Tab() > nDocMaxTab) maEndPos.SetTab(nDocMaxTab);

    nNumFormat = 0; // Initialized in GetNumberFormat
    pAttrArray = 0;
    nAttrEndRow = 0;
}

SCROW ScValueIterator::GetRow() const
{
    // Position of the head of the current block + offset within the block
    // equals the logical element position.
    return maCurPos.first->position + maCurPos.second;
}

void ScValueIterator::IncBlock()
{
    ++maCurPos.first;
    maCurPos.second = 0;
}

void ScValueIterator::IncPos()
{
    if (maCurPos.second + 1 < maCurPos.first->size)
        // Move within the same block.
        ++maCurPos.second;
    else
        // Move to the next block.
        IncBlock();
}

void ScValueIterator::SetPos(size_t nPos)
{
    maCurPos = mpCells->position(maCurPos.first, nPos);
}

bool ScValueIterator::GetThis(double& rValue, sal_uInt16& rErr)
{
    while (true)
    {
        bool bNextColumn = maCurPos.first == mpCells->end();
        if (!bNextColumn)
        {
            if (GetRow() > maEndPos.Row())
                bNextColumn = true;
        }

        ScColumn* pCol = NULL;
        if (bNextColumn)
        {
            // Find the next available column.
            do
            {
                ++mnCol;
                if (mnCol > maEndPos.Col())
                {
                    mnCol = maStartPos.Col();
                    ++mnTab;
                    if (mnTab > maEndPos.Tab())
                    {
                        rErr = 0;
                        return false; // Over and out
                    }
                }
                pCol = &(pDoc->maTabs[mnTab])->aCol[mnCol];
            }
            while (pCol->IsEmptyData());

            mpCells = &pCol->maCells;
            maCurPos = mpCells->position(maStartPos.Row());
        }

        SCROW nCurRow = GetRow();
        SCROW nLastRow;
        if (bSubTotal && pDoc->maTabs[mnTab]->RowFiltered(nCurRow, NULL, &nLastRow))
        {
            // Skip all filtered rows for subtotal mode.
            SetPos(nLastRow+1);
            continue;
        }

        switch (maCurPos.first->type)
        {
            case sc::element_type_numeric:
            {
                bNumValid = false;
                rValue = sc::numeric_block::at(*maCurPos.first->data, maCurPos.second);
                rErr = 0;
                if (bCalcAsShown)
                {
                    ScAttrArray_IterGetNumberFormat(nNumFormat, pAttrArray,
                        nAttrEndRow, pCol->pAttrArray, nCurRow, pDoc);
                    rValue = pDoc->RoundValueAsShown(rValue, nNumFormat);
                }
                return true; // Found it!
            }
            break;
            case sc::element_type_formula:
            {
                ScFormulaCell& rCell = *sc::formula_block::at(*maCurPos.first->data, maCurPos.second);
                if (bSubTotal && rCell.IsSubTotal())
                {
                    // Skip subtotal formula cells.
                    IncPos();
                    break;
                }

                if (rCell.GetErrorOrValue(rErr, rValue))
                {
                    bNumValid = false;
                    return true; // Found it!
                }
                else if (bTextAsZero)
                {
                    rValue = 0.0;
                    bNumValid = false;
                    return true;
                }
                IncPos();
            }
            break;
            case sc::element_type_string :
            case sc::element_type_edittext :
            {
                if (bTextAsZero)
                {
                    rErr = 0;
                    rValue = 0.0;
                    nNumFmtType = NUMBERFORMAT_NUMBER;
                    nNumFmtIndex = 0;
                    bNumValid = true;
                    return true;
                }
                IncBlock();
            }
            break;
            case sc::element_type_empty:
            default:
                // Skip the whole block.
                IncBlock();
        }
    }
}

void ScValueIterator::GetCurNumFmtInfo( short& nType, sal_uLong& nIndex )
{
    if (!bNumValid && mnTab < pDoc->GetTableCount())
    {
        SCROW nCurRow = GetRow();
        const ScColumn* pCol = &(pDoc->maTabs[mnTab])->aCol[mnCol];
        nNumFmtIndex = pCol->GetNumberFormat(nCurRow);
        nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
        bNumValid = true;
    }

    nType = nNumFmtType;
    nIndex = nNumFmtIndex;
}

bool ScValueIterator::GetFirst(double& rValue, sal_uInt16& rErr)
{
    mnCol = maStartPos.Col();
    mnTab = maStartPos.Tab();

    ScTable* pTab = pDoc->FetchTable(mnTab);
    if (!pTab)
        return false;

    nNumFormat = 0; // Initialized in GetNumberFormat
    pAttrArray = 0;
    nAttrEndRow = 0;

    mpCells = &pTab->aCol[maStartPos.Col()].maCells;
    maCurPos = mpCells->position(maStartPos.Row());
    return GetThis(rValue, rErr);
}

bool ScValueIterator::GetNext(double& rValue, sal_uInt16& rErr)
{
    IncPos();
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

const sc::CellStoreType* ScDBQueryDataIterator::GetColumnCellStore(ScDocument& rDoc, SCTAB nTab, SCCOL nCol)
{
    ScTable* pTab = rDoc.FetchTable(nTab);
    if (!pTab)
        return NULL;

    return &pTab->aCol[nCol].maCells;
}

const ScAttrArray* ScDBQueryDataIterator::GetAttrArrayByCol(ScDocument& rDoc, SCTAB nTab, SCCOL nCol)
{
    if (nTab >= rDoc.GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    ScColumn* pCol = &rDoc.maTabs[nTab]->aCol[nCol];
    return pCol->pAttrArray;
}

bool ScDBQueryDataIterator::IsQueryValid(
    ScDocument& rDoc, const ScQueryParam& rParam, SCTAB nTab, SCROW nRow, ScRefCellValue& rCell)
{
    if (nTab >= rDoc.GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    return rDoc.maTabs[nTab]->ValidQuery(nRow, rParam, &rCell);
}

// ----------------------------------------------------------------------------

ScDBQueryDataIterator::DataAccessInternal::DataAccessInternal(const ScDBQueryDataIterator* pParent, ScDBQueryParamInternal* pParam, ScDocument* pDoc) :
    DataAccess(pParent),
    mpCells(NULL),
    mpParam(pParam),
    mpDoc(pDoc),
    bCalcAsShown( pDoc->GetDocOptions().IsCalcAsShown() )
{
    nCol = mpParam->mnField;
    nRow = mpParam->nRow1;
    nTab = mpParam->nTab;
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
    // Start with the current row position, and find the first row position
    // that satisfies the query.

    // TODO: The following line nFirstQueryField is supposed to be used some
    // way. Find out how it's supposed to be used and fix this bug.

    // SCCOLROW nFirstQueryField = mpParam->GetEntry(0).nField;
    while (true)
    {
        if (maCurPos.first == mpCells->end() || nRow > mpParam->nRow2)
        {
            // Bottom of the range reached. Bail out.
            rValue.mnError = 0;
            return false;
        }

        if (maCurPos.first->type == sc::element_type_empty)
        {
            // Skip the whole empty block.
            incBlock();
            continue;
        }

        ScRefCellValue aCell;
        aCell.assign(maCurPos.first, maCurPos.second);

        if (ScDBQueryDataIterator::IsQueryValid(*mpDoc, *mpParam, nTab, nRow, aCell))
        {
            switch (aCell.meType)
            {
                case CELLTYPE_VALUE:
                {
                    rValue.mfValue = aCell.mfValue;
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
                    if (aCell.mpFormula->IsValue())
                    {
                        rValue.mfValue = aCell.mpFormula->GetValue();
                        rValue.mbIsNumber = true;
                        mpDoc->GetNumberFormatInfo(
                            nNumFmtType, nNumFmtIndex, ScAddress(nCol, nRow, nTab));
                        rValue.mnError = aCell.mpFormula->GetErrCode();
                        return true; // Found it!
                    }
                    else if(mpParam->mbSkipString)
                        incPos();
                    else
                    {
                        rValue.maString = aCell.mpFormula->GetString();
                        rValue.mfValue = 0.0;
                        rValue.mnError = aCell.mpFormula->GetErrCode();
                        rValue.mbIsNumber = false;
                        return true;
                    }
                }
                break;
                case CELLTYPE_STRING:
                case CELLTYPE_EDIT:
                    if (mpParam->mbSkipString)
                        incPos();
                    else
                    {
                        rValue.maString = aCell.getString(mpDoc);
                        rValue.mfValue = 0.0;
                        rValue.mnError = 0;
                        rValue.mbIsNumber = false;
                        return true;
                    }
                break;
                default:
                    incPos();
            }
        }
        else
            incPos();
    }
// statement unreachable
}

bool ScDBQueryDataIterator::DataAccessInternal::getFirst(Value& rValue)
{
    if (mpParam->bHasHeader)
        ++nRow;

    mpCells = ScDBQueryDataIterator::GetColumnCellStore(*mpDoc, nTab, nCol);
    if (!mpCells)
        return false;

    maCurPos = mpCells->position(nRow);
    return getCurrent(rValue);
}

bool ScDBQueryDataIterator::DataAccessInternal::getNext(Value& rValue)
{
    if (!mpCells || maCurPos.first == mpCells->end())
        return false;

    incPos();
    return getCurrent(rValue);
}

void ScDBQueryDataIterator::DataAccessInternal::incBlock()
{
    ++maCurPos.first;
    maCurPos.second = 0;

    nRow = maCurPos.first->position;
}

void ScDBQueryDataIterator::DataAccessInternal::incPos()
{
    if (maCurPos.second + 1 < maCurPos.first->size)
    {
        // Move within the same block.
        ++maCurPos.second;
        ++nRow;
    }
    else
        // Move to the next block.
        incBlock();
}

void ScDBQueryDataIterator::DataAccessInternal::setPos(size_t nPos)
{
    maCurPos = mpCells->position(maCurPos.first, nPos);
    nRow = nPos;
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
                upperCase(aMatStr);
                OUString aQueryStr = rEntry.GetQueryItem().maString;
                upperCase(aQueryStr);
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
    mbSubTotal(bSTotal)
{
    init();
}

void ScCellIterator::incBlock()
{
    ++maCurColPos.first;
    maCurColPos.second = 0;

    maCurPos.SetRow(maCurColPos.first->position);
}

void ScCellIterator::incPos()
{
    if (maCurColPos.second + 1 < maCurColPos.first->size)
    {
        // Move within the same block.
        ++maCurColPos.second;
        maCurPos.IncRow();
    }
    else
        // Move to the next block.
        incBlock();
}

void ScCellIterator::setPos(size_t nPos)
{
    maCurColPos = getColumn()->maCells.position(maCurColPos.first, nPos);
    maCurPos.SetRow(nPos);
}

const ScColumn* ScCellIterator::getColumn() const
{
    return &mpDoc->maTabs[maCurPos.Tab()]->aCol[maCurPos.Col()];
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
    const ScColumn* pCol = getColumn();

    while (true)
    {
        bool bNextColumn = maCurColPos.first == pCol->maCells.end();
        if (!bNextColumn)
        {
            if (maCurPos.Row() > maEndPos.Row())
                bNextColumn = true;
        }

        if (bNextColumn)
        {
            // Move to the next column.
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
                pCol = getColumn();
            }
            while (pCol->IsEmptyData());

            maCurColPos = pCol->maCells.position(maCurPos.Row());
        }

        if (maCurColPos.first->type == sc::element_type_empty)
        {
            incBlock();
            continue;
        }

        SCROW nLastRow;
        if (mbSubTotal && pCol->GetDoc().maTabs[maCurPos.Tab()]->RowFiltered(maCurPos.Row(), NULL, &nLastRow))
        {
            // Skip all filtered rows for subtotal mode.
            setPos(nLastRow+1);
            continue;
        }

        if (maCurColPos.first->type == sc::element_type_formula)
        {
            const ScFormulaCell* pCell = sc::formula_block::at(*maCurColPos.first->data, maCurColPos.second);
            if (pCell->IsSubTotal())
            {
                // Skip subtotal formula cells.
                incPos();
                continue;
            }
        }

        maCurCell.assign(maCurColPos.first, maCurColPos.second);
        return true;
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
    const ScColumn* pCol = getColumn();

    maCurColPos = pCol->maCells.position(maCurPos.Row());
    return getCurrent();
}

bool ScCellIterator::next()
{
    incPos();
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

void ScQueryCellIterator::InitPos()
{
    nRow = mpParam->nRow1;
    if (mpParam->bHasHeader && mpParam->bByRow)
        ++nRow;
    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
    maCurPos = pCol->maCells.position(nRow);
}

void ScQueryCellIterator::IncPos()
{
    if (maCurPos.second + 1 < maCurPos.first->size)
    {
        // Move within the same block.
        ++maCurPos.second;
        ++nRow;
    }
    else
        // Move to the next block.
        IncBlock();
}

void ScQueryCellIterator::IncBlock()
{
    ++maCurPos.first;
    maCurPos.second = 0;

    nRow = maCurPos.first->position;
}

bool ScQueryCellIterator::GetThis()
{
    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    const ScQueryEntry& rEntry = mpParam->GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

    SCCOLROW nFirstQueryField = rEntry.nField;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        rItem.meType != ScQueryEntry::ByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !mpParam->bHasHeader && rItem.meType == ScQueryEntry::ByString &&
        ((mpParam->bByRow && nRow == mpParam->nRow1) ||
         (!mpParam->bByRow && nCol == mpParam->nCol1));

    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
    while (true)
    {
        bool bNextColumn = maCurPos.first == pCol->maCells.end();
        if (!bNextColumn)
        {
            if (nRow > mpParam->nRow2)
                bNextColumn = true;
        }

        if (bNextColumn)
        {
            do
            {
                if ( ++nCol > mpParam->nCol2 )
                    return false; // Over and out
                if ( bAdvanceQuery )
                {
                    AdvanceQueryParamEntryField();
                    nFirstQueryField = rEntry.nField;
                }
                pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
            }
            while (pCol->IsEmptyData());

            InitPos();

            bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
                !mpParam->bHasHeader && rItem.meType == ScQueryEntry::ByString &&
                mpParam->bByRow;
        }

        if (maCurPos.first->type == sc::element_type_empty)
        {
            IncBlock();
            continue;
        }

        ScRefCellValue aCell;
        aCell.assign(maCurPos.first, maCurPos.second);

        if (bAllStringIgnore && aCell.hasString())
            IncPos();
        else
        {
            bool bTestEqualCondition = false;
            if ( pDoc->maTabs[nTab]->ValidQuery( nRow, *mpParam,
                    (nCol == static_cast<SCCOL>(nFirstQueryField) ? &aCell : NULL),
                    (nTestEqualCondition ? &bTestEqualCondition : NULL) ) )
            {
                if ( nTestEqualCondition && bTestEqualCondition )
                    nTestEqualCondition |= nTestEqualConditionMatched;
                return !aCell.isEmpty(); // Found it!
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
                    return false;
                }
                bool bStop;
                if (bFirstStringIgnore)
                {
                    if (aCell.hasString())
                    {
                        IncPos();
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
                    return false;
                }
            }
            else
                IncPos();
        }
        bFirstStringIgnore = false;
    }
}

bool ScQueryCellIterator::GetFirst()
{
    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    nCol = mpParam->nCol1;
    InitPos();
    return GetThis();
}

bool ScQueryCellIterator::GetNext()
{
    IncPos();
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
    nFoundCol = MAXCOL+1;
    nFoundRow = MAXROW+1;
    SetStopOnMismatch( true ); // assume sorted keys
    SetTestEqualCondition( true );
    bIgnoreMismatchOnLeadingStrings = bIgnoreMismatchOnLeadingStringsP;
    bool bRegExp = mpParam->bRegExp && mpParam->GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByString;
    bool bBinary = !bRegExp && mpParam->bByRow && (mpParam->GetEntry(0).eOp ==
            SC_LESS_EQUAL || mpParam->GetEntry(0).eOp == SC_GREATER_EQUAL);
    if (bBinary ? (BinarySearch() ? GetThis() : 0) : GetFirst())
    {
        // First equal entry or last smaller than (greater than) entry.
        PositionType aPosSave;
        bool bNext = false;
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            aPosSave = maCurPos;
        }
        while ( !IsEqualConditionFulfilled() && (bNext = GetNext()));

        // There may be no pNext but equal condition fulfilled if regular
        // expressions are involved. Keep the found entry and proceed.
        if (!bNext && !IsEqualConditionFulfilled())
        {
            // Step back to last in range and adjust position markers for
            // GetNumberFormat() or similar.
            nCol = nFoundCol;
            nRow = nFoundRow;
            maCurPos = aPosSave;
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
        PositionType aPosSave;
        bIgnoreMismatchOnLeadingStrings = false;
        SetTestEqualCondition( false );
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            aPosSave = maCurPos;
        } while (GetNext());

        // Step back conditions are the same as above
        nCol = nFoundCol;
        nRow = nFoundRow;
        maCurPos = aPosSave;
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
            PositionType aPosSave;
            do
            {
                nFoundCol = GetCol();
                nFoundRow = GetRow();
                aPosSave = maCurPos;
                SetStopOnMismatch( true );
            } while (GetNext());
            nCol = nFoundCol;
            nRow = nFoundRow;
            maCurPos = aPosSave;
        }
    }
    return (nFoundCol <= MAXCOL) && (nFoundRow <= MAXROW);
}


bool ScQueryCellIterator::BinarySearch()
{
    // TODO: This will be extremely slow with mdds::multi_type_vector.

    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    nCol = mpParam->nCol1;
    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
    if (pCol->IsEmptyData())
        return false;

    PositionType aHiPos, aLoPos;
    ScRefCellValue aCell;

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

    aLoPos = pCol->maCells.position(nRow);
    if (bFirstStringIgnore && aLoPos.first->type == sc::element_type_string)
    {
        OUString aCellStr;
        sal_uLong nFormat = pCol->GetNumberFormat(toLogicalPos(aLoPos));
        aCell.assign(aLoPos.first, aLoPos.second);
        ScCellFormat::GetInputString(aCell, nFormat, aCellStr, rFormatter, pDoc);
        sal_Int32 nTmp = pCollator->compareString(aCellStr, rEntry.GetQueryItem().maString);
        if ((rEntry.eOp == SC_LESS_EQUAL && nTmp > 0) ||
                (rEntry.eOp == SC_GREATER_EQUAL && nTmp < 0) ||
                (rEntry.eOp == SC_EQUAL && nTmp != 0))
        {
            // Skip the first string value at low point.
            incPos(aLoPos);
        }
    }

    aHiPos = pCol->maCells.position(mpParam->nRow2);
    if (bAllStringIgnore)
    {
        // Skip all string cells, but never go past the high point.
        if (aLoPos.first->type == sc::element_type_string)
        {
            if (aLoPos.first == pCol->maCells.end())
                // This is the last block. Move to the last element in this block.
                aLoPos.second = aLoPos.first->size - 1;
            else
                // Move to the next block.
                incBlock(aLoPos);
        }

        if (toLogicalPos(aLoPos) > toLogicalPos(aHiPos))
            // Never go past the high point.
            aLoPos = aHiPos;
    }

    // Bookkeeping values for breaking up the binary search in case the data
    // range isn't strictly sorted.
    PositionType aLastInRange = aLoPos;
    PositionType aFirstLastInRange = aLastInRange;
    double fLastInRangeValue = bLessEqual ?
        -(::std::numeric_limits<double>::max()) :
            ::std::numeric_limits<double>::max();
    OUString aLastInRangeString;
    if (!bLessEqual)
        aLastInRangeString = OUString(sal_Unicode(0xFFFF));
    if (aLastInRange.first != pCol->maCells.end())
    {
        aCell.assign(aLastInRange.first, aLastInRange.second);
        if (aCell.hasString())
        {
            sal_uLong nFormat = pCol->GetNumberFormat(toLogicalPos(aLastInRange));
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
    size_t nLogicalLow = toLogicalPos(aLoPos), nLogicalHigh = toLogicalPos(aHiPos);
    while (nLogicalLow <= nLogicalHigh && !bDone)
    {
        size_t nMid = (nLogicalLow+nLogicalHigh)/2;
        size_t i = nMid;
        if (i > nLogicalHigh)
        {
            if (nMid > 0)
                nLogicalHigh = nMid - 1;
            else
                bDone = true;
            continue; // while
        }

        PositionType aPos = pCol->maCells.position(i);
        bool bStr = aPos.first->type == sc::element_type_string;
        nRes = 0;

        // compares are content<query:-1, content>query:1
        // Cell value comparison similar to ScTable::ValidQuery()
        if (!bStr && !bByString)
        {
            double nCellVal;
            aCell.assign(aPos.first, aPos.second);
            switch (aCell.meType)
            {
                case CELLTYPE_VALUE :
                    nCellVal = aCell.mfValue;
                    break;
                case CELLTYPE_FORMULA :
                    nCellVal = aCell.mpFormula->GetValue();
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
                        aLastInRange = aPos;
                    }
                    else if (fLastInRangeValue > nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        aLastInRange = aFirstLastInRange;
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
                        aLastInRange = aPos;
                    }
                    else if (fLastInRangeValue < nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        aLastInRange = aFirstLastInRange;
                        bDone = true;
                    }
                }
            }
        }
        else if (bStr && bByString)
        {
            OUString aCellStr;
            sal_uLong nFormat = pCol->GetNumberFormat(i);
            aCell.assign(aPos.first, aPos.second);
            ScCellFormat::GetInputString(aCell, nFormat, aCellStr, rFormatter, pDoc);

            nRes = pCollator->compareString(aCellStr, rEntry.GetQueryItem().maString);
            if (nRes < 0 && bLessEqual)
            {
                sal_Int32 nTmp = pCollator->compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp < 0)
                {
                    aLastInRangeString = aCellStr;
                    aLastInRange = aPos;
                }
                else if (nTmp > 0)
                {
                    // not strictly sorted, continue with GetThis()
                    aLastInRange = aFirstLastInRange;
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
                    aLastInRange = aPos;
                }
                else if (nTmp < 0)
                {
                    // not strictly sorted, continue with GetThis()
                    aLastInRange = aFirstLastInRange;
                    bDone = true;
                }
            }
        }
        else if (!bStr && bByString)
        {
            nRes = -1; // numeric < string
            if (bLessEqual)
                aLastInRange = aPos;
        }
        else // if (bStr && !bByString)
        {
            nRes = 1; // string > numeric
            if (!bLessEqual)
                aLastInRange = aPos;
        }
        if (nRes < 0)
        {
            if (bLessEqual)
                nLogicalLow = nMid + 1;
            else // assumed to be SC_GREATER_EQUAL
            {
                if (nMid > 0)
                    nLogicalHigh = nMid - 1;
                else
                    bDone = true;
            }
        }
        else if (nRes > 0)
        {
            if (bLessEqual)
            {
                if (nMid > 0)
                    nLogicalHigh = nMid - 1;
                else
                    bDone = true;
            }
            else // assumed to be SC_GREATER_EQUAL
                nLogicalLow = nMid + 1;
        }
        else
        {
            aLoPos = aPos;
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
        aLoPos = aLastInRange;
    }

    if (aLoPos.first != pCol->maCells.end() && toLogicalPos(aLoPos) <= static_cast<size_t>(mpParam->nRow2))
    {
        nRow = toLogicalPos(aLoPos);
        maCurPos = aLoPos;
        return true;
    }
    else
    {
        nRow = mpParam->nRow2 + 1;
        // Set current position to the last possible row.
        maCurPos.first = pCol->maCells.end();
        --maCurPos.first;
        maCurPos.second = maCurPos.first->size - 1;
        return false;
    }
}


//-------------------------------------------------------------------------------

ScHorizontalCellIterator::ScHorizontalCellIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) :
    maColPositions(nCol2-nCol1+1),
    pDoc( pDocument ),
    mnTab( nTable ),
    nStartCol( nCol1 ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 ),
    mnCol( nCol1 ),
    mnRow( nRow1 ),
    bMore(false)
{
    if (mnTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");

    pNextRows = new SCROW[ nCol2-nCol1+1 ];
    pNextIndices = new SCSIZE[ nCol2-nCol1+1 ];

    SetTab( mnTab );
}

ScHorizontalCellIterator::~ScHorizontalCellIterator()
{
    delete [] pNextRows;
    delete [] pNextIndices;
}

void ScHorizontalCellIterator::SetTab( SCTAB nTabP )
{
    bMore = false;
    mnTab = nTabP;
    mnRow = nStartRow;
    mnCol = nStartCol;

    // Set the start position in each column.
    for (SCCOL i = nStartCol; i <= nEndCol; ++i)
    {
        ScColumn* pCol = &pDoc->maTabs[mnTab]->aCol[i];
        ColParam& rParam = maColPositions[i-nStartCol];
        rParam.maPos = pCol->maCells.position(nStartRow).first;
        rParam.maEnd = pCol->maCells.end();
        if (rParam.maPos != rParam.maEnd)
            bMore = true;
    }

    if (!bMore)
        return;

    ColParam& rParam = maColPositions[0];
    if (rParam.maPos == rParam.maEnd || rParam.maPos->type == sc::element_type_empty)
        // Skip to the first non-empty cell.
        Advance();
}

ScRefCellValue* ScHorizontalCellIterator::GetNext( SCCOL& rCol, SCROW& rRow )
{
    if (!bMore)
        return NULL;

    // Return the current non-empty cell, and move the cursor to the next one.
    rCol = mnCol;
    rRow = mnRow;

    ColParam& r = maColPositions[mnCol-nStartCol];
    size_t nOffset = static_cast<size_t>(mnRow) - r.maPos->position;
    maCurCell.assign(r.maPos, nOffset);
    Advance();

    return &maCurCell;
}

bool ScHorizontalCellIterator::GetPos( SCCOL& rCol, SCROW& rRow )
{
    rCol = mnCol;
    rRow = mnRow;
    return bMore;
}

namespace {

bool advanceBlock(size_t nRow, sc::CellStoreType::const_iterator& rPos, const sc::CellStoreType::const_iterator& rEnd)
{
    if (nRow < rPos->position + rPos->size)
        // Block already contains the specified row. Nothing to do.
        return true;

    // This block is behind the current row position. Advance the block.
    for (++rPos; rPos != rEnd; ++rPos)
    {
        if (nRow < rPos->position + rPos->size)
            // Found the block that contains the specified row.
            return true;
    }

    // No more blocks.
    return false;
}

}

void ScHorizontalCellIterator::Advance()
{
    // Find the next non-empty cell in the current row.
    for (SCCOL i = mnCol+1; i <= nEndCol; ++i)
    {
        ColParam& r = maColPositions[i-nStartCol];
        if (r.maPos == r.maEnd)
            continue;

        size_t nRow = static_cast<size_t>(mnRow);
        if (nRow < r.maPos->position)
            continue;

        if (!advanceBlock(nRow, r.maPos, r.maEnd))
            continue;

        if (r.maPos->type == sc::element_type_empty)
            continue;

        // Found in the current row.
        mnCol = i;
        bMore = true;
        return;
    }

    // Move to the next row that has at least one non-empty cell.
    ++mnRow;
    while (mnRow <= nEndRow)
    {
        size_t nRow = static_cast<size_t>(mnRow);
        size_t nNextRow = MAXROW+1;
        size_t nNextRowPos = 0;
        for (size_t i = nNextRowPos, n = maColPositions.size(); i < n; ++i)
        {
            ColParam& r = maColPositions[i];
            if (r.maPos == r.maEnd)
                // This column has ended.
                continue;

            if (nRow < r.maPos->position)
            {
                // This block is ahread of the current row position. Skip it.
                if (r.maPos->position < nNextRow)
                {
                    nNextRow = r.maPos->position;
                    nNextRowPos = i;
                }
                continue;
            }

            if (!advanceBlock(nRow, r.maPos, r.maEnd))
                continue;

            if (r.maPos->type == sc::element_type_empty)
            {
                // Empty block. Move to the next block and try next column.
                ++r.maPos;
                if (r.maPos->position < nNextRow)
                {
                    nNextRow = r.maPos->position;
                    nNextRowPos = i;
                }
                continue;
            }

            // Found a non-empty cell block!
            mnCol = i + nStartCol;
            mnRow = nRow;
            bMore = true;
            return;
        }

        if (nNextRow > static_cast<size_t>(MAXROW))
        {
            // No more blocks to search.
            bMore = false;
            return;
        }

        mnRow = nNextRow; // move to the next non-empty row.
    }

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
