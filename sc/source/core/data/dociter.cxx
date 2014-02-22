/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <rowheightcontext.hxx>

#include "tools/fract.hxx"
#include "editeng/editobj.hxx"
#include "svl/sharedstring.hxx"

#include <vector>

using ::rtl::math::approxEqual;
using ::std::vector;
using ::std::set;



#define debugiter(...)



namespace {

template<typename _Iter>
void incBlock(std::pair<_Iter, size_t>& rPos)
{
    
    ++rPos.first;
    rPos.second = 0;
}

template<typename _Iter>
void decBlock(std::pair<_Iter, size_t>& rPos)
{
    
    --rPos.first;
    rPos.second = rPos.first->size - 1;
}

template<typename _Iter>
void incPos(std::pair<_Iter, size_t>& rPos)
{
    if (rPos.second + 1 < rPos.first->size)
        
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

    nNumFormat = 0; 
    pAttrArray = 0;
    nAttrEndRow = 0;
}

SCROW ScValueIterator::GetRow() const
{
    
    
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
        
        ++maCurPos.second;
    else
        
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

        ScColumn* pCol = &(pDoc->maTabs[mnTab])->aCol[mnCol];
        if (bNextColumn)
        {
            
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
                        return false; 
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
                return true; 
            }
            break;
            case sc::element_type_formula:
            {
                ScFormulaCell& rCell = *sc::formula_block::at(*maCurPos.first->data, maCurPos.second);
                if (bSubTotal && rCell.IsSubTotal())
                {
                    
                    IncPos();
                    break;
                }

                if (rCell.GetErrorOrValue(rErr, rValue))
                {
                    bNumValid = false;
                    return true; 
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

    nNumFormat = 0; 
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
    ScDocument& rDoc, const ScQueryParam& rParam, SCTAB nTab, SCROW nRow, ScRefCellValue* pCell)
{
    if (nTab >= rDoc.GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    return rDoc.maTabs[nTab]->ValidQuery(nRow, rParam, pCell);
}



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
            rItem.maString.getString(), nIndex, rItem.mfVal);
        rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
    }
    nNumFormat = 0; 
    pAttrArray = 0;
    nAttrEndRow = 0;
}

ScDBQueryDataIterator::DataAccessInternal::~DataAccessInternal()
{
}

bool ScDBQueryDataIterator::DataAccessInternal::getCurrent(Value& rValue)
{
    
    

    
    
    SCCOLROW nFirstQueryField = mpParam->GetEntry(0).nField;
    ScRefCellValue aCell;

    while (true)
    {
        if (maCurPos.first == mpCells->end() || nRow > mpParam->nRow2)
        {
            
            rValue.mnError = 0;
            return false;
        }

        if (maCurPos.first->type == sc::element_type_empty)
        {
            
            incBlock();
            continue;
        }

        ScRefCellValue* pCell = NULL;
        if (nCol == static_cast<SCCOL>(nFirstQueryField))
        {
            aCell = sc::toRefCell(maCurPos.first, maCurPos.second);
            pCell = &aCell;
        }

        if (ScDBQueryDataIterator::IsQueryValid(*mpDoc, *mpParam, nTab, nRow, pCell))
        {
            if (!pCell)
                aCell = sc::toRefCell(maCurPos.first, maCurPos.second);
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
                    return true; 
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
                        return true; 
                    }
                    else if(mpParam->mbSkipString)
                        incPos();
                    else
                    {
                        rValue.maString = aCell.mpFormula->GetString().getString();
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
        
        ++maCurPos.second;
        ++nRow;
    }
    else
        
        incBlock();
}

void ScDBQueryDataIterator::DataAccessInternal::setPos(size_t nPos)
{
    maCurPos = mpCells->position(maCurPos.first, nPos);
    nRow = nPos;
}



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
    
    
    for ( ;mnCurRow < mnRows; ++mnCurRow)
    {
        const ScMatrix& rMat = *mpParam->mpMatrix;
        if (rMat.IsEmpty(mpParam->mnField, mnCurRow))
            
            continue;

        bool bIsStrVal = rMat.IsString(mpParam->mnField, mnCurRow);
        if (bIsStrVal && mpParam->mbSkipString)
            continue;

        if (isValidQuery(mnCurRow, rMat))
        {
            rValue.maString = rMat.GetString(mpParam->mnField, mnCurRow).getString();
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
                
                continue;
        }

        bool bValid = false;

        SCSIZE nField = static_cast<SCSIZE>(rEntry.nField);
        if (isQueryByValue(rItem, rMat, nField, nRow))
        {
            
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
            
            do
            {
                
                svl::SharedString aMatStr = rMat.GetString(nField, nRow);
                svl::SharedString aQueryStr = rEntry.GetQueryItem().maString;
                bool bDone = false;
                rtl_uString* p1 = mpParam->bCaseSens ? aMatStr.getData() : aMatStr.getDataIgnoreCase();
                rtl_uString* p2 = mpParam->bCaseSens ? aQueryStr.getData() : aQueryStr.getDataIgnoreCase();
                switch (rEntry.eOp)
                {
                    case SC_EQUAL:
                        bValid = (p1 == p2);
                        bDone = true;
                    break;
                    case SC_NOT_EQUAL:
                        bValid = (p1 != p2);
                        bDone = true;
                    break;
                    default:
                        ;
                }

                if (bDone)
                    break;

                
                sal_Int32 nCompare = rCollator.compareString(aMatStr.getString(), aQueryStr.getString());
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
            
            aResults.push_back(bValid);
        else if (rEntry.eConnect == SC_AND)
        {
            
            size_t n = aResults.size();
            aResults[n-1] = aResults[n-1] && bValid;
        }
        else
            
            aResults.push_back(bValid);
    }

    
    vector<bool>::const_iterator itr = aResults.begin(), itrEnd = aResults.end();
    for (; itr != itrEnd; ++itr)
        if (*itr)
            return true;

    return false;
}



ScDBQueryDataIterator::Value::Value() :
    mnError(0), mbIsNumber(true)
{
    ::rtl::math::setNan(&mfValue);
}



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
        
        ++maCurColPos.second;
        maCurPos.IncRow();
    }
    else
        
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
        maEndPos.IncTab(-1); 

    if (maStartPos.Tab() > maEndPos.Tab())
        maStartPos.SetTab(maEndPos.Tab());

    maCurPos = maStartPos;

    if (!mpDoc->maTabs[maCurPos.Tab()])
    {
        OSL_FAIL("Table not found");
        maStartPos = ScAddress(MAXCOL+1, MAXROW+1, MAXTAB+1); 
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
                        return false; 
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
            
            setPos(nLastRow+1);
            continue;
        }

        if (maCurColPos.first->type == sc::element_type_formula)
        {
            const ScFormulaCell* pCell = sc::formula_block::at(*maCurColPos.first->data, maCurColPos.second);
            if (pCell->IsSubTotal())
            {
                
                incPos();
                continue;
            }
        }

        maCurCell = sc::toRefCell(maCurColPos.first, maCurColPos.second);
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
            aRet.mpString = new svl::SharedString(*maCurCell.mpString);
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
    if (bMod) 
    {
        SCSIZE nCount = mpParam->GetEntryCount();
        for (i = 0; (i < nCount) && (mpParam->GetEntry(i).bDoQuery); ++i)
        {
            ScQueryEntry& rEntry = mpParam->GetEntry(i);
            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            sal_uInt32 nIndex = 0;
            bool bNumber = pDoc->GetFormatTable()->IsNumberFormat(
                rItem.maString.getString(), nIndex, rItem.mfVal);
            rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
        }
    }
    nNumFormat = 0; 
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
        
        ++maCurPos.second;
        ++nRow;
    }
    else
        
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
                    return false; 
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

