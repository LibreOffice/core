/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
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
************************************************************************/
#include "mysqlc_driver.hxx"
#include "mysqlc_connection.hxx"
#include "mysqlc_general.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace connectivity::mysqlc;

#include <cppuhelper/supportsservice.hxx>
#include <stdio.h>

#include <cppconn/exception.h>
#ifdef SYSTEM_MYSQL_CPPCONN
    #include <mysql_driver.h>
#endif

MysqlCDriver::MysqlCDriver(const Reference< XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex)
    ,m_xFactory(_rxFactory)
#ifndef SYSTEM_MYSQL_CPPCONN
    ,m_hCppConnModule( nullptr )
    ,m_bAttemptedLoadCppConn( false )
#endif
{
    OSL_TRACE("MysqlCDriver::MysqlCDriver");
    cppDriver = nullptr;
}

void MysqlCDriver::disposing()
{
    OSL_TRACE("MysqlCDriver::disposing");
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroied so all our connections have to be destroied as well
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is()) {
            xComp->dispose();
        }
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}

// static ServiceInfo
rtl::OUString MysqlCDriver::getImplementationName_Static()
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getImplementationName_Static");
    return rtl::OUString( "com.sun.star.comp.sdbc.mysqlc.MysqlCDriver"  );
}

Sequence< rtl::OUString > MysqlCDriver::getSupportedServiceNames_Static()
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getSupportedServiceNames_Static");
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence<OUString> aSNS { "com.sun.star.sdbc.Driver" };
    return aSNS;
}

rtl::OUString SAL_CALL MysqlCDriver::getImplementationName()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("MysqlCDriver::getImplementationName");
    return getImplementationName_Static();
}

sal_Bool SAL_CALL MysqlCDriver::supportsService(const rtl::OUString& _rServiceName)
    throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< rtl::OUString > SAL_CALL MysqlCDriver::getSupportedServiceNames()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("MysqlCDriver::getSupportedServiceNames");
    return getSupportedServiceNames_Static();
}

#ifndef SYSTEM_MYSQL_CPPCONN
extern "C" { static void SAL_CALL thisModule() {} }
#endif

void MysqlCDriver::impl_initCppConn_lck_throw()
{
#ifdef SYSTEM_MYSQL_CPPCONN
    cppDriver = get_driver_instance();
#else
#ifdef BUNDLE_MARIADB
    if ( !m_bAttemptedLoadCConn )
    {
        const rtl::OUString sModuleName(BUNDLE_MARIADB);
        m_hCConnModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, 0 );
        m_bAttemptedLoadCConn = true;
    }

    // attempted to load - was it successful?
    if ( !m_hCConnModule )
    {
        OSL_FAIL( "MysqlCDriver::impl_initCppConn_lck_throw: could not load the " BUNDLE_MARIADB " library!");
        throw SQLException(
            "Unable to load the " BUNDLE_MARIADB " library.",
            *this,
            rtl::OUString( "08001"  ),  // "unable to connect"
            0,
            Any()
        );
    }
#endif
    if ( !m_bAttemptedLoadCppConn )
    {
        const rtl::OUString sModuleName(CPPCONN_LIB);
        m_hCppConnModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, 0 );
        m_bAttemptedLoadCppConn = true;
    }

    // attempted to load - was it successful?
    if ( !m_hCppConnModule )
    {
        OSL_FAIL( "MysqlCDriver::impl_initCppConn_lck_throw: could not load the " CPPCONN_LIB " library!");
        throw SQLException(
            "Unable to load the " CPPCONN_LIB " library.",
            *this,
            rtl::OUString( "08001"  ),  // "unable to connect"
            0,
            Any()
        );
    }

    // find the factory symbol
    const rtl::OUString sSymbolName = "sql_mysql_get_driver_instance";
    typedef void* (* FGetMySQLDriver)();

    const FGetMySQLDriver pFactoryFunction = reinterpret_cast<FGetMySQLDriver>( osl_getFunctionSymbol( m_hCppConnModule, sSymbolName.pData ) );
    if ( !pFactoryFunction )
    {
        OSL_FAIL( "MysqlCDriver::impl_initCppConn_lck_throw: could not find the factory symbol in " CPPCONN_LIB "!");
        throw SQLException(
            CPPCONN_LIB " is invalid: missing the driver factory function.",
            *this,
            rtl::OUString( "08001"  ),  // "unable to connect"
            0,
            Any()
        );
    }

    cppDriver = static_cast< sql::Driver* >( (*pFactoryFunction)() );
