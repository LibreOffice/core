/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "datasourceconnector.hxx"
#include <osl/diagnose.h>
#include "dbustrings.hrc"
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <osl/thread.h>
#include <comphelper/extract.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <connectivity/dbexception.hxx>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include "UITools.hxx"
#include <vcl/stdtext.hxx>
#include <vcl/button.hxx>
#include <svl/filenotation.hxx>
#include <tools/diagnose_ex.h>
#include <cppuhelper/exc_hlp.hxx>
#include "dbu_misc.hrc"
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
