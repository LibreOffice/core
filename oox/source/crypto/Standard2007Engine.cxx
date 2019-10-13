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

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/io/XSequenceOutputStream.hpp>
#include <oox/crypto/CryptTools.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <rtl/digest.h>
#include <rtl/random.h>

#include <comphelper/hash.hxx>
#include <comphelper/sequenceashashmap.hxx>

using namespace css::io;
using namespace css::uno;

namespace oox {
namespace core {

extern "C" SAL_DLLPUBLIC_EXPORT XInterface*
    com_sun_star_comp_oox_crypto_Standard2007_get_implementation(
        XComponentContext* pCtx, Sequence<Any> const& /*arguments*/)
{
    return cppu::acquire(new Standard2007Engine(pCtx/*, arguments*/));
}

/* =========================================================================== */
/*  Kudos to Caolan McNamara who provided the core decryption implementations. */
/* =========================================================================== */
namespace
{

void lclRandomGenerateValues(sal_uInt8* aArray, sal_uInt32 aSize)
{
    rtlRandomPool aRandomPool = rtl_random_createPool();
    rtl_random_getBytes(aRandomPool, aArray, aSize);
    rtl_random_destroyPool(aRandomPool);
}

static const OUString lclCspName = "Microsoft Enhanced RSA and AES Cryptographic Provider";
constexpr const sal_uInt32 AES128Size = 16;

} // end anonymous namespace

Standard2007Engine::Standard2007Engine(const css::uno::Reference<css::uno::XComponentContext>& rxContext)
 : mxContext(rxContext)
{

}

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

sal_Bool Standard2007Engine::generateEncryptionKey(const OUString& password)
{
    mKey.clear();
    /*
        KeySize (4 bytes): An unsigned integer that specifies the number of bits in the encryption key.
        MUST be a multiple of 8. MUST be one of the values in the following table:
        Algorithm   Value                               Comment
        Any         0x00000000                          Determined by Flags
        RC4         0x00000028 – 0x00000080             (inclusive) 8-bit increments.
        AES         0x00000080, 0x000000C0, 0x00000100  128, 192 or 256-bit
    */
    if (mInfo.header.keyBits > 8192) // should we strictly enforce the above 256 bit limit ?
        return false;
    mKey.resize(mInfo.header.keyBits / 8, 0);
    if (mKey.empty())
        return false;

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

sal_Bool Standard2007Engine::decrypt(const css::uno::Reference<css::io::XInputStream>& rxInputStream,
    css::uno::Reference<css::io::XOutputStream>& rxOutputStream)
{
    BinaryXInputStream aInputStream(rxInputStream, true);
    BinaryXOutputStream aOutputStream(rxOutputStream, true);

    sal_uInt32 totalSize = aInputStream.readuInt32(); // Document unencrypted size - 4 bytes
    aInputStream.skip(4); // Reserved 4 Bytes

    std::vector<sal_uInt8> iv;
    Decrypt aDecryptor(mKey, iv, Crypto::AES_128_ECB);
    std::vector<sal_uInt8> inputBuffer (4096);
    std::vector<sal_uInt8> outputBuffer(4096);
    sal_uInt32 inputLength;
    sal_uInt32 outputLength;
    sal_uInt32 remaining = totalSize;

    while ((inputLength = aInputStream.readMemory(inputBuffer.data(), inputBuffer.size())) > 0)
    {
        outputLength = aDecryptor.update(outputBuffer, inputBuffer, inputLength);
        sal_uInt32 writeLength = std::min(outputLength, remaining);
        aOutputStream.writeMemory(outputBuffer.data(), writeLength);
        remaining -= outputLength;
    }

    rxOutputStream->flush();

    return true;
}

sal_Bool Standard2007Engine::checkDataIntegrity()
{
    return true;
}

css::uno::Sequence<css::beans::NamedValue> Standard2007Engine::createEncryptionData(const OUString& rPassword)
{
    comphelper::SequenceAsHashMap aEncryptionData;
    aEncryptionData["OOXPassword"] <<= rPassword;
    aEncryptionData["CryptoType"] <<= OUString("Standard2007Engine");

    return aEncryptionData.getAsConstNamedValueList();
}

sal_Bool Standard2007Engine::setupEncryption(const css::uno::Sequence<css::beans::NamedValue>& rMediaEncData)
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

    OUString sPassword;
    for (int i = 0; i < rMediaEncData.getLength(); i++)
    {
        if (rMediaEncData[i].Name == "OOXPassword")
        {
            OUString sCryptoType;
            rMediaEncData[i].Value >>= sPassword;
        }
    }

    if (!calculateEncryptionKey(sPassword))
        return false;

    if (!generateVerifier())
        return false;

    return true;
}

css::uno::Sequence<sal_Int8> Standard2007Engine::writeEncryptionInfo()
{
    Reference<XOutputStream> aEncryptionInfoStream(
        mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream rStream(aEncryptionInfoStream, false);

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

    rStream.close();
    aEncryptionInfoStream->flush();

    Reference<XSequenceOutputStream> aEncryptionInfoSequenceStream(aEncryptionInfoStream, UNO_QUERY);
    return aEncryptionInfoSequenceStream->getWrittenBytes();
}

css::uno::Sequence<sal_Int8> Standard2007Engine::writeEncryptedDocument(const css::uno::Reference<css::io::XInputStream> &  rxInputStream)
{
    Reference<XOutputStream> aOutputStream(
        mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream aBinaryOutputStream(aOutputStream, false);

    BinaryXInputStream aBinaryInputStream(rxInputStream, false);
    Reference<XSeekable> xSeekable(rxInputStream, UNO_QUERY);

    aBinaryOutputStream.WriteUInt32(xSeekable->getLength()); // size
    aBinaryOutputStream.WriteUInt32(0U);    // reserved

    std::vector<sal_uInt8> inputBuffer(1024);
    std::vector<sal_uInt8> outputBuffer(1024);

    sal_uInt32 inputLength;
    sal_uInt32 outputLength;

    std::vector<sal_uInt8> iv;
    Encrypt aEncryptor(mKey, iv, Crypto::AES_128_ECB);

    while ((inputLength = aBinaryInputStream.readMemory(inputBuffer.data(), inputBuffer.size())) > 0)
    {
        // increase size to multiple of 16 (size of mKey) if necessary
        inputLength = inputLength % AES128Size == 0 ?
                            inputLength : roundUp(inputLength, AES128Size);
        outputLength = aEncryptor.update(outputBuffer, inputBuffer, inputLength);
        aBinaryOutputStream.writeMemory(outputBuffer.data(), outputLength);
    }

    Reference<XSequenceOutputStream> aSequenceStream(aOutputStream, UNO_QUERY);
    return aSequenceStream->getWrittenBytes();
}

css::uno::Sequence<css::beans::NamedValue> Standard2007Engine::encrypt(const css::uno::Reference<css::io::XInputStream> &  rxInputStream)
{
    if (mKey.empty())
        return css::uno::Sequence<css::beans::NamedValue>();

    comphelper::SequenceAsHashMap aStreams;

    aStreams["EncryptedPackage"] <<= writeEncryptedDocument(rxInputStream);
    aStreams["EncryptionInfo"] <<= writeEncryptionInfo();
    return aStreams.getAsConstNamedValueList();
}

css::uno::Reference<css::io::XInputStream> Standard2007Engine::getStream(const css::uno::Sequence<css::beans::NamedValue> & rStreams, const OUString sStreamName)
{
    for (const auto & aStream : rStreams)
    {
        if (aStream.Name == sStreamName)
        {
            css::uno::Sequence<sal_Int8> aSeq;
            aStream.Value >>= aSeq;
            Reference<XInputStream> aStream(css::io::SequenceInputStream::createStreamFromSequence(mxContext, aSeq), UNO_QUERY_THROW);
            return aStream;
        }
    }
    return nullptr;
}

sal_Bool Standard2007Engine::readEncryptionInfo(const css::uno::Sequence<css::beans::NamedValue>& aStreams)
{
    Reference<css::io::XInputStream> rxInputStream = getStream(aStreams, "EncryptionInfo");
    BinaryXInputStream aBinaryStream(rxInputStream, false);
    aBinaryStream.readuInt32();    // Version

    mInfo.header.flags = aBinaryStream.readuInt32();
    if (getFlag(mInfo.header.flags, msfilter::ENCRYPTINFO_EXTERNAL))
        return false;

    sal_uInt32 nHeaderSize = aBinaryStream.readuInt32();

    sal_uInt32 actualHeaderSize = sizeof(mInfo.header);

    if (nHeaderSize < actualHeaderSize)
        return false;

    mInfo.header.flags = aBinaryStream.readuInt32();
    mInfo.header.sizeExtra = aBinaryStream.readuInt32();
    mInfo.header.algId = aBinaryStream.readuInt32();
    mInfo.header.algIdHash = aBinaryStream.readuInt32();
    mInfo.header.keyBits = aBinaryStream.readuInt32();
    mInfo.header.providedType = aBinaryStream.readuInt32();
    mInfo.header.reserved1 = aBinaryStream.readuInt32();
    mInfo.header.reserved2 = aBinaryStream.readuInt32();

    aBinaryStream.skip(nHeaderSize - actualHeaderSize);

    mInfo.verifier.saltSize = aBinaryStream.readuInt32();
    aBinaryStream.readArray(mInfo.verifier.salt, SAL_N_ELEMENTS(mInfo.verifier.salt));
    aBinaryStream.readArray(mInfo.verifier.encryptedVerifier, SAL_N_ELEMENTS(mInfo.verifier.encryptedVerifier));
    mInfo.verifier.encryptedVerifierHashSize = aBinaryStream.readuInt32();
    aBinaryStream.readArray(mInfo.verifier.encryptedVerifierHash, SAL_N_ELEMENTS(mInfo.verifier.encryptedVerifierHash));

    if (mInfo.verifier.saltSize != 16)
        return false;

    // check flags and algorithm IDs, required are AES128 and SHA-1
    if (!getFlag(mInfo.header.flags, msfilter::ENCRYPTINFO_CRYPTOAPI))
        return false;

    if (!getFlag(mInfo.header.flags, msfilter::ENCRYPTINFO_AES))
        return false;

    // algorithm ID 0 defaults to AES128 too, if ENCRYPTINFO_AES flag is set
    if (mInfo.header.algId != 0 && mInfo.header.algId != msfilter::ENCRYPT_ALGO_AES128)
        return false;

    // hash algorithm ID 0 defaults to SHA-1 too
    if (mInfo.header.algIdHash != 0 && mInfo.header.algIdHash != msfilter::ENCRYPT_HASH_SHA1)
        return false;

    if (mInfo.verifier.encryptedVerifierHashSize != 20)
        return false;

    return !aBinaryStream.isEof();
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
