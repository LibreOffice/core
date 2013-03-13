/**
 * XMLSec library
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 *
 * Copyright..................................
 *
 * Contributor(s): _____________________________
 *
 */

/**
 * In order to ensure that particular crypto operation is performed on
 * particular crypto device, a subclass of xmlSecList is used to store slot and
 * mechanism information.
 *
 * In the list, a slot is bound with a mechanism. If the mechanism is available,
 * this mechanism only can perform on the slot; otherwise, it can perform on
 * every eligibl slot in the list.
 *
 * When try to find a slot for a particular mechanism, the slot bound with
 * available mechanism will be looked up firstly.
 */
#include "globals.h"
#include <string.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/errors.h>
#include <xmlsec/list.h>

#include <xmlsec/nss/tokens.h>

int
xmlSecNssKeySlotSetMechList(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE_PTR mechanismList
) {
    int counter ;

    xmlSecAssert2( keySlot != NULL , -1 ) ;

    if( keySlot->mechanismList != CK_NULL_PTR ) {
        xmlFree( keySlot->mechanismList ) ;

        for( counter = 0 ; *( mechanismList + counter ) != CKM_INVALID_MECHANISM ; counter ++ ) ;
        keySlot->mechanismList = ( CK_MECHANISM_TYPE_PTR )xmlMalloc( ( counter + 1 ) * sizeof( CK_MECHANISM_TYPE ) ) ;
        if( keySlot->mechanismList == NULL ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                NULL ,
                NULL ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;
            return( -1 );
        }
        for( ; counter >= 0 ; counter -- )
            *( keySlot->mechanismList + counter ) = *(  mechanismList + counter ) ;
    }

    return( 0 );
}

int
xmlSecNssKeySlotEnableMech(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE mechanism
) {
    int counter ;
    CK_MECHANISM_TYPE_PTR newList ;

    xmlSecAssert2( keySlot != NULL , -1 ) ;

    if( mechanism != CKM_INVALID_MECHANISM ) {
        for( counter = 0 ; *( keySlot->mechanismList + counter ) != CKM_INVALID_MECHANISM ; counter ++ ) ;
        newList = ( CK_MECHANISM_TYPE_PTR )xmlMalloc( ( counter + 1 + 1 ) * sizeof( CK_MECHANISM_TYPE ) ) ;
        if( newList == NULL ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                NULL ,
                NULL ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;
            return( -1 );
        }
        *( newList + counter + 1 ) = CKM_INVALID_MECHANISM ;
        *( newList + counter ) = mechanism ;
        for( counter -= 1 ; counter >= 0 ; counter -- )
            *( newList + counter ) = *(  keySlot->mechanismList + counter ) ;

        xmlFree( keySlot->mechanismList ) ;
        keySlot->mechanismList = newList ;
    }

    return(0);
}

int
xmlSecNssKeySlotDisableMech(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE mechanism
) {
    int counter ;

    xmlSecAssert2( keySlot != NULL , -1 ) ;

    for( counter = 0 ; *( keySlot->mechanismList + counter ) != CKM_INVALID_MECHANISM ; counter ++ ) {
        if( *( keySlot->mechanismList + counter ) == mechanism ) {
            for( ; *( keySlot->mechanismList + counter ) != CKM_INVALID_MECHANISM ; counter ++ ) {
                *( keySlot->mechanismList + counter ) = *( keySlot->mechanismList + counter + 1 ) ;
            }

            break ;
        }
    }

    return(0);
}

CK_MECHANISM_TYPE_PTR
xmlSecNssKeySlotGetMechList(
    xmlSecNssKeySlotPtr keySlot
) {
    if( keySlot != NULL )
        return keySlot->mechanismList ;
    else
        return NULL ;
}

int
xmlSecNssKeySlotSetSlot(
    xmlSecNssKeySlotPtr keySlot ,
    PK11SlotInfo* slot
) {
    xmlSecAssert2( keySlot != NULL , -1 ) ;

    if( slot != NULL && keySlot->slot != slot ) {
        if( keySlot->slot != NULL )
            PK11_FreeSlot( keySlot->slot ) ;

        if( keySlot->mechanismList != NULL ) {
            xmlFree( keySlot->mechanismList ) ;
            keySlot->mechanismList = NULL ;
        }

        keySlot->slot = PK11_ReferenceSlot( slot ) ;
    }

    return(0);
}

