/*************************************************************************
 *
 *  $RCSfile: dapidata.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-20 09:15:16 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <tools/debug.hxx>
#include <vcl/waitobj.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/data/XDatabaseFavorites.hpp>
#include <com/sun/star/data/XDatabaseEngine.hpp>
#include <com/sun/star/data/XDatabaseWorkspace.hpp>
#include <com/sun/star/data/XDatabase.hpp>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace com::sun::star;

#include "dapidata.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "dapitype.hrc"
#include "dpsdbtab.hxx"         // ScImportSourceDesc

//-------------------------------------------------------------------------

#define DP_SERVICE_DBENGINE         "com.sun.star.data.DatabaseEngine"

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
    aGbFrame        ( this, ScResId( GB_FRAME ) )
{
    FreeResource();

    WaitObject aWait( this );       // initializing the database service the first time takes a while

    try
    {
        //  get database names

        uno::Reference<data::XDatabaseFavorites> xFavorites(
                comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString::createFromAscii( DP_SERVICE_DBENGINE ) ),
                uno::UNO_QUERY);
        if (xFavorites.is())
        {
            uno::Sequence<beans::PropertyValue> aFavorites = xFavorites->getFavorites();
            long nCount = aFavorites.getLength();
            const beans::PropertyValue* pArray = aFavorites.getConstArray();
            for (long nPos = 0; nPos < nCount; nPos++)
            {
                String aName = pArray[nPos].Name;
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
        uno::Reference<data::XDatabaseEngine> xEngine(
                comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString::createFromAscii( DP_SERVICE_DBENGINE ) ),
                uno::UNO_QUERY);
        if ( !xEngine.is() ) return;

        // read default workspace (like in FmFormView::CreateFieldControl)

        uno::Reference<container::XIndexAccess> xWsps( xEngine->getWorkspaces(), uno::UNO_QUERY );
        if ( !xWsps.is() ) return;
        uno::Any aElement( xWsps->getByIndex(0) );
        uno::Reference<data::XDatabaseWorkspace> xWorkspace;
        aElement >>= xWorkspace;

        uno::Reference<data::XDatabase> xDatabase = xWorkspace->open( aDatabaseName );
        uno::Reference<container::XNameAccess> xAccess;
        if ( nSelect == DP_TYPELIST_TABLE )
        {
            //  get all tables
            uno::Reference<data::XDatabaseConnection> xConnection( xDatabase, uno::UNO_QUERY );
            if ( !xConnection.is() ) return;
            xAccess = uno::Reference<container::XNameAccess>( xConnection->getTables(), uno::UNO_QUERY );
        }
        else
        {
            // get all queries
            xAccess = uno::Reference<container::XNameAccess>( xDatabase->getQueries(), uno::UNO_QUERY );
        }
        if( !xAccess.is() ) return;

        //  fill list

        uno::Sequence<rtl::OUString> aSeq = xAccess->getElementNames();
        long nCount = aSeq.getLength();
        const rtl::OUString* pArray = aSeq.getConstArray();
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




