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

#include <oox/crypto/Standard2007Engine.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>

using namespace css;

class CryptoTest : public CppUnit::TestFixture
{
public:
    void testStandard2007();

    CPPUNIT_TEST_SUITE(CryptoTest);
    CPPUNIT_TEST(testStandard2007);
    CPPUNIT_TEST_SUITE_END();
};

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
