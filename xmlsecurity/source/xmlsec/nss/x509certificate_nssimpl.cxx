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

#include "nssrenam.h" // rename problematic symbols
#include "nspr.h"
#include "nss.h"
#include "secder.h"

#include "hasht.h"
#include "secoid.h"
#include "pk11func.h"

#include <sal/config.h>
#include <comphelper/servicehelper.hxx>
#include "x509certificate_nssimpl.hxx"

#include "certificateextension_xmlsecimpl.hxx"

#include "sanextension_nssimpl.hxx"

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::security ;
using ::rtl::OUString ;

using ::com::sun::star::security::XCertificate ;
using ::com::sun::star::util::DateTime ;

X509Certificate_NssImpl :: X509Certificate_NssImpl() :
    m_pCert( NULL )
{
}

X509Certificate_NssImpl :: ~X509Certificate_NssImpl() {
    if( m_pCert != NULL ) {
        CERT_DestroyCertificate( m_pCert ) ;
    }
}

//Methods from XCertificate
sal_Int16 SAL_CALL X509Certificate_NssImpl :: getVersion() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL ) {
        if( m_pCert->version.len > 0 ) {
            return ( char )*( m_pCert->version.data ) ;
        } else
            return 0 ;
    } else {
        return -1 ;
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl :: getSerialNumber() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL && m_pCert->serialNumber.len > 0 ) {
        Sequence< sal_Int8 > serial( m_pCert->serialNumber.len ) ;
        for( unsigned int i = 0 ; i < m_pCert->serialNumber.len ; i ++ )
            serial[i] = *( m_pCert->serialNumber.data + i ) ;

        return serial ;
    } else {
        return ::com::sun::star::uno::Sequence< sal_Int8 >();
    }
}

::rtl::OUString SAL_CALL X509Certificate_NssImpl :: getIssuerName() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL ) {
        return OUString(m_pCert->issuerName , PL_strlen(m_pCert->issuerName) , RTL_TEXTENCODING_UTF8) ;
    } else {
        return OUString() ;
    }
}

::rtl::OUString SAL_CALL X509Certificate_NssImpl :: getSubjectName() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL ) {
        return OUString(m_pCert->subjectName , PL_strlen(m_pCert->subjectName) , RTL_TEXTENCODING_UTF8);
    } else {
        return OUString() ;
    }
}

