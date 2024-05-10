/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_oox.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <algorithm>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>

#include <oox/crypto/Standard2007Engine.hxx>
#include <oox/crypto/AgileEngine.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif

using namespace css;

class CryptoTest : public CppUnit::TestFixture
{
public:
    virtual ~CryptoTest() override;
    void testCryptoHash();
    void testRoundUp();
    void testStandard2007();
    void testAgileEncryptionVerifier();
    void testAgileEncryptionInfoWritingAndParsing();
    void testAgileDataIntegrityHmacKey();
    void testAgileEncryptingAndDecrypting();

    CPPUNIT_TEST_SUITE(CryptoTest);
    CPPUNIT_TEST(testCryptoHash);
    CPPUNIT_TEST(testRoundUp);
    CPPUNIT_TEST(testStandard2007);
    CPPUNIT_TEST(testAgileEncryptionVerifier);
    CPPUNIT_TEST(testAgileEncryptionInfoWritingAndParsing);
    CPPUNIT_TEST(testAgileDataIntegrityHmacKey);
    CPPUNIT_TEST(testAgileEncryptingAndDecrypting);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{
std::string toString(std::vector<sal_uInt8> const& aInput)
{
    std::stringstream aStream;
    for (auto const& aValue : aInput)
    {
        aStream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(aValue);
    }

    return aStream.str();
}
}

CryptoTest::~CryptoTest()
{
#if USE_TLS_NSS
    NSS_Shutdown();
#endif
}

void CryptoTest::testCryptoHash()
{
    // Check examples from Wikipedia (https://en.wikipedia.org/wiki/HMAC)
    OString aContentString("The quick brown fox jumps over the lazy dog"_ostr);
    std::vector<sal_uInt8> aContent(aContentString.getStr(),
                                    aContentString.getStr() + aContentString.getLength());
    std::vector<sal_uInt8> aKey = { 'k', 'e', 'y' };
    {
        oox::crypto::CryptoHash aCryptoHash(aKey, oox::crypto::CryptoHashType::SHA1);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(std::string("de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9"),
                             toString(aHash));
    }

    {
        oox::crypto::CryptoHash aCryptoHash(aKey, oox::crypto::CryptoHashType::SHA256);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(
            std::string("f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8"),
            toString(aHash));
    }

    {
        oox::crypto::CryptoHash aCryptoHash(aKey, oox::crypto::CryptoHashType::SHA384);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(std::string("d7f4727e2c0b39ae0f1e40cc96f60242d5b7801841cea6fc592c5d3e1"
                                         "ae50700582a96cf35e1e554995fe4e03381c237"),
                             toString(aHash));
    }

    {
        oox::crypto::CryptoHash aCryptoHash(aKey, oox::crypto::CryptoHashType::SHA512);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(
            std::string("b42af09057bac1e2d41708e48a902e09b5ff7f12ab428a4fe86653c73dd248fb82f948a549"
                        "f7b791a5b41915ee4d1ec3935357e4e2317250d0372afa2ebeeb3a"),
            toString(aHash));
    }
}

void CryptoTest::testRoundUp()
{
    CPPUNIT_ASSERT_EQUAL(16, oox::crypto::roundUp(16, 16));
    CPPUNIT_ASSERT_EQUAL(32, oox::crypto::roundUp(32, 16));
    CPPUNIT_ASSERT_EQUAL(64, oox::crypto::roundUp(64, 16));

    CPPUNIT_ASSERT_EQUAL(16, oox::crypto::roundUp(01, 16));
    CPPUNIT_ASSERT_EQUAL(32, oox::crypto::roundUp(17, 16));
    CPPUNIT_ASSERT_EQUAL(32, oox::crypto::roundUp(31, 16));
}

void CryptoTest::testStandard2007()
{
    oox::crypto::Standard2007Engine aEngine;
    {
        aEngine.setupEncryption(u"Password"_ustr);

        SvMemoryStream aEncryptionInfo;
        oox::BinaryXOutputStream aBinaryEncryptionInfoOutputStream(
            new utl::OSeekableOutputStreamWrapper(aEncryptionInfo), false);

        aEngine.writeEncryptionInfo(aBinaryEncryptionInfoOutputStream);
        aBinaryEncryptionInfoOutputStream.close();

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(224), aEncryptionInfo.GetSize());
    }

