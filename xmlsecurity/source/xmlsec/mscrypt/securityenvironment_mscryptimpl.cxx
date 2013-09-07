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

#include <string.h>

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include "Windows.h"
#include "WinCrypt.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <sal/config.h>
#include <sal/macros.h>
#include <osl/thread.h>
#include "securityenvironment_mscryptimpl.hxx"

#include "x509certificate_mscryptimpl.hxx"
#include <comphelper/servicehelper.hxx>

#include "xmlsecurity/xmlsec-wrapper.h"
#include "xmlsec/mscrypto/akmngr.h"

#include <xmlsecurity/biginteger.hxx>

#include <rtl/locale.h>
#include <osl/nlsupport.h>
#include <osl/process.h>

#include "../diagnose.hxx"

using namespace xmlsecurity;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::security::XCertificate ;

extern X509Certificate_MSCryptImpl* MswcryCertContextToXCert( PCCERT_CONTEXT cert ) ;

struct CertErrorToString{
    DWORD error;
    char * name;
};

CertErrorToString arErrStrings[] =
{
    { 0x00000000, "CERT_TRUST_NO_ERROR"},
    { 0x00000001, "CERT_TRUST_IS_NOT_TIME_VALID"},
    { 0x00000002, "CERT_TRUST_IS_NOT_TIME_NESTED"},
    { 0x00000004, "CERT_TRUST_IS_REVOKED" },
    { 0x00000008, "CERT_TRUST_IS_NOT_SIGNATURE_VALID" },
    { 0x00000010, "CERT_TRUST_IS_NOT_SIGNATURE_VALID"},
    { 0x00000020, "CERT_TRUST_IS_UNTRUSTED_ROOT"},
    { 0x00000040, "CERT_TRUST_REVOCATION_STATUS_UNKNOWN"},
    { 0x00000080, "CERT_TRUST_IS_CYCLIC"},
    { 0x00000100, "CERT_TRUST_INVALID_EXTENSION"},
    { 0x00000200, "CERT_TRUST_INVALID_POLICY_CONSTRAINTS"},
    { 0x00000400, "CERT_TRUST_INVALID_BASIC_CONSTRAINTS"},
    { 0x00000800, "CERT_TRUST_INVALID_NAME_CONSTRAINTS"},
    { 0x00001000, "CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT"},
    { 0x00002000, "CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT"},
    { 0x00004000, "CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT"},
    { 0x00008000, "CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT"},
    { 0x01000000, "CERT_TRUST_IS_OFFLINE_REVOCATION"},
    { 0x02000000, "CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY"},
    { 0x04000000, "CERT_TRUST_IS_EXPLICIT_DISTRUST"},
    { 0x08000000, "CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT"},
    //Chain errors
    { 0x00010000, "CERT_TRUST_IS_PARTIAL_CHAIN"},
    { 0x00020000, "CERT_TRUST_CTL_IS_NOT_TIME_VALID"},
    { 0x00040000, "CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID"},
    { 0x00080000, "CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE"}
};

void traceTrustStatus(DWORD err)
{
    xmlsec_trace("The certificate error status is: ");
    if (err == 0)
        xmlsec_trace("%s", arErrStrings[0].name);
    for (int i = 1; i < SAL_N_ELEMENTS(arErrStrings); i++)
    {
        if (arErrStrings[i].error & err)
            xmlsec_trace("%s", arErrStrings[i].name);
    }
}

SecurityEnvironment_MSCryptImpl :: SecurityEnvironment_MSCryptImpl( const Reference< XMultiServiceFactory >& aFactory ) : m_hProv( NULL ) , m_pszContainer( NULL ) , m_hKeyStore( NULL ), m_hCertStore( NULL ), m_tSymKeyList() , m_tPubKeyList() , m_tPriKeyList(), m_xServiceManager( aFactory ), m_bEnableDefault( sal_False ), m_hMySystemStore(NULL), m_hRootSystemStore(NULL), m_hTrustSystemStore(NULL), m_hCaSystemStore(NULL){

}

