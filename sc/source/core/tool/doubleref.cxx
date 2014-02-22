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

#include "doubleref.hxx"
#include "formulacell.hxx"
#include "global.hxx"
#include "document.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"
#include "globstr.hrc"
#include "scmatrix.hxx"

#include "svl/sharedstringpool.hxx"

#include <memory>
#include <vector>

using ::std::auto_ptr;
using ::std::vector;

namespace {

void lcl_uppercase(OUString& rStr)
{
    rStr = ScGlobal::pCharClass->uppercase(rStr.trim());
}

bool lcl_createStarQuery(
    svl::SharedStringPool& rPool, ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
{
    
    
    
    
    
    
    
    

    if (pQueryRef->getColSize() < 4)
        return false;

    sal_Bool bValid;
    OUString aCellStr;
    SCSIZE nIndex = 0;
    SCROW nRow = 0;
    SCROW nRows = pDBRef->getRowSize();
    SCSIZE nNewEntries = static_cast<SCSIZE>(nRows);
    pParam->Resize(nNewEntries);

    do
    {
        ScQueryEntry& rEntry = pParam->GetEntry(nIndex);

        bValid = false;

        if (nIndex > 0)
        {
            
            aCellStr = pQueryRef->getString(0, nRow);
            lcl_uppercase(aCellStr);
            if ( aCellStr.equals(ScGlobal::GetRscString(STR_TABLE_UND)) )
            {
                rEntry.eConnect = SC_AND;
                bValid = sal_True;
            }
            else if ( aCellStr.equals(ScGlobal::GetRscString(STR_TABLE_ODER)) )
            {
                rEntry.eConnect = SC_OR;
                bValid = sal_True;
            }
        }

        if ((nIndex < 1) || bValid)
        {
            
            aCellStr = pQueryRef->getString(1, nRow);
            SCCOL nField = pDBRef->findFieldColumn(aCellStr); 
            if (ValidCol(nField))
            {
                rEntry.nField = nField;
                bValid = true;
            }
            else
                bValid = false;
        }

        if (bValid)
        {
            
            aCellStr = pQueryRef->getString(2, nRow);
            lcl_uppercase(aCellStr);
            const sal_Unicode* p = aCellStr.getStr();
            if (p[0] == '<')
            {
                if (p[1] == '>')
                    rEntry.eOp = SC_NOT_EQUAL;
                else if (p[1] == '=')
                    rEntry.eOp = SC_LESS_EQUAL;
                else
                    rEntry.eOp = SC_LESS;
            }
            else if (p[0] == '>')
            {
                if (p[1] == '=')
                    rEntry.eOp = SC_GREATER_EQUAL;
                else
                    rEntry.eOp = SC_GREATER;
            }
            else if (p[0] == '=')
                rEntry.eOp = SC_EQUAL;

        }

        if (bValid)
        {
            
            rEntry.GetQueryItem().maString =
                rPool.intern(pQueryRef->getString(3, nRow));
            rEntry.bDoQuery = true;
        }
        nIndex++;
        nRow++;
    }
    while (bValid && (nRow < nRows) /* && (nIndex < MAXQUERY) */ );
    return bValid;
}

bool lcl_createExcelQuery(
    svl::SharedStringPool& rPool, ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
{
    bool bValid = true;
    SCCOL nCols = pQueryRef->getColSize();
    SCROW nRows = pQueryRef->getRowSize();
    vector<SCCOL> aFields(nCols);
    SCCOL nCol = 0;
    while (bValid && (nCol < nCols))
    {
        OUString aQueryStr = pQueryRef->getString(nCol, 0);
        SCCOL nField = pDBRef->findFieldColumn(aQueryStr);
        if (ValidCol(nField))
            aFields[nCol] = nField;
        else
            bValid = false;
        ++nCol;
    }

    if (bValid)
    {
        
        
        SCSIZE nVisible = pQueryRef->getVisibleDataCellCount();
        if ( nVisible > SCSIZE_MAX / sizeof(void*) )
        {
            OSL_FAIL("too many filter criteria");
            nVisible = 0;
        }

        SCSIZE nNewEntries = nVisible;
        pParam->Resize( nNewEntries );

        SCSIZE nIndex = 0;
        SCROW nRow = 1;
        OUString aCellStr;
        while (nRow < nRows)
        {
            nCol = 0;
            while (nCol < nCols)
            {
                aCellStr = pQueryRef->getString(nCol, nRow);
                aCellStr = ScGlobal::pCharClass->uppercase( aCellStr );
                if (!aCellStr.isEmpty())
                {
                    if (nIndex < nNewEntries)
                    {
                        pParam->GetEntry(nIndex).nField = aFields[nCol];
                        pParam->FillInExcelSyntax(rPool, aCellStr, nIndex);
                        nIndex++;
                        if (nIndex < nNewEntries)
                            pParam->GetEntry(nIndex).eConnect = SC_AND;
                    }
                    else
                        bValid = false;
                }
                nCol++;
            }
            nRow++;
            if (nIndex < nNewEntries)
                pParam->GetEntry(nIndex).eConnect = SC_OR;
        }
    }
    return bValid;
}

bool lcl_fillQueryEntries(
    svl::SharedStringPool& rPool, ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
{
    SCSIZE nCount = pParam->GetEntryCount();
    for (SCSIZE i = 0; i < nCount; ++i)
        pParam->GetEntry(i).Clear();

    
    bool bValid = lcl_createStarQuery(rPool, pParam, pDBRef, pQueryRef);
    
    if (!bValid)
        bValid = lcl_createExcelQuery(rPool, pParam, pDBRef, pQueryRef);

    nCount = pParam->GetEntryCount();
    if (bValid)
    {
        
        for (SCSIZE i = 0; i < nCount; ++i)
            pParam->GetEntry(i).GetQueryItem().meType = ScQueryEntry::ByString;
    }
    else
    {
        
        for (SCSIZE i = 0; i < nCount; ++i)
            pParam->GetEntry(i).Clear();
    }
    return bValid;
}

}

ScDBRangeBase::ScDBRangeBase(ScDocument* pDoc, RefType eType) :
    mpDoc(pDoc), meType(eType)
{
}

ScDBRangeBase::~ScDBRangeBase()
{
}

bool ScDBRangeBase::fillQueryEntries(ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef) const
{
    if (!pDBRef)
        return false;

    return lcl_fillQueryEntries(getDoc()->GetSharedStringPool(), pParam, pDBRef, this);
}

void ScDBRangeBase::fillQueryOptions(ScQueryParamBase* pParam)
{
    pParam->bHasHeader = true;
    pParam->bByRow = true;
    pParam->bInplace = true;
    pParam->bCaseSens = false;
    pParam->bRegExp = false;
    pParam->bDuplicate = true;
}

ScDocument* ScDBRangeBase::getDoc() const
{
    return mpDoc;
}

ScDBInternalRange::ScDBInternalRange(ScDocument* pDoc, const ScRange& rRange) :
    ScDBRangeBase(pDoc, INTERNAL), maRange(rRange)
{
}

ScDBInternalRange::~ScDBInternalRange()
{
}

const ScRange& ScDBInternalRange::getRange() const
{
    return maRange;
}

SCCOL ScDBInternalRange::getColSize() const
{
    return maRange.aEnd.Col() - maRange.aStart.Col() + 1;
}

SCROW ScDBInternalRange::getRowSize() const
{
    return maRange.aEnd.Row() - maRange.aStart.Row() + 1;
}

SCSIZE ScDBInternalRange::getVisibleDataCellCount() const
{
    SCCOL nCols = getColSize();
    SCROW nRows = getRowSize();
    if (nRows <= 1)
        return 0;

    return (nRows-1)*nCols;
}

OUString ScDBInternalRange::getString(SCCOL nCol, SCROW nRow) const
{
    OUString aStr;
    const ScAddress& s = maRange.aStart;
    
    
    
    getDoc()->GetInputString(s.Col() + nCol, s.Row() + nRow, maRange.aStart.Tab(), aStr);
    return aStr;
}

SCCOL ScDBInternalRange::getFirstFieldColumn() const
{
    return getRange().aStart.Col();
}

SCCOL ScDBInternalRange::findFieldColumn(SCCOL nIndex) const
{
    const ScRange& rRange = getRange();
    const ScAddress& s = rRange.aStart;

    SCCOL nDBCol1 = s.Col();

    
    return nIndex + nDBCol1 - 1;
}

SCCOL ScDBInternalRange::findFieldColumn(const OUString& rStr, sal_uInt16* pErr) const
{
    const ScAddress& s = maRange.aStart;
    const ScAddress& e = maRange.aEnd;
    OUString aUpper = rStr;
    lcl_uppercase(aUpper);

    SCCOL nDBCol1 = s.Col();
    SCROW nDBRow1 = s.Row();
    SCTAB nDBTab1 = s.Tab();
    SCCOL nDBCol2 = e.Col();

    SCCOL   nField = nDBCol1;
    sal_Bool bFound = sal_False;

    OUString aCellStr;
    ScAddress aLook( nDBCol1, nDBRow1, nDBTab1 );
    while (!bFound && (aLook.Col() <= nDBCol2))
    {
        sal_uInt16 nErr = getDoc()->GetStringForFormula( aLook, aCellStr );
        if (pErr)
            *pErr = nErr;
        lcl_uppercase(aCellStr);
        bFound = ScGlobal::GetpTransliteration()->isEqual(aCellStr, aUpper);
        if (!bFound)
            aLook.IncCol();
    }
    nField = aLook.Col();

    return bFound ? nField : -1;
}

ScDBQueryParamBase* ScDBInternalRange::createQueryParam(const ScDBRangeBase* pQueryRef) const
{
    auto_ptr<ScDBQueryParamInternal> pParam(new ScDBQueryParamInternal);

    
    const ScAddress& s = maRange.aStart;
    const ScAddress& e = maRange.aEnd;
    pParam->nCol1 = s.Col();
    pParam->nRow1 = s.Row();
    pParam->nCol2 = e.Col();
    pParam->nRow2 = e.Row();
    pParam->nTab  = s.Tab();

    fillQueryOptions(pParam.get());

    
    if (!pQueryRef->fillQueryEntries(pParam.get(), this))
        return NULL;

    return pParam.release();
}

bool ScDBInternalRange::isRangeEqual(const ScRange& rRange) const
{
    return maRange == rRange;
}

ScDBExternalRange::ScDBExternalRange(ScDocument* pDoc, const ScMatrixRef& pMat) :
    ScDBRangeBase(pDoc, EXTERNAL), mpMatrix(pMat)
{
    SCSIZE nC, nR;
    mpMatrix->GetDimensions(nC, nR);
    mnCols = static_cast<SCCOL>(nC);
    mnRows = static_cast<SCROW>(nR);
}

ScDBExternalRange::~ScDBExternalRange()
{
}

SCCOL ScDBExternalRange::getColSize() const
{
    return mnCols;
}

SCROW ScDBExternalRange::getRowSize() const
{
    return mnRows;
}

SCSIZE ScDBExternalRange::getVisibleDataCellCount() const
{
    SCCOL nCols = getColSize();
    SCROW nRows = getRowSize();
    if (nRows <= 1)
        return 0;

    return (nRows-1)*nCols;
}

OUString ScDBExternalRange::getString(SCCOL nCol, SCROW nRow) const
{
    if (nCol >= mnCols || nRow >= mnRows)
        return OUString();

    return mpMatrix->GetString(nCol, nRow).getString();
}

SCCOL ScDBExternalRange::getFirstFieldColumn() const
{
    return 0;
}

SCCOL ScDBExternalRange::findFieldColumn(SCCOL nIndex) const
{
    return nIndex - 1;
}

SCCOL ScDBExternalRange::findFieldColumn(const OUString& rStr, sal_uInt16* pErr) const
{
    if (pErr)
        pErr = 0;

    OUString aUpper = rStr;
    lcl_uppercase(aUpper);
    for (SCCOL i = 0; i < mnCols; ++i)
    {
        OUString aUpperVal = mpMatrix->GetString(i, 0).getString();
        lcl_uppercase(aUpperVal);
        if (aUpper.equals(aUpperVal))
            return i;
    }
    return -1;
}

ScDBQueryParamBase* ScDBExternalRange::createQueryParam(const ScDBRangeBase* pQueryRef) const
{
    auto_ptr<ScDBQueryParamMatrix> pParam(new ScDBQueryParamMatrix);
    pParam->mpMatrix = mpMatrix;
    fillQueryOptions(pParam.get());

    
    if (!pQueryRef->fillQueryEntries(pParam.get(), this))
        return NULL;

    return pParam.release();
}

bool ScDBExternalRange::isRangeEqual(const ScRange& /*rRange*/) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
