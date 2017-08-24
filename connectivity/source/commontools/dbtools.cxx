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

#include <connectivity/CommonTools.hxx>
#include "TConnection.hxx"
#include <connectivity/ParameterCont.hxx>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/BooleanComparisonMode.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/ErrorMessageDialog.hpp>
#include <com/sun/star/sdb/ParametersRequest.hpp>
#include <com/sun/star/sdb/RowSetVetoException.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/ConnectionPool.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XDriverManager.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/propertysequence.hxx>
#include <connectivity/conncleanup.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/statementcomposer.hxx>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <cppuhelper/implbase.hxx>
#include "strings.hrc"
#include "resource/sharedresources.hxx"
#include <connectivity/OSubComponent.hxx>

#include <algorithm>
#include <iterator>
#include <set>

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
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::form;
using namespace connectivity;

namespace dbtools
{

namespace
{
    typedef sal_Bool (SAL_CALL XDatabaseMetaData::*FMetaDataSupport)();
}

sal_Int32 getDefaultNumberFormat(const Reference< XPropertySet >& _xColumn,
                                 const Reference< XNumberFormatTypes >& _xTypes,
                                 const Locale& _rLocale)
{
    OSL_ENSURE(_xTypes.is() && _xColumn.is(), "dbtools::getDefaultNumberFormat: invalid arg !");
    if (!_xTypes.is() || !_xColumn.is())
        return NumberFormat::UNDEFINED;

    sal_Int32 nDataType = 0;
    sal_Int32 nScale = 0;
    try
    {
        // determine the datatype of the column
        _xColumn->getPropertyValue("Type") >>= nDataType;

        if (DataType::NUMERIC == nDataType || DataType::DECIMAL == nDataType)
            _xColumn->getPropertyValue("Scale") >>= nScale;
    }
    catch (Exception&)
    {
        return NumberFormat::UNDEFINED;
    }
    return getDefaultNumberFormat(nDataType,
                    nScale,
                    ::cppu::any2bool(_xColumn->getPropertyValue("IsCurrency")),
                    _xTypes,
                    _rLocale);
}

sal_Int32 getDefaultNumberFormat(sal_Int32 _nDataType,
                                 sal_Int32 _nScale,
                                 bool _bIsCurrency,
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
        case DataType::BOOLEAN:
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
                    OUString sNewFormat = xFormats->generateFormat( 0, _rLocale, false, false, (sal_Int16)_nScale, 1);

                    // and add it to the formatter if necessary
                    nFormat = xFormats->queryKey(sNewFormat, _rLocale, false);
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
        case DataType::CLOB:
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
        case DataType::REF:
        default:
            nFormat = _xTypes->getStandardFormat(NumberFormat::UNDEFINED, _rLocale);
    }
    return nFormat;
}

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

Reference< XDataSource> getDataSource_allowException(
            const OUString& _rsTitleOrPath,
            const Reference< XComponentContext >& _rxContext )
{
    ENSURE_OR_RETURN( !_rsTitleOrPath.isEmpty(), "getDataSource_allowException: invalid arg !", nullptr );

    Reference< XDatabaseContext> xDatabaseContext = DatabaseContext::create(_rxContext);

    return Reference< XDataSource >( xDatabaseContext->getByName( _rsTitleOrPath ), UNO_QUERY );
}

Reference< XDataSource > getDataSource(
            const OUString& _rsTitleOrPath,
            const Reference< XComponentContext >& _rxContext )
{
    Reference< XDataSource > xDS;
    try
    {
        xDS = getDataSource_allowException( _rsTitleOrPath, _rxContext );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return xDS;
}

Reference< XConnection > getConnection_allowException(
            const OUString& _rsTitleOrPath,
            const OUString& _rsUser,
            const OUString& _rsPwd,
            const Reference< XComponentContext>& _rxContext)
{
    Reference< XDataSource> xDataSource( getDataSource_allowException(_rsTitleOrPath, _rxContext) );
    Reference<XConnection> xConnection;
    if (xDataSource.is())
    {
        // do it with interaction handler
        if(_rsUser.isEmpty() || _rsPwd.isEmpty())
        {
            Reference<XPropertySet> xProp(xDataSource,UNO_QUERY);
            OUString sPwd, sUser;
            bool bPwdReq = false;
            try
            {
                xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPwd;
                bPwdReq = ::cppu::any2bool(xProp->getPropertyValue("IsPasswordRequired"));
                xProp->getPropertyValue("User") >>= sUser;
            }
            catch(Exception&)
            {
                OSL_FAIL("dbtools::getConnection: error while retrieving data source properties!");
            }
            if(bPwdReq && sPwd.isEmpty())
            {   // password required, but empty -> connect using an interaction handler
                Reference<XCompletedConnection> xConnectionCompletion(xProp, UNO_QUERY);
                if (xConnectionCompletion.is())
                {   // instantiate the default SDB interaction handler
                    Reference< XInteractionHandler > xHandler(
                        InteractionHandler::createWithParent(_rxContext, nullptr), UNO_QUERY );
                    xConnection = xConnectionCompletion->connectWithCompletion(xHandler);
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

Reference< XConnection> getConnection_withFeedback(const OUString& _rDataSourceName,
        const OUString& _rUser, const OUString& _rPwd, const Reference< XComponentContext>& _rxContext)
{
    Reference< XConnection > xReturn;
    try
    {
        xReturn = getConnection_allowException(_rDataSourceName, _rUser, _rPwd, _rxContext);
    }
    catch(SQLException&)
    {
        // allowed to pass
        throw;
    }
    catch(Exception&)
    {
        OSL_FAIL("::dbtools::getConnection_withFeedback: unexpected (non-SQL) exception caught!");
    }
    return xReturn;
}

Reference< XConnection> getConnection(const Reference< XRowSet>& _rxRowSet)
{
    Reference< XConnection> xReturn;
    Reference< XPropertySet> xRowSetProps(_rxRowSet, UNO_QUERY);
    if (xRowSetProps.is())
        xRowSetProps->getPropertyValue("ActiveConnection") >>= xReturn;
    return xReturn;
}

// helper function which allows to implement both the connectRowset and the ensureRowSetConnection semantics
// if connectRowset (which is deprecated) is removed, this function and one of its parameters are
// not needed anymore, the whole implementation can be moved into ensureRowSetConnection then)
SharedConnection lcl_connectRowSet(const Reference< XRowSet>& _rxRowSet, const Reference< XComponentContext >& _rxContext,
        bool _bSetAsActiveConnection, bool _bAttachAutoDisposer )
{
    SharedConnection xConnection;

    do
    {
        Reference< XPropertySet> xRowSetProps(_rxRowSet, UNO_QUERY);
        if ( !xRowSetProps.is() )
            break;

        // 1. already connected?
        Reference< XConnection > xExistingConn(
            xRowSetProps->getPropertyValue("ActiveConnection"),
            UNO_QUERY );

        if  (   xExistingConn.is()
            // 2. embedded in a database?
            ||  isEmbeddedInDatabase( _rxRowSet, xExistingConn )
            // 3. is there a connection in the parent hierarchy?
            ||  ( xExistingConn = findConnection( _rxRowSet ) ).is()
            )
        {
            if ( _bSetAsActiveConnection )
            {
                xRowSetProps->setPropertyValue("ActiveConnection", makeAny( xExistingConn ) );
                // no auto disposer needed, since we did not create the connection
            }

            xConnection.reset( xExistingConn, SharedConnection::NoTakeOwnership );
            break;
        }

        // build a connection with its current settings (4. data source name, or 5. URL)

        const OUString sUserProp( "User" );
        OUString sDataSourceName;
        xRowSetProps->getPropertyValue("DataSourceName") >>= sDataSourceName;
        OUString sURL;
        xRowSetProps->getPropertyValue("URL") >>= sURL;

        Reference< XConnection > xPureConnection;
        if (!sDataSourceName.isEmpty())
        {   // the row set's data source property is set
            // -> try to connect, get user and pwd setting for that
            OUString sUser, sPwd;

            if (hasProperty(sUserProp, xRowSetProps))
                xRowSetProps->getPropertyValue(sUserProp) >>= sUser;
            if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD), xRowSetProps))
                xRowSetProps->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPwd;

            xPureConnection = getConnection_allowException( sDataSourceName, sUser, sPwd, _rxContext );
        }
        else if (!sURL.isEmpty())
        {   // the row set has no data source, but a connection url set
            // -> try to connection with that url
            Reference< XConnectionPool > xDriverManager;
            try {
                xDriverManager = ConnectionPool::create( _rxContext );
            } catch( const Exception& ) {  }
            if (xDriverManager.is())
            {
                OUString sUser, sPwd;
                if (hasProperty(sUserProp, xRowSetProps))
                    xRowSetProps->getPropertyValue(sUserProp) >>= sUser;
                if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD), xRowSetProps))
                    xRowSetProps->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPwd;
                if (!sUser.isEmpty())
                {   // use user and pwd together with the url
                    auto aInfo(::comphelper::InitPropertySequence({
                        { "user", makeAny(sUser) },
                        { "password", makeAny(sPwd) }
                    }));
                    xPureConnection = xDriverManager->getConnectionWithInfo( sURL, aInfo );
                }
                else
                    // just use the url
                    xPureConnection = xDriverManager->getConnection( sURL );
            }
        }
        xConnection.reset(
            xPureConnection,
            _bAttachAutoDisposer ? SharedConnection::NoTakeOwnership : SharedConnection::TakeOwnership
            /* take ownership if and only if we're *not* going to auto-dispose the connection */
        );

        // now if we created a connection, forward it to the row set
        if ( xConnection.is() && _bSetAsActiveConnection )
        {
            try
            {
                if ( _bAttachAutoDisposer )
                {
                    rtl::Reference<OAutoConnectionDisposer> pAutoDispose = new OAutoConnectionDisposer( _rxRowSet, xConnection );
                }
                else
                    xRowSetProps->setPropertyValue(
                        "ActiveConnection",
                        makeAny( xConnection.getTyped() )
                    );
            }
            catch(Exception&)
            {
                OSL_FAIL("EXception when we set the new active connection!");
            }
        }
    }
    while ( false );

    return xConnection;
}

