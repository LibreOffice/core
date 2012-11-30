/**
 * XMLSec library
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 *
 * Copyright (c) 2003 Sun Microsystems, Inc.  All rights reserved.
 *
 * Contributor(s): _____________________________
 *
 */
#ifndef __XMLSEC_NSS_TOKENS_H__
#define __XMLSEC_NSS_TOKENS_H__

#include <string.h>

#include <nss.h>
#include <pk11func.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * xmlSecNssKeySlotListId
 *
 * The crypto mechanism list klass
 */
#define xmlSecNssKeySlotListId xmlSecNssKeySlotListGetKlass()
XMLSEC_CRYPTO_EXPORT xmlSecPtrListId xmlSecNssKeySlotListGetKlass( void ) ;

/*******************************************
 * KeySlot interfaces
 *******************************************/
/**
 * Internal NSS key slot data
 * @mechanismList:      the mechanisms that the slot bound with.
 * @slot:               the pkcs slot
 *
 * This context is located after xmlSecPtrList
 */
typedef struct _xmlSecNssKeySlot    xmlSecNssKeySlot ;
typedef struct _xmlSecNssKeySlot*   xmlSecNssKeySlotPtr ;

struct _xmlSecNssKeySlot {
    CK_MECHANISM_TYPE_PTR   mechanismList ; /* mech. array, NULL ternimated */
    PK11SlotInfo*           slot ;
} ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssKeySlotSetMechList(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE_PTR mechanismList
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssKeySlotEnableMech(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE mechanism
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssKeySlotDisableMech(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE mechanism
) ;

XMLSEC_CRYPTO_EXPORT CK_MECHANISM_TYPE_PTR
xmlSecNssKeySlotGetMechList(
    xmlSecNssKeySlotPtr keySlot
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssKeySlotSetSlot(
    xmlSecNssKeySlotPtr keySlot ,
    PK11SlotInfo* slot
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssKeySlotInitialize(
    xmlSecNssKeySlotPtr keySlot ,
    PK11SlotInfo* slot
) ;

XMLSEC_CRYPTO_EXPORT void
xmlSecNssKeySlotFinalize(
    xmlSecNssKeySlotPtr keySlot
) ;

XMLSEC_CRYPTO_EXPORT PK11SlotInfo*
xmlSecNssKeySlotGetSlot(
    xmlSecNssKeySlotPtr keySlot
) ;

XMLSEC_CRYPTO_EXPORT xmlSecNssKeySlotPtr
xmlSecNssKeySlotCreate() ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssKeySlotCopy(
    xmlSecNssKeySlotPtr newKeySlot ,
    xmlSecNssKeySlotPtr keySlot
) ;

XMLSEC_CRYPTO_EXPORT xmlSecNssKeySlotPtr
xmlSecNssKeySlotDuplicate(
    xmlSecNssKeySlotPtr keySlot
) ;

XMLSEC_CRYPTO_EXPORT void
xmlSecNssKeySlotDestroy(
        xmlSecNssKeySlotPtr keySlot
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssKeySlotBindMech(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE type
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssKeySlotSupportMech(
    xmlSecNssKeySlotPtr keySlot ,
    CK_MECHANISM_TYPE type
) ;


/************************************************************************
 * PKCS#11 crypto token interfaces
 *
 * A PKCS#11 slot repository will be defined internally. From the
 * repository, a user can specify a particular slot for a certain crypto
 * mechanism.
 *
 * In some situation, some cryptographic operation should act in a user
 * designated devices. The interfaces defined here provide the way. If
 * the user do not initialize the repository distinctly, the interfaces
 * use the default functions provided by NSS itself.
 *
 ************************************************************************/
/**
 * Initialize NSS pkcs#11 slot repository
 *
 * Returns 0 if success or -1 if an error occurs.
 */
XMLSEC_CRYPTO_EXPORT int xmlSecNssSlotInitialize( void ) ;

/**
 * Shutdown and destroy NSS pkcs#11 slot repository
 */
XMLSEC_CRYPTO_EXPORT void xmlSecNssSlotShutdown() ;

/**
 * Get PKCS#11 slot handler
 * @type    the mechanism that the slot must support.
 *
 * Returns a pointer to PKCS#11 slot or NULL if an error occurs.
 *
 * Notes: The returned handler must be destroied distinctly.
 */
XMLSEC_CRYPTO_EXPORT PK11SlotInfo* xmlSecNssSlotGet( CK_MECHANISM_TYPE type ) ;

/**
 * Adopt a pkcs#11 slot with a mechanism into the repository
 * @slot:   the pkcs#11 slot.
 * @mech:   the mechanism.
 *
 * If @mech is available( @mech != CKM_INVALID_MECHANISM ), every operation with
 * this mechanism only can perform on the @slot.
 *
 * Returns 0 if success or -1 if an error occurs.
 */
XMLSEC_CRYPTO_EXPORT int xmlSecNssSlotAdopt( PK11SlotInfo* slot, CK_MECHANISM_TYPE mech ) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __XMLSEC_NSS_TOKENS_H__ */


