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

#include "connection.hxx"
#include "dbastrings.hrc"
#include "datasource.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "statement.hxx"
#include "preparedstatement.hxx"
#include "callablestatement.hxx"
#include "ContainerMediator.hxx"
#include "SingleSelectQueryComposer.hxx"
#include "querycomposer.hxx"
#include "sdbcoretools.hxx"

#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/tools/ConnectionTools.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/reflection/ProxyFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbmetadata.hxx>
#include <connectivity/dbexception.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <comphelper/extract.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdb::application;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::graphic;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::dbtools;

using ::com::sun::star::sdb::tools::XTableName;
using ::com::sun::star::sdb::tools::XObjectNames;
using ::com::sun::star::sdb::tools::XDataSourceMetaData;

namespace dbaccess
{

// XServiceInfo
OUString OConnection::getImplementationName(  ) throw(RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getImplementationName" );
    return OUString("com.sun.star.comp.dbaccess.Connection");
}

sal_Bool OConnection::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::supportsService" );
    return findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

Sequence< OUString > OConnection::getSupportedServiceNames(  ) throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getSupportedServiceNames" );
    Sequence< OUString > aSupported = OConnectionWrapper::getSupportedServiceNames();

    if ( 0 == findValue( aSupported, SERVICE_SDB_CONNECTION, sal_True ).getLength() )
    {
        sal_Int32 nLen = aSupported.getLength();
        aSupported.realloc( nLen + 1 );
        aSupported[ nLen ] = SERVICE_SDB_CONNECTION;
    }

    return aSupported;
}

// XCloseable
void OConnection::close(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::close" );
    // being closed is the same as being disposed
    dispose();
}

sal_Bool OConnection::isClosed(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::isClosed" );
    MutexGuard aGuard(m_aMutex);
    return !m_xMasterConnection.is();
}

// XConnection
Reference< XStatement >  OConnection::createStatement(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::createStatement" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference< XStatement > xStatement;
    Reference< XStatement > xMasterStatement = m_xMasterConnection->createStatement();
    if ( xMasterStatement.is() )
    {
        xStatement = new OStatement(this, xMasterStatement);
        m_aStatements.push_back(WeakReferenceHelper(xStatement));
    }
    return xStatement;
}

Reference< XPreparedStatement >  OConnection::prepareStatement(const OUString& sql) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::prepareStatement" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    // TODO convert the SQL to SQL the driver understands
    Reference< XPreparedStatement > xStatement;
    Reference< XPreparedStatement > xMasterStatement = m_xMasterConnection->prepareStatement(sql);
    if ( xMasterStatement.is() )
    {
        xStatement = new OPreparedStatement(this, xMasterStatement);
        m_aStatements.push_back(WeakReferenceHelper(xStatement));
    }
    return xStatement;
}

Reference< XPreparedStatement >  OConnection::prepareCall(const OUString& sql) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::prepareCall" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference< XPreparedStatement > xStatement;
    Reference< XPreparedStatement > xMasterStatement = m_xMasterConnection->prepareCall(sql);
    if ( xMasterStatement.is() )
    {
        xStatement = new OCallableStatement(this, xMasterStatement);
        m_aStatements.push_back(WeakReferenceHelper(xStatement));
    }
    return xStatement;
}

OUString OConnection::nativeSQL(const OUString& sql) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::nativeSQL" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->nativeSQL(sql);
}

void OConnection::setAutoCommit(sal_Bool autoCommit) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::setAutoCommit" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setAutoCommit(autoCommit);
}

sal_Bool OConnection::getAutoCommit(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::getAutoCommit" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getAutoCommit();
}

void OConnection::commit(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::commit" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->commit();
}

void OConnection::rollback(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::rollback" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->rollback();
}

Reference< XDatabaseMetaData >  OConnection::getMetaData(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::getMetaData" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getMetaData();
}

void OConnection::setReadOnly(sal_Bool readOnly) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::setReadOnly" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setReadOnly(readOnly);
}

sal_Bool OConnection::isReadOnly(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::isReadOnly" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->isReadOnly();
}

