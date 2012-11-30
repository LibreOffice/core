/**
 * XMLSec library
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 *
 * Copyright ..........................
 */
#ifndef __XMLSEC_NSS_CIPHERS_H__
#define __XMLSEC_NSS_CIPHERS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <xmlsec/xmlsec.h>
#include <xmlsec/keys.h>
#include <xmlsec/transforms.h>


XMLSEC_CRYPTO_EXPORT int xmlSecNssSymKeyDataAdoptKey( xmlSecKeyDataPtr data,
                                    PK11SymKey* symkey ) ;

XMLSEC_CRYPTO_EXPORT xmlSecKeyDataPtr xmlSecNssSymKeyDataKeyAdopt( PK11SymKey* symKey ) ;

XMLSEC_CRYPTO_EXPORT PK11SymKey*   xmlSecNssSymKeyDataGetKey(xmlSecKeyDataPtr data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __XMLSEC_NSS_CIPHERS_H__ */



