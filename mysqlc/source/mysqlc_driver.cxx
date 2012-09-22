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
using ::rtl::OUString;
#include <stdio.h>

#include <cppconn/exception.h>
#ifdef SYSTEM_MYSQL_CPPCONN
    #include <mysql_driver.h>
#endif


/* {{{ MysqlCDriver::MysqlCDriver() -I- */
MysqlCDriver::MysqlCDriver(const Reference< XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex)
    ,m_xFactory(_rxFactory)
#ifndef SYSTEM_MYSQL_CPPCONN
    ,m_hCppConnModule( NULL )
    ,m_bAttemptedLoadCppConn( false )
#endif
{
    OSL_TRACE("MysqlCDriver::MysqlCDriver");
    cppDriver = NULL;
}
/* }}} */


/* {{{ MysqlCDriver::disposing() -I- */
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
/* }}} */


// static ServiceInfo
/* {{{ MysqlCDriver::getImplementationName_Static() -I- */
OUString MysqlCDriver::getImplementationName_Static()
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getImplementationName_Static");
    return ::rtl::OUString( "com.sun.star.comp.sdbc.mysqlc.MysqlCDriver"  );
}
/* }}} */


/* {{{ MysqlCDriver::getSupportedServiceNames_Static() -I- */
Sequence< OUString > MysqlCDriver::getSupportedServiceNames_Static()
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getSupportedServiceNames_Static");
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< OUString > aSNS(1);
    aSNS[0] = OUString("com.sun.star.sdbc.Driver");
    return aSNS;
}
/* }}} */


/* {{{ MysqlCDriver::getImplementationName() -I- */
OUString SAL_CALL MysqlCDriver::getImplementationName()
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getImplementationName");
    return getImplementationName_Static();
}
/* }}} */


/* {{{ MysqlCDriver::supportsService() -I- */
sal_Bool SAL_CALL MysqlCDriver::supportsService(const OUString& _rServiceName)
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::supportsService");
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported){}

    return (pSupported != pEnd);
}
/* }}} */


/* {{{ MysqlCDriver::getSupportedServiceNames() -I- */
Sequence< OUString > SAL_CALL MysqlCDriver::getSupportedServiceNames()
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getSupportedServiceNames");
    return getSupportedServiceNames_Static();
}
/* }}} */


extern "C" { static void SAL_CALL thisModule() {} }

void MysqlCDriver::impl_initCppConn_lck_throw()
{
#ifdef SYSTEM_MYSQL_CPPCONN
    cppDriver = get_driver_instance();
#else
    if ( !m_bAttemptedLoadCppConn )
    {
        const ::rtl::OUString sModuleName(CPPCONN_LIB );
        m_hCppConnModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, 0 );
        m_bAttemptedLoadCppConn = true;
    }

    // attempted to load - was it successful?
    if ( !m_hCppConnModule )
    {
        OSL_FAIL( "MysqlCDriver::impl_initCppConn_lck_throw: could not load the " CPPCONN_LIB " library!");
        throw SQLException(
            ::rtl::OUString( "Unable to load the " CPPCONN_LIB " library."  ),
            *this,
            ::rtl::OUString( "08001"  ),  // "unable to connect"
            0,
            Any()
        );
    }

    // find the factory symbol
    const ::rtl::OUString sSymbolName = ::rtl::OUString( "sql_mysql_get_driver_instance"  );
    typedef void* (* FGetMySQLDriver)();

    const FGetMySQLDriver pFactoryFunction = (FGetMySQLDriver)( osl_getFunctionSymbol( m_hCppConnModule, sSymbolName.pData ) );
    if ( !pFactoryFunction )
    {
        OSL_FAIL( "MysqlCDriver::impl_initCppConn_lck_throw: could not find the factory symbol in " CPPCONN_LIB "!");
        throw SQLException(
            ::rtl::OUString( CPPCONN_LIB " is invalid: missing the driver factory function."  ),
            *this,
            ::rtl::OUString( "08001"  ),  // "unable to connect"
            0,
            Any()
        );
    }

    cppDriver = static_cast< sql::Driver* >( (*pFactoryFunction)() );
