/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/Standard2007Engine.hxx>

#include <oox/crypto/CryptTools.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <osl/time.h>
#include <rtl/digest.h>
#include <rtl/random.h>

#include <comphelper/hash.hxx>

namespace oox {
namespace core {

/* =========================================================================== */
/*  Kudos to Caolan McNamara who provided the core decryption implementations. */
/* =========================================================================== */
namespace
{

void lclRandomGenerateValues(sal_uInt8* aArray, sal_uInt32 aSize)
{
    TimeValue aTime;
    osl_getSystemTime(&aTime);
    rtlRandomPool aRandomPool = rtl_random_createPool();
    rtl_random_addBytes(aRandomPool, &aTime, 8);
    rtl_random_getBytes(aRandomPool, aArray, aSize);
    rtl_random_destroyPool(aRandomPool);
}

static const OUString lclCspName = "Microsoft Enhanced RSA and AES Cryptographic Provider";

} // end anonymous namespace

bool Standard2007Engine::generateVerifier()
{
    // only support key of size 128 bit (16 byte)
    if (mKey.size() != 16)
        return false;

    std::vector<sal_uInt8> verifier(msfilter::ENCRYPTED_VERIFIER_LENGTH);
    std::vector<sal_uInt8> encryptedVerifier(msfilter::ENCRYPTED_VERIFIER_LENGTH);

    lclRandomGenerateValues(verifier.data(), verifier.size());

    std::vector<sal_uInt8> iv;
    Encrypt aEncryptorVerifier(mKey, iv, Crypto::AES_128_ECB);
    if (aEncryptorVerifier.update(encryptedVerifier, verifier) != msfilter::ENCRYPTED_VERIFIER_LENGTH)
        return false;
    std::copy(encryptedVerifier.begin(), encryptedVerifier.end(), mInfo.verifier.encryptedVerifier);

    mInfo.verifier.encryptedVerifierHashSize = msfilter::SHA1_HASH_LENGTH;
    std::vector<sal_uInt8> hash = comphelper::Hash::calculateHash(verifier.data(), verifier.size(), comphelper::HashType::SHA1);
    hash.resize(msfilter::SHA256_HASH_LENGTH, 0);

    std::vector<sal_uInt8> encryptedHash(msfilter::SHA256_HASH_LENGTH, 0);

    Encrypt aEncryptorHash(mKey, iv, Crypto::AES_128_ECB);
    aEncryptorHash.update(encryptedHash, hash, hash.size());
    std::copy(encryptedHash.begin(), encryptedHash.end(), mInfo.verifier.encryptedVerifierHash);

    return true;
}

bool Standard2007Engine::calculateEncryptionKey(const OUString& rPassword)
{
    sal_uInt32 saltSize = mInfo.verifier.saltSize;
    sal_uInt32 passwordByteLength = rPassword.getLength() * 2;
    const sal_uInt8* saltArray = mInfo.verifier.salt;

    // Prepare initial data -> salt + password (in 16-bit chars)
    std::vector<sal_uInt8> initialData(saltSize + passwordByteLength);
    std::copy(saltArray, saltArray + saltSize, initialData.begin());

    const sal_uInt8* passwordByteArray = reinterpret_cast<const sal_uInt8*>(rPassword.getStr());

    std::copy(
        passwordByteArray,
        passwordByteArray + passwordByteLength,
        initialData.begin() + saltSize);

    // use "hash" vector for result of sha1 hashing
    // calculate SHA1 hash of initialData
    std::vector<sal_uInt8> hash = comphelper::Hash::calculateHash(initialData.data(), initialData.size(), comphelper::HashType::SHA1);

    // data = iterator (4bytes) + hash
    std::vector<sal_uInt8> data(msfilter::SHA1_HASH_LENGTH + 4, 0);

    for (sal_Int32 i = 0; i < 50000; ++i)
    {
        ByteOrderConverter::writeLittleEndian(data.data(), i);
        std::copy(hash.begin(), hash.end(), data.begin() + 4);
        hash = comphelper::Hash::calculateHash(data.data(), data.size(), comphelper::HashType::SHA1);
    }
    std::copy(hash.begin(), hash.end(), data.begin() );
    std::fill(data.begin() + msfilter::SHA1_HASH_LENGTH, data.end(), 0 );

    hash = comphelper::Hash::calculateHash(data.data(), data.size(), comphelper::HashType::SHA1);

    // derive key
    std::vector<sal_uInt8> buffer(64, 0x36);
    for (size_t i = 0; i < hash.size(); ++i)
        buffer[i] ^= hash[i];

    hash = comphelper::Hash::calculateHash(buffer.data(), buffer.size(), comphelper::HashType::SHA1);
    if (mKey.size() > hash.size())
        return false;
    std::copy(hash.begin(), hash.begin() + mKey.size(), mKey.begin());

    return true;
}

bool Standard2007Engine::generateEncryptionKey(const OUString& password)
{
    mKey.clear();
    mKey.resize(mInfo.header.keyBits / 8, 0);

    calculateEncryptionKey(password);

    std::vector<sal_uInt8> encryptedVerifier(msfilter::ENCRYPTED_VERIFIER_LENGTH);
    std::copy(
        mInfo.verifier.encryptedVerifier,
        mInfo.verifier.encryptedVerifier + msfilter::ENCRYPTED_VERIFIER_LENGTH,
        encryptedVerifier.begin());

    std::vector<sal_uInt8> encryptedHash(msfilter::SHA256_HASH_LENGTH);
    std::copy(
        mInfo.verifier.encryptedVerifierHash,
        mInfo.verifier.encryptedVerifierHash + msfilter::SHA256_HASH_LENGTH,
        encryptedHash.begin());

    std::vector<sal_uInt8> verifier(encryptedVerifier.size(), 0);
    Decrypt::aes128ecb(verifier, encryptedVerifier, mKey);

    std::vector<sal_uInt8> verifierHash(encryptedHash.size(), 0);
    Decrypt::aes128ecb(verifierHash, encryptedHash, mKey);

    std::vector<sal_uInt8> hash = comphelper::Hash::calculateHash(verifier.data(), verifier.size(), comphelper::HashType::SHA1);

    return std::equal(hash.begin(), hash.end(), verifierHash.begin());
}

bool Standard2007Engine::decrypt(BinaryXInputStream& aInputStream,
                                 BinaryXOutputStream& aOutputStream)
{
    aInputStream.skip(4); // Document unencrypted size - 4 bytes
    aInputStream.skip(4); // Reserved 4 Bytes

    std::vector<sal_uInt8> iv;
    Decrypt aDecryptor(mKey, iv, Crypto::AES_128_ECB);
    std::vector<sal_uInt8> inputBuffer (4096);
    std::vector<sal_uInt8> outputBuffer(4096);
    sal_uInt32 inputLength;
    sal_uInt32 outputLength;

    while ((inputLength = aInputStream.readMemory(inputBuffer.data(), inputBuffer.size())) > 0)
    {
        outputLength = aDecryptor.update(outputBuffer, inputBuffer, inputLength);
        aOutputStream.writeMemory(outputBuffer.data(), outputLength);
    }
    return true;
}

void Standard2007Engine::writeEncryptionInfo(const OUString& password, BinaryXOutputStream& rStream)
{
    mInfo.header.flags        = msfilter::ENCRYPTINFO_AES | msfilter::ENCRYPTINFO_CRYPTOAPI;
    mInfo.header.algId        = msfilter::ENCRYPT_ALGO_AES128;
    mInfo.header.algIdHash    = msfilter::ENCRYPT_HASH_SHA1;
    mInfo.header.keyBits      = msfilter::ENCRYPT_KEY_SIZE_AES_128;
    mInfo.header.providedType = msfilter::ENCRYPT_PROVIDER_TYPE_AES;

    lclRandomGenerateValues(mInfo.verifier.salt, mInfo.verifier.saltSize);
    const sal_Int32 keyLength = mInfo.header.keyBits / 8;

    mKey.clear();
    mKey.resize(keyLength, 0);

    if (!calculateEncryptionKey(password))
        return;

    if (!generateVerifier())
        return;

    rStream.WriteUInt32(msfilter::VERSION_INFO_2007_FORMAT);

    sal_uInt32 cspNameSize = (lclCspName.getLength() * 2) + 2;

    sal_uInt32 encryptionHeaderSize = static_cast<sal_uInt32>(sizeof(msfilter::EncryptionStandardHeader));

    rStream.WriteUInt32(mInfo.header.flags);
    sal_uInt32 headerSize = encryptionHeaderSize + cspNameSize;
    rStream.WriteUInt32(headerSize);

    rStream.writeMemory(&mInfo.header, encryptionHeaderSize);
    rStream.writeUnicodeArray(lclCspName);
    rStream.WriteUInt16(0);

    rStream.writeMemory(&mInfo.verifier, sizeof(msfilter::EncryptionVerifierAES));
}

void Standard2007Engine::encrypt(BinaryXInputStream& aInputStream,
                                 BinaryXOutputStream& aOutputStream)
{
    std::vector<sal_uInt8> inputBuffer(1024);
    std::vector<sal_uInt8> outputBuffer(1024);

    sal_uInt32 inputLength;
    sal_uInt32 outputLength;

    std::vector<sal_uInt8> iv;
    Encrypt aEncryptor(mKey, iv, Crypto::AES_128_ECB);

    while ((inputLength = aInputStream.readMemory(inputBuffer.data(), inputBuffer.size())) > 0)
    {
        inputLength = inputLength % 16 == 0 ? inputLength : ((inputLength / 16) * 16) + 16;
        outputLength = aEncryptor.update(outputBuffer, inputBuffer, inputLength);
        aOutputStream.writeMemory(outputBuffer.data(), outputLength);
    }
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
