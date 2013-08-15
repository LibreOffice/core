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

#if USE_TLS_OPENSSL
#include <openssl/evp.h>
#endif // USE_TLS_OPENSSL
#if USE_TLS_NSS
#include <nss.h>
#include <pk11pub.h>
#endif // USE_TLS_NSS
#include <rtl/digest.h>

#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/mediadescriptor.hxx>

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

using namespace std;

/* =========================================================================== */
/*  Kudos to Caolan McNamara who provided the core decryption implementations. */
/* =========================================================================== */

namespace {

void lclRandomGenerateValues( sal_Int32 nLength, sal_uInt8* aArray )
{
    TimeValue aTime;
    osl_getSystemTime( &aTime );
    rtlRandomPool aRandomPool = rtl_random_createPool ();
    rtl_random_addBytes ( aRandomPool, &aTime, 8 );
    rtl_random_getBytes ( aRandomPool, aArray, nLength );
    rtl_random_destroyPool ( aRandomPool );
}

void lclDeriveKey( const sal_uInt8* pnHash, sal_uInt32 nHashLen, vector<sal_uInt8>& rKey, sal_uInt32 aKeyLength )
{
    // De facto we are always called with nRequiredKeyLen == 16, at least currently
    assert(aKeyLength == 16);

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

#if 0 // for now nRequiredKeyLen will always be 16 and thus less than
      // RTL_DIGEST_LENGTH_SHA1==20, see assert above...
    if( aKeyLength > RTL_DIGEST_LENGTH_SHA1 )
    {
        // This memcpy call generates a (bogus?) warning when
        // compiling with gcc 4.7 and 4.8 and optimising: array
        // subscript is above array bounds.
        std::copy(pnX1, pnX1 + aKeyLength - RTL_DIGEST_LENGTH_SHA1, rKey.begin() + RTL_DIGEST_LENGTH_SHA1);
        aKeyLength = RTL_DIGEST_LENGTH_SHA1;
    }
#endif
    std::copy(pnX1, pnX1 + aKeyLength, rKey.begin());
    //memcpy( pnKeyDerived, pnX1, nRequiredKeyLen );
}

bool lclGenerateVerifier(PackageEncryptionInfo& rEncryptionInfo, const vector<sal_uInt8>& rKey, sal_uInt32 nKeySize)
{
    // only support key of size 128 bit (16 byte)
    if (nKeySize != 16)
        return false;

    sal_uInt8 aVerifier[ENCRYPTED_VERIFIER_LENGTH];
    sal_Int32 aVerifierSize = sizeof(aVerifier);
    lclRandomGenerateValues(aVerifierSize, aVerifier);

#if USE_TLS_OPENSSL
    {
        EVP_CIPHER_CTX aContext;
        EVP_CIPHER_CTX_init( &aContext );
        EVP_EncryptInit_ex( &aContext, EVP_aes_128_ecb(), NULL, &rKey[0], 0 );
        EVP_CIPHER_CTX_set_padding( &aContext, 0 );
        int aWrittenLength = 0;
        EVP_EncryptUpdate( &aContext, rEncryptionInfo.verifier.encryptedVerifier, &aWrittenLength, aVerifier, aVerifierSize );
        if (aWrittenLength != ENCRYPTED_VERIFIER_LENGTH)
            return false;
        EVP_CIPHER_CTX_cleanup( &aContext );
    }

#endif // USE_TLS_OPENSSL

    sal_uInt8 pSha1Hash[ENCRYPTED_VERIFIER_HASH_LENGTH];
    memset(pSha1Hash, 0, sizeof(pSha1Hash));
    rEncryptionInfo.verifier.encryptedVerifierHashSize = RTL_DIGEST_LENGTH_SHA1;

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, aVerifier, aVerifierSize );
    rtl_digest_get( aDigest, pSha1Hash, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

#if USE_TLS_OPENSSL
    {
        int aWrittenLength = 0;

        EVP_CIPHER_CTX aContext;
        EVP_CIPHER_CTX_init( &aContext );
        EVP_EncryptInit_ex( &aContext, EVP_aes_128_ecb(), NULL, &rKey[0], 0 );
        EVP_CIPHER_CTX_set_padding( &aContext, 0 );
        EVP_EncryptUpdate( &aContext, rEncryptionInfo.verifier.encryptedVerifierHash, &aWrittenLength, pSha1Hash, sizeof(pSha1Hash) );
        EVP_CIPHER_CTX_cleanup( &aContext );
    }
#endif // USE_TLS_OPENSSL

    return true;
}

bool lclCheckEncryptionData( const sal_uInt8* pnKey, sal_uInt32 nKeySize, const sal_uInt8* pnVerifier, sal_uInt32 nVerifierSize, const sal_uInt8* pnVerifierHash, sal_uInt32 nVerifierHashSize )
{
    // the only currently supported algorithm needs key size 128
    if ( nKeySize != 16 || nVerifierSize != 16 )
        return false;

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

    return memcmp( pnSha1Hash, pnTmpVerifierHash, RTL_DIGEST_LENGTH_SHA1 ) == 0;
}

bool lclGenerateEncryptionKey( const PackageEncryptionInfo& rEncryptionInfo, const OUString& rPassword, vector<sal_uInt8>& aKey, sal_uInt32 aKeyLength )
{
    size_t nBufferSize = rEncryptionInfo.verifier.saltSize + 2 * rPassword.getLength();
    sal_uInt8* pnBuffer = new sal_uInt8[ nBufferSize ];
    memcpy( pnBuffer, rEncryptionInfo.verifier.salt, rEncryptionInfo.verifier.saltSize );

    sal_uInt8* pnPasswordLoc = pnBuffer + rEncryptionInfo.verifier.saltSize;
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

    lclDeriveKey( pnHash, RTL_DIGEST_LENGTH_SHA1, aKey, aKeyLength );
    delete[] pnHash;
    return true;
}

} // namespace

