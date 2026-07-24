/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <querycomposer.hxx>
#include <strings.hxx>
#include <composertools.hxx>
#include <algorithm>

using namespace dbaccess;
using namespace comphelper;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace cpo::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;


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

void OQueryComposer::disposing()
{
    ::comphelper::disposeComponent(m_xComposerHelper);
    ::comphelper::disposeComponent(m_xComposer);
}

// css::lang::XTypeProvider
Sequence< Type > OQueryComposer::getTypes()
{
    return ::comphelper::concatSequences(OSubComponent::getTypes(),OQueryComposer_BASE::getTypes());
}

Sequence< sal_Int8 > OQueryComposer::getImplementationId()
{
    return cpo::uno::Sequence<sal_Int8>();
}

Any OQueryComposer::queryInterface( const Type & rType )
{
    Any aRet = OSubComponent::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OQueryComposer_BASE::queryInterface(rType);
    return aRet;
}

// XServiceInfo
OUString OQueryComposer::getImplementationName(  )
{
    return u"com.sun.star.sdb.dbaccess.OQueryComposer"_ustr;
}

bool OQueryComposer::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > OQueryComposer::getSupportedServiceNames(  )
{
    return { SERVICE_SDB_SQLQUERYCOMPOSER };
}

// XSQLQueryComposer
OUString OQueryComposer::getQuery(  )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    Reference<XPropertySet> xProp(m_xComposer,UNO_QUERY);
    OUString sQuery;
    if ( xProp.is() )
        xProp->getPropertyValue(PROPERTY_ORIGINAL) >>= sQuery;
    return sQuery;
}

void OQueryComposer::setQuery( const OUString& command )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    m_aFilters.clear();
    m_xComposer->setQuery(command);
    m_sOrgFilter = m_xComposer->getFilter();
    m_sOrgOrder = m_xComposer->getOrder();
}

OUString OQueryComposer::getComposedQuery(  )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);

    return m_xComposer->getQuery();
}

OUString OQueryComposer::getFilter(  )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    FilterCreator aFilterCreator;
    aFilterCreator = std::for_each(m_aFilters.begin(),m_aFilters.end(),aFilterCreator);
    return aFilterCreator.getComposedAndClear();
}

Sequence< Sequence< PropertyValue > > OQueryComposer::getStructuredFilter(  )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);
    return m_xComposer->getStructuredFilter();
}

OUString OQueryComposer::getOrder(  )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    OrderCreator aOrderCreator;
    aOrderCreator = std::for_each(m_aOrders.begin(),m_aOrders.end(),aOrderCreator);
    return aOrderCreator.getComposedAndClear();
}

void OQueryComposer::appendFilterByColumn( const Reference< XPropertySet >& column )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xComposerHelper->setQuery(getQuery());
    m_xComposerHelper->setFilter(OUString());
    m_xComposerHelper->appendFilterByColumn(column, true, SQLFilterOperator::EQUAL);

    FilterCreator aFilterCreator;
    aFilterCreator.append(getFilter());
    aFilterCreator.append(m_xComposerHelper->getFilter());

    setFilter( aFilterCreator.getComposedAndClear() );
}

void OQueryComposer::appendOrderByColumn( const Reference< XPropertySet >& column, bool ascending )
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

void OQueryComposer::setFilter( const OUString& filter )
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

void OQueryComposer::setOrder( const OUString& order )
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
Reference< XNameAccess > OQueryComposer::getTables(  )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XTablesSupplier>(m_xComposer,UNO_QUERY_THROW)->getTables();
}

// XColumnsSupplier
Reference< XNameAccess > OQueryComposer::getColumns(  )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XColumnsSupplier>(m_xComposer,UNO_QUERY_THROW)->getColumns();
}

Reference< XIndexAccess > OQueryComposer::getParameters(  )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XParametersSupplier>(m_xComposer,UNO_QUERY_THROW)->getParameters();
}

void OQueryComposer::acquire() noexcept
{
    OSubComponent::acquire();
}

void OQueryComposer::release() noexcept
{
    OSubComponent::release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
