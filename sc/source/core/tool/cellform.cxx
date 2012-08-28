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

void ScCellFormat::GetString( ScBaseCell* pCell, sal_uLong nFormat, rtl::OUString& rString,
                              Color** ppColor, SvNumberFormatter& rFormatter,
                              sal_Bool bNullVals,
                              sal_Bool bFormula,
                              ScForceTextFmt eForceTextFmt,
                              bool bUseStarFormat )
{
    *ppColor = NULL;
    if (&rFormatter==NULL)
    {
        rString = rtl::OUString();
        return;
    }

    CellType eType = pCell->GetCellType();
    switch(eType)
    {
        case CELLTYPE_STRING:
            {
                rtl::OUString aCellString = ((ScStringCell*)pCell)->GetString();
                rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor, bUseStarFormat );
            }
            break;
        case CELLTYPE_EDIT:
            {
                rtl::OUString aCellString = ((ScEditCell*)pCell)->GetString();
                rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );
            }
            break;
        case CELLTYPE_VALUE:
            {
                double nValue = ((ScValueCell*)pCell)->GetValue();
                if ( !bNullVals && nValue == 0.0 )
                    rString = rtl::OUString();
                else
                {
                    if( eForceTextFmt == ftCheck )
                    {
                        if( nFormat && rFormatter.IsTextFormat( nFormat ) )
                            eForceTextFmt = ftForce;
                    }
                    if( eForceTextFmt == ftForce )
                    {
                        rtl::OUString aTemp;
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
                        rString = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("..."));
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
                            rString = rtl::OUString();
                        else if ( pFCell->IsValue() )
                        {
                            double fValue = pFCell->GetValue();
                            if ( !bNullVals && fValue == 0.0 )
                                rString = rtl::OUString();
                            else
                                rFormatter.GetOutputString( fValue, nFormat, rString, ppColor, bUseStarFormat );
                        }
                        else
                        {
                            rtl::OUString aCellString = pFCell->GetString();
                            rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor, bUseStarFormat );
                        }
                    }
                }
            }
            break;
        default:
            rString = rtl::OUString();
            break;
    }
}

void ScCellFormat::GetInputString( ScBaseCell* pCell, sal_uLong nFormat, rtl::OUString& rString,
                                      SvNumberFormatter& rFormatter )
{
    if (&rFormatter==NULL)
    {
        rString = rtl::OUString();
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