void OConnection::setCatalog(const OUString& catalog) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::setCatalog" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setCatalog(catalog);
}

OUString OConnection::getCatalog(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::getCatalog" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getCatalog();
}

void OConnection::setTransactionIsolation(sal_Int32 level) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::setTransactionIsolation" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setTransactionIsolation(level);
}

sal_Int32 OConnection::getTransactionIsolation(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::getTransactionIsolation" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getTransactionIsolation();
}

Reference< XNameAccess >  OConnection::getTypeMap(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::getTypeMap" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getTypeMap();
}

void OConnection::setTypeMap(const Reference< XNameAccess > & typeMap) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::setTypeMap" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setTypeMap(typeMap);
}

// OConnection
DBG_NAME(OConnection)

OConnection::OConnection(ODatabaseSource& _rDB
                         , Reference< XConnection >& _rxMaster
                         , const Reference< XComponentContext >& _rxORB)
            :OSubComponent(m_aMutex, static_cast< OWeakObject* >(&_rDB))
                // as the queries reroute their refcounting to us, this m_aMutex is okey. If the queries
                // container would do it's own refcounting, it would have to aquire m_pMutex
                // same for tables
            ,m_aTableFilter(_rDB.m_pImpl->m_aTableFilter)
            ,m_aTableTypeFilter(_rDB.m_pImpl->m_aTableTypeFilter)
            ,m_aContext( _rxORB )
            ,m_xMasterConnection(_rxMaster)
            ,m_pTables(NULL)
            ,m_pViews(NULL)
            ,m_aWarnings( Reference< XWarningsSupplier >( _rxMaster, UNO_QUERY ) )
            ,m_nInAppend(0)
            ,m_bSupportsViews(sal_False)
            ,m_bSupportsUsers(sal_False)
            ,m_bSupportsGroups(sal_False)
{
    SAL_INFO("dbaccess", "OConnection::OConnection" );
    DBG_CTOR(OConnection,NULL);
    osl_atomic_increment(&m_refCount);

    try
    {
        Reference< XProxyFactory > xProxyFactory = ProxyFactory::create( m_aContext );
        Reference<XAggregation> xAgg = xProxyFactory->createProxy(_rxMaster.get());
        setDelegation(xAgg,m_refCount);
        OSL_ENSURE(m_xConnection.is(), "OConnection::OConnection : invalid master connection !");
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    m_xTableUIProvider = m_xTableUIProvider.query( m_xMasterConnection );

    try
    {
        m_xQueries = OQueryContainer::create(Reference< XNameContainer >(_rDB.getQueryDefinitions(), UNO_QUERY), this, _rxORB, &m_aWarnings).get();

        sal_Bool bCase = sal_True;
        Reference<XDatabaseMetaData> xMeta;
        try
        {
            xMeta = getMetaData();
            bCase = xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers();
        }
        catch(const SQLException&)
        {
        }
        Reference< XNameContainer > xTableDefinitions(_rDB.getTables(),UNO_QUERY);
        m_pTables = new OTableContainer( *this, m_aMutex, this, bCase, xTableDefinitions, this, &m_aWarnings,m_nInAppend );

        // check if we supports types
        if ( xMeta.is() )
        {
            Reference<XResultSet> xRes = xMeta->getTableTypes();
            if(xRes.is())
            {
                OUString sView("VIEW");
                Reference<XRow> xRow(xRes,UNO_QUERY);
                while(xRes->next())
                {
                    OUString sValue = xRow->getString(1);
                    if( !xRow->wasNull() && sValue == sView)
                    {
                        m_bSupportsViews = sal_True;
                        break;
                    }
                }
            }
            // some dbs don't support this type so we should ask if a XViewsSupplier is supported
            if(!m_bSupportsViews)
            {
                Reference< XViewsSupplier > xMaster(getMasterTables(),UNO_QUERY);

                if (xMaster.is() && xMaster->getViews().is())
                    m_bSupportsViews = sal_True;
            }
            if(m_bSupportsViews)
            {
                m_pViews = new OViewContainer(*this, m_aMutex, this, bCase,this,&m_aWarnings,m_nInAppend);
                m_pViews->addContainerListener(m_pTables);
                m_pTables->addContainerListener(m_pViews);
            }
            m_bSupportsUsers = Reference< XUsersSupplier> (getMasterTables(),UNO_QUERY).is();
            m_bSupportsGroups = Reference< XGroupsSupplier> (getMasterTables(),UNO_QUERY).is();

            impl_checkTableQueryNames_nothrow();
        }
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    osl_atomic_decrement( &m_refCount );
}

OConnection::~OConnection()
{
    delete m_pTables;
    delete m_pViews;
    DBG_DTOR(OConnection,NULL);
}

// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings() throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getWarnings" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_aWarnings.getWarnings();
}

void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::clearWarnings" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_aWarnings.clearWarnings();
}

