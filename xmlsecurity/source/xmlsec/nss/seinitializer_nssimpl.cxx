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
#include "precompiled_xmlsecurity.hxx"

/*
 * Turn off DEBUG Assertions
 */
#ifdef _DEBUG
    #define _DEBUG_WAS_DEFINED _DEBUG
    #undef _DEBUG
#else
    #undef _DEBUG_WAS_DEFINED
#endif

/*
 * and turn off the additional virtual methods which are part of some interfaces when compiled
 * with debug
 */
#ifdef DEBUG
    #define DEBUG_WAS_DEFINED DEBUG
    #undef DEBUG
#else
    #undef DEBUG_WAS_DEFINED
#endif

#include <sal/types.h>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <tools/debug.hxx>
#include <rtl/logfile.hxx>

#include "seinitializer_nssimpl.hxx"
#include "securityenvironment_nssimpl.hxx"

#include <nspr.h>
#include <cert.h>
#include <nss.h>
#include <pk11pub.h>
#include <secmod.h>
#include <nssckbi.h>


namespace css = ::com::sun::star;
namespace cssu = css::uno;
namespace cssl = css::lang;
namespace cssxc = css::xml::crypto;

using namespace com::sun::star;
using ::rtl::OUString;
using ::rtl::OString;

#define SE_SERVICE_NAME "com.sun.star.xml.crypto.SEInitializer"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.SEInitializer_NssImpl"
#define SECURITY_ENVIRONMENT "com.sun.star.xml.crypto.SecurityEnvironment"
#define SECURITY_CONTEXT "com.sun.star.xml.crypto.XMLSecurityContext"

SEInitializer_NssImpl::SEInitializer_NssImpl(
    const css::uno::Reference< css::lang::XMultiServiceFactory > &rxMSF )
{
    mxMSF = rxMSF;
}

SEInitializer_NssImpl::~SEInitializer_NssImpl()
{
}

/* XSEInitializer */
cssu::Reference< cssxc::XXMLSecurityContext > SAL_CALL
    SEInitializer_NssImpl::createSecurityContext( const ::rtl::OUString& )
    throw (cssu::RuntimeException)
{
    CERTCertDBHandle    *pCertHandle = NULL ;

    if( !initNSS( mxMSF ) )
        return NULL;

    pCertHandle = CERT_GetDefaultCertDB() ;

    try
    {
        /* Build XML Security Context */
        const rtl::OUString sSecyrutyContext ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_CONTEXT ) );
        cssu::Reference< cssxc::XXMLSecurityContext > xSecCtx( mxMSF->createInstance ( sSecyrutyContext ), cssu::UNO_QUERY );
        if( !xSecCtx.is() )
            return NULL;

        const rtl::OUString sSecyrutyEnvironment ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_ENVIRONMENT ) );
        cssu::Reference< cssxc::XSecurityEnvironment > xSecEnv( mxMSF->createInstance ( sSecyrutyEnvironment ), cssu::UNO_QUERY );
        cssu::Reference< cssl::XUnoTunnel > xEnvTunnel( xSecEnv , cssu::UNO_QUERY ) ;
        if( !xEnvTunnel.is() )
            return NULL;
        SecurityEnvironment_NssImpl* pSecEnv = reinterpret_cast<SecurityEnvironment_NssImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xEnvTunnel->getSomething(SecurityEnvironment_NssImpl::getUnoTunnelId() ))) ;
        pSecEnv->setCertDb(pCertHandle);

        sal_Int32 n = xSecCtx->addSecurityEnvironment(xSecEnv);
        //originally the SecurityEnvironment with the internal slot was set as default
        xSecCtx->setDefaultSecurityEnvironmentIndex( n );
        return xSecCtx;
    }
    catch( cssu::Exception& )
    {
        //PK11_LogoutAll();
        //NSS_Shutdown();
        return NULL;
    }
}

void SAL_CALL SEInitializer_NssImpl::freeSecurityContext( const cssu::Reference< cssxc::XXMLSecurityContext >& )
    throw (cssu::RuntimeException)
{
    /*
     * because the security context will free all its content when it
     * is destructed, so here no free process for the security context
     * is needed.
     */
    //PK11_LogoutAll();
    //NSS_Shutdown();
}

rtl::OUString SEInitializer_NssImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{

    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL SEInitializer_NssImpl_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SE_SERVICE_NAME )) || ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( NSS_SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SE_SERVICE_NAME ) );
    pArray[1] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( NSS_SERVICE_NAME ) );
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL SEInitializer_NssImpl_createInstance( const cssu::Reference< cssl::XMultiServiceFactory > & rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new SEInitializer_NssImpl(rSMgr);
}

/* XServiceInfo */
rtl::OUString SAL_CALL SEInitializer_NssImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_getImplementationName();
}
sal_Bool SAL_CALL SEInitializer_NssImpl::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_getSupportedServiceNames();
}

