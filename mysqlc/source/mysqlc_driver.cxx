/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/
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

#include <preextstl.h>
#include <cppconn/exception.h>
#include <mysql_driver.h>
#include <postextstl.h>


/* {{{ MysqlCDriver::MysqlCDriver() -I- */
MysqlCDriver::MysqlCDriver(const Reference< XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex)
    ,m_xFactory(_rxFactory)
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
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sdbc.mysqlc.MysqlCDriver" ) );
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
    aSNS[0] = OUString::createFromAscii("com.sun.star.sdbc.Driver");
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
    cppDriver = get_driver_instance();
    if ( !cppDriver )
    {
        throw SQLException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unable to obtain the MySQL_Driver instance from Connector/C++." ) ),
            *this,
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "08001" ) ),  // "unable to connect"
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
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MySQLCDriver::connect: internal error." ) ), *this );
    }

    Reference< XConnection > xConn;
    // create a new connection with the given properties and append it to our vector
    try {
        OConnection* pCon = new OConnection(*this, cppDriver);
        xConn = pCon;

        pCon->construct(url,info);
        m_xConnections.push_back(WeakReferenceHelper(*pCon));
    }
    catch (sql::SQLException &e)
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
    return (!url.compareTo(OUString::createFromAscii("sdbc:mysqlc:"), sizeof("sdbc:mysqlc:")-1));
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
                OUString(RTL_CONSTASCII_USTRINGPARAM("Hostname"))
                ,OUString(RTL_CONSTASCII_USTRINGPARAM("Name of host"))
                ,sal_True
                ,OUString::createFromAscii("localhost")
                ,Sequence< OUString >())
            );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Port"))
                ,OUString(RTL_CONSTASCII_USTRINGPARAM("Port"))
                ,sal_True
                ,OUString::createFromAscii("3306")
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
    if (osl_decrementInterlockedCount(&_refCount) == 0) {
        osl_incrementInterlockedCount(&_refCount);

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
        osl_incrementInterlockedCount(&_refCount);
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