::com::sun::star::util::DateTime SAL_CALL X509Certificate_NssImpl :: getNotValidBefore() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL ) {
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

        dateTime.HundredthSeconds = static_cast< sal_Int16 >( explTime.tm_usec / 1000  );
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

::com::sun::star::util::DateTime SAL_CALL X509Certificate_NssImpl :: getNotValidAfter() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL ) {
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

        dateTime.HundredthSeconds = static_cast< sal_Int16 >( explTime.tm_usec / 1000  );
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

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl :: getIssuerUniqueID() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL && m_pCert->issuerID.len > 0 ) {
        Sequence< sal_Int8 > issuerUid( m_pCert->issuerID.len ) ;
        for( unsigned int i = 0 ; i < m_pCert->issuerID.len ; i ++ )
            issuerUid[i] = *( m_pCert->issuerID.data + i ) ;

        return issuerUid ;
    } else {
        return ::com::sun::star::uno::Sequence< sal_Int8 >();
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl :: getSubjectUniqueID() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL && m_pCert->subjectID.len > 0 ) {
        Sequence< sal_Int8 > subjectUid( m_pCert->subjectID.len ) ;
        for( unsigned int i = 0 ; i < m_pCert->subjectID.len ; i ++ )
            subjectUid[i] = *( m_pCert->subjectID.data + i ) ;

        return subjectUid ;
    } else {
        return ::com::sun::star::uno::Sequence< sal_Int8 >();
    }
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > > SAL_CALL X509Certificate_NssImpl :: getExtensions() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL && m_pCert->extensions != NULL ) {
        CERTCertExtension** extns ;
        CertificateExtension_XmlSecImpl* pExtn ;
        sal_Bool crit ;
        int len ;

        for( len = 0, extns = m_pCert->extensions; *extns != NULL; len ++, extns ++ ) ;
        Sequence< Reference< XCertificateExtension > > xExtns( len ) ;

        for( extns = m_pCert->extensions, len = 0; *extns != NULL; extns ++, len ++ ) {
            const SECItem id = (*extns)->id;
            ::rtl::OString oidString(CERT_GetOidString(&id));

            // remove "OID." prefix if existing
            ::rtl::OString objID;
            ::rtl::OString oid("OID.");
            if (oidString.match(oid))
                objID = oidString.copy(oid.getLength());
            else
                objID = oidString;

            if ( objID.equals("2.5.29.17") )
                pExtn = (CertificateExtension_XmlSecImpl*) new SanExtensionImpl() ;
            else
                pExtn = new CertificateExtension_XmlSecImpl() ;

            if( (*extns)->critical.data == NULL )
                crit = sal_False ;
            else
                crit = ( (*extns)->critical.data[0] == 0xFF ) ? sal_True : sal_False ;
            pExtn->setCertExtn( (*extns)->value.data, (*extns)->value.len, (unsigned char*)objID.getStr(), objID.getLength(), crit ) ;

            xExtns[len] = pExtn ;
        }

        return xExtns ;
    } else {
        return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > > ();
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > SAL_CALL X509Certificate_NssImpl :: findCertificateExtension( const ::com::sun::star::uno::Sequence< sal_Int8 >& oid ) throw (::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL && m_pCert->extensions != NULL ) {
        CertificateExtension_XmlSecImpl* pExtn ;
        CERTCertExtension** extns ;
        SECItem idItem ;
        sal_Bool crit ;

        idItem.data = ( unsigned char* )&oid[0] ;
        idItem.len = oid.getLength() ;

        pExtn = NULL ;
        for( extns = m_pCert->extensions; *extns != NULL; extns ++ ) {
            if( SECITEM_CompareItem( &idItem, &(*extns)->id ) == SECEqual ) {
                const SECItem id = (*extns)->id;
                ::rtl::OString objId(CERT_GetOidString(&id));
                if ( objId.equals("OID.2.5.29.17") )
                    pExtn = (CertificateExtension_XmlSecImpl*) new SanExtensionImpl() ;
                else
                    pExtn = new CertificateExtension_XmlSecImpl() ;
                if( (*extns)->critical.data == NULL )
                    crit = sal_False ;
                else
                    crit = ( (*extns)->critical.data[0] == 0xFF ) ? sal_True : sal_False ;
                pExtn->setCertExtn( (*extns)->value.data, (*extns)->value.len, (*extns)->id.data, (*extns)->id.len, crit ) ;
            }
        }

        return pExtn ;
    } else {
        return NULL ;
    }
}


::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl :: getEncoded() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL && m_pCert->derCert.len > 0 ) {
        Sequence< sal_Int8 > rawCert( m_pCert->derCert.len ) ;

        for( unsigned int i = 0 ; i < m_pCert->derCert.len ; i ++ )
            rawCert[i] = *( m_pCert->derCert.data + i ) ;

        return rawCert ;
    } else {
        return ::com::sun::star::uno::Sequence< sal_Int8 >();
    }
}

//Helper methods
void X509Certificate_NssImpl :: setCert( CERTCertificate* cert ) {
    if( m_pCert != NULL ) {
        CERT_DestroyCertificate( m_pCert ) ;
        m_pCert = NULL ;
    }

    if( cert != NULL ) {
        m_pCert = CERT_DupCertificate( cert ) ;
    }
}

const CERTCertificate* X509Certificate_NssImpl :: getNssCert() const {
    if( m_pCert != NULL ) {
        return m_pCert ;
    } else {
        return NULL ;
    }
}

