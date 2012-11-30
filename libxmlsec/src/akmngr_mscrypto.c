/**
 * XMLSec library
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 *
 * Copyright.........................
 */
#include "globals.h"

#include <xmlsec/xmlsec.h>
#include <xmlsec/keys.h>
#include <xmlsec/keysmngr.h>
#include <xmlsec/transforms.h>
#include <xmlsec/errors.h>

#include <xmlsec/mscrypto/crypto.h>
#include <xmlsec/mscrypto/keysstore.h>
#include <xmlsec/mscrypto/akmngr.h>
#include <xmlsec/mscrypto/x509.h>

/**
 * xmlSecMSCryptoAppliedKeysMngrCreate:
 * @hKeyStore:      the pointer to key store.
 * @hCertStore:     the pointer to certificate database.
 *
 * Create and load key store and certificate database into keys manager
 *
 * Returns keys manager pointer on success or NULL otherwise.
 */
xmlSecKeysMngrPtr
xmlSecMSCryptoAppliedKeysMngrCreate(
    HCERTSTORE hKeyStore ,
    HCERTSTORE hCertStore
) {
    xmlSecKeyDataStorePtr   certStore = NULL ;
    xmlSecKeysMngrPtr       keyMngr = NULL ;
    xmlSecKeyStorePtr       keyStore = NULL ;

    keyStore = xmlSecKeyStoreCreate( xmlSecMSCryptoKeysStoreId ) ;
    if( keyStore == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeyStoreCreate" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return NULL ;
    }

    /*-
     * At present, MS Crypto engine do not provide a way to setup a key store.
     */
    if( keyStore != NULL ) {
        /*TODO: binding key store.*/
    }

    keyMngr = xmlSecKeysMngrCreate() ;
    if( keyMngr == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeysMngrCreate" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;

        xmlSecKeyStoreDestroy( keyStore ) ;
        return NULL ;
    }

    /*-
     * Add key store to manager, from now on keys manager destroys the store if
     * needed
     */
    if( xmlSecKeysMngrAdoptKeysStore( keyMngr, keyStore ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecKeyStoreGetName( keyStore ) ) ,
            "xmlSecKeysMngrAdoptKeyStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;

        xmlSecKeyStoreDestroy( keyStore ) ;
        xmlSecKeysMngrDestroy( keyMngr ) ;
        return NULL ;
    }

    /*-
     * Initialize crypto library specific data in keys manager
     */
    if( xmlSecMSCryptoKeysMngrInit( keyMngr ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecMSCryptoKeysMngrInit" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;

        xmlSecKeysMngrDestroy( keyMngr ) ;
        return NULL ;
    }

    /*-
     * Set certificate databse to X509 key data store
     */
    /*-
     * At present, MS Crypto engine do not provide a way to setup a cert store.
     */

    /*-
     * Set the getKey callback
     */
    keyMngr->getKey = xmlSecKeysMngrGetKey ;

    return keyMngr ;
}

int
xmlSecMSCryptoAppliedKeysMngrSymKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    HCRYPTKEY           symKey
) {
    /*TODO: import the key into keys manager.*/
    return(0) ;
}

int
xmlSecMSCryptoAppliedKeysMngrPubKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    HCRYPTKEY   pubKey
) {
    /*TODO: import the key into keys manager.*/
    return(0) ;
}

int
xmlSecMSCryptoAppliedKeysMngrPriKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    HCRYPTKEY   priKey
) {
    /*TODO: import the key into keys manager.*/
    return(0) ;
}

int
xmlSecMSCryptoAppliedKeysMngrAdoptKeyStore (
    xmlSecKeysMngrPtr   mngr ,
    HCERTSTORE keyStore
) {
    xmlSecKeyDataStorePtr x509Store ;

    xmlSecAssert2( mngr != NULL, -1 ) ;
    xmlSecAssert2( keyStore != NULL, -1 ) ;

    x509Store = xmlSecKeysMngrGetDataStore( mngr, xmlSecMSCryptoX509StoreId ) ;
    if( x509Store == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeysMngrGetDataStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( -1 ) ;
    }

    if( xmlSecMSCryptoX509StoreAdoptKeyStore( x509Store, keyStore ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecKeyDataStoreGetName( x509Store ) ) ,
            "xmlSecMSCryptoX509StoreAdoptKeyStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( -1 ) ;
    }

    return( 0 ) ;
}

int
xmlSecMSCryptoAppliedKeysMngrAdoptTrustedStore (
    xmlSecKeysMngrPtr   mngr ,
    HCERTSTORE trustedStore
) {
    xmlSecKeyDataStorePtr x509Store ;

    xmlSecAssert2( mngr != NULL, -1 ) ;
    xmlSecAssert2( trustedStore != NULL, -1 ) ;

    x509Store = xmlSecKeysMngrGetDataStore( mngr, xmlSecMSCryptoX509StoreId ) ;
    if( x509Store == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeysMngrGetDataStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( -1 ) ;
    }

    if( xmlSecMSCryptoX509StoreAdoptTrustedStore( x509Store, trustedStore ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecKeyDataStoreGetName( x509Store ) ) ,
            "xmlSecMSCryptoX509StoreAdoptKeyStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( -1 ) ;
    }

    return( 0 ) ;
}

int
xmlSecMSCryptoAppliedKeysMngrAdoptUntrustedStore (
    xmlSecKeysMngrPtr   mngr ,
    HCERTSTORE untrustedStore
) {
    xmlSecKeyDataStorePtr x509Store ;

    xmlSecAssert2( mngr != NULL, -1 ) ;
    xmlSecAssert2( untrustedStore != NULL, -1 ) ;

    x509Store = xmlSecKeysMngrGetDataStore( mngr, xmlSecMSCryptoX509StoreId ) ;
    if( x509Store == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeysMngrGetDataStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( -1 ) ;
    }

    if( xmlSecMSCryptoX509StoreAdoptUntrustedStore( x509Store, untrustedStore ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecKeyDataStoreGetName( x509Store ) ) ,
            "xmlSecMSCryptoX509StoreAdoptKeyStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( -1 ) ;
    }

    return( 0 ) ;
}


