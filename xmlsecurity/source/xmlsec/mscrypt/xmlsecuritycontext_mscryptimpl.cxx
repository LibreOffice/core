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
#include <sal/config.h>
#include <rtl/uuid.h>
#include "securityenvironment_mscryptimpl.hxx"

#include "xmlsecuritycontext_mscryptimpl.hxx"
#include "xmlstreamio.hxx"

#include "xmlsec/xmlsec.h"
#include "xmlsec/keysmngr.h"
#include "xmlsec/crypto.h"
#include "xmlsec/mscrypto/akmngr.h"

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;

XMLSecurityContext_MSCryptImpl :: XMLSecurityContext_MSCryptImpl( const Reference< XMultiServiceFactory >& aFactory )
    ://m_pKeysMngr( NULL ) ,
     m_xServiceManager( aFactory ),
     m_xSecurityEnvironment( NULL )
{
    //Init xmlsec library
    if( xmlSecInit() < 0 ) {
        throw RuntimeException() ;
    }

    //Init xmlsec crypto engine library
    if( xmlSecCryptoInit() < 0 ) {
        xmlSecShutdown() ;
        throw RuntimeException() ;
    }

    //Enable external stream handlers
    if( xmlEnableStreamInputCallbacks() < 0 ) {
        xmlSecCryptoShutdown() ;
        xmlSecShutdown() ;
        throw RuntimeException() ;
    }
}

XMLSecurityContext_MSCryptImpl :: ~XMLSecurityContext_MSCryptImpl() {
    xmlDisableStreamInputCallbacks() ;
    xmlSecCryptoShutdown() ;
    xmlSecShutdown() ;
}

//i39448 : new methods
sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::addSecurityEnvironment(
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment)
    throw (::com::sun::star::security::SecurityInfrastructureException, ::com::sun::star::uno::RuntimeException)
{
    if( !aSecurityEnvironment.is() )
    {
        throw RuntimeException() ;
    }

    m_xSecurityEnvironment = aSecurityEnvironment;

    return 0;
}


sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::getSecurityEnvironmentNumber(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return 1;
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_MSCryptImpl::getSecurityEnvironmentByIndex( sal_Int32 index )
    throw (::com::sun::star::uno::RuntimeException)
{
    if (index == 0)
    {
        return m_xSecurityEnvironment;
    }
    else
        throw RuntimeException() ;
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_MSCryptImpl::getSecurityEnvironment(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return m_xSecurityEnvironment;
}

sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::getDefaultSecurityEnvironmentIndex(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}

void SAL_CALL XMLSecurityContext_MSCryptImpl::setDefaultSecurityEnvironmentIndex( sal_Int32 /*nDefaultEnvIndex*/ )
    throw (::com::sun::star::uno::RuntimeException)
{
    //dummy
}

#if 0
/* XXMLSecurityContext */
void SAL_CALL XMLSecurityContext_MSCryptImpl :: setSecurityEnvironment( const Reference< XSecurityEnvironment >& aSecurityEnvironment ) throw( com::sun::star::security::SecurityInfrastructureException ) {
    HCERTSTORE hkeyStore ;
    HCERTSTORE hCertStore ;
    HCRYPTKEY symKey ;
    HCRYPTKEY pubKey ;
    HCRYPTKEY priKey ;
    unsigned int i ;

    if( !aSecurityEnvironment.is() )
        throw RuntimeException() ;

    m_xSecurityEnvironment = aSecurityEnvironment ;

    //Clear key manager
    if( m_pKeysMngr != NULL ) {
        xmlSecKeysMngrDestroy( m_pKeysMngr ) ;
        m_pKeysMngr = NULL ;
    }

    //Create key manager
    Reference< XUnoTunnel > xEnvTunnel( m_xSecurityEnvironment , UNO_QUERY ) ;
    if( !xEnvTunnel.is() ) {
        throw RuntimeException() ;
    }

    SecurityEnvironment_MSCryptImpl* pSecEnv = ( SecurityEnvironment_MSCryptImpl* )xEnvTunnel->getSomething( SecurityEnvironment_MSCryptImpl::getUnoTunnelId() ) ;
    if( pSecEnv == NULL )
        throw RuntimeException() ;

    hkeyStore = pSecEnv->getCryptoSlot() ;
    hCertStore = pSecEnv->getCertDb() ;

    /*-
     * The following lines is based on the of xmlsec-mscrypto crypto engine
     */
    m_pKeysMngr = xmlSecMSCryptoAppliedKeysMngrCreate( hkeyStore , hCertStore ) ;
    if( m_pKeysMngr == NULL )
        throw RuntimeException() ;

    /*-
     * Adopt symmetric key into keys manager
     */
    for( i = 0 ; ( symKey = pSecEnv->getSymKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecMSCryptoAppliedKeysMngrSymKeyLoad( m_pKeysMngr, symKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }

    /*-
     * Adopt asymmetric public key into keys manager
     */
    for( i = 0 ; ( pubKey = pSecEnv->getPubKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecMSCryptoAppliedKeysMngrPubKeyLoad( m_pKeysMngr, pubKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }

    /*-
     * Adopt asymmetric private key into keys manager
     */
    for( i = 0 ; ( priKey = pSecEnv->getPriKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecMSCryptoAppliedKeysMngrPriKeyLoad( m_pKeysMngr, priKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }

    /*-
     * Adopt system default certificate store.
     */
    if( pSecEnv->defaultEnabled() ) {
        HCERTSTORE hSystemStore ;

        //Add system key store into the keys manager.
        hSystemStore = CertOpenSystemStore( 0, "MY" ) ;
        if( hSystemStore != NULL ) {
            if( xmlSecMSCryptoAppliedKeysMngrAdoptKeyStore( m_pKeysMngr, hSystemStore ) < 0 ) {
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                throw RuntimeException() ;
            }
        }

        //Add system root store into the keys manager.
        hSystemStore = CertOpenSystemStore( 0, "Root" ) ;
        if( hSystemStore != NULL ) {
            if( xmlSecMSCryptoAppliedKeysMngrAdoptTrustedStore( m_pKeysMngr, hSystemStore ) < 0 ) {
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                throw RuntimeException() ;
            }
        }

        //Add system trusted store into the keys manager.
        hSystemStore = CertOpenSystemStore( 0, "Trust" ) ;
        if( hSystemStore != NULL ) {
            if( xmlSecMSCryptoAppliedKeysMngrAdoptUntrustedStore( m_pKeysMngr, hSystemStore ) < 0 ) {
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                throw RuntimeException() ;
            }
        }

        //Add system CA store into the keys manager.
        hSystemStore = CertOpenSystemStore( 0, "CA" ) ;
        if( hSystemStore != NULL ) {
            if( xmlSecMSCryptoAppliedKeysMngrAdoptUntrustedStore( m_pKeysMngr, hSystemStore ) < 0 ) {
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                throw RuntimeException() ;
            }
        }
    }
}

/* XXMLSecurityContext */
Reference< XSecurityEnvironment > SAL_CALL XMLSecurityContext_MSCryptImpl :: getSecurityEnvironment()
    throw (RuntimeException)
{
    return  m_xSecurityEnvironment ;
}
#endif

/* XInitialization */
void SAL_CALL XMLSecurityContext_MSCryptImpl :: initialize( const Sequence< Any >& /*aArguments*/ ) throw( Exception, RuntimeException ) {
    // TBD
} ;

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContext_MSCryptImpl :: getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContext_MSCryptImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSecurityContext_MSCryptImpl :: getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSecurityContext_MSCryptImpl :: impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.xml.crypto.XMLSecurityContext" ) ;
    return seqServiceNames ;
}

OUString XMLSecurityContext_MSCryptImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString::createFromAscii( "com.sun.star.xml.security.bridge.xmlsec.XMLSecurityContext_MSCryptImpl" ) ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSecurityContext_MSCryptImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLSecurityContext_MSCryptImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLSecurityContext_MSCryptImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

#if 0
/* XUnoTunnel */
sal_Int64 SAL_CALL XMLSecurityContext_MSCryptImpl :: getSomething( const Sequence< sal_Int8 >& aIdentifier )
throw (RuntimeException)
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return ( sal_Int64 )this ;
    }
    return 0 ;
}

/* XUnoTunnel extension */
const Sequence< sal_Int8>& XMLSecurityContext_MSCryptImpl :: getUnoTunnelId() {
    static Sequence< sal_Int8 >* pSeq = 0 ;
    if( !pSeq ) {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
        if( !pSeq ) {
            static Sequence< sal_Int8> aSeq( 16 ) ;
            rtl_createUuid( ( sal_uInt8* )aSeq.getArray() , 0 , sal_True ) ;
            pSeq = &aSeq ;
        }
    }
    return *pSeq ;
}

/* XUnoTunnel extension */
XMLSecurityContext_MSCryptImpl* XMLSecurityContext_MSCryptImpl :: getImplementation( const Reference< XInterface > xObj ) {
    Reference< XUnoTunnel > xUT( xObj , UNO_QUERY ) ;
    if( xUT.is() ) {
        return ( XMLSecurityContext_MSCryptImpl* )xUT->getSomething( getUnoTunnelId() ) ;
    } else
        return NULL ;
}

/* Native methods */
xmlSecKeysMngrPtr XMLSecurityContext_MSCryptImpl :: keysManager() throw( Exception, RuntimeException ) {
    return m_pKeysMngr ;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
