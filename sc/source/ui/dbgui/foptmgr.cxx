/*************************************************************************
 *
 *  $RCSfile: foptmgr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mh $ $Date: 2001-10-23 09:05:11 $
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

// System - Includes ---------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif


// INCLUDE -------------------------------------------------------------------

#include <vcl/morebtn.hxx>
#include <svtools/stdctrl.hxx>

#include "anyrefdg.hxx"
#include "rangeutl.hxx"
#include "dbcolect.hxx"
#include "viewdata.hxx"
#include "document.hxx"

#define _FOPTMGR_CXX
#include "foptmgr.hxx"
#undef _FOPTMGR_CXX

//----------------------------------------------------------------------------

ScFilterOptionsMgr::ScFilterOptionsMgr(
                                Dialog*             ptrDlg,
                                ScViewData*         ptrViewData,
                                const ScQueryParam& refQueryData,
                                MoreButton&         refBtnMore,
                                CheckBox&           refBtnCase,
                                CheckBox&           refBtnRegExp,
                                CheckBox&           refBtnHeader,
                                CheckBox&           refBtnUnique,
                                CheckBox&           refBtnCopyResult,
                                CheckBox&           refBtnDestPers,
                                ListBox&            refLbCopyArea,
                                Edit&               refEdCopyArea,
                                ScRefButton&        refRbCopyArea,
                                FixedText&          refFtDbAreaLabel,
                                FixedInfo&          refFtDbArea,
                                FixedLine&          refFlOptions,
                                const String&       refStrNoName,
                                const String&       refStrUndefined )

    :   pDlg            ( ptrDlg ),
        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData ? ptrViewData->GetDocument() : NULL ),
        rQueryData      ( refQueryData ),
        rBtnMore        ( refBtnMore ),
        rBtnCase        ( refBtnCase ),
        rBtnRegExp      ( refBtnRegExp ),
        rBtnHeader      ( refBtnHeader ),
        rBtnUnique      ( refBtnUnique ),
        rBtnCopyResult  ( refBtnCopyResult ),
        rBtnDestPers    ( refBtnDestPers ),
        rLbCopyPos      ( refLbCopyArea ),
        rEdCopyPos      ( refEdCopyArea ),
        rRbCopyPos      ( refRbCopyArea ),
        rFtDbAreaLabel  ( refFtDbAreaLabel ),
        rFtDbArea       ( refFtDbArea ),
        rFlOptions      ( refFlOptions ),
        rStrNoName      ( refStrNoName ),
        rStrUndefined   ( refStrUndefined )
{
    Init();
}


//----------------------------------------------------------------------------

ScFilterOptionsMgr::~ScFilterOptionsMgr()
{
    USHORT nEntries = rLbCopyPos.GetEntryCount();
    USHORT i;

    for ( i=2; i<nEntries; i++ )
        delete (String*)rLbCopyPos.GetEntryData( i );
}


//----------------------------------------------------------------------------

void ScFilterOptionsMgr::Init()
{
    DBG_ASSERT( pViewData && pDoc, "Init failed :-/" );

    rLbCopyPos.SetSelectHdl  ( LINK( this, ScFilterOptionsMgr, LbPosSelHdl ) );
    rEdCopyPos.SetModifyHdl  ( LINK( this, ScFilterOptionsMgr, EdPosModifyHdl ) );
    rBtnCopyResult.SetClickHdl( LINK( this, ScFilterOptionsMgr, BtnCopyResultHdl ) );

    rBtnMore.AddWindow( &rBtnCase );
    rBtnMore.AddWindow( &rBtnRegExp );
    rBtnMore.AddWindow( &rBtnHeader );
    rBtnMore.AddWindow( &rBtnUnique );
    rBtnMore.AddWindow( &rBtnCopyResult );
    rBtnMore.AddWindow( &rBtnDestPers );
    rBtnMore.AddWindow( &rLbCopyPos );
    rBtnMore.AddWindow( &rEdCopyPos );
    rBtnMore.AddWindow( &rRbCopyPos );
    rBtnMore.AddWindow( &rFtDbAreaLabel );
    rBtnMore.AddWindow( &rFtDbArea );
    rBtnMore.AddWindow( &rFlOptions );

    rBtnCase    .Check( rQueryData.bCaseSens );
    rBtnHeader  .Check( rQueryData.bHasHeader );
    rBtnRegExp  .Check( rQueryData.bRegExp );
    rBtnUnique  .Check( !rQueryData.bDuplicate );

    if ( pViewData && pDoc )
    {
        String          theAreaStr;
        ScRange         theCurArea ( ScAddress( rQueryData.nCol1,
                                                rQueryData.nRow1,
                                                pViewData->GetTabNo() ),
                                     ScAddress( rQueryData.nCol2,
                                                rQueryData.nRow2,
                                                pViewData->GetTabNo() ) );
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        String          theDbArea;
        String          theDbName   = rStrNoName;

        theCurArea.Format( theAreaStr, SCR_ABS_3D, pDoc );

        // Zielbereichsliste fuellen

        rLbCopyPos.Clear();
        rLbCopyPos.InsertEntry( rStrUndefined, 0 );

        ScAreaNameIterator aIter( pDoc );
        String aName;
        ScRange aRange;
        String aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            USHORT nInsert = rLbCopyPos.InsertEntry( aName );

            aRange.aStart.Format( aRefStr, SCA_ABS_3D, pDoc );
            rLbCopyPos.SetEntryData( nInsert, new String( aRefStr ) );
        }

        rBtnDestPers.Check( TRUE );         // beim Aufruf immer an
        rLbCopyPos.SelectEntryPos( 0 );
        rEdCopyPos.SetText( EMPTY_STRING );

        /*
         * Ueberpruefen, ob es sich bei dem uebergebenen
         * Bereich um einen Datenbankbereich handelt:
         */

        theDbArea = theAreaStr;

        if ( pDBColl )
        {
            ScAddress&  rStart  = theCurArea.aStart;
            ScAddress&  rEnd    = theCurArea.aEnd;
            ScDBData*   pDBData = pDBColl->GetDBAtArea( rStart.Tab(),
                                                        rStart.Col(), rStart.Row(),
                                                        rEnd.Col(),   rEnd.Row() );
            if ( pDBData )
            {
                rBtnHeader.Check( pDBData->HasHeader() );
                pDBData->GetName( theDbName );

                if ( theDbName != rStrNoName )
                {
                    rBtnHeader.Disable();
                }
            }
        }

        theDbArea.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" ("));
        theDbArea += theDbName;
        theDbArea += ')';
        rFtDbArea.SetText( theDbArea );

        //------------------------------------------------------
        // Kopierposition:

        if ( !rQueryData.bInplace )
        {
            String aString;

            ScAddress( rQueryData.nDestCol,
                       rQueryData.nDestRow,
                       rQueryData.nDestTab
                     ).Format( aString, SCA_ABS_3D, pDoc );

            rBtnCopyResult.Check( TRUE );
            rEdCopyPos.SetText( aString );
            EdPosModifyHdl( &rEdCopyPos );
            rLbCopyPos.Enable();
            rEdCopyPos.Enable();
            rRbCopyPos.Enable();
            rBtnDestPers.Enable();
        }
        else
        {
            rBtnCopyResult.Check( FALSE );
            rEdCopyPos.SetText( EMPTY_STRING );
            rLbCopyPos.Disable();
            rEdCopyPos.Disable();
            rRbCopyPos.Disable();
            rBtnDestPers.Disable();
        }
    }
    else
        rEdCopyPos.SetText( EMPTY_STRING );
}


