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

// INCLUDE ---------------------------------------------------------------

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <bf_svtools/zforlist.hxx>

#include "dbdocutl.hxx"
#include "document.hxx"
#include "cell.hxx"
namespace binfilter {

using namespace ::com::sun::star;

#define D_TIMEFACTOR              86400.0

// -----------------------------------------------------------------------

// static
/*N*/ void ScDatabaseDocUtil::PutData( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab,
/*N*/ 								const uno::Reference<sdbc::XRow>& xRow, long nRowPos,
/*N*/ 								long nType, BOOL bCurrency, BOOL* pSimpleFlag )
/*N*/ {
/*N*/ 	String aString;
/*N*/ 	double nVal = 0.0;
/*N*/ 	BOOL bValue = FALSE;
/*N*/ 	BOOL bEmptyFlag = FALSE;
/*N*/ 	BOOL bError = FALSE;
/*N*/ 	ULONG nFormatIndex = 0;
/*N*/ 
/*N*/ 	//!	wasNull calls only if null value was found?
/*N*/ 
/*N*/ 	try
/*N*/ 	{
/*N*/ 		switch ( nType )
/*N*/ 		{
/*N*/ 			case sdbc::DataType::BIT:
/*N*/ 				//!	use language from doc (here, date/time and currency)?
/*N*/ 				nFormatIndex = pDoc->GetFormatTable()->GetStandardFormat(
/*N*/ 									NUMBERFORMAT_LOGICAL, ScGlobal::eLnge );
/*N*/ 				nVal = (xRow->getBoolean(nRowPos) ? 1 : 0);
/*N*/ 				bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
/*N*/ 				bValue = TRUE;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case sdbc::DataType::TINYINT:
/*N*/ 			case sdbc::DataType::SMALLINT:
/*N*/ 			case sdbc::DataType::INTEGER:
/*N*/ 			case sdbc::DataType::BIGINT:
/*N*/ 			case sdbc::DataType::FLOAT:
/*N*/ 			case sdbc::DataType::REAL:
/*N*/ 			case sdbc::DataType::DOUBLE:
/*N*/ 			case sdbc::DataType::NUMERIC:
/*N*/ 			case sdbc::DataType::DECIMAL:
/*N*/ 				//!	do the conversion here?
/*N*/ 				nVal = xRow->getDouble(nRowPos);
/*N*/ 				bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
/*N*/ 				bValue = TRUE;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case sdbc::DataType::CHAR:
/*N*/ 			case sdbc::DataType::VARCHAR:
/*N*/ 			case sdbc::DataType::LONGVARCHAR:
/*N*/ 				aString = xRow->getString(nRowPos);
/*N*/ 				bEmptyFlag = ( aString.Len() == 0 ) && xRow->wasNull();
/*N*/ 				break;
/*N*/ 
/*N*/ 			case sdbc::DataType::DATE:
/*N*/ 				{
/*N*/ 					SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
/*N*/ 					nFormatIndex = pFormTable->GetStandardFormat(
/*N*/ 										NUMBERFORMAT_DATE, ScGlobal::eLnge );
/*N*/ 
/*N*/ 					util::Date aDate = xRow->getDate(nRowPos);
/*N*/ 					nVal = Date( aDate.Day, aDate.Month, aDate.Year ) -
/*N*/ 												*pFormTable->GetNullDate();
/*N*/ 					bEmptyFlag = xRow->wasNull();
/*N*/ 					bValue = TRUE;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 			case sdbc::DataType::TIME:
/*N*/ 				{
/*N*/ 					SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
/*N*/ 					nFormatIndex = pFormTable->GetStandardFormat(
/*N*/ 										NUMBERFORMAT_TIME, ScGlobal::eLnge );
/*N*/ 
/*N*/ 					util::Time aTime = xRow->getTime(nRowPos);
/*N*/ 					nVal = ( aTime.Hours * 3600 + aTime.Minutes * 60 +
/*N*/ 							 aTime.Seconds + aTime.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
/*N*/ 					bEmptyFlag = xRow->wasNull();
/*N*/ 					bValue = TRUE;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 			case sdbc::DataType::TIMESTAMP:
/*N*/ 				{
/*N*/ 					SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
/*N*/ 					nFormatIndex = pFormTable->GetStandardFormat(
/*N*/ 										NUMBERFORMAT_DATETIME, ScGlobal::eLnge );
/*N*/ 
/*N*/ 					util::DateTime aStamp = xRow->getTimestamp(nRowPos);
/*N*/ 					nVal = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) -
/*N*/ 												*pFormTable->GetNullDate() ) +
/*N*/ 						   ( aStamp.Hours * 3600 + aStamp.Minutes * 60 +
/*N*/ 							 aStamp.Seconds + aStamp.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
/*N*/ 					bEmptyFlag = xRow->wasNull();
/*N*/ 					bValue = TRUE;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 			case sdbc::DataType::SQLNULL:
/*N*/ 				bEmptyFlag = TRUE;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case sdbc::DataType::BINARY:
/*N*/ 			case sdbc::DataType::VARBINARY:
/*N*/ 			case sdbc::DataType::LONGVARBINARY:
/*N*/ 			default:
/*N*/ 				bError = TRUE;		// unknown type
/*N*/ 		}
/*N*/ 	}
/*N*/ 	catch ( uno::Exception& )
/*N*/ 	{
/*N*/ 		bError = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bValue && bCurrency )
/*N*/ 		nFormatIndex = pDoc->GetFormatTable()->GetStandardFormat(
/*N*/ 							NUMBERFORMAT_CURRENCY, ScGlobal::eLnge );
/*N*/ 
/*N*/ 	ScBaseCell* pCell;
/*N*/ 	if (bEmptyFlag)
/*N*/ 	{
/*N*/ 		pCell = NULL;
/*N*/ 		pDoc->PutCell( nCol, nRow, nTab, pCell );
/*N*/ 	}
/*N*/ 	else if (bError)
/*N*/ 	{
/*N*/ 		pDoc->SetError( nCol, nRow, nTab, NOVALUE );
/*N*/ 	}
/*N*/ 	else if (bValue)
/*N*/ 	{
/*?*/ 		pCell = new ScValueCell( nVal );
/*N*/ 		if (nFormatIndex == 0)
/*N*/ 			pDoc->PutCell( nCol, nRow, nTab, pCell );
/*N*/ 		else
/*N*/ 			pDoc->PutCell( nCol, nRow, nTab, pCell, nFormatIndex );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (aString.Len())
/*N*/ 		{
/*N*/ 			pCell = ScBaseCell::CreateTextCell( aString, pDoc );
/*N*/ 			if ( pSimpleFlag && pCell->GetCellType() == CELLTYPE_EDIT )
/*N*/ 				*pSimpleFlag = FALSE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pCell = NULL;
/*N*/ 		pDoc->PutCell( nCol, nRow, nTab, pCell );
/*N*/ 	}
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
