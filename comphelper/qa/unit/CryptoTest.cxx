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
};

CPPUNIT_TEST_FIXTURE(CryptoTest, testCryptoHash)
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

CPPUNIT_TEST_FIXTURE(CryptoTest, testRoundUp)
{
    CPPUNIT_ASSERT_EQUAL(16, comphelper::roundUp(16, 16));
    CPPUNIT_ASSERT_EQUAL(32, comphelper::roundUp(32, 16));
    CPPUNIT_ASSERT_EQUAL(64, comphelper::roundUp(64, 16));

    CPPUNIT_ASSERT_EQUAL(16, comphelper::roundUp(01, 16));
    CPPUNIT_ASSERT_EQUAL(32, comphelper::roundUp(17, 16));
    CPPUNIT_ASSERT_EQUAL(32, comphelper::roundUp(31, 16));
}

CPPUNIT_TEST_FIXTURE(CryptoTest, testEncrypt_AES256_CBC)
{
    std::vector<sal_uInt8> key = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11,
                                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22,
                                   0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32 };

    std::vector<sal_uInt8> iv = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                  0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };

    std::vector<sal_uInt8> original = { 's', 'e', 'c', 'r', 'e', 't', '\0' };

    std::vector<sal_uInt8> encrypted(original.size());

    {
        sal_uInt32 nWrittenSize = 0;
        comphelper::Encrypt aEncryptor(key, iv, comphelper::CryptoType::AES_256_CBC);
        nWrittenSize = aEncryptor.update(encrypted, original);

        // nothing should be written as the size of the input is not a multiple of block size
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), nWrittenSize);
    }

    {
        sal_uInt32 nWrittenSize = 0;
        comphelper::Encrypt aEncryptor(key, iv, comphelper::CryptoType::AES_256_CBC);

        original.resize(16, 0); // apply padding to make it multiple of block size
        encrypted.resize(16, 0);

        CPPUNIT_ASSERT_EQUAL(std::string("73656372657400000000000000000000"),
                             comphelper::hashToString(original));

        nWrittenSize = aEncryptor.update(encrypted, original);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(16), nWrittenSize);

        CPPUNIT_ASSERT_EQUAL(std::string("e75cb91a34377c09c354c24fcef345a6"),
                             comphelper::hashToString(encrypted));

        std::vector<sal_uInt8> decrypted(encrypted.size(), 0);

        comphelper::Decrypt aDecryptor(key, iv, comphelper::CryptoType::AES_256_CBC);
        nWrittenSize = aDecryptor.update(decrypted, encrypted);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(16), nWrittenSize);

        CPPUNIT_ASSERT_EQUAL(std::string("73656372657400000000000000000000"),
                             comphelper::hashToString(decrypted));
    }
}

CPPUNIT_TEST_FIXTURE(CryptoTest, testEncrypt_AES256_ECB)
{
    std::vector<sal_uInt8> key = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11,
                                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22,
                                   0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32 };

    std::vector<sal_uInt8> iv = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                  0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };

    std::vector<sal_uInt8> original = { 's', 'e', 'c', 'r', 'e', 't', '\0' };

    std::vector<sal_uInt8> encrypted(original.size());

    {
        sal_uInt32 nWrittenSize = 0;
        comphelper::Encrypt aEncryptor(key, iv, comphelper::CryptoType::AES_256_ECB);
        nWrittenSize = aEncryptor.update(encrypted, original);

        // nothing should be written as the size of the input is not a multiple of block size
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), nWrittenSize);
    }

    {
        sal_uInt32 nWrittenSize = 0;
        comphelper::Encrypt aEncryptor(key, iv, comphelper::CryptoType::AES_256_ECB);

        original.resize(16, 0); // apply padding to make it multiple of block size
        encrypted.resize(16, 0);

        CPPUNIT_ASSERT_EQUAL(std::string("73656372657400000000000000000000"),
                             comphelper::hashToString(original));

        nWrittenSize = aEncryptor.update(encrypted, original);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(16), nWrittenSize);

        CPPUNIT_ASSERT_EQUAL(std::string("abf7abec9a6b58c089e902397c47ac49"),
                             comphelper::hashToString(encrypted));

        std::vector<sal_uInt8> decrypted(encrypted.size(), 0);

        comphelper::Decrypt aDecryptor(key, iv, comphelper::CryptoType::AES_256_ECB);
        nWrittenSize = aDecryptor.update(decrypted, encrypted);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(16), nWrittenSize);

        CPPUNIT_ASSERT_EQUAL(std::string("73656372657400000000000000000000"),
                             comphelper::hashToString(decrypted));
    }
}

