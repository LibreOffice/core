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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include <bf_sfx2/objsh.hxx>
#include <bf_svtools/zforlist.hxx>

#include "cellform.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "bf_sc.hrc"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

// Err527 Workaround
const ScFormulaCell* pLastFormulaTreeTop = 0;

// -----------------------------------------------------------------------

/*N*/ void ScCellFormat::GetString( ScBaseCell* pCell, ULONG nFormat, String& rString,
/*N*/ 							  Color** ppColor, SvNumberFormatter& rFormatter,
/*N*/ 							  BOOL bNullVals,
/*N*/ 							  BOOL bFormula,
/*N*/ 							  ScForceTextFmt eForceTextFmt )
/*N*/ {
/*N*/ 	*ppColor = NULL;
/*N*/ 	if (&rFormatter==NULL)
/*N*/ 	{
/*N*/ 		rString.Erase();
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	CellType eType = pCell->GetCellType();
/*N*/ 	switch(eType)
/*N*/ 	{
/*N*/ 		case CELLTYPE_STRING:
/*N*/ 			{
/*N*/ 				String aCellString;
/*N*/ 				((ScStringCell*)pCell)->GetString( aCellString );
/*N*/ 				rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case CELLTYPE_EDIT:
/*N*/ 			{
/*N*/ 				String aCellString;
/*N*/ 				((ScEditCell*)pCell)->GetString( aCellString );
/*N*/ 				rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case CELLTYPE_VALUE:
/*N*/ 			{
/*N*/ 				double nValue = ((ScValueCell*)pCell)->GetValue();
/*N*/ 				if ( !bNullVals && nValue == 0.0 )
/*N*/ 					rString.Erase();
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( eForceTextFmt == ftCheck )
/*N*/ 					{
/*N*/ 						if( nFormat && rFormatter.IsTextFormat( nFormat ) )
/*N*/ 							eForceTextFmt = ftForce;
/*N*/ 					}
/*N*/ 					if( eForceTextFmt == ftForce )
/*N*/ 					{
/*?*/ 						String aTemp;
/*?*/ 						rFormatter.GetOutputString( nValue, 0, aTemp, ppColor );
/*?*/ 						rFormatter.GetOutputString( aTemp, nFormat, rString, ppColor );
/*N*/ 					}
/*N*/ 					else
/*N*/ 						rFormatter.GetOutputString( nValue, nFormat, rString, ppColor );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case CELLTYPE_FORMULA:
/*N*/ 			{
/*N*/ 				ScFormulaCell*	pFCell = (ScFormulaCell*)pCell;
/*N*/ 				if ( bFormula )
/*?*/ 					pFCell->GetFormula( rString );
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// #62160# Ein via Interpreter gestartetes Makro, das hart
/*N*/ 					// auf Formelzellen zugreift, bekommt einen CellText, auch
/*N*/ 					// wenn dadurch ein weiterer Interpreter gestartet wird,
/*N*/ 					// aber nicht wenn diese Zelle gerade interpretiert wird.
/*N*/ 					// IdleCalc startet generell keine weiteren Interpreter,
/*N*/ 					// um keine Err522 (zirkulaer) zu bekommen.
/*N*/ 					if ( pFCell->GetDocument()->IsInInterpreter() &&
/*N*/ 							(!pFCell->GetDocument()->GetMacroInterpretLevel()
/*N*/ 							|| pFCell->IsRunning()) )
/*N*/ 					{
/*?*/ 						rString.AssignAscii( RTL_CONSTASCII_STRINGPARAM("...") );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						USHORT nErrCode = pFCell->GetErrCode();
/*N*/ 						if ( nErrCode == errInterpOverflow )
/*N*/ 						{	// maxrecursion ausbuegeln, Err527 Workaround
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScDocument* pDoc = pFCell->GetDocument();
/*N*/ 						}
/*N*/ 
/*N*/ 						// erst nach dem Interpretieren (GetErrCode) das Zahlformat holen:
/*N*/ 						if ( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
/*N*/ 							nFormat = pFCell->GetStandardFormat( rFormatter,
/*N*/ 								nFormat );
/*N*/ 
/*N*/ 						if (nErrCode != 0)
/*N*/ 							rString = ScGlobal::GetErrorString(nErrCode);
/*N*/ 						else if ( pFCell->IsValue() )
/*N*/ 						{
/*N*/ 							double fValue = pFCell->GetValue();
/*N*/ 							if ( !bNullVals && fValue == 0.0 )
/*?*/ 								rString.Erase();
/*N*/ 							else
/*N*/ 								rFormatter.GetOutputString( fValue, nFormat, rString, ppColor );
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							String aCellString;
/*N*/ 							pFCell->GetString( aCellString );
/*N*/ 							rFormatter.GetOutputString( aCellString, nFormat, rString, ppColor );
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			rString.Erase();
/*N*/ 			break;
/*N*/ 	}
/*N*/ }

/*N*/ void ScCellFormat::GetInputString( ScBaseCell* pCell, ULONG nFormat, String& rString,
/*N*/ 									  SvNumberFormatter& rFormatter )
/*N*/ {
/*N*/ 	if (&rFormatter==NULL)
/*N*/ 	{
/*N*/ 		rString.Erase();
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	CellType eType = pCell->GetCellType();
/*N*/ 	switch(eType)
/*N*/ 	{
/*N*/ 		case CELLTYPE_STRING:
/*N*/ 			{
/*N*/ 				((ScStringCell*)pCell)->GetString( rString );
/*N*/ 			}
/*N*/ 			break;
/*?*/ 		case CELLTYPE_EDIT:
/*?*/ 			{
/*?*/ 				((ScEditCell*)pCell)->GetString( rString );
/*?*/ 			}
/*?*/ 			break;
/*?*/ 		case CELLTYPE_VALUE:
/*?*/ 			{
/*?*/ 				double nValue = ((ScValueCell*)pCell)->GetValue();
/*?*/ 				rFormatter.GetInputLineString( nValue, nFormat, rString );
/*?*/ 			}
/*?*/ 			break;
/*N*/ 		case CELLTYPE_FORMULA:
/*N*/ 			{
/*N*/ 				if (((ScFormulaCell*)pCell)->IsValue())
/*N*/ 				{
/*?*/ 					double nValue = ((ScFormulaCell*)pCell)->GetValue();
/*?*/ 					rFormatter.GetInputLineString( nValue, nFormat, rString );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					((ScFormulaCell*)pCell)->GetString( rString );
/*N*/ 				}
/*N*/ 
/*N*/ 				USHORT nErrCode = ((ScFormulaCell*)pCell)->GetErrCode();
/*N*/ 				if (nErrCode != 0)
/*N*/ 				{
/*?*/ 					rString.Erase();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*?*/ 		default:
/*?*/ 			rString.Erase();
/*?*/ 			break;
/*N*/ 	}
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