        ScRefCellValue aCell = sc::toRefCell(maCurPos.first, maCurPos.second);

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
                return !aCell.isEmpty(); 
            }
            else if ( nStopOnMismatch )
            {
                
                
                
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
            break;  
    }
}


bool ScQueryCellIterator::FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
        SCROW& nFoundRow, bool bSearchForEqualAfterMismatch,
        bool bIgnoreMismatchOnLeadingStringsP )
{
    
    
    
    struct BoolResetter
    {
        bool& mr;
        bool  mb;
        BoolResetter( bool& r, bool b ) : mr(r), mb(r) { r = b; }
        ~BoolResetter() { mr = mb; }
    } aRangeLookupResetter( mpParam->mbRangeLookup, true);

    nFoundCol = MAXCOL+1;
    nFoundRow = MAXROW+1;
    SetStopOnMismatch( true ); 
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
        
        PositionType aPosSave;
        bool bNext = false;
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            aPosSave = maCurPos;
        }
        while ( !IsEqualConditionFulfilled() && (bNext = GetNext()));

        
        
        if (!bNext && !IsEqualConditionFulfilled())
        {
            
            
            SCCOL nColDiff = nCol - nFoundCol;
            nCol = nFoundCol;
            nRow = nFoundRow;
            maCurPos = aPosSave;
            if (mpParam->mbRangeLookup)
            {
                
                
                
                mpParam->mbRangeLookup = false;
                
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
                            break;  
                    }
                }
                
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
                        
                    }
                }
            }
            else
                break;  
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

        
        nCol = nFoundCol;
        nRow = nFoundRow;
        maCurPos = aPosSave;
        return true;
    }
    if ( (bSearchForEqualAfterMismatch || mpParam->bRegExp) &&
            StoppedOnMismatch() )
    {
        
        
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
                        
                    }
                }
            }
            else
                break;  
        }
        SetStopOnMismatch( false );
        SetTestEqualCondition( false );
        if (GetNext())
        {
            
            
            
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

namespace {

/**
 * This class sequentially indexes non-empty cells in order, from the top of
 * the block where the start row position is, to the bottom of the block
 * where the end row position is.  It skips all empty blocks that may be
 * present in between.
 *
 * The index value is an offset from the first element of the first block
 * disregarding all empty cell blocks.
 */
class NonEmptyCellIndexer
{
    typedef std::map<size_t, sc::CellStoreType::const_iterator> BlockMapType;

    BlockMapType maBlockMap;

    const sc::CellStoreType& mrCells;

    size_t mnLowIndex;
    size_t mnHighIndex;

    bool mbValid;

public:

    typedef std::pair<ScRefCellValue, SCROW> CellType;

    /**
     * @param rCells cell storage container
     * @param nStartRow logical start row position
     * @param nEndRow logical end row position, inclusive.
     * @param bSkipTopStrBlock when true, skip all leading string cells.
     */
    NonEmptyCellIndexer(
        const sc::CellStoreType& rCells, SCROW nStartRow, SCROW nEndRow, bool bSkipTopStrBlock ) :
        mrCells(rCells), mnLowIndex(0), mnHighIndex(0), mbValid(true)
    {
        if (nEndRow < nStartRow)
        {
            mbValid = false;
            return;
        }

        

        sc::CellStoreType::const_position_type aLoPos = mrCells.position(nStartRow);
        if (aLoPos.first->type == sc::element_type_empty)
            incBlock(aLoPos);

        if (aLoPos.first == rCells.end())
        {
            mbValid = false;
            return;
        }

        if (bSkipTopStrBlock)
        {
            
            while (aLoPos.first->type == sc::element_type_string ||
                   aLoPos.first->type == sc::element_type_edittext ||
                   aLoPos.first->type == sc::element_type_empty)
            {
                incBlock(aLoPos);
                if (aLoPos.first == rCells.end())
                {
                    mbValid = false;
                    return;
                }
            }
        }

        SCROW nFirstRow = aLoPos.first->position;
        SCROW nLastRow = aLoPos.first->position + aLoPos.first->size - 1;

        if (nFirstRow > nEndRow)
        {
            
            
            mbValid = false;
            return;
        }

        
        if (nFirstRow < nStartRow)
            mnLowIndex = nStartRow - nFirstRow;
        else
        {
            
            
            mnLowIndex = 0;
        }

        if (nEndRow < nLastRow)
        {
            assert(nEndRow > nFirstRow);
            mnHighIndex = nEndRow - nFirstRow;

            maBlockMap.insert(BlockMapType::value_type(aLoPos.first->size, aLoPos.first));
            return;
        }

        

        sc::CellStoreType::const_position_type aHiPos = mrCells.position(aLoPos.first, nEndRow);
        if (aHiPos.first->type == sc::element_type_empty)
        {
            
            decBlock(aHiPos);

            if (aHiPos.first == mrCells.begin())
            {
                mbValid = false;
                return;
            }
        }

        
        

        size_t nPos = 0;
        sc::CellStoreType::const_iterator itBlk = aLoPos.first;
        while (itBlk != aHiPos.first)
        {
            if (itBlk->type == sc::element_type_empty)
            {
                ++itBlk;
                continue;
            }

            nPos += itBlk->size;
            maBlockMap.insert(BlockMapType::value_type(nPos, itBlk));
            ++itBlk;

            if (itBlk->type == sc::element_type_empty)
                ++itBlk;

            assert(itBlk != mrCells.end());
        }

        assert(itBlk == aHiPos.first);
        nPos += itBlk->size;
        maBlockMap.insert(BlockMapType::value_type(nPos, itBlk));

        
        BlockMapType::const_reverse_iterator ri = maBlockMap.rbegin();
        mnHighIndex = ri->first;
        mnHighIndex -= ri->second->size;
        mnHighIndex += aHiPos.second;
    }

    sc::CellStoreType::const_position_type getPosition( size_t nIndex ) const
    {
        assert(mbValid);
        assert(mnLowIndex <= nIndex);
        assert(nIndex <= mnHighIndex);

        sc::CellStoreType::const_position_type aRet(mrCells.end(), 0);

        BlockMapType::const_iterator it = maBlockMap.upper_bound(nIndex);
        if (it == maBlockMap.end())
            return aRet;

        sc::CellStoreType::const_iterator itBlk = it->second;
        size_t nBlkIndex = it->first - itBlk->size; 
        assert(nBlkIndex <= nIndex);
        assert(nIndex < it->first);

        size_t nOffset = nIndex - nBlkIndex;
        aRet.first = itBlk;
        aRet.second = nOffset;
        return aRet;
    }

    CellType getCell( size_t nIndex ) const
    {
        std::pair<ScRefCellValue, SCROW> aRet;
        aRet.second = -1;

        sc::CellStoreType::const_position_type aPos = getPosition(nIndex);
        if (aPos.first == mrCells.end())
            return aRet;

        aRet.first = sc::toRefCell(aPos.first, aPos.second);
        aRet.second = aPos.first->position + aPos.second;
        return aRet;
    }

    size_t getLowIndex() const { return mnLowIndex; }

    size_t getHighIndex() const { return mnHighIndex; }

    bool isValid() const { return mbValid; }
};

}

bool ScQueryCellIterator::BinarySearch()
{
    

    if (nTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");
    nCol = mpParam->nCol1;
    ScColumn* pCol = &(pDoc->maTabs[nTab])->aCol[nCol];
    if (pCol->IsEmptyData())
        return false;

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
        ++nRow;

    ScRefCellValue aCell;
    if (bFirstStringIgnore)
    {
        sc::CellStoreType::const_position_type aPos = pCol->maCells.position(nRow);
        if (aPos.first->type == sc::element_type_string || aPos.first->type == sc::element_type_edittext)
        {
            aCell = sc::toRefCell(aPos.first, aPos.second);
            sal_uLong nFormat = pCol->GetNumberFormat(nRow);
            OUString aCellStr;
            ScCellFormat::GetInputString(aCell, nFormat, aCellStr, rFormatter, pDoc);
            sal_Int32 nTmp = pCollator->compareString(aCellStr, rEntry.GetQueryItem().maString.getString());
            if ((rEntry.eOp == SC_LESS_EQUAL && nTmp > 0) ||
                    (rEntry.eOp == SC_GREATER_EQUAL && nTmp < 0) ||
                    (rEntry.eOp == SC_EQUAL && nTmp != 0))
                ++nRow;
        }
    }

    NonEmptyCellIndexer aIndexer(pCol->maCells, nRow, mpParam->nRow2, bAllStringIgnore);
    if (!aIndexer.isValid())
        return false;

    size_t nLo = aIndexer.getLowIndex();
    size_t nHi = aIndexer.getHighIndex();
    NonEmptyCellIndexer::CellType aCellData;

    
    
    size_t nLastInRange = nLo;
    size_t nFirstLastInRange = nLastInRange;
    double fLastInRangeValue = bLessEqual ?
        -(::std::numeric_limits<double>::max()) :
            ::std::numeric_limits<double>::max();
    OUString aLastInRangeString;
    if (!bLessEqual)
        aLastInRangeString = OUString(sal_Unicode(0xFFFF));

    aCellData = aIndexer.getCell(nLastInRange);
    aCell = aCellData.first;
    if (aCell.hasString())
    {
        sal_uLong nFormat = pCol->GetNumberFormat(aCellData.second);
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
                
            }
        }
    }

    sal_Int32 nRes = 0;
    bool bFound = false;
    bool bDone = false;
    while (nLo <= nHi && !bDone)
    {
        size_t nMid = (nLo+nHi)/2;
        size_t i = nMid;
        if (i > nHi)
        {
            if (nMid > 0)
                nHi = nMid - 1;
            else
                bDone = true;
            continue; 
        }

        aCellData = aIndexer.getCell(i);
        aCell = aCellData.first;
        bool bStr = aCell.hasString();
        nRes = 0;

        
        
        if (!bStr && !bByString)
        {
            double nCellVal;
            switch (aCell.meType)
            {
                case CELLTYPE_VALUE :
                case CELLTYPE_FORMULA :
                    nCellVal = aCell.getValue();
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
                        
                        nLastInRange = nFirstLastInRange;
                        bDone = true;
                    }
                }
            }
        }
        else if (bStr && bByString)
        {
            OUString aCellStr;
            sal_uLong nFormat = pCol->GetNumberFormat(aCellData.second);
            ScCellFormat::GetInputString(aCell, nFormat, aCellStr, rFormatter, pDoc);

            nRes = pCollator->compareString(aCellStr, rEntry.GetQueryItem().maString.getString());
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
                    
                    nLastInRange = nFirstLastInRange;
                    bDone = true;
                }
            }
        }
        else if (!bStr && bByString)
        {
            nRes = -1; 
            if (bLessEqual)
                nLastInRange = i;
        }
        else 
        {
            nRes = 1; 
            if (!bLessEqual)
                nLastInRange = i;
        }
        if (nRes < 0)
        {
            if (bLessEqual)
                nLo = nMid + 1;
            else 
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
            else 
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
        
        
        
        
        
        
        
        
        nLo = nLastInRange;
    }

    aCellData = aIndexer.getCell(nLo);
    if (nLo <= nHi && aCellData.second <= mpParam->nRow2)
    {
        nRow = aCellData.second;
        maCurPos = aIndexer.getPosition(nLo);
        return true;
    }
    else
    {
        nRow = mpParam->nRow2 + 1;
        
        maCurPos.first = pCol->maCells.end();
        --maCurPos.first;
        maCurPos.second = maCurPos.first->size - 1;
        return false;
    }
}




