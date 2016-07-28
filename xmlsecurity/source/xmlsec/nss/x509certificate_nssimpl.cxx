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

#include "nssrenam.h"
#include "nspr.h"
#include "nss.h"
#include "secder.h"

#include "hasht.h"
#include "secoid.h"
#include "pk11func.h"

#include <sal/config.h>
#include <comphelper/servicehelper.hxx>
#include <rtl/ref.hxx>
#include "x509certificate_nssimpl.hxx"

#include "certificateextension_xmlsecimpl.hxx"

#include "sanextension_nssimpl.hxx"
#include <tools/time.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::security ;

using ::com::sun::star::security::XCertificate ;
using ::com::sun::star::util::DateTime ;

X509Certificate_NssImpl::X509Certificate_NssImpl() :
    m_pCert( nullptr )
{
}

X509Certificate_NssImpl::~X509Certificate_NssImpl() {
    if( m_pCert != nullptr ) {
        CERT_DestroyCertificate( m_pCert ) ;
    }
}

//Methods from XCertificate
sal_Int16 SAL_CALL X509Certificate_NssImpl::getVersion() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr ) {
        if( m_pCert->version.len > 0 ) {
            return ( char )*( m_pCert->version.data ) ;
        } else
            return 0 ;
    } else {
        return -1 ;
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getSerialNumber() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr && m_pCert->serialNumber.len > 0 ) {
        Sequence< sal_Int8 > serial( m_pCert->serialNumber.len ) ;
        for( unsigned int i = 0 ; i < m_pCert->serialNumber.len ; i ++ )
            serial[i] = *( m_pCert->serialNumber.data + i ) ;

        return serial ;
    } else {
        return css::uno::Sequence< sal_Int8 >();
    }
}

OUString SAL_CALL X509Certificate_NssImpl::getIssuerName() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr ) {
        return OUString(m_pCert->issuerName , PL_strlen(m_pCert->issuerName) , RTL_TEXTENCODING_UTF8) ;
    } else {
        return OUString() ;
    }
}

OUString SAL_CALL X509Certificate_NssImpl::getSubjectName() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr ) {
        return OUString(m_pCert->subjectName , PL_strlen(m_pCert->subjectName) , RTL_TEXTENCODING_UTF8);
    } else {
        return OUString() ;
    }
}

css::util::DateTime SAL_CALL X509Certificate_NssImpl::getNotValidBefore() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr ) {
        SECStatus rv ;
        PRTime notBefore ;
        PRExplodedTime explTime ;
        DateTime dateTime ;

        rv = DER_DecodeTimeChoice( &notBefore, &m_pCert->validity.notBefore ) ;
        if( rv ) {
            return DateTime() ;
        }

        //Convert the time to readable local time
        PR_ExplodeTime( notBefore, PR_LocalTimeParameters, &explTime ) ;

        dateTime.NanoSeconds = static_cast< sal_Int32 >( explTime.tm_usec * ::tools::Time::nanoPerMicro  );
        dateTime.Seconds = static_cast< sal_Int16 >( explTime.tm_sec  );
        dateTime.Minutes = static_cast< sal_Int16 >( explTime.tm_min  );
        dateTime.Hours = static_cast< sal_Int16 >( explTime.tm_hour  );
        dateTime.Day = static_cast< sal_Int16 >( explTime.tm_mday  );
        dateTime.Month = static_cast< sal_Int16 >( explTime.tm_month+1  );
        dateTime.Year = static_cast< sal_Int16 >( explTime.tm_year  );

        return dateTime ;
    } else {
        return DateTime() ;
    }
}

