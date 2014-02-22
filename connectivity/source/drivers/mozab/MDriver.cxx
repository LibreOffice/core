/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/macros.h>
#include "MDriver.hxx"
#include "MConnection.hxx"
#include "connectivity/dbexception.hxx"
#include <cppuhelper/supportsservice.hxx>
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
        
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL MozabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
        {
            return *(new MozabDriver( _rxFactory ));
        }
    }
}

MozabDriver::MozabDriver(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex), m_xMSFactory( _rxFactory )
    ,m_hModule(NULL)
    ,m_pCreationFunc(NULL)
{
}

MozabDriver::~MozabDriver()
{
}

void MozabDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();
    connectivity::OWeakRefArray().swap(m_xConnections); 

    ODriver_BASE::disposing();
    if(m_hModule)
    {
        m_pCreationFunc = NULL;
        osl_unloadModule(m_hModule);
        m_hModule = NULL;
    }
}



OUString MozabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString(MOZAB_DRIVER_IMPL_NAME);
        
        
}

Sequence< OUString > MozabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    
    
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = "com.sun.star.sdbc.Driver";
    return aSNS;
}


OUString SAL_CALL MozabDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL MozabDriver::supportsService( const OUString& _rServiceName ) throw(RuntimeException)
{
    return cppu::supportsService(this, _rServiceName);
}


Sequence< OUString > SAL_CALL MozabDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}


Reference< XConnection > SAL_CALL MozabDriver::connect( const OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( !ensureInit() )
        return NULL;

    if ( ! acceptsURL( url ) )
        return NULL;
    
    Reference< XConnection > xCon;
    if (m_pCreationFunc)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        
        Reference<XInterface> xInstance = m_xMSFactory->createInstance("com.sun.star.mozilla.MozillaBootstrap");
        OSL_ENSURE( xInstance.is(), "failed to create instance" );

        OConnection* pCon = reinterpret_cast<OConnection*>((*m_pCreationFunc)(this));
        xCon = pCon;    
        pCon->construct(url,info);              
        m_xConnections.push_back(WeakReferenceHelper(*pCon));

    }
    else
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_LIB,
                "$libname$", OUString( SVLIBRARY( "mozabdrv" ) )
             ) );

        ::dbtools::throwGenericSQLException(sError,*this);
    }

    return xCon;
}

sal_Bool SAL_CALL MozabDriver::acceptsURL( const OUString& url )
        throw(SQLException, RuntimeException)
{
    if ( !ensureInit() )
        return sal_False;

    
    return impl_classifyURL(url) != Unknown;
}

Sequence< DriverPropertyInfo > SAL_CALL MozabDriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    if ( !ensureInit() )
        return Sequence< DriverPropertyInfo >();

    if ( acceptsURL(url) )
    {
        if ( impl_classifyURL(url) != LDAP )
            return Sequence< DriverPropertyInfo >();

        ::std::vector< DriverPropertyInfo > aDriverInfo;
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("BaseDN")
                ,OUString("Base DN.")
                ,sal_False
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("MaxRowCount")
                ,OUString("Records (max.)")
                ,sal_False
                ,OUString("100")
                ,Sequence< OUString >())
                );
        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }
    ::connectivity::SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL MozabDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1; 
}

sal_Int32 SAL_CALL MozabDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0; 
}

EDriverType MozabDriver::impl_classifyURL( const OUString& url )
{
    
    //
    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    OUString aAddrbookURI(url.copy(nLen+1));
    
    nLen = aAddrbookURI.indexOf(':');
    OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        
        if ( !aAddrbookURI.isEmpty() )
            aAddrbookScheme= aAddrbookURI;
        else if(url == "sdbc:address:" )
            return Unknown; 
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
        if ( aAddrbookScheme.equalsAscii( aSchemeMap[i].pScheme ) )
            return aSchemeMap[i].eType;
    }

    return Unknown;
}


namespace
{
    template< typename FUNCTION >
    void lcl_getFunctionFromModuleOrUnload( oslModule& _rModule, const sal_Char* _pAsciiSymbolName, FUNCTION& _rFunction )
    {
        _rFunction = NULL;
        if ( _rModule )
        {
            const OUString sSymbolName = OUString::createFromAscii( _pAsciiSymbolName );
            _rFunction = (FUNCTION)( osl_getFunctionSymbol( _rModule, sSymbolName.pData ) );

            if ( !_rFunction )
            {   
                OUStringBuffer aBuf;
                aBuf.append( "lcl_getFunctionFromModuleOrUnload: could not find the symbol " );
                aBuf.append( sSymbolName );
                OSL_FAIL( aBuf.makeStringAndClear().getStr() );
                osl_unloadModule( _rModule );
                _rModule = NULL;
            }
        }
    }
}


extern "C" { static void SAL_CALL thisModule() {} }

bool MozabDriver::ensureInit()
{
    if ( m_hModule )
        return true;

    OSL_ENSURE(NULL == m_pCreationFunc, "MozabDriver::ensureInit: inconsistence: already have a factory function!");

    const OUString sModuleName(SVLIBRARY( "mozabdrv" ));

    
    m_hModule = osl_loadModuleRelative(&thisModule, sModuleName.pData, 0);
    OSL_ENSURE(NULL != m_hModule, "MozabDriver::ensureInit: could not load the mozabdrv library!");
    if ( !m_hModule )
        return false;

    OSetMozabServiceFactory pSetFactoryFunc( NULL );

    lcl_getFunctionFromModuleOrUnload( m_hModule, "setMozabServiceFactory",          pSetFactoryFunc   );
    lcl_getFunctionFromModuleOrUnload( m_hModule, "OMozabConnection_CreateInstance", m_pCreationFunc   );

    if ( !m_hModule )
        
        return false;

    if ( m_xMSFactory.is() )
    {
        
        
        m_xMSFactory->acquire();
        ( *pSetFactoryFunc )( m_xMSFactory.get() );
    }

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