#endif
    if ( !cppDriver )
    {
        throw SQLException(
            ::rtl::OUString( "Unable to obtain the MySQL_Driver instance from Connector/C++."  ),
            *this,
            ::rtl::OUString( "08001"  ),  // "unable to connect"
            0,
            Any()
        );
    }
}

/* {{{ MysqlCDriver::connect() -I- */
Reference< XConnection > SAL_CALL MysqlCDriver::connect(const OUString& url, const Sequence< PropertyValue >& info)
    throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OSL_TRACE("MysqlCDriver::connect");
    if (!acceptsURL(url)) {
        return NULL;
    }

    if ( !cppDriver )
    {
        impl_initCppConn_lck_throw();
        OSL_POSTCOND( cppDriver, "MySQLCDriver::connect: internal error." );
        if ( !cppDriver )
            throw RuntimeException( ::rtl::OUString( "MySQLCDriver::connect: internal error."  ), *this );
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
/* }}} */


/* {{{ MysqlCDriver::acceptsURL() -I- */
sal_Bool SAL_CALL MysqlCDriver::acceptsURL(const OUString& url)
        throw(SQLException, RuntimeException)
{
    OSL_TRACE("MysqlCDriver::acceptsURL");
    return (!url.compareTo(OUString("sdbc:mysqlc:"), sizeof("sdbc:mysqlc:")-1));
}
/* }}} */


/* {{{ MysqlCDriver::getPropertyInfo() -I- */
Sequence< DriverPropertyInfo > SAL_CALL MysqlCDriver::getPropertyInfo(const OUString& url, const Sequence< PropertyValue >& /* info */)
    throw(SQLException, RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getPropertyInfo");
    if (acceptsURL(url)) {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("Hostname")
                ,OUString("Name of host")
                ,sal_True
                ,OUString("localhost")
                ,Sequence< OUString >())
            );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("Port")
                ,OUString("Port")
                ,sal_True
                ,OUString("3306")
                ,Sequence< OUString >())
            );
        return Sequence< DriverPropertyInfo >(&(aDriverInfo[0]),aDriverInfo.size());
    }

    return Sequence< DriverPropertyInfo >();
}
/* }}} */


/* {{{ MysqlCDriver::getMajorVersion() -I- */
sal_Int32 SAL_CALL MysqlCDriver::getMajorVersion()
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getMajorVersion");
    return MYSQLC_VERSION_MAJOR;
}
/* }}} */


/* {{{ MysqlCDriver::getMinorVersion() -I- */
sal_Int32 SAL_CALL MysqlCDriver::getMinorVersion()
    throw(RuntimeException)
{
    OSL_TRACE("MysqlCDriver::getMinorVersion");
    return MYSQLC_VERSION_MINOR;
}
/* }}} */


namespace connectivity
{
namespace mysqlc
{

Reference< XInterface >  SAL_CALL MysqlCDriver_CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
    throw(::com::sun::star::uno::Exception)
{
    return(*(new MysqlCDriver(_rxFactory)));
}

/* {{{ connectivity::mysqlc::release() -I- */
void release(oslInterlockedCount& _refCount,
             ::cppu::OBroadcastHelper& rBHelper,
             Reference< XInterface >& _xInterface,
             ::com::sun::star::lang::XComponent* _pObject)
{
    if (osl_atomic_decrement(&_refCount) == 0) {
        osl_atomic_increment(&_refCount);

        if (!rBHelper.bDisposed && !rBHelper.bInDispose) {
            // remember the parent
            Reference< XInterface > xParent;
            {
                ::osl::MutexGuard aGuard(rBHelper.rMutex);
                xParent = _xInterface;
                _xInterface = NULL;
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
/* }}} */



/* {{{ connectivity::mysqlc::checkDisposed() -I- */
void checkDisposed(sal_Bool _bThrow)
    throw (DisposedException)
{
    if (_bThrow) {
        throw DisposedException();
    }
}
/* }}} */

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
