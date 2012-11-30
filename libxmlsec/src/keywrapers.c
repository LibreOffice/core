/**
 *
 * XMLSec library
 *
 * AES Algorithm support
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 *
 * Copyright .................................
 */
#include "globals.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <nss.h>
#include <pk11func.h>
#include <hasht.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>
#include <xmlsec/keys.h>
#include <xmlsec/transforms.h>
#include <xmlsec/errors.h>

#include <xmlsec/nss/crypto.h>
#include <xmlsec/nss/ciphers.h>

#define XMLSEC_NSS_AES128_KEY_SIZE      16
#define XMLSEC_NSS_AES192_KEY_SIZE      24
#define XMLSEC_NSS_AES256_KEY_SIZE      32
#define XMLSEC_NSS_DES3_KEY_SIZE        24
#define XMLSEC_NSS_DES3_KEY_LENGTH      24
#define XMLSEC_NSS_DES3_IV_LENGTH       8
#define XMLSEC_NSS_DES3_BLOCK_LENGTH    8

static xmlSecByte xmlSecNssKWDes3Iv[XMLSEC_NSS_DES3_IV_LENGTH] = {
    0x4a, 0xdd, 0xa2, 0x2c, 0x79, 0xe8, 0x21, 0x05
};

/*********************************************************************
 *
 * key wrap transforms
 *
 ********************************************************************/
typedef struct _xmlSecNssKeyWrapCtx         xmlSecNssKeyWrapCtx ;
typedef struct _xmlSecNssKeyWrapCtx*        xmlSecNssKeyWrapCtxPtr ;

#define xmlSecNssKeyWrapSize    \
    ( sizeof( xmlSecTransform ) + sizeof( xmlSecNssKeyWrapCtx ) )

#define xmlSecNssKeyWrapGetCtx( transform ) \
    ( ( xmlSecNssKeyWrapCtxPtr )( ( ( xmlSecByte* )( transform ) ) + sizeof( xmlSecTransform ) ) )

struct _xmlSecNssKeyWrapCtx {
    CK_MECHANISM_TYPE       cipher ;
    PK11SymKey*             symkey ;
    xmlSecKeyDataId         keyId ;
    xmlSecBufferPtr         material ; /* to be encrypted/decrypted key material */
} ;

static int      xmlSecNssKeyWrapInitialize(xmlSecTransformPtr transform);
static void     xmlSecNssKeyWrapFinalize(xmlSecTransformPtr transform);
static int      xmlSecNssKeyWrapSetKeyReq(xmlSecTransformPtr transform,
                             xmlSecKeyReqPtr keyReq);
static int      xmlSecNssKeyWrapSetKey(xmlSecTransformPtr transform,
                             xmlSecKeyPtr key);
static int      xmlSecNssKeyWrapExecute(xmlSecTransformPtr transform,
                             int last,
                             xmlSecTransformCtxPtr transformCtx);
static xmlSecSize   xmlSecNssKeyWrapGetKeySize(xmlSecTransformPtr transform);

static int
xmlSecNssKeyWrapCheckId(
    xmlSecTransformPtr transform
) {
    #ifndef XMLSEC_NO_DES
    if( xmlSecTransformCheckId( transform, xmlSecNssTransformKWDes3Id ) ) {
        return(1);
    }
    #endif /* XMLSEC_NO_DES */

    #ifndef XMLSEC_NO_AES
    if( xmlSecTransformCheckId( transform, xmlSecNssTransformKWAes128Id ) ||
        xmlSecTransformCheckId( transform, xmlSecNssTransformKWAes192Id ) ||
        xmlSecTransformCheckId( transform, xmlSecNssTransformKWAes256Id ) ) {

        return(1);
    }
    #endif /* XMLSEC_NO_AES */

    return(0);
}

static xmlSecSize
xmlSecNssKeyWrapGetKeySize(xmlSecTransformPtr transform) {
#ifndef XMLSEC_NO_DES
    if( xmlSecTransformCheckId( transform, xmlSecNssTransformKWDes3Id ) ) {
        return(XMLSEC_NSS_DES3_KEY_SIZE);
    } else
#endif /* XMLSEC_NO_DES */

#ifndef XMLSEC_NO_AES
    if(xmlSecTransformCheckId(transform, xmlSecNssTransformKWAes128Id)) {
        return(XMLSEC_NSS_AES128_KEY_SIZE);
    } else if(xmlSecTransformCheckId(transform, xmlSecNssTransformKWAes192Id)) {
        return(XMLSEC_NSS_AES192_KEY_SIZE);
    } else if(xmlSecTransformCheckId(transform, xmlSecNssTransformKWAes256Id)) {
        return(XMLSEC_NSS_AES256_KEY_SIZE);
    } else if(xmlSecTransformCheckId(transform, xmlSecNssTransformKWAes256Id)) {
        return(XMLSEC_NSS_AES256_KEY_SIZE);
    } else
#endif /* XMLSEC_NO_AES */

    if(1)
        return(0);
}


