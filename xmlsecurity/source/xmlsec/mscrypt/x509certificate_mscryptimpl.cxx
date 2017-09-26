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

#include <sal/config.h>
#include <comphelper/servicehelper.hxx>
#include <comphelper/windowserrorstring.hxx>
#include "x509certificate_mscryptimpl.hxx"
#include "certificateextension_xmlsecimpl.hxx"
#include "sanextension_mscryptimpl.hxx"

#include "oid.hxx"

#include <rtl/locale.h>
#include <osl/nlsupport.h>
#include <osl/process.h>

#include <memory>
#include <utility>
#include <vector>
#include <tools/time.hxx>

// Needed only for Windows XP.
#ifndef CERT_SHA256_HASH_PROP_ID
#define CERT_SHA256_HASH_PROP_ID 107
#endif

using namespace com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::security ;

using ::com::sun::star::security::XCertificate ;
using ::com::sun::star::util::DateTime ;

/*Returns the index within rRawString where sTypeName starts and where it ends.
    The starting index is pair.first. The ending index in pair.second points
    one char after the last character of the type.
    sTypeName can be
    "S" or "CN" (without ""). Do not use spaces at the beginning of the type name.
    If the type name is not found then pair.first and pair.second are -1.
*/
std::pair< sal_Int32, sal_Int32 >
findTypeInDN(const OUString& rRawString, const OUString& sTypeName)
{
    std::pair< sal_Int32, sal_Int32 > retVal;
    bool bInEscape = false;
    bool bInValue = false;
    bool bFound = false;
    sal_Int32 nTypeNameStart = 0;
    sal_Int32 length = rRawString.getLength();

    for (sal_Int32 i = 0; i < length; i++)
    {
        sal_Unicode c = rRawString[i];

        if (c == '=')
        {
            if (! bInValue)
            {
                OUString sType = rRawString.copy(nTypeNameStart, i - nTypeNameStart);
                sType = sType.trim();
                if (sType.equalsIgnoreAsciiCase(sTypeName))
                {
                    bFound = true;
                    break;
                }
            }
        }
        else if (c == '"')
        {
            if (!bInEscape)
            {
                //If this is the quote is the first of the couple which enclose the
                //whole value, because the value contains special characters
                //then we just drop it. That is, this character must be followed by
                //a character which is not '"'.
                if ( i + 1 < length && rRawString[i+1] == '"')
                    bInEscape = true;
                else
                    bInValue = !bInValue; //value is enclosed in " "
            }
            else
            {
                //This quote is escaped by a preceding quote and therefore is
                //part of the value
                bInEscape = false;
            }
        }
        else if (c == ',' || c == '+')
        {
            //The comma separate the attribute value pairs.
            //If the comma is not part of a value (the value would then be enclosed in '"'),
            //then we have reached the end of the value
            if (!bInValue)
            {
                //The next char is the start of the new type
                nTypeNameStart = i + 1;
            }
        }
    }

    //Found the Type Name, but there can still be spaces after the last comma
    //and the beginning of the type.
    if (bFound)
    {
        while (true)
        {
            sal_Unicode c = rRawString[nTypeNameStart];
            if (c != ' ' && c != '\t')
                //found
                break;
            nTypeNameStart ++;
        }
        // search end (one after last letter)
        sal_Int32 nTypeNameEnd = nTypeNameStart;
        nTypeNameEnd++;
        while (true)
        {
             sal_Unicode c = rRawString[nTypeNameEnd];
             if (c == ' ' || c == '\t' || c == '=')
                 break;
             nTypeNameEnd++;
        }
        retVal = std::make_pair(nTypeNameStart, nTypeNameEnd);
    }
    else
    {
        retVal = std::make_pair(-1, -1);
    }
    return retVal;
}


/*
  MS Crypto uses the 'S' tag (equal to the 'ST' tag in NSS), but the NSS can't recognise
  it, so the 'S' tag should be changed to 'ST' tag. However I am not sure if this is necessary
  anymore, because we provide always the signers certificate when signing. So libmlsec can find
  the private key based on the provided certificate (X509Certificate element) and does not need
  the issuer name (X509IssuerName element). The issuer name in the xml signature has also no
  effect for the signature nor the certificate validation.
  In many RFCs, for example 4519, on speaks of 'ST'. However, the certificate does not contain
  strings for type names. Instead it uses OIDs.
 */