Reference< XConnection> connectRowset(const Reference< XRowSet>& _rxRowSet, const Reference< XComponentContext >& _rxContext,
    bool _bSetAsActiveConnection )
{
    SharedConnection xConnection = lcl_connectRowSet( _rxRowSet, _rxContext, _bSetAsActiveConnection, true );
    return xConnection.getTyped();
}

SharedConnection ensureRowSetConnection(const Reference< XRowSet>& _rxRowSet, const Reference< XComponentContext>& _rxContext,
    bool _bUseAutoConnectionDisposer )
{
    return lcl_connectRowSet( _rxRowSet, _rxContext, true, _bUseAutoConnectionDisposer );
}

Reference< XNameAccess> getTableFields(const Reference< XConnection>& _rxConn,const OUString& _rName)
{
    Reference< XComponent > xDummy;
    return getFieldsByCommandDescriptor( _rxConn, CommandType::TABLE, _rName, xDummy );
}

Reference< XNameAccess> getPrimaryKeyColumns_throw(const Any& i_aTable)
{
    const Reference< XPropertySet > xTable(i_aTable,UNO_QUERY_THROW);
    return getPrimaryKeyColumns_throw(xTable);
}

Reference< XNameAccess> getPrimaryKeyColumns_throw(const Reference< XPropertySet >& i_xTable)
{
    Reference<XNameAccess> xKeyColumns;
    const Reference<XKeysSupplier> xKeySup(i_xTable,UNO_QUERY);
    if ( xKeySup.is() )
    {
        const Reference<XIndexAccess> xKeys = xKeySup->getKeys();
        if ( xKeys.is() )
        {
            ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
            const OUString sPropName = rPropMap.getNameByIndex(PROPERTY_ID_TYPE);
            Reference<XPropertySet> xProp;
            const sal_Int32 nCount = xKeys->getCount();
            for(sal_Int32 i = 0;i< nCount;++i)
            {
                xProp.set(xKeys->getByIndex(i),UNO_QUERY_THROW);
                if ( xProp.is() )
                {
                    sal_Int32 nKeyType = 0;
                    xProp->getPropertyValue(sPropName) >>= nKeyType;
                    if(KeyType::PRIMARY == nKeyType)
                    {
                        const Reference<XColumnsSupplier> xKeyColsSup(xProp,UNO_QUERY_THROW);
                        xKeyColumns = xKeyColsSup->getColumns();
                        break;
                    }
                }
            }
        }
    }

    return xKeyColumns;
}

namespace
{
    enum FieldLookupState
    {
        HANDLE_TABLE, HANDLE_QUERY, HANDLE_SQL, RETRIEVE_OBJECT, RETRIEVE_COLUMNS, DONE, FAILED
    };
}

Reference< XNameAccess > getFieldsByCommandDescriptor( const Reference< XConnection >& _rxConnection,
    const sal_Int32 _nCommandType, const OUString& _rCommand,
    Reference< XComponent >& _rxKeepFieldsAlive, SQLExceptionInfo* _pErrorInfo )
{
    OSL_PRECOND( _rxConnection.is(), "::dbtools::getFieldsByCommandDescriptor: invalid connection!" );
    OSL_PRECOND( ( CommandType::TABLE == _nCommandType ) || ( CommandType::QUERY == _nCommandType ) || ( CommandType::COMMAND == _nCommandType ),
        "::dbtools::getFieldsByCommandDescriptor: invalid command type!" );
    OSL_PRECOND( !_rCommand.isEmpty(), "::dbtools::getFieldsByCommandDescriptor: invalid command (empty)!" );

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
                    OUString sStatementToExecute( _rCommand );

                    // well, the main problem here is to handle statements which contain a parameter
                    // If we would simply execute a parametrized statement, then this will fail because
                    // we cannot supply any parameter values.
                    // Thus, we try to analyze the statement, and to append a WHERE 0=1 filter criterion
                    // This should cause every driver to not really execute the statement, but to return
                    // an empty result set with the proper structure. We then can use this result set
                    // to retrieve the columns.

                    try
                    {
                        Reference< XMultiServiceFactory > xComposerFac( _rxConnection, UNO_QUERY );

                        if ( xComposerFac.is() )
                        {
                            Reference< XSingleSelectQueryComposer > xComposer(xComposerFac->createInstance("com.sun.star.sdb.SingleSelectQueryComposer"),UNO_QUERY);
                            if ( xComposer.is() )
                            {
                                xComposer->setQuery( sStatementToExecute );

                                // Now set the filter to a dummy restriction which will result in an empty
                                // result set.
                                xComposer->setFilter( "0=1" );
                                sStatementToExecute = xComposer->getQuery( );
                            }
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
                    _rxKeepFieldsAlive.set(xStatement, css::uno::UNO_QUERY);

                    // set the "MaxRows" to 0. This is just in case our attempt to append a 0=1 filter
                    // failed - in this case, the MaxRows restriction should at least ensure that there
                    // is no data returned (which would be potentially expensive)
                    Reference< XPropertySet > xStatementProps( xStatement,UNO_QUERY );
                    try
                    {
                        if ( xStatementProps.is() )
                            xStatementProps->setPropertyValue( "MaxRows",  makeAny( sal_Int32( 0 ) ) );
                    }
                    catch( const Exception& )
                    {
                        OSL_FAIL( "::dbtools::getFieldsByCommandDescriptor: could not set the MaxRows!" );
                        // oh damn. Not much of a chance to recover, we will no retrieve the complete
                        // full blown result set
                    }

                    xSupplyColumns.set(xStatement->executeQuery(), css::uno::UNO_QUERY);
                    // this should have given us a result set which does not contain any data, but
                    // the structural information we need

                    // so the next state is to get the columns
                    eState = RETRIEVE_COLUMNS;
                }
                break;

                default:
                    OSL_FAIL( "::dbtools::getFieldsByCommandDescriptor: oops! unhandled state here!" );
                    eState = FAILED;
            }
        }
    }
    catch( const SQLContext& e ) { if ( _pErrorInfo ) *_pErrorInfo = SQLExceptionInfo( e ); }
    catch( const SQLWarning& e ) { if ( _pErrorInfo ) *_pErrorInfo = SQLExceptionInfo( e ); }
    catch( const SQLException& e ) { if ( _pErrorInfo ) *_pErrorInfo = SQLExceptionInfo( e ); }
    catch( const Exception& )
    {
        OSL_FAIL( "::dbtools::getFieldsByCommandDescriptor: caught an exception while retrieving the fields!" );
    }

    return xFields;
}

