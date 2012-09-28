/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _RTL_CIPHER_H_
#define _RTL_CIPHER_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtlCipher interface.
 *
 *======================================================================*/
/** Cipher Handle opaque type.
 */
typedef void* rtlCipher;


/** Cipher Algorithm enumeration.
    @see rtl_cipher_create()
 */
enum __rtl_CipherAlgorithm
{
    rtl_Cipher_AlgorithmBF,
    rtl_Cipher_AlgorithmARCFOUR,
    rtl_Cipher_AlgorithmInvalid,
    rtl_Cipher_Algorithm_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Cipher Algorithm type.
 */
typedef enum __rtl_CipherAlgorithm rtlCipherAlgorithm;


/** Cipher Mode enumeration.
    @see rtl_cipher_create()
 */
enum __rtl_CipherMode
{
    rtl_Cipher_ModeECB,
    rtl_Cipher_ModeCBC,
    rtl_Cipher_ModeStream,
    rtl_Cipher_ModeInvalid,
    rtl_Cipher_Mode_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Cipher Mode type.
 */
typedef enum __rtl_CipherMode rtlCipherMode;


/** Cipher Direction enumeration.
    @see rtl_cipher_init()
 */
enum __rtl_CipherDirection
{
    rtl_Cipher_DirectionBoth,
    rtl_Cipher_DirectionDecode,
    rtl_Cipher_DirectionEncode,
    rtl_Cipher_DirectionInvalid,
    rtl_Cipher_Direction_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Cipher Direction type.
 */
typedef enum __rtl_CipherDirection rtlCipherDirection;


/** Error Code enumeration.
 */
enum __rtl_CipherError
{
    rtl_Cipher_E_None,
    rtl_Cipher_E_Argument,
    rtl_Cipher_E_Algorithm,
    rtl_Cipher_E_Direction,
    rtl_Cipher_E_Mode,
    rtl_Cipher_E_BufferSize,
    rtl_Cipher_E_Memory,
    rtl_Cipher_E_Unknown,
    rtl_Cipher_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Error Code type.
 */
typedef enum __rtl_CipherError rtlCipherError;


/** Create a cipher handle for the given algorithm and mode.
    @see rtlCipherAlgorithm
    @see rtlCipherMode

    @param  Algorithm [in] cipher algorithm.
    @param  Mode      [in] cipher mode.
    @return Cipher handle, or 0 upon failure.
 */
SAL_DLLPUBLIC rtlCipher SAL_CALL rtl_cipher_create (
    rtlCipherAlgorithm Algorithm,
    rtlCipherMode      Mode
) SAL_THROW_EXTERN_C();


/** Inititialize a cipher for the given direction.
    @see rtlCipherDirection

    @param  Cipher    [in] cipher handle.
    @param  Direction [in] cipher direction.
    @param  pKeyData  [in] key material buffer.
    @param  nKeyLen   [in] key material length in bytes.
    @param  pArgData  [in] initialization vector buffer.
    @param  nArgLen   [in] initialization vector length in bytes.
    @return rtl_Cipher_E_None upon success.
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_init (
    rtlCipher           Cipher,
    rtlCipherDirection  Direction,
    const sal_uInt8    *pKeyData, sal_Size nKeyLen,
    const sal_uInt8    *pArgData, sal_Size nArgLen
) SAL_THROW_EXTERN_C();


/** Encode a buffer under a given cipher algorithm.
    @precond Initialized for a compatible cipher direction.
    @see     rtl_cipher_init()

    @param  Cipher  [in]  cipher handle.
    @param  pData   [in]  plaintext buffer.
    @param  nDatLen [in]  plaintext length in bytes.
    @param  pBuffer [out] ciphertext buffer.
    @param  nBufLen [in]  ciphertext length in bytes.
    @return rtl_Cipher_E_None upon success.
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_encode (
    rtlCipher   Cipher,
    const void *pData,   sal_Size nDatLen,
    sal_uInt8  *pBuffer, sal_Size nBufLen
) SAL_THROW_EXTERN_C();


/** Decode a buffer under a given cipher algorithm.
    @precond Initialized for a compatible cipher direction.
    @see     rtl_cipher_init()

    @param  Cipher  [in]  cipher handle.
    @param  pData   [in]  ciphertext buffer.
    @param  nDatLen [in]  ciphertext length in bytes.
    @param  pBuffer [out] plaintext buffer.
    @param  nBufLen [in]  plaintext length in bytes.
    @return rtl_Cipher_E_None upon success.
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_decode (
    rtlCipher   Cipher,
    const void *pData,   sal_Size nDatLen,
    sal_uInt8  *pBuffer, sal_Size nBufLen
) SAL_THROW_EXTERN_C();


/** Destroy a cipher handle.
    @param  Cipher [in] cipher handle to be destroyed.
    @return None. Cipher handle destroyed and invalid.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_cipher_destroy (
    rtlCipher Cipher
) SAL_THROW_EXTERN_C();


/*========================================================================
 *
 * rtl_cipherBF (Blowfish) interface.
 *
 *======================================================================*/
/** Create a Blowfish cipher handle for the given mode.
    @descr The Blowfish block cipher algorithm is specified in
    Bruce Schneier: Applied Cryptography, 2nd edition, ch. 14.3

    @see rtl_cipher_create()
 */
SAL_DLLPUBLIC rtlCipher SAL_CALL rtl_cipher_createBF (
    rtlCipherMode Mode
) SAL_THROW_EXTERN_C();


/** Inititialize a Blowfish cipher for the given direction.
    @see rtl_cipher_init()
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_initBF (
    rtlCipher          Cipher,
    rtlCipherDirection Direction,
    const sal_uInt8 *pKeyData, sal_Size nKeyLen,
    const sal_uInt8 *pArgData, sal_Size nArgLen
) SAL_THROW_EXTERN_C();


/** Encode a buffer under the Blowfish cipher algorithm.
    @see rtl_cipher_encode()
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_encodeBF (
    rtlCipher   Cipher,
    const void *pData,   sal_Size nDatLen,
    sal_uInt8  *pBuffer, sal_Size nBufLen
) SAL_THROW_EXTERN_C();


/** Decode a buffer under the Blowfish cipher algorithm.
    @see rtl_cipher_decode()
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_decodeBF (
    rtlCipher   Cipher,
    const void *pData,   sal_Size nDatLen,
    sal_uInt8  *pBuffer, sal_Size nBufLen
) SAL_THROW_EXTERN_C();


/** Destroy a Blowfish cipher handle.
    @see rtl_cipher_destroy()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_cipher_destroyBF (
    rtlCipher Cipher
) SAL_THROW_EXTERN_C();


/*========================================================================
 *
 * rtl_cipherARCFOUR (RC4) interface.
 *
 *======================================================================*/
/** Create a RC4 cipher handle for the given mode.
    @descr The RC4 symmetric stream cipher algorithm is specified in
    Bruce Schneier: Applied Cryptography, 2nd edition, ch. 17.1

    @see rtl_cipher_create()

    @param  Mode [in] cipher mode. Must be rtl_Cipher_ModeStream.
    @return Cipher handle, or 0 upon failure.
 */
SAL_DLLPUBLIC rtlCipher SAL_CALL rtl_cipher_createARCFOUR (
    rtlCipherMode Mode
) SAL_THROW_EXTERN_C();


/** Inititialize a RC4 cipher for the given direction.
    @see rtl_cipher_init()
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_initARCFOUR (
    rtlCipher          Cipher,
    rtlCipherDirection Direction,
    const sal_uInt8 *pKeyData, sal_Size nKeyLen,
    const sal_uInt8 *pArgData, sal_Size nArgLen
) SAL_THROW_EXTERN_C();


/** Encode a buffer under the RC4 cipher algorithm.
    @see rtl_cipher_encode()
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_encodeARCFOUR (
    rtlCipher   Cipher,
    const void *pData,   sal_Size nDatLen,
    sal_uInt8  *pBuffer, sal_Size nBufLen
) SAL_THROW_EXTERN_C();


/** Decode a buffer under the RC4 cipher algorithm.
    @see rtl_cipher_decode()
 */
SAL_DLLPUBLIC rtlCipherError SAL_CALL rtl_cipher_decodeARCFOUR (
    rtlCipher   Cipher,
    const void *pData,   sal_Size nDatLen,
    sal_uInt8  *pBuffer, sal_Size nBufLen
) SAL_THROW_EXTERN_C();


/** Destroy a RC4 cipher handle.
    @see rtl_cipher_destroy()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_cipher_destroyARCFOUR (
    rtlCipher Cipher
) SAL_THROW_EXTERN_C();


/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* !_RTL_CIPHER_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
