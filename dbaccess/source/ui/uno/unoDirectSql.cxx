/*************************************************************************
 *
 *  $RCSfile: unoDirectSql.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:24:12 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

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
    //---------------------------------------------------------------------
    ODirectSQLDialog::ODirectSQLDialog(const Reference< XMultiServiceFactory >& _rxORB)
        :ODirectSQLDialog_BASE( _rxORB )
    {
    }

    //---------------------------------------------------------------------
    ODirectSQLDialog::~ODirectSQLDialog()
    {
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
        Reference< XConnection > xConnection;
        try
        {
            // the connection the row set is working with
            ODatasourceConnector aDSConnector(m_xORB,_pParent);
            xConnection = aDSConnector.connect(m_sInitialSelection);
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ODirectSQLDialog::createDialog: caught an exception!" );
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
                aProperty.Value >>= m_sInitialSelection;
            }
        }
        ODirectSQLDialog_BASE::implInitialize(_rValue);
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................

