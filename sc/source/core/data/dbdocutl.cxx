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

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <svl/zforlist.hxx>

#include "dbdocutl.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "formula/errorcodes.hxx"

using namespace ::com::sun::star;

#define D_TIMEFACTOR              86400.0

// -----------------------------------------------------------------------

// static
void ScDatabaseDocUtil::PutData( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const uno::Reference<sdbc::XRow>& xRow, long nRowPos,
                                long nType, sal_Bool bCurrency, sal_Bool* pSimpleFlag )
{
    String aString;
    double nVal = 0.0;
    sal_Bool bValue = sal_False;
    sal_Bool bEmptyFlag = sal_False;
    sal_Bool bError = sal_False;
    sal_uLong nFormatIndex = 0;

    //! wasNull calls only if null value was found?

    try
    {
        switch ( nType )
        {
            case sdbc::DataType::BIT:
            case sdbc::DataType::BOOLEAN:
                //! use language from doc (here, date/time and currency)?
                nFormatIndex = pDoc->GetFormatTable()->GetStandardFormat(
                                    NUMBERFORMAT_LOGICAL, ScGlobal::eLnge );
                nVal = (xRow->getBoolean(nRowPos) ? 1 : 0);
                bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
                bValue = sal_True;
                break;

            case sdbc::DataType::TINYINT:
            case sdbc::DataType::SMALLINT:
            case sdbc::DataType::INTEGER:
            case sdbc::DataType::BIGINT:
            case sdbc::DataType::FLOAT:
            case sdbc::DataType::REAL:
            case sdbc::DataType::DOUBLE:
            case sdbc::DataType::NUMERIC:
            case sdbc::DataType::DECIMAL:
                //! do the conversion here?
                nVal = xRow->getDouble(nRowPos);
                bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
                bValue = sal_True;
                break;

            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
                aString = xRow->getString(nRowPos);
                bEmptyFlag = ( aString.Len() == 0 ) && xRow->wasNull();
                break;

            case sdbc::DataType::DATE:
                {
                    SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
                    nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_DATE, ScGlobal::eLnge );

                    util::Date aDate = xRow->getDate(nRowPos);
                    nVal = Date( aDate.Day, aDate.Month, aDate.Year ) -
                                                *pFormTable->GetNullDate();
                    bEmptyFlag = xRow->wasNull();
                    bValue = sal_True;
                }
                break;

            case sdbc::DataType::TIME:
                {
                    SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
                    nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_TIME, ScGlobal::eLnge );

                    util::Time aTime = xRow->getTime(nRowPos);
                    nVal = ( aTime.Hours * 3600 + aTime.Minutes * 60 +
                             aTime.Seconds + aTime.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                    bEmptyFlag = xRow->wasNull();
                    bValue = sal_True;
                }
                break;

            case sdbc::DataType::TIMESTAMP:
                {
                    SvNumberFormatter* pFormTable = pDoc->GetFormatTable();
                    nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_DATETIME, ScGlobal::eLnge );

                    util::DateTime aStamp = xRow->getTimestamp(nRowPos);
                    nVal = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) -
                                                *pFormTable->GetNullDate() ) +
                           ( aStamp.Hours * 3600 + aStamp.Minutes * 60 +
                             aStamp.Seconds + aStamp.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                    bEmptyFlag = xRow->wasNull();
                    bValue = sal_True;
                }
                break;

            case sdbc::DataType::SQLNULL:
                bEmptyFlag = sal_True;
                break;

            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                bError = sal_True;      // unknown type
        }
    }
    catch ( uno::Exception& )
    {
        bError = sal_True;
    }

    if ( bValue && bCurrency )
        nFormatIndex = pDoc->GetFormatTable()->GetStandardFormat(
                            NUMBERFORMAT_CURRENCY, ScGlobal::eLnge );

    ScBaseCell* pCell;
    if (bEmptyFlag)
    {
        pCell = NULL;
        pDoc->PutCell( nCol, nRow, nTab, pCell );
    }
    else if (bError)
    {
        pDoc->SetError( nCol, nRow, nTab, NOTAVAILABLE );
    }
    else if (bValue)
    {
        pCell = new ScValueCell( nVal );
        if (nFormatIndex == 0)
            pDoc->PutCell( nCol, nRow, nTab, pCell );
        else
            pDoc->PutCell( nCol, nRow, nTab, pCell, nFormatIndex );
    }
    else
    {
        if (aString.Len())
        {
            pCell = ScBaseCell::CreateTextCell( aString, pDoc );
            if ( pSimpleFlag && pCell->GetCellType() == CELLTYPE_EDIT )
                *pSimpleFlag = sal_False;
        }
        else
            pCell = NULL;
        pDoc->PutCell( nCol, nRow, nTab, pCell );
    }
}


