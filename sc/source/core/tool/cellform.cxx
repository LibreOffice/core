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

#include <sfx2/objsh.hxx>
#include <svl/smplhint.hxx>
#include <svl/zforlist.hxx>

#include "cellform.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "cellvalue.hxx"
#include "formula/errorcodes.hxx"
#include "sc.hrc"

// STATIC DATA -----------------------------------------------------------

// Err527 Workaround
const ScFormulaCell* pLastFormulaTreeTop = 0;

void ScCellFormat::GetString( ScRefCellValue& rCell, sal_uLong nFormat, OUString& rString,
                              Color** ppColor, SvNumberFormatter& rFormatter, const ScDocument* pDoc,
                              bool bNullVals, bool bFormula, ScForceTextFmt eForceTextFmt,
                              bool bUseStarFormat )
{
    *ppColor = NULL;
    if (&rFormatter==NULL)
    {
        rString = EMPTY_OUSTRING;
        return;
    }

    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
            rFormatter.GetOutputString(*rCell.mpString, nFormat, rString, ppColor, bUseStarFormat);
        break;
        case CELLTYPE_EDIT:
            rFormatter.GetOutputString(rCell.getString(pDoc), nFormat, rString, ppColor );
        break;
        case CELLTYPE_VALUE:
        {
            double nValue = rCell.mfValue;
            if (!bNullVals && nValue == 0.0)
                rString = OUString();
            else
            {
                if( eForceTextFmt == ftCheck )
                {
                    if( nFormat && rFormatter.IsTextFormat( nFormat ) )
                        eForceTextFmt = ftForce;
                }
                if( eForceTextFmt == ftForce )
                {
                    OUString aTemp;
                    rFormatter.GetOutputString( nValue, 0, aTemp, ppColor );
                    rFormatter.GetOutputString( aTemp, nFormat, rString, ppColor );
                }
                else
                    rFormatter.GetOutputString( nValue, nFormat, rString, ppColor, bUseStarFormat );
            }
        }
        break;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell*  pFCell = rCell.mpFormula;
            if ( bFormula )
            {
                pFCell->GetFormula( rString );
            }
            else
            {
                // A macro started from the interpreter, which has
                // access to Formular Cells, becomes a CellText, even if
                // that triggers further interpretation, except if those
                // cells are already being interpreted.
                // IdleCalc generally doesn't trigger futher interpretation,
                // as not to get Err522 (circular).
                if ( pFCell->GetDocument()->IsInInterpreter() &&
                        (!pFCell->GetDocument()->GetMacroInterpretLevel()
                        || pFCell->IsRunning()) )
                {
                    rString = OUString("...");
                }
                else
                {
                    sal_uInt16 nErrCode = pFCell->GetErrCode();

                    if (nErrCode != 0)
                        rString = ScGlobal::GetErrorString(nErrCode);
                    else if ( pFCell->IsEmptyDisplayedAsString() )
                        rString = OUString();
                    else if ( pFCell->IsValue() )
                    {
                        double fValue = pFCell->GetValue();
                        if ( !bNullVals && fValue == 0.0 )
                            rString = OUString();
                        else if ( pFCell->IsHybridValueCell() )
                            rString = pFCell->GetString();
                        else
                            rFormatter.GetOutputString( fValue, nFormat, rString, ppColor, bUseStarFormat );
                    }
                    else
                    {
                        OUString aCellString = pFCell->GetString();
                        rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor, bUseStarFormat );
                    }
                }
            }
        }
        break;
        default:
            rString = OUString();
            break;
    }
}