OUString replaceTagSWithTagST(OUString const & oldDN)
{
    std::pair<sal_Int32, sal_Int32 > pairIndex = findTypeInDN(oldDN, "S");

    if (pairIndex.first != -1)
    {
        return oldDN.copy(0, pairIndex.first)+"ST"+oldDN.copy(pairIndex.second);
    }
    return oldDN;
}
/* end */

X509Certificate_MSCryptImpl::X509Certificate_MSCryptImpl() :
    m_pCertContext( nullptr )
{
}

X509Certificate_MSCryptImpl::~X509Certificate_MSCryptImpl() {
    if( m_pCertContext != nullptr ) {
        CertFreeCertificateContext( m_pCertContext ) ;
    }
}

//Methods from XCertificate
sal_Int16 SAL_CALL X509Certificate_MSCryptImpl::getVersion() {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr ) {
        return ( char )m_pCertContext->pCertInfo->dwVersion ;
    } else {
        return -1 ;
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getSerialNumber() {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr ) {
        Sequence< sal_Int8 > serial( m_pCertContext->pCertInfo->SerialNumber.cbData ) ;
        for( unsigned int i = 0 ; i < m_pCertContext->pCertInfo->SerialNumber.cbData ; i ++ )
            serial[i] = *( m_pCertContext->pCertInfo->SerialNumber.pbData + m_pCertContext->pCertInfo->SerialNumber.cbData - i - 1 ) ;

        return serial ;
    } else {
        return Sequence< sal_Int8 >();
    }
}

OUString SAL_CALL X509Certificate_MSCryptImpl::getIssuerName() {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr ) {
        DWORD cchIssuer = CertNameToStrW(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
            &( m_pCertContext->pCertInfo->Issuer ),
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
            nullptr, 0
        ) ;

        // Here the cbIssuer count the last 0x00 , take care.
        if( cchIssuer != 0 ) {
            auto issuer = std::unique_ptr<wchar_t>(new wchar_t[ cchIssuer ]);

            cchIssuer = CertNameToStrW(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                &( m_pCertContext->pCertInfo->Issuer ),
                CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
                issuer.get(), cchIssuer
            ) ;

            if( cchIssuer <= 0 ) {
                throw RuntimeException() ;
            }

            if(issuer.get()[cchIssuer -1] == 0) cchIssuer--; //delimit the last 0x00;
            OUString xIssuer(SAL_U(issuer.get()), cchIssuer) ;

            return replaceTagSWithTagST(xIssuer);
        } else {
            return OUString() ;
        }
    } else {
        return OUString() ;
    }
}

OUString SAL_CALL X509Certificate_MSCryptImpl::getSubjectName()
{
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr )
    {
        DWORD cchSubject = CertNameToStrW(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
            &( m_pCertContext->pCertInfo->Subject ),
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
            nullptr, 0
        ) ;

        if( cchSubject != 0 )
        {
            auto subject = std::unique_ptr<wchar_t>(new wchar_t[ cchSubject ]);

            cchSubject = CertNameToStrW(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                &( m_pCertContext->pCertInfo->Subject ),
                CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
                subject.get(), cchSubject
            ) ;

            if( cchSubject <= 0 ) {
                throw RuntimeException() ;
            }

            OUString xSubject(SAL_U(subject.get()));

            return replaceTagSWithTagST(xSubject);
        } else
        {
            return OUString() ;
        }
    }
    else
    {
        return OUString() ;
    }
}

css::util::DateTime SAL_CALL X509Certificate_MSCryptImpl::getNotValidBefore() {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr ) {
        SYSTEMTIME explTime ;
        DateTime dateTime ;
        FILETIME localFileTime;

        if (FileTimeToLocalFileTime(&( m_pCertContext->pCertInfo->NotBefore ), &localFileTime))
        {
            if( FileTimeToSystemTime( &localFileTime, &explTime ) ) {
                //Convert the time to readable local time
                dateTime.NanoSeconds = explTime.wMilliseconds * ::tools::Time::nanoPerMilli ;
                dateTime.Seconds = explTime.wSecond ;
                dateTime.Minutes = explTime.wMinute ;
                dateTime.Hours = explTime.wHour ;
                dateTime.Day = explTime.wDay ;
                dateTime.Month = explTime.wMonth ;
                dateTime.Year = explTime.wYear ;
            }
        }

        return dateTime ;
    } else {
        return DateTime() ;
    }
}

