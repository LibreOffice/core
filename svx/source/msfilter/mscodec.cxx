/*************************************************************************
 *
 *  $RCSfile: mscodec.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 08:55:12 $
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

#ifndef SVX_MSCODEC_HXX
#include "mscodec.hxx"
#endif

#include <osl/diagnose.h>
#include <algorithm>
#include <tools/solar.h>

namespace svx {

// ============================================================================

namespace {

/** Rotates rnValue left by nBits bits. */
template< typename Type >
inline void lclRotateLeft( Type& rnValue, sal_uInt8 nBits )
{
    OSL_ASSERT( nBits < sizeof( Type ) * 8 );
    rnValue = static_cast< Type >( (rnValue << nBits) | (rnValue >> (sizeof( Type ) * 8 - nBits)) );
}

/** Rotates the lower nWidth bits of rnValue left by nBits bits. */
template< typename Type >
inline void lclRotateLeft( Type& rnValue, sal_uInt8 nBits, sal_uInt8 nWidth )
{
    OSL_ASSERT( (nBits < nWidth) && (nWidth < sizeof( Type ) * 8) );
    Type nMask = static_cast< Type >( (1UL << nWidth) - 1 );
    rnValue = static_cast< Type >(
        ((rnValue << nBits) | ((rnValue & nMask) >> (nWidth - nBits))) & nMask );
}

sal_Size lclGetLen( const sal_uInt8* pnPassData, sal_Size nBufferSize )
{
    sal_Size nLen = 0;
    while( (nLen < nBufferSize) && pnPassData[ nLen ] ) ++nLen;
    return nLen;
}

sal_uInt16 lclGetKey( const sal_uInt8* pnPassData, sal_Size nBufferSize )
{
    sal_Size nLen = lclGetLen( pnPassData, nBufferSize );
    if( !nLen ) return 0;

    sal_uInt16 nKey = 0;
    sal_uInt16 nKeyBase = 0x8000;
    sal_uInt16 nKeyEnd = 0xFFFF;
    const sal_uInt8* pnChar = pnPassData + nLen - 1;
    for( sal_Size nIndex = 0; nIndex < nLen; ++nIndex, --pnChar )
    {
        sal_uInt8 cChar = *pnChar & 0x7F;
        for( sal_uInt8 nBit = 0; nBit < 8; ++nBit )
        {
            lclRotateLeft( nKeyBase, 1 );
            if( nKeyBase & 1 ) nKeyBase ^= 0x1020;
            if( cChar & 1 ) nKey ^= nKeyBase;
            cChar >>= 1;
            lclRotateLeft( nKeyEnd, 1 );
            if( nKeyEnd & 1 ) nKeyEnd ^= 0x1020;
        }
    }
    return nKey ^ nKeyEnd;
}

sal_uInt16 lclGetHash( const sal_uInt8* pnPassData, sal_Size nBufferSize )
{
    sal_Size nLen = lclGetLen( pnPassData, nBufferSize );

    sal_uInt16 nHash = static_cast< sal_uInt16 >( nLen );
    if( nLen )
        nHash ^= 0xCE4B;

    const sal_uInt8* pnChar = pnPassData;
    for( sal_Size nIndex = 0; nIndex < nLen; ++nIndex, ++pnChar )
    {
        sal_uInt16 cChar = *pnChar;
        sal_uInt8 nRot = static_cast< sal_uInt8 >( (nIndex + 1) % 15 );
        lclRotateLeft( cChar, nRot, 15 );
        nHash ^= cChar;
    }
    return nHash;
}


} // namespace

// ============================================================================

MSCodec_Xor95::MSCodec_Xor95(int nRotateDistance) :
    mnOffset( 0 ),
    mnKey( 0 ),
    mnHash( 0 ),
    mnRotateDistance( nRotateDistance )
{
    (void)memset( mpnKey, 0, sizeof( mpnKey ) );
}

MSCodec_Xor95::~MSCodec_Xor95()
{
    (void)memset( mpnKey, 0, sizeof( mpnKey ) );
    mnKey = mnHash = 0;
}