OUString ScCellFormat::GetString(
    ScDocument& rDoc, const ScAddress& rPos, sal_uLong nFormat, Color** ppColor,
    SvNumberFormatter& rFormatter, bool bNullVals, bool bFormula, ScForceTextFmt eForceTextFmt,
    bool bUseStarFormat )
{
    OUString aString;
    *ppColor = NULL;

    CellType eType = rDoc.GetCellType(rPos);
    switch (eType)
    {
        case CELLTYPE_STRING:
        {
            OUString aCellString = rDoc.GetString(rPos);
            rFormatter.GetOutputString(aCellString, nFormat, aString, ppColor, bUseStarFormat);
        }
        break;
        case CELLTYPE_EDIT:
        {
            OUString aCellString = rDoc.GetString(rPos);
            rFormatter.GetOutputString(aCellString, nFormat, aString, ppColor);
        }
        break;
        case CELLTYPE_VALUE:
        {
            double nValue = rDoc.GetValue(rPos);
            if (!bNullVals && nValue == 0.0) aString = OUString();
            else
            {
                if (eForceTextFmt == ftCheck)
                {
                    if (nFormat && rFormatter.IsTextFormat(nFormat)) eForceTextFmt = ftForce;
                }
                if (eForceTextFmt == ftForce)
                {
                    OUString aTemp;
                    rFormatter.GetOutputString(nValue, 0, aTemp, ppColor);
                    rFormatter.GetOutputString(aTemp, nFormat, aString, ppColor);
                }
                else rFormatter.GetOutputString(nValue, nFormat, aString, ppColor, bUseStarFormat);
            }
        }
        break;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFCell = rDoc.GetFormulaCell(rPos);
            if (bFormula)
            {
                pFCell->GetFormula(aString);
            }
            else
            {
                // A macro started from the interpreter, which has
                // access to Formular Cells, becomes a CellText, even if
                // that triggers further interpretation, except if those
                // cells are already being interpreted.
                // IdleCalc generally doesn't trigger futher interpretation,
                // as not to get Err522 (circular).
                if (pFCell->GetDocument()->IsInInterpreter() &&
                    (!pFCell->GetDocument()->GetMacroInterpretLevel()
                     || pFCell->IsRunning()))
                {
                    aString = OUString("...");
                }
                else
                {
                    sal_uInt16 nErrCode = pFCell->GetErrCode();

                    if (nErrCode != 0) aString = ScGlobal::GetErrorString(nErrCode);
                    else if (pFCell->IsEmptyDisplayedAsString()) aString = OUString();
                    else if (pFCell->IsValue())
                    {
                        double fValue = pFCell->GetValue();
                        if (!bNullVals && fValue == 0.0) aString = OUString();
                        else if (pFCell->IsHybridValueCell()) aString = pFCell->GetString();
                        else rFormatter.GetOutputString(fValue, nFormat, aString, ppColor, bUseStarFormat);
                    }
                    else
                    {
                        OUString aCellString = pFCell->GetString();
                        rFormatter.GetOutputString(aCellString, nFormat, aString, ppColor, bUseStarFormat);
                    }
                }
            }
        }
        break;
        default:
            ;
    }
    return aString;
}

void ScCellFormat::GetInputString(
    ScRefCellValue& rCell, sal_uLong nFormat, OUString& rString, SvNumberFormatter& rFormatter, const ScDocument* pDoc )
{
    if (&rFormatter == NULL)
    {
        rString = EMPTY_OUSTRING;
        return;
    }

    OUString aString = rString;
    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            aString = rCell.getString(pDoc);
        break;
        case CELLTYPE_VALUE:
            rFormatter.GetInputLineString(rCell.mfValue, nFormat, aString );
        break;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFC = rCell.mpFormula;
            if (pFC->IsEmptyDisplayedAsString())
                aString = EMPTY_OUSTRING;
            else if (pFC->IsValue())
                rFormatter.GetInputLineString(pFC->GetValue(), nFormat, aString);
            else
                aString = pFC->GetString();

            sal_uInt16 nErrCode = pFC->GetErrCode();
            if (nErrCode != 0)
                aString = EMPTY_OUSTRING;
        }
        break;
        default:
            aString = EMPTY_OUSTRING;
            break;
    }
    rString = aString;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