namespace
{
    struct CompareTypeByName : public ::std::binary_function< Type, Type, bool >
    {
        bool operator() ( const Type& _rLHS, const Type& _rRHS ) const
        {
            return _rLHS.getTypeName() < _rRHS.getTypeName();
        }
    };
    typedef ::std::set< Type, CompareTypeByName > TypeBag;

    void lcl_copyTypes( TypeBag& _out_rTypes, const Sequence< Type >& _rTypes )
    {
        ::std::copy( _rTypes.getConstArray(), _rTypes.getConstArray() + _rTypes.getLength(),
            ::std::insert_iterator< TypeBag >( _out_rTypes, _out_rTypes.begin() ) );
    }
}

// com::sun::star::lang::XTypeProvider
Sequence< Type > OConnection::getTypes() throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getTypes" );
    TypeBag aNormalizedTypes;

    lcl_copyTypes( aNormalizedTypes, OSubComponent::getTypes() );
    lcl_copyTypes( aNormalizedTypes, OConnection_Base::getTypes() );
    lcl_copyTypes( aNormalizedTypes, ::connectivity::OConnectionWrapper::getTypes() );

    if ( !m_bSupportsViews )
        aNormalizedTypes.erase( XViewsSupplier::static_type() );
    if ( !m_bSupportsUsers )
        aNormalizedTypes.erase( XUsersSupplier::static_type() );
    if ( !m_bSupportsGroups )
        aNormalizedTypes.erase( XGroupsSupplier::static_type() );

    Sequence< Type > aSupportedTypes( aNormalizedTypes.size() );
    ::std::copy( aNormalizedTypes.begin(), aNormalizedTypes.end(), aSupportedTypes.getArray() );
    return aSupportedTypes;
}

Sequence< sal_Int8 > OConnection::getImplementationId() throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getImplementationId" );
    return getUnoTunnelImplementationId();
}

// com::sun::star::uno::XInterface
Any OConnection::queryInterface( const Type & rType ) throw (RuntimeException)
{
    if ( !m_bSupportsViews && rType.equals( XViewsSupplier::static_type() ) )
        return Any();
    else if ( !m_bSupportsUsers && rType.equals( XUsersSupplier::static_type() ) )
        return Any();
    else if ( !m_bSupportsGroups && rType.equals( XGroupsSupplier::static_type() ) )
        return Any();
    Any aReturn = OSubComponent::queryInterface( rType );
    if (!aReturn.hasValue())
    {
        aReturn = OConnection_Base::queryInterface( rType );
        if (!aReturn.hasValue())
            aReturn = OConnectionWrapper::queryInterface( rType );
    }
    return aReturn;
}

void OConnection::acquire() throw ()
{
    // include this one when you want to see who calls it (call graph)
    OSubComponent::acquire();
}

void OConnection::release() throw ()
{
    // include this one when you want to see who calls it (call graph)
    OSubComponent::release();
}

