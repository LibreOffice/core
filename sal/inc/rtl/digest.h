/*************************************************************************
 *
 *  $RCSfile: digest.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mhu $ $Date: 2001-05-03 20:39:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#ifndef _RTL_DIGEST_H_
#define _RTL_DIGEST_H_ "$Revision: 1.2 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtlDigest.
 *
 *======================================================================*/
typedef void* rtlDigest;

typedef enum {
    rtl_Digest_AlgorithmMD2,
    rtl_Digest_AlgorithmMD5,
    rtl_Digest_AlgorithmSHA,
    rtl_Digest_AlgorithmSHA1,

    rtl_Digest_AlgorithmHMAC_MD5,
    rtl_Digest_AlgorithmHMAC_SHA1,

    rtl_Digest_AlgorithmInvalid,
    rtl_Digest_Algorithm_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} rtlDigestAlgorithm;

typedef enum {
    rtl_Digest_E_None,
    rtl_Digest_E_Argument,
    rtl_Digest_E_Algorithm,
    rtl_Digest_E_BufferSize,
    rtl_Digest_E_Memory,
    rtl_Digest_E_Unknown,
    rtl_Digest_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} rtlDigestError;

rtlDigest SAL_CALL rtl_digest_create  (rtlDigestAlgorithm Algorithm);
void      SAL_CALL rtl_digest_destroy (rtlDigest Digest);

rtlDigestAlgorithm SAL_CALL rtl_digest_queryAlgorithm (rtlDigest Digest);
sal_uInt32         SAL_CALL rtl_digest_queryLength    (rtlDigest Digest);

rtlDigestError SAL_CALL rtl_digest_update (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen);

rtlDigestError SAL_CALL rtl_digest_get (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

/*========================================================================
 *
 * rtl_digest_MD2 interface.
 *
 * Reference:
 *   RFC 1319 (Informational)
 *     The MD2 Message-Digest Algorithm
 *
 *======================================================================*/
#define RTL_DIGEST_LENGTH_MD2 16

rtlDigest SAL_CALL rtl_digest_createMD2  (void);
void      SAL_CALL rtl_digest_destroyMD2 (rtlDigest Digest);

rtlDigestError SAL_CALL rtl_digest_updateMD2 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen);

rtlDigestError SAL_CALL rtl_digest_getMD2 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

rtlDigestError SAL_CALL rtl_digest_MD2 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen);

/*========================================================================
 *
 * rtl_digest_MD5 interface.
 *
 * Reference:
 *   RFC 1321 (Informational)
 *     The MD5 Message-Digest Algorithm
 *
 *======================================================================*/
#define RTL_DIGEST_LENGTH_MD5 16

rtlDigest SAL_CALL rtl_digest_createMD5  (void);
void      SAL_CALL rtl_digest_destroyMD5 (rtlDigest Digest);

rtlDigestError SAL_CALL rtl_digest_updateMD5 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen);

rtlDigestError SAL_CALL rtl_digest_getMD5 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

rtlDigestError SAL_CALL rtl_digest_MD5 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen);

/*========================================================================
 *
 * rtl_digest_SHA interface.
 *
 * Reference:
 *   FIPS PUB 180 (Superseded by FIPS PUB 180-1)
 *     Secure Hash Standard
 *
 *======================================================================*/
#define RTL_DIGEST_LENGTH_SHA 20

rtlDigest SAL_CALL rtl_digest_createSHA  (void);
void      SAL_CALL rtl_digest_destroySHA (rtlDigest Digest);

rtlDigestError SAL_CALL rtl_digest_updateSHA (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen);

rtlDigestError SAL_CALL rtl_digest_getSHA (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

rtlDigestError SAL_CALL rtl_digest_SHA (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen);

/*========================================================================
 *
 * rtl_digest_SHA1 interface.
 *
 * Reference:
 *   FIPS PUB 180-1 (Supersedes FIPS PUB 180)
 *     Secure Hash Standard
 *
 *======================================================================*/
#define RTL_DIGEST_LENGTH_SHA1 20

rtlDigest SAL_CALL rtl_digest_createSHA1  (void);
void      SAL_CALL rtl_digest_destroySHA1 (rtlDigest Digest);

rtlDigestError SAL_CALL rtl_digest_updateSHA1 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen);

rtlDigestError SAL_CALL rtl_digest_getSHA1 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

rtlDigestError SAL_CALL rtl_digest_SHA1 (
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen);

/*========================================================================
 *
 * rtl_digest_HMAC_MD5 interface.
 *
 * Reference:
 *   RFC 2104 (Informational)
 *     HMAC: Keyed-Hashing for Message Authentication
 *
 *======================================================================*/
#define RTL_DIGEST_LENGTH_HMAC_MD5 RTL_DIGEST_LENGTH_MD5

rtlDigest SAL_CALL rtl_digest_createHMAC_MD5  (void);
void      SAL_CALL rtl_digest_destroyHMAC_MD5 (rtlDigest Digest);

rtlDigestError SAL_CALL rtl_digest_initHMAC_MD5 (
    rtlDigest Digest,
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen);

rtlDigestError SAL_CALL rtl_digest_updateHMAC_MD5 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen);

rtlDigestError SAL_CALL rtl_digest_getHMAC_MD5 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

/*========================================================================
 *
 * rtl_digest_HMAC_SHA1 interface.
 *
 * Reference:
 *   RFC 2104 (Informational)
 *     HMAC: Keyed-Hashing for Message Authentication
 *   RFC 2898 (Informational)
 *     PKCS #5: Password-Based Cryptography Specification Version 2.0
 *
 *======================================================================*/
#define RTL_DIGEST_LENGTH_HMAC_SHA1 RTL_DIGEST_LENGTH_SHA1

rtlDigest SAL_CALL rtl_digest_createHMAC_SHA1  (void);
void      SAL_CALL rtl_digest_destroyHMAC_SHA1 (rtlDigest Digest);

rtlDigestError SAL_CALL rtl_digest_initHMAC_SHA1 (
    rtlDigest Digest,
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen);

rtlDigestError SAL_CALL rtl_digest_updateHMAC_SHA1 (
    rtlDigest Digest,
    const void *pData, sal_uInt32 nDatLen);

rtlDigestError SAL_CALL rtl_digest_getHMAC_SHA1 (
    rtlDigest Digest,
    sal_uInt8 *pBuffer, sal_uInt32 nBufLen);

/*========================================================================
 *
 * rtl_digest_PBKDF2 interface.
 *
 * Reference:
 *   RFC 2898 (Informational)
 *     PKCS #5: Password-Based Cryptography Specification Version 2.0
 *
 *======================================================================*/
/** Password-Based Key Derivation Function.

    @param  pKeyData  [out] derived key
    @param  nKeyLen   [in]  derived key length
    @param  pPassData [in]  password
    @param  nPassLen  [in]  password length
    @param  pSaltData [in]  salt
    @param  nSaltLen  [in]  salt length
    @param  nCount    [in]  iteration count

    @return rtl_Digest_E_None upon success.
*/
rtlDigestError SAL_CALL rtl_digest_PBKDF2 (
    sal_uInt8       *pKeyData , sal_uInt32 nKeyLen,
    const sal_uInt8 *pPassData, sal_uInt32 nPassLen,
    const sal_uInt8 *pSaltData, sal_uInt32 nSaltLen,
    sal_uInt32       nCount);

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _RTL_DIGEST_H_ */