void MSCodec_Xor95::InitKey( const sal_uInt8 pnPassData[ 16 ] )
{
    mnKey = lclGetKey( pnPassData, 16 );
    mnHash = lclGetHash( pnPassData, 16 );

    (void)memcpy( mpnKey, pnPassData, 16 );

    static const sal_uInt8 spnFillChars[] =
    {
        0xBB, 0xFF, 0xFF, 0xBA,
        0xFF, 0xFF, 0xB9, 0x80,
        0x00, 0xBE, 0x0F, 0x00,
        0xBF, 0x0F, 0x00
    };

    sal_Size nIndex;
    sal_Size nLen = lclGetLen( pnPassData, 16 );
    const sal_uInt8* pnFillChar = spnFillChars;
    for( nIndex = nLen; nIndex < sizeof( mpnKey ); ++nIndex, ++pnFillChar )
        mpnKey[ nIndex ] = *pnFillChar;

    SVBT16 pnOrigKey;
    ShortToSVBT16( mnKey, pnOrigKey );
    sal_uInt8* pnKeyChar = mpnKey;
    for( nIndex = 0; nIndex < sizeof( mpnKey ); ++nIndex, ++pnKeyChar )
    {
        *pnKeyChar ^= pnOrigKey[ nIndex & 0x01 ];
        lclRotateLeft( *pnKeyChar, mnRotateDistance );
    }
}

bool MSCodec_Xor95::VerifyKey( sal_uInt16 nKey, sal_uInt16 nHash ) const
{
    return (nKey == mnKey) && (nHash == mnHash);
}

void MSCodec_Xor95::InitCipher()
{
    mnOffset = 0;
}

void MSCodec_XorXLS95::Decode( sal_uInt8* pnData, sal_Size nBytes )
{
    const sal_uInt8* pnCurrKey = mpnKey + mnOffset;
    const sal_uInt8* pnKeyLast = mpnKey + 0x0F;

    for( const sal_uInt8* pnDataEnd = pnData + nBytes; pnData < pnDataEnd; ++pnData )
    {
        lclRotateLeft( *pnData, 3 );
        *pnData ^= *pnCurrKey;
        if( pnCurrKey < pnKeyLast ) ++pnCurrKey; else pnCurrKey = mpnKey;
    }

    // update mnOffset
    Skip( nBytes );
}

void MSCodec_XorWord95::Decode( sal_uInt8* pnData, sal_Size nBytes )
{
    const sal_uInt8* pnCurrKey = mpnKey + mnOffset;
    const sal_uInt8* pnKeyLast = mpnKey + 0x0F;

    for( const sal_uInt8* pnDataEnd = pnData + nBytes; pnData < pnDataEnd; ++pnData )
    {
        const sal_uInt8 cChar = *pnData ^ *pnCurrKey;
    if (*pnData && cChar)
        *pnData = cChar;
        if( pnCurrKey < pnKeyLast ) ++pnCurrKey; else pnCurrKey = mpnKey;
    }

    // update mnOffset
    Skip( nBytes );
}


void MSCodec_Xor95::Skip( sal_Size nBytes )
{
    mnOffset = (mnOffset + nBytes) & 0x0F;
}

sal_uInt16 MSCodec_Xor95::GetHash( const sal_uInt8* pnPassData, sal_Size nSize )
{
    return lclGetHash( pnPassData, nSize );
}

// ============================================================================

MSCodec_Std97::MSCodec_Std97 ()
{
    m_hCipher = rtl_cipher_create (
        rtl_Cipher_AlgorithmARCFOUR, rtl_Cipher_ModeStream);
    OSL_ASSERT(m_hCipher != 0);

    m_hDigest = rtl_digest_create (
        rtl_Digest_AlgorithmMD5);
    OSL_ASSERT(m_hDigest != 0);

    (void)memset (m_pDigestValue, 0, sizeof(m_pDigestValue));
}

MSCodec_Std97::~MSCodec_Std97 ()
{
    (void)memset (m_pDigestValue, 0, sizeof(m_pDigestValue));
    rtl_digest_destroy (m_hDigest);
    rtl_cipher_destroy (m_hCipher);
}

void MSCodec_Std97::InitKey (
    const sal_uInt16 pPassData[16],
    const sal_uInt8  pUnique[16])
{
    sal_uInt8 pKeyData[64];
    int       i, n;

    // Fill PassData into KeyData.
    (void)memset (pKeyData, 0, sizeof(pKeyData));
    for (i = 0, n = 16; (i < n) && pPassData[i]; i++)
    {
        pKeyData[2*i    ] = ((pPassData[i] >> 0) & 0xff);
        pKeyData[2*i + 1] = ((pPassData[i] >> 8) & 0xff);
    }
    pKeyData[2*i] = 0x80;
    pKeyData[ 56] = i << 4;

    // Fill raw digest of KeyData into KeyData.
    (void)rtl_digest_updateMD5 (
        m_hDigest, pKeyData, sizeof(pKeyData));
    (void)rtl_digest_rawMD5 (
        m_hDigest, pKeyData, RTL_DIGEST_LENGTH_MD5);

    // Update digest with KeyData and Unique.
    for (i = 0; i < 16; i++)
    {
        rtl_digest_updateMD5 (m_hDigest, pKeyData, 5);
        rtl_digest_updateMD5 (m_hDigest, pUnique, 16);
    }

    // Update digest with padding.
    pKeyData[16] = 0x80;
    (void)memset (pKeyData + 17, 0, sizeof(pKeyData) - 17);
    pKeyData[56] = 0x80;
    pKeyData[57] = 0x0a;

    rtl_digest_updateMD5 (
        m_hDigest, &(pKeyData[16]), sizeof(pKeyData) - 16);

    // Fill raw digest of above updates into DigestValue.
    rtl_digest_rawMD5 (
        m_hDigest, m_pDigestValue, sizeof(m_pDigestValue));

    // Erase KeyData array and leave.
    (void)memset (pKeyData, 0, sizeof(pKeyData));
}

