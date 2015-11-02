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

#include "cellform.hxx"

#include <sfx2/objsh.hxx>
#include <svl/smplhint.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstring.hxx>

#include "formulacell.hxx"
#include "document.hxx"
#include "cellvalue.hxx"
#include <formula/errorcodes.hxx>
#include "sc.hrc"
#include <editutil.hxx>

// STATIC DATA
// Err527 Workaround
const ScFormulaCell* pLastFormulaTreeTop = 0;

void ScCellFormat::GetString( ScRefCellValue& rCell, sal_uLong nFormat, OUString& rString,
                              Color** ppColor, SvNumberFormatter& rFormatter, const ScDocument* pDoc,
                              bool bNullVals, bool bFormula, ScForceTextFmt eForceTextFmt,
                              bool bUseStarFormat )
{
    *ppColor = NULL;

    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
            rFormatter.GetOutputString(rCell.mpString->getString(), nFormat, rString, ppColor, bUseStarFormat);
        break;
        case CELLTYPE_EDIT:
            rFormatter.GetOutputString(rCell.getString(pDoc), nFormat, rString, ppColor );
        break;
        case CELLTYPE_VALUE:
        {
            double nValue = rCell.mfValue;
            if (!bNullVals && nValue == 0.0)
                rString.clear();
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
                // IdleCalc generally doesn't trigger further interpretation,
                // as not to get Err522 (circular).
                if ( pFCell->GetDocument()->IsInInterpreter() &&
                        (!pFCell->GetDocument()->GetMacroInterpretLevel()
                        || pFCell->IsRunning()) )
                {
                    rString = "...";
                }
                else
                {
                    sal_uInt16 nErrCode = pFCell->GetErrCode();

                    if (nErrCode != 0)
                        rString = ScGlobal::GetErrorString(nErrCode);
                    else if ( pFCell->IsEmptyDisplayedAsString() )
                        rString.clear();
                    else if ( pFCell->IsValue() )
                    {
                        double fValue = pFCell->GetValue();
                        if ( !bNullVals && fValue == 0.0 )
                            rString.clear();
                        else if ( pFCell->IsHybridValueCell() )
                            rString = pFCell->GetString().getString();
                        else
                            rFormatter.GetOutputString( fValue, nFormat, rString, ppColor, bUseStarFormat );
                    }
                    else
                    {
                        rFormatter.GetOutputString( pFCell->GetString().getString(),
                                                    nFormat, rString, ppColor, bUseStarFormat );
                    }
                }
            }
        }
        break;
        default:
            rString.clear();
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
            ScRefCellValue aCell(rDoc, rPos);
            rFormatter.GetOutputString(aCell.mpString->getString(), nFormat, aString, ppColor, bUseStarFormat);
        }
        break;
        case CELLTYPE_EDIT:
        {
            ScRefCellValue aCell(rDoc, rPos);
            rFormatter.GetOutputString(aCell.getString(&rDoc), nFormat, aString, ppColor);
        }
        break;
        case CELLTYPE_VALUE:
        {
            double nValue = rDoc.GetValue(rPos);
            if (!bNullVals && nValue == 0.0) aString.clear();
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
            if (!pFCell)
                return aString;
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
                // IdleCalc generally doesn't trigger further interpretation,
                // as not to get Err522 (circular).
                if (pFCell->GetDocument()->IsInInterpreter() &&
                    (!pFCell->GetDocument()->GetMacroInterpretLevel()
                     || pFCell->IsRunning()))
                {
                    aString = "...";
                }
                else
                {
                    sal_uInt16 nErrCode = pFCell->GetErrCode();

                    if (nErrCode != 0) aString = ScGlobal::GetErrorString(nErrCode);
                    else if (pFCell->IsEmptyDisplayedAsString()) aString.clear();
                    else if (pFCell->IsValue())
                    {
                        double fValue = pFCell->GetValue();
                        if (!bNullVals && fValue == 0.0) aString.clear();
                        else if (pFCell->IsHybridValueCell()) aString = pFCell->GetString().getString();
                        else rFormatter.GetOutputString(fValue, nFormat, aString, ppColor, bUseStarFormat);
                    }
                    else
                    {
                        rFormatter.GetOutputString(pFCell->GetString().getString(),
                                                   nFormat, aString, ppColor, bUseStarFormat);
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
                aString = pFC->GetString().getString();

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

OUString ScCellFormat::GetOutputString( ScDocument& rDoc, const ScAddress& rPos, ScRefCellValue& rCell )
{
    if (rCell.isEmpty())
        return EMPTY_OUSTRING;

    OUString aVal;

    if (rCell.meType == CELLTYPE_EDIT)
    {
        //  GetString converts line breaks into spaces in EditCell,
        //  but here we need the line breaks
        const EditTextObject* pData = rCell.mpEditText;
        if (pData)
        {
            ScFieldEditEngine& rEngine = rDoc.GetEditEngine();
            rEngine.SetText(*pData);
            aVal = rEngine.GetText();
        }
        //  also do not format EditCells as numbers
        //  (fitting to output)
    }
    else
    {
        //  like in GetString for document (column)
        Color* pColor;
        sal_uLong nNumFmt = rDoc.GetNumberFormat(rPos);
        aVal = GetString(rDoc, rPos, nNumFmt, &pColor, *rDoc.GetFormatTable());
    }
    return aVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