static int
xmlSecNssKeyWrapInitialize(xmlSecTransformPtr transform) {
    xmlSecNssKeyWrapCtxPtr context ;
    int ret;

    xmlSecAssert2(xmlSecNssKeyWrapCheckId(transform), -1);
    xmlSecAssert2(xmlSecTransformCheckSize(transform, xmlSecNssKeyWrapSize), -1);

    context = xmlSecNssKeyWrapGetCtx( transform ) ;
    xmlSecAssert2( context != NULL , -1 ) ;

    #ifndef XMLSEC_NO_DES
    if( transform->id == xmlSecNssTransformKWDes3Id ) {
        context->cipher = CKM_DES3_CBC ;
        context->keyId = xmlSecNssKeyDataDesId ;
    } else
    #endif      /* XMLSEC_NO_DES */

    #ifndef XMLSEC_NO_AES
    if( transform->id == xmlSecNssTransformKWAes128Id ) {
    /*  context->cipher = CKM_NETSCAPE_AES_KEY_WRAP ;*/
        context->cipher = CKM_AES_CBC ;
        context->keyId = xmlSecNssKeyDataAesId ;
    } else
    if( transform->id == xmlSecNssTransformKWAes192Id ) {
    /*  context->cipher = CKM_NETSCAPE_AES_KEY_WRAP ;*/
        context->cipher = CKM_AES_CBC ;
        context->keyId = xmlSecNssKeyDataAesId ;
    } else
    if( transform->id == xmlSecNssTransformKWAes256Id ) {
    /*  context->cipher = CKM_NETSCAPE_AES_KEY_WRAP ;*/
        context->cipher = CKM_AES_CBC ;
        context->keyId = xmlSecNssKeyDataAesId ;
    } else
    #endif      /* XMLSEC_NO_AES */


    if( 1 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
            NULL ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    context->symkey = NULL ;
    context->material = NULL ;

    return(0);
}

static void
xmlSecNssKeyWrapFinalize(xmlSecTransformPtr transform) {
    xmlSecNssKeyWrapCtxPtr context ;

    xmlSecAssert(xmlSecNssKeyWrapCheckId(transform));
    xmlSecAssert(xmlSecTransformCheckSize(transform, xmlSecNssKeyWrapSize));

    context = xmlSecNssKeyWrapGetCtx( transform ) ;
    xmlSecAssert( context != NULL ) ;

    if( context->symkey != NULL ) {
        PK11_FreeSymKey( context->symkey ) ;
        context->symkey = NULL ;
    }

    if( context->material != NULL ) {
        xmlSecBufferDestroy(context->material);
        context->material = NULL ;
    }
}

static int
xmlSecNssKeyWrapSetKeyReq(xmlSecTransformPtr transform,  xmlSecKeyReqPtr keyReq) {
    xmlSecNssKeyWrapCtxPtr context ;
    xmlSecSize cipherSize = 0 ;


    xmlSecAssert2(xmlSecNssKeyWrapCheckId(transform), -1);
    xmlSecAssert2(xmlSecTransformCheckSize(transform, xmlSecNssKeyWrapSize), -1);
    xmlSecAssert2((transform->operation == xmlSecTransformOperationEncrypt) || (transform->operation == xmlSecTransformOperationDecrypt), -1);
    xmlSecAssert2(keyReq != NULL, -1);

    context = xmlSecNssKeyWrapGetCtx( transform ) ;
    xmlSecAssert2( context != NULL , -1 ) ;

    keyReq->keyId    = context->keyId;
    keyReq->keyType  = xmlSecKeyDataTypeSymmetric;
    if(transform->operation == xmlSecTransformOperationEncrypt) {
        keyReq->keyUsage = xmlSecKeyUsageEncrypt;
    } else {
        keyReq->keyUsage = xmlSecKeyUsageDecrypt;
    }

    keyReq->keyBitsSize = xmlSecNssKeyWrapGetKeySize( transform ) ;

    return(0);
}

static int
xmlSecNssKeyWrapSetKey(xmlSecTransformPtr transform, xmlSecKeyPtr key) {
    xmlSecNssKeyWrapCtxPtr context = NULL ;
    xmlSecKeyDataPtr    keyData = NULL ;
    PK11SymKey*         symkey = NULL ;

    xmlSecAssert2(xmlSecNssKeyWrapCheckId(transform), -1);
    xmlSecAssert2(xmlSecTransformCheckSize(transform, xmlSecNssKeyWrapSize), -1);
    xmlSecAssert2((transform->operation == xmlSecTransformOperationEncrypt) || (transform->operation == xmlSecTransformOperationDecrypt), -1);
    xmlSecAssert2(key != NULL, -1);

    context = xmlSecNssKeyWrapGetCtx( transform ) ;
    if( context == NULL || context->keyId == NULL || context->symkey != NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecTransformGetName( transform ) ) ,
            "xmlSecNssKeyWrapGetCtx" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }
    xmlSecAssert2( xmlSecKeyCheckId( key, context->keyId ), -1 ) ;

    keyData = xmlSecKeyGetValue( key ) ;
    if( keyData == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecKeyGetName( key ) ) ,
            "xmlSecKeyGetValue" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    if( ( symkey = xmlSecNssSymKeyDataGetKey( keyData ) ) == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecKeyDataGetName( keyData ) ) ,
            "xmlSecNssSymKeyDataGetKey" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    context->symkey = symkey ;

    return(0) ;
}

/**
 * key wrap transform
 */
