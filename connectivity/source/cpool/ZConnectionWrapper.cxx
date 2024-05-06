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

#include "ZConnectionWrapper.hxx"

using namespace connectivity;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

OConnectionWeakWrapper::OConnectionWeakWrapper(Reference< XAggregation >& _xConnection)
    : OConnectionWeakWrapper_BASE(m_aMutex)
{
    setDelegation(_xConnection,m_refCount);
    OSL_ENSURE(m_xConnection.is(),"OConnectionWeakWrapper: Connection must be valid!");
}

OConnectionWeakWrapper::~OConnectionWeakWrapper()
{
    if ( !OConnectionWeakWrapper_BASE::rBHelper.bDisposed )
    {
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}
// XServiceInfo

IMPLEMENT_SERVICE_INFO(OConnectionWeakWrapper, u"com.sun.star.sdbc.drivers.OConnectionWeakWrapper"_ustr, u"com.sun.star.sdbc.Connection"_ustr)


Reference< XStatement > SAL_CALL OConnectionWeakWrapper::createStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->createStatement();
}

Reference< XPreparedStatement > SAL_CALL OConnectionWeakWrapper::prepareStatement( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->prepareStatement(sql);
}

Reference< XPreparedStatement > SAL_CALL OConnectionWeakWrapper::prepareCall( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->prepareCall(sql);
}

OUString SAL_CALL OConnectionWeakWrapper::nativeSQL( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->nativeSQL(sql);
}

void SAL_CALL OConnectionWeakWrapper::setAutoCommit( sal_Bool autoCommit )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);

    m_xConnection->setAutoCommit(autoCommit);
}

sal_Bool SAL_CALL OConnectionWeakWrapper::getAutoCommit(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getAutoCommit();
}

void SAL_CALL OConnectionWeakWrapper::commit(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->commit();
}

void SAL_CALL OConnectionWeakWrapper::rollback(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->rollback();
}

sal_Bool SAL_CALL OConnectionWeakWrapper::isClosed(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_xConnection->isClosed();
}

Reference< XDatabaseMetaData > SAL_CALL OConnectionWeakWrapper::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getMetaData();
}

void SAL_CALL OConnectionWeakWrapper::setReadOnly( sal_Bool readOnly )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->setReadOnly(readOnly);
}

sal_Bool SAL_CALL OConnectionWeakWrapper::isReadOnly(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->isReadOnly();
}

void SAL_CALL OConnectionWeakWrapper::setCatalog( const OUString& catalog )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->setCatalog(catalog);
}

OUString SAL_CALL OConnectionWeakWrapper::getCatalog(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getCatalog();
}

void SAL_CALL OConnectionWeakWrapper::setTransactionIsolation( sal_Int32 level )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->setTransactionIsolation(level);
}

sal_Int32 SAL_CALL OConnectionWeakWrapper::getTransactionIsolation(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getTransactionIsolation();
}

Reference< css::container::XNameAccess > SAL_CALL OConnectionWeakWrapper::getTypeMap(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    return m_xConnection->getTypeMap();
}

void SAL_CALL OConnectionWeakWrapper::setTypeMap( const Reference< css::container::XNameAccess >& typeMap )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);


    m_xConnection->setTypeMap(typeMap);
}

// XCloseable
void SAL_CALL OConnectionWeakWrapper::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnectionWeakWrapper_BASE::rBHelper.bDisposed);

    }
    dispose();
}

void OConnectionWeakWrapper::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OConnectionWeakWrapper_BASE::disposing();
    OConnectionWrapper::disposing();
}

// css::lang::XUnoTunnel
IMPLEMENT_FORWARD_REFCOUNT( OConnectionWeakWrapper, OConnectionWeakWrapper_BASE )

css::uno::Any SAL_CALL OConnectionWeakWrapper::queryInterface( const css::uno::Type& _rType )
{
    css::uno::Any aReturn = OConnectionWeakWrapper_BASE::queryInterface( _rType );
    if ( !aReturn.hasValue() )
        aReturn = OConnectionWrapper::queryInterface( _rType );
    return aReturn;
}

IMPLEMENT_FORWARD_XTYPEPROVIDER2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
