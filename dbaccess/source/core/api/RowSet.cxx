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

#include <sal/config.h>
#include <sal/log.hxx>

#include <map>
#include <utility>

#include <string.h>
#include "RowSet.hxx"
#include <stringconstants.hxx>
#include <sdbcoretools.hxx>
#include <SingleSelectQueryComposer.hxx>
#include "CRowSetColumn.hxx"
#include "CRowSetDataColumn.hxx"
#include "RowSetCache.hxx"
#include <strings.hrc>
#include <core_resource.hxx>
#include <tablecontainer.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <com/sun/star/sdb/RowChangeAction.hpp>
#include <com/sun/star/sdb/RowSetVetoException.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <comphelper/property.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/BlobHelper.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <unotools/syslocale.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/solar.h>
#include <unotools/configmgr.hxx>

using namespace utl;
using namespace dbaccess;
using namespace connectivity;
using namespace comphelper;
using namespace dbtools;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::util;
using namespace ::cppu;
using namespace ::osl;

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_dba_ORowSet_get_implementation(css::uno::XComponentContext* context,
                                                 css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ORowSet(context));
}

#define NOTIFY_LISTENERS_CHECK(_rListeners,T,method)                             \
    std::vector< Reference< XInterface > > aListenerSeq = _rListeners.getElements(); \
                                                                                  \
    _rGuard.clear();                                                              \
    bool bCheck = std::all_of(aListenerSeq.rbegin(), aListenerSeq.rend(),         \
        [&aEvt](Reference<XInterface>& rxItem) {                                  \
            try                                                                   \
            {                                                                     \
                return static_cast<bool>(static_cast<T*>(rxItem.get())->method(aEvt)); \
            }                                                                     \
            catch( RuntimeException& )                                            \
            {                                                                     \
                return true;                                                      \
            }                                                                     \
        });                                                                       \
    _rGuard.reset();


namespace dbaccess
{
ORowSet::ORowSet( const Reference< css::uno::XComponentContext >& _rxContext )
    :ORowSet_BASE1(m_aMutex)
    ,ORowSetBase( _rxContext, ORowSet_BASE1::rBHelper, &m_aMutex )
    ,m_aPrematureParamValues(new ORowSetValueVector)
    ,m_aParameterValueForCache(new ORowSetValueVector)
    ,m_aRowsetListeners(*m_pMutex)
    ,m_aApproveListeners(*m_pMutex)
    ,m_aRowsChangeListener(*m_pMutex)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_nFetchSize(50)
    ,m_nMaxFieldSize(0)
    ,m_nMaxRows(0)
    ,m_nQueryTimeOut(0)
    ,m_nCommandType(CommandType::COMMAND)
    ,m_nTransactionIsolation(0)
    ,m_nPrivileges(0)
    ,m_nLastKnownRowCount(0)
    ,m_nInAppend(0)
    ,m_bInsertingRow(false)
    ,m_bLastKnownRowCountFinal(false)
    ,m_bUseEscapeProcessing(true)
    ,m_bApplyFilter(false)
    ,m_bCommandFacetsDirty( true )
    ,m_bParametersDirty( true )
    ,m_bModified(false)
    ,m_bRebuildConnOnExecute(false)
    ,m_bIsBookmarkable(true)
    ,m_bNew(false)
    ,m_bCanUpdateInsertedRows(true)
    ,m_bOwnConnection(false)
    ,m_bPropChangeNotifyEnabled(true)
{
    m_nResultSetType = ResultSetType::SCROLL_SENSITIVE;
    m_nResultSetConcurrency = ResultSetConcurrency::UPDATABLE;
    m_pMySelf = this;
    m_aActiveConnection <<= m_xActiveConnection;

    sal_Int32 const nRBT = PropertyAttribute::READONLY   | PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;
    sal_Int32 const nRT  = PropertyAttribute::READONLY   | PropertyAttribute::TRANSIENT;
    sal_Int32 const nBT  = PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;

    m_aPrematureParamValues->get().resize( 0 );

    // sdb.RowSet Properties
    registerMayBeVoidProperty(PROPERTY_ACTIVE_CONNECTION,PROPERTY_ID_ACTIVE_CONNECTION, PropertyAttribute::MAYBEVOID|PropertyAttribute::TRANSIENT|PropertyAttribute::BOUND, &m_aActiveConnection,   cppu::UnoType<XConnection>::get());
    registerProperty(PROPERTY_DATASOURCENAME,       PROPERTY_ID_DATASOURCENAME,         PropertyAttribute::BOUND,       &m_aDataSourceName,     ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_COMMAND,              PROPERTY_ID_COMMAND,                PropertyAttribute::BOUND,       &m_aCommand,            ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_COMMAND_TYPE,         PROPERTY_ID_COMMAND_TYPE,           PropertyAttribute::BOUND,       &m_nCommandType,        ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_ACTIVECOMMAND,        PROPERTY_ID_ACTIVECOMMAND,          nRBT,                           &m_aActiveCommand,      ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_IGNORERESULT,         PROPERTY_ID_IGNORERESULT,           PropertyAttribute::BOUND,       &m_bIgnoreResult,       cppu::UnoType<bool>::get());
    registerProperty(PROPERTY_FILTER,               PROPERTY_ID_FILTER,                 PropertyAttribute::BOUND,       &m_aFilter,             ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_HAVING_CLAUSE,        PROPERTY_ID_HAVING_CLAUSE,          PropertyAttribute::BOUND,       &m_aHavingClause,       ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_GROUP_BY,             PROPERTY_ID_GROUP_BY,               PropertyAttribute::BOUND,       &m_aGroupBy,            ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_APPLYFILTER,          PROPERTY_ID_APPLYFILTER,            PropertyAttribute::BOUND,       &m_bApplyFilter,        cppu::UnoType<bool>::get());
    registerProperty(PROPERTY_ORDER,                PROPERTY_ID_ORDER,                  PropertyAttribute::BOUND,       &m_aOrder,              ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_PRIVILEGES,           PROPERTY_ID_PRIVILEGES,             nRT,                            &m_nPrivileges,         ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_ISMODIFIED,           PROPERTY_ID_ISMODIFIED,             nBT,                            &m_bModified,           cppu::UnoType<bool>::get());
    registerProperty(PROPERTY_ISNEW,                PROPERTY_ID_ISNEW,                  nRBT,                           &m_bNew,                cppu::UnoType<bool>::get());
    registerProperty(PROPERTY_SINGLESELECTQUERYCOMPOSER,PROPERTY_ID_SINGLESELECTQUERYCOMPOSER,  nRT,                    &m_xComposer,   cppu::UnoType<XSingleSelectQueryComposer>::get());

    // sdbcx.ResultSet Properties
    registerProperty(PROPERTY_ISBOOKMARKABLE,       PROPERTY_ID_ISBOOKMARKABLE,         nRT,                            &m_bIsBookmarkable,      cppu::UnoType<bool>::get());
    registerProperty(PROPERTY_CANUPDATEINSERTEDROWS,PROPERTY_ID_CANUPDATEINSERTEDROWS,  nRT,                            &m_bCanUpdateInsertedRows,      cppu::UnoType<bool>::get());
    // sdbc.ResultSet Properties
    registerProperty(PROPERTY_RESULTSETCONCURRENCY, PROPERTY_ID_RESULTSETCONCURRENCY,   PropertyAttribute::TRANSIENT,   &m_nResultSetConcurrency,::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_RESULTSETTYPE,        PROPERTY_ID_RESULTSETTYPE,          PropertyAttribute::TRANSIENT,   &m_nResultSetType,      ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_FETCHDIRECTION,       PROPERTY_ID_FETCHDIRECTION,         PropertyAttribute::TRANSIENT,   &m_nFetchDirection,     ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_FETCHSIZE,            PROPERTY_ID_FETCHSIZE,              PropertyAttribute::TRANSIENT,   &m_nFetchSize,          ::cppu::UnoType<sal_Int32>::get());

    // sdbc.RowSet Properties
    registerProperty(PROPERTY_URL,                  PROPERTY_ID_URL,                    0,                              &m_aURL,                ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_TRANSACTIONISOLATION, PROPERTY_ID_TRANSACTIONISOLATION,   PropertyAttribute::TRANSIENT,   &m_nTransactionIsolation,::cppu::UnoType<sal_Int32>::get());
    registerMayBeVoidProperty(PROPERTY_TYPEMAP,     PROPERTY_ID_TYPEMAP,                PropertyAttribute::MAYBEVOID|PropertyAttribute::TRANSIENT,  &m_aTypeMap,            cppu::UnoType<XNameAccess>::get());
    registerProperty(PROPERTY_ESCAPE_PROCESSING,PROPERTY_ID_ESCAPE_PROCESSING,  PropertyAttribute::BOUND,       &m_bUseEscapeProcessing,cppu::UnoType<bool>::get()  );
    registerProperty(PROPERTY_QUERYTIMEOUT,         PROPERTY_ID_QUERYTIMEOUT,           PropertyAttribute::TRANSIENT,   &m_nQueryTimeOut,       ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_MAXFIELDSIZE,         PROPERTY_ID_MAXFIELDSIZE,           PropertyAttribute::TRANSIENT,   &m_nMaxFieldSize,       ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_MAXROWS,              PROPERTY_ID_MAXROWS,                0,                              &m_nMaxRows,            ::cppu::UnoType<sal_Int32>::get() );
    registerProperty(PROPERTY_USER,                 PROPERTY_ID_USER,                   PropertyAttribute::TRANSIENT,   &m_aUser,               ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_PASSWORD,             PROPERTY_ID_PASSWORD,               PropertyAttribute::TRANSIENT,   &m_aPassword,           ::cppu::UnoType<OUString>::get());

    registerProperty(PROPERTY_UPDATE_CATALOGNAME,   PROPERTY_ID_UPDATE_CATALOGNAME,     PropertyAttribute::BOUND,       &m_aUpdateCatalogName,  ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_UPDATE_SCHEMANAME,    PROPERTY_ID_UPDATE_SCHEMANAME,      PropertyAttribute::BOUND,       &m_aUpdateSchemaName,   ::cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_UPDATE_TABLENAME,     PROPERTY_ID_UPDATE_TABLENAME,       PropertyAttribute::BOUND,       &m_aUpdateTableName,    ::cppu::UnoType<OUString>::get());

    // ???
    registerProperty(PROPERTY_CHANGE_NOTIFICATION_ENABLED, PROPERTY_ID_PROPCHANGE_NOTIFY, PropertyAttribute::BOUND,     &m_bPropChangeNotifyEnabled, cppu::UnoType<bool>::get());
}

ORowSet::~ORowSet()
{
    if ( !m_rBHelper.bDisposed && !m_rBHelper.bInDispose )
    {
        SAL_WARN("dbaccess", "Please check who doesn't dispose this component!");
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

void ORowSet::getPropertyDefaultByHandle( sal_Int32 _nHandle, Any& _rDefault ) const
{
    switch( _nHandle )
    {
        case PROPERTY_ID_COMMAND_TYPE:
            _rDefault <<= static_cast<sal_Int32>(CommandType::COMMAND);
            break;
        case PROPERTY_ID_IGNORERESULT:
            _rDefault <<= false;
            break;
        case PROPERTY_ID_APPLYFILTER:
            _rDefault <<= false;
            break;
        case PROPERTY_ID_ISMODIFIED:
            _rDefault <<= false;
            break;
        case PROPERTY_ID_ISBOOKMARKABLE:
            _rDefault <<= true;
            break;
        case PROPERTY_ID_CANUPDATEINSERTEDROWS:
            _rDefault <<= true;
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            _rDefault <<= ResultSetType::SCROLL_INSENSITIVE;
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            _rDefault <<= ResultSetConcurrency::UPDATABLE;
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            _rDefault <<= FetchDirection::FORWARD;
            break;
        case PROPERTY_ID_FETCHSIZE:
            _rDefault <<= static_cast<sal_Int32>(1);
            break;
        case PROPERTY_ID_ESCAPE_PROCESSING:
            _rDefault <<= true;
            break;
        case PROPERTY_ID_MAXROWS:
            _rDefault <<= sal_Int32( 0 );
            break;
        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_HAVING_CLAUSE:
        case PROPERTY_ID_GROUP_BY:
        case PROPERTY_ID_ORDER:
        case PROPERTY_ID_UPDATE_CATALOGNAME:
        case PROPERTY_ID_UPDATE_SCHEMANAME:
        case PROPERTY_ID_UPDATE_TABLENAME:
            _rDefault <<= OUString();
            break;
    }
}

void SAL_CALL ORowSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISMODIFIED:
            m_bModified = cppu::any2bool(rValue);
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            if( m_nResultSetType == ResultSetType::FORWARD_ONLY)
                throw Exception("resultsettype is FORWARD_ONLY", nullptr);
            [[fallthrough]];
        default:
            OPropertyStateContainer::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }

    if  (   ( nHandle == PROPERTY_ID_ACTIVE_CONNECTION )
        ||  ( nHandle == PROPERTY_ID_DATASOURCENAME )
        ||  ( nHandle == PROPERTY_ID_COMMAND )
        ||  ( nHandle == PROPERTY_ID_COMMAND_TYPE )
        ||  ( nHandle == PROPERTY_ID_IGNORERESULT )
        ||  ( nHandle == PROPERTY_ID_FILTER )
        ||  ( nHandle == PROPERTY_ID_HAVING_CLAUSE )
        ||  ( nHandle == PROPERTY_ID_GROUP_BY )
        ||  ( nHandle == PROPERTY_ID_APPLYFILTER )
        ||  ( nHandle == PROPERTY_ID_ORDER )
        ||  ( nHandle == PROPERTY_ID_URL )
        ||  ( nHandle == PROPERTY_ID_USER )
        )
    {
        m_bCommandFacetsDirty = true;
    }


    switch(nHandle)
    {
        case PROPERTY_ID_ACTIVE_CONNECTION:
            // the new connection
            {
                assert(m_aActiveConnection == rValue);
                Reference< XConnection > xNewConnection(m_aActiveConnection,UNO_QUERY);
                setActiveConnection(xNewConnection, false);
            }

            m_bOwnConnection        = false;
            m_bRebuildConnOnExecute = false;
            break;

        case PROPERTY_ID_DATASOURCENAME:
            if(!m_xStatement.is())
            {
                Reference< XConnection >  xNewConn;
                Any aNewConn;
                aNewConn <<= xNewConn;
                setFastPropertyValue(PROPERTY_ID_ACTIVE_CONNECTION, aNewConn);
            }
            else
                m_bRebuildConnOnExecute = true;
            break;
        case PROPERTY_ID_FETCHSIZE:
            if(m_pCache)
            {
                m_pCache->setFetchSize(m_nFetchSize);
                fireRowcount();
            }
            break;
        case PROPERTY_ID_URL:
            // is the connection-to-be-built determined by the url (which is the case if m_aDataSourceName is empty) ?
            if (m_aDataSourceName.isEmpty())
            {
                // are we active at the moment ?
                if (m_xStatement.is())
                    // yes -> the next execute needs to rebuild our connection because of this new property
                    m_bRebuildConnOnExecute = true;
                else
                {   // no -> drop our active connection (if we have one) as it doesn't correspond to this new property value anymore
                    Reference< XConnection >  xNewConn;
                    Any aNewConn;
                    aNewConn <<= xNewConn;
                    setFastPropertyValue(PROPERTY_ID_ACTIVE_CONNECTION, aNewConn);
                }
            }
            m_bOwnConnection = true;
            break;
        case PROPERTY_ID_TYPEMAP:
            m_xTypeMap.set(m_aTypeMap, css::uno::UNO_QUERY);
            break;
        case PROPERTY_ID_PROPCHANGE_NOTIFY:
            m_bPropChangeNotifyEnabled = ::cppu::any2bool(rValue);
            break;
        default:
            break;
    };
}

void SAL_CALL ORowSet::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_pCache)
    {
        switch(nHandle)
        {
        case PROPERTY_ID_ISMODIFIED:
            rValue <<= m_bModified;
            break;
        case PROPERTY_ID_ISNEW:
            rValue <<= m_bNew;
            break;
        case PROPERTY_ID_PRIVILEGES:
            rValue <<= m_pCache->m_nPrivileges;
            break;
        case PROPERTY_ID_ACTIVE_CONNECTION:
            rValue <<= m_xActiveConnection;
            break;
        case PROPERTY_ID_TYPEMAP:
            rValue <<= m_xTypeMap;
            break;
        default:
            ORowSetBase::getFastPropertyValue(rValue,nHandle);
        }
    }
    else
    {
        switch(nHandle)
        {
            case PROPERTY_ID_ACTIVE_CONNECTION:
                rValue <<= m_xActiveConnection;
                break;
            case PROPERTY_ID_TYPEMAP:
                rValue <<= m_xTypeMap;
                break;
            case PROPERTY_ID_PROPCHANGE_NOTIFY:
                rValue <<= m_bPropChangeNotifyEnabled;
                break;
            default:
                ORowSetBase::getFastPropertyValue(rValue,nHandle);
        }
    }
}

