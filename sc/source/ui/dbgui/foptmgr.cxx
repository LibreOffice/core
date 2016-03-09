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

#include <svtools/stdctrl.hxx>

#include "anyrefdg.hxx"
#include "rangeutl.hxx"
#include "dbdata.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "queryparam.hxx"
#include "globalnames.hxx"

#include "foptmgr.hxx"

// ScFilterOptionsMgr (.ui's option helper)

ScFilterOptionsMgr::ScFilterOptionsMgr(
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
                                const OUString&     refStrUndefined )

    :   pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData ? ptrViewData->GetDocument() : nullptr ),
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

ScFilterOptionsMgr::~ScFilterOptionsMgr()
{
    const sal_Int32 nEntries = pLbCopyArea->GetEntryCount();

    for ( sal_Int32 i=2; i<nEntries; i++ )
        delete static_cast<OUString*>(pLbCopyArea->GetEntryData( i ));
}

void ScFilterOptionsMgr::Init()
{
//moggi:TODO
    OSL_ENSURE( pViewData && pDoc, "Init failed :-/" );

    pLbCopyArea->SetSelectHdl  ( LINK( this, ScFilterOptionsMgr, LbAreaSelHdl ) );
    pEdCopyArea->SetModifyHdl  ( LINK( this, ScFilterOptionsMgr, EdAreaModifyHdl ) );
    pBtnCopyResult->SetToggleHdl ( LINK( this, ScFilterOptionsMgr, BtnCopyResultHdl ) );

    pBtnCase   ->Check( rQueryData.bCaseSens );
    pBtnHeader ->Check( rQueryData.bHasHeader );
    pBtnRegExp ->Check( rQueryData.eSearchType == utl::SearchParam::SRCH_REGEXP );
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

        theAreaStr = theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDoc, eConv);

        // Zielbereichsliste fuellen

        pLbCopyArea->Clear();
        pLbCopyArea->InsertEntry( rStrUndefined, 0 );

        ScAreaNameIterator aIter( pDoc );
        OUString aName;
        ScRange aRange;
        while ( aIter.Next( aName, aRange ) )
        {
            const sal_Int32 nInsert = pLbCopyArea->InsertEntry( aName );

            OUString aRefStr(aRange.aStart.Format(ScRefFlags::ADDR_ABS_3D, pDoc, eConv));
            pLbCopyArea->SetEntryData( nInsert, new OUString( aRefStr ) );
        }

        pBtnDestPers->Check();         // beim Aufruf immer an
        pLbCopyArea->SelectEntryPos( 0 );
        pEdCopyArea->SetText( EMPTY_OUSTRING );

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

        if ( theDbName != STR_DB_LOCAL_NONAME )
        {
            theDbArea.append(" (");
            theDbArea.append(theDbName).append(')');
            pFtDbArea->SetText( theDbArea.makeStringAndClear() );
        }
        else
        {
            pFtDbAreaLabel->SetText( OUString() );
            pFtDbArea->SetText( OUString() );
        }

        // Kopierposition:

        if ( !rQueryData.bInplace )
        {
            OUString aString =
                ScAddress( rQueryData.nDestCol,
                           rQueryData.nDestRow,
                           rQueryData.nDestTab
                         ).Format(ScRefFlags::ADDR_ABS_3D, pDoc, eConv);

            pBtnCopyResult->Check();
            pEdCopyArea->SetText( aString );
            EdAreaModifyHdl( *pEdCopyArea );
            pLbCopyArea->Enable();
            pEdCopyArea->Enable();
            pRbCopyArea->Enable();
            pBtnDestPers->Enable();
        }
        else
        {
            pBtnCopyResult->Check( false );
            pEdCopyArea->SetText( EMPTY_OUSTRING );
            pLbCopyArea->Disable();
            pEdCopyArea->Disable();
            pRbCopyArea->Disable();
            pBtnDestPers->Disable();
        }
    }
    else
        pEdCopyArea->SetText( EMPTY_OUSTRING );
}

bool ScFilterOptionsMgr::VerifyPosStr( const OUString& rPosStr ) const
{
    OUString aPosStr( rPosStr );
    sal_Int32 nColonPos = aPosStr.indexOf( ':' );

    if ( -1 != nColonPos )
        aPosStr = aPosStr.copy( 0, nColonPos );

    ScRefFlags nResult = ScAddress().Parse( aPosStr, pDoc, pDoc->GetAddressConvention() );

    return (nResult & ScRefFlags::VALID) == ScRefFlags::VALID;
}

// Handler:

IMPL_LINK_TYPED( ScFilterOptionsMgr, LbAreaSelHdl, ListBox&, rLb, void )
{
    if ( &rLb == pLbCopyArea )
    {
        OUString aString;
        const sal_Int32 nSelPos = pLbCopyArea->GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *static_cast<OUString*>(pLbCopyArea->GetEntryData( nSelPos ));

        pEdCopyArea->SetText( aString );
    }
}

IMPL_LINK_TYPED( ScFilterOptionsMgr, EdAreaModifyHdl, Edit&, rEd, void )
{
    if ( &rEd == pEdCopyArea )
    {
        OUString  theCurPosStr = rEd.GetText();
        ScRefFlags  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

        if ( (nResult & ScRefFlags::VALID) == ScRefFlags::VALID)
        {
            const sal_Int32 nCount = pLbCopyArea->GetEntryCount();

            for ( sal_Int32 i=2; i<nCount; ++i )
            {
                OUString* pStr = static_cast<OUString*>(pLbCopyArea->GetEntryData( i ));
                if (theCurPosStr == *pStr)
                {
                    pLbCopyArea->SelectEntryPos( i );
                    return;
                }
            }

        }
        pLbCopyArea->SelectEntryPos( 0 );
    }
}

IMPL_LINK_TYPED( ScFilterOptionsMgr, BtnCopyResultHdl, CheckBox&, rBox, void )
{
    if ( &rBox == pBtnCopyResult )
    {
        if ( rBox.IsChecked() )
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
