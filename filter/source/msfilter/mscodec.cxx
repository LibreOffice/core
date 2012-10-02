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

#include "filter/msfilter/mscodec.hxx"

#include <osl/diagnose.h>
#include <algorithm>
#include <string.h>
#include <tools/solar.h>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/docpasswordhelper.hxx>

#define DEBUG_MSO_ENCRYPTION_STD97 0

#if DEBUG_MSO_ENCRYPTION_STD97
#include <stdio.h>
#endif

using namespace ::com::sun::star;

namespace msfilter {

// ============================================================================

namespace {

/** Rotates rnValue left by nBits bits. */
template< typename Type >
inline void lclRotateLeft( Type& rnValue, int nBits )
{
    OSL_ASSERT(
        nBits >= 0 &&
        sal::static_int_cast< unsigned int >(nBits) < sizeof( Type ) * 8 );
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

sal_Bool MSCodec_Xor95::InitCodec( const uno::Sequence< beans::NamedValue >& aData )
{
    sal_Bool bResult = sal_False;

    ::comphelper::SequenceAsHashMap aHashData( aData );
    uno::Sequence< sal_Int8 > aKey = aHashData.getUnpackedValueOrDefault( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XOR95EncryptionKey" ) ), uno::Sequence< sal_Int8 >() );

    if ( aKey.getLength() == 16 )
    {
        (void)memcpy( mpnKey, aKey.getConstArray(), 16 );
        bResult = sal_True;

        mnKey = (sal_uInt16)aHashData.getUnpackedValueOrDefault( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XOR95BaseKey" ) ), (sal_Int16)0 );
        mnHash = (sal_uInt16)aHashData.getUnpackedValueOrDefault( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XOR95PasswordHash" ) ), (sal_Int16)0 );
    }
    else
        OSL_FAIL( "Unexpected key size!\n" );

    return bResult;
}

uno::Sequence< beans::NamedValue > MSCodec_Xor95::GetEncryptionData()
{
    ::comphelper::SequenceAsHashMap aHashData;
    aHashData[ ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XOR95EncryptionKey" ) ) ] <<= uno::Sequence<sal_Int8>( (sal_Int8*)mpnKey, 16 );
    aHashData[ ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XOR95BaseKey" ) ) ] <<= (sal_Int16)mnKey;
    aHashData[ ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XOR95PasswordHash" ) ) ] <<= (sal_Int16)mnHash;

    return aHashData.getAsConstNamedValueList();
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
    (void)memset (m_pDocId, 0, sizeof(m_pDocId));
}

MSCodec_Std97::~MSCodec_Std97 ()
{
    (void)memset (m_pDigestValue, 0, sizeof(m_pDigestValue));
    (void)memset (m_pDocId, 0, sizeof(m_pDocId));
    rtl_digest_destroy (m_hDigest);
    rtl_cipher_destroy (m_hCipher);
}

#if DEBUG_MSO_ENCRYPTION_STD97
static void lcl_PrintDigest(const sal_uInt8* pDigest, const char* msg)
{
    printf("digest: (%s)\n", msg);
    for (int i = 0; i < 16; ++i)
        printf("%2.2x ", pDigest[i]);
    printf("\n");
}
#else
static void lcl_PrintDigest(const sal_uInt8* /*pDigest*/, const char* /*msg*/)
{
}
#endif

sal_Bool MSCodec_Std97::InitCodec( const uno::Sequence< beans::NamedValue >& aData )
{
#if DEBUG_MSO_ENCRYPTION_STD97
    fprintf(stdout, "MSCodec_Std97::InitCodec: --begin\n");fflush(stdout);
#endif
    sal_Bool bResult = sal_False;

    ::comphelper::SequenceAsHashMap aHashData( aData );
    uno::Sequence< sal_Int8 > aKey = aHashData.getUnpackedValueOrDefault( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "STD97EncryptionKey" ) ), uno::Sequence< sal_Int8 >() );

    if ( aKey.getLength() == RTL_DIGEST_LENGTH_MD5 )
    {
        (void)memcpy( m_pDigestValue, aKey.getConstArray(), RTL_DIGEST_LENGTH_MD5 );
        uno::Sequence< sal_Int8 > aUniqueID = aHashData.getUnpackedValueOrDefault( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "STD97UniqueID" ) ), uno::Sequence< sal_Int8 >() );
        if ( aUniqueID.getLength() == 16 )
        {
            (void)memcpy( m_pDocId, aUniqueID.getConstArray(), 16 );
            bResult = sal_True;
            lcl_PrintDigest(m_pDigestValue, "digest value");
            lcl_PrintDigest(m_pDocId, "DocId value");
        }
        else
            OSL_FAIL( "Unexpected document ID!\n" );
    }
    else
        OSL_FAIL( "Unexpected key size!\n" );

    return bResult;
}

uno::Sequence< beans::NamedValue > MSCodec_Std97::GetEncryptionData()
{
    ::comphelper::SequenceAsHashMap aHashData;
    aHashData[ ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "STD97EncryptionKey" ) ) ] <<= uno::Sequence< sal_Int8 >( (sal_Int8*)m_pDigestValue, RTL_DIGEST_LENGTH_MD5 );
    aHashData[ ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "STD97UniqueID" ) ) ] <<= uno::Sequence< sal_Int8 >( (sal_Int8*)m_pDocId, 16 );