// css::XTypeProvider
Sequence< Type > SAL_CALL ORowSet::getTypes()
{
    OTypeCollection aTypes(cppu::UnoType<XPropertySet>::get(),
                            cppu::UnoType<XFastPropertySet>::get(),
                            cppu::UnoType<XMultiPropertySet>::get(),
                           ::comphelper::concatSequences(ORowSet_BASE1::getTypes(),ORowSetBase::getTypes()));
    return aTypes.getTypes();
}

Sequence< sal_Int8 > SAL_CALL ORowSet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// css::XInterface
Any SAL_CALL ORowSet::queryInterface( const Type & rType )
{
    return ORowSet_BASE1::queryInterface( rType);
}

void SAL_CALL ORowSet::acquire() throw()
{
    ORowSet_BASE1::acquire();
}

void SAL_CALL ORowSet::release() throw()
{
    ORowSet_BASE1::release();
}

// css::XUnoTunnel
sal_Int64 SAL_CALL ORowSet::getSomething( const Sequence< sal_Int8 >& rId )
{
    if (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}

Sequence< sal_Int8 > ORowSet::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId s_Id;

    return s_Id.getImplementationId();
}

// css::XAggregation
Any SAL_CALL ORowSet::queryAggregation( const Type& rType )
{
    Any aRet(ORowSetBase::queryInterface(rType));
    if (!aRet.hasValue())
        aRet = ORowSet_BASE1::queryAggregation(rType);
    return aRet;
}

// css::XServiceInfo
OUString SAL_CALL ORowSet::getImplementationName()
{
    return OUString("com.sun.star.comp.dba.ORowSet");
}

sal_Bool SAL_CALL ORowSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL ORowSet::getSupportedServiceNames()
{
    Sequence< OUString > aSNS( 5 );
    aSNS[0] = SERVICE_SDBC_RESULTSET;
    aSNS[1] = SERVICE_SDBC_ROWSET;
    aSNS[2] = SERVICE_SDBCX_RESULTSET;
    aSNS[3] = SERVICE_SDB_RESULTSET;
    aSNS[4] = SERVICE_SDB_ROWSET;
    return aSNS;
}

// OComponentHelper
void SAL_CALL ORowSet::disposing()
{
    OPropertyStateContainer::disposing();

    MutexGuard aGuard(m_aMutex);
    EventObject aDisposeEvent;
    aDisposeEvent.Source = static_cast< XComponent* >(this);
    m_aRowsetListeners.disposeAndClear( aDisposeEvent );
    m_aApproveListeners.disposeAndClear( aDisposeEvent );
    m_aRowsChangeListener.disposeAndClear( aDisposeEvent );

    freeResources( true );

    // remove myself as dispose listener
    Reference< XComponent >  xComponent(m_xActiveConnection, UNO_QUERY);
    if (xComponent.is())
    {
        Reference<XEventListener> xEvt;
        query_aggregation(this,xEvt);
        xComponent->removeEventListener(xEvt);
    }

    m_aActiveConnection = Any(); // the any contains a reference too
    if(m_bOwnConnection)
        ::comphelper::disposeComponent(m_xActiveConnection);
    m_xActiveConnection = nullptr;


    ORowSetBase::disposing();
}

void ORowSet::freeResources( bool _bComplete )
{
    MutexGuard aGuard(m_aMutex);

    // free all clones
    for (auto const& clone : m_aClones)
    {
        Reference< XComponent > xComp(clone.get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aClones.clear();

    doCancelModification();

    m_aBookmark     = Any();
    m_bBeforeFirst  = true;
    m_bAfterLast    = false;
    m_bNew          = false;
    m_bModified     = false;
    m_bIsInsertRow  = false;
    m_bLastKnownRowCountFinal = false;
    m_nLastKnownRowCount      = 0;

    if ( _bComplete )
    {
        // the columns must be disposed before the querycomposer is disposed because
        // their owner can be the composer
        TDataColumns().swap(m_aDataColumns);// clear and resize capacity
        std::vector<bool>().swap(m_aReadOnlyDataColumns);

        m_xColumns      = nullptr;
        if ( m_pColumns )
            m_pColumns->disposing();
        // dispose the composer to avoid that everybody knows that the querycomposer is eol
        try { ::comphelper::disposeComponent( m_xComposer ); }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
            m_xComposer = nullptr;
        }

        // let our warnings container forget the reference to the (possibly disposed) old result set
        m_aWarnings.setExternalWarnings( nullptr );

        m_pCache.reset();

        impl_resetTables_nothrow();

        m_xStatement    = nullptr;
        m_xTypeMap      = nullptr;

        if ( m_aOldRow.is() )
            m_aOldRow->clearRow();

        impl_disposeParametersContainer_nothrow();

        m_bCommandFacetsDirty = true;
    }
}

void ORowSet::setActiveConnection( Reference< XConnection > const & _rxNewConn, bool _bFireEvent )
{
    if (_rxNewConn.get() == m_xActiveConnection.get())
        // nothing to do
        return;

    // remove the event listener for the old connection
    Reference< XComponent >  xComponent(m_xActiveConnection, UNO_QUERY);
    if (xComponent.is())
    {
        Reference<XEventListener> xListener;
        query_aggregation(this, xListener);
        xComponent->removeEventListener(xListener);
    }

    // if we owned the connection, remember it for later disposing
    if(m_bOwnConnection)
        m_xOldConnection = m_xActiveConnection;

    // for firing the PropertyChangeEvent
    sal_Int32 nHandle = PROPERTY_ID_ACTIVE_CONNECTION;
    Any aOldConnection; aOldConnection <<= m_xActiveConnection;
    Any aNewConnection; aNewConnection <<= _rxNewConn;

    // set the new connection
    m_xActiveConnection = _rxNewConn;
    if (m_xActiveConnection.is())
        m_aActiveConnection <<= m_xActiveConnection;
    else
        m_aActiveConnection.clear();

    // fire the event
    if (_bFireEvent)
        fire(&nHandle, &aNewConnection, &aOldConnection, 1, false);

    // register as event listener for the new connection
    xComponent.set(m_xActiveConnection,UNO_QUERY);
    if (xComponent.is())
    {
        Reference<XEventListener> xListener;
        query_aggregation(this, xListener);
        xComponent->addEventListener(xListener);
    }
}

// css::XEventListener
void SAL_CALL ORowSet::disposing( const css::lang::EventObject& Source )
{
    // close rowset because the connection is going to be deleted (someone told me :-)
    Reference<XConnection> xCon(Source.Source,UNO_QUERY);
    if(m_xActiveConnection == xCon)
    {
        close();
        {
            MutexGuard aGuard( m_aMutex );
            Reference< XConnection > xXConnection;
            setActiveConnection( xXConnection );
        }
    }
}

// XCloseable
void SAL_CALL ORowSet::close(  )
{
    {
        MutexGuard aGuard( m_aMutex );
        ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    }
    // additionals things to set
    freeResources( true );
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ORowSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& SAL_CALL ORowSet::getInfoHelper()
{
    return *::comphelper::OPropertyArrayUsageHelper<ORowSet>::getArrayHelper();
}

void ORowSet::updateValue(sal_Int32 columnIndex,const ORowSetValue& x)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);
    checkUpdateIterator();

    ORowSetValueVector::Vector& rRow = (*m_aCurrentRow)->get();
    ORowSetNotifier aNotify(this,rRow);
    m_pCache->updateValue(columnIndex,x,rRow,aNotify.getChangedColumns());
    m_bModified = m_bModified || !aNotify.getChangedColumns().empty();
    aNotify.firePropertyChange();
}

// XRowUpdate
void SAL_CALL ORowSet::updateNull( sal_Int32 columnIndex )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);
    checkUpdateIterator();

    ORowSetValueVector::Vector& rRow = (*m_aCurrentRow)->get();
    ORowSetNotifier aNotify(this,rRow);
    m_pCache->updateNull(columnIndex,rRow,aNotify.getChangedColumns());
    m_bModified = m_bModified || !aNotify.getChangedColumns().empty();
    aNotify.firePropertyChange();
}

void SAL_CALL ORowSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x )
{
    updateValue(columnIndex, static_cast<bool>(x));
}

