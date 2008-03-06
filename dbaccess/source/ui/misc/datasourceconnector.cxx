/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datasourceconnector.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:28:54 $
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
#include "precompiled_dbaccess.hxx"

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
#include <svtools/filenotation.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
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
    Reference< XConnection > ODatasourceConnector::connect(const ::rtl::OUString& _rDataSourceName, sal_Bool _bShowError) const
    {
        Reference< XConnection > xConnection;

        OSL_ENSURE(isValid(), "ODatasourceConnector::connect: invalid object!");
        if (!isValid())
            return xConnection;

        // get the data source
        Reference< XDataSource > xDatasource(
            getDataSourceByName_displayError( _rDataSourceName, m_pErrorMessageParent, m_xORB, _bShowError ),
            UNO_QUERY
        );
        if ( xDatasource.is() )
            xConnection = connect( xDatasource, _bShowError );
        return xConnection;
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
                Reference< XCompletedConnection > xConnectionCompletion( _xDataSource, UNO_QUERY_THROW );

                Reference< XModel > xModel( getDataSourceOrModel( _xDataSource ), UNO_QUERY_THROW );
                ::comphelper::NamedValueCollection aArgs( xModel->getArgs() );
                Reference< XInteractionHandler > xHandler( aArgs.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() ) );

                if ( !xHandler.is() )
                {
                    // instantiate the default SDB interaction handler
                    xHandler = Reference< XInteractionHandler >( m_xORB->createInstance( SERVICE_SDB_INTERACTION_HANDLER ), UNO_QUERY );
                    if ( !xHandler.is() )
                        ShowServiceNotAvailableError(m_pErrorMessageParent, String(SERVICE_SDB_INTERACTION_HANDLER), sal_True);
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
        catch(SQLContext& e) { aInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aInfo = SQLExceptionInfo(e); }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !_bShowError )
            return xConnection;

        // was there and error?
        if ( aInfo.isValid() )
        {
            if ( m_sContextInformation.getLength() )
            {
                SQLException aError;
                aError.Message = m_sContextInformation;
                aError.NextException = aInfo.get();

                aInfo = aError;
            }

            showError(aInfo, m_pErrorMessageParent, m_xORB);
            return xConnection;
        }

        // was there a warning?
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

                    SQLContext aContext;
                    aContext.Message = sMessage;
                    aContext.NextException = aWarnings;
                    showError( SQLExceptionInfo( aContext ), m_pErrorMessageParent, m_xORB );
                }
                xConnectionWarnings->clearWarnings();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        return xConnection;
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

