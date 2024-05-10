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

#include <oox/ole/vbaexport.hxx>

class TestVbaEncryption : public CppUnit::TestFixture
{
public:
#if 0
    // an initial test for the encryption taken from the spec
    void testSimple1();

    void testSimple2();
#endif

    void testProjKey1();

    CPPUNIT_TEST_SUITE(TestVbaEncryption);
    // CPPUNIT_TEST(testSimple1);
    // CPPUNIT_TEST(testSimple2);
    CPPUNIT_TEST(testProjKey1);
    CPPUNIT_TEST_SUITE_END();
};

#if 0
void TestVbaEncryption::testSimple1()
{
    sal_uInt8 nSeed = 0x07;
    sal_uInt8 nProjKey = 0xDF;
    sal_uInt16 nLength = 0x04;
    sal_uInt8 pData[] = { 0x00, 0x00, 0x00, 0x00 };

    SvMemoryStream aEncryptedStream(4096, 4096);
    VBAEncryption aEncryption(pData, nLength, aEncryptedStream,
            &nSeed, nProjKey);
    aEncryption.write();
}

void TestVbaEncryption::testSimple2()
{
    sal_uInt8 nSeed = 0x15;
    sal_uInt8 nProjKey = 0xDF;
    sal_uInt16 nLength = 0x01;
    sal_uInt8 pData[] = { 0xFF };

    SvMemoryStream aEncryptedStream(4096, 4096);
    VBAEncryption aEncryption(pData, nLength, aEncryptedStream,
            &nSeed, nProjKey);
    aEncryption.write();
    sal_uInt8 pExpectedData[] = "1517CAF1D6F9D7F9D706";
    size_t length = sizeof(pExpectedData);
    aEncryptedStream.Seek(0);
    for (size_t i = 0; i < length; ++i)
    {
        unsigned char val = 0;
        aEncryptedStream.ReadUChar(val);
        CPPUNIT_ASSERT_EQUAL((int)pExpectedData[i], (int)sal_uInt8(val));
    }
}
#endif

void TestVbaEncryption::testProjKey1()
{
    sal_uInt8 nProjKey
        = VBAEncryption::calculateProjKey(u"{917DED54-440B-4FD1-A5C1-74ACF261E600}"_ustr);
    CPPUNIT_ASSERT_EQUAL(int(0xdf), static_cast<int>(nProjKey));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestVbaEncryption);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
