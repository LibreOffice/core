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
#include <vcl/settings.hxx>

#include <vcl/BitmapWriteAccess.hxx>
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
    void testDrawModeBitmapEx();

    CPPUNIT_TEST_SUITE(VclDrawModeTest);

    CPPUNIT_TEST(testDrawModeLineColor);
    CPPUNIT_TEST(testDrawModeFillColor);
    CPPUNIT_TEST(testDrawModeHatchColor);
    CPPUNIT_TEST(testDrawModeTextColor);
    CPPUNIT_TEST(testDrawModeFontColor);
    CPPUNIT_TEST(testDrawModeBitmapEx);

    CPPUNIT_TEST_SUITE_END();
};

void VclDrawModeTest::testDrawModeLineColor()
{
    const Color aColor = COL_RED;
    const sal_uInt8 cLum = aColor.GetLuminance();
    const StyleSettings aStyleSettings;

    CPPUNIT_ASSERT_EQUAL(
        COL_BLACK, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        COL_WHITE, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        Color(cLum, cLum, cLum),
        vcl::drawmode::GetLineColor(aColor, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aStyleSettings.GetWindowTextColor(),
        vcl::drawmode::GetLineColor(aColor, DrawModeFlags::SettingsLine, aStyleSettings));

    Color aTransparentRed = COL_RED;
    aTransparentRed.SetAlpha(100);

    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        vcl::drawmode::GetLineColor(aTransparentRed, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        vcl::drawmode::GetLineColor(aTransparentRed, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        vcl::drawmode::GetLineColor(aTransparentRed, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        vcl::drawmode::GetLineColor(aTransparentRed, DrawModeFlags::SettingsLine, aStyleSettings));

    // noops
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::NoFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::BlackFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::WhiteFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::GrayFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::SettingsFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::BlackText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::WhiteText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::GrayText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::SettingsText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::BlackBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::WhiteBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetLineColor(aColor, DrawModeFlags::GrayBitmap, aStyleSettings));
}

void VclDrawModeTest::testDrawModeFillColor()
{
    const Color aColor = COL_RED;
    const sal_uInt8 cLum = aColor.GetLuminance();
    const StyleSettings aStyleSettings;

    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::NoFill,
                                                                      aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        COL_BLACK, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::BlackFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        COL_WHITE, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::WhiteFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        Color(cLum, cLum, cLum),
        vcl::drawmode::GetFillColor(aColor, DrawModeFlags::GrayFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aStyleSettings.GetWindowColor(),
        vcl::drawmode::GetFillColor(aColor, DrawModeFlags::SettingsFill, aStyleSettings));

    Color aTransparentRed = COL_RED;
    aTransparentRed.SetAlpha(100);

    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        vcl::drawmode::GetFillColor(aTransparentRed, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        vcl::drawmode::GetFillColor(aTransparentRed, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        vcl::drawmode::GetFillColor(aTransparentRed, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aTransparentRed,
        vcl::drawmode::GetFillColor(aTransparentRed, DrawModeFlags::SettingsLine, aStyleSettings));

    // noops
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::SettingsLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::BlackText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::WhiteText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::GrayText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::SettingsText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::BlackBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::WhiteBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetFillColor(aColor, DrawModeFlags::GrayBitmap, aStyleSettings));
}

void VclDrawModeTest::testDrawModeHatchColor()
{
    const Color aColor = COL_RED;
    const sal_uInt8 cLum = aColor.GetLuminance();
    const StyleSettings aStyleSettings;

    CPPUNIT_ASSERT_EQUAL(
        COL_BLACK, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        COL_WHITE, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        Color(cLum, cLum, cLum),
        vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aStyleSettings.GetWindowTextColor(),
        vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::SettingsLine, aStyleSettings));

    // noops
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::NoFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::BlackFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::WhiteFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::GrayFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::SettingsFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::BlackText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::WhiteText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::GrayText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::SettingsText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::BlackBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::WhiteBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetHatchColor(aColor, DrawModeFlags::GrayBitmap, aStyleSettings));
}

void VclDrawModeTest::testDrawModeTextColor()
{
    const Color aColor = COL_RED;
    const sal_uInt8 cLum = aColor.GetLuminance();
    const StyleSettings aStyleSettings;

    CPPUNIT_ASSERT_EQUAL(
        COL_BLACK, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::BlackText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        COL_WHITE, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::WhiteText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        Color(cLum, cLum, cLum),
        vcl::drawmode::GetTextColor(aColor, DrawModeFlags::GrayText, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aStyleSettings.GetWindowTextColor(),
        vcl::drawmode::GetTextColor(aColor, DrawModeFlags::SettingsText, aStyleSettings));

    // noops
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::BlackLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::WhiteLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::GrayLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::SettingsLine, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::NoFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::BlackFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::WhiteFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::GrayFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::SettingsFill, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::BlackBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::WhiteBitmap, aStyleSettings));
    CPPUNIT_ASSERT_EQUAL(
        aColor, vcl::drawmode::GetTextColor(aColor, DrawModeFlags::GrayBitmap, aStyleSettings));
}

