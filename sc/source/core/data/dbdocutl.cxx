/*************************************************************************
 *
 *  $RCSfile: dbdocutl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-03 19:22:09 $
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

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <svtools/zforlist.hxx>

#include "dbdocutl.hxx"
#include "document.hxx"
#include "cell.hxx"

using namespace ::com::sun::star;

#define D_TIMEFACTOR              86400.0

// -----------------------------------------------------------------------

// static
void ScDatabaseDocUtil::PutData( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab,
                                const uno::Reference<sdbc::XRow>& xRow, long nRowPos,
                                long nType, BOOL bCurrency, BOOL* pSimpleFlag )
{
    String aString;
    double nVal = 0.0;
    BOOL bValue = FALSE;
    BOOL bEmptyFlag = FALSE;
    BOOL bError = FALSE;
    ULONG nFormatIndex = 0;

    //! wasNull calls only if null value was found?

    try
    {
        switch ( nType )
        {
            case sdbc::DataType::BIT:
                //! use language from doc (here, date/time and currency)?
                nFormatIndex = pDoc->GetFormatTable()->GetStandardFormat(
                                    NUMBERFORMAT_LOGICAL, ScGlobal::eLnge );
                nVal = (xRow->getBoolean(nRowPos) ? 1 : 0);
                bEmptyFlag = ( nVal == 0.0 ) && xRow->wasNull();
                bValue = TRUE;
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
                bValue = TRUE;
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
                    bValue = TRUE;
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
                    bValue = TRUE;
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
                    bValue = TRUE;
                }
                break;

            case sdbc::DataType::SQLNULL:
                bEmptyFlag = TRUE;
                break;

            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                bError = TRUE;      // unknown type
        }
    }
    catch ( uno::Exception& )
    {
        bError = TRUE;
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
        pDoc->SetError( nCol, nRow, nTab, NOVALUE );
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
                *pSimpleFlag = FALSE;
        }
        else
            pCell = NULL;
        pDoc->PutCell( nCol, nRow, nTab, pCell );
    }
}