EncryptionStandardHeader::EncryptionStandardHeader()
{
    flags        = 0;
    sizeExtra    = 0;
    algId        = 0;
    algIdHash    = 0;
    keySize      = 0;
    providedType = 0;
    reserved1    = 0;
    reserved2    = 0;
}

EncryptionVerifierAES::EncryptionVerifierAES()
{
    saltSize = SALT_LENGTH;
    memset(salt, 0, sizeof(salt));
    memset(encryptedVerifier, 0, sizeof(encryptedVerifier));
    memset(encryptedVerifierHash, 0, sizeof(encryptedVerifierHash));
}

AesEncoder::AesEncoder(Reference< XStream > xDocumentStream, oox::ole::OleStorage& rOleStorage, OUString aPassword) :
    mxDocumentStream(xDocumentStream),
    mrOleStorage(rOleStorage),
    maPassword(aPassword)
{
}

bool AesEncoder::checkEncryptionInfo(vector<sal_uInt8>& aKey, sal_uInt32 aKeyLength)
{
   return lclCheckEncryptionData(
            &aKey[0], aKeyLength,
            mEncryptionInfo.verifier.encryptedVerifier, sizeof(mEncryptionInfo.verifier.encryptedVerifier),
            mEncryptionInfo.verifier.encryptedVerifierHash, ENCRYPTED_VERIFIER_HASH_LENGTH);
}

bool AesEncoder::writeEncryptionInfo( BinaryOutputStream& rStream )
{
    rStream.writeValue(VERSION_INFO_2007_FORMAT);

    const OUString cspName = "Microsoft Enhanced RSA and AES Cryptographic Provider";
    sal_Int32      cspNameSize = (cspName.getLength() * 2) + 2;

    sal_Int32 encryptionHeaderSize = static_cast<sal_Int32>(sizeof(EncryptionStandardHeader));

    rStream << mEncryptionInfo.header.flags;
    sal_uInt32 headerSize = encryptionHeaderSize + cspNameSize;
    rStream << headerSize;

    rStream.writeMemory(&mEncryptionInfo.header, encryptionHeaderSize);
    rStream.writeUnicodeArray(cspName);
    rStream.writeValue<sal_uInt16>(0);

    sal_Int32 encryptionVerifierSize = static_cast<sal_Int32>(sizeof(EncryptionVerifierAES));
    rStream.writeMemory(&mEncryptionInfo.verifier, encryptionVerifierSize);

    return true;
}