static int
xmlSecNssKeyWrapCtxInit(
    xmlSecNssKeyWrapCtxPtr      ctx ,
    xmlSecBufferPtr             in ,
    xmlSecBufferPtr             out ,
    int                         encrypt ,
    xmlSecTransformCtxPtr       transformCtx
) {
    xmlSecSize          blockSize ;

    xmlSecAssert2( ctx != NULL , -1 ) ;
    xmlSecAssert2( ctx->cipher != CKM_INVALID_MECHANISM , -1 ) ;
    xmlSecAssert2( ctx->symkey != NULL , -1 ) ;
    xmlSecAssert2( ctx->keyId != NULL , -1 ) ;
    xmlSecAssert2( in != NULL , -1 ) ;
    xmlSecAssert2( out != NULL , -1 ) ;
    xmlSecAssert2( transformCtx != NULL , -1 ) ;

    if( ctx->material != NULL ) {
        xmlSecBufferDestroy( ctx->material ) ;
        ctx->material = NULL ;
    }

    if( ( blockSize = PK11_GetBlockSize( ctx->cipher , NULL ) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "PK11_GetBlockSize" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    ctx->material = xmlSecBufferCreate( blockSize ) ;
    if( ctx->material == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferCreate" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    /* read raw key material into context */
    if( xmlSecBufferSetData( ctx->material, xmlSecBufferGetData(in), xmlSecBufferGetSize(in) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferSetData" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    if( xmlSecBufferRemoveHead( in , xmlSecBufferGetSize(in) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferRemoveHead" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    return(0);
}

/**
 * key wrap transform update
 */
static int
xmlSecNssKeyWrapCtxUpdate(
    xmlSecNssKeyWrapCtxPtr      ctx ,
    xmlSecBufferPtr             in ,
    xmlSecBufferPtr             out ,
    int                         encrypt ,
    xmlSecTransformCtxPtr       transformCtx
) {
    xmlSecAssert2( ctx != NULL , -1 ) ;
    xmlSecAssert2( ctx->cipher != CKM_INVALID_MECHANISM , -1 ) ;
    xmlSecAssert2( ctx->symkey != NULL , -1 ) ;
    xmlSecAssert2( ctx->keyId != NULL , -1 ) ;
    xmlSecAssert2( ctx->material != NULL , -1 ) ;
    xmlSecAssert2( in != NULL , -1 ) ;
    xmlSecAssert2( out != NULL , -1 ) ;
    xmlSecAssert2( transformCtx != NULL , -1 ) ;

    /* read raw key material and append into context */
    if( xmlSecBufferAppend( ctx->material, xmlSecBufferGetData(in), xmlSecBufferGetSize(in) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferAppend" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    if( xmlSecBufferRemoveHead( in , xmlSecBufferGetSize(in) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferRemoveHead" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    return(0);
}

static int
xmlSecNssKWDes3BufferReverse(xmlSecByte *buf, xmlSecSize size) {
    xmlSecSize s;
    xmlSecSize i;
    xmlSecByte c;

    xmlSecAssert2(buf != NULL, -1);

    s = size / 2;
    --size;
    for(i = 0; i < s; ++i) {
    c = buf[i];
    buf[i] = buf[size - i];
    buf[size - i] = c;
    }
    return(0);
}

static xmlSecByte *
xmlSecNssComputeSHA1(const xmlSecByte *in, xmlSecSize inSize,
             xmlSecByte *out, xmlSecSize outSize)
{
    PK11Context *context = NULL;
    SECStatus s;
    xmlSecByte *digest = NULL;
    unsigned int len;

    xmlSecAssert2(in != NULL, NULL);
    xmlSecAssert2(out != NULL, NULL);
    xmlSecAssert2(outSize >= SHA1_LENGTH, NULL);

    /* Create a context for hashing (digesting) */
    context = PK11_CreateDigestContext(SEC_OID_SHA1);
    if (context == NULL) {
    xmlSecError(XMLSEC_ERRORS_HERE,
            NULL,
            "PK11_CreateDigestContext",
            XMLSEC_ERRORS_R_CRYPTO_FAILED,
            "error code = %d", PORT_GetError());
    goto done;
    }

    s = PK11_DigestBegin(context);
    if (s != SECSuccess) {
    xmlSecError(XMLSEC_ERRORS_HERE,
            NULL,
            "PK11_DigestBegin",
            XMLSEC_ERRORS_R_CRYPTO_FAILED,
            "error code = %d", PORT_GetError());
    goto done;
    }

    s = PK11_DigestOp(context, in, inSize);
    if (s != SECSuccess) {
    xmlSecError(XMLSEC_ERRORS_HERE,
            NULL,
            "PK11_DigestOp",
            XMLSEC_ERRORS_R_CRYPTO_FAILED,
            "error code = %d", PORT_GetError());
    goto done;
    }

    s = PK11_DigestFinal(context, out, &len, outSize);
    if (s != SECSuccess) {
    xmlSecError(XMLSEC_ERRORS_HERE,
            NULL,
            "PK11_DigestFinal",
            XMLSEC_ERRORS_R_CRYPTO_FAILED,
            "error code = %d", PORT_GetError());
    goto done;
    }
    xmlSecAssert2(len == SHA1_LENGTH, NULL);

    digest = out;

done:
    if (context != NULL) {
    PK11_DestroyContext(context, PR_TRUE);
    }
    return (digest);
}

static int
xmlSecNssKWDes3Encrypt(
    PK11SymKey*                 symKey ,
    CK_MECHANISM_TYPE           cipherMech ,
    const xmlSecByte*           iv ,
    xmlSecSize                  ivSize ,
    const xmlSecByte*           in ,
    xmlSecSize                  inSize ,
    xmlSecByte*                 out ,
    xmlSecSize                  outSize ,
    int                         enc
) {
    PK11Context*        EncContext = NULL;
    SECItem             ivItem ;
    SECItem*            secParam = NULL ;
    int                 tmp1_outlen;
    unsigned int        tmp2_outlen;
    int                 result_len = -1;
    SECStatus           rv;

    xmlSecAssert2( cipherMech != CKM_INVALID_MECHANISM , -1 ) ;
    xmlSecAssert2( symKey != NULL , -1 ) ;
    xmlSecAssert2(iv != NULL, -1);
    xmlSecAssert2(ivSize == XMLSEC_NSS_DES3_IV_LENGTH, -1);
    xmlSecAssert2(in != NULL, -1);
    xmlSecAssert2(inSize > 0, -1);
    xmlSecAssert2(out != NULL, -1);
    xmlSecAssert2(outSize >= inSize, -1);

    /* Prepare IV */
    ivItem.data = ( unsigned char* )iv ;
    ivItem.len = ivSize ;

    secParam = PK11_ParamFromIV(cipherMech, &ivItem);
    if (secParam == NULL) {
        xmlSecError(XMLSEC_ERRORS_HERE,
            NULL,
            "PK11_ParamFromIV",
            XMLSEC_ERRORS_R_CRYPTO_FAILED,
            "Error code = %d", PORT_GetError());
        goto done;
    }

    EncContext = PK11_CreateContextBySymKey(cipherMech,
                            enc ? CKA_ENCRYPT : CKA_DECRYPT,
                        symKey, secParam);
    if (EncContext == NULL) {
        xmlSecError(XMLSEC_ERRORS_HERE,
            NULL,
            "PK11_CreateContextBySymKey",
            XMLSEC_ERRORS_R_CRYPTO_FAILED,
            "Error code = %d", PORT_GetError());
        goto done;
    }

    tmp1_outlen = tmp2_outlen = 0;
    rv = PK11_CipherOp(EncContext, out, &tmp1_outlen, outSize,
               (unsigned char *)in, inSize);
    if (rv != SECSuccess) {
        xmlSecError(XMLSEC_ERRORS_HERE,
            NULL,
            "PK11_CipherOp",
            XMLSEC_ERRORS_R_CRYPTO_FAILED,
            "Error code = %d", PORT_GetError());
        goto done;
    }

    rv = PK11_DigestFinal(EncContext, out+tmp1_outlen,
                  &tmp2_outlen, outSize-tmp1_outlen);
    if (rv != SECSuccess) {
        xmlSecError(XMLSEC_ERRORS_HERE,
            NULL,
            "PK11_DigestFinal",
            XMLSEC_ERRORS_R_CRYPTO_FAILED,
            "Error code = %d", PORT_GetError());
        goto done;
    }

    result_len = tmp1_outlen + tmp2_outlen;

done:
    if (secParam) {
        SECITEM_FreeItem(secParam, PR_TRUE);
    }
    if (EncContext) {
    PK11_DestroyContext(EncContext, PR_TRUE);
    }

    return(result_len);
}

static int
xmlSecNssKeyWrapDesOp(
    xmlSecNssKeyWrapCtxPtr      ctx ,
    int                         encrypt ,
    xmlSecBufferPtr             result
) {
    xmlSecByte sha1[SHA1_LENGTH];
    xmlSecByte iv[XMLSEC_NSS_DES3_IV_LENGTH];
    xmlSecByte* in;
    xmlSecSize inSize;
    xmlSecByte* out;
    xmlSecSize outSize;
    xmlSecSize s;
    int ret;
    SECStatus status;

    xmlSecAssert2( ctx != NULL , -1 ) ;
    xmlSecAssert2( ctx->cipher != CKM_INVALID_MECHANISM , -1 ) ;
    xmlSecAssert2( ctx->symkey != NULL , -1 ) ;
    xmlSecAssert2( ctx->keyId != NULL , -1 ) ;
    xmlSecAssert2( ctx->material != NULL , -1 ) ;
    xmlSecAssert2( result != NULL , -1 ) ;

    in = xmlSecBufferGetData(ctx->material);
    inSize = xmlSecBufferGetSize(ctx->material) ;
    out = xmlSecBufferGetData(result);
    outSize = xmlSecBufferGetMaxSize(result) ;
    if( encrypt ) {
        /* step 2: calculate sha1 and CMS */
        if(xmlSecNssComputeSHA1(in, inSize, sha1, SHA1_LENGTH) == NULL) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecNssComputeSHA1",
                XMLSEC_ERRORS_R_CRYPTO_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }

        /* step 3: construct WKCKS */
        memcpy(out, in, inSize);
        memcpy(out + inSize, sha1, XMLSEC_NSS_DES3_BLOCK_LENGTH);

        /* step 4: generate random iv */
        status = PK11_GenerateRandom(iv, XMLSEC_NSS_DES3_IV_LENGTH);
        if(status != SECSuccess) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "PK11_GenerateRandom",
                XMLSEC_ERRORS_R_CRYPTO_FAILED,
                "error code = %d", PORT_GetError());
            return(-1);
        }

        /* step 5: first encryption, result is TEMP1 */
        ret = xmlSecNssKWDes3Encrypt( ctx->symkey, ctx->cipher,
                    iv, XMLSEC_NSS_DES3_IV_LENGTH,
                    out, inSize + XMLSEC_NSS_DES3_IV_LENGTH,
                    out, outSize, 1);
        if(ret < 0) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecNssKWDes3Encrypt",
                XMLSEC_ERRORS_R_XMLSEC_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }

        /* step 6: construct TEMP2=IV || TEMP1 */
        memmove(out + XMLSEC_NSS_DES3_IV_LENGTH, out,
            inSize + XMLSEC_NSS_DES3_IV_LENGTH);
        memcpy(out, iv, XMLSEC_NSS_DES3_IV_LENGTH);
            s = ret + XMLSEC_NSS_DES3_IV_LENGTH;

        /* step 7: reverse octets order, result is TEMP3 */
        ret = xmlSecNssKWDes3BufferReverse(out, s);
        if(ret < 0) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecNssKWDes3BufferReverse",
                XMLSEC_ERRORS_R_XMLSEC_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }

        /* step 8: second encryption with static IV */
        ret = xmlSecNssKWDes3Encrypt( ctx->symkey, ctx->cipher,
                    xmlSecNssKWDes3Iv, XMLSEC_NSS_DES3_IV_LENGTH,
                    out, s,
                    out, outSize, 1);
        if(ret < 0) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecNssKWDes3Encrypt",
                XMLSEC_ERRORS_R_XMLSEC_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }
        s = ret;

        if( xmlSecBufferSetSize( result , s ) < 0 ) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecBufferSetSize",
                XMLSEC_ERRORS_R_XMLSEC_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }
    } else {
        /* step 2: first decryption with static IV, result is TEMP3 */
        ret = xmlSecNssKWDes3Encrypt( ctx->symkey, ctx->cipher,
                    xmlSecNssKWDes3Iv, XMLSEC_NSS_DES3_IV_LENGTH,
                    in, inSize,
                    out, outSize, 0);
        if((ret < 0) || (ret < XMLSEC_NSS_DES3_IV_LENGTH)) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecNssKWDes3Encrypt",
                XMLSEC_ERRORS_R_XMLSEC_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }
        s = ret;

        /* step 3: reverse octets order in TEMP3, result is TEMP2 */
        ret = xmlSecNssKWDes3BufferReverse(out, s);
        if(ret < 0) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecNssKWDes3BufferReverse",
                XMLSEC_ERRORS_R_XMLSEC_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }

        /* steps 4 and 5: get IV and decrypt second time, result is WKCKS */
        ret = xmlSecNssKWDes3Encrypt( ctx->symkey, ctx->cipher,
                    out, XMLSEC_NSS_DES3_IV_LENGTH,
                    out+XMLSEC_NSS_DES3_IV_LENGTH, s-XMLSEC_NSS_DES3_IV_LENGTH,
                    out, outSize, 0);
        if((ret < 0) || (ret < XMLSEC_NSS_DES3_BLOCK_LENGTH)) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecNssKWDes3Encrypt",
                XMLSEC_ERRORS_R_XMLSEC_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }
        s = ret - XMLSEC_NSS_DES3_IV_LENGTH;

        /* steps 6 and 7: calculate SHA1 and validate it */
        if(xmlSecNssComputeSHA1(out, s, sha1, SHA1_LENGTH) == NULL) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecNssComputeSHA1",
                XMLSEC_ERRORS_R_CRYPTO_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
         }

        if(memcmp(sha1, out + s, XMLSEC_NSS_DES3_BLOCK_LENGTH) != 0) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                 NULL,
                XMLSEC_ERRORS_R_INVALID_DATA,
                "SHA1 does not match");
            return(-1);
        }

        if( xmlSecBufferSetSize( result , s ) < 0 ) {
            xmlSecError(XMLSEC_ERRORS_HERE,
                NULL,
                "xmlSecBufferSetSize",
                XMLSEC_ERRORS_R_XMLSEC_FAILED,
                XMLSEC_ERRORS_NO_MESSAGE);
            return(-1);
        }
    }

    return(0);
}

