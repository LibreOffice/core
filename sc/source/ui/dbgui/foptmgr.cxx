/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/morebtn.hxx>
#include <svtools/stdctrl.hxx>

#include "anyrefdg.hxx"
#include "rangeutl.hxx"
#include "dbdata.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "queryparam.hxx"
#include "globalnames.hxx"

#define _FOPTMGR_CXX
#define _NEWFOPTMGR_CXX
#include "foptmgr.hxx"
#undef _NEWFOPTMGR_CXX
#undef _FOPTMGR_CXX

//----------------------------------------------------------------------------

ScFilterOptionsMgr::ScFilterOptionsMgr(
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
                                formula::RefButton&     refRbCopyArea,
                                FixedText&          refFtDbAreaLabel,
                                FixedInfo&          refFtDbArea,
                                FixedLine&          refFlOptions,
                                const String&       refStrUndefined )

    :   pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData ? ptrViewData->GetDocument() : NULL ),
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
        rStrUndefined   ( refStrUndefined ),
        rQueryData      ( refQueryData )
{
    Init();
}


//----------------------------------------------------------------------------

ScFilterOptionsMgr::~ScFilterOptionsMgr()
{
    sal_uInt16 nEntries = rLbCopyPos.GetEntryCount();
    sal_uInt16 i;

    for ( i=2; i<nEntries; i++ )
        delete (String*)rLbCopyPos.GetEntryData( i );
}


//----------------------------------------------------------------------------

