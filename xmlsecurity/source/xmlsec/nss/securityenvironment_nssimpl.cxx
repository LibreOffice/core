/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: securityenvironment_nssimpl.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:16:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

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
#include "certt.h"
#include "prerror.h"

#include <xmlsec/xmlsec.h>
#include <xmlsec/keysmngr.h>
#include <xmlsec/crypto.h>
#include <xmlsec/base64.h>
#include <xmlsec/strings.h>

#include <tools/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <svtools/docpasswdrequest.hxx>
#include <xmlsecurity/biginteger.hxx>
#include <rtl/logfile.h>

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#include "boost/scoped_array.hpp"

// MM : added for password exception
#include <com/sun/star/security/NoPasswordException.hpp>
namespace csss = ::com::sun::star::security;
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

char* GetPasswordFunction( PK11SlotInfo* pSlot, PRBool bRetry, void* /*arg*/ )
{
    uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
    if ( xMSF.is() )
    {
        uno::Reference < task::XInteractionHandler > xInteractionHandler(
            xMSF->createInstance( rtl::OUString::createFromAscii("com.sun.star.task.InteractionHandler") ), uno::UNO_QUERY );

        if ( xInteractionHandler.is() )
        {
            task::PasswordRequestMode eMode = bRetry ? task::PasswordRequestMode_PASSWORD_REENTER : task::PasswordRequestMode_PASSWORD_ENTER;
            RequestDocumentPassword* pPasswordRequest = new RequestDocumentPassword( eMode, ::rtl::OUString::createFromAscii(PK11_GetTokenName(pSlot)) );

            uno::Reference< task::XInteractionRequest > xRequest( pPasswordRequest );
            xInteractionHandler->handle( xRequest );

            if ( pPasswordRequest->isPassword() )
            {
                ByteString aPassword = ByteString( String( pPasswordRequest->getPassword() ), gsl_getSystemTextEncoding() );
                USHORT nLen = aPassword.Len();
                char* pPassword = (char*) PORT_Alloc( nLen+1 ) ;
                pPassword[nLen] = 0;
                memcpy( pPassword, aPassword.GetBuffer(), nLen );
                return pPassword;
            }
        }
    }
    return NULL;
}

SecurityEnvironment_NssImpl :: SecurityEnvironment_NssImpl( const Reference< XMultiServiceFactory >& ) :
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
void SAL_CALL SecurityEnvironment_NssImpl :: initialize( const Sequence< Any >& ) throw( Exception, RuntimeException ) {
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
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
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
        return reinterpret_cast<SecurityEnvironment_NssImpl*>(
            sal::static_int_cast<sal_uIntPtr>(xUT->getSomething( getUnoTunnelId() ))) ;
    } else
        return NULL ;
}