//----------------------------------------------------------------------------

BOOL ScFilterOptionsMgr::VerifyPosStr( const String& rPosStr ) const
{
    String aPosStr( rPosStr );
    xub_StrLen nColonPos = aPosStr.Search( ':' );

    if ( STRING_NOTFOUND != nColonPos )
        aPosStr.Erase( nColonPos );

    USHORT nResult = ScAddress().Parse( aPosStr, pDoc );

    return ( SCA_VALID == (nResult & SCA_VALID) );
}


//----------------------------------------------------------------------------
// Handler:

//----------------------------------------------------------------------------

IMPL_LINK( ScFilterOptionsMgr, LbPosSelHdl, ListBox*, pLb )
{
    if ( pLb == &rLbCopyPos )
    {
        String aString;
        USHORT nSelPos = rLbCopyPos.GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *(String*)rLbCopyPos.GetEntryData( nSelPos );

        rEdCopyPos.SetText( aString );
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterOptionsMgr, EdPosModifyHdl, Edit*, pEd )
{
    if ( pEd == &rEdCopyPos )
    {
        String  theCurPosStr = pEd->GetText();
        USHORT  nResult = ScAddress().Parse( theCurPosStr, pDoc );

        if ( SCA_VALID == (nResult & SCA_VALID) )
        {
            String* pStr    = NULL;
            BOOL    bFound  = FALSE;
            USHORT  i       = 0;
            USHORT  nCount  = rLbCopyPos.GetEntryCount();

            for ( i=2; i<nCount && !bFound; i++ )
            {
                pStr = (String*)rLbCopyPos.GetEntryData( i );
                bFound = (theCurPosStr == *pStr);
            }

            if ( bFound )
                rLbCopyPos.SelectEntryPos( --i );
            else
                rLbCopyPos.SelectEntryPos( 0 );
        }
        else
            rLbCopyPos.SelectEntryPos( 0 );
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterOptionsMgr, BtnCopyResultHdl, CheckBox*, pBox )
{
    if ( pBox == &rBtnCopyResult )
    {
        if ( pBox->IsChecked() )
        {
            rBtnDestPers.Enable();
            rLbCopyPos.Enable();
            rEdCopyPos.Enable();
            rRbCopyPos.Enable();
            rEdCopyPos.GrabFocus();
        }
        else
        {
            rBtnDestPers.Disable();
            rLbCopyPos.Disable();
            rEdCopyPos.Disable();
            rRbCopyPos.Disable();
        }
    }

    return 0;
}