bool AesEncoder::encode()
{
    Reference< XInputStream > xInputStream ( mxDocumentStream->getInputStream(), UNO_SET_THROW );
    Reference< XSeekable > xSeekable( xInputStream, UNO_QUERY );

    if (!xSeekable.is())
        return false;

    sal_uInt32 aLength = xSeekable->getLength();

    if (!mrOleStorage.isStorage())
        return false;

    Reference< XOutputStream > xEncryptionInfo( mrOleStorage.openOutputStream( "EncryptionInfo" ), UNO_SET_THROW );

    mEncryptionInfo.header.flags        = ENCRYPTINFO_AES | ENCRYPTINFO_CRYPTOAPI;
    mEncryptionInfo.header.algId        = ENCRYPT_ALGO_AES128;
    mEncryptionInfo.header.algIdHash    = ENCRYPT_HASH_SHA1;
    mEncryptionInfo.header.keySize      = ENCRYPT_KEY_SIZE_AES_128;
    mEncryptionInfo.header.providedType = ENCRYPT_PROVIDER_TYPE_AES;

    lclRandomGenerateValues( mEncryptionInfo.verifier.saltSize, mEncryptionInfo.verifier.salt );

    const sal_Int32 keyLength = mEncryptionInfo.header.keySize / 8;
    vector<sal_uInt8> aKey;
    aKey.resize(keyLength, 0);

    assert(keyLength == 16);

    lclGenerateEncryptionKey(mEncryptionInfo, maPassword, aKey, keyLength);

    sal_uInt8 key[16];
    std::copy(aKey.begin(), aKey.end(), key);

    lclGenerateVerifier(mEncryptionInfo, aKey, keyLength);

    if (!checkEncryptionInfo(aKey, keyLength))
        return false;

    BinaryXOutputStream aEncryptionInfoBinaryOutputStream( xEncryptionInfo, false );
    writeEncryptionInfo( aEncryptionInfoBinaryOutputStream );
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

bool AesDecoder::checkCurrentEncryptionData()
{
    return lclCheckEncryptionData(
                &mKey[0], mKeyLength,
                mEncryptionInfo.verifier.encryptedVerifier, ENCRYPTED_VERIFIER_LENGTH,
                mEncryptionInfo.verifier.encryptedVerifierHash, ENCRYPTED_VERIFIER_HASH_LENGTH );
}

bool AesDecoder::checkEncryptionData(const Sequence<NamedValue>& rEncryptionData)
{
    SequenceAsHashMap aHashData( rEncryptionData );
    Sequence<sal_Int8> aKey          = aHashData.getUnpackedValueOrDefault( "AES128EncryptionKey", Sequence<sal_Int8>() );
    Sequence<sal_Int8> aVerifier     = aHashData.getUnpackedValueOrDefault( "AES128EncryptionVerifier", Sequence<sal_Int8>() );
    Sequence<sal_Int8> aVerifierHash = aHashData.getUnpackedValueOrDefault( "AES128EncryptionVerifierHash", Sequence<sal_Int8>() );

    return lclCheckEncryptionData(
        reinterpret_cast<const sal_uInt8*>( aKey.getConstArray() ), aKey.getLength(),
        reinterpret_cast<const sal_uInt8*>( aVerifier.getConstArray() ), aVerifier.getLength(),
        reinterpret_cast<const sal_uInt8*>( aVerifierHash.getConstArray() ), aVerifierHash.getLength() );
}

bool AesDecoder::generateEncryptionKey(const OUString& rPassword)
{
    return lclGenerateEncryptionKey(mEncryptionInfo, rPassword, mKey, mKeyLength);
}

AesDecoder::AesDecoder(oox::ole::OleStorage& rOleStorage) :
    mrOleStorage(rOleStorage),
    mKeyLength(0)
{
#if USE_TLS_NSS
    // Initialize NSS, database functions are not needed
    NSS_NoDB_Init( NULL );
#endif // USE_TLS_NSS
}

bool AesDecoder::readEncryptionInfoFromStream( BinaryInputStream& rStream )
{
    sal_uInt32 aVersion;
    rStream >> aVersion;

    if (aVersion != VERSION_INFO_2007_FORMAT)
        return false;

    rStream >> mEncryptionInfo.header.flags;
    if( getFlag( mEncryptionInfo.header.flags, ENCRYPTINFO_EXTERNAL ) )
        return false;

    sal_uInt32 nHeaderSize;
    rStream >> nHeaderSize;

    sal_uInt32 actualHeaderSize = sizeof(mEncryptionInfo.header);

    if( (nHeaderSize < actualHeaderSize) )
        return false;

    rStream >> mEncryptionInfo.header;
    rStream.skip( nHeaderSize - actualHeaderSize );
    rStream >> mEncryptionInfo.verifier;

    if( mEncryptionInfo.verifier.saltSize != 16 )
        return false;
    return !rStream.isEof();
}

bool AesDecoder::readEncryptionInfo()
{
    if( !mrOleStorage.isStorage() )
        return false;

    Reference< XInputStream > xEncryptionInfo( mrOleStorage.openInputStream( "EncryptionInfo" ), UNO_SET_THROW );

    // read the encryption info stream
    BinaryXInputStream aInputStream( xEncryptionInfo, true );
    bool bValidInfo = readEncryptionInfoFromStream( aInputStream );

    if (!bValidInfo)
        return false;

    // check flags and algorithm IDs, required are AES128 and SHA-1
    bool bImplemented =
        getFlag( mEncryptionInfo.header.flags , ENCRYPTINFO_CRYPTOAPI ) &&
        getFlag( mEncryptionInfo.header.flags, ENCRYPTINFO_AES ) &&
        // algorithm ID 0 defaults to AES128 too, if ENCRYPTINFO_AES flag is set
        ((mEncryptionInfo.header.algId == 0) || (mEncryptionInfo.header.algId == ENCRYPT_ALGO_AES128)) &&
        // hash algorithm ID 0 defaults to SHA-1 too
        ((mEncryptionInfo.header.algIdHash == 0) || (mEncryptionInfo.header.algIdHash == ENCRYPT_HASH_SHA1)) &&
        (mEncryptionInfo.verifier.encryptedVerifierHashSize == 20);

    mKeyLength = (mEncryptionInfo.header.keySize / 8);
    mKey.clear();
    mKey.resize(mKeyLength, 0);

    return bImplemented;
}

Sequence<NamedValue> AesDecoder::createEncryptionData()
{
    Sequence<NamedValue> aResult;

    if (mKeyLength > 0)
    {
        SequenceAsHashMap aEncryptionData;
        EncryptionVerifierAES& verifier = mEncryptionInfo.verifier;
        aEncryptionData["AES128EncryptionKey"] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( &mKey[0] ), mKeyLength );
        aEncryptionData["AES128EncryptionSalt"] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( verifier.salt ), verifier.saltSize );
        aEncryptionData["AES128EncryptionVerifier"] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( verifier.encryptedVerifier ), sizeof( verifier.encryptedVerifier ) );
        aEncryptionData["AES128EncryptionVerifierHash"] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( verifier.encryptedVerifierHash ), sizeof( verifier.encryptedVerifierHash ) );
        aResult = aEncryptionData.getAsConstNamedValueList();
    }

    return aResult;
}

