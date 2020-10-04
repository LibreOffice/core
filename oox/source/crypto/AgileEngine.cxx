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

#include <sax/tools/converter.hxx>

#include <comphelper/hash.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/random.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/sequence.hxx>

#include <filter/msfilter/mscodec.hxx>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>
#include <sax/fastattribs.hxx>

#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>

using namespace css;
using namespace css::beans;
using namespace css::io;
using namespace css::lang;
using namespace css::uno;
using namespace css::xml::sax;
using namespace css::xml;

namespace oox::crypto {

namespace {

OUString stripNamespacePrefix(OUString const & rsInputName)
{
    return rsInputName.copy(rsInputName.indexOf(":") + 1);
}

class AgileTokenHandler : public sax_fastparser::FastTokenHandlerBase
{
public:
    virtual sal_Int32 SAL_CALL getTokenFromUTF8(const Sequence< sal_Int8 >& /*nIdentifier*/) override
    {
        return FastToken::DONTKNOW;
    }

    virtual Sequence<sal_Int8> SAL_CALL getUTF8Identifier(sal_Int32 /*nToken*/) override
    {
        return Sequence<sal_Int8>();
    }

    virtual sal_Int32 getTokenDirect( const char * /* pToken */, sal_Int32 /* nLength */ ) const override
    {
        return -1;
    }
};

class AgileDocumentHandler : public ::cppu::WeakImplHelper<XFastDocumentHandler>
{
    AgileEncryptionInfo& mInfo;

public:
    explicit AgileDocumentHandler(AgileEncryptionInfo& rInfo) :
        mInfo(rInfo)
    {}

    void SAL_CALL startDocument() override {}
    void SAL_CALL endDocument() override {}
    void SAL_CALL processingInstruction( const OUString& /*rTarget*/, const OUString& /*rData*/ ) override {}
    void SAL_CALL setDocumentLocator( const Reference< XLocator >& /*xLocator*/ ) override {}
    void SAL_CALL startFastElement( sal_Int32 /*Element*/, const Reference< XFastAttributeList >& /*Attribs*/ ) override {}

    void SAL_CALL startUnknownElement( const OUString& /*aNamespace*/, const OUString& rName, const Reference< XFastAttributeList >& aAttributeList ) override
    {
        const OUString& rLocalName = stripNamespacePrefix(rName);

        const css::uno::Sequence<Attribute> aUnknownAttributes = aAttributeList->getUnknownAttributes();
        for (const Attribute& rAttribute : aUnknownAttributes)
        {
            const OUString& rAttrLocalName = stripNamespacePrefix(rAttribute.Name);

            if (rAttrLocalName == "spinCount")
            {
                ::sax::Converter::convertNumber(mInfo.spinCount, rAttribute.Value);
            }
            else if (rAttrLocalName == "saltSize")
            {
                ::sax::Converter::convertNumber(mInfo.saltSize, rAttribute.Value);
            }
            else if (rAttrLocalName == "blockSize")
            {
                ::sax::Converter::convertNumber(mInfo.blockSize, rAttribute.Value);
            }
            else if (rAttrLocalName == "keyBits")
            {
                ::sax::Converter::convertNumber(mInfo.keyBits, rAttribute.Value);
            }
            else if (rAttrLocalName == "hashSize")
            {
                ::sax::Converter::convertNumber(mInfo.hashSize, rAttribute.Value);
            }
            else if (rAttrLocalName == "cipherAlgorithm")
            {
                mInfo.cipherAlgorithm = rAttribute.Value;
            }
            else if (rAttrLocalName == "cipherChaining")
            {
                mInfo.cipherChaining = rAttribute.Value;
            }
            else if (rAttrLocalName == "hashAlgorithm")
            {
                mInfo.hashAlgorithm = rAttribute.Value;
            }
            else if (rAttrLocalName == "saltValue")
            {
                Sequence<sal_Int8> saltValue;
                comphelper::Base64::decode(saltValue, rAttribute.Value);
                if (rLocalName == "encryptedKey")
                    mInfo.saltValue = comphelper::sequenceToContainer<std::vector<sal_uInt8>>(saltValue);
                else if (rLocalName == "keyData")
                    mInfo.keyDataSalt = comphelper::sequenceToContainer<std::vector<sal_uInt8>>(saltValue);
            }
            else if (rAttrLocalName == "encryptedVerifierHashInput")
            {
                Sequence<sal_Int8> encryptedVerifierHashInput;
                comphelper::Base64::decode(encryptedVerifierHashInput, rAttribute.Value);
                mInfo.encryptedVerifierHashInput = comphelper::sequenceToContainer<std::vector<sal_uInt8>>(encryptedVerifierHashInput);
            }
            else if (rAttrLocalName == "encryptedVerifierHashValue")
            {
                Sequence<sal_Int8> encryptedVerifierHashValue;
                comphelper::Base64::decode(encryptedVerifierHashValue, rAttribute.Value);
                mInfo.encryptedVerifierHashValue = comphelper::sequenceToContainer<std::vector<sal_uInt8>>(encryptedVerifierHashValue);
            }
            else if (rAttrLocalName == "encryptedKeyValue")
            {
                Sequence<sal_Int8> encryptedKeyValue;
                comphelper::Base64::decode(encryptedKeyValue, rAttribute.Value);
                mInfo.encryptedKeyValue = comphelper::sequenceToContainer<std::vector<sal_uInt8>>(encryptedKeyValue);
            }
            if (rAttrLocalName == "encryptedHmacKey")
            {
                Sequence<sal_Int8> aValue;
                comphelper::Base64::decode(aValue, rAttribute.Value);
                mInfo.hmacEncryptedKey = comphelper::sequenceToContainer<std::vector<sal_uInt8>>(aValue);
            }
            if (rAttrLocalName == "encryptedHmacValue")
            {
                Sequence<sal_Int8> aValue;
                comphelper::Base64::decode(aValue, rAttribute.Value);
                mInfo.hmacEncryptedValue = comphelper::sequenceToContainer<std::vector<sal_uInt8>>(aValue);
            }
        }
    }