    SvMemoryStream aUnencryptedInput;
    SvMemoryStream aEncryptedStream;

    OString aTestString = "1234567890ABCDEFG"_ostr;

    aUnencryptedInput.WriteBytes(aTestString.getStr(), aTestString.getLength() + 1);
    aUnencryptedInput.Seek(STREAM_SEEK_TO_BEGIN);

    {
        uno::Reference<io::XInputStream> xInputStream(
            new utl::OSeekableInputStreamWrapper(aUnencryptedInput));
        uno::Reference<io::XOutputStream> xOutputStream(
            new utl::OSeekableOutputStreamWrapper(aEncryptedStream));

        aEngine.encrypt(xInputStream, xOutputStream, aUnencryptedInput.GetSize());

        xOutputStream->flush();

        const sal_uInt8* pData = static_cast<const sal_uInt8*>(aEncryptedStream.GetData());
        sal_uInt64 nSize = aEncryptedStream.GetSize();

        std::vector<sal_uInt8> aData(nSize);
        std::copy(pData, pData + nSize, aData.data());

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(40), nSize);
    }

    aEncryptedStream.Seek(STREAM_SEEK_TO_BEGIN);
    SvMemoryStream aUnencryptedOutput;

    {
        oox::BinaryXInputStream aBinaryInputStream(
            new utl::OSeekableInputStreamWrapper(aEncryptedStream), true);
        oox::BinaryXOutputStream aBinaryOutputStream(
            new utl::OSeekableOutputStreamWrapper(aUnencryptedOutput), true);

        aEngine.decrypt(aBinaryInputStream, aBinaryOutputStream);
        aBinaryOutputStream.close();
        aBinaryInputStream.close();

        const char* pData = static_cast<const char*>(aUnencryptedOutput.GetData());
        sal_uInt64 nSize = aUnencryptedOutput.GetSize();

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(18), nSize);

        OString aString(pData);

        CPPUNIT_ASSERT_EQUAL(aTestString, aString);
    }
}

