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


#undef SC_DLLIMPLEMENTATION




#include <vcl/waitobj.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

using namespace com::sun::star;

#include "dapidata.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "dapitype.hrc"
#include "miscuno.hxx"
#include "dpsdbtab.hxx"         // ScImportSourceDesc



//  entries in the "type" ListBox
#define DP_TYPELIST_TABLE   0
#define DP_TYPELIST_QUERY   1
#define DP_TYPELIST_SQLNAT  3


ScDataPilotDatabaseDlg::ScDataPilotDatabaseDlg( Window* pParent ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DAPIDATA ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aFtDatabase     ( this, ScResId( FT_DATABASE ) ),
    aLbDatabase     ( this, ScResId( LB_DATABASE ) ),
    aFtObject       ( this, ScResId( FT_OBJECT ) ),
    aCbObject       ( this, ScResId( CB_OBJECT ) ),
    aFtType         ( this, ScResId( FT_OBJTYPE ) ),
    aLbType         ( this, ScResId( LB_OBJTYPE ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    FreeResource();

    WaitObject aWait( this );       // initializing the database service the first time takes a while

    try
    {
        //  get database names

        uno::Reference<sdb::XDatabaseContext> xContext = sdb::DatabaseContext::create(
                comphelper::getProcessComponentContext() );
        uno::Sequence<OUString> aNames = xContext->getElementNames();
        long nCount = aNames.getLength();
        const OUString* pArray = aNames.getConstArray();
        for (long nPos = 0; nPos < nCount; nPos++)
        {
            String aName = pArray[nPos];
            aLbDatabase.InsertEntry( aName );
        }
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("exception in database");
    }

    aLbDatabase.SelectEntryPos( 0 );
    aLbType.SelectEntryPos( 0 );

    FillObjects();

    aLbDatabase.SetSelectHdl( LINK( this, ScDataPilotDatabaseDlg, SelectHdl ) );
    aLbType.SetSelectHdl( LINK( this, ScDataPilotDatabaseDlg, SelectHdl ) );
}

ScDataPilotDatabaseDlg::~ScDataPilotDatabaseDlg()
{
}

void ScDataPilotDatabaseDlg::GetValues( ScImportSourceDesc& rDesc )
{
    sal_uInt16 nSelect = aLbType.GetSelectEntryPos();

    rDesc.aDBName = aLbDatabase.GetSelectEntry();
    rDesc.aObject = aCbObject.GetText();

    if (rDesc.aDBName.isEmpty() || rDesc.aObject.isEmpty())
        rDesc.nType = sheet::DataImportMode_NONE;
    else if ( nSelect == DP_TYPELIST_TABLE )
        rDesc.nType = sheet::DataImportMode_TABLE;
    else if ( nSelect == DP_TYPELIST_QUERY )
        rDesc.nType = sheet::DataImportMode_QUERY;
    else
        rDesc.nType = sheet::DataImportMode_SQL;

    rDesc.bNative = ( nSelect == DP_TYPELIST_SQLNAT );
}

IMPL_LINK_NOARG(ScDataPilotDatabaseDlg, SelectHdl)
{
    FillObjects();
    return 0;
}

void ScDataPilotDatabaseDlg::FillObjects()
{
    aCbObject.Clear();

    String aDatabaseName = aLbDatabase.GetSelectEntry();
    if (!aDatabaseName.Len())
        return;

    sal_uInt16 nSelect = aLbType.GetSelectEntryPos();
    if ( nSelect > DP_TYPELIST_QUERY )
        return;                                 // only tables and queries

    try
    {
        //  open connection (for tables or queries)

        uno::Reference<sdb::XDatabaseContext> xContext = sdb::DatabaseContext::create(
                comphelper::getProcessComponentContext() );

        uno::Any aSourceAny = xContext->getByName( aDatabaseName );
        uno::Reference<sdb::XCompletedConnection> xSource(
                ScUnoHelpFunctions::AnyToInterface( aSourceAny ), uno::UNO_QUERY );
        if ( !xSource.is() ) return;

        uno::Reference<task::XInteractionHandler> xHandler(
            task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(), 0),
            uno::UNO_QUERY_THROW);

        uno::Reference<sdbc::XConnection> xConnection = xSource->connectWithCompletion( xHandler );

        uno::Sequence<OUString> aNames;
        if ( nSelect == DP_TYPELIST_TABLE )
        {
            //  get all tables

            uno::Reference<sdbcx::XTablesSupplier> xTablesSupp( xConnection, uno::UNO_QUERY );
            if ( !xTablesSupp.is() ) return;

            uno::Reference<container::XNameAccess> xTables = xTablesSupp->getTables();
            if ( !xTables.is() ) return;

            aNames = xTables->getElementNames();
        }
        else
        {
            //  get all queries

            uno::Reference<sdb::XQueriesSupplier> xQueriesSupp( xConnection, uno::UNO_QUERY );
            if ( !xQueriesSupp.is() ) return;

            uno::Reference<container::XNameAccess> xQueries = xQueriesSupp->getQueries();
            if ( !xQueries.is() ) return;

            aNames = xQueries->getElementNames();
        }

        //  fill list

        long nCount = aNames.getLength();
        const OUString* pArray = aNames.getConstArray();
        for( long nPos=0; nPos<nCount; nPos++ )
        {
            String aName = pArray[nPos];
            aCbObject.InsertEntry( aName );
        }
    }
    catch(uno::Exception&)
    {
        //  this may happen if an invalid database is selected -> no DBG_ERROR
        OSL_FAIL("exception in database");
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
