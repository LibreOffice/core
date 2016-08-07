/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"

#ifndef DBAUI_UNODIRECTSQL_HXX
#include "unoDirectSql.hxx"
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONNECTION_XCONNECTION_HPP_
#include <com/sun/star/connection/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _DBACCESS_UI_DIRECTSQL_HXX_
#include "directsql.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBAUI_DATASOURCECONNECTOR_HXX_
#include "datasourceconnector.hxx"
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif


extern "C" void SAL_CALL createRegistryInfo_ODirectSQLDialog()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::ODirectSQLDialog > aAutoRegistration;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;

    //=====================================================================
    //= ODirectSQLDialog
    //=====================================================================
DBG_NAME(ODirectSQLDialog)
//---------------------------------------------------------------------
    ODirectSQLDialog::ODirectSQLDialog(const Reference< XMultiServiceFactory >& _rxORB)
        :ODirectSQLDialog_BASE( _rxORB )
    {
        DBG_CTOR(ODirectSQLDialog,NULL);

    }

    //---------------------------------------------------------------------
    ODirectSQLDialog::~ODirectSQLDialog()
    {

        DBG_DTOR(ODirectSQLDialog,NULL);
    }

    //---------------------------------------------------------------------
    IMPLEMENT_IMPLEMENTATION_ID( ODirectSQLDialog )

    //---------------------------------------------------------------------
    IMPLEMENT_SERVICE_INFO1_STATIC( ODirectSQLDialog, "com.sun.star.comp.sdb.DirectSQLDialog", SERVICE_SDB_DIRECTSQLDIALOG )

    //---------------------------------------------------------------------
    IMPLEMENT_PROPERTYCONTAINER_DEFAULTS( ODirectSQLDialog )

    //---------------------------------------------------------------------
    Dialog* ODirectSQLDialog::createDialog(Window* _pParent)
    {
        // obtain all the objects needed for the dialog
        Reference< XConnection > xConnection = m_xActiveConnection;
        if ( !xConnection.is() )
        {
            try
            {
                // the connection the row set is working with
                ODatasourceConnector aDSConnector(m_aContext.getLegacyServiceFactory(),_pParent);
                xConnection = aDSConnector.connect( m_sInitialSelection, NULL );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        if ( !xConnection.is() )
            // can't create the dialog if I have improper settings
            return NULL;

        return new DirectSQLDialog( _pParent, xConnection);
    }
    //---------------------------------------------------------------------
    void ODirectSQLDialog::implInitialize(const Any& _rValue)
    {
        PropertyValue aProperty;
        if (_rValue >>= aProperty)
        {
            if (0 == aProperty.Name.compareToAscii("InitialSelection"))
            {
                OSL_VERIFY( aProperty.Value >>= m_sInitialSelection );
                return;
            }
            else if (0 == aProperty.Name.compareToAscii("ActiveConnection"))
            {
                m_xActiveConnection.set( aProperty.Value, UNO_QUERY );
                OSL_ENSURE( m_xActiveConnection.is(), "ODirectSQLDialog::implInitialize: invalid connection!" );
                return;
            }
        }
        ODirectSQLDialog_BASE::implInitialize(_rValue);
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................