void CryptoTest::testAgileEncryptionVerifier()
{
    oox::crypto::AgileEngine aEngine;

    OUString aPassword(u"Password"_ustr);

    aEngine.setupEncryptionParameters(
        { 100000, 16, 128, 20, 16, u"AES"_ustr, u"ChainingModeCBC"_ustr, u"SHA1"_ustr });

    CPPUNIT_ASSERT_EQUAL(true, aEngine.generateAndEncryptVerifierHash(aPassword));
    CPPUNIT_ASSERT_EQUAL(false, aEngine.decryptAndCheckVerifierHash(u"Wrong"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, aEngine.decryptAndCheckVerifierHash(aPassword));

    aEngine.setupEncryptionParameters(
        { 100000, 16, 128, 48, 16, u"AES"_ustr, u"ChainingModeCBC"_ustr, u"SHA384"_ustr });

    CPPUNIT_ASSERT_EQUAL(true, aEngine.generateAndEncryptVerifierHash(aPassword));
    CPPUNIT_ASSERT_EQUAL(false, aEngine.decryptAndCheckVerifierHash(u"Wrong"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, aEngine.decryptAndCheckVerifierHash(aPassword));

    aEngine.setupEncryptionParameters(
        { 100000, 16, 256, 64, 16, u"AES"_ustr, u"ChainingModeCBC"_ustr, u"SHA512"_ustr });

    CPPUNIT_ASSERT_EQUAL(true, aEngine.generateAndEncryptVerifierHash(aPassword));
    CPPUNIT_ASSERT_EQUAL(false, aEngine.decryptAndCheckVerifierHash(u"Wrong"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, aEngine.decryptAndCheckVerifierHash(aPassword));
}

void CryptoTest::testAgileEncryptionInfoWritingAndParsing()
{
    OUString aPassword(u"Password"_ustr);
    std::vector<sal_uInt8> aKeyDataSalt;

    { // Preset AES128 - SHA1
        SvMemoryStream aEncryptionInfo;
        {
            oox::crypto::AgileEngine aEngine;

            aEngine.setPreset(oox::crypto::AgileEncryptionPreset::AES_128_SHA1);
            aEngine.setupEncryption(aPassword);
            aKeyDataSalt = aEngine.getInfo().keyDataSalt;

            oox::BinaryXOutputStream aBinaryEncryptionInfoOutputStream(
                new utl::OSeekableOutputStreamWrapper(aEncryptionInfo), true);

            aEngine.writeEncryptionInfo(aBinaryEncryptionInfoOutputStream);
            aBinaryEncryptionInfoOutputStream.close();

            CPPUNIT_ASSERT_EQUAL(sal_uInt64(996), aEncryptionInfo.GetSize());
        }

        aEncryptionInfo.Seek(STREAM_SEEK_TO_BEGIN);

        {
            oox::crypto::AgileEngine aEngine;

            uno::Reference<io::XInputStream> xInputStream(
                new utl::OSeekableInputStreamWrapper(aEncryptionInfo));

            xInputStream->skipBytes(4); // Encryption type -> Agile

            CPPUNIT_ASSERT(aEngine.readEncryptionInfo(xInputStream));

            oox::crypto::AgileEncryptionInfo& rInfo = aEngine.getInfo();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(100000), rInfo.spinCount);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(16), rInfo.saltSize);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(128), rInfo.keyBits);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(20), rInfo.hashSize);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(16), rInfo.blockSize);
            CPPUNIT_ASSERT_EQUAL(u"AES"_ustr, rInfo.cipherAlgorithm);
            CPPUNIT_ASSERT_EQUAL(u"ChainingModeCBC"_ustr, rInfo.cipherChaining);
            CPPUNIT_ASSERT_EQUAL(u"SHA1"_ustr, rInfo.hashAlgorithm);
            CPPUNIT_ASSERT_EQUAL(toString(aKeyDataSalt), toString(rInfo.keyDataSalt));

            CPPUNIT_ASSERT_EQUAL(false, aEngine.decryptAndCheckVerifierHash(u"Wrong"_ustr));
            CPPUNIT_ASSERT_EQUAL(true, aEngine.decryptAndCheckVerifierHash(aPassword));
        }
    }

    { // Preset AES128 - SHA384
        SvMemoryStream aEncryptionInfo;
        {
            oox::crypto::AgileEngine aEngine;

            aEngine.setPreset(oox::crypto::AgileEncryptionPreset::AES_128_SHA384);
            aEngine.setupEncryption(aPassword);
            aKeyDataSalt = aEngine.getInfo().keyDataSalt;

            oox::BinaryXOutputStream aBinaryEncryptionInfoOutputStream(
                new utl::OSeekableOutputStreamWrapper(aEncryptionInfo), true);

            aEngine.writeEncryptionInfo(aBinaryEncryptionInfoOutputStream);
            aBinaryEncryptionInfoOutputStream.close();

            CPPUNIT_ASSERT_EQUAL(sal_uInt64(1040), aEncryptionInfo.GetSize());
        }

        aEncryptionInfo.Seek(STREAM_SEEK_TO_BEGIN);

        {
            oox::crypto::AgileEngine aEngine;

            uno::Reference<io::XInputStream> xInputStream(
                new utl::OSeekableInputStreamWrapper(aEncryptionInfo));

            xInputStream->skipBytes(4); // Encryption type -> Agile

            CPPUNIT_ASSERT(aEngine.readEncryptionInfo(xInputStream));

            oox::crypto::AgileEncryptionInfo& rInfo = aEngine.getInfo();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(100000), rInfo.spinCount);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(16), rInfo.saltSize);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(128), rInfo.keyBits);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(48), rInfo.hashSize);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(16), rInfo.blockSize);
            CPPUNIT_ASSERT_EQUAL(u"AES"_ustr, rInfo.cipherAlgorithm);
            CPPUNIT_ASSERT_EQUAL(u"ChainingModeCBC"_ustr, rInfo.cipherChaining);
            CPPUNIT_ASSERT_EQUAL(u"SHA384"_ustr, rInfo.hashAlgorithm);
            CPPUNIT_ASSERT_EQUAL(toString(aKeyDataSalt), toString(rInfo.keyDataSalt));

            CPPUNIT_ASSERT_EQUAL(false, aEngine.decryptAndCheckVerifierHash(u"Wrong"_ustr));
            CPPUNIT_ASSERT_EQUAL(true, aEngine.decryptAndCheckVerifierHash(aPassword));
        }
    }

    { // Preset AES256 - SHA512
        SvMemoryStream aEncryptionInfo;
        {
            oox::crypto::AgileEngine aEngine;

            aEngine.setPreset(oox::crypto::AgileEncryptionPreset::AES_256_SHA512);
            aEngine.setupEncryption(aPassword);
            aKeyDataSalt = aEngine.getInfo().keyDataSalt;

            oox::BinaryXOutputStream aBinaryEncryptionInfoOutputStream(
                new utl::OSeekableOutputStreamWrapper(aEncryptionInfo), true);

            aEngine.writeEncryptionInfo(aBinaryEncryptionInfoOutputStream);
            aBinaryEncryptionInfoOutputStream.close();

            CPPUNIT_ASSERT_EQUAL(sal_uInt64(1112), aEncryptionInfo.GetSize());
        }

        aEncryptionInfo.Seek(STREAM_SEEK_TO_BEGIN);

        {
            oox::crypto::AgileEngine aEngine;

            uno::Reference<io::XInputStream> xInputStream(
                new utl::OSeekableInputStreamWrapper(aEncryptionInfo));

            xInputStream->skipBytes(4); // Encryption type -> Agile

            CPPUNIT_ASSERT(aEngine.readEncryptionInfo(xInputStream));

            oox::crypto::AgileEncryptionInfo& rInfo = aEngine.getInfo();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(100000), rInfo.spinCount);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(16), rInfo.saltSize);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(256), rInfo.keyBits);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(64), rInfo.hashSize);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(16), rInfo.blockSize);
            CPPUNIT_ASSERT_EQUAL(u"AES"_ustr, rInfo.cipherAlgorithm);
            CPPUNIT_ASSERT_EQUAL(u"ChainingModeCBC"_ustr, rInfo.cipherChaining);
            CPPUNIT_ASSERT_EQUAL(u"SHA512"_ustr, rInfo.hashAlgorithm);
            CPPUNIT_ASSERT_EQUAL(toString(aKeyDataSalt), toString(rInfo.keyDataSalt));

            CPPUNIT_ASSERT_EQUAL(false, aEngine.decryptAndCheckVerifierHash(u"Wrong"_ustr));
            CPPUNIT_ASSERT_EQUAL(true, aEngine.decryptAndCheckVerifierHash(aPassword));
        }
    }
}

