/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <algorithm>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>

#include <oox/crypto/CryptTools.hxx>
#include <oox/crypto/Standard2007Engine.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>

using namespace css;

class CryptoTest : public CppUnit::TestFixture
{
public:
    void testCryptoHash();
    void testRoundUp();
    void testStandard2007();

    CPPUNIT_TEST_SUITE(CryptoTest);
    CPPUNIT_TEST(testCryptoHash);
    CPPUNIT_TEST(testRoundUp);
    CPPUNIT_TEST(testStandard2007);
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

void CryptoTest::testCryptoHash()
{
    // Check examples from Wikipedia (https://en.wikipedia.org/wiki/HMAC)
    OString aContentString("The quick brown fox jumps over the lazy dog");
    std::vector<sal_uInt8> aContent(aContentString.getStr(),
                                    aContentString.getStr() + aContentString.getLength());
    std::vector<sal_uInt8> aKey = { 'k', 'e', 'y' };
    {
        oox::core::CryptoHash aCryptoHash(aKey, oox::core::CryptoHashType::SHA1);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(std::string("de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9"),
                             toString(aHash));
    }

    {
        oox::core::CryptoHash aCryptoHash(aKey, oox::core::CryptoHashType::SHA256);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(
            std::string("f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8"),
            toString(aHash));
    }

    {
        oox::core::CryptoHash aCryptoHash(aKey, oox::core::CryptoHashType::SHA512);
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
    CPPUNIT_ASSERT_EQUAL(16, oox::core::roundUp(16, 16));
    CPPUNIT_ASSERT_EQUAL(32, oox::core::roundUp(32, 16));
    CPPUNIT_ASSERT_EQUAL(64, oox::core::roundUp(64, 16));

    CPPUNIT_ASSERT_EQUAL(16, oox::core::roundUp(01, 16));
    CPPUNIT_ASSERT_EQUAL(32, oox::core::roundUp(17, 16));
    CPPUNIT_ASSERT_EQUAL(32, oox::core::roundUp(31, 16));
}

void CryptoTest::testStandard2007()
{
    oox::core::Standard2007Engine aEngine;
    {
        SvMemoryStream aEncryptionInfo;
        oox::BinaryXOutputStream aBinaryEncryptionInfoOutputStream(
            new utl::OSeekableOutputStreamWrapper(aEncryptionInfo), false);

        aEngine.writeEncryptionInfo("Password", aBinaryEncryptionInfoOutputStream);
        aBinaryEncryptionInfoOutputStream.close();

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(224), aEncryptionInfo.GetSize());
    }

    SvMemoryStream aUnencryptedInput;
    SvMemoryStream aEncryptedStream;

    OString aTestString = OUStringToOString("1234567890ABCDEFG", RTL_TEXTENCODING_UTF8);

    aUnencryptedInput.WriteOString(aTestString);
    aUnencryptedInput.Seek(STREAM_SEEK_TO_BEGIN);

    {
        oox::BinaryXInputStream aBinaryInputStream(
            new utl::OSeekableInputStreamWrapper(aUnencryptedInput), true);
        oox::BinaryXOutputStream aBinaryOutputStream(
            new utl::OSeekableOutputStreamWrapper(aEncryptedStream), true);

        aEncryptedStream.WriteUInt32(aUnencryptedInput.GetSize());
        aEncryptedStream.WriteUInt32(0U);

        aEngine.encrypt(aBinaryInputStream, aBinaryOutputStream);
        aBinaryOutputStream.close();
        aBinaryInputStream.close();

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

        const sal_Char* pData = static_cast<const sal_Char*>(aUnencryptedOutput.GetData());
        sal_uInt64 nSize = aUnencryptedOutput.GetSize();

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(32), nSize);

        OString aString(pData);

        CPPUNIT_ASSERT_EQUAL(aTestString, aString);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(CryptoTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
