/**
 * XMLSec library
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 *
 * Copyright ..........................
 */
#ifndef __XMLSEC_MSCRYPTO_AKMNGR_H__
#define __XMLSEC_MSCRYPTO_AKMNGR_H__

#include <windows.h>
#include <wincrypt.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/keys.h>
#include <xmlsec/transforms.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

XMLSEC_CRYPTO_EXPORT xmlSecKeysMngrPtr
xmlSecMSCryptoAppliedKeysMngrCreate(
    HCERTSTORE keyStore ,
    HCERTSTORE certStore
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecMSCryptoAppliedKeysMngrSymKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    HCRYPTKEY   symKey
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecMSCryptoAppliedKeysMngrPubKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    HCRYPTKEY   pubKey
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecMSCryptoAppliedKeysMngrPriKeyLoad(
    xmlSecKeysMngrPtr   mngr ,
    HCRYPTKEY   priKey
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecMSCryptoAppliedKeysMngrAdoptKeyStore (
    xmlSecKeysMngrPtr   mngr ,
    HCERTSTORE keyStore
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecMSCryptoAppliedKeysMngrAdoptTrustedStore (
    xmlSecKeysMngrPtr   mngr ,
    HCERTSTORE trustedStore
) ;

XMLSEC_CRYPTO_EXPORT int
xmlSecMSCryptoAppliedKeysMngrAdoptUntrustedStore (
    xmlSecKeysMngrPtr   mngr ,
    HCERTSTORE untrustedStore
) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __XMLSEC_MSCRYPTO_AKMNGR_H__ */