void CryptoTest::testAgileDataIntegrityHmacKey()
{
    OUString aPassword(u"Password"_ustr);

    std::vector<sal_uInt8> aKeyDataSalt;

    std::vector<sal_uInt8> aHmacKey;
    std::vector<sal_uInt8> aHmacEncryptedKey;

    SvMemoryStream aEncryptionInfo;
    {
        oox::crypto::AgileEngine aEngine;
        aEngine.setupEncryption(aPassword);
        oox::BinaryXOutputStream aBinaryEncryptionInfoOutputStream(
            new utl::OSeekableOutputStreamWrapper(aEncryptionInfo), true);
        aEngine.writeEncryptionInfo(aBinaryEncryptionInfoOutputStream);
        aBinaryEncryptionInfoOutputStream.close();

        aHmacKey = aEngine.getInfo().hmacKey;
        aKeyDataSalt = aEngine.getInfo().keyDataSalt;
        aHmacEncryptedKey = aEngine.getInfo().hmacEncryptedKey;
    }

    aEncryptionInfo.Seek(STREAM_SEEK_TO_BEGIN);

    {
        oox::crypto::AgileEngine aEngine;

        uno::Reference<io::XInputStream> xInputStream(
            new utl::OSeekableInputStreamWrapper(aEncryptionInfo));

        xInputStream->skipBytes(4); // Encryption type -> Agile

        CPPUNIT_ASSERT(aEngine.readEncryptionInfo(xInputStream));
        CPPUNIT_ASSERT(aEngine.generateEncryptionKey(aPassword));

        CPPUNIT_ASSERT_EQUAL(toString(aKeyDataSalt), toString(aEngine.getInfo().keyDataSalt));

        CPPUNIT_ASSERT_EQUAL(toString(aHmacEncryptedKey),
                             toString(aEngine.getInfo().hmacEncryptedKey));

        CPPUNIT_ASSERT_EQUAL(size_t(64), aHmacKey.size());
        CPPUNIT_ASSERT_EQUAL(toString(aHmacKey), toString(aEngine.getInfo().hmacKey));
    }
}