// OSubComponent
void OConnection::disposing()
{
    SAL_INFO("dbaccess", "OConnection::disposing" );
    MutexGuard aGuard(m_aMutex);

    OSubComponent::disposing();
    OConnectionWrapper::disposing();

    OWeakRefArrayIterator aEnd = m_aStatements.end();
    for (OWeakRefArrayIterator i = m_aStatements.begin(); aEnd != i; ++i)
    {
        Reference<XComponent> xComp(i->get(),UNO_QUERY);
        ::comphelper::disposeComponent(xComp);
    }
    m_aStatements.clear();
    m_xMasterTables = NULL;

    if(m_pTables)
        m_pTables->dispose();
    if(m_pViews)
        m_pViews->dispose();

    ::comphelper::disposeComponent(m_xQueries);

    OWeakRefArrayIterator aComposerEnd = m_aComposers.end();
    for (OWeakRefArrayIterator j = m_aComposers.begin(); aComposerEnd != j; ++j)
    {
        Reference<XComponent> xComp(j->get(),UNO_QUERY);
        ::comphelper::disposeComponent(xComp);
    }

    m_aComposers.clear();

    try
    {
        if (m_xMasterConnection.is())
            m_xMasterConnection->close();
    }
    catch(const Exception&)
    {
    }
    m_xMasterConnection = NULL;
}

// XChild
Reference< XInterface >  OConnection::getParent(void) throw( RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::getParent" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xParent;
}

void OConnection::setParent(const Reference< XInterface > & /*Parent*/) throw( NoSupportException, RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::setParent" );
    throw NoSupportException();
}

// XSQLQueryComposerFactory
Reference< XSQLQueryComposer >  OConnection::createQueryComposer(void) throw( RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::createQueryComposer" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    //  Reference< XNumberFormatsSupplier >  xSupplier = pParent->getNumberFormatsSupplier();
    Reference< XSQLQueryComposer >  xComposer( new OQueryComposer( this ) );
    m_aComposers.push_back(WeakReferenceHelper(xComposer));
    return xComposer;
}

void OConnection::impl_fillTableFilter()
{
    Reference<XPropertySet> xProp(getParent(),UNO_QUERY);
    if ( xProp.is() )
    {
        xProp->getPropertyValue(PROPERTY_TABLEFILTER)       >>= m_aTableFilter;
        xProp->getPropertyValue(PROPERTY_TABLETYPEFILTER)   >>= m_aTableTypeFilter;
    }
}

void OConnection::refresh(const Reference< XNameAccess >& _rToBeRefreshed)
{
    SAL_INFO("dbaccess", "OConnection::refresh" );
    if ( _rToBeRefreshed == Reference< XNameAccess >(m_pTables) )
    {
        if (m_pTables && !m_pTables->isInitialized())
        {
            impl_fillTableFilter();
            // check if our "master connection" can supply tables
            getMasterTables();

            if (m_xMasterTables.is() && m_xMasterTables->getTables().is())
            {   // yes -> wrap them
                m_pTables->construct(m_xMasterTables->getTables(),m_aTableFilter, m_aTableTypeFilter);
            }
            else
            {   // no -> use an own container
                m_pTables->construct(m_aTableFilter, m_aTableTypeFilter);
            }
        }
    }
    else if ( _rToBeRefreshed == Reference< XNameAccess >(m_pViews) )
    {
        if (m_pViews && !m_pViews->isInitialized())
        {
            impl_fillTableFilter();
            // check if our "master connection" can supply tables
            Reference< XViewsSupplier > xMaster(getMasterTables(),UNO_QUERY);

            if (xMaster.is() && xMaster->getViews().is())
                m_pViews->construct(xMaster->getViews(),m_aTableFilter, m_aTableTypeFilter);
            else
                m_pViews->construct(m_aTableFilter, m_aTableTypeFilter);
        }
    }
}

// XTablesSupplier
Reference< XNameAccess >  OConnection::getTables() throw( RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::getTables" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    refresh(m_pTables);

    return m_pTables;
}

Reference< XNameAccess > SAL_CALL OConnection::getViews(  ) throw(RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getViews" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    refresh(m_pViews);

    return m_pViews;
}

