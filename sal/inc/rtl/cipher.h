/*************************************************************************
 *
 *  $RCSfile: cipher.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:14 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_CIPHER_H_
#define _RTL_CIPHER_H_ "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtlCipher interface.
 *
 *======================================================================*/
/** Opaque cipher handle.
 */
typedef void* rtlCipher;


/** rtlCipherAlgorithm.
 *
 *  @see rtl_cipher_create.
 */
typedef enum
{
    rtl_Cipher_AlgorithmBF,
    rtl_Cipher_AlgorithmInvalid,
    rtl_Cipher_Algorithm_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} rtlCipherAlgorithm;


/** rtlCipherMode.
 *
 *  @see rtl_cipher_create.
 */
typedef enum
{
    rtl_Cipher_ModeECB,
    rtl_Cipher_ModeCBC,
    rtl_Cipher_ModeStream,
    rtl_Cipher_ModeInvalid,
    rtl_Cipher_Mode_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} rtlCipherMode;


/** rtlCipherDirection.
 *
 *  @see rtl_cipher_init.
 */
typedef enum
{
    rtl_Cipher_DirectionBoth,
    rtl_Cipher_DirectionDecode,
    rtl_Cipher_DirectionEncode,
    rtl_Cipher_DirectionInvalid,
    rtl_Cipher_Direction_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} rtlCipherDirection;


/** rtlCipherError.
 */
typedef enum
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
} rtlCipherError;


/** rtl_cipher_create.
 *  Create a cipher handle for the given algorithm and mode.
 *
 *  @param  Algorithm [in]
 *  @param  Mode      [in]
 *  @return Cipher handle, or 0 upon failure.
 */
rtlCipher SAL_CALL rtl_cipher_create (
    rtlCipherAlgorithm Algorithm,
    rtlCipherMode      Mode);


/** rtl_cipher_init.
 *
 *  @param  Cipher    [in]
 *  @param  Direction [in]
 *  @param  pKeyData  [in]
 *  @param  nKeyLen   [in]
 *  @param  pArgData  [in]
 *  @param  nArgLen   [in]
 *  @return rtl_Cipher_E_None upon success.
 */
rtlCipherError SAL_CALL rtl_cipher_init (
    rtlCipher           Cipher,
    rtlCipherDirection  Direction,
    const sal_uInt8    *pKeyData, sal_uInt32 nKeyLen,
    const sal_uInt8    *pArgData, sal_uInt32 nArgLen);


/** rtl_cipher_encode.
 *
 *  @param  Cipher  [in]
 *  @param  pData   [in]
 *  @param  nDatLen [in]
 *  @param  pBuffer [out]
 *  @param  nBufLen [in]
 *  @return rtl_Cipher_E_None upon success.
 */
rtlCipherError SAL_CALL rtl_cipher_encode (
    rtlCipher   Cipher,
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen);


/** rtl_cipher_decode.
 *
 *  @param  Cipher  [in]
 *  @param  pData   [in]
 *  @param  nDatLen [in]
 *  @param  pBuffer [out]
 *  @param  nBufLen [in]
 *  @return rtl_Cipher_E_None upon success.
 */
rtlCipherError SAL_CALL rtl_cipher_decode (
    rtlCipher   Cipher,
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen);


/** rtl_cipher_destroy.
 *
 *  @param  Cipher [in] handle to be destroyed.
 *  @return None. Cipher handle destroyed and invalid.
 */
void SAL_CALL rtl_cipher_destroy (rtlCipher Cipher);


/*========================================================================
 *
 * rtl_cipherBF (Blowfish) interface.
 *
 * Reference:
 *   Bruce Schneier: Applied Cryptography, 2nd edition, ch. 14.3
 *
 *======================================================================*/
/** rtl_cipher_createBF.
 *
 *  @return Cipher handle, or 0 upon failure.
 *  @see    rtl_cipher_create.
 */
rtlCipher SAL_CALL rtl_cipher_createBF (rtlCipherMode Mode);


/** rtl_cipher_initBF.
 *
 *  @return rtl_Cipher_E_None upon success.
 *  @see    rtl_cipher_init.
 */
rtlCipherError SAL_CALL rtl_cipher_initBF (
    rtlCipher          Cipher,
    rtlCipherDirection Direction,
    const sal_uInt8 *pKeyData, sal_uInt32 nKeyLen,
    const sal_uInt8 *pArgData, sal_uInt32 nArgLen);


/** rtl_cipher_encodeBF.
 *
 *  @return rtl_Cipher_E_None upon success.
 *  @see    rtl_cipher_encode.
 */
rtlCipherError SAL_CALL rtl_cipher_encodeBF (
    rtlCipher   Cipher,
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen);


/** rtl_cipher_decodeBF.
 *
 *  @return rtl_Cipher_E_None upon success.
 *  @see    rtl_cipher_decode.
 */
rtlCipherError SAL_CALL rtl_cipher_decodeBF (
    rtlCipher   Cipher,
    const void *pData,   sal_uInt32 nDatLen,
    sal_uInt8  *pBuffer, sal_uInt32 nBufLen);


/** rtl_cipher_destroyBF.
 *
 *  @param  Cipher [in] handle to be destroyed.
 *  @return None. Cipher handle destroyed and invalid.
 *  @see    rtl_cipher_destroy.
 */
void SAL_CALL rtl_cipher_destroyBF (rtlCipher Cipher);

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* !_RTL_CIPHER_H_ */

