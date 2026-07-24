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

#include <callablestatement.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <strings.hxx>

using namespace dbaccess;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace cpo::uno;
using namespace ::cppu;
using namespace ::osl;

// css::lang::XTypeProvider
Sequence< Type > OCallableStatement::getTypes()
{
    OTypeCollection aTypes(cppu::UnoType<XRow>::get(),
                           cppu::UnoType<XOutParameters>::get(),
                            OPreparedStatement::getTypes() );

    return aTypes.getTypes();
}

Sequence< sal_Int8 > OCallableStatement::getImplementationId()
{
    return cpo::uno::Sequence<sal_Int8>();
}

// css::uno::XInterface
Any OCallableStatement::queryInterface( const Type & rType )
{
    Any aIface = OPreparedStatement::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XRow * >( this ),
                    static_cast< XOutParameters * >( this ));
    return aIface;
}

void OCallableStatement::acquire() noexcept
{
    OPreparedStatement::acquire();
}

void OCallableStatement::release() noexcept
{
    OPreparedStatement::release();
}

// XServiceInfo
OUString OCallableStatement::getImplementationName(  )
{
    return u"com.sun.star.sdb.OCallableStatement"_ustr;
}

Sequence< OUString > OCallableStatement::getSupportedServiceNames(  )
{
    return { SERVICE_SDBC_CALLABLESTATEMENT, SERVICE_SDB_CALLABLESTATEMENT };
}

// XOutParameters
void OCallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName )
{
    MutexGuard aGuard(m_aMutex);

    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY_THROW)->registerOutParameter( parameterIndex, sqlType, typeName );
}

void OCallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY_THROW)->registerNumericOutParameter( parameterIndex, sqlType, scale );
}

// XRow
bool OCallableStatement::wasNull(  )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->wasNull();
}

OUString OCallableStatement::getString( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getString( columnIndex );
}

bool OCallableStatement::getBoolean( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getBoolean( columnIndex );
}

sal_Int8 OCallableStatement::getByte( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getByte( columnIndex );
}

sal_Int16 OCallableStatement::getShort( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getShort( columnIndex );
}

sal_Int32 OCallableStatement::getInt( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getInt( columnIndex );
}

sal_Int64 OCallableStatement::getLong( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getLong( columnIndex );
}

float OCallableStatement::getFloat( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getFloat( columnIndex );
}

double OCallableStatement::getDouble( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getDouble( columnIndex );
}

Sequence< sal_Int8 > OCallableStatement::getBytes( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getBytes( columnIndex );
}

css::util::Date OCallableStatement::getDate( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getDate( columnIndex );
}

css::util::Time OCallableStatement::getTime( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getTime( columnIndex );
}

css::util::DateTime OCallableStatement::getTimestamp( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getTimestamp( columnIndex );
}

Reference< css::io::XInputStream > OCallableStatement::getBinaryStream( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getBinaryStream( columnIndex );
}

Reference< css::io::XInputStream > OCallableStatement::getCharacterStream( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getCharacterStream( columnIndex );
}

Any OCallableStatement::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& typeMap )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getObject( columnIndex, typeMap );
}

Reference< XRef > OCallableStatement::getRef( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getRef( columnIndex );
}

Reference< XBlob > OCallableStatement::getBlob( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getBlob( columnIndex );
}

Reference< XClob > OCallableStatement::getClob( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getClob( columnIndex );
}

Reference< XArray > OCallableStatement::getArray( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(WeakComponentImplHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY_THROW)->getArray( columnIndex );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