css::util::DateTime SAL_CALL X509Certificate_MSCryptImpl::getNotValidAfter() {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr ) {
        SYSTEMTIME explTime ;
        DateTime dateTime ;
        FILETIME localFileTime;

        if (FileTimeToLocalFileTime(&( m_pCertContext->pCertInfo->NotAfter ), &localFileTime))
        {
            if( FileTimeToSystemTime( &localFileTime, &explTime ) ) {
                //Convert the time to readable local time
                dateTime.NanoSeconds = explTime.wMilliseconds * ::tools::Time::nanoPerMilli ;
                dateTime.Seconds = explTime.wSecond ;
                dateTime.Minutes = explTime.wMinute ;
                dateTime.Hours = explTime.wHour ;
                dateTime.Day = explTime.wDay ;
                dateTime.Month = explTime.wMonth ;
                dateTime.Year = explTime.wYear ;
            }
        }

        return dateTime ;
    } else {
        return DateTime() ;
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getIssuerUniqueID() {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr ) {
        Sequence< sal_Int8 > issuerUid( m_pCertContext->pCertInfo->IssuerUniqueId.cbData ) ;
        for( unsigned int i = 0 ; i < m_pCertContext->pCertInfo->IssuerUniqueId.cbData; i ++ )
            issuerUid[i] = *( m_pCertContext->pCertInfo->IssuerUniqueId.pbData + i ) ;

        return issuerUid ;
    } else {
        return Sequence< sal_Int8 >();
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getSubjectUniqueID() {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr ) {
        Sequence< sal_Int8 > subjectUid( m_pCertContext->pCertInfo->SubjectUniqueId.cbData ) ;
        for( unsigned int i = 0 ; i < m_pCertContext->pCertInfo->SubjectUniqueId.cbData; i ++ )
            subjectUid[i] = *( m_pCertContext->pCertInfo->SubjectUniqueId.pbData + i ) ;

        return subjectUid ;
    } else {
        return Sequence< sal_Int8 >();
    }
}

css::uno::Sequence< css::uno::Reference< css::security::XCertificateExtension > > SAL_CALL X509Certificate_MSCryptImpl::getExtensions() {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr && m_pCertContext->pCertInfo->cExtension != 0 ) {
        CertificateExtension_XmlSecImpl* xExtn ;
        Sequence< Reference< XCertificateExtension > > xExtns( m_pCertContext->pCertInfo->cExtension ) ;

        for( unsigned int i = 0; i < m_pCertContext->pCertInfo->cExtension; i++ ) {
            CERT_EXTENSION* pExtn = &(m_pCertContext->pCertInfo->rgExtension[i]) ;


            OUString objId = OUString::createFromAscii( pExtn->pszObjId );

            if ( objId == "2.5.29.17" )
                xExtn = reinterpret_cast<CertificateExtension_XmlSecImpl*>(new SanExtensionImpl());
            else
                xExtn = new CertificateExtension_XmlSecImpl;
            if( xExtn == nullptr )
                throw RuntimeException() ;

            xExtn->setCertExtn( pExtn->Value.pbData, pExtn->Value.cbData, reinterpret_cast<unsigned char*>(pExtn->pszObjId), strlen( pExtn->pszObjId ), pExtn->fCritical ) ;

            xExtns[i] = xExtn ;
        }

        return xExtns ;
    } else {
        return Sequence< Reference< XCertificateExtension > >();
    }
}

css::uno::Reference< css::security::XCertificateExtension > SAL_CALL X509Certificate_MSCryptImpl::findCertificateExtension( const css::uno::Sequence< sal_Int8 >& /*oid*/ ) {
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr && m_pCertContext->pCertInfo->cExtension != 0 ) {
        CertificateExtension_XmlSecImpl* xExtn ;

        xExtn = nullptr ;
        for( unsigned int i = 0; i < m_pCertContext->pCertInfo->cExtension; i++ ) {
            CERT_EXTENSION* pExtn = &( m_pCertContext->pCertInfo->rgExtension[i] ) ;

            //TODO: Compare the oid
            if( false ) {
                xExtn = new CertificateExtension_XmlSecImpl;
                if( xExtn == nullptr )
                    throw RuntimeException() ;

                xExtn->setCertExtn( pExtn->Value.pbData, pExtn->Value.cbData, reinterpret_cast<unsigned char*>(pExtn->pszObjId), strlen( pExtn->pszObjId ), pExtn->fCritical ) ;
            }
        }

        return xExtn ;
    } else {
        return nullptr ;
    }
}


