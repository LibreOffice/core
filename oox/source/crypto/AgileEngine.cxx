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

#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
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

namespace oox {
namespace core {

namespace {

OUString stripNamespacePrefix(OUString const & rsInputName)
{
    return rsInputName.copy(rsInputName.indexOf(":") + 1);
}

class AgileTokenHandler : public cppu::WeakImplHelper<XFastTokenHandler>
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

        for (const Attribute& rAttribute : aAttributeList->getUnknownAttributes())
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
    aHashFinal = comphelper::DocPasswordHelper::GetOoxHashAsVector( rPassword, mInfo.saltValue,
            mInfo.spinCount, comphelper::Hash::IterCount::PREPEND, mInfo.hashAlgorithm);
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

bool AgileEngine::readEncryptionInfo(uno::Reference<io::XInputStream> & rxInputStream)
{
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
        mInfo.hashSize        == msfilter::SHA1_HASH_LENGTH)
    {
        return true;
    }

    // AES 256 CBC with SHA512
    if (mInfo.keyBits         == 256 &&
        mInfo.cipherAlgorithm == "AES" &&
        mInfo.cipherChaining  == "ChainingModeCBC" &&
        mInfo.hashAlgorithm   == "SHA512" &&
        mInfo.hashSize        == msfilter::SHA512_HASH_LENGTH)
    {
        return true;
    }

    return false;
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
