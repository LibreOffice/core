/*************************************************************************
 *
 *  $RCSfile: mdrivermanager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 08:54:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>

#ifndef _CONNECTIVITY_DRIVERMANAGER_HXX_
#include "drivermanager.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_ELEMENTEXISTEXCEPTION_HPP_
#include <com/sun/star/container/ElementExistException.hpp>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;

#define SERVICE_SDBC_DRIVER     ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver")

//==========================================================================
//= ODriverEnumeration
//==========================================================================
class ODriverEnumeration : public ::cppu::ImplHelper1< XEnumeration >
{
    friend class OSDBCDriverManager;

    oslInterlockedCount     m_refCount;

    DECLARE_STL_VECTOR(OSDBCDriverManager::SdbcDriver, Drivers);
    Drivers             m_aDrivers;
    sal_Int32           m_nPos;

    ~ODriverEnumeration();
public:
    ODriverEnumeration(const Drivers& _rDriverSequence);

// XInterface
    virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException);

// XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements( ) throw(RuntimeException);
    virtual Any SAL_CALL nextElement( ) throw(NoSuchElementException, WrappedTargetException, RuntimeException);
};

//--------------------------------------------------------------------------
ODriverEnumeration::ODriverEnumeration(const Drivers& _rDriverSequence)
    :m_aDrivers(_rDriverSequence)
    ,m_nPos(0)
{
}

//--------------------------------------------------------------------------
ODriverEnumeration::~ODriverEnumeration()
{
}

//--------------------------------------------------------------------------
void SAL_CALL ODriverEnumeration::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    osl_incrementInterlockedCount(&m_refCount);
}

//--------------------------------------------------------------------------
void SAL_CALL ODriverEnumeration::release() throw(::com::sun::star::uno::RuntimeException)
{
    osl_decrementInterlockedCount(&m_refCount);
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL ODriverEnumeration::hasMoreElements(  ) throw(RuntimeException)
{
    return m_nPos < m_aDrivers.size();
}

//--------------------------------------------------------------------------
Any SAL_CALL ODriverEnumeration::nextElement(  ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    if (!hasMoreElements())
        throw NoSuchElementException();
    return makeAny(m_aDrivers[m_nPos++]);
}

//==========================================================================
//= OSDBCDriverManager
//==========================================================================
//--------------------------------------------------------------------------
OSDBCDriverManager::OSDBCDriverManager(const Reference< XMultiServiceFactory >& _rxFactory)
    :m_xServiceFactory(_rxFactory)
    ,m_nLoginTimeout(NULL)
{
    // bootstrap all objects supporting the .sdb.Driver service

    Reference< XContentEnumerationAccess > xEnumAccess(_rxFactory, UNO_QUERY);
    Reference< XEnumeration > xEnumDrivers;
    if (xEnumAccess.is())
        xEnumDrivers = xEnumAccess->createContentEnumeration(SERVICE_SDBC_DRIVER);

    if (xEnumDrivers.is())
    {
        while (xEnumDrivers->hasMoreElements())
        {
            Any aCurrent = xEnumDrivers->nextElement();
            Reference< XSingleServiceFactory > xFactory;

            if (!::cppu::extractInterface(xFactory, aCurrent))
                continue;

            Reference< XDriver > xDriver(xFactory->createInstance(), UNO_QUERY);
            if (xDriver.is())
                m_aDriversBS.push_back(xDriver);
        }
    }
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    osl_incrementInterlockedCount(&m_refCount);
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::release() throw(::com::sun::star::uno::RuntimeException)
{
    osl_decrementInterlockedCount(&m_refCount);
}

//--------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OSDBCDriverManager::getConnection( const ::rtl::OUString& _rURL ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Reference< XConnection > xConnection;
    Reference< XDriver > xDriver = implGetDriverForURL(_rURL);
    if (xDriver.is())
        // TODO : handle the login timeout
        xConnection = xDriver->connect(_rURL, Sequence< PropertyValue >());
        // may throw an exception

    return xConnection;
}

//--------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OSDBCDriverManager::getConnectionWithInfo( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rInfo ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Reference< XConnection > xConnection;
    Reference< XDriver > xDriver = implGetDriverForURL(_rURL);
    if (xDriver.is())
        // TODO : handle the login timeout
        xConnection = xDriver->connect(_rURL, _rInfo);
        // may throw an exception

    return xConnection;
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::setLoginTimeout( sal_Int32 seconds ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    m_nLoginTimeout = seconds;
}

//--------------------------------------------------------------------------
sal_Int32 SAL_CALL OSDBCDriverManager::getLoginTimeout(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_nLoginTimeout;
}

//--------------------------------------------------------------------------
Reference< XEnumeration > SAL_CALL OSDBCDriverManager::createEnumeration(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    ODriverEnumeration::Drivers aDrivers(m_aDriversBS);
    for (ConstRuntimeDriversIterator aLoop = m_aDriversRT.begin(); aLoop != m_aDriversRT.end(); ++aLoop)
        aDrivers.push_back(aLoop->second);

    return new ODriverEnumeration(aDrivers);
}

//--------------------------------------------------------------------------
::com::sun::star::uno::Type SAL_CALL OSDBCDriverManager::getElementType(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::getCppuType(static_cast< Reference< XDriver >* >(NULL));
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OSDBCDriverManager::hasElements(  ) throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return (m_aDriversBS.size() + m_aDriversRT.size()) != 0;
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSDBCDriverManager::getImplementationName(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return getImplementationName_Static();
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OSDBCDriverManager::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;

    return sal_False;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OSDBCDriverManager::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OSDBCDriverManager::CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return static_cast<XDriverManager*>(new OSDBCDriverManager(_rxFactory));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSDBCDriverManager::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbc.OSDBCDriverManager");
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OSDBCDriverManager::getSupportedServiceNames_Static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.DriverManager");
    return aSupported;
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OSDBCDriverManager::getRegisteredObject( const ::rtl::OUString& _rName ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ConstRuntimeDriversIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch == m_aDriversRT.end())
        throw NoSuchElementException();

    return aSearch->second;
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::registerObject( const ::rtl::OUString& _rName, const Reference< XInterface >& _rxObject ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ConstRuntimeDriversIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch != m_aDriversRT.end())
        throw ElementExistException();

    Reference< XDriver > xNewDriver(_rxObject, UNO_QUERY);
    if (!xNewDriver.is())
        throw IllegalArgumentException();

    m_aDriversRT[_rName] = xNewDriver;
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::revokeObject( const ::rtl::OUString& _rName ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ConstRuntimeDriversIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch == m_aDriversRT.end())
        throw NoSuchElementException();

    m_aDriversRT.erase(_rName);
}

//--------------------------------------------------------------------------
Reference< XDriver > SAL_CALL OSDBCDriverManager::getDriverByURL( const ::rtl::OUString& _rURL ) throw(RuntimeException)
{
    return implGetDriverForURL(_rURL);
}

//--------------------------------------------------------------------------
Reference< XDriver > OSDBCDriverManager::implGetDriverForURL(const ::rtl::OUString& _rURL)
{
    // search all bootstrapped drivers
    for (   ConstBootstrappedDriversIterator aSearchBS = m_aDriversBS.begin();
            aSearchBS != m_aDriversBS.end();
            ++aSearchBS
        )
    {
        try
        {
            if ((*aSearchBS)->acceptsURL(_rURL))
                return *aSearchBS;
        }
        catch(SQLException&)
        {
        }
    }

    // search all drivers registered at runtime
    for (   ConstRuntimeDriversIterator aSearchRT = m_aDriversRT.begin();
            aSearchRT != m_aDriversRT.end();
            ++aSearchRT
        )
    {
        try
        {
            if (aSearchRT->second->acceptsURL(_rURL))
                return aSearchRT->second;
        }
        catch(SQLException&)
        {
        }
    }


    return Reference< XDriver >();
}


