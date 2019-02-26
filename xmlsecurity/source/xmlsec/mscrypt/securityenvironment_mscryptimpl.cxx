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

#include <sal/config.h>

#include <cstddef>
#include <string.h>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <WinCrypt.h>
#include <sal/macros.h>
#include <osl/thread.h>
#include "securityenvironment_mscryptimpl.hxx"

#include "x509certificate_mscryptimpl.hxx"
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <xmlsec-wrapper.h>
#include "akmngr.hxx"

#include <biginteger.hxx>

#include <comphelper/windowserrorstring.hxx>
#include <sal/log.hxx>
#include <rtl/locale.h>
#include <osl/nlsupport.h>
#include <osl/process.h>
#include <o3tl/char16_t2wchar_t.hxx>
#include <svl/cryptosign.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::security::XCertificate ;

static X509Certificate_MSCryptImpl* MswcryCertContextToXCert( PCCERT_CONTEXT cert ) ;

struct CertErrorToString{
    DWORD error;
    char const * name;
};

CertErrorToString const arErrStrings[] =
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

static void traceTrustStatus(DWORD err)
{
    if (err == 0)
        SAL_INFO("xmlsecurity.xmlsec", "  " << arErrStrings[0].name);
    for (std::size_t i = 1; i < SAL_N_ELEMENTS(arErrStrings); i++)
    {
        if (arErrStrings[i].error & err)
            SAL_INFO("xmlsecurity.xmlsec", "  " << arErrStrings[i].name);
    }
}

SecurityEnvironment_MSCryptImpl::SecurityEnvironment_MSCryptImpl( const uno::Reference< uno::XComponentContext >& xContext ) : m_hProv( NULL ) , m_pszContainer( nullptr ) , m_hKeyStore( nullptr ), m_hCertStore( nullptr ), m_hMySystemStore(nullptr), m_hRootSystemStore(nullptr), m_hTrustSystemStore(nullptr), m_hCaSystemStore(nullptr), m_bEnableDefault( false ){

    m_xServiceManager.set(xContext, uno::UNO_QUERY);
}

