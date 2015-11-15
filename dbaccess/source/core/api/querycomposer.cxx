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

#include <string.h>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <comphelper/sequence.hxx>
#include <com/sun/star/uno/XAggregation.hpp>
#include <comphelper/processfactory.hxx>
#include "dbastrings.hrc"
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/types.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <unotools/syslocale.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include "querycomposer.hxx"
#include "HelperCollections.hxx"
#include "composertools.hxx"
#include <algorithm>

using namespace dbaccess;
using namespace dbtools;
using namespace comphelper;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::cppu;
using namespace ::osl;
using namespace ::utl;



OQueryComposer::OQueryComposer(const Reference< XConnection>& _xConnection)
 : OSubComponent(m_aMutex,_xConnection)
{
    OSL_ENSURE(_xConnection.is()," Connection can't be null!");

    Reference<XMultiServiceFactory> xFac( _xConnection, UNO_QUERY_THROW );
    m_xComposer.set( xFac->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );
    m_xComposerHelper.set( xFac->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );
}

OQueryComposer::~OQueryComposer()
{
}

void SAL_CALL OQueryComposer::disposing()
{
    ::comphelper::disposeComponent(m_xComposerHelper);
    ::comphelper::disposeComponent(m_xComposer);
}

// css::lang::XTypeProvider
Sequence< Type > SAL_CALL OQueryComposer::getTypes() throw (RuntimeException, std::exception)
{
    return ::comphelper::concatSequences(OSubComponent::getTypes(),OQueryComposer_BASE::getTypes());
}

Sequence< sal_Int8 > SAL_CALL OQueryComposer::getImplementationId() throw (RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

Any SAL_CALL OQueryComposer::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    Any aRet = OSubComponent::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OQueryComposer_BASE::queryInterface(rType);
    return aRet;
}

// XServiceInfo
OUString OQueryComposer::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return OUString("com.sun.star.sdb.dbaccess.OQueryComposer");
}

sal_Bool OQueryComposer::supportsService( const OUString& _rServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > OQueryComposer::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    Sequence<OUString> aSNS { SERVICE_SDB_SQLQUERYCOMPOSER };
    return aSNS;
}

// XSQLQueryComposer
OUString SAL_CALL OQueryComposer::getQuery(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    Reference<XPropertySet> xProp(m_xComposer,UNO_QUERY);
    OUString sQuery;
    if ( xProp.is() )
        xProp->getPropertyValue(PROPERTY_ORIGINAL) >>= sQuery;
    return sQuery;
}

void SAL_CALL OQueryComposer::setQuery( const OUString& command ) throw(SQLException, RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    m_aFilters.clear();
    m_xComposer->setQuery(command);
    m_sOrgFilter = m_xComposer->getFilter();
    m_sOrgOrder = m_xComposer->getOrder();
}

OUString SAL_CALL OQueryComposer::getComposedQuery(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);

    return m_xComposer->getQuery();
}

OUString SAL_CALL OQueryComposer::getFilter(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    FilterCreator aFilterCreator;
    aFilterCreator = ::std::for_each(m_aFilters.begin(),m_aFilters.end(),aFilterCreator);
    return aFilterCreator.getComposedAndClear();
}

Sequence< Sequence< PropertyValue > > SAL_CALL OQueryComposer::getStructuredFilter(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);
    return m_xComposer->getStructuredFilter();
}

OUString SAL_CALL OQueryComposer::getOrder(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    OrderCreator aOrderCreator;
    aOrderCreator = ::std::for_each(m_aOrders.begin(),m_aOrders.end(),aOrderCreator);
    return aOrderCreator.getComposedAndClear();
}

void SAL_CALL OQueryComposer::appendFilterByColumn( const Reference< XPropertySet >& column ) throw(SQLException, RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xComposerHelper->setQuery(getQuery());
    m_xComposerHelper->setFilter(OUString());
    m_xComposerHelper->appendFilterByColumn(column, sal_True, SQLFilterOperator::EQUAL);

    FilterCreator aFilterCreator;
    aFilterCreator.append(getFilter());
    aFilterCreator.append(m_xComposerHelper->getFilter());

    setFilter( aFilterCreator.getComposedAndClear() );
}

void SAL_CALL OQueryComposer::appendOrderByColumn( const Reference< XPropertySet >& column, sal_Bool ascending ) throw(SQLException, RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xComposerHelper->setQuery(getQuery());
    m_xComposerHelper->setOrder(OUString());
    m_xComposerHelper->appendOrderByColumn(column,ascending);

    OrderCreator aOrderCreator;
    aOrderCreator.append(getOrder());
    aOrderCreator.append(m_xComposerHelper->getOrder());

    setOrder(aOrderCreator.getComposedAndClear());
}

void SAL_CALL OQueryComposer::setFilter( const OUString& filter ) throw(SQLException, RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    FilterCreator aFilterCreator;
    aFilterCreator.append(m_sOrgFilter);
    aFilterCreator.append(filter);

    m_aFilters.clear();
    if ( !filter.isEmpty() )
        m_aFilters.push_back(filter);

    m_xComposer->setFilter( aFilterCreator.getComposedAndClear() );
}

void SAL_CALL OQueryComposer::setOrder( const OUString& order ) throw(SQLException, RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );

    OrderCreator aOrderCreator;
    aOrderCreator.append(m_sOrgOrder);
    aOrderCreator.append(order);

    m_aOrders.clear();
    if ( !order.isEmpty() )
        m_aOrders.push_back(order);

    m_xComposer->setOrder(aOrderCreator.getComposedAndClear());
}

// XTablesSupplier
Reference< XNameAccess > SAL_CALL OQueryComposer::getTables(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XTablesSupplier>(m_xComposer,UNO_QUERY)->getTables();
}

// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OQueryComposer::getColumns(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XColumnsSupplier>(m_xComposer,UNO_QUERY)->getColumns();
}

Reference< XIndexAccess > SAL_CALL OQueryComposer::getParameters(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XParametersSupplier>(m_xComposer,UNO_QUERY)->getParameters();
}

void SAL_CALL OQueryComposer::acquire() throw()
{
    OSubComponent::acquire();
}

void SAL_CALL OQueryComposer::release() throw()
{
    OSubComponent::release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
