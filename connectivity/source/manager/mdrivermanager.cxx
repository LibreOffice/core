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


#include <stdio.h>

#include "mdrivermanager.hxx"
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <tools/diagnose_ex.h>
#include <comphelper/extract.hxx>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <iterator>

#include <o3tl/compat_functional.hxx>

namespace drivermanager
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::logging;
using namespace ::osl;

#define SERVICE_SDBC_DRIVER     ::rtl::OUString("com.sun.star.sdbc.Driver")

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

    /// an STL functor which extracts a SdbcDriver from a DriverAccess
    struct ExtractDriverFromAccess : public ::std::unary_function< DriverAccess, SdbcDriver >
    {
        SdbcDriver operator()( const DriverAccess& _rAccess ) const
        {
            return _rAccess.xDriver;
        }
    };

    typedef ::o3tl::unary_compose< ExtractDriverFromAccess, EnsureDriver > ExtractAfterLoad_BASE;
    /// an STL functor which loads a driver described by a DriverAccess, and extracts the SdbcDriver
    struct ExtractAfterLoad : public ExtractAfterLoad_BASE
    {
        ExtractAfterLoad() : ExtractAfterLoad_BASE( ExtractDriverFromAccess(), EnsureDriver() ) { }
    };

    struct ExtractDriverFromCollectionElement : public ::std::unary_function< DriverCollection::value_type, SdbcDriver >
    {
        SdbcDriver operator()( const DriverCollection::value_type& _rElement ) const
        {
            return _rElement.second;
        }
    };

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

    static sal_Int32 lcl_getDriverPrecedence( const ::comphelper::ComponentContext& _rContext, Sequence< ::rtl::OUString >& _rPrecedence )
    {
        _rPrecedence.realloc( 0 );
        try
        {
            // some strings we need
            const ::rtl::OUString sDriverManagerConfigLocation(  "org.openoffice.Office.DataAccess/DriverManager" );
            const ::rtl::OUString sDriverPreferenceLocation(  "DriverPrecedence" );
            const ::rtl::OUString sNodePathArgumentName(  "nodepath" );
            const ::rtl::OUString sNodeAccessServiceName(  "com.sun.star.configuration.ConfigurationAccess" );

            // create a configuration provider
            Reference< XMultiServiceFactory > xConfigurationProvider(
                com::sun::star::configuration::theDefaultProvider::get(
                    _rContext.getUNOContext() ) );

            // one argument for creating the node access: the path to the configuration node
            Sequence< Any > aCreationArgs(1);
            aCreationArgs[0] <<= NamedValue( sNodePathArgumentName, makeAny( sDriverManagerConfigLocation ) );

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
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return _rPrecedence.getLength();
    }

    /// an STL argorithm compatible predicate comparing two DriverAccess instances by their implementation names
    struct CompareDriverAccessByName : public ::std::binary_function< DriverAccess, DriverAccess, bool >
    {
        //.................................................................
        bool operator()( const DriverAccess& lhs, const DriverAccess& rhs )
        {
            return lhs.sImplementationName < rhs.sImplementationName ? true : false;
        }
    };

    /// and STL argorithm compatible predicate comparing a DriverAccess' impl name to a string
    struct EqualDriverAccessToName : public ::std::binary_function< DriverAccess, ::rtl::OUString, bool >
    {
        ::rtl::OUString m_sImplName;
        EqualDriverAccessToName(const ::rtl::OUString& _sImplName) : m_sImplName(_sImplName){}
        //.................................................................
        bool operator()( const DriverAccess& lhs)
        {
            return lhs.sImplementationName.equals(m_sImplName);
        }
    };

//==========================================================================
//= OSDBCDriverManager
//==========================================================================
//--------------------------------------------------------------------------
OSDBCDriverManager::OSDBCDriverManager( const Reference< XComponentContext >& _rxContext )
    :m_aContext( _rxContext )
    ,m_aEventLogger( _rxContext, "org.openoffice.logging.sdbc.DriverManager" )
    ,m_aDriverConfig(m_aContext.getLegacyServiceFactory())
    ,m_nLoginTimeout(0)
{
    // bootstrap all objects supporting the .sdb.Driver service
    bootstrapDrivers();

    // initialize the drivers order
    initializeDriverPrecedence();
}

//---------------------------------------------------------------------
OSDBCDriverManager::~OSDBCDriverManager()
{
}

void OSDBCDriverManager::bootstrapDrivers()
{
    Reference< XContentEnumerationAccess > xEnumAccess( m_aContext.getLegacyServiceFactory(), UNO_QUERY );
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

                    m_aEventLogger.log( LogLevel::CONFIG,
                        "found SDBC driver $1$, no need to load it",
                        aDriverDescriptor.sImplementationName
                    );
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

                            m_aEventLogger.log( LogLevel::CONFIG,
                                "found SDBC driver $1$, needed to load it",
                                aDriverDescriptor.sImplementationName
                            );
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
    if ( m_aDriversBS.empty() )
        // nothing to do
        return;

    try
    {
        // get the precedence of the drivers from the configuration
        Sequence< ::rtl::OUString > aDriverOrder;
        if ( 0 == lcl_getDriverPrecedence( m_aContext, aDriverOrder ) )
            // nothing to do
            return;

        // aDriverOrder now is the list of driver implementation names in the order they should be used

        if ( m_aEventLogger.isLoggable( LogLevel::CONFIG ) )
        {
            sal_Int32 nOrderedCount = aDriverOrder.getLength();
            for ( sal_Int32 i=0; i<nOrderedCount; ++i )
            m_aEventLogger.log( LogLevel::CONFIG,
                "configuration's driver order: driver $1$ of $2$: $3$",
                (sal_Int32)(i + 1), nOrderedCount, aDriverOrder[i]
            );
        }

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
            DriverAccess driver_order;
            driver_order.sImplementationName = *pDriverOrder;

            // look for the impl name in the DriverAccess array
            ::std::pair< DriverAccessArrayIterator, DriverAccessArrayIterator > aPos =
                ::std::equal_range( aNoPrefDriversStart, m_aDriversBS.end(), driver_order, CompareDriverAccessByName() );

            if ( aPos.first != aPos.second )
            {   // we have a DriverAccess with this impl name

                OSL_ENSURE( ::std::distance( aPos.first, aPos.second ) == 1,
                    "OSDBCDriverManager::initializeDriverPrecedence: more than one driver with this impl name? How this?" );
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
        OSL_FAIL("OSDBCDriverManager::initializeDriverPrecedence: caught an exception while sorting the drivers!");
    }
}

//--------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OSDBCDriverManager::getConnection( const ::rtl::OUString& _rURL ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    m_aEventLogger.log( LogLevel::INFO,
        "connection requested for URL $1$",
        _rURL
    );

    Reference< XConnection > xConnection;
    Reference< XDriver > xDriver = implGetDriverForURL(_rURL);
    if (xDriver.is())
    {
        // TODO : handle the login timeout
        xConnection = xDriver->connect(_rURL, Sequence< PropertyValue >());
        // may throw an exception
        m_aEventLogger.log( LogLevel::INFO,
            "connection retrieved for URL $1$",
            _rURL
        );
    }

    return xConnection;
}

