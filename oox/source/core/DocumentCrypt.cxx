/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "oox/core/DocumentCrypt.hxx"
#include <config_oox.h>

#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#if USE_TLS_OPENSSL
#include <openssl/evp.h>
#endif // USE_TLS_OPENSSL
#if USE_TLS_NSS
#include <nss.h>
#include <pk11pub.h>
#endif // USE_TLS_NSS
#include <rtl/digest.h>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"

#include <osl/time.h>
#include <rtl/random.h>

#include <com/sun/star/io/XSeekable.hpp>

namespace oox {
namespace core {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::comphelper::MediaDescriptor;
using ::comphelper::SequenceAsHashMap;

/* =========================================================================== */
/*  Kudos to Caolan McNamara who provided the core decryption implementations. */
/* =========================================================================== */

namespace {

const sal_uInt32 ENCRYPTINFO_CRYPTOAPI      = 0x00000004;
const sal_uInt32 ENCRYPTINFO_DOCPROPS       = 0x00000008;
const sal_uInt32 ENCRYPTINFO_EXTERNAL       = 0x00000010;
const sal_uInt32 ENCRYPTINFO_AES            = 0x00000020;

const sal_uInt32 ENCRYPT_ALGO_AES128        = 0x0000660E;
const sal_uInt32 ENCRYPT_ALGO_AES192        = 0x0000660F;
const sal_uInt32 ENCRYPT_ALGO_AES256        = 0x00006610;
const sal_uInt32 ENCRYPT_ALGO_RC4           = 0x00006801;

const sal_uInt32 ENCRYPT_HASH_SHA1          = 0x00008004;

const sal_uInt32 ENCRYPT_KEY_SIZE_AES_128   = 0x00000080;
const sal_uInt32 ENCRYPT_KEY_SIZE_AES_192   = 0x000000C0;
const sal_uInt32 ENCRYPT_KEY_SIZE_AES_256   = 0x00000100;

const sal_uInt32 ENCRYPT_PROVIDER_TYPE_AES  = 0x00000018;
const sal_uInt32 ENCRYPT_PROVIDER_TYPE_RC4  = 0x00000001;

struct PackageEncryptionInfo
{
    sal_uInt8           mpnSalt[ 16 ];
    sal_uInt8           mpnEncrVerifier[ 16 ];
    sal_uInt8           mpnEncrVerifierHash[ 32 ];
    sal_uInt32          mnFlags;
    sal_uInt32          mnAlgorithmId;
    sal_uInt32          mnAlgorithmIdHash;
    sal_uInt32          mnKeySize;
    sal_uInt32          mnSaltSize;
    sal_uInt32          mnVerifierHashSize;
};

void lclRandomGenerateValues( sal_Int32 nLength, sal_uInt8* aArray )
{
    TimeValue aTime;
    osl_getSystemTime( &aTime );
    rtlRandomPool aRandomPool = rtl_random_createPool ();
    rtl_random_addBytes ( aRandomPool, &aTime, 8 );
    rtl_random_getBytes ( aRandomPool, aArray, nLength );
    rtl_random_destroyPool ( aRandomPool );
}

bool lclReadEncryptionInfo( PackageEncryptionInfo& rEncrInfo, BinaryInputStream& rStrm )
{
    rStrm.skip( 4 );
    rStrm >> rEncrInfo.mnFlags;
    if( getFlag( rEncrInfo.mnFlags, ENCRYPTINFO_EXTERNAL ) )
        return false;

    sal_uInt32 nHeaderSize, nRepeatedFlags;
    rStrm >> nHeaderSize >> nRepeatedFlags;
    if( (nHeaderSize < 20) || (nRepeatedFlags != rEncrInfo.mnFlags) )
        return false;

    rStrm.skip( 4 );
    rStrm >> rEncrInfo.mnAlgorithmId >> rEncrInfo.mnAlgorithmIdHash >> rEncrInfo.mnKeySize;
    rStrm.skip( nHeaderSize - 20 );
    rStrm >> rEncrInfo.mnSaltSize;
    if( rEncrInfo.mnSaltSize != 16 )
        return false;

    rStrm.readMemory( rEncrInfo.mpnSalt, 16 );
    rStrm.readMemory( rEncrInfo.mpnEncrVerifier, 16 );
    rStrm >> rEncrInfo.mnVerifierHashSize;
    rStrm.readMemory( rEncrInfo.mpnEncrVerifierHash, 32 );
    return !rStrm.isEof();
}

struct EncryptionStandardHeader {
    sal_uInt32 flags;
    sal_uInt32 sizeExtra;
    sal_uInt32 algId;         // if flag AES && CRYPTOAPI this defaults to 128-bit AES
    sal_uInt32 algIdHash;     // 0 - determined by flags - defaults to SHA-1 if not external
    sal_uInt32 keySize;       // 0 - determined by flags, 128, 192, 256 for AES
    sal_uInt32 providedType;
    sal_uInt32 reserved1;
    sal_uInt32 reserved2;
};

struct EncryptionVerifierAES {
    sal_uInt32 saltSize; // must be 0x00000010
    sal_uInt8  salt[16]; //
    sal_uInt8  encryptedVerifier[16];     // randomly generated verifier value
    sal_uInt32 verifierHashSize;
    sal_uInt8  encryptedVerifierHash[32];
};

bool lclWriteEncryptionInfo( PackageEncryptionInfo& rEncrInfo, BinaryOutputStream& rStream )
{
    const sal_uInt16 versionInfoMajor = 0x003;
    const sal_uInt16 versionInfoMinor = 0x002;

    rStream.writeValue(versionInfoMajor);
    rStream.writeValue(versionInfoMinor);

    const OUString cspName = "Microsoft Enhanced RSA and AES Cryptographic Provider";
    sal_Int32      cspNameSize = (cspName.getLength() * 2) + 2;

    EncryptionStandardHeader encryptionHeader;
    sal_Int32 encryptionHeaderSize = static_cast<sal_Int32>(sizeof(EncryptionStandardHeader));
    memset(&encryptionHeader, 0, encryptionHeaderSize);

    EncryptionVerifierAES encryptionVerifier;
    sal_Int32 encryptionVerifierSize = static_cast<sal_Int32>(sizeof(EncryptionVerifierAES));
    memset(&encryptionVerifier, 0, encryptionVerifierSize);

    rStream << rEncrInfo.mnFlags;

    sal_uInt32 headerSize = encryptionHeaderSize + cspNameSize;
    rStream << headerSize;

    encryptionHeader.flags = rEncrInfo.mnFlags;
    encryptionHeader.algId = rEncrInfo.mnAlgorithmId;
    encryptionHeader.algIdHash = rEncrInfo.mnAlgorithmIdHash;
    encryptionHeader.keySize = rEncrInfo.mnKeySize;
    encryptionHeader.providedType = ENCRYPT_PROVIDER_TYPE_AES;

    rStream.writeMemory(&encryptionHeader, encryptionHeaderSize);
    rStream.writeUnicodeArray(cspName);
    rStream.writeValue<sal_uInt16>(0);

    if (rEncrInfo.mnSaltSize != 16)
        return false;

    encryptionVerifier.saltSize = rEncrInfo.mnSaltSize;

    memcpy(&encryptionVerifier.salt, rEncrInfo.mpnSalt, 16);

    memcpy(&encryptionVerifier.encryptedVerifier, rEncrInfo.mpnEncrVerifier, 16);

    encryptionVerifier.verifierHashSize = rEncrInfo.mnVerifierHashSize;

    memcpy(encryptionVerifier.encryptedVerifierHash, rEncrInfo.mpnEncrVerifierHash, 32);

    rStream.writeMemory(&encryptionVerifier, encryptionVerifierSize);

    return true;
}

void lclDeriveKey( const sal_uInt8* pnHash, sal_uInt32 nHashLen, sal_uInt8* pnKeyDerived, sal_uInt32 nRequiredKeyLen )
{
    sal_uInt8 pnBuffer[ 64 ];
    memset( pnBuffer, 0x36, sizeof( pnBuffer ) );
    for( sal_uInt32 i = 0; i < nHashLen; ++i )
        pnBuffer[ i ] ^= pnHash[ i ];

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, pnBuffer, sizeof( pnBuffer ) );
    sal_uInt8 pnX1[ RTL_DIGEST_LENGTH_SHA1 ];
    rtl_digest_get( aDigest, pnX1, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    memset( pnBuffer, 0x5C, sizeof( pnBuffer ) );
    for( sal_uInt32 i = 0; i < nHashLen; ++i )
        pnBuffer[ i ] ^= pnHash[ i ];

    aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, pnBuffer, sizeof( pnBuffer ) );
    sal_uInt8 pnX2[ RTL_DIGEST_LENGTH_SHA1 ];
    rtl_digest_get( aDigest, pnX2, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    if( nRequiredKeyLen > RTL_DIGEST_LENGTH_SHA1 )
    {
        memcpy( pnKeyDerived + RTL_DIGEST_LENGTH_SHA1, pnX2, nRequiredKeyLen - RTL_DIGEST_LENGTH_SHA1 );
        nRequiredKeyLen = RTL_DIGEST_LENGTH_SHA1;
    }
    memcpy( pnKeyDerived, pnX1, nRequiredKeyLen );
}

bool lclGenerateVerifier(PackageEncryptionInfo& rEncryptionInfo, const sal_uInt8* pKey, sal_uInt32 nKeySize)
{
    bool bResult = false;

    if (nKeySize != 16)
        return bResult;

    sal_uInt8 aVerifier[16];
    sal_Int32 aVerifierSize = sizeof(aVerifier);
    memset( aVerifier, 0, aVerifierSize );
    lclRandomGenerateValues(aVerifierSize, aVerifier);

#if USE_TLS_OPENSSL
    {
        EVP_CIPHER_CTX aContext;
        EVP_CIPHER_CTX_init( &aContext );
        EVP_EncryptInit_ex( &aContext, EVP_aes_128_ecb(), NULL, pKey, 0 );
        EVP_CIPHER_CTX_set_padding( &aContext, 0 );
        int aEncryptedVerifierSize = 0;
        EVP_EncryptUpdate( &aContext, rEncryptionInfo.mpnEncrVerifier, &aEncryptedVerifierSize, aVerifier, aVerifierSize );
        EVP_CIPHER_CTX_cleanup( &aContext );
    }

#endif // USE_TLS_OPENSSL

    sal_uInt8 pSha1Hash[ 32 ];
    memset(pSha1Hash, 0, 32);
    rEncryptionInfo.mnVerifierHashSize = RTL_DIGEST_LENGTH_SHA1;

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, aVerifier, aVerifierSize );
    rtl_digest_get( aDigest, pSha1Hash, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

#if USE_TLS_OPENSSL
    {
        memset(rEncryptionInfo.mpnEncrVerifierHash, 0, rEncryptionInfo.mnVerifierHashSize);
        int written = 0;

        EVP_CIPHER_CTX aContext;
        EVP_CIPHER_CTX_init( &aContext );
        EVP_EncryptInit_ex( &aContext, EVP_aes_128_ecb(), NULL, pKey, 0 );
        EVP_CIPHER_CTX_set_padding( &aContext, 0 );
        EVP_EncryptUpdate( &aContext, rEncryptionInfo.mpnEncrVerifierHash, &written, pSha1Hash, 32 );
        EVP_CIPHER_CTX_cleanup( &aContext );
    }

#endif // USE_TLS_OPENSSL

    bResult = true;

    return bResult;
}


bool lclCheckEncryptionData( const sal_uInt8* pnKey, sal_uInt32 nKeySize, const sal_uInt8* pnVerifier, sal_uInt32 nVerifierSize, const sal_uInt8* pnVerifierHash, sal_uInt32 nVerifierHashSize )
{
    bool bResult = false;

    // the only currently supported algorithm needs key size 128
    if ( nKeySize == 16 && nVerifierSize == 16 )
    {
        // check password
#if USE_TLS_OPENSSL
        EVP_CIPHER_CTX aes_ctx;
        EVP_CIPHER_CTX_init( &aes_ctx );
        EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, pnKey, 0 );
        EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );
        int nOutLen = 0;
        sal_uInt8 pnTmpVerifier[ 16 ];
        (void) memset( pnTmpVerifier, 0, sizeof(pnTmpVerifier) );