Sequence< OUString > getFieldNamesByCommandDescriptor( const Reference< XConnection >& _rxConnection,
    const sal_Int32 _nCommandType, const OUString& _rCommand,
    SQLExceptionInfo* _pErrorInfo )
{
    // get the container for the fields
    Reference< XComponent > xKeepFieldsAlive;
    Reference< XNameAccess > xFieldContainer = getFieldsByCommandDescriptor( _rxConnection, _nCommandType, _rCommand, xKeepFieldsAlive, _pErrorInfo );

    // get the names of the fields
    Sequence< OUString > aNames;
    if ( xFieldContainer.is() )
        aNames = xFieldContainer->getElementNames();

    // clean up any temporary objects which have been created
    disposeComponent( xKeepFieldsAlive );

    // outta here
    return aNames;
}

SQLException prependErrorInfo( const SQLException& _rChainedException, const Reference< XInterface >& _rxContext,
    const OUString& _rAdditionalError, const StandardSQLState _eSQLState )
{
    return SQLException( _rAdditionalError, _rxContext,
        _eSQLState == StandardSQLState::ERROR_UNSPECIFIED ? OUString() : getStandardSQLState( _eSQLState ),
        0, makeAny( _rChainedException ) );
}

namespace
{
    struct NameComponentSupport
    {
        const bool  bCatalogs;
        const bool  bSchemas;

        NameComponentSupport( const bool _bCatalogs, const bool _bSchemas )
            :bCatalogs( _bCatalogs )
            ,bSchemas( _bSchemas )
        {
        }
    };

    NameComponentSupport lcl_getNameComponentSupport( const Reference< XDatabaseMetaData >& _rxMetaData, EComposeRule _eComposeRule )
    {
        OSL_PRECOND( _rxMetaData.is(), "lcl_getNameComponentSupport: invalid meta data!" );

        FMetaDataSupport pCatalogCall = &XDatabaseMetaData::supportsCatalogsInDataManipulation;
        FMetaDataSupport pSchemaCall = &XDatabaseMetaData::supportsSchemasInDataManipulation;
        bool bIgnoreMetaData = false;

        switch ( _eComposeRule )
        {
            case EComposeRule::InTableDefinitions:
                pCatalogCall = &XDatabaseMetaData::supportsCatalogsInTableDefinitions;
                pSchemaCall = &XDatabaseMetaData::supportsSchemasInTableDefinitions;
                break;
            case EComposeRule::InIndexDefinitions:
                pCatalogCall = &XDatabaseMetaData::supportsCatalogsInIndexDefinitions;
                pSchemaCall = &XDatabaseMetaData::supportsSchemasInIndexDefinitions;
                break;
            case EComposeRule::InProcedureCalls:
                pCatalogCall = &XDatabaseMetaData::supportsCatalogsInProcedureCalls;
                pSchemaCall = &XDatabaseMetaData::supportsSchemasInProcedureCalls;
                break;
            case EComposeRule::InPrivilegeDefinitions:
                pCatalogCall = &XDatabaseMetaData::supportsCatalogsInPrivilegeDefinitions;
                pSchemaCall = &XDatabaseMetaData::supportsSchemasInPrivilegeDefinitions;
                break;
            case EComposeRule::Complete:
                bIgnoreMetaData = true;
                break;
            case EComposeRule::InDataManipulation:
                // already properly set above
                break;
        }
        return NameComponentSupport(
            bIgnoreMetaData || (_rxMetaData.get()->*pCatalogCall)(),
            bIgnoreMetaData || (_rxMetaData.get()->*pSchemaCall)()
        );
    }
}

static OUString impl_doComposeTableName( const Reference< XDatabaseMetaData >& _rxMetaData,
                const OUString& _rCatalog, const OUString& _rSchema, const OUString& _rName,
                bool _bQuote, EComposeRule _eComposeRule )
{
    OSL_ENSURE(_rxMetaData.is(), "impl_doComposeTableName : invalid meta data !");
    if ( !_rxMetaData.is() )
        return OUString();
    OSL_ENSURE(!_rName.isEmpty(), "impl_doComposeTableName : at least the name should be non-empty !");

    const OUString sQuoteString = _rxMetaData->getIdentifierQuoteString();
    const NameComponentSupport aNameComps( lcl_getNameComponentSupport( _rxMetaData, _eComposeRule ) );

    OUStringBuffer aComposedName;

    OUString sCatalogSep;
    bool bCatlogAtStart = true;
    if ( !_rCatalog.isEmpty() && aNameComps.bCatalogs )
    {
        sCatalogSep     = _rxMetaData->getCatalogSeparator();
        bCatlogAtStart  = _rxMetaData->isCatalogAtStart();

        if ( bCatlogAtStart && !sCatalogSep.isEmpty())
        {
            aComposedName.append( _bQuote ? quoteName( sQuoteString, _rCatalog ) : _rCatalog );
            aComposedName.append( sCatalogSep );
        }
    }

    if ( !_rSchema.isEmpty() && aNameComps.bSchemas )
    {
        aComposedName.append( _bQuote ? quoteName( sQuoteString, _rSchema ) : _rSchema );
        aComposedName.append( "." );
    }

    aComposedName.append( _bQuote ? quoteName( sQuoteString, _rName ) : _rName );

    if  (   !_rCatalog.isEmpty()
        &&  !bCatlogAtStart
        &&  !sCatalogSep.isEmpty()
        &&  aNameComps.bCatalogs
        )
    {
        aComposedName.append( sCatalogSep );
        aComposedName.append( _bQuote ? quoteName( sQuoteString, _rCatalog ) : _rCatalog );
    }

    return aComposedName.makeStringAndClear();
}

OUString quoteTableName(const Reference< XDatabaseMetaData>& _rxMeta
                               , const OUString& _rName
                               , EComposeRule _eComposeRule)
{
    OUString sCatalog, sSchema, sTable;
    qualifiedNameComponents(_rxMeta,_rName,sCatalog,sSchema,sTable,_eComposeRule);
    return impl_doComposeTableName( _rxMeta, sCatalog, sSchema, sTable, true, _eComposeRule );
}

void qualifiedNameComponents(const Reference< XDatabaseMetaData >& _rxConnMetaData, const OUString& _rQualifiedName, OUString& _rCatalog, OUString& _rSchema, OUString& _rName,EComposeRule _eComposeRule)
{
    OSL_ENSURE(_rxConnMetaData.is(), "QualifiedNameComponents : invalid meta data!");

    NameComponentSupport aNameComps( lcl_getNameComponentSupport( _rxConnMetaData, _eComposeRule ) );

    OUString sSeparator = _rxConnMetaData->getCatalogSeparator();

    OUString sName(_rQualifiedName);
    // do we have catalogs ?
    if ( aNameComps.bCatalogs )
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
            // Catalogue name at the end
            sal_Int32 nIndex = sName.lastIndexOf(sSeparator);
            if (-1 != nIndex)
            {
                _rCatalog = sName.copy(nIndex + 1);
                sName = sName.copy(0, nIndex);
            }
        }
    }

    if ( aNameComps.bSchemas )
    {
        sal_Int32 nIndex = sName.indexOf('.');
        //  OSL_ENSURE(-1 != nIndex, "QualifiedNameComponents : no schema separator!");
        if ( nIndex != -1 )
            _rSchema = sName.copy(0, nIndex);
        sName = sName.copy(nIndex + 1);
    }

    _rName = sName;
}

Reference< XNumberFormatsSupplier> getNumberFormats(
            const Reference< XConnection>& _rxConn,
            bool _bAlloweDefault,
            const Reference< XComponentContext>& _rxContext)
{
    // ask the parent of the connection (should be an DatabaseAccess)
    Reference< XNumberFormatsSupplier> xReturn;
    Reference< XChild> xConnAsChild(_rxConn, UNO_QUERY);
    OUString sPropFormatsSupplier( "NumberFormatsSupplier" );
    if (xConnAsChild.is())
    {
        Reference< XPropertySet> xConnParentProps(xConnAsChild->getParent(), UNO_QUERY);
        if (xConnParentProps.is() && hasProperty(sPropFormatsSupplier, xConnParentProps))
            xConnParentProps->getPropertyValue(sPropFormatsSupplier) >>= xReturn;
    }
    else if(_bAlloweDefault && _rxContext.is())
    {
        xReturn = NumberFormatsSupplier::createWithDefaultLocale( _rxContext );
    }
    return xReturn;
}