CPPUNIT_TEST_FIXTURE(CryptoTest, testEncrypt_AES192_CBC)
{
    std::vector<sal_uInt8> key = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11,
                                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22,
                                   0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32 };

    std::vector<sal_uInt8> iv = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                  0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };

    std::vector<sal_uInt8> original = { 's', 'e', 'c', 'r', 'e', 't', '\0' };

    std::vector<sal_uInt8> encrypted(original.size());

    {
        sal_uInt32 nWrittenSize = 0;
        comphelper::Encrypt aEncryptor(key, iv, comphelper::CryptoType::AES_192_CBC);
        nWrittenSize = aEncryptor.update(encrypted, original);

        // nothing should be written as the size of the input is not a multiple of block size
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), nWrittenSize);
    }

    {
        sal_uInt32 nWrittenSize = 0;
        comphelper::Encrypt aEncryptor(key, iv, comphelper::CryptoType::AES_192_CBC);

        original.resize(16, 0); // apply padding to make it multiple of block size
        encrypted.resize(16, 0);

        CPPUNIT_ASSERT_EQUAL(std::string("73656372657400000000000000000000"),
                             comphelper::hashToString(original));

        nWrittenSize = aEncryptor.update(encrypted, original);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(16), nWrittenSize);

        CPPUNIT_ASSERT_EQUAL(std::string("e75cb91a34377c09c354c24fcef345a6"),
                             comphelper::hashToString(encrypted));

        std::vector<sal_uInt8> decrypted(encrypted.size(), 0);

        comphelper::Decrypt aDecryptor(key, iv, comphelper::CryptoType::AES_192_CBC);
        nWrittenSize = aDecryptor.update(decrypted, encrypted);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(16), nWrittenSize);

        CPPUNIT_ASSERT_EQUAL(std::string("73656372657400000000000000000000"),
                             comphelper::hashToString(decrypted));
    }
}

CPPUNIT_TEST_FIXTURE(CryptoTest, testEncrypt_AES192_ECB)
{
    std::vector<sal_uInt8> key = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11,
                                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22,
                                   0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32 };

    std::vector<sal_uInt8> iv = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                  0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };

    std::vector<sal_uInt8> original = { 's', 'e', 'c', 'r', 'e', 't', '\0' };

    std::vector<sal_uInt8> encrypted(original.size());

    {
        sal_uInt32 nWrittenSize = 0;
        comphelper::Encrypt aEncryptor(key, iv, comphelper::CryptoType::AES_192_ECB);
        nWrittenSize = aEncryptor.update(encrypted, original);

        // nothing should be written as the size of the input is not a multiple of block size
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), nWrittenSize);
    }

    {
        sal_uInt32 nWrittenSize = 0;
        comphelper::Encrypt aEncryptor(key, iv, comphelper::CryptoType::AES_192_ECB);

        original.resize(16, 0); // apply padding to make it multiple of block size
        encrypted.resize(16, 0);

        CPPUNIT_ASSERT_EQUAL(std::string("73656372657400000000000000000000"),
                             comphelper::hashToString(original));

        nWrittenSize = aEncryptor.update(encrypted, original);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(16), nWrittenSize);

        CPPUNIT_ASSERT_EQUAL(std::string("abf7abec9a6b58c089e902397c47ac49"),
                             comphelper::hashToString(encrypted));

        std::vector<sal_uInt8> decrypted(encrypted.size(), 0);

        comphelper::Decrypt aDecryptor(key, iv, comphelper::CryptoType::AES_192_ECB);
        nWrittenSize = aDecryptor.update(decrypted, encrypted);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(16), nWrittenSize);

        CPPUNIT_ASSERT_EQUAL(std::string("73656372657400000000000000000000"),
                             comphelper::hashToString(decrypted));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