int
xmlSecNssKeySlotInitialize(
    xmlSecNssKeySlotPtr keySlot ,
    PK11SlotInfo* slot
) {
    xmlSecAssert2( keySlot != NULL , -1 ) ;
    xmlSecAssert2( keySlot->slot == NULL , -1 ) ;
    xmlSecAssert2( keySlot->mechanismList == NULL , -1 ) ;

    if( slot != NULL ) {
        keySlot->slot = PK11_ReferenceSlot( slot ) ;
    }

    return(0);
}

void
xmlSecNssKeySlotFinalize(
    xmlSecNssKeySlotPtr keySlot
) {
    xmlSecAssert( keySlot != NULL ) ;

    if( keySlot->mechanismList != NULL ) {
        xmlFree( keySlot->mechanismList ) ;
        keySlot->mechanismList = NULL ;
    }

    if( keySlot->slot != NULL ) {
        PK11_FreeSlot( keySlot->slot ) ;
        keySlot->slot = NULL ;
    }

}

PK11SlotInfo*
xmlSecNssKeySlotGetSlot(
    xmlSecNssKeySlotPtr keySlot
) {
    if( keySlot != NULL )
        return keySlot->slot ;
    else
        return NULL ;
}

xmlSecNssKeySlotPtr
xmlSecNssKeySlotCreate() {
    xmlSecNssKeySlotPtr keySlot ;

    /* Allocates a new xmlSecNssKeySlot and fill the fields */
    keySlot = ( xmlSecNssKeySlotPtr )xmlMalloc( sizeof( xmlSecNssKeySlot ) ) ;
    if( keySlot == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            NULL ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( NULL );
    }
    memset( keySlot, 0, sizeof( xmlSecNssKeySlot ) ) ;

    return( keySlot ) ;
}

int
xmlSecNssKeySlotCopy(
    xmlSecNssKeySlotPtr newKeySlot ,
    xmlSecNssKeySlotPtr keySlot
) {
    CK_MECHANISM_TYPE_PTR mech ;
    int counter ;

    xmlSecAssert2( newKeySlot != NULL , -1 ) ;
    xmlSecAssert2( keySlot != NULL , -1 ) ;

    if( keySlot->slot != NULL && newKeySlot->slot != keySlot->slot ) {
        if( newKeySlot->slot != NULL )
            PK11_FreeSlot( newKeySlot->slot ) ;

        newKeySlot->slot = PK11_ReferenceSlot( keySlot->slot ) ;
    }

    if( keySlot->mechanismList != CK_NULL_PTR ) {
        xmlFree( newKeySlot->mechanismList ) ;

        for( counter = 0 ; *( keySlot->mechanismList + counter ) != CKM_INVALID_MECHANISM ; counter ++ ) ;
        newKeySlot->mechanismList = ( CK_MECHANISM_TYPE_PTR )xmlMalloc( ( counter + 1 ) * sizeof( CK_MECHANISM_TYPE ) ) ;
        if( newKeySlot->mechanismList == NULL ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                NULL ,
                NULL ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;
            return( -1 );
        }
        for( ; counter >= 0 ; counter -- )
            *( newKeySlot->mechanismList + counter ) = *(  keySlot->mechanismList + counter ) ;
    }

    return( 0 );
}

