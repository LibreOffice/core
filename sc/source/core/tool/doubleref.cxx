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

using ::rtl::OUString;

// ============================================================================

ScDoubleRefBase::ScDoubleRefBase(ScDocument* pDoc, RefType eType) :
    mpDoc(pDoc), meType(eType)
{
}

ScDoubleRefBase::~ScDoubleRefBase()
{
}

ScDoubleRefBase::RefType ScDoubleRefBase::getType() const
{
    return meType;
}

ScDocument* ScDoubleRefBase::getDoc()
{
    return mpDoc;
}

// ============================================================================

ScInternalDoubleRef::ScInternalDoubleRef(ScDocument* pDoc, const ScRange& rRange) :
    ScDoubleRefBase(pDoc, INTERNAL), maRange(rRange)
{
}

ScInternalDoubleRef::~ScInternalDoubleRef()
{
}

const ScRange& ScInternalDoubleRef::getRange() const
{
    return maRange;
}

SCCOL ScInternalDoubleRef::getFirstFieldColumn()
{
    return getRange().aStart.Col();
}

SCCOL ScInternalDoubleRef::findFieldColumn(SCCOL nColIndex)
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

sal_uInt16 ScInternalDoubleRef::getCellString(String& rStr, ScBaseCell* pCell)
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

SCCOL ScInternalDoubleRef::findFieldColumn(const OUString& rStr, sal_uInt16& rErr)
{
    const ScRange& rRange = getRange();
    const ScAddress& s = rRange.aStart;
    const ScAddress& e = rRange.aEnd;

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
        rErr = getCellString( aCellStr, pCell );
        bFound = ScGlobal::pTransliteration->isEqual(aCellStr, rStr);
        if (!bFound)
            aLook.IncCol();
    }
    nField = aLook.Col();

    return bFound ? nField : -1;
}

void ScInternalDoubleRef::initQueryParam(ScQueryParam& rParam) const
{
    const ScAddress& s = maRange.aStart;
    const ScAddress& e = maRange.aEnd;
    rParam.nCol1 = s.Col();
    rParam.nRow1 = s.Row();
    rParam.nCol2 = e.Col();
    rParam.nRow2 = e.Row();
    rParam.nTab  = s.Tab();
    rParam.bHasHeader = TRUE;
    rParam.bByRow = TRUE;
    rParam.bInplace = TRUE;
    rParam.bCaseSens = FALSE;
    rParam.bRegExp = FALSE;
    rParam.bDuplicate = TRUE;
}

bool ScInternalDoubleRef::isRangeEqual(const ScRange& rRange) const
{
    return maRange == rRange;
}

// ============================================================================

ScExternalDoubleRef::ScExternalDoubleRef(ScDocument* pDoc) :
    ScDoubleRefBase(pDoc, EXTERNAL)
{
}

ScExternalDoubleRef::~ScExternalDoubleRef()
{
}

SCCOL ScExternalDoubleRef::getFirstFieldColumn()
{
    return -1;
}

SCCOL ScExternalDoubleRef::findFieldColumn(SCCOL nColIndex)
{
    return -1;
}

SCCOL ScExternalDoubleRef::findFieldColumn(const OUString& rStr, sal_uInt16& rErr)
{
    return -1;
}

void ScExternalDoubleRef::initQueryParam(ScQueryParam& rParam) const
{
}

bool ScExternalDoubleRef::isRangeEqual(const ScRange& /*rRange*/) const
{
    return false;
}