        /*int*/ EVP_DecryptUpdate( &aes_ctx, pnTmpVerifier, &nOutLen, pnVerifier, nVerifierSize );
        EVP_CIPHER_CTX_cleanup( &aes_ctx );

        EVP_CIPHER_CTX_init( &aes_ctx );
        EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, pnKey, 0 );
        EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );
        sal_uInt8* pnTmpVerifierHash = new sal_uInt8[nVerifierHashSize];
        (void) memset( pnTmpVerifierHash, 0, nVerifierHashSize );

        /*int*/ EVP_DecryptUpdate( &aes_ctx, pnTmpVerifierHash, &nOutLen, pnVerifierHash, nVerifierHashSize );
        EVP_CIPHER_CTX_cleanup( &aes_ctx );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
        PK11SlotInfo *aSlot( PK11_GetBestSlot( CKM_AES_ECB, NULL ) );
        sal_uInt8 *key( new sal_uInt8[ nKeySize ] );
        (void) memcpy( key, pnKey, nKeySize * sizeof(sal_uInt8) );

        SECItem keyItem;
        keyItem.type = siBuffer;
        keyItem.data = key;
        keyItem.len  = nKeySize;

        PK11SymKey *symKey( PK11_ImportSymKey( aSlot, CKM_AES_ECB, PK11_OriginUnwrap, CKA_ENCRYPT, &keyItem, NULL ) );
        SECItem *secParam( PK11_ParamFromIV( CKM_AES_ECB, NULL ) );
        PK11Context *encContext( PK11_CreateContextBySymKey( CKM_AES_ECB, CKA_DECRYPT, symKey, secParam ) );

