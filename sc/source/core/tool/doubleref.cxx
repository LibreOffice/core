/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: interpre.hxx,v $
 * $Revision: 1.35.44.2 $
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

#include "doubleref.hxx"
#include "cell.hxx"
#include "global.hxx"
#include "document.hxx"
#include "queryparam.hxx"
#include "globstr.hrc"

#include <memory>
#include <vector>

using ::rtl::OUString;
using ::std::auto_ptr;
using ::std::vector;

namespace {

void lcl_toUpper(OUString& rStr)
{
    rStr = ScGlobal::pCharClass->toUpper(rStr.trim(), 0, static_cast<xub_StrLen>(rStr.getLength()));
}

bool lcl_createStarQuery(ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
{
    // A valid StarQuery must be at least 4 columns wide. To be precise it
    // should be exactly 4 columns ...
    // Additionally, if this wasn't checked, a formula pointing to a valid 1-3
    // column Excel style query range immediately left to itself would result
    // in a circular reference when the field name or operator or value (first
    // to third query range column) is obtained (#i58354#). Furthermore, if the
    // range wasn't sufficiently specified data changes wouldn't flag formula
    // cells for recalculation.

    if (pQueryRef->getColSize() < 4)
        return false;

    sal_Bool bValid;
    sal_Bool bFound;
    OUString aCellStr;
    SCSIZE nIndex = 0;
    SCROW nRow = 0;
    SCROW nRows = pDBRef->getRowSize();
    SCSIZE nNewEntries = static_cast<SCSIZE>(nRows);
    pParam->Resize(nNewEntries);

    do
    {
        ScQueryEntry& rEntry = pParam->GetEntry(nIndex);

        bValid = sal_False;

        if (nIndex > 0)
        {
            // For all entries after the first one, check the and/or connector in the first column.
            aCellStr = pQueryRef->getString(0, nRow);
            lcl_toUpper(aCellStr);
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
            // field name in the 2nd column.
            bFound = sal_False;
            aCellStr = pQueryRef->getString(1, nRow);
            SCCOL nField = pDBRef->findFieldColumn(aCellStr); // TODO: must be case insensitive comparison.
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
            // equality, non-equality operator in the 3rd column.
            bFound = sal_False;
            aCellStr = pQueryRef->getString(2, nRow);
            lcl_toUpper(aCellStr);
            const sal_Unicode* p = aCellStr.getStr();
            if (p[0] == sal_Unicode('<'))
            {
                if (p[1] == sal_Unicode('>'))
                    rEntry.eOp = SC_NOT_EQUAL;
                else if (p[1] == sal_Unicode('='))
                    rEntry.eOp = SC_LESS_EQUAL;
                else
                    rEntry.eOp = SC_LESS;
            }
            else if (p[0] == sal_Unicode('>'))
            {
                if (p[1] == sal_Unicode('='))
                    rEntry.eOp = SC_GREATER_EQUAL;
                else
                    rEntry.eOp = SC_GREATER;
            }
            else if (p[0] == sal_Unicode('='))
                rEntry.eOp = SC_EQUAL;

        }

        if (bValid)
        {
            // Finally, the right-hand-side value in the 4th column.
            *rEntry.pStr = pQueryRef->getString(3, nRow);
            rEntry.bDoQuery = sal_True;
        }
        nIndex++;
        nRow++;
    }
    while (bValid && (nRow < nRows) /* && (nIndex < MAXQUERY) */ );
    return bValid;
}

bool lcl_createExcelQuery(
    ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
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
//      sal_uLong nVisible = 0;
//      for ( nCol=nCol1; nCol<=nCol2; nCol++ )
//          nVisible += aCol[nCol].VisibleCount( nRow1+1, nRow2 );

        // Count the number of visible cells (excluding the header row).  Each
        // visible cell corresponds with a single query.
        SCSIZE nVisible = pQueryRef->getVisibleDataCellCount();
        if ( nVisible > SCSIZE_MAX / sizeof(void*) )
        {
            DBG_ERROR("zu viele Filterkritierien");
            nVisible = 0;
        }

        SCSIZE nNewEntries = nVisible;
        pParam->Resize( nNewEntries );

        SCSIZE nIndex = 0;
        SCROW nRow = 1;
        String aCellStr;
        while (nRow < nRows)
        {
            nCol = 0;
            while (nCol < nCols)
            {
                aCellStr = pQueryRef->getString(nCol, nRow);
                ScGlobal::pCharClass->toUpper( aCellStr );
                if (aCellStr.Len() > 0)
                {
                    if (nIndex < nNewEntries)
                    {
                        pParam->GetEntry(nIndex).nField = aFields[nCol];
                        pParam->FillInExcelSyntax(aCellStr, nIndex);
                        nIndex++;
                        if (nIndex < nNewEntries)
                            pParam->GetEntry(nIndex).eConnect = SC_AND;
                    }
                    else
                        bValid = sal_False;
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
    ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
{
    SCSIZE nCount = pParam->GetEntryCount();
    for (SCSIZE i = 0; i < nCount; ++i)
        pParam->GetEntry(i).Clear();

    // Standard QueryTabelle
    bool bValid = lcl_createStarQuery(pParam, pDBRef, pQueryRef);
    // Excel QueryTabelle
    if (!bValid)
        bValid = lcl_createExcelQuery(pParam, pDBRef, pQueryRef);

    nCount = pParam->GetEntryCount();
    if (bValid)
    {
        //  bQueryByString muss gesetzt sein
        for (SCSIZE i = 0; i < nCount; ++i)
            pParam->GetEntry(i).bQueryByString = true;
    }
    else
    {
        //  nix
        for (SCSIZE i = 0; i < nCount; ++i)
            pParam->GetEntry(i).Clear();
    }
    return bValid;
}

}

// ============================================================================

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

    return lcl_fillQueryEntries(pParam, pDBRef, this);
}

void ScDBRangeBase::fillQueryOptions(ScQueryParamBase* pParam)
{
    pParam->bHasHeader = true;
    pParam->bByRow = true;
    pParam->bInplace = true;
    pParam->bCaseSens = false;
    pParam->bRegExp = false;
    pParam->bDuplicate = true;
    pParam->bMixedComparison = false;
}

ScDocument* ScDBRangeBase::getDoc() const
{
    return mpDoc;
}

// ============================================================================

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
    String aStr;
    const ScAddress& s = maRange.aStart;
    // #i109200# this is used in formula calculation, use GetInputString, not GetString
    // (consistent with ScDBInternalRange::getCellString)
    // GetStringForFormula is not used here, to allow querying for date values.
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
    const ScAddress& e = rRange.aEnd;

    SCCOL nDBCol1 = s.Col();
    SCCOL nDBCol2 = e.Col();

    if ( nIndex <= 0 || nIndex > (nDBCol2 - nDBCol1 + 1) )
        return nDBCol1;

    return Min(nDBCol2, static_cast<SCCOL>(nDBCol1 + nIndex - 1));
}

SCCOL ScDBInternalRange::findFieldColumn(const OUString& rStr, sal_uInt16* pErr) const
{
    const ScAddress& s = maRange.aStart;
    const ScAddress& e = maRange.aEnd;
    OUString aUpper = rStr;
    lcl_toUpper(aUpper);

    SCCOL nDBCol1 = s.Col();
    SCROW nDBRow1 = s.Row();
    SCTAB nDBTab1 = s.Tab();
    SCCOL nDBCol2 = e.Col();

    SCCOL   nField = nDBCol1;
    sal_Bool    bFound = sal_True;

    bFound = sal_False;
    OUString aCellStr;
    ScAddress aLook( nDBCol1, nDBRow1, nDBTab1 );
    while (!bFound && (aLook.Col() <= nDBCol2))
    {
        sal_uInt16 nErr = getDoc()->GetStringForFormula( aLook, aCellStr );
        if (pErr)
            *pErr = nErr;
        lcl_toUpper(aCellStr);
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

    // Set the database range first.
    const ScAddress& s = maRange.aStart;
    const ScAddress& e = maRange.aEnd;
    pParam->nCol1 = s.Col();
    pParam->nRow1 = s.Row();
    pParam->nCol2 = e.Col();
    pParam->nRow2 = e.Row();
    pParam->nTab  = s.Tab();

    fillQueryOptions(pParam.get());

    // Now construct the query entries from the query range.
    if (!pQueryRef->fillQueryEntries(pParam.get(), this))
        return NULL;

    return pParam.release();
}

bool ScDBInternalRange::isRangeEqual(const ScRange& rRange) const
{
    return maRange == rRange;
}

// ============================================================================

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

    return mpMatrix->GetString(nCol, nRow);
}

SCCOL ScDBExternalRange::getFirstFieldColumn() const
{
    return 0;
}

SCCOL ScDBExternalRange::findFieldColumn(SCCOL nIndex) const
{
    if (nIndex < 1)
        // 1st field
        return 0;

    if (nIndex > mnCols)
        // last field
        return mnCols - 1;

    return nIndex - 1;
}

SCCOL ScDBExternalRange::findFieldColumn(const OUString& rStr, sal_uInt16* pErr) const
{
    if (pErr)
        pErr = 0;

    OUString aUpper = rStr;
    lcl_toUpper(aUpper);
    for (SCCOL i = 0; i < mnCols; ++i)
    {
        OUString aUpperVal = mpMatrix->GetString(i, 0);
        lcl_toUpper(aUpperVal);
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

    // Now construct the query entries from the query range.
    if (!pQueryRef->fillQueryEntries(pParam.get(), this))
        return NULL;

    return pParam.release();
}

bool ScDBExternalRange::isRangeEqual(const ScRange& /*rRange*/) const
{
    return false;
}

