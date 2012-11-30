/**
 * XMLSec library
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 *
 * Copyright ..........................
 */
#ifndef __XMLSEC_NSS_AKMNGR_H__
#define __XMLSEC_NSS_AKMNGR_H__

#include <nss.h>
#include <nspr.h>
#include <pk11func.h>
#include <cert.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/keys.h>
#include <xmlsec/transforms.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

XMLSEC_CRYPTO_EXPORT xmlSecKeysMngrPtr
xmlSecNssAppliedKeysMngrCreate(
    PK11SlotInfo** slots,
    int cSlots,
    CERTCertDBHandle* handler
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssAppliedKeysMngrSymKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    PK11SymKey*         symKey
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssAppliedKeysMngrPubKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    SECKEYPublicKey*    pubKey
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecNssAppliedKeysMngrPriKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    SECKEYPrivateKey*   priKey
) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __XMLSEC_NSS_AKMNGR_H__ */



