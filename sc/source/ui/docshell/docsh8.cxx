/*************************************************************************
 *
 *  $RCSfile: docsh8.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
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

#if defined(OS2) && defined(BLC)
// ohne -vi- verhaspelt sich der Dummbatz bei SdbCursor::Variable() inlining
#pragma option -vi-
#endif

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <sdb/sdbcol.hxx>
#include <sdb/sdbconn.hxx>
#include <sdb/sdbcurs.hxx>
#include <tools/list.hxx>
#include "scitems.hxx"
#include <offmgr/sba.hrc>
#include <offmgr/sbaobj.hxx>
#include <offmgr/app.hxx>
#include <svtools/zforlist.hxx>
#include <vcl/virdev.hxx>
#include <svtools/converter.hxx>

#include "scerrors.hxx"
#include "docsh.hxx"
#include "filter.hxx"
#include "cell.hxx"
#include "progress.hxx"
#include "globstr.hrc"

// STATIC DATA -----------------------------------------------------------

ULONG ScDocShell::SbaSdbImport(const String& rName, const String& rParStr,
            BOOL bHeader, BOOL bSimpleColWidth[MAXCOL+1] )
{
    ULONG nErr = eERR_OK;
    SbaObject* pSbaObject = ((OfficeApplication*)SFX_APP())->GetSbaObject();
    DBG_ASSERT( pSbaObject, "pSbaObject==NULL" );
    SdbConnectionRef xConnect = pSbaObject->OpenConnection(rParStr);

    if( !xConnect.Is() )
        nErr = SCERR_IMPORT_CONNECT;
    else
    {
        USHORT nCol = 0;
        USHORT nRow = 0;
        UINT16 nCount;
        UINT16 i;
        //! fuer den RowCount einen extra Cursor anlegen
        SdbCursorRef xCount = xConnect->CreateCursor( SDB_SNAPSHOT, SDB_READONLY | SDB_FORWARDONLY );
        // laut Ocke soll das per SQL am schnellsten sein
        String aSel = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
                        "SELECT COUNT ( * ) FROM \"" ));
        aSel += rName;
        aSel += '"';
        xCount->Open( aSel, FALSE );

//      ULONG nRowCount = ( xCount->Status().IsError() ? 1 :
//          xCount->Variable(1)->GetLong() );

        ULONG nRowCount = ( xCount->Status().IsError() ? 1 :
            (*xCount->GetRow())[1]->toINT32() );

        xCount->Close();
        SdbCursorRef xCur = xConnect->CreateCursor( SDB_SNAPSHOT, SDB_READONLY | SDB_FORWARDONLY );
        SdbCursor* pCursor = &xCur;
        DBG_ASSERT( pCursor, "Cursor" );
        pCursor->Open( rName, TRUE );

        if ( !pCursor->Status().IsError() )
        {
            if ( nRowCount > MAXROW + 1 )
                nRowCount = MAXROW + 1;
            ScProgress aProgress( this, ScGlobal::GetRscString( STR_LOAD_DOC ),
                nRowCount );

            const SdbColumnsRef pColumns = pCursor->GetColumns();
            nCount = pColumns->Count();
            if ( nCount > (bHeader ? 1 : 0) )
                aDocument.DoColResize( 0, 0, nCount - (bHeader ? 2 : 1),
                    (bHeader ? nRowCount + 1 : nRowCount) );

            // Spaltenkoepfe
            SdbDatabaseType* pType = new SdbDatabaseType[nCount+1];
            if (nCount < 2)
            {
                DBG_ERROR( "ScDocShell::dBaseImport: Keine Spalten!" );
            }
            else if ( nCount > MAXCOL+1 )
                nErr = SCWARN_IMPORT_RANGE_OVERFLOW;        // Warnung ausgeben

            String aStr;
            for (i=1; i<nCount; i++)                        // 0 = Bookmark
            {
                const SdbColumn* pCol = pColumns->Column(i);
                aStr = pCol->GetName();
                switch ( pType[i] = pCol->GetType() )
                {
                    case SDB_DBTYPE_BOOLEAN :
                        aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",L" ));
                        break;
                    case SDB_DBTYPE_DATE :
                        aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",D" ));
                        break;
                    case SDB_DBTYPE_LONGVARCHAR :
                        aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",M" ));
                        break;
                    case SDB_DBTYPE_VARCHAR :
                        aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",C," ));
                        aStr += String::CreateFromInt32( pCol->GetLength() );
                        break;
                    case SDB_DBTYPE_DECIMAL :
                        aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ",N," ));
                        aStr += String::CreateFromInt32(
                                    SvDbaseConverter::ConvertPrecisionToDbase(
                                        pCol->GetLength(), pCol->GetScale() ) );
                        aStr += ',';
                        aStr += String::CreateFromInt32( pCol->GetScale() );
                        break;
                }
                if ( bHeader )
                {
                    aDocument.SetString( nCol, nRow, 0, aStr );
                    ++nCol;
                }
            }
            if ( bHeader )
                ++nRow;     // Daten

            //! pass standard format keys to PutVariable?
#if 0
            SvNumberFormatter* pFormatter = aDocument.GetFormatTable();
            ULONG nFormatBool = pFormatter->
                GetStandardFormat( NUMBERFORMAT_LOGICAL, ScGlobal::eLnge );
            ULONG nFormatDate = pFormatter->
                GetStandardFormat( NUMBERFORMAT_DATE, ScGlobal::eLnge );

            Date* pScDate = pFormatter->GetNullDate();
            Date aRefDate(30,12,1899);
//          Date aRefDate(3,1,1900);
            double nDateDiff;
            if ( pScDate && ! (*pScDate == aRefDate) )
                nDateDiff = (double)(aRefDate - *pScDate);
            else
                nDateDiff = 0;
#endif

            pCursor->First();
            while (pCursor->IsInRange())
            {
                if ( nRow > MAXROW )
                {
                    nErr = SCWARN_IMPORT_RANGE_OVERFLOW;        // Warnung ausgeben
                    break;
                }

                nCol = 0;
                for (i=1; i<nCount; i++)                        // 0 = Bookmark
                {
                    //  All data is in ODbVariant objects and handled in PutVariable
                    //  (including bSimpleColWidth)

                    BOOL* pSimple = (i <= MAXCOL+1) ? (&bSimpleColWidth[i-1]) : NULL;
                    ODbVariantRef xVar = (*pCursor->GetRow())[i];
                    aDocument.PutVariable( nCol, nRow, 0, xVar.getBodyPtr(), pType[i], pSimple );

                    ++nCol;
                }
                ++nRow;
                pCursor->Next();
                aProgress.SetStateOnPercent( nRow );
            }
            delete [] pType;
        }
        else
            nErr = SCERR_IMPORT_CURSOR;
        pCursor->Close();
    }
    return nErr;
}


