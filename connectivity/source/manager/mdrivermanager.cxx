/*************************************************************************
 *
 *  $RCSfile: mdrivermanager.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-24 06:09:35 $
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
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace connectivity
{
namespace sdbc
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;

#define SERVICE_SDBC_DRIVER     ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver")

void throwNoSuchElementException() throw(NoSuchElementException)
{
    throw NoSuchElementException();
}

//==========================================================================
//= ODriverEnumeration
//==========================================================================
class ODriverEnumeration : public ::cppu::WeakImplHelper1< XEnumeration >
{
    friend class OSDBCDriverManager;

    DECLARE_STL_VECTOR(OSDBCDriverManager::SdbcDriver, Drivers);
    Drivers             m_aDrivers;
    sal_Int32           m_nPos;

protected:
    virtual ~ODriverEnumeration();
public:
    ODriverEnumeration(const Drivers& _rDriverSequence);

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
sal_Bool SAL_CALL ODriverEnumeration::hasMoreElements(  ) throw(RuntimeException)
{
    return m_nPos < m_aDrivers.size();
}

//--------------------------------------------------------------------------
Any SAL_CALL ODriverEnumeration::nextElement(  ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    if (!hasMoreElements())
        throwNoSuchElementException();
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
        Reference< XSingleServiceFactory > xFactory;
        while (xEnumDrivers->hasMoreElements())
        {
            if (::cppu::extractInterface(xFactory, xEnumDrivers->nextElement()))
            {
                Reference< XDriver > xDriver(xFactory->createInstance(), UNO_QUERY);
                if (xDriver.is())
                    m_aDriversBS.push_back(xDriver);
            }
        }
    }
    initializeDriverPrecedence();
}

//--------------------------------------------------------------------------
void OSDBCDriverManager::initializeDriverPrecedence()
{
    if (!m_aDriversBS.size())
        // nothing to do
        return;

    try
    {
        // some strings we need
        const ::rtl::OUString sConfigurationProviderServiceName =
            ::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider");
        const ::rtl::OUString sDriverManagerConfigLocation =
            ::rtl::OUString::createFromAscii("org.openoffice.Office.DataAccess/DriverManager");
        const ::rtl::OUString sDriverPreferenceLocation =
            ::rtl::OUString::createFromAscii("DriverPrecedence");
        const ::rtl::OUString sNodePathArgumentName =
            ::rtl::OUString::createFromAscii("nodepath");
        const ::rtl::OUString sNodeAccessServiceName =
            ::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess");

        // create a configuration provider
        Reference< XMultiServiceFactory > xConfigurationProvider(
            m_xServiceFactory->createInstance(sConfigurationProviderServiceName),
            UNO_QUERY);
        OSL_ENSURE(xConfigurationProvider.is(), "OSDBCDriverManager::initializeDriverPrecedence: could not instantiate the configuration provider!");
        if (xConfigurationProvider.is())
        {
            // one argument for creating the node access: the path to the configuration node
            Sequence< Any > aCreationArgs(1);
            aCreationArgs[0] <<= PropertyValue(sNodePathArgumentName, 0, makeAny(sDriverManagerConfigLocation), PropertyState_DIRECT_VALUE);

            // create the node access
            Reference< XNameAccess > xDriverManagerNode(xConfigurationProvider->createInstanceWithArguments(sNodeAccessServiceName, aCreationArgs), UNO_QUERY);

            OSL_ENSURE(xDriverManagerNode.is(), "OSDBCDriverManager::initializeDriverPrecedence: could not open my configuration node!");
            if (xDriverManagerNode.is())
            {
                // obtain the preference list
                Any aPreferences = xDriverManagerNode->getByName(sDriverPreferenceLocation);
                Sequence< ::rtl::OUString > aDriverOrder;
#ifdef _DEBUG
                sal_Bool bSuccess =
#endif
                aPreferences >>= aDriverOrder;
                OSL_ENSURE(bSuccess || !aPreferences.hasValue(), "OSDBCDriverManager::initializeDriverPrecedence: invalid value for the preferences node (no string sequence but not NULL)!");

                if (!aDriverOrder.getLength())
                    // nothing to do
                    return;

                // we have a list of driver implementation names which specify the order to use

                // first collect the implementation names of our bootstrapped drivers
                DECLARE_STL_USTRINGACCESS_MAP( sal_Int32, MapString2Int);
                MapString2Int aDriverImplNames;
                for (   ConstBootstrappedDriversIterator aDriverLoop = m_aDriversBS.begin();
                        aDriverLoop != m_aDriversBS.end();
                        ++aDriverLoop
                    )
                {
                    Reference< XServiceInfo > xDriverSI(*aDriverLoop, UNO_QUERY);
                    OSL_ENSURE(xDriverSI.is(), "OSDBCDriverManager::initializeDriverPrecedence: encountered a driver without service info!");
                    if (xDriverSI.is())
                        aDriverImplNames.insert(MapString2Int::value_type(xDriverSI->getImplementationName(),sal_Int32(aDriverLoop - m_aDriversBS.begin())));
                }

                BootstrappedDrivers aSortedDrivers;
                aSortedDrivers.reserve(3);
                    // this will be the sorted drivers
                ::std::set< sal_Int32 > aPreferedDriversOriginalPos;
                    // the drivers in m_aDriversBS which have been inserted in aSortedDrivers

                // loop through the sequence telling us the preferred driver order
                const ::rtl::OUString* pDriverOrder = aDriverOrder.getConstArray();
                const ::rtl::OUString* pDriverOrderEnd = pDriverOrder + aDriverOrder.getLength();
                sal_Int32 nPreferredDriverInsertPosition = 0;
                    // the position in m_aDriversBS where the current preferred driver is to be inserted
                for (;pDriverOrder < pDriverOrderEnd; ++pDriverOrder)
                {
                    MapString2IntIterator aThisDriverPos = aDriverImplNames.find(*pDriverOrder);
                    if (aDriverImplNames.end() != aThisDriverPos)
                    {   // we know this driver
                        sal_Int32 nThisDriverPos = aThisDriverPos->second;

                        aSortedDrivers.push_back(m_aDriversBS[nThisDriverPos]);
                        aPreferedDriversOriginalPos.insert(nThisDriverPos);
                    }
                }

                if (aPreferedDriversOriginalPos.size() != m_aDriversBS.size())
                {
                    // we still have drivers in m_aDriversBS which have not been inserted into aSortedDrivers
                    // -> copy the remaining drivers
                    sal_Int32 i = 0;
                    for (   ConstBootstrappedDriversIterator aDriverLoop = m_aDriversBS.begin();
                            aDriverLoop != m_aDriversBS.end();
                            ++aDriverLoop, ++i
                        )
                    {
                        if (aPreferedDriversOriginalPos.end() == aPreferedDriversOriginalPos.find(i))
                        {   // the driver has not been inserted into aSortedDrivers, yet
                            aSortedDrivers.push_back(*aDriverLoop);
                        }
                    }
                }

                OSL_ENSURE(m_aDriversBS.size() == aSortedDrivers.size(), "OSDBCDriverManager::initializeDriverPrecedence: inconsistence!");

                // now we have it ...
                m_aDriversBS = aSortedDrivers;
            }
        }
    }
    catch (Exception&)
    {
        OSL_ENSURE(sal_False, "OSDBCDriverManager::initializeDriverPrecedence: caught an exception while sorting the drivers!");
    }
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
    return !(m_aDriversBS.empty() && m_aDriversRT.empty());
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSDBCDriverManager::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OSDBCDriverManager::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
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
        throwNoSuchElementException();

    return aSearch->second;
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::registerObject( const ::rtl::OUString& _rName, const Reference< XInterface >& _rxObject ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ConstRuntimeDriversIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch == m_aDriversRT.end())
    {
        Reference< XDriver > xNewDriver(_rxObject, UNO_QUERY);
        if (xNewDriver.is())
            m_aDriversRT.insert(RuntimeDrivers::value_type(_rName, xNewDriver));
        else
            throw IllegalArgumentException();
    }
    else
        throw ElementExistException();
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::revokeObject( const ::rtl::OUString& _rName ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    RuntimeDriversIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch == m_aDriversRT.end())
        throwNoSuchElementException();

    m_aDriversRT.erase(aSearch); // we already have the iterator so we could use it
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

}   // namespace connectivity
}   // namespace sdbc


