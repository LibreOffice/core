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

#include <callablestatement.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/property.hxx>
#include "stringconstants.hxx"
#include "strings.hxx"

using namespace dbaccess;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
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
    return css::uno::Sequence<sal_Int8>();
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

void OCallableStatement::acquire() throw ()
{
    OPreparedStatement::acquire();
}

void OCallableStatement::release() throw ()
{
    OPreparedStatement::release();
}

// XServiceInfo
OUString OCallableStatement::getImplementationName(  )
{
    return OUString("com.sun.star.sdb.OCallableStatement");
}

Sequence< OUString > OCallableStatement::getSupportedServiceNames(  )
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = SERVICE_SDBC_CALLABLESTATEMENT;
    aSNS.getArray()[1] = SERVICE_SDB_CALLABLESTATEMENT;
    return aSNS;
}

// XOutParameters
void SAL_CALL OCallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName )
{
    MutexGuard aGuard(m_aMutex);

    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY)->registerOutParameter( parameterIndex, sqlType, typeName );
}

void SAL_CALL OCallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY)->registerNumericOutParameter( parameterIndex, sqlType, scale );
}

// XRow
sal_Bool SAL_CALL OCallableStatement::wasNull(  )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->wasNull();
}

OUString SAL_CALL OCallableStatement::getString( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getString( columnIndex );
}

sal_Bool SAL_CALL OCallableStatement::getBoolean( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBoolean( columnIndex );
}

sal_Int8 SAL_CALL OCallableStatement::getByte( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getByte( columnIndex );
}

sal_Int16 SAL_CALL OCallableStatement::getShort( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getShort( columnIndex );
}

sal_Int32 SAL_CALL OCallableStatement::getInt( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getInt( columnIndex );
}

sal_Int64 SAL_CALL OCallableStatement::getLong( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getLong( columnIndex );
}

float SAL_CALL OCallableStatement::getFloat( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getFloat( columnIndex );
}

double SAL_CALL OCallableStatement::getDouble( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getDouble( columnIndex );
}

Sequence< sal_Int8 > SAL_CALL OCallableStatement::getBytes( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBytes( columnIndex );
}

css::util::Date SAL_CALL OCallableStatement::getDate( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getDate( columnIndex );
}

css::util::Time SAL_CALL OCallableStatement::getTime( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getTime( columnIndex );
}

css::util::DateTime SAL_CALL OCallableStatement::getTimestamp( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getTimestamp( columnIndex );
}

Reference< css::io::XInputStream > SAL_CALL OCallableStatement::getBinaryStream( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBinaryStream( columnIndex );
}

Reference< css::io::XInputStream > SAL_CALL OCallableStatement::getCharacterStream( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getCharacterStream( columnIndex );
}

Any SAL_CALL OCallableStatement::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& typeMap )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getObject( columnIndex, typeMap );
}

Reference< XRef > SAL_CALL OCallableStatement::getRef( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getRef( columnIndex );
}

Reference< XBlob > SAL_CALL OCallableStatement::getBlob( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBlob( columnIndex );
}

Reference< XClob > SAL_CALL OCallableStatement::getClob( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getClob( columnIndex );
}

Reference< XArray > SAL_CALL OCallableStatement::getArray( sal_Int32 columnIndex )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getArray( columnIndex );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
