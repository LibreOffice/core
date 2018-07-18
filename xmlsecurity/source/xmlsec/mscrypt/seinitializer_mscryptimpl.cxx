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

#include <xmlsec-wrapper.h>
#include <xmlsec/mscng/app.h>
#include <com/sun/star/xml/crypto/SecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XMLSecurityContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <svl/cryptosign.hxx>

using namespace com::sun::star;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;

SEInitializer_MSCryptImpl::SEInitializer_MSCryptImpl(
    const uno::Reference< uno::XComponentContext > &rxContext)
    :mxContext( rxContext )
{
}

SEInitializer_MSCryptImpl::~SEInitializer_MSCryptImpl()
{
}

/* XSEInitializer */
uno::Reference< cssxc::XXMLSecurityContext > SAL_CALL
    SEInitializer_MSCryptImpl::createSecurityContext(
    const OUString& sCertDB )
{
    const char* n_pCertStore ;
    HCERTSTORE  n_hStoreHandle ;
    OString sCertDir;

    //Initialize the crypto engine
    if( sCertDB.getLength() > 0 )
    {
        sCertDir = OString(sCertDB.getStr(), sCertDB.getLength(), RTL_TEXTENCODING_ASCII_US);
        n_pCertStore = sCertDir.getStr();
        n_hStoreHandle = CertOpenSystemStoreW( NULL, o3tl::toW(sCertDB.getStr())) ;
        if( n_hStoreHandle == nullptr )
        {
            return nullptr;
        }
    }
    else
    {
        n_pCertStore = nullptr ;
        n_hStoreHandle = nullptr ;
    }

    xmlSecMSCngAppInit(n_pCertStore);

    try {
        /* Build Security Environment */
        uno::Reference< cssxc::XSecurityEnvironment > xSecEnv = cssxc::SecurityEnvironment::create( mxContext );

        /* Setup key slot and certDb */
        uno::Reference< cssl::XUnoTunnel > xSecEnvTunnel( xSecEnv, uno::UNO_QUERY_THROW );
        SecurityEnvironment_MSCryptImpl* pSecEnv = reinterpret_cast<SecurityEnvironment_MSCryptImpl*>(xSecEnvTunnel->getSomething( SecurityEnvironment_MSCryptImpl::getUnoTunnelId() ));
        if( pSecEnv == nullptr )
        {
            if( n_hStoreHandle != nullptr )
            {
                CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;
            }

            xmlSecMSCngAppShutdown();
            return nullptr;
        }

        if( n_hStoreHandle != nullptr )
        {
            pSecEnv->setCryptoSlot( n_hStoreHandle ) ;
            pSecEnv->setCertDb( n_hStoreHandle ) ;
        }
        else
        {
            pSecEnv->enableDefaultCrypt( true ) ;
        }

        /* Build XML Security Context */
        uno::Reference< cssxc::XXMLSecurityContext > xSecCtx = cssxc::XMLSecurityContext::create( mxContext );

        xSecCtx->setDefaultSecurityEnvironmentIndex(xSecCtx->addSecurityEnvironment( xSecEnv )) ;
        return xSecCtx;
    }
    catch( uno::Exception& )
    {
        if( n_hStoreHandle != nullptr )
        {
            CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        }

        xmlSecMSCngAppShutdown();
        return nullptr;
    }
}

void SAL_CALL SEInitializer_MSCryptImpl::freeSecurityContext( const uno::Reference< cssxc::XXMLSecurityContext >&)
{
    /*
    uno::Reference< cssxc::XSecurityEnvironment > xSecEnv
        = securityContext->getSecurityEnvironment();

    if( xSecEnv.is() )
    {
        uno::Reference< cssl::XUnoTunnel > xEnvTunnel( xSecEnv , uno::UNO_QUERY ) ;
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

    xmlSecMSCngAppShutdown();
}

/* XServiceInfo */
OUString SAL_CALL SEInitializer_MSCryptImpl::getImplementationName()
{
    return OUString("com.sun.star.xml.crypto.SEInitializer");
}

sal_Bool SAL_CALL SEInitializer_MSCryptImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

uno::Sequence< OUString > SAL_CALL SEInitializer_MSCryptImpl::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet { "com.sun.star.xml.crypto.SEInitializer" };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_SEInitializer_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SEInitializer_MSCryptImpl(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