void TransferFormComponentProperties(
            const Reference< XPropertySet>& xOldProps,
            const Reference< XPropertySet>& xNewProps,
            const Locale& _rLocale)
{
try
{
    OSL_ENSURE( xOldProps.is() && xNewProps.is(), "TransferFormComponentProperties: invalid source/dest!" );
    if ( !xOldProps.is() || !xNewProps.is() )
        return;

    // First we copy all the Props, that are available in source and target and have the same description
    Reference< XPropertySetInfo> xOldInfo( xOldProps->getPropertySetInfo());
    Reference< XPropertySetInfo> xNewInfo( xNewProps->getPropertySetInfo());

    Sequence< Property> aOldProperties = xOldInfo->getProperties();
    Sequence< Property> aNewProperties = xNewInfo->getProperties();
    int nNewLen = aNewProperties.getLength();

    Property* pOldProps = aOldProperties.getArray();
    Property* pNewProps = aNewProperties.getArray();

    OUString sPropFormatsSupplier("FormatsSupplier");
    OUString sPropCurrencySymbol("CurrencySymbol");
    OUString sPropDecimals("Decimals");
    OUString sPropEffectiveMin("EffectiveMin");
    OUString sPropEffectiveMax("EffectiveMax");
    OUString sPropEffectiveDefault("EffectiveDefault");
    OUString sPropDefaultText("DefaultText");
    OUString sPropDefaultDate("DefaultDate");
    OUString sPropDefaultTime("DefaultTime");
    OUString sPropValueMin("ValueMin");
    OUString sPropValueMax("ValueMax");
    OUString sPropDecimalAccuracy("DecimalAccuracy");
    OUString sPropClassId("ClassId");
    OUString sFormattedServiceName( "com.sun.star.form.component.FormattedField" );

    for (sal_Int32 i=0; i<aOldProperties.getLength(); ++i)
    {
        if ( pOldProps[i].Name != "DefaultControl" && pOldProps[i].Name != "LabelControl" )
        {
            // binary search
            Property* pResult = std::lower_bound(
                pNewProps, pNewProps + nNewLen, pOldProps[i], ::comphelper::PropertyCompareByName());

            if (   ( pResult != aNewProperties.end() )
                && ( pResult->Name == pOldProps[i].Name )
                && ( (pResult->Attributes & PropertyAttribute::READONLY) == 0 )
                && ( pResult->Type.equals(pOldProps[i].Type)) )
            {   // Attributes match and the property is not read-only
                try
                {
                    xNewProps->setPropertyValue(pResult->Name, xOldProps->getPropertyValue(pResult->Name));
                }
                catch(IllegalArgumentException const & exc)
                {
                    SAL_WARN( "connectivity.commontools", "TransferFormComponentProperties : could not transfer the value for property \""
                                << pResult->Name << "\" " << exc.Message);
                }
            }
        }
    }

    // for formatted fields (either old or new) we have some special treatments
    Reference< XServiceInfo > xSI( xOldProps, UNO_QUERY );
    bool bOldIsFormatted = xSI.is() && xSI->supportsService( sFormattedServiceName );
    xSI.set( xNewProps, UNO_QUERY );
    bool bNewIsFormatted = xSI.is() && xSI->supportsService( sFormattedServiceName );

    if (!bOldIsFormatted && !bNewIsFormatted)
        return; // nothing to do

    if (bOldIsFormatted && bNewIsFormatted)
        // if both fields are formatted we do no conversions
        return;

    if (bOldIsFormatted)
    {
        // get some properties from the selected format and put them in the new Set
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
                        // If the source value hasn't been set then don't copy it
                        // so we don't overwrite the default value
                        xNewProps->setPropertyValue(sPropCurrencySymbol, aVal);
                }
                if (hasProperty(sPropDecimals, xFormat) && hasProperty(sPropDecimals, xNewProps))
                    xNewProps->setPropertyValue(sPropDecimals, xFormat->getPropertyValue(sPropDecimals));
            }
        }

        // a potential Min-Max-Conversion
        Any aEffectiveMin( xOldProps->getPropertyValue(sPropEffectiveMin) );
        if (aEffectiveMin.hasValue())
        {   // Unlike the ValueMin the EffectiveMin can be void
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

        // then we can still convert and copy the default values
        Any aEffectiveDefault( xOldProps->getPropertyValue(sPropEffectiveDefault) );
        if (aEffectiveDefault.hasValue())
        {
            bool bIsString = aEffectiveDefault.getValueType().getTypeClass() == TypeClass_STRING;
            OSL_ENSURE(bIsString || aEffectiveDefault.getValueType().getTypeClass() == TypeClass_DOUBLE,
                "TransferFormComponentProperties : invalid property type !");
                // The Effective-Properties should always be void or string or double ....

            if (hasProperty(sPropDefaultDate, xNewProps) && !bIsString)
            {   // (to convert a OUString into a date will not always succeed, because it might be bound to a text-column,
                // but we can work with a double)
                Date aDate = DBTypeConversion::toDate(getDouble(aEffectiveDefault));
                xNewProps->setPropertyValue(sPropDefaultDate, makeAny(aDate));
            }

            if (hasProperty(sPropDefaultTime, xNewProps) && !bIsString)
            {   // Completely analogous to time
                css::util::Time aTime = DBTypeConversion::toTime(getDouble(aEffectiveDefault));
                xNewProps->setPropertyValue(sPropDefaultTime, makeAny(aTime));
            }

            if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE), xNewProps) && !bIsString)
            {   // Here we can simply pass the double
                xNewProps->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE), aEffectiveDefault);
            }

            if (hasProperty(sPropDefaultText, xNewProps) && bIsString)
            {   // and here the OUString
                xNewProps->setPropertyValue(sPropDefaultText, aEffectiveDefault);
            }

            // nyi: The translation between doubles and OUString would offer more alternatives
        }
    }

    // The other direction: the new Control shall be formatted
    if (bNewIsFormatted)
    {
        // first the formatting
        // we can't set a Supplier, so the new Set must bring one in
        Reference< XNumberFormatsSupplier> xSupplier;
        xNewProps->getPropertyValue(sPropFormatsSupplier) >>= xSupplier;
        if (xSupplier.is())
        {
            Reference< XNumberFormats> xFormats(xSupplier->getNumberFormats());

            // Set number of decimals
            sal_Int16 nDecimals = 2;
            if (hasProperty(sPropDecimalAccuracy, xOldProps))
                xOldProps->getPropertyValue(sPropDecimalAccuracy) >>= nDecimals;

            // base format (depending on the ClassId of the old Set)
            sal_Int32 nBaseKey = 0;
            if (hasProperty(sPropClassId, xOldProps))
            {
                Reference< XNumberFormatTypes> xTypeList(xFormats, UNO_QUERY);
                if (xTypeList.is())
                {
                    sal_Int16 nClassId = 0;
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

            // With this we can generate a new format ...
            OUString sNewFormat = xFormats->generateFormat(nBaseKey, _rLocale, false, false, nDecimals, 0);
            // No thousands separator, negative numbers are not in red, no leading zeros

            // ... and add at FormatsSupplier (if needed)
            sal_Int32 nKey = xFormats->queryKey(sNewFormat, _rLocale, false);
            if (nKey == (sal_Int32)-1)
            {   // not added yet in my formatter ...
                nKey = xFormats->addNew(sNewFormat, _rLocale);
            }

            xNewProps->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FORMATKEY), makeAny(nKey));
        }

        // min-/max-Value
        Any aNewMin, aNewMax;
        if (hasProperty(sPropValueMin, xOldProps))
            aNewMin = xOldProps->getPropertyValue(sPropValueMin);
        if (hasProperty(sPropValueMax, xOldProps))
            aNewMax = xOldProps->getPropertyValue(sPropValueMax);
        xNewProps->setPropertyValue(sPropEffectiveMin, aNewMin);
        xNewProps->setPropertyValue(sPropEffectiveMax, aNewMax);

        // Default-Value
        Any aNewDefault;
        if (hasProperty(sPropDefaultDate, xOldProps))
        {
            Any aDate( xOldProps->getPropertyValue(sPropDefaultDate) );
            if (aDate.hasValue())
                aNewDefault <<= DBTypeConversion::toDouble(*o3tl::doAccess<Date>(aDate));
        }

        if (hasProperty(sPropDefaultTime, xOldProps))
        {
            Any aTime( xOldProps->getPropertyValue(sPropDefaultTime) );
            if (aTime.hasValue())
                aNewDefault <<= DBTypeConversion::toDouble(*o3tl::doAccess<Time>(aTime));
        }

        // double or OUString will be copied directly
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
    OSL_FAIL( "TransferFormComponentProperties: caught an exception!" );
}
}

