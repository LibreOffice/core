/*************************************************************************
 *
 *  $RCSfile: securityenvironment_mscryptimpl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mmi $ $Date: 2004-08-02 03:32:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _XSECURITYENVIRONMENT_MSCRYPTIMPL_HXX_
#include "securityenvironment_mscryptimpl.hxx"
#endif

#ifndef _X509CERTIFICATE_NSSIMPL_HXX_
#include "x509certificate_mscryptimpl.hxx"
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#include "Windows.h"
#include "WinCrypt.h"

#include "xmlsec/xmlsec.h"
#include "xmlsec/keysmngr.h"
#include "xmlsec/crypto.h"
#include <xmlsec/base64.h>

#include <xmlsecurity/biginteger.hxx>

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::security::XCertificate ;

extern X509Certificate_MSCryptImpl* MswcryCertContextToXCert( PCCERT_CONTEXT cert ) ;

SecurityEnvironment_MSCryptImpl :: SecurityEnvironment_MSCryptImpl( const Reference< XMultiServiceFactory >& aFactory ) : m_hProv( NULL ) , m_pszContainer( NULL ) , m_hKeyStore( NULL ), m_hCertStore( NULL ), m_tSymKeyList() , m_tPubKeyList() , m_tPriKeyList(), m_xServiceManager( aFactory ), m_bEnableDefault( sal_False ) {
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

    if( !m_tSymKeyList.empty()  ) {
        std::list< HCRYPTKEY >::iterator symKeyIt ;

        for( symKeyIt = m_tSymKeyList.begin() ; symKeyIt != m_tSymKeyList.end() ; symKeyIt ++ )
            CryptDestroyKey( *symKeyIt ) ;
    }

    if( !m_tPubKeyList.empty()  ) {
        std::list< HCRYPTKEY >::iterator pubKeyIt ;

        for( pubKeyIt = m_tPubKeyList.begin() ; pubKeyIt != m_tPubKeyList.end() ; pubKeyIt ++ )
            CryptDestroyKey( *pubKeyIt ) ;
    }

    if( !m_tPriKeyList.empty()  ) {
        std::list< HCRYPTKEY >::iterator priKeyIt ;

        for( priKeyIt = m_tPriKeyList.begin() ; priKeyIt != m_tPriKeyList.end() ; priKeyIt ++ )
            CryptDestroyKey( *priKeyIt ) ;
    }
}

/* XInitialization */
void SAL_CALL SecurityEnvironment_MSCryptImpl :: initialize( const Sequence< Any >& aArguments ) throw( Exception, RuntimeException ) {
    //TODO
} ;

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
    seqServiceNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.xml.crypto.SecurityEnvironment" ) ;
    return seqServiceNames ;
}

OUString SecurityEnvironment_MSCryptImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString::createFromAscii( "com.sun.star.xml.security.bridge.xmlsec.SecurityEnvironment_MSCryptImpl" ) ;
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
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return ( sal_Int64 )this ;
    }
    return 0 ;
}