css::util::DateTime SAL_CALL X509Certificate_NssImpl::getNotValidAfter() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr ) {
        SECStatus rv ;
        PRTime notAfter ;
        PRExplodedTime explTime ;
        DateTime dateTime ;

        rv = DER_DecodeTimeChoice( &notAfter, &m_pCert->validity.notAfter ) ;
        if( rv ) {
            return DateTime() ;
        }

        //Convert the time to readable local time
        PR_ExplodeTime( notAfter, PR_LocalTimeParameters, &explTime ) ;

        dateTime.NanoSeconds = static_cast< sal_Int16 >( explTime.tm_usec * ::tools::Time::nanoPerMicro );
        dateTime.Seconds = static_cast< sal_Int16 >( explTime.tm_sec  );
        dateTime.Minutes = static_cast< sal_Int16 >( explTime.tm_min  );
        dateTime.Hours = static_cast< sal_Int16 >( explTime.tm_hour  );
        dateTime.Day = static_cast< sal_Int16 >( explTime.tm_mday  );
        dateTime.Month = static_cast< sal_Int16 >( explTime.tm_month+1  );
        dateTime.Year = static_cast< sal_Int16 >( explTime.tm_year  );

        return dateTime ;
    } else {
        return DateTime() ;
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getIssuerUniqueID() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr && m_pCert->issuerID.len > 0 ) {
        Sequence< sal_Int8 > issuerUid( m_pCert->issuerID.len ) ;
        for( unsigned int i = 0 ; i < m_pCert->issuerID.len ; i ++ )
            issuerUid[i] = *( m_pCert->issuerID.data + i ) ;

        return issuerUid ;
    } else {
        return css::uno::Sequence< sal_Int8 >();
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getSubjectUniqueID() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr && m_pCert->subjectID.len > 0 ) {
        Sequence< sal_Int8 > subjectUid( m_pCert->subjectID.len ) ;
        for( unsigned int i = 0 ; i < m_pCert->subjectID.len ; i ++ )
            subjectUid[i] = *( m_pCert->subjectID.data + i ) ;

        return subjectUid ;
    } else {
        return css::uno::Sequence< sal_Int8 >();
    }
}

css::uno::Sequence< css::uno::Reference< css::security::XCertificateExtension > > SAL_CALL X509Certificate_NssImpl::getExtensions() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr && m_pCert->extensions != nullptr ) {
        CERTCertExtension** extns ;
        int len ;

        for( len = 0, extns = m_pCert->extensions; *extns != nullptr; len ++, extns ++ ) ;
        Sequence< Reference< XCertificateExtension > > xExtns( len ) ;

        for( extns = m_pCert->extensions, len = 0; *extns != nullptr; extns ++, len ++ ) {
            const SECItem id = (*extns)->id;
            OString oidString(CERT_GetOidString(&id));

            bool crit;
            if( (*extns)->critical.data == nullptr )
                crit = false ;
            else
                crit = (*extns)->critical.data[0] == 0xFF;

            // remove "OID." prefix if existing
            OString objID;
            OString oid("OID.");
            if (oidString.match(oid))
                objID = oidString.copy(oid.getLength());
            else
                objID = oidString;

            unsigned char* value = (*extns)->value.data;
            unsigned int vlen = (*extns)->value.len;
            unsigned char* objid = reinterpret_cast<unsigned char *>(const_cast<char *>(objID.getStr()));
            unsigned int objidlen = objID.getLength();

            if (objID.equals("2.5.29.17"))
            {
                SanExtensionImpl* pExtn = new SanExtensionImpl();
                pExtn->setCertExtn(value, vlen, objid, objidlen, crit);
                xExtns[len] = pExtn ;
            }
            else
            {
                CertificateExtension_XmlSecImpl* pExtn = new CertificateExtension_XmlSecImpl();
                pExtn->setCertExtn(value, vlen, objid, objidlen, crit);
                xExtns[len] = pExtn;
            }
        }

        return xExtns ;
    } else {
        return css::uno::Sequence< css::uno::Reference< css::security::XCertificateExtension > > ();
    }
}

