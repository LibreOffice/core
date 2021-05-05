/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>

#include <tools/color.hxx>

#include <vcl/font.hxx>
#include <vcl/metric.hxx>
#include <vcl/settings.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <drawmode.hxx>

class VclDrawModeTest : public test::BootstrapFixture
{
public:
    VclDrawModeTest()
        : BootstrapFixture(true, false)
    {
    }

    void testDrawModeLineColor();
    void testDrawModeFillColor();
    void testDrawModeHatchColor();
    void testDrawModeTextColor();
    void testDrawModeFontColor();
    void testDrawModeBitmap();
    void testDrawModeBitmapEx();

    CPPUNIT_TEST_SUITE(VclDrawModeTest);

    CPPUNIT_TEST(testDrawModeLineColor);
    CPPUNIT_TEST(testDrawModeFillColor);
    CPPUNIT_TEST(testDrawModeHatchColor);
    CPPUNIT_TEST(testDrawModeTextColor);
    CPPUNIT_TEST(testDrawModeFontColor);
    CPPUNIT_TEST(testDrawModeBitmap);
    CPPUNIT_TEST(testDrawModeBitmapEx);

    CPPUNIT_TEST_SUITE_END();
};

void VclDrawModeTest::testDrawModeLineColor()
{
    const Color aColor = COL_RED;
    const sal_uInt8 cLum = aColor.GetLuminance();
    const StyleSettings aStyleSettings;

    CPPUNIT_ASSERT_EQUAL(COL_BLACK,
                         GetDrawModeLineColor(aColor, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE,
                         GetDrawModeLineColor(aColor, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(Color(cLum, cLum, cLum),
                         GetDrawModeLineColor(aColor, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetFontColor(),
                         GetDrawModeLineColor(aColor, DrawModeFlags::SettingsLine, aStyleSettings));

    Color aTransparentRed = COL_RED;
    aTransparentRed.SetAlpha(100);

    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        GetDrawModeLineColor(aTransparentRed, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        GetDrawModeLineColor(aTransparentRed, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        GetDrawModeLineColor(aTransparentRed, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        GetDrawModeLineColor(aTransparentRed, DrawModeFlags::SettingsLine, aStyleSettings));

    // noops
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::NoFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::BlackFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::WhiteFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::GrayFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::SettingsFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::BlackText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::WhiteText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::GrayText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::SettingsText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::BlackBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::WhiteBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeLineColor(aColor, DrawModeFlags::GrayBitmap, aStyleSettings));
}

void VclDrawModeTest::testDrawModeFillColor()
{
    const Color aColor = COL_RED;
    const sal_uInt8 cLum = aColor.GetLuminance();
    const StyleSettings aStyleSettings;

    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT,
                         GetDrawModeFillColor(aColor, DrawModeFlags::NoFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK,
                         GetDrawModeFillColor(aColor, DrawModeFlags::BlackFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE,
                         GetDrawModeFillColor(aColor, DrawModeFlags::WhiteFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(Color(cLum, cLum, cLum),
                         GetDrawModeFillColor(aColor, DrawModeFlags::GrayFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetWindowColor(),
                         GetDrawModeFillColor(aColor, DrawModeFlags::SettingsFill, aStyleSettings));

    Color aTransparentRed = COL_RED;
    aTransparentRed.SetAlpha(100);

    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        GetDrawModeFillColor(aTransparentRed, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        GetDrawModeFillColor(aTransparentRed, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        GetDrawModeFillColor(aTransparentRed, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        GetDrawModeFillColor(aTransparentRed, DrawModeFlags::SettingsLine, aStyleSettings));

    // noops
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::SettingsLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::BlackText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::WhiteText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::GrayText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::SettingsText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::BlackBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::WhiteBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeFillColor(aColor, DrawModeFlags::GrayBitmap, aStyleSettings));
}

void VclDrawModeTest::testDrawModeHatchColor()
{
    const Color aColor = COL_RED;
    const sal_uInt8 cLum = aColor.GetLuminance();
    const StyleSettings aStyleSettings;

    CPPUNIT_ASSERT_EQUAL(COL_BLACK,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(Color(cLum, cLum, cLum),
                         GetDrawModeHatchColor(aColor, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aStyleSettings.GetFontColor(),
        GetDrawModeHatchColor(aColor, DrawModeFlags::SettingsLine, aStyleSettings));

    // noops
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::NoFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::BlackFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::WhiteFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::GrayFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, GetDrawModeHatchColor(aColor, DrawModeFlags::SettingsFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::BlackText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::WhiteText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::GrayText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, GetDrawModeHatchColor(aColor, DrawModeFlags::SettingsText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::BlackBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::WhiteBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeHatchColor(aColor, DrawModeFlags::GrayBitmap, aStyleSettings));
}

void VclDrawModeTest::testDrawModeTextColor()
{
    const Color aColor = COL_RED;
    const sal_uInt8 cLum = aColor.GetLuminance();
    const StyleSettings aStyleSettings;

    CPPUNIT_ASSERT_EQUAL(COL_BLACK,
                         GetDrawModeTextColor(aColor, DrawModeFlags::BlackText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE,
                         GetDrawModeTextColor(aColor, DrawModeFlags::WhiteText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(Color(cLum, cLum, cLum),
                         GetDrawModeTextColor(aColor, DrawModeFlags::GrayText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetFontColor(),
                         GetDrawModeTextColor(aColor, DrawModeFlags::SettingsText, aStyleSettings));

    // noops
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::SettingsLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::NoFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::BlackFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::WhiteFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::GrayFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::SettingsFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::BlackBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::WhiteBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(aColor,
                         GetDrawModeTextColor(aColor, DrawModeFlags::GrayBitmap, aStyleSettings));
}

void VclDrawModeTest::testDrawModeFontColor()
{
    const StyleSettings aStyleSettings;

    vcl::Font aFont;
    aFont.SetFillColor(COL_RED);

    // black text and fill
    aFont.SetTransparent(false);
    vcl::Font aTestFont = GetDrawModeFont(
        aFont, DrawModeFlags::BlackText | DrawModeFlags::BlackFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = GetDrawModeFont(aFont, DrawModeFlags::BlackText | DrawModeFlags::BlackFill,
                                aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());

    // white text and fill
    aFont.SetTransparent(false);
    aTestFont = GetDrawModeFont(aFont, DrawModeFlags::WhiteText | DrawModeFlags::WhiteFill,
                                aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = GetDrawModeFont(aFont, DrawModeFlags::WhiteText | DrawModeFlags::WhiteFill,
                                aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());

    // gray text and fill
    const sal_uInt8 cTextLum = aFont.GetColor().GetLuminance();
    const Color aTextGray(cTextLum, cTextLum, cTextLum);
    const sal_uInt8 cFillLum = aFont.GetFillColor().GetLuminance();
    const Color aFillGray(cFillLum, cFillLum, cFillLum);

    aFont.SetTransparent(false);
    aTestFont
        = GetDrawModeFont(aFont, DrawModeFlags::GrayText | DrawModeFlags::GrayFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(aTextGray, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(aFillGray, aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont
        = GetDrawModeFont(aFont, DrawModeFlags::GrayText | DrawModeFlags::GrayFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(aTextGray, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());

    // no text fill
    aFont.SetTransparent(false);
    aTestFont = GetDrawModeFont(aFont, DrawModeFlags::NoFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = GetDrawModeFont(aFont, DrawModeFlags::NoFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());

    // white text and fill
    aFont.SetTransparent(false);
    aTestFont = GetDrawModeFont(aFont, DrawModeFlags::SettingsText | DrawModeFlags::SettingsFill,
                                aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetFontColor(), aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetWindowColor(), aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = GetDrawModeFont(aFont, DrawModeFlags::SettingsText | DrawModeFlags::SettingsFill,
                                aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetFontColor(), aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());
}

void VclDrawModeTest::testDrawModeBitmap()
{
    const StyleSettings aStyleSettings;

    Bitmap aBmp(Size(1, 1), vcl::PixelFormat::N24_BPP);
    BitmapWriteAccess aBmpAccess(aBmp);
    aBmpAccess.SetPixel(0, 0, BitmapColor(COL_RED));

    {
        Bitmap aResultBitmap(GetDrawModeBitmap(aBmp, DrawModeFlags::GrayBitmap));
        Bitmap::ScopedReadAccess pReadAccess(aResultBitmap);

        const BitmapColor& rColor = pReadAccess->GetColor(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetBlue()));
    }

    // any other operation other than DrawModeFlags::GrayBitmap is a noop
    {
        Bitmap aResultBitmap(GetDrawModeBitmap(aBmp, DrawModeFlags::NoFill));
        Bitmap::ScopedReadAccess pReadAccess(aResultBitmap);

        const BitmapColor& rColor = pReadAccess->GetColor(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x80), sal_Int32(rColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00), sal_Int32(rColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00), sal_Int32(rColor.GetBlue()));
    }
}

void VclDrawModeTest::testDrawModeBitmapEx()
{
    const StyleSettings aStyleSettings;

    Bitmap aBmp(Size(1, 1), vcl::PixelFormat::N24_BPP);
    BitmapWriteAccess aBmpAccess(aBmp);
    aBmpAccess.SetPixel(0, 0, BitmapColor(COL_RED));

    BitmapEx aBmpEx(aBmp);

    {
        BitmapEx aResultBitmapEx(GetDrawModeBitmapEx(aBmpEx, DrawModeFlags::GrayBitmap));
        Bitmap aResultBitmap(aResultBitmapEx.GetBitmap());
        Bitmap::ScopedReadAccess pReadAccess(aResultBitmap);

        const BitmapColor& rColor = pReadAccess->GetColor(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetBlue()));
    }

    // any other operation other than DrawModeFlags::GrayBitmap is a noop
    {
        BitmapEx aResultBitmapEx(GetDrawModeBitmapEx(aBmpEx, DrawModeFlags::NoFill));
        Bitmap aResultBitmap(aResultBitmapEx.GetBitmap());
        Bitmap::ScopedReadAccess pReadAccess(aResultBitmap);

        const BitmapColor& rColor = pReadAccess->GetColor(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x80), sal_Int32(rColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00), sal_Int32(rColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00), sal_Int32(rColor.GetBlue()));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclDrawModeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
