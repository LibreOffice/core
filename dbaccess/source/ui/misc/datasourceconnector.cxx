/*************************************************************************
 *
 *  $RCSfile: datasourceconnector.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:08:51 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_DATASOURCECONNECTOR_HXX_
#include "datasourceconnector.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;

    //=====================================================================
    //= ODatasourceConnector
    //=====================================================================
    //---------------------------------------------------------------------
    ODatasourceConnector::ODatasourceConnector(const Reference< XMultiServiceFactory >& _rxORB, Window* _pMessageParent)
        :m_xORB(_rxORB)
        ,m_pErrorMessageParent(_pMessageParent)
    {
        implConstruct();
    }

    //---------------------------------------------------------------------
    ODatasourceConnector::ODatasourceConnector( const Reference< XMultiServiceFactory >& _rxORB, Window* _pMessageParent,
        const ::rtl::OUString& _rContextInformation, const ::rtl::OUString& _rContextDetails )
        :m_xORB(_rxORB)
        ,m_pErrorMessageParent(_pMessageParent)
        ,m_sContextInformation( _rContextInformation )
        ,m_sContextDetails( _rContextDetails )
    {
        implConstruct();
    }

    //---------------------------------------------------------------------
    void ODatasourceConnector::implConstruct()
    {
        OSL_ENSURE(m_xORB.is(), "ODatasourceConnector::implConstruct: invalid ORB!");
        if (m_xORB.is())
        {
            try
            {
                Reference< XInterface > xContext = m_xORB->createInstance(SERVICE_SDB_DATABASECONTEXT);
                OSL_ENSURE(xContext.is(), "ODatasourceConnector::implConstruct: got no data source context!");
                m_xDatabaseContext.set(xContext,UNO_QUERY);
                OSL_ENSURE(m_xDatabaseContext.is() || !xContext.is(), "ODatasourceConnector::ODatasourceConnector: missing the XNameAccess interface on the data source context!");
            }
            catch(const Exception&)
            {
                OSL_ENSURE(sal_False, "ODatasourceConnector::implConstruct: caught an exception while creating the data source context!");
            }
        }
    }

    //---------------------------------------------------------------------
    Reference< XConnection > ODatasourceConnector::connect(const ::rtl::OUString& _rDataSourceName, sal_Bool _bShowError) const
    {
        Reference< XConnection > xConnection;

        OSL_ENSURE(isValid(), "ODatasourceConnector::connect: invalid object!");
        if (!isValid())
            return xConnection;

        // get the data source
        Reference< XDataSource > xDatasource;
        try
        {
            m_xDatabaseContext->getByName(_rDataSourceName) >>= xDatasource;
        }
        catch(Exception&)
        {
        }
        return connect(xDatasource,_bShowError);
    }
    //---------------------------------------------------------------------
    Reference< XConnection > ODatasourceConnector::connect(const Reference< XDataSource>& _xDataSource, sal_Bool _bShowError) const
    {
        Reference< XConnection > xConnection;

        OSL_ENSURE(isValid(), "ODatasourceConnector::connect: invalid object!");
        if (!isValid())
            return xConnection;

        if (!_xDataSource.is())
        {
            OSL_ENSURE(sal_False,   "ODatasourceConnector::connect: could not retrieve the data source!");
            return xConnection;
        }

        // get user/password
        ::rtl::OUString sPassword, sUser;
        sal_Bool bPwdRequired = sal_False;
        Reference<XPropertySet> xProp(_xDataSource,UNO_QUERY);
        try
        {
            xProp->getPropertyValue(PROPERTY_PASSWORD) >>= sPassword;
            xProp->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED) >>= bPwdRequired;
            xProp->getPropertyValue(PROPERTY_USER) >>= sUser;
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "ODatasourceConnector::connect: error while retrieving data source properties!");
        }

        // try to connect
        SQLExceptionInfo aInfo;
        try
        {
            if (bPwdRequired && !sPassword.getLength())
            {   // password required, but empty -> connect using an interaction handler
                Reference< XCompletedConnection > xConnectionCompletion(_xDataSource, UNO_QUERY);
                if (!xConnectionCompletion.is())
                {
                    OSL_ENSURE(sal_False, "ODatasourceConnector::connect: missing an interface ... need an error message here!");
                }
                else
                {   // instantiate the default SDB interaction handler
                    Reference< XInteractionHandler > xHandler(m_xORB->createInstance(SERVICE_SDB_INTERACTION_HANDLER), UNO_QUERY);
                    if (!xHandler.is())
                    {
                        ShowServiceNotAvailableError(m_pErrorMessageParent, String(SERVICE_SDB_INTERACTION_HANDLER), sal_True);
                    }
                    else
                    {
                        xConnection = xConnectionCompletion->connectWithCompletion(xHandler);
                    }
                }
            }
            else
            {
                xConnection = _xDataSource->getConnection(sUser, sPassword);
            }
        }
        catch(SQLContext& e) { aInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aInfo = SQLExceptionInfo(e); }
        catch(Exception&) { OSL_ENSURE(sal_False, "SbaTableQueryBrowser::OnExpandEntry: could not connect - unknown exception!"); }

        // display the error (if any)
        if ( _bShowError && aInfo.isValid() )
        {
            if ( m_sContextInformation.getLength() )
            {
                SQLContext aContext;

                aContext.Message = m_sContextInformation;
                aContext.Details = m_sContextDetails;
                aContext.NextException = aInfo.get();

                aInfo = aContext;
            }

            showError(aInfo, m_pErrorMessageParent, m_xORB);
        }

        return xConnection;
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