xmlSecNssKeySlotPtr
xmlSecNssKeySlotDuplicate(
    xmlSecNssKeySlotPtr keySlot
) {
    xmlSecNssKeySlotPtr newKeySlot ;
    int ret ;

    xmlSecAssert2( keySlot != NULL , NULL ) ;

    newKeySlot = xmlSecNssKeySlotCreate() ;
    if( newKeySlot == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            NULL ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( NULL );
    }

    if( xmlSecNssKeySlotCopy( newKeySlot, keySlot ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            NULL ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( NULL );
    }

    return( newKeySlot );
}

void
xmlSecNssKeySlotDestroy(
        xmlSecNssKeySlotPtr keySlot
) {
    xmlSecAssert( keySlot != NULL ) ;

    if( keySlot->mechanismList != NULL )
        xmlFree( keySlot->mechanismList ) ;

    if( keySlot->slot != NULL )
        PK11_FreeSlot( keySlot->slot ) ;

    xmlFree( keySlot ) ;
}

int
xmlSecNssKeySlotBindMech(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE type
) {
    int counter ;

    xmlSecAssert2( keySlot != NULL , 0 ) ;
    xmlSecAssert2( keySlot->slot != NULL , 0 ) ;
    xmlSecAssert2( type != CKM_INVALID_MECHANISM , 0 ) ;

    for( counter = 0 ; *( keySlot->mechanismList + counter ) != CKM_INVALID_MECHANISM ; counter ++ ) {
        if( *( keySlot->mechanismList + counter ) == type )
            return(1) ;
    }

    return( 0 ) ;
}

int
xmlSecNssKeySlotSupportMech(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE type
) {
    xmlSecAssert2( keySlot != NULL , 0 ) ;
    xmlSecAssert2( keySlot->slot != NULL , 0 ) ;
    xmlSecAssert2( type != CKM_INVALID_MECHANISM , 0 ) ;

    if( PK11_DoesMechanism( keySlot->slot , type ) == PR_TRUE ) {
        return(1);
    } else
        return(0);
}

void
xmlSecNssKeySlotDebugDump(
    xmlSecNssKeySlotPtr keySlot ,
    FILE* output
) {
    xmlSecAssert( keySlot != NULL ) ;
    xmlSecAssert( output != NULL ) ;

    fprintf( output, "== KEY SLOT\n" );
}

void
xmlSecNssKeySlotDebugXmlDump(
    xmlSecNssKeySlotPtr keySlot ,
    FILE* output
) {
}

/**
 * Key Slot List
 */
#ifdef __MINGW32__ // for runtime-pseudo-reloc
static struct _xmlSecPtrListKlass xmlSecNssKeySlotPtrListKlass = {
#else
static xmlSecPtrListKlass xmlSecNssKeySlotPtrListKlass = {
#endif
    BAD_CAST "mechanism-list",
    (xmlSecPtrDuplicateItemMethod)xmlSecNssKeySlotDuplicate,
    (xmlSecPtrDestroyItemMethod)xmlSecNssKeySlotDestroy,
    (xmlSecPtrDebugDumpItemMethod)xmlSecNssKeySlotDebugDump,
    (xmlSecPtrDebugDumpItemMethod)xmlSecNssKeySlotDebugXmlDump,
};

xmlSecPtrListId
xmlSecNssKeySlotListGetKlass(void) {
    return(&xmlSecNssKeySlotPtrListKlass);
}


/*-
 * Global PKCS#11 crypto token repository -- Key slot list
 */
static xmlSecPtrListPtr _xmlSecNssKeySlotList = NULL ;

PK11SlotInfo*
xmlSecNssSlotGet(
    CK_MECHANISM_TYPE type
) {
    PK11SlotInfo*           slot = NULL ;
    xmlSecNssKeySlotPtr     keySlot ;
    xmlSecSize              ksSize ;
    xmlSecSize              ksPos ;
    char                    flag ;

    if( _xmlSecNssKeySlotList == NULL ) {
        slot = PK11_GetBestSlot( type , NULL ) ;
    } else {
        ksSize = xmlSecPtrListGetSize( _xmlSecNssKeySlotList ) ;

        /*-
         * Firstly, checking whether the mechanism is bound with a special slot.
         * If no bound slot, we try to find the first eligible slot in the list.
         */
        for( flag = 0, ksPos = 0 ; ksPos < ksSize ; ksPos ++ ) {
            keySlot = ( xmlSecNssKeySlotPtr )xmlSecPtrListGetItem( _xmlSecNssKeySlotList, ksPos ) ;
            if( keySlot != NULL && xmlSecNssKeySlotBindMech( keySlot, type ) ) {
                slot = xmlSecNssKeySlotGetSlot( keySlot ) ;
                flag = 2 ;
            } else if( flag == 0 && xmlSecNssKeySlotSupportMech( keySlot, type ) ) {
                slot = xmlSecNssKeySlotGetSlot( keySlot ) ;
                flag = 1 ;
            }

            if( flag == 2 )
                break ;
        }
        if( slot != NULL )
            slot = PK11_ReferenceSlot( slot ) ;
    }

    if( slot != NULL && PK11_NeedLogin( slot ) ) {
        if( PK11_Authenticate( slot , PR_TRUE , NULL ) != SECSuccess ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                NULL ,
                NULL ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;
            PK11_FreeSlot( slot ) ;
            return( NULL );
        }
    }

    return slot ;
}

int
xmlSecNssSlotInitialize(
    void
) {
    if( _xmlSecNssKeySlotList != NULL ) {
        xmlSecPtrListDestroy( _xmlSecNssKeySlotList ) ;
        _xmlSecNssKeySlotList = NULL ;
    }

    _xmlSecNssKeySlotList = xmlSecPtrListCreate( xmlSecNssKeySlotListId ) ;
    if( _xmlSecNssKeySlotList == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            NULL ,
            XMLSEC_ERRORS_R_XMLSEC_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return( -1 );
    }

    return(0);
}

void
xmlSecNssSlotShutdown(
    void
) {
    if( _xmlSecNssKeySlotList != NULL ) {
        xmlSecPtrListDestroy( _xmlSecNssKeySlotList ) ;
        _xmlSecNssKeySlotList = NULL ;
    }
}

int
xmlSecNssSlotAdopt(
    PK11SlotInfo* slot,
    CK_MECHANISM_TYPE type
) {
    xmlSecNssKeySlotPtr     keySlot ;
    xmlSecSize              ksSize ;
    xmlSecSize              ksPos ;
    char                    flag ;

    xmlSecAssert2( _xmlSecNssKeySlotList != NULL, -1 ) ;
    xmlSecAssert2( slot != NULL, -1 ) ;

    ksSize = xmlSecPtrListGetSize( _xmlSecNssKeySlotList ) ;

    /*-
     * Firstly, checking whether the slot is in the repository already.
     */
    flag = 0 ;
    for( ksPos = 0 ; ksPos < ksSize ; ksPos ++ ) {
        keySlot = ( xmlSecNssKeySlotPtr )xmlSecPtrListGetItem( _xmlSecNssKeySlotList, ksPos ) ;
        /* If find the slot in the list */
        if( keySlot != NULL && xmlSecNssKeySlotGetSlot( keySlot ) == slot ) {
            /* If mechnism type is valid, bind the slot with the mechanism */
            if( type != CKM_INVALID_MECHANISM ) {
                if( xmlSecNssKeySlotEnableMech( keySlot, type ) < 0 ) {
                    xmlSecError( XMLSEC_ERRORS_HERE ,
                        NULL ,
                        NULL ,
                        XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                        XMLSEC_ERRORS_NO_MESSAGE ) ;
                    return(-1);
                }
            }

            flag = 1 ;
        }
    }

    /* If the slot do not in the list, add a new item to the list */
    if( flag == 0 ) {
        /* Create a new KeySlot */
        keySlot = xmlSecNssKeySlotCreate() ;
        if( keySlot == NULL ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                NULL ,
                NULL ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;
            return(-1);
        }

        /* Initialize the keySlot with a slot */
        if( xmlSecNssKeySlotInitialize( keySlot, slot ) < 0 ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                NULL ,
                NULL ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;
            xmlSecNssKeySlotDestroy( keySlot ) ;
            return(-1);
        }

        /* If mechnism type is valid, bind the slot with the mechanism */
        if( type != CKM_INVALID_MECHANISM ) {
            if( xmlSecNssKeySlotEnableMech( keySlot, type ) < 0 ) {
                xmlSecError( XMLSEC_ERRORS_HERE ,
                    NULL ,
                    NULL ,
                    XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                    XMLSEC_ERRORS_NO_MESSAGE ) ;
                xmlSecNssKeySlotDestroy( keySlot ) ;
                return(-1);
            }
        }

        /* Add keySlot into the list */
        if( xmlSecPtrListAdd( _xmlSecNssKeySlotList, keySlot ) < 0 ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                NULL ,
                NULL ,
                XMLSEC_ERRORS_R_XMLSEC_FAILED ,
                XMLSEC_ERRORS_NO_MESSAGE ) ;
            xmlSecNssKeySlotDestroy( keySlot ) ;
            return(-1);
        }
    }

    return(0);
}