void VclDrawModeTest::testDrawModeFontColor()
{
    const StyleSettings aStyleSettings;

    vcl::Font aFont;
    aFont.SetFillColor(COL_RED);

    // black text and fill
    aFont.SetTransparent(false);
    vcl::Font aTestFont = vcl::drawmode::GetFont(
        aFont, DrawModeFlags::BlackText | DrawModeFlags::BlackFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = vcl::drawmode::GetFont(aFont, DrawModeFlags::BlackText | DrawModeFlags::BlackFill,
                                       aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());

    // white text and fill
    aFont.SetTransparent(false);
    aTestFont = vcl::drawmode::GetFont(aFont, DrawModeFlags::WhiteText | DrawModeFlags::WhiteFill,
                                       aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = vcl::drawmode::GetFont(aFont, DrawModeFlags::WhiteText | DrawModeFlags::WhiteFill,
                                       aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());

    // gray text and fill
    const sal_uInt8 cTextLum = aFont.GetColor().GetLuminance();
    const Color aTextGray(cTextLum, cTextLum, cTextLum);
    const sal_uInt8 cFillLum = aFont.GetFillColor().GetLuminance();
    const Color aFillGray(cFillLum, cFillLum, cFillLum);

    aFont.SetTransparent(false);
    aTestFont = vcl::drawmode::GetFont(aFont, DrawModeFlags::GrayText | DrawModeFlags::GrayFill,
                                       aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(aTextGray, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(aFillGray, aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = vcl::drawmode::GetFont(aFont, DrawModeFlags::GrayText | DrawModeFlags::GrayFill,
                                       aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(aTextGray, aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());

    // no text fill
    aFont.SetTransparent(false);
    aTestFont = vcl::drawmode::GetFont(aFont, DrawModeFlags::NoFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = vcl::drawmode::GetFont(aFont, DrawModeFlags::NoFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());

    // white text and fill
    aFont.SetTransparent(false);
    aTestFont = vcl::drawmode::GetFont(
        aFont, DrawModeFlags::SettingsText | DrawModeFlags::SettingsFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetWindowTextColor(), aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetWindowColor(), aTestFont.GetFillColor());

    aFont.SetTransparent(true);
    aTestFont = vcl::drawmode::GetFont(
        aFont, DrawModeFlags::SettingsText | DrawModeFlags::SettingsFill, aStyleSettings);
    CPPUNIT_ASSERT_EQUAL(aStyleSettings.GetWindowTextColor(), aTestFont.GetColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, aTestFont.GetFillColor());
}

void VclDrawModeTest::testDrawModeBitmapEx()
{
    const StyleSettings aStyleSettings;

    Bitmap aBmp(Size(1, 1), vcl::PixelFormat::N24_BPP);
    BitmapWriteAccess(aBmp).SetPixel(0, 0, BitmapColor(COL_RED));

    BitmapEx aBmpEx(aBmp);

    {
        BitmapEx aResultBitmapEx(vcl::drawmode::GetBitmapEx(aBmpEx, DrawModeFlags::GrayBitmap));
        Bitmap aResultBitmap(aResultBitmapEx.GetBitmap());
        BitmapScopedReadAccess pReadAccess(aResultBitmap);

        const BitmapColor aColor = pReadAccess->GetColor(0, 0);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0x26, 0x26, 0x26), aColor);
    }

    // any other operation other than DrawModeFlags::GrayBitmap is a noop
    {
        BitmapEx aResultBitmapEx(vcl::drawmode::GetBitmapEx(aBmpEx, DrawModeFlags::NoFill));
        Bitmap aResultBitmap(aResultBitmapEx.GetBitmap());
        BitmapScopedReadAccess pReadAccess(aResultBitmap);

        const BitmapColor aColor = pReadAccess->GetColor(0, 0);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_RED), aColor);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclDrawModeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
