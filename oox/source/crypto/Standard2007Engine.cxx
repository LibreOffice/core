/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "oox/crypto/Standard2007Engine.hxx"

#include <osl/time.h>
#include <rtl/random.h>

namespace oox {
namespace core {

using namespace std;

/* =========================================================================== */
/*  Kudos to Caolan McNamara who provided the core decryption implementations. */
/* =========================================================================== */
namespace
{

void lclRandomGenerateValues(sal_uInt8* aArray, sal_uInt32 aSize)
{
    TimeValue aTime;
    osl_getSystemTime( &aTime );
    rtlRandomPool aRandomPool = rtl_random_createPool ();
    rtl_random_addBytes ( aRandomPool, &aTime, 8 );
    rtl_random_getBytes ( aRandomPool, aArray, aSize );
    rtl_random_destroyPool ( aRandomPool );
}

static const OUString lclCspName = "Microsoft Enhanced RSA and AES Cryptographic Provider";

} // namespace

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
{
    saltSize = SALT_LENGTH;
    memset(salt, 0, sizeof(salt));
    memset(encryptedVerifier, 0, sizeof(encryptedVerifier));
    memset(encryptedVerifierHash, 0, sizeof(encryptedVerifierHash));
}

Standard2007Engine::Standard2007Engine() :
    CryptoEngine()
{}

Standard2007Engine::~Standard2007Engine()
{}

StandardEncryptionInfo& Standard2007Engine::getInfo()
{
    return mInfo;
}

bool Standard2007Engine::generateVerifier()
{
    // only support key of size 128 bit (16 byte)
    if (mKey.size() != 16)
        return false;

    sal_uInt32 outputLength;
    vector<sal_uInt8> verifier(ENCRYPTED_VERIFIER_LENGTH);
    vector<sal_uInt8> encryptedVerifier(ENCRYPTED_VERIFIER_LENGTH);

    lclRandomGenerateValues(&verifier[0], verifier.size());

    vector<sal_uInt8> iv;
    Encrypt aEncryptorVerifier(mKey, iv, Crypto::AES_128_ECB);
    outputLength = aEncryptorVerifier.update(encryptedVerifier, verifier);
    if (outputLength != ENCRYPTED_VERIFIER_LENGTH)
        return false;
    std::copy(encryptedVerifier.begin(), encryptedVerifier.end(), mInfo.verifier.encryptedVerifier);

    vector<sal_uInt8> hash(RTL_DIGEST_LENGTH_SHA1, 0);
    mInfo.verifier.encryptedVerifierHashSize = RTL_DIGEST_LENGTH_SHA1;
    sha1(hash, verifier);
    hash.resize(ENCRYPTED_VERIFIER_HASH_LENGTH, 0);

    vector<sal_uInt8> encryptedHash(ENCRYPTED_VERIFIER_HASH_LENGTH, 0);

    Encrypt aEncryptorHash(mKey, iv, Crypto::AES_128_ECB);
    outputLength = aEncryptorHash.update(encryptedHash, hash, hash.size());
    std::copy(encryptedHash.begin(), encryptedHash.end(), mInfo.verifier.encryptedVerifierHash);

    return true;
}

bool Standard2007Engine::calculateEncryptionKey(const OUString& rPassword)
{
    sal_uInt32 saltSize = mInfo.verifier.saltSize;
    sal_uInt32 passwordByteLength = rPassword.getLength() * 2;
    const sal_uInt8* saltArray = mInfo.verifier.salt;

    // Prepare initial data -> salt + password (in 16-bit chars)
    vector<sal_uInt8> initialData(saltSize + passwordByteLength);
    std::copy(saltArray, saltArray + saltSize, initialData.begin());

    const sal_uInt8* passwordByteArray = reinterpret_cast<const sal_uInt8*>(rPassword.getStr());

    std::copy(
        passwordByteArray,
        passwordByteArray + passwordByteLength,
        initialData.begin() + saltSize);

    // use "hash" vector for result of sha1 hashing
    vector<sal_uInt8> hash(RTL_DIGEST_LENGTH_SHA1, 0);

    // calculate SHA1 hash of initialData
    sha1(hash, initialData);

    // data = iterator (4bytes) + hash
    vector<sal_uInt8> data(RTL_DIGEST_LENGTH_SHA1 + 4, 0);

    for (int i = 0; i < 50000; i++)
    {
        ByteOrderConverter::writeLittleEndian( &data[0], i );
        std::copy(hash.begin(), hash.end(), data.begin() + 4);
        sha1(hash, data);
    }
    std::copy(hash.begin(), hash.end(), data.begin() );
    std::fill(data.begin() + RTL_DIGEST_LENGTH_SHA1, data.end(), 0 );

    sha1(hash, data);

    // derive key
    vector<sal_uInt8> buffer(64, 0x36);
    for( sal_uInt32 i = 0; i < hash.size(); ++i )
        buffer[i] ^= hash[i];

    sha1(hash, buffer);
    std::copy(hash.begin(), hash.begin() + mKey.size(), mKey.begin());

    return true;
}

bool Standard2007Engine::generateEncryptionKey(const OUString& password)
{
    mKey.clear();
    mKey.resize(mInfo.header.keyBits / 8, 0);

    calculateEncryptionKey(password);

    vector<sal_uInt8> encryptedVerifier(ENCRYPTED_VERIFIER_LENGTH);
    std::copy(
        mInfo.verifier.encryptedVerifier,
        mInfo.verifier.encryptedVerifier + ENCRYPTED_VERIFIER_LENGTH,
        encryptedVerifier.begin());

    vector<sal_uInt8> encryptedVerifierHash(ENCRYPTED_VERIFIER_HASH_LENGTH);
    std::copy(
        mInfo.verifier.encryptedVerifierHash,
        mInfo.verifier.encryptedVerifierHash + ENCRYPTED_VERIFIER_HASH_LENGTH,
        encryptedVerifierHash.begin());

    return checkEncryptionData(
                mKey, mKey.size(),
                encryptedVerifier, encryptedVerifier.size(),
                encryptedVerifierHash, encryptedVerifierHash.size() );
}

bool Standard2007Engine::decrypt(
                            BinaryXInputStream& aInputStream,
                            BinaryXOutputStream& aOutputStream)
{
    sal_uInt32 totalSize;
    aInputStream >> totalSize; // Document unencrypted size - 4 bytes
    aInputStream.skip( 4 );    // Reserved 4 Bytes

    vector<sal_uInt8> iv;
    Decrypt aDecryptor(mKey, iv, Crypto::AES_128_ECB);
    vector<sal_uInt8> inputBuffer (4096);
    vector<sal_uInt8> outputBuffer(4096);
    sal_uInt32 inputLength;
    sal_uInt32 outputLength;

    while( (inputLength = aInputStream.readMemory( &inputBuffer[0], inputBuffer.size() )) > 0 )
    {
        outputLength = aDecryptor.update(outputBuffer, inputBuffer, inputLength);
        aOutputStream.writeMemory( &outputBuffer[0], outputLength );
    }
    return true;
}

bool Standard2007Engine::checkEncryptionData(
            vector<sal_uInt8> key, sal_uInt32 keySize,
            vector<sal_uInt8> encryptedVerifier, sal_uInt32 verifierSize,
            vector<sal_uInt8> encryptedHash, sal_uInt32 hashSize )
{
    // the only currently supported algorithm needs key size 128
    if ( keySize != 16 || verifierSize != 16 )
        return false;

    vector<sal_uInt8> verifier(verifierSize, 0);
    Decrypt::aes128ecb(verifier, encryptedVerifier, key);

    vector<sal_uInt8> verifierHash(hashSize, 0);
    Decrypt::aes128ecb(verifierHash, encryptedHash, key);

    vector<sal_uInt8> hash(RTL_DIGEST_LENGTH_SHA1, 0);
    sha1(hash, verifier);

    return std::equal( hash.begin(), hash.end(), verifierHash.begin() );
}

bool Standard2007Engine::writeEncryptionInfo(const OUString& password, BinaryXOutputStream& rStream)
{
    mInfo.header.flags        = ENCRYPTINFO_AES | ENCRYPTINFO_CRYPTOAPI;
    mInfo.header.algId        = ENCRYPT_ALGO_AES128;
    mInfo.header.algIdHash    = ENCRYPT_HASH_SHA1;
    mInfo.header.keyBits      = ENCRYPT_KEY_SIZE_AES_128;
    mInfo.header.providedType = ENCRYPT_PROVIDER_TYPE_AES;

    lclRandomGenerateValues(mInfo.verifier.salt, mInfo.verifier.saltSize);
    const sal_Int32 keyLength = mInfo.header.keyBits / 8;

    mKey.clear();
    mKey.resize(keyLength, 0);

    if (!calculateEncryptionKey(password))
        return false;

    if (!generateVerifier())
        return false;

    rStream.writeValue(VERSION_INFO_2007_FORMAT);

    sal_uInt32 cspNameSize = (lclCspName.getLength() * 2) + 2;

    sal_uInt32 encryptionHeaderSize = static_cast<sal_uInt32>(sizeof(EncryptionStandardHeader));

    rStream << mInfo.header.flags;
    sal_uInt32 headerSize = encryptionHeaderSize + cspNameSize;
    rStream << headerSize;

    rStream.writeMemory(&mInfo.header, encryptionHeaderSize);
    rStream.writeUnicodeArray(lclCspName);
    rStream.writeValue<sal_uInt16>(0);

    sal_uInt32 encryptionVerifierSize = static_cast<sal_uInt32>(sizeof(EncryptionVerifierAES));
    rStream.writeMemory(&mInfo.verifier, encryptionVerifierSize);

    return true;
}

bool Standard2007Engine::encrypt(
                            BinaryXInputStream& aInputStream,
                            BinaryXOutputStream& aOutputStream)
{
    vector<sal_uInt8> inputBuffer(1024);
    vector<sal_uInt8> outputBuffer(1024);

    sal_uInt32 inputLength;
    sal_uInt32 outputLength;

    vector<sal_uInt8> iv;
    Encrypt aEncryptor(mKey, iv, Crypto::AES_128_ECB);

    while( (inputLength = aInputStream.readMemory( &inputBuffer[0], inputBuffer.size() )) > 0 )
    {
        inputLength = inputLength % 16 == 0 ? inputLength : ((inputLength / 16) * 16) + 16;
        outputLength = aEncryptor.update(outputBuffer, inputBuffer, inputLength);
        aOutputStream.writeMemory( &outputBuffer[0], outputLength );
    }
    return true;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