css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getEncoded() {
    if( m_pCertContext != nullptr && m_pCertContext->cbCertEncoded > 0 ) {
        Sequence< sal_Int8 > rawCert( m_pCertContext->cbCertEncoded ) ;

        for( unsigned int i = 0 ; i < m_pCertContext->cbCertEncoded ; i ++ )
            rawCert[i] = *( m_pCertContext->pbCertEncoded + i ) ;

        return rawCert ;
    } else {
        return Sequence< sal_Int8 >();
    }
}

//Helper methods
void X509Certificate_MSCryptImpl::setMswcryCert( const CERT_CONTEXT* cert ) {
    if( m_pCertContext != nullptr ) {
        CertFreeCertificateContext( m_pCertContext ) ;
        m_pCertContext = nullptr ;
    }

    if( cert != nullptr ) {
        m_pCertContext = CertDuplicateCertificateContext( cert ) ;
    }
}

const CERT_CONTEXT* X509Certificate_MSCryptImpl::getMswcryCert() const {
    if( m_pCertContext != nullptr ) {
        return m_pCertContext ;
    } else {
        return nullptr ;
    }
}

void X509Certificate_MSCryptImpl::setRawCert( Sequence< sal_Int8 > const & rawCert ) {
    if( m_pCertContext != nullptr ) {
        CertFreeCertificateContext( m_pCertContext ) ;
        m_pCertContext = nullptr ;
    }

    if( rawCert.getLength() != 0 ) {
        m_pCertContext = CertCreateCertificateContext( X509_ASN_ENCODING, reinterpret_cast<const sal_uInt8*>(&rawCert[0]), rawCert.getLength() ) ;
    }
}

/* XUnoTunnel */
sal_Int64 SAL_CALL X509Certificate_MSCryptImpl::getSomething( const Sequence< sal_Int8 >& aIdentifier ) {
    if( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return reinterpret_cast<sal_Int64>(this);
    }
    return 0 ;
}

/* XUnoTunnel extension */

namespace
{
    class theX509Certificate_MSCryptImplUnoTunnelId  : public rtl::Static< UnoTunnelIdInit, theX509Certificate_MSCryptImplUnoTunnelId > {};
}

const Sequence< sal_Int8>& X509Certificate_MSCryptImpl::getUnoTunnelId() {
    return theX509Certificate_MSCryptImplUnoTunnelId::get().getSeq();
}

/* XUnoTunnel extension */
X509Certificate_MSCryptImpl* X509Certificate_MSCryptImpl::getImplementation( const Reference< XInterface >& rObj ) {
    Reference< XUnoTunnel > xUT( rObj , UNO_QUERY ) ;
    if( xUT.is() ) {
        return reinterpret_cast<X509Certificate_MSCryptImpl*>(xUT->getSomething( getUnoTunnelId() ));
    } else
        return nullptr ;
}

OUString findOIDDescription(char const *oid)
{
    OUString ouOID = OUString::createFromAscii( oid );
    for (int i=0; i<nOID; i++)
    {
        OUString item = OUString::createFromAscii( OIDs[i].oid );
        if (ouOID == item)
        {
            return OUString::createFromAscii( OIDs[i].desc );
        }
    }

    return OUString() ;
}

css::uno::Sequence< sal_Int8 > getThumbprint(const CERT_CONTEXT* pCertContext, DWORD dwPropId)
{
    if( pCertContext != nullptr )
    {
        DWORD cbData = dwPropId == CERT_SHA256_HASH_PROP_ID ? 32 : 20;
        unsigned char fingerprint[32];
        if (CertGetCertificateContextProperty(pCertContext, dwPropId, fingerprint, &cbData))
        {
            Sequence< sal_Int8 > thumbprint( cbData ) ;
            for( unsigned int i = 0 ; i < cbData ; i ++ )
            {
                thumbprint[i] = fingerprint[i];
            }

            return thumbprint;
        }
        else
        {
            DWORD e = GetLastError();
            cbData = e;
        }
    }

    return Sequence< sal_Int8 >();
}