// XQueriesSupplier
Reference< XNameAccess >  OConnection::getQueries(void) throw( RuntimeException )
{
    SAL_INFO("dbaccess", "OConnection::getQueries" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    return m_xQueries;
}

// ::com::sun::star::sdb::XCommandPreparation
Reference< XPreparedStatement >  SAL_CALL OConnection::prepareCommand( const OUString& command, sal_Int32 commandType ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::prepareCommand" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    OUString aStatement;
    switch (commandType)
    {
        case CommandType::TABLE:
            {
                aStatement = "SELECT * FROM ";

                OUString sCatalog, sSchema, sTable;
                ::dbtools::qualifiedNameComponents( getMetaData(), command, sCatalog, sSchema, sTable, ::dbtools::eInDataManipulation );
                aStatement += ::dbtools::composeTableNameForSelect( this, sCatalog, sSchema, sTable );
            }
            break;
        case CommandType::QUERY:
            if ( m_xQueries->hasByName(command) )
            {
                Reference< XPropertySet > xQuery(m_xQueries->getByName(command),UNO_QUERY);
                xQuery->getPropertyValue(PROPERTY_COMMAND) >>= aStatement;
            }
            break;
        default:
            aStatement = command;
    }
    // TODO EscapeProcessing
    return prepareStatement(aStatement);
}

Reference< XInterface > SAL_CALL OConnection::createInstance( const OUString& _sServiceSpecifier ) throw (Exception, RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::createInstance" );
    Reference< XServiceInfo > xRet;
    if ( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER == _sServiceSpecifier || _sServiceSpecifier == "com.sun.star.sdb.SingleSelectQueryAnalyzer" )
    {
        xRet = new OSingleSelectQueryComposer( getTables(),this, m_aContext );
        m_aComposers.push_back(WeakReferenceHelper(xRet));
    }
    else
    {
        if ( !_sServiceSpecifier.isEmpty() )
        {
            TSupportServices::iterator aFind = m_aSupportServices.find(_sServiceSpecifier);
            if ( aFind == m_aSupportServices.end() )
            {
                Sequence<Any> aArgs(1);
                Reference<XConnection> xMy(this);
                aArgs[0] <<= NamedValue("ActiveConnection",makeAny(xMy));
                aFind = m_aSupportServices.insert(
                           TSupportServices::value_type(
                               _sServiceSpecifier,
                               m_aContext->getServiceManager()->createInstanceWithArgumentsAndContext(_sServiceSpecifier, aArgs, m_aContext)
                           )).first;
            }
            return aFind->second;
        }
    }
    return xRet;
}

Reference< XInterface > SAL_CALL OConnection::createInstanceWithArguments( const OUString& _sServiceSpecifier, const Sequence< Any >& /*Arguments*/ ) throw (Exception, RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::createInstanceWithArguments" );
    return createInstance(_sServiceSpecifier);
}

Sequence< OUString > SAL_CALL OConnection::getAvailableServiceNames(  ) throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getAvailableServiceNames" );
    Sequence< OUString > aRet(1);
    aRet[0] = SERVICE_NAME_SINGLESELECTQUERYCOMPOSER;
    return aRet;
}

Reference< XTablesSupplier > OConnection::getMasterTables()
{
    SAL_INFO("dbaccess", "OConnection::getMasterTables" );
// check if out "master connection" can supply tables
    if(!m_xMasterTables.is())
    {
        try
        {
            Reference<XDatabaseMetaData> xMeta = getMetaData();
            if ( xMeta.is() )
                m_xMasterTables = ::dbtools::getDataDefinitionByURLAndConnection( xMeta->getURL(), m_xMasterConnection, m_aContext );
        }
        catch(const SQLException&)
        {
        }
    }
    return m_xMasterTables;
}

// XUsersSupplier
Reference< XNameAccess > SAL_CALL OConnection::getUsers(  ) throw(RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getUsers" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference<XUsersSupplier> xUsr(getMasterTables(),UNO_QUERY);
    return xUsr.is() ? xUsr->getUsers() : Reference< XNameAccess >();
}

// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OConnection::getGroups(  ) throw(RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getGroups" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    Reference<XGroupsSupplier> xGrp(getMasterTables(),UNO_QUERY);
    return xGrp.is() ? xGrp->getGroups() : Reference< XNameAccess >();
}

void OConnection::impl_loadConnectionTools_throw()
{
    SAL_INFO("dbaccess", "OConnection::impl_loadConnectionTools_throw" );

    m_xConnectionTools = css::sdb::tools::ConnectionTools::createWithConnection( m_aContext, this );
}

Reference< XTableName > SAL_CALL OConnection::createTableName(  ) throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::createTableName" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->createTableName();
}

Reference< XObjectNames > SAL_CALL OConnection::getObjectNames(  ) throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getObjectNames" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->getObjectNames();
}

Reference< XDataSourceMetaData > SAL_CALL OConnection::getDataSourceMetaData(  ) throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getDataSourceMetaData" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->getDataSourceMetaData();
}

Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getFieldsByCommandDescriptor( ::sal_Int32 commandType, const OUString& command, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& keepFieldsAlive ) throw (::com::sun::star::sdbc::SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getFieldsByCommandDescriptor" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->getFieldsByCommandDescriptor(commandType,command,keepFieldsAlive);
}

Reference< XSingleSelectQueryComposer > SAL_CALL OConnection::getComposer( ::sal_Int32 commandType, const OUString& command ) throw (::com::sun::star::uno::RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getComposer" );
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->getComposer(commandType,command);
}

void OConnection::impl_checkTableQueryNames_nothrow()
{
    SAL_INFO("dbaccess", "OConnection::impl_checkTableQueryNames_nothrow" );
    DatabaseMetaData aMeta( static_cast< XConnection* >( this ) );
    if ( !aMeta.supportsSubqueriesInFrom() )
        // nothing to do
        return;

    try
    {
        Reference< XNameAccess > xTables( getTables() );
        Sequence< OUString > aTableNames( xTables->getElementNames() );
        ::std::set< OUString > aSortedTableNames( aTableNames.getConstArray(), aTableNames.getConstArray() + aTableNames.getLength() );

        Reference< XNameAccess > xQueries( getQueries() );
        Sequence< OUString > aQueryNames( xQueries->getElementNames() );

        for (   const OUString* pQueryName = aQueryNames.getConstArray();
                pQueryName != aQueryNames.getConstArray() + aQueryNames.getLength();
                ++pQueryName
            )
        {
            if ( aSortedTableNames.find( *pQueryName ) != aSortedTableNames.end() )
            {
                OUString sConflictWarning( DBACORE_RESSTRING( RID_STR_CONFLICTING_NAMES ) );
                m_aWarnings.appendWarning( sConflictWarning, "01SB0", *this );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

Reference< XGraphic > SAL_CALL OConnection::getTableIcon( const OUString& _TableName, ::sal_Int32 _ColorMode ) throw (RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getTableIcon" );
    Reference< XGraphic > xReturn;

    // ask our aggregate
    if ( m_xTableUIProvider.is() )
        xReturn = m_xTableUIProvider->getTableIcon( _TableName, _ColorMode );

    // ask ourself
    // well, we don't have own functionality here ...
    // In the future, we might decide to delegate the complete handling to this interface.
    // In this case, we would need to load the icon here.

    return xReturn;
}

Reference< XInterface > SAL_CALL OConnection::getTableEditor( const Reference< XDatabaseDocumentUI >& _DocumentUI, const OUString& _TableName ) throw (IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SAL_INFO("dbaccess", "OConnection::getTableEditor" );
    Reference< XInterface > xReturn;

    // ask our aggregate
    if ( m_xTableUIProvider.is() )
        xReturn = m_xTableUIProvider->getTableEditor( _DocumentUI, _TableName );

    // ask ourself
    // well, we don't have own functionality here ...
    // In the future, we might decide to delegate the complete handling to this interface.
    // In this case, we would need to instantiate an css.sdb.TableDesign here.

    return xReturn;
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