bool canInsert(const Reference< XPropertySet>& _rxCursorSet)
{
    return (_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue("Privileges")) & Privilege::INSERT) != 0);
}

bool canUpdate(const Reference< XPropertySet>& _rxCursorSet)
{
    return (_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue("Privileges")) & Privilege::UPDATE) != 0);
}

bool canDelete(const Reference< XPropertySet>& _rxCursorSet)
{
    return (_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue("Privileges")) & Privilege::DELETE) != 0);
}

Reference< XDataSource> findDataSource(const Reference< XInterface >& _xParent)
{
    Reference< XOfficeDatabaseDocument> xDatabaseDocument(_xParent, UNO_QUERY);
    Reference< XDataSource> xDataSource;
    if ( xDatabaseDocument.is() )
        xDataSource = xDatabaseDocument->getDataSource();
    if ( !xDataSource.is() )
        xDataSource.set(_xParent, UNO_QUERY);
    if (!xDataSource.is())
    {
        Reference< XChild> xChild(_xParent, UNO_QUERY);
        if ( xChild.is() )
            xDataSource = findDataSource(xChild->getParent());
    }
    return xDataSource;
}

Reference< XSingleSelectQueryComposer > getComposedRowSetStatement( const Reference< XPropertySet >& _rxRowSet, const Reference< XComponentContext >& _rxContext )
{
    Reference< XSingleSelectQueryComposer > xComposer;
    try
    {
        Reference< XConnection> xConn = connectRowset( Reference< XRowSet >( _rxRowSet, UNO_QUERY ), _rxContext, true );
        if ( xConn.is() )       // implies _rxRowSet.is()
        {
            // build the statement the row set is based on (can't use the ActiveCommand property of the set
            // as this reflects the status after the last execute, not the currently set properties)

            sal_Int32 nCommandType = CommandType::COMMAND;
            OUString sCommand;
            bool bEscapeProcessing = false;

            OSL_VERIFY( _rxRowSet->getPropertyValue("CommandType") >>= nCommandType      );
            OSL_VERIFY( _rxRowSet->getPropertyValue("Command") >>= sCommand          );
            OSL_VERIFY( _rxRowSet->getPropertyValue("EscapeProcessing") >>= bEscapeProcessing );

            StatementComposer aComposer( xConn, sCommand, nCommandType, bEscapeProcessing );
            // append sort
            aComposer.setOrder( getString( _rxRowSet->getPropertyValue("Order") ) );

            // append filter
            bool bApplyFilter = true;
            _rxRowSet->getPropertyValue("ApplyFilter") >>= bApplyFilter;
            if ( bApplyFilter )
                aComposer.setFilter( getString( _rxRowSet->getPropertyValue("Filter") ) );

            aComposer.getQuery();

            xComposer = aComposer.getComposer();
            aComposer.setDisposeComposer( false );
        }
    }
    catch( const SQLException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return xComposer;
}

Reference< XSingleSelectQueryComposer > getCurrentSettingsComposer(
                const Reference< XPropertySet>& _rxRowSetProps,
                const Reference< XComponentContext>& _rxContext)
{
    Reference< XSingleSelectQueryComposer > xReturn;
    try
    {
        xReturn = getComposedRowSetStatement( _rxRowSetProps, _rxContext );
    }
    catch( const SQLException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        OSL_FAIL( "::getCurrentSettingsComposer : caught an exception !" );
    }

    return xReturn;
}

OUString composeTableName( const Reference< XDatabaseMetaData >& _rxMetaData,
                        const OUString& _rCatalog,
                        const OUString& _rSchema,
                        const OUString& _rName,
                        bool _bQuote,
                        EComposeRule _eComposeRule)
{
    return impl_doComposeTableName( _rxMetaData, _rCatalog, _rSchema, _rName, _bQuote, _eComposeRule );
}

OUString composeTableNameForSelect( const Reference< XConnection >& _rxConnection,
    const OUString& _rCatalog, const OUString& _rSchema, const OUString& _rName )
{
    bool bUseCatalogInSelect = isDataSourcePropertyEnabled( _rxConnection, "UseCatalogInSelect", true );
    bool bUseSchemaInSelect = isDataSourcePropertyEnabled( _rxConnection, "UseSchemaInSelect", true );

    return impl_doComposeTableName(
        _rxConnection->getMetaData(),
        bUseCatalogInSelect ? _rCatalog : OUString(),
        bUseSchemaInSelect ? _rSchema : OUString(),
        _rName,
        true,
        EComposeRule::InDataManipulation
    );
}

namespace
{
    void lcl_getTableNameComponents( const Reference<XPropertySet>& _xTable,
        OUString& _out_rCatalog, OUString& _out_rSchema, OUString& _out_rName )
    {
        ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        Reference< XPropertySetInfo > xInfo;
        if (_xTable.is())
            xInfo = _xTable->getPropertySetInfo();
        if (    xInfo.is()
            &&  xInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_NAME)) )
        {
            if (    xInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))
                &&  xInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) )
            {
                _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)) >>= _out_rCatalog;
                _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))  >>= _out_rSchema;
            }
            _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))        >>= _out_rName;
        }
        else
            OSL_FAIL( "::dbtools::lcl_getTableNameComponents: this is no table object!" );
    }
}

OUString composeTableNameForSelect( const Reference< XConnection >& _rxConnection, const Reference<XPropertySet>& _xTable )
{
    OUString sCatalog, sSchema, sName;
    lcl_getTableNameComponents( _xTable, sCatalog, sSchema, sName );

    return composeTableNameForSelect( _rxConnection, sCatalog, sSchema, sName );
}

OUString composeTableName(const Reference<XDatabaseMetaData>& _xMetaData,
                                 const Reference<XPropertySet>& _xTable,
                                 EComposeRule _eComposeRule,
                                 bool _bSuppressCatalog,
                                 bool _bSuppressSchema,
                                 bool _bQuote )
{
    OUString sCatalog, sSchema, sName;
    lcl_getTableNameComponents( _xTable, sCatalog, sSchema, sName );

    return impl_doComposeTableName(
            _xMetaData,
            _bSuppressCatalog ? OUString() : sCatalog,
            _bSuppressSchema ? OUString() : sSchema,
            sName,
            _bQuote,
            _eComposeRule
        );
}

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

OUString createUniqueName( const Sequence< OUString >& _rNames, const OUString& _rBaseName, bool _bStartWithNumber )
{
    std::set< OUString > aUsedNames;
    std::copy(
        _rNames.begin(),
        _rNames.end(),
        std::insert_iterator< std::set< OUString > >( aUsedNames, aUsedNames.end() )
    );

    OUString sName( _rBaseName );
    sal_Int32 nPos = 1;
    if ( _bStartWithNumber )
        sName += OUString::number( nPos );

    while ( aUsedNames.find( sName ) != aUsedNames.end() )
    {
        sName = _rBaseName + OUString::number( ++nPos );
    }
    return sName;
}

OUString createUniqueName(const Reference<XNameAccess>& _rxContainer,const OUString& _rBaseName, bool _bStartWithNumber)
{
    Sequence< OUString > aElementNames;

    OSL_ENSURE( _rxContainer.is(), "createUniqueName: invalid container!" );
    if ( _rxContainer.is() )
        aElementNames = _rxContainer->getElementNames();

    return createUniqueName( aElementNames, _rBaseName, _bStartWithNumber );
}

void showError(const SQLExceptionInfo& _rInfo,
               const Reference< XWindow>& _xParent,
               const Reference< XComponentContext >& _rxContext)
{
    if (_rInfo.isValid())
    {
        try
        {
            Reference< XExecutableDialog > xErrorDialog = ErrorMessageDialog::create( _rxContext, "", _xParent, _rInfo.get() );
            xErrorDialog->execute();
        }
        catch(const Exception&)
        {
            OSL_FAIL("showError: could not display the error message!");
        }
    }
}