void SAL_CALL ORowSet::updateByte( sal_Int32 columnIndex, sal_Int8 x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateShort( sal_Int32 columnIndex, sal_Int16 x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateInt( sal_Int32 columnIndex, sal_Int32 x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateLong( sal_Int32 columnIndex, sal_Int64 x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateFloat( sal_Int32 columnIndex, float x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateDouble( sal_Int32 columnIndex, double x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateString( sal_Int32 columnIndex, const OUString& x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateDate( sal_Int32 columnIndex, const css::util::Date& x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateTime( sal_Int32 columnIndex, const css::util::Time& x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL ORowSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);
    checkUpdateIterator();

    {
        Sequence<sal_Int8> aSeq;
        if(x.is())
            x->readBytes(aSeq,length);
        updateValue(columnIndex,aSeq);
    }
}

void SAL_CALL ORowSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);
    checkUpdateIterator();
    ORowSetValueVector::Vector& rRow = (*m_aCurrentRow)->get();
    ORowSetNotifier aNotify(this,rRow);
    m_pCache->updateCharacterStream(columnIndex,x,length,rRow,aNotify.getChangedColumns());
    m_bModified = m_bModified || !aNotify.getChangedColumns().empty();
    aNotify.firePropertyChange();
}

void SAL_CALL ORowSet::updateObject( sal_Int32 columnIndex, const Any& x )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);
    checkUpdateIterator();

    Any aNewValue = x;

    if ( m_pColumns )
    {
        Reference<XPropertySet> xColumn(m_pColumns->getByIndex(columnIndex-1),UNO_QUERY);
        sal_Int32 nColType = 0;
        xColumn->getPropertyValue(PROPERTY_TYPE) >>= nColType;
        switch( nColType )
        {
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            {
                double nValue = 0;
                if ( x >>= nValue )
                {
                    if ( DataType::TIMESTAMP == nColType )
                        aNewValue <<= dbtools::DBTypeConversion::toDateTime( nValue );
                    else if ( DataType::DATE == nColType )
                        aNewValue <<= dbtools::DBTypeConversion::toDate( nValue );
                    else
                        aNewValue <<= dbtools::DBTypeConversion::toTime( nValue );
                }
                break;
            }
        }
    }

    if (!::dbtools::implUpdateObject(this, columnIndex, aNewValue))
    {   // there is no other updateXXX call which can handle the value in x
        ORowSetValueVector::Vector& rRow = (*m_aCurrentRow)->get();
        ORowSetNotifier aNotify(this,rRow);
        m_pCache->updateObject(columnIndex,aNewValue,rRow,aNotify.getChangedColumns());
        m_bModified = m_bModified || !aNotify.getChangedColumns().empty();
        aNotify.firePropertyChange();
    }
}

void SAL_CALL ORowSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/ )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);
    checkUpdateIterator();
    ORowSetValueVector::Vector& rRow = (*m_aCurrentRow)->get();
    ORowSetNotifier aNotify(this,rRow);
    m_pCache->updateNumericObject(columnIndex,x,rRow,aNotify.getChangedColumns());
    m_bModified = m_bModified || !aNotify.getChangedColumns().empty();
    aNotify.firePropertyChange();
}

namespace
{
    class ProtectFlag
    {
        bool& m_rInsertingRow;
    public:
        explicit ProtectFlag(bool& rInsertingRow)
            : m_rInsertingRow(rInsertingRow)
        {
            if (m_rInsertingRow)
            {
                throw std::runtime_error("recursion in insertRow");
            }
            m_rInsertingRow = true;
        }
        ~ProtectFlag()
        {
            m_rInsertingRow = false;
        }
    };
}

// XResultSetUpdate
void SAL_CALL ORowSet::insertRow()
{
    ProtectFlag aFlagControl(m_bInsertingRow);

    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    // insertRow is not allowed when
    // standing not on the insert row nor
    // when the row isn't modified
    // or the concurrency is read only
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    if(!m_pCache || !m_bNew || !m_bModified || m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY)
        throwFunctionSequenceException(*this);

    // remember old value for fire
    bool bOld = m_bNew;

    ORowSetRow aOldValues;
    if ( !m_aCurrentRow.isNull() )
        aOldValues = new ORowSetValueVector( *(*m_aCurrentRow));
    Sequence<Any> aChangedBookmarks;
    RowsChangeEvent aEvt(*this,RowChangeAction::INSERT,1,aChangedBookmarks);
    notifyAllListenersRowBeforeChange(aGuard,aEvt);

    std::vector< Any > aBookmarks;
    bool bInserted = m_pCache->insertRow(aBookmarks);

    // make sure that our row is set to the new inserted row before clearing the insert flags in the cache
    m_pCache->resetInsertRow(bInserted);

    // notification order
    // - column values
    setCurrentRow( false, true, aOldValues, aGuard ); // we don't move here

    // read-only flag restored
    impl_restoreDataColumnsWriteable_throw();

    // - rowChanged
    notifyAllListenersRowChanged(aGuard,aEvt);

    if ( !aBookmarks.empty() )
    {
        RowsChangeEvent aUpEvt(*this,RowChangeAction::UPDATE,aBookmarks.size(),comphelper::containerToSequence(aBookmarks));
        notifyAllListenersRowChanged(aGuard,aUpEvt);
    }

    // - IsModified
    if(!m_bModified)
        fireProperty(PROPERTY_ID_ISMODIFIED,false,true);
    OSL_ENSURE( !m_bModified, "ORowSet::insertRow: just updated, but _still_ modified?" );

    // - IsNew
    if(m_bNew != bOld)
        fireProperty(PROPERTY_ID_ISNEW,m_bNew,bOld);

    // - RowCount/IsRowCountFinal
    fireRowcount();
}

void SAL_CALL ORowSet::updateRow(  )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    // not allowed when standing on insert row
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    if ( !m_pCache || m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY || m_bNew || ((m_pCache->m_nPrivileges & Privilege::UPDATE ) != Privilege::UPDATE) )
        throwFunctionSequenceException(*this);


    if(m_bModified)
    {
        ORowSetRow aOldValues;
        if ( !m_aCurrentRow.isNull() )
            aOldValues = new ORowSetValueVector( *(*m_aCurrentRow) );

        Sequence<Any> aChangedBookmarks;
        RowsChangeEvent aEvt(*this,RowChangeAction::UPDATE,1,aChangedBookmarks);
        notifyAllListenersRowBeforeChange(aGuard,aEvt);

        std::vector< Any > aBookmarks;
        m_pCache->updateRow(m_aCurrentRow.operator ->(),aBookmarks);
        if ( !aBookmarks.empty() )
            aEvt.Bookmarks = comphelper::containerToSequence(aBookmarks);
        aEvt.Rows += aBookmarks.size();
        m_aBookmark     = m_pCache->getBookmark();
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = false;
        if ( m_pCache->m_aMatrixIter != m_pCache->getEnd() && (*m_pCache->m_aMatrixIter).is() )
        {
            if ( m_pCache->isResultSetChanged() )
            {
                impl_rebuild_throw(aGuard);
            }
            else
            {
                m_aOldRow->setRow(new ORowSetValueVector(*(*m_aCurrentRow)));

                // notification order
                // - column values
                ORowSetBase::firePropertyChange(aOldValues);
            }
            // - rowChanged
            notifyAllListenersRowChanged(aGuard,aEvt);

            // - IsModified
            if(!m_bModified)
                fireProperty(PROPERTY_ID_ISMODIFIED,false,true);
            OSL_ENSURE( !m_bModified, "ORowSet::updateRow: just updated, but _still_ modified?" );

            // - RowCount/IsRowCountFinal
            fireRowcount();
        }
        else if ( !m_bAfterLast ) // the update went wrong
        {
            ::dbtools::throwSQLException( DBA_RES( RID_STR_UPDATE_FAILED ), StandardSQLState::INVALID_CURSOR_POSITION, *this );
        }
    }
}

void SAL_CALL ORowSet::deleteRow(  )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_DELETE_BEFORE_AFTER ), StandardSQLState::INVALID_CURSOR_POSITION, *this );
    if ( m_bNew )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_DELETE_INSERT_ROW ), StandardSQLState::INVALID_CURSOR_POSITION, *this );
    if  ( m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_RESULT_IS_READONLY ), StandardSQLState::FUNCTION_SEQUENCE_ERROR, *this );
    if ( ( m_pCache->m_nPrivileges & Privilege::DELETE ) != Privilege::DELETE )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_DELETE_PRIVILEGE ), StandardSQLState::FUNCTION_SEQUENCE_ERROR, *this );
    if ( rowDeleted() )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_ROW_ALREADY_DELETED ), StandardSQLState::FUNCTION_SEQUENCE_ERROR, *this );

    // this call position the cache indirect
    Any aBookmarkToDelete( m_aBookmark );
    positionCache( MOVE_NONE );
    sal_Int32 nDeletePosition = m_pCache->getRow();

    notifyRowSetAndClonesRowDelete( aBookmarkToDelete );

    ORowSetRow aOldValues;
    if ( m_pCache->m_aMatrixIter != m_pCache->getEnd() && m_pCache->m_aMatrixIter->is() )
        aOldValues = new ORowSetValueVector( *(*(m_pCache->m_aMatrixIter)) );

    Sequence<Any> aChangedBookmarks;
    RowsChangeEvent aEvt(*this,RowChangeAction::DELETE,1,aChangedBookmarks);
    notifyAllListenersRowBeforeChange(aGuard,aEvt);

    m_pCache->deleteRow();
    notifyRowSetAndClonesRowDeleted( aBookmarkToDelete, nDeletePosition );

    ORowSetNotifier aNotifier( this );
        // this will call cancelRowModification on the cache if necessary

    // notification order
    // - rowChanged
    notifyAllListenersRowChanged(aGuard,aEvt);

    // - IsModified
    // - IsNew
    aNotifier.fire( );

    // - RowCount/IsRowCountFinal
    fireRowcount();
}

void ORowSet::implCancelRowUpdates( bool _bNotifyModified )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );
    if ( m_bBeforeFirst || m_bAfterLast || rowDeleted() )
        return; // nothing to do so return

    checkCache();
    // cancelRowUpdates is not allowed when:
    // - standing on the insert row
    // - the concurrency is read only
    // - the current row is deleted
    if ( m_bNew || m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY )
        throwFunctionSequenceException(*this);

    positionCache( MOVE_NONE );

    ORowSetRow aOldValues;
    if ( !m_bModified && _bNotifyModified && !m_aCurrentRow.isNull() )
        aOldValues = new ORowSetValueVector( *(*m_aCurrentRow) );

    m_pCache->cancelRowUpdates();

    m_aBookmark     = m_pCache->getBookmark();
    m_aCurrentRow   = m_pCache->m_aMatrixIter;
    m_bIsInsertRow  = false;

    // notification order
    // IsModified
    if( !m_bModified && _bNotifyModified )
    {
        // - column values
        ORowSetBase::firePropertyChange(aOldValues);
        fireProperty(PROPERTY_ID_ISMODIFIED,false,true);
    }
}

void SAL_CALL ORowSet::cancelRowUpdates(  )
{
    implCancelRowUpdates( true );
}

void SAL_CALL ORowSet::addRowSetListener( const Reference< XRowSetListener >& listener )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    if(listener.is())
        m_aRowsetListeners.addInterface(listener);
}

void SAL_CALL ORowSet::removeRowSetListener( const Reference< XRowSetListener >& listener )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    if(listener.is())
        m_aRowsetListeners.removeInterface(listener);
}

void ORowSet::notifyAllListeners(::osl::ResettableMutexGuard& _rGuard)
{
    EventObject aEvt(*m_pMySelf);
    _rGuard.clear();
    m_aRowsetListeners.notifyEach( &XRowSetListener::rowSetChanged, aEvt );
    _rGuard.reset();
}

void ORowSet::notifyAllListenersCursorMoved(::osl::ResettableMutexGuard& _rGuard)
{
    EventObject aEvt(*m_pMySelf);
    _rGuard.clear();
    m_aRowsetListeners.notifyEach( &XRowSetListener::cursorMoved, aEvt );
    _rGuard.reset();
}

void ORowSet::notifyAllListenersRowChanged(::osl::ResettableMutexGuard& _rGuard, const RowsChangeEvent& aEvt)
{
    _rGuard.clear();
    m_aRowsetListeners.notifyEach( &XRowSetListener::rowChanged, static_cast<EventObject>(aEvt) );
    m_aRowsChangeListener.notifyEach( &XRowsChangeListener::rowsChanged, aEvt );
    _rGuard.reset();
}

