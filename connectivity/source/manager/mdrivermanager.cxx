/*************************************************************************
 *
 *  $RCSfile: mdrivermanager.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:39:55 $
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

#include <algorithm>
#include <functional>

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

    DECLARE_STL_VECTOR( SdbcDriver, DriverArray );
    DriverArray                 m_aDrivers;
    ConstDriverArrayIterator    m_aPos;
    // order matters!

protected:
    virtual ~ODriverEnumeration();
public:
    ODriverEnumeration(const DriverArray& _rDriverSequence);

// XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements( ) throw(RuntimeException);
    virtual Any SAL_CALL nextElement( ) throw(NoSuchElementException, WrappedTargetException, RuntimeException);
};

//--------------------------------------------------------------------------
ODriverEnumeration::ODriverEnumeration(const DriverArray& _rDriverSequence)
    :m_aDrivers( _rDriverSequence )
    ,m_aPos( m_aDrivers.begin() )
{
}

//--------------------------------------------------------------------------
ODriverEnumeration::~ODriverEnumeration()
{
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL ODriverEnumeration::hasMoreElements(  ) throw(RuntimeException)
{
    return m_aPos != m_aDrivers.end();
}

//--------------------------------------------------------------------------
Any SAL_CALL ODriverEnumeration::nextElement(  ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    if ( !hasMoreElements() )
        throwNoSuchElementException();

    return makeAny( *m_aPos++ );
}

    //=====================================================================
    //= helper
    //=====================================================================
    //---------------------------------------------------------------------
    //--- 24.08.01 11:27:59 -----------------------------------------------

    /// an STL functor which ensures that a SdbcDriver described by a DriverAccess is loaded
    struct EnsureDriver : public ::std::unary_function< DriverAccess, DriverAccess >
    {
        const DriverAccess& operator()( const DriverAccess& _rDescriptor ) const
        {
            if ( !_rDescriptor.xDriver.is() )
                // we did not load this driver, yet
                if ( _rDescriptor.xComponentFactory.is() )
                    // we have a factory for it
                    const_cast< DriverAccess& >( _rDescriptor ).xDriver = _rDescriptor.xDriver.query( _rDescriptor.xComponentFactory->createInstance() );
            return _rDescriptor;
        }
    };

    //---------------------------------------------------------------------
    //--- 24.08.01 11:28:04 -----------------------------------------------

    /// an STL functor which extracts a SdbcDriver from a DriverAccess
    struct ExtractDriverFromAccess : public ::std::unary_function< DriverAccess, SdbcDriver >
    {
        SdbcDriver operator()( const DriverAccess& _rAccess ) const
        {
            return _rAccess.xDriver;
        }
    };

    //---------------------------------------------------------------------
    //--- 24.08.01 12:37:50 -----------------------------------------------

    typedef ::std::unary_compose< ExtractDriverFromAccess, EnsureDriver > ExtractAfterLoad_BASE;
    /// an STL functor which loads a driver described by a DriverAccess, and extracts the SdbcDriver
    struct ExtractAfterLoad : public ExtractAfterLoad_BASE
    {
        ExtractAfterLoad() : ExtractAfterLoad_BASE( ExtractDriverFromAccess(), EnsureDriver() ) { }
    };

    //---------------------------------------------------------------------
    //--- 24.08.01 11:42:36 -----------------------------------------------

    struct ExtractDriverFromCollectionElement : public ::std::unary_function< DriverCollection::value_type, SdbcDriver >
    {
        SdbcDriver operator()( const DriverCollection::value_type& _rElement ) const
        {
            return _rElement.second;
        }
    };

    //---------------------------------------------------------------------
    //--- 24.08.01 11:51:03 -----------------------------------------------

    // predicate for checking whether or not a driver accepts a given URL
    class AcceptsURL : public ::std::unary_function< SdbcDriver, bool >
    {
    protected:
        const ::rtl::OUString& m_rURL;

    public:
        // ctor
        AcceptsURL( const ::rtl::OUString& _rURL ) : m_rURL( _rURL ) { }

        //.................................................................
        bool operator()( const SdbcDriver& _rDriver ) const
        {
            // ask the driver
            if ( _rDriver.is() && _rDriver->acceptsURL( m_rURL ) )
                return true;

            // does not accept ...
            return false;
        }
    };

    //---------------------------------------------------------------------
    //--- 24.08.01 12:51:54 -----------------------------------------------

    static sal_Int32 lcl_getDriverPrecedence( const Reference< XMultiServiceFactory >&  _rxFactory, Sequence< ::rtl::OUString >& _rPrecedence )
    {
        _rPrecedence.realloc( 0 );
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
                _rxFactory->createInstance(sConfigurationProviderServiceName),
                UNO_QUERY);
            OSL_ENSURE(xConfigurationProvider.is(), "lcl_getDriverPrecedence: could not instantiate the configuration provider!");
            if (xConfigurationProvider.is())
            {
                // one argument for creating the node access: the path to the configuration node
                Sequence< Any > aCreationArgs(1);
                aCreationArgs[0] <<= PropertyValue(sNodePathArgumentName, 0, makeAny(sDriverManagerConfigLocation), PropertyState_DIRECT_VALUE);

                // create the node access
                Reference< XNameAccess > xDriverManagerNode(xConfigurationProvider->createInstanceWithArguments(sNodeAccessServiceName, aCreationArgs), UNO_QUERY);

                OSL_ENSURE(xDriverManagerNode.is(), "lcl_getDriverPrecedence: could not open my configuration node!");
                if (xDriverManagerNode.is())
                {
                    // obtain the preference list
                    Any aPreferences = xDriverManagerNode->getByName(sDriverPreferenceLocation);
    #if OSL_DEBUG_LEVEL > 0
                    sal_Bool bSuccess =
    #endif
                    aPreferences >>= _rPrecedence;
                    OSL_ENSURE(bSuccess || !aPreferences.hasValue(), "lcl_getDriverPrecedence: invalid value for the preferences node (no string sequence but not NULL)!");
                }
            }
        }
        catch( const Exception& e)
        {
            e;  // make compiler happy
            OSL_ENSURE( sal_False, "lcl_getDriverPrecedence: caught an exception!" );
        }

        return _rPrecedence.getLength();
    }

    //---------------------------------------------------------------------
    //--- 24.08.01 13:01:56 -----------------------------------------------

    /// an STL argorithm compatible predicate comparing two DriverAccess instances by their implementation names
    struct CompareDriverAccessByName : public ::std::binary_function< DriverAccess, DriverAccess, bool >
    {
        //.................................................................
        bool operator()( const DriverAccess& lhs, const DriverAccess& rhs )
        {
            return lhs.sImplementationName < rhs.sImplementationName ? true : false;
        }
    };

    //---------------------------------------------------------------------
    //--- 24.08.01 13:08:17 -----------------------------------------------

    /// and STL argorithm compatible predicate comparing a DriverAccess' impl name to a string
    struct CompareDriverAccessToName : public ::std::binary_function< DriverAccess, ::rtl::OUString, bool >
    {
        //.................................................................
        bool operator()( const DriverAccess& lhs, const ::rtl::OUString& rhs )
        {
            return lhs.sImplementationName < rhs ? true : false;
        }
        //.................................................................
        bool operator()( const ::rtl::OUString& lhs, const DriverAccess& rhs )
        {
            return lhs < rhs.sImplementationName ? true : false;
        }
    };

//==========================================================================
//= OSDBCDriverManager
//==========================================================================
//--------------------------------------------------------------------------
OSDBCDriverManager::OSDBCDriverManager(const Reference< XMultiServiceFactory >& _rxFactory)
    :m_xServiceFactory(_rxFactory)
    ,m_nLoginTimeout(NULL)
{
    // bootstrap all objects supporting the .sdb.Driver service
    bootstrapDrivers();

    // initialize the drivers order
    initializeDriverPrecedence();
}

//---------------------------------------------------------------------
//--- 24.08.01 11:15:32 -----------------------------------------------

void OSDBCDriverManager::bootstrapDrivers()
{
    Reference< XContentEnumerationAccess > xEnumAccess(m_xServiceFactory, UNO_QUERY);
    Reference< XEnumeration > xEnumDrivers;
    if (xEnumAccess.is())
        xEnumDrivers = xEnumAccess->createContentEnumeration(SERVICE_SDBC_DRIVER);

    OSL_ENSURE( xEnumDrivers.is(), "OSDBCDriverManager::bootstrapDrivers: no enumeration for the drivers available!" );
    if (xEnumDrivers.is())
    {
        Reference< XSingleServiceFactory > xFactory;
        Reference< XServiceInfo > xSI;
        while (xEnumDrivers->hasMoreElements())
        {
            ::cppu::extractInterface( xFactory, xEnumDrivers->nextElement() );
            OSL_ENSURE( xFactory.is(), "OSDBCDriverManager::bootstrapDrivers: no factory extracted" );

            if ( xFactory.is() )
            {
                // we got a factory for the driver
                DriverAccess aDriverDescriptor;
                sal_Bool bValidDescriptor = sal_False;

                // can it tell us something about the implementation name?
                xSI = xSI.query( xFactory );
                if ( xSI.is() )
                {   // yes -> no need to load the driver immediately (load it later when needed)
                    aDriverDescriptor.sImplementationName = xSI->getImplementationName();
                    aDriverDescriptor.xComponentFactory = xFactory;
                    bValidDescriptor = sal_True;
                }
                else
                {
                    // no -> create the driver
                    Reference< XDriver > xDriver( xFactory->createInstance(), UNO_QUERY );
                    OSL_ENSURE( xDriver.is(), "OSDBCDriverManager::bootstrapDrivers: a driver which is no driver?!" );

                    if ( xDriver.is() )
                    {
                        aDriverDescriptor.xDriver = xDriver;
                        // and obtain it's implementation name
                        xSI = xSI.query( xDriver );
                        OSL_ENSURE( xSI.is(), "OSDBCDriverManager::bootstrapDrivers: a driver without service info?" );
                        if ( xSI.is() )
                        {
                            aDriverDescriptor.sImplementationName = xSI->getImplementationName();
                            bValidDescriptor = sal_True;
                        }
                    }
                }

                if ( bValidDescriptor )
                {
                    m_aDriversBS.push_back( aDriverDescriptor );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------
void OSDBCDriverManager::initializeDriverPrecedence()
{
    if (!m_aDriversBS.size())
        // nothing to do
        return;

    try
    {
        // get the precedence of the drivers from the configuration
        Sequence< ::rtl::OUString > aDriverOrder;
        if ( 0 == lcl_getDriverPrecedence( m_xServiceFactory, aDriverOrder ) )
            // nothing to do
            return;

        // aDriverOrder now is the list of driver implementation names in the order they should be used

        // sort our bootstrapped drivers
        ::std::sort( m_aDriversBS.begin(), m_aDriversBS.end(), CompareDriverAccessByName() );

        // loop through the names in the precedence order
        const ::rtl::OUString* pDriverOrder     =                   aDriverOrder.getConstArray();
        const ::rtl::OUString* pDriverOrderEnd  =   pDriverOrder +  aDriverOrder.getLength();

        // the first driver for which there is no preference
        DriverAccessArrayIterator aNoPrefDriversStart = m_aDriversBS.begin();
            // at the moment this is the first of all drivers we know

        for ( ; ( pDriverOrder < pDriverOrderEnd ) && ( aNoPrefDriversStart != m_aDriversBS.end() ); ++pDriverOrder )
        {
            // look for the impl name in the DriverAccess array
            ::std::pair< DriverAccessArrayIterator, DriverAccessArrayIterator > aPos =
                ::std::equal_range( aNoPrefDriversStart, m_aDriversBS.end(), *pDriverOrder, CompareDriverAccessToName() );

            if ( aPos.first != aPos.second )
            {   // we have a DriverAccess with this impl name

                OSL_ENSURE( ::std::distance( aPos.first, aPos.second ) == 1,
                    "OSDBCDriverManager::initializeDriverPrecedence: move than one driver with this impl name? How this?" );
                // move the DriverAccess pointed to by aPos.first to the position pointed to by aNoPrefDriversStart

                if ( aPos.first != aNoPrefDriversStart )
                {   // if this does not hold, the DriverAccess alread has the correct position

                    // rotate the range [aNoPrefDriversStart, aPos.second) right 1 element
                    ::std::rotate( aNoPrefDriversStart, aPos.second - 1, aPos.second );
                }

                // next round we start searching and pos right
                ++aNoPrefDriversStart;
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

    ODriverEnumeration::DriverArray aDrivers;

    // ensure that all our bootstrapped drivers are insatntiated
    ::std::for_each( m_aDriversBS.begin(), m_aDriversBS.end(), EnsureDriver() );

    // copy the bootstrapped drivers
    ::std::transform(
        m_aDriversBS.begin(),               // "copy from" start
        m_aDriversBS.end(),                 // "copy from" end
        ::std::back_inserter( aDrivers ),   // insert into
        ExtractDriverFromAccess()           // transformation to apply (extract a driver from a driver access)
    );

    // append the runtime drivers
    ::std::transform(
        m_aDriversRT.begin(),                   // "copy from" start
        m_aDriversRT.end(),                     // "copy from" end
        ::std::back_inserter( aDrivers ),       // insert into
        ExtractDriverFromCollectionElement()    // transformation to apply (extract a driver from a driver access)
    );

    return new ODriverEnumeration( aDrivers );
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
    ConstDriverCollectionIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch == m_aDriversRT.end())
        throwNoSuchElementException();

    return aSearch->second;
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::registerObject( const ::rtl::OUString& _rName, const Reference< XInterface >& _rxObject ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ConstDriverCollectionIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch == m_aDriversRT.end())
    {
        Reference< XDriver > xNewDriver(_rxObject, UNO_QUERY);
        if (xNewDriver.is())
            m_aDriversRT.insert(DriverCollection::value_type(_rName, xNewDriver));
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
    DriverCollectionIterator aSearch = m_aDriversRT.find(_rName);
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
    Reference< XDriver > xReturn;

    {
        // search all bootstrapped drivers
        DriverAccessArrayIterator aPos = ::std::find_if(
            m_aDriversBS.begin(),       // begin of search range
            m_aDriversBS.end(),         // end of search range
            unary_compose< AcceptsURL, ExtractAfterLoad >( AcceptsURL( _rURL ), ExtractAfterLoad() )
                                        // compose two functors: extract the driver from the access, then ask the resulting driver for acceptance
        );

        // found something?
        if ( m_aDriversBS.end() != aPos )
            xReturn = aPos->xDriver;
    }

    if ( !xReturn.is() )
    {
        // no -> search the runtime drivers
        DriverCollectionIterator aPos = ::std::find_if(
            m_aDriversRT.begin(),       // begin of search range
            m_aDriversRT.end(),         // end of search range
            unary_compose< AcceptsURL, ExtractDriverFromCollectionElement >( AcceptsURL( _rURL ), ExtractDriverFromCollectionElement() )
                                        // compose two functors: extract the driver from the access, then ask the resulting driver for acceptance
        );

        if ( m_aDriversRT.end() != aPos )
            xReturn = aPos->second;
    }

    return xReturn;
}

}   // namespace connectivity
}   // namespace sdbc


