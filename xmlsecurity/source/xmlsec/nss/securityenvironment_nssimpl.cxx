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
#include "cert.h"
#include "secerr.h"
#include "ocsp.h"

#include <sal/config.h>
#include <sal/macros.h>
#include "securityenvironment_nssimpl.hxx"
#include "x509certificate_nssimpl.hxx"
#include <comphelper/servicehelper.hxx>
#include "../diagnose.hxx"

#include "xmlsecurity/xmlsec-wrapper.h"

#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/docpasswordrequest.hxx>
#include <xmlsecurity/biginteger.hxx>
#include <sal/log.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <vector>
#include "boost/scoped_array.hpp"
#include <osl/thread.h>

#include "secerror.hxx"

// added for password exception
#include <com/sun/star/security/NoPasswordException.hpp>
namespace csss = ::com::sun::star::security;
using namespace xmlsecurity;
using namespace ::com::sun::star::security;
using namespace com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::security::XCertificate ;

extern X509Certificate_NssImpl* NssCertToXCert( CERTCertificate* cert ) ;
extern X509Certificate_NssImpl* NssPrivKeyToXCert( SECKEYPrivateKey* ) ;


struct UsageDescription
{
    SECCertificateUsage usage;
    char const* description;

    UsageDescription()
    : usage( certificateUsageCheckAllUsages )
    , description( NULL )
    {}

    UsageDescription( SECCertificateUsage i_usage, char const* i_description )
    : usage( i_usage )
    , description( i_description )
    {}

    UsageDescription( const UsageDescription& aDescription )
    : usage( aDescription.usage )
    , description( aDescription.description )
    {}

    UsageDescription& operator =( const UsageDescription& aDescription )
    {
        usage = aDescription.usage;
        description = aDescription.description;
        return *this;
    }
};



char* GetPasswordFunction( PK11SlotInfo* pSlot, PRBool bRetry, void* /*arg*/ )
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < task::XInteractionHandler2 > xInteractionHandler(
        task::InteractionHandler::createWithParent(xContext, 0) );

    task::PasswordRequestMode eMode = bRetry ? task::PasswordRequestMode_PASSWORD_REENTER : task::PasswordRequestMode_PASSWORD_ENTER;
    ::comphelper::DocPasswordRequest* pPasswordRequest = new ::comphelper::DocPasswordRequest(
        ::comphelper::DocPasswordRequestType_STANDARD, eMode, OUString::createFromAscii(PK11_GetTokenName(pSlot)) );

    uno::Reference< task::XInteractionRequest > xRequest( pPasswordRequest );
    xInteractionHandler->handle( xRequest );

    if ( pPasswordRequest->isPassword() )
    {
        OString aPassword(OUStringToOString(
            pPasswordRequest->getPassword(),
            osl_getThreadTextEncoding()));
        sal_Int32 nLen = aPassword.getLength();
        char* pPassword = (char*) PORT_Alloc( nLen+1 ) ;
        pPassword[nLen] = 0;
        memcpy( pPassword, aPassword.getStr(), nLen );
        return pPassword;
    }
    return NULL;
}

SecurityEnvironment_NssImpl :: SecurityEnvironment_NssImpl() :
m_pHandler( NULL ) , m_tSymKeyList() , m_tPubKeyList() , m_tPriKeyList() {

    PK11_SetPasswordFunc( GetPasswordFunction ) ;
}

SecurityEnvironment_NssImpl :: ~SecurityEnvironment_NssImpl() {

    PK11_SetPasswordFunc( NULL ) ;

    for (CIT_SLOTS i = m_Slots.begin(); i != m_Slots.end(); i++)
    {
        PK11_FreeSlot(*i);
    }

    if( !m_tSymKeyList.empty()  ) {
        std::list< PK11SymKey* >::iterator symKeyIt ;

        for( symKeyIt = m_tSymKeyList.begin() ; symKeyIt != m_tSymKeyList.end() ; ++symKeyIt )
            PK11_FreeSymKey( *symKeyIt ) ;
    }

    if( !m_tPubKeyList.empty()  ) {
        std::list< SECKEYPublicKey* >::iterator pubKeyIt ;

        for( pubKeyIt = m_tPubKeyList.begin() ; pubKeyIt != m_tPubKeyList.end() ; ++pubKeyIt )
            SECKEY_DestroyPublicKey( *pubKeyIt ) ;
    }

    if( !m_tPriKeyList.empty()  ) {
        std::list< SECKEYPrivateKey* >::iterator priKeyIt ;

        for( priKeyIt = m_tPriKeyList.begin() ; priKeyIt != m_tPriKeyList.end() ; ++priKeyIt )
            SECKEY_DestroyPrivateKey( *priKeyIt ) ;
    }
}

/* XServiceInfo */
OUString SAL_CALL SecurityEnvironment_NssImpl :: getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL SecurityEnvironment_NssImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SecurityEnvironment_NssImpl :: getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > SecurityEnvironment_NssImpl :: impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames.getArray()[0] = OUString("com.sun.star.xml.crypto.SecurityEnvironment") ;
    return seqServiceNames ;
}

