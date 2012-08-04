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
#include "x509certificate_mscryptimpl.hxx"
#include "certificateextension_xmlsecimpl.hxx"
#include "sanextension_mscryptimpl.hxx"

#include "oid.hxx"

#include <rtl/locale.h>
#include <osl/nlsupport.h>
#include <osl/process.h>
#include <utility>

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::security ;
using ::rtl::OUString ;

using ::com::sun::star::security::XCertificate ;
using ::com::sun::star::util::DateTime ;

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

/*Resturns the index withing rRawString where sTypeName starts and where it ends.
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

OUString replaceTagSWithTagST(OUString oldDN)
{
    std::pair<sal_Int32, sal_Int32 > pairIndex = findTypeInDN(oldDN, OUSTR("S"));

    if (pairIndex.first != -1)
    {
        OUString newDN = oldDN.copy(0, pairIndex.first);
        newDN += OUSTR("ST");
        newDN += oldDN.copy(pairIndex.second);
        return newDN;
    }
    return oldDN;
}
/* end */

X509Certificate_MSCryptImpl :: X509Certificate_MSCryptImpl() :
    m_pCertContext( NULL )
{
}

X509Certificate_MSCryptImpl :: ~X509Certificate_MSCryptImpl() {
    if( m_pCertContext != NULL ) {
        CertFreeCertificateContext( m_pCertContext ) ;
    }
}

//Methods from XCertificate
sal_Int16 SAL_CALL X509Certificate_MSCryptImpl :: getVersion() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        return ( char )m_pCertContext->pCertInfo->dwVersion ;
    } else {
        return -1 ;
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl :: getSerialNumber() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        Sequence< sal_Int8 > serial( m_pCertContext->pCertInfo->SerialNumber.cbData ) ;
        for( unsigned int i = 0 ; i < m_pCertContext->pCertInfo->SerialNumber.cbData ; i ++ )
            serial[i] = *( m_pCertContext->pCertInfo->SerialNumber.pbData + m_pCertContext->pCertInfo->SerialNumber.cbData - i - 1 ) ;

        return serial ;
    } else {
        return Sequence< sal_Int8 >();
    }
}

::rtl::OUString SAL_CALL X509Certificate_MSCryptImpl :: getIssuerName() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        char* issuer ;
        DWORD cbIssuer ;

        cbIssuer = CertNameToStr(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
            &( m_pCertContext->pCertInfo->Issuer ),
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
            NULL, 0
        ) ;

        // Here the cbIssuer count the last 0x00 , take care.
        if( cbIssuer != 0 ) {
            issuer = new char[ cbIssuer ] ;
            if( issuer == NULL )
                throw RuntimeException() ;

            cbIssuer = CertNameToStr(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                &( m_pCertContext->pCertInfo->Issuer ),
                CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
                issuer, cbIssuer
            ) ;

            if( cbIssuer <= 0 ) {
                delete [] issuer ;
                throw RuntimeException() ;
            }

            // for correct encoding
            sal_uInt16 encoding ;
            rtl_Locale *pLocale = NULL ;
            osl_getProcessLocale( &pLocale ) ;
            encoding = osl_getTextEncodingFromLocale( pLocale ) ;

            if(issuer[cbIssuer-1] == 0) cbIssuer--; //delimit the last 0x00;
            OUString xIssuer(issuer , cbIssuer ,encoding ) ;
            delete [] issuer ;

            return replaceTagSWithTagST(xIssuer);
        } else {
            return OUString() ;
        }
    } else {
        return OUString() ;
    }
}