SecurityEnvironment_MSCryptImpl::~SecurityEnvironment_MSCryptImpl() {

    if( m_hProv != NULL ) {
        CryptReleaseContext( m_hProv, 0 ) ;
        m_hProv = NULL ;
    }

    if( m_pszContainer != nullptr ) {
        //TODO: Don't know whether or not it should be released now.
        m_pszContainer = nullptr ;
    }

    if( m_hCertStore != nullptr ) {
        CertCloseStore( m_hCertStore, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        m_hCertStore = nullptr ;
    }

    if( m_hKeyStore != nullptr ) {
        CertCloseStore( m_hKeyStore, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        m_hKeyStore = nullptr ;
    }

    //i120675, close the store handles
    if( m_hMySystemStore != nullptr ) {
        CertCloseStore( m_hMySystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        m_hMySystemStore = nullptr ;
    }

    if( m_hRootSystemStore != nullptr ) {
        CertCloseStore( m_hRootSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        m_hRootSystemStore = nullptr ;
    }

    if( m_hTrustSystemStore != nullptr ) {
        CertCloseStore( m_hTrustSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        m_hTrustSystemStore = nullptr ;
    }

    if( m_hCaSystemStore != nullptr ) {
        CertCloseStore( m_hCaSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        m_hCaSystemStore = nullptr ;
    }
}

/* XServiceInfo */
OUString SAL_CALL SecurityEnvironment_MSCryptImpl::getImplementationName() {
    return OUString("com.sun.star.xml.crypto.SecurityEnvironment");
}

/* XServiceInfo */
sal_Bool SAL_CALL SecurityEnvironment_MSCryptImpl::supportsService( const OUString& serviceName) {
    uno::Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return true ;
    }
    return false ;
}

/* XServiceInfo */
uno::Sequence< OUString > SAL_CALL SecurityEnvironment_MSCryptImpl::getSupportedServiceNames() {
    uno::Sequence<OUString> seqServiceNames { "com.sun.star.xml.crypto.SecurityEnvironment" };
    return seqServiceNames ;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL SecurityEnvironment_MSCryptImpl::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
{
    if( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return reinterpret_cast<sal_Int64>(this);
    }
    return 0 ;
}

/* XUnoTunnel extension */


namespace
{
    class theSecurityEnvironment_MSCryptImplUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSecurityEnvironment_MSCryptImplUnoTunnelId > {};
}

const uno::Sequence< sal_Int8>& SecurityEnvironment_MSCryptImpl::getUnoTunnelId() {
    return theSecurityEnvironment_MSCryptImplUnoTunnelId::get().getSeq();
}

/* XUnoTunnel extension */
SecurityEnvironment_MSCryptImpl* SecurityEnvironment_MSCryptImpl::getImplementation( const uno::Reference< XInterface >& rObj ) {
    uno::Reference< XUnoTunnel > xUT( rObj , uno::UNO_QUERY ) ;
    if( xUT.is() ) {
        return reinterpret_cast<SecurityEnvironment_MSCryptImpl*>(xUT->getSomething( getUnoTunnelId() ));
    } else
        return nullptr ;
}

HCRYPTPROV SecurityEnvironment_MSCryptImpl::getCryptoProvider() {
    return m_hProv ;
}

void SecurityEnvironment_MSCryptImpl::setCryptoProvider( HCRYPTPROV aProv ) {
    if( m_hProv != NULL ) {
        CryptReleaseContext( m_hProv, 0 ) ;
        m_hProv = NULL ;
    }

    if( aProv != NULL ) {
        m_hProv = aProv ;
    }
}

LPCTSTR SecurityEnvironment_MSCryptImpl::getKeyContainer() {
    return m_pszContainer ;
}

void SecurityEnvironment_MSCryptImpl::setKeyContainer( LPCTSTR aKeyContainer ) {
    //TODO: Don't know whether or not it should be copied.
    m_pszContainer = aKeyContainer ;
}


HCERTSTORE SecurityEnvironment_MSCryptImpl::getCryptoSlot() {
    return m_hKeyStore ;
}

void SecurityEnvironment_MSCryptImpl::setCryptoSlot( HCERTSTORE aSlot) {
    if( m_hKeyStore != nullptr ) {
        CertCloseStore( m_hKeyStore, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        m_hKeyStore = nullptr ;
    }

    if( aSlot != nullptr ) {
        m_hKeyStore = CertDuplicateStore( aSlot ) ;
    }
}

HCERTSTORE SecurityEnvironment_MSCryptImpl::getCertDb() {
    return m_hCertStore ;
}

void SecurityEnvironment_MSCryptImpl::setCertDb( HCERTSTORE aCertDb ) {
    if( m_hCertStore != nullptr ) {
        CertCloseStore( m_hCertStore, CERT_CLOSE_STORE_FORCE_FLAG ) ;
        m_hCertStore = nullptr ;
    }

    if( aCertDb != nullptr ) {
        m_hCertStore = CertDuplicateStore( aCertDb ) ;
    }
}

#ifdef SAL_LOG_INFO

// Based on sample code from MSDN

static OUString get_system_name(const void *pvSystemStore,
                            DWORD dwFlags)
{
    LPCWSTR ppwszSystemName;
    if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG)
    {
        _CERT_SYSTEM_STORE_RELOCATE_PARA const * pRelocatePara;
        pRelocatePara = static_cast<_CERT_SYSTEM_STORE_RELOCATE_PARA const *>(pvSystemStore);
        ppwszSystemName = pRelocatePara->pwszSystemStore;
    }
    else
    {
        ppwszSystemName = static_cast<LPCWSTR>(pvSystemStore);
    }
    return o3tl::toU(ppwszSystemName);
}

extern "C" {

static BOOL WINAPI cert_enum_physical_store_callback(const void *,
                                                         DWORD dwFlags,
                                                         LPCWSTR pwszStoreName,
                                                         PCERT_PHYSICAL_STORE_INFO,
                                                         void *,
                                                         void *)
{
    OUString name(o3tl::toU(pwszStoreName));
    if (dwFlags & CERT_PHYSICAL_STORE_PREDEFINED_ENUM_FLAG)
        name += " (implicitly created)";
    SAL_INFO("xmlsecurity.xmlsec", "  Physical store: " << name);

    return TRUE;
}

static BOOL WINAPI cert_enum_system_store_callback(const void *pvSystemStore,
                                                       DWORD dwFlags,
                                                       PCERT_SYSTEM_STORE_INFO,
                                                       void *,
                                                       void *)
{
    SAL_INFO("xmlsecurity.xmlsec", "System store: " << get_system_name(pvSystemStore, dwFlags));

    if (!CertEnumPhysicalStore(pvSystemStore,
                               dwFlags,
                               nullptr,
                               cert_enum_physical_store_callback))
    {
        DWORD dwErr = GetLastError();
        if (!(ERROR_FILE_NOT_FOUND == dwErr ||
              ERROR_NOT_SUPPORTED == dwErr))
        {
            SAL_WARN("xmlsecurity.xmlsec", "CertEnumPhysicalStore failed:" << WindowsErrorString(GetLastError()));
        }
    }
    return TRUE;
}

}

#endif

//Methods from XSecurityEnvironment
uno::Sequence< uno::Reference < XCertificate > > SecurityEnvironment_MSCryptImpl::getPersonalCertificates()
{
    sal_Int32 length ;
    X509Certificate_MSCryptImpl* xcert ;
    std::list< X509Certificate_MSCryptImpl* > certsList ;
    PCCERT_CONTEXT pCertContext = nullptr;

    //firstly, we try to find private keys in given key store.
    if( m_hKeyStore != nullptr ) {
        pCertContext = CertEnumCertificatesInStore( m_hKeyStore, pCertContext );
        while (pCertContext)
        {
            xcert = MswcryCertContextToXCert( pCertContext ) ;
            if( xcert != nullptr )
                certsList.push_back( xcert ) ;
            pCertContext = CertEnumCertificatesInStore( m_hKeyStore, pCertContext );
        }
    }

    //Thirdly, we try to find certificate from system default key store.
    if( m_bEnableDefault ) {
        HCERTSTORE hSystemKeyStore ;
        DWORD      dwKeySpec;
        NCRYPT_KEY_HANDLE hCryptKey;

#ifdef SAL_LOG_INFO
        CertEnumSystemStore(CERT_SYSTEM_STORE_CURRENT_USER, nullptr, nullptr, cert_enum_system_store_callback);
#endif

        hSystemKeyStore = CertOpenSystemStoreW( 0, L"MY" ) ;
        if( hSystemKeyStore != nullptr ) {
            pCertContext = CertEnumCertificatesInStore( hSystemKeyStore, pCertContext );
            while (pCertContext)
            {
                // for checking whether the certificate is a personal certificate or not.
                DWORD dwFlags = CRYPT_ACQUIRE_COMPARE_KEY_FLAG | CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG;
                HCRYPTPROV_OR_NCRYPT_KEY_HANDLE* phCryptProvOrNCryptKey = &hCryptKey;
                if(!(CryptAcquireCertificatePrivateKey(pCertContext,
                        dwFlags,
                        nullptr,
                        phCryptProvOrNCryptKey,
                        &dwKeySpec,
                        nullptr)))
                {
                    // Not Privatekey found. SKIP this one.
                    pCertContext = CertEnumCertificatesInStore( hSystemKeyStore, pCertContext );
                    continue;
                }
                // then TODO : Check the personal cert is valid or not.

                xcert = MswcryCertContextToXCert( pCertContext ) ;
                if( xcert != nullptr )
                    certsList.push_back( xcert ) ;
                pCertContext = CertEnumCertificatesInStore( hSystemKeyStore, pCertContext );
            }
        }

        CertCloseStore( hSystemKeyStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
    }

    length = certsList.size() ;
    if( length != 0 ) {
        int i = 0;
        uno::Sequence< uno::Reference< XCertificate > > certSeq( length ) ;

        for( const auto& rXCert : certsList ) {
            certSeq[i] = rXCert ;
            ++i;
        }

        return certSeq ;
    }

    return uno::Sequence< uno::Reference< XCertificate > >() ;
}


uno::Reference< XCertificate > SecurityEnvironment_MSCryptImpl::getCertificate( const OUString& issuerName, const uno::Sequence< sal_Int8 >& serialNumber ) {
    unsigned int i ;
    X509Certificate_MSCryptImpl *xcert = nullptr ;
    PCCERT_CONTEXT pCertContext = nullptr ;
    HCERTSTORE hCertStore = nullptr ;
    CRYPT_INTEGER_BLOB cryptSerialNumber ;
    CERT_INFO certInfo ;

    // for correct encoding
    sal_uInt16 encoding ;
    rtl_Locale *pLocale = nullptr ;
    osl_getProcessLocale( &pLocale ) ;
    encoding = osl_getTextEncodingFromLocale( pLocale ) ;

    //Create cert info from issue and serial
    LPCWSTR pszName = o3tl::toW( issuerName.getStr() );

    if( ! ( CertStrToNameW(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
        pszName ,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG | CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG,
        nullptr ,
        nullptr ,
        &certInfo.Issuer.cbData, nullptr ) )
    ) {
        return nullptr ;
    }

    certInfo.Issuer.pbData = static_cast<BYTE*>(malloc( certInfo.Issuer.cbData ));
    if(!certInfo.Issuer.pbData)
        throw uno::RuntimeException() ;

    if( ! ( CertStrToNameW(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
        pszName ,
        CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG | CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG,
        nullptr ,
        certInfo.Issuer.pbData ,
        &certInfo.Issuer.cbData, nullptr ) )
    ) {
        free( certInfo.Issuer.pbData ) ;
        return nullptr ;
    }

    //Get the SerialNumber
    cryptSerialNumber.cbData = serialNumber.getLength() ;
    cryptSerialNumber.pbData = static_cast<BYTE*>(malloc( cryptSerialNumber.cbData));
    if (!cryptSerialNumber.pbData)
    {
        free( certInfo.Issuer.pbData ) ;
        throw uno::RuntimeException() ;
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
            if(m_hKeyStore == nullptr) continue ;
            hCertStore = m_hKeyStore ;
            break;
        case 1:
            if(m_hCertStore == nullptr) continue ;
            hCertStore = m_hCertStore ;
            break;
        case 2:
            hCertStore = CertOpenSystemStoreW( 0, L"MY" ) ;
            if(hCertStore == nullptr || !m_bEnableDefault) continue ;
            break;
        case 3:
            hCertStore = CertOpenSystemStoreW( 0, L"Root" ) ;
            if(hCertStore == nullptr || !m_bEnableDefault) continue ;
            break;
        case 4:
            hCertStore = CertOpenSystemStoreW( 0, L"Trust" ) ;
            if(hCertStore == nullptr || !m_bEnableDefault) continue ;
            break;
        case 5:
            hCertStore = CertOpenSystemStoreW( 0, L"CA" ) ;
            if(hCertStore == nullptr || !m_bEnableDefault) continue ;
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
            nullptr
        ) ;

        if(i != 0 && i != 1) CertCloseStore( hCertStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
        if( pCertContext != nullptr ) break ; // Found the certificate.

    }

    free(cryptSerialNumber.pbData);
    free(certInfo.Issuer.pbData);

    if( pCertContext != nullptr ) {
        xcert = MswcryCertContextToXCert(pCertContext);
        CertFreeCertificateContext(pCertContext);
    } else {
        xcert = nullptr ;
    }

    return xcert ;
}

uno::Reference< XCertificate > SecurityEnvironment_MSCryptImpl::getCertificate( const OUString& issuerName, const OUString& serialNumber ) {
    uno::Sequence< sal_Int8 > serial = xmlsecurity::numericStringToBigInteger( serialNumber ) ;
    return getCertificate( issuerName, serial ) ;
}

uno::Sequence< uno::Reference < XCertificate > > SecurityEnvironment_MSCryptImpl::buildCertificatePath( const uno::Reference< XCertificate >& begin ) {
    PCCERT_CHAIN_CONTEXT pChainContext ;
    PCCERT_CONTEXT pCertContext ;
    const X509Certificate_MSCryptImpl* xcert ;

    CERT_ENHKEY_USAGE   enhKeyUsage ;
    CERT_USAGE_MATCH    certUsage ;
    CERT_CHAIN_PARA     chainPara ;

    enhKeyUsage.cUsageIdentifier = 0 ;
    enhKeyUsage.rgpszUsageIdentifier = nullptr ;
    certUsage.dwType = USAGE_MATCH_TYPE_AND ;
    certUsage.Usage = enhKeyUsage ;
    chainPara.cbSize = sizeof( CERT_CHAIN_PARA ) ;
    chainPara.RequestedUsage = certUsage ;

    uno::Reference< XUnoTunnel > xCertTunnel( begin, uno::UNO_QUERY_THROW ) ;
    xcert = reinterpret_cast<X509Certificate_MSCryptImpl*>(xCertTunnel->getSomething( X509Certificate_MSCryptImpl::getUnoTunnelId() ));
    if( xcert == nullptr ) {
        throw uno::RuntimeException() ;
    }

    pCertContext = xcert->getMswcryCert() ;

    pChainContext = nullptr ;

    BOOL bChain = FALSE;
    if( pCertContext != nullptr )
    {
        HCERTSTORE hAdditionalStore = nullptr;
        HCERTSTORE hCollectionStore = nullptr;
        if (m_hCertStore && m_hKeyStore)
        {
            //Merge m_hCertStore and m_hKeyStore into one store.
            hCollectionStore = CertOpenStore(
                CERT_STORE_PROV_COLLECTION ,
                0 ,
                NULL ,
                0 ,
                nullptr
                ) ;
            if (hCollectionStore != nullptr)
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
        if (hAdditionalStore == nullptr && m_hCertStore)
            hAdditionalStore = m_hCertStore;
        else if (hAdditionalStore == nullptr && m_hKeyStore)
            hAdditionalStore = m_hKeyStore;
        else
            hAdditionalStore = nullptr;

        //CertGetCertificateChain searches by default in MY, CA, ROOT and TRUST
        bChain = CertGetCertificateChain(
            nullptr ,
            pCertContext ,
            nullptr , //use current system time
            hAdditionalStore,
            &chainPara ,
            CERT_CHAIN_REVOCATION_CHECK_CHAIN | CERT_CHAIN_TIMESTAMP_TIME ,
            nullptr ,
            &pChainContext);
        if (!bChain)
            pChainContext = nullptr;

        //Close the additional store
        CertCloseStore(hCollectionStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }

    if(bChain &&  pChainContext != nullptr && pChainContext->cChain > 0 )
    {
        PCCERT_CONTEXT pCertInChain ;
        PCERT_SIMPLE_CHAIN pCertChain ;
        X509Certificate_MSCryptImpl* pCert ;

        pCertChain = pChainContext->rgpChain[0] ;
        if( pCertChain->cElement ) {
            uno::Sequence< uno::Reference< XCertificate > > xCertChain( pCertChain->cElement ) ;

            for( unsigned int i = 0 ; i < pCertChain->cElement ; i ++ ) {
                if( pCertChain->rgpElement[i] )
                    pCertInChain = pCertChain->rgpElement[i]->pCertContext ;
                else
                    pCertInChain = nullptr ;

                if( pCertInChain != nullptr ) {
                    pCert = MswcryCertContextToXCert( pCertInChain ) ;
                    if( pCert != nullptr )
                        xCertChain[i] = pCert ;
                }
            }

            CertFreeCertificateChain( pChainContext ) ;
            pChainContext = nullptr ;

            return xCertChain ;
        }
    }
    if (pChainContext)
        CertFreeCertificateChain(pChainContext);

    return uno::Sequence< uno::Reference < XCertificate > >();
}

uno::Reference< XCertificate > SecurityEnvironment_MSCryptImpl::createCertificateFromRaw( const uno::Sequence< sal_Int8 >& rawCertificate ) {
    X509Certificate_MSCryptImpl* xcert ;

    if( rawCertificate.getLength() > 0 ) {
        xcert = new X509Certificate_MSCryptImpl() ;
        xcert->setRawCert( rawCertificate ) ;
    } else {
        xcert = nullptr ;
    }

    return xcert ;
}

uno::Reference< XCertificate > SecurityEnvironment_MSCryptImpl::createCertificateFromAscii( const OUString& asciiCertificate ) {

    OString oscert = OUStringToOString( asciiCertificate , RTL_TEXTENCODING_ASCII_US ) ;

    xmlChar* chCert = xmlStrndup( reinterpret_cast<const xmlChar*>(oscert.getStr()), static_cast<int>(oscert.getLength()) ) ;

    xmlSecSize certSize = xmlSecBase64Decode( chCert, chCert, xmlStrlen( chCert ) ) ;

    uno::Sequence< sal_Int8 > rawCert( certSize ) ;
    for( xmlSecSize i = 0 ; i < certSize ; i ++ )
        rawCert[i] = *( chCert + i ) ;

    xmlFree( chCert ) ;

    return createCertificateFromRaw( rawCert ) ;
}


static HCERTSTORE getCertStoreForIntermediatCerts(
    const uno::Sequence< uno::Reference< css::security::XCertificate > >& seqCerts)
{
    HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, 0, nullptr);
    if (store == nullptr)
        return nullptr;

    for (int i = 0; i < seqCerts.getLength(); i++)
    {
        SAL_INFO("xmlsecurity.xmlsec", "Added temporary certificate: " << seqCerts[i]->getSubjectName());

        uno::Sequence<sal_Int8> data = seqCerts[i]->getEncoded();
        PCCERT_CONTEXT cert = CertCreateCertificateContext(
            X509_ASN_ENCODING, reinterpret_cast<const BYTE*>(&data[0]), data.getLength());
        //Adding the certificate creates a copy and not just increases the ref count
        //Therefore we free later the certificate that we now add
        CertAddCertificateContextToStore(store, cert, CERT_STORE_ADD_ALWAYS, nullptr);
        CertFreeCertificateContext(cert);
    }
    return store;
}

//We return only valid or invalid, as long as the API documentation expresses
//explicitly that all validation steps are carried out even if one or several
//errors occur. See also
//http://wiki.openoffice.org/wiki/Certificate_Path_Validation#Validation_status
sal_Int32 SecurityEnvironment_MSCryptImpl::verifyCertificate(
    const uno::Reference< css::security::XCertificate >& aCert,
    const uno::Sequence< uno::Reference< css::security::XCertificate > >& seqCerts)
{
    sal_Int32 validity = 0;
    PCCERT_CHAIN_CONTEXT pChainContext = nullptr;
    PCCERT_CONTEXT pCertContext = nullptr;

    uno::Reference< XUnoTunnel > xCertTunnel( aCert, uno::UNO_QUERY_THROW ) ;

    SAL_INFO("xmlsecurity.xmlsec", "Start verification of certificate: " << aCert->getSubjectName());

    auto xcert = reinterpret_cast<const X509Certificate_MSCryptImpl*>
            (xCertTunnel->getSomething( X509Certificate_MSCryptImpl::getUnoTunnelId() ));
    if( xcert == nullptr ) {
        throw uno::RuntimeException() ;
    }

    pCertContext = xcert->getMswcryCert() ;

    CERT_ENHKEY_USAGE   enhKeyUsage ;
    CERT_USAGE_MATCH    certUsage ;
    CERT_CHAIN_PARA     chainPara ;
    memset(&chainPara, 0, sizeof(CERT_CHAIN_PARA));

    //Prepare parameter for CertGetCertificateChain
    enhKeyUsage.cUsageIdentifier = 0 ;
    enhKeyUsage.rgpszUsageIdentifier = nullptr ;
    certUsage.dwType = USAGE_MATCH_TYPE_AND ;
    certUsage.Usage = enhKeyUsage ;
    chainPara.cbSize = sizeof( CERT_CHAIN_PARA ) ;
    chainPara.RequestedUsage = certUsage ;


    HCERTSTORE hCollectionStore = nullptr;
    HCERTSTORE hIntermediateCertsStore = nullptr;
    BOOL bChain = FALSE;
    if( pCertContext != nullptr )
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
            nullptr
            ) ;
        if (hCollectionStore != nullptr)
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
        SAL_INFO("xmlsecurity.xmlsec", "Verifying cert using revocation information.");
        bChain = CertGetCertificateChain(
            nullptr ,
            pCertContext ,
            nullptr , //use current system time
            hCollectionStore,
            &chainPara ,
            CERT_CHAIN_REVOCATION_CHECK_CHAIN | CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,
            nullptr ,
            &pChainContext);

        if (bChain && pChainContext->cChain > 0)
        {
            SAL_INFO("xmlsecurity.xmlsec", "Overall error status (all chains):");
            traceTrustStatus(pChainContext->TrustStatus.dwErrorStatus);
            //highest quality chains come first
            PCERT_SIMPLE_CHAIN pSimpleChain = pChainContext->rgpChain[0];
            SAL_INFO("xmlsecurity.xmlsec", "Error status of first chain:");
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
                    //clear about if all other tests are performed if an error occurs,
                    //we test again, without requiring revocation checking.
                    CertFreeCertificateChain(pChainContext);
                    pChainContext = nullptr;
                    SAL_INFO("xmlsecurity.xmlsec", "Checking again but without requiring revocation information.");
                    bChain = CertGetCertificateChain(
                        nullptr ,
                        pCertContext ,
                        nullptr , //use current system time
                        hCollectionStore,
                        &chainPara ,
                        0,
                        nullptr ,
                        &pChainContext);
                    if (bChain
                        && pChainContext->cChain > 0
                        && pChainContext->rgpChain[0]->TrustStatus.dwErrorStatus == CERT_TRUST_NO_ERROR)
                    {
                        SAL_INFO("xmlsecurity.xmlsec", "Certificate is valid.");
                        validity = css::security::CertificateValidity::VALID;
                    }
                    else
                    {
                        SAL_INFO("xmlsecurity.xmlsec", "Certificate is invalid.");
                    }
                }
                else
                {
                    //valid and revocation information available
                    SAL_INFO("xmlsecurity.xmlsec", "Certificate is valid.");
                    validity = css::security::CertificateValidity::VALID;
                }
            }
            else
            {
                //invalid
                SAL_INFO("xmlsecurity.xmlsec", "Certificate is invalid.");
                validity = css::security::CertificateValidity::INVALID ;
            }
        }
        else
        {
            SAL_INFO("xmlsecurity.xmlsec", "CertGetCertificateChaine failed.");
        }
    }

    if (pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
        pChainContext = nullptr;
    }

    //Close the additional store, do not destroy the contained certs
    CertCloseStore(hCollectionStore, CERT_CLOSE_STORE_CHECK_FLAG);
    //Close the temporary store containing the intermediate certificates and make
    //sure all certificates are deleted.
    CertCloseStore(hIntermediateCertsStore, CERT_CLOSE_STORE_CHECK_FLAG);

    return validity ;
}

sal_Int32 SecurityEnvironment_MSCryptImpl::getCertificateCharacters( const css::uno::Reference< css::security::XCertificate >& aCert ) {
    sal_Int32 characters ;
    PCCERT_CONTEXT pCertContext ;
    const X509Certificate_MSCryptImpl* xcert ;

    uno::Reference< XUnoTunnel > xCertTunnel( aCert, uno::UNO_QUERY_THROW ) ;
    xcert = reinterpret_cast<X509Certificate_MSCryptImpl*>(xCertTunnel->getSomething( X509Certificate_MSCryptImpl::getUnoTunnelId() ));
    if( xcert == nullptr ) {
        throw uno::RuntimeException() ;
    }

    pCertContext = xcert->getMswcryCert() ;

    characters = 0x00000000 ;

    //Firstly, make sentence whether or not the cert is self-signed.
    if( CertCompareCertificateName( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &(pCertContext->pCertInfo->Subject), &(pCertContext->pCertInfo->Issuer) ) ) {
        characters |= css::security::CertificateCharacters::SELF_SIGNED ;
    } else {
        characters &= ~ css::security::CertificateCharacters::SELF_SIGNED ;
    }

    //Secondly, make sentence whether or not the cert has a private key.
    {
        BOOL    fCallerFreeProv ;
        DWORD   dwKeySpec ;
        NCRYPT_KEY_HANDLE hKey = 0;
        DWORD dwFlags = CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG;
        HCRYPTPROV_OR_NCRYPT_KEY_HANDLE* phCryptProvOrNCryptKey = &hKey;
        if( CryptAcquireCertificatePrivateKey( pCertContext ,
                   dwFlags,
                   nullptr ,
                   phCryptProvOrNCryptKey,
                   &dwKeySpec,
                   &fCallerFreeProv )
        ) {
            characters |=  css::security::CertificateCharacters::HAS_PRIVATE_KEY ;

            if (hKey && fCallerFreeProv)
                NCryptFreeObject(hKey);
        } else {
            characters &= ~ css::security::CertificateCharacters::HAS_PRIVATE_KEY ;
        }
    }
    return characters ;
}

void SecurityEnvironment_MSCryptImpl::enableDefaultCrypt( bool enable ) {
    m_bEnableDefault = enable ;
}

bool SecurityEnvironment_MSCryptImpl::defaultEnabled() {
    return m_bEnableDefault ;
}

static X509Certificate_MSCryptImpl* MswcryCertContextToXCert( PCCERT_CONTEXT cert )
{
    X509Certificate_MSCryptImpl* xcert ;

    if( cert != nullptr ) {
        xcert = new X509Certificate_MSCryptImpl() ;
        xcert->setMswcryCert( cert ) ;
    } else {
        xcert = nullptr ;
    }

    return xcert ;
}

OUString SecurityEnvironment_MSCryptImpl::getSecurityEnvironmentInformation()
{
    return OUString("Microsoft Crypto API");
}

xmlSecKeysMngrPtr SecurityEnvironment_MSCryptImpl::createKeysManager() {

    xmlSecKeysMngrPtr pKeysMngr = nullptr ;

    /*-
     * The following lines is based on the of xmlsec-mscrypto crypto engine
     */
    pKeysMngr = xmlsecurity::MSCryptoAppliedKeysMngrCreate() ;
    if( pKeysMngr == nullptr )
        throw uno::RuntimeException() ;

    /*-
     * Adopt system default certificate store.
     */
    if( defaultEnabled() ) {
        //Add system key store into the keys manager.
        m_hMySystemStore = CertOpenSystemStoreW( 0, L"MY" ) ;
        if( m_hMySystemStore != nullptr ) {
            if( xmlsecurity::MSCryptoAppliedKeysMngrAdoptKeyStore( pKeysMngr, m_hMySystemStore ) < 0 ) {
                CertCloseStore( m_hMySystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                m_hMySystemStore = nullptr;
                throw uno::RuntimeException() ;
            }
            m_hMySystemStore = nullptr;
        }

        //Add system root store into the keys manager.
        m_hRootSystemStore = CertOpenSystemStoreW( 0, L"Root" ) ;
        if( m_hRootSystemStore != nullptr ) {
            if( xmlsecurity::MSCryptoAppliedKeysMngrAdoptTrustedStore( pKeysMngr, m_hRootSystemStore ) < 0 ) {
                CertCloseStore( m_hRootSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                m_hRootSystemStore = nullptr;
                throw uno::RuntimeException() ;
            }
            m_hRootSystemStore = nullptr;
        }

        //Add system trusted store into the keys manager.
        m_hTrustSystemStore = CertOpenSystemStoreW( 0, L"Trust" ) ;
        if( m_hTrustSystemStore != nullptr ) {
            if( xmlsecurity::MSCryptoAppliedKeysMngrAdoptUntrustedStore( pKeysMngr, m_hTrustSystemStore ) < 0 ) {
                CertCloseStore( m_hTrustSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                m_hTrustSystemStore = nullptr;
                throw uno::RuntimeException() ;
            }
            m_hTrustSystemStore = nullptr;
        }

        //Add system CA store into the keys manager.
        m_hCaSystemStore = CertOpenSystemStoreW( 0, L"CA" ) ;
        if( m_hCaSystemStore != nullptr ) {
            if( xmlsecurity::MSCryptoAppliedKeysMngrAdoptUntrustedStore( pKeysMngr, m_hCaSystemStore ) < 0 ) {
                CertCloseStore( m_hCaSystemStore, CERT_CLOSE_STORE_CHECK_FLAG ) ;
                m_hCaSystemStore = nullptr;
                throw uno::RuntimeException() ;
            }
            m_hCaSystemStore = nullptr;
        }
    }

    return pKeysMngr ;
}
void SecurityEnvironment_MSCryptImpl::destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) {
    if( pKeysMngr != nullptr ) {
        xmlSecKeysMngrDestroy( pKeysMngr ) ;
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_SecurityEnvironment_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SecurityEnvironment_MSCryptImpl(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
