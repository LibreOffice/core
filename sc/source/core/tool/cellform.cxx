/*************************************************************************
 *
 *  $RCSfile: cellform.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <sfx2/objsh.hxx>
#include <svtools/smplhint.hxx>
#include <svtools/zforlist.hxx>

#include "cellform.hxx"
#include "cell.hxx"
#include "compiler.hxx"
#include "document.hxx"
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
                    // #62160# Ein via Interpreter gestartetes Makro, das hart
                    // auf Formelzellen zugreift, bekommt einen CellText, auch
                    // wenn dadurch ein weiterer Interpreter gestartet wird,
                    // aber nicht wenn diese Zelle gerade interpretiert wird.
                    // IdleCalc startet generell keine weiteren Interpreter,
                    // um keine Err522 (zirkulaer) zu bekommen.
                    if ( pFCell->GetDocument()->IsInInterpreter() &&
                            (!pFCell->GetDocument()->GetMacroInterpretLevel()
                            || pFCell->IsRunning()) )
                    {
                        rString.AssignAscii( RTL_CONSTASCII_STRINGPARAM("...") );
                    }
                    else
                    {
                        USHORT nErrCode = pFCell->GetErrCode();
                        if ( nErrCode == errInterpOverflow )
                        {   // maxrecursion ausbuegeln, Err527 Workaround
                            ScDocument* pDoc = pFCell->GetDocument();
                            // z.B. nach Import von Fremdformaten FormulaTree
                            // aufbauen
                            if ( !pDoc->IsInFormulaTree( pFCell ) )
                                pFCell->SetDirty();
                            if ( pDoc->GetAutoCalc()
                              && !pDoc->GetHardRecalcState()
                              && pLastFormulaTreeTop != pDoc->GetFormulaTree() )
                            {
                                pLastFormulaTreeTop = pDoc->GetFormulaTree();
                                // ohne ProgressBar falls fehlendes Repaint:
                                // pDoc->CalcFormulaTree( FALSE, TRUE );
                                //! austesten
                                pDoc->CalcFormulaTree( FALSE, FALSE );
                                nErrCode = pFCell->GetErrCode();
                                if ( nErrCode != errInterpOverflow )
                                {   // von hinten durch die Brust ins Auge ...
                                    pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
                                    pDoc->ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );
                                }
                            }
                        }

                        // erst nach dem Interpretieren (GetErrCode) das Zahlformat holen:
                        if ( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                            nFormat = pFCell->GetStandardFormat( rFormatter,
                                nFormat );

                        if (nErrCode != 0)
                            rString = ScGlobal::GetErrorString(nErrCode);
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
                if (((ScFormulaCell*)pCell)->IsValue())
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