bool ORowSet::notifyAllListenersCursorBeforeMove(::osl::ResettableMutexGuard& _rGuard)
{
    EventObject aEvt(*m_pMySelf);
    NOTIFY_LISTENERS_CHECK(m_aApproveListeners,XRowSetApproveListener,approveCursorMove);
    return bCheck;
}

void ORowSet::notifyAllListenersRowBeforeChange(::osl::ResettableMutexGuard& _rGuard,const RowChangeEvent &aEvt)
{
    NOTIFY_LISTENERS_CHECK(m_aApproveListeners,XRowSetApproveListener,approveRowChange);
    if ( !bCheck )
        m_aErrors.raiseTypedException( sdb::ErrorCondition::ROW_SET_OPERATION_VETOED, *this, ::cppu::UnoType< RowSetVetoException >::get() );
}

void ORowSet::fireRowcount()
{
    sal_Int32 nCurrentRowCount( impl_getRowCount() );
    bool bCurrentRowCountFinal( m_pCache->m_bRowCountFinal );

    if ( m_nLastKnownRowCount != nCurrentRowCount )
    {
        sal_Int32 nHandle = PROPERTY_ID_ROWCOUNT;
        Any aNew,aOld;
        aNew <<= nCurrentRowCount; aOld <<= m_nLastKnownRowCount;
        fire(&nHandle,&aNew,&aOld,1,false);
        m_nLastKnownRowCount = nCurrentRowCount;
    }
    if ( !m_bLastKnownRowCountFinal && ( m_bLastKnownRowCountFinal != bCurrentRowCountFinal ) )
    {
        sal_Int32 nHandle = PROPERTY_ID_ISROWCOUNTFINAL;
        Any aNew,aOld;
        aNew <<= bCurrentRowCountFinal;
        aOld <<= m_bLastKnownRowCountFinal;
        fire(&nHandle,&aNew,&aOld,1,false);
        m_bLastKnownRowCountFinal = bCurrentRowCountFinal;
    }
}

void SAL_CALL ORowSet::moveToInsertRow(  )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();
    if ( ( m_pCache->m_nPrivileges & Privilege::INSERT ) != Privilege::INSERT )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_INSERT_PRIVILEGE ), StandardSQLState::GENERAL_ERROR, *this );

    if ( notifyAllListenersCursorBeforeMove( aGuard ) )
    {
        // remember old value for fire
        ORowSetRow aOldValues;
        if ( rowDeleted() )
        {
            positionCache( MOVE_FORWARD );
            m_pCache->next();
            setCurrentRow( true, false, aOldValues, aGuard);
        }
        else
            positionCache( MOVE_NONE );

        // check before because the resultset could be empty
        if  (   !m_bBeforeFirst
            &&  !m_bAfterLast
            &&  m_pCache->m_aMatrixIter != m_pCache->getEnd()
            &&  m_pCache->m_aMatrixIter->is()
            )
            aOldValues = new ORowSetValueVector( *(*(m_pCache->m_aMatrixIter)) );

        const bool bNewState = m_bNew;
        const bool bModState = m_bModified;

        m_pCache->moveToInsertRow();
        m_aCurrentRow = m_pCache->m_aInsertRow;
        m_bIsInsertRow  = true;

        // set read-only flag to false
        impl_setDataColumnsWriteable_throw();

        // notification order
        // - column values
        ORowSetBase::firePropertyChange(aOldValues);

        // - cursorMoved
        notifyAllListenersCursorMoved(aGuard);

        // - IsModified
        if ( bModState != m_bModified )
            fireProperty( PROPERTY_ID_ISMODIFIED, m_bModified, bModState );

        // - IsNew
        if ( bNewState != m_bNew )
            fireProperty( PROPERTY_ID_ISNEW, m_bNew, bNewState );

        // - RowCount/IsRowCountFinal
        fireRowcount();
    }
}

void ORowSet::impl_setDataColumnsWriteable_throw()
{
    impl_restoreDataColumnsWriteable_throw();
    m_aReadOnlyDataColumns.resize(m_aDataColumns.size(),false);
    std::vector<bool, std::allocator<bool> >::iterator aReadIter = m_aReadOnlyDataColumns.begin();
    for (auto const& dataColumn : m_aDataColumns)
    {
        bool bReadOnly = false;
        dataColumn->getPropertyValue(PROPERTY_ISREADONLY) >>= bReadOnly;
        *aReadIter = bReadOnly;

        dataColumn->setPropertyValue(PROPERTY_ISREADONLY,makeAny(false));
        ++aReadIter;
    }
}

void ORowSet::impl_restoreDataColumnsWriteable_throw()
{
    assert(m_aDataColumns.size() == m_aReadOnlyDataColumns.size() || m_aReadOnlyDataColumns.size() == 0 );
    TDataColumns::const_iterator aIter = m_aDataColumns.begin();
    for (auto const& readOnlyDataColumn : m_aReadOnlyDataColumns)
    {
        (*aIter)->setPropertyValue(PROPERTY_ISREADONLY, makeAny( static_cast<bool>(readOnlyDataColumn)) );
        ++aIter;
    }
    m_aReadOnlyDataColumns.clear();
}

void SAL_CALL ORowSet::moveToCurrentRow(  )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    if ( !m_pCache->m_bNew && !m_bModified )
        // nothing to do if we're not on the insertion row, and not modified otherwise
        return;

    if ( rowDeleted() )
        // this would perhaps even justify a RuntimeException ....
        // if the current row is deleted, then no write access to this row should be possible. So,
        // m_bModified should be true. Also, as soon as somebody calls moveToInsertRow,
        // our current row should not be deleted anymore. So, we should not have survived the above
        // check "if ( !m_pCache->m_bNew && !m_bModified )"
        ::dbtools::throwSQLException( DBA_RES( RID_STR_ROW_ALREADY_DELETED ), StandardSQLState::FUNCTION_SEQUENCE_ERROR, *this );

    if ( notifyAllListenersCursorBeforeMove( aGuard ) )
    {
        positionCache( MOVE_NONE_REFRESH );

        ORowSetNotifier aNotifier( this );

        // notification order
        // - cursorMoved
        notifyAllListenersCursorMoved(aGuard);

        // - IsModified
        // - IsNew
        aNotifier.fire();
    }
}

// XRow
sal_Bool SAL_CALL ORowSet::wasNull(  )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return ( m_pCache && isInsertRow() ) ? ((*m_pCache->m_aInsertRow)->get())[m_nLastColumnIndex].isNull() : ORowSetBase::wasNull();
}

const ORowSetValue& ORowSet::getInsertValue(sal_Int32 columnIndex)
{
    checkCache();

    if ( m_pCache && isInsertRow() )
        return  ((*m_pCache->m_aInsertRow)->get())[m_nLastColumnIndex = columnIndex];

    return getValue(columnIndex);
}

