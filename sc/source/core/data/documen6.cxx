/*************************************************************************
 *
 *  $RCSfile: documen6.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:14 $
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

#include <svtools/zforlist.hxx>
#include <sdb/sdbcol.hxx>
#include <sdb/variant.hxx>
#include <vos/xception.hxx>

#include "document.hxx"
#include "cell.hxx"

//
//  this file is compiled with exceptions enabled
//  put functions here that need exceptions!
//

// -----------------------------------------------------------------------

void ScDocument::PutVariable( USHORT nCol, USHORT nRow, USHORT nTab,
                                const ODbVariant* pVar, long nType, BOOL* pSimpleFlag )
{
    if (nTab<=MAXTAB && pTab[nTab])
    {
        String aString;
        double nVal = 0.0;
        BOOL bValue = FALSE;
        BOOL bEmptyFlag = FALSE;
        BOOL bError = FALSE;
        ULONG nFormatIndex = 0;

        if ( !pVar || pVar->isNull() )
            bEmptyFlag = TRUE;
        else
        {
            TRY
            {
                switch ( (SdbDatabaseType)nType )
                {
                    case SDB_DBTYPE_CHAR:
                    case SDB_DBTYPE_VARCHAR:
                    case SDB_DBTYPE_LONGVARCHAR:
                        aString = String( pVar->toString() );
                        break;

                    case SDB_DBTYPE_DECIMAL:
                    case SDB_DBTYPE_NUMERIC:
                    case SDB_DBTYPE_TINYINT:
                    case SDB_DBTYPE_SMALLINT:
                    case SDB_DBTYPE_INTEGER:
                    case SDB_DBTYPE_BIGINT:
                    case SDB_DBTYPE_REAL:
                    case SDB_DBTYPE_DOUBLE:
                        nVal = pVar->toDouble();
                        bValue = TRUE;
                        break;

                    case SDB_DBTYPE_CURRENCY:
                        nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_CURRENCY, ScGlobal::eLnge );
                        nVal = pVar->toDouble();
                        bValue = TRUE;
                        break;

                    case SDB_DBTYPE_BOOLEAN:
                        nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_LOGICAL, ScGlobal::eLnge );
                        nVal = (pVar->toBOOL() ? 1 : 0);
                        bValue = TRUE;
                        break;

                    case SDB_DBTYPE_DATE:
                        //! subtract base date difference if var contains numeric value?
                        nFormatIndex = pFormTable->GetStandardFormat(
                                        NUMBERFORMAT_DATE, ScGlobal::eLnge );
                        nVal = pVar->toDate() - *pFormTable->GetNullDate();
                        bValue = TRUE;
                        break;

                    default:
                        aString = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("[unknown type]"));
                }
            }
            CATCH_ALL()
            {
                bError = TRUE;
            }
            END_CATCH
        }

        ScBaseCell* pCell;
        if (bEmptyFlag)
        {
            pCell = NULL;
            PutCell( nCol, nRow, nTab, pCell );
        }
        else if (bError)
        {
            SetError( nCol, nRow, nTab, NOVALUE );
        }
        else if (bValue)
        {
            pCell = new ScValueCell( nVal );
            if (nFormatIndex == 0)
                PutCell( nCol, nRow, nTab, pCell );
            else
                PutCell( nCol, nRow, nTab, pCell, nFormatIndex );
        }
        else
        {
            pCell = ScBaseCell::CreateTextCell( aString, this );
            if ( pSimpleFlag && pCell->GetCellType() == CELLTYPE_EDIT )
                *pSimpleFlag = FALSE;
            PutCell( nCol, nRow, nTab, pCell );
        }
    }
}