ScHorizontalCellIterator::ScHorizontalCellIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) :
    pDoc( pDocument ),
    mnTab( nTable ),
    nStartCol( nCol1 ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 ),
    mnCol( nCol1 ),
    mnRow( nRow1 ),
    mbMore( false )
{
    if (mnTab >= pDoc->GetTableCount())
        OSL_FAIL("try to access index out of bounds, FIX IT");

    pNextRows = new SCROW[ nCol2-nCol1+1 ];
    pNextIndices = new SCSIZE[ nCol2-nCol1+1 ];
    maColPositions.reserve( nCol2-nCol1+1 );

    SetTab( mnTab );
}

ScHorizontalCellIterator::~ScHorizontalCellIterator()
{
    delete [] pNextRows;
    delete [] pNextIndices;
}

void ScHorizontalCellIterator::SetTab( SCTAB nTabP )
{
    mbMore = false;
    mnTab = nTabP;
    mnRow = nStartRow;
    mnCol = nStartCol;
    maColPositions.resize(0);

    
    for (SCCOL i = nStartCol; i <= nEndCol; ++i)
    {
        ScColumn* pCol = &pDoc->maTabs[mnTab]->aCol[i];
        ColParam aParam;
        aParam.maPos = pCol->maCells.position(nStartRow).first;
        aParam.maEnd = pCol->maCells.end();
        aParam.mnCol = i;

        
        while (aParam.maPos != aParam.maEnd) {
            if (aParam.maPos->type == sc::element_type_empty)
                ++aParam.maPos;
            else
            {
                maColPositions.push_back( aParam );
                break;
            }
        }
    }

    if (maColPositions.size() == 0)
        return;

    maColPos = maColPositions.begin();
    mbMore = true;
    SkipInvalid();
}