        int nOutLen(0);
        sal_uInt8 pnTmpVerifier[ 16 ];
        (void) memset( pnTmpVerifier, 0, sizeof(pnTmpVerifier) );

        PK11_CipherOp( encContext, pnTmpVerifier, &nOutLen, sizeof(pnTmpVerifier), const_cast<sal_uInt8*>(pnVerifier), nVerifierSize );

        sal_uInt8* pnTmpVerifierHash = new sal_uInt8[nVerifierHashSize];
        (void) memset( pnTmpVerifierHash, 0, nVerifierHashSize );
        PK11_CipherOp( encContext, pnTmpVerifierHash, &nOutLen, nVerifierHashSize, const_cast<sal_uInt8*>(pnVerifierHash), nVerifierHashSize );

        PK11_DestroyContext( encContext, PR_TRUE );
        PK11_FreeSymKey( symKey );
        SECITEM_FreeItem( secParam, PR_TRUE );
        delete[] key;
#endif // USE_TLS_NSS

        rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
        rtl_digest_update( aDigest, pnTmpVerifier, sizeof( pnTmpVerifier ) );
        sal_uInt8 pnSha1Hash[ RTL_DIGEST_LENGTH_SHA1 ];
        rtl_digest_get( aDigest, pnSha1Hash, RTL_DIGEST_LENGTH_SHA1 );
        rtl_digest_destroy( aDigest );

