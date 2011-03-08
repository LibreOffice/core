/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

void ScCellFormat::GetString( ScBaseCell* pCell, ULONG nFormat, String& rString,
                              Color** ppColor, SvNumberFormatter& rFormatter,
                              BOOL bNullVals,
                              BOOL bFormula,
                              ScForceTextFmt eForceTextFmt )
{
    *ppColor = NULL;
    if (&rFormatter==NULL)
    {
        rString.Erase();
        return;
    }

    CellType eType = pCell->GetCellType();
    switch(eType)
    {
        case CELLTYPE_STRING:
            {
                String aCellString;
                ((ScStringCell*)pCell)->GetString( aCellString );
                rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );
            }
            break;
        case CELLTYPE_EDIT:
            {
                String aCellString;
                ((ScEditCell*)pCell)->GetString( aCellString );
                rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );
            }
            break;
        case CELLTYPE_VALUE:
            {
                double nValue = ((ScValueCell*)pCell)->GetValue();
                if ( !bNullVals && nValue == 0.0 )
                    rString.Erase();
                else
                {
                    if( eForceTextFmt == ftCheck )
                    {
                        if( nFormat && rFormatter.IsTextFormat( nFormat ) )
                            eForceTextFmt = ftForce;
                    }
                    if( eForceTextFmt == ftForce )
                    {
                        String aTemp;
                        rFormatter.GetOutputString( nValue, 0, aTemp, ppColor );
                        rFormatter.GetOutputString( aTemp, nFormat, rString, ppColor );
                    }
                    else
                        rFormatter.GetOutputString( nValue, nFormat, rString, ppColor );
                }
            }
            break;
        case CELLTYPE_FORMULA:
            {
                ScFormulaCell*  pFCell = (ScFormulaCell*)pCell;
                if ( bFormula )
                    pFCell->GetFormula( rString );
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
                        rString.AssignAscii( RTL_CONSTASCII_STRINGPARAM("...") );
                    }
                    else
                    {
                        USHORT nErrCode = pFCell->GetErrCode();

                        // get the number format only after interpretation (GetErrCode):
                        if ( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                            nFormat = pFCell->GetStandardFormat( rFormatter,
                                nFormat );

                        if (nErrCode != 0)
                            rString = ScGlobal::GetErrorString(nErrCode);
                        else if ( pFCell->IsEmptyDisplayedAsString() )
                            rString.Erase();
                        else if ( pFCell->IsValue() )
                        {
                            double fValue = pFCell->GetValue();
                            if ( !bNullVals && fValue == 0.0 )
                                rString.Erase();
                            else
                                rFormatter.GetOutputString( fValue, nFormat, rString, ppColor );
                        }
                        else
                        {
                            String aCellString;
                            pFCell->GetString( aCellString );
                            rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );
                        }
                    }
                }
            }
            break;
        default:
            rString.Erase();
            break;
    }
}

void ScCellFormat::GetInputString( ScBaseCell* pCell, ULONG nFormat, String& rString,
                                      SvNumberFormatter& rFormatter )
{
    if (&rFormatter==NULL)
    {
        rString.Erase();
        return;
    }

    CellType eType = pCell->GetCellType();
    switch(eType)
    {
        case CELLTYPE_STRING:
            {
                ((ScStringCell*)pCell)->GetString( rString );
            }
            break;
        case CELLTYPE_EDIT:
            {
                ((ScEditCell*)pCell)->GetString( rString );
            }
            break;
        case CELLTYPE_VALUE:
            {
                double nValue = ((ScValueCell*)pCell)->GetValue();
                rFormatter.GetInputLineString( nValue, nFormat, rString );
            }
            break;
        case CELLTYPE_FORMULA:
            {
                if (((ScFormulaCell*)pCell)->IsEmptyDisplayedAsString())
                {
                    rString.Erase();
                }
                else if (((ScFormulaCell*)pCell)->IsValue())
                {
                    double nValue = ((ScFormulaCell*)pCell)->GetValue();
                    rFormatter.GetInputLineString( nValue, nFormat, rString );
                }
                else
                {
                    ((ScFormulaCell*)pCell)->GetString( rString );
                }

                USHORT nErrCode = ((ScFormulaCell*)pCell)->GetErrCode();
                if (nErrCode != 0)
                {
                    rString.Erase();
                }
            }
            break;
        default:
            rString.Erase();
            break;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