OUString SAL_CALL ORowSet::getString( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

sal_Bool SAL_CALL ORowSet::getBoolean( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    // the extra cast is to recognise the "true" or "false" strings
    return static_cast<bool>(getInsertValue(columnIndex));
}

sal_Int8 SAL_CALL ORowSet::getByte( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

sal_Int16 SAL_CALL ORowSet::getShort( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

sal_Int32 SAL_CALL ORowSet::getInt( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

sal_Int64 SAL_CALL ORowSet::getLong( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

float SAL_CALL ORowSet::getFloat( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

double SAL_CALL ORowSet::getDouble( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

Sequence< sal_Int8 > SAL_CALL ORowSet::getBytes( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

css::util::Date SAL_CALL ORowSet::getDate( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

css::util::Time SAL_CALL ORowSet::getTime( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

css::util::DateTime SAL_CALL ORowSet::getTimestamp( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}

Reference< css::io::XInputStream > SAL_CALL ORowSet::getBinaryStream( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    if ( m_pCache && isInsertRow() )
    {
        checkCache();
        return new ::comphelper::SequenceInputStream(((*m_pCache->m_aInsertRow)->get())[m_nLastColumnIndex = columnIndex].getSequence());
    }

    return ORowSetBase::getBinaryStream(columnIndex);
}

Reference< css::io::XInputStream > SAL_CALL ORowSet::getCharacterStream( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    if(m_pCache && isInsertRow() )
    {
        checkCache();
        return new ::comphelper::SequenceInputStream(((*m_pCache->m_aInsertRow)->get())[m_nLastColumnIndex = columnIndex].getSequence());
    }

    return ORowSetBase::getCharacterStream(columnIndex);
}

Any SAL_CALL ORowSet::getObject( sal_Int32 columnIndex, const Reference< XNameAccess >& /*typeMap*/ )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex).makeAny();
}

Reference< XRef > SAL_CALL ORowSet::getRef( sal_Int32 /*columnIndex*/ )
{
    return Reference< XRef >();
}

Reference< XBlob > SAL_CALL ORowSet::getBlob( sal_Int32 columnIndex )
{
    if ( m_pCache && isInsertRow() )
    {
        checkCache();
        return new ::connectivity::BlobHelper(((*m_pCache->m_aInsertRow)->get())[m_nLastColumnIndex = columnIndex].getSequence());
    }
    return ORowSetBase::getBlob(columnIndex);
}

Reference< XClob > SAL_CALL ORowSet::getClob( sal_Int32 columnIndex )
{
    return Reference< XClob >(getInsertValue(columnIndex).makeAny(),UNO_QUERY);
}

Reference< XArray > SAL_CALL ORowSet::getArray( sal_Int32 /*columnIndex*/ )
{
    return Reference< XArray >();
}

void SAL_CALL ORowSet::executeWithCompletion( const Reference< XInteractionHandler >& _rxHandler )
{
    if (!_rxHandler.is())
        execute();

    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    // tell everybody that we will change the result set
    approveExecution();

    ResettableMutexGuard aGuard( m_aMutex );

    try
    {
        freeResources( m_bCommandFacetsDirty );

        // calc the connection to be used
        if (m_xActiveConnection.is() && m_bRebuildConnOnExecute)
        {
            // there was a setProperty(ActiveConnection), but a setProperty(DataSource) _after_ that, too
            Reference< XConnection > xXConnection;
            setActiveConnection( xXConnection );
        }
        calcConnection( _rxHandler );
        m_bRebuildConnOnExecute = false;

        Reference< XSingleSelectQueryComposer > xComposer = getCurrentSettingsComposer( this, m_aContext );
        Reference<XParametersSupplier>  xParameters(xComposer, UNO_QUERY);

        Reference<XIndexAccess>  xParamsAsIndicies = xParameters.is() ? xParameters->getParameters() : Reference<XIndexAccess>();
        const sal_Int32 nParamCount = xParamsAsIndicies.is() ? xParamsAsIndicies->getCount() : 0;
        if ( m_aParametersSet.size() < static_cast<size_t>(nParamCount) )
            m_aParametersSet.resize( nParamCount ,false);

        ::dbtools::askForParameters( xComposer, this, m_xActiveConnection, _rxHandler,m_aParametersSet );
    }
    // ensure that only the allowed exceptions leave this block
    catch(SQLException&)
    {
        throw;
    }
    catch(RuntimeException&)
    {
        throw;
    }
    catch(Exception const &)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("dbaccess", "ORowSet::executeWithCompletion: caught an unexpected exception type while filling in the parameters! " << exceptionToString(ex));
    }

    // we're done with the parameters, now for the real execution

    //  do the real execute
    execute_NoApprove_NoNewConn(aGuard);
}

Reference< XIndexAccess > SAL_CALL ORowSet::getParameters(  )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    if ( m_bCommandFacetsDirty )
        // need to rebuild the parameters, since some property which contributes to the
        // complete command, and thus the parameters, changed
        impl_disposeParametersContainer_nothrow();

    if ( !m_pParameters.get() && !m_aCommand.isEmpty() )
    {
        try
        {
            OUString sNotInterestedIn;
            impl_initComposer_throw( sNotInterestedIn );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }

    // our caller could change our parameters at any time
    m_bParametersDirty = true;

    return m_pParameters.get();
}

void ORowSet::approveExecution()
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    EventObject aEvt(*this);

    OInterfaceIteratorHelper2 aApproveIter( m_aApproveListeners );
    while ( aApproveIter.hasMoreElements() )
    {
        Reference< XRowSetApproveListener > xListener( static_cast< XRowSetApproveListener* >( aApproveIter.next() ) );
        try
        {
            if ( xListener.is() && !xListener->approveRowSetChange( aEvt ) )
                throw RowSetVetoException();
        }
        catch ( const DisposedException& e )
        {
            if ( e.Context == xListener )
                aApproveIter.remove();
        }
        catch ( const RuntimeException& ) { throw; }
        catch ( const RowSetVetoException& ) { throw; }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

// XRowSet
void SAL_CALL ORowSet::execute(  )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    // tell everybody that we will change the result set
    approveExecution();

    ResettableMutexGuard aGuard( m_aMutex );
    freeResources( m_bCommandFacetsDirty );

    // calc the connection to be used
    if (m_xActiveConnection.is() && m_bRebuildConnOnExecute) {
        // there was a setProperty(ActiveConnection), but a setProperty(DataSource) _after_ that, too
        Reference< XConnection> xXConnection;
        setActiveConnection( xXConnection );
    }

    calcConnection(nullptr);
    m_bRebuildConnOnExecute = false;

    // do the real execute
    execute_NoApprove_NoNewConn(aGuard);
}

void ORowSet::setStatementResultSetType( const Reference< XPropertySet >& _rxStatement, sal_Int32 _nDesiredResultSetType, sal_Int32 _nDesiredResultSetConcurrency )
{
    OSL_ENSURE( _rxStatement.is(), "ORowSet::setStatementResultSetType: invalid statement - this will crash!" );

    sal_Int32 nResultSetType( _nDesiredResultSetType );
    sal_Int32 nResultSetConcurrency( _nDesiredResultSetConcurrency );

    // there *might* be a data source setting which tells use to be more defensive with those settings
    // #i15113#
    bool bRespectDriverRST = false;
    Any aSetting;
    if ( getDataSourceSetting( ::dbaccess::getDataSource( m_xActiveConnection ), "RespectDriverResultSetType", aSetting ) )
    {
        OSL_VERIFY( aSetting >>= bRespectDriverRST );
    }

    if ( bRespectDriverRST )
    {
        // try type/concurrency settings with decreasing usefulness, and rely on what the connection claims
        // to support
        Reference< XDatabaseMetaData > xMeta( m_xActiveConnection->getMetaData() );

        sal_Int32 nCharacteristics[5][2] =
        {   { ResultSetType::SCROLL_SENSITIVE, ResultSetConcurrency::UPDATABLE },
            { ResultSetType::SCROLL_INSENSITIVE, ResultSetConcurrency::UPDATABLE },
            { ResultSetType::SCROLL_SENSITIVE, ResultSetConcurrency::READ_ONLY },
            { ResultSetType::SCROLL_INSENSITIVE, ResultSetConcurrency::READ_ONLY },
            { ResultSetType::FORWARD_ONLY, ResultSetConcurrency::READ_ONLY }
        };
        sal_Int32 i=0;
        if ( m_xActiveConnection->getMetaData()->isReadOnly() )
            i = 2; // if the database is read-only we only should use read-only concurrency

        for ( ; i<5; ++i )
        {
            nResultSetType = nCharacteristics[i][0];
            nResultSetConcurrency = nCharacteristics[i][1];

            // don't try type/concurrency pairs which are more featured than what our caller requested
            if ( nResultSetType > _nDesiredResultSetType )
                continue;
            if ( nResultSetConcurrency > _nDesiredResultSetConcurrency )
                continue;

            if ( xMeta.is() && xMeta->supportsResultSetConcurrency( nResultSetType, nResultSetConcurrency ) )
                break;
        }
    }

    _rxStatement->setPropertyValue( PROPERTY_RESULTSETTYPE, makeAny( nResultSetType ) );
    _rxStatement->setPropertyValue( PROPERTY_RESULTSETCONCURRENCY, makeAny( nResultSetConcurrency ) );
}

void ORowSet::impl_ensureStatement_throw()
{
    OUString sCommandToExecute;
    if(m_bCommandFacetsDirty)
    {
        impl_initComposer_throw( sCommandToExecute );
    }
    else
    {
        sCommandToExecute = m_bUseEscapeProcessing ? m_xComposer->getQueryWithSubstitution() : m_aActiveCommand;
    }

    try
    {
        m_xStatement = m_xActiveConnection->prepareStatement( sCommandToExecute );
        if ( !m_xStatement.is() )
        {
            ::dbtools::throwSQLException( DBA_RES( RID_STR_INTERNAL_ERROR ), StandardSQLState::GENERAL_ERROR, *this );
        }

        Reference< XPropertySet > xStatementProps( m_xStatement, UNO_QUERY_THROW );
        // set the result set type and concurrency
        try
        {
            xStatementProps->setPropertyValue( PROPERTY_USEBOOKMARKS, makeAny( true ) );
            xStatementProps->setPropertyValue( PROPERTY_MAXROWS, makeAny( m_nMaxRows ) );

            setStatementResultSetType( xStatementProps, m_nResultSetType, m_nResultSetConcurrency );
        }
        catch ( const Exception& )
        {
            // this exception doesn't matter here because when we catch an exception
            // then the driver doesn't support this feature
        }
    }
    catch( const SQLException& )
    {
        SQLExceptionInfo aError( ::cppu::getCaughtException() );
        OSL_ENSURE( aError.isValid(), "ORowSet::impl_makeNewStatement_throw: caught an SQLException which we cannot analyze!" );

        // append information about what we were actually going to execute
        try
        {
            OUString sInfo(DBA_RES_PARAM( RID_STR_COMMAND_LEADING_TO_ERROR, "$command$", sCommandToExecute )  );
            aError.append( SQLExceptionInfo::TYPE::SQLContext, sInfo );
        }
        catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION("dbaccess"); }

        // propagate
        aError.doThrow();
    }
}

Reference< XResultSet > ORowSet::impl_prepareAndExecute_throw()
{
    impl_ensureStatement_throw();

    m_aParameterValueForCache->get().resize(1);
    Reference< XParameters > xParam( m_xStatement, UNO_QUERY_THROW );
    size_t nParamCount( m_pParameters.is() ? m_pParameters->size() : m_aPrematureParamValues->get().size() );
    for ( size_t i=1; i<=nParamCount; ++i )
    {
        ORowSetValue& rParamValue( getParameterStorage( static_cast<sal_Int32>(i) ) );
        ::dbtools::setObjectWithInfo( xParam, i, rParamValue.makeAny(), rParamValue.getTypeKind() );
        m_aParameterValueForCache->get().push_back(rParamValue);
    }
    m_bParametersDirty = false;

    Reference< XResultSet > xResultSet(m_xStatement->executeQuery());

    OUString aComposedUpdateTableName;
    if ( !m_aUpdateTableName.isEmpty() )
        aComposedUpdateTableName = composeTableName( m_xActiveConnection->getMetaData(), m_aUpdateCatalogName, m_aUpdateSchemaName, m_aUpdateTableName, false, ::dbtools::EComposeRule::InDataManipulation );

    SAL_INFO("dbaccess", "ORowSet::impl_prepareAndExecute_throw: creating cache" );
    m_pCache.reset(
        new ORowSetCache(xResultSet, m_xComposer.get(), m_aContext, aComposedUpdateTableName,
               m_bModified, m_bNew, *m_aParameterValueForCache, m_aFilter, m_nMaxRows));
    if ( m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY )
    {
        m_nPrivileges = Privilege::SELECT;
        m_pCache->m_nPrivileges = Privilege::SELECT;
    }
    m_pCache->setFetchSize(m_nFetchSize);
    m_aCurrentRow   = m_pCache->createIterator(this);
    m_bIsInsertRow  = false;
    m_aOldRow       = m_pCache->registerOldRow();

    return xResultSet;
}

void ORowSet::impl_initializeColumnSettings_nothrow( const Reference< XPropertySet >& _rxTemplateColumn, const Reference< XPropertySet >& _rxRowSetColumn )
{
    OSL_ENSURE( _rxTemplateColumn.is() && _rxRowSetColumn.is(),
        "ORowSet::impl_initializeColumnSettings_nothrow: this will crash!" );

    bool bHaveAnyColumnSetting = false;
    try
    {
        Reference< XPropertySetInfo > xInfo( _rxTemplateColumn->getPropertySetInfo(), UNO_QUERY_THROW );

        // a number of properties is plain copied
        const OUString aPropertyNames[] = {
            OUString(PROPERTY_ALIGN), OUString(PROPERTY_RELATIVEPOSITION), OUString(PROPERTY_WIDTH), OUString(PROPERTY_HIDDEN), OUString(PROPERTY_CONTROLMODEL),
            OUString(PROPERTY_HELPTEXT), OUString(PROPERTY_CONTROLDEFAULT)
        };
        for (const auto & aPropertyName : aPropertyNames)
        {
            if ( xInfo->hasPropertyByName( aPropertyName ) )
            {
                _rxRowSetColumn->setPropertyValue( aPropertyName, _rxTemplateColumn->getPropertyValue( aPropertyName ) );
                bHaveAnyColumnSetting = true;
            }
        }

        // the format key is slightly more complex
        sal_Int32 nFormatKey = 0;
        if( xInfo->hasPropertyByName( PROPERTY_NUMBERFORMAT ) )
        {
            _rxTemplateColumn->getPropertyValue( PROPERTY_NUMBERFORMAT ) >>= nFormatKey;
            bHaveAnyColumnSetting = true;
        }
        if ( !nFormatKey && m_xNumberFormatTypes.is() )
            nFormatKey = ::dbtools::getDefaultNumberFormat( _rxTemplateColumn, m_xNumberFormatTypes, SvtSysLocale().GetLanguageTag().getLocale() );
        _rxRowSetColumn->setPropertyValue( PROPERTY_NUMBERFORMAT, makeAny( nFormatKey ) );
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
        return;
    }

    if ( bHaveAnyColumnSetting )
        return;

    // the template column could not provide *any* setting. Okay, probably it's a parser column, which
    // does not offer those. However, perhaps the template column refers to a table column, which we
    // can use as new template column
    try
    {
        Reference< XPropertySetInfo > xInfo( _rxTemplateColumn->getPropertySetInfo(), UNO_QUERY_THROW );
        if ( !xInfo->hasPropertyByName( PROPERTY_TABLENAME ) )
            // no chance
            return;

        OUString sTableName;
        OSL_VERIFY( _rxTemplateColumn->getPropertyValue( PROPERTY_TABLENAME ) >>= sTableName );

        Reference< XNameAccess > xTables( impl_getTables_throw(), UNO_QUERY_THROW );
        if ( !xTables->hasByName( sTableName ) )
            // no chance
            return;

        Reference< XColumnsSupplier > xTableColSup( xTables->getByName( sTableName ), UNO_QUERY_THROW );
        Reference< XNameAccess > xTableCols( xTableColSup->getColumns(), UNO_QUERY_THROW );

        OUString sTableColumnName;

        // get the "Name" or (preferred) "RealName" property of the column
        OUString sNamePropertyName( PROPERTY_NAME );
        if ( xInfo->hasPropertyByName( PROPERTY_REALNAME ) )
            sNamePropertyName = PROPERTY_REALNAME;
        OSL_VERIFY( _rxTemplateColumn->getPropertyValue( sNamePropertyName ) >>= sTableColumnName );

        if ( !xTableCols->hasByName( sTableColumnName ) )
            return;

        Reference< XPropertySet > xTableColumn( xTableCols->getByName( sTableColumnName ), UNO_QUERY_THROW );
        impl_initializeColumnSettings_nothrow( xTableColumn, _rxRowSetColumn );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void ORowSet::execute_NoApprove_NoNewConn(ResettableMutexGuard& _rClearForNotification)
{
    // now we can dispose our old connection
    ::comphelper::disposeComponent(m_xOldConnection);
    m_xOldConnection = nullptr;

    // do we need a new statement
    if ( m_bCommandFacetsDirty )
    {
        m_xStatement    = nullptr;
        m_xComposer     = nullptr;

        Reference< XResultSet > xResultSet( impl_prepareAndExecute_throw() );

        // let our warnings container forget the reference to the (possibly disposed) old result set
        m_aWarnings.setExternalWarnings( nullptr );
        // clear all current warnings
        m_aWarnings.clearWarnings();
        // let the warnings container know about the new "external warnings"
        m_aWarnings.setExternalWarnings( Reference< XWarningsSupplier >( xResultSet, UNO_QUERY ) );

        // get the locale
        Locale aLocale = SvtSysLocale().GetLanguageTag().getLocale();

        // get the numberformatTypes
        OSL_ENSURE(m_xActiveConnection.is(),"No ActiveConnection");
        Reference< XNumberFormatsSupplier> xNumberFormat = ::dbtools::getNumberFormats(m_xActiveConnection);
        if ( xNumberFormat.is() )
            m_xNumberFormatTypes.set(xNumberFormat->getNumberFormats(),UNO_QUERY);

        ::rtl::Reference< ::connectivity::OSQLColumns> aColumns = new ::connectivity::OSQLColumns();
        std::vector< OUString> aNames;
        OUString aDescription;

        const std::map<sal_Int32,sal_Int32>& rKeyColumns = m_pCache->getKeyColumns();
        if(!m_xColumns.is())
        {
            SAL_INFO("dbaccess", "ORowSet::execute_NoApprove_NoNewConn::creating columns" );
            // use the meta data
            Reference<XResultSetMetaDataSupplier> xMetaSup(m_xStatement,UNO_QUERY);
            try
            {
                Reference<XResultSetMetaData> xMetaData = xMetaSup->getMetaData();
                if ( xMetaData.is() )
                {
                    sal_Int32 nCount = xMetaData->getColumnCount();
                    m_aDataColumns.reserve(nCount+1);
                    aColumns->get().reserve(nCount+1);
                    std::map< OUString, int > aColumnMap;
                    for (sal_Int32 i = 0 ; i < nCount; ++i)
                    {
                        // retrieve the name of the column
                        OUString sName = xMetaData->getColumnName(i + 1);
                        // check for duplicate entries
                        if(aColumnMap.find(sName) != aColumnMap.end())
                        {
                            OUString sAlias(sName);
                            sal_Int32 searchIndex=1;
                            while(aColumnMap.find(sAlias) != aColumnMap.end())
                            {
                                sAlias = sName + OUString::number(searchIndex++);
                            }
                            sName = sAlias;
                        }
                        ORowSetDataColumn* pColumn = new ORowSetDataColumn( getMetaData(),
                                                                            this,
                                                                            this,
                                                                            i+1,
                                                                            m_xActiveConnection->getMetaData(),
                                                                            aDescription,
                                                                            OUString(),
                            [this] (sal_Int32 const column) -> ORowSetValue const& {
                                return this->getInsertValue(column);
                            });
                        aColumnMap.insert(std::make_pair(sName,0));
                        aColumns->get().emplace_back(pColumn);
                        pColumn->setName(sName);
                        aNames.push_back(sName);
                        m_aDataColumns.push_back(pColumn);

                        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_ISREADONLY,makeAny(rKeyColumns.find(i+1) != rKeyColumns.end()));

                        try
                        {
                            sal_Int32 nFormatKey = 0;
                            if(m_xNumberFormatTypes.is())
                                nFormatKey = ::dbtools::getDefaultNumberFormat(pColumn,m_xNumberFormatTypes,aLocale);


                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_NUMBERFORMAT,makeAny(nFormatKey));
                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_RELATIVEPOSITION,makeAny(sal_Int32(i+1)));
                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_WIDTH,makeAny(sal_Int32(227)));
                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_ALIGN,makeAny(sal_Int32(0)));
                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_HIDDEN, css::uno::Any(false));
                        }
                        catch(Exception&)
                        {
                        }
                    }
                }
            }
            catch (SQLException&)
            {
            }
        }
        else
        {
            // create the rowset columns
            Reference< XResultSetMetaData > xMeta( getMetaData(), UNO_QUERY_THROW );
            sal_Int32 nCount = xMeta->getColumnCount();
            m_aDataColumns.reserve(nCount+1);
            aColumns->get().reserve(nCount+1);
            std::set< Reference< XPropertySet > > aAllColumns;

            for(sal_Int32 i=1; i <= nCount ;++i)
            {
                OUString sName = xMeta->getColumnName(i);
                OUString sColumnLabel = xMeta->getColumnLabel(i);

                // retrieve the column number |i|
                Reference<XPropertySet> xColumn;
                {
                    bool bReFetchName = false;
                    if (m_xColumns->hasByName(sColumnLabel))
                        m_xColumns->getByName(sColumnLabel) >>= xColumn;
                    if (!xColumn.is() && m_xColumns->hasByName(sName))
                        m_xColumns->getByName(sName) >>= xColumn;

                    // check if column already in the list we need another
                    if ( aAllColumns.find( xColumn ) != aAllColumns.end() )
                    {
                        xColumn = nullptr;
                        bReFetchName = true;
                        sColumnLabel.clear();
                    }
                    if(!xColumn.is())
                    {
                        // no column found so we could look at the position i
                        Reference<XIndexAccess> xIndexAccess(m_xColumns,UNO_QUERY);
                        if(xIndexAccess.is() && i <= xIndexAccess->getCount())
                        {
                            xIndexAccess->getByIndex(i-1) >>= xColumn;
                        }
                        else
                        {
                            Sequence< OUString> aSeq = m_xColumns->getElementNames();
                            if( i <= aSeq.getLength())
                            {
                                m_xColumns->getByName(aSeq.getConstArray()[i-1]) >>= xColumn;
                            }
                        }
                    }
                    if(bReFetchName && xColumn.is())
                        xColumn->getPropertyValue(PROPERTY_NAME) >>= sName;
                    aAllColumns.insert( xColumn );
                }

                // create a RowSetDataColumn
                {
                    Reference<XPropertySetInfo> xInfo = xColumn.is() ? xColumn->getPropertySetInfo() : Reference<XPropertySetInfo>();
                    if(xInfo.is() && xInfo->hasPropertyByName(PROPERTY_DESCRIPTION))
                        aDescription = comphelper::getString(xColumn->getPropertyValue(PROPERTY_DESCRIPTION));

                    OUString sParseLabel;
                    if ( xColumn.is() )
                    {
                        xColumn->getPropertyValue(PROPERTY_LABEL) >>= sParseLabel;
                    }
                    ORowSetDataColumn* pColumn = new ORowSetDataColumn( getMetaData(),
                                                                        this,
                                                                        this,
                                                                        i,
                                                                        m_xActiveConnection->getMetaData(),
                                                                        aDescription,
                                                                        sParseLabel,
                        [this] (sal_Int32 const column) -> ORowSetValue const& {
                            return this->getInsertValue(column);
                        });
                    aColumns->get().emplace_back(pColumn);

                    pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_ISREADONLY,makeAny(rKeyColumns.find(i) != rKeyColumns.end()));

                    if(sColumnLabel.isEmpty())
                    {
                        if(xColumn.is())
                            xColumn->getPropertyValue(PROPERTY_NAME) >>= sColumnLabel;
                        else
                            sColumnLabel = DBA_RES( RID_STR_EXPRESSION1 );
                    }
                    pColumn->setName(sColumnLabel);
                    aNames.push_back(sColumnLabel);
                    m_aDataColumns.push_back(pColumn);

                    if ( xColumn.is() )
                        impl_initializeColumnSettings_nothrow( xColumn, pColumn );
                }
            }
        }
        // now create the columns we need
        if(m_pColumns)
            m_pColumns->assign(aColumns,aNames);
        else
        {
            Reference<XDatabaseMetaData> xMeta = m_xActiveConnection->getMetaData();
            m_pColumns.reset( new ORowSetDataColumns(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers(),
                                                aColumns,*this,m_aColumnsMutex,aNames) );
        }
    }
    else // !m_bCommandFacetsDirty
    {
        Reference< XResultSet > xResultSet;
        if(m_bParametersDirty)
        {
            xResultSet = impl_prepareAndExecute_throw();
        }
        else
        {
            xResultSet = m_xStatement->executeQuery();
            m_pCache->reset(xResultSet);
        }
        // let our warnings container forget the reference to the (possibly disposed) old result set
        m_aWarnings.setExternalWarnings( nullptr );
        // clear all current warnings
        m_aWarnings.clearWarnings();
        // let the warnings container know about the new "external warnings"
        m_aWarnings.setExternalWarnings( Reference< XWarningsSupplier >( xResultSet, UNO_QUERY ) );
    }
    checkCache();
    // notify the rowset listeners
    notifyAllListeners(_rClearForNotification);
}

// XRowSetApproveBroadcaster
void SAL_CALL ORowSet::addRowSetApproveListener( const Reference< XRowSetApproveListener >& listener )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    m_aApproveListeners.addInterface(listener);
}