OUString SecurityEnvironment_NssImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString("com.sun.star.xml.security.bridge.xmlsec.SecurityEnvironment_NssImpl") ;
}

//Helper for registry
Reference< XInterface > SAL_CALL SecurityEnvironment_NssImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& ) throw( RuntimeException ) {
    return Reference< XInterface >( *new SecurityEnvironment_NssImpl ) ;
}

Reference< XSingleServiceFactory > SecurityEnvironment_NssImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL SecurityEnvironment_NssImpl :: getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw( RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0 ;
}

/* XUnoTunnel extension */

namespace
{
    class theSecurityEnvironment_NssImplUnoTunnelId  : public rtl::Static< UnoTunnelIdInit, theSecurityEnvironment_NssImplUnoTunnelId > {};
}

const Sequence< sal_Int8>& SecurityEnvironment_NssImpl :: getUnoTunnelId() {
    return theSecurityEnvironment_NssImplUnoTunnelId::get().getSeq();
}

OUString SecurityEnvironment_NssImpl::getSecurityEnvironmentInformation() throw( ::com::sun::star::uno::RuntimeException )
{
    OUStringBuffer buff;
    for (CIT_SLOTS is = m_Slots.begin(); is != m_Slots.end(); is++)
    {
        buff.append(OUString::createFromAscii(PK11_GetTokenName(*is)));
        buff.appendAscii("\n");
    }
    return buff.makeStringAndClear();
}

void SecurityEnvironment_NssImpl::addCryptoSlot( PK11SlotInfo* aSlot) throw( Exception , RuntimeException )
{
    PK11_ReferenceSlot(aSlot);
    m_Slots.push_back(aSlot);
}

CERTCertDBHandle* SecurityEnvironment_NssImpl :: getCertDb() throw( Exception , RuntimeException ) {
    return m_pHandler ;
}

//Could we have multiple cert dbs?
void SecurityEnvironment_NssImpl :: setCertDb( CERTCertDBHandle* aCertDb ) throw( Exception , RuntimeException ) {
    m_pHandler = aCertDb ;
}