void X509Certificate_NssImpl :: setRawCert( Sequence< sal_Int8 > rawCert ) throw ( ::com::sun::star::uno::RuntimeException) {
    CERTCertificate* cert ;
    SECItem certItem ;

    certItem.data = ( unsigned char* )&rawCert[0] ;
    certItem.len = rawCert.getLength() ;

    cert = CERT_DecodeDERCertificate( &certItem, PR_TRUE, NULL ) ;
    if( cert == NULL )
        throw RuntimeException() ;

    if( m_pCert != NULL ) {
        CERT_DestroyCertificate( m_pCert ) ;
        m_pCert = NULL ;
    }

    m_pCert = cert ;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL X509Certificate_NssImpl :: getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw( RuntimeException ) {
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

const Sequence< sal_Int8>& X509Certificate_NssImpl :: getUnoTunnelId() {
    return theX509Certificate_NssImplUnoTunnelId::get().getSeq();
}

/* XUnoTunnel extension */
X509Certificate_NssImpl* X509Certificate_NssImpl :: getImplementation( const Reference< XInterface > xObj ) {
    Reference< XUnoTunnel > xUT( xObj , UNO_QUERY ) ;
    if( xUT.is() ) {
        return reinterpret_cast<X509Certificate_NssImpl*>(
            sal::static_int_cast<sal_uIntPtr>(xUT->getSomething( getUnoTunnelId() )));
    } else
        return NULL ;
}

::rtl::OUString getAlgorithmDescription(SECAlgorithmID *aid)
{
    SECOidTag tag;
    tag = SECOID_GetAlgorithmTag(aid);

    const char *pDesc = SECOID_FindOIDTagDescription(tag);

    return rtl::OUString::createFromAscii( pDesc ) ;
}

::com::sun::star::uno::Sequence< sal_Int8 > getThumbprint(CERTCertificate *pCert, SECOidTag id)
{
    if( pCert != NULL )
    {
        unsigned char fingerprint[20];
        int length = ((id == SEC_OID_MD5)?MD5_LENGTH:SHA1_LENGTH);

        memset(fingerprint, 0, sizeof fingerprint);
        PK11_HashBuf(id, fingerprint, pCert->derCert.data, pCert->derCert.len);

        Sequence< sal_Int8 > thumbprint( length ) ;
        for( int i = 0 ; i < length ; i ++ )
            thumbprint[i] = fingerprint[i];

        return thumbprint;
    }
    else
    {
        return ::com::sun::star::uno::Sequence< sal_Int8 >();
    }
}

::rtl::OUString SAL_CALL X509Certificate_NssImpl::getSubjectPublicKeyAlgorithm()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCert != NULL )
    {
        return getAlgorithmDescription(&(m_pCert->subjectPublicKeyInfo.algorithm));
    }
    else
    {
        return OUString() ;
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getSubjectPublicKeyValue()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCert != NULL )
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

    return ::com::sun::star::uno::Sequence< sal_Int8 >();
}

::rtl::OUString SAL_CALL X509Certificate_NssImpl::getSignatureAlgorithm()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCert != NULL )
    {
        return getAlgorithmDescription(&(m_pCert->signature));
    }
    else
    {
        return OUString() ;
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getSHA1Thumbprint()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    return getThumbprint(m_pCert, SEC_OID_SHA1);
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getMD5Thumbprint()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    return getThumbprint(m_pCert, SEC_OID_MD5);
}

sal_Int32 SAL_CALL X509Certificate_NssImpl::getCertificateUsage(  )
    throw ( ::com::sun::star::uno::RuntimeException)
{
    SECStatus rv;
    SECItem tmpitem;
    sal_Int32 usage;

    rv = CERT_FindKeyUsageExtension(m_pCert, &tmpitem);
    if ( rv == SECSuccess )
    {
        usage = tmpitem.data[0];
        PORT_Free(tmpitem.data);
        tmpitem.data = NULL;
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