void SAL_CALL ORowSet::removeRowSetApproveListener( const Reference< XRowSetApproveListener >& listener )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    m_aApproveListeners.removeInterface(listener);
}

// XRowsChangeBroadcaster
void SAL_CALL ORowSet::addRowsChangeListener( const Reference< XRowsChangeListener >& listener )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    m_aRowsChangeListener.addInterface(listener);
}

void SAL_CALL ORowSet::removeRowsChangeListener( const Reference< XRowsChangeListener >& listener )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    m_aRowsChangeListener.removeInterface(listener);
}

// XResultSetAccess
Reference< XResultSet > SAL_CALL ORowSet::createResultSet(  )
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    if(m_xStatement.is())
    {
        ORowSetClone* pClone = new ORowSetClone( m_aContext, *this, m_pMutex );
        Reference< XResultSet > xRet(pClone);
        m_aClones.emplace_back(xRet);
        return xRet;
    }
    return Reference< XResultSet >();
}

// css::util::XCancellable
void SAL_CALL ORowSet::cancel(  )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
}

// css::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL ORowSet::deleteRows( const Sequence< Any >& rows )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    if(!m_pCache || m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY)
        throwFunctionSequenceException(*this);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    Sequence<Any> aChangedBookmarks;
    RowsChangeEvent aEvt(*this,RowChangeAction::DELETE,rows.getLength(),aChangedBookmarks);
    // notify the rowset listeners
    notifyAllListenersRowBeforeChange(aGuard,aEvt);

    Sequence< sal_Int32 > aResults( rows.getLength() );
    const Any* row = rows.getConstArray();
    const Any* rowEnd = rows.getConstArray() + rows.getLength();
    sal_Int32* result = aResults.getArray();
    for ( ; row != rowEnd; ++row, ++result )
    {
        *result = 0;
        if ( !m_pCache->moveToBookmark( *row ) )
            continue;
        sal_Int32 nDeletePosition = m_pCache->getRow();

        // first notify the clones so that they can save their position
        notifyRowSetAndClonesRowDelete( *row );

        // now delete the row
        if ( !m_pCache->deleteRow() )
            continue;
        *result = 1;
        // now notify that we have deleted
        notifyRowSetAndClonesRowDeleted( *row, nDeletePosition );
    }
    aEvt.Rows = aResults.getLength();

    // we have to check if we stand on the insert row and if so we have to reset it
    ORowSetNotifier aNotifier( this );
        // this will call cancelRowModification on the cache if necessary
    // notification order
    // - rowChanged
    notifyAllListenersRowChanged(aGuard,aEvt);

    // - IsModified
    // - IsNew
    aNotifier.fire();

    // - RowCount/IsRowCountFinal
    fireRowcount();

    return aResults;
}

void ORowSet::notifyRowSetAndClonesRowDelete( const Any& _rBookmark )
{
    // notify ourself
    onDeleteRow( _rBookmark );
    // notify the clones
    for (auto const& elem : m_aClones)
    {
        Reference< XUnoTunnel > xTunnel(elem.get(),UNO_QUERY);
        if(xTunnel.is())
        {
            ORowSetClone* pClone = reinterpret_cast<ORowSetClone*>(xTunnel->getSomething(ORowSetClone::getUnoTunnelImplementationId()));
            if(pClone)
                pClone->onDeleteRow( _rBookmark );
        }
    }
}

void ORowSet::notifyRowSetAndClonesRowDeleted( const Any& _rBookmark, sal_Int32 _nPos )
{
    // notify ourself
    onDeletedRow( _rBookmark, _nPos );
    // notify the clones
    for (auto const& clone : m_aClones)
    {
        Reference< XUnoTunnel > xTunnel(clone.get(),UNO_QUERY);
        if(xTunnel.is())
        {
            ORowSetClone* pClone = reinterpret_cast<ORowSetClone*>(xTunnel->getSomething(ORowSetClone::getUnoTunnelImplementationId()));
            if(pClone)
                pClone->onDeletedRow( _rBookmark, _nPos );
        }
    }
}

Reference< XConnection >  ORowSet::calcConnection(const Reference< XInteractionHandler >& _rxHandler)
{
    MutexGuard aGuard(m_aMutex);
    if (!m_xActiveConnection.is())
    {
        Reference< XConnection > xNewConn;
        if ( !m_aDataSourceName.isEmpty() )
        {
            Reference< XDatabaseContext > xDatabaseContext( DatabaseContext::create(m_aContext) );
            try
            {
                Reference< XDataSource > xDataSource( xDatabaseContext->getByName( m_aDataSourceName ), UNO_QUERY_THROW );

                // try connecting with the interaction handler
                Reference< XCompletedConnection > xComplConn( xDataSource, UNO_QUERY );
                if ( _rxHandler.is() && xComplConn.is() )
                {
                    xNewConn = xComplConn->connectWithCompletion( _rxHandler );
                }
                else
                {
                    xNewConn = xDataSource->getConnection( m_aUser, m_aPassword );
                }
            }
            catch ( const SQLException& )
            {
                throw;
            }
            catch ( const Exception& )
            {
                Any aError = ::cppu::getCaughtException();
                OUString sMessage = ResourceManager::loadString( RID_NO_SUCH_DATA_SOURCE,
                    "$name$", m_aDataSourceName, "$error$", extractExceptionMessage( m_aContext, aError ) );
                ::dbtools::throwGenericSQLException( sMessage, *this, aError );
            }
        }
        setActiveConnection(xNewConn);
        m_bOwnConnection = true;
    }
    return m_xActiveConnection;
}

