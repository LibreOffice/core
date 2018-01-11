/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/AgileEngine.hxx>

#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>

#include <comphelper/hash.hxx>

namespace oox {
namespace core {

namespace {

const sal_uInt32 constSegmentLength = 4096;

bool hashCalc(std::vector<sal_uInt8>& output,
              std::vector<sal_uInt8>& input,
              const OUString& sAlgorithm )
{
    if (sAlgorithm == "SHA1")
    {
        std::vector<unsigned char> out = comphelper::Hash::calculateHash(input.data(), input.size(), comphelper::HashType::SHA1);
        output = out;
        return true;
    }
    else if (sAlgorithm == "SHA512")
    {
        std::vector<unsigned char> out = comphelper::Hash::calculateHash(input.data(), input.size(), comphelper::HashType::SHA512);
        output = out;
        return true;
    }
    return false;
}

} // namespace

Crypto::CryptoType AgileEngine::cryptoType(const AgileEncryptionInfo& rInfo)
{
    if (rInfo.keyBits == 128 && rInfo.cipherAlgorithm == "AES" && rInfo.cipherChaining == "ChainingModeCBC")
        return Crypto::AES_128_CBC;
    else if (rInfo.keyBits == 256 && rInfo.cipherAlgorithm == "AES" && rInfo.cipherChaining == "ChainingModeCBC")
        return Crypto::AES_256_CBC;
    return Crypto::UNKNOWN;
}

void AgileEngine::calculateBlock(
    std::vector<sal_uInt8> const & rBlock,
    std::vector<sal_uInt8>& rHashFinal,
    std::vector<sal_uInt8>& rInput,
    std::vector<sal_uInt8>& rOutput)
{
    std::vector<sal_uInt8> hash(mInfo.hashSize, 0);
    std::vector<sal_uInt8> dataFinal(mInfo.hashSize + rBlock.size(), 0);
    std::copy(rHashFinal.begin(), rHashFinal.end(), dataFinal.begin());
    std::copy(rBlock.begin(), rBlock.end(), dataFinal.begin() + mInfo.hashSize);

    hashCalc(hash, dataFinal, mInfo.hashAlgorithm);

    sal_Int32 keySize = mInfo.keyBits / 8;
    std::vector<sal_uInt8> key(keySize, 0);

    std::copy(hash.begin(), hash.begin() + keySize, key.begin());

    Decrypt aDecryptor(key, mInfo.saltValue, cryptoType(mInfo));
    aDecryptor.update(rOutput, rInput);
}

void AgileEngine::calculateHashFinal(const OUString& rPassword, std::vector<sal_uInt8>& aHashFinal)
{
    sal_Int32 saltSize = mInfo.saltSize;
    std::vector<sal_uInt8>& salt = mInfo.saltValue;

    sal_uInt32 passwordByteLength = rPassword.getLength() * 2;

    std::vector<sal_uInt8> initialData(saltSize + passwordByteLength);
    std::copy(salt.begin(), salt.end(), initialData.begin());

    const sal_uInt8* passwordByteArray = reinterpret_cast<const sal_uInt8*>(rPassword.getStr());

    std::copy(
        passwordByteArray,
        passwordByteArray + passwordByteLength,
        initialData.begin() + saltSize);

    std::vector<sal_uInt8> hash(mInfo.hashSize, 0);

    hashCalc(hash, initialData, mInfo.hashAlgorithm);

    std::vector<sal_uInt8> data(mInfo.hashSize + 4, 0);

    for (sal_Int32 i = 0; i < mInfo.spinCount; i++)
    {
        ByteOrderConverter::writeLittleEndian(data.data(), i);
        std::copy(hash.begin(), hash.end(), data.begin() + 4);
        hashCalc(hash, data, mInfo.hashAlgorithm);
    }

    std::copy(hash.begin(), hash.end(), aHashFinal.begin());
}

bool AgileEngine::generateEncryptionKey(const OUString& rPassword)
{
    static const std::vector<sal_uInt8> constBlock1{ 0xfe, 0xa7, 0xd2, 0x76, 0x3b, 0x4b, 0x9e, 0x79 };
    static const std::vector<sal_uInt8> constBlock2{ 0xd7, 0xaa, 0x0f, 0x6d, 0x30, 0x61, 0x34, 0x4e };
    static const std::vector<sal_uInt8> constBlock3{ 0x14, 0x6e, 0x0b, 0xe7, 0xab, 0xac, 0xd0, 0xd6 };

    mKey.clear();
    mKey.resize(mInfo.keyBits / 8, 0);

    std::vector<sal_uInt8> hashFinal(mInfo.hashSize, 0);
    calculateHashFinal(rPassword, hashFinal);

    std::vector<sal_uInt8>& encryptedHashInput = mInfo.encryptedVerifierHashInput;
    std::vector<sal_uInt8> hashInput(mInfo.saltSize, 0);
    calculateBlock(constBlock1, hashFinal, encryptedHashInput, hashInput);

    std::vector<sal_uInt8>& encryptedHashValue = mInfo.encryptedVerifierHashValue;
    std::vector<sal_uInt8> hashValue(encryptedHashValue.size(), 0);
    calculateBlock(constBlock2, hashFinal, encryptedHashValue, hashValue);

    std::vector<sal_uInt8> hash(mInfo.hashSize, 0);
    hashCalc(hash, hashInput, mInfo.hashAlgorithm);

    if (hash.size() <= hashValue.size() && std::equal(hash.begin(), hash.end(), hashValue.begin()))
    {
        std::vector<sal_uInt8>& encryptedKeyValue = mInfo.encryptedKeyValue;
        calculateBlock(constBlock3, hashFinal, encryptedKeyValue, mKey);
        return true;
    }

    return false;
}

bool AgileEngine::decrypt(BinaryXInputStream& aInputStream,
                          BinaryXOutputStream& aOutputStream)
{
    sal_uInt32 totalSize = aInputStream.readuInt32(); // Document unencrypted size - 4 bytes
    aInputStream.skip(4);  // Reserved 4 Bytes

    std::vector<sal_uInt8>& keyDataSalt = mInfo.keyDataSalt;

    sal_uInt32 saltSize = mInfo.saltSize;
    sal_uInt32 keySize = mInfo.keyBits / 8;

    sal_uInt32 segment = 0;

    std::vector<sal_uInt8> saltWithBlockKey(saltSize + sizeof(segment), 0);
    std::copy(keyDataSalt.begin(), keyDataSalt.end(), saltWithBlockKey.begin());

    std::vector<sal_uInt8> hash(mInfo.hashSize, 0);
    std::vector<sal_uInt8> iv(keySize, 0);

    std::vector<sal_uInt8> inputBuffer(constSegmentLength);
    std::vector<sal_uInt8> outputBuffer(constSegmentLength);
    sal_uInt32 inputLength;
    sal_uInt32 outputLength;
    sal_uInt32 remaining = totalSize;

    while ((inputLength = aInputStream.readMemory(inputBuffer.data(), constSegmentLength)) > 0)
    {
        sal_uInt8* segmentBegin = reinterpret_cast<sal_uInt8*>(&segment);
        sal_uInt8* segmentEnd   = segmentBegin + sizeof(segment);
        std::copy(segmentBegin, segmentEnd, saltWithBlockKey.begin() + saltSize);

        hashCalc(hash, saltWithBlockKey, mInfo.hashAlgorithm);

        // Only if hash > keySize
        std::copy(hash.begin(), hash.begin() + keySize, iv.begin());

        Decrypt aDecryptor(mKey, iv, AgileEngine::cryptoType(mInfo));
        outputLength = aDecryptor.update(outputBuffer, inputBuffer, inputLength);

        sal_uInt32 writeLength = std::min(outputLength, remaining);
        aOutputStream.writeMemory(outputBuffer.data(), writeLength);

        remaining -= outputLength;
        segment++;
    }

    return true;
}

void AgileEngine::writeEncryptionInfo(
                        const OUString& /*aPassword*/,
                        BinaryXOutputStream& /*rStream*/)
{
    // Agile encrypting is not supported for now
}

void AgileEngine::encrypt(
                    BinaryXInputStream& /*aInputStream*/,
                    BinaryXOutputStream& /*aOutputStream*/)
{
    // Agile encrypting is not supported for now
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
