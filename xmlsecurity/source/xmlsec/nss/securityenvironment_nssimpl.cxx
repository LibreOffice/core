/*************************************************************************
 *
 *  $RCSfile: securityenvironment_nssimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-19 11:35:46 $
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

#ifndef _XSECURITYENVIRONMENT_NSSIMPL_HXX_
#include "securityenvironment_nssimpl.hxx"
#endif

#ifndef _X509CERTIFICATE_NSSIMPL_HXX_
#include "x509certificate_nssimpl.hxx"
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#include "nspr.h"
#include "nss.h"
#include "secport.h"
#include "secitem.h"
#include "secder.h"
#include "secerr.h"
#include "limits.h"

#include "xmlsec/xmlsec.h"
#include "xmlsec/keysmngr.h"
#include "xmlsec/crypto.h"
#include <xmlsec/base64.h>
#include "xmlsec/strings.h"

#include <tools/string.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <svtools/docpasswdrequest.hxx>

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

// MM : added for password exception
#include <com/sun/star/security/NoPasswordException.hpp>
using namespace ::com::sun::star::security;

using namespace com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::security::XCertificate ;

extern X509Certificate_NssImpl* NssCertToXCert( CERTCertificate* cert ) ;
extern X509Certificate_NssImpl* NssPrivKeyToXCert( SECKEYPrivateKey* ) ;
extern Sequence< sal_Int8 > numericStringToBigInteger( OUString ) ;
extern OUString bigIntegerToNumericString( Sequence< sal_Int8 > serial ) ;

char* GetPasswordFunction( PK11SlotInfo* pSlot, PRBool bRetry, void* arg )
{
    char* pPassword = NULL;

    uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
    if ( xMSF.is() )
    {
        uno::Reference < task::XInteractionHandler > xInteractionHandler(
            xMSF->createInstance( rtl::OUString::createFromAscii("com.sun.star.task.InteractionHandler") ), uno::UNO_QUERY );

        if ( xInteractionHandler.is() )
        {
            task::PasswordRequestMode eMode = bRetry ? task::PasswordRequestMode_PASSWORD_REENTER : task::PasswordRequestMode_PASSWORD_ENTER;
            RequestDocumentPassword* pPasswordRequest = new RequestDocumentPassword( eMode, ::rtl::OUString() );

            uno::Reference< task::XInteractionRequest > xRequest( pPasswordRequest );
            xInteractionHandler->handle( xRequest );

            if ( pPasswordRequest->isPassword() )
            {
                // We need TOOLS anyway, because of class RequestDocumentPassword.
                // If this changes, use rtl String functions for this...
                ByteString aPassword = ByteString( String( pPasswordRequest->getPassword() ), gsl_getSystemTextEncoding() );
                USHORT nLen = aPassword.Len();
                pPassword = (char*) PORT_Alloc( nLen+1 ) ;
                pPassword[nLen] = 0;
                memcpy( pPassword, aPassword.GetBuffer(), nLen );
            }
        }
    }
#ifdef DEBUG
    else
    {
        // TEST Password is 'sceri'
        pPassword = ( char* )PORT_Alloc( 20 ) ;
        pPassword[0]='s';
        pPassword[1]='c';
        pPassword[2]='e';
        pPassword[3]='r';
        pPassword[4]='i';
        pPassword[5]=0x0;
   }
#endif

    return pPassword;
}

SecurityEnvironment_NssImpl :: SecurityEnvironment_NssImpl( const Reference< XMultiServiceFactory >& aFactory ) : m_pSlot( NULL ) , m_pHandler( NULL ) , m_tSymKeyList() , m_tPubKeyList() , m_tPriKeyList() {

    PK11_SetPasswordFunc( GetPasswordFunction ) ;
}

SecurityEnvironment_NssImpl :: ~SecurityEnvironment_NssImpl() {

    PK11_SetPasswordFunc( NULL ) ;

    if( m_pSlot != NULL ) {
        PK11_FreeSlot( m_pSlot ) ;
        m_pSlot = NULL ;
    }

    if( !m_tSymKeyList.empty()  ) {
        std::list< PK11SymKey* >::iterator symKeyIt ;

        for( symKeyIt = m_tSymKeyList.begin() ; symKeyIt != m_tSymKeyList.end() ; symKeyIt ++ )
            PK11_FreeSymKey( *symKeyIt ) ;
    }

    if( !m_tPubKeyList.empty()  ) {
        std::list< SECKEYPublicKey* >::iterator pubKeyIt ;

        for( pubKeyIt = m_tPubKeyList.begin() ; pubKeyIt != m_tPubKeyList.end() ; pubKeyIt ++ )
            SECKEY_DestroyPublicKey( *pubKeyIt ) ;
    }

    if( !m_tPriKeyList.empty()  ) {
        std::list< SECKEYPrivateKey* >::iterator priKeyIt ;

        for( priKeyIt = m_tPriKeyList.begin() ; priKeyIt != m_tPriKeyList.end() ; priKeyIt ++ )
            SECKEY_DestroyPrivateKey( *priKeyIt ) ;
    }
}

/* XInitialization */
void SAL_CALL SecurityEnvironment_NssImpl :: initialize( const Sequence< Any >& aArguments ) throw( Exception, RuntimeException ) {
    // TBD
} ;

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
    seqServiceNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.xml.crypto.SecurityEnvironment" ) ;
    return seqServiceNames ;
}