bool AesDecoder::decode( com::sun::star::uno::Reference< com::sun::star::io::XStream > xDocumentStream )
{
    if( !mrOleStorage.isStorage() )
        return false;

    // open the required input streams in the encrypted package
    Reference< XInputStream > xEncryptedPackage( mrOleStorage.openInputStream( "EncryptedPackage" ), UNO_SET_THROW );

    // create temporary file for unencrypted package
    Reference< XOutputStream > xDecryptedPackage( xDocumentStream->getOutputStream(), UNO_SET_THROW );
    BinaryXOutputStream aDecryptedPackage( xDecryptedPackage, true );
    BinaryXInputStream aEncryptedPackage( xEncryptedPackage, true );

#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX aes_ctx;
    EVP_CIPHER_CTX_init( &aes_ctx );
    EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, &mKey.front(), 0 );
    EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    PK11SlotInfo* aSlot( PK11_GetBestSlot( CKM_AES_ECB, NULL ) );
    sal_uInt8* key = new sal_uInt8[ mKeyLength ];
    std::copy(mKey.begin(), mKey.end(), key);

    SECItem keyItem;
    keyItem.type = siBuffer;
    keyItem.data = key;
    keyItem.len  = mKeyLength;

    PK11SymKey* symKey( PK11_ImportSymKey( aSlot, CKM_AES_ECB, PK11_OriginUnwrap, CKA_ENCRYPT, &keyItem, NULL ) );
    SECItem* secParam( PK11_ParamFromIV( CKM_AES_ECB, NULL ) );
    PK11Context* encContext( PK11_CreateContextBySymKey( CKM_AES_ECB, CKA_DECRYPT, symKey, secParam ) );
#endif // USE_TLS_NSS

    sal_uInt8 pnInBuffer[ 1024 ];
    sal_uInt8 pnOutBuffer[ 1024 ];
    sal_Int32 nInLen;
    int nOutLen;
    aEncryptedPackage.skip( 8 ); // decrypted size
    while( (nInLen = aEncryptedPackage.readMemory( pnInBuffer, sizeof( pnInBuffer ) )) > 0 )
    {
#if USE_TLS_OPENSSL
        EVP_DecryptUpdate( &aes_ctx, pnOutBuffer, &nOutLen, pnInBuffer, nInLen );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
        PK11_CipherOp( encContext, pnOutBuffer, &nOutLen, sizeof(pnOutBuffer), pnInBuffer, nInLen );
#endif // USE_TLS_NSS
        aDecryptedPackage.writeMemory( pnOutBuffer, nOutLen );
    }
#if USE_TLS_OPENSSL
    EVP_DecryptFinal_ex( &aes_ctx, pnOutBuffer, &nOutLen );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    uint finalLength;
    PK11_DigestFinal( encContext, pnOutBuffer, &finalLength, nInLen - nOutLen );
    nOutLen = finalLength;
#endif // USE_TLS_NSS
    aDecryptedPackage.writeMemory( pnOutBuffer, nOutLen );

#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX_cleanup( &aes_ctx );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    PK11_DestroyContext( encContext, PR_TRUE );
    PK11_FreeSymKey( symKey );
    SECITEM_FreeItem( secParam, PR_TRUE );
    delete[] key;
#endif // USE_TLS_NSS
    xDecryptedPackage->flush();
    aDecryptedPackage.seekToStart();

    return true;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