void ScFilterOptionsMgr::Init()
{
//moggi:TODO
    OSL_ENSURE( pViewData && pDoc, "Init failed :-/" );

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
        OUString theAreaStr;
        ScRange         theCurArea ( ScAddress( rQueryData.nCol1,
                                                rQueryData.nRow1,
                                                pViewData->GetTabNo() ),
                                     ScAddress( rQueryData.nCol2,
                                                rQueryData.nRow2,
                                                pViewData->GetTabNo() ) );
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        OUStringBuffer theDbArea;
        OUString   theDbName(STR_DB_LOCAL_NONAME);
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        theCurArea.Format( theAreaStr, SCR_ABS_3D, pDoc, eConv );

        // Zielbereichsliste fuellen

        rLbCopyPos.Clear();
        rLbCopyPos.InsertEntry( rStrUndefined, 0 );

        ScAreaNameIterator aIter( pDoc );
        String aName;
        ScRange aRange;
        String aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            sal_uInt16 nInsert = rLbCopyPos.InsertEntry( aName );

            aRange.aStart.Format( aRefStr, SCA_ABS_3D, pDoc, eConv );
            rLbCopyPos.SetEntryData( nInsert, new String( aRefStr ) );
        }

        rBtnDestPers.Check( sal_True );         // beim Aufruf immer an
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
            const ScDBData* pDBData = pDBColl->GetDBAtArea(
                rStart.Tab(), rStart.Col(), rStart.Row(), rEnd.Col(), rEnd.Row());

            if ( pDBData )
            {
                rBtnHeader.Check( pDBData->HasHeader() );
                theDbName = pDBData->GetName();

                if ( theDbName == STR_DB_LOCAL_NONAME )
                    rBtnHeader.Enable();
                else
                    rBtnHeader.Disable();
            }
        }

        if ( !theDbName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(STR_DB_LOCAL_NONAME)) )
        {
            theDbArea.appendAscii(RTL_CONSTASCII_STRINGPARAM(" ("));
            theDbArea.append(theDbName).append(')');
            rFtDbArea.SetText( theDbArea.makeStringAndClear() );
        }
        else
        {
            rFtDbAreaLabel.SetText( OUString() );
            rFtDbArea.SetText( OUString() );
        }

        //------------------------------------------------------
        // Kopierposition:

        if ( !rQueryData.bInplace )
        {
            String aString;

            ScAddress( rQueryData.nDestCol,
                       rQueryData.nDestRow,
                       rQueryData.nDestTab
                     ).Format( aString, SCA_ABS_3D, pDoc, eConv );

            rBtnCopyResult.Check( sal_True );
            rEdCopyPos.SetText( aString );
            EdPosModifyHdl( &rEdCopyPos );
            rLbCopyPos.Enable();
            rEdCopyPos.Enable();
            rRbCopyPos.Enable();
            rBtnDestPers.Enable();
        }
        else
        {
            rBtnCopyResult.Check( false );
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

sal_Bool ScFilterOptionsMgr::VerifyPosStr( const String& rPosStr ) const
{
    String aPosStr( rPosStr );
    xub_StrLen nColonPos = aPosStr.Search( ':' );

    if ( STRING_NOTFOUND != nColonPos )
        aPosStr.Erase( nColonPos );

    sal_uInt16 nResult = ScAddress().Parse( aPosStr, pDoc, pDoc->GetAddressConvention() );

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
        sal_uInt16 nSelPos = rLbCopyPos.GetSelectEntryPos();

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
        sal_uInt16  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

        if ( SCA_VALID == (nResult & SCA_VALID) )
        {
            String* pStr    = NULL;
            sal_Bool    bFound  = false;
            sal_uInt16  i       = 0;
            sal_uInt16  nCount  = rLbCopyPos.GetEntryCount();

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
//----------------------------------------------------------------------------
// ScNewFilterOptionsMgr (.ui's option helper)
//----------------------------------------------------------------------------

ScNewFilterOptionsMgr::ScNewFilterOptionsMgr(
                                ScViewData*         ptrViewData,
                                const ScQueryParam& refQueryData,
                                CheckBox*           refBtnCase,
                                CheckBox*           refBtnRegExp,
                                CheckBox*           refBtnHeader,
                                CheckBox*           refBtnUnique,
                                CheckBox*           refBtnCopyResult,
                                CheckBox*           refBtnDestPers,
                                ListBox*            refLbCopyArea,
                                Edit*               refEdCopyArea,
                                formula::RefButton*     refRbCopyArea,
                                FixedText*          refFtDbAreaLabel,
                                FixedText*          refFtDbArea,
                                const String&       refStrUndefined )

    :   pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData ? ptrViewData->GetDocument() : NULL ),
        pBtnCase        ( refBtnCase ),
        pBtnRegExp      ( refBtnRegExp ),
        pBtnHeader      ( refBtnHeader ),
        pBtnUnique      ( refBtnUnique ),
        pBtnCopyResult  ( refBtnCopyResult ),
        pBtnDestPers    ( refBtnDestPers ),
        pLbCopyArea      ( refLbCopyArea ),
        pEdCopyArea      ( refEdCopyArea ),
        pRbCopyArea      ( refRbCopyArea ),
        pFtDbAreaLabel  ( refFtDbAreaLabel ),
        pFtDbArea       ( refFtDbArea ),
        rStrUndefined   ( refStrUndefined ),
        rQueryData      ( refQueryData )
{
    Init();
}


//----------------------------------------------------------------------------

ScNewFilterOptionsMgr::~ScNewFilterOptionsMgr()
{
    sal_uInt16 nEntries = pLbCopyArea->GetEntryCount();
    sal_uInt16 i;

    for ( i=2; i<nEntries; i++ )
        delete (String*)pLbCopyArea->GetEntryData( i );
}


//----------------------------------------------------------------------------

void ScNewFilterOptionsMgr::Init()
{
//moggi:TODO
    OSL_ENSURE( pViewData && pDoc, "Init failed :-/" );

    pLbCopyArea->SetSelectHdl  ( LINK( this, ScNewFilterOptionsMgr, LbAreaSelHdl ) );
    pEdCopyArea->SetModifyHdl  ( LINK( this, ScNewFilterOptionsMgr, EdAreaModifyHdl ) );
    pBtnCopyResult->SetToggleHdl ( LINK( this, ScNewFilterOptionsMgr, BtnCopyResultHdl ) );

    pBtnCase   ->Check( rQueryData.bCaseSens );
    pBtnHeader ->Check( rQueryData.bHasHeader );
    pBtnRegExp ->Check( rQueryData.bRegExp );
    pBtnUnique ->Check( !rQueryData.bDuplicate );

    if ( pViewData && pDoc )
    {
        OUString theAreaStr;
        ScRange         theCurArea ( ScAddress( rQueryData.nCol1,
                                                rQueryData.nRow1,
                                                pViewData->GetTabNo() ),
                                     ScAddress( rQueryData.nCol2,
                                                rQueryData.nRow2,
                                                pViewData->GetTabNo() ) );
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        OUStringBuffer theDbArea;
        OUString   theDbName(STR_DB_LOCAL_NONAME);
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        theCurArea.Format( theAreaStr, SCR_ABS_3D, pDoc, eConv );

        // Zielbereichsliste fuellen

        pLbCopyArea->Clear();
        pLbCopyArea->InsertEntry( rStrUndefined, 0 );

        ScAreaNameIterator aIter( pDoc );
        String aName;
        ScRange aRange;
        String aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            sal_uInt16 nInsert = pLbCopyArea->InsertEntry( aName );

            aRange.aStart.Format( aRefStr, SCA_ABS_3D, pDoc, eConv );
            pLbCopyArea->SetEntryData( nInsert, new String( aRefStr ) );
        }

        pBtnDestPers->Check( sal_True );         // beim Aufruf immer an
        pLbCopyArea->SelectEntryPos( 0 );
        pEdCopyArea->SetText( EMPTY_STRING );

        /*
         * Ueberpruefen, ob es sich bei dem uebergebenen
         * Bereich um einen Datenbankbereich handelt:
         */

        theDbArea = theAreaStr;

        if ( pDBColl )
        {
            ScAddress&  rStart  = theCurArea.aStart;
            ScAddress&  rEnd    = theCurArea.aEnd;
            const ScDBData* pDBData = pDBColl->GetDBAtArea(
                rStart.Tab(), rStart.Col(), rStart.Row(), rEnd.Col(), rEnd.Row());

            if ( pDBData )
            {
                pBtnHeader->Check( pDBData->HasHeader() );
                theDbName = pDBData->GetName();

                if ( theDbName == STR_DB_LOCAL_NONAME )
                    pBtnHeader->Enable();
                else
                    pBtnHeader->Disable();
            }
        }

        if ( !theDbName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(STR_DB_LOCAL_NONAME)) )
        {
            theDbArea.appendAscii(RTL_CONSTASCII_STRINGPARAM(" ("));
            theDbArea.append(theDbName).append(')');
            pFtDbArea->SetText( theDbArea.makeStringAndClear() );
        }
        else
        {
            pFtDbAreaLabel->SetText( OUString() );
            pFtDbArea->SetText( OUString() );
        }

        //------------------------------------------------------
        // Kopierposition:

        if ( !rQueryData.bInplace )
        {
            String aString;

            ScAddress( rQueryData.nDestCol,
                       rQueryData.nDestRow,
                       rQueryData.nDestTab
                     ).Format( aString, SCA_ABS_3D, pDoc, eConv );

            pBtnCopyResult->Check( sal_True );
            pEdCopyArea->SetText( aString );
            EdAreaModifyHdl( pEdCopyArea );
            pLbCopyArea->Enable();
            pEdCopyArea->Enable();
            pRbCopyArea->Enable();
            pBtnDestPers->Enable();
        }
        else
        {
            pBtnCopyResult->Check( false );
            pEdCopyArea->SetText( EMPTY_STRING );
            pLbCopyArea->Disable();
            pEdCopyArea->Disable();
            pRbCopyArea->Disable();
            pBtnDestPers->Disable();
        }
    }
    else
        pEdCopyArea->SetText( EMPTY_STRING );
}