Reference< XNameAccess > ORowSet::impl_getTables_throw()
{
    Reference< XNameAccess > xTables;

    Reference< XTablesSupplier >  xTablesAccess( m_xActiveConnection, UNO_QUERY );
    if ( xTablesAccess.is() )
    {
        xTables.set( xTablesAccess->getTables(), UNO_QUERY_THROW );
    }
    else if ( m_xTables )
    {
        xTables = m_xTables.get();
    }
    else
    {
        if ( !m_xActiveConnection.is() )
            throw SQLException(DBA_RES(RID_STR_CONNECTION_INVALID),*this,SQLSTATE_GENERAL,1000,Any() );

        bool bCase = true;
        try
        {
            Reference<XDatabaseMetaData> xMeta = m_xActiveConnection->getMetaData();
            bCase = xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers();
        }
        catch(SQLException&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        m_xTables = new OTableContainer(*this,m_aMutex,m_xActiveConnection,bCase,nullptr,nullptr,m_nInAppend);
        xTables = m_xTables.get();
        Sequence<OUString> aTableFilter { "%" };
        m_xTables->construct(aTableFilter,Sequence< OUString>());
    }

    return xTables;
}

void ORowSet::impl_resetTables_nothrow()
{
    if ( !m_xTables )
        return;

    try
    {
        m_xTables->dispose();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    m_xTables.clear();
}

void ORowSet::impl_initComposer_throw( OUString& _out_rCommandToExecute )
{
    bool bUseEscapeProcessing = impl_buildActiveCommand_throw( );
    _out_rCommandToExecute = m_aActiveCommand;
    if ( !bUseEscapeProcessing )
        return;

    if (m_bCommandFacetsDirty)
        m_xComposer = nullptr;

    Reference< XMultiServiceFactory > xFactory( m_xActiveConnection, UNO_QUERY );
    if ( !m_xComposer.is() && xFactory.is() )
    {
        try
        {
            m_xComposer.set( xFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );
        }
        catch (const Exception& ) { m_xComposer = nullptr; }
    }
    if ( !m_xComposer.is() )
        m_xComposer = new OSingleSelectQueryComposer( impl_getTables_throw(), m_xActiveConnection, m_aContext );

    m_xComposer->setCommand( m_aCommand,m_nCommandType );
    m_aActiveCommand = m_xComposer->getQuery();

    m_xComposer->setFilter( m_bApplyFilter ? m_aFilter : OUString() );
    m_xComposer->setHavingClause( m_bApplyFilter ? m_aHavingClause : OUString() );

    if ( m_bIgnoreResult )
    {   // append a "0=1" filter
        // don't simply overwrite an existent filter, this would lead to problems if this existent
        // filter contains parameters (since a keyset may add parameters itself)
        m_xComposer->setElementaryQuery( m_xComposer->getQuery( ) );
        m_xComposer->setFilter( "0 = 1" );
    }

    m_xComposer->setOrder( m_aOrder );
    m_xComposer->setGroup( m_aGroupBy );

    if ( !m_xColumns.is() )
    {
        Reference< XColumnsSupplier > xCols( m_xComposer, UNO_QUERY_THROW );
        m_xColumns = xCols->getColumns();
    }

    impl_initParametersContainer_nothrow();

    _out_rCommandToExecute = m_xComposer->getQueryWithSubstitution();

    m_bCommandFacetsDirty = false;
}

bool ORowSet::impl_buildActiveCommand_throw()
{
    // create the sql command
    // from a table name or get the command out of a query (not a view)
    // the last use the command as it is
    bool bDoEscapeProcessing = m_bUseEscapeProcessing;

    m_aActiveCommand.clear();
    OUString sCommand;

    if ( m_aCommand.isEmpty() )
        return bDoEscapeProcessing;

    switch (m_nCommandType)
    {
        case CommandType::TABLE:
        {
            impl_resetTables_nothrow();
            if ( bDoEscapeProcessing )
            {
                Reference< XNameAccess > xTables( impl_getTables_throw() );
                if ( xTables->hasByName(m_aCommand) )
                {
                }
                else
                {
                    OUString sMessage( DBA_RES( RID_STR_TABLE_DOES_NOT_EXIST ) );
                    throwGenericSQLException(sMessage.replaceAll( "$table$", m_aCommand ),*this);
                }
            }
            else
            {
                sCommand = "SELECT * FROM ";
                OUString sCatalog, sSchema, sTable;
                ::dbtools::qualifiedNameComponents( m_xActiveConnection->getMetaData(), m_aCommand, sCatalog, sSchema, sTable, ::dbtools::EComposeRule::InDataManipulation );
                sCommand += ::dbtools::composeTableNameForSelect( m_xActiveConnection, sCatalog, sSchema, sTable );
            }
        }
        break;

        case CommandType::QUERY:
        {
            Reference< XQueriesSupplier >  xQueriesAccess(m_xActiveConnection, UNO_QUERY);
            if (!xQueriesAccess.is())
                throw SQLException(DBA_RES(RID_STR_NO_XQUERIESSUPPLIER),*this,OUString(),0,Any());
            Reference< css::container::XNameAccess >  xQueries(xQueriesAccess->getQueries());
            if (xQueries->hasByName(m_aCommand))
            {
                Reference< XPropertySet > xQuery(xQueries->getByName(m_aCommand),UNO_QUERY);
                OSL_ENSURE(xQuery.is(),"ORowSet::impl_buildActiveCommand_throw: Query is NULL!");
                if ( xQuery.is() )
                {
                    xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
                    xQuery->getPropertyValue(PROPERTY_ESCAPE_PROCESSING) >>= bDoEscapeProcessing;
                    if ( bDoEscapeProcessing != m_bUseEscapeProcessing )
                    {
                        bool bOldValue = m_bUseEscapeProcessing;
                        m_bUseEscapeProcessing = bDoEscapeProcessing;
                        fireProperty(PROPERTY_ID_ESCAPE_PROCESSING,bOldValue,bDoEscapeProcessing);
                    }

                    OUString aCatalog,aSchema,aTable;
                    xQuery->getPropertyValue(PROPERTY_UPDATE_CATALOGNAME)   >>= aCatalog;
                    xQuery->getPropertyValue(PROPERTY_UPDATE_SCHEMANAME)    >>= aSchema;
                    xQuery->getPropertyValue(PROPERTY_UPDATE_TABLENAME)     >>= aTable;
                    if(!aTable.isEmpty())
                        m_aUpdateTableName = composeTableName( m_xActiveConnection->getMetaData(), aCatalog, aSchema, aTable, false, ::dbtools::EComposeRule::InDataManipulation );
                }
            }
            else
            {
                OUString sMessage( DBA_RES( RID_STR_QUERY_DOES_NOT_EXIST ) );
                throwGenericSQLException(sMessage.replaceAll( "$table$", m_aCommand ),*this);
            }
        }
        break;

        default:
            sCommand = m_aCommand;
            break;
    }

    m_aActiveCommand = sCommand;

    if ( m_aActiveCommand.isEmpty() && !bDoEscapeProcessing )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_SQL_COMMAND ), StandardSQLState::FUNCTION_SEQUENCE_ERROR, *this );

    return bDoEscapeProcessing;
}

void ORowSet::impl_initParametersContainer_nothrow()
{
    OSL_PRECOND( !m_pParameters.is(), "ORowSet::impl_initParametersContainer_nothrow: already initialized the parameters!" );

    m_pParameters = new param::ParameterWrapperContainer( m_xComposer.get() );
    // copy the premature parameters into the final ones
    size_t nParamCount( std::min( m_pParameters->size(), m_aPrematureParamValues->get().size() ) );
    for ( size_t i=0; i<nParamCount; ++i )
    {
        (*m_pParameters)[i] = m_aPrematureParamValues->get()[i];
    }
}

void ORowSet::impl_disposeParametersContainer_nothrow()
{
    if ( !m_pParameters.is() )
        return;

    // copy the actual values to our "premature" ones, to preserve them for later use
    size_t nParamCount( m_pParameters->size() );
    m_aPrematureParamValues->get().resize( nParamCount );
    for ( size_t i=0; i<nParamCount; ++i )
    {
        m_aPrematureParamValues->get()[i] = (*m_pParameters)[i];
    }

    m_pParameters->dispose();
    m_pParameters = nullptr;
}

ORowSetValue& ORowSet::getParameterStorage(sal_Int32 parameterIndex)
{
    ::connectivity::checkDisposed( ORowSet_BASE1::rBHelper.bDisposed );
    if ( parameterIndex < 1 )
        throwInvalidIndexException( *this );

    if ( m_aParametersSet.size() < static_cast<size_t>(parameterIndex) )
        m_aParametersSet.resize( parameterIndex ,false);
    m_aParametersSet[parameterIndex - 1] = true;

    if ( m_pParameters.is() )
    {
        if ( m_bCommandFacetsDirty )
        // need to rebuild the parameters, since some property which contributes to the
        // complete command, and thus the parameters, changed
            impl_disposeParametersContainer_nothrow();
        if ( m_pParameters.is() )
        {
            if ( static_cast<size_t>(parameterIndex) > m_pParameters->size() )
                throwInvalidIndexException( *this );
            return (*m_pParameters)[ parameterIndex - 1 ];
        }
    }

    if ( m_aPrematureParamValues->get().size() < static_cast<size_t>(parameterIndex) )
        m_aPrematureParamValues->get().resize( parameterIndex );
    return m_aPrematureParamValues->get()[ parameterIndex - 1 ];
}

// XParameters
void SAL_CALL ORowSet::setNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/ )
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    getParameterStorage( parameterIndex ).setNull();
    m_bParametersDirty = true;
}

void SAL_CALL ORowSet::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& /*typeName*/ )
{
    setNull( parameterIndex, sqlType );
}

void ORowSet::setParameter(sal_Int32 parameterIndex, const ORowSetValue& x)
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    getParameterStorage( parameterIndex ) = x;
    m_bParametersDirty = true;
}

void SAL_CALL ORowSet::setBoolean( sal_Int32 parameterIndex, sal_Bool x )
{
    setParameter(parameterIndex, static_cast<bool>(x));
}