    void SAL_CALL endFastElement( sal_Int32 /*aElement*/ ) override
    {}
    void SAL_CALL endUnknownElement( const OUString& /*aNamespace*/, const OUString& /*aName*/ ) override
    {}

    Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 /*aElement*/, const Reference< XFastAttributeList >& /*aAttribs*/ ) override
    {
        return nullptr;
    }

    Reference< XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& /*aNamespace*/, const OUString& /*aName*/, const Reference< XFastAttributeList >& /*aAttribs*/ ) override
    {
        return this;
    }

    void SAL_CALL characters( const OUString& /*aChars*/ ) override
    {}
};

constexpr const sal_uInt32 constSegmentLength = 4096;

const std::vector<sal_uInt8> constBlock1 { 0xfe, 0xa7, 0xd2, 0x76, 0x3b, 0x4b, 0x9e, 0x79 };
const std::vector<sal_uInt8> constBlock2 { 0xd7, 0xaa, 0x0f, 0x6d, 0x30, 0x61, 0x34, 0x4e };
const std::vector<sal_uInt8> constBlock3 { 0x14, 0x6e, 0x0b, 0xe7, 0xab, 0xac, 0xd0, 0xd6 };
const std::vector<sal_uInt8> constBlockHmac1 { 0x5f, 0xb2, 0xad, 0x01, 0x0c, 0xb9, 0xe1, 0xf6 };
const std::vector<sal_uInt8> constBlockHmac2 { 0xa0, 0x67, 0x7f, 0x02, 0xb2, 0x2c, 0x84, 0x33 };

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

CryptoHashType cryptoHashTypeFromString(OUString const & sAlgorithm)
{
    if (sAlgorithm == "SHA512")
        return CryptoHashType::SHA512;
    return CryptoHashType::SHA1;
}

} // namespace

AgileEngine::AgileEngine()
    : meEncryptionPreset(AgileEncryptionPreset::AES_256_SHA512)
{}

Crypto::CryptoType AgileEngine::cryptoType(const AgileEncryptionInfo& rInfo)
{
    if (rInfo.keyBits == 128 && rInfo.cipherAlgorithm == "AES" && rInfo.cipherChaining == "ChainingModeCBC")
        return Crypto::AES_128_CBC;
    else if (rInfo.keyBits == 256 && rInfo.cipherAlgorithm == "AES" && rInfo.cipherChaining == "ChainingModeCBC")
        return Crypto::AES_256_CBC;
    return Crypto::UNKNOWN;
}