static int
xmlSecNssKeyWrapAesOp(
    xmlSecNssKeyWrapCtxPtr      ctx ,
    int                         encrypt ,
    xmlSecBufferPtr             result
) {
    PK11Context*        cipherCtx = NULL;
    SECItem             ivItem ;
    SECItem*            secParam = NULL ;
    xmlSecSize          inSize ;
    xmlSecSize          inBlocks ;
    int                 blockSize ;
    int                 midSize ;
    int                 finSize ;
    xmlSecByte*         out ;
    xmlSecSize          outSize;

    xmlSecAssert2( ctx != NULL , -1 ) ;
    xmlSecAssert2( ctx->cipher != CKM_INVALID_MECHANISM , -1 ) ;
    xmlSecAssert2( ctx->symkey != NULL , -1 ) ;
    xmlSecAssert2( ctx->keyId != NULL , -1 ) ;
    xmlSecAssert2( ctx->material != NULL , -1 ) ;
    xmlSecAssert2( result != NULL , -1 ) ;

    /* Do not set any IV */
    memset(&ivItem, 0, sizeof(ivItem));

    /* Get block size */
    if( ( blockSize = PK11_GetBlockSize( ctx->cipher , NULL ) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "PK11_GetBlockSize" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    inSize = xmlSecBufferGetSize( ctx->material ) ;
    if( xmlSecBufferSetMaxSize( result , inSize + blockSize ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferSetMaxSize" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    /* Get Param for context initialization */
    if( ( secParam = PK11_ParamFromIV( ctx->cipher , &ivItem ) ) == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "PK11_ParamFromIV" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    cipherCtx = PK11_CreateContextBySymKey( ctx->cipher , encrypt ? CKA_ENCRYPT : CKA_DECRYPT , ctx->symkey , secParam ) ;
    if( cipherCtx == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "PK11_CreateContextBySymKey" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        SECITEM_FreeItem( secParam , PR_TRUE ) ;
        return(-1);
    }

    out = xmlSecBufferGetData(result) ;
    outSize = xmlSecBufferGetMaxSize(result) ;
    if( PK11_CipherOp( cipherCtx , out, &midSize , outSize , xmlSecBufferGetData( ctx->material ) , inSize ) != SECSuccess ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "PK11_CipherOp" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    if( PK11_DigestFinal( cipherCtx , out + midSize , &finSize , outSize - midSize ) != SECSuccess ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "PK11_DigestFinal" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    if( xmlSecBufferSetSize( result , midSize + finSize ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferSetSize" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    return 0 ;
}

/**
 * Block cipher transform final
 */
static int
xmlSecNssKeyWrapCtxFinal(
    xmlSecNssKeyWrapCtxPtr      ctx ,
    xmlSecBufferPtr             in ,
    xmlSecBufferPtr             out ,
    int                         encrypt ,
    xmlSecTransformCtxPtr       transformCtx
) {
    PK11SymKey*         targetKey ;
    xmlSecSize          blockSize ;
    xmlSecBufferPtr     result ;

    xmlSecAssert2( ctx != NULL , -1 ) ;
    xmlSecAssert2( ctx->cipher != CKM_INVALID_MECHANISM , -1 ) ;
    xmlSecAssert2( ctx->symkey != NULL , -1 ) ;
    xmlSecAssert2( ctx->keyId != NULL , -1 ) ;
    xmlSecAssert2( ctx->material != NULL , -1 ) ;
    xmlSecAssert2( in != NULL , -1 ) ;
    xmlSecAssert2( out != NULL , -1 ) ;
    xmlSecAssert2( transformCtx != NULL , -1 ) ;

    /* read raw key material and append into context */
    if( xmlSecBufferAppend( ctx->material, xmlSecBufferGetData(in), xmlSecBufferGetSize(in) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferAppend" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    if( xmlSecBufferRemoveHead( in , xmlSecBufferGetSize(in) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferRemoveHead" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    /* Now we get all of the key materail */
    /* from now on we will wrap or unwrap the key */
    if( ( blockSize = PK11_GetBlockSize( ctx->cipher , NULL ) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "PK11_GetBlockSize" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    result = xmlSecBufferCreate( blockSize ) ;
    if( result == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferCreate" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    switch( ctx->cipher ) {
        case CKM_DES3_CBC :
            if( xmlSecNssKeyWrapDesOp(ctx, encrypt, result) < 0 ) {
                xmlSecError( XMLSEC_ERRORS_HERE ,
                    NULL ,
                    "xmlSecNssKeyWrapDesOp" ,
                    XMLSEC_ERRORS_R_CRYPTO_FAILED ,
                    XMLSEC_ERRORS_NO_MESSAGE ) ;
                xmlSecBufferDestroy(result);
                return(-1);
            }
            break ;
    /*  case CKM_NETSCAPE_AES_KEY_WRAP :*/
        case CKM_AES_CBC :
            if( xmlSecNssKeyWrapAesOp(ctx, encrypt, result) < 0 ) {
                xmlSecError( XMLSEC_ERRORS_HERE ,
                    NULL ,
                    "xmlSecNssKeyWrapAesOp" ,
                    XMLSEC_ERRORS_R_CRYPTO_FAILED ,
                    XMLSEC_ERRORS_NO_MESSAGE ) ;
                xmlSecBufferDestroy(result);
                return(-1);
            }
            break ;
    }

    /* Write output */
    if( xmlSecBufferAppend( out, xmlSecBufferGetData(result), xmlSecBufferGetSize(result) ) < 0 ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            NULL ,
            "xmlSecBufferAppend" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        xmlSecBufferDestroy(result);
        return(-1);
    }
    xmlSecBufferDestroy(result);

    return(0);
}

static int
xmlSecNssKeyWrapExecute(xmlSecTransformPtr transform, int last, xmlSecTransformCtxPtr transformCtx) {
    xmlSecNssKeyWrapCtxPtr  context = NULL ;
    xmlSecBufferPtr         inBuf, outBuf ;
    int                     operation ;
    int                     rtv ;

    xmlSecAssert2( xmlSecNssKeyWrapCheckId( transform ), -1 ) ;
    xmlSecAssert2( xmlSecTransformCheckSize( transform, xmlSecNssKeyWrapSize ), -1 ) ;
    xmlSecAssert2( ( transform->operation == xmlSecTransformOperationEncrypt ) || ( transform->operation == xmlSecTransformOperationDecrypt ), -1 ) ;
    xmlSecAssert2( transformCtx != NULL , -1 ) ;

    context = xmlSecNssKeyWrapGetCtx( transform ) ;
    if( context == NULL ) {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecTransformGetName( transform ) ) ,
            "xmlSecNssKeyWrapGetCtx" ,
            XMLSEC_ERRORS_R_CRYPTO_FAILED ,
            XMLSEC_ERRORS_NO_MESSAGE ) ;
        return(-1);
    }

    inBuf = &( transform->inBuf ) ;
    outBuf = &( transform->outBuf ) ;

    if( transform->status == xmlSecTransformStatusNone ) {
        transform->status = xmlSecTransformStatusWorking ;
    }

    operation = ( transform->operation == xmlSecTransformOperationEncrypt ) ? 1 : 0 ;
    if( transform->status == xmlSecTransformStatusWorking ) {
        if( context->material == NULL ) {
            rtv = xmlSecNssKeyWrapCtxInit( context, inBuf , outBuf , operation , transformCtx ) ;
            if( rtv < 0 ) {
                xmlSecError( XMLSEC_ERRORS_HERE ,
                    xmlSecErrorsSafeString( xmlSecTransformGetName( transform ) ) ,
                    "xmlSecNssKeyWrapCtxInit" ,
                    XMLSEC_ERRORS_R_INVALID_STATUS ,
                    XMLSEC_ERRORS_NO_MESSAGE ) ;
                return(-1);
            }
        }

        if( context->material == NULL && last != 0 ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                xmlSecErrorsSafeString( xmlSecTransformGetName( transform ) ) ,
                NULL ,
                XMLSEC_ERRORS_R_INVALID_STATUS ,
                "No enough data to intialize transform" ) ;
            return(-1);
        }

        if( context->material != NULL ) {
            rtv = xmlSecNssKeyWrapCtxUpdate( context, inBuf , outBuf , operation , transformCtx ) ;
            if( rtv < 0 ) {
                xmlSecError( XMLSEC_ERRORS_HERE ,
                    xmlSecErrorsSafeString( xmlSecTransformGetName( transform ) ) ,
                    "xmlSecNssKeyWrapCtxUpdate" ,
                    XMLSEC_ERRORS_R_INVALID_STATUS ,
                    XMLSEC_ERRORS_NO_MESSAGE ) ;
                return(-1);
            }
        }

        if( last ) {
            rtv = xmlSecNssKeyWrapCtxFinal( context, inBuf , outBuf , operation , transformCtx ) ;
            if( rtv < 0 ) {
                xmlSecError( XMLSEC_ERRORS_HERE ,
                    xmlSecErrorsSafeString( xmlSecTransformGetName( transform ) ) ,
                    "xmlSecNssKeyWrapCtxFinal" ,
                    XMLSEC_ERRORS_R_INVALID_STATUS ,
                    XMLSEC_ERRORS_NO_MESSAGE ) ;
                return(-1);
            }
            transform->status = xmlSecTransformStatusFinished ;
        }
    } else if( transform->status == xmlSecTransformStatusFinished ) {
        if( xmlSecBufferGetSize( inBuf ) != 0 ) {
            xmlSecError( XMLSEC_ERRORS_HERE ,
                xmlSecErrorsSafeString( xmlSecTransformGetName( transform ) ) ,
                NULL ,
                XMLSEC_ERRORS_R_INVALID_STATUS ,
                "status=%d", transform->status ) ;
            return(-1);
        }
    } else {
        xmlSecError( XMLSEC_ERRORS_HERE ,
            xmlSecErrorsSafeString( xmlSecTransformGetName( transform ) ) ,
            NULL ,
            XMLSEC_ERRORS_R_INVALID_STATUS ,
            "status=%d", transform->status ) ;
        return(-1);
    }

    return(0);
}

#ifndef XMLSEC_NO_AES


#ifdef __MINGW32__ // for runtime-pseudo-reloc
static struct _xmlSecTransformKlass xmlSecNssKWAes128Klass = {
#else
static xmlSecTransformKlass xmlSecNssKWAes128Klass = {
#endif
    /* klass/object sizes */
    sizeof(xmlSecTransformKlass),       /* xmlSecSize klassSize */
    xmlSecNssKeyWrapSize,               /* xmlSecSize objSize */

    xmlSecNameKWAes128,             /* const xmlChar* name; */
    xmlSecHrefKWAes128,             /* const xmlChar* href; */
    xmlSecTransformUsageEncryptionMethod,   /* xmlSecAlgorithmUsage usage; */

    xmlSecNssKeyWrapInitialize,             /* xmlSecTransformInitializeMethod initialize; */
    xmlSecNssKeyWrapFinalize,           /* xmlSecTransformFinalizeMethod finalize; */
    NULL,                   /* xmlSecTransformNodeReadMethod readNode; */
    NULL,                   /* xmlSecTransformNodeWriteMethod writeNode; */
    xmlSecNssKeyWrapSetKeyReq,          /* xmlSecTransformSetKeyMethod setKeyReq; */
    xmlSecNssKeyWrapSetKey,         /* xmlSecTransformSetKeyMethod setKey; */
    NULL,                   /* xmlSecTransformValidateMethod validate; */
    xmlSecTransformDefaultGetDataType,      /* xmlSecTransformGetDataTypeMethod getDataType; */
    xmlSecTransformDefaultPushBin,      /* xmlSecTransformPushBinMethod pushBin; */
    xmlSecTransformDefaultPopBin,       /* xmlSecTransformPopBinMethod popBin; */
    NULL,                   /* xmlSecTransformPushXmlMethod pushXml; */
    NULL,                   /* xmlSecTransformPopXmlMethod popXml; */
    xmlSecNssKeyWrapExecute,            /* xmlSecTransformExecuteMethod execute; */

    NULL,                   /* void* reserved0; */
    NULL,                   /* void* reserved1; */
};

#ifdef __MINGW32__ // for runtime-pseudo-reloc
static struct _xmlSecTransformKlass xmlSecNssKWAes192Klass = {
#else
static xmlSecTransformKlass xmlSecNssKWAes192Klass = {
#endif
    /* klass/object sizes */
    sizeof(xmlSecTransformKlass),       /* xmlSecSize klassSize */
    xmlSecNssKeyWrapSize,               /* xmlSecSize objSize */

    xmlSecNameKWAes192,             /* const xmlChar* name; */
    xmlSecHrefKWAes192,             /* const xmlChar* href; */
    xmlSecTransformUsageEncryptionMethod,   /* xmlSecAlgorithmUsage usage; */

    xmlSecNssKeyWrapInitialize,             /* xmlSecTransformInitializeMethod initialize; */
    xmlSecNssKeyWrapFinalize,           /* xmlSecTransformFinalizeMethod finalize; */
    NULL,                   /* xmlSecTransformNodeReadMethod readNode; */
    NULL,                   /* xmlSecTransformNodeWriteMethod writeNode; */
    xmlSecNssKeyWrapSetKeyReq,          /* xmlSecTransformSetKeyMethod setKeyReq; */
    xmlSecNssKeyWrapSetKey,         /* xmlSecTransformSetKeyMethod setKey; */
    NULL,                   /* xmlSecTransformValidateMethod validate; */
    xmlSecTransformDefaultGetDataType,      /* xmlSecTransformGetDataTypeMethod getDataType; */
    xmlSecTransformDefaultPushBin,      /* xmlSecTransformPushBinMethod pushBin; */
    xmlSecTransformDefaultPopBin,       /* xmlSecTransformPopBinMethod popBin; */
    NULL,                   /* xmlSecTransformPushXmlMethod pushXml; */
    NULL,                   /* xmlSecTransformPopXmlMethod popXml; */
    xmlSecNssKeyWrapExecute,            /* xmlSecTransformExecuteMethod execute; */

    NULL,                   /* void* reserved0; */
    NULL,                   /* void* reserved1; */
};

#ifdef __MINGW32__ // for runtime-pseudo-reloc
static struct _xmlSecTransformKlass xmlSecNssKWAes256Klass = {
#else
static xmlSecTransformKlass xmlSecNssKWAes256Klass = {
#endif
    /* klass/object sizes */
    sizeof(xmlSecTransformKlass),       /* xmlSecSize klassSize */
    xmlSecNssKeyWrapSize,               /* xmlSecSize objSize */

    xmlSecNameKWAes256,             /* const xmlChar* name; */
    xmlSecHrefKWAes256,             /* const xmlChar* href; */
    xmlSecTransformUsageEncryptionMethod,   /* xmlSecAlgorithmUsage usage; */

    xmlSecNssKeyWrapInitialize,             /* xmlSecTransformInitializeMethod initialize; */
    xmlSecNssKeyWrapFinalize,           /* xmlSecTransformFinalizeMethod finalize; */
    NULL,                   /* xmlSecTransformNodeReadMethod readNode; */
    NULL,                   /* xmlSecTransformNodeWriteMethod writeNode; */
    xmlSecNssKeyWrapSetKeyReq,          /* xmlSecTransformSetKeyMethod setKeyReq; */
    xmlSecNssKeyWrapSetKey,         /* xmlSecTransformSetKeyMethod setKey; */
    NULL,                   /* xmlSecTransformValidateMethod validate; */
    xmlSecTransformDefaultGetDataType,      /* xmlSecTransformGetDataTypeMethod getDataType; */
    xmlSecTransformDefaultPushBin,      /* xmlSecTransformPushBinMethod pushBin; */
    xmlSecTransformDefaultPopBin,       /* xmlSecTransformPopBinMethod popBin; */
    NULL,                   /* xmlSecTransformPushXmlMethod pushXml; */
    NULL,                   /* xmlSecTransformPopXmlMethod popXml; */
    xmlSecNssKeyWrapExecute,            /* xmlSecTransformExecuteMethod execute; */

    NULL,                   /* void* reserved0; */
    NULL,                   /* void* reserved1; */
};

/**
 * xmlSecNssTransformKWAes128GetKlass:
 *
 * The AES-128 key wrapper transform klass.
 *
 * Returns AES-128 key wrapper transform klass.
 */
xmlSecTransformId
xmlSecNssTransformKWAes128GetKlass(void) {
    return(&xmlSecNssKWAes128Klass);
}

/**
 * xmlSecNssTransformKWAes192GetKlass:
 *
 * The AES-192 key wrapper transform klass.
 *
 * Returns AES-192 key wrapper transform klass.
 */
xmlSecTransformId
xmlSecNssTransformKWAes192GetKlass(void) {
    return(&xmlSecNssKWAes192Klass);
}

/**
 *
 * The AES-256 key wrapper transform klass.
 *
 * Returns AES-256 key wrapper transform klass.
 */
xmlSecTransformId
xmlSecNssTransformKWAes256GetKlass(void) {
    return(&xmlSecNssKWAes256Klass);
}

#endif /* XMLSEC_NO_AES */


#ifndef XMLSEC_NO_DES

#ifdef __MINGW32__ // for runtime-pseudo-reloc
static struct _xmlSecTransformKlass xmlSecNssKWDes3Klass = {
#else
static xmlSecTransformKlass xmlSecNssKWDes3Klass = {
#endif
    /* klass/object sizes */
    sizeof(xmlSecTransformKlass),       /* xmlSecSize klassSize */
    xmlSecNssKeyWrapSize,           /* xmlSecSize objSize */

    xmlSecNameKWDes3,               /* const xmlChar* name; */
    xmlSecHrefKWDes3,               /* const xmlChar* href; */
    xmlSecTransformUsageEncryptionMethod,   /* xmlSecAlgorithmUsage usage; */

    xmlSecNssKeyWrapInitialize,             /* xmlSecTransformInitializeMethod initialize; */
    xmlSecNssKeyWrapFinalize,           /* xmlSecTransformFinalizeMethod finalize; */
    NULL,                   /* xmlSecTransformNodeReadMethod readNode; */
    NULL,                   /* xmlSecTransformNodeWriteMethod writeNode; */
    xmlSecNssKeyWrapSetKeyReq,          /* xmlSecTransformSetKeyMethod setKeyReq; */
    xmlSecNssKeyWrapSetKey,         /* xmlSecTransformSetKeyMethod setKey; */
    NULL,                   /* xmlSecTransformValidateMethod validate; */
    xmlSecTransformDefaultGetDataType,      /* xmlSecTransformGetDataTypeMethod getDataType; */
    xmlSecTransformDefaultPushBin,      /* xmlSecTransformPushBinMethod pushBin; */
    xmlSecTransformDefaultPopBin,       /* xmlSecTransformPopBinMethod popBin; */
    NULL,                   /* xmlSecTransformPushXmlMethod pushXml; */
    NULL,                   /* xmlSecTransformPopXmlMethod popXml; */
    xmlSecNssKeyWrapExecute,            /* xmlSecTransformExecuteMethod execute; */

    NULL,                   /* void* reserved0; */
    NULL,                   /* void* reserved1; */
};

/**
 * xmlSecNssTransformKWDes3GetKlass:
 *
 * The Triple DES key wrapper transform klass.
 *
 * Returns Triple DES key wrapper transform klass.
 */
xmlSecTransformId
xmlSecNssTransformKWDes3GetKlass(void) {
    return(&xmlSecNssKWDes3Klass);
}

#endif /* XMLSEC_NO_DES */

