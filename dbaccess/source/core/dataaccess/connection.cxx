/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connection.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:04:33 $
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

#ifndef _DBA_CORE_CONNECTION_HXX_
#include "connection.hxx"
#endif

#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#include "datasource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_COREAPI_STATEMENT_HXX_
#include "statement.hxx"
#endif
#ifndef _DBA_COREAPI_PREPAREDSTATEMENT_HXX_
#include "preparedstatement.hxx"
#endif
#ifndef _DBA_COREAPI_CALLABLESTATEMENT_HXX_
#include "callablestatement.hxx"
#endif
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
#endif
#ifndef DBACCESS_CORE_API_SINGLESELECTQUERYCOMPOSER_HXX
#include "SingleSelectQueryComposer.hxx"
#endif
#ifndef DBACCESS_CORE_API_QUERYCOMPOSER_HXX
#include "querycomposer.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX
#include <connectivity/dbmetadata.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

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

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OConnection::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.dbaccess.Connection");
}
//------------------------------------------------------------------------------
sal_Bool OConnection::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OConnection::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported = OConnectionWrapper::getSupportedServiceNames();

    if ( 0 == findValue( aSupported, SERVICE_SDB_CONNECTION, sal_True ).getLength() )
    {
        sal_Int32 nLen = aSupported.getLength();
        aSupported.realloc( nLen + 1 );
        aSupported[ nLen ] = SERVICE_SDB_CONNECTION;
    }

    return aSupported;
}

// XCloseable
//------------------------------------------------------------------------------
void OConnection::close(void) throw( SQLException, RuntimeException )
{
    // being closed is the same as being disposed
    dispose();
}

//------------------------------------------------------------------------------
sal_Bool OConnection::isClosed(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    return !m_xMasterConnection.is();
}

// XConnection
//------------------------------------------------------------------------------
Reference< XStatement >  OConnection::createStatement(void) throw( SQLException, RuntimeException )
{
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
//------------------------------------------------------------------------------
Reference< XPreparedStatement >  OConnection::prepareStatement(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
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

//------------------------------------------------------------------------------
Reference< XPreparedStatement >  OConnection::prepareCall(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
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

//------------------------------------------------------------------------------
rtl::OUString OConnection::nativeSQL(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->nativeSQL(sql);
}

//------------------------------------------------------------------------------
void OConnection::setAutoCommit(sal_Bool autoCommit) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setAutoCommit(autoCommit);
}

//------------------------------------------------------------------------------
sal_Bool OConnection::getAutoCommit(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getAutoCommit();
}

//------------------------------------------------------------------------------
void OConnection::commit(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->commit();
}

//------------------------------------------------------------------------------
void OConnection::rollback(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->rollback();
}

//------------------------------------------------------------------------------
Reference< XDatabaseMetaData >  OConnection::getMetaData(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getMetaData();
}

//------------------------------------------------------------------------------
void OConnection::setReadOnly(sal_Bool readOnly) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setReadOnly(readOnly);
}

//------------------------------------------------------------------------------
sal_Bool OConnection::isReadOnly(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->isReadOnly();
}

//------------------------------------------------------------------------------
void OConnection::setCatalog(const rtl::OUString& catalog) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setCatalog(catalog);
}

//------------------------------------------------------------------------------
rtl::OUString OConnection::getCatalog(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getCatalog();
}

//------------------------------------------------------------------------------
void OConnection::setTransactionIsolation(sal_Int32 level) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setTransactionIsolation(level);
}

//------------------------------------------------------------------------------
sal_Int32 OConnection::getTransactionIsolation(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getTransactionIsolation();
}

//------------------------------------------------------------------------------
Reference< XNameAccess >  OConnection::getTypeMap(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getTypeMap();
}

