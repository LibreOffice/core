/*************************************************************************
 *
 *  $RCSfile: x509certificate_nssimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-14 08:12:26 $
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

#ifndef _X509CERTIFICATE_NSSIMPL_HXX_
#include "x509certificate_nssimpl.hxx"
#endif

#ifndef _CERTIFICATEEXTENSION_NSSIMPL_HXX_
#include "certificateextension_xmlsecimpl.hxx"
#endif

#include "nspr.h"
#include "nss.h"
#include "secder.h"

//MM : added by MM
#include "secoid.h"
//MM : end


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
        return NULL ;
    }
}

::rtl::OUString SAL_CALL X509Certificate_NssImpl :: getIssuerName() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL ) {
        return OUString::createFromAscii( m_pCert->issuerName ) ;
    } else {
        return OUString() ;
    }
}

::rtl::OUString SAL_CALL X509Certificate_NssImpl :: getSubjectName() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL ) {
        return OUString::createFromAscii( m_pCert->subjectName ) ;
    } else {
        return OUString() ;
    }
}

::com::sun::star::util::DateTime SAL_CALL X509Certificate_NssImpl :: getNotBefore() throw ( ::com::sun::star::uno::RuntimeException) {
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
        dateTime.Month = explTime.tm_month ;
        dateTime.Year = explTime.tm_year ;

        return dateTime ;
    } else {
        return DateTime() ;
    }
}

::com::sun::star::util::DateTime SAL_CALL X509Certificate_NssImpl :: getNotAfter() throw ( ::com::sun::star::uno::RuntimeException) {
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
        dateTime.Month = explTime.tm_month ;
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
        return NULL ;
    }
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl :: getSubjectUniqueID() throw ( ::com::sun::star::uno::RuntimeException) {
    if( m_pCert != NULL && m_pCert->subjectID.len > 0 ) {
        Sequence< sal_Int8 > subjectUid( m_pCert->subjectID.len ) ;
        for( unsigned int i = 0 ; i < m_pCert->subjectID.len ; i ++ )
            subjectUid[i] = *( m_pCert->subjectID.data + i ) ;

        return subjectUid ;
    } else {
        return NULL ;
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
        return NULL ;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > SAL_CALL X509Certificate_NssImpl :: findCertExtension( const ::com::sun::star::uno::Sequence< sal_Int8 >& oid ) throw (::com::sun::star::uno::RuntimeException) {
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
        return NULL ;
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
        return ( sal_Int64 )this ;
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
        return ( X509Certificate_NssImpl* )xUT->getSomething( getUnoTunnelId() ) ;
    } else
        return NULL ;
}

// MM : added by MM
::rtl::OUString getAlgorithmDescription(SECAlgorithmID *aid)
{
    SECOidTag tag;
    tag = SECOID_GetAlgorithmTag(aid);

    const char *pDesc = SECOID_FindOIDTagDescription(tag);

    return rtl::OUString::createFromAscii( pDesc ) ;
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

    return NULL ;
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

::rtl::OUString SAL_CALL X509Certificate_NssImpl::getThumbprintAlgorithm()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    //MM : dummy
    return OUString();
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL X509Certificate_NssImpl::getThumbprint()
    throw ( ::com::sun::star::uno::RuntimeException)
{
    //MM : dummy
    return NULL ;
}

// MM : end

