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

std::size_t lclGetLen( const sal_uInt8* pnPassData, std::size_t nBufferSize )
{
    std::size_t nLen = 0;
    while( (nLen < nBufferSize) && pnPassData[ nLen ] ) ++nLen;
    return nLen;
}

sal_uInt16 lclGetKey( const sal_uInt8* pnPassData, std::size_t nBufferSize )
{
    std::size_t nLen = lclGetLen( pnPassData, nBufferSize );
    if( !nLen ) return 0;

    sal_uInt16 nKey = 0;
    sal_uInt16 nKeyBase = 0x8000;
    sal_uInt16 nKeyEnd = 0xFFFF;
    const sal_uInt8* pnChar = pnPassData + nLen - 1;
    for( std::size_t nIndex = 0; nIndex < nLen; ++nIndex, --pnChar )
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

sal_uInt16 lclGetHash( const sal_uInt8* pnPassData, std::size_t nBufferSize )
{
    std::size_t nLen = lclGetLen( pnPassData, nBufferSize );

    sal_uInt16 nHash = static_cast< sal_uInt16 >( nLen );
    if( nLen )
        nHash ^= 0xCE4B;

    const sal_uInt8* pnChar = pnPassData;
    for( std::size_t nIndex = 0; nIndex < nLen; ++nIndex, ++pnChar )
    {
        sal_uInt16 cChar = *pnChar;
        sal_uInt8 nRot = static_cast< sal_uInt8 >( (nIndex + 1) % 15 );
        lclRotateLeft( cChar, nRot, 15 );
        nHash ^= cChar;
    }
    return nHash;
}


} // namespace


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
        0xBF, 0x0F, 0x00, 0x00
    };

    std::size_t nLen = lclGetLen( pnPassData, 16 );
    const sal_uInt8* pnFillChar = spnFillChars;
    for (std::size_t nIndex = nLen; nIndex < sizeof(mpnKey); ++nIndex, ++pnFillChar)
        mpnKey[ nIndex ] = *pnFillChar;

    SVBT16 pnOrigKey;
    ShortToSVBT16( mnKey, pnOrigKey );
    sal_uInt8* pnKeyChar = mpnKey;
    for (std::size_t nIndex = 0; nIndex < sizeof(mpnKey); ++nIndex, ++pnKeyChar)
    {
        *pnKeyChar ^= pnOrigKey[ nIndex & 0x01 ];
        lclRotateLeft( *pnKeyChar, mnRotateDistance );
    }
}

bool MSCodec_Xor95::InitCodec( const uno::Sequence< beans::NamedValue >& aData )
{
    bool bResult = false;

    ::comphelper::SequenceAsHashMap aHashData( aData );
    uno::Sequence< sal_Int8 > aKey = aHashData.getUnpackedValueOrDefault("XOR95EncryptionKey", uno::Sequence< sal_Int8 >() );

    if ( aKey.getLength() == 16 )
    {
        (void)memcpy( mpnKey, aKey.getConstArray(), 16 );
        bResult = true;

        mnKey = (sal_uInt16)aHashData.getUnpackedValueOrDefault("XOR95BaseKey", (sal_Int16)0 );
        mnHash = (sal_uInt16)aHashData.getUnpackedValueOrDefault("XOR95PasswordHash", (sal_Int16)0 );
    }
    else
        OSL_FAIL( "Unexpected key size!\n" );

    return bResult;
}

