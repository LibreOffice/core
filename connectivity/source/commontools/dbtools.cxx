/*************************************************************************
 *
 *  $RCSfile: dbtools.cxx,v $
 *
 *  $Revision: 1.51 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:52:59 $
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

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _CONNECTIVITY_CONNCLEANUP_HXX_
#include <connectivity/conncleanup.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERMANAGER_HPP_
#include <com/sun/star/sdbc/XDriverManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XPARAMETERSSUPPLIER_HPP_
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_PARAMETERSREQUEST_HPP_
#include <com/sun/star/sdb/ParametersRequest.hpp>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XINTERACTIONSUPPLYPARAMETERS_HPP_
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWSETVETOEXCEPTION_HPP_
#include <com/sun/star/sdb/RowSetVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPARAMETERS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
#endif
#include <algorithm>

using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::form;
using namespace connectivity;

//.........................................................................
namespace dbtools
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::task;
//  using namespace cppu;
//  using namespace osl;

//==============================================================================
//==============================================================================
sal_Int32 getDefaultNumberFormat(const Reference< XPropertySet >& _xColumn,
                                 const Reference< XNumberFormatTypes >& _xTypes,
                                 const Locale& _rLocale)
{
    OSL_ENSURE(_xTypes.is() && _xColumn.is(), "dbtools::getDefaultNumberFormat: invalid arg !");
    if (!_xTypes.is() || !_xColumn.is())
        return NumberFormat::UNDEFINED;

    sal_Int32 nDataType;
    sal_Int32 nScale = 0;
    try
    {
        // determine the datatype of the column
        _xColumn->getPropertyValue(::rtl::OUString::createFromAscii("Type")) >>= nDataType;

        if (DataType::NUMERIC == nDataType || DataType::DECIMAL == nDataType)
            _xColumn->getPropertyValue(::rtl::OUString::createFromAscii("Scale")) >>= nScale;
    }
    catch (Exception&)
    {
        return NumberFormat::UNDEFINED;
    }
    return getDefaultNumberFormat(nDataType,
                    nScale,
                    ::cppu::any2bool(_xColumn->getPropertyValue(::rtl::OUString::createFromAscii("IsCurrency"))),
                    _xTypes,
                    _rLocale);
}

//------------------------------------------------------------------
sal_Int32 getDefaultNumberFormat(sal_Int32 _nDataType,
                                 sal_Int32 _nScale,
                                 sal_Bool _bIsCurrency,
                                 const Reference< XNumberFormatTypes >& _xTypes,
                                 const Locale& _rLocale)
{
    OSL_ENSURE(_xTypes.is() , "dbtools::getDefaultNumberFormat: invalid arg !");
    if (!_xTypes.is())
        return NumberFormat::UNDEFINED;

    sal_Int32 nFormat = 0;
    sal_Int32 nNumberType   = _bIsCurrency ? NumberFormat::CURRENCY : NumberFormat::NUMBER;
    switch (_nDataType)
    {
        case DataType::BIT:
            nFormat = _xTypes->getStandardFormat(NumberFormat::LOGICAL, _rLocale);
            break;
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
        case DataType::BIGINT:
        case DataType::FLOAT:
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::NUMERIC:
        case DataType::DECIMAL:
        {
            try
            {
                nFormat = _xTypes->getStandardFormat((sal_Int16)nNumberType, _rLocale);
                if(_nScale > 0)
                {
                    // generate a new format if necessary
                    Reference< XNumberFormats > xFormats(_xTypes, UNO_QUERY);
                    ::rtl::OUString sNewFormat = xFormats->generateFormat( 0L, _rLocale, sal_False, sal_False, (sal_Int16)_nScale, sal_True);

                    // and add it to the formatter if necessary
                    nFormat = xFormats->queryKey(sNewFormat, _rLocale, sal_False);
                    if (nFormat == (sal_Int32)-1)
                        nFormat = xFormats->addNew(sNewFormat, _rLocale);
                }
            }
            catch (Exception&)
            {
                nFormat = _xTypes->getStandardFormat((sal_Int16)nNumberType, _rLocale);
            }
        }   break;
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            nFormat = _xTypes->getStandardFormat(NumberFormat::TEXT, _rLocale);
            break;
        case DataType::DATE:
            nFormat = _xTypes->getStandardFormat(NumberFormat::DATE, _rLocale);
            break;
        case DataType::TIME:
            nFormat = _xTypes->getStandardFormat(NumberFormat::TIME, _rLocale);
            break;
        case DataType::TIMESTAMP:
            nFormat = _xTypes->getStandardFormat(NumberFormat::DATETIME, _rLocale);
            break;
        case DataType::BINARY:
        case DataType::VARBINARY:
        case DataType::LONGVARBINARY:
        case DataType::SQLNULL:
        case DataType::OTHER:
        case DataType::OBJECT:
        case DataType::DISTINCT:
        case DataType::STRUCT:
        case DataType::ARRAY:
        case DataType::BLOB:
        case DataType::CLOB:
        case DataType::REF:
        default:
            nFormat = NumberFormat::UNDEFINED;
    }
    return nFormat;
}

//==============================================================================
//------------------------------------------------------------------------------
Reference< XConnection> findConnection(const Reference< XInterface >& xParent)
{
    Reference< XConnection> xConnection(xParent, UNO_QUERY);
    if (!xConnection.is())
    {
        Reference< XChild> xChild(xParent, UNO_QUERY);
        if (xChild.is())
            xConnection = findConnection(xChild->getParent());
    }
    return xConnection;
}

//------------------------------------------------------------------------------
Reference< XDataSource> getDataSource(
            const ::rtl::OUString& _rsTitleOrPath,
            const Reference< XMultiServiceFactory>& _rxFactory)
{
    OSL_ENSURE(_rsTitleOrPath.getLength(), "::getDataSource : invalid arg !");

    Reference< XDataSource>  xDatasource;

    // is it a favorite title ?
    Reference< XNameAccess> xDatabaseContext(
        _rxFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseContext")),UNO_QUERY);

    if ( xDatabaseContext.is() )
    {
        try
        {
            xDatabaseContext->getByName(_rsTitleOrPath) >>= xDatasource;
        }
        catch(Exception)
        {} // no assertion needed
    }

    return xDatasource;
}

//------------------------------------------------------------------------------
Reference< XConnection > getConnection_allowException(
            const ::rtl::OUString& _rsTitleOrPath,
            const ::rtl::OUString& _rsUser,
            const ::rtl::OUString& _rsPwd,
            const Reference< XMultiServiceFactory>& _rxFactory)
{
    Reference< XDataSource> xDataSource( getDataSource(_rsTitleOrPath, _rxFactory) );
    Reference<XConnection> xConnection;
    if (xDataSource.is())
    {
        // do it with interaction handler
        if(!_rsUser.getLength() || !_rsPwd.getLength())
        {
            Reference<XPropertySet> xProp(xDataSource,UNO_QUERY);
            ::rtl::OUString sPwd, sUser;
            sal_Bool bPwdReq = sal_False;
            try
            {
                xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPwd;
                bPwdReq = ::cppu::any2bool(xProp->getPropertyValue(::rtl::OUString::createFromAscii("IsPasswordRequired")));
                xProp->getPropertyValue(::rtl::OUString::createFromAscii("User")) >>= sUser;
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "dbtools::getConnection: error while retrieving data source properties!");
            }
            if(bPwdReq && !sPwd.getLength())
            {   // password required, but empty -> connect using an interaction handler
                Reference<XCompletedConnection> xConnectionCompletion(xProp, UNO_QUERY);
                if (xConnectionCompletion.is())
                {   // instantiate the default SDB interaction handler
                    Reference< XInteractionHandler > xHandler(_rxFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdb.InteractionHandler")), UNO_QUERY);
                    OSL_ENSURE(xHandler.is(), "dbtools::getConnection service com.sun.star.sdb.InteractionHandler not available!");
                    if (xHandler.is())
                    {
                        xConnection = xConnectionCompletion->connectWithCompletion(xHandler);
                    }
                }
            }
            else
                xConnection = xDataSource->getConnection(sUser, sPwd);
        }
        if(!xConnection.is()) // try to get one if not already have one, just to make sure
            xConnection = xDataSource->getConnection(_rsUser, _rsPwd);
    }
    return xConnection;
}

//------------------------------------------------------------------------------
Reference< XConnection> getConnection_withFeedback(const ::rtl::OUString& _rDataSourceName,
        const ::rtl::OUString& _rUser, const ::rtl::OUString& _rPwd, const Reference< XMultiServiceFactory>& _rxFactory)
    SAL_THROW ( (SQLException) )
{
    Reference< XConnection > xReturn;
    try
    {
        xReturn = getConnection_allowException(_rDataSourceName, _rUser, _rPwd, _rxFactory);
    }
    catch(SQLException&)
    {
        // allowed to pass
        throw;
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "::dbtools::getConnection_withFeedback: unexpected (non-SQL) exception caught!");
    }
    return xReturn;
}

//------------------------------------------------------------------------------
Reference< XConnection> getConnection(
            const ::rtl::OUString& _rsTitleOrPath,
            const ::rtl::OUString& _rsUser,
            const ::rtl::OUString& _rsPwd,
            const Reference< XMultiServiceFactory>& _rxFactory)
{
    Reference< XConnection > xReturn;
    try
    {
        xReturn = getConnection_allowException(_rsTitleOrPath, _rsUser, _rsPwd, _rxFactory);
    }
    catch(Exception&)
    {
    }

    // TODO: if there were not dozens of places which rely on getConnection not throwing an exception ....
    // I would change this ...

    return xReturn;
}

//------------------------------------------------------------------------------
Reference< XConnection> getConnection(const Reference< XRowSet>& _rxRowSet) throw (RuntimeException)
{
    Reference< XConnection> xReturn;
    Reference< XPropertySet> xRowSetProps(_rxRowSet, UNO_QUERY);
    if (xRowSetProps.is())
        xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection")) >>= xReturn;
    return xReturn;
}

//------------------------------------------------------------------------------
Reference< XConnection> calcConnection(
            const Reference< XRowSet>& _rxRowSet,
            const Reference< XMultiServiceFactory>& _rxFactory)
            throw (SQLException, RuntimeException)
{
    return connectRowset( _rxRowSet, _rxFactory, sal_True );
}

//------------------------------------------------------------------------------
Reference< XConnection> connectRowset(const Reference< XRowSet>& _rxRowSet, const Reference< XMultiServiceFactory>& _rxFactory,
    sal_Bool _bSetAsActiveConnection )  SAL_THROW ( (SQLException, RuntimeException) )
{
    Reference< XConnection> xReturn;
    Reference< XPropertySet> xRowSetProps(_rxRowSet, UNO_QUERY);
    if (xRowSetProps.is())
    {
        Any aConn( xRowSetProps->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ActiveConnection"))) );
        aConn >>= xReturn;

        if (!xReturn.is())
        {
            // first look if there is a connection in the parent hierarchy
            xReturn = findConnection(_rxRowSet);
            if (!xReturn.is())
            {
                static const ::rtl::OUString s_sUserProp = ::rtl::OUString::createFromAscii("User");
                // the row set didn't supply a connection -> build one with it's current settings
                ::rtl::OUString sDataSourceName;
                xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("DataSourceName")) >>= sDataSourceName;
                ::rtl::OUString sURL;
                xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("URL")) >>= sURL;
                if (sDataSourceName.getLength())
                {   // the row set's data source property is set
                    // -> try to connect, get user and pwd setting for that
                    ::rtl::OUString sUser, sPwd;

                    if (hasProperty(s_sUserProp, xRowSetProps))
                        xRowSetProps->getPropertyValue(s_sUserProp) >>= sUser;
                    if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD), xRowSetProps))
                        xRowSetProps->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPwd;
                    xReturn = getConnection_allowException(sDataSourceName, sUser, sPwd, _rxFactory);
                }
                else if (sURL.getLength())
                {   // the row set has no data source, but a connection url set
                    // -> try to connection with that url
                    Reference< XDriverManager > xDriverManager(
                        _rxFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.sdbc.DriverManager")), UNO_QUERY);
                    if (xDriverManager.is())
                    {
                        ::rtl::OUString sUser, sPwd;
                        if (hasProperty(s_sUserProp, xRowSetProps))
                            xRowSetProps->getPropertyValue(s_sUserProp) >>= sUser;
                        if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD), xRowSetProps))
                            xRowSetProps->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPwd;
                        if (sUser.getLength())
                        {   // use user and pwd together with the url
                            Sequence< PropertyValue> aInfo(2);
                            aInfo.getArray()[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user"));
                            aInfo.getArray()[0].Value <<= sUser;
                            aInfo.getArray()[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("password"));
                            aInfo.getArray()[1].Value <<= sPwd;
                            xReturn = xDriverManager->getConnectionWithInfo(sURL, aInfo);
                        }
                        else
                            // just use the url
                            xReturn = xDriverManager->getConnection(sURL);
                    }
                }
            }

            // now if we got a connection, forward it to the row set
            // (The row set will take ownership of that conn and use it for the next execute.
            // If one of the properties affecting the connection (DataSource, URL) is set afterwards,
            // it will free our connection and build a new one with the new parameters on the next execute.
            // At least the service descriptions says so :)
            if (xReturn.is() && _bSetAsActiveConnection)
            {
                try
                {
                    OAutoConnectionDisposer* pAutoDispose = new OAutoConnectionDisposer(_rxRowSet, xReturn);
                    Reference< XPropertyChangeListener > xEnsureDelete(pAutoDispose);
                }
                catch(Exception&)
                {
                    OSL_ENSURE(0,"EXception when we set the new active connection!");
                }
            }
        }
    }

    return xReturn;
}

//------------------------------------------------------------------------------
Reference< XNameAccess> getTableFields(const Reference< XConnection>& _rxConn,const ::rtl::OUString& _rName)
{
    Reference< XComponent > xDummy;
    return getFieldsByCommandDescriptor( _rxConn, CommandType::TABLE, _rName, xDummy );
}

//------------------------------------------------------------------------------
namespace
{
    enum FieldLookupState
    {
        HANDLE_TABLE, HANDLE_QUERY, HANDLE_SQL, RETRIEVE_OBJECT, RETRIEVE_COLUMNS, DONE, FAILED
    };
}

//------------------------------------------------------------------------------
Reference< XNameAccess > getFieldsByCommandDescriptor( const Reference< XConnection >& _rxConnection,
    const sal_Int32 _nCommandType, const ::rtl::OUString& _rCommand,
    Reference< XComponent >& _rxKeepFieldsAlive, SQLExceptionInfo* _pErrorInfo ) SAL_THROW( ( ) )
{
    OSL_PRECOND( _rxConnection.is(), "::dbtools::getFieldsByCommandDescriptor: invalid connection!" );
    OSL_PRECOND( ( CommandType::TABLE == _nCommandType ) || ( CommandType::QUERY == _nCommandType ) || ( CommandType::COMMAND == _nCommandType ),
        "::dbtools::getFieldsByCommandDescriptor: invalid command type!" );
    OSL_PRECOND( _rCommand.getLength(), "::dbtools::getFieldsByCommandDescriptor: invalid command (empty)!" );

    Reference< XNameAccess > xFields;

    // reset the error
    if ( _pErrorInfo )
        *_pErrorInfo = SQLExceptionInfo();
    // reset the ownership holder
    _rxKeepFieldsAlive.clear();

    // go for the fields
    try
    {
        // some kind of state machine to ease the sharing of code
        FieldLookupState eState = FAILED;
        switch ( _nCommandType )
        {
            case CommandType::TABLE:
                eState = HANDLE_TABLE;
                break;
            case CommandType::QUERY:
                eState = HANDLE_QUERY;
                break;
            case CommandType::COMMAND:
                eState = HANDLE_SQL;
                break;
        }

        // needed in various states:
        Reference< XNameAccess > xObjectCollection;
        Reference< XColumnsSupplier > xSupplyColumns;

        // go!
        while ( ( DONE != eState ) && ( FAILED != eState ) )
        {
            switch ( eState )
            {
                case HANDLE_TABLE:
                {
                    // initial state for handling the tables

                    // get the table objects
                    Reference< XTablesSupplier > xSupplyTables( _rxConnection, UNO_QUERY );
                    if ( xSupplyTables.is() )
                        xObjectCollection = xSupplyTables->getTables();
                    // if something went wrong 'til here, then this will be handled in the next state

                    // next state: get the object
                    eState = RETRIEVE_OBJECT;
                }
                break;

                case HANDLE_QUERY:
                {
                    // initial state for handling the tables

                    // get the table objects
                    Reference< XQueriesSupplier > xSupplyQueries( _rxConnection, UNO_QUERY );
                    if ( xSupplyQueries.is() )
                        xObjectCollection = xSupplyQueries->getQueries();
                    // if something went wrong 'til here, then this will be handled in the next state

                    // next state: get the object
                    eState = RETRIEVE_OBJECT;
                }
                break;

                case RETRIEVE_OBJECT:
                    // here we should have an object (aka query or table) collection, and are going
                    // to retrieve the desired object

                    // next state: default to FAILED
                    eState = FAILED;

                    OSL_ENSURE( xObjectCollection.is(), "::dbtools::getFieldsByCommandDescriptor: invalid connection (no sdb.Connection, or no Tables-/QueriesSupplier)!");
                    if ( xObjectCollection.is() )
                    {
                        if ( xObjectCollection.is() && xObjectCollection->hasByName( _rCommand ) )
                        {
                            xObjectCollection->getByName( _rCommand ) >>= xSupplyColumns;
                                // (xSupplyColumns being NULL will be handled in the next state)

                            // next: go for the columns
                            eState = RETRIEVE_COLUMNS;
                        }
                    }
                    break;

                case RETRIEVE_COLUMNS:
                    OSL_ENSURE( xSupplyColumns.is(), "::dbtools::getFieldsByCommandDescriptor: could not retrieve the columns supplier!" );

                    // next state: default to FAILED
                    eState = FAILED;

                    if ( xSupplyColumns.is() )
                    {
                        xFields = xSupplyColumns->getColumns();
                        // that's it
                        eState = DONE;
                    }
                    break;

                case HANDLE_SQL:
                {
                    ::rtl::OUString sStatementToExecute( _rCommand );

                    // well, the main problem here is to handle statements which contain a parameter
                    // If we would simply execute a parametrized statement, then this will fail because
                    // we cannot supply any parameter values.
                    // Thus, we try to analyze the statement, and to append a WHERE 0=1 filter criterion
                    // This should cause every driver to not really execute the statement, but to return
                    // an empty result set with the proper structure. We then can use this result set
                    // to retrieve the columns.

                    try
                    {
                        Reference< XSQLQueryComposerFactory > xComposerFac( _rxConnection, UNO_QUERY );
                        Reference< XSQLQueryComposer > xComposer;
                        if ( xComposerFac.is() )
                            xComposer = xComposerFac->createQueryComposer( );
                        if ( xComposer.is() )
                        {
                            xComposer->setQuery( sStatementToExecute );

                            // Now set the filter to a dummy restriction which will result in an empty
                            // result set.

                            // Unfortunately, if the statement already has a non-empty filter it is not
                            // removed when setting a new one. Instead, the statement set with "setQuery",
                            // acts as basis, everything added later (setFilter/setOrder and such) is
                            // _added_. So we need to strip the original WHERE clause (if there is one)
                            // manually
                            {
                                ::rtl::OUString sComplete = xComposer->getComposedQuery( );
                                    // we norm it: now there's really a "WHERE", not only a "where" or such ...

                                sal_Int32 nWherePos = sComplete.lastIndexOf( ::rtl::OUString::createFromAscii( "WHERE" ) );
                                if ( -1 < nWherePos )
                                {   // there indeed already is a where clause
                                    sComplete = sComplete.copy( 0, nWherePos );
                                        // this is not correct. The "WHERE" may have been a part of e.g. a filter itself
                                        // (something like "WHERE <field> = 'WHERE'"), but without an API
                                        // for _analyzing_ (and not only _composing_) queries, we don't have
                                        // much of a chance ...
                                    try
                                    {
                                        xComposer->setQuery( sComplete );
                                    }
                                    catch( const Exception& )
                                    {
                                        // just in case we found the wrong WHERE substring ....
                                    }
                                }
                            }

                            xComposer->setFilter( ::rtl::OUString::createFromAscii( "0=1" ) );
                            sStatementToExecute = xComposer->getComposedQuery( );
                        }
                    }
                    catch( const Exception& )
                    {
                        // silent this error, this was just a try. If we're here, we did not change sStatementToExecute,
                        // so it will still be _rCommand, which then will be executed without being touched
                    }

                    // now execute
                    Reference< XPreparedStatement > xStatement = _rxConnection->prepareStatement( sStatementToExecute );
                    // transfer ownership of this temporary object to the caller
                    _rxKeepFieldsAlive = _rxKeepFieldsAlive.query( xStatement );

                    // set the "MaxRows" to 0. This is just in case our attempt to append a 0=1 filter
                    // failed - in this case, the MaxRows restriction should at least ensure that there
                    // is no data returned (which would be potentially expensive)
                    Reference< XPropertySet > xStatementProps( xStatement,UNO_QUERY );
                    try
                    {
                        if ( xStatementProps.is() )
                            xStatementProps->setPropertyValue(
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxRows" ) ),
                                makeAny( sal_Int32( 0 ) )
                            );
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( sal_False, "::dbtools::getFieldsByCommandDescriptor: could not set the MaxRows!" );
                        // oh damn. Not much of a chance to recover, we will no retrieve the complete
                        // full blown result set
                    }

                    xSupplyColumns = xSupplyColumns.query( xStatement->executeQuery() );
                    // this should have given us a result set which does not contain any data, but
                    // the structural information we need

                    // so the next state is to get the columns
                    eState = RETRIEVE_COLUMNS;
                }
                break;

                default:
                    OSL_ENSURE( sal_False, "::dbtools::getFieldsByCommandDescriptor: oops! unhandled state here!" );
                    eState = FAILED;
            }
        }
    }
    catch( const SQLContext& e ) { if ( _pErrorInfo ) *_pErrorInfo = SQLExceptionInfo( e ); }
    catch( const SQLWarning& e ) { if ( _pErrorInfo ) *_pErrorInfo = SQLExceptionInfo( e ); }
    catch( const SQLException& e ) { if ( _pErrorInfo ) *_pErrorInfo = SQLExceptionInfo( e ); }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "::dbtools::getFieldsByCommandDescriptor: caught an exception while retrieving the fields!" );
    }

    return xFields;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > getFieldNamesByCommandDescriptor( const Reference< XConnection >& _rxConnection,
    const sal_Int32 _nCommandType, const ::rtl::OUString& _rCommand,
    SQLExceptionInfo* _pErrorInfo ) SAL_THROW( ( ) )
{
    // get the container for the fields
    Reference< XComponent > xKeepFieldsAlive;
    Reference< XNameAccess > xFieldContainer = getFieldsByCommandDescriptor( _rxConnection, _nCommandType, _rCommand, xKeepFieldsAlive, _pErrorInfo );

    // get the names of the fields
    Sequence< ::rtl::OUString > aNames;
    if ( xFieldContainer.is() )
        aNames = xFieldContainer->getElementNames();

    // clean up any temporary objects which have been created
    disposeComponent( xKeepFieldsAlive );

    // outta here
    return aNames;
}

//------------------------------------------------------------------------------
SQLContext prependContextInfo(const SQLException& _rException, const Reference< XInterface >& _rxContext, const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails)
{
    // determine the type of the exception
    SQLExceptionInfo aInfo(_rException);

    // the new first chain element
    SQLContext aContextDescription(_rContextDescription, _rxContext, ::rtl::OUString(), 0, aInfo.get(), _rContextDetails);
    return aContextDescription;
}
//------------------------------------------------------------------------------
::rtl::OUString quoteTableName(const Reference< XDatabaseMetaData>& _rxMeta
                               , const ::rtl::OUString& _rName
                               , EComposeRule _eComposeRule
                               , sal_Bool _bUseCatalogInSelect
                               , sal_Bool _bUseSchemaInSelect)
{
    ::rtl::OUString sCatalog,sSchema,sTable,sQuotedName;
    qualifiedNameComponents(_rxMeta,_rName,sCatalog,sSchema,sTable,_eComposeRule);
    composeTableName(_rxMeta,sCatalog,sSchema,sTable,sQuotedName,sal_True,_eComposeRule,_bUseCatalogInSelect,_bUseSchemaInSelect);

    return sQuotedName;
}

//------------------------------------------------------------------------------
void qualifiedNameComponents(const Reference< XDatabaseMetaData >& _rxConnMetaData, const ::rtl::OUString& _rQualifiedName, ::rtl::OUString& _rCatalog, ::rtl::OUString& _rSchema, ::rtl::OUString& _rName,EComposeRule _eComposeRule)
{
    OSL_ENSURE(_rxConnMetaData.is(), "QualifiedNameComponents : invalid meta data!");
    ::rtl::OUString sSeparator = _rxConnMetaData->getCatalogSeparator();

    ::rtl::OUString sName(_rQualifiedName);
    // do we have catalogs ?
    if ( _rxConnMetaData->supportsCatalogsInDataManipulation() )
    {
        if (_rxConnMetaData->isCatalogAtStart())
        {
            // search for the catalog name at the beginning
            sal_Int32 nIndex = sName.indexOf(sSeparator);
            if (-1 != nIndex)
            {
                _rCatalog = sName.copy(0, nIndex);
                sName = sName.copy(nIndex + 1);
            }
        }
        else
        {
            // Katalogname am Ende
            sal_Int32 nIndex = sName.lastIndexOf(sSeparator);
            if (-1 != nIndex)
            {
                _rCatalog = sName.copy(nIndex + 1);
                sName = sName.copy(0, nIndex);
            }
        }
    }

    if ( _rxConnMetaData->supportsSchemasInDataManipulation() )
    {
        sal_Int32 nIndex = sName.indexOf((sal_Unicode)'.');
        //  OSL_ENSURE(-1 != nIndex, "QualifiedNameComponents : no schema separator!");
        if ( nIndex != -1 )
            _rSchema = sName.copy(0, nIndex);
        sName = sName.copy(nIndex + 1);
    }

    _rName = sName;
}

//------------------------------------------------------------------------------
Reference< XNumberFormatsSupplier> getNumberFormats(
            const Reference< XConnection>& _rxConn,
            sal_Bool _bAlloweDefault,
            const Reference< XMultiServiceFactory>& _rxFactory)
{
    // ask the parent of the connection (should be an DatabaseAccess)
    Reference< XNumberFormatsSupplier> xReturn;
    Reference< XChild> xConnAsChild(_rxConn, UNO_QUERY);
    ::rtl::OUString sPropFormatsSupplier = ::rtl::OUString::createFromAscii("NumberFormatsSupplier");
    if (xConnAsChild.is())
    {
        Reference< XPropertySet> xConnParentProps(xConnAsChild->getParent(), UNO_QUERY);
        if (xConnParentProps.is() && hasProperty(sPropFormatsSupplier, xConnParentProps))
            xConnParentProps->getPropertyValue(sPropFormatsSupplier) >>= xReturn;
    }
    else if(_bAlloweDefault && _rxFactory.is())
    {
        xReturn = Reference< XNumberFormatsSupplier>(_rxFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatsSupplier")),UNO_QUERY);
    }
    return xReturn;
}

//==============================================================================
//------------------------------------------------------------------------------
void TransferFormComponentProperties(
            const Reference< XPropertySet>& xOldProps,
            const Reference< XPropertySet>& xNewProps,
            const Locale& _rLocale)
{
try
{
    // kopieren wir erst mal alle Props, die in Quelle und Ziel vorhanden sind und identische Beschreibungen haben
    Reference< XPropertySetInfo> xOldInfo( xOldProps->getPropertySetInfo());
    Reference< XPropertySetInfo> xNewInfo( xNewProps->getPropertySetInfo());

    Sequence< Property> aOldProperties = xOldInfo->getProperties();
    Sequence< Property> aNewProperties = xNewInfo->getProperties();
    int nNewLen = aNewProperties.getLength();

    Property* pOldProps = aOldProperties.getArray();
    Property* pNewProps = aNewProperties.getArray();

    ::rtl::OUString sPropDefaultControl(::rtl::OUString::createFromAscii("DefaultControl"));
    ::rtl::OUString sPropLabelControl(::rtl::OUString::createFromAscii("LabelControl"));
    ::rtl::OUString sPropFormatsSupplier(::rtl::OUString::createFromAscii("FormatsSupplier"));
    ::rtl::OUString sPropCurrencySymbol(::rtl::OUString::createFromAscii("CurrencySymbol"));
    ::rtl::OUString sPropDecimals(::rtl::OUString::createFromAscii("Decimals"));
    ::rtl::OUString sPropEffectiveMin(::rtl::OUString::createFromAscii("EffectiveMin"));
    ::rtl::OUString sPropEffectiveMax(::rtl::OUString::createFromAscii("EffectiveMax"));
    ::rtl::OUString sPropEffectiveDefault(::rtl::OUString::createFromAscii("EffectiveDefault"));
    ::rtl::OUString sPropDefaultText(::rtl::OUString::createFromAscii("DefaultText"));
    ::rtl::OUString sPropDefaultDate(::rtl::OUString::createFromAscii("DefaultDate"));
    ::rtl::OUString sPropDefaultTime(::rtl::OUString::createFromAscii("DefaultTime"));
    ::rtl::OUString sPropValueMin(::rtl::OUString::createFromAscii("ValueMin"));
    ::rtl::OUString sPropValueMax(::rtl::OUString::createFromAscii("ValueMax"));
    ::rtl::OUString sPropDecimalAccuracy(::rtl::OUString::createFromAscii("DecimalAccuracy"));
    ::rtl::OUString sPropClassId(::rtl::OUString::createFromAscii("ClassId"));
    ::rtl::OUString sFormattedServiceName( ::rtl::OUString::createFromAscii( "com.sun.star.form.component.FormattedField" ) );

    for (sal_Int16 i=0; i<aOldProperties.getLength(); ++i)
    {
        if  (   (!pOldProps[i].Name.equals(sPropDefaultControl))
            &&  (!pOldProps[i].Name.equals(sPropLabelControl))
            )
        {
            // binaere Suche
            Property* pResult = ::std::lower_bound(pNewProps, pNewProps + nNewLen,pOldProps[i].Name, ::comphelper::PropertyStringLessFunctor());

//          Property* pResult = (Property*) bsearch(pOldProps + i, (void*)pNewProps, nNewLen, sizeof(Property),
//              &PropertyCompare);
            if (pResult
                && ( pResult != pNewProps + nNewLen && pResult->Name == pOldProps[i].Name )
                && ( pResult->Attributes == pOldProps[i].Attributes )
                && ( (pResult->Attributes & PropertyAttribute::READONLY) == 0 )
                && ( pResult->Type.equals(pOldProps[i].Type)) )
            {   // Attribute stimmen ueberein und Property ist nicht read-only
                try
                {
                    xNewProps->setPropertyValue(pResult->Name, xOldProps->getPropertyValue(pResult->Name));
                }
                catch(IllegalArgumentException& e)
                {
                    e;
#ifdef DBG_UTIL
                    ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii("TransferFormComponentProperties : could not transfer the value for property \"");
                    sMessage += pResult->Name;
                    sMessage += ::rtl::OUString::createFromAscii("\"");;
                    OSL_ENSURE(sal_False, ::rtl::OUStringToOString(sMessage, RTL_TEXTENCODING_ASCII_US));
#endif
                }
            }
        }
    }


    // fuer formatierte Felder (entweder alt oder neu) haben wir ein paar Sonderbehandlungen
    Reference< XServiceInfo > xSI( xOldProps, UNO_QUERY );
    sal_Bool bOldIsFormatted = xSI.is() && xSI->supportsService( sFormattedServiceName );
    xSI = Reference< XServiceInfo >( xNewProps, UNO_QUERY );
    sal_Bool bNewIsFormatted = xSI.is() && xSI->supportsService( sFormattedServiceName );

    if (!bOldIsFormatted && !bNewIsFormatted)
        return; // nothing to do

    if (bOldIsFormatted && bNewIsFormatted)
        // nein, wenn beide formatierte Felder sind, dann machen wir keinerlei Konvertierungen
        // Das geht zu weit ;)
        return;

    if (bOldIsFormatted)
    {
        // aus dem eingestellten Format ein paar Properties rausziehen und zum neuen Set durchschleifen
        Any aFormatKey( xOldProps->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FORMATKEY)) );
        if (aFormatKey.hasValue())
        {
            Reference< XNumberFormatsSupplier> xSupplier;
            xOldProps->getPropertyValue(sPropFormatsSupplier) >>= xSupplier;
            if (xSupplier.is())
            {
                Reference< XNumberFormats> xFormats(xSupplier->getNumberFormats());
                Reference< XPropertySet> xFormat(xFormats->getByKey(getINT32(aFormatKey)));
                if (hasProperty(sPropCurrencySymbol, xFormat))
                {
                    Any aVal( xFormat->getPropertyValue(sPropCurrencySymbol) );
                    if (aVal.hasValue() && hasProperty(sPropCurrencySymbol, xNewProps))
                        // (wenn die Quelle das nicht gesetzt hat, dann auch nicht kopieren, um den
                        // Default-Wert nicht zu ueberschreiben
                        xNewProps->setPropertyValue(sPropCurrencySymbol, aVal);
                }
                if (hasProperty(sPropDecimals, xFormat) && hasProperty(sPropDecimals, xNewProps))
                    xNewProps->setPropertyValue(sPropDecimals, xFormat->getPropertyValue(sPropDecimals));
            }
        }

        // eine eventuelle-Min-Max-Konvertierung
        Any aEffectiveMin( xOldProps->getPropertyValue(sPropEffectiveMin) );
        if (aEffectiveMin.hasValue())
        {   // im Gegensatz zu ValueMin kann EffectiveMin void sein
            if (hasProperty(sPropValueMin, xNewProps))
            {
                OSL_ENSURE(aEffectiveMin.getValueType().getTypeClass() == TypeClass_DOUBLE,
                    "TransferFormComponentProperties : invalid property type !");
                xNewProps->setPropertyValue(sPropValueMin, aEffectiveMin);
            }
        }
        Any aEffectiveMax( xOldProps->getPropertyValue(sPropEffectiveMax) );
        if (aEffectiveMax.hasValue())
        {   // analog
            if (hasProperty(sPropValueMax, xNewProps))
            {
                OSL_ENSURE(aEffectiveMax.getValueType().getTypeClass() == TypeClass_DOUBLE,
                    "TransferFormComponentProperties : invalid property type !");
                xNewProps->setPropertyValue(sPropValueMax, aEffectiveMax);
            }
        }

        // dann koennen wir noch Default-Werte konvertieren und uebernehmen
        Any aEffectiveDefault( xOldProps->getPropertyValue(sPropEffectiveDefault) );
        if (aEffectiveDefault.hasValue())
        {
            sal_Bool bIsString = aEffectiveDefault.getValueType().getTypeClass() == TypeClass_STRING;
            OSL_ENSURE(bIsString || aEffectiveDefault.getValueType().getTypeClass() == TypeClass_DOUBLE,
                "TransferFormComponentProperties : invalid property type !");
                // die Effective-Properties sollten immer void oder string oder double sein ....

            if (hasProperty(sPropDefaultDate, xNewProps) && !bIsString)
            {   // (einen ::rtl::OUString in ein Datum zu konvertieren muss nicht immer klappen, denn das ganze kann ja an
                // eine Textspalte gebunden gewesen sein, aber mit einem double koennen wir was anfangen)
                Date aDate = DBTypeConversion::toDate(getDouble(aEffectiveDefault));
                xNewProps->setPropertyValue(sPropDefaultDate, makeAny(aDate));
            }

            if (hasProperty(sPropDefaultTime, xNewProps) && !bIsString)
            {   // voellig analog mit Zeit
                Time aTime = DBTypeConversion::toTime(getDouble(aEffectiveDefault));
                xNewProps->setPropertyValue(sPropDefaultTime, makeAny(aTime));
            }

            if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE), xNewProps) && !bIsString)
            {   // hier koennen wir einfach das double durchreichen
                xNewProps->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE), aEffectiveDefault);
            }

            if (hasProperty(sPropDefaultText, xNewProps) && bIsString)
            {   // und hier den ::rtl::OUString
                xNewProps->setPropertyValue(sPropDefaultText, aEffectiveDefault);
            }

            // nyi: die Uebersetzung zwischen doubles und ::rtl::OUString wuerde noch mehr Moeglichkeien eroeffnen
        }
    }

    // die andere Richtung : das neu Control soll formatiert sein
    if (bNewIsFormatted)
    {
        // zuerst die Formatierung
        // einen Supplier koennen wir nicht setzen, also muss das neue Set schon einen mitbringen
        Reference< XNumberFormatsSupplier> xSupplier;
        xNewProps->getPropertyValue(sPropFormatsSupplier) >>= xSupplier;
        if (xSupplier.is())
        {
            Reference< XNumberFormats> xFormats(xSupplier->getNumberFormats());

            // Dezimal-Stellen
            sal_Int16 nDecimals = 2;
            if (hasProperty(sPropDecimalAccuracy, xOldProps))
                xOldProps->getPropertyValue(sPropDecimalAccuracy) >>= nDecimals;

            // Grund-Format (je nach ClassId des alten Sets)
            sal_Int32 nBaseKey = 0;
            if (hasProperty(sPropClassId, xOldProps))
            {
                Reference< XNumberFormatTypes> xTypeList(xFormats, UNO_QUERY);
                if (xTypeList.is())
                {
                    sal_Int16 nClassId;
                    xOldProps->getPropertyValue(sPropClassId) >>= nClassId;
                    switch (nClassId)
                    {
                        case FormComponentType::DATEFIELD :
                            nBaseKey = xTypeList->getStandardFormat(NumberFormat::DATE, _rLocale);
                            break;

                        case FormComponentType::TIMEFIELD :
                            nBaseKey = xTypeList->getStandardFormat(NumberFormat::TIME, _rLocale);
                            break;

                        case FormComponentType::CURRENCYFIELD :
                            nBaseKey = xTypeList->getStandardFormat(NumberFormat::CURRENCY, _rLocale);
                            break;
                    }
                }
            }

            // damit koennen wir ein neues Format basteln ...
            ::rtl::OUString sNewFormat = xFormats->generateFormat(nBaseKey, _rLocale, sal_False, sal_False, nDecimals, 0);
                // kein Tausender-Trennzeichen, negative Zahlen nicht in Rot, keine fuehrenden Nullen

            // ... und zum FormatsSupplier hinzufuegen (wenn noetig)
            sal_Int32 nKey = xFormats->queryKey(sNewFormat, _rLocale, sal_False);
            if (nKey == (sal_Int32)-1)
            {   // noch nicht vorhanden in meinem Formatter ...
                nKey = xFormats->addNew(sNewFormat, _rLocale);
            }

            xNewProps->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FORMATKEY), makeAny((sal_Int32)nKey));
        }

        // min-/max-Werte
        Any aNewMin, aNewMax;
        if (hasProperty(sPropValueMin, xOldProps))
            aNewMin = xOldProps->getPropertyValue(sPropValueMin);
        if (hasProperty(sPropValueMax, xOldProps))
            aNewMax = xOldProps->getPropertyValue(sPropValueMax);
        xNewProps->setPropertyValue(sPropEffectiveMin, aNewMin);
        xNewProps->setPropertyValue(sPropEffectiveMax, aNewMax);

        // Default-Wert
        Any aNewDefault;
        if (hasProperty(sPropDefaultDate, xOldProps))
        {
            Any aDate( xOldProps->getPropertyValue(sPropDefaultDate) );
            if (aDate.hasValue())
                aNewDefault <<= DBTypeConversion::toDouble(*(Date*)aDate.getValue());
        }

        if (hasProperty(sPropDefaultTime, xOldProps))
        {
            Any aTime( xOldProps->getPropertyValue(sPropDefaultTime) );
            if (aTime.hasValue())
                aNewDefault <<= DBTypeConversion::toDouble(*(Time*)aTime.getValue());
        }

        // double oder ::rtl::OUString werden direkt uebernommen
        if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE), xOldProps))
            aNewDefault = xOldProps->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE));
        if (hasProperty(sPropDefaultText, xOldProps))
            aNewDefault = xOldProps->getPropertyValue(sPropDefaultText);

        if (aNewDefault.hasValue())
            xNewProps->setPropertyValue(sPropEffectiveDefault, aNewDefault);
    }
}
catch(const Exception&)
{
    OSL_ENSURE( sal_False, "TransferFormComponentProperties: caught an exception!" );
}
}

//------------------------------------------------------------------------------
sal_Bool canInsert(const Reference< XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & Privilege::INSERT) != 0));
}

//------------------------------------------------------------------------------
sal_Bool canUpdate(const Reference< XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & Privilege::UPDATE) != 0));
}

//------------------------------------------------------------------------------
sal_Bool canDelete(const Reference< XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & Privilege::DELETE) != 0));
}
// -----------------------------------------------------------------------------
Reference< XDataSource> findDataSource(const Reference< XInterface >& _xParent)
{
    Reference< XDataSource> xDataSource(_xParent, UNO_QUERY);
    if (!xDataSource.is())
    {
        Reference< XChild> xChild(_xParent, UNO_QUERY);
        if ( xChild.is() )
            xDataSource = findDataSource(xChild->getParent());
    }
    return xDataSource;
}
//------------------------------------------------------------------------------
::rtl::OUString getComposedRowSetStatement( const Reference< XPropertySet >& _rxRowSet, const Reference< XMultiServiceFactory>& _rxFactory,
                                   sal_Bool _bUseRowSetFilter, sal_Bool _bUseRowSetOrder, Reference< XSQLQueryComposer >* _pxComposer )
    SAL_THROW( ( SQLException ) )
{
    ::rtl::OUString sStatement;
    try
    {
        Reference< XConnection> xConn( calcConnection( Reference< XRowSet >( _rxRowSet, UNO_QUERY ), _rxFactory ) );
        if ( xConn.is() )       // implies _rxRowSet.is()
        {
            // build the statement the row set is based on (can't use the ActiveCommand property of the set
            // as this reflects the status after the last execute, not the currently set properties)

            const ::rtl::OUString sPropCommandType  = ::rtl::OUString::createFromAscii( "CommandType" );
            const ::rtl::OUString sPropFilter       = ::rtl::OUString::createFromAscii( "Filter"      );
            const ::rtl::OUString sPropOrder        = ::rtl::OUString::createFromAscii( "Order"       );
            const ::rtl::OUString sPropApplyFilter  = ::rtl::OUString::createFromAscii( "ApplyFilter" );

            const ::rtl::OUString sPropCommand      = OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_COMMAND          );
            const ::rtl::OUString sPropEspaceProc   = OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_ESCAPEPROCESSING );

            // first ensure we have all properties needed
            if  (   hasProperty( sPropCommand,      _rxRowSet )
                &&  hasProperty( sPropCommandType,  _rxRowSet )
                &&  hasProperty( sPropFilter,       _rxRowSet )
                &&  hasProperty( sPropOrder,        _rxRowSet )
                &&  hasProperty( sPropEspaceProc,   _rxRowSet )
                &&  hasProperty( sPropApplyFilter,  _rxRowSet )
                )
            {
                sal_Int32 nCommandType = CommandType::COMMAND;
                ::rtl::OUString sCommand;
                sal_Bool bEscapeProcessing = sal_False;

                _rxRowSet->getPropertyValue( sPropCommandType ) >>= nCommandType     ;
                _rxRowSet->getPropertyValue( sPropCommand     ) >>= sCommand         ;
                _rxRowSet->getPropertyValue( sPropEspaceProc  ) >>= bEscapeProcessing;

                switch ( nCommandType )
                {
                    case CommandType::COMMAND:
                        if ( bEscapeProcessing )
                            sStatement = sCommand;
                        // (in case of no escape processing  we assume a not parseable statement)
                        break;

                    case CommandType::TABLE:
                    {
                        if ( !sCommand.getLength() )
                            break;

                        sal_Bool bUseCatalogInSelect = isDataSourcePropertyEnabled(xConn,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalogInSelect")),sal_True);
                        sal_Bool bUseSchemaInSelect = isDataSourcePropertyEnabled(xConn,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSchemaInSelect")),sal_True);
                        sStatement = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * FROM "));
                        sStatement += quoteTableName(xConn->getMetaData(), sCommand,eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
                    }
                    break;

                    case CommandType::QUERY:
                    {
                        // ask the connection for the query
                        Reference< XQueriesSupplier > xSupplyQueries( xConn, UNO_QUERY );
                        Reference< XNameAccess >      xQueries;
                        if ( xSupplyQueries.is() )
                            xQueries = xQueries.query( xSupplyQueries->getQueries() );
                        OSL_ENSURE( xQueries.is(), "getComposedRowSetStatement: a connection which cannot supply queries?" );
                        if ( !xQueries.is() )
                            break;

                        if ( !xQueries->hasByName( sCommand ) )
                            break;

                        Reference< XPropertySet > xQuery;
                        xQueries->getByName( sCommand ) >>= xQuery;
                        OSL_ENSURE( xQuery.is(), "getComposedRowSetStatement: invalid query!" );
                        if ( !xQuery.is() )
                            break;

                        //  a native query ?
                        sal_Bool bQueryEscapeProcessing = sal_False;
                        xQuery->getPropertyValue( sPropEspaceProc ) >>= bQueryEscapeProcessing;
                        if ( !bQueryEscapeProcessing )
                            break;

                        // the command used by the query
                        xQuery->getPropertyValue( sPropCommand ) >>= sStatement;

                        // use a composer to build a statement from the query filter/order props
                        Reference< XSQLQueryComposerFactory> xFactory( xConn, UNO_QUERY );
                        Reference< XSQLQueryComposer> xComposer;
                        if (xFactory.is())
                            xComposer = xFactory->createQueryComposer();
                        if (!xComposer.is() || !sStatement.getLength())
                            break;

                        // the "basic" statement
                        xComposer->setQuery( sStatement );

                        // the sort order
                        if ( hasProperty( sPropOrder, xQuery ) )
                            xComposer->setOrder( getString( xQuery->getPropertyValue( sPropOrder ) ) );

                        // the filter
                        sal_Bool bApplyFilter = sal_True;
                        if ( hasProperty( sPropApplyFilter, xQuery ) )
                            bApplyFilter = getBOOL( xQuery->getPropertyValue( sPropApplyFilter ) );

                        if ( bApplyFilter )
                            xComposer->setFilter( getString( xQuery->getPropertyValue( sPropFilter ) ) );

                        // the composed statement
                        sStatement = xComposer->getComposedQuery();
                    }
                    break;

                    default:
                        OSL_ENSURE(sal_False, "getComposedRowSetStatement: no table, no query, no statement - what else ?!");
                        break;
                }
            }

            if ( sStatement.getLength() && ( _bUseRowSetFilter || _bUseRowSetOrder ) )
            {
                // create an composer
                Reference< XSQLQueryComposerFactory > xFactory( xConn, UNO_QUERY );
                Reference< XSQLQueryComposer > xComposer;
                if ( xFactory.is() )
                    xComposer = xFactory->createQueryComposer();
                if ( xComposer.is() )
                {
                    xComposer->setQuery( sStatement );

                    // append sort
                    if ( _bUseRowSetOrder )
                        xComposer->setOrder( getString( _rxRowSet->getPropertyValue( sPropOrder ) ) );

                    // append filter
                    if ( _bUseRowSetFilter )
                    {
                        sal_Bool bApplyFilter = sal_True;
                        _rxRowSet->getPropertyValue( sPropApplyFilter ) >>= bApplyFilter;
                        if ( bApplyFilter )
                            xComposer->setFilter( getString( _rxRowSet->getPropertyValue( sPropFilter ) ) );
                    }

                    sStatement = xComposer->getComposedQuery();
                    if ( _pxComposer )
                        *_pxComposer = xComposer;
                }
            }
        }
    }
    catch( const SQLException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "getComposedRowSetStatement: caught an (non-SQL) exception!" );
    }

    return sStatement;
}

//------------------------------------------------------------------------------
::rtl::OUString getComposedRowSetStatement(
                    const Reference< XPropertySet >& _rxRowSet, const Reference< XMultiServiceFactory>& _rxFactory,
                    sal_Bool _bUseRowSetFilter, sal_Bool _bUseRowSetOrder )
{
    return getComposedRowSetStatement( _rxRowSet, _rxFactory, _bUseRowSetFilter, _bUseRowSetOrder, NULL );
}

//------------------------------------------------------------------------------
Reference< XSQLQueryComposer> getCurrentSettingsComposer(
                const Reference< XPropertySet>& _rxRowSetProps,
                const Reference< XMultiServiceFactory>& _rxFactory)
{
    Reference< XSQLQueryComposer > xReturn;
    try
    {
        getComposedRowSetStatement( _rxRowSetProps, _rxFactory, sal_True, sal_True, &xReturn );
    }
    catch( const SQLException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "::getCurrentSettingsComposer : caught an exception !" );
    }

    return xReturn;
}
namespace
{
    class OMetaDataWrapper
    {
        const Reference< XDatabaseMetaData > m_xMetaData;
    public:
        OMetaDataWrapper(const Reference< XDatabaseMetaData >& _xMetaData) : m_xMetaData(_xMetaData){}

        bool  supportsSchemasInDataManipulation(  )
        {
            return m_xMetaData->supportsSchemasInDataManipulation() ? true : false;
        }
        bool  supportsSchemasInProcedureCalls(  )
        {
            return m_xMetaData->supportsSchemasInProcedureCalls() ? true : false;
        }
        bool  supportsSchemasInTableDefinitions(  )
        {
            return m_xMetaData->supportsSchemasInTableDefinitions() ? true : false;
        }
        bool  supportsSchemasInIndexDefinitions(  )
        {
            return m_xMetaData->supportsSchemasInIndexDefinitions() ? true : false;
        }
        bool  supportsSchemasInPrivilegeDefinitions(  )
        {
            return m_xMetaData->supportsSchemasInPrivilegeDefinitions() ? true : false;
        }
        bool  supportsCatalogsInDataManipulation(  )
        {
            return m_xMetaData->supportsCatalogsInDataManipulation() ? true : false;
        }
        bool  supportsCatalogsInProcedureCalls(  )
        {
            return m_xMetaData->supportsCatalogsInProcedureCalls() ? true : false;
        }
        bool  supportsCatalogsInTableDefinitions(  )
        {
            return m_xMetaData->supportsCatalogsInTableDefinitions() ? true : false;
        }
        bool  supportsCatalogsInIndexDefinitions(  )
        {
            return m_xMetaData->supportsCatalogsInIndexDefinitions() ? true : false;
        }
        bool  supportsCatalogsInPrivilegeDefinitions(  )
        {
            return m_xMetaData->supportsCatalogsInPrivilegeDefinitions() ? true : false;
        }
    };
}
//--------------------------------------------------------------------------
void composeTableName(  const Reference< XDatabaseMetaData >& _rxMetaData,
                        const ::rtl::OUString& _rCatalog,
                        const ::rtl::OUString& _rSchema,
                        const ::rtl::OUString& _rName,
                        ::rtl::OUString& _rComposedName,
                        sal_Bool _bQuote,
                        EComposeRule _eComposeRule
                        , sal_Bool _bUseCatalogInSelect
                        , sal_Bool _bUseSchemaInSelect)
{
    OSL_ENSURE(_rxMetaData.is(), "composeTableName : invalid meta data !");
    OSL_ENSURE(_rName.getLength(), "composeTableName : at least the name should be non-empty !");

    ::std::mem_fun_t<bool,OMetaDataWrapper> aCatalogCall = ::std::mem_fun(&OMetaDataWrapper::supportsCatalogsInDataManipulation);
    ::std::mem_fun_t<bool,OMetaDataWrapper> aSchemaCall = ::std::mem_fun(&OMetaDataWrapper::supportsSchemasInDataManipulation);

    switch ( _eComposeRule )
    {
        case eInTableDefinitions:
            aCatalogCall = ::std::mem_fun(&OMetaDataWrapper::supportsCatalogsInTableDefinitions);
            aSchemaCall = ::std::mem_fun(&OMetaDataWrapper::supportsSchemasInTableDefinitions);
            break;
        case eInIndexDefinitions:
            aCatalogCall = ::std::mem_fun(&OMetaDataWrapper::supportsCatalogsInIndexDefinitions);
            aSchemaCall = ::std::mem_fun(&OMetaDataWrapper::supportsSchemasInIndexDefinitions);
            break;
        case eInProcedureCalls:
            aCatalogCall = ::std::mem_fun(&OMetaDataWrapper::supportsCatalogsInProcedureCalls);
            aSchemaCall = ::std::mem_fun(&OMetaDataWrapper::supportsSchemasInProcedureCalls);
            break;
        case eInPrivilegeDefinitions:
            aCatalogCall = ::std::mem_fun(&OMetaDataWrapper::supportsCatalogsInPrivilegeDefinitions);
            aSchemaCall = ::std::mem_fun(&OMetaDataWrapper::supportsSchemasInPrivilegeDefinitions);
            break;
    }


    ::rtl::OUString sQuoteString = _rxMetaData->getIdentifierQuoteString();
#define QUOTE(s,s2) if (_bQuote) s += quoteName(sQuoteString,s2); else s += s2

    static ::rtl::OUString sEmpty;
    static ::rtl::OUString sSeparator = ::rtl::OUString::createFromAscii(".");

    OMetaDataWrapper aWrapper(_rxMetaData);
    _rComposedName = sEmpty;
    ::rtl::OUString sCatalogSep;
    sal_Bool bCatlogAtStart = sal_True;
    if ( _bUseCatalogInSelect && _rCatalog.getLength() && aCatalogCall(&aWrapper) )
    {
        sCatalogSep     = _rxMetaData->getCatalogSeparator();
        bCatlogAtStart  = _rxMetaData->isCatalogAtStart();

        if ( bCatlogAtStart && sCatalogSep.getLength())
        {
            QUOTE(_rComposedName,_rCatalog);
            _rComposedName += sCatalogSep;
        }
    }

    if ( _bUseSchemaInSelect && _rSchema.getLength() && aSchemaCall(&aWrapper) )
    {
        QUOTE(_rComposedName,_rSchema);
        _rComposedName += sSeparator;
    }

    QUOTE(_rComposedName,_rName);

    if ( _bUseCatalogInSelect && _rCatalog.getLength() && !bCatlogAtStart && sCatalogSep.getLength() && aCatalogCall(&aWrapper) )
    {
        _rComposedName += sCatalogSep;
        QUOTE(_rComposedName,_rCatalog);
    }
}
// -----------------------------------------------------------------------------
sal_Int32 getSearchColumnFlag( const Reference< XConnection>& _rxConn,sal_Int32 _nDataType)
{
    sal_Int32 nSearchFlag = 0;
    Reference<XResultSet> xSet = _rxConn->getMetaData()->getTypeInfo();
    if(xSet.is())
    {
        Reference<XRow> xRow(xSet,UNO_QUERY);
        while(xSet->next())
        {
            if(xRow->getInt(2) == _nDataType)
            {
                nSearchFlag = xRow->getInt(9);
                break;
            }
        }
    }
    return nSearchFlag;
}
// -----------------------------------------------------------------------------
::rtl::OUString createUniqueName(const Reference<XNameAccess>& _rxContainer,const ::rtl::OUString& _rBaseName,sal_Bool _bStartWithNumber)
{
    ::rtl::OUString sName(_rBaseName);
    sal_Int32 nPos = 1;
    if ( _bStartWithNumber )
        sName += ::rtl::OUString::valueOf(nPos);

    OSL_ENSURE( _rxContainer.is() ,"No valid container!");
    if ( _rxContainer.is() )
    {
        while(_rxContainer->hasByName(sName))
        {
            sName = _rBaseName;
            sName += ::rtl::OUString::valueOf(++nPos);
        }
    }
    return sName;
}
// -----------------------------------------------------------------------------
void showError(const SQLExceptionInfo& _rInfo,
               const Reference< XWindow>& _xParent,
               const Reference< XMultiServiceFactory >& _xFactory)
{
    if (_rInfo.isValid())
    {
        try
        {
            Sequence< Any > aArgs(2);
            aArgs[0] <<= PropertyValue(::rtl::OUString::createFromAscii("SQLException"), 0, _rInfo.get(), PropertyState_DIRECT_VALUE);
            aArgs[1] <<= PropertyValue(::rtl::OUString::createFromAscii("ParentWindow"), 0, makeAny(_xParent), PropertyState_DIRECT_VALUE);

            static ::rtl::OUString s_sDialogServiceName = ::rtl::OUString::createFromAscii("com.sun.star.sdb.ErrorMessageDialog");
            Reference< XExecutableDialog > xErrorDialog(
                _xFactory->createInstanceWithArguments(s_sDialogServiceName, aArgs), UNO_QUERY);
            if (xErrorDialog.is())
                xErrorDialog->execute();
            else
            {
                OSL_ENSURE(0,"dbtools::showError: no XExecutableDialog found!");
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"showError: could not display the error message!");
        }
    }
}

// -------------------------------------------------------------------------
sal_Bool implUpdateObject(const Reference< XRowUpdate >& _rxUpdatedObject,
    const sal_Int32 _nColumnIndex, const Any& _rValue) SAL_THROW ( ( SQLException, RuntimeException ) )
{
    sal_Bool bSuccessfullyReRouted = sal_True;
    switch (_rValue.getValueTypeClass())
    {
        case TypeClass_ANY:
        {
            Any aInnerValue;
            _rValue >>= aInnerValue;
            bSuccessfullyReRouted = implUpdateObject(_rxUpdatedObject, _nColumnIndex, aInnerValue);
        }
        break;

        case TypeClass_VOID:
            _rxUpdatedObject->updateNull(_nColumnIndex);
            break;

        case TypeClass_STRING:
            _rxUpdatedObject->updateString(_nColumnIndex, *(rtl::OUString*)_rValue.getValue());
            break;

        case TypeClass_BOOLEAN:
            _rxUpdatedObject->updateBoolean(_nColumnIndex, *(sal_Bool *)_rValue.getValue());
            break;

        case TypeClass_BYTE:
            _rxUpdatedObject->updateByte(_nColumnIndex, *(sal_Int8 *)_rValue.getValue());
            break;

        case TypeClass_UNSIGNED_SHORT:
        case TypeClass_SHORT:
            _rxUpdatedObject->updateShort(_nColumnIndex, *(sal_Int16*)_rValue.getValue());
            break;

        case TypeClass_CHAR:
            _rxUpdatedObject->updateString(_nColumnIndex,::rtl::OUString((sal_Unicode *)_rValue.getValue(),1));
            break;

        case TypeClass_UNSIGNED_LONG:
        case TypeClass_LONG:
            _rxUpdatedObject->updateInt(_nColumnIndex, *(sal_Int32*)_rValue.getValue());
            break;

        case TypeClass_FLOAT:
            _rxUpdatedObject->updateFloat(_nColumnIndex, *(float*)_rValue.getValue());
            break;

        case TypeClass_DOUBLE:
            _rxUpdatedObject->updateDouble(_nColumnIndex, *(double*)_rValue.getValue());
            break;

        case TypeClass_SEQUENCE:
            if (_rValue.getValueType() == ::getCppuType((const Sequence< sal_Int8 > *)0))
                _rxUpdatedObject->updateBytes(_nColumnIndex, *(Sequence<sal_Int8>*)_rValue.getValue());
            else
                bSuccessfullyReRouted = sal_False;
            break;
        case TypeClass_STRUCT:
            if (_rValue.getValueType() == ::getCppuType((const DateTime*)0))
                _rxUpdatedObject->updateTimestamp(_nColumnIndex, *(DateTime*)_rValue.getValue());
            else if (_rValue.getValueType() == ::getCppuType((const Date*)0))
                _rxUpdatedObject->updateDate(_nColumnIndex, *(Date*)_rValue.getValue());
            else if (_rValue.getValueType() == ::getCppuType((const Time*)0))
                _rxUpdatedObject->updateTime(_nColumnIndex, *(Time*)_rValue.getValue());
            else
                bSuccessfullyReRouted = sal_False;
            break;

        case TypeClass_INTERFACE:
            if (_rValue.getValueType() == ::getCppuType(static_cast<Reference< XInputStream>*>(NULL)))
            {
                Reference< XInputStream >  xStream;
                _rValue >>= xStream;
                _rxUpdatedObject->updateBinaryStream(_nColumnIndex, xStream, xStream->available());
                break;
            }
            // run through
        default:
            bSuccessfullyReRouted = sal_False;
    }

    return bSuccessfullyReRouted;
}
// -------------------------------------------------------------------------
sal_Bool implSetObject( const Reference< XParameters >& _rxParameters,
                        const sal_Int32 _nColumnIndex, const Any& _rValue) SAL_THROW ( ( SQLException, RuntimeException ) )
{
    sal_Bool bSuccessfullyReRouted = sal_True;
    switch (_rValue.getValueTypeClass())
    {
        case TypeClass_HYPER:
        {
            sal_Int64 nValue = 0;
            OSL_VERIFY( _rValue >>= nValue );
            _rxParameters->setLong( _nColumnIndex, nValue );
        }
        break;

        case TypeClass_ANY:
        {
            Any aInnerValue;
            _rValue >>= aInnerValue;
            bSuccessfullyReRouted = implSetObject(_rxParameters, _nColumnIndex, aInnerValue);
        }
        break;

        case TypeClass_VOID:
            _rxParameters->setNull(_nColumnIndex,DataType::VARCHAR);
            break;

        case TypeClass_STRING:
            _rxParameters->setString(_nColumnIndex, *(rtl::OUString*)_rValue.getValue());
            break;

        case TypeClass_BOOLEAN:
            _rxParameters->setBoolean(_nColumnIndex, *(sal_Bool *)_rValue.getValue());
            break;

        case TypeClass_BYTE:
            _rxParameters->setByte(_nColumnIndex, *(sal_Int8 *)_rValue.getValue());
            break;

        case TypeClass_UNSIGNED_SHORT:
        case TypeClass_SHORT:
            _rxParameters->setShort(_nColumnIndex, *(sal_Int16*)_rValue.getValue());
            break;

        case TypeClass_CHAR:
            _rxParameters->setString(_nColumnIndex, ::rtl::OUString((sal_Unicode *)_rValue.getValue(),1));
            break;

        case TypeClass_UNSIGNED_LONG:
        case TypeClass_LONG:
            _rxParameters->setInt(_nColumnIndex, *(sal_Int32*)_rValue.getValue());
            break;

        case TypeClass_FLOAT:
            _rxParameters->setFloat(_nColumnIndex, *(float*)_rValue.getValue());
            break;

        case TypeClass_DOUBLE:
            _rxParameters->setDouble(_nColumnIndex, *(double*)_rValue.getValue());
            break;

        case TypeClass_SEQUENCE:
            if (_rValue.getValueType() == ::getCppuType((const Sequence< sal_Int8 > *)0))
            {
                _rxParameters->setBytes(_nColumnIndex, *(Sequence<sal_Int8>*)_rValue.getValue());
            }
            else
                bSuccessfullyReRouted = sal_False;
            break;
        case TypeClass_STRUCT:
            if (_rValue.getValueType() == ::getCppuType((const DateTime*)0))
                _rxParameters->setTimestamp(_nColumnIndex, *(DateTime*)_rValue.getValue());
            else if (_rValue.getValueType() == ::getCppuType((const Date*)0))
                _rxParameters->setDate(_nColumnIndex, *(Date*)_rValue.getValue());
            else if (_rValue.getValueType() == ::getCppuType((const Time*)0))
                _rxParameters->setTime(_nColumnIndex, *(Time*)_rValue.getValue());
            else
                bSuccessfullyReRouted = sal_False;
            break;

        case TypeClass_INTERFACE:
            if (_rValue.getValueType() == ::getCppuType(static_cast<Reference< XInputStream>*>(NULL)))
            {
                Reference< XInputStream >  xStream;
                _rValue >>= xStream;
                _rxParameters->setBinaryStream(_nColumnIndex, xStream, xStream->available());
                break;
            }
            // run through
        default:
            bSuccessfullyReRouted = sal_False;

    }

    return bSuccessfullyReRouted;
}

//==================================================================
// OParameterContinuation
//==================================================================
class OParameterContinuation : public OInteraction< XInteractionSupplyParameters >
{
    Sequence< PropertyValue >       m_aValues;

protected:
    virtual ~OParameterContinuation();
public:
    OParameterContinuation() { }

    Sequence< PropertyValue >   getValues() const { return m_aValues; }

// XInteractionSupplyParameters
    virtual void SAL_CALL setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException);
};

OParameterContinuation::~OParameterContinuation()
{
}
//------------------------------------------------------------------
void SAL_CALL OParameterContinuation::setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException)
{
    m_aValues = _rValues;
}
//..................................................................


    // -----------------------------------------------------------------------------
void askForParameters(const Reference<XSQLQueryComposer> & _xComposer,
                      const Reference<XParameters>& _xParameters,
                      const Reference< XConnection>& _xConnection,
                      const Reference< XInteractionHandler >& _rxHandler)
{
    OSL_ENSURE(_xComposer.is(),"dbtools::askForParameters XSQLQueryComposer is null!");
    OSL_ENSURE(_xParameters.is(),"dbtools::askForParameters XParameters is null!");
    OSL_ENSURE(_xConnection.is(),"dbtools::askForParameters XConnection is null!");
    OSL_ENSURE(_rxHandler.is(),"dbtools::askForParameters XInteractionHandler is null!");

    // we have to set this here again because getCurrentSettingsComposer can force a setpropertyvalue
    Reference<XParametersSupplier>  xParameters = Reference<XParametersSupplier> (_xComposer, UNO_QUERY);

    Reference<XIndexAccess>  xParamsAsIndicies = xParameters.is() ? xParameters->getParameters() : Reference<XIndexAccess>();
    Reference<XNameAccess>   xParamsAsNames(xParamsAsIndicies, UNO_QUERY);
    sal_Int32 nParamCount = xParamsAsIndicies.is() ? xParamsAsIndicies->getCount() : 0;
    if (nParamCount)
    {
        // build an interaction request
        // two continuations (Ok and Cancel)
        OInteractionAbort* pAbort = new OInteractionAbort;
        OParameterContinuation* pParams = new OParameterContinuation;
        // the request
        ParametersRequest aRequest;
        aRequest.Parameters = xParamsAsIndicies;
        aRequest.Connection = _xConnection;
        OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aRequest));
        Reference< XInteractionRequest > xRequest(pRequest);
        // some knittings
        pRequest->addContinuation(pAbort);
        pRequest->addContinuation(pParams);

        // execute the request
        _rxHandler->handle(xRequest);

        if (!pParams->wasSelected())
            // canceled by the user (i.e. (s)he canceled the dialog)
            throw RowSetVetoException();

        // now transfer the values from the continuation object to the parameter columns
        Sequence< PropertyValue > aFinalValues = pParams->getValues();
        const PropertyValue* pFinalValues = aFinalValues.getConstArray();
        for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
        {
            Reference< XPropertySet > xParamColumn;
            ::cppu::extractInterface(xParamColumn, xParamsAsIndicies->getByIndex(i));
            if (xParamColumn.is())
            {
#ifdef DBG_UTIL
                ::rtl::OUString sName;
                xParamColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sName;
                OSL_ENSURE(sName.equals(pFinalValues->Name), "::dbaui::askForParameters: inconsistent parameter names!");
#endif
                // determine the field type and ...
                sal_Int32 nParamType = 0;
                xParamColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)) >>= nParamType;
                // ... the scale of the parameter column
                sal_Int32 nScale = 0;
                if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE), xParamColumn))
                    xParamColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)) >>= nScale;
                // and set the value
                _xParameters->setObjectWithInfo(i + 1, pFinalValues->Value, nParamType, nScale);
                    // (the index of the parameters is one-based)
            }
        }
    }
}
// -----------------------------------------------------------------------------
void setObjectWithInfo(const Reference<XParameters>& _xParams,
                       sal_Int32 parameterIndex,
                       const Any& x,
                       sal_Int32 sqlType,
                       sal_Int32 scale)  throw(SQLException, RuntimeException)
{
    if(!x.hasValue())
        _xParams->setNull(parameterIndex,sqlType);
    else
    {
        switch(sqlType)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                _xParams->setString(parameterIndex,::comphelper::getString(x));
                break;
            case DataType::BIGINT:
                {
                    sal_Int64 nValue;
                    if(x >>= nValue)
                    {
                        _xParams->setLong(parameterIndex,nValue);
                        break;
                    }
                }
                break;

            case DataType::FLOAT:
            case DataType::REAL:
                {
                    float nValue;
                    if(x >>= nValue)
                    {
                        _xParams->setFloat(parameterIndex,nValue);
                        break;
                    }
                }
                // run through if we couldn't set a float value
            case DataType::DOUBLE:
                _xParams->setDouble(parameterIndex,::comphelper::getDouble(x));
                break;
            case DataType::DATE:
                {
                    ::com::sun::star::util::Date aValue;
                    if(x >>= aValue)
                        _xParams->setDate(parameterIndex,aValue);
                }
                break;
            case DataType::TIME:
                {
                    ::com::sun::star::util::Time aValue;
                    if(x >>= aValue)
                        _xParams->setTime(parameterIndex,aValue);
                }
                break;
            case DataType::TIMESTAMP:
                {
                    ::com::sun::star::util::DateTime aValue;
                    if(x >>= aValue)
                        _xParams->setTimestamp(parameterIndex,aValue);
                }
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                {
                    Sequence< sal_Int8> aBytes;
                    if(x >>= aBytes)
                        _xParams->setBytes(parameterIndex,aBytes);
                    else
                    {
                        Reference< XBlob > xBlob;
                        if(x >>= xBlob)
                            _xParams->setBlob(parameterIndex,xBlob);
                        else
                        {
                            Reference< XClob > xClob;
                            if(x >>= xClob)
                                _xParams->setClob(parameterIndex,xClob);
                            else
                            {
                                Reference< ::com::sun::star::io::XInputStream > xBinStream;
                                if(x >>= xBinStream)
                                    _xParams->setBinaryStream(parameterIndex,xBinStream,xBinStream->available());
                            }
                        }
                    }
                }
                break;
            case DataType::BIT:
                _xParams->setBoolean(parameterIndex,::cppu::any2bool(x));
                break;
            case DataType::TINYINT:
                _xParams->setByte(parameterIndex,(sal_Int8)::comphelper::getINT32(x));
                break;
            case DataType::SMALLINT:
                _xParams->setShort(parameterIndex,(sal_Int16)::comphelper::getINT32(x));
                break;
            case DataType::INTEGER:
                _xParams->setInt(parameterIndex,::comphelper::getINT32(x));
                break;
            default:
                {
                    ::rtl::OUString aVal = ::rtl::OUString::createFromAscii("Unknown SQL Type for PreparedStatement.setObjectWithInfo (SQL Type=");
                    aVal += ::rtl::OUString::valueOf(sqlType);
                    throw SQLException( aVal,_xParams,::rtl::OUString(),0,Any());
                }
        }
    }
}
//.........................................................................
}   // namespace dbtools
//.........................................................................

//.........................................................................
namespace connectivity
{
//.........................................................................

void release(oslInterlockedCount& _refCount,
             ::cppu::OBroadcastHelper& rBHelper,
             ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
             ::com::sun::star::lang::XComponent* _pObject)
{
    if (osl_decrementInterlockedCount( &_refCount ) == 0)
    {
        osl_incrementInterlockedCount( &_refCount );

        if (!rBHelper.bDisposed && !rBHelper.bInDispose)
        {
            // remember the parent
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xParent;
            {
                ::osl::MutexGuard aGuard( rBHelper.rMutex );
                xParent = _xInterface;
                _xInterface = NULL;
            }

            // First dispose
            _pObject->dispose();

            // only the alive ref holds the object
            OSL_ASSERT( _refCount == 1 );

            // release the parent in the ~
            if (xParent.is())
            {
                ::osl::MutexGuard aGuard( rBHelper.rMutex );
                _xInterface = xParent;
            }

//                  // destroy the object if xHoldAlive decrement the refcount to 0
//                  m_pDerivedImplementation->WEAK::release();
        }
    }
    else
        osl_incrementInterlockedCount( &_refCount );
}

void checkDisposed(sal_Bool _bThrow) throw ( DisposedException )
{
    if (_bThrow)
        throw DisposedException();

}
// -------------------------------------------------------------------------
    OSQLColumns::const_iterator find(   OSQLColumns::const_iterator __first,
                                        OSQLColumns::const_iterator __last,
                                        const ::rtl::OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
    {
        ::rtl::OUString sName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
        return find(__first,__last,sName,_rVal,_rCase);
    }
    // -------------------------------------------------------------------------
    OSQLColumns::const_iterator findRealName(   OSQLColumns::const_iterator __first,
                                        OSQLColumns::const_iterator __last,
                                        const ::rtl::OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
    {
        ::rtl::OUString sRealName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME);
        return find(__first,__last,sRealName,_rVal,_rCase);
    }
    // -------------------------------------------------------------------------
    OSQLColumns::const_iterator find(   OSQLColumns::const_iterator __first,
                                        OSQLColumns::const_iterator __last,
                                        const ::rtl::OUString& _rProp,
                                        const ::rtl::OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
    {
        while (__first != __last && !_rCase(getString((*__first)->getPropertyValue(_rProp)),_rVal))
            ++__first;
        return __first;
    }

// -----------------------------------------------------------------------------
} //namespace connectivity
// -----------------------------------------------------------------------------