void SecurityEnvironment_NssImpl :: adoptSymKey( PK11SymKey* aSymKey ) throw( Exception , RuntimeException ) {
    PK11SymKey* symkey ;
    std::list< PK11SymKey* >::iterator keyIt ;

    if( aSymKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tSymKeyList.begin() ; keyIt != m_tSymKeyList.end() ; ++keyIt ) {
            if( *keyIt == aSymKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        symkey = PK11_ReferenceSymKey( aSymKey ) ;
        if( symkey == NULL )
            throw RuntimeException() ;

        try {
            m_tSymKeyList.push_back( symkey ) ;
        } catch ( Exception& ) {
            PK11_FreeSymKey( symkey ) ;
        }
    }
}

void SecurityEnvironment_NssImpl :: rejectSymKey( PK11SymKey* aSymKey ) throw( Exception , RuntimeException ) {
    PK11SymKey* symkey ;
    std::list< PK11SymKey* >::iterator keyIt ;

    if( aSymKey != NULL ) {
        for( keyIt = m_tSymKeyList.begin() ; keyIt != m_tSymKeyList.end() ; ++keyIt ) {
            if( *keyIt == aSymKey ) {
                symkey = *keyIt ;
                PK11_FreeSymKey( symkey ) ;
                m_tSymKeyList.erase( keyIt ) ;
                break ;
            }
        }
    }
}

PK11SymKey* SecurityEnvironment_NssImpl :: getSymKey( unsigned int position ) throw( Exception , RuntimeException ) {
    PK11SymKey* symkey ;
    std::list< PK11SymKey* >::iterator keyIt ;
    unsigned int pos ;

    symkey = NULL ;
    for( pos = 0, keyIt = m_tSymKeyList.begin() ; pos < position && keyIt != m_tSymKeyList.end() ; pos ++ , keyIt ++ ) ;

    if( pos == position && keyIt != m_tSymKeyList.end() )
        symkey = *keyIt ;

    return symkey ;
}

void SecurityEnvironment_NssImpl :: adoptPubKey( SECKEYPublicKey* aPubKey ) throw( Exception , RuntimeException ) {
    SECKEYPublicKey*    pubkey ;
    std::list< SECKEYPublicKey* >::iterator keyIt ;

    if( aPubKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tPubKeyList.begin() ; keyIt != m_tPubKeyList.end() ; ++keyIt ) {
            if( *keyIt == aPubKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        pubkey = SECKEY_CopyPublicKey( aPubKey ) ;
        if( pubkey == NULL )
            throw RuntimeException() ;

        try {
            m_tPubKeyList.push_back( pubkey ) ;
        } catch ( Exception& ) {
            SECKEY_DestroyPublicKey( pubkey ) ;
        }
    }
}

void SecurityEnvironment_NssImpl :: rejectPubKey( SECKEYPublicKey* aPubKey ) throw( Exception , RuntimeException ) {
    SECKEYPublicKey*    pubkey ;
    std::list< SECKEYPublicKey* >::iterator keyIt ;

    if( aPubKey != NULL ) {
        for( keyIt = m_tPubKeyList.begin() ; keyIt != m_tPubKeyList.end() ; ++keyIt ) {
            if( *keyIt == aPubKey ) {
                pubkey = *keyIt ;
                SECKEY_DestroyPublicKey( pubkey ) ;
                m_tPubKeyList.erase( keyIt ) ;
                break ;
            }
        }
    }
}

SECKEYPublicKey* SecurityEnvironment_NssImpl :: getPubKey( unsigned int position ) throw( Exception , RuntimeException ) {
    SECKEYPublicKey* pubkey ;
    std::list< SECKEYPublicKey* >::iterator keyIt ;
    unsigned int pos ;

    pubkey = NULL ;
    for( pos = 0, keyIt = m_tPubKeyList.begin() ; pos < position && keyIt != m_tPubKeyList.end() ; pos ++ , keyIt ++ ) ;

    if( pos == position && keyIt != m_tPubKeyList.end() )
        pubkey = *keyIt ;

    return pubkey ;
}

void SecurityEnvironment_NssImpl :: adoptPriKey( SECKEYPrivateKey* aPriKey ) throw( Exception , RuntimeException ) {
    SECKEYPrivateKey*   prikey ;
    std::list< SECKEYPrivateKey* >::iterator keyIt ;

    if( aPriKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tPriKeyList.begin() ; keyIt != m_tPriKeyList.end() ; ++keyIt ) {
            if( *keyIt == aPriKey )
                return ;
        }

        //If we do not find the key in the list, add a new node
        prikey = SECKEY_CopyPrivateKey( aPriKey ) ;
        if( prikey == NULL )
            throw RuntimeException() ;

        try {
            m_tPriKeyList.push_back( prikey ) ;
        } catch ( Exception& ) {
            SECKEY_DestroyPrivateKey( prikey ) ;
        }
    }
}

void SecurityEnvironment_NssImpl :: rejectPriKey( SECKEYPrivateKey* aPriKey ) throw( Exception , RuntimeException ) {
    SECKEYPrivateKey*   prikey ;
    std::list< SECKEYPrivateKey* >::iterator keyIt ;

    if( aPriKey != NULL ) {
        for( keyIt = m_tPriKeyList.begin() ; keyIt != m_tPriKeyList.end() ; ++keyIt ) {
            if( *keyIt == aPriKey ) {
                prikey = *keyIt ;
                SECKEY_DestroyPrivateKey( prikey ) ;
                m_tPriKeyList.erase( keyIt ) ;
                break ;
            }
        }
    }
}

SECKEYPrivateKey* SecurityEnvironment_NssImpl :: getPriKey( unsigned int position ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException )  {
    SECKEYPrivateKey* prikey ;
    std::list< SECKEYPrivateKey* >::iterator keyIt ;
    unsigned int pos ;

    prikey = NULL ;
    for( pos = 0, keyIt = m_tPriKeyList.begin() ; pos < position && keyIt != m_tPriKeyList.end() ; pos ++ , keyIt ++ ) ;

    if( pos == position && keyIt != m_tPriKeyList.end() )
        prikey = *keyIt ;

    return prikey ;
}

void SecurityEnvironment_NssImpl::updateSlots()
{
    //In case new tokens are present then we can obtain the corresponding slot
    PK11SlotList * soltList = NULL;
    PK11SlotListElement * soltEle = NULL;
    PK11SlotInfo * pSlot = NULL;
       PK11SymKey * pSymKey = NULL;

    osl::MutexGuard guard(m_mutex);

    m_Slots.clear();
    m_tSymKeyList.clear();

    soltList = PK11_GetAllTokens( CKM_INVALID_MECHANISM, PR_FALSE, PR_FALSE, NULL ) ;
    if( soltList != NULL )
    {
        for( soltEle = soltList->head ; soltEle != NULL; soltEle = soltEle->next )
        {
            pSlot = soltEle->slot ;

            if(pSlot != NULL)
            {
                SAL_INFO(
                    "xmlsecurity.xmlsec",
                    "Found a slot: SlotName=" << PK11_GetSlotName(pSlot)
                        << ", TokenName=" << PK11_GetTokenName(pSlot));

//The following code which is commented out checks if a slot, that is a smart card for example, is
//              able to generate a symmetric key of type CKM_DES3_CBC. If this fails then this token
//              will not be used. This key is possibly used for the encryption service. However, all
//              interfaces and services used for public key signature and encryption are not published
//              and the encryption is not used in OOo. Therefore it does not do any harm to remove
//              this code, hence allowing smart cards which cannot generate this type of key.
//
//              By doing this, the encryption may fail if a smart card is being used which does not
//              support this key generation.
//
                pSymKey = PK11_KeyGen( pSlot , CKM_DES3_CBC, NULL, 128, NULL ) ;
//              if( pSymKey == NULL )
//              {
//                  PK11_FreeSlot( pSlot ) ;
//                  SAL_INFO( "xmlsecurity", "XMLSEC: Error - pSymKey is NULL" );
//                  continue;
//              }
                addCryptoSlot(pSlot);
                PK11_FreeSlot( pSlot ) ;
                pSlot = NULL;

                if (pSymKey != NULL)
                {
                    adoptSymKey( pSymKey ) ;
                    PK11_FreeSymKey( pSymKey ) ;
                    pSymKey = NULL;
                }

            }// end of if(pSlot != NULL)
        }// end of for
    }// end of if( soltList != NULL )

}


Sequence< Reference < XCertificate > >
SecurityEnvironment_NssImpl::getPersonalCertificates() throw( SecurityException , RuntimeException )
{
    sal_Int32 length ;
    X509Certificate_NssImpl* xcert ;
    std::list< X509Certificate_NssImpl* > certsList ;

    updateSlots();
    //firstly, we try to find private keys in slot
    for (CIT_SLOTS is = m_Slots.begin(); is != m_Slots.end(); is++)
    {
        PK11SlotInfo *slot = *is;
        SECKEYPrivateKeyList* priKeyList ;
        SECKEYPrivateKeyListNode* curPri ;

        if( PK11_NeedLogin(slot ) ) {
            SECStatus nRet = PK11_Authenticate(slot, PR_TRUE, NULL);
            //PK11_Authenticate may fail in case the a slot has not been initialized.
            //this is the case if the user has a new profile, so that they have never
            //added a personal certificate.
            if( nRet != SECSuccess && PORT_GetError() != SEC_ERROR_IO) {
                throw NoPasswordException();
            }
        }

        priKeyList = PK11_ListPrivateKeysInSlot(slot) ;
        if( priKeyList != NULL )
        {
            for( curPri = PRIVKEY_LIST_HEAD( priKeyList );
                !PRIVKEY_LIST_END( curPri, priKeyList ) && curPri != NULL ;
                curPri = PRIVKEY_LIST_NEXT( curPri ) )
            {
                xcert = NssPrivKeyToXCert( curPri->key ) ;
                if( xcert != NULL )
                    certsList.push_back( xcert ) ;
            }
            SECKEY_DestroyPrivateKeyList( priKeyList ) ;
        }


    }

    //secondly, we try to find certificate from registered private keys.
    if( !m_tPriKeyList.empty()  ) {
        std::list< SECKEYPrivateKey* >::iterator priKeyIt ;

        for( priKeyIt = m_tPriKeyList.begin() ; priKeyIt != m_tPriKeyList.end() ; ++priKeyIt ) {
            xcert = NssPrivKeyToXCert( *priKeyIt ) ;
            if( xcert != NULL )
                certsList.push_back( xcert ) ;
        }
    }

    length = certsList.size() ;
    if( length != 0 ) {
        int i ;
        std::list< X509Certificate_NssImpl* >::iterator xcertIt ;
        Sequence< Reference< XCertificate > > certSeq( length ) ;

        for( i = 0, xcertIt = certsList.begin(); xcertIt != certsList.end(); ++xcertIt, ++i ) {
            certSeq[i] = *xcertIt ;
        }

        return certSeq ;
    }

    return Sequence< Reference < XCertificate > > ();
}

Reference< XCertificate > SecurityEnvironment_NssImpl :: getCertificate( const OUString& issuerName, const Sequence< sal_Int8 >& serialNumber ) throw( SecurityException , RuntimeException )
{
    X509Certificate_NssImpl* xcert = NULL;

    if( m_pHandler != NULL ) {
        CERTIssuerAndSN issuerAndSN ;
        CERTCertificate* cert ;
        CERTName* nmIssuer ;
        char* chIssuer ;
        SECItem* derIssuer ;
        PRArenaPool* arena ;

        arena = PORT_NewArena( DER_DEFAULT_CHUNKSIZE ) ;
        if( arena == NULL )
            throw RuntimeException() ;

        // Create cert info from issue and serial
        OString ostr = OUStringToOString( issuerName , RTL_TEXTENCODING_UTF8 ) ;
        chIssuer = PL_strndup( ( char* )ostr.getStr(), ( int )ostr.getLength() ) ;
        nmIssuer = CERT_AsciiToName( chIssuer ) ;
        if( nmIssuer == NULL ) {
            PL_strfree( chIssuer ) ;
            PORT_FreeArena( arena, PR_FALSE ) ;
            return NULL; // no need for exception cf. i40394
        }

        derIssuer = SEC_ASN1EncodeItem( arena, NULL, ( void* )nmIssuer, SEC_ASN1_GET( CERT_NameTemplate ) ) ;
        if( derIssuer == NULL ) {
            PL_strfree( chIssuer ) ;
            CERT_DestroyName( nmIssuer ) ;
            PORT_FreeArena( arena, PR_FALSE ) ;
            throw RuntimeException() ;
        }

        memset( &issuerAndSN, 0, sizeof( issuerAndSN ) ) ;

        issuerAndSN.derIssuer.data = derIssuer->data ;
        issuerAndSN.derIssuer.len = derIssuer->len ;

        issuerAndSN.serialNumber.data = ( unsigned char* )&serialNumber[0] ;
        issuerAndSN.serialNumber.len = serialNumber.getLength() ;

        cert = CERT_FindCertByIssuerAndSN( m_pHandler, &issuerAndSN ) ;
        if( cert != NULL ) {
            xcert = NssCertToXCert( cert ) ;
        } else {
            xcert = NULL ;
        }

        PL_strfree( chIssuer ) ;
        CERT_DestroyName( nmIssuer ) ;
        //SECITEM_FreeItem( derIssuer, PR_FALSE ) ;
        CERT_DestroyCertificate( cert ) ;
        PORT_FreeArena( arena, PR_FALSE ) ;
    } else {
        xcert = NULL ;
    }

    return xcert ;
}

Reference< XCertificate > SecurityEnvironment_NssImpl :: getCertificate( const OUString& issuerName, const OUString& serialNumber ) throw( SecurityException , RuntimeException ) {
    Sequence< sal_Int8 > serial = numericStringToBigInteger( serialNumber ) ;
    return getCertificate( issuerName, serial ) ;
}

Sequence< Reference < XCertificate > > SecurityEnvironment_NssImpl :: buildCertificatePath( const Reference< XCertificate >& begin ) throw( SecurityException , RuntimeException ) {
    const X509Certificate_NssImpl* xcert ;
    const CERTCertificate* cert ;
    CERTCertList* certChain ;

    Reference< XUnoTunnel > xCertTunnel( begin, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xcert = reinterpret_cast<X509Certificate_NssImpl*>(
        sal::static_int_cast<sal_uIntPtr>(xCertTunnel->getSomething( X509Certificate_NssImpl::getUnoTunnelId() ))) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    cert = xcert->getNssCert() ;
    if( cert != NULL ) {
        int64 timeboundary ;

        //Get the system clock time
        timeboundary = PR_Now() ;

        certChain = CERT_GetCertChainFromCert( ( CERTCertificate* )cert, timeboundary, certUsageAnyCA ) ;
    } else {
        certChain = NULL ;
    }

    if( certChain != NULL ) {
        X509Certificate_NssImpl* pCert ;
        CERTCertListNode* node ;
        int len ;

        for( len = 0, node = CERT_LIST_HEAD( certChain ); !CERT_LIST_END( node, certChain ); node = CERT_LIST_NEXT( node ), len ++ ) ;
        Sequence< Reference< XCertificate > > xCertChain( len ) ;

        for( len = 0, node = CERT_LIST_HEAD( certChain ); !CERT_LIST_END( node, certChain ); node = CERT_LIST_NEXT( node ), len ++ ) {
            pCert = new X509Certificate_NssImpl() ;
            if( pCert == NULL ) {
                CERT_DestroyCertList( certChain ) ;
                throw RuntimeException() ;
            }

            pCert->setCert( node->cert ) ;

            xCertChain[len] = pCert ;
        }

        CERT_DestroyCertList( certChain ) ;

        return xCertChain ;
    }

    return Sequence< Reference < XCertificate > >();
}

Reference< XCertificate > SecurityEnvironment_NssImpl :: createCertificateFromRaw( const Sequence< sal_Int8 >& rawCertificate ) throw( SecurityException , RuntimeException ) {
    X509Certificate_NssImpl* xcert ;

    if( rawCertificate.getLength() > 0 ) {
        xcert = new X509Certificate_NssImpl() ;
        if( xcert == NULL )
            throw RuntimeException() ;

        xcert->setRawCert( rawCertificate ) ;
    } else {
        xcert = NULL ;
    }

    return xcert ;
}

Reference< XCertificate > SecurityEnvironment_NssImpl :: createCertificateFromAscii( const OUString& asciiCertificate ) throw( SecurityException , RuntimeException )
{
    xmlChar* chCert ;
    xmlSecSize certSize ;

    OString oscert = OUStringToOString( asciiCertificate , RTL_TEXTENCODING_ASCII_US ) ;

    chCert = xmlStrndup( ( const xmlChar* )oscert.getStr(), ( int )oscert.getLength() ) ;

    certSize = xmlSecBase64Decode( chCert, ( xmlSecByte* )chCert, xmlStrlen( chCert ) ) ;

    if(certSize > 0)
    {
        Sequence< sal_Int8 > rawCert( certSize ) ;
        for( unsigned int i = 0 ; i < certSize ; i ++ )
            rawCert[i] = *( chCert + i ) ;

        xmlFree( chCert ) ;

        return createCertificateFromRaw( rawCert ) ;
    }
    else
    {
        return NULL;
    }
}

sal_Int32 SecurityEnvironment_NssImpl ::
verifyCertificate( const Reference< csss::XCertificate >& aCert,
                   const Sequence< Reference< csss::XCertificate > >&  intermediateCerts )
    throw( ::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException )
{
    sal_Int32 validity = csss::CertificateValidity::INVALID;
    const X509Certificate_NssImpl* xcert ;
    const CERTCertificate* cert ;
    ::std::vector<CERTCertificate*> vecTmpNSSCertificates;
    Reference< XUnoTunnel > xCertTunnel( aCert, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xmlsec_trace("Start verification of certificate: \n %s \n",
              OUStringToOString(
                  aCert->getSubjectName(), osl_getThreadTextEncoding()).getStr());

    xcert = reinterpret_cast<X509Certificate_NssImpl*>(
       sal::static_int_cast<sal_uIntPtr>(xCertTunnel->getSomething( X509Certificate_NssImpl::getUnoTunnelId() ))) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    //CERT_PKIXVerifyCert does not take a db as argument. It will therefore
    //internally use CERT_GetDefaultCertDB
    //Make sure m_pHandler is the default DB
    OSL_ASSERT(m_pHandler == CERT_GetDefaultCertDB());
    CERTCertDBHandle * certDb = m_pHandler != NULL ? m_pHandler : CERT_GetDefaultCertDB();
    cert = xcert->getNssCert() ;
    if( cert != NULL )
    {

        //prepare the intermediate certificates
        for (sal_Int32 i = 0; i < intermediateCerts.getLength(); i++)
        {
            Sequence<sal_Int8> der = intermediateCerts[i]->getEncoded();
            SECItem item;
            item.type = siBuffer;
            item.data = (unsigned char*)der.getArray();
            item.len = der.getLength();

            CERTCertificate* certTmp = CERT_NewTempCertificate(certDb, &item,
                                           NULL     /* nickname */,
                                           PR_FALSE /* isPerm */,
                                           PR_TRUE  /* copyDER */);
             if (!certTmp)
             {
                 xmlsec_trace("Failed to add a temporary certificate: %s",
                           OUStringToOString(intermediateCerts[i]->getIssuerName(),
                                             osl_getThreadTextEncoding()).getStr());

             }
             else
             {
                 xmlsec_trace("Added temporary certificate: %s",
                           certTmp->subjectName ? certTmp->subjectName : "");
                 vecTmpNSSCertificates.push_back(certTmp);
             }
        }


        SECStatus status ;

        CERTVerifyLog log;
        log.arena = PORT_NewArena(512);
        log.head = log.tail = NULL;
        log.count = 0;

        CERT_EnableOCSPChecking(certDb);
        CERT_DisableOCSPDefaultResponder(certDb);
        CERTValOutParam cvout[5];
        CERTValInParam cvin[3];
        int ncvinCount=0;

#if ( NSS_VMAJOR > 3 ) || ( NSS_VMAJOR == 3 && NSS_VMINOR > 12 ) || ( NSS_VMAJOR == 3 && NSS_VMINOR == 12 && NSS_VPATCH > 0 )
        cvin[ncvinCount].type = cert_pi_useAIACertFetch;
        cvin[ncvinCount].value.scalar.b = PR_TRUE;
        ncvinCount++;
#endif

        PRUint64 revFlagsLeaf[2];
        PRUint64 revFlagsChain[2];
        CERTRevocationFlags rev;
        rev.leafTests.number_of_defined_methods = 2;
        rev.leafTests.cert_rev_flags_per_method = revFlagsLeaf;
        //the flags are defined in cert.h
        //We check both leaf and chain.
        //It is enough if one revocation method has fresh info,
        //but at least one must have some. Otherwise validation fails.
        //!!! using leaf test and CERT_REV_MI_REQUIRE_SOME_FRESH_INFO_AVAILABLE
        // when validating a root certificate will result in "revoked". Usually
        //there is no revocation information available for the root cert because
        //it must be trusted anyway and it does itself issue revocation information.
        //When we use the flag here and OOo shows the certification path then the root
        //cert is invalid while all other can be valid. It would probably best if
        //this interface method returned the whole chain.
        //Otherwise we need to check if the certificate is self-signed and if it is
        //then not use the flag when doing the leaf-test.
        rev.leafTests.cert_rev_flags_per_method[cert_revocation_method_crl] =
            CERT_REV_M_TEST_USING_THIS_METHOD
            | CERT_REV_M_IGNORE_IMPLICIT_DEFAULT_SOURCE;
        rev.leafTests.cert_rev_flags_per_method[cert_revocation_method_ocsp] =
            CERT_REV_M_TEST_USING_THIS_METHOD
            | CERT_REV_M_IGNORE_IMPLICIT_DEFAULT_SOURCE;
        rev.leafTests.number_of_preferred_methods = 0;
        rev.leafTests.preferred_methods = NULL;
        rev.leafTests.cert_rev_method_independent_flags =
            CERT_REV_MI_TEST_ALL_LOCAL_INFORMATION_FIRST;

        rev.chainTests.number_of_defined_methods = 2;
        rev.chainTests.cert_rev_flags_per_method = revFlagsChain;
        rev.chainTests.cert_rev_flags_per_method[cert_revocation_method_crl] =
            CERT_REV_M_TEST_USING_THIS_METHOD
            | CERT_REV_M_IGNORE_IMPLICIT_DEFAULT_SOURCE;
        rev.chainTests.cert_rev_flags_per_method[cert_revocation_method_ocsp] =
            CERT_REV_M_TEST_USING_THIS_METHOD
            | CERT_REV_M_IGNORE_IMPLICIT_DEFAULT_SOURCE;
        rev.chainTests.number_of_preferred_methods = 0;
        rev.chainTests.preferred_methods = NULL;
        rev.chainTests.cert_rev_method_independent_flags =
            CERT_REV_MI_TEST_ALL_LOCAL_INFORMATION_FIRST;


        cvin[ncvinCount].type = cert_pi_revocationFlags;
        cvin[ncvinCount].value.pointer.revocation = &rev;
        ncvinCount++;
        // does not work, not implemented yet in 3.12.4
//         cvin[ncvinCount].type = cert_pi_keyusage;
//         cvin[ncvinCount].value.scalar.ui = KU_DIGITAL_SIGNATURE;
//         ncvinCount++;
        cvin[ncvinCount].type = cert_pi_end;

        cvout[0].type = cert_po_trustAnchor;
        cvout[0].value.pointer.cert = NULL;
        cvout[1].type = cert_po_errorLog;
        cvout[1].value.pointer.log = &log;
        cvout[2].type = cert_po_end;

        // We check SSL server certificates, CA certificates and signing sertificates.
        //
        // ToDo check keyusage, looking at CERT_KeyUsageAndTypeForCertUsage (
        // mozilla/security/nss/lib/certdb/certdb.c indicates that
        // certificateUsageSSLClient, certificateUsageSSLServer and certificateUsageSSLCA
        // are sufficient. They cover the key usages for digital signature, key agreement
        // and encipherment and certificate signature

        //never use the following usages because they are not checked properly
        // certificateUsageUserCertImport
        // certificateUsageVerifyCA
        // certificateUsageAnyCA
        // certificateUsageProtectedObjectSigner

        UsageDescription arUsages[5];
        arUsages[0] = UsageDescription( certificateUsageSSLClient, "certificateUsageSSLClient"  );
        arUsages[1] = UsageDescription( certificateUsageSSLServer, "certificateUsageSSLServer"  );
        arUsages[2] = UsageDescription( certificateUsageSSLCA, "certificateUsageSSLCA"  );
        arUsages[3] = UsageDescription( certificateUsageEmailSigner, "certificateUsageEmailSigner" );
        arUsages[4] = UsageDescription( certificateUsageEmailRecipient, "certificateUsageEmailRecipient" );

        int numUsages = SAL_N_ELEMENTS(arUsages);
        for (int i = 0; i < numUsages; i++)
        {
            xmlsec_trace("Testing usage %d of %d: %s (0x%x)", i + 1,
                      numUsages, arUsages[i].description, (int) arUsages[i].usage);

            status = CERT_PKIXVerifyCert(const_cast<CERTCertificate *>(cert), arUsages[i].usage,
                                         cvin, cvout, NULL);
            if( status == SECSuccess )
            {
                xmlsec_trace("CERT_PKIXVerifyCert returned SECSuccess.");
                //When an intermediate or root certificate is checked then we expect the usage
                //certificateUsageSSLCA. This, however, will be only set when in the trust settings dialog
                //the button "This certificate can identify websites" is checked. If for example only
                //"This certificate can identify mail users" is set then the end certificate can
                //be validated and the returned usage will conain certificateUsageEmailRecipient.
                //But checking directly the root or intermediate certificate will fail. In the
                //certificate path view the end certificate will be shown as valid but the others
                //will be displayed as invalid.

                validity = csss::CertificateValidity::VALID;
                xmlsec_trace("Certificate is valid.\n");
                CERTCertificate * issuerCert = cvout[0].value.pointer.cert;
                if (issuerCert)
                {
                    xmlsec_trace("Root certificate: %s", issuerCert->subjectName);
                    CERT_DestroyCertificate(issuerCert);
                };

                break;
            }
            else
            {
                PRIntn err = PR_GetError();
                xmlsec_trace("Error: , %d = %s", err, getCertError(err));

                /* Display validation results */
                if ( log.count > 0)
                {
                    CERTVerifyLogNode *node = NULL;
                    printChainFailure(&log);

                    for (node = log.head; node; node = node->next) {
                        if (node->cert)
                            CERT_DestroyCertificate(node->cert);
                    }
                    log.head = log.tail = NULL;
                    log.count = 0;
                }
                xmlsec_trace("Certificate is invalid.\n");
            }
        }

    }
    else
    {
        validity = ::com::sun::star::security::CertificateValidity::INVALID ;
    }

    //Destroying the temporary certificates
    std::vector<CERTCertificate*>::const_iterator cert_i;
    for (cert_i = vecTmpNSSCertificates.begin(); cert_i != vecTmpNSSCertificates.end(); ++cert_i)
    {
        xmlsec_trace("Destroying temporary certificate");
        CERT_DestroyCertificate(*cert_i);
    }
    return validity ;
}

sal_Int32 SecurityEnvironment_NssImpl::getCertificateCharacters(
    const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& aCert ) throw( ::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException ) {
    sal_Int32 characters ;
    const X509Certificate_NssImpl* xcert ;
    const CERTCertificate* cert ;

    Reference< XUnoTunnel > xCertTunnel( aCert, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xcert = reinterpret_cast<X509Certificate_NssImpl*>(
        sal::static_int_cast<sal_uIntPtr>(xCertTunnel->getSomething( X509Certificate_NssImpl::getUnoTunnelId() ))) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    cert = xcert->getNssCert() ;

    characters = 0x00000000 ;

    //Firstly, find out whether or not the cert is self-signed.
    if( SECITEM_CompareItem( &(cert->derIssuer), &(cert->derSubject) ) == SECEqual ) {
        characters |= ::com::sun::star::security::CertificateCharacters::SELF_SIGNED ;
    } else {
        characters &= ~ ::com::sun::star::security::CertificateCharacters::SELF_SIGNED ;
    }

    //Secondly, find out whether or not the cert has a private key.

    /*
     * i40394
     *
     * mmi : need to check whether the cert's slot is valid first
     */
    SECKEYPrivateKey* priKey = NULL;

    if (cert->slot != NULL)
    {
        priKey = PK11_FindPrivateKeyFromCert( cert->slot, ( CERTCertificate* )cert, NULL ) ;
    }
    if(priKey == NULL)
    {
        for (CIT_SLOTS is = m_Slots.begin(); is != m_Slots.end(); is++)
        {
            priKey = PK11_FindPrivateKeyFromCert(*is, (CERTCertificate*)cert, NULL);
            if (priKey)
                break;
        }
    }
    if( priKey != NULL ) {
        characters |=  ::com::sun::star::security::CertificateCharacters::HAS_PRIVATE_KEY ;

        SECKEY_DestroyPrivateKey( priKey ) ;
    } else {
        characters &= ~ ::com::sun::star::security::CertificateCharacters::HAS_PRIVATE_KEY ;
    }

    return characters ;
}

X509Certificate_NssImpl* NssCertToXCert( CERTCertificate* cert )
{
    X509Certificate_NssImpl* xcert ;

    if( cert != NULL ) {
        xcert = new X509Certificate_NssImpl() ;
        if( xcert == NULL ) {
            xcert = NULL ;
        } else {
            xcert->setCert( cert ) ;
        }
    } else {
        xcert = NULL ;
    }

    return xcert ;
}

X509Certificate_NssImpl* NssPrivKeyToXCert( SECKEYPrivateKey* priKey )
{
    CERTCertificate* cert ;
    X509Certificate_NssImpl* xcert ;

    if( priKey != NULL ) {
        cert = PK11_GetCertFromPrivateKey( priKey ) ;

        if( cert != NULL ) {
            xcert = NssCertToXCert( cert ) ;
        } else {
            xcert = NULL ;
        }

        CERT_DestroyCertificate( cert ) ;
    } else {
        xcert = NULL ;
    }

    return xcert ;
}


/* Native methods */
xmlSecKeysMngrPtr SecurityEnvironment_NssImpl::createKeysManager() throw( Exception, RuntimeException ) {

    unsigned int i ;
    CERTCertDBHandle* handler = NULL ;
    PK11SymKey* symKey = NULL ;
    SECKEYPublicKey* pubKey = NULL ;
    SECKEYPrivateKey* priKey = NULL ;
    xmlSecKeysMngrPtr pKeysMngr = NULL ;

    handler = this->getCertDb() ;

    /*-
     * The following lines is based on the private version of xmlSec-NSS
     * crypto engine
     */
    int cSlots = m_Slots.size();
    boost::scoped_array<PK11SlotInfo*> sarSlots(new PK11SlotInfo*[cSlots]);
    PK11SlotInfo**  slots = sarSlots.get();
    int count = 0;
    for (CIT_SLOTS islots = m_Slots.begin();islots != m_Slots.end(); islots++, count++)
        slots[count] = *islots;

    pKeysMngr = xmlSecNssAppliedKeysMngrCreate(slots, cSlots, handler ) ;
    if( pKeysMngr == NULL )
        throw RuntimeException() ;

    /*-
     * Adopt symmetric key into keys manager
     */
    for( i = 0 ; ( symKey = this->getSymKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecNssAppliedKeysMngrSymKeyLoad( pKeysMngr, symKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }

    /*-
     * Adopt asymmetric public key into keys manager
     */
    for( i = 0 ; ( pubKey = this->getPubKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecNssAppliedKeysMngrPubKeyLoad( pKeysMngr, pubKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }

    /*-
     * Adopt asymmetric private key into keys manager
     */
    for( i = 0 ; ( priKey = this->getPriKey( i ) ) != NULL ; i ++ ) {
        if( xmlSecNssAppliedKeysMngrPriKeyLoad( pKeysMngr, priKey ) < 0 ) {
            throw RuntimeException() ;
        }
    }
    return pKeysMngr ;
}
void SecurityEnvironment_NssImpl::destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) throw( Exception, RuntimeException ) {
    if( pKeysMngr != NULL ) {
        xmlSecKeysMngrDestroy( pKeysMngr ) ;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
