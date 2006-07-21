/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dapidata.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:20:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include <tools/debug.hxx>
#include <vcl/waitobj.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>

using namespace com::sun::star;

#include "dapidata.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "dapitype.hrc"
#include "miscuno.hxx"
#include "dpsdbtab.hxx"         // ScImportSourceDesc

//-------------------------------------------------------------------------

#define DP_SERVICE_DBCONTEXT        "com.sun.star.sdb.DatabaseContext"
#define SC_SERVICE_INTHANDLER       "com.sun.star.sdb.InteractionHandler"

//  entries in the "type" ListBox
#define DP_TYPELIST_TABLE   0
#define DP_TYPELIST_QUERY   1
#define DP_TYPELIST_SQL     2
#define DP_TYPELIST_SQLNAT  3

//-------------------------------------------------------------------------

ScDataPilotDatabaseDlg::ScDataPilotDatabaseDlg( Window* pParent ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DAPIDATA ) ),
    //
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aFtDatabase     ( this, ScResId( FT_DATABASE ) ),
    aLbDatabase     ( this, ScResId( LB_DATABASE ) ),
    aFtObject       ( this, ScResId( FT_OBJECT ) ),
    aCbObject       ( this, ScResId( CB_OBJECT ) ),
    aFtType         ( this, ScResId( FT_OBJTYPE ) ),
    aLbType         ( this, ScResId( LB_OBJTYPE ) ),
    aFlFrame        ( this, ScResId( FL_FRAME ) )
{
    FreeResource();

    WaitObject aWait( this );       // initializing the database service the first time takes a while

    try
    {
        //  get database names

        uno::Reference<container::XNameAccess> xContext(
                comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString::createFromAscii( DP_SERVICE_DBCONTEXT ) ),
                uno::UNO_QUERY);
        if (xContext.is())
        {
            uno::Sequence<rtl::OUString> aNames = xContext->getElementNames();
            long nCount = aNames.getLength();
            const rtl::OUString* pArray = aNames.getConstArray();
            for (long nPos = 0; nPos < nCount; nPos++)
            {
                String aName = pArray[nPos];
                aLbDatabase.InsertEntry( aName );
            }
        }
    }
    catch(uno::Exception&)
    {
        DBG_ERROR("exception in database");
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
    USHORT nSelect = aLbType.GetSelectEntryPos();

    rDesc.aDBName = aLbDatabase.GetSelectEntry();
    rDesc.aObject = aCbObject.GetText();

    if ( !rDesc.aDBName.Len() || !rDesc.aObject.Len() )
        rDesc.nType = sheet::DataImportMode_NONE;
    else if ( nSelect == DP_TYPELIST_TABLE )
        rDesc.nType = sheet::DataImportMode_TABLE;
    else if ( nSelect == DP_TYPELIST_QUERY )
        rDesc.nType = sheet::DataImportMode_QUERY;
    else
        rDesc.nType = sheet::DataImportMode_SQL;

    rDesc.bNative = ( nSelect == DP_TYPELIST_SQLNAT );
}

IMPL_LINK( ScDataPilotDatabaseDlg, SelectHdl, ListBox*, pLb )
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

    USHORT nSelect = aLbType.GetSelectEntryPos();
    if ( nSelect > DP_TYPELIST_QUERY )
        return;                                 // only tables and queries

    try
    {
        //  open connection (for tables or queries)

        uno::Reference<container::XNameAccess> xContext(
                comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString::createFromAscii( DP_SERVICE_DBCONTEXT ) ),
                uno::UNO_QUERY);
        if ( !xContext.is() ) return;

        uno::Any aSourceAny = xContext->getByName( aDatabaseName );
        uno::Reference<sdb::XCompletedConnection> xSource(
                ScUnoHelpFunctions::AnyToInterface( aSourceAny ), uno::UNO_QUERY );
        if ( !xSource.is() ) return;

        uno::Reference<task::XInteractionHandler> xHandler(
                comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString::createFromAscii( SC_SERVICE_INTHANDLER ) ),
                uno::UNO_QUERY);

        uno::Reference<sdbc::XConnection> xConnection = xSource->connectWithCompletion( xHandler );

        uno::Sequence<rtl::OUString> aNames;
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
        const rtl::OUString* pArray = aNames.getConstArray();
        for( long nPos=0; nPos<nCount; nPos++ )
        {
            String aName = pArray[nPos];
            aCbObject.InsertEntry( aName );
        }
    }
    catch(uno::Exception&)
    {
        //  #71604# this may happen if an invalid database is selected -> no DBG_ERROR
        DBG_WARNING("exception in database");
    }
}




