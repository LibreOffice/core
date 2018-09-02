/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_RTL_DIGEST_H
#define INCLUDED_RTL_DIGEST_H

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Digest Handle opaque type.
 */
typedef void* rtlDigest;


/** Digest Algorithm enumeration.
    @see rtl_digest_create()
 */
enum __rtl_DigestAlgorithm
{
    rtl_Digest_AlgorithmMD2,
    rtl_Digest_AlgorithmMD5,
    rtl_Digest_AlgorithmSHA,
    rtl_Digest_AlgorithmSHA1,

    rtl_Digest_AlgorithmHMAC_MD5,
    rtl_Digest_AlgorithmHMAC_SHA1,

    rtl_Digest_AlgorithmInvalid,
    rtl_Digest_Algorithm_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Digest Algorithm type.
 */
typedef enum __rtl_DigestAlgorithm rtlDigestAlgorithm;


/** Error Code enumeration.
 */
enum __rtl_DigestError
{
    rtl_Digest_E_None,
    rtl_Digest_E_Argument,
    rtl_Digest_E_Algorithm,
    rtl_Digest_E_BufferSize,
    rtl_Digest_E_Memory,
    rtl_Digest_E_Unknown,
    rtl_Digest_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Error Code type.
 */
typedef enum __rtl_DigestError rtlDigestError;


/** Create a digest handle for the given algorithm.
    @see rtlDigestAlgorithm

    @param[in] Algorithm digest algorithm.
    @return Digest handle, or 0 upon failure.
 */
SAL_DLLPUBLIC rtlDigest SAL_CALL rtl_digest_create  (
    rtlDigestAlgorithm Algorithm
) SAL_THROW_EXTERN_C();


/** Destroy a digest handle.
    @post Digest handle destroyed and invalid.
    @param[in] Digest digest handle to be destroyed.
    @return   None.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_digest_destroy (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();


/** Query the algorithm of a given digest.
    @param[in] Digest digest handle.
    @return digest algorithm, or <code>rtl_Digest_AlgorithmInvalid</code> upon failure.
 */
SAL_DLLPUBLIC rtlDigestAlgorithm SAL_CALL rtl_digest_queryAlgorithm (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();


/** Query the length of a given digest.
    @param[in] Digest digest handle.
    @return digest length, or 0 upon failure.
 */
SAL_DLLPUBLIC sal_uInt32 SAL_CALL rtl_digest_queryLength (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();


/** Initialize a digest with given data.
    @param[in] Digest  digest handle.
    @param[in] pData   data buffer.
    @param[in] nDatLen data length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_init (
    rtlDigest Digest,
    const sal_uInt8 *pData, sal_uInt32 nDatLen
) SAL_THROW_EXTERN_C();


/** Update a digest with given data.
    @param[in] Digest  digest handle.
    @param[in] pData   data buffer.
    @param[in] nDatLen data length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_update (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen
) SAL_THROW_EXTERN_C();


/** Finalize a digest and retrieve the digest value.
    @pre  Digest value length must not be less than digest length.
    @post Digest initialized to accept another update sequence.
    @see      rtl_digest_queryLength()
    @see      rtl_digest_update()

    @param[in] Digest  digest handle.
    @param[in] pBuffer digest value buffer.
    @param[in] nBufLen digest value length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_get (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

#define RTL_DIGEST_LENGTH_MD2 16

/** Create a MD2 digest handle.

    The MD2 digest algorithm is specified in
    RFC 1319 (Informational)
      The MD2 Message-Digest Algorithm

    @see rtl_digest_create()
 */
SAL_DLLPUBLIC rtlDigest SAL_CALL rtl_digest_createMD2 (void) SAL_THROW_EXTERN_C();


/** Destroy a MD2 digest handle.
    @see rtl_digest_destroy()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_digest_destroyMD2 (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();

/** Update a MD2 digest with given data.
    @see rtl_digest_update()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_updateMD2 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen
) SAL_THROW_EXTERN_C();

/** Finalize a MD2 digest and retrieve the digest value.
    @see rtl_digest_get()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_getMD2 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/** Evaluate a MD2 digest value from given data.

    This function performs an optimized call sequence on a
    single data buffer, avoiding digest creation and destruction.

    @see rtl_digest_updateMD2()
    @see rtl_digest_getMD2()

    @param[in] pData   data buffer.
    @param[in] nDatLen data length.
    @param[in] pBuffer digest value buffer.
    @param[in] nBufLen digest value length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_MD2 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

#define RTL_DIGEST_LENGTH_MD5 16

/** Create a MD5 digest handle.

    The MD5 digest algorithm is specified in
    RFC 1321 (Informational)
      The MD5 Message-Digest Algorithm

    @see rtl_digest_create()
 */
SAL_DLLPUBLIC rtlDigest SAL_CALL rtl_digest_createMD5 (void) SAL_THROW_EXTERN_C();

/** Destroy a MD5 digest handle.
    @see rtl_digest_destroy()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_digest_destroyMD5 (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();

/** Update a MD5 digest with given data.
    @see rtl_digest_update()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_updateMD5 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen
) SAL_THROW_EXTERN_C();

/** Finalize a MD5 digest and retrieve the digest value.
    @see rtl_digest_get()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_getMD5 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/** Retrieve the raw (not finalized) MD5 digest value.

    This function is a non-standard replacement for
    rtl_digest_getMD5() and must be used with caution.

    @post Digest initialized to accept another update sequence.
    @see rtl_digest_get()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_rawMD5 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/** Evaluate a MD5 digest value from given data.

    This function performs an optimized call sequence on a
    single data buffer, avoiding digest creation and destruction.

    @see rtl_digest_updateMD5()
    @see rtl_digest_getMD5()

    @param[in] pData   data buffer.
    @param[in] nDatLen data length.
    @param[in] pBuffer digest value buffer.
    @param[in] nBufLen digest value length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_MD5 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

#define RTL_DIGEST_LENGTH_SHA 20

/** Create a SHA digest handle.

    The SHA digest algorithm is specified in
    FIPS PUB 180 (Superseded by FIPS PUB 180-1)
      Secure Hash Standard

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_create()
 */
SAL_DLLPUBLIC rtlDigest SAL_CALL rtl_digest_createSHA (void) SAL_THROW_EXTERN_C();

/** Destroy a SHA digest handle.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_destroy()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_digest_destroySHA (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();


/** Update a SHA digest with given data.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_update()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_updateSHA (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen
) SAL_THROW_EXTERN_C();

/** Finalize a SHA digest and retrieve the digest value.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_get()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_getSHA (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/** Evaluate a SHA digest value from given data.

    This function performs an optimized call sequence on a
    single data buffer, avoiding digest creation and destruction.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_updateSHA()
    @see rtl_digest_getSHA()

    @param[in] pData   data buffer.
    @param[in] nDatLen data length.
    @param[in] pBuffer digest value buffer.
    @param[in] nBufLen digest value length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_SHA (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/*========================================================================
 *
 * rtl_digest_SHA1 interface.
 *
 *======================================================================*/
#define RTL_DIGEST_LENGTH_SHA1 20

/** Create a SHA1 digest handle.

    The SHA1 digest algorithm is specified in
    FIPS PUB 180-1 (Supersedes FIPS PUB 180)
      Secure Hash Standard

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_create()
 */
SAL_DLLPUBLIC rtlDigest SAL_CALL rtl_digest_createSHA1 (void) SAL_THROW_EXTERN_C();

/** Destroy a SHA1 digest handle.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_destroy()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_digest_destroySHA1 (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();

/** Update a SHA1 digest with given data.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_update()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_updateSHA1 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen
) SAL_THROW_EXTERN_C();

/** Finalize a SHA1 digest and retrieve the digest value.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_get()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_getSHA1 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/** Evaluate a SHA1 digest value from given data.

    This function performs an optimized call sequence on a
    single data buffer, avoiding digest creation and destruction.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_updateSHA1()
    @see rtl_digest_getSHA1()

    @param[in] pData   data buffer.
    @param[in] nDatLen data length.
    @param[in] pBuffer digest value buffer.
    @param[in] nBufLen digest value length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_SHA1 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

#define RTL_DIGEST_LENGTH_HMAC_MD5 RTL_DIGEST_LENGTH_MD5

/** Create a HMAC_MD5 digest handle.

    The HMAC_MD5 digest algorithm is specified in

    RFC 2104 (Informational)
      HMAC: Keyed-Hashing for Message Authentication

    @see rtl_digest_create()
 */
SAL_DLLPUBLIC rtlDigest SAL_CALL rtl_digest_createHMAC_MD5 (void) SAL_THROW_EXTERN_C();

/** Destroy a HMAC_MD5 digest handle.
    @see rtl_digest_destroy()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_digest_destroyHMAC_MD5 (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();

/** Initialize a HMAC_MD5 digest.
    @see rtl_digest_init()

    @param[in] Digest   digest handle.
    @param[in] pKeyData key material buffer.
    @param[in] nKeyLen  key material length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_initHMAC_MD5 (
    rtlDigest Digest,
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen
) SAL_THROW_EXTERN_C();

/** Update a HMAC_MD5 digest with given data.
    @see rtl_digest_update()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_updateHMAC_MD5 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen
) SAL_THROW_EXTERN_C();

/** Finalize a HMAC_MD5 digest and retrieve the digest value.
    @see rtl_digest_get()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_getHMAC_MD5 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/** Evaluate a HMAC_MD5 digest value from given data.

    This function performs an optimized call sequence on a
    single data buffer, avoiding digest creation and destruction.

    @see rtl_digest_initHMAC_MD5()
    @see rtl_digest_updateHMAC_MD5()
    @see rtl_digest_getHMAC_MD5()

    @param[in] pKeyData key material buffer.
    @param[in] nKeyLen  key material length.
    @param[in] pData    data buffer.
    @param[in] nDatLen  data length.
    @param[in] pBuffer  digest value buffer.
    @param[in] nBufLen  digest value length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_HMAC_MD5 (
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen,
    const void      *pData,    sal_uInt32 nDatLen,
    sal_uInt8       *pBuffer,  sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

#define RTL_DIGEST_LENGTH_HMAC_SHA1 RTL_DIGEST_LENGTH_SHA1

/** Create a HMAC_SHA1 digest handle.

    The HMAC_SHA1 digest algorithm is specified in
    RFC 2104 (Informational)
      HMAC: Keyed-Hashing for Message Authentication
    RFC 2898 (Informational)
      PKCS #5: Password-Based Cryptography Specification Version 2.0

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_create()
 */
SAL_DLLPUBLIC rtlDigest SAL_CALL rtl_digest_createHMAC_SHA1 (void) SAL_THROW_EXTERN_C();

/** Destroy a HMAC_SHA1 digest handle.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_destroy()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_digest_destroyHMAC_SHA1 (
    rtlDigest Digest
) SAL_THROW_EXTERN_C();

/** Initialize a HMAC_SHA1 digest.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_init()

    @param[in] Digest   digest handle.
    @param[in] pKeyData key material buffer.
    @param[in] nKeyLen  key material length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_initHMAC_SHA1 (
    rtlDigest Digest,
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen
) SAL_THROW_EXTERN_C();

/** Update a HMAC_SHA1 digest with given data.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_update()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_updateHMAC_SHA1 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen
) SAL_THROW_EXTERN_C();

/** Finalize a HMAC_SHA1 digest and retrieve the digest value.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_get()
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_getHMAC_SHA1 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/** Evaluate a HMAC_SHA1 digest value from given data.

    This function performs an optimized call sequence on a
    single data buffer, avoiding digest creation and destruction.

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility.

    @see rtl_digest_initHMAC_SHA1()
    @see rtl_digest_updateHMAC_SHA1()
    @see rtl_digest_getHMAC_SHA1()

    @param[in] pKeyData key material buffer.
    @param[in] nKeyLen  key material length.
    @param[in] pData    data buffer.
    @param[in] nDatLen  data length.
    @param[in] pBuffer  digest value buffer.
    @param[in] nBufLen  digest value length.

    @retval rtl_Digest_E_None upon success.
 */
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_HMAC_SHA1 (
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen,
    const void      *pData,    sal_uInt32 nDatLen,
    sal_uInt8       *pBuffer,  sal_uInt32 nBufLen
) SAL_THROW_EXTERN_C();

/** Password-Based Key Derivation Function.

    The PBKDF2 key derivation function is specified in
    RFC 2898 (Informational)
      PKCS #5: Password-Based Cryptography Specification Version 2.0

    @deprecated The implementation is buggy and generates incorrect results
                for 52 <= (len % 64) <= 55; use only for bug-compatibility
                or if the input is guaranteed to have a good length
                by a start-key derivation round.

    @param[out] pKeyData  derived key
    @param[in]  nKeyLen   derived key length
    @param[in]  pPassData password
    @param[in]  nPassLen  password length
    @param[in]  pSaltData salt
    @param[in]  nSaltLen  salt length
    @param[in]  nCount    iteration count

    @retval rtl_Digest_E_None upon success.
*/
SAL_DLLPUBLIC rtlDigestError SAL_CALL rtl_digest_PBKDF2 (
    sal_uInt8       *pKeyData , sal_uInt32 nKeyLen,
    const sal_uInt8 *pPassData, sal_uInt32 nPassLen,
    const sal_uInt8 *pSaltData, sal_uInt32 nSaltLen,
    sal_uInt32       nCount
) SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_RTL_DIGEST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