SecurityEnvironment_MSCryptImpl :: ~SecurityEnvironment_MSCryptImpl() {

    if( m_hProv != NULL ) {
        CryptReleaseContext( m_hProv, 0 ) ;
        m_hProv = NULL ;
    }

    if( m_pszContainer != NULL ) {
        //TODO: Don't know whether or not it should be released now.
        m_pszContainer = NULL ;
    }

    if( m_hCertStore != NULL ) {
        CertCloseStore( m_hCertStore, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        m_hCertStore = NULL ;
    }

    if( m_hKeyStore != NULL ) {
        CertCloseStore( m_hKeyStore, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        m_hKeyStore = NULL ;
    }

    //i120675, close the store handles
    if( m_hMySystemStore != NULL ) {
        CertCloseStore( m_hMySystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        m_hMySystemStore = NULL ;
    }

    if( m_hRootSystemStore != NULL ) {
        CertCloseStore( m_hRootSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        m_hRootSystemStore = NULL ;
    }

    if( m_hTrustSystemStore != NULL ) {
        CertCloseStore( m_hTrustSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        m_hTrustSystemStore = NULL ;
    }

    if( m_hCaSystemStore != NULL ) {
        CertCloseStore( m_hCaSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        m_hCaSystemStore = NULL ;
    }

    if( !m_tSymKeyList.empty()  ) {
        std::list< HCRYPTKEY >::iterator symKeyIt ;

        for( symKeyIt = m_tSymKeyList.begin() ; symKeyIt != m_tSymKeyList.end() ; ++symKeyIt )
            CryptDestroyKey( *symKeyIt ) ;
    }

    if( !m_tPubKeyList.empty()  ) {
        std::list< HCRYPTKEY >::iterator pubKeyIt ;

        for( pubKeyIt = m_tPubKeyList.begin() ; pubKeyIt != m_tPubKeyList.end() ; ++pubKeyIt )
            CryptDestroyKey( *pubKeyIt ) ;
    }

    if( !m_tPriKeyList.empty()  ) {
        std::list< HCRYPTKEY >::iterator priKeyIt ;

        for( priKeyIt = m_tPriKeyList.begin() ; priKeyIt != m_tPriKeyList.end() ; ++priKeyIt )
            CryptDestroyKey( *priKeyIt ) ;
    }

}

/* XServiceInfo */
OUString SAL_CALL SecurityEnvironment_MSCryptImpl :: getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL SecurityEnvironment_MSCryptImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SecurityEnvironment_MSCryptImpl :: getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > SecurityEnvironment_MSCryptImpl :: impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames.getArray()[0] = OUString("com.sun.star.xml.crypto.SecurityEnvironment") ;
    return seqServiceNames ;
}

OUString SecurityEnvironment_MSCryptImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString("com.sun.star.xml.security.bridge.xmlsec.SecurityEnvironment_MSCryptImpl") ;
}

//Helper for registry
Reference< XInterface > SAL_CALL SecurityEnvironment_MSCryptImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new SecurityEnvironment_MSCryptImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > SecurityEnvironment_MSCryptImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL SecurityEnvironment_MSCryptImpl :: getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw( RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return ( sal_Int64 )this ;
    }
    return 0 ;
}

/* XUnoTunnel extension */


namespace
{
    class theSecurityEnvironment_MSCryptImplUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSecurityEnvironment_MSCryptImplUnoTunnelId > {};
}

const Sequence< sal_Int8>& SecurityEnvironment_MSCryptImpl :: getUnoTunnelId() {
    return theSecurityEnvironment_MSCryptImplUnoTunnelId::get().getSeq();
}

/* XUnoTunnel extension */
SecurityEnvironment_MSCryptImpl* SecurityEnvironment_MSCryptImpl :: getImplementation( const Reference< XInterface > xObj ) {
    Reference< XUnoTunnel > xUT( xObj , UNO_QUERY ) ;
    if( xUT.is() ) {
        return ( SecurityEnvironment_MSCryptImpl* )xUT->getSomething( getUnoTunnelId() ) ;
    } else
        return NULL ;
}

/* Native methods */
HCRYPTPROV SecurityEnvironment_MSCryptImpl :: getCryptoProvider() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) {
    return m_hProv ;
}

void SecurityEnvironment_MSCryptImpl :: setCryptoProvider( HCRYPTPROV aProv ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) {
    if( m_hProv != NULL ) {
        CryptReleaseContext( m_hProv, 0 ) ;
        m_hProv = NULL ;
    }

    if( aProv != NULL ) {
        m_hProv = aProv ;
    }
}

LPCTSTR SecurityEnvironment_MSCryptImpl :: getKeyContainer() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) {
    return m_pszContainer ;
}

void SecurityEnvironment_MSCryptImpl :: setKeyContainer( LPCTSTR aKeyContainer ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) {
    //TODO: Don't know whether or not it should be copied.
    m_pszContainer = aKeyContainer ;
}


HCERTSTORE SecurityEnvironment_MSCryptImpl :: getCryptoSlot() throw( Exception , RuntimeException ) {
    return m_hKeyStore ;
}

void SecurityEnvironment_MSCryptImpl :: setCryptoSlot( HCERTSTORE aSlot) throw( Exception , RuntimeException ) {
    if( m_hKeyStore != NULL ) {
        CertCloseStore( m_hKeyStore, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        m_hKeyStore = NULL ;
    }

    if( aSlot != NULL ) {
        m_hKeyStore = CertDuplicateStore( aSlot ) ;
    }
}

HCERTSTORE SecurityEnvironment_MSCryptImpl :: getCertDb() throw( Exception , RuntimeException ) {
    return m_hCertStore ;
}

void SecurityEnvironment_MSCryptImpl :: setCertDb( HCERTSTORE aCertDb ) throw( Exception , RuntimeException ) {
    if( m_hCertStore != NULL ) {
        CertCloseStore( m_hCertStore, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        m_hCertStore = NULL ;
    }

    if( aCertDb != NULL ) {
        m_hCertStore = CertDuplicateStore( aCertDb ) ;
    }
}

void SecurityEnvironment_MSCryptImpl :: adoptSymKey( HCRYPTKEY aSymKey ) throw( Exception , RuntimeException ) {
    HCRYPTKEY   symkey ;
    std::list< HCRYPTKEY >::iterator keyIt ;

    if( aSymKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tSymKeyList.begin() ; keyIt != m_tSymKeyList.end() ; ++keyIt ) {
            if( *keyIt == aSymKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        symkey = aSymKey ;

        try {
            m_tSymKeyList.push_back( symkey ) ;
        } catch ( Exception& ) {
            CryptDestroyKey( symkey ) ;
        }
    }
}

void SecurityEnvironment_MSCryptImpl :: rejectSymKey( HCRYPTKEY aSymKey ) throw( Exception , RuntimeException ) {
    HCRYPTKEY symkey ;
    std::list< HCRYPTKEY >::iterator keyIt ;

    if( aSymKey != NULL ) {
        for( keyIt = m_tSymKeyList.begin() ; keyIt != m_tSymKeyList.end() ; ++keyIt ) {
            if( *keyIt == aSymKey ) {
                symkey = *keyIt ;
                CryptDestroyKey( symkey ) ;
                m_tSymKeyList.erase( keyIt ) ;
                break ;
            }
        }
    }
}

HCRYPTKEY SecurityEnvironment_MSCryptImpl :: getSymKey( unsigned int position ) throw( Exception , RuntimeException ) {
    HCRYPTKEY symkey ;
    std::list< HCRYPTKEY >::iterator keyIt ;
    unsigned int pos ;

    symkey = NULL ;
    for( pos = 0, keyIt = m_tSymKeyList.begin() ; pos < position && keyIt != m_tSymKeyList.end() ; ++pos , ++keyIt ) ;

    if( pos == position && keyIt != m_tSymKeyList.end() )
        symkey = *keyIt ;

    return symkey ;
}

void SecurityEnvironment_MSCryptImpl :: adoptPubKey( HCRYPTKEY aPubKey ) throw( Exception , RuntimeException ) {
    HCRYPTKEY   pubkey ;
    std::list< HCRYPTKEY >::iterator keyIt ;

    if( aPubKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tPubKeyList.begin() ; keyIt != m_tPubKeyList.end() ; ++keyIt ) {
            if( *keyIt == aPubKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        pubkey = aPubKey ;

        try {
            m_tPubKeyList.push_back( pubkey ) ;
        } catch ( Exception& ) {
            CryptDestroyKey( pubkey ) ;
        }
    }
}

void SecurityEnvironment_MSCryptImpl :: rejectPubKey( HCRYPTKEY aPubKey ) throw( Exception , RuntimeException ) {
    HCRYPTKEY pubkey ;
    std::list< HCRYPTKEY >::iterator keyIt ;

    if( aPubKey != NULL ) {
        for( keyIt = m_tPubKeyList.begin() ; keyIt != m_tPubKeyList.end() ; ++keyIt ) {
            if( *keyIt == aPubKey ) {
                pubkey = *keyIt ;
                CryptDestroyKey( pubkey ) ;
                m_tPubKeyList.erase( keyIt ) ;
                break ;
            }
        }
    }
}

HCRYPTKEY SecurityEnvironment_MSCryptImpl :: getPubKey( unsigned int position ) throw( Exception , RuntimeException ) {
    HCRYPTKEY pubkey ;
    std::list< HCRYPTKEY >::iterator keyIt ;
    unsigned int pos ;

    pubkey = NULL ;
    for( pos = 0, keyIt = m_tPubKeyList.begin() ; pos < position && keyIt != m_tPubKeyList.end() ; ++pos , ++keyIt ) ;

    if( pos == position && keyIt != m_tPubKeyList.end() )
        pubkey = *keyIt ;

    return pubkey ;
}

void SecurityEnvironment_MSCryptImpl :: adoptPriKey( HCRYPTKEY aPriKey ) throw( Exception , RuntimeException ) {
    HCRYPTKEY   prikey ;
    std::list< HCRYPTKEY >::iterator keyIt ;

    if( aPriKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tPriKeyList.begin() ; keyIt != m_tPriKeyList.end() ; ++keyIt ) {
            if( *keyIt == aPriKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        prikey = aPriKey ;

        try {
            m_tPriKeyList.push_back( prikey ) ;
        } catch ( Exception& ) {
            CryptDestroyKey( prikey ) ;
        }
    }
}

void SecurityEnvironment_MSCryptImpl :: rejectPriKey( HCRYPTKEY aPriKey ) throw( Exception , RuntimeException ) {
    HCRYPTKEY   prikey ;
    std::list< HCRYPTKEY >::iterator keyIt ;

    if( aPriKey != NULL ) {
        for( keyIt = m_tPriKeyList.begin() ; keyIt != m_tPriKeyList.end() ; ++keyIt ) {
            if( *keyIt == aPriKey ) {
                prikey = *keyIt ;
                CryptDestroyKey( prikey ) ;
                m_tPriKeyList.erase( keyIt ) ;
                break ;
            }
        }
    }
}

HCRYPTKEY SecurityEnvironment_MSCryptImpl :: getPriKey( unsigned int position ) throw( Exception , RuntimeException ) {
    HCRYPTKEY prikey ;
    std::list< HCRYPTKEY >::iterator keyIt ;
    unsigned int pos ;

    prikey = NULL ;
    for( pos = 0, keyIt = m_tPriKeyList.begin() ; pos < position && keyIt != m_tPriKeyList.end() ; ++pos , ++keyIt ) ;

    if( pos == position && keyIt != m_tPriKeyList.end() )
        prikey = *keyIt ;

    return prikey ;
}

//Methods from XSecurityEnvironment
Sequence< Reference < XCertificate > > SecurityEnvironment_MSCryptImpl :: getPersonalCertificates() throw( SecurityException , RuntimeException )
{
    sal_Int32 length ;
    X509Certificate_MSCryptImpl* xcert ;
    std::list< X509Certificate_MSCryptImpl* > certsList ;
    PCCERT_CONTEXT pCertContext = NULL;

    //firstly, we try to find private keys in given key store.
    if( m_hKeyStore != NULL ) {
        pCertContext = CertEnumCertificatesInStore( m_hKeyStore, pCertContext );
        while (pCertContext)
        {
            xcert = MswcryCertContextToXCert( pCertContext ) ;
            if( xcert != NULL )
                certsList.push_back( xcert ) ;
            pCertContext = CertEnumCertificatesInStore( m_hKeyStore, pCertContext );
        }
    }

    //secondly, we try to find certificate from registered private keys.
    if( !m_tPriKeyList.empty()  ) {
        //TODO: Don't know whether or not it is necessary ans possible.
    }

    //Thirdly, we try to find certificate from system default key store.
    if( m_bEnableDefault ) {
        HCERTSTORE hSystemKeyStore ;
        DWORD      dwKeySpec;
        HCRYPTPROV hCryptProv;

        hSystemKeyStore = CertOpenSystemStore( 0, "MY" ) ;
        if( hSystemKeyStore != NULL ) {
            pCertContext = CertEnumCertificatesInStore( hSystemKeyStore, pCertContext );
            while (pCertContext)
            {
                // for checking whether the certificate is a personal certificate or not.
                if(!(CryptAcquireCertificatePrivateKey(pCertContext,
                        CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
                        NULL,
                        &hCryptProv,
                        &dwKeySpec,
                        NULL)))
                {
                    // Not Privatekey found. SKIP this one.
                    pCertContext = CertEnumCertificatesInStore( hSystemKeyStore, pCertContext );
                    continue;
                }
                // then TODO : Check the personal cert is valid or not.

                xcert = MswcryCertContextToXCert( pCertContext ) ;
                if( xcert != NULL )
                    certsList.push_back( xcert ) ;
                pCertContext = CertEnumCertificatesInStore( hSystemKeyStore, pCertContext );
            }
        }

        CertCloseStore( hSystemKeyStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
    }

    length = certsList.size() ;
    if( length != 0 ) {
        int i ;
        std::list< X509Certificate_MSCryptImpl* >::iterator xcertIt ;
        Sequence< Reference< XCertificate > > certSeq( length ) ;

        for( i = 0, xcertIt = certsList.begin(); xcertIt != certsList.end(); ++xcertIt, ++i ) {
            certSeq[i] = *xcertIt ;
        }

        return certSeq ;
    }

    return Sequence< Reference< XCertificate > >() ;
}


Reference< XCertificate > SecurityEnvironment_MSCryptImpl :: getCertificate( const OUString& issuerName, const Sequence< sal_Int8 >& serialNumber ) throw( SecurityException , RuntimeException ) {
    unsigned int i ;
    LPSTR   pszName ;
    X509Certificate_MSCryptImpl *xcert = NULL ;
    PCCERT_CONTEXT pCertContext = NULL ;
    HCERTSTORE hCertStore = NULL ;
    CRYPT_INTEGER_BLOB cryptSerialNumber ;
    CERT_INFO certInfo ;

    // for correct encoding
    sal_uInt16 encoding ;
    rtl_Locale *pLocale = NULL ;
    osl_getProcessLocale( &pLocale ) ;
    encoding = osl_getTextEncodingFromLocale( pLocale ) ;

    //Create cert info from issue and serial
    OString oissuer = OUStringToOString( issuerName , encoding ) ;
    pszName = ( char* )oissuer.getStr() ;

    if( ! ( CertStrToName(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
        pszName ,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG | CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG,
        NULL ,
        NULL ,
        &certInfo.Issuer.cbData, NULL ) )
    ) {
        return NULL ;
    }

    certInfo.Issuer.pbData = ( BYTE* )malloc( certInfo.Issuer.cbData );
    if(!certInfo.Issuer.pbData)
        throw RuntimeException() ;

    if( ! ( CertStrToName(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
        pszName ,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG | CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG,
        NULL ,
        ( BYTE* )certInfo.Issuer.pbData ,
        &certInfo.Issuer.cbData, NULL ) )
    ) {
        free( certInfo.Issuer.pbData ) ;
        return NULL ;
    }

    //Get the SerialNumber
    cryptSerialNumber.cbData = serialNumber.getLength() ;
    cryptSerialNumber.pbData = ( BYTE* )malloc( cryptSerialNumber.cbData);
    if (!cryptSerialNumber.pbData)
    {
        free( certInfo.Issuer.pbData ) ;
        throw RuntimeException() ;
    }
    for( i = 0; i < cryptSerialNumber.cbData; i ++ )
        cryptSerialNumber.pbData[i] = serialNumber[ cryptSerialNumber.cbData - i - 1 ] ;

    certInfo.SerialNumber.cbData = cryptSerialNumber.cbData ;
    certInfo.SerialNumber.pbData = cryptSerialNumber.pbData ;

    // Get the Cert from all store.
    for( i = 0 ; i < 6 ; i ++ )
    {
        switch(i)
        {
        case 0:
            if(m_hKeyStore == NULL) continue ;
            hCertStore = m_hKeyStore ;
            break;
        case 1:
            if(m_hCertStore == NULL) continue ;
            hCertStore = m_hCertStore ;
            break;
        case 2:
            hCertStore = CertOpenSystemStore( 0, "MY" ) ;
            if(hCertStore == NULL || !m_bEnableDefault) continue ;
            break;
        case 3:
            hCertStore = CertOpenSystemStore( 0, "Root" ) ;
            if(hCertStore == NULL || !m_bEnableDefault) continue ;
            break;
        case 4:
            hCertStore = CertOpenSystemStore( 0, "Trust" ) ;
            if(hCertStore == NULL || !m_bEnableDefault) continue ;
            break;
        case 5:
            hCertStore = CertOpenSystemStore( 0, "CA" ) ;
            if(hCertStore == NULL || !m_bEnableDefault) continue ;
            break;
        default:
            i=6;
            continue;
        }

/*******************************************************************************
 * This code reserved for remind us there are another way to find one cert by
 * IssuerName&serialnumber. You can use the code to replaced the function
 * CertFindCertificateInStore IF and ONLY IF you must find one special cert in
 * certStore but can not be found by CertFindCertificateInStore , then , you
 * should also change the same part in libxmlsec/.../src/mscrypto/x509vfy.c#875.
 * By Chandler Peng(chandler.peng@sun.com)
 *****/
/*******************************************************************************
        pCertContext = NULL ;
        found = 0;
        do{
            //  1. enum the certs has same string in the issuer string.
            pCertContext = CertEnumCertificatesInStore( hCertStore , pCertContext ) ;
            if( pCertContext != NULL )
            {
                // 2. check the cert's issuer name .
                char* issuer = NULL ;
                DWORD cbIssuer = 0 ;

                cbIssuer = CertNameToStr(
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                    &( pCertContext->pCertInfo->Issuer ),
                    CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
                    NULL, 0
                ) ;

                if( cbIssuer == 0 ) continue ; // discard this cert;

                issuer = (char *)malloc( cbIssuer ) ;
                if( issuer == NULL )  // discard this cert;
                {
                    free( cryptSerialNumber.pbData) ;
                    free( certInfo.Issuer.pbData ) ;
                    CertFreeCertificateContext( pCertContext ) ;
                    if(i != 0 && i != 1) CertCloseStore( hCertStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                    throw RuntimeException() ;
                }

                cbIssuer = CertNameToStr(
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                    &( pCertContext->pCertInfo->Issuer ),
                    CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
                    issuer, cbIssuer
                ) ;

                if( cbIssuer <= 0 )
                {
                    free( issuer ) ;
                    continue ;// discard this cert;
                }

                if(strncmp(pszName , issuer , cbIssuer) != 0)
                {
                    free( issuer ) ;
                    continue ;// discard this cert;
                }
                free( issuer ) ;

                // 3. check the serial number.
                if( memcmp( cryptSerialNumber.pbData , pCertContext->pCertInfo->SerialNumber.pbData  , cryptSerialNumber.cbData ) != 0 )
                {
                    continue ;// discard this cert;
                }

                // 4. confirm and break;
                found = 1;
                break ;
            }

        }while(pCertContext);

        if(i != 0 && i != 1) CertCloseStore( hCertStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        if( found != 0 ) break; // Found the certificate.
********************************************************************************/

        pCertContext = CertFindCertificateInStore(
            hCertStore,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,
            CERT_FIND_SUBJECT_CERT,
            &certInfo,
            NULL
        ) ;

        if(i != 0 && i != 1) CertCloseStore( hCertStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        if( pCertContext != NULL ) break ; // Found the certificate.

    }

    if( cryptSerialNumber.pbData ) free( cryptSerialNumber.pbData ) ;
    if( certInfo.Issuer.pbData ) free( certInfo.Issuer.pbData ) ;

    if( pCertContext != NULL ) {
        xcert = MswcryCertContextToXCert( pCertContext ) ;
        if( pCertContext ) CertFreeCertificateContext( pCertContext ) ;
    } else {
        xcert = NULL ;
    }

    return xcert ;
}

Reference< XCertificate > SecurityEnvironment_MSCryptImpl :: getCertificate( const OUString& issuerName, const OUString& serialNumber ) throw( SecurityException , RuntimeException ) {
    Sequence< sal_Int8 > serial = numericStringToBigInteger( serialNumber ) ;
    return getCertificate( issuerName, serial ) ;
}

Sequence< Reference < XCertificate > > SecurityEnvironment_MSCryptImpl :: buildCertificatePath( const Reference< XCertificate >& begin ) throw( SecurityException , RuntimeException ) {
    PCCERT_CHAIN_CONTEXT pChainContext ;
    PCCERT_CONTEXT pCertContext ;
    const X509Certificate_MSCryptImpl* xcert ;

    CERT_ENHKEY_USAGE   enhKeyUsage ;
    CERT_USAGE_MATCH    certUsage ;
    CERT_CHAIN_PARA     chainPara ;

    enhKeyUsage.cUsageIdentifier = 0 ;
    enhKeyUsage.rgpszUsageIdentifier = NULL ;
    certUsage.dwType = USAGE_MATCH_TYPE_AND ;
    certUsage.Usage = enhKeyUsage ;
    chainPara.cbSize = sizeof( CERT_CHAIN_PARA ) ;
    chainPara.RequestedUsage = certUsage ;

    Reference< XUnoTunnel > xCertTunnel( begin, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xcert = ( X509Certificate_MSCryptImpl* )xCertTunnel->getSomething( X509Certificate_MSCryptImpl::getUnoTunnelId() ) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    pCertContext = xcert->getMswcryCert() ;

    pChainContext = NULL ;

    BOOL bChain = FALSE;
    if( pCertContext != NULL )
    {
        HCERTSTORE hAdditionalStore = NULL;
        HCERTSTORE hCollectionStore = NULL;
        if (m_hCertStore && m_hKeyStore)
        {
            //Merge m_hCertStore and m_hKeyStore into one store.
            hCollectionStore = CertOpenStore(
                CERT_STORE_PROV_COLLECTION ,
                0 ,
                NULL ,
                0 ,
                NULL
                ) ;
            if (hCollectionStore != NULL)
            {
                CertAddStoreToCollection (
                     hCollectionStore ,
                     m_hCertStore ,
                     CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                     0) ;
                CertAddStoreToCollection (
                     hCollectionStore ,
                     m_hCertStore ,
                     CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                     0) ;
                hAdditionalStore = hCollectionStore;
            }

        }

        //if the merge of both stores failed then we add only m_hCertStore
        if (hAdditionalStore == NULL && m_hCertStore)
            hAdditionalStore = m_hCertStore;
        else if (hAdditionalStore == NULL && m_hKeyStore)
            hAdditionalStore = m_hKeyStore;
        else
            hAdditionalStore = NULL;

        //CertGetCertificateChain searches by default in MY, CA, ROOT and TRUST
        bChain = CertGetCertificateChain(
            NULL ,
            pCertContext ,
            NULL , //use current system time
            hAdditionalStore,
            &chainPara ,
            CERT_CHAIN_REVOCATION_CHECK_CHAIN | CERT_CHAIN_TIMESTAMP_TIME ,
            NULL ,
            &pChainContext);
        if (!bChain)
            pChainContext = NULL;

        //Close the additional store
       CertCloseStore(hCollectionStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }

    if(bChain &&  pChainContext != NULL && pChainContext->cChain > 0 )
    {
        PCCERT_CONTEXT pCertInChain ;
        PCERT_SIMPLE_CHAIN pCertChain ;
        X509Certificate_MSCryptImpl* pCert ;

        pCertChain = pChainContext->rgpChain[0] ;
        if( pCertChain->cElement ) {
            Sequence< Reference< XCertificate > > xCertChain( pCertChain->cElement ) ;

            for( unsigned int i = 0 ; i < pCertChain->cElement ; i ++ ) {
                if( pCertChain->rgpElement[i] )
                    pCertInChain = pCertChain->rgpElement[i]->pCertContext ;
                else
                    pCertInChain = NULL ;

                if( pCertInChain != NULL ) {
                    pCert = MswcryCertContextToXCert( pCertInChain ) ;
                    if( pCert != NULL )
                        xCertChain[i] = pCert ;
                }
            }

            CertFreeCertificateChain( pChainContext ) ;
            pChainContext = NULL ;

            return xCertChain ;
        }
    }
    if (pChainContext)
        CertFreeCertificateChain(pChainContext);

    return Sequence< Reference < XCertificate > >();
}

Reference< XCertificate > SecurityEnvironment_MSCryptImpl :: createCertificateFromRaw( const Sequence< sal_Int8 >& rawCertificate ) throw( SecurityException , RuntimeException ) {
    X509Certificate_MSCryptImpl* xcert ;

    if( rawCertificate.getLength() > 0 ) {
        xcert = new X509Certificate_MSCryptImpl() ;
        if( xcert == NULL )
            throw RuntimeException() ;

        xcert->setRawCert( rawCertificate ) ;
    } else {
        xcert = NULL ;
    }

    return xcert ;
}

Reference< XCertificate > SecurityEnvironment_MSCryptImpl :: createCertificateFromAscii( const OUString& asciiCertificate ) throw( SecurityException , RuntimeException ) {
    xmlChar* chCert ;
    xmlSecSize certSize ;

    OString oscert = OUStringToOString( asciiCertificate , RTL_TEXTENCODING_ASCII_US ) ;

    chCert = xmlStrndup( ( const xmlChar* )oscert.getStr(), ( int )oscert.getLength() ) ;

    certSize = xmlSecBase64Decode( chCert, ( xmlSecByte* )chCert, xmlStrlen( chCert ) ) ;

    Sequence< sal_Int8 > rawCert( certSize ) ;
    for( unsigned int i = 0 ; i < certSize ; i ++ )
        rawCert[i] = *( chCert + i ) ;

    xmlFree( chCert ) ;

    return createCertificateFromRaw( rawCert ) ;
}


HCERTSTORE getCertStoreForIntermediatCerts(
    const Sequence< Reference< ::com::sun::star::security::XCertificate > >& seqCerts)
{
    HCERTSTORE store = NULL;
    store = CertOpenStore(
        CERT_STORE_PROV_MEMORY, 0, NULL, 0, NULL);
    if (store == NULL)
        return NULL;

    for (int i = 0; i < seqCerts.getLength(); i++)
    {
        xmlsec_trace("Added temporary certificate: \n%s",
                     OUStringToOString(seqCerts[i]->getSubjectName(),
                                       osl_getThreadTextEncoding()).getStr());


        Sequence<sal_Int8> data = seqCerts[i]->getEncoded();
        PCCERT_CONTEXT cert = CertCreateCertificateContext(
            X509_ASN_ENCODING, ( const BYTE* )&data[0], data.getLength());
        //Adding the certificate creates a copy and not just increases the ref count
        //Therefore we free later the certificate that we now add
        CertAddCertificateContextToStore(store, cert, CERT_STORE_ADD_ALWAYS, NULL);
        CertFreeCertificateContext(cert);
    }
    return store;
}

//We return only valid or invalid, as long as the API documentation expresses
//explicitly that all validation steps are carried out even if one or several
//errors occur. See also
//http://wiki.openoffice.org/wiki/Certificate_Path_Validation#Validation_status
sal_Int32 SecurityEnvironment_MSCryptImpl :: verifyCertificate(
    const Reference< ::com::sun::star::security::XCertificate >& aCert,
    const Sequence< Reference< ::com::sun::star::security::XCertificate > >& seqCerts)
    throw( ::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException )
{
    sal_Int32 validity = 0;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    const X509Certificate_MSCryptImpl* xcert = NULL;

    Reference< XUnoTunnel > xCertTunnel( aCert, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xmlsec_trace("Start verification of certificate: \n %s",
                 OUStringToOString(
                     aCert->getSubjectName(), osl_getThreadTextEncoding()).getStr());

    xcert = ( X509Certificate_MSCryptImpl* )xCertTunnel->getSomething( X509Certificate_MSCryptImpl::getUnoTunnelId() ) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    pCertContext = xcert->getMswcryCert() ;

    CERT_ENHKEY_USAGE   enhKeyUsage ;
    CERT_USAGE_MATCH    certUsage ;
    CERT_CHAIN_PARA     chainPara ;
    memset(&chainPara, 0, sizeof(CERT_CHAIN_PARA));

    //Prepare parameter for CertGetCertificateChain
    enhKeyUsage.cUsageIdentifier = 0 ;
    enhKeyUsage.rgpszUsageIdentifier = NULL ;
    certUsage.dwType = USAGE_MATCH_TYPE_AND ;
    certUsage.Usage = enhKeyUsage ;
    chainPara.cbSize = sizeof( CERT_CHAIN_PARA ) ;
    chainPara.RequestedUsage = certUsage ;


    HCERTSTORE hCollectionStore = NULL;
    HCERTSTORE hIntermediateCertsStore = NULL;
    BOOL bChain = FALSE;
    if( pCertContext != NULL )
    {
        hIntermediateCertsStore =
            getCertStoreForIntermediatCerts(seqCerts);

        //Merge m_hCertStore and m_hKeyStore and the store of the intermediate
        //certificates into one store.
        hCollectionStore = CertOpenStore(
            CERT_STORE_PROV_COLLECTION ,
            0 ,
            NULL ,
            0 ,
            NULL
            ) ;
        if (hCollectionStore != NULL)
        {
            CertAddStoreToCollection (
                hCollectionStore ,
                m_hCertStore ,
                CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                0) ;
            CertAddStoreToCollection (
                hCollectionStore ,
                m_hCertStore ,
                CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                0) ;
            CertAddStoreToCollection (
                hCollectionStore,
                hIntermediateCertsStore,
                CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG,
                0);

        }

        //CertGetCertificateChain searches by default in MY, CA, ROOT and TRUST
        //We do not check revocation of the root. In most cases there are none.
        //Then we would get CERT_TRUST_REVOCATION_STATUS_UNKNOWN
        xmlsec_trace("Verifying cert using revocation information.");
        bChain = CertGetCertificateChain(
            NULL ,
            pCertContext ,
            NULL , //use current system time
            hCollectionStore,
            &chainPara ,
            CERT_CHAIN_REVOCATION_CHECK_CHAIN | CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,
            NULL ,
            &pChainContext);

        if (bChain && pChainContext->cChain > 0)
        {
            xmlsec_trace("Overall error status (all chains):");
            traceTrustStatus(pChainContext->TrustStatus.dwErrorStatus);
            //highest quality chains come first
            PCERT_SIMPLE_CHAIN pSimpleChain = pChainContext->rgpChain[0];
            xmlsec_trace("Error status of first chain: ");
            traceTrustStatus(pSimpleChain->TrustStatus.dwErrorStatus);

            //CERT_TRUST_REVOCATION_STATUS_UNKNOWN is also set if a certificate
            //has no AIA(OCSP) or CRLDP extension and there is no CRL locally installed.
            DWORD revocationFlags = CERT_TRUST_REVOCATION_STATUS_UNKNOWN |
                CERT_TRUST_IS_OFFLINE_REVOCATION;
            DWORD otherErrorsMask = ~revocationFlags;
            if( !(pSimpleChain->TrustStatus.dwErrorStatus & otherErrorsMask))

            {
                //No errors except maybe those caused by missing revocation information
                //Check if there are errors
                if ( pSimpleChain->TrustStatus.dwErrorStatus & revocationFlags)
                {
                    //No revocation information. Because MSDN documentation is not
                    //clear about if all other tests are performed if an error occurrs,
                    //we test again, without requiring revocation checking.
                    CertFreeCertificateChain(pChainContext);
                    pChainContext = NULL;
                    xmlsec_trace("Checking again but without requiring revocation information.");
                    bChain = CertGetCertificateChain(
                        NULL ,
                        pCertContext ,
                        NULL , //use current system time
                        hCollectionStore,
                        &chainPara ,
                        0,
                        NULL ,
                        &pChainContext);
                    if (bChain
                        && pChainContext->cChain > 0
                        && pChainContext->rgpChain[0]->TrustStatus.dwErrorStatus == CERT_TRUST_NO_ERROR)
                    {
                        xmlsec_trace("Certificate is valid.\n");
                        validity = ::com::sun::star::security::CertificateValidity::VALID;
                    }
                    else
                    {
                        xmlsec_trace("Certificate is invalid.\n");
                    }
                }
                else
                {
                    //valid and revocation information available
                    xmlsec_trace("Certificate is valid.\n");
                    validity = ::com::sun::star::security::CertificateValidity::VALID;
                }
            }
            else
            {
                //invalid
                xmlsec_trace("Certificate is invalid.\n");
                validity = ::com::sun::star::security::CertificateValidity::INVALID ;
            }
        }
        else
        {
            xmlsec_trace("CertGetCertificateChaine failed.\n");
        }
    }

    if (pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
        pChainContext = NULL;
    }

    //Close the additional store, do not destroy the contained certs
    CertCloseStore(hCollectionStore, CERT_CLOSE_STORE_CHECK_FLAG);
    //Close the temporary store containing the intermediate certificates and make
    //sure all certificates are deleted.
    CertCloseStore(hIntermediateCertsStore, CERT_CLOSE_STORE_CHECK_FLAG);

    return validity ;
}

sal_Int32 SecurityEnvironment_MSCryptImpl :: getCertificateCharacters( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& aCert ) throw( ::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException ) {
    sal_Int32 characters ;
    PCCERT_CONTEXT pCertContext ;
    const X509Certificate_MSCryptImpl* xcert ;

    Reference< XUnoTunnel > xCertTunnel( aCert, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xcert = ( X509Certificate_MSCryptImpl* )xCertTunnel->getSomething( X509Certificate_MSCryptImpl::getUnoTunnelId() ) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    pCertContext = xcert->getMswcryCert() ;

    characters = 0x00000000 ;

    //Firstly, make sentence whether or not the cert is self-signed.
    if( CertCompareCertificateName( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &(pCertContext->pCertInfo->Subject), &(pCertContext->pCertInfo->Issuer) ) ) {
        characters |= ::com::sun::star::security::CertificateCharacters::SELF_SIGNED ;
    } else {
        characters &= ~ ::com::sun::star::security::CertificateCharacters::SELF_SIGNED ;
    }

    //Secondly, make sentence whether or not the cert has a private key.
    {
        BOOL    fCallerFreeProv ;
        DWORD   dwKeySpec ;
        HCRYPTPROV  hProv ;
        if( CryptAcquireCertificatePrivateKey( pCertContext ,
                   0 ,
                   NULL ,
                   &( hProv ) ,
                   &( dwKeySpec ) ,
                   &( fCallerFreeProv ) )
        ) {
            characters |=  ::com::sun::star::security::CertificateCharacters::HAS_PRIVATE_KEY ;

            if( hProv != NULL && fCallerFreeProv )
                CryptReleaseContext( hProv, 0 ) ;
        } else {
            characters &= ~ ::com::sun::star::security::CertificateCharacters::HAS_PRIVATE_KEY ;
        }
    }
    return characters ;
}

void SecurityEnvironment_MSCryptImpl :: enableDefaultCrypt( sal_Bool enable ) throw( Exception, RuntimeException ) {
    m_bEnableDefault = enable ;
}

sal_Bool SecurityEnvironment_MSCryptImpl :: defaultEnabled() throw( Exception, RuntimeException ) {
    return m_bEnableDefault ;
}

X509Certificate_MSCryptImpl* MswcryCertContextToXCert( PCCERT_CONTEXT cert )
{
    X509Certificate_MSCryptImpl* xcert ;

    if( cert != NULL ) {
        xcert = new X509Certificate_MSCryptImpl() ;
        if( xcert != NULL ) {
            xcert->setMswcryCert( cert ) ;
        }
    } else {
        xcert = NULL ;
    }

    return xcert ;
}

OUString SecurityEnvironment_MSCryptImpl::getSecurityEnvironmentInformation() throw( ::com::sun::star::uno::RuntimeException )
{
    return OUString("Microsoft Crypto API");
}

/* Native methods */
xmlSecKeysMngrPtr SecurityEnvironment_MSCryptImpl :: createKeysManager() throw( Exception, RuntimeException ) {

    unsigned int i ;
    HCRYPTKEY symKey ;
    HCRYPTKEY pubKey ;
    HCRYPTKEY priKey ;
    xmlSecKeysMngrPtr pKeysMngr = NULL ;

    /*-
     * The following lines is based on the of xmlsec-mscrypto crypto engine
     */
    pKeysMngr = xmlSecMSCryptoAppliedKeysMngrCreate( m_hKeyStore , m_hCertStore ) ;
    if( pKeysMngr == NULL )
        throw RuntimeException() ;

    /*-
     * Adopt symmetric key into keys manager
     */
    for( i = 0 ; ( symKey = getSymKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecMSCryptoAppliedKeysMngrSymKeyLoad( pKeysMngr, symKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }

    /*-
     * Adopt asymmetric public key into keys manager
     */
    for( i = 0 ; ( pubKey = getPubKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecMSCryptoAppliedKeysMngrPubKeyLoad( pKeysMngr, pubKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }

    /*-
     * Adopt asymmetric private key into keys manager
     */
    for( i = 0 ; ( priKey = getPriKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecMSCryptoAppliedKeysMngrPriKeyLoad( pKeysMngr, priKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }

    /*-
     * Adopt system default certificate store.
     */
    if( defaultEnabled() ) {
        //Add system key store into the keys manager.
        m_hMySystemStore = CertOpenSystemStore( 0, "MY" ) ;
        if( m_hMySystemStore != NULL ) {
            if( xmlSecMSCryptoAppliedKeysMngrAdoptKeyStore( pKeysMngr, m_hMySystemStore ) < 0 ) {
                CertCloseStore( m_hMySystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                m_hMySystemStore = NULL;
                throw RuntimeException() ;
            }
        }

        //Add system root store into the keys manager.
        m_hRootSystemStore = CertOpenSystemStore( 0, "Root" ) ;
        if( m_hRootSystemStore != NULL ) {
            if( xmlSecMSCryptoAppliedKeysMngrAdoptTrustedStore( pKeysMngr, m_hRootSystemStore ) < 0 ) {
                CertCloseStore( m_hRootSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                m_hRootSystemStore = NULL;
                throw RuntimeException() ;
            }
        }

        //Add system trusted store into the keys manager.
        m_hTrustSystemStore = CertOpenSystemStore( 0, "Trust" ) ;
        if( m_hTrustSystemStore != NULL ) {
            if( xmlSecMSCryptoAppliedKeysMngrAdoptUntrustedStore( pKeysMngr, m_hTrustSystemStore ) < 0 ) {
                CertCloseStore( m_hTrustSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                m_hTrustSystemStore = NULL;
                throw RuntimeException() ;
            }
        }

        //Add system CA store into the keys manager.
        m_hCaSystemStore = CertOpenSystemStore( 0, "CA" ) ;
        if( m_hCaSystemStore != NULL ) {
            if( xmlSecMSCryptoAppliedKeysMngrAdoptUntrustedStore( pKeysMngr, m_hCaSystemStore ) < 0 ) {
                CertCloseStore( m_hCaSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                m_hCaSystemStore = NULL;
                throw RuntimeException() ;
            }
        }
    }

    return pKeysMngr ;
}
void SecurityEnvironment_MSCryptImpl :: destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) throw( Exception, RuntimeException ) {
    if( pKeysMngr != NULL ) {
        xmlSecKeysMngrDestroy( pKeysMngr ) ;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
