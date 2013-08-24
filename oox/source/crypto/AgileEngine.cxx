/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "oox/crypto/AgileEngine.hxx"

namespace oox {
namespace core {

using namespace std;

namespace {

static const vector<sal_uInt8> vectorBlock1({ 0xfe, 0xa7, 0xd2, 0x76, 0x3b, 0x4b, 0x9e, 0x79 });
static const vector<sal_uInt8> vectorBlock2({ 0xd7, 0xaa, 0x0f, 0x6d, 0x30, 0x61, 0x34, 0x4e });
static const vector<sal_uInt8> vectorBlock3({ 0x14, 0x6e, 0x0b, 0xe7, 0xab, 0xac, 0xd0, 0xd6 });

bool hashCalc( std::vector<sal_uInt8>& output,
               std::vector<sal_uInt8>& input,
               const OUString& algorithm )
{
    if (algorithm == "SHA1")
        return sha1(output, input);
    else if (algorithm == "SHA512")
        return sha512(output, input);
    return false;
}

} // namespace

AgileEngine::AgileEngine() :
    CryptoEngine()
{}

AgileEngine::~AgileEngine()
{}

AgileEncryptionInfo& AgileEngine::getInfo()
{
    return mInfo;
}

Crypto::CryptoType AgileEngine::cryptoType(const AgileEncryptionInfo& rInfo)
{
    if (rInfo.keyBits == 128 && rInfo.cipherAlgorithm == "AES" && rInfo.cipherChaining == "ChainingModeCBC")
        return Crypto::AES_128_CBC;
    else if (rInfo.keyBits == 256 && rInfo.cipherAlgorithm == "AES" && rInfo.cipherChaining == "ChainingModeCBC")
        return Crypto::AES_256_CBC;
    return Crypto::UNKNOWN;
}

bool AgileEngine::calculateBlock(
    const vector<sal_uInt8>& rBlock,
    vector<sal_uInt8>& rHashFinal,
    vector<sal_uInt8>& rInput,
    vector<sal_uInt8>& rOutput)
{
    vector<sal_uInt8> hash(mInfo.hashSize, 0);
    vector<sal_uInt8> salt = mInfo.saltValue;
    vector<sal_uInt8> dataFinal(mInfo.hashSize + rBlock.size(), 0);
    std::copy(rHashFinal.begin(), rHashFinal.end(), dataFinal.begin());
    std::copy(
            rBlock.begin(),
            rBlock.begin() + rBlock.size(),
            dataFinal.begin() + mInfo.hashSize);

    hashCalc(hash, dataFinal, mInfo.hashAlgorithm);

    sal_Int32 keySize = mInfo.keyBits / 8;
    vector<sal_uInt8> key(keySize, 0);

    std::copy(hash.begin(), hash.begin() + keySize, key.begin());

    Decrypt aDecryptor(key, salt, cryptoType(mInfo));
    aDecryptor.update(rOutput, rInput);

    return true;
}

bool AgileEngine::calculateHashFinal(const OUString& rPassword, vector<sal_uInt8>& aHashFinal)
{
    sal_Int32 saltSize = mInfo.saltSize;
    vector<sal_uInt8> salt = mInfo.saltValue;
    sal_uInt32 passwordByteLength = rPassword.getLength() * 2;

    vector<sal_uInt8> initialData(saltSize + passwordByteLength);
    std::copy(salt.begin(), salt.end(), initialData.begin());

    const sal_uInt8* passwordByteArray = reinterpret_cast<const sal_uInt8*>(rPassword.getStr());

    std::copy(
        passwordByteArray,
        passwordByteArray + passwordByteLength,
        initialData.begin() + saltSize);

    vector<sal_uInt8> hash(mInfo.hashSize, 0);

    hashCalc(hash, initialData, mInfo.hashAlgorithm);

    vector<sal_uInt8> data(mInfo.hashSize + 4, 0);

    for (int i = 0; i < mInfo.spinCount; i++)
    {
        ByteOrderConverter::writeLittleEndian( &data[0], i );
        std::copy(hash.begin(), hash.end(), data.begin() + 4);
        hashCalc(hash, data, mInfo.hashAlgorithm);
    }

    std::copy(hash.begin(), hash.end(), aHashFinal.begin());

    return true;
}

bool AgileEngine::generateEncryptionKey(const OUString& rPassword)
{
    mKey.clear();
    mKey.resize(mInfo.keyBits / 8, 0);

    vector<sal_uInt8> hashFinal(mInfo.hashSize, 0);
    calculateHashFinal(rPassword, hashFinal);

    vector<sal_uInt8> encryptedHashInput = mInfo.encryptedVerifierHashInput;
    vector<sal_uInt8> hashInput(mInfo.saltSize, 0);
    calculateBlock(vectorBlock1, hashFinal, encryptedHashInput, hashInput);

    vector<sal_uInt8> encryptedHashValue = mInfo.encryptedVerifierHashValue;
    vector<sal_uInt8> hashValue(encryptedHashValue.size(), 0);
    calculateBlock(vectorBlock2, hashFinal, encryptedHashValue, hashValue);

    vector<sal_uInt8> hash(mInfo.hashSize, 0);
    hashCalc(hash, hashInput, mInfo.hashAlgorithm);

    if (std::equal (hash.begin(), hash.end(), hashValue.begin()) )
    {
        vector<sal_uInt8> encryptedKeyValue = mInfo.encryptedKeyValue;
        calculateBlock(vectorBlock3, hashFinal, encryptedKeyValue, mKey);
        return true;
    }

    return false;
}

bool AgileEngine::decrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream)
{
    sal_uInt32 totalSize;
    aInputStream >> totalSize; // Document unencrypted size - 4 bytes
    aInputStream.skip( 4 );    // Reserved 4 Bytes

    vector<sal_uInt8> keyDataSalt = mInfo.keyDataSalt;

    sal_uInt32 saltSize = mInfo.saltSize;
    sal_uInt32 keySize = mInfo.keyBits / 8;

    sal_uInt32 segment = 0;

    vector<sal_uInt8> saltWithBlockKey(saltSize + sizeof(segment), 0);
    std::copy(keyDataSalt.begin(), keyDataSalt.end(), saltWithBlockKey.begin());

    vector<sal_uInt8> hash(mInfo.hashSize, 0);
    vector<sal_uInt8> iv(keySize, 0);

    vector<sal_uInt8> inputBuffer (SEGMENT_LENGTH);
    vector<sal_uInt8> outputBuffer(SEGMENT_LENGTH);
    sal_uInt32 inputLength;
    sal_uInt32 outputLength;
    sal_uInt32 remaining = totalSize;

    while( (inputLength = aInputStream.readMemory( &inputBuffer[0], SEGMENT_LENGTH )) > 0 )
    {
        sal_uInt8* segmentBegin = reinterpret_cast<sal_uInt8*>(&segment);
        sal_uInt8* segmentEnd   = segmentBegin + sizeof(segment);
        std::copy(segmentBegin, segmentEnd, saltWithBlockKey.begin() + saltSize);

        hashCalc(hash, saltWithBlockKey, mInfo.hashAlgorithm);

        // Only if hash > keySize
        std::copy(hash.begin(), hash.begin() + keySize, iv.begin());

        Decrypt aDecryptor(mKey, iv, AgileEngine::cryptoType(mInfo));
        outputLength = aDecryptor.update(outputBuffer, inputBuffer, inputLength);

        sal_uInt32 writeLength = outputLength > remaining ? remaining : outputLength;
        aOutputStream.writeMemory( &outputBuffer[0], writeLength );

        remaining -= outputLength;
        segment++;
    }

    return true;
}

bool AgileEngine::writeEncryptionInfo(
                        const OUString& /*aPassword*/,
                        BinaryXOutputStream& /*rStream*/)
{
    return false; // Agile encrypting is not supported for now
}

bool AgileEngine::encrypt(
                    BinaryXInputStream& /*aInputStream*/,
                    BinaryXOutputStream& /*aOutputStream*/)
{
    return false; // Agile encrypting is not supported for now
}


} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
