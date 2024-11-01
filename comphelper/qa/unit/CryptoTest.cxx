/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_oox.h>
#include <comphelper/crypto/Crypto.hxx>
#include <comphelper/hash.hxx>

#include <rtl/ustring.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#if USE_TLS_NSS
#include <nss.h>
#endif

class CryptoTest : public CppUnit::TestFixture
{
public:
    virtual void tearDown()
    {
#if USE_TLS_NSS
        NSS_Shutdown();
#endif
    }

    void testCryptoHash();
    void testRoundUp();

    CPPUNIT_TEST_SUITE(CryptoTest);
    CPPUNIT_TEST(testCryptoHash);
    CPPUNIT_TEST(testRoundUp);
    CPPUNIT_TEST_SUITE_END();
};

void CryptoTest::testCryptoHash()
{
    // Check examples from Wikipedia (https://en.wikipedia.org/wiki/HMAC)
    OString aContentString("The quick brown fox jumps over the lazy dog"_ostr);
    std::vector<sal_uInt8> aContent(aContentString.getStr(),
                                    aContentString.getStr() + aContentString.getLength());
    std::vector<sal_uInt8> aKey = { 'k', 'e', 'y' };
    {
        comphelper::CryptoHash aCryptoHash(aKey, comphelper::CryptoHashType::SHA1);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(std::string("de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9"),
                             comphelper::hashToString(aHash));
    }

    {
        comphelper::CryptoHash aCryptoHash(aKey, comphelper::CryptoHashType::SHA256);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(
            std::string("f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8"),
            comphelper::hashToString(aHash));
    }

    {
        comphelper::CryptoHash aCryptoHash(aKey, comphelper::CryptoHashType::SHA384);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(std::string("d7f4727e2c0b39ae0f1e40cc96f60242d5b7801841cea6fc592c5d3e1"
                                         "ae50700582a96cf35e1e554995fe4e03381c237"),
                             comphelper::hashToString(aHash));
    }

    {
        comphelper::CryptoHash aCryptoHash(aKey, comphelper::CryptoHashType::SHA512);
        aCryptoHash.update(aContent);
        std::vector<sal_uInt8> aHash = aCryptoHash.finalize();
        CPPUNIT_ASSERT_EQUAL(
            std::string("b42af09057bac1e2d41708e48a902e09b5ff7f12ab428a4fe86653c73dd248fb82f948a549"
                        "f7b791a5b41915ee4d1ec3935357e4e2317250d0372afa2ebeeb3a"),
            comphelper::hashToString(aHash));
    }
}

void CryptoTest::testRoundUp()
{
    CPPUNIT_ASSERT_EQUAL(16, comphelper::roundUp(16, 16));
    CPPUNIT_ASSERT_EQUAL(32, comphelper::roundUp(32, 16));
    CPPUNIT_ASSERT_EQUAL(64, comphelper::roundUp(64, 16));

    CPPUNIT_ASSERT_EQUAL(16, comphelper::roundUp(01, 16));
    CPPUNIT_ASSERT_EQUAL(32, comphelper::roundUp(17, 16));
    CPPUNIT_ASSERT_EQUAL(32, comphelper::roundUp(31, 16));
}

CPPUNIT_TEST_SUITE_REGISTRATION(CryptoTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
