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

#include "oox/core/binarycodec.hxx"
#include <algorithm>
#include <string.h>
#include "oox/helper/attributelist.hxx"

namespace oox {
namespace core {

// ============================================================================

namespace {

/** Rotates rnValue left by nBits bits. */
template< typename Type >
inline void lclRotateLeft( Type& rnValue, size_t nBits )
{
    OSL_ENSURE( nBits < sizeof( Type ) * 8, "lclRotateLeft - rotation count overflow" );
    rnValue = static_cast< Type >( (rnValue << nBits) | (rnValue >> (sizeof( Type ) * 8 - nBits)) );
}

/** Rotates the lower nWidth bits of rnValue left by nBits bits. */
template< typename Type >
inline void lclRotateLeft( Type& rnValue, size_t nBits, size_t nWidth )
{
    OSL_ENSURE( (nBits < nWidth) && (nWidth < sizeof( Type ) * 8), "lclRotateLeft - rotation count overflow" );
    Type nMask = static_cast< Type >( (1UL << nWidth) - 1 );
    rnValue = static_cast< Type >(
        ((rnValue << nBits) | ((rnValue & nMask) >> (nWidth - nBits))) & nMask );
}

sal_Int32 lclGetLen( const sal_uInt8* pnPassData, sal_Int32 nBufferSize )
{
    sal_Int32 nLen = 0;
    while( (nLen < nBufferSize) && pnPassData[ nLen ] ) ++nLen;
    return nLen;
}

sal_uInt16 lclGetKey( const sal_uInt8* pnPassData, sal_Int32 nBufferSize )
{
    sal_Int32 nLen = lclGetLen( pnPassData, nBufferSize );
    if( nLen <= 0 ) return 0;

    sal_uInt16 nKey = 0;
    sal_uInt16 nKeyBase = 0x8000;
    sal_uInt16 nKeyEnd = 0xFFFF;
    const sal_uInt8* pnChar = pnPassData + nLen - 1;
    for( sal_Int32 nIndex = 0; nIndex < nLen; ++nIndex, --pnChar )
    {
        sal_uInt8 cChar = *pnChar & 0x7F;
        for( size_t nBit = 0; nBit < 8; ++nBit )
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

sal_uInt16 lclGetHash( const sal_uInt8* pnPassData, sal_Int32 nBufferSize )
{
    sal_Int32 nLen = lclGetLen( pnPassData, nBufferSize );

    sal_uInt16 nHash = static_cast< sal_uInt16 >( nLen );
    if( nLen > 0 )
        nHash ^= 0xCE4B;

    const sal_uInt8* pnChar = pnPassData;
    for( sal_Int32 nIndex = 0; nIndex < nLen; ++nIndex, ++pnChar )
    {
        sal_uInt16 cChar = *pnChar;
        size_t nRot = static_cast< size_t >( (nIndex + 1) % 15 );
        lclRotateLeft( cChar, nRot, 15 );
        nHash ^= cChar;
    }
    return nHash;
}

} // namespace

// ============================================================================

/*static*/ sal_uInt16 CodecHelper::getPasswordHash( const AttributeList& rAttribs, sal_Int32 nElement )
{
    sal_Int32 nPasswordHash = rAttribs.getIntegerHex( nElement, 0 );
    OSL_ENSURE( (0 <= nPasswordHash) && (nPasswordHash <= SAL_MAX_UINT16), "CodecHelper::getPasswordHash - invalid password hash" );
    return static_cast< sal_uInt16 >( ((0 <= nPasswordHash) && (nPasswordHash <= SAL_MAX_UINT16)) ? nPasswordHash : 0 );
}

// ============================================================================

BinaryCodec_XOR::BinaryCodec_XOR( CodecType eCodecType ) :
    meCodecType( eCodecType ),
    mnOffset( 0 ),
    mnBaseKey( 0 ),
    mnHash( 0 )
{
    (void)memset( mpnKey, 0, sizeof( mpnKey ) );
}

BinaryCodec_XOR::~BinaryCodec_XOR()
{
    (void)memset( mpnKey, 0, sizeof( mpnKey ) );
    mnBaseKey = mnHash = 0;
}

void BinaryCodec_XOR::initKey( const sal_uInt8 pnPassData[ 16 ] )
{
    // calculate base key and hash from passed password
    mnBaseKey = lclGetKey( pnPassData, 16 );
    mnHash = lclGetHash( pnPassData, 16 );

     static const sal_uInt8 spnFillChars[] =
    {
        0xBB, 0xFF, 0xFF, 0xBA,
        0xFF, 0xFF, 0xB9, 0x80,
        0x00, 0xBE, 0x0F, 0x00,
        0xBF, 0x0F, 0x00
    };

    (void)memcpy( mpnKey, pnPassData, 16 );
    sal_Int32 nIndex;
    sal_Int32 nLen = lclGetLen( pnPassData, 16 );
    const sal_uInt8* pnFillChar = spnFillChars;
    for( nIndex = nLen; nIndex < static_cast< sal_Int32 >( sizeof( mpnKey ) ); ++nIndex, ++pnFillChar )
        mpnKey[ nIndex ] = *pnFillChar;

    // rotation of key values is application dependent
    size_t nRotateSize = 0;
    switch( meCodecType )
    {
        case CODEC_WORD:    nRotateSize = 7;    break;
        case CODEC_EXCEL:   nRotateSize = 2;    break;
        // compiler will warn, if new codec type is introduced and not handled here
    }

    // use little-endian base key to create key array
    sal_uInt8 pnBaseKeyLE[ 2 ];
    pnBaseKeyLE[ 0 ] = static_cast< sal_uInt8 >( mnBaseKey );
    pnBaseKeyLE[ 1 ] = static_cast< sal_uInt8 >( mnBaseKey >> 8 );
    sal_uInt8* pnKeyChar = mpnKey;
    for( nIndex = 0; nIndex < static_cast< sal_Int32 >( sizeof( mpnKey ) ); ++nIndex, ++pnKeyChar )
    {
        *pnKeyChar ^= pnBaseKeyLE[ nIndex & 1 ];
        lclRotateLeft( *pnKeyChar, nRotateSize );
    }
}

bool BinaryCodec_XOR::verifyKey( sal_uInt16 nKey, sal_uInt16 nHash ) const
{
    return (nKey == mnBaseKey) && (nHash == mnHash);
}

void BinaryCodec_XOR::startBlock()
{
    mnOffset = 0;
}

bool BinaryCodec_XOR::decode( sal_uInt8* pnDestData, const sal_uInt8* pnSrcData, sal_Int32 nBytes )
{
    const sal_uInt8* pnCurrKey = mpnKey + mnOffset;
    const sal_uInt8* pnKeyLast = mpnKey + 0x0F;

    // switch/case outside of the for loop (performance)
    const sal_uInt8* pnSrcDataEnd = pnSrcData + nBytes;
    switch( meCodecType )
    {
        case CODEC_WORD:
        {
            for( ; pnSrcData < pnSrcDataEnd; ++pnSrcData, ++pnDestData )
            {
                sal_uInt8 nData = *pnSrcData ^ *pnCurrKey;
                if( (*pnSrcData != 0) && (nData != 0) )
                    *pnDestData = nData;
                if( pnCurrKey < pnKeyLast ) ++pnCurrKey; else pnCurrKey = mpnKey;
            }
        }
        break;
        case CODEC_EXCEL:
        {
            for( ; pnSrcData < pnSrcDataEnd; ++pnSrcData, ++pnDestData )
            {
                *pnDestData = *pnSrcData;
                lclRotateLeft( *pnDestData, 3 );
                *pnDestData ^= *pnCurrKey;
                if( pnCurrKey < pnKeyLast ) ++pnCurrKey; else pnCurrKey = mpnKey;
            }
        }
        break;
        // compiler will warn, if new codec type is introduced and not handled here
    }

    // update offset and leave
    return skip( nBytes );
}

bool BinaryCodec_XOR::skip( sal_Int32 nBytes )
{
    mnOffset = static_cast< sal_Int32 >( (mnOffset + nBytes) & 0x0F );
    return true;
}

sal_uInt16 BinaryCodec_XOR::getHash( const sal_uInt8* pnPassData, sal_Int32 nSize )
{
    return lclGetHash( pnPassData, nSize );
}

// ============================================================================

BinaryCodec_RCF::BinaryCodec_RCF()
{
    mhCipher = rtl_cipher_create( rtl_Cipher_AlgorithmARCFOUR, rtl_Cipher_ModeStream );
    OSL_ENSURE( mhCipher != 0, "BinaryCodec_RCF::BinaryCodec_RCF - cannot create cipher" );

    mhDigest = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    OSL_ENSURE( mhDigest != 0, "BinaryCodec_RCF::BinaryCodec_RCF - cannot create digest" );

    (void)memset( mpnDigestValue, 0, sizeof( mpnDigestValue ) );
}

BinaryCodec_RCF::~BinaryCodec_RCF()
{
    (void)memset( mpnDigestValue, 0, sizeof( mpnDigestValue ) );
    rtl_digest_destroy( mhDigest );
    rtl_cipher_destroy( mhCipher );
}

void BinaryCodec_RCF::initKey( const sal_uInt16 pnPassData[ 16 ], const sal_uInt8 pnSalt[ 16 ] )
{
    // create little-endian key data array from password data
    sal_uInt8 pnKeyData[ 64 ];
    (void)memset( pnKeyData, 0, sizeof( pnKeyData ) );

    const sal_uInt16* pnCurrPass = pnPassData;
    const sal_uInt16* pnPassEnd = pnPassData + 16;
    sal_uInt8* pnCurrKey = pnKeyData;
    size_t nPassSize = 0;
    for( ; (pnCurrPass < pnPassEnd) && (*pnCurrPass != 0); ++pnCurrPass, ++nPassSize )
    {
        *pnCurrKey++ = static_cast< sal_uInt8 >( *pnCurrPass );
        *pnCurrKey++ = static_cast< sal_uInt8 >( *pnCurrPass >> 8 );
    }
    pnKeyData[ 2 * nPassSize ] = 0x80;
    pnKeyData[ 56 ] = static_cast< sal_uInt8 >( nPassSize << 4 );

    // fill raw digest of key data into key data
    (void)rtl_digest_updateMD5( mhDigest, pnKeyData, sizeof( pnKeyData ) );
    (void)rtl_digest_rawMD5( mhDigest, pnKeyData, RTL_DIGEST_LENGTH_MD5 );

    // update digest with key data and passed salt data
    for( size_t nIndex = 0; nIndex < 16; ++nIndex )
    {
        rtl_digest_updateMD5( mhDigest, pnKeyData, 5 );
        rtl_digest_updateMD5( mhDigest, pnSalt, 16 );
    }

    // update digest with padding
    pnKeyData[ 16 ] = 0x80;
    (void)memset( pnKeyData + 17, 0, sizeof( pnKeyData ) - 17 );
    pnKeyData[ 56 ] = 0x80;
    pnKeyData[ 57 ] = 0x0A;
    rtl_digest_updateMD5( mhDigest, pnKeyData + 16, sizeof( pnKeyData ) - 16 );

    // fill raw digest of above updates into digest value
    rtl_digest_rawMD5( mhDigest, mpnDigestValue, sizeof( mpnDigestValue ) );

    // erase key data array and leave
    (void)memset( pnKeyData, 0, sizeof( pnKeyData ) );
}

bool BinaryCodec_RCF::verifyKey( const sal_uInt8 pnVerifier[ 16 ], const sal_uInt8 pnVerifierHash[ 16 ] )
{
    if( !startBlock( 0 ) )
        return false;

    sal_uInt8 pnDigest[ RTL_DIGEST_LENGTH_MD5 ];
    sal_uInt8 pnBuffer[ 64 ];

    // decode salt data into buffer
    rtl_cipher_decode( mhCipher, pnVerifier, 16, pnBuffer, sizeof( pnBuffer ) );

    pnBuffer[ 16 ] = 0x80;
    (void)memset( pnBuffer + 17, 0, sizeof( pnBuffer ) - 17 );
    pnBuffer[ 56 ] = 0x80;

    // fill raw digest of buffer into digest
    rtl_digest_updateMD5( mhDigest, pnBuffer, sizeof( pnBuffer ) );
    rtl_digest_rawMD5( mhDigest, pnDigest, sizeof( pnDigest ) );

    // decode original salt digest into buffer
    rtl_cipher_decode( mhCipher, pnVerifierHash, 16, pnBuffer, sizeof( pnBuffer ) );

    // compare buffer with computed digest
    bool bResult = memcmp( pnBuffer, pnDigest, sizeof( pnDigest ) ) == 0;

    // erase buffer and digest arrays and leave
    (void)memset( pnBuffer, 0, sizeof( pnBuffer ) );
    (void)memset( pnDigest, 0, sizeof( pnDigest ) );
    return bResult;
}

bool BinaryCodec_RCF::startBlock( sal_Int32 nCounter )
{
    // initialize key data array
    sal_uInt8 pnKeyData[ 64 ];
    (void)memset( pnKeyData, 0, sizeof( pnKeyData ) );

    // fill 40 bit of digest value into [0..4]
    (void)memcpy( pnKeyData, mpnDigestValue, 5 );

    // fill little-endian counter into [5..8], static_cast masks out unneeded bits
    pnKeyData[ 5 ] = static_cast< sal_uInt8 >( nCounter );
    pnKeyData[ 6 ] = static_cast< sal_uInt8 >( nCounter >> 8 );
    pnKeyData[ 7 ] = static_cast< sal_uInt8 >( nCounter >> 16 );
    pnKeyData[ 8 ] = static_cast< sal_uInt8 >( nCounter >> 24 );

    pnKeyData[ 9 ] = 0x80;
    pnKeyData[ 56 ] = 0x48;

    // fill raw digest of key data into key data
    (void)rtl_digest_updateMD5( mhDigest, pnKeyData, sizeof( pnKeyData ) );
    (void)rtl_digest_rawMD5( mhDigest, pnKeyData, RTL_DIGEST_LENGTH_MD5 );

    // initialize cipher with key data (for decoding)
    rtlCipherError eResult =
        rtl_cipher_init( mhCipher, rtl_Cipher_DirectionDecode, pnKeyData, RTL_DIGEST_LENGTH_MD5, 0, 0 );

    // rrase key data array and leave
    (void)memset( pnKeyData, 0, sizeof( pnKeyData ) );
    return eResult == rtl_Cipher_E_None;
}

bool BinaryCodec_RCF::decode( sal_uInt8* pnDestData, const sal_uInt8* pnSrcData, sal_Int32 nBytes )
{
    rtlCipherError eResult = rtl_cipher_decode( mhCipher,
        pnSrcData, static_cast< sal_Size >( nBytes ),
        pnDestData, static_cast< sal_Size >( nBytes ) );
    return eResult == rtl_Cipher_E_None;
}

bool BinaryCodec_RCF::skip( sal_Int32 nBytes )
{
    // decode dummy data in memory to update internal state of RC4 cipher
    sal_uInt8 pnDummy[ 1024 ];
    sal_Int32 nBytesLeft = nBytes;
    bool bResult = true;
    while( bResult && (nBytesLeft > 0) )
    {
        sal_Int32 nBlockLen = ::std::min( nBytesLeft, static_cast< sal_Int32 >( sizeof( pnDummy ) ) );
        bResult = decode( pnDummy, pnDummy, nBlockLen );
        nBytesLeft -= nBlockLen;
    }
    return bResult;
}

// ============================================================================

} // namespace core
} // namespace oox

