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


/*
 * Turn off DEBUG Assertions
 */
#ifdef _DEBUG
#undef _DEBUG
#endif

/*
 * and turn off the additional virtual methods which are part of some interfaces when compiled
 * with debug
 */
#ifdef DEBUG
#undef DEBUG
#endif

#include <comphelper/processfactory.hxx>
#include <sal/types.h>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/logfile.hxx>

#include "seinitializer_nssimpl.hxx"
#include "securityenvironment_nssimpl.hxx"

#include <nspr.h>
#include <cert.h>
#include <nss.h>
#include <pk11pub.h>
#include <secmod.h>
#include <nssckbi.h>


namespace cssl = css::lang;
namespace cssxc = css::xml::crypto;

using namespace com::sun::star;
using ::rtl::OUString;
using ::rtl::OString;

#define SE_SERVICE_NAME "com.sun.star.xml.crypto.SEInitializer"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.SEInitializer_NssImpl"
#define SECURITY_ENVIRONMENT "com.sun.star.xml.crypto.SecurityEnvironment"
#define SECURITY_CONTEXT "com.sun.star.xml.crypto.XMLSecurityContext"

SEInitializer_NssImpl::SEInitializer_NssImpl( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
{
    m_xContext = rxContext;
}

SEInitializer_NssImpl::~SEInitializer_NssImpl()
{
}

/* XSEInitializer */
uno::Reference< cssxc::XXMLSecurityContext > SAL_CALL
    SEInitializer_NssImpl::createSecurityContext( const ::rtl::OUString& )
    throw (uno::RuntimeException)
{
    CERTCertDBHandle    *pCertHandle = NULL ;

    if( !initNSS( m_xContext ) )
        return NULL;

    pCertHandle = CERT_GetDefaultCertDB() ;

    try
    {
        /* Build XML Security Context */
        const rtl::OUString sSecyrutyContext ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_CONTEXT ) );
        uno::Reference< cssxc::XXMLSecurityContext > xSecCtx( m_xContext->getServiceManager()->createInstanceWithContext(sSecyrutyContext, m_xContext), uno::UNO_QUERY );
        if( !xSecCtx.is() )
            return NULL;

        const rtl::OUString sSecyrutyEnvironment ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_ENVIRONMENT ) );
        uno::Reference< cssxc::XSecurityEnvironment > xSecEnv( m_xContext->getServiceManager()->createInstanceWithContext(sSecyrutyEnvironment, m_xContext), uno::UNO_QUERY );
        uno::Reference< cssl::XUnoTunnel > xEnvTunnel( xSecEnv , uno::UNO_QUERY ) ;
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
    catch( const uno::Exception& )
    {
        //PK11_LogoutAll();
        //NSS_Shutdown();
        return NULL;
    }
}

void SAL_CALL SEInitializer_NssImpl::freeSecurityContext( const uno::Reference< cssxc::XXMLSecurityContext >& )
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{

    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL SEInitializer_NssImpl_supportsService( const rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ( ServiceName == SE_SERVICE_NAME || ServiceName == NSS_SERVICE_NAME );
}

uno::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl_getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence < rtl::OUString > aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SE_SERVICE_NAME ) );
    pArray[1] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( NSS_SERVICE_NAME ) );
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL SEInitializer_NssImpl_createInstance( const uno::Reference< lang::XMultiServiceFactory > & rxMSF)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*) new SEInitializer_NssImpl(comphelper::getComponentContext(rxMSF));
}

/* XServiceInfo */
rtl::OUString SAL_CALL SEInitializer_NssImpl::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return SEInitializer_NssImpl_getImplementationName();
}
sal_Bool SAL_CALL SEInitializer_NssImpl::supportsService( const rtl::OUString& rServiceName )
    throw (uno::RuntimeException)
{
    return SEInitializer_NssImpl_supportsService( rServiceName );
}
uno::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    return SEInitializer_NssImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