ScRefCellValue* ScHorizontalCellIterator::GetNext( SCCOL& rCol, SCROW& rRow )
{
    if (!mbMore)
    {
        debugiter("no more !\n");
        return NULL;
    }

    
    ColParam& r = *maColPos;

    rCol = mnCol = r.mnCol;
    rRow = mnRow;
    debugiter("return col %d row %d\n", (int)rCol, (int)rRow);

    size_t nOffset = static_cast<size_t>(mnRow) - r.maPos->position;
    maCurCell = sc::toRefCell(r.maPos, nOffset);
    Advance();
    debugiter("advance to: col %d row %d\n", (int)maColPos->mnCol, (int)mnRow);

    return &maCurCell;
}

bool ScHorizontalCellIterator::GetPos( SCCOL& rCol, SCROW& rRow )
{
    rCol = mnCol;
    rRow = mnRow;
    return mbMore;
}




bool ScHorizontalCellIterator::SkipInvalidInRow()
{
    assert (mbMore);
    assert (maColPos != maColPositions.end());

    
    while( maColPos != maColPositions.end() )
    {
        ColParam& r = *maColPos;
        assert (r.maPos != r.maEnd);

        size_t nRow = static_cast<size_t>(mnRow);

        if (nRow >= r.maPos->position)
        {
            if (nRow < r.maPos->position + r.maPos->size)
            {
                mnCol = maColPos->mnCol;
                debugiter("found valid cell at column %d, row %d\n",
                          (int)mnCol, (int)mnRow);
                assert(r.maPos->type != sc::element_type_empty);
                return true;
            }
            else
            {
                bool bMoreBlocksInColumn = false;
                
                for (++r.maPos; r.maPos != r.maEnd; ++r.maPos)
                {
                    if (nRow < r.maPos->position + r.maPos->size &&
                        r.maPos->type != sc::element_type_empty)
                    {
                        bMoreBlocksInColumn = true;
                        break;
                    }
                }
                if (!bMoreBlocksInColumn)
                {
                    debugiter("remove column %d at row %d\n",
                              (int)maColPos->mnCol, (int)nRow);
                    maColPos = maColPositions.erase(maColPos);
                    if (maColPositions.size() == 0)
                    {
                        debugiter("no more columns\n");
                        mbMore = false;
                    }
                }
                else
                {
                    debugiter("advanced column %d to block starting row %d, retying\n",
                              (int)maColPos->mnCol, r.maPos->position);
                }
            }
        }
        else
        {
            debugiter("skip empty cells at column %d, row %d\n",
                      (int)maColPos->mnCol, (int)nRow);
            maColPos++;
        }
    }

    
    if (maColPositions.size() == 0)
    {
        debugiter("no more live columns left - done\n");
        mbMore = false;
        return true;
    }

    return false;
}


