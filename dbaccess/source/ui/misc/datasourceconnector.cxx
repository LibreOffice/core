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

#ifndef _DBAUI_DATASOURCECONNECTOR_HXX_
#include "datasourceconnector.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
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
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
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
#ifndef COMPHELPER_NAMEDVALUECOLLECTION_HXX
#include <comphelper/namedvaluecollection.hxx>
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
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef SVTOOLS_FILENOTATION_HXX
#include <svl/filenotation.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#include "moduledbu.hxx"

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
    using namespace ::com::sun::star::frame;
    using namespace ::dbtools;
    using ::svt::OFileNotation;

    //=====================================================================
    //= ODatasourceConnector
    //=====================================================================
    //---------------------------------------------------------------------
    ODatasourceConnector::ODatasourceConnector(const Reference< XMultiServiceFactory >& _rxORB, Window* _pMessageParent)
        :m_pErrorMessageParent(_pMessageParent)
        ,m_xORB(_rxORB)
    {
    }

    //---------------------------------------------------------------------
    ODatasourceConnector::ODatasourceConnector( const Reference< XMultiServiceFactory >& _rxORB, Window* _pMessageParent,
        const ::rtl::OUString& _rContextInformation )
        :m_pErrorMessageParent(_pMessageParent)
        ,m_xORB(_rxORB)
        ,m_sContextInformation( _rContextInformation )
    {
    }

    //---------------------------------------------------------------------
    Reference< XConnection > ODatasourceConnector::connect( const ::rtl::OUString& _rDataSourceName,
        ::dbtools::SQLExceptionInfo* _pErrorInfo ) const
    {
        Reference< XConnection > xConnection;

        OSL_ENSURE(isValid(), "ODatasourceConnector::connect: invalid object!");
        if (!isValid())
            return xConnection;

        // get the data source
        Reference< XDataSource > xDatasource(
            getDataSourceByName( _rDataSourceName, m_pErrorMessageParent, m_xORB, _pErrorInfo ),
            UNO_QUERY
        );

        if ( xDatasource.is() )
            xConnection = connect( xDatasource, _pErrorInfo );
        return xConnection;
    }

    //---------------------------------------------------------------------
    Reference< XConnection > ODatasourceConnector::connect(const Reference< XDataSource>& _xDataSource,
        ::dbtools::SQLExceptionInfo* _pErrorInfo ) const
    {
        Reference< XConnection > xConnection;

        OSL_ENSURE( isValid() && _xDataSource.is(), "ODatasourceConnector::connect: invalid object or argument!" );
        if ( !isValid() || !_xDataSource.is() )
            return xConnection;

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
            DBG_UNHANDLED_EXCEPTION();
        }

        // try to connect
        SQLExceptionInfo aInfo;
        try
        {
            if (bPwdRequired && !sPassword.getLength())
            {   // password required, but empty -> connect using an interaction handler
                Reference< XCompletedConnection > xConnectionCompletion( _xDataSource, UNO_QUERY_THROW );

                Reference< XModel > xModel( getDataSourceOrModel( _xDataSource ), UNO_QUERY_THROW );
                ::comphelper::NamedValueCollection aArgs( xModel->getArgs() );
                Reference< XInteractionHandler > xHandler( aArgs.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() ) );

                if ( !xHandler.is() )
                {
                    // instantiate the default SDB interaction handler
                    xHandler = Reference< XInteractionHandler >( m_xORB->createInstance( SERVICE_TASK_INTERACTION_HANDLER ), UNO_QUERY );
                    if ( !xHandler.is() )
                        ShowServiceNotAvailableError(m_pErrorMessageParent, (::rtl::OUString)SERVICE_TASK_INTERACTION_HANDLER, sal_True);
                }

                if ( xHandler.is() )
                {
                    xConnection = xConnectionCompletion->connectWithCompletion(xHandler);
                }
            }
            else
            {
                xConnection = _xDataSource->getConnection(sUser, sPassword);
            }
        }
        catch( const SQLException& )
        {
            aInfo = ::cppu::getCaughtException();
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !aInfo.isValid() )
        {
            // there was no error during connecting, but perhaps a warning?
            Reference< XWarningsSupplier > xConnectionWarnings( xConnection, UNO_QUERY );
            if ( xConnectionWarnings.is() )
            {
                try
                {
                    Any aWarnings( xConnectionWarnings->getWarnings() );
                    if ( aWarnings.hasValue() )
                    {
                        String sMessage( ModuleRes( STR_WARNINGS_DURING_CONNECT ) );
                        sMessage.SearchAndReplaceAscii( "$buttontext$", Button::GetStandardText( BUTTON_MORE ) );
                        sMessage = OutputDevice::GetNonMnemonicString( sMessage );

                        SQLWarning aContext;
                        aContext.Message = sMessage;
                        aContext.NextException = aWarnings;
                        aInfo = aContext;
                    }
                    xConnectionWarnings->clearWarnings();
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
        else
        {
            if ( m_sContextInformation.getLength() )
            {
                SQLException aError;
                aError.Message = m_sContextInformation;
                aError.NextException = aInfo.get();

                aInfo = aError;
            }
        }

        // was there an error?
        if ( aInfo.isValid() )
        {
            if ( _pErrorInfo )
            {
                *_pErrorInfo = aInfo;
            }
            else
            {
                showError( aInfo, m_pErrorMessageParent, m_xORB );
            }
        }
        return xConnection;
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