::rtl::OUString SAL_CALL X509Certificate_MSCryptImpl :: getSubjectName() throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL )
    {
        wchar_t* subject ;
        DWORD cbSubject ;

        cbSubject = CertNameToStrW(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
            &( m_pCertContext->pCertInfo->Subject ),
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
            NULL, 0
        ) ;

        if( cbSubject != 0 )
        {
            subject = new wchar_t[ cbSubject ] ;
            if( subject == NULL )
                throw RuntimeException() ;

            cbSubject = CertNameToStrW(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                &( m_pCertContext->pCertInfo->Subject ),
                CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
                subject, cbSubject
            ) ;

            if( cbSubject <= 0 ) {
                delete [] subject ;
                throw RuntimeException() ;
            }

            OUString xSubject(reinterpret_cast<const sal_Unicode*>(subject));
            delete [] subject ;

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

::com::sun::star::util::DateTime SAL_CALL X509Certificate_MSCryptImpl :: getNotValidBefore() throw ( ::com::sun::star::uno::RuntimeException ) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        SYSTEMTIME explTime ;
        DateTime dateTime ;
        FILETIME localFileTime;

        if (FileTimeToLocalFileTime(&( m_pCertContext->pCertInfo->NotBefore ), &localFileTime))
        {
            if( FileTimeToSystemTime( &localFileTime, &explTime ) ) {
                //Convert the time to readable local time
                dateTime.HundredthSeconds = explTime.wMilliseconds / 100 ;
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

::com::sun::star::util::DateTime SAL_CALL X509Certificate_MSCryptImpl :: getNotValidAfter() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        SYSTEMTIME explTime ;
        DateTime dateTime ;
        FILETIME localFileTime;

        if (FileTimeToLocalFileTime(&( m_pCertContext->pCertInfo->NotAfter ), &localFileTime))
        {
            if( FileTimeToSystemTime( &localFileTime, &explTime ) ) {
                //Convert the time to readable local time
                dateTime.HundredthSeconds = explTime.wMilliseconds / 100 ;
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

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl :: getIssuerUniqueID() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        Sequence< sal_Int8 > issuerUid( m_pCertContext->pCertInfo->IssuerUniqueId.cbData ) ;
        for( unsigned int i = 0 ; i < m_pCertContext->pCertInfo->IssuerUniqueId.cbData; i ++ )
            issuerUid[i] = *( m_pCertContext->pCertInfo->IssuerUniqueId.pbData + i ) ;

        return issuerUid ;
    } else {
        return Sequence< sal_Int8 >();
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl :: getSubjectUniqueID() throw ( ::com::sun::star::uno::RuntimeException ) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        Sequence< sal_Int8 > subjectUid( m_pCertContext->pCertInfo->SubjectUniqueId.cbData ) ;
        for( unsigned int i = 0 ; i < m_pCertContext->pCertInfo->SubjectUniqueId.cbData; i ++ )
            subjectUid[i] = *( m_pCertContext->pCertInfo->SubjectUniqueId.pbData + i ) ;

        return subjectUid ;
    } else {
        return Sequence< sal_Int8 >();
    }
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > > SAL_CALL X509Certificate_MSCryptImpl :: getExtensions() throw ( ::com::sun::star::uno::RuntimeException ) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL && m_pCertContext->pCertInfo->cExtension != 0 ) {
        CertificateExtension_XmlSecImpl* xExtn ;
        CERT_EXTENSION* pExtn ;
        Sequence< Reference< XCertificateExtension > > xExtns( m_pCertContext->pCertInfo->cExtension ) ;

        for( unsigned int i = 0; i < m_pCertContext->pCertInfo->cExtension; i++ ) {
            pExtn = &(m_pCertContext->pCertInfo->rgExtension[i]) ;


            ::rtl::OUString objId = ::rtl::OUString::createFromAscii( pExtn->pszObjId );

            if ( objId == "2.5.29.17" )
                xExtn = (CertificateExtension_XmlSecImpl*) new SanExtensionImpl() ;
            else
                xExtn = new CertificateExtension_XmlSecImpl() ;
            if( xExtn == NULL )
                throw RuntimeException() ;

            xExtn->setCertExtn( pExtn->Value.pbData, pExtn->Value.cbData, ( unsigned char* )pExtn->pszObjId, strlen( pExtn->pszObjId ), sal::static_int_cast<sal_Bool>(pExtn->fCritical) ) ;

            xExtns[i] = xExtn ;
        }

        return xExtns ;
    } else {
        return Sequence< Reference< XCertificateExtension > >();
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > SAL_CALL X509Certificate_MSCryptImpl :: findCertificateExtension( const ::com::sun::star::uno::Sequence< sal_Int8 >& /*oid*/ ) throw (::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL && m_pCertContext->pCertInfo->cExtension != 0 ) {
        CertificateExtension_XmlSecImpl* xExtn ;
        CERT_EXTENSION* pExtn ;
        Sequence< Reference< XCertificateExtension > > xExtns( m_pCertContext->pCertInfo->cExtension ) ;

        xExtn = NULL ;
        for( unsigned int i = 0; i < m_pCertContext->pCertInfo->cExtension; i++ ) {
            pExtn = &( m_pCertContext->pCertInfo->rgExtension[i] ) ;

            //TODO: Compare the oid
            if( 0 ) {
                xExtn = new CertificateExtension_XmlSecImpl() ;
                if( xExtn == NULL )
                    throw RuntimeException() ;

                xExtn->setCertExtn( pExtn->Value.pbData, pExtn->Value.cbData, ( unsigned char* )pExtn->pszObjId, strlen( pExtn->pszObjId ), sal::static_int_cast<sal_Bool>(pExtn->fCritical) ) ;
            }
        }

        return xExtn ;
    } else {
        return NULL ;
    }
}


::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl :: getEncoded() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->cbCertEncoded > 0 ) {
        Sequence< sal_Int8 > rawCert( m_pCertContext->cbCertEncoded ) ;

        for( unsigned int i = 0 ; i < m_pCertContext->cbCertEncoded ; i ++ )
            rawCert[i] = *( m_pCertContext->pbCertEncoded + i ) ;

        return rawCert ;
    } else {
        return Sequence< sal_Int8 >();
    }
}

//Helper methods
void X509Certificate_MSCryptImpl :: setMswcryCert( const CERT_CONTEXT* cert ) {
    if( m_pCertContext != NULL ) {
        CertFreeCertificateContext( m_pCertContext ) ;
        m_pCertContext = NULL ;
    }

    if( cert != NULL ) {
        m_pCertContext = CertDuplicateCertificateContext( cert ) ;
    }
}

const CERT_CONTEXT* X509Certificate_MSCryptImpl :: getMswcryCert() const {
    if( m_pCertContext != NULL ) {
        return m_pCertContext ;
    } else {
        return NULL ;
    }
}

void X509Certificate_MSCryptImpl :: setRawCert( Sequence< sal_Int8 > rawCert ) throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL ) {
        CertFreeCertificateContext( m_pCertContext ) ;
        m_pCertContext = NULL ;
    }

    if( rawCert.getLength() != 0 ) {
        m_pCertContext = CertCreateCertificateContext( X509_ASN_ENCODING, ( const sal_uInt8* )&rawCert[0], rawCert.getLength() ) ;
    }
}

