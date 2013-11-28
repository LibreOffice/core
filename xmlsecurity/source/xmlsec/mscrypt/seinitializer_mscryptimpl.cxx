/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        rtl::OString sCertDir( OUStringToOString( sCertDB, RTL_TEXTENCODING_ASCII_US));
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

