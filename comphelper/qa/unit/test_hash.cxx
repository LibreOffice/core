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
#include <comphelper/hash.hxx>
#include <comphelper/docpasswordhelper.hxx>

#include <rtl/ustring.hxx>
#include <iomanip>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#if USE_TLS_NSS
#include <nss.h>
#endif

class TestHash : public CppUnit::TestFixture
{
public:
    void testMD5();
    void testSHA1();
    void testSHA256();
    void testSHA512();
    void testSHA512_NoSaltNoSpin();
    void testSHA512_saltspin();

    virtual void tearDown()
    {
#if USE_TLS_NSS
        NSS_Shutdown();
#endif
    }
    CPPUNIT_TEST_SUITE(TestHash);
    CPPUNIT_TEST(testMD5);
    CPPUNIT_TEST(testSHA1);
    CPPUNIT_TEST(testSHA256);
    CPPUNIT_TEST(testSHA512);
    CPPUNIT_TEST(testSHA512_NoSaltNoSpin);
    CPPUNIT_TEST(testSHA512_saltspin);
    CPPUNIT_TEST_SUITE_END();
};

namespace {

std::string tostring(const std::vector<unsigned char>& a)
{
    std::stringstream aStrm;
    for (auto& i:a)
    {
        aStrm << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(i);
    }

    return aStrm.str();
}

}

void TestHash::testMD5()
{
    comphelper::Hash aHash(comphelper::HashType::MD5);
    const char* const pInput = "";
    aHash.update(reinterpret_cast<const unsigned char*>(pInput), 0);
    std::vector<unsigned char> calculate_hash = aHash.finalize();
    CPPUNIT_ASSERT_EQUAL(size_t(16), calculate_hash.size());
    CPPUNIT_ASSERT_EQUAL(std::string("d41d8cd98f00b204e9800998ecf8427e"), tostring(calculate_hash));
}

void TestHash::testSHA1()
{
    comphelper::Hash aHash(comphelper::HashType::SHA1);
    const char* const pInput = "";
    aHash.update(reinterpret_cast<const unsigned char*>(pInput), 0);
    std::vector<unsigned char> calculate_hash = aHash.finalize();
    CPPUNIT_ASSERT_EQUAL(size_t(20), calculate_hash.size());
    CPPUNIT_ASSERT_EQUAL(std::string("da39a3ee5e6b4b0d3255bfef95601890afd80709"), tostring(calculate_hash));
}

void TestHash::testSHA256()
{
    comphelper::Hash aHash(comphelper::HashType::SHA256);
    const char* const pInput = "";
    aHash.update(reinterpret_cast<const unsigned char*>(pInput), 0);
    std::vector<unsigned char> calculate_hash = aHash.finalize();
    CPPUNIT_ASSERT_EQUAL(size_t(32), calculate_hash.size());
    CPPUNIT_ASSERT_EQUAL(std::string("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"), tostring(calculate_hash));
}

void TestHash::testSHA512()
{
    comphelper::Hash aHash(comphelper::HashType::SHA512);
    const char* const pInput = "";
    aHash.update(reinterpret_cast<const unsigned char*>(pInput), 0);
    std::vector<unsigned char> calculate_hash = aHash.finalize();
    CPPUNIT_ASSERT_EQUAL(size_t(64), calculate_hash.size());
    std::string aStr("cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
    CPPUNIT_ASSERT_EQUAL(aStr, tostring(calculate_hash));
}

// Must be identical to testSHA512()
void TestHash::testSHA512_NoSaltNoSpin()
{
    const char* const pInput = "";
    std::vector<unsigned char> calculate_hash =
        comphelper::Hash::calculateHash( reinterpret_cast<const unsigned char*>(pInput), 0,
                nullptr, 0, 0, comphelper::Hash::IterCount::NONE, comphelper::HashType::SHA512);
    CPPUNIT_ASSERT_EQUAL(size_t(64), calculate_hash.size());
    std::string aStr("cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
    CPPUNIT_ASSERT_EQUAL(aStr, tostring(calculate_hash));
}

// Password, salt, hash and spin count taken from OOXML sheetProtection of
// tdf#104250 https://bugs.documentfoundation.org/attachment.cgi?id=129104
void TestHash::testSHA512_saltspin()
{
    const OUString aHash = comphelper::DocPasswordHelper::GetOoxHashAsBase64( u"pwd"_ustr, u"876MLoKTq42+/DLp415iZQ==", 100000,
            comphelper::Hash::IterCount::APPEND, u"SHA-512");
    CPPUNIT_ASSERT_EQUAL(u"5l3mgNHXpWiFaBPv5Yso1Xd/UifWvQWmlDnl/hsCYbFT2sJCzorjRmBCQ/3qeDu6Q/4+GIE8a1DsdaTwYh1q2g=="_ustr, aHash);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestHash);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
