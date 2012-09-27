/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

void ScCellFormat::GetString( ScBaseCell* pCell, sal_uLong nFormat, String& rString,
                              Color** ppColor, SvNumberFormatter& rFormatter,
                              sal_Bool bNullVals,
                              sal_Bool bFormula,
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
                        sal_uInt16 nErrCode = pFCell->GetErrCode();

                        // erst nach dem Interpretieren (GetErrCode) das Zahlformat holen:
                        if ( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                            nFormat = pFCell->GetStandardFormat( rFormatter,
                                nFormat );

                        if (nErrCode != 0)
                            rString = ScGlobal::GetErrorString(nErrCode);
                        else if ( pFCell->IsEmptyDisplayedAsString() )
                            rString.Erase();
                        else if ( pFCell->IsValue() )
                        {
                            if(pFCell->GetFormatType() == NUMBERFORMAT_LOGICAL)
                            {
                                String aCellString;
                                double fValue = pFCell->GetValue();
                                if(fValue)
                                    aCellString = rFormatter.GetTrueString();
                                else
                                    aCellString = rFormatter.GetFalseString();
                                rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );

                            }
                            else
                            {
                                double fValue = pFCell->GetValue();
                                if ( !bNullVals && fValue == 0.0 )
                                    rString.Erase();
                                else
                                    rFormatter.GetOutputString( fValue, nFormat, rString, ppColor );
                            }
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

void ScCellFormat::GetInputString( ScBaseCell* pCell, sal_uLong nFormat, String& rString,
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

                sal_uInt16 nErrCode = ((ScFormulaCell*)pCell)->GetErrCode();
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