OUString SAL_CALL X509Certificate_MSCryptImpl::getSubjectPublicKeyAlgorithm()
{
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr )
    {
        CRYPT_ALGORITHM_IDENTIFIER algorithm = m_pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm;
        return findOIDDescription( algorithm.pszObjId ) ;
    }
    else
    {
        return OUString() ;
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getSubjectPublicKeyValue()
{
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr )
    {
        CRYPT_BIT_BLOB publicKey = m_pCertContext->pCertInfo->SubjectPublicKeyInfo.PublicKey;

        Sequence< sal_Int8 > key( publicKey.cbData ) ;
        for( unsigned int i = 0 ; i < publicKey.cbData ; i++ )
        {
            key[i] = *(publicKey.pbData + i) ;
        }

        return key;
    }
    else
    {
        return Sequence< sal_Int8 >();
    }
}

OUString SAL_CALL X509Certificate_MSCryptImpl::getSignatureAlgorithm()
{
    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr )
    {
        CRYPT_ALGORITHM_IDENTIFIER algorithm = m_pCertContext->pCertInfo->SignatureAlgorithm;
        return findOIDDescription( algorithm.pszObjId ) ;
    }
    else
    {
        return OUString() ;
    }
}

uno::Sequence<sal_Int8> X509Certificate_MSCryptImpl::getSHA256Thumbprint()
{
    return getThumbprint(m_pCertContext, CERT_SHA256_HASH_PROP_ID);
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getSHA1Thumbprint()
{
    return getThumbprint(m_pCertContext, CERT_SHA1_HASH_PROP_ID);
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getMD5Thumbprint()
{
    return getThumbprint(m_pCertContext, CERT_MD5_HASH_PROP_ID);
}

CertificateKind SAL_CALL X509Certificate_MSCryptImpl::getCertificateKind()
{
    return CertificateKind_X509;
}

sal_Int32 SAL_CALL X509Certificate_MSCryptImpl::getCertificateUsage(  )
{
    sal_Int32 usage =
        CERT_DATA_ENCIPHERMENT_KEY_USAGE |
        CERT_DIGITAL_SIGNATURE_KEY_USAGE |
        CERT_KEY_AGREEMENT_KEY_USAGE |
        CERT_KEY_CERT_SIGN_KEY_USAGE |
        CERT_KEY_ENCIPHERMENT_KEY_USAGE |
        CERT_NON_REPUDIATION_KEY_USAGE |
        CERT_OFFLINE_CRL_SIGN_KEY_USAGE;

    if( m_pCertContext != nullptr && m_pCertContext->pCertInfo != nullptr && m_pCertContext->pCertInfo->cExtension != 0 )
    {
        CERT_EXTENSION* pExtn = CertFindExtension(
            szOID_KEY_USAGE,
            m_pCertContext->pCertInfo->cExtension,
            m_pCertContext->pCertInfo->rgExtension);

        if (pExtn != nullptr)
        {
            DWORD length = 0;
            bool rc = CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_KEY_USAGE,
                pExtn->Value.pbData,
                pExtn->Value.cbData,
                0,
                nullptr,
                &length);

            if (!rc)
                SAL_WARN("xmlsecurity.xmlsec", "CryptDecodeObject failed: " << WindowsErrorString(GetLastError()));
            else
            {
                std::vector<char>buffer(length);

                rc = CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_KEY_USAGE,
                    pExtn->Value.pbData,
                    pExtn->Value.cbData,
                    0,
                    buffer.data(),
                    &length);

                CRYPT_BIT_BLOB *blob = reinterpret_cast<CRYPT_BIT_BLOB*>(buffer.data());
                if (!rc)
                    SAL_WARN("xmlsecurity.xmlsec", "CryptDecodeObject failed: " << WindowsErrorString(GetLastError()));
                else if (blob->cbData == 1)
                    usage = blob->pbData[0];
                else
                    SAL_WARN("xmlsecurity.xmlsec", "CryptDecodeObject(X509_KEY_USAGE) returned unexpected amount of data: " << blob->cbData);
            }
        }
    }

    return usage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