#endif
    if ( !cppDriver )
    {
        throw SQLException(
            "Unable to obtain the MySQL_Driver instance from Connector/C++.",
            *this,
            rtl::OUString( "08001"  ),  // "unable to connect"
            0,
            Any()
        );
    }
}

Reference< XConnection > SAL_CALL MysqlCDriver::connect(const rtl::OUString& url, const Sequence< PropertyValue >& info)
    throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OSL_TRACE("MysqlCDriver::connect");
    if (!acceptsURL(url)) {
        return nullptr;
    }

    if ( !cppDriver )
    {
        impl_initCppConn_lck_throw();
        if ( !cppDriver )
            throw RuntimeException("MySQLCDriver::connect: internal error.", *this );
    }

    Reference< XConnection > xConn;
    // create a new connection with the given properties and append it to our vector
    try {
        OConnection* pCon = new OConnection(*this, cppDriver);
        xConn = pCon;

        pCon->construct(url,info);
        m_xConnections.push_back(WeakReferenceHelper(*pCon));
    }
    catch (const sql::SQLException &e)
    {
        mysqlc_sdbc_driver::translateAndThrow(e, *this, getDefaultEncoding());
    }
    return xConn;
}

sal_Bool SAL_CALL MysqlCDriver::acceptsURL(const rtl::OUString& url)
        throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("MysqlCDriver::acceptsURL");
    return url.startsWith("sdbc:mysqlc:");
}

Sequence< DriverPropertyInfo > SAL_CALL MysqlCDriver::getPropertyInfo(const rtl::OUString& url, const Sequence< PropertyValue >& /* info */)
    throw(SQLException, RuntimeException, std::exception)
{
    OSL_TRACE("MysqlCDriver::getPropertyInfo");
    if (acceptsURL(url)) {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        aDriverInfo.push_back(DriverPropertyInfo(
                rtl::OUString("Hostname")
                ,rtl::OUString("Name of host")
                ,sal_True
                ,rtl::OUString("localhost")
                ,Sequence< rtl::OUString >())
            );
        aDriverInfo.push_back(DriverPropertyInfo(
                rtl::OUString("Port")
                ,rtl::OUString("Port")
                ,sal_True
                ,rtl::OUString("3306")
                ,Sequence< rtl::OUString >())
            );
        return Sequence< DriverPropertyInfo >(&(aDriverInfo[0]),aDriverInfo.size());
    }

    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL MysqlCDriver::getMajorVersion()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("MysqlCDriver::getMajorVersion");
    return MARIADBC_VERSION_MAJOR;
}

sal_Int32 SAL_CALL MysqlCDriver::getMinorVersion()
    throw(RuntimeException, std::exception)
{
    OSL_TRACE("MysqlCDriver::getMinorVersion");
    return MARIADBC_VERSION_MINOR;
}

namespace connectivity
{
namespace mysqlc
{

Reference< XInterface >  SAL_CALL MysqlCDriver_CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
    throw(css::uno::Exception)
{
    return(*(new MysqlCDriver(_rxFactory)));
}

void release(oslInterlockedCount& _refCount,
             ::cppu::OBroadcastHelper& rBHelper,
             Reference< XInterface >& _xInterface,
             css::lang::XComponent* _pObject)
{
    if (osl_atomic_decrement(&_refCount) == 0) {
        osl_atomic_increment(&_refCount);

        if (!rBHelper.bDisposed && !rBHelper.bInDispose) {
            // remember the parent
            Reference< XInterface > xParent;
            {
                ::osl::MutexGuard aGuard(rBHelper.rMutex);
                xParent = _xInterface;
                _xInterface = nullptr;
            }

            // First dispose
            _pObject->dispose();

            // only the alive ref holds the object
            OSL_ASSERT(_refCount == 1);

            // release the parent in the destructor
            if (xParent.is()) {
                ::osl::MutexGuard aGuard(rBHelper.rMutex);
                _xInterface = xParent;
            }
        }
    } else {
        osl_atomic_increment(&_refCount);
    }
}

void checkDisposed(bool _bThrow)
    throw (DisposedException)
{
    if (_bThrow) {
        throw DisposedException();
    }
}

} /* mysqlc */
} /* connectivity */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
