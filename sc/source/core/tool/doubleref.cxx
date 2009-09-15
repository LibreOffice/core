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

using ::rtl::OUString;
using ::std::auto_ptr;

#include <stdio.h>
#include <string>
#include <sys/time.h>

namespace {

class StackPrinter
{
public:
    explicit StackPrinter(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~StackPrinter()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

}

namespace {


bool CreateStarQuery(ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
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

    BOOL bValid;
    BOOL bFound;
    String aCellStr;
    SCSIZE nIndex = 0;
    SCROW nRow = 0;
    SCROW nRows = pDBRef->getRowSize();
    SCSIZE nNewEntries = static_cast<SCSIZE>(nRows);
    pParam->Resize(nNewEntries);

    do
    {
        ScQueryEntry& rEntry = pParam->GetEntry(nIndex);

        bValid = FALSE;

        if (nIndex > 0)
        {
            // For all entries after the first one, check the and/or connector in the first column.
            aCellStr = pQueryRef->getString(0, nRow);
            aCellStr.EraseTrailingChars();
            aCellStr.EraseLeadingChars();
            ScGlobal::pCharClass->toUpper(aCellStr);
            if ( aCellStr == ScGlobal::GetRscString(STR_TABLE_UND) )
            {
                rEntry.eConnect = SC_AND;
                bValid = TRUE;
            }
            else if ( aCellStr == ScGlobal::GetRscString(STR_TABLE_ODER) )
            {
                rEntry.eConnect = SC_OR;
                bValid = TRUE;
            }
        }

        if ((nIndex < 1) || bValid)
        {
            // field name in the 2nd column.
            bFound = FALSE;
            aCellStr = pQueryRef->getString(1, nRow);
            // TODO: remove leading/trailing blanks.
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
            bFound = FALSE;
            aCellStr = pQueryRef->getString(2, nRow);
            aCellStr.EraseTrailingChars();
            aCellStr.EraseLeadingChars();
            ScGlobal::pCharClass->toUpper(aCellStr);
            if (aCellStr.GetChar(0) == '<')
            {
                if (aCellStr.GetChar(1) == '>')
                    rEntry.eOp = SC_NOT_EQUAL;
                else if (aCellStr.GetChar(1) == '=')
                    rEntry.eOp = SC_LESS_EQUAL;
                else
                    rEntry.eOp = SC_LESS;
            }
            else if (aCellStr.GetChar(0) == '>')
            {
                if (aCellStr.GetChar(1) == '=')
                    rEntry.eOp = SC_GREATER_EQUAL;
                else
                    rEntry.eOp = SC_GREATER;
            }
            else if (aCellStr.GetChar(0) == '=')
                rEntry.eOp = SC_EQUAL;

        }

        if (bValid)
        {
            // Finally, the right-hand-side value in the 4th column.
            *rEntry.pStr = pQueryRef->getString(3, nRow);
            rEntry.bDoQuery = TRUE;
        }
        nIndex++;
        nRow++;
    }
    while (bValid && (nRow < nRows) /* && (nIndex < MAXQUERY) */ );
    return bValid;
}

bool CreateExcelQuery(
    ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
{
    bool bValid = true;
    SCCOL nCols = pQueryRef->getColSize();
    SCROW nRows = pQueryRef->getRowSize();
    SCCOL* pFields = new SCCOL[nCols];
    SCCOL nCol = 0;
    while (bValid && (nCol < nCols))
    {
        String aQueryStr = pQueryRef->getString(nCol, 0);
//      GetInputString(nCol, nRow1, aQueryStr);
        SCCOL nField = pDBRef->findFieldColumn(aQueryStr);
        if (ValidCol(nField))
            pFields[nCol] = nField;
        else
            bValid = false;
        ++nCol;
    }

    if (bValid)
    {
//      ULONG nVisible = 0;
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
                        pParam->GetEntry(nIndex).nField = pFields[nCol];
                        pParam->FillInExcelSyntax(aCellStr, nIndex);
                        nIndex++;
                        if (nIndex < nNewEntries)
                            pParam->GetEntry(nIndex).eConnect = SC_AND;
                    }
                    else
                        bValid = FALSE;
                }
                nCol++;
            }
            nRow++;
            if (nIndex < nNewEntries)
                pParam->GetEntry(nIndex).eConnect = SC_OR;
        }
    }
    delete [] pFields;
    return bValid;
}