bool implUpdateObject(const Reference< XRowUpdate >& _rxUpdatedObject,
    const sal_Int32 _nColumnIndex, const Any& _rValue)
{
    bool bSuccessfullyReRouted = true;
    switch (_rValue.getValueTypeClass())
    {
        case TypeClass_ANY:
        {
            bSuccessfullyReRouted = implUpdateObject(_rxUpdatedObject, _nColumnIndex, _rValue);
        }
        break;

        case TypeClass_VOID:
            _rxUpdatedObject->updateNull(_nColumnIndex);
            break;

        case TypeClass_STRING:
            _rxUpdatedObject->updateString(_nColumnIndex, *o3tl::forceAccess<OUString>(_rValue));
            break;

        case TypeClass_BOOLEAN:
            _rxUpdatedObject->updateBoolean(_nColumnIndex, *o3tl::forceAccess<bool>(_rValue));
            break;

        case TypeClass_BYTE:
            _rxUpdatedObject->updateByte(_nColumnIndex, *o3tl::forceAccess<sal_Int8>(_rValue));
            break;

        case TypeClass_UNSIGNED_SHORT:
        case TypeClass_SHORT:
            _rxUpdatedObject->updateShort(_nColumnIndex, *o3tl::forceAccess<sal_Int16>(_rValue));
            break;

        case TypeClass_CHAR:
            _rxUpdatedObject->updateString(_nColumnIndex,OUString(*o3tl::forceAccess<sal_Unicode>(_rValue)));
            break;

        case TypeClass_UNSIGNED_LONG:
        case TypeClass_LONG:
            _rxUpdatedObject->updateInt(_nColumnIndex, *o3tl::forceAccess<sal_Int32>(_rValue));
            break;

        case TypeClass_HYPER:
        {
            sal_Int64 nValue = 0;
            OSL_VERIFY( _rValue >>= nValue );
            _rxUpdatedObject->updateLong( _nColumnIndex, nValue );
        }
        break;

        case TypeClass_FLOAT:
            _rxUpdatedObject->updateFloat(_nColumnIndex, *o3tl::forceAccess<float>(_rValue));
            break;

        case TypeClass_DOUBLE:
            _rxUpdatedObject->updateDouble(_nColumnIndex, *o3tl::forceAccess<double>(_rValue));
            break;

        case TypeClass_SEQUENCE:
            if (auto s = o3tl::tryAccess<Sequence< sal_Int8 >>(_rValue))
                _rxUpdatedObject->updateBytes(_nColumnIndex, *s);
            else
                bSuccessfullyReRouted = false;
            break;
        case TypeClass_STRUCT:
            if (auto s1 = o3tl::tryAccess<DateTime>(_rValue))
                _rxUpdatedObject->updateTimestamp(_nColumnIndex, *s1);
            else if (auto s2 = o3tl::tryAccess<Date>(_rValue))
                _rxUpdatedObject->updateDate(_nColumnIndex, *s2);
            else if (auto s3 = o3tl::tryAccess<Time>(_rValue))
                _rxUpdatedObject->updateTime(_nColumnIndex, *s3);
            else
                bSuccessfullyReRouted = false;
            break;

        case TypeClass_INTERFACE:
            if (auto xStream = o3tl::tryAccess<Reference<XInputStream>>(_rValue))
            {
                _rxUpdatedObject->updateBinaryStream(_nColumnIndex, *xStream, (*xStream)->available());
                break;
            }
            SAL_FALLTHROUGH;
        default:
            bSuccessfullyReRouted = false;
    }

    return bSuccessfullyReRouted;
}

bool implSetObject( const Reference< XParameters >& _rxParameters,
                        const sal_Int32 _nColumnIndex, const Any& _rValue)
{
    bool bSuccessfullyReRouted = true;
    switch (_rValue.getValueTypeClass())
    {
        case TypeClass_UNSIGNED_HYPER:
        {
            sal_uInt64 nValue = 0;
            OSL_VERIFY( _rValue >>= nValue );
            _rxParameters->setString(_nColumnIndex, OUString::number(nValue));
        }
        break;

        case TypeClass_UNSIGNED_LONG:
        case TypeClass_HYPER:
        {
            sal_Int64 nValue = 0;
            OSL_VERIFY( _rValue >>= nValue );
            _rxParameters->setLong( _nColumnIndex, nValue );
        }
        break;

        case TypeClass_ANY:
        {
            bSuccessfullyReRouted = implSetObject(_rxParameters, _nColumnIndex, _rValue);
        }
        break;

        case TypeClass_VOID:
            _rxParameters->setNull(_nColumnIndex,DataType::VARCHAR);
            break;

        case TypeClass_STRING:
            _rxParameters->setString(_nColumnIndex, *o3tl::forceAccess<OUString>(_rValue));
            break;

        case TypeClass_BOOLEAN:
            _rxParameters->setBoolean(_nColumnIndex, *o3tl::forceAccess<bool>(_rValue));
            break;

        case TypeClass_BYTE:
            _rxParameters->setByte(_nColumnIndex, *o3tl::forceAccess<sal_Int8>(_rValue));
            break;

        case TypeClass_SHORT:
            _rxParameters->setShort(_nColumnIndex, *o3tl::forceAccess<sal_Int16>(_rValue));
            break;

        case TypeClass_CHAR:
            _rxParameters->setString(_nColumnIndex, OUString(*o3tl::forceAccess<sal_Unicode>(_rValue)));
            break;

        case TypeClass_UNSIGNED_SHORT:
        case TypeClass_LONG:
        {
            sal_Int32 nValue = 0;
            OSL_VERIFY( _rValue >>= nValue );
            _rxParameters->setInt(_nColumnIndex, nValue);
            break;
        }

        case TypeClass_FLOAT:
            _rxParameters->setFloat(_nColumnIndex, *o3tl::forceAccess<float>(_rValue));
            break;

        case TypeClass_DOUBLE:
            _rxParameters->setDouble(_nColumnIndex, *o3tl::forceAccess<double>(_rValue));
            break;

        case TypeClass_SEQUENCE:
            if (auto s = o3tl::tryAccess<Sequence< sal_Int8 >>(_rValue))
            {
                _rxParameters->setBytes(_nColumnIndex, *s);
            }
            else
                bSuccessfullyReRouted = false;
            break;
        case TypeClass_STRUCT:
            if (auto s1 = o3tl::tryAccess<DateTime>(_rValue))
                _rxParameters->setTimestamp(_nColumnIndex, *s1);
            else if (auto s2 = o3tl::tryAccess<Date>(_rValue))
                _rxParameters->setDate(_nColumnIndex, *s2);
            else if (auto s3 = o3tl::tryAccess<Time>(_rValue))
                _rxParameters->setTime(_nColumnIndex, *s3);
            else
                bSuccessfullyReRouted = false;
            break;

        case TypeClass_INTERFACE:
            if (_rValue.getValueType() == cppu::UnoType<XInputStream>::get())
            {
                Reference< XInputStream >  xStream;
                _rValue >>= xStream;
                _rxParameters->setBinaryStream(_nColumnIndex, xStream, xStream->available());
                break;
            }
            SAL_FALLTHROUGH;
        default:
            bSuccessfullyReRouted = false;

    }

    return bSuccessfullyReRouted;
}

namespace
{
    class OParameterWrapper : public ::cppu::WeakImplHelper< XIndexAccess >
    {
        std::vector<bool, std::allocator<bool> >       m_aSet;
        Reference<XIndexAccess> m_xSource;
    public:
        OParameterWrapper(const std::vector<bool, std::allocator<bool> >& _aSet,const Reference<XIndexAccess>& _xSource) : m_aSet(_aSet),m_xSource(_xSource){}
    private:
        // css::container::XElementAccess
        virtual Type SAL_CALL getElementType() override
        {
            return m_xSource->getElementType();
        }
        virtual sal_Bool SAL_CALL hasElements(  ) override
        {
            if ( m_aSet.empty() )
                return m_xSource->hasElements();
            return std::count(m_aSet.begin(),m_aSet.end(),false) != 0;
        }
        // css::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount(  ) override
        {
            if ( m_aSet.empty() )
                return m_xSource->getCount();
            return std::count(m_aSet.begin(),m_aSet.end(),false);
        }
        virtual Any SAL_CALL getByIndex( sal_Int32 Index ) override
        {
            if ( m_aSet.empty() )
                return m_xSource->getByIndex(Index);
            if ( m_aSet.size() < (size_t)Index )
                throw IndexOutOfBoundsException();

            std::vector<bool, std::allocator<bool> >::const_iterator aIter = m_aSet.begin();
            std::vector<bool, std::allocator<bool> >::const_iterator aEnd = m_aSet.end();
            sal_Int32 i = 0;
            sal_Int32 nParamPos = -1;
            for(; aIter != aEnd && i <= Index; ++aIter)
            {
                ++nParamPos;
                if ( !*aIter )
                {
                    ++i;
                }
            }
            return m_xSource->getByIndex(nParamPos);
        }
    };
}

