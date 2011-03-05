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



#include "nssrenam.h"
#include "nspr.h"
#include "nss.h"
#include "secder.h"

#include "hasht.h"
#include "secoid.h"
#include "pk11func.h"

#include <sal/config.h>
#include <rtl/uuid.h>
#include "x509certificate_nssimpl.hxx"

#include "certificateextension_xmlsecimpl.hxx"


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

        dateTime.HundredthSeconds = explTime.tm_usec / 1000 ;
        dateTime.Seconds = explTime.tm_sec ;
        dateTime.Minutes = explTime.tm_min ;
        dateTime.Hours = explTime.tm_hour ;
        dateTime.Day = explTime.tm_mday ;
        dateTime.Month = explTime.tm_month+1 ;
        dateTime.Year = explTime.tm_year ;

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

        dateTime.HundredthSeconds = explTime.tm_usec / 1000 ;
        dateTime.Seconds = explTime.tm_sec ;
        dateTime.Minutes = explTime.tm_min ;
        dateTime.Hours = explTime.tm_hour ;
        dateTime.Day = explTime.tm_mday ;
        dateTime.Month = explTime.tm_month+1 ;
        dateTime.Year = explTime.tm_year ;

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
            pExtn = new CertificateExtension_XmlSecImpl() ;
            if( (*extns)->critical.data == NULL )
                crit = sal_False ;
            else
                crit = ( (*extns)->critical.data[0] == 0xFF ) ? sal_True : sal_False ;
            pExtn->setCertExtn( (*extns)->value.data, (*extns)->value.len, (*extns)->id.data, (*extns)->id.len, crit ) ;

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
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0 ;
}

/* XUnoTunnel extension */
const Sequence< sal_Int8>& X509Certificate_NssImpl :: getUnoTunnelId() {
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