uno::Sequence< beans::NamedValue > MSCodec_Xor95::GetEncryptionData()
{
    ::comphelper::SequenceAsHashMap aHashData;
    aHashData[ OUString( "XOR95EncryptionKey" ) ] <<= uno::Sequence<sal_Int8>( reinterpret_cast<sal_Int8*>(mpnKey), 16 );
    aHashData[ OUString( "XOR95BaseKey" ) ] <<= (sal_Int16)mnKey;
    aHashData[ OUString( "XOR95PasswordHash" ) ] <<= (sal_Int16)mnHash;

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

void MSCodec_XorXLS95::Decode( sal_uInt8* pnData, std::size_t nBytes )
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

void MSCodec_XorWord95::Decode( sal_uInt8* pnData, std::size_t nBytes )
{
    const sal_uInt8* pnCurrKey = mpnKey + mnOffset;
    const sal_uInt8* pnKeyLast = mpnKey + 0x0F;

    for( const sal_uInt8* pnDataEnd = pnData + nBytes; pnData < pnDataEnd; ++pnData )
    {
        const sal_uInt8 cChar = *pnData ^ *pnCurrKey;
        if (*pnData && cChar)
            *pnData = cChar;

        if( pnCurrKey < pnKeyLast )
            ++pnCurrKey;
        else
            pnCurrKey = mpnKey;
    }

    // update mnOffset
    Skip( nBytes );
}


void MSCodec_Xor95::Skip( std::size_t nBytes )
{
    mnOffset = (mnOffset + nBytes) & 0x0F;
}

MSCodec97::MSCodec97(size_t nHashLen)
    : m_nHashLen(nHashLen)
    , m_hCipher(rtl_cipher_create(rtl_Cipher_AlgorithmARCFOUR, rtl_Cipher_ModeStream))
    , m_aDocId(16, 0)
    , m_aDigestValue(nHashLen, 0)
{
    assert(m_hCipher != nullptr);
}

MSCodec_Std97::MSCodec_Std97()
    : MSCodec97(RTL_DIGEST_LENGTH_MD5)
{
    m_hDigest = rtl_digest_create(rtl_Digest_AlgorithmMD5);
    assert(m_hDigest != nullptr);
}

MSCodec_CryptoAPI::MSCodec_CryptoAPI()
    : MSCodec97(RTL_DIGEST_LENGTH_SHA1)
{
}

MSCodec97::~MSCodec97()
{
    (void)memset(m_aDigestValue.data(), 0, m_aDigestValue.size());
    (void)memset(m_aDocId.data(), 0, m_aDocId.size());
    rtl_cipher_destroy(m_hCipher);
}

MSCodec_Std97::~MSCodec_Std97()
{
    rtl_digest_destroy(m_hDigest);
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
static inline void lcl_PrintDigest(const sal_uInt8* /*pDigest*/, const char* /*msg*/)
{
}
#endif

bool MSCodec97::InitCodec( const uno::Sequence< beans::NamedValue >& aData )
{
#if DEBUG_MSO_ENCRYPTION_STD97
    fprintf(stdout, "MSCodec_Std97::InitCodec: --begin\n");fflush(stdout);
#endif
    bool bResult = false;

    ::comphelper::SequenceAsHashMap aHashData( aData );
    uno::Sequence< sal_Int8 > aKey = aHashData.getUnpackedValueOrDefault("STD97EncryptionKey", uno::Sequence< sal_Int8 >() );
    const size_t nKeyLen = aKey.getLength();
    if (nKeyLen == m_nHashLen)
    {
        assert(m_aDigestValue.size() == m_nHashLen);
        (void)memcpy(m_aDigestValue.data(), aKey.getConstArray(), m_nHashLen);
        uno::Sequence< sal_Int8 > aUniqueID = aHashData.getUnpackedValueOrDefault("STD97UniqueID", uno::Sequence< sal_Int8 >() );
        if ( aUniqueID.getLength() == 16 )
        {
            assert(m_aDocId.size() == static_cast<size_t>(aUniqueID.getLength()));
            (void)memcpy(m_aDocId.data(), aUniqueID.getConstArray(), m_aDocId.size());
            bResult = true;
            lcl_PrintDigest(m_aDigestValue.data(), "digest value");
            lcl_PrintDigest(m_aDocId.data(), "DocId value");
        }
        else
            OSL_FAIL( "Unexpected document ID!\n" );
    }
    else
        OSL_FAIL( "Unexpected key size!\n" );

    return bResult;
}

uno::Sequence< beans::NamedValue > MSCodec97::GetEncryptionData()
{
    ::comphelper::SequenceAsHashMap aHashData;
    assert(m_aDigestValue.size() == m_nHashLen);
    aHashData[ OUString( "STD97EncryptionKey" ) ] <<= uno::Sequence< sal_Int8 >( reinterpret_cast<sal_Int8*>(m_aDigestValue.data()), m_nHashLen );
    aHashData[ OUString( "STD97UniqueID" ) ] <<= uno::Sequence< sal_Int8 >( reinterpret_cast<sal_Int8*>(m_aDocId.data()), m_aDocId.size() );

    return aHashData.getAsConstNamedValueList();
}

void MSCodec_Std97::InitKey (
    const sal_uInt16 pPassData[16],
    const sal_uInt8  pDocId[16])
{
#if DEBUG_MSO_ENCRYPTION_STD97
    fprintf(stdout, "MSCodec_Std97::InitKey: --begin\n");fflush(stdout);
#endif
    uno::Sequence< sal_Int8 > aKey = ::comphelper::DocPasswordHelper::GenerateStd97Key(pPassData, pDocId);
    // Fill raw digest of above updates into DigestValue.

    const size_t nKeyLen = aKey.getLength();
    if (m_aDigestValue.size() == nKeyLen)
        (void)memcpy(m_aDigestValue.data(), aKey.getConstArray(), m_aDigestValue.size());
    else
        memset(m_aDigestValue.data(), 0, m_aDigestValue.size());

    lcl_PrintDigest(m_aDigestValue.data(), "digest value");

    (void)memcpy (m_aDocId.data(), pDocId, 16);

    lcl_PrintDigest(m_aDocId.data(), "DocId value");
}

void MSCodec_CryptoAPI::InitKey (
    const sal_uInt16 pPassData[16],
    const sal_uInt8  pDocId[16])
{
    sal_uInt32 saltSize = 16;

    // Prepare initial data -> salt + password (in 16-bit chars)
    std::vector<sal_uInt8> initialData(pDocId, pDocId + saltSize);

    // Fill PassData into KeyData.
    for (sal_Int32 nInd = 0; nInd < 16 && pPassData[nInd]; ++nInd)
    {
        initialData.push_back(sal::static_int_cast<sal_uInt8>((pPassData[nInd] >> 0) & 0xff));
        initialData.push_back(sal::static_int_cast<sal_uInt8>((pPassData[nInd] >> 8) & 0xff));
    }

    // calculate SHA1 hash of initialData
    rtl_digest_SHA1(initialData.data(), initialData.size(), m_aDigestValue.data(), m_aDigestValue.size());

    lcl_PrintDigest(m_aDigestValue.data(), "digest value");

    (void)memcpy(m_aDocId.data(), pDocId, 16);

    lcl_PrintDigest(m_aDocId.data(), "DocId value");
}

bool MSCodec97::VerifyKey(const sal_uInt8* pSaltData, const sal_uInt8* pSaltDigest)
{
    // both the salt data and salt digest (hash) come from the document being imported.

#if DEBUG_MSO_ENCRYPTION_STD97
    fprintf(stdout, "MSCodec97::VerifyKey: \n");
    lcl_PrintDigest(pSaltData, "salt data");
    lcl_PrintDigest(pSaltDigest, "salt hash");
#endif
    bool result = false;

    if (InitCipher(0))
    {
        std::vector<sal_uInt8> aDigest(m_nHashLen);
        GetDigestFromSalt(pSaltData, aDigest.data());

        std::vector<sal_uInt8> aBuffer(m_nHashLen);
        // Decode original SaltDigest into Buffer.
        rtl_cipher_decode(m_hCipher, pSaltDigest, m_nHashLen, aBuffer.data(), m_nHashLen);

        // Compare Buffer with computed Digest.
        result = (memcmp(aBuffer.data(), aDigest.data(), m_nHashLen) == 0);

        // Erase Buffer and Digest arrays.
        rtl_secureZeroMemory(aBuffer.data(), m_nHashLen);
        rtl_secureZeroMemory(aDigest.data(), m_nHashLen);
    }

    return result;
}

void MSCodec_CryptoAPI::GetDigestFromSalt(const sal_uInt8* pSaltData, sal_uInt8* pDigest)
{
    std::vector<sal_uInt8> verifier(16);
    rtl_cipher_decode(m_hCipher,
        pSaltData, 16, verifier.data(), verifier.size());

    rtl_digest_SHA1(verifier.data(), verifier.size(), pDigest, RTL_DIGEST_LENGTH_SHA1);
}

bool MSCodec_Std97::InitCipher(sal_uInt32 nCounter)
{
    sal_uInt8      pKeyData[64]; // 512-bit message block

    // Initialize KeyData array.
    (void)memset (pKeyData, 0, sizeof(pKeyData));

    // Fill 40 bit of DigestValue into [0..4].
    (void)memcpy (pKeyData, m_aDigestValue.data(), 5);

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
    rtlCipherError result = rtl_cipher_init (
        m_hCipher, rtl_Cipher_DirectionBoth,
        pKeyData, RTL_DIGEST_LENGTH_MD5, nullptr, 0);

    // Erase KeyData array and leave.
    rtl_secureZeroMemory (pKeyData, sizeof(pKeyData));

    return (result == rtl_Cipher_E_None);
}

bool MSCodec_CryptoAPI::InitCipher(sal_uInt32 nCounter)
{
    // data = hash + iterator (4bytes)
    std::vector<sal_uInt8> aKeyData(m_aDigestValue);
    aKeyData.push_back(sal_uInt8((nCounter >>  0) & 0xff));
    aKeyData.push_back(sal_uInt8((nCounter >>  8) & 0xff));
    aKeyData.push_back(sal_uInt8((nCounter >> 16) & 0xff));
    aKeyData.push_back(sal_uInt8((nCounter >> 24) & 0xff));

    std::vector<sal_uInt8> hash(RTL_DIGEST_LENGTH_SHA1);
    rtl_digest_SHA1(aKeyData.data(), aKeyData.size(), hash.data(), RTL_DIGEST_LENGTH_SHA1);

    rtlCipherError result =
        rtl_cipher_init(m_hCipher, rtl_Cipher_DirectionDecode,
                        hash.data(), ENCRYPT_KEY_SIZE_AES_128/8, nullptr, 0);

    return (result == rtl_Cipher_E_None);
}

void MSCodec_Std97::CreateSaltDigest( const sal_uInt8 nSaltData[16], sal_uInt8 nSaltDigest[16] )
{
#if DEBUG_MSO_ENCRYPTION_STD97
    lcl_PrintDigest(nSaltData, "salt data");
#endif
    if (InitCipher(0))
    {
        sal_uInt8 pDigest[RTL_DIGEST_LENGTH_MD5];
        GetDigestFromSalt(nSaltData, pDigest);

        rtl_cipher_decode (
            m_hCipher, pDigest, 16, pDigest, sizeof(pDigest));

        (void)memcpy(nSaltDigest, pDigest, 16);
    }
}

bool MSCodec97::Encode (
    const void *pData,   std::size_t nDatLen,
    sal_uInt8  *pBuffer, std::size_t nBufLen)
{
    rtlCipherError result = rtl_cipher_encode(
        m_hCipher, pData, nDatLen, pBuffer, nBufLen);

    return (result == rtl_Cipher_E_None);
}

bool MSCodec97::Decode (
    const void *pData,   std::size_t nDatLen,
    sal_uInt8  *pBuffer, std::size_t nBufLen)
{
    rtlCipherError result = rtl_cipher_decode(
        m_hCipher, pData, nDatLen, pBuffer, nBufLen);

    return (result == rtl_Cipher_E_None);
}

bool MSCodec97::Skip(std::size_t nDatLen)
{
    sal_uInt8 pnDummy[ 1024 ];
    std::size_t nDatLeft = nDatLen;
    bool bResult = true;

    while (bResult && nDatLeft)
    {
        std::size_t nBlockLen = ::std::min< std::size_t >( nDatLeft, sizeof(pnDummy) );
        bResult = Decode( pnDummy, nBlockLen, pnDummy, nBlockLen );
        nDatLeft -= nBlockLen;
    }

    return bResult;
}

void MSCodec_Std97::GetDigestFromSalt(const sal_uInt8* pSaltData, sal_uInt8* pDigest)
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

        rtl_secureZeroMemory (pBuffer, sizeof(pBuffer));
        rtl_secureZeroMemory (pDigest, sizeof(pDigest));
    }
}