void askForParameters(const Reference< XSingleSelectQueryComposer >& _xComposer,
                      const Reference<XParameters>& _xParameters,
                      const Reference< XConnection>& _xConnection,
                      const Reference< XInteractionHandler >& _rxHandler,
                      const std::vector<bool, std::allocator<bool> >& _aParametersSet)
{
    OSL_ENSURE(_xComposer.is(),"dbtools::askForParameters XSQLQueryComposer is null!");
    OSL_ENSURE(_xParameters.is(),"dbtools::askForParameters XParameters is null!");
    OSL_ENSURE(_xConnection.is(),"dbtools::askForParameters XConnection is null!");
    OSL_ENSURE(_rxHandler.is(),"dbtools::askForParameters XInteractionHandler is null!");

    // we have to set this here again because getCurrentSettingsComposer can force a setpropertyvalue
    Reference<XParametersSupplier>  xParameters(_xComposer, UNO_QUERY);

    Reference<XIndexAccess>  xParamsAsIndicies = xParameters.is() ? xParameters->getParameters() : Reference<XIndexAccess>();
    sal_Int32 nParamCount = xParamsAsIndicies.is() ? xParamsAsIndicies->getCount() : 0;
    std::vector<bool, std::allocator<bool> > aNewParameterSet( _aParametersSet );
    if ( nParamCount && std::count(aNewParameterSet.begin(),aNewParameterSet.end(),true) != nParamCount )
    {
        static const OUString PROPERTY_NAME(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME));
        aNewParameterSet.resize(nParamCount ,false);
        typedef std::map< OUString, std::vector<sal_Int32> > TParameterPositions;
        TParameterPositions aParameterNames;
        for(sal_Int32 i = 0; i < nParamCount; ++i)
        {
            Reference<XPropertySet> xParam(xParamsAsIndicies->getByIndex(i),UNO_QUERY);
            OUString sName;
            xParam->getPropertyValue(PROPERTY_NAME) >>= sName;

            TParameterPositions::const_iterator aFind = aParameterNames.find(sName);
            if ( aFind != aParameterNames.end() )
                aNewParameterSet[i] = true;
            aParameterNames[sName].push_back(i+1);
        }
        // build an interaction request
        // two continuations (Ok and Cancel)
        OInteractionAbort* pAbort = new OInteractionAbort;
        OParameterContinuation* pParams = new OParameterContinuation;
        // the request
        ParametersRequest aRequest;
        Reference<XIndexAccess> xWrappedParameters = new OParameterWrapper(aNewParameterSet,xParamsAsIndicies);
        aRequest.Parameters = xWrappedParameters;
        aRequest.Connection = _xConnection;
        OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aRequest));
        Reference< XInteractionRequest > xRequest(pRequest);
        // some knittings
        pRequest->addContinuation(pAbort);
        pRequest->addContinuation(pParams);

        // execute the request
        _rxHandler->handle(xRequest);

        if (!pParams->wasSelected())
        {
            // canceled by the user (i.e. (s)he canceled the dialog)
            RowSetVetoException e;
            e.ErrorCode = ParameterInteractionCancelled;
            throw e;
        }

        // now transfer the values from the continuation object to the parameter columns
        Sequence< PropertyValue > aFinalValues = pParams->getValues();
        const PropertyValue* pFinalValues = aFinalValues.getConstArray();
        for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
        {
            Reference< XPropertySet > xParamColumn(xWrappedParameters->getByIndex(i),UNO_QUERY);
            if (xParamColumn.is())
            {
                OUString sName;
                xParamColumn->getPropertyValue(PROPERTY_NAME) >>= sName;
                OSL_ENSURE(sName == pFinalValues->Name, "::dbaui::askForParameters: inconsistent parameter names!");

                // determine the field type and ...
                sal_Int32 nParamType = 0;
                xParamColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)) >>= nParamType;
                // ... the scale of the parameter column
                sal_Int32 nScale = 0;
                if (hasProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE), xParamColumn))
                    xParamColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)) >>= nScale;
                    // (the index of the parameters is one-based)
                TParameterPositions::const_iterator aFind = aParameterNames.find(pFinalValues->Name);
                std::vector<sal_Int32>::const_iterator aIterPos = aFind->second.begin();
                std::vector<sal_Int32>::const_iterator aEndPos = aFind->second.end();
                for(;aIterPos != aEndPos;++aIterPos)
                {
                    if ( _aParametersSet.empty() || !_aParametersSet[(*aIterPos)-1] )
                    {
                        _xParameters->setObjectWithInfo(*aIterPos, pFinalValues->Value, nParamType, nScale);
                    }
                }
            }
        }
    }
}

void setObjectWithInfo(const Reference<XParameters>& _xParams,
                       sal_Int32 parameterIndex,
                       const Any& x,
                       sal_Int32 sqlType,
                       sal_Int32 scale)
{
    ORowSetValue aVal;
    aVal.fill(x);
    setObjectWithInfo(_xParams,parameterIndex,aVal,sqlType,scale);
}

void setObjectWithInfo(const Reference<XParameters>& _xParams,
                       sal_Int32 parameterIndex,
                       const ::connectivity::ORowSetValue& _rValue,
                       sal_Int32 sqlType,
                       sal_Int32 scale)
{
    if ( _rValue.isNull() )
        _xParams->setNull(parameterIndex,sqlType);
    else
    {
        switch(sqlType)
        {
            case DataType::DECIMAL:
            case DataType::NUMERIC:
                _xParams->setObjectWithInfo(parameterIndex,_rValue.makeAny(),sqlType,scale);
                break;
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                _xParams->setString(parameterIndex,_rValue);
                break;
            case DataType::CLOB:
                {
                    Any x(_rValue.makeAny());
                    OUString sValue;
                    if ( x >>= sValue )
                        _xParams->setString(parameterIndex,sValue);
                    else
                    {
                        Reference< XClob > xClob;
                        if(x >>= xClob)
                            _xParams->setClob(parameterIndex,xClob);
                        else
                        {
                            Reference< css::io::XInputStream > xStream;
                            if(x >>= xStream)
                                _xParams->setCharacterStream(parameterIndex,xStream,xStream->available());
                        }
                    }
                }
                break;
            case DataType::BIGINT:
                if ( _rValue.isSigned() )
                    _xParams->setLong(parameterIndex,_rValue);
                else
                    _xParams->setString(parameterIndex,_rValue);
                break;

            case DataType::FLOAT:
                _xParams->setFloat(parameterIndex,_rValue);
                break;
            case DataType::REAL:
            case DataType::DOUBLE:
                _xParams->setDouble(parameterIndex,_rValue);
                break;
            case DataType::DATE:
                _xParams->setDate(parameterIndex,_rValue);
                break;
            case DataType::TIME:
                _xParams->setTime(parameterIndex,_rValue);
                break;
            case DataType::TIMESTAMP:
                _xParams->setTimestamp(parameterIndex,_rValue);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
                {
                    Any x(_rValue.makeAny());
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
                                Reference< css::io::XInputStream > xBinStream;
                                if(x >>= xBinStream)
                                    _xParams->setBinaryStream(parameterIndex,xBinStream,xBinStream->available());
                            }
                        }
                    }
                }
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                _xParams->setBoolean(parameterIndex,static_cast<bool>(_rValue));
                break;
            case DataType::TINYINT:
                if ( _rValue.isSigned() )
                    _xParams->setByte(parameterIndex,_rValue);
                else
                    _xParams->setShort(parameterIndex,_rValue);
                break;
            case DataType::SMALLINT:
                if ( _rValue.isSigned() )
                    _xParams->setShort(parameterIndex,_rValue);
                else
                    _xParams->setInt(parameterIndex,_rValue);
                break;
            case DataType::INTEGER:
                if ( _rValue.isSigned() )
                    _xParams->setInt(parameterIndex,_rValue);
                else
                    _xParams->setLong(parameterIndex,_rValue);
                break;
            default:
                {
                    ::connectivity::SharedResources aResources;
                    const OUString sError( aResources.getResourceStringWithSubstitution(
                            STR_UNKNOWN_PARA_TYPE,
                            "$position$", OUString::number(parameterIndex)
                         ) );
                    ::dbtools::throwGenericSQLException(sError,nullptr);
                }
        }
    }
}