        bResult = ( memcmp( pnSha1Hash, pnTmpVerifierHash, RTL_DIGEST_LENGTH_SHA1 ) == 0 );
    }

    return bResult;
}

// ----------------------------------------------------------------------------

Sequence< NamedValue > lclGenerateEncryptionKey( const PackageEncryptionInfo& rEncrInfo, const OUString& rPassword, sal_uInt8* pnKey, sal_uInt32 nRequiredKeyLen )
{
    size_t nBufferSize = rEncrInfo.mnSaltSize + 2 * rPassword.getLength();
    sal_uInt8* pnBuffer = new sal_uInt8[ nBufferSize ];
    memcpy( pnBuffer, rEncrInfo.mpnSalt, rEncrInfo.mnSaltSize );

    sal_uInt8* pnPasswordLoc = pnBuffer + rEncrInfo.mnSaltSize;
    const sal_Unicode* pStr = rPassword.getStr();
    for( sal_Int32 i = 0, nLen = rPassword.getLength(); i < nLen; ++i, ++pStr, pnPasswordLoc += 2 )
        ByteOrderConverter::writeLittleEndian( pnPasswordLoc, static_cast< sal_uInt16 >( *pStr ) );

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, pnBuffer, nBufferSize );
    delete[] pnBuffer;

    size_t nHashSize = RTL_DIGEST_LENGTH_SHA1 + 4;
    sal_uInt8* pnHash = new sal_uInt8[ nHashSize ];
    rtl_digest_get( aDigest, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    for( sal_uInt32 i = 0; i < 50000; ++i )
    {
        ByteOrderConverter::writeLittleEndian( pnHash, i );
        aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
        rtl_digest_update( aDigest, pnHash, nHashSize );
        rtl_digest_get( aDigest, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
        rtl_digest_destroy( aDigest );
    }

    memmove( pnHash, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
    memset( pnHash + RTL_DIGEST_LENGTH_SHA1, 0, 4 );
    aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, pnHash, nHashSize );
    rtl_digest_get( aDigest, pnHash, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    lclDeriveKey( pnHash, RTL_DIGEST_LENGTH_SHA1, pnKey, nRequiredKeyLen );
    delete[] pnHash;

    Sequence< NamedValue > aResult;
    if( lclCheckEncryptionData( pnKey, nRequiredKeyLen, rEncrInfo.mpnEncrVerifier, sizeof( rEncrInfo.mpnEncrVerifier ), rEncrInfo.mpnEncrVerifierHash, sizeof( rEncrInfo.mpnEncrVerifierHash ) ) )
    {
        SequenceAsHashMap aEncryptionData;
        aEncryptionData[ "AES128EncryptionKey" ] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( pnKey ), nRequiredKeyLen );
        aEncryptionData[ "AES128EncryptionSalt" ] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( rEncrInfo.mpnSalt ), rEncrInfo.mnSaltSize );
        aEncryptionData[ "AES128EncryptionVerifier" ] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( rEncrInfo.mpnEncrVerifier ), sizeof( rEncrInfo.mpnEncrVerifier ) );
        aEncryptionData[ "AES128EncryptionVerifierHash" ] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( rEncrInfo.mpnEncrVerifierHash ), sizeof( rEncrInfo.mpnEncrVerifierHash ) );
        aResult = aEncryptionData.getAsConstNamedValueList();
    }

    return aResult;
}

} // namespace