css::uno::Reference< css::security::XCertificateExtension > SAL_CALL X509Certificate_NssImpl::findCertificateExtension( const css::uno::Sequence< sal_Int8 >& oid ) throw (css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr && m_pCert->extensions != nullptr ) {
        CERTCertExtension** extns ;
        SECItem idItem ;

        idItem.data = reinterpret_cast<unsigned char *>(const_cast<sal_Int8 *>(oid.getConstArray()));
        idItem.len = oid.getLength() ;

        css::uno::Reference<css::security::XCertificateExtension> xExtn;
        for( extns = m_pCert->extensions; *extns != nullptr; extns ++ ) {
            if( SECITEM_CompareItem( &idItem, &(*extns)->id ) == SECEqual ) {
                const SECItem id = (*extns)->id;
                OString objId(CERT_GetOidString(&id));

                bool crit;
                if( (*extns)->critical.data == nullptr )
                    crit = false ;
                else
                    crit = (*extns)->critical.data[0] == 0xFF;

                unsigned char* value = (*extns)->value.data;
                unsigned int vlen = (*extns)->value.len;
                unsigned char* objid = (*extns)->id.data;
                unsigned int objidlen = (*extns)->id.len;

                if ( objId.equals("OID.2.5.29.17") )
                {
                    rtl::Reference<SanExtensionImpl> xSanImpl(
                        new SanExtensionImpl());
                    xSanImpl->setCertExtn(value, vlen, objid, objidlen, crit);
                    xExtn = xSanImpl.get();
                }
                else
                {
                    rtl::Reference<CertificateExtension_XmlSecImpl> xSecImpl(
                        new CertificateExtension_XmlSecImpl());
                    xSecImpl->setCertExtn(value, vlen, objid, objidlen, crit);
                    xExtn = xSecImpl.get();
                }
                break;
            }
        }

        return xExtn;
    } else {
        return nullptr ;
    }
}


css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getEncoded() throw ( css::uno::RuntimeException, std::exception) {
    if( m_pCert != nullptr && m_pCert->derCert.len > 0 ) {
        Sequence< sal_Int8 > rawCert( m_pCert->derCert.len ) ;

        for( unsigned int i = 0 ; i < m_pCert->derCert.len ; i ++ )
            rawCert[i] = *( m_pCert->derCert.data + i ) ;

        return rawCert ;
    } else {
        return css::uno::Sequence< sal_Int8 >();
    }
}

//Helper methods
void X509Certificate_NssImpl::setCert( CERTCertificate* cert ) {
    if( m_pCert != nullptr ) {
        CERT_DestroyCertificate( m_pCert ) ;
        m_pCert = nullptr ;
    }

    if( cert != nullptr ) {
        m_pCert = CERT_DupCertificate( cert ) ;
    }
}

const CERTCertificate* X509Certificate_NssImpl::getNssCert() const {
    if( m_pCert != nullptr ) {
        return m_pCert ;
    } else {
        return nullptr ;
    }
}

