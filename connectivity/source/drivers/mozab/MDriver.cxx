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

#include <sal/macros.h>
#include "MDriver.hxx"
#include "MConnection.hxx"
#include "connectivity/dbexception.hxx"
#include "resource/mozab_res.hrc"
#include "resource/common_res.hrc"

#include <tools/solar.h>
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace connectivity::mozab;

namespace connectivity
{
    namespace mozab
    {
        //------------------------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL MozabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
        {
            return *(new MozabDriver( _rxFactory ));
        }
    }
}
// --------------------------------------------------------------------------------
MozabDriver::MozabDriver(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex), m_xMSFactory( _rxFactory )
    ,m_hModule(NULL)
    ,m_pCreationFunc(NULL)
{
}
// -----------------------------------------------------------------------------
MozabDriver::~MozabDriver()
{
}
// --------------------------------------------------------------------------------
void MozabDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroied so all our connections have to be destroied as well
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();
    connectivity::OWeakRefArray().swap(m_xConnections); // this really clears

    ODriver_BASE::disposing();
    if(m_hModule)
    {
        m_pCreationFunc = NULL;
        osl_unloadModule(m_hModule);
        m_hModule = NULL;
    }
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString MozabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString(MOZAB_DRIVER_IMPL_NAME);
        // this name is referenced in the configuration and in the mozab.xml
        // Please take care when changing it.
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > MozabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString( "com.sun.star.sdbc.Driver");
    return aSNS;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL MozabDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL MozabDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL MozabDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL MozabDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( !ensureInit() )
        return NULL;

    if ( ! acceptsURL( url ) )
        return NULL;
    // create a new connection with the given properties and append it to our vector
    Reference< XConnection > xCon;
    if (m_pCreationFunc)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        //We must make sure we create an com.sun.star.mozilla.MozillaBootstrap brfore call any mozilla codes
        Reference<XInterface> xInstance = m_xMSFactory->createInstance(::rtl::OUString( "com.sun.star.mozilla.MozillaBootstrap") );
        OSL_ENSURE( xInstance.is(), "failed to create instance" );

        OConnection* pCon = reinterpret_cast<OConnection*>((*m_pCreationFunc)(this));
        xCon = pCon;    // important here because otherwise the connection could be deleted inside (refcount goes -> 0)
        pCon->construct(url,info);              // late constructor call which can throw exception and allows a correct dtor call when so
        m_xConnections.push_back(WeakReferenceHelper(*pCon));

    }
    else
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_LIB,
                "$libname$", ::rtl::OUString( SVLIBRARY( "mozabdrv" ) )
             ) );

        ::dbtools::throwGenericSQLException(sError,*this);
    }

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL MozabDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    if ( !ensureInit() )
        return sal_False;

    // here we have to look if we support this url format
    return impl_classifyURL(url) != Unknown;
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL MozabDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    if ( !ensureInit() )
        return Sequence< DriverPropertyInfo >();

    if ( acceptsURL(url) )
    {
        if ( impl_classifyURL(url) != LDAP )
            return Sequence< DriverPropertyInfo >();

        ::std::vector< DriverPropertyInfo > aDriverInfo;
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("BaseDN")
                ,::rtl::OUString("Base DN.")
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("MaxRowCount")
                ,::rtl::OUString("Records (max.)")
                ,sal_False
                ,::rtl::OUString("100")
                ,Sequence< ::rtl::OUString >())
                );
        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }
    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    // if you have something special to say return it here :-)
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL MozabDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1; // depends on you
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL MozabDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0; // depends on you
}
// --------------------------------------------------------------------------------
EDriverType MozabDriver::impl_classifyURL( const ::rtl::OUString& url )
{
    // Skip 'sdbc:mozab: part of URL
    //
    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    ::rtl::OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        // There isn't any subschema: - but could be just subschema
        if ( !aAddrbookURI.isEmpty() )
            aAddrbookScheme= aAddrbookURI;
        else if(url == ::rtl::OUString("sdbc:address:") )
            return Unknown; // TODO check
        else
            return Unknown;
    }
    else
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);

    struct __scheme_map
    {
        EDriverType eType;
        const sal_Char* pScheme;
    } aSchemeMap[] =
    {
#if defined(WNT)
        { Outlook,          "outlook" },
        { OutlookExpress,   "outlookexp" },
#endif
        { Mozilla,          "mozilla" },
        { ThunderBird,      "thunderbird" },
        { LDAP,             "ldap" }
    };

    for ( size_t i=0; i < sizeof( aSchemeMap ) / sizeof( aSchemeMap[0] ); ++i )
    {
        if ( aAddrbookScheme.compareToAscii( aSchemeMap[i].pScheme ) == 0 )
            return aSchemeMap[i].eType;
    }

    return Unknown;
}

// --------------------------------------------------------------------------------
namespace
{
    template< typename FUNCTION >
    void lcl_getFunctionFromModuleOrUnload( oslModule& _rModule, const sal_Char* _pAsciiSymbolName, FUNCTION& _rFunction )
    {
        _rFunction = NULL;
        if ( _rModule )
        {
            const ::rtl::OUString sSymbolName = ::rtl::OUString::createFromAscii( _pAsciiSymbolName );
            _rFunction = (FUNCTION)( osl_getFunctionSymbol( _rModule, sSymbolName.pData ) );

            if ( !_rFunction )
            {   // did not find the symbol
                rtl::OUStringBuffer aBuf;
                aBuf.append( "lcl_getFunctionFromModuleOrUnload: could not find the symbol " );
                aBuf.append( sSymbolName );
                OSL_FAIL( aBuf.makeStringAndClear().getStr() );
                osl_unloadModule( _rModule );
                _rModule = NULL;
            }
        }
    }
}

// -----------------------------------------------------------------------------
extern "C" { static void SAL_CALL thisModule() {} }

bool MozabDriver::ensureInit()
{
    if ( m_hModule )
        return true;

    OSL_ENSURE(NULL == m_pCreationFunc, "MozabDriver::ensureInit: inconsistence: already have a factory function!");

    const ::rtl::OUString sModuleName(SVLIBRARY( "mozabdrv" ));

    // load the mozabdrv library
    m_hModule = osl_loadModuleRelative(&thisModule, sModuleName.pData, 0);
    OSL_ENSURE(NULL != m_hModule, "MozabDriver::ensureInit: could not load the mozabdrv library!");
    if ( !m_hModule )
        return false;

    OSetMozabServiceFactory pSetFactoryFunc( NULL );

    lcl_getFunctionFromModuleOrUnload( m_hModule, "setMozabServiceFactory",          pSetFactoryFunc   );
    lcl_getFunctionFromModuleOrUnload( m_hModule, "OMozabConnection_CreateInstance", m_pCreationFunc   );

    if ( !m_hModule )
        // one of the symbols did not exist
        return false;

    if ( m_xMSFactory.is() )
    {
        // for purpose of transfer safety, the interface needs to be acuired once
        // (will be release by the callee)
        m_xMSFactory->acquire();
        ( *pSetFactoryFunc )( m_xMSFactory.get() );
    }

    return true;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
