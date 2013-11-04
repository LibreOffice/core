/**
 * XMLSec library
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 *
 * Copyright.........................
 */
#include "globals.h"

#include <nspr.h>
#include <nss.h>
#include <pk11func.h>
#include <cert.h>
#include <keyhi.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/keys.h>
#include <xmlsec/transforms.h>
#include <xmlsec/errors.h>

#include <xmlsec/nss/crypto.h>
#include <xmlsec/nss/tokens.h>
#include <xmlsec/nss/akmngr.h>
#include <xmlsec/nss/pkikeys.h>
#include <xmlsec/nss/ciphers.h>
#include <xmlsec/nss/keysstore.h>

/**
 * xmlSecNssAppliedKeysMngrCreate:
 * @slot:           array of pointers to NSS PKCS#11 slot information.
 * @cSlots:         number of slots in the array
 * @handler:        the pointer to NSS certificate database.
 *
 * Create and load NSS crypto slot and certificate database into keys manager
 *
 * Returns keys manager pointer on success or NULL otherwise.
 */
xmlSecKeysMngrPtr
xmlSecNssAppliedKeysMngrCreate(
    PK11SlotInfo** slots,
    int cSlots,
    CERTCertDBHandle* handler
) {
    xmlSecKeyDataStorePtr   certStore = NULL ;
    xmlSecKeysMngrPtr       keyMngr = NULL ;
    xmlSecKeyStorePtr       keyStore = NULL ;
    int islot = 0;
    keyStore = xmlSecKeyStoreCreate( xmlSecNssKeysStoreId ) ;
    if( keyStore == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeyStoreCreate" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return NULL ;
    }

    for (islot = 0; islot < cSlots; islot++)
    {
        xmlSecNssKeySlotPtr     keySlot ;

        /* Create a key slot */
        keySlot = xmlSecNssKeySlotCreate() ;
        if( keySlot == NULL ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                xmlSecErrorsSafeString( xmlSecKeyStoreGetName( keyStore ) ) ,
                "xmlSecNssKeySlotCreate" ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;

            xmlSecKeyStoreDestroy( keyStore ) ;
            return NULL ;
        }

        /* Set slot */
        if( xmlSecNssKeySlotSetSlot( keySlot , slots[islot] ) < 0 ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                xmlSecErrorsSafeString( xmlSecKeyStoreGetName( keyStore ) ) ,
                "xmlSecNssKeySlotSetSlot" ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;

            xmlSecKeyStoreDestroy( keyStore ) ;
            xmlSecNssKeySlotDestroy( keySlot ) ;
            return NULL ;
        }

        /* Adopt keySlot */
        if( xmlSecNssKeysStoreAdoptKeySlot( keyStore , keySlot ) < 0 ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                xmlSecErrorsSafeString( xmlSecKeyStoreGetName( keyStore ) ) ,
                "xmlSecNssKeysStoreAdoptKeySlot" ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;

            xmlSecKeyStoreDestroy( keyStore ) ;
            xmlSecNssKeySlotDestroy( keySlot ) ;
            return NULL ;
        }
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
    if( xmlSecNssKeysMngrInit( keyMngr ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeysMngrCreate" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;

        xmlSecKeysMngrDestroy( keyMngr ) ;
        return NULL ;
    }

    /*-
     * Set certificate databse to X509 key data store
     */
    /**
     * Because Tej's implementation of certDB use the default DB, so I ignore
     * the certDB handler at present. I'll modify the cert store sources to
     * accept particular certDB instead of default ones.
    certStore = xmlSecKeysMngrGetDataStore( keyMngr , xmlSecNssKeyDataStoreX509Id ) ;
    if( certStore == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecKeyStoreGetName( keyStore ) ) ,
            "xmlSecKeysMngrGetDataStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;

        xmlSecKeysMngrDestroy( keyMngr ) ;
        return NULL ;
    }

    if( xmlSecNssKeyDataStoreX509SetCertDb( certStore , handler ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecKeyStoreGetName( keyStore ) ) ,
            "xmlSecNssKeyDataStoreX509SetCertDb" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;

        xmlSecKeysMngrDestroy( keyMngr ) ;
        return NULL ;
    }
    */

    /*-
     * Set the getKey callback
     */
    keyMngr->getKey = xmlSecKeysMngrGetKey ;

    return keyMngr ;
}

int
xmlSecNssAppliedKeysMngrSymKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    PK11SymKey*         symKey
) {
    xmlSecKeyPtr        key ;
    xmlSecKeyDataPtr    data ;
    xmlSecKeyStorePtr   keyStore ;

    xmlSecAssert2( mngr != NULL , -1 ) ;
    xmlSecAssert2( symKey != NULL , -1 ) ;

    keyStore = xmlSecKeysMngrGetKeysStore( mngr ) ;
    if( keyStore == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeysMngrGetKeysStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1) ;
    }
    xmlSecAssert2( xmlSecKeyStoreCheckId( keyStore , xmlSecNssKeysStoreId ) , -1 ) ;

    data = xmlSecNssSymKeyDataKeyAdopt( symKey ) ;
    if( data == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1) ;
    }

    key = xmlSecKeyCreate() ;
    if( key == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDataDestroy( data ) ;
        return(-1) ;
    }

    if( xmlSecKeySetValue( key , data ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDataDestroy( data ) ;
        return(-1) ;
    }

    if( xmlSecNssKeysStoreAdoptKey( keyStore, key ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDestroy( key ) ;
        return(-1) ;
    }

    return(0) ;
}

int
xmlSecNssAppliedKeysMngrPubKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    SECKEYPublicKey*    pubKey
) {
    xmlSecKeyPtr        key ;
    xmlSecKeyDataPtr    data ;
    xmlSecKeyStorePtr   keyStore ;

    xmlSecAssert2( mngr != NULL , -1 ) ;
    xmlSecAssert2( pubKey != NULL , -1 ) ;

    keyStore = xmlSecKeysMngrGetKeysStore( mngr ) ;
    if( keyStore == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeysMngrGetKeysStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1) ;
    }
    xmlSecAssert2( xmlSecKeyStoreCheckId( keyStore , xmlSecNssKeysStoreId ) , -1 ) ;

    data = xmlSecNssPKIAdoptKey( NULL, pubKey ) ;
    if( data == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssPKIAdoptKey" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1) ;
    }

    key = xmlSecKeyCreate() ;
    if( key == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDataDestroy( data ) ;
        return(-1) ;
    }

    if( xmlSecKeySetValue( key , data ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDataDestroy( data ) ;
        return(-1) ;
    }

    if( xmlSecNssKeysStoreAdoptKey( keyStore, key ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDestroy( key ) ;
        return(-1) ;
    }

    return(0) ;
}

int
xmlSecNssAppliedKeysMngrPriKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    SECKEYPrivateKey*   priKey
) {
    xmlSecKeyPtr        key ;
    xmlSecKeyDataPtr    data ;
    xmlSecKeyStorePtr   keyStore ;

    xmlSecAssert2( mngr != NULL , -1 ) ;
    xmlSecAssert2( priKey != NULL , -1 ) ;

    keyStore = xmlSecKeysMngrGetKeysStore( mngr ) ;
    if( keyStore == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecKeysMngrGetKeysStore" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1) ;
    }
    xmlSecAssert2( xmlSecKeyStoreCheckId( keyStore , xmlSecNssKeysStoreId ) , -1 ) ;

    data = xmlSecNssPKIAdoptKey( priKey, NULL ) ;
    if( data == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssPKIAdoptKey" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1) ;
    }

    key = xmlSecKeyCreate() ;
    if( key == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDataDestroy( data ) ;
        return(-1) ;
    }

    if( xmlSecKeySetValue( key , data ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDataDestroy( data ) ;
        return(-1) ;
    }

    if( xmlSecNssKeysStoreAdoptKey( keyStore, key ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecNssSymKeyDataKeyAdopt" ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecKeyDestroy( key ) ;
        return(-1) ;
    }

    return(0) ;
}