::rtl::OUString SecurityEnvironment_NssImpl::getSecurityEnvironmentInformation() throw( ::com::sun::star::uno::RuntimeException )
{
    rtl::OUString result;
    ::rtl::OUStringBuffer buff;
    for (CIT_SLOTS is = m_Slots.begin(); is != m_Slots.end(); is++)
    {
        buff.append(rtl::OUString::createFromAscii(PK11_GetTokenName(*is)));
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
                RTL_LOGFILE_TRACE2( "XMLSEC: Found a slot: SlotName=%s, TokenName=%s", PK11_GetSlotName(pSlot), PK11_GetTokenName(pSlot) );
                pSymKey = PK11_KeyGen( pSlot , CKM_DES3_CBC, NULL, 128, NULL ) ;
                if( pSymKey == NULL )
                {
                    PK11_FreeSlot( pSlot ) ;
                    RTL_LOGFILE_TRACE( "XMLSEC: Error - pSymKey is NULL" );
                    continue;
                }

                addCryptoSlot(pSlot);
                PK11_FreeSlot( pSlot ) ;
                pSlot = NULL;

                adoptSymKey( pSymKey ) ;
                PK11_FreeSymKey( pSymKey ) ;
                pSymKey = NULL;

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
        if( priKeyList != NULL ) {
            for( curPri = PRIVKEY_LIST_HEAD( priKeyList );
                !PRIVKEY_LIST_END( curPri, priKeyList ) && curPri != NULL ;
                curPri = PRIVKEY_LIST_NEXT( curPri ) ) {
                xcert = NssPrivKeyToXCert( curPri->key ) ;
                if( xcert != NULL )
                    certsList.push_back( xcert ) ;
            }
        }

        SECKEY_DestroyPrivateKeyList( priKeyList ) ;
    }

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

                /*
                 * mmi : because MS Crypto use the 'S' tag (equal to the 'ST' tag in NSS), but the NSS can't recognise
                 *      it, so the 'S' tag should be changed to 'ST' tag
                 *
                 * PS  : it can work, but inside libxmlsec, the 'S' tag is till used to find cert in NSS engine, so it
                 *       is not useful at all. (comment out now)
                 */

                /*
                sal_Int32 nIndex = 0;
                OUString newIssuerName;
                do
                {
                    OUString aToken = issuerName.getToken( 0, ',', nIndex ).trim();
                    if (aToken.compareToAscii("S=",2) == 0)
                    {
                        newIssuerName+=OUString::createFromAscii("ST=");
                        newIssuerName+=aToken.copy(2);
                    }
                    else
                    {
                        newIssuerName+=aToken;
                    }

                    if (nIndex >= 0)
                    {
                        newIssuerName+=OUString::createFromAscii(",");
                    }
                } while ( nIndex >= 0 );
                */

                /* end */

        //Create cert info from issue and serial
        rtl::OString ostr = rtl::OUStringToOString( issuerName , RTL_TEXTENCODING_UTF8 ) ;
        chIssuer = PL_strndup( ( char* )ostr.getStr(), ( int )ostr.getLength() ) ;
        nmIssuer = CERT_AsciiToName( chIssuer ) ;
        if( nmIssuer == NULL ) {
            PL_strfree( chIssuer ) ;
            PORT_FreeArena( arena, PR_FALSE ) ;

            /*
             * i40394
             *
             * mmi : no need to throw exception
             *       just return "no found"
             */
            //throw RuntimeException() ;
            return NULL;
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
    sal_Int32 validity = 0;
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
    if( cert != NULL )
        {
        int64 timeboundary ;
        SECStatus status ;

        //Get the system clock time
        timeboundary = PR_Now() ;
        SECCertificateUsage usage = 0;

        // create log

    CERTVerifyLog realLog;
        CERTVerifyLog *log;

    log = &realLog;


    log->count = 0;
    log->head = NULL;
    log->tail = NULL;
        log->arena = PORT_NewArena( DER_DEFAULT_CHUNKSIZE );

        //CERTVerifyLog *log;
        //PRArenaPool *arena;

        //arena = PORT_NewArena( DER_DEFAULT_CHUNKSIZE );
        //log = PORT_ArenaZNew( arena, CERTVerifyLog );
        //log->arena = arena;
        validity = csss::CertificateValidity::INVALID;

        CERTCertificateList * certList;

        certList = CERT_CertChainFromCert( (CERTCertificateStr *) cert, (SECCertUsage) 0, 0);


        if( m_pHandler != NULL )
        {
            //JL: We must not pass a particular usage in the requiredUsages argument (the 4th) because,
            //then ONLY these are verified. For example, we pass
            //certificateUsageSSLClient | certificateUsageSSLServer. Then checking a certificate which
            // is a valid certificateUsageEmailSigner but no certificateUsageSSLClient | certificateUsageSSLServer
            //will result in CertificateValidity::INVALID.
            //Only if the argument "requiredUsages" has a value (other than zero)
            //then the function will return SECFailure in case
            //the certificate is not suitable for the provided usage. That is, in the previous
            //example the function returns SECFailure.
            status = CERT_VerifyCertificate(
                m_pHandler, ( CERTCertificate* )cert, PR_TRUE,
                (SECCertificateUsage)0, timeboundary , NULL, log, &usage);
        }
        else
        {
            status = CERT_VerifyCertificate(
                CERT_GetDefaultCertDB(), ( CERTCertificate* )cert,
                PR_TRUE, (SECCertificateUsage)0, timeboundary ,NULL, log, &usage);
        }

        if( status == SECSuccess )
        {
            // JL & TKR : certificateUsageUserCertImport,
            // certificateUsageVerifyCA and certificateUsageAnyCA dont check the chain

            //When an intermediate or root certificate is checked then we expect the usage
            //certificateUsageSSLCA. This, however, will be only set when in the trust settings dialog
            //the button "This certificate can identify websites" is checked. If for example only
            //"This certificate can identify mail users" is set then the end certificate can
            //be validated and the returned usage will conain certificateUsageEmailRecipient.
            //But checking directly the root or intermediate certificate will fail. In the
            //certificate path view the end certificate will be shown as valid but the others
            //will be displayed as invalid.

            if (usage & certificateUsageEmailSigner
                || usage & certificateUsageEmailRecipient
                || usage & certificateUsageSSLCA
                || usage & certificateUsageSSLServer
                || usage & certificateUsageSSLClient
                // || usage & certificateUsageUserCertImport
                // || usage & certificateUsageVerifyCA
                || usage & certificateUsageStatusResponder )
                // || usage & certificateUsageAnyCA )
                validity = csss::CertificateValidity::VALID;
            else
                validity = csss::CertificateValidity::INVALID;

        }
        // always check what kind of error occured, even SECStatus says Success
        //JL: When we call CERT_VerifyCertificate whit the parameter requiredUsages == 0 then all
        //possible usages are checked. Then there are certainly usages for which the certificate
        //is not intended. For these usages there will be NO flag set in the argument returnedUsages
        // (the last arg) and there will be error codes set in the log. Therefore we cannot
        //set the CertificateValidity to INVALID because there is a log entry.
//         CERTVerifyLogNode *logNode = 0;

//         logNode = log->head;
//         while ( logNode != NULL )
//         {
//             sal_Int32 errorCode = 0;
//             errorCode = logNode->error;

//             switch ( errorCode )
//             {
//                 // JL & TKR: Any error are treated as invalid because we cannot say that we get all occurred errors from NSS
// /*
//                 case ( SEC_ERROR_REVOKED_CERTIFICATE ):
//                     validity |= csss::CertificateValidity::REVOKED;
//                 break;
//                 case ( SEC_ERROR_EXPIRED_CERTIFICATE ):
//                     validity |= csss::CertificateValidity::TIME_INVALID;
//                 break;
//                 case ( SEC_ERROR_CERT_USAGES_INVALID):
//                     validity |= csss::CertificateValidity::INVALID;
//                 break;
//                 case ( SEC_ERROR_UNTRUSTED_ISSUER ):
//                 case ( SEC_ERROR_UNTRUSTED_CERT ):
//                     validity |= csss::CertificateValidity::UNTRUSTED;
//                 break;
//  */
//                 default:
//                     validity |= csss::CertificateValidity::INVALID;
//                 break;
//             }
//             logNode = logNode->next;
//        }
    }
    else
    {
        validity = ::com::sun::star::security::CertificateValidity::INVALID ;
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
