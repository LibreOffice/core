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
#include <comphelper/property.hxx>
#include "dbastrings.hrc"

using namespace dbaccess;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;

// com::sun::star::lang::XTypeProvider
Sequence< Type > OCallableStatement::getTypes() throw (RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getTypes" );
    OTypeCollection aTypes(::getCppuType( (const Reference< XRow > *)0 ),
                           ::getCppuType( (const Reference< XOutParameters > *)0 ),
                            OPreparedStatement::getTypes() );

    return aTypes.getTypes();
}

Sequence< sal_Int8 > OCallableStatement::getImplementationId() throw (RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getImplementationId" );
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
Any OCallableStatement::queryInterface( const Type & rType ) throw (RuntimeException, std::exception)
{
    //SAL_INFO("dbaccess", "OCallableStatement::queryInterface" );
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
OUString OCallableStatement::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getImplementationName" );
    return OUString("com.sun.star.sdb.OCallableStatement");
}

Sequence< OUString > OCallableStatement::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getSupportedServiceNames" );
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = SERVICE_SDBC_CALLABLESTATEMENT;
    aSNS.getArray()[1] = SERVICE_SDB_CALLABLESTATEMENT;
    return aSNS;
}

// XOutParameters
void SAL_CALL OCallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::registerOutParameter" );
    MutexGuard aGuard(m_aMutex);

    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY)->registerOutParameter( parameterIndex, sqlType, typeName );
}

void SAL_CALL OCallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::registerNumericOutParameter" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY)->registerNumericOutParameter( parameterIndex, sqlType, scale );
}

// XRow
sal_Bool SAL_CALL OCallableStatement::wasNull(  ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::wasNull" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->wasNull();
}

OUString SAL_CALL OCallableStatement::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getString" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getString( columnIndex );
}

sal_Bool SAL_CALL OCallableStatement::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getBoolean" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBoolean( columnIndex );
}

sal_Int8 SAL_CALL OCallableStatement::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getByte" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getByte( columnIndex );
}

sal_Int16 SAL_CALL OCallableStatement::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getShort" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getShort( columnIndex );
}

sal_Int32 SAL_CALL OCallableStatement::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getInt" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getInt( columnIndex );
}

sal_Int64 SAL_CALL OCallableStatement::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getLong" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getLong( columnIndex );
}

float SAL_CALL OCallableStatement::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getFloat" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getFloat( columnIndex );
}

double SAL_CALL OCallableStatement::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getDouble" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getDouble( columnIndex );
}

Sequence< sal_Int8 > SAL_CALL OCallableStatement::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getBytes" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBytes( columnIndex );
}

::com::sun::star::util::Date SAL_CALL OCallableStatement::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getDate" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getDate( columnIndex );
}

::com::sun::star::util::Time SAL_CALL OCallableStatement::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getTime" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getTime( columnIndex );
}

::com::sun::star::util::DateTime SAL_CALL OCallableStatement::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getTimestamp" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getTimestamp( columnIndex );
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getBinaryStream" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBinaryStream( columnIndex );
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getCharacterStream" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getCharacterStream( columnIndex );
}

Any SAL_CALL OCallableStatement::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getObject" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getObject( columnIndex, typeMap );
}

Reference< XRef > SAL_CALL OCallableStatement::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getRef" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getRef( columnIndex );
}

Reference< XBlob > SAL_CALL OCallableStatement::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getBlob" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBlob( columnIndex );
}

Reference< XClob > SAL_CALL OCallableStatement::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getClob" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getClob( columnIndex );
}

Reference< XArray > SAL_CALL OCallableStatement::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OCallableStatement::getArray" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getArray( columnIndex );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