void MSCodec97::GetDocId( sal_uInt8 pDocId[16] )
{
    assert(m_aDocId.size() == 16);
    (void)memcpy(pDocId, m_aDocId.data(), 16);
}

EncryptionStandardHeader::EncryptionStandardHeader()
{
    flags        = 0;
    sizeExtra    = 0;
    algId        = 0;
    algIdHash    = 0;
    keyBits      = 0;
    providedType = 0;
    reserved1    = 0;
    reserved2    = 0;
}

EncryptionVerifierAES::EncryptionVerifierAES()
    : saltSize(SALT_LENGTH)
    , encryptedVerifierHashSize(SHA1_HASH_LENGTH)
{
    memset(salt, 0, sizeof(salt));
    memset(encryptedVerifier, 0, sizeof(encryptedVerifier));
    memset(encryptedVerifierHash, 0, sizeof(encryptedVerifierHash));
}

EncryptionVerifierRC4::EncryptionVerifierRC4()
    : saltSize(SALT_LENGTH)
    , encryptedVerifierHashSize(SHA1_HASH_LENGTH)
{
    memset(salt, 0, sizeof(salt));
    memset(encryptedVerifier, 0, sizeof(encryptedVerifier));
    memset(encryptedVerifierHash, 0, sizeof(encryptedVerifierHash));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
