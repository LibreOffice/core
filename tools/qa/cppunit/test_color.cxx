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
#include <tools/color.hxx>
#include <tools/stream.hxx>

namespace
{

class Test: public CppUnit::TestFixture
{
public:
    void test_asRGBColor();
    void test_readAndWriteStream();
    void test_ApplyTintOrShade();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_asRGBColor);
    CPPUNIT_TEST(test_readAndWriteStream);
    CPPUNIT_TEST(test_ApplyTintOrShade);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test_asRGBColor()
{
    Color aColor;
    aColor = COL_BLACK;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("000000"));

    aColor = COL_WHITE;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("ffffff"));

    aColor = COL_RED;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("800000"));

    aColor = COL_TRANSPARENT;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("ffffff"));

    aColor = COL_BLUE;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("000080"));

    aColor.SetRed(0x12);
    aColor.SetGreen(0x34);
    aColor.SetBlue(0x56);
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("123456"));

    aColor = COL_AUTO;
    CPPUNIT_ASSERT_EQUAL(aColor.AsRGBHexString(), OUString("ffffff"));
}

void Test::test_readAndWriteStream()
{
    {
        SvMemoryStream aStream;
        Color aWriteColor(0x12, 0x34, 0x56);
        Color aReadColor;

        WriteColor(aStream, aWriteColor);

        aStream.Seek(STREAM_SEEK_TO_BEGIN);

        ReadColor(aStream, aReadColor);

        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x12), aReadColor.GetRed());
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x34), aReadColor.GetGreen());
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x56), aReadColor.GetBlue());
    }
}

bool checkTintShade(sal_uInt8 nR, sal_uInt8 nG, sal_uInt8 nB, OUString const & sReference, sal_Int16 nTintShade, OUString const & sExpected)
{
    Color aColor(nR, nG, nB);
    if (sReference != aColor.AsRGBHexString())
    {
        fprintf(stderr, "FAILED: Input and reference color mismatch %s\n",
                sReference.toUtf8().getStr());
        return false;
    }
    aColor.ApplyTintOrShade(nTintShade);
    if (sExpected != aColor.AsRGBHexString())
    {
        fprintf(stderr, "FAILED: Reference color is %s which differs from expect %s\n",
                sReference.toUtf8().getStr(),
                sExpected.toUtf8().getStr());
        return false;
    }
    return true;
}

void Test::test_ApplyTintOrShade()
{
    // BLACK reference

    // 5% tint
    CPPUNIT_ASSERT(checkTintShade(0x00, 0x00, 0x00, "000000",  500, "0d0d0d"));
    // 15% tint
    CPPUNIT_ASSERT(checkTintShade(0x00, 0x00, 0x00, "000000",  1500, "262626"));
    // 25% tint
    CPPUNIT_ASSERT(checkTintShade(0x00, 0x00, 0x00, "000000",  2500, "404040"));
    // 50% tint
    CPPUNIT_ASSERT(checkTintShade(0x00, 0x00, 0x00, "000000",  5000, "808080"));
    // 100% tint
    CPPUNIT_ASSERT(checkTintShade(0x00, 0x00, 0x00, "000000", 10000, "ffffff"));

    // WHITE reference

    // 5% shade
    CPPUNIT_ASSERT(checkTintShade(0xff, 0xff, 0xff, "ffffff",   -500, "f2f2f2"));
    // 15% shade
    CPPUNIT_ASSERT(checkTintShade(0xff, 0xff, 0xff, "ffffff",  -1500, "d9d9d9"));
    // 25% shade
    CPPUNIT_ASSERT(checkTintShade(0xff, 0xff, 0xff, "ffffff",  -2500, "bfbfbf"));
    // 50% shade
    CPPUNIT_ASSERT(checkTintShade(0xff, 0xff, 0xff, "ffffff",  -5000, "808080"));
    // 100% shade
    CPPUNIT_ASSERT(checkTintShade(0xff, 0xff, 0xff, "ffffff", -10000, "000000"));

    // GREY reference

    // 0% - no change
    CPPUNIT_ASSERT(checkTintShade(0x80, 0x80, 0x80, "808080",      0, "808080"));

    // 25% tint
    CPPUNIT_ASSERT(checkTintShade(0x80, 0x80, 0x80, "808080",   2500, "a0a0a0"));
    // 50% tint
    CPPUNIT_ASSERT(checkTintShade(0x80, 0x80, 0x80, "808080",   5000, "c0c0c0"));
    // 100% tint
    CPPUNIT_ASSERT(checkTintShade(0x80, 0x80, 0x80, "808080",  10000, "ffffff"));

    // 25% shade
    CPPUNIT_ASSERT(checkTintShade(0x80, 0x80, 0x80, "808080",  -2500, "606060"));
    // 50% shade
    CPPUNIT_ASSERT(checkTintShade(0x80, 0x80, 0x80, "808080",  -5000, "404040"));
    // 100% shade
    CPPUNIT_ASSERT(checkTintShade(0x80, 0x80, 0x80, "808080", -10000, "000000"));

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
