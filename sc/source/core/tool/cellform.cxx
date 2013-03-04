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
#include "cell.hxx"
#include "document.hxx"
#include "formula/errorcodes.hxx"
#include "sc.hrc"

// STATIC DATA -----------------------------------------------------------

// Err527 Workaround
const ScFormulaCell* pLastFormulaTreeTop = 0;

// -----------------------------------------------------------------------

void ScCellFormat::GetString( ScBaseCell* pCell, sal_uLong nFormat, OUString& rString,
                              Color** ppColor, SvNumberFormatter& rFormatter,
                              sal_Bool bNullVals,
                              sal_Bool bFormula,
                              ScForceTextFmt eForceTextFmt,
                              bool bUseStarFormat )
{
    *ppColor = NULL;
    if (&rFormatter==NULL)
    {
        rString = OUString();
        return;
    }

    CellType eType = pCell->GetCellType();
    switch(eType)
    {
        case CELLTYPE_STRING:
            {
                OUString aCellString = ((ScStringCell*)pCell)->GetString();
                rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor, bUseStarFormat );
            }
            break;
        case CELLTYPE_EDIT:
            {
                OUString aCellString = ((ScEditCell*)pCell)->GetString();
                rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );
            }
            break;
        case CELLTYPE_VALUE:
            {
                double nValue = ((ScValueCell*)pCell)->GetValue();
                if ( !bNullVals && nValue == 0.0 )
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
                ScFormulaCell*  pFCell = (ScFormulaCell*)pCell;
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

                        // get the number format only after interpretation (GetErrCode):
                        if ( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                            nFormat = pFCell->GetStandardFormat( rFormatter,
                                nFormat );

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

void ScCellFormat::GetInputString( ScBaseCell* pCell, sal_uLong nFormat, OUString& rString,
                                      SvNumberFormatter& rFormatter )
{
    if (&rFormatter==NULL)
    {
        rString = OUString();
        return;
    }

    String aString = rString;
    CellType eType = pCell->GetCellType();
    switch(eType)
    {
        case CELLTYPE_STRING:
            {
                aString = ((ScStringCell*)pCell)->GetString();
            }
            break;
        case CELLTYPE_EDIT:
            {
                aString = ((ScEditCell*)pCell)->GetString();
            }
            break;
        case CELLTYPE_VALUE:
            {
                double nValue = ((ScValueCell*)pCell)->GetValue();
                rFormatter.GetInputLineString( nValue, nFormat, aString );
            }
            break;
        case CELLTYPE_FORMULA:
            {
                if (((ScFormulaCell*)pCell)->IsEmptyDisplayedAsString())
                {
                    aString.Erase();
                }
                else if (((ScFormulaCell*)pCell)->IsValue())
                {
                    double nValue = ((ScFormulaCell*)pCell)->GetValue();
                    rFormatter.GetInputLineString( nValue, nFormat, aString );
                }
                else
                {
                    aString = ((ScFormulaCell*)pCell)->GetString();
                }

                sal_uInt16 nErrCode = ((ScFormulaCell*)pCell)->GetErrCode();
                if (nErrCode != 0)
                {
                    aString.Erase();
                }
            }
            break;
        default:
            aString.Erase();
            break;
    }
    rString = aString;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
