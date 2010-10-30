/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include <callablestatement.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <tools/debug.hxx>
#include "dbastrings.hrc"
#include <rtl/logfile.hxx>

using namespace dbaccess;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;

// com::sun::star::lang::XTypeProvider
Sequence< Type > OCallableStatement::getTypes() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getTypes" );
    OTypeCollection aTypes(::getCppuType( (const Reference< XRow > *)0 ),
                           ::getCppuType( (const Reference< XOutParameters > *)0 ),
                            OPreparedStatement::getTypes() );

    return aTypes.getTypes();
}

Sequence< sal_Int8 > OCallableStatement::getImplementationId() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getImplementationId" );
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
Any OCallableStatement::queryInterface( const Type & rType ) throw (RuntimeException)
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::queryInterface" );
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
rtl::OUString OCallableStatement::getImplementationName(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getImplementationName" );
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.OCallableStatement"));
}

Sequence< ::rtl::OUString > OCallableStatement::getSupportedServiceNames(  ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getSupportedServiceNames" );
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS.getArray()[0] = SERVICE_SDBC_CALLABLESTATEMENT;
    aSNS.getArray()[1] = SERVICE_SDB_CALLABLESTATEMENT;
    return aSNS;
}

// XOutParameters
void SAL_CALL OCallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::registerOutParameter" );
    MutexGuard aGuard(m_aMutex);

    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY)->registerOutParameter( parameterIndex, sqlType, typeName );
}

void SAL_CALL OCallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::registerNumericOutParameter" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XOutParameters >(m_xAggregateAsSet, UNO_QUERY)->registerNumericOutParameter( parameterIndex, sqlType, scale );
}

// XRow
sal_Bool SAL_CALL OCallableStatement::wasNull(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::wasNull" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->wasNull();
}

::rtl::OUString SAL_CALL OCallableStatement::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getString" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getString( columnIndex );
}

sal_Bool SAL_CALL OCallableStatement::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getBoolean" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBoolean( columnIndex );
}

sal_Int8 SAL_CALL OCallableStatement::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getByte" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getByte( columnIndex );
}

sal_Int16 SAL_CALL OCallableStatement::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getShort" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getShort( columnIndex );
}

sal_Int32 SAL_CALL OCallableStatement::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getInt" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getInt( columnIndex );
}

sal_Int64 SAL_CALL OCallableStatement::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getLong" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getLong( columnIndex );
}

float SAL_CALL OCallableStatement::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getFloat" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getFloat( columnIndex );
}

double SAL_CALL OCallableStatement::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getDouble" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getDouble( columnIndex );
}

Sequence< sal_Int8 > SAL_CALL OCallableStatement::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getBytes" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBytes( columnIndex );
}

::com::sun::star::util::Date SAL_CALL OCallableStatement::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getDate" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getDate( columnIndex );
}

::com::sun::star::util::Time SAL_CALL OCallableStatement::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getTime" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getTime( columnIndex );
}

::com::sun::star::util::DateTime SAL_CALL OCallableStatement::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getTimestamp" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getTimestamp( columnIndex );
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getBinaryStream" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBinaryStream( columnIndex );
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getCharacterStream" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getCharacterStream( columnIndex );
}

Any SAL_CALL OCallableStatement::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getObject" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getObject( columnIndex, typeMap );
}

Reference< XRef > SAL_CALL OCallableStatement::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getRef" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getRef( columnIndex );
}

Reference< XBlob > SAL_CALL OCallableStatement::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getBlob" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getBlob( columnIndex );
}

Reference< XClob > SAL_CALL OCallableStatement::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getClob" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getClob( columnIndex );
}

Reference< XArray > SAL_CALL OCallableStatement::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCallableStatement::getArray" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return Reference< XRow >(m_xAggregateAsSet, UNO_QUERY)->getArray( columnIndex );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
