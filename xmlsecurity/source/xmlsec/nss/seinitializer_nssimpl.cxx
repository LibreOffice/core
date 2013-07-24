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
#include <com/sun/star/xml/crypto/SecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XMLSecurityContext.hpp>

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

#define SE_SERVICE_NAME "com.sun.star.xml.crypto.SEInitializer"

SEInitializer_NssImpl::SEInitializer_NssImpl( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
{
    m_xContext = rxContext;
}

SEInitializer_NssImpl::~SEInitializer_NssImpl()
{
}

/* XSEInitializer */
uno::Reference< cssxc::XXMLSecurityContext > SAL_CALL
    SEInitializer_NssImpl::createSecurityContext( const OUString& )
    throw (uno::RuntimeException)
{
    CERTCertDBHandle    *pCertHandle = NULL ;

    if( !initNSS( m_xContext ) )
        return NULL;

    pCertHandle = CERT_GetDefaultCertDB() ;

    try
    {
        /* Build XML Security Context */
        uno::Reference< cssxc::XXMLSecurityContext > xSecCtx = cssxc::XMLSecurityContext::create( m_xContext );

        uno::Reference< cssxc::XSecurityEnvironment > xSecEnv = cssxc::SecurityEnvironment::create( m_xContext );
        uno::Reference< lang::XUnoTunnel > xSecEnvTunnel(xSecEnv, uno::UNO_QUERY_THROW);
        SecurityEnvironment_NssImpl* pSecEnv = reinterpret_cast<SecurityEnvironment_NssImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xSecEnvTunnel->getSomething(SecurityEnvironment_NssImpl::getUnoTunnelId() ))) ;
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

OUString SEInitializer_NssImpl_getImplementationName ()
    throw (uno::RuntimeException)
{
    return OUString ("com.sun.star.xml.security.bridge.xmlsec.SEInitializer_NssImpl" );
}

sal_Bool SAL_CALL SEInitializer_NssImpl_supportsService( const OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ( ServiceName == SE_SERVICE_NAME || ServiceName == NSS_SERVICE_NAME );
}

uno::Sequence< OUString > SAL_CALL SEInitializer_NssImpl_getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( SE_SERVICE_NAME );
    pArray[1] =  OUString ( NSS_SERVICE_NAME );
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL SEInitializer_NssImpl_createInstance( const uno::Reference< lang::XMultiServiceFactory > & rxMSF)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*) new SEInitializer_NssImpl(comphelper::getComponentContext(rxMSF));
}

/* XServiceInfo */
OUString SAL_CALL SEInitializer_NssImpl::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return SEInitializer_NssImpl_getImplementationName();
}
sal_Bool SAL_CALL SEInitializer_NssImpl::supportsService( const OUString& rServiceName )
    throw (uno::RuntimeException)
{
    return SEInitializer_NssImpl_supportsService( rServiceName );
}
uno::Sequence< OUString > SAL_CALL SEInitializer_NssImpl::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    return SEInitializer_NssImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