static std::vector<sal_uInt8> calculateIV(comphelper::HashType eType,
                             std::vector<sal_uInt8> const & rSalt,
                             std::vector<sal_uInt8> const & rBlock,
                             sal_Int32 nCipherBlockSize)
{
    comphelper::Hash aHasher(eType);
    aHasher.update(rSalt.data(), rSalt.size());
    aHasher.update(rBlock.data(), rBlock.size());
    std::vector<sal_uInt8> aIV = aHasher.finalize();
    aIV.resize(roundUp(sal_Int32(aIV.size()), nCipherBlockSize), 0x36);
    return aIV;
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

void AgileEngine::encryptBlock(
    std::vector<sal_uInt8> const & rBlock,
    std::vector<sal_uInt8> & rHashFinal,
    std::vector<sal_uInt8> & rInput,
    std::vector<sal_uInt8> & rOutput)
{
    std::vector<sal_uInt8> hash(mInfo.hashSize, 0);
    std::vector<sal_uInt8> dataFinal(mInfo.hashSize + rBlock.size(), 0);
    std::copy(rHashFinal.begin(), rHashFinal.end(), dataFinal.begin());
    std::copy(rBlock.begin(), rBlock.end(), dataFinal.begin() + mInfo.hashSize);

    hashCalc(hash, dataFinal, mInfo.hashAlgorithm);

    sal_Int32 keySize = mInfo.keyBits / 8;
    std::vector<sal_uInt8> key(keySize, 0);

    std::copy(hash.begin(), hash.begin() + keySize, key.begin());

    Encrypt aEncryptor(key, mInfo.saltValue, cryptoType(mInfo));

    aEncryptor.update(rOutput, rInput);
}

void AgileEngine::calculateHashFinal(const OUString& rPassword, std::vector<sal_uInt8>& aHashFinal)
{
    aHashFinal = comphelper::DocPasswordHelper::GetOoxHashAsVector(
                    rPassword, mInfo.saltValue, mInfo.spinCount,
                    comphelper::Hash::IterCount::PREPEND, mInfo.hashAlgorithm);
}

namespace
{

bool generateBytes(std::vector<sal_uInt8> & rBytes, sal_Int32 nSize)
{
    size_t nMax = std::min(rBytes.size(), size_t(nSize));

    for (size_t i = 0; i < nMax; ++i)
    {
        rBytes[i] = sal_uInt8(comphelper::rng::uniform_uint_distribution(0, 0xFF));
    }

    return true;
}

} // end anonymous namespace

bool AgileEngine::decryptAndCheckVerifierHash(OUString const & rPassword)
{
    std::vector<sal_uInt8>& encryptedHashValue = mInfo.encryptedVerifierHashValue;
    size_t encryptedHashValueSize = encryptedHashValue.size();
    size_t nHashValueSize = mInfo.hashSize;
    if (nHashValueSize > encryptedHashValueSize)
        return false;

    std::vector<sal_uInt8> hashFinal(nHashValueSize, 0);
    calculateHashFinal(rPassword, hashFinal);

    std::vector<sal_uInt8>& encryptedHashInput = mInfo.encryptedVerifierHashInput;
    // SALT - needs to be a multiple of block size (?)
    sal_Int32 nSaltSize = roundUp(mInfo.saltSize, mInfo.blockSize);
    std::vector<sal_uInt8> hashInput(nSaltSize, 0);
    calculateBlock(constBlock1, hashFinal, encryptedHashInput, hashInput);

    std::vector<sal_uInt8> hashValue(encryptedHashValueSize, 0);
    calculateBlock(constBlock2, hashFinal, encryptedHashValue, hashValue);

    std::vector<sal_uInt8> hash(nHashValueSize, 0);
    hashCalc(hash, hashInput, mInfo.hashAlgorithm);

    return std::equal(hash.begin(), hash.end(), hashValue.begin());
}

void AgileEngine::decryptEncryptionKey(OUString const & rPassword)
{
    sal_Int32 nKeySize = mInfo.keyBits / 8;

    mKey.clear();
    mKey.resize(nKeySize, 0);

    std::vector<sal_uInt8> aPasswordHash(mInfo.hashSize, 0);
    calculateHashFinal(rPassword, aPasswordHash);

    calculateBlock(constBlock3, aPasswordHash, mInfo.encryptedKeyValue, mKey);
}

// TODO: Rename
bool AgileEngine::generateEncryptionKey(OUString const & rPassword)
{
    bool bResult = decryptAndCheckVerifierHash(rPassword);

    if (bResult)
    {
        decryptEncryptionKey(rPassword);
        decryptHmacKey();
        decryptHmacValue();
    }
    return bResult;
}

bool AgileEngine::decryptHmacKey()
{
    // Initialize hmacKey
    mInfo.hmacKey.clear();
    mInfo.hmacKey.resize(mInfo.hmacEncryptedKey.size(), 0);

    // Calculate IV
    comphelper::HashType eType;
    if (mInfo.hashAlgorithm == "SHA1")
        eType = comphelper::HashType::SHA1;
    else if (mInfo.hashAlgorithm == "SHA512")
        eType = comphelper::HashType::SHA512;
    else
        return false;

    std::vector<sal_uInt8> iv = calculateIV(eType, mInfo.keyDataSalt, constBlockHmac1, mInfo.blockSize);

    // Decrypt without key, calculated iv
    Decrypt aDecrypt(mKey, iv, cryptoType(mInfo));
    aDecrypt.update(mInfo.hmacKey, mInfo.hmacEncryptedKey);

    mInfo.hmacKey.resize(mInfo.hashSize, 0);

    return true;
}

bool AgileEngine::decryptHmacValue()
{
    // Initialize hmacHash
    mInfo.hmacHash.clear();
    mInfo.hmacHash.resize(mInfo.hmacEncryptedValue.size(), 0);

    // Calculate IV
    comphelper::HashType eType;
    if (mInfo.hashAlgorithm == "SHA1")
        eType = comphelper::HashType::SHA1;
    else if (mInfo.hashAlgorithm == "SHA512")
        eType = comphelper::HashType::SHA512;
    else
        return false;
    std::vector<sal_uInt8> iv = calculateIV(eType, mInfo.keyDataSalt, constBlockHmac2, mInfo.blockSize);

    // Decrypt without key, calculated iv
    Decrypt aDecrypt(mKey, iv, cryptoType(mInfo));
    aDecrypt.update(mInfo.hmacHash, mInfo.hmacEncryptedValue);

    mInfo.hmacHash.resize(mInfo.hashSize, 0);

    return true;
}

bool AgileEngine::checkDataIntegrity()
{
    bool bResult = (mInfo.hmacHash.size() == mInfo.hmacCalculatedHash.size() &&
               std::equal(mInfo.hmacHash.begin(), mInfo.hmacHash.end(), mInfo.hmacCalculatedHash.begin()));

    return bResult;
}

bool AgileEngine::decrypt(BinaryXInputStream& aInputStream,
                          BinaryXOutputStream& aOutputStream)
{
    CryptoHash aCryptoHash(mInfo.hmacKey, cryptoHashTypeFromString(mInfo.hashAlgorithm));

    sal_uInt32 totalSize = aInputStream.readuInt32(); // Document unencrypted size - 4 bytes
    // account for size in HMAC
    std::vector<sal_uInt8> aSizeBytes(sizeof(sal_uInt32));
    ByteOrderConverter::writeLittleEndian(aSizeBytes.data(), totalSize);
    aCryptoHash.update(aSizeBytes);

    aInputStream.skip(4);  // Reserved 4 Bytes
    // account for reserved 4 bytes (must be 0)
    std::vector<sal_uInt8> aReserved{0,0,0,0};
    aCryptoHash.update(aReserved);

    // setup decryption
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

    while ((inputLength = aInputStream.readMemory(inputBuffer.data(), inputBuffer.size())) > 0)
    {
        auto p = saltWithBlockKey.begin() + saltSize;
        p[0] = segment & 0xFF;
        p[1] = (segment >> 8) & 0xFF;
        p[2] = (segment >> 16) & 0xFF;
        p[3] = segment >> 24;

        hashCalc(hash, saltWithBlockKey, mInfo.hashAlgorithm);

        // Only if hash > keySize
        std::copy(hash.begin(), hash.begin() + keySize, iv.begin());

        Decrypt aDecryptor(mKey, iv, AgileEngine::cryptoType(mInfo));
        outputLength = aDecryptor.update(outputBuffer, inputBuffer, inputLength);

        sal_uInt32 writeLength = std::min(outputLength, remaining);

        aCryptoHash.update(inputBuffer, inputLength);

        aOutputStream.writeMemory(outputBuffer.data(), writeLength);

        remaining -= outputLength;
        segment++;
    }

    mInfo.hmacCalculatedHash = aCryptoHash.finalize();

    return true;
}

bool AgileEngine::readEncryptionInfo(uno::Reference<io::XInputStream> & rxInputStream)
{
    // Check reserved value
    std::vector<sal_uInt8> aExpectedReservedBytes(sizeof(sal_uInt32));
    ByteOrderConverter::writeLittleEndian(aExpectedReservedBytes.data(), msfilter::AGILE_ENCRYPTION_RESERVED);

    uno::Sequence<sal_Int8> aReadReservedBytes(sizeof(sal_uInt32));
    rxInputStream->readBytes(aReadReservedBytes, aReadReservedBytes.getLength());

    if (!std::equal(aReadReservedBytes.begin(), aReadReservedBytes.end(), aExpectedReservedBytes.begin()))
        return false;

    mInfo.spinCount = 0;
    mInfo.saltSize = 0;
    mInfo.keyBits = 0;
    mInfo.hashSize = 0;
    mInfo.blockSize = 0;

    Reference<XFastDocumentHandler> xFastDocumentHandler(new AgileDocumentHandler(mInfo));
    Reference<XFastTokenHandler> xFastTokenHandler(new AgileTokenHandler);

    Reference<XFastParser> xParser(css::xml::sax::FastParser::create(comphelper::getProcessComponentContext()));

    xParser->setFastDocumentHandler(xFastDocumentHandler);
    xParser->setTokenHandler(xFastTokenHandler);

    InputSource aInputSource;
    aInputSource.aInputStream = rxInputStream;
    xParser->parseStream(aInputSource);

    // CHECK info data
    if (2 > mInfo.blockSize || mInfo.blockSize > 4096)
        return false;

    if (0 > mInfo.spinCount || mInfo.spinCount > 10000000)
        return false;

    if (1 > mInfo.saltSize|| mInfo.saltSize > 65536) // Check
        return false;

    // AES 128 CBC with SHA1
    if (mInfo.keyBits         == 128 &&
        mInfo.cipherAlgorithm == "AES" &&
        mInfo.cipherChaining  == "ChainingModeCBC" &&
        mInfo.hashAlgorithm   == "SHA1" &&
        mInfo.hashSize        == comphelper::SHA1_HASH_LENGTH)
    {
        return true;
    }

    // AES 256 CBC with SHA512
    if (mInfo.keyBits         == 256 &&
        mInfo.cipherAlgorithm == "AES" &&
        mInfo.cipherChaining  == "ChainingModeCBC" &&
        mInfo.hashAlgorithm   == "SHA512" &&
        mInfo.hashSize        == comphelper::SHA512_HASH_LENGTH)
    {
        return true;
    }

    return false;
}

bool AgileEngine::generateAndEncryptVerifierHash(OUString const & rPassword)
{
    if (!generateBytes(mInfo.saltValue, mInfo.saltSize))
        return false;

    std::vector<sal_uInt8> unencryptedVerifierHashInput(mInfo.saltSize);
    if (!generateBytes(unencryptedVerifierHashInput, mInfo.saltSize))
        return false;

    // HASH - needs to be modified to be multiple of block size
    sal_Int32 nVerifierHash = roundUp(mInfo.hashSize, mInfo.blockSize);
    std::vector<sal_uInt8> unencryptedVerifierHashValue;
    if (!hashCalc(unencryptedVerifierHashValue, unencryptedVerifierHashInput, mInfo.hashAlgorithm))
        return false;
    unencryptedVerifierHashValue.resize(nVerifierHash, 0);

    std::vector<sal_uInt8> hashFinal(mInfo.hashSize, 0);
    calculateHashFinal(rPassword, hashFinal);

    encryptBlock(constBlock1, hashFinal, unencryptedVerifierHashInput, mInfo.encryptedVerifierHashInput);

    encryptBlock(constBlock2, hashFinal, unencryptedVerifierHashValue, mInfo.encryptedVerifierHashValue);

    return true;
}

bool AgileEngine::encryptHmacKey()
{
    // Initialize hmacKey
    mInfo.hmacKey.clear();
    mInfo.hmacKey.resize(mInfo.hashSize, 0);

    if (!generateBytes(mInfo.hmacKey, mInfo.hashSize))
        return false;

    // Encrypted salt must be multiple of block size
    sal_Int32 nEncryptedSaltSize = oox::crypto::roundUp(mInfo.hashSize, mInfo.blockSize);

    // We need to extend hmacSalt to multiple of block size, padding with 0x36
    std::vector<sal_uInt8> extendedSalt(mInfo.hmacKey);
    extendedSalt.resize(nEncryptedSaltSize, 0x36);

    // Initialize hmacEncryptedKey
    mInfo.hmacEncryptedKey.clear();
    mInfo.hmacEncryptedKey.resize(nEncryptedSaltSize, 0);

    // Calculate IV
    comphelper::HashType eType;
    if (mInfo.hashAlgorithm == "SHA1")
        eType = comphelper::HashType::SHA1;
    else if (mInfo.hashAlgorithm == "SHA512")
        eType = comphelper::HashType::SHA512;
    else
        return false;

    std::vector<sal_uInt8> iv = calculateIV(eType, mInfo.keyDataSalt, constBlockHmac1, mInfo.blockSize);

    // Encrypt without key, calculated iv
    Encrypt aEncryptor(mKey, iv, cryptoType(mInfo));
    aEncryptor.update(mInfo.hmacEncryptedKey, extendedSalt);

    return true;
}

bool AgileEngine::encryptHmacValue()
{
    sal_Int32 nEncryptedValueSize = roundUp(mInfo.hashSize, mInfo.blockSize);
    mInfo.hmacEncryptedValue.clear();
    mInfo.hmacEncryptedValue.resize(nEncryptedValueSize, 0);

    std::vector<sal_uInt8> extendedHash(mInfo.hmacHash);
    extendedHash.resize(nEncryptedValueSize, 0x36);

    // Calculate IV
    comphelper::HashType eType;
    if (mInfo.hashAlgorithm == "SHA1")
        eType = comphelper::HashType::SHA1;
    else if (mInfo.hashAlgorithm == "SHA512")
        eType = comphelper::HashType::SHA512;
    else
        return false;

    std::vector<sal_uInt8> iv = calculateIV(eType, mInfo.keyDataSalt, constBlockHmac2, mInfo.blockSize);

    // Encrypt without key, calculated iv
    Encrypt aEncryptor(mKey, iv, cryptoType(mInfo));
    aEncryptor.update(mInfo.hmacEncryptedValue, extendedHash);

    return true;
}

bool AgileEngine::encryptEncryptionKey(OUString const & rPassword)
{
    sal_Int32 nKeySize = mInfo.keyBits / 8;

    mKey.clear();
    mKey.resize(nKeySize, 0);

    mInfo.encryptedKeyValue.clear();
    mInfo.encryptedKeyValue.resize(nKeySize, 0);

    if (!generateBytes(mKey, nKeySize))
        return false;

    std::vector<sal_uInt8> aPasswordHash(mInfo.hashSize, 0);
    calculateHashFinal(rPassword, aPasswordHash);

    encryptBlock(constBlock3, aPasswordHash, mKey, mInfo.encryptedKeyValue);

    return true;
}

bool AgileEngine::setupEncryption(OUString const & rPassword)
{
    if (meEncryptionPreset == AgileEncryptionPreset::AES_128_SHA1)
        setupEncryptionParameters({ 100000, 16, 128, 20, 16, OUString("AES"), OUString("ChainingModeCBC"), OUString("SHA1") });
    else
        setupEncryptionParameters({ 100000, 16, 256, 64, 16, OUString("AES"), OUString("ChainingModeCBC"), OUString("SHA512") });

    return setupEncryptionKey(rPassword);
}

void AgileEngine::setupEncryptionParameters(AgileEncryptionParameters const & rAgileEncryptionParameters)
{
    mInfo.spinCount = rAgileEncryptionParameters.spinCount;
    mInfo.saltSize = rAgileEncryptionParameters.saltSize;
    mInfo.keyBits = rAgileEncryptionParameters.keyBits;
    mInfo.hashSize = rAgileEncryptionParameters.hashSize;
    mInfo.blockSize = rAgileEncryptionParameters.blockSize;

    mInfo.cipherAlgorithm = rAgileEncryptionParameters.cipherAlgorithm;
    mInfo.cipherChaining = rAgileEncryptionParameters.cipherChaining;
    mInfo.hashAlgorithm = rAgileEncryptionParameters.hashAlgorithm;

    mInfo.keyDataSalt.resize(mInfo.saltSize);
    mInfo.saltValue.resize(mInfo.saltSize);
    mInfo.encryptedVerifierHashInput.resize(mInfo.saltSize);
    mInfo.encryptedVerifierHashValue.resize(roundUp(mInfo.hashSize, mInfo.blockSize), 0);
}

bool AgileEngine::setupEncryptionKey(OUString const & rPassword)
{
    if (!generateAndEncryptVerifierHash(rPassword))
        return false;
    if (!encryptEncryptionKey(rPassword))
        return false;
    if (!generateBytes(mInfo.keyDataSalt, mInfo.saltSize))
        return false;
    if (!encryptHmacKey())
        return false;
    return true;
}

void AgileEngine::writeEncryptionInfo(BinaryXOutputStream & rStream)
{
    rStream.WriteUInt32(msfilter::VERSION_INFO_AGILE);
    rStream.WriteUInt32(msfilter::AGILE_ENCRYPTION_RESERVED);

    SvMemoryStream aMemStream;
    tools::XmlWriter aXmlWriter(&aMemStream);

    if (aXmlWriter.startDocument(0/*nIndent*/))
    {
        aXmlWriter.startElement("", "encryption", "http://schemas.microsoft.com/office/2006/encryption");
        aXmlWriter.attribute("xmlns:p", OString("http://schemas.microsoft.com/office/2006/keyEncryptor/password"));

        aXmlWriter.startElement("keyData");
        aXmlWriter.attribute("saltSize", mInfo.saltSize);
        aXmlWriter.attribute("blockSize", mInfo.blockSize);
        aXmlWriter.attribute("keyBits", mInfo.keyBits);
        aXmlWriter.attribute("hashSize", mInfo.hashSize);
        aXmlWriter.attribute("cipherAlgorithm", mInfo.cipherAlgorithm);
        aXmlWriter.attribute("cipherChaining", mInfo.cipherChaining);
        aXmlWriter.attribute("hashAlgorithm", mInfo.hashAlgorithm);
        aXmlWriter.attributeBase64("saltValue", mInfo.keyDataSalt);
        aXmlWriter.endElement();

        aXmlWriter.startElement("dataIntegrity");
        aXmlWriter.attributeBase64("encryptedHmacKey", mInfo.hmacEncryptedKey);
        aXmlWriter.attributeBase64("encryptedHmacValue", mInfo.hmacEncryptedValue);
        aXmlWriter.endElement();

        aXmlWriter.startElement("keyEncryptors");
        aXmlWriter.startElement("keyEncryptor");
        aXmlWriter.attribute("uri", OString("http://schemas.microsoft.com/office/2006/keyEncryptor/password"));

        aXmlWriter.startElement("p", "encryptedKey", "");
        aXmlWriter.attribute("spinCount", mInfo.spinCount);
        aXmlWriter.attribute("saltSize", mInfo.saltSize);
        aXmlWriter.attribute("blockSize", mInfo.blockSize);
        aXmlWriter.attribute("keyBits", mInfo.keyBits);
        aXmlWriter.attribute("hashSize", mInfo.hashSize);
        aXmlWriter.attribute("cipherAlgorithm", mInfo.cipherAlgorithm);
        aXmlWriter.attribute("cipherChaining", mInfo.cipherChaining);
        aXmlWriter.attribute("hashAlgorithm", mInfo.hashAlgorithm);
        aXmlWriter.attributeBase64("saltValue", mInfo.saltValue);
        aXmlWriter.attributeBase64("encryptedVerifierHashInput", mInfo.encryptedVerifierHashInput);
        aXmlWriter.attributeBase64("encryptedVerifierHashValue", mInfo.encryptedVerifierHashValue);
        aXmlWriter.attributeBase64("encryptedKeyValue", mInfo.encryptedKeyValue);
        aXmlWriter.endElement();

        aXmlWriter.endElement();
        aXmlWriter.endElement();

        aXmlWriter.endElement();
        aXmlWriter.endDocument();
    }
    rStream.writeMemory(aMemStream.GetData(), aMemStream.GetSize());
}

void AgileEngine::encrypt(const css::uno::Reference<css::io::XInputStream> &  rxInputStream,
                          css::uno::Reference<css::io::XOutputStream> & rxOutputStream,
                          sal_uInt32 nSize)
{
    CryptoHash aCryptoHash(mInfo.hmacKey, cryptoHashTypeFromString(mInfo.hashAlgorithm));

    BinaryXOutputStream aBinaryOutputStream(rxOutputStream, false);
    BinaryXInputStream aBinaryInputStream(rxInputStream, false);

    std::vector<sal_uInt8> aSizeBytes(sizeof(sal_uInt32));
    ByteOrderConverter::writeLittleEndian(aSizeBytes.data(), nSize);
    aBinaryOutputStream.writeMemory(aSizeBytes.data(), aSizeBytes.size()); // size
    aCryptoHash.update(aSizeBytes, aSizeBytes.size());

    std::vector<sal_uInt8> aNull{0,0,0,0};
    aBinaryOutputStream.writeMemory(aNull.data(), aNull.size()); // reserved
    aCryptoHash.update(aNull, aNull.size());

    std::vector<sal_uInt8>& keyDataSalt = mInfo.keyDataSalt;

    sal_uInt32 saltSize = mInfo.saltSize;
    sal_uInt32 keySize = mInfo.keyBits / 8;

    sal_uInt32 nSegment = 0;
    sal_uInt32 nSegmentByteSize = sizeof(nSegment);

    std::vector<sal_uInt8> saltWithBlockKey(saltSize + nSegmentByteSize, 0);
    std::copy(keyDataSalt.begin(), keyDataSalt.end(), saltWithBlockKey.begin());

    std::vector<sal_uInt8> hash(mInfo.hashSize, 0);
    std::vector<sal_uInt8> iv(keySize, 0);

    std::vector<sal_uInt8> inputBuffer(constSegmentLength);
    std::vector<sal_uInt8> outputBuffer(constSegmentLength);
    sal_uInt32 inputLength;
    sal_uInt32 outputLength;

    while ((inputLength = aBinaryInputStream.readMemory(inputBuffer.data(), inputBuffer.size())) > 0)
    {
        sal_uInt32 correctedInputLength = inputLength % mInfo.blockSize == 0 ?
                        inputLength : oox::crypto::roundUp(inputLength, sal_uInt32(mInfo.blockSize));

        // Update Key
        auto p = saltWithBlockKey.begin() + saltSize;
        p[0] = nSegment & 0xFF;
        p[1] = (nSegment >> 8) & 0xFF;
        p[2] = (nSegment >> 16) & 0xFF;
        p[3] = nSegment >> 24;

        hashCalc(hash, saltWithBlockKey, mInfo.hashAlgorithm);

        // Only if hash > keySize
        std::copy(hash.begin(), hash.begin() + keySize, iv.begin());

        Encrypt aEncryptor(mKey, iv, AgileEngine::cryptoType(mInfo));
        outputLength = aEncryptor.update(outputBuffer, inputBuffer, correctedInputLength);
        aBinaryOutputStream.writeMemory(outputBuffer.data(), outputLength);
        aCryptoHash.update(outputBuffer, outputLength);

        nSegment++;
    }
    mInfo.hmacHash = aCryptoHash.finalize();
    encryptHmacValue();
}

} // namespace oox::crypto

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