    return aHashData.getAsConstNamedValueList();
}

void MSCodec_Std97::InitKey (
    const sal_uInt16 pPassData[16],
    const sal_uInt8  pDocId[16])
{
#if DEBUG_MSO_ENCRYPTION_STD97
    fprintf(stdout, "MSCodec_Std97::InitKey: --begin\n");fflush(stdout);
#endif
    uno::Sequence< sal_Int8 > aKey = ::comphelper::DocPasswordHelper::GenerateStd97Key( pPassData, uno::Sequence< sal_Int8 >( (sal_Int8*)pDocId, 16 ) );
    // Fill raw digest of above updates into DigestValue.

    if ( aKey.getLength() == sizeof(m_pDigestValue) )
        (void)memcpy ( m_pDigestValue, aKey.getConstArray(), sizeof(m_pDigestValue) );
    else
        memset( m_pDigestValue, 0, sizeof(m_pDigestValue) );

    lcl_PrintDigest(m_pDigestValue, "digest value");

    (void)memcpy (m_pDocId, pDocId, 16);

    lcl_PrintDigest(m_pDocId, "DocId value");
}

bool MSCodec_Std97::VerifyKey (
    const sal_uInt8 pSaltData[16],
    const sal_uInt8 pSaltDigest[16])
{
    // both the salt data and salt digest (hash) come from the document being imported.

#if DEBUG_MSO_ENCRYPTION_STD97
    fprintf(stdout, "MSCodec_Std97::VerifyKey: \n");
    lcl_PrintDigest(pSaltData, "salt data");
    lcl_PrintDigest(pSaltDigest, "salt hash");
#endif
    bool result = false;

    if (InitCipher(0))
    {
        sal_uInt8 pDigest[RTL_DIGEST_LENGTH_MD5];
        GetDigestFromSalt(pSaltData, pDigest);

        sal_uInt8 pBuffer[16];
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
    sal_uInt8      pKeyData[64]; // 512-bit message block

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
        m_hCipher, rtl_Cipher_DirectionBoth,
        pKeyData, RTL_DIGEST_LENGTH_MD5, 0, 0);

    // Erase KeyData array and leave.
    (void)memset (pKeyData, 0, sizeof(pKeyData));

    return (result == rtl_Cipher_E_None);
}

bool MSCodec_Std97::CreateSaltDigest( const sal_uInt8 nSaltData[16], sal_uInt8 nSaltDigest[16] )
{
#if DEBUG_MSO_ENCRYPTION_STD97
    lcl_PrintDigest(nSaltData, "salt data");
#endif
    bool result = false;

    if (InitCipher(0))
    {
        sal_uInt8 pDigest[RTL_DIGEST_LENGTH_MD5];
        GetDigestFromSalt(nSaltData, pDigest);

        rtl_cipher_decode (
            m_hCipher, pDigest, 16, pDigest, sizeof(pDigest));

        (void)memcpy(nSaltDigest, pDigest, 16);
    }

    return (result);
}

bool MSCodec_Std97::Encode (
    const void *pData,   sal_Size nDatLen,
    sal_uInt8  *pBuffer, sal_Size nBufLen)
{
    rtlCipherError result;

    result = rtl_cipher_encode (
        m_hCipher, pData, nDatLen, pBuffer, nBufLen);

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

void MSCodec_Std97::GetDigestFromSalt( const sal_uInt8 pSaltData[16], sal_uInt8 pDigest[16] )
{
    sal_uInt8 pBuffer[64];
    sal_uInt8 pDigestLocal[16];

    // Decode SaltData into Buffer.
    rtl_cipher_decode (
        m_hCipher, pSaltData, 16, pBuffer, sizeof(pBuffer));

    // set the 129th bit to make the buffer 128-bit in length.
    pBuffer[16] = 0x80;

    // erase the rest of the buffer with zeros.
    (void)memset (pBuffer + 17, 0, sizeof(pBuffer) - 17);

    // set the 441st bit.
    pBuffer[56] = 0x80;

    // Fill raw digest of Buffer into Digest.
    rtl_digest_updateMD5 (
        m_hDigest, pBuffer, sizeof(pBuffer));
    rtl_digest_rawMD5 (
        m_hDigest, pDigestLocal, sizeof(pDigestLocal));

    memcpy(pDigest, pDigestLocal, 16);
}

void MSCodec_Std97::GetEncryptKey (
    const sal_uInt8 pSalt[16],
    sal_uInt8 pSaltData[16],
    sal_uInt8 pSaltDigest[16])
{
    if (InitCipher(0))
    {
        sal_uInt8 pDigest[RTL_DIGEST_LENGTH_MD5];
        sal_uInt8 pBuffer[64];

        rtl_cipher_encode (
            m_hCipher, pSalt, 16, pSaltData, sizeof(pBuffer));

        (void)memcpy( pBuffer, pSalt, 16 );

        pBuffer[16] = 0x80;
        (void)memset (pBuffer + 17, 0, sizeof(pBuffer) - 17);
        pBuffer[56] = 0x80;

        rtl_digest_updateMD5 (
            m_hDigest, pBuffer, sizeof(pBuffer));
        rtl_digest_rawMD5 (
            m_hDigest, pDigest, sizeof(pDigest));

        rtl_cipher_encode (
            m_hCipher, pDigest, 16, pSaltDigest, 16);

        (void)memset (pBuffer, 0, sizeof(pBuffer));
        (void)memset (pDigest, 0, sizeof(pDigest));
    }
}

void MSCodec_Std97::GetDocId( sal_uInt8 pDocId[16] )
{
    if ( sizeof( m_pDocId ) == 16 )
        (void)memcpy( pDocId, m_pDocId, 16 );
}

// ============================================================================

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
