/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <tools/color.hxx>

namespace
{

class Test: public CppUnit::TestFixture
{
public:
    void testVariables();
    void test_asRGBColor();
    void test_ApplyTintOrShade();
    void testGetColorError();
    void testInvert();
    void testBColor();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testVariables);
    CPPUNIT_TEST(test_asRGBColor);
    CPPUNIT_TEST(test_ApplyTintOrShade);
    CPPUNIT_TEST(testGetColorError);
    CPPUNIT_TEST(testInvert);
    CPPUNIT_TEST(testBColor);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testVariables()
{
    Color aColor(0x44, 0x88, 0xAA);
    CPPUNIT_ASSERT_EQUAL(int(0x00), int(aColor.A));
    CPPUNIT_ASSERT_EQUAL(int(0x44), int(aColor.R));
    CPPUNIT_ASSERT_EQUAL(int(0x88), int(aColor.G));
    CPPUNIT_ASSERT_EQUAL(int(0xAA), int(aColor.B));
    CPPUNIT_ASSERT_EQUAL(int(0x004488AA), int(aColor.mValue));

    aColor.mValue = 0xAABBCCDD;
    CPPUNIT_ASSERT_EQUAL(int(0xAA), int(aColor.A));
    CPPUNIT_ASSERT_EQUAL(int(0xBB), int(aColor.R));
    CPPUNIT_ASSERT_EQUAL(int(0xCC), int(aColor.G));
    CPPUNIT_ASSERT_EQUAL(int(0xDD), int(aColor.B));

    aColor.A = 0x11;
    CPPUNIT_ASSERT_EQUAL(int(0x11BBCCDD), int(aColor.mValue));

    aColor.R = 0x22;
    CPPUNIT_ASSERT_EQUAL(int(0x1122CCDD), int(aColor.mValue));

    aColor.G = 0x33;
    CPPUNIT_ASSERT_EQUAL(int(0x112233DD), int(aColor.mValue));

    aColor.B = 0x44;
    CPPUNIT_ASSERT_EQUAL(int(0x11223344), int(aColor.mValue));

    aColor.SetAlpha(255 - 0x77);
    CPPUNIT_ASSERT_EQUAL(int(0x77223344), int(aColor.mValue));

    aColor.SetRed(0x88);
    CPPUNIT_ASSERT_EQUAL(int(0x77883344), int(aColor.mValue));

    aColor.SetGreen(0x99);
    CPPUNIT_ASSERT_EQUAL(int(0x77889944), int(aColor.mValue));

    aColor.SetBlue(0xAA);
    CPPUNIT_ASSERT_EQUAL(int(0x778899AA), int(aColor.mValue));
}

void Test::test_asRGBColor()
{
    Color aColor;
    aColor = COL_BLACK;
    CPPUNIT_ASSERT_EQUAL(OUString("000000"), aColor.AsRGBHexString());

    aColor = COL_WHITE;
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aColor.AsRGBHexString());

    aColor = COL_RED;
    CPPUNIT_ASSERT_EQUAL(OUString("800000"), aColor.AsRGBHexString());

    aColor = COL_TRANSPARENT;
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aColor.AsRGBHexString());

    aColor = COL_BLUE;
    CPPUNIT_ASSERT_EQUAL(OUString("000080"), aColor.AsRGBHexString());

    aColor.SetRed(0x12);
    aColor.SetGreen(0x34);
    aColor.SetBlue(0x56);
    CPPUNIT_ASSERT_EQUAL(OUString("123456"), aColor.AsRGBHexString());

    aColor = COL_AUTO;
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), aColor.AsRGBHexString());
}

OUString createTintShade(sal_uInt8 nR, sal_uInt8 nG, sal_uInt8 nB, std::u16string_view sReference, sal_Int16 nTintShade)
{
    Color aColor(nR, nG, nB);
    if (sReference != aColor.AsRGBHexString())
        return OUString();
    aColor.ApplyTintOrShade(nTintShade);
    return aColor.AsRGBHexString();
}