OUString SecurityEnvironment_NssImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString::createFromAscii( "com.sun.star.xml.security.bridge.xmlsec.SecurityEnvironment_NssImpl" ) ;
}

//Helper for registry
Reference< XInterface > SAL_CALL SecurityEnvironment_NssImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new SecurityEnvironment_NssImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > SecurityEnvironment_NssImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL SecurityEnvironment_NssImpl :: getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw( RuntimeException )
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return ( sal_Int64 )this ;
    }
    return 0 ;
}

/* XUnoTunnel extension */
const Sequence< sal_Int8>& SecurityEnvironment_NssImpl :: getUnoTunnelId() {
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
SecurityEnvironment_NssImpl* SecurityEnvironment_NssImpl :: getImplementation( const Reference< XInterface > xObj ) {
    Reference< XUnoTunnel > xUT( xObj , UNO_QUERY ) ;
    if( xUT.is() ) {
        return ( SecurityEnvironment_NssImpl* )xUT->getSomething( getUnoTunnelId() ) ;
    } else
        return NULL ;
}

/* Native methods */
PK11SlotInfo* SecurityEnvironment_NssImpl :: getCryptoSlot() throw( Exception , RuntimeException ) {
    return m_pSlot ;
}

void SecurityEnvironment_NssImpl :: setCryptoSlot( PK11SlotInfo* aSlot) throw( Exception , RuntimeException ) {
    if( m_pSlot != NULL ) {
        PK11_FreeSlot( m_pSlot ) ;
        m_pSlot = NULL ;
    }

    if( aSlot != NULL ) {
        m_pSlot = PK11_ReferenceSlot( aSlot ) ;
    }
}

CERTCertDBHandle* SecurityEnvironment_NssImpl :: getCertDb() throw( Exception , RuntimeException ) {
    return m_pHandler ;
}

void SecurityEnvironment_NssImpl :: setCertDb( CERTCertDBHandle* aCertDb ) throw( Exception , RuntimeException ) {
    m_pHandler = aCertDb ;
}

void SecurityEnvironment_NssImpl :: adoptSymKey( PK11SymKey* aSymKey ) throw( Exception , RuntimeException ) {
    PK11SymKey* symkey ;
    std::list< PK11SymKey* >::iterator keyIt ;

    if( aSymKey != NULL ) {
        //First try to find the key in the list
        for( keyIt = m_tSymKeyList.begin() ; keyIt != m_tSymKeyList.end() ; keyIt ++ ) {
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
        for( keyIt = m_tSymKeyList.begin() ; keyIt != m_tSymKeyList.end() ; keyIt ++ ) {
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
        for( keyIt = m_tPubKeyList.begin() ; keyIt != m_tPubKeyList.end() ; keyIt ++ ) {
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
        for( keyIt = m_tPubKeyList.begin() ; keyIt != m_tPubKeyList.end() ; keyIt ++ ) {
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
        for( keyIt = m_tPriKeyList.begin() ; keyIt != m_tPriKeyList.end() ; keyIt ++ ) {
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
        for( keyIt = m_tPriKeyList.begin() ; keyIt != m_tPriKeyList.end() ; keyIt ++ ) {
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

Sequence< Reference < XCertificate > > SecurityEnvironment_NssImpl :: getPersonalCertificates() throw( SecurityException , RuntimeException ) {
    sal_Int32 length ;
    X509Certificate_NssImpl* xcert ;
    std::list< X509Certificate_NssImpl* > certsList ;

    //firstly, we try to find private keys in slot
    if( m_pSlot != NULL ) {
        SECKEYPrivateKeyList* priKeyList ;
        SECKEYPrivateKeyListNode* curPri ;

        if( PK11_NeedLogin( m_pSlot ) ) {
            SECStatus nRet = PK11_Authenticate( m_pSlot, PR_TRUE, NULL );
            if( nRet != SECSuccess ) {
                throw NoPasswordException();
            }
        }

        priKeyList = PK11_ListPrivateKeysInSlot( m_pSlot ) ;
        if( priKeyList != NULL ) {
            for( curPri = PRIVKEY_LIST_HEAD( priKeyList ); !PRIVKEY_LIST_END( curPri, priKeyList ) && curPri != NULL ; curPri = PRIVKEY_LIST_NEXT( curPri ) ) {
                xcert = NssPrivKeyToXCert( curPri->key ) ;
                if( xcert != NULL )
                    certsList.push_back( xcert ) ;
            }
        }

        SECKEY_DestroyPrivateKeyList( priKeyList ) ;
    }

    //Deprecated
    /*-------
    {
        PK11SlotList* soltList ;
        PK11SlotListElement* soltEle ;

        SECKEYPrivateKeyList* priKeyList ;
        SECKEYPrivateKeyListNode* curPri ;

        soltList = PK11_GetAllTokens( CKM_INVALID_MECHANISM, PR_FALSE, PR_FALSE, NULL ) ;
        if( soltList != NULL ) {
            for( soltEle = soltList->head; soltEle != NULL; soltEle = soltEle->next ) {
                if( PK11_NeedLogin( soltEle->slot ) ) {
                    if( PK11_Authenticate( soltEle->slot, PR_TRUE, NULL ) != SECSuccess ) {
                        return NULL ;
                    }
                }

                priKeyList = PK11_ListPrivateKeysInSlot( soltEle->slot ) ;
                if( priKeyList != NULL ) {
                    for( curPri = PRIVKEY_LIST_HEAD( priKeyList ); !PRIVKEY_LIST_END( curPri, priKeyList ) && curPri != NULL ; curPri = PRIVKEY_LIST_NEXT( curPri ) ) {
                        xcert = NssPrivKeyToXCert( curPri->key ) ;
                        if( xcert != NULL )
                            certsList.push_back( xcert ) ;
                    }
                }

                SECKEY_DestroyPrivateKeyList( priKeyList ) ;
            }
        }
    }
    ----------*/

    //secondly, we try to find certificate from registered private keys.
    if( !m_tPriKeyList.empty()  ) {
        std::list< SECKEYPrivateKey* >::iterator priKeyIt ;

        for( priKeyIt = m_tPriKeyList.begin() ; priKeyIt != m_tPriKeyList.end() ; priKeyIt ++ ) {
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

        for( i = 0, xcertIt = certsList.begin(); xcertIt != certsList.end(); xcertIt ++, i++ ) {
            certSeq[i] = *xcertIt ;
        }

        return certSeq ;
    }

    return NULL ;
}

Reference< XCertificate > SecurityEnvironment_NssImpl :: getCertificate( const OUString& issuerName, const Sequence< sal_Int8 >& serialNumber ) throw( SecurityException , RuntimeException ) {
    X509Certificate_NssImpl* xcert ;

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

        rtl::OString ostr = rtl::OUStringToOString( issuerName , RTL_TEXTENCODING_ASCII_US ) ;
        chIssuer = PL_strndup( ( char* )ostr.getStr(), ( int )ostr.getLength() ) ;
        nmIssuer = CERT_AsciiToName( chIssuer ) ;
        if( nmIssuer == NULL ) {
            PL_strfree( chIssuer ) ;
            PORT_FreeArena( arena, PR_FALSE ) ;
            throw RuntimeException() ;
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

    xcert = ( X509Certificate_NssImpl* )xCertTunnel->getSomething( X509Certificate_NssImpl::getUnoTunnelId() ) ;
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

    return NULL ;
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

Reference< XCertificate > SecurityEnvironment_NssImpl :: createCertificateFromAscii( const OUString& asciiCertificate ) throw( SecurityException , RuntimeException ) {
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

sal_Int32 SecurityEnvironment_NssImpl :: verifyCertificate( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& aCert ) throw( ::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException ) {
    sal_Int32 validity ;
    const X509Certificate_NssImpl* xcert ;
    const CERTCertificate* cert ;

    Reference< XUnoTunnel > xCertTunnel( aCert, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xcert = ( X509Certificate_NssImpl* )xCertTunnel->getSomething( X509Certificate_NssImpl::getUnoTunnelId() ) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    cert = xcert->getNssCert() ;
    if( cert != NULL ) {
        int64 timeboundary ;
        SECStatus status ;

        //Get the system clock time
        timeboundary = PR_Now() ;

        if( m_pHandler != NULL ) {
            status = CERT_VerifyCertificate( m_pHandler, ( CERTCertificate* )cert, PR_FALSE, (SECCertificateUsage)0, timeboundary , NULL, NULL, NULL ) ;
        } else {
            status = CERT_VerifyCertificate( CERT_GetDefaultCertDB(), ( CERTCertificate* )cert, PR_FALSE, (SECCertificateUsage)0, timeboundary , NULL, NULL, NULL ) ;
        }

        if( status == SECSuccess ) {
            validity = ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_VALID ;
        } else {
            validity = ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_INVALID ;
            switch( status ) {
                case SEC_ERROR_BAD_SIGNATURE :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_SIGNATURE_INVALID ;
                    break ;
                case SEC_ERROR_EXPIRED_CERTIFICATE :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_TIMEOUT ;
                    break ;
                case SEC_ERROR_REVOKED_CERTIFICATE :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_REVOKED ;
                    break ;
                case SEC_ERROR_UNKNOWN_ISSUER :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_ISSUER_UNKNOWN ;
                    break ;
                case SEC_ERROR_UNTRUSTED_ISSUER :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_ISSUER_UNTRUSTED ;
                    break ;
                case SEC_ERROR_UNTRUSTED_CERT :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_UNTRUSTED ;
                    break ;
                case SEC_ERROR_CERT_VALID :
                case SEC_ERROR_CERT_NOT_VALID :
                    break ;
                case SEC_ERROR_EXPIRED_ISSUER_CERTIFICATE :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_ISSUER_INVALID ;
                    break ;
                case SEC_ERROR_CA_CERT_INVALID :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_ROOT_INVALID ;
                    break ;
                case SEC_ERROR_UNKNOWN_CRITICAL_EXTENSION :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_EXTENSION_INVALID ;
                    break ;
                case SEC_ERROR_UNKNOWN_CERT :
                    validity |= ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_CHAIN_INCOMPLETE ;
                    break ;
            }
        }
    } else {
        validity = ::com::sun::star::security::CertificateValidity::CERT_VALIDITY_INVALID ;
    }

    return validity ;
}

sal_Int32 SecurityEnvironment_NssImpl :: getCertificateCharacters( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& aCert ) throw( ::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException ) {
    sal_Int32 characters ;
    const X509Certificate_NssImpl* xcert ;
    const CERTCertificate* cert ;

    Reference< XUnoTunnel > xCertTunnel( aCert, UNO_QUERY ) ;
    if( !xCertTunnel.is() ) {
        throw RuntimeException() ;
    }

    xcert = ( X509Certificate_NssImpl* )xCertTunnel->getSomething( X509Certificate_NssImpl::getUnoTunnelId() ) ;
    if( xcert == NULL ) {
        throw RuntimeException() ;
    }

    cert = xcert->getNssCert() ;

    characters = 0x00000000 ;

    //Firstly, make sentence whether or not the cert is self-signed.
    if( SECITEM_CompareItem( &(cert->derIssuer), &(cert->derSubject) ) ) {
        characters |= ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_SELF_SIGNED ;
    } else {
        characters &= ~ ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_SELF_SIGNED ;
    }

    //Secondly, make sentence whether or not the cert has a private key.
    {
        SECKEYPrivateKey* priKey ;

        priKey = PK11_FindPrivateKeyFromCert( cert->slot, ( CERTCertificate* )cert, NULL ) ;
        if( priKey == NULL || m_pSlot != NULL )
            priKey = PK11_FindPrivateKeyFromCert( m_pSlot, ( CERTCertificate* )cert, NULL ) ;

        if( priKey != NULL ) {
            characters |=  ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_HAS_PRIVATE_KEY ;

            SECKEY_DestroyPrivateKey( priKey ) ;
        } else {
            characters &= ~ ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_HAS_PRIVATE_KEY ;
        }
    }

    //Thirdly, make sentence whether or not the cert is trusted.
    {
        CERTCertificate* tempCert ;
        CERTIssuerAndSN issuerAndSN ;

        memset( &issuerAndSN, 0, sizeof( issuerAndSN ) ) ;

        issuerAndSN.derIssuer.data = cert->derIssuer.data;
        issuerAndSN.derIssuer.len = cert->derIssuer.len;

        issuerAndSN.serialNumber.data = cert->serialNumber.data;
        issuerAndSN.serialNumber.len = cert->serialNumber.len;

        if( m_pSlot != NULL )
            tempCert = PK11_FindCertByIssuerAndSN( &m_pSlot, &issuerAndSN, NULL ) ;
        else
            tempCert = NULL ;

        if( tempCert != NULL ) {
                characters |=  ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_TRUSTED ;
                CERT_DestroyCertificate( tempCert ) ;
            } else {
                characters &= ~ ::com::sun::star::security::CertificateCharacters::CERT_CHARACTER_TRUSTED ;
            }
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

/*-
 * This is just one temporary conversion
 */
Sequence< sal_Int8 > numericStringToBigInteger (
    OUString serialNumber
) {
    char* chSerial ;
    unsigned long ui ;
    unsigned char bb[5] ;
    int len ;

    rtl::OString oseri = rtl::OUStringToOString( serialNumber , RTL_TEXTENCODING_ASCII_US ) ;

    chSerial = PL_strndup( ( char* )oseri.getStr(), ( int )oseri.getLength() ) ;
    ui = PORT_Atoi( chSerial ) ;
    PL_strfree( chSerial ) ;

    bb[0] = 0;
    bb[1] = (unsigned char) (ui >> 24);
    bb[2] = (unsigned char) (ui >> 16);
    bb[3] = (unsigned char) (ui >> 8);
    bb[4] = (unsigned char) (ui);

    /*
    ** Small integers are encoded in a single byte. Larger integers
    ** require progressively more space.
    */
    if (ui > 0x7f) {
        if (ui > 0x7fff) {
            if (ui > 0x7fffffL) {
                if (ui >= 0x80000000L) {
                    len = 5;
                } else {
                    len = 4;
                }
            } else {
                len = 3;
            }
        } else {
            len = 2;
        }
    } else {
        len = 1;
    }

    Sequence< sal_Int8 > serial( len ) ;
    for( int i = 0 ; i < len ; i ++ )
        serial[i] = *( bb + sizeof( bb ) - len + i ) ;

    return serial ;
}

/*-
 * This is just one temporary conversion
 */
OUString bigIntegerToNumericString ( Sequence< sal_Int8 > serial)
{
    OUString aRet;

    if ( serial.getLength() )
    {
        SECItem snItem ;
        long sn ;

        snItem.data = ( unsigned char* )&serial[0] ;
        snItem.len = serial.getLength() ;

        sn = DER_GetInteger( &snItem ) ;
        if( sn != ULONG_MAX )
        {
            char str[10] ;
            int len ;
            len = sprintf( str, "%d", sn ) ;
            aRet = OUString::createFromAscii( str ) ;
        }
    }
    return aRet;
}
