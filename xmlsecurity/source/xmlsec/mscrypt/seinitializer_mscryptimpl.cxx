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
#include "precompiled_xmlsecurity.hxx"

#include "seinitializer_mscryptimpl.hxx"

#include "securityenvironment_mscryptimpl.hxx"

#include "xmlsec/strings.h"
#include "xmlsec/mscrypto/app.h"

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;

#define SERVICE_NAME "com.sun.star.xml.crypto.SEInitializer"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.SEInitializer_MSCryptImpl"
#define SECURITY_ENVIRONMENT "com.sun.star.xml.crypto.SecurityEnvironment"
#define SECURITY_CONTEXT "com.sun.star.xml.crypto.XMLSecurityContext"

SEInitializer_MSCryptImpl::SEInitializer_MSCryptImpl(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > &rxMSF)
    :mxMSF( rxMSF )
{
}

SEInitializer_MSCryptImpl::~SEInitializer_MSCryptImpl()
{
}

/* XSEInitializer */
cssu::Reference< cssxc::XXMLSecurityContext > SAL_CALL
    SEInitializer_MSCryptImpl::createSecurityContext(
    const rtl::OUString& sCertDB )
    throw (cssu::RuntimeException)
{
    const char* n_pCertStore ;
    HCERTSTORE  n_hStoreHandle ;

    //Initialize the crypto engine
    if( sCertDB.getLength() > 0 )
    {
        rtl::OString sCertDir(sCertDB, sCertDB.getLength(), RTL_TEXTENCODING_ASCII_US);
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
        const rtl::OUString sSecyrutyEnvironment ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_ENVIRONMENT ) );
        cssu::Reference< cssxc::XSecurityEnvironment > xSecEnv( mxMSF->createInstance ( sSecyrutyEnvironment ), cssu::UNO_QUERY );
        if( !xSecEnv.is() )
        {
            if( n_hStoreHandle != NULL )
            {
                CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;
            }

            xmlSecMSCryptoAppShutdown() ;
            return NULL;
        }

        /* Setup key slot and certDb */
        cssu::Reference< cssl::XUnoTunnel > xEnvTunnel( xSecEnv , cssu::UNO_QUERY ) ;
        if( !xEnvTunnel.is() )
        {
            if( n_hStoreHandle != NULL )
            {
                CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;
            }

            xmlSecMSCryptoAppShutdown() ;
            return NULL;
        }

        SecurityEnvironment_MSCryptImpl* pSecEnv = ( SecurityEnvironment_MSCryptImpl* )xEnvTunnel->getSomething( SecurityEnvironment_MSCryptImpl::getUnoTunnelId() ) ;
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
        const rtl::OUString sSecyrutyContext ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_CONTEXT ) );
        cssu::Reference< cssxc::XXMLSecurityContext > xSecCtx( mxMSF->createInstance ( sSecyrutyContext ), cssu::UNO_QUERY );
        if( !xSecCtx.is() )
        {
            if( n_hStoreHandle != NULL )
            {
                CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;
            }

            xmlSecMSCryptoAppShutdown() ;
            return NULL;
        }

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

rtl::OUString SEInitializer_MSCryptImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL SEInitializer_MSCryptImpl_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL SEInitializer_MSCryptImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

cssu::Reference< cssu::XInterface > SAL_CALL SEInitializer_MSCryptImpl_createInstance( const cssu::Reference< cssl::XMultiServiceFactory > & rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new SEInitializer_MSCryptImpl(rSMgr);
}

/* XServiceInfo */
rtl::OUString SAL_CALL SEInitializer_MSCryptImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return SEInitializer_MSCryptImpl_getImplementationName();
}
sal_Bool SAL_CALL SEInitializer_MSCryptImpl::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return SEInitializer_MSCryptImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL SEInitializer_MSCryptImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return SEInitializer_MSCryptImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