void SAL_CALL ORowSet::setByte( sal_Int32 parameterIndex, sal_Int8 x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setShort( sal_Int32 parameterIndex, sal_Int16 x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setInt( sal_Int32 parameterIndex, sal_Int32 x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setLong( sal_Int32 parameterIndex, sal_Int64 x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setFloat( sal_Int32 parameterIndex, float x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setDouble( sal_Int32 parameterIndex, double x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setString( sal_Int32 parameterIndex, const OUString& x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setDate( sal_Int32 parameterIndex, const css::util::Date& x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setTime( sal_Int32 parameterIndex, const css::util::Time& x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x )
{
    setParameter(parameterIndex,x);
}

void SAL_CALL ORowSet::setBinaryStream( sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    ORowSetValue& rParamValue( getParameterStorage( parameterIndex ) );

    try
    {
        Sequence <sal_Int8> aData;
        x->readBytes(aData, length);
        rParamValue = aData;
        m_bParametersDirty = true;
        x->closeInput();
    }
    catch( Exception const & )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw SQLException("ORowSet::setBinaryStream", *this, "S1000", 0,anyEx);
    }
}

void SAL_CALL ORowSet::setCharacterStream( sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    ORowSetValue& rParamValue( getParameterStorage( parameterIndex ) );
    try
    {
        Sequence <sal_Int8> aData;
        OUString aDataStr;
        // the data is given as character data and the length defines the character length
        sal_Int32 nSize = x->readBytes(aData, length * sizeof(sal_Unicode));
        if (nSize / sizeof(sal_Unicode))
            aDataStr = OUString(reinterpret_cast<sal_Unicode const *>(aData.getConstArray()), nSize / sizeof(sal_Unicode));
        m_bParametersDirty = true;
        rParamValue = aDataStr;
        rParamValue.setTypeKind( DataType::LONGVARCHAR );
        x->closeInput();
    }
    catch( Exception const & )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw SQLException("ORowSet::setCharacterStream", *this, "S1000", 0, anyEx);
    }
}

void SAL_CALL ORowSet::setObject( sal_Int32 parameterIndex, const Any& x )
{
    if ( !::dbtools::implSetObject( this, parameterIndex, x ) )
    {   // there is no other setXXX call which can handle the value in x
        throw SQLException();
    }
    m_bParametersDirty = true;
}

void SAL_CALL ORowSet::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 /*scale*/ )
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    ORowSetValue& rParamValue( getParameterStorage( parameterIndex ) );
    setObject( parameterIndex, x );
    rParamValue.setTypeKind( targetSqlType );
}

void SAL_CALL ORowSet::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setRef", *this );
}

void SAL_CALL ORowSet::setBlob( sal_Int32 /*parameterIndex*/, const Reference< XBlob >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setBlob", *this );
}

void SAL_CALL ORowSet::setClob( sal_Int32 /*parameterIndex*/, const Reference< XClob >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setClob", *this );
}

void SAL_CALL ORowSet::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setArray", *this );
}

void SAL_CALL ORowSet::clearParameters(  )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    size_t nParamCount( m_pParameters.is() ? m_pParameters->size() : m_aPrematureParamValues->get().size() );
    for ( size_t i=1; i<=nParamCount; ++i )
        getParameterStorage( static_cast<sal_Int32>(i) ).setNull();
    m_aParametersSet.clear();
}

Any SAL_CALL ORowSet::getWarnings(  )
{
    return m_aWarnings.getWarnings();
}

void SAL_CALL ORowSet::clearWarnings(  )
{
    m_aWarnings.clearWarnings();
}

void ORowSet::doCancelModification( )
{
    if ( isModification() )
    {
        // read-only flag restored
        impl_restoreDataColumnsWriteable_throw();
        m_pCache->cancelRowModification();
    }
    m_bModified = false;
    m_bIsInsertRow = false;
}

bool ORowSet::isModification( )
{
    return isNew();
}

bool ORowSet::isModified( )
{
    return m_bModified;
}

bool ORowSet::isNew( )
{
    return m_bNew;
}

bool ORowSet::isPropertyChangeNotificationEnabled() const
{
    return m_bPropChangeNotifyEnabled;
}

void ORowSet::checkUpdateIterator()
{
    if(!m_bIsInsertRow)
    {
        m_pCache->setUpdateIterator(m_aCurrentRow);
        m_aCurrentRow = m_pCache->m_aInsertRow;
        m_bIsInsertRow = true;
    }
}

void ORowSet::checkUpdateConditions(sal_Int32 columnIndex)
{
    checkCache();
    if ( m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY)
        ::dbtools::throwSQLException( DBA_RES( RID_STR_RESULT_IS_READONLY ), StandardSQLState::GENERAL_ERROR, *this );

    if ( rowDeleted() )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_ROW_ALREADY_DELETED ), StandardSQLState::INVALID_CURSOR_POSITION, *this );

    if ( m_aCurrentRow.isNull() )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_INVALID_CURSOR_STATE ), StandardSQLState::INVALID_CURSOR_STATE, *this );

    if ( columnIndex <= 0 || sal_Int32((*m_aCurrentRow)->get().size()) <= columnIndex )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_INVALID_INDEX ), StandardSQLState::INVALID_DESCRIPTOR_INDEX, *this );
}

void SAL_CALL ORowSet::refreshRow(  )
{

    ORowSetNotifier aNotifier( this );
        // this will call cancelRowModification on the cache if necessary

    // notification order:
    if ( m_bModified && m_pCache )
        implCancelRowUpdates( false ); // do _not_ notify the IsModify - will do this ourself below

    // - column values
    ORowSetBase::refreshRow();

    // - IsModified
    // - IsNew
    aNotifier.fire( );
}

void ORowSet::impl_rebuild_throw(::osl::ResettableMutexGuard& _rGuard)
{
    Reference< XResultSet > xResultSet(m_xStatement->executeQuery());
    m_pCache->reset(xResultSet);
    m_aWarnings.setExternalWarnings( Reference< XWarningsSupplier >( xResultSet, UNO_QUERY ) );
    notifyAllListeners(_rGuard);
}

// ***********************************************************
//  ORowSetClone
// ***********************************************************

ORowSetClone::ORowSetClone( const Reference<XComponentContext>& _rContext, ORowSet& rParent, ::osl::Mutex* _pMutex )
             :OSubComponent(m_aMutex, rParent)
             ,ORowSetBase( _rContext, OComponentHelper::rBHelper, _pMutex )
             ,m_pParent(&rParent)
             ,m_nFetchDirection(rParent.m_nFetchDirection)
             ,m_nFetchSize(rParent.m_nFetchSize)
             ,m_bIsBookmarkable(true)
{

    m_nResultSetType        = rParent.m_nResultSetType;
    m_nResultSetConcurrency = ResultSetConcurrency::READ_ONLY;
    m_pMySelf               = this;
    m_bClone                = true;
    m_bBeforeFirst          = rParent.m_bBeforeFirst;
    m_bAfterLast            = rParent.m_bAfterLast;
    m_pCache                = rParent.m_pCache;
    m_aBookmark             = rParent.m_aBookmark;
    m_aCurrentRow           = m_pCache->createIterator(this);
    m_xNumberFormatTypes    = rParent.m_xNumberFormatTypes;

    m_aOldRow = m_pCache->registerOldRow();

    ::rtl::Reference< ::connectivity::OSQLColumns> aColumns = new ::connectivity::OSQLColumns();
    std::vector< OUString> aNames;

    OUString aDescription;
    Locale aLocale = SvtSysLocale().GetLanguageTag().getLocale();

    if ( rParent.m_pColumns )
    {
        Sequence< OUString> aSeq = rParent.m_pColumns->getElementNames();
        const OUString* pIter    = aSeq.getConstArray();
        const OUString* pEnd     = pIter + aSeq.getLength();
        aColumns->get().reserve(aSeq.getLength()+1);
        for(sal_Int32 i=1;pIter != pEnd ;++pIter,++i)
        {
            Reference<XPropertySet> xColumn;
            rParent.m_pColumns->getByName(*pIter) >>= xColumn;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION))
                aDescription = comphelper::getString(xColumn->getPropertyValue(PROPERTY_DESCRIPTION));

            OUString sParseLabel;
            xColumn->getPropertyValue(PROPERTY_LABEL) >>= sParseLabel;
            ORowSetColumn* pColumn = new ORowSetColumn( rParent.getMetaData(),
                                                                this,
                                                                i,
                                                                rParent.m_xActiveConnection->getMetaData(),
                                                                aDescription,
                                                                sParseLabel,
                [this] (sal_Int32 const column) -> ORowSetValue const& {
                    return this->getValue(column);
                });
            aColumns->get().emplace_back(pColumn);
            pColumn->setName(*pIter);
            aNames.push_back(*pIter);
            m_aDataColumns.push_back(pColumn);

            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_ALIGN,xColumn->getPropertyValue(PROPERTY_ALIGN));
            sal_Int32 nFormatKey = 0;
            xColumn->getPropertyValue(PROPERTY_NUMBERFORMAT) >>= nFormatKey;
            if(!nFormatKey && xColumn.is() && m_xNumberFormatTypes.is())
                nFormatKey = ::dbtools::getDefaultNumberFormat(xColumn,m_xNumberFormatTypes,aLocale);
            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_NUMBERFORMAT,makeAny(nFormatKey));
            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_RELATIVEPOSITION,xColumn->getPropertyValue(PROPERTY_RELATIVEPOSITION));
            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_WIDTH,xColumn->getPropertyValue(PROPERTY_WIDTH));
            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_HIDDEN,xColumn->getPropertyValue(PROPERTY_HIDDEN));
            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_CONTROLMODEL,xColumn->getPropertyValue(PROPERTY_CONTROLMODEL));
            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_HELPTEXT,xColumn->getPropertyValue(PROPERTY_HELPTEXT));
            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_CONTROLDEFAULT,xColumn->getPropertyValue(PROPERTY_CONTROLDEFAULT));

        }
    }
    Reference<XDatabaseMetaData> xMeta = rParent.m_xActiveConnection->getMetaData();
    m_pColumns.reset( new ORowSetDataColumns(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers(),
                                        aColumns,*this,m_aMutex,aNames) );

    sal_Int32 const nRT = PropertyAttribute::READONLY   | PropertyAttribute::TRANSIENT;

    // sdb.RowSet Properties
    registerMayBeVoidProperty(PROPERTY_ACTIVE_CONNECTION,PROPERTY_ID_ACTIVE_CONNECTION, PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   &rParent.m_aActiveConnection,   cppu::UnoType<XConnection>::get());
    registerProperty(PROPERTY_RESULTSETCONCURRENCY, PROPERTY_ID_RESULTSETCONCURRENCY,   PropertyAttribute::READONLY,    &m_nResultSetConcurrency,::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_RESULTSETTYPE,        PROPERTY_ID_RESULTSETTYPE,          PropertyAttribute::READONLY,    &m_nResultSetType,      ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_FETCHDIRECTION,       PROPERTY_ID_FETCHDIRECTION,         PropertyAttribute::TRANSIENT,   &m_nFetchDirection,     ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_FETCHSIZE,            PROPERTY_ID_FETCHSIZE,              PropertyAttribute::TRANSIENT,   &m_nFetchSize,          ::cppu::UnoType<sal_Int32>::get());
    registerProperty(PROPERTY_ISBOOKMARKABLE,       PROPERTY_ID_ISBOOKMARKABLE,         nRT,                            &m_bIsBookmarkable,      cppu::UnoType<bool>::get());
}

ORowSetClone::~ORowSetClone()
{
}

// css::XTypeProvider
Sequence< Type > ORowSetClone::getTypes()
{
    return ::comphelper::concatSequences(OSubComponent::getTypes(),ORowSetBase::getTypes());
}

// css::XInterface
Any ORowSetClone::queryInterface( const Type & rType )
{
    Any aRet = ORowSetBase::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OSubComponent::queryInterface(rType);
    return aRet;
}

void ORowSetClone::acquire() throw()
{
    OSubComponent::acquire();
}

void ORowSetClone::release() throw()
{
    OSubComponent::release();
}

// XServiceInfo
OUString ORowSetClone::getImplementationName(  )
{
    return OUString("com.sun.star.sdb.ORowSetClone");
}

sal_Bool ORowSetClone::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > ORowSetClone::getSupportedServiceNames(  )
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBC_RESULTSET;
    aSNS[1] = SERVICE_SDB_RESULTSET;
    return aSNS;
}

// OComponentHelper
void ORowSetClone::disposing()
{
    MutexGuard aGuard( m_aMutex );
    ORowSetBase::disposing();

    m_pParent   = nullptr;
    m_pMutex    = &m_aMutex; // this must be done here because someone could hold a ref to us and try to do something
    OSubComponent::disposing();
}

// XCloseable
void ORowSetClone::close()
{
    {
        MutexGuard aGuard( m_aMutex );
        if (OComponentHelper::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ORowSetClone::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& SAL_CALL ORowSetClone::getInfoHelper()
{
    return *::comphelper::OPropertyArrayUsageHelper<ORowSetClone>::getArrayHelper();
}

Sequence< sal_Int8 > ORowSetClone::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId implId;

    return implId.getImplementationId();
}

// css::XUnoTunnel
sal_Int64 SAL_CALL ORowSetClone::getSomething( const Sequence< sal_Int8 >& rId )
{
    if (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}

void SAL_CALL ORowSetClone::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
{
    if ( nHandle == PROPERTY_ID_FETCHSIZE )
    {
        if ( m_pParent )
            m_pParent->setFastPropertyValue_NoBroadcast( nHandle, rValue );
    }

    OPropertyStateContainer::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}

void ORowSetClone::doCancelModification( )
{
}

bool ORowSetClone::isModification( )
{
    return false;
}

bool ORowSetClone::isModified( )
{
    return false;
}

bool ORowSetClone::isNew( )
{
    return false;
}

void SAL_CALL ORowSetClone::execute(  )
{
    throwFunctionNotSupportedSQLException( "RowSetClone::XRowSet::execute", *this );
}

void SAL_CALL ORowSetClone::addRowSetListener( const Reference< XRowSetListener >& )
{
    throwFunctionNotSupportedRuntimeException( "RowSetClone::XRowSet", *this );
}

void SAL_CALL ORowSetClone::removeRowSetListener( const Reference< XRowSetListener >& )
{
    throwFunctionNotSupportedRuntimeException( "RowSetClone::XRowSet", *this );
}

} // dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