SCROW ScHorizontalCellIterator::FindNextNonEmptyRow()
{
    size_t nNextRow = MAXROW+1;

    for (std::vector<ColParam>::iterator it = maColPositions.begin();
         it != maColPositions.end(); ++it)
    {
        ColParam& r = *it;

        assert(static_cast<size_t>(mnRow) <= r.maPos->position);
        nNextRow = std::min (nNextRow, static_cast<size_t>(r.maPos->position));
    }

    SCROW nRow = std::max(static_cast<SCROW>(nNextRow), mnRow);
    debugiter("Next non empty row is %d\n", (int) nRow);
    return nRow;
}

void ScHorizontalCellIterator::Advance()
{
    assert (mbMore);
    assert (maColPos != maColPositions.end());

    maColPos++;

    SkipInvalid();
}

void ScHorizontalCellIterator::SkipInvalid()
{
    if (maColPos == maColPositions.end() ||
        !SkipInvalidInRow())
    {
        mnRow++;

        if (mnRow > nEndRow)
        {
            mbMore = false;
            return;
        }

        maColPos = maColPositions.begin();
        debugiter("moving to next row\n");
        if (SkipInvalidInRow())
        {
            debugiter("moved to valid cell in next row (or end)\n");
            return;
        }

        mnRow = FindNextNonEmptyRow();
        maColPos = maColPositions.begin();
        bool bCorrect = SkipInvalidInRow();
        assert (bCorrect); (void) bCorrect;
    }

    if (mnRow > nEndRow)
        mbMore = false;
}



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

    nNumFormat = 0; 
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
                default: ;   
            }
        }
    }
    return bFound;
}



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
                nSkipTo = nThisEnd; 
        }
        else
            bEmpty = false; 

        pIndices[nPos] = nIndex;
        pNextEnd[nPos] = nThisEnd;
        ppPatterns[nPos] = pPattern;
    }

    if (bEmpty)
        nRow = nSkipTo; 

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
                ++nCol; 
                return pPat; 
            }
        }

        
        ++nRow;
        if ( nRow > nEndRow ) 
            return NULL; 

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
                        bEmpty = false; 

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
                bEmpty = false; 
        }

        if (bEmpty)
        {
            SCCOL nCount = nEndCol-nStartCol+1;
            SCROW nSkipTo = pNextEnd[0]; 
            for (i=1; i<nCount; i++)
                if ( pNextEnd[i] < nSkipTo )
                    nSkipTo = pNextEnd[i];
            nRow = nSkipTo; 
        }
        bRowEmpty = bEmpty;
        nCol = nStartCol; 
    }
}



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
    
    if ( pCell && IsGreater( nNextCol, nNextRow, nCellCol, nCellRow ) )
        pCell = aCellIter.GetNext( nCellCol, nCellRow );

    while (pCell && pCell->isEmpty())
        pCell = aCellIter.GetNext( nCellCol, nCellRow );

    if ( pPattern && IsGreater( nNextCol, nNextRow, nAttrCol2, nAttrRow ) )
        pPattern = aAttrIter.GetNext( nAttrCol1, nAttrCol2, nAttrRow );

    if ( pPattern && nAttrRow == nNextRow && nAttrCol1 < nNextCol )
        nAttrCol1 = nNextCol;

    
    bool bFound = true;
    bool bUseCell = false;

    if ( pCell && pPattern )
    {
        if ( IsGreater( nCellCol, nCellRow, nAttrCol1, nAttrRow ) ) 
        {
            maFoundCell.clear();
            pFoundPattern = pPattern;
            nFoundRow = nAttrRow;
            nFoundStartCol = nAttrCol1;
            if ( nCellRow == nAttrRow && nCellCol <= nAttrCol2 ) 
                nFoundEndCol = nCellCol - 1; 
            else
                nFoundEndCol = nAttrCol2; 
        }
        else
        {
            bUseCell = true;
            if ( nAttrRow == nCellRow && nAttrCol1 == nCellCol ) 
                pFoundPattern = pPattern;
            else
                pFoundPattern = NULL;
        }
    }
    else if ( pCell ) 
    {
        pFoundPattern = NULL;
        bUseCell = true; 
    }
    else if ( pPattern ) 
    {
        maFoundCell.clear();
        pFoundPattern = pPattern;
        nFoundRow = nAttrRow;
        nFoundStartCol = nAttrCol1;
        nFoundEndCol = nAttrCol2;
    }
    else 
        bFound = false;

    if ( bUseCell ) 
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
    return NULL;  
}



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
    sc::RowHeightContext aCxt(mfPPTX, mfPPTY, aZoom, aZoom, mpOutDev);
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
                aCxt, aData.mnRow1, aData.mnRow2, &aProgress, nProgressStart);

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
    sc::RowHeightContext aCxt(mfPPTX, mfPPTY, aZoom, aZoom, mpOutDev);
    sal_uLong nProgressStart = 0;
    for (SCTAB nTab = 0; nTab < mrDoc.GetTableCount(); ++nTab)
    {
        if (!ValidTab(nTab) || !mrDoc.maTabs[nTab])
            continue;

        mrDoc.maTabs[nTab]->SetOptimalHeight(aCxt, 0, MAXROW, &aProgress, nProgressStart);
        nProgressStart += mrDoc.maTabs[nTab]->GetWeightedCount();
    }
}



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
    return NULL; 
}



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