//------------------------------------------------------------------------------
void OConnection::setTypeMap(const Reference< XNameAccess > & typeMap) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setTypeMap(typeMap);
}
//==========================================================================
//= OConnection
//==========================================================================
DBG_NAME(OConnection)
//--------------------------------------------------------------------------
OConnection::OConnection(ODatabaseSource& _rDB
                         , Reference< XConnection >& _rxMaster
                         , const Reference< XMultiServiceFactory >& _rxORB)
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
    DBG_CTOR(OConnection,NULL);
    osl_incrementInterlockedCount(&m_refCount);

    try
    {
        Reference< XProxyFactory > xProxyFactory(
                _rxORB->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.ProxyFactory"))),UNO_QUERY);
        Reference<XAggregation> xAgg = xProxyFactory->createProxy(_rxMaster.get());
        setDelegation(xAgg,m_refCount);
        DBG_ASSERT(m_xConnection.is(), "OConnection::OConnection : invalid master connection !");
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    m_xTableUIProvider = m_xTableUIProvider.query( m_xMasterConnection );

    try
    {
        m_xQueries = new OQueryContainer(Reference< XNameContainer >(_rDB.getQueryDefinitions( ),UNO_QUERY), this,_rxORB, &m_aWarnings);

        sal_Bool bCase = sal_True;
        Reference<XDatabaseMetaData> xMeta;
        try
        {
            xMeta = getMetaData();
            bCase = xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers();
        }
        catch(SQLException&)
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
                ::rtl::OUString sView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
                Reference<XRow> xRow(xRes,UNO_QUERY);
                while(xRes->next())
                {
                    ::rtl::OUString sValue = xRow->getString(1);
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
    catch(Exception&)
    {
    }
    osl_decrementInterlockedCount( &m_refCount );
}

//--------------------------------------------------------------------------
OConnection::~OConnection()
{
    delete m_pTables;
    delete m_pViews;
    DBG_DTOR(OConnection,NULL);
}


// XWarningsSupplier
//--------------------------------------------------------------------------
Any SAL_CALL OConnection::getWarnings() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_aWarnings.getWarnings();
}

//--------------------------------------------------------------------------
void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_aWarnings.clearWarnings();
}

//--------------------------------------------------------------------------
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
//--------------------------------------------------------------------------
Sequence< Type > OConnection::getTypes() throw (RuntimeException)
{
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

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OConnection::getImplementationId() throw (RuntimeException)
{
    return getUnoTunnelImplementationId();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
void OConnection::acquire() throw ()
{
    OSubComponent::acquire();
}

//--------------------------------------------------------------------------
void OConnection::release() throw ()
{
    OSubComponent::release();
}

// OSubComponent
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    MutexGuard aGuard(m_aMutex);

    OSubComponent::disposing();
    OConnectionWrapper::disposing();

    for (OWeakRefArrayIterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
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

    for (OWeakRefArrayIterator j = m_aComposers.begin(); m_aComposers.end() != j; ++j)
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
    catch(Exception)
    {
    }
    m_xMasterConnection = NULL;
}

// XChild
//------------------------------------------------------------------------------
Reference< XInterface >  OConnection::getParent(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xParent;
}

//------------------------------------------------------------------------------
void OConnection::setParent(const Reference< XInterface > & /*Parent*/) throw( NoSupportException, RuntimeException )
{
    throw NoSupportException();
}

// XSQLQueryComposerFactory
//------------------------------------------------------------------------------
Reference< XSQLQueryComposer >  OConnection::createQueryComposer(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    //  Reference< XNumberFormatsSupplier >  xSupplier = pParent->getNumberFormatsSupplier();
    Reference< XSQLQueryComposer >  xComposer( new OQueryComposer( this ) );
    m_aComposers.push_back(WeakReferenceHelper(xComposer));
    return xComposer;
}
// -----------------------------------------------------------------------------
void OConnection::refresh(const Reference< XNameAccess >& _rToBeRefreshed)
{
    if ( _rToBeRefreshed == Reference< XNameAccess >(m_pTables) )
    {
        if (!m_pTables->isInitialized())
        {
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
        if (!m_pViews->isInitialized())
        {
            // check if our "master connection" can supply tables
            Reference< XViewsSupplier > xMaster(getMasterTables(),UNO_QUERY);

            if (xMaster.is() && xMaster->getViews().is())
                m_pViews->construct(xMaster->getViews(),m_aTableFilter, m_aTableTypeFilter);
            else
                m_pViews->construct(m_aTableFilter, m_aTableTypeFilter);
        }
    }
}
// -----------------------------------------------------------------------------

// XTablesSupplier
//------------------------------------------------------------------------------
Reference< XNameAccess >  OConnection::getTables() throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    refresh(m_pTables);

    return m_pTables;
}
// -----------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL OConnection::getViews(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    refresh(m_pViews);

    return m_pViews;
}
// XQueriesSupplier
//------------------------------------------------------------------------------
Reference< XNameAccess >  OConnection::getQueries(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    return m_xQueries;
}

// ::com::sun::star::sdb::XCommandPreparation
//------------------------------------------------------------------------------
Reference< XPreparedStatement >  SAL_CALL OConnection::prepareCommand( const ::rtl::OUString& command, sal_Int32 commandType ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    rtl::OUString aStatement;
    switch (commandType)
    {
        case CommandType::TABLE:
            {
                aStatement = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * FROM "));

                ::rtl::OUString sCatalog, sSchema, sTable;
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
// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OConnection::createInstance( const ::rtl::OUString& _sServiceSpecifier ) throw (Exception, RuntimeException)
{
    Reference< XServiceInfo > xRet;
    if  (  ( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER == _sServiceSpecifier )
        || ( _sServiceSpecifier.equalsAscii( "com.sun.star.sdb.SingleSelectQueryAnalyzer" ) )
        )
    {
        xRet = new OSingleSelectQueryComposer( getTables(),this, m_aContext );
        m_aComposers.push_back(WeakReferenceHelper(xRet));
    }
    return Reference< XInterface >(xRet,UNO_QUERY);
}
// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OConnection::createInstanceWithArguments( const ::rtl::OUString& _sServiceSpecifier, const Sequence< Any >& /*Arguments*/ ) throw (Exception, RuntimeException)
{
    return createInstance(_sServiceSpecifier);
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OConnection::getAvailableServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = SERVICE_NAME_SINGLESELECTQUERYCOMPOSER;
    return aRet;
}
// -----------------------------------------------------------------------------
Reference< XTablesSupplier > OConnection::getMasterTables()
{
// check if out "master connection" can supply tables
    if(!m_xMasterTables.is())
    {
        try
        {
            Reference<XDatabaseMetaData> xMeta = getMetaData();
            if ( xMeta.is() )
                m_xMasterTables = ::dbtools::getDataDefinitionByURLAndConnection( xMeta->getURL(), m_xMasterConnection, m_aContext.getLegacyServiceFactory() );
        }
        catch(SQLException&)
        {
        }
    }
    return m_xMasterTables;
}
// -----------------------------------------------------------------------------
// XUsersSupplier
Reference< XNameAccess > SAL_CALL OConnection::getUsers(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference<XUsersSupplier> xUsr(getMasterTables(),UNO_QUERY);
    return xUsr.is() ? xUsr->getUsers() : Reference< XNameAccess >();
}
// -----------------------------------------------------------------------------
// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OConnection::getGroups(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    Reference<XGroupsSupplier> xGrp(getMasterTables(),UNO_QUERY);
    return xGrp.is() ? xGrp->getGroups() : Reference< XNameAccess >();
}

// -----------------------------------------------------------------------------
void OConnection::impl_loadConnectionTools_throw()
{
    Sequence< Any > aArguments( 1 );
    aArguments[0] <<= NamedValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Connection" ) ), makeAny( Reference< XConnection >( this ) ) );

    if ( !m_aContext.createComponentWithArguments( "com.sun.star.sdb.tools.ConnectionTools", aArguments, m_xConnectionTools ) )
        throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "service not registered: com.sun.star.sdb.tools.ConnectionTools" ) ), *this );
}

// -----------------------------------------------------------------------------
Reference< tools::XTableName > SAL_CALL OConnection::createTableName(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->createTableName();
}

// -----------------------------------------------------------------------------
Reference< tools::XObjectNames > SAL_CALL OConnection::getObjectNames(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->getObjectNames();
}

// -----------------------------------------------------------------------------
Reference< tools::XDataSourceMetaData > SAL_CALL OConnection::getDataSourceMetaData(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->getDataSourceMetaData();
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OConnection::getFieldsByCommandDescriptor( ::sal_Int32 commandType, const ::rtl::OUString& command, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& keepFieldsAlive ) throw (::com::sun::star::sdbc::SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->getFieldsByCommandDescriptor(commandType,command,keepFieldsAlive);
}
//--------------------------------------------------------------------
Reference< XSingleSelectQueryComposer > SAL_CALL OConnection::getComposer( ::sal_Int32 commandType, const ::rtl::OUString& command ) throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    impl_loadConnectionTools_throw();

    return m_xConnectionTools->getComposer(commandType,command);
}

// -----------------------------------------------------------------------------
void OConnection::impl_checkTableQueryNames_nothrow()
{
    DatabaseMetaData aMeta( static_cast< XConnection* >( this ) );
    if ( !aMeta.supportsSubqueriesInFrom() )
        // nothing to do
        return;

    try
    {
        Reference< XNameAccess > xTables( getTables() );
        Sequence< ::rtl::OUString > aTableNames( xTables->getElementNames() );
        ::std::set< ::rtl::OUString > aSortedTableNames( aTableNames.getConstArray(), aTableNames.getConstArray() + aTableNames.getLength() );

        Reference< XNameAccess > xQueries( getQueries() );
        Sequence< ::rtl::OUString > aQueryNames( xQueries->getElementNames() );

        for (   const ::rtl::OUString* pQueryName = aQueryNames.getConstArray();
                pQueryName != aQueryNames.getConstArray() + aQueryNames.getLength();
                ++pQueryName
            )
        {
            if ( aSortedTableNames.find( *pQueryName ) != aSortedTableNames.end() )
            {
                ::rtl::OUString sConflictWarning( DBACORE_RESSTRING( RID_STR_CONFLICTING_NAMES ) );
                m_aWarnings.appendWarning( sConflictWarning, "01SB0", *this );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
Reference< XGraphic > SAL_CALL OConnection::getTableIcon( const ::rtl::OUString& _TableName, ::sal_Int32 _ColorMode ) throw (RuntimeException)
{
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

// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OConnection::getTableEditor( const Reference< XDatabaseDocumentUI >& _DocumentUI, const ::rtl::OUString& _TableName ) throw (IllegalArgumentException, WrappedTargetException, RuntimeException)
{
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


//........................................................................
}   // namespace dbaccess
//........................................................................