void X509Certificate_NssImpl::setRawCert( const Sequence< sal_Int8 >& rawCert ) throw ( css::uno::RuntimeException) {
    CERTCertificate* cert ;
    SECItem certItem ;

    certItem.data = reinterpret_cast<unsigned char *>(const_cast<sal_Int8 *>(rawCert.getConstArray()));
    certItem.len = rawCert.getLength() ;

    cert = CERT_DecodeDERCertificate( &certItem, PR_TRUE, nullptr ) ;
    if( cert == nullptr )
        throw RuntimeException() ;

    if( m_pCert != nullptr ) {
        CERT_DestroyCertificate( m_pCert ) ;
        m_pCert = nullptr ;
    }

    m_pCert = cert ;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL X509Certificate_NssImpl::getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw( RuntimeException, std::exception ) {
    if( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0 ;
}

/* XUnoTunnel extension */

namespace
{
    class theX509Certificate_NssImplUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theX509Certificate_NssImplUnoTunnelId > {};
}

const Sequence< sal_Int8>& X509Certificate_NssImpl::getUnoTunnelId() {
    return theX509Certificate_NssImplUnoTunnelId::get().getSeq();
}

OUString getAlgorithmDescription(SECAlgorithmID *aid)
{
    SECOidTag tag;
    tag = SECOID_GetAlgorithmTag(aid);

    const char *pDesc = SECOID_FindOIDTagDescription(tag);

    return OUString::createFromAscii( pDesc ) ;
}

css::uno::Sequence< sal_Int8 > getThumbprint(CERTCertificate *pCert, SECOidTag id)
{
    if( pCert != nullptr )
    {
        SECStatus rv;
        unsigned char fingerprint[32];
        int length = 0;
        switch (id)
        {
        case SEC_OID_MD5:
            length = MD5_LENGTH;
            break;
        case SEC_OID_SHA1:
            length = SHA1_LENGTH;
            break;
        case SEC_OID_SHA256:
            length = SHA256_LENGTH;
            break;
        default:
            break;
        }

        memset(fingerprint, 0, sizeof fingerprint);
        rv = PK11_HashBuf(id, fingerprint, pCert->derCert.data, pCert->derCert.len);
        if(rv == SECSuccess)
        {
            Sequence< sal_Int8 > thumbprint( length ) ;
            for( int i = 0 ; i < length ; i ++ )
                thumbprint[i] = fingerprint[i];

            return thumbprint;
        }
    }
    return css::uno::Sequence< sal_Int8 >();
}

OUString SAL_CALL X509Certificate_NssImpl::getSubjectPublicKeyAlgorithm()
    throw ( css::uno::RuntimeException, std::exception)
{
    if( m_pCert != nullptr )
    {
        return getAlgorithmDescription(&(m_pCert->subjectPublicKeyInfo.algorithm));
    }
    else
    {
        return OUString() ;
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getSubjectPublicKeyValue()
    throw ( css::uno::RuntimeException, std::exception)
{
    if( m_pCert != nullptr )
    {
        SECItem spk = m_pCert->subjectPublicKeyInfo.subjectPublicKey;
        DER_ConvertBitString(&spk);

        if ( spk.len>0)
        {
            Sequence< sal_Int8 > key( spk.len ) ;
            for( unsigned int i = 0 ; i < spk.len ; i ++ )
            {
                key[i] = *( spk.data + i ) ;
            }

            return key ;
        }
    }

    return css::uno::Sequence< sal_Int8 >();
}

OUString SAL_CALL X509Certificate_NssImpl::getSignatureAlgorithm()
    throw ( css::uno::RuntimeException, std::exception)
{
    if( m_pCert != nullptr )
    {
        return getAlgorithmDescription(&(m_pCert->signature));
    }
    else
    {
        return OUString() ;
    }
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getSHA1Thumbprint()
    throw ( css::uno::RuntimeException, std::exception)
{
    return getThumbprint(m_pCert, SEC_OID_SHA1);
}

uno::Sequence<sal_Int8> X509Certificate_NssImpl::getSHA256Thumbprint() throw (uno::RuntimeException, std::exception)
{
    return getThumbprint(m_pCert, SEC_OID_SHA256);
}

css::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getMD5Thumbprint()
    throw ( css::uno::RuntimeException, std::exception)
{
    return getThumbprint(m_pCert, SEC_OID_MD5);
}

sal_Int32 SAL_CALL X509Certificate_NssImpl::getCertificateUsage(  )
    throw ( css::uno::RuntimeException, std::exception)
{
    SECStatus rv;
    SECItem tmpitem;
    sal_Int32 usage;

    rv = CERT_FindKeyUsageExtension(m_pCert, &tmpitem);
    if ( rv == SECSuccess )
    {
        usage = tmpitem.data[0];
        PORT_Free(tmpitem.data);
        tmpitem.data = nullptr;
    }
    else
    {
        usage = KU_ALL;
    }

    /*
     * to make the nss implementation compatible with MSCrypto,
     * the following usage is ignored
     *
     *
    if ( CERT_GovtApprovedBitSet(m_pCert) )
    {
        usage |= KU_NS_GOVT_APPROVED;
    }
    */

    return usage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
