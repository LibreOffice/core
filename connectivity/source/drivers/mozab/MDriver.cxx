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
#include "precompiled_connectivity.hxx"
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
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MOZAB_DRIVER_IMPL_NAME));
        // this name is referenced in the configuration and in the mozab.xml
        // Please take care when changing it.
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > MozabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.Driver"));
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
        Reference<XInterface> xInstance = m_xMSFactory->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap")) );
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
                "$libname$", ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SVLIBRARY( "mozabdrv" )) )
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
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseDN"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Base DN."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MaxRowCount"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Records (max.)"))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("100"))
                ,Sequence< ::rtl::OUString >())
                );
        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }
    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    // if you have somthing special to say return it here :-)
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
        if ( aAddrbookURI.getLength() > 0 )
            aAddrbookScheme= aAddrbookURI;
        else if(url == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:")) )
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

    for ( size_t i=0; i < SAL_N_ELEMENTS( aSchemeMap ); ++i )
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
                OSL_FAIL( ::rtl::OString( "lcl_getFunctionFromModuleOrUnload: could not find the symbol " ) + ::rtl::OString( _pAsciiSymbolName ) );
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

    const ::rtl::OUString sModuleName(RTL_CONSTASCII_USTRINGPARAM(SVLIBRARY( "mozabdrv" )));

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