/* XUnoTunnel extension */
const Sequence< sal_Int8>& SecurityEnvironment_MSCryptImpl :: getUnoTunnelId() {
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
        if( !CryptContextAddRef( aProv, NULL, NULL ) )
            throw Exception() ;
        else
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
        for( keyIt = m_tSymKeyList.begin() ; keyIt != m_tSymKeyList.end() ; keyIt ++ ) {
            if( *keyIt == aSymKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        if( !CryptDuplicateKey( aSymKey, NULL, 0, &symkey ) )
            throw RuntimeException() ;

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
        for( keyIt = m_tSymKeyList.begin() ; keyIt != m_tSymKeyList.end() ; keyIt ++ ) {
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
    for( pos = 0, keyIt = m_tSymKeyList.begin() ; pos < position && keyIt != m_tSymKeyList.end() ; pos ++ , keyIt ++ ) ;

    if( pos == position && keyIt != m_tSymKeyList.end() )
        symkey = *keyIt ;

    return symkey ;
}

void SecurityEnvironment_MSCryptImpl :: adoptPubKey( HCRYPTKEY aPubKey ) throw( Exception , RuntimeException ) {
    HCRYPTKEY   pubkey ;
    std::list< HCRYPTKEY >::iterator keyIt ;

    if( aPubKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tPubKeyList.begin() ; keyIt != m_tPubKeyList.end() ; keyIt ++ ) {
            if( *keyIt == aPubKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        if( !CryptDuplicateKey( aPubKey, NULL, 0, &pubkey ) )
            throw RuntimeException() ;

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
        for( keyIt = m_tPubKeyList.begin() ; keyIt != m_tPubKeyList.end() ; keyIt ++ ) {
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
    for( pos = 0, keyIt = m_tPubKeyList.begin() ; pos < position && keyIt != m_tPubKeyList.end() ; pos ++ , keyIt ++ ) ;

    if( pos == position && keyIt != m_tPubKeyList.end() )
        pubkey = *keyIt ;

    return pubkey ;
}

void SecurityEnvironment_MSCryptImpl :: adoptPriKey( HCRYPTKEY aPriKey ) throw( Exception , RuntimeException ) {
    HCRYPTKEY   prikey ;
    std::list< HCRYPTKEY >::iterator keyIt ;

    if( aPriKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tPriKeyList.begin() ; keyIt != m_tPriKeyList.end() ; keyIt ++ ) {
            if( *keyIt == aPriKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        if( !CryptDuplicateKey( aPriKey, NULL, 0, &prikey ) )
            throw RuntimeException() ;

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
        for( keyIt = m_tPriKeyList.begin() ; keyIt != m_tPriKeyList.end() ; keyIt ++ ) {
            if( *keyIt == aPriKey ) {
                prikey = *keyIt ;
                CryptDestroyKey( prikey ) ;
                m_tPriKeyList.erase( keyIt ) ;
                break ;
            }
        }
    }
}

HCRYPTKEY SecurityEnvironment_MSCryptImpl :: getPriKey( unsigned int position ) throw( SecurityException , RuntimeException ) {
    HCRYPTKEY prikey ;
    std::list< HCRYPTKEY >::iterator keyIt ;
    unsigned int pos ;

    prikey = NULL ;
    for( pos = 0, keyIt = m_tPriKeyList.begin() ; pos < position && keyIt != m_tPriKeyList.end() ; pos ++ , keyIt ++ ) ;

    if( pos == position && keyIt != m_tPriKeyList.end() )
        prikey = *keyIt ;

    return prikey ;
}

//Methods from XSecurityEnvironment
Sequence< Reference < XCertificate > > SecurityEnvironment_MSCryptImpl :: getPersonalCertificates() throw( SecurityException , RuntimeException ) {
    sal_Int32 length ;
    X509Certificate_MSCryptImpl* xcert ;
    std::list< X509Certificate_MSCryptImpl* > certsList ;
    PCCERT_CONTEXT pCertContext ;

    //firstly, we try to find private keys in given key store.
    if( m_hKeyStore != NULL ) {
        pCertContext = NULL ;
        while( pCertContext = CertEnumCertificatesInStore( m_hKeyStore, pCertContext ) ) {
            xcert = MswcryCertContextToXCert( pCertContext ) ;
            if( xcert != NULL )
                certsList.push_back( xcert ) ;
        }
    }

    //secondly, we try to find certificate from registered private keys.
    if( !m_tPriKeyList.empty()  ) {
        //TODO: Don't know whether or not it is necessary ans possible.
    }

    //Thirdly, we try to find certificate from system default key store.
    if( m_bEnableDefault ) {
        HCERTSTORE hSystemKeyStore ;

        /*
        hSystemKeyStore = CertOpenStore(
                CERT_STORE_PROV_SYSTEM ,
                0 ,
                NULL ,
                CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG | CERT_STORE_OPEN_EXISTING_FLAG ,
                L"MY"
            ) ;
        */
        hSystemKeyStore = CertOpenSystemStore( 0, "MY" ) ;
        if( hSystemKeyStore != NULL ) {
            pCertContext = NULL ;
            while( pCertContext = CertEnumCertificatesInStore( hSystemKeyStore, pCertContext ) ) {
                xcert = MswcryCertContextToXCert( pCertContext ) ;
                if( xcert != NULL )
                    certsList.push_back( xcert ) ;
            }
        }

        CertCloseStore( hSystemKeyStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
    }

    length = certsList.size() ;
    if( length != 0 ) {
        int i ;
        std::list< X509Certificate_MSCryptImpl* >::iterator xcertIt ;
        Sequence< Reference< XCertificate > > certSeq( length ) ;

        for( i = 0, xcertIt = certsList.begin(); xcertIt != certsList.end(); xcertIt ++, i++ ) {
            certSeq[i] = *xcertIt ;
        }

        return certSeq ;
    }

    return Sequence< Reference< XCertificate > >() ;
}

Reference< XCertificate > SecurityEnvironment_MSCryptImpl :: getCertificate( const OUString& issuerName, const Sequence< sal_Int8 >& serialNumber ) throw( SecurityException , RuntimeException ) {
    X509Certificate_MSCryptImpl* xcert ;
    PCCERT_CONTEXT pCertContext ;
    LPSTR   pszName ;
    CERT_INFO certInfo ;

    //Create cert info from issue and serial
    rtl::OString oissuer = rtl::OUStringToOString( issuerName , RTL_TEXTENCODING_ASCII_US ) ;
    pszName = ( char* )oissuer.getStr() ;

    if( ! ( CertStrToName(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
        pszName ,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
        NULL ,
        NULL ,
        &certInfo.Issuer.cbData, NULL ) )
    ) {
        throw SecurityException() ;
    }

    if( !( certInfo.Issuer.pbData = ( BYTE* )malloc( certInfo.Issuer.cbData ) ) ) {
        throw RuntimeException() ;
    }

    if( ! ( CertStrToName(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
        pszName ,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
        NULL ,
        ( BYTE* )certInfo.Issuer.pbData ,
        &certInfo.Issuer.cbData, NULL ) )
    ) {
        free( certInfo.Issuer.pbData ) ;
        throw SecurityException() ;
    }

    certInfo.SerialNumber.cbData = serialNumber.getLength() ;
    if( !( certInfo.SerialNumber.pbData = ( BYTE* )malloc( certInfo.SerialNumber.cbData ) ) ) {
        free( certInfo.Issuer.pbData ) ;
        throw SecurityException() ;
    }

    for( unsigned int i = 0; i < certInfo.SerialNumber.cbData; i ++ )
        certInfo.SerialNumber.pbData[i] = serialNumber[ certInfo.SerialNumber.cbData - i - 1 ] ;

    pCertContext = NULL ;

    //Above all, we try to find the certificate in the given key store.
    if( m_hKeyStore != NULL ) {
        //Find the certificate from the store.
        pCertContext = CertFindCertificateInStore(
            m_hKeyStore,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,
            CERT_FIND_SUBJECT_CERT,
            &certInfo,
            NULL
        ) ;
    }

    //firstly, we try to find the certificate in the given cert store.
    if( m_hCertStore != NULL ) {
        //Find the certificate from the store.
        pCertContext = CertFindCertificateInStore(
            m_hCertStore,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,
            CERT_FIND_SUBJECT_CERT,
            &certInfo,
            NULL
        ) ;
    }

    //Secondly, we try to find certificate from system default key store.
    if( pCertContext == NULL && m_bEnableDefault ) {
        HCERTSTORE hSystemKeyStore ;

        hSystemKeyStore = CertOpenSystemStore( 0, "MY" ) ;
        if( hSystemKeyStore != NULL ) {
            pCertContext = CertFindCertificateInStore(
                hSystemKeyStore,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                0,
                CERT_FIND_SUBJECT_CERT,
                &certInfo,
                NULL
            ) ;
        }

        CertCloseStore( hSystemKeyStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
    }

    //Thirdly, we try to find certificate from system default root cert store.
    if( pCertContext == NULL && m_bEnableDefault ) {
        HCERTSTORE hSystemRootStore ;

        hSystemRootStore = CertOpenSystemStore( 0, "Root" ) ;
        if( hSystemRootStore != NULL ) {
            pCertContext = CertFindCertificateInStore(
                hSystemRootStore,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                0,
                CERT_FIND_SUBJECT_CERT,
                &certInfo,
                NULL
            ) ;
        }

        CertCloseStore( hSystemRootStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
    }

    //Fourthly, we try to find certificate from system default trust cert store.
    if( pCertContext == NULL && m_bEnableDefault ) {
        HCERTSTORE hSystemTrustStore ;

        hSystemTrustStore = CertOpenSystemStore( 0, "Trust" ) ;
        if( hSystemTrustStore != NULL ) {
            pCertContext = CertFindCertificateInStore(
                hSystemTrustStore,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                0,
                CERT_FIND_SUBJECT_CERT,
                &certInfo,
                NULL
            ) ;
        }

        CertCloseStore( hSystemTrustStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
    }

    //Fifthly, we try to find certificate from system default CA cert store.
    if( pCertContext == NULL && m_bEnableDefault ) {
        HCERTSTORE hSystemCAStore ;

        hSystemCAStore = CertOpenSystemStore( 0, "CA" ) ;
        if( hSystemCAStore != NULL ) {
            pCertContext = CertFindCertificateInStore(
                hSystemCAStore,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                0,
                CERT_FIND_SUBJECT_CERT,
                &certInfo,
                NULL
            ) ;
        }

        CertCloseStore( hSystemCAStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
    }


    free( certInfo.Issuer.pbData ) ;
    free( certInfo.SerialNumber.pbData ) ;

    if( pCertContext != NULL ) {
        xcert = MswcryCertContextToXCert( pCertContext ) ;
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

    //Above all, we try to find the certificate in the given key store.
    if( pCertContext != NULL ) {
        if( !CertGetCertificateChain(
                NULL ,
                pCertContext ,
                NULL ,
                m_hKeyStore ,
                &chainPara ,
                0 ,
                NULL ,
                &pChainContext
            )
        ) {
            pChainContext = NULL ;
        }
    }

    //firstly, we try to find the certificate in the given cert store.
    if( pCertContext != NULL && pChainContext != NULL ) {
        if( !CertGetCertificateChain(
                NULL ,
                pCertContext ,
                NULL ,
                m_hCertStore ,
                &chainPara ,
                0 ,
                NULL ,
                &pChainContext
            )
        ) {
            pChainContext = NULL ;
        }
    }

    //Secondly, we try to find certificate from system default system store.
    if( pCertContext != NULL && pChainContext != NULL ) {
        HCERTSTORE hCollectionStore ;

        hCollectionStore = CertOpenStore(
                CERT_STORE_PROV_COLLECTION ,
                0 ,
                NULL ,
                0 ,
                NULL
            ) ;

        if( hCollectionStore != NULL ) {
            HCERTSTORE hSystemStore ;

            //Add system key store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "MY" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    1
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Add system root store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "Root" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    2
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Add system trust store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "Trust" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    3
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Add system CA store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "CA" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    4
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Get the chain from the collection store.
            if( !CertGetCertificateChain(
                    NULL ,
                    pCertContext ,
                    NULL ,
                    hCollectionStore ,
                    &chainPara ,
                    0 ,
                    NULL ,
                    &pChainContext
                )
            ) {
                pChainContext = NULL ;
            }

            CertCloseStore( hCollectionStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        }
    }

    if( pChainContext != NULL && pChainContext->cChain > 0 ) {
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

    return NULL ;
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

    rtl::OString oscert = rtl::OUStringToOString( asciiCertificate , RTL_TEXTENCODING_ASCII_US ) ;

    chCert = xmlStrndup( ( const xmlChar* )oscert.getStr(), ( int )oscert.getLength() ) ;

    certSize = xmlSecBase64Decode( chCert, ( xmlSecByte* )chCert, xmlStrlen( chCert ) ) ;

    Sequence< sal_Int8 > rawCert( certSize ) ;
    for( unsigned int i = 0 ; i < certSize ; i ++ )
        rawCert[i] = *( chCert + i ) ;

    xmlFree( chCert ) ;

    return createCertificateFromRaw( rawCert ) ;
}

sal_Int32 SecurityEnvironment_MSCryptImpl :: verifyCertificate( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& aCert ) throw( ::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException ) {
    sal_Int32 validity ;
    PCCERT_CHAIN_CONTEXT pChainContext ;
    PCCERT_CONTEXT pCertContext ;
    const X509Certificate_MSCryptImpl* xcert ;
    FILETIME fTime ;
    DWORD chainStatus ;

    CERT_ENHKEY_USAGE   enhKeyUsage ;
    CERT_USAGE_MATCH    certUsage ;
    CERT_CHAIN_PARA     chainPara ;

    Reference< XUnoTunnel > xCertTunnel( aCert, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xcert = ( X509Certificate_MSCryptImpl* )xCertTunnel->getSomething( X509Certificate_MSCryptImpl::getUnoTunnelId() ) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    pCertContext = xcert->getMswcryCert() ;

    //Get the current system time.
    GetSystemTimeAsFileTime( &fTime ) ;

    /*-
     * Build and validate the certificate
     */
    enhKeyUsage.cUsageIdentifier = 0 ;
    enhKeyUsage.rgpszUsageIdentifier = NULL ;
    certUsage.dwType = USAGE_MATCH_TYPE_AND ;
    certUsage.Usage = enhKeyUsage ;
    chainPara.cbSize = sizeof( CERT_CHAIN_PARA ) ;
    chainPara.RequestedUsage = certUsage ;

    pChainContext = NULL ;

    //Above all, we try to find the certificate in the given key store.
    if( pCertContext != NULL ) {
        if( !CertGetCertificateChain(
                NULL ,
                pCertContext ,
                &fTime ,
                m_hKeyStore ,
                &chainPara ,
                CERT_CHAIN_REVOCATION_CHECK_CHAIN | CERT_CHAIN_TIMESTAMP_TIME ,
                NULL ,
                &pChainContext
            )
        ) {
            pChainContext = NULL ;
        }
    }

    //firstly, we try to find the certificate in the given cert store.
    if( pCertContext != NULL && pChainContext != NULL ) {
        if( !CertGetCertificateChain(
                NULL ,
                pCertContext ,
                &fTime ,
                m_hCertStore ,
                &chainPara ,
                CERT_CHAIN_REVOCATION_CHECK_CHAIN | CERT_CHAIN_TIMESTAMP_TIME ,
                NULL ,
                &pChainContext
            )
        ) {
            pChainContext = NULL ;
        }
    }

    //Secondly, we try to find certificate from system default system store.
    if( pCertContext != NULL && pChainContext != NULL ) {
        HCERTSTORE hCollectionStore ;

        hCollectionStore = CertOpenStore(
                CERT_STORE_PROV_COLLECTION ,
                0 ,
                NULL ,
                0 ,
                NULL
            ) ;

        if( hCollectionStore != NULL ) {
            HCERTSTORE hSystemStore ;

            //Add system key store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "MY" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    1
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Add system root store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "Root" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    2
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Add system trust store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "Trust" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    3
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Add system CA store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "CA" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    4
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Get the chain from the collection store.
            if( !CertGetCertificateChain(
                    NULL ,
                    pCertContext ,
                    &fTime ,
                    hCollectionStore ,
                    &chainPara ,
                    CERT_CHAIN_REVOCATION_CHECK_CHAIN | CERT_CHAIN_TIMESTAMP_TIME ,
                    NULL ,
                    &pChainContext
                )
            ) {
                pChainContext = NULL ;
            }

            CertCloseStore( hCollectionStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        }
    }

    if( pChainContext != NULL ) {
        chainStatus = pChainContext->TrustStatus.dwErrorStatus ;
        CertFreeCertificateChain( pChainContext ) ;

        if( chainStatus == CERT_TRUST_NO_ERROR ) {
            validity = !( ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_INVALID ) ;
        } else {
            validity = ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_INVALID ;
        }

        if( ( chainStatus & CERT_TRUST_IS_NOT_TIME_VALID ) == CERT_TRUST_IS_NOT_TIME_VALID ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_TIMEOUT ;
        }

        if( ( chainStatus & CERT_TRUST_IS_NOT_TIME_NESTED ) == CERT_TRUST_IS_NOT_TIME_NESTED ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_TIMEOUT ;
        }

        if( ( chainStatus & CERT_TRUST_IS_REVOKED ) == CERT_TRUST_IS_REVOKED ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_REVOKED ;
        }

        if( ( chainStatus & CERT_TRUST_IS_OFFLINE_REVOCATION ) == CERT_TRUST_IS_OFFLINE_REVOCATION ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_REVOKED ;
        }

        if( ( chainStatus & CERT_TRUST_IS_NOT_SIGNATURE_VALID ) == CERT_TRUST_IS_NOT_SIGNATURE_VALID ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_SIGNATURE_INVALID ;
        }

        if( ( chainStatus & CERT_TRUST_IS_UNTRUSTED_ROOT ) == CERT_TRUST_IS_UNTRUSTED_ROOT ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_ROOT_UNTRUSTED ;
        }

        if( ( chainStatus & CERT_TRUST_REVOCATION_STATUS_UNKNOWN ) == CERT_TRUST_REVOCATION_STATUS_UNKNOWN ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_UNKNOWN_REVOKATION ;
        }

        if( ( chainStatus & CERT_TRUST_INVALID_EXTENSION ) == CERT_TRUST_INVALID_EXTENSION ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_EXTENSION_INVALID ;
        }

        if( ( chainStatus & CERT_TRUST_IS_PARTIAL_CHAIN ) == CERT_TRUST_IS_PARTIAL_CHAIN ) {
            validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_CHAIN_INCOMPLETE ;
        }
    } else {
        validity = ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_INVALID ;
    }

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
        characters |= ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_SELF_SIGNED ;
    } else {
        characters &= ~ ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_SELF_SIGNED ;
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
            characters |=  ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_HAS_PRIVATE_KEY ;

            if( hProv != NULL && fCallerFreeProv )
                CryptReleaseContext( hProv, 0 ) ;
        } else {
            characters &= ~ ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_HAS_PRIVATE_KEY ;
        }
    }

    //Thirdly, make sentence whether or not the cert is trusted.
    {
        HCERTSTORE hCollectionStore ;
        PCCERT_CONTEXT pTempCert ;

        hCollectionStore = CertOpenStore(
                CERT_STORE_PROV_COLLECTION ,
                0 ,
                NULL ,
                0 ,
                NULL
            ) ;

        if( hCollectionStore != NULL ) {
            HCERTSTORE hSystemStore ;

            //Add system key store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "MY" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    1
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Add system root store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "Root" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    2
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Add system trust store to the collection.
            hSystemStore = CertOpenSystemStore( 0, "Trust" ) ;
            if( hSystemStore != NULL ) {
                CertAddStoreToCollection (
                    hCollectionStore ,
                    hSystemStore ,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG ,
                    3
                ) ;
                CertCloseStore( hSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
            }

            //Find the cert in the collection store.
            pTempCert = CertFindCertificateInStore(
                hCollectionStore ,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                0 ,
                CERT_FIND_SUBJECT_NAME,
                &( pCertContext->pCertInfo->Subject ) ,
                NULL
            ) ;

            if( pTempCert != NULL && CertCompareCertificate( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pCertContext->pCertInfo, pTempCert->pCertInfo ) ) {
                characters |=  ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_TRUSTED ;
            } else {
                characters &= ~ ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_TRUSTED ;
            }
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

