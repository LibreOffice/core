/*************************************************************************
 *
 *  $RCSfile: connection.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:40 $
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
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#include "datasource.hxx"
#endif
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
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
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _UTL_UNO3_DB_TOOLS_HXX_
#include <unotools/dbtools.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#include "querycomposer.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;
using namespace ::osl;
using namespace ::utl;
using namespace ::cppu;
using namespace dbaccess;

//==========================================================================
//= OConnectionRerouter
//==========================================================================
DBG_NAME(OConnectionRerouter)
//--------------------------------------------------------------------------
OConnectionRerouter::OConnectionRerouter(const Reference< XConnection >& _rxMaster)
                    :m_xMasterConnection(_rxMaster)
{
    DBG_CTOR(OConnectionRerouter, NULL);
    DBG_ASSERT(m_xMasterConnection.is(), "OConnectionRerouter::OConnectionRerouter : invalid master connection !");
    if (!Reference< XWarningsSupplier >::query(m_xMasterConnection).is())
    {
        DBG_ERROR("OConnectionRerouter::OConnectionRerouter : the connection is assumed to be a warnings supplier ! Won't use it !");
        // as we're the owner of the conn and don't want to use it -> dispose
        Reference< XComponent > xConnComp(m_xMasterConnection, UNO_QUERY);
        if (xConnComp.is())
            xConnComp->dispose();

        m_xMasterConnection = NULL;
    }
}

//--------------------------------------------------------------------------
OConnectionRerouter::~OConnectionRerouter()
{
    DBG_DTOR(OConnectionRerouter, NULL);
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OConnectionRerouter::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OConnectionRerouter");
}

//------------------------------------------------------------------------------
sal_Bool OConnectionRerouter::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OConnectionRerouter::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBC_CONNECTION;
    aSNS[1] = SERVICE_SDB_CONNECTION;
    return aSNS;
}

// XCloseable
//------------------------------------------------------------------------------
void OConnectionRerouter::close(void) throw( SQLException, RuntimeException )
{
    checkDisposed();
    Reference< XComponent > xDerivedComp;
    if (queryInterface(::getCppuType(&xDerivedComp)) >>= xDerivedComp)
        // we ourself do not have an XComponent interface, but our derived classes do
        xDerivedComp->dispose();
}

//------------------------------------------------------------------------------
sal_Bool OConnectionRerouter::isClosed(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    return !m_xMasterConnection.is();
}

// XWarningsSupplier
//------------------------------------------------------------------------------
Any OConnectionRerouter::getWarnings(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference< XWarningsSupplier >  xIFace(m_xMasterConnection, UNO_QUERY);
        // assumed to be non-NULL, checked in the ctor
    return xIFace->getWarnings();
}

//------------------------------------------------------------------------------
void OConnectionRerouter::clearWarnings(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference< XWarningsSupplier >  xIFace(m_xMasterConnection, UNO_QUERY);
        // assumed to be non-NULL, checked in the ctor
    xIFace->clearWarnings();
}

// XConnection
//------------------------------------------------------------------------------
Reference< XStatement >  OConnectionRerouter::createStatement(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference< XStatement > xMasterStatement = m_xMasterConnection->createStatement();
    Reference< XStatement > xStatement = new OStatement(this, xMasterStatement);
    m_aStatements.push_back(WeakReferenceHelper(xStatement));
    return xStatement;
}

//------------------------------------------------------------------------------
Reference< XPreparedStatement >  OConnectionRerouter::prepareStatement(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    // TODO convert the SQL to SQL the driver understands
    Reference< XPreparedStatement > xMasterStatement = m_xMasterConnection->prepareStatement(sql);
    Reference< XPreparedStatement > xStatement = new OPreparedStatement(this, xMasterStatement);
    m_aStatements.push_back(WeakReferenceHelper(xStatement));
    return xStatement;
}

//------------------------------------------------------------------------------
Reference< XPreparedStatement >  OConnectionRerouter::prepareCall(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    Reference< XPreparedStatement > xMasterStatement = prepareCall(sql);
    Reference< XPreparedStatement > xStatement = new OCallableStatement(this, xMasterStatement);
    m_aStatements.push_back(WeakReferenceHelper(xStatement));
    return xStatement;
}

//------------------------------------------------------------------------------
rtl::OUString OConnectionRerouter::nativeSQL(const rtl::OUString& sql) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->nativeSQL(sql);
}

//------------------------------------------------------------------------------
void OConnectionRerouter::setAutoCommit(sal_Bool autoCommit) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setAutoCommit(autoCommit);
}

//------------------------------------------------------------------------------
sal_Bool OConnectionRerouter::getAutoCommit(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getAutoCommit();
}

//------------------------------------------------------------------------------
void OConnectionRerouter::commit(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->commit();
}

//------------------------------------------------------------------------------
void OConnectionRerouter::rollback(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->rollback();
}

//------------------------------------------------------------------------------
Reference< XDatabaseMetaData >  OConnectionRerouter::getMetaData(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getMetaData();
}

//------------------------------------------------------------------------------
void OConnectionRerouter::setReadOnly(sal_Bool readOnly) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setReadOnly(readOnly);
}

//------------------------------------------------------------------------------
sal_Bool OConnectionRerouter::isReadOnly(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->isReadOnly();
}

//------------------------------------------------------------------------------
void OConnectionRerouter::setCatalog(const rtl::OUString& catalog) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setCatalog(catalog);
}

//------------------------------------------------------------------------------
rtl::OUString OConnectionRerouter::getCatalog(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getCatalog();
}

//------------------------------------------------------------------------------
void OConnectionRerouter::setTransactionIsolation(sal_Int32 level) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setTransactionIsolation(level);
}

//------------------------------------------------------------------------------
sal_Int32 OConnectionRerouter::getTransactionIsolation(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getTransactionIsolation();
}

//------------------------------------------------------------------------------
Reference< XNameAccess >  OConnectionRerouter::getTypeMap(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    return m_xMasterConnection->getTypeMap();
}

//------------------------------------------------------------------------------
void OConnectionRerouter::setTypeMap(const Reference< XNameAccess > & typeMap) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();
    m_xMasterConnection->setTypeMap(typeMap);
}

//------------------------------------------------------------------------------
void OConnectionRerouter::disposing()
{
    MutexGuard aGuard(m_aMutex);

    for (OWeakRefArrayIterator i = m_aStatements.begin(); m_aStatements.end() != i; i++)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();

    // do not dispose the master connection ! this is the responsibility of the server component !
}

//==========================================================================
//= OConnection
//==========================================================================
DBG_NAME(OConnection)
//--------------------------------------------------------------------------
OConnection::OConnection(ODatabaseSource& _rDB, const Reference< XConnection >& _rxMaster, const Reference< XMultiServiceFactory >& _rxORB)
            :OConnectionRerouter(_rxMaster)
            ,OSubComponent(m_aMutex, static_cast< OWeakObject* >(&_rDB))
            ,m_aQueries(*this, m_aMutex, static_cast< XNameContainer* >(&_rDB.m_aCommandDefinitions), _rDB.m_aCommandDefinitions.getConfigLocation(), _rxORB)
                // as the queries reroute their refcounting to us, this m_aMutex is okey. If the queries
                // container would do it's own refcounting, it would have to aquire m_pMutex
            ,m_aTables(*this, m_aMutex)
                // same for tables
            ,m_aTableFilter(_rDB.m_aTableFilter)
            ,m_aTableTypeFilter(_rDB.m_aTableTypeFilter)
            ,m_xORB(_rxORB)
{
    DBG_CTOR(OConnection,NULL);

    // initialize the queries
    Reference< XRegistryKey > xParentConfigRoot(_rDB.m_xConfigurationNode);
    DBG_ASSERT(xParentConfigRoot.is(), "OConnection::OConnection : invalid configuration location of my parent !");
}

//--------------------------------------------------------------------------
OConnection::~OConnection()
{
    DBG_DTOR(OConnection,NULL);
}


// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OConnection::getTypes() throw (RuntimeException)
{
    return concatSequences(OSubComponent::getTypes(), OConnectionRerouter::getTypes(), OConnection_Base::getTypes());
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OConnection::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OConnection::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aReturn = OSubComponent::queryInterface( rType );
    if (!aReturn.hasValue())
        aReturn = OConnectionRerouter::queryInterface( rType );
    if (!aReturn.hasValue())
        aReturn = OConnection_Base::queryInterface( rType );
    return aReturn;
}

//--------------------------------------------------------------------------
void OConnection::acquire() throw (RuntimeException)
{
    OSubComponent::acquire();
}

//--------------------------------------------------------------------------
void OConnection::release() throw (RuntimeException)
{
    OSubComponent::release();
}

// OSubComponent
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    MutexGuard aGuard(m_aMutex);

    OSubComponent::disposing();
    OConnectionRerouter::disposing();

    m_aTables.dispose();
    m_aQueries.dispose();

    for (OWeakRefArrayIterator j = m_aComposers.begin(); m_aComposers.end() != j; j++)
    {
        Reference< XComponent > xComp(j->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }

    m_aComposers.clear();

    Reference< XCloseable > xMasterComp(m_xMasterConnection, UNO_QUERY);
    if (xMasterComp.is())
        xMasterComp->close();
    m_xMasterConnection = NULL;
}

// XChild
//------------------------------------------------------------------------------
Reference< XInterface >  OConnection::getParent(void) throw( RuntimeException )
{
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

    // get the supplier of the database
    ODatabaseSource* pParent = NULL;
    if (!getImplementation(pParent, m_xParent))
        throw RuntimeException();

    //  Reference< XNumberFormatsSupplier >  xSupplier = pParent->getNumberFormatsSupplier();
    Reference< XSQLQueryComposer >  xComposer(new OQueryComposer(getTables(),this, m_xORB));
    m_aComposers.push_back(WeakReferenceHelper(xComposer));
    return xComposer;
}

// XTablesSupplier
//------------------------------------------------------------------------------
Reference< XNameAccess >  OConnection::getTables() throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    if (!m_aTables.isInitialized())
    {
        // check if out "master connection" can supply tables
        Reference< XDriverAccess> xManager(m_xORB->createInstance(SERVICE_SDBC_DRIVERMANAGER), UNO_QUERY);
        Reference< XDataDefinitionSupplier > xSupp(xManager->getDriverByURL(m_xMasterConnection->getMetaData()->getURL()),UNO_QUERY);
        Reference< XTablesSupplier > xMasterTables;
        if(xSupp.is())
            xMasterTables = xSupp->getDataDefinitionByConnection(m_xMasterConnection);

        if (xMasterTables.is() && xMasterTables->getTables().is())
        {   // yes -> wrap them
            m_aTables.construct(xMasterTables->getTables(),m_aTableFilter, m_aTableTypeFilter);
        }
        else
        {   // no -> use an own container
            m_aTables.construct(this,m_aTableFilter, m_aTableTypeFilter);
        }
    }

    return static_cast< XNameAccess* >(&m_aTables);
}

// XQueriesSupplier
//------------------------------------------------------------------------------
Reference< XNameAccess >  OConnection::getQueries(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    return static_cast<XNameAccess*>(&m_aQueries);
}

// ::com::sun::star::sdb::XCommandPreparation
//------------------------------------------------------------------------------
Reference< XPreparedStatement >  SAL_CALL OConnection::prepareCommand( const ::rtl::OUString& command, sal_Int32 commandType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed();

    rtl::OUString aStatement;
    switch (commandType)
    {
        case CommandType::TABLE:
            aStatement = rtl::OUString::createFromAscii("SELECT * FROM ");
            aStatement += ::utl::quoteTableName(getMetaData(), command);
            break;
        case CommandType::QUERY:
            if (m_aQueries.hasByName(command))
            {
                Reference< XPropertySet > xQuery;
                m_aQueries.getByName(command) >>= xQuery;
                xQuery->getPropertyValue(PROPERTY_COMMAND) >>= aStatement;
            }
            break;
        default:
            aStatement = command;
    }
    // TODO EscapeProcessing
    return prepareStatement(aStatement);
}