void CryptoTest::testAgileEncryptingAndDecrypting()
{
    OUString aPassword(u"Password"_ustr);

    SvMemoryStream aEncryptionInfo;
    SvMemoryStream aEncryptedStream;

    OString aTestString = "1234567890ABCDEFGH"_ostr;

    {
        oox::crypto::AgileEngine aEngine;

        // Setup input
        SvMemoryStream aUnencryptedInput;
        uno::Reference<io::XInputStream> xInputStream(
            new utl::OSeekableInputStreamWrapper(aUnencryptedInput));

        aUnencryptedInput.WriteBytes(aTestString.getStr(), aTestString.getLength() + 1);
        aUnencryptedInput.Seek(STREAM_SEEK_TO_BEGIN);

        // Setup output
        uno::Reference<io::XOutputStream> xOutputStream(
            new utl::OSeekableOutputStreamWrapper(aEncryptedStream));

        // Write content
        aEngine.setupEncryption(aPassword);
        aEngine.encrypt(xInputStream, xOutputStream, aUnencryptedInput.GetSize());
        xOutputStream->flush();

        // Check content
        sal_uInt64 nSize = aEncryptedStream.GetSize();

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(40), nSize);

        // Setup and write encryption info
        oox::BinaryXOutputStream aBinaryEncryptionInfoOutputStream(
            new utl::OSeekableOutputStreamWrapper(aEncryptionInfo), true);
        aEngine.writeEncryptionInfo(aBinaryEncryptionInfoOutputStream);
        aBinaryEncryptionInfoOutputStream.close();
    }

    aEncryptedStream.Seek(STREAM_SEEK_TO_BEGIN);
    aEncryptionInfo.Seek(STREAM_SEEK_TO_BEGIN);

    {
        oox::crypto::AgileEngine aEngine;

        // Read encryption info
        uno::Reference<io::XInputStream> xEncryptionInfo(
            new utl::OSeekableInputStreamWrapper(aEncryptionInfo));

        xEncryptionInfo->skipBytes(4); // Encryption type -> Agile

        CPPUNIT_ASSERT(aEngine.readEncryptionInfo(xEncryptionInfo));

        // Setup password
        CPPUNIT_ASSERT(aEngine.generateEncryptionKey(aPassword));

        // Setup encrypted input stream
        oox::BinaryXInputStream aBinaryInputStream(
            new utl::OSeekableInputStreamWrapper(aEncryptedStream), true);

        // Setup output stream
        SvMemoryStream aUnencryptedOutput;
        oox::BinaryXOutputStream aBinaryOutputStream(
            new utl::OSeekableOutputStreamWrapper(aUnencryptedOutput), true);

        // Decrypt
        aEngine.decrypt(aBinaryInputStream, aBinaryOutputStream);
        aBinaryOutputStream.close();
        aBinaryInputStream.close();

        // Check decrypted output
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(19), aUnencryptedOutput.GetSize());

        OString aString(static_cast<const char*>(aUnencryptedOutput.GetData()));
        CPPUNIT_ASSERT_EQUAL(aTestString, aString);

        // Check data integrity
        CPPUNIT_ASSERT_EQUAL(true, aEngine.checkDataIntegrity());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(CryptoTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