//--------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OSDBCDriverManager::getConnectionWithInfo( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rInfo ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    m_aEventLogger.log( LogLevel::INFO,
        "connection with info requested for URL $1$",
        _rURL
    );

    Reference< XConnection > xConnection;
    Reference< XDriver > xDriver = implGetDriverForURL(_rURL);
    if (xDriver.is())
    {
        // TODO : handle the login timeout
        xConnection = xDriver->connect(_rURL, _rInfo);
        // may throw an exception
        m_aEventLogger.log( LogLevel::INFO,
            "connection with info retrieved for URL $1$",
            _rURL
        );
    }

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

    // ensure that all our bootstrapped drivers are instantiated
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
    return getImplementationName_static();
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
    return getSupportedServiceNames_static();
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OSDBCDriverManager::Create( const Reference< XMultiServiceFactory >& _rxFactory )
{
    ::comphelper::ComponentContext aContext( _rxFactory );
    return *( new OSDBCDriverManager( aContext.getUNOContext() ) );
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSDBCDriverManager::getImplementationName_static(  ) throw(RuntimeException)
{
    return ::rtl::OUString("com.sun.star.comp.sdbc.OSDBCDriverManager");
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OSDBCDriverManager::getSupportedServiceNames_static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = getSingletonName_static();
    return aSupported;
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSDBCDriverManager::getSingletonName_static(  ) throw(RuntimeException)
{
    return ::rtl::OUString(  "com.sun.star.sdbc.DriverManager"  );
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OSDBCDriverManager::getRegisteredObject( const ::rtl::OUString& _rName ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ConstDriverCollectionIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch == m_aDriversRT.end())
        throwNoSuchElementException();

    return aSearch->second.get();
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::registerObject( const ::rtl::OUString& _rName, const Reference< XInterface >& _rxObject ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    m_aEventLogger.log( LogLevel::INFO,
        "attempt to register new driver for name $1$",
        _rName
    );

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

    m_aEventLogger.log( LogLevel::INFO,
        "new driver registered for name $1$",
        _rName
    );
}

//--------------------------------------------------------------------------
void SAL_CALL OSDBCDriverManager::revokeObject( const ::rtl::OUString& _rName ) throw(Exception, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    m_aEventLogger.log( LogLevel::INFO,
        "attempt to revoke driver for name $1$",
        _rName
    );

    DriverCollectionIterator aSearch = m_aDriversRT.find(_rName);
    if (aSearch == m_aDriversRT.end())
        throwNoSuchElementException();

    m_aDriversRT.erase(aSearch); // we already have the iterator so we could use it

    m_aEventLogger.log( LogLevel::INFO,
        "driver revoked for name $1$",
        _rName
    );
}

//--------------------------------------------------------------------------
Reference< XDriver > SAL_CALL OSDBCDriverManager::getDriverByURL( const ::rtl::OUString& _rURL ) throw(RuntimeException)
{
    m_aEventLogger.log( LogLevel::INFO,
        "driver requested for URL $1$",
        _rURL
    );

    Reference< XDriver > xDriver( implGetDriverForURL( _rURL ) );

    if ( xDriver.is() )
        m_aEventLogger.log( LogLevel::INFO,
            "driver obtained for URL $1$",
            _rURL
        );

    return xDriver;
}

//--------------------------------------------------------------------------
Reference< XDriver > OSDBCDriverManager::implGetDriverForURL(const ::rtl::OUString& _rURL)
{
    Reference< XDriver > xReturn;

    {
        const ::rtl::OUString sDriverFactoryName = m_aDriverConfig.getDriverFactoryName(_rURL);

        EqualDriverAccessToName aEqual(sDriverFactoryName);
        DriverAccessArray::iterator aFind = ::std::find_if(m_aDriversBS.begin(),m_aDriversBS.end(),aEqual);
        if ( aFind == m_aDriversBS.end() )
        {
            // search all bootstrapped drivers
            aFind = ::std::find_if(
                m_aDriversBS.begin(),       // begin of search range
                m_aDriversBS.end(),         // end of search range
                o3tl::unary_compose< AcceptsURL, ExtractAfterLoad >( AcceptsURL( _rURL ), ExtractAfterLoad() )
                                            // compose two functors: extract the driver from the access, then ask the resulting driver for acceptance
            );
        } // if ( m_aDriversBS.find(sDriverFactoryName ) == m_aDriversBS.end() )
        else
        {
            EnsureDriver aEnsure;
            aEnsure(*aFind);
        }

        // found something?
        if ( m_aDriversBS.end() != aFind && aFind->xDriver.is() && aFind->xDriver->acceptsURL(_rURL) )
            xReturn = aFind->xDriver;
    }

    if ( !xReturn.is() )
    {
        // no -> search the runtime drivers
        DriverCollectionIterator aPos = ::std::find_if(
            m_aDriversRT.begin(),       // begin of search range
            m_aDriversRT.end(),         // end of search range
            o3tl::unary_compose< AcceptsURL, ExtractDriverFromCollectionElement >( AcceptsURL( _rURL ), ExtractDriverFromCollectionElement() )
                                        // compose two functors: extract the driver from the access, then ask the resulting driver for acceptance
        );

        if ( m_aDriversRT.end() != aPos )
            xReturn = aPos->second;
    }

    return xReturn;
}

}   // namespace drivermanager

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