//----------------------------------------------------------------------------
// Handler:

//----------------------------------------------------------------------------

IMPL_LINK( ScNewFilterOptionsMgr, LbAreaSelHdl, ListBox*, pLb )
{
    if ( pLb == pLbCopyArea )
    {
        String aString;
        sal_uInt16 nSelPos = pLbCopyArea->GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *(String*)pLbCopyArea->GetEntryData( nSelPos );

        pEdCopyArea->SetText( aString );
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScNewFilterOptionsMgr, EdAreaModifyHdl, Edit*, pEd )
{
    if ( pEd == pEdCopyArea )
    {
        String  theCurPosStr = pEd->GetText();
        sal_uInt16  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

        if ( SCA_VALID == (nResult & SCA_VALID) )
        {
            String* pStr    = NULL;
            sal_Bool    bFound  = false;
            sal_uInt16  i       = 0;
            sal_uInt16  nCount  = pLbCopyArea->GetEntryCount();

            for ( i=2; i<nCount && !bFound; i++ )
            {
                pStr = (String*)pLbCopyArea->GetEntryData( i );
                bFound = (theCurPosStr == *pStr);
            }

            if ( bFound )
                pLbCopyArea->SelectEntryPos( --i );
            else
                pLbCopyArea->SelectEntryPos( 0 );
        }
        else
            pLbCopyArea->SelectEntryPos( 0 );
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScNewFilterOptionsMgr, BtnCopyResultHdl, CheckBox*, pBox )
{
    if ( pBox == pBtnCopyResult )
    {
        if ( pBox->IsChecked() )
        {
            pBtnDestPers->Enable();
            pLbCopyArea->Enable();
            pEdCopyArea->Enable();
            pRbCopyArea->Enable();
            pEdCopyArea->GrabFocus();
        }
        else
        {
            pBtnDestPers->Disable();
            pLbCopyArea->Disable();
            pEdCopyArea->Disable();
            pRbCopyArea->Disable();
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
