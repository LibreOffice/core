/*************************************************************************
 *
 *  $RCSfile: connection.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:08:12 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _DBA_CORE_CONNECTION_HXX_
#include "connection.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#include "datasource.hxx"
#endif
#ifndef _DBA_COREAPI_STATEMENT_HXX_
#include <statement.hxx>
#endif
#ifndef _DBA_COREAPI_PREPAREDSTATEMENT_HXX_
#include <preparedstatement.hxx>
#endif
#ifndef _DBA_COREAPI_CALLABLESTATEMENT_HXX_
#include <callablestatement.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
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
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef DBACCESS_CORE_API_QUERYCOMPOSER_HXX
#include "querycomposer.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
#endif
#ifndef DBACCESS_CORE_API_SINGLESELECTQUERYCOMPOSER_HXX
#include "SingleSelectQueryComposer.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::container;
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
    return !m_xConnection.is();
}

// XConnection
//------------------------------------------------------------------------------
Reference< XStatement >  OConnection::createStatement(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference< XStatement > xStatement;
    Reference< XStatement > xMasterStatement = m_xConnection->createStatement();
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
    Reference< XPreparedStatement > xMasterStatement = m_xConnection->prepareStatement(sql);
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
    Reference< XPreparedStatement > xMasterStatement = prepareCall(sql);
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
    return m_xConnection->nativeSQL(sql);
}

//------------------------------------------------------------------------------
void OConnection::setAutoCommit(sal_Bool autoCommit) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xConnection->setAutoCommit(autoCommit);
}

//------------------------------------------------------------------------------
sal_Bool OConnection::getAutoCommit(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xConnection->getAutoCommit();
}

//------------------------------------------------------------------------------
void OConnection::commit(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xConnection->commit();
}

//------------------------------------------------------------------------------
void OConnection::rollback(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xConnection->rollback();
}

//------------------------------------------------------------------------------
Reference< XDatabaseMetaData >  OConnection::getMetaData(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xConnection->getMetaData();
}

//------------------------------------------------------------------------------
void OConnection::setReadOnly(sal_Bool readOnly) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xConnection->setReadOnly(readOnly);
}

//------------------------------------------------------------------------------
sal_Bool OConnection::isReadOnly(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xConnection->isReadOnly();
}

//------------------------------------------------------------------------------
void OConnection::setCatalog(const rtl::OUString& catalog) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xConnection->setCatalog(catalog);
}

//------------------------------------------------------------------------------
rtl::OUString OConnection::getCatalog(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xConnection->getCatalog();
}

//------------------------------------------------------------------------------
void OConnection::setTransactionIsolation(sal_Int32 level) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xConnection->setTransactionIsolation(level);
}

//------------------------------------------------------------------------------
sal_Int32 OConnection::getTransactionIsolation(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xConnection->getTransactionIsolation();
}

//------------------------------------------------------------------------------
Reference< XNameAccess >  OConnection::getTypeMap(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xConnection->getTypeMap();
}

//------------------------------------------------------------------------------
void OConnection::setTypeMap(const Reference< XNameAccess > & typeMap) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xConnection->setTypeMap(typeMap);
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
            ,m_aTableFilter(_rDB.m_aTableFilter)
            ,m_aTableTypeFilter(_rDB.m_aTableTypeFilter)
            ,m_xORB(_rxORB)
            ,m_xMasterConnection(_rxMaster)
            ,m_pTables(NULL)
            ,m_pViews(NULL)
            ,m_bSupportsViews(sal_False)
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
        if (!m_xProxyConnection->queryAggregation(getCppuType( (Reference<XWarningsSupplier>*)0)).hasValue())
        {
            DBG_ERROR("OConnection::OConnection : the connection is assumed to be a warnings supplier ! Won't use it !");
            // as we're the owner of the conn and don't want to use it -> dispose
            Reference< XComponent > xConnComp;
            ::comphelper::query_aggregation(m_xProxyConnection,xConnComp);
            if (xConnComp.is())
                xConnComp->dispose();

            m_xMasterConnection = NULL;
        }
    }
    catch(const Exception&)
    {
    }
    try
    {
        m_pQueries = new OQueryContainer(Reference< XNameContainer >(_rDB.getQueryDefinitions( ),UNO_QUERY), this,_rxORB, this);
        m_xQueries = m_pQueries;

        sal_Bool bCase = sal_True;
        Reference<XDatabaseMetaData> xMeta;
        try
        {
            xMeta = getMetaData();
            bCase = xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers();
        }
        catch(SQLException&)
        {
        }
        Reference< XNameContainer > xDefNames(_rDB.getTables(),UNO_QUERY);
        m_pTables = new OTableContainer(*this, m_aMutex,this, bCase, xDefNames,this,this);

        // check if we supports types
        if ( xMeta.is() )
        {
            Reference<XResultSet> xRes = xMeta->getTableTypes();
            if(xRes.is())
            {
                ::rtl::OUString sView = ::rtl::OUString::createFromAscii("VIEW");
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
            // some dbs doesn't support this type so we should ask if a XViewsSupplier is supported
            if(!m_bSupportsViews)
            {
                Reference< XViewsSupplier > xMaster;
                m_xMasterTables = ::dbtools::getDataDefinitionByURLAndConnection(xMeta->getURL(),m_xMasterConnection,m_xORB);
                xMaster.set(m_xMasterTables,UNO_QUERY);

                if (xMaster.is() && xMaster->getViews().is())
                    m_bSupportsViews = sal_True;
            }
            if(m_bSupportsViews)
            {
                m_pViews = new OViewContainer(*this, m_aMutex, this, bCase,this,this);
                m_pViews->addContainerListener(m_pTables);
                m_pTables->addContainerListener(m_pViews);
            }
        }
    }
    catch(const SQLException&)
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


// IWarningsContainer
//------------------------------------------------------------------------------
void OConnection::appendWarning(const SQLException& _rWarning)
{
    implConcatWarnings(m_aAdditionalWarnings, makeAny(_rWarning));
}

//------------------------------------------------------------------------------
void OConnection::appendWarning(const SQLContext& _rContext)
{
    implConcatWarnings(m_aAdditionalWarnings, makeAny(_rContext));
}

//------------------------------------------------------------------------------
void OConnection::appendWarning(const SQLWarning& _rWarning)
{
    implConcatWarnings(m_aAdditionalWarnings, makeAny(_rWarning));
}

// XWarningsSupplier
//--------------------------------------------------------------------------
Any SAL_CALL OConnection::getWarnings() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    Reference<XWarningsSupplier> xWarnings;
    ::comphelper::query_aggregation(m_xProxyConnection,xWarnings);
    Any aReturn = xWarnings->getWarnings();
    if (!m_aAdditionalWarnings.hasValue())
        return aReturn;
    else
    {
        // copy m_aAdditionalWarnings, and append the original warnings
        Any aOverallWarnings(m_aAdditionalWarnings);
        implConcatWarnings(aOverallWarnings, aReturn);
        return aOverallWarnings;
    }
}

//--------------------------------------------------------------------------
void OConnection::implConcatWarnings(Any& _rChainLeft, const Any& _rChainRight)
{
    if (!_rChainLeft.hasValue())
        _rChainLeft = _rChainRight;
    else
    {
        // to travel the chain by reference (and not by value), we need the getValue ...
        // looks like a hack, but the meaning of getValue is documented, and it's the only chance for reference-traveling ....

        DBG_ASSERT(SQLExceptionInfo(_rChainLeft).isValid(), "OConnection::appendWarning: invalid warnings chain (this will crash)!");

        const SQLException* pChainTravel = static_cast<const SQLException*>(_rChainLeft.getValue());
        SQLExceptionIteratorHelper aReferenceIterHelper(pChainTravel);
        while (aReferenceIterHelper.hasMoreElements())
            pChainTravel = aReferenceIterHelper.next();

        // reached the end of the chain, and pChainTravel points to the last element
        const_cast<SQLException*>(pChainTravel)->NextException = _rChainRight;
    }
}

//--------------------------------------------------------------------------
void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    Reference<XWarningsSupplier> xWarnings;
    ::comphelper::query_aggregation(m_xProxyConnection,xWarnings);
    xWarnings->clearWarnings();
    m_aAdditionalWarnings.clear();
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OConnection::getTypes() throw (RuntimeException)
{
    if ( m_bSupportsViews )
        return concatSequences(OSubComponent::getTypes(), OConnection_Base::getTypes());

    // here views are supported
    Sequence<Type> aTypes = OSubComponent::getTypes();
    Sequence<Type> aConTypes = OConnection_Base::getTypes();
    sal_Int32 nSize = aTypes.getLength();
    aTypes.realloc(aTypes.getLength() + aConTypes.getLength() - 1);
    Type* pBegin    = aConTypes.getArray();
    Type* pEnd      = pBegin + aConTypes.getLength();
    Type aTypeToHide = getCppuType( (Reference<XViewsSupplier>*)0);
    for (; pBegin != pEnd; ++pBegin)
    {
        if(*pBegin != aTypeToHide)
            aTypes.getArray()[nSize++] = *pBegin;
    }
    return aTypes;
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
    if(!m_bSupportsViews && rType == getCppuType( (Reference<XViewsSupplier>*)0))
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
void OConnection::setParent(const Reference< XInterface > & Parent) throw( NoSupportException, RuntimeException )
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
    Reference< XSQLQueryComposer >  xComposer(new OQueryComposer(getTables(),this, m_xORB));
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

            // check if out "master connection" can supply tables
            if(!m_xMasterTables.is())
            {
                try
                {
                    Reference<XDatabaseMetaData> xMeta = getMetaData();
                    if ( xMeta.is() )
                        m_xMasterTables = ::dbtools::getDataDefinitionByURLAndConnection(xMeta->getURL(),m_xMasterConnection,m_xORB);
                }
                catch(SQLException&)
                {
                }
            }


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
            // check if out "master connection" can supply tables
            Reference< XViewsSupplier > xMaster(m_xMasterTables,UNO_QUERY);
            if(!m_xMasterTables.is())
            {
                try
                {
                    Reference<XDatabaseMetaData> xMeta = getMetaData();
                    if ( xMeta.is() )
                        m_xMasterTables = ::dbtools::getDataDefinitionByURLAndConnection(xMeta->getURL(),m_xMasterConnection,m_xORB);
                    xMaster.set(m_xMasterTables,UNO_QUERY);
                }
                catch(SQLException&)
                {
                }
            }

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
                sal_Bool bUseCatalogInSelect = ::dbtools::isDataSourcePropertyEnabled(*this,PROPERTY_USECATALOGINSELECT,sal_True);
                sal_Bool bUseSchemaInSelect = ::dbtools::isDataSourcePropertyEnabled(*this,PROPERTY_USESCHEMAINSELECT,sal_True);
                aStatement = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * FROM "));
                aStatement += ::dbtools::quoteTableName(getMetaData(), command,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
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
    if ( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER == _sServiceSpecifier )
    {
        xRet = new OSingleSelectQueryComposer(getTables(),this, m_xORB);
        m_aComposers.push_back(WeakReferenceHelper(xRet));
    }
    return Reference< XInterface >(xRet,UNO_QUERY);
}
// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OConnection::createInstanceWithArguments( const ::rtl::OUString& _sServiceSpecifier, const Sequence< Any >& Arguments ) throw (Exception, RuntimeException)
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
//........................................................................
}   // namespace dbaccess
//........................................................................