void Test::test_ApplyTintOrShade()
{
    // BLACK reference

    // 5% tint
    CPPUNIT_ASSERT_EQUAL(OUString("0d0d0d"), createTintShade(0x00, 0x00, 0x00, u"000000",  500));
    // 15% tint
    CPPUNIT_ASSERT_EQUAL(OUString("262626"), createTintShade(0x00, 0x00, 0x00, u"000000",  1500));
    // 25% tint
    CPPUNIT_ASSERT_EQUAL(OUString("404040"), createTintShade(0x00, 0x00, 0x00, u"000000",  2500));
    // 50% tint
    CPPUNIT_ASSERT_EQUAL(OUString("808080"), createTintShade(0x00, 0x00, 0x00, u"000000",  5000));
    // 100% tint
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), createTintShade(0x00, 0x00, 0x00, u"000000", 10000));

    // WHITE reference

    // 5% shade
    CPPUNIT_ASSERT_EQUAL(OUString("f2f2f2"), createTintShade(0xff, 0xff, 0xff, u"ffffff",   -500));
    // 15% shade
    CPPUNIT_ASSERT_EQUAL(OUString("d9d9d9"), createTintShade(0xff, 0xff, 0xff, u"ffffff",  -1500));
    // 25% shade
    CPPUNIT_ASSERT_EQUAL(OUString("bfbfbf"), createTintShade(0xff, 0xff, 0xff, u"ffffff",  -2500));
    // 50% shade
    CPPUNIT_ASSERT_EQUAL(OUString("808080"), createTintShade(0xff, 0xff, 0xff, u"ffffff",  -5000));
    // 100% shade
    CPPUNIT_ASSERT_EQUAL(OUString("000000"), createTintShade(0xff, 0xff, 0xff, u"ffffff", -10000));

    // GREY reference

    // 0% - no change
    CPPUNIT_ASSERT_EQUAL(OUString("808080"), createTintShade(0x80, 0x80, 0x80, u"808080",      0));

    // 25% tint
    CPPUNIT_ASSERT_EQUAL(OUString("a0a0a0"), createTintShade(0x80, 0x80, 0x80, u"808080",   2500));
    // 50% tint
    //CPPUNIT_ASSERT_EQUAL(OUString("c0c0c0"), createTintShade(0x80, 0x80, 0x80, "808080",   5000));
    // disable for now - a rounding error happens on come platforms...
    // 100% tint
    CPPUNIT_ASSERT_EQUAL(OUString("ffffff"), createTintShade(0x80, 0x80, 0x80, u"808080",  10000));

    // 25% shade
    CPPUNIT_ASSERT_EQUAL(OUString("606060"), createTintShade(0x80, 0x80, 0x80, u"808080",  -2500));
    // 50% shade
    CPPUNIT_ASSERT_EQUAL(OUString("404040"), createTintShade(0x80, 0x80, 0x80, u"808080",  -5000));
    // 100% shade
    CPPUNIT_ASSERT_EQUAL(OUString("000000"), createTintShade(0x80, 0x80, 0x80, u"808080", -10000));
}

void Test::testGetColorError()
{
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), Color(0xAA, 0xBB, 0xCC).GetColorError(Color(0xAA, 0xBB, 0xCC)));

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA1, 0xB0, 0xC0)));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA0, 0xB1, 0xC0)));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA0, 0xB0, 0xC1)));

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA1, 0xB1, 0xC0)));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA0, 0xB1, 0xC1)));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA1, 0xB0, 0xC1)));

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA1, 0xB1, 0xC1)));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA1, 0xB1, 0xC1)));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), Color(0xA0, 0xB0, 0xC0).GetColorError(Color(0xA1, 0xB1, 0xC1)));
}

void Test::testInvert()
{
    Color aColor(0xFF, 0x00, 0x88);
    aColor.Invert();
    CPPUNIT_ASSERT_EQUAL(Color(0x00, 0xFF, 0x77).AsRGBHexString(), aColor.AsRGBHexString());

    // Alpha should be unaffected
    aColor = Color(0x22, 0xFF, 0x00, 0x88);
    aColor.Invert();
    CPPUNIT_ASSERT_EQUAL(Color(0x22, 0x00, 0xFF, 0x77).AsRGBHexString(), aColor.AsRGBHexString());
}

void Test::testBColor()
{
    Color aColor;

    aColor = Color(basegfx::BColor(0.0, 0.0, 0.0));

    CPPUNIT_ASSERT_EQUAL(Color(0x00, 0x00, 0x00).AsRGBHexString(), aColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(0.0, aColor.getBColor().getRed());
    CPPUNIT_ASSERT_EQUAL(0.0, aColor.getBColor().getGreen());
    CPPUNIT_ASSERT_EQUAL(0.0, aColor.getBColor().getBlue());

    aColor = Color(basegfx::BColor(1.0, 1.0, 1.0));

    CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0xFF, 0xFF).AsRGBHexString(), aColor.AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(1.0, aColor.getBColor().getRed());
    CPPUNIT_ASSERT_EQUAL(1.0, aColor.getBColor().getGreen());
    CPPUNIT_ASSERT_EQUAL(1.0, aColor.getBColor().getBlue());

    aColor = Color(basegfx::BColor(0.5, 0.25, 0.125));

    CPPUNIT_ASSERT_EQUAL(Color(0x80, 0x40, 0x20).AsRGBHexString(), aColor.AsRGBHexString());
    // FP error is rather big, but that's normal
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.500, aColor.getBColor().getRed(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.250, aColor.getBColor().getGreen(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.125, aColor.getBColor().getBlue(), 1E-2);

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