void getBooleanComparisonPredicate( const OUString& _rExpression, const bool _bValue, const sal_Int32 _nBooleanComparisonMode,
    OUStringBuffer& _out_rSQLPredicate )
{
    switch ( _nBooleanComparisonMode )
    {
    case BooleanComparisonMode::IS_LITERAL:
        _out_rSQLPredicate.append( _rExpression );
        if ( _bValue )
            _out_rSQLPredicate.append( " IS TRUE" );
        else
            _out_rSQLPredicate.append( " IS FALSE" );
        break;

    case BooleanComparisonMode::EQUAL_LITERAL:
        _out_rSQLPredicate.append( _rExpression );
        _out_rSQLPredicate.appendAscii( _bValue ? " = TRUE" : " = FALSE" );
        break;

    case BooleanComparisonMode::ACCESS_COMPAT:
        if ( _bValue )
        {
            _out_rSQLPredicate.append( " NOT ( ( " );
            _out_rSQLPredicate.append( _rExpression );
            _out_rSQLPredicate.append( " = 0 ) OR ( " );
            _out_rSQLPredicate.append( _rExpression );
            _out_rSQLPredicate.append( " IS NULL ) )" );
        }
        else
        {
            _out_rSQLPredicate.append( _rExpression );
            _out_rSQLPredicate.append( " = 0" );
        }
        break;

    case BooleanComparisonMode::EQUAL_INTEGER:
        // fall through
    default:
        _out_rSQLPredicate.append( _rExpression );
        _out_rSQLPredicate.appendAscii( _bValue ? " = 1" : " = 0" );
        break;
    }
}

}   // namespace dbtools

namespace connectivity
{
void release(oslInterlockedCount& _refCount,
             ::cppu::OBroadcastHelper& rBHelper,
             Reference< XInterface >& _xInterface,
             css::lang::XComponent* _pObject) throw ()
{
    if (osl_atomic_decrement( &_refCount ) == 0)
    {
        osl_atomic_increment( &_refCount );

        if (!rBHelper.bDisposed && !rBHelper.bInDispose)
        {
            // remember the parent
            Reference< XInterface > xParent;
            {
                ::osl::MutexGuard aGuard( rBHelper.rMutex );
                xParent = _xInterface;
                _xInterface = nullptr;
            }

            // First dispose
            try {
                _pObject->dispose();
            } catch (css::uno::RuntimeException & e) {
                SAL_WARN(
                    "connectivity.commontools",
                    "Caught exception during dispose, " << e.Message);
            }

            // only the alive ref holds the object
            OSL_ASSERT( _refCount == 1 );

            // release the parent in the ~
            if (xParent.is())
            {
                ::osl::MutexGuard aGuard( rBHelper.rMutex );
                _xInterface = xParent;
            }
        }
    }
    else
        osl_atomic_increment( &_refCount );
}

void checkDisposed(bool _bThrow)
{
    if (_bThrow)
        throw DisposedException();

}

OSQLColumns::Vector::const_iterator find(const OSQLColumns::Vector::const_iterator& first,
                                        const OSQLColumns::Vector::const_iterator& last,
                                        const OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
{
    OUString sName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    return find(first,last,sName,_rVal,_rCase);
}

OSQLColumns::Vector::const_iterator findRealName(const OSQLColumns::Vector::const_iterator& first,
                                        const OSQLColumns::Vector::const_iterator& last,
                                        const OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
{
    OUString sRealName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME);
    return find(first,last,sRealName,_rVal,_rCase);
}

OSQLColumns::Vector::const_iterator find(OSQLColumns::Vector::const_iterator first,
                                        const OSQLColumns::Vector::const_iterator& last,
                                        const OUString& _rProp,
                                        const OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase)
{
    while (first != last && !_rCase(getString((*first)->getPropertyValue(_rProp)),_rVal))
        ++first;
    return first;
}

namespace dbase
{
    bool dbfDecodeCharset(rtl_TextEncoding &_out_encoding, sal_uInt8 nType, sal_uInt8 nCodepage)
    {
        switch (nType)
        {
        // dBaseIII header doesn't contain language driver ID
        // See http://dbase.free.fr/tlcharge/structure%20tables.pdf
        case dBaseIII:
        case dBaseIIIMemo:
            break;
        case dBaseIV:
        case dBaseV:
        case VisualFoxPro:
        case VisualFoxProAuto:
        case dBaseFS:
        case dBaseFSMemo:
        case dBaseIVMemoSQL:
        case FoxProMemo:
        {
            if (nCodepage != 0x00)
            {
                auto eEncoding(RTL_TEXTENCODING_DONTKNOW);
                switch(nCodepage)
                {
                case 0x01: eEncoding = RTL_TEXTENCODING_IBM_437; break;       // DOS USA  code page 437
                case 0x02: eEncoding = RTL_TEXTENCODING_IBM_850; break;       // DOS Multilingual code page 850
                case 0x03: eEncoding = RTL_TEXTENCODING_MS_1252; break;       // Windows ANSI code page 1252
                case 0x04: eEncoding = RTL_TEXTENCODING_APPLE_ROMAN; break;   // Standard Macintosh
                case 0x64: eEncoding = RTL_TEXTENCODING_IBM_852; break;       // EE MS-DOS    code page 852
                case 0x65: eEncoding = RTL_TEXTENCODING_IBM_866; break;       // Russian MS-DOS   code page 866
                case 0x66: eEncoding = RTL_TEXTENCODING_IBM_865; break;       // Nordic MS-DOS    code page 865
                case 0x67: eEncoding = RTL_TEXTENCODING_IBM_861; break;       // Icelandic MS-DOS
                //case 0x68: eEncoding = ; break;     // Kamenicky (Czech) MS-DOS
                //case 0x69: eEncoding = ; break;     // Mazovia (Polish) MS-DOS
                case 0x6A: eEncoding = RTL_TEXTENCODING_IBM_737; break;       // Greek MS-DOS (437G)
                case 0x6B: eEncoding = RTL_TEXTENCODING_IBM_857; break;       // Turkish MS-DOS
                case 0x6C: eEncoding = RTL_TEXTENCODING_IBM_863; break;       // MS-DOS, Canada
                case 0x78: eEncoding = RTL_TEXTENCODING_MS_950; break;        // Windows, Traditional Chinese
                case 0x79: eEncoding = RTL_TEXTENCODING_MS_949; break;        // Windows, Korean (Hangul)
                case 0x7A: eEncoding = RTL_TEXTENCODING_MS_936; break;        // Windows, Simplified Chinese
                case 0x7B: eEncoding = RTL_TEXTENCODING_MS_932; break;        // Windows, Japanese (Shift-jis)
                case 0x7C: eEncoding = RTL_TEXTENCODING_MS_874; break;        // Windows, Thai
                case 0x7D: eEncoding = RTL_TEXTENCODING_MS_1255; break;       // Windows, Hebrew
                case 0x7E: eEncoding = RTL_TEXTENCODING_MS_1256; break;       // Windows, Arabic
                case 0x96: eEncoding = RTL_TEXTENCODING_APPLE_CYRILLIC; break;    // Russian Macintosh
                case 0x97: eEncoding = RTL_TEXTENCODING_APPLE_CENTEURO; break;    // Eastern European Macintosh
                case 0x98: eEncoding = RTL_TEXTENCODING_APPLE_GREEK; break;   // Greek Macintosh
                case 0xC8: eEncoding = RTL_TEXTENCODING_MS_1250; break;       // Windows EE   code page 1250
                case 0xC9: eEncoding = RTL_TEXTENCODING_MS_1251; break;       // Russian Windows
                case 0xCA: eEncoding = RTL_TEXTENCODING_MS_1254; break;       // Turkish Windows
                case 0xCB: eEncoding = RTL_TEXTENCODING_MS_1253; break;       // Greek Windows
                case 0xCC: eEncoding = RTL_TEXTENCODING_MS_1257; break;       // Windows, Baltic
                }
                if(eEncoding != RTL_TEXTENCODING_DONTKNOW)
                {
                    _out_encoding = eEncoding;
                    return true;
                }
            }
        }
        }
        return false;
    }

    bool dbfReadCharset(rtl_TextEncoding &nCharSet, SvStream* dbf_Stream)
    {
        sal_uInt8 nType=0;
        dbf_Stream->ReadUChar( nType );

        dbf_Stream->Seek(STREAM_SEEK_TO_BEGIN + 29);
        if (dbf_Stream->IsEof())
        {
            return false;
        }
        else
        {
            sal_uInt8 nEncoding=0;
            dbf_Stream->ReadUChar( nEncoding );
            return dbfDecodeCharset(nCharSet, nType, nEncoding);
        }
    }

}

} //namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