/* XUnoTunnel */
sal_Int64 SAL_CALL X509Certificate_MSCryptImpl :: getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw( RuntimeException ) {
    if( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return ( sal_Int64 )this ;
    }
    return 0 ;
}

/* XUnoTunnel extension */

namespace
{
    class theX509Certificate_MSCryptImplUnoTunnelId  : public rtl::Static< UnoTunnelIdInit, theX509Certificate_MSCryptImplUnoTunnelId > {};
}

const Sequence< sal_Int8>& X509Certificate_MSCryptImpl :: getUnoTunnelId() {
    return theX509Certificate_MSCryptImplUnoTunnelId::get().getSeq();
}

/* XUnoTunnel extension */
X509Certificate_MSCryptImpl* X509Certificate_MSCryptImpl :: getImplementation( const Reference< XInterface > xObj ) {
    Reference< XUnoTunnel > xUT( xObj , UNO_QUERY ) ;
    if( xUT.is() ) {
        return ( X509Certificate_MSCryptImpl* )xUT->getSomething( getUnoTunnelId() ) ;
    } else
        return NULL ;
}

::rtl::OUString findOIDDescription(char *oid)
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

::com::sun::star::uno::Sequence< sal_Int8 > getThumbprint(const CERT_CONTEXT* pCertContext, DWORD dwPropId)
{
    if( pCertContext != NULL )
    {
        DWORD cbData = 20;
        unsigned char fingerprint[20];
        if (CertGetCertificateContextProperty(pCertContext, dwPropId, (void*)fingerprint, &cbData))
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

::rtl::OUString SAL_CALL X509Certificate_MSCryptImpl::getSubjectPublicKeyAlgorithm()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL )
    {
        CRYPT_ALGORITHM_IDENTIFIER algorithm = m_pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm;
        return findOIDDescription( algorithm.pszObjId ) ;
    }
    else
    {
        return OUString() ;
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getSubjectPublicKeyValue()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL )
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

::rtl::OUString SAL_CALL X509Certificate_MSCryptImpl::getSignatureAlgorithm()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL )
    {
        CRYPT_ALGORITHM_IDENTIFIER algorithm = m_pCertContext->pCertInfo->SignatureAlgorithm;
        return findOIDDescription( algorithm.pszObjId ) ;
    }
    else
    {
        return OUString() ;
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getSHA1Thumbprint()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    return getThumbprint(m_pCertContext, CERT_SHA1_HASH_PROP_ID);
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getMD5Thumbprint()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    return getThumbprint(m_pCertContext, CERT_MD5_HASH_PROP_ID);
}

sal_Int32 SAL_CALL X509Certificate_MSCryptImpl::getCertificateUsage(  )
    throw ( ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 usage =
        CERT_DATA_ENCIPHERMENT_KEY_USAGE |
        CERT_DIGITAL_SIGNATURE_KEY_USAGE |
        CERT_KEY_AGREEMENT_KEY_USAGE |
        CERT_KEY_CERT_SIGN_KEY_USAGE |
        CERT_KEY_ENCIPHERMENT_KEY_USAGE |
        CERT_NON_REPUDIATION_KEY_USAGE |
        CERT_OFFLINE_CRL_SIGN_KEY_USAGE;

    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL && m_pCertContext->pCertInfo->cExtension != 0 )
    {
        CERT_EXTENSION* pExtn = CertFindExtension(
            szOID_KEY_USAGE,
            m_pCertContext->pCertInfo->cExtension,
            m_pCertContext->pCertInfo->rgExtension);

        if (pExtn != NULL)
        {
            CERT_KEY_USAGE_RESTRICTION_INFO keyUsage;
            DWORD length = sizeof(CERT_KEY_USAGE_RESTRICTION_INFO);

            bool rc = CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_KEY_USAGE,
                pExtn->Value.pbData,
                pExtn->Value.cbData,
                CRYPT_DECODE_NOCOPY_FLAG,
                (void *)&keyUsage,
                &length);

            if (rc && keyUsage.RestrictedKeyUsage.cbData!=0)
            {
                usage = (sal_Int32)keyUsage.RestrictedKeyUsage.pbData;
            }
        }
    }

    return usage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