AesEncoder::AesEncoder(Reference< XStream > xDocumentStream, oox::ole::OleStorage& rOleStorage, OUString aPassword) :
    mxDocumentStream(xDocumentStream),
    mrOleStorage(rOleStorage),
    maPassword(aPassword)
{
}

bool AesEncoder::encode()
{
    Reference< XInputStream > xInputStream ( mxDocumentStream->getInputStream(), UNO_SET_THROW );
    Reference< XSeekable > xSeekable( xInputStream, UNO_QUERY );

    if (!xSeekable.is())
        return false;

    sal_uInt32 aLength = xSeekable->getLength();
    printf("%d\n", aLength);

    if (!mrOleStorage.isStorage())
        return false;

    Reference< XOutputStream > xEncryptionInfo( mrOleStorage.openOutputStream( "EncryptionInfo" ), UNO_SET_THROW );

    PackageEncryptionInfo rEncrInfo;
    rEncrInfo.mnFlags = ENCRYPTINFO_AES | ENCRYPTINFO_CRYPTOAPI;
    rEncrInfo.mnAlgorithmId = ENCRYPT_ALGO_AES128;
    rEncrInfo.mnAlgorithmIdHash = ENCRYPT_HASH_SHA1;
    rEncrInfo.mnKeySize = ENCRYPT_KEY_SIZE_AES_128;

    rEncrInfo.mnSaltSize = 16;

    lclRandomGenerateValues( rEncrInfo.mnSaltSize, rEncrInfo.mpnSalt );

    sal_Int32 keyLength = rEncrInfo.mnKeySize / 8;
    sal_uInt8 key[16];
    memset(key, 0, keyLength);

    lclGenerateEncryptionKey(rEncrInfo, maPassword, key, keyLength);

    lclGenerateVerifier(rEncrInfo, key, keyLength);

    bool aResult = lclCheckEncryptionData(key, keyLength, rEncrInfo.mpnEncrVerifier, 16, rEncrInfo.mpnEncrVerifierHash, 32);
    if (!aResult)
        return false;

    BinaryXOutputStream aEncryptionInfoBinaryOutputStream( xEncryptionInfo, false );
    lclWriteEncryptionInfo( rEncrInfo, aEncryptionInfoBinaryOutputStream );
    aEncryptionInfoBinaryOutputStream.close();

    xEncryptionInfo->flush();
    xEncryptionInfo->closeOutput();

    Reference< XOutputStream > xEncryptedPackage( mrOleStorage.openOutputStream( "EncryptedPackage" ), UNO_SET_THROW );
    BinaryXOutputStream aEncryptedPackageStream( xEncryptedPackage, false );

    BinaryXInputStream aDocumentInputStream( xInputStream, false );
    aDocumentInputStream.seekToStart();

#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX aContext;
    EVP_CIPHER_CTX_init( &aContext );
    EVP_EncryptInit_ex( &aContext, EVP_aes_128_ecb(), NULL, key, 0 );
    EVP_CIPHER_CTX_set_padding( &aContext, 0 );

    sal_uInt8 inBuffer[ 1024 ];
    sal_uInt8 outBuffer[ 1024 ];

    sal_Int32 inLength;
    int outLength;

    aEncryptedPackageStream.writeValue<sal_uInt32>( aLength ); // size
    aEncryptedPackageStream.writeValue<sal_uInt32>( 0 ); // size

    do
    {
        inLength = aDocumentInputStream.readMemory( inBuffer, sizeof( inBuffer ) );
        if (inLength > 0)
        {
            inLength = inLength % 16 == 0 ? inLength : ((inLength/16)*16)+16;
            EVP_EncryptUpdate( &aContext, outBuffer, &outLength, inBuffer, inLength );
            aEncryptedPackageStream.writeMemory( outBuffer, outLength );
        }
    }
    while (inLength > 0);

    EVP_CIPHER_CTX_cleanup( &aContext );

#endif // USE_TLS_OPENSSL

    aEncryptedPackageStream.seekToStart();
    aEncryptedPackageStream.close();

    aDocumentInputStream.seekToStart();
    aDocumentInputStream.close();

    xEncryptedPackage->flush();
    xEncryptedPackage->closeOutput();

    return true;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
