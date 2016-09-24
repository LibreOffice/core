/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <rtl/ustring.hxx>
#include <vector>

#include <tools/tenccvt.hxx>

//Tests for getBestMSEncodingByChar

namespace
{

    class Test: public CppUnit::TestFixture
    {
    public:
        void testEncoding(rtl_TextEncoding eEncoding);

        void test1258();
        void test1257();
        void test1256();
        void test1255();
        void test1254();
        void test1253();
        void test1252();
        void test1251();
        void test1250();
        void test874();

        CPPUNIT_TEST_SUITE(Test);
        CPPUNIT_TEST(test1258);
        CPPUNIT_TEST(test1257);
        CPPUNIT_TEST(test1256);
        CPPUNIT_TEST(test1255);
        CPPUNIT_TEST(test1254);
        CPPUNIT_TEST(test1253);
        CPPUNIT_TEST(test1252);
        CPPUNIT_TEST(test1251);
        CPPUNIT_TEST(test1250);
        CPPUNIT_TEST(test874);
        CPPUNIT_TEST_SUITE_END();
    };

    void Test::testEncoding(rtl_TextEncoding eEncoding)
    {
        //Taking the single byte legacy encodings, fill in all possible values
        std::vector<sal_Char> aAllChars(255);
        for (int i = 1; i <= 255; ++i)
            aAllChars[i-1] = static_cast<sal_Char>(i);

        //Some slots are unused, so don't map to private, just set them to 'X'
        sal_uInt32 convertFlags = OUSTRING_TO_OSTRING_CVTFLAGS ^ RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE;
        OUString sOrigText(&aAllChars[0], aAllChars.size(), eEncoding, convertFlags);
        sOrigText = sOrigText.replace( 0xfffd, 'X' );

        //Should clearly be equal
        sal_Int32 nLength = aAllChars.size();
        CPPUNIT_ASSERT_EQUAL(sOrigText.getLength(), nLength);

        OUString sFinalText;

        //Split up in chunks of the same encoding returned by
        //getBestMSEncodingByChar, convert to it, and back
        rtl_TextEncoding ePrevEncoding = RTL_TEXTENCODING_DONTKNOW;
        const sal_Unicode *pStr = sOrigText.getStr();
        sal_Int32 nChunkStart=0;
        for (int i = 0; i < 255; ++i)
        {
            rtl_TextEncoding eCurrEncoding = getBestMSEncodingByChar(pStr[i]);
            if (eCurrEncoding != ePrevEncoding)
            {
                OString aChunk(pStr+nChunkStart, i-nChunkStart, ePrevEncoding);
                sFinalText += OStringToOUString(aChunk, ePrevEncoding);
                nChunkStart = i;
            }
            ePrevEncoding = eCurrEncoding;
        }
        if (nChunkStart < 255)
        {
            OString aChunk(pStr+nChunkStart, 255-nChunkStart, ePrevEncoding);
            sFinalText += OStringToOUString(aChunk, ePrevEncoding);
        }

        //Final text should be the same as original
        CPPUNIT_ASSERT_EQUAL(sOrigText, sFinalText);
    }

    void Test::test1252()
    {
        testEncoding(RTL_TEXTENCODING_MS_1252);
    }

    void Test::test874()
    {
        testEncoding(RTL_TEXTENCODING_MS_874);
    }

    void Test::test1258()
    {
        testEncoding(RTL_TEXTENCODING_MS_1258);
    }

    void Test::test1257()
    {
        testEncoding(RTL_TEXTENCODING_MS_1257);
    }

    void Test::test1256()
    {
        testEncoding(RTL_TEXTENCODING_MS_1256);
    }

    void Test::test1255()
    {
        testEncoding(RTL_TEXTENCODING_MS_1255);
    }

    void Test::test1254()
    {
        testEncoding(RTL_TEXTENCODING_MS_1254);
    }

    void Test::test1253()
    {
        testEncoding(RTL_TEXTENCODING_MS_1253);
    }

    void Test::test1251()
    {
        testEncoding(RTL_TEXTENCODING_MS_1251);
    }

    void Test::test1250()
    {
        testEncoding(RTL_TEXTENCODING_MS_1250);
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
