/*************************************************************************
 *
 *  $RCSfile: x509certificate_mscryptimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-14 08:12:25 $
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

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _X509CERTIFICATE_MSCRYPTIMPL_HXX_
#include "x509certificate_mscryptimpl.hxx"
#endif

#ifndef _CERTIFICATEEXTENSION_XMLSECIMPL_HXX_
#include "certificateextension_xmlsecimpl.hxx"
#endif

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::security ;
using ::rtl::OUString ;

using ::com::sun::star::security::XCertificate ;
using ::com::sun::star::util::DateTime ;

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
        return NULL ;
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
                delete issuer ;
                throw RuntimeException() ;
            }

            OUString xIssuer = OUString::createFromAscii( issuer ) ;
            delete issuer ;

            return xIssuer ;
        } else {
            return OUString() ;
        }
    } else {
        return OUString() ;
    }
}

::rtl::OUString SAL_CALL X509Certificate_MSCryptImpl :: getSubjectName() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        char* subject ;
        DWORD cbSubject ;

        cbSubject = CertNameToStr(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
            &( m_pCertContext->pCertInfo->Subject ),
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
            NULL, 0
        ) ;

        if( cbSubject != 0 ) {
            subject = new char[ cbSubject ] ;
            if( subject == NULL )
                throw RuntimeException() ;

            cbSubject = CertNameToStr(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                &( m_pCertContext->pCertInfo->Subject ),
                CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG ,
                subject, cbSubject
            ) ;

            if( cbSubject <= 0 ) {
                delete subject ;
                throw RuntimeException() ;
            }

            OUString xSubject = OUString::createFromAscii( subject ) ;
            delete subject ;

            return xSubject ;
        } else {
            return OUString() ;
        }
    } else {
        return OUString() ;
    }
}

::com::sun::star::util::DateTime SAL_CALL X509Certificate_MSCryptImpl :: getNotBefore() throw ( ::com::sun::star::uno::RuntimeException ) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        SYSTEMTIME explTime ;
        DateTime dateTime ;

        if( FileTimeToSystemTime( &( m_pCertContext->pCertInfo->NotBefore ), &explTime ) ) {
            //Convert the time to readable local time
            dateTime.HundredthSeconds = explTime.wMilliseconds / 100 ;
            dateTime.Seconds = explTime.wSecond ;
            dateTime.Minutes = explTime.wMinute ;
            dateTime.Hours = explTime.wHour ;
            dateTime.Day = explTime.wDay ;
            dateTime.Month = explTime.wMonth ;
            dateTime.Year = explTime.wYear ;
        }

        return dateTime ;
    } else {
        return DateTime() ;
    }
}

::com::sun::star::util::DateTime SAL_CALL X509Certificate_MSCryptImpl :: getNotAfter() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        SYSTEMTIME explTime ;
        DateTime dateTime ;

        if( FileTimeToSystemTime( &( m_pCertContext->pCertInfo->NotAfter ), &explTime ) ) {
            //Convert the time to readable local time
            dateTime.HundredthSeconds = explTime.wMilliseconds / 100 ;
            dateTime.Seconds = explTime.wSecond ;
            dateTime.Minutes = explTime.wMinute ;
            dateTime.Hours = explTime.wHour ;
            dateTime.Day = explTime.wDay ;
            dateTime.Month = explTime.wMonth ;
            dateTime.Year = explTime.wYear ;
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
        return NULL ;
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl :: getSubjectUniqueID() throw ( ::com::sun::star::uno::RuntimeException ) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL ) {
        Sequence< sal_Int8 > subjectUid( m_pCertContext->pCertInfo->SubjectUniqueId.cbData ) ;
        for( unsigned int i = 0 ; i < m_pCertContext->pCertInfo->SubjectUniqueId.cbData; i ++ )
            subjectUid[i] = *( m_pCertContext->pCertInfo->SubjectUniqueId.pbData + i ) ;

        return subjectUid ;
    } else {
        return NULL ;
    }
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > > SAL_CALL X509Certificate_MSCryptImpl :: getExtensions() throw ( ::com::sun::star::uno::RuntimeException ) {
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL && m_pCertContext->pCertInfo->cExtension != 0 ) {
        CertificateExtension_XmlSecImpl* xExtn ;
        CERT_EXTENSION* pExtn ;
        Sequence< Reference< XCertificateExtension > > xExtns( m_pCertContext->pCertInfo->cExtension ) ;

        for( unsigned int i = 0; i < m_pCertContext->pCertInfo->cExtension; i++ ) {
            pExtn = &(m_pCertContext->pCertInfo->rgExtension[i]) ;

            xExtn = new CertificateExtension_XmlSecImpl() ;
            if( xExtn == NULL )
                throw RuntimeException() ;

            xExtn->setCertExtn( pExtn->Value.pbData, pExtn->Value.cbData, ( unsigned char* )pExtn->pszObjId, strlen( pExtn->pszObjId ), pExtn->fCritical ) ;

            xExtns[i] = xExtn ;
        }

        return xExtns ;
    } else {
        return NULL ;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > SAL_CALL X509Certificate_MSCryptImpl :: findCertExtension( const ::com::sun::star::uno::Sequence< sal_Int8 >& oid ) throw (::com::sun::star::uno::RuntimeException) {
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

                xExtn->setCertExtn( pExtn->Value.pbData, pExtn->Value.cbData, ( unsigned char* )pExtn->pszObjId, strlen( pExtn->pszObjId ), pExtn->fCritical ) ;
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
        return NULL ;
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
        m_pCertContext = CertCreateCertificateContext( X509_ASN_ENCODING, ( const BYTE* )&rawCert[0], rawCert.getLength() ) ;
    }
}

/* XUnoTunnel */
sal_Int64 SAL_CALL X509Certificate_MSCryptImpl :: getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw( RuntimeException ) {
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return ( sal_Int64 )this ;
    }
    return 0 ;
}

/* XUnoTunnel extension */
const Sequence< sal_Int8>& X509Certificate_MSCryptImpl :: getUnoTunnelId() {
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
X509Certificate_MSCryptImpl* X509Certificate_MSCryptImpl :: getImplementation( const Reference< XInterface > xObj ) {
    Reference< XUnoTunnel > xUT( xObj , UNO_QUERY ) ;
    if( xUT.is() ) {
        return ( X509Certificate_MSCryptImpl* )xUT->getSomething( getUnoTunnelId() ) ;
    } else
        return NULL ;
}

// MM : added by MM
::rtl::OUString SAL_CALL X509Certificate_MSCryptImpl::getSubjectPublicKeyAlgorithm()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL )
    {
        CRYPT_ALGORITHM_IDENTIFIER algorithm = m_pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm;
        return OUString::createFromAscii( algorithm.pszObjId ) ;
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
        return NULL ;
    }
}

::rtl::OUString SAL_CALL X509Certificate_MSCryptImpl::getSignatureAlgorithm()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    if( m_pCertContext != NULL && m_pCertContext->pCertInfo != NULL )
    {
        CRYPT_ALGORITHM_IDENTIFIER algorithm = m_pCertContext->pCertInfo->SignatureAlgorithm;
        return OUString::createFromAscii( algorithm.pszObjId ) ;
    }
    else
    {
        return OUString() ;
    }
}

::rtl::OUString SAL_CALL X509Certificate_MSCryptImpl::getThumbprintAlgorithm()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    //MM : dummy
    return OUString();
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_MSCryptImpl::getThumbprint()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    //MM : dummy
    return NULL ;
}

// MM : end

