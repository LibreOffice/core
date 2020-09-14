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

#include <osl/diagnose.h>

#include <rangeutl.hxx>
#include <dbdata.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <queryparam.hxx>
#include <globalnames.hxx>

#include <foptmgr.hxx>
#include <formula/funcutl.hxx>

// ScFilterOptionsMgr (.ui's option helper)

ScFilterOptionsMgr::ScFilterOptionsMgr(
                                ScViewData*         ptrViewData,
                                const ScQueryParam& refQueryData,
                                weld::CheckButton* refBtnCase,
                                weld::CheckButton* refBtnRegExp,
                                weld::CheckButton* refBtnHeader,
                                weld::CheckButton* refBtnUnique,
                                weld::CheckButton* refBtnCopyResult,
                                weld::CheckButton* refBtnDestPers,
                                weld::ComboBox* refLbCopyArea,
                                formula::RefEdit* refEdCopyArea,
                                formula::RefButton* refRbCopyArea,
                                weld::Label* refFtDbAreaLabel,
                                weld::Label* refFtDbArea,
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
}

void ScFilterOptionsMgr::Init()
{
//moggi:TODO
    OSL_ENSURE( pViewData && pDoc, "Init failed :-/" );

    pLbCopyArea->connect_changed( LINK( this, ScFilterOptionsMgr, LbAreaSelHdl ) );
    pEdCopyArea->SetModifyHdl  ( LINK( this, ScFilterOptionsMgr, EdAreaModifyHdl ) );
    pBtnCopyResult->connect_toggled( LINK( this, ScFilterOptionsMgr, BtnCopyResultHdl ) );

    pBtnCase->set_active( rQueryData.bCaseSens );
    pBtnHeader->set_active( rQueryData.bHasHeader );
    pBtnRegExp->set_active( rQueryData.eSearchType == utl::SearchParam::SearchType::Regexp );
    pBtnUnique->set_active( !rQueryData.bDuplicate );

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
        OUString theDbArea;
        OUString   theDbName(STR_DB_LOCAL_NONAME);
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        theAreaStr = theCurArea.Format(*pDoc, ScRefFlags::RANGE_ABS_3D, eConv);

        // fill the target area list

        pLbCopyArea->clear();
        pLbCopyArea->append_text(rStrUndefined);

        ScAreaNameIterator aIter( *pDoc );
        OUString aName;
        ScRange aRange;
        while ( aIter.Next( aName, aRange ) )
        {
            OUString aRefStr(aRange.aStart.Format(ScRefFlags::ADDR_ABS_3D, pDoc, eConv));
            pLbCopyArea->append(aRefStr, aName);
        }

        pBtnDestPers->set_active(true);         // always on when called
        pLbCopyArea->set_active( 0 );
        pEdCopyArea->SetText( EMPTY_OUSTRING );

        /*
         * Check whether the transferred area is a database area:
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
                pBtnHeader->set_active( pDBData->HasHeader() );
                theDbName = pDBData->GetName();

                pBtnHeader->set_sensitive(theDbName == STR_DB_LOCAL_NONAME);
            }
        }

        if ( theDbName != STR_DB_LOCAL_NONAME )
        {
            theDbArea += " (" + theDbName + ")";

            pFtDbArea->set_label( theDbArea );
        }
        else
        {
            pFtDbAreaLabel->set_label( OUString() );
            pFtDbArea->set_label( OUString() );
        }

        // position to copy to:

        if ( !rQueryData.bInplace )
        {
            OUString aString =
                ScAddress( rQueryData.nDestCol,
                           rQueryData.nDestRow,
                           rQueryData.nDestTab
                         ).Format(ScRefFlags::ADDR_ABS_3D, pDoc, eConv);

            pBtnCopyResult->set_active(true);
            pEdCopyArea->SetText( aString );
            EdAreaModifyHdl( *pEdCopyArea );
            pLbCopyArea->set_sensitive(true);
            pEdCopyArea->GetWidget()->set_sensitive(true);
            pRbCopyArea->GetWidget()->set_sensitive(true);
            pBtnDestPers->set_sensitive(true);
        }
        else
        {
            pBtnCopyResult->set_active( false );
            pEdCopyArea->SetText( EMPTY_OUSTRING );
            pLbCopyArea->set_sensitive(false);
            pEdCopyArea->GetWidget()->set_sensitive(false);
            pRbCopyArea->GetWidget()->set_sensitive(false);
            pBtnDestPers->set_sensitive(false);
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

IMPL_LINK( ScFilterOptionsMgr, LbAreaSelHdl, weld::ComboBox&, rLb, void )
{
    if ( &rLb == pLbCopyArea )
    {
        OUString aString;
        const sal_Int32 nSelPos = pLbCopyArea->get_active();

        if ( nSelPos > 0 )
            aString = pLbCopyArea->get_id(nSelPos);

        pEdCopyArea->SetText( aString );
    }
}

IMPL_LINK( ScFilterOptionsMgr, EdAreaModifyHdl, formula::RefEdit&, rEd, void )
{
    if ( &rEd != pEdCopyArea )
        return;

    OUString  theCurPosStr = rEd.GetText();
    ScRefFlags  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

    if ( (nResult & ScRefFlags::VALID) == ScRefFlags::VALID)
    {
        const sal_Int32 nCount = pLbCopyArea->get_count();

        for ( sal_Int32 i=2; i<nCount; ++i )
        {
            OUString aStr = pLbCopyArea->get_id(i);
            if (theCurPosStr == aStr)
            {
                pLbCopyArea->set_active( i );
                return;
            }
        }

    }
    pLbCopyArea->set_active( 0 );
}

IMPL_LINK( ScFilterOptionsMgr, BtnCopyResultHdl, weld::ToggleButton&, rBox, void )
{
    if ( &rBox != pBtnCopyResult )
        return;

    if ( rBox.get_active() )
    {
        pBtnDestPers->set_sensitive(true);
        pLbCopyArea->set_sensitive(true);
        pEdCopyArea->GetWidget()->set_sensitive(true);
        pRbCopyArea->GetWidget()->set_sensitive(true);
        pEdCopyArea->GrabFocus();
    }
    else
    {
        pBtnDestPers->set_sensitive(false);
        pLbCopyArea->set_sensitive(false);
        pEdCopyArea->GetWidget()->set_sensitive(false);
        pRbCopyArea->GetWidget()->set_sensitive(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
