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


#include "seinitializer_mscryptimpl.hxx"

#include "securityenvironment_mscryptimpl.hxx"

#include "xmlsecurity/xmlsec-wrapper.h"
#include "xmlsec/mscrypto/app.h"
#include <com/sun/star/xml/crypto/SecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XMLSecurityContext.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;

SEInitializer_MSCryptImpl::SEInitializer_MSCryptImpl(
    const cssu::Reference< cssu::XComponentContext > &rxContext)
    :mxContext( rxContext )
{
}

SEInitializer_MSCryptImpl::~SEInitializer_MSCryptImpl()
{
}

/* XSEInitializer */
cssu::Reference< cssxc::XXMLSecurityContext > SAL_CALL
    SEInitializer_MSCryptImpl::createSecurityContext(
    const OUString& sCertDB )
    throw (cssu::RuntimeException)
{
    const char* n_pCertStore ;
    HCERTSTORE  n_hStoreHandle ;

    //Initialize the crypto engine
    if( sCertDB.getLength() > 0 )
    {
        OString sCertDir(sCertDB.getStr(), sCertDB.getLength(), RTL_TEXTENCODING_ASCII_US);
        n_pCertStore = sCertDir.getStr();
        n_hStoreHandle = CertOpenSystemStore( NULL, n_pCertStore ) ;
        if( n_hStoreHandle == NULL )
        {
            return NULL;
        }
    }
    else
    {
        n_pCertStore = NULL ;
        n_hStoreHandle = NULL ;
    }

    xmlSecMSCryptoAppInit( n_pCertStore ) ;

    try {
        /* Build Security Environment */
        cssu::Reference< cssxc::XSecurityEnvironment > xSecEnv = cssxc::SecurityEnvironment::create( mxContext );

        /* Setup key slot and certDb */
        cssu::Reference< cssl::XUnoTunnel > xSecEnvTunnel( xSecEnv, cssu::UNO_QUERY_THROW );
        SecurityEnvironment_MSCryptImpl* pSecEnv = ( SecurityEnvironment_MSCryptImpl* )xSecEnvTunnel->getSomething( SecurityEnvironment_MSCryptImpl::getUnoTunnelId() ) ;
        if( pSecEnv == NULL )
        {
            if( n_hStoreHandle != NULL )
            {
                CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;
            }

            xmlSecMSCryptoAppShutdown() ;
            return NULL;
        }

        if( n_hStoreHandle != NULL )
        {
            pSecEnv->setCryptoSlot( n_hStoreHandle ) ;
            pSecEnv->setCertDb( n_hStoreHandle ) ;
        }
        else
        {
            pSecEnv->enableDefaultCrypt( sal_True ) ;
        }

        /* Build XML Security Context */
        cssu::Reference< cssxc::XXMLSecurityContext > xSecCtx = cssxc::XMLSecurityContext::create( mxContext );

        xSecCtx->setDefaultSecurityEnvironmentIndex(xSecCtx->addSecurityEnvironment( xSecEnv )) ;
        return xSecCtx;
    }
    catch( cssu::Exception& )
    {
        if( n_hStoreHandle != NULL )
        {
            CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        }

        xmlSecMSCryptoAppShutdown() ;
        return NULL;
    }
}

void SAL_CALL SEInitializer_MSCryptImpl::freeSecurityContext( const cssu::Reference< cssxc::XXMLSecurityContext >&)
    throw (cssu::RuntimeException)
{
    /*
    cssu::Reference< cssxc::XSecurityEnvironment > xSecEnv
        = securityContext->getSecurityEnvironment();

    if( xSecEnv.is() )
    {
        cssu::Reference< cssl::XUnoTunnel > xEnvTunnel( xSecEnv , cssu::UNO_QUERY ) ;
        if( xEnvTunnel.is() )
        {
            SecurityEnvironment_MSCryptImpl* pSecEnv = ( SecurityEnvironment_MSCryptImpl* )xEnvTunnel->getSomething( SecurityEnvironment_MSCryptImpl::getUnoTunnelId() ) ;
            HCERTSTORE n_hStoreHandle = pSecEnv->getCryptoSlot();

            if( n_hStoreHandle != NULL )
            {
                CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;
                pSecEnv->setCryptoSlot( NULL ) ;
                pSecEnv->setCertDb( NULL ) ;
            }

            xmlSecMSCryptoAppShutdown() ;
        }
    }
    */

    xmlSecMSCryptoAppShutdown() ;
}

OUString SEInitializer_MSCryptImpl_getImplementationName()
    throw (cssu::RuntimeException)
{
    return OUString( "com.sun.star.xml.security.bridge.xmlsec.SEInitializer_MSCryptImpl" );
}

cssu::Sequence< OUString > SAL_CALL SEInitializer_MSCryptImpl_getSupportedServiceNames()
    throw (cssu::RuntimeException)
{
    cssu::Sequence<OUString> aRet { "com.sun.star.xml.crypto.SEInitializer" };
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL SEInitializer_MSCryptImpl_createInstance( const cssu::Reference< cssl::XMultiServiceFactory > & rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new SEInitializer_MSCryptImpl( comphelper::getComponentContext(rSMgr) );
}

/* XServiceInfo */
OUString SAL_CALL SEInitializer_MSCryptImpl::getImplementationName()
    throw (cssu::RuntimeException)
{
    return SEInitializer_MSCryptImpl_getImplementationName();
}

sal_Bool SAL_CALL SEInitializer_MSCryptImpl::supportsService( const OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return cppu::supportsService( this, rServiceName );
}

cssu::Sequence< OUString > SAL_CALL SEInitializer_MSCryptImpl::getSupportedServiceNames()
    throw (cssu::RuntimeException)
{
    return SEInitializer_MSCryptImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