bool FillQueryEntries(
    ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef, const ScDBRangeBase* pQueryRef)
{
    SCSIZE nCount = pParam->GetEntryCount();
    for (SCSIZE i = 0; i < nCount; ++i)
        pParam->GetEntry(i).Clear();

    // Standard QueryTabelle
    bool bValid = CreateStarQuery(pParam, pDBRef, pQueryRef);
    // Excel QueryTabelle
    if (!bValid)
        bValid = CreateExcelQuery(pParam, pDBRef, pQueryRef);

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

ScDBRangeBase::RefType ScDBRangeBase::getType() const
{
    return meType;
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
    getDoc()->GetString(nCol, nRow, maRange.aStart.Tab(), aStr);
    return aStr;
}

SCCOL ScDBInternalRange::getFirstFieldColumn() const
{
    return getRange().aStart.Col();
}

SCCOL ScDBInternalRange::findFieldColumn(SCCOL nColIndex) const
{
    const ScRange& rRange = getRange();
    const ScAddress& s = rRange.aStart;
    const ScAddress& e = rRange.aEnd;

    SCCOL nDBCol1 = s.Col();
    SCCOL nDBCol2 = e.Col();

    if ( nColIndex <= 0 || nColIndex > (nDBCol2 - nDBCol1 + 1) )
        return nDBCol1;

    return Min(nDBCol2, static_cast<SCCOL>(nDBCol1 + nColIndex - 1));
}

sal_uInt16 ScDBInternalRange::getCellString(String& rStr, ScBaseCell* pCell) const
{
    sal_uInt16 nErr = 0;
    if (pCell)
    {
        SvNumberFormatter* pFormatter = getDoc()->GetFormatTable();
        switch (pCell->GetCellType())
        {
            case CELLTYPE_STRING:
                ((ScStringCell*) pCell)->GetString(rStr);
            break;
            case CELLTYPE_EDIT:
                ((ScEditCell*) pCell)->GetString(rStr);
            break;
            case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
                nErr = pFCell->GetErrCode();
                if (pFCell->IsValue())
                {
                    double fVal = pFCell->GetValue();
                    ULONG nIndex = pFormatter->GetStandardFormat(
                                        NUMBERFORMAT_NUMBER,
                                        ScGlobal::eLnge);
                    pFormatter->GetInputLineString(fVal, nIndex, rStr);
                }
                else
                    pFCell->GetString(rStr);
            }
            break;
            case CELLTYPE_VALUE:
            {
                double fVal = ((ScValueCell*) pCell)->GetValue();
                ULONG nIndex = pFormatter->GetStandardFormat(
                                        NUMBERFORMAT_NUMBER,
                                        ScGlobal::eLnge);
                pFormatter->GetInputLineString(fVal, nIndex, rStr);
            }
            break;
            default:
                rStr = ScGlobal::GetEmptyString();
            break;
        }
    }
    else
        rStr = ScGlobal::GetEmptyString();

    return nErr;
}

SCCOL ScDBInternalRange::findFieldColumn(const OUString& rStr, sal_uInt16* pErr) const
{
    const ScAddress& s = maRange.aStart;
    const ScAddress& e = maRange.aEnd;

    SCCOL nDBCol1 = s.Col();
    SCROW nDBRow1 = s.Row();
    SCTAB nDBTab1 = s.Tab();
    SCCOL nDBCol2 = e.Col();

    SCCOL   nField = nDBCol1;
    BOOL    bFound = TRUE;

    bFound = FALSE;
    String aCellStr;
    ScAddress aLook( nDBCol1, nDBRow1, nDBTab1 );
    while (!bFound && (aLook.Col() <= nDBCol2))
    {
        ScBaseCell* pCell = getDoc()->GetCell( aLook );
        sal_uInt16 nErr = getCellString( aCellStr, pCell );
        if (pErr)
            *pErr = nErr;
        bFound = ScGlobal::pTransliteration->isEqual(aCellStr, rStr);
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
    pParam->bHasHeader = TRUE;
    pParam->bByRow = TRUE;
    pParam->bInplace = TRUE;
    pParam->bCaseSens = FALSE;
    pParam->bRegExp = FALSE;
    pParam->bDuplicate = TRUE;

    // Now construct the query entries from the query range.
    if (!pQueryRef->fillQueryEntries(pParam.get(), this))
        return NULL;

    return pParam.release();
}

bool ScDBInternalRange::isRangeEqual(const ScRange& rRange) const
{
    return maRange == rRange;
}

bool ScDBInternalRange::fillQueryEntries(ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef) const
{
    if (!pDBRef)
        return false;

    return FillQueryEntries(pParam, pDBRef, this);
}

// ============================================================================

ScDBExternalRange::ScDBExternalRange(ScDocument* pDoc, const ScMatrixRef& pMat) :
    ScDBRangeBase(pDoc, EXTERNAL), mpMatrix(pMat)
{
    SCSIZE nC, nR;
    mpMatrix->GetDimensions(nC, nR);
    mnCols = nC;
    mnRows = nR;
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
    return -1;
}

SCCOL ScDBExternalRange::findFieldColumn(SCCOL /*nColIndex*/) const
{
    return -1;
}

SCCOL ScDBExternalRange::findFieldColumn(const OUString& /*rStr*/, sal_uInt16* /*pErr*/) const
{
    return -1;
}

ScDBQueryParamBase* ScDBExternalRange::createQueryParam(const ScDBRangeBase* /*pQueryRef*/) const
{
    return NULL;
}

bool ScDBExternalRange::isRangeEqual(const ScRange& /*rRange*/) const
{
    return false;
}

bool ScDBExternalRange::fillQueryEntries(ScQueryParamBase* /*pParam*/, const ScDBRangeBase* /*pDBRef*/) const
{
    StackPrinter __stack_printer__("ScDBExternalRange::fillQueryEntries");

    return false;
}