bool MSCodec_Std97::VerifyKey (
    const sal_uInt8 pSaltData[16],
    const sal_uInt8 pSaltDigest[16])
{
    bool result = false;

    if (InitCipher(0))
    {
        sal_uInt8 pDigest[RTL_DIGEST_LENGTH_MD5];
        sal_uInt8 pBuffer[64];

        // Decode SaltData into Buffer.
        rtl_cipher_decode (
            m_hCipher, pSaltData, 16, pBuffer, sizeof(pBuffer));

        pBuffer[16] = 0x80;
        (void)memset (pBuffer + 17, 0, sizeof(pBuffer) - 17);
        pBuffer[56] = 0x80;

        // Fill raw digest of Buffer into Digest.
        rtl_digest_updateMD5 (
            m_hDigest, pBuffer, sizeof(pBuffer));
        rtl_digest_rawMD5 (
            m_hDigest, pDigest, sizeof(pDigest));

        // Decode original SaltDigest into Buffer.
        rtl_cipher_decode (
            m_hCipher, pSaltDigest, 16, pBuffer, sizeof(pBuffer));

        // Compare Buffer with computed Digest.
        result = (memcmp (pBuffer, pDigest, sizeof(pDigest)) == 0);

        // Erase Buffer and Digest arrays.
        (void)memset (pBuffer, 0, sizeof(pBuffer));
        (void)memset (pDigest, 0, sizeof(pDigest));
    }

    return (result);
}

bool MSCodec_Std97::InitCipher (sal_uInt32 nCounter)
{
    rtlCipherError result;
    sal_uInt8      pKeyData[64];

    // Initialize KeyData array.
    (void)memset (pKeyData, 0, sizeof(pKeyData));

    // Fill 40 bit of DigestValue into [0..4].
    (void)memcpy (pKeyData, m_pDigestValue, 5);

    // Fill counter into [5..8].
    pKeyData[ 5] = sal_uInt8((nCounter >>  0) & 0xff);
    pKeyData[ 6] = sal_uInt8((nCounter >>  8) & 0xff);
    pKeyData[ 7] = sal_uInt8((nCounter >> 16) & 0xff);
    pKeyData[ 8] = sal_uInt8((nCounter >> 24) & 0xff);

    pKeyData[ 9] = 0x80;
    pKeyData[56] = 0x48;

    // Fill raw digest of KeyData into KeyData.
    (void)rtl_digest_updateMD5 (
        m_hDigest, pKeyData, sizeof(pKeyData));
    (void)rtl_digest_rawMD5 (
        m_hDigest, pKeyData, RTL_DIGEST_LENGTH_MD5);

    // Initialize Cipher with KeyData (for decoding).
    result = rtl_cipher_init (
        m_hCipher, rtl_Cipher_DirectionDecode,
        pKeyData, RTL_DIGEST_LENGTH_MD5, 0, 0);

    // Erase KeyData array and leave.
    (void)memset (pKeyData, 0, sizeof(pKeyData));

    return (result == rtl_Cipher_E_None);
}

bool MSCodec_Std97::Decode (
    const void *pData,   sal_Size nDatLen,
    sal_uInt8  *pBuffer, sal_Size nBufLen)
{
    rtlCipherError result;

    result = rtl_cipher_decode (
        m_hCipher, pData, nDatLen, pBuffer, nBufLen);

    return (result == rtl_Cipher_E_None);
}

bool MSCodec_Std97::Skip( sal_Size nDatLen )
{
    sal_uInt8 pnDummy[ 1024 ];
    sal_Size nDatLeft = nDatLen;
    bool bResult = true;

    while (bResult && nDatLeft)
    {
        sal_Size nBlockLen = ::std::min< sal_Size >( nDatLeft, sizeof(pnDummy) );
        bResult = Decode( pnDummy, nBlockLen, pnDummy, nBlockLen );
        nDatLeft -= nBlockLen;
    }

    return bResult;
}

// ============================================================================

} // namespace svx

