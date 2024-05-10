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

#include <vcl/font.hxx>

#include <font/EmphasisMark.hxx>

class VclFontTest : public test::BootstrapFixture
{
public:
    VclFontTest() : BootstrapFixture(true, false) {}

    void testName();
    void testWeight();
    void testWidthType();
    void testPitch();
    void testItalic();
    void testAlignment();
    void testQuality();
    void testEmphasisMarkShouldBePosAboveWhenSimplifiedChinese();
    void testEmphasisMarkShouldBePosAboveWhenNotSimplifiedChinese();
    void testEmphasisMarkInitAsNone();
    void testEmphasisMarkInitAsDot();
    void testEmphasisMarkInitAsDisc();
    void testEmphasisMarkInitAsAccent();
    void testEmphasisMarkInitAsStyle();

    CPPUNIT_TEST_SUITE(VclFontTest);
    CPPUNIT_TEST(testName);
    CPPUNIT_TEST(testWeight);
    CPPUNIT_TEST(testWidthType);
    CPPUNIT_TEST(testPitch);
    CPPUNIT_TEST(testItalic);
    CPPUNIT_TEST(testAlignment);
    CPPUNIT_TEST(testQuality);
    CPPUNIT_TEST(testEmphasisMarkShouldBePosAboveWhenSimplifiedChinese);
    CPPUNIT_TEST(testEmphasisMarkShouldBePosAboveWhenNotSimplifiedChinese);
    CPPUNIT_TEST(testEmphasisMarkInitAsNone);
    CPPUNIT_TEST(testEmphasisMarkInitAsDot);
    CPPUNIT_TEST(testEmphasisMarkInitAsDisc);
    CPPUNIT_TEST(testEmphasisMarkInitAsAccent);
    CPPUNIT_TEST(testEmphasisMarkInitAsStyle);
    CPPUNIT_TEST_SUITE_END();
};

void VclFontTest::testName()
{
    vcl::Font aFont;

    CPPUNIT_ASSERT_MESSAGE( "Family name should be empty", aFont.GetFamilyName().isEmpty());
    CPPUNIT_ASSERT_MESSAGE( "Style name should be empty", aFont.GetStyleName().isEmpty());
    aFont.SetFamilyName(u"Test family name"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Family name should not be empty", u"Test family name"_ustr, aFont.GetFamilyName());
    aFont.SetStyleName(u"Test style name"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Style name should not be empty", u"Test style name"_ustr, aFont.GetStyleName());
}

void VclFontTest::testWeight()
{
    vcl::Font aFont;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Weight should be WEIGHT_DONTKNOW", FontWeight::WEIGHT_DONTKNOW, aFont.GetWeight());

    aFont.SetWeight(FontWeight::WEIGHT_BLACK);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Weight should be WEIGHT_BLACK", FontWeight::WEIGHT_BLACK, aFont.GetWeight());
}

void VclFontTest::testWidthType()
{
    vcl::Font aFont;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font width should be WIDTH_DONTKNOW", FontWidth::WIDTH_DONTKNOW, aFont.GetWidthType());

    aFont.SetWidthType(FontWidth::WIDTH_EXPANDED);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font width should be EXPANDED", FontWidth::WIDTH_EXPANDED, aFont.GetWidthType());
}

void VclFontTest::testItalic()
{
    vcl::Font aFont;

    // shouldn't this be set to ITALIC_DONTKNOW? currently it defaults to ITALIC_NONE
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Italic should be ITALIC_NONE", FontItalic::ITALIC_NONE, aFont.GetItalic());

    aFont.SetItalic(FontItalic::ITALIC_NORMAL);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Italic should be EXPANDED", FontItalic::ITALIC_NORMAL, aFont.GetItalic());
}


void VclFontTest::testAlignment()
{
    vcl::Font aFont;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Text alignment should be ALIGN_TOP", TextAlign::ALIGN_TOP, aFont.GetAlignment());

    aFont.SetAlignment(TextAlign::ALIGN_BASELINE);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Text alignment should be ALIGN_BASELINE", TextAlign::ALIGN_BASELINE, aFont.GetAlignment());
}


void VclFontTest::testPitch()
{
    vcl::Font aFont;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Pitch should be PITCH_DONTKNOW", FontPitch::PITCH_DONTKNOW, aFont.GetPitch());

    aFont.SetPitch(FontPitch::PITCH_FIXED);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Pitch should be PITCH_FIXED", FontPitch::PITCH_FIXED, aFont.GetPitch());
}

void VclFontTest::testQuality()
{
    vcl::Font aFont;

    CPPUNIT_ASSERT_EQUAL( int(0), aFont.GetQuality() );

    aFont.SetQuality( 100 );
    CPPUNIT_ASSERT_EQUAL( int(100), aFont.GetQuality() );

    aFont.IncreaseQualityBy( 50 );
    CPPUNIT_ASSERT_EQUAL( int(150), aFont.GetQuality() );

    aFont.DecreaseQualityBy( 100 );
    CPPUNIT_ASSERT_EQUAL( int(50), aFont.GetQuality() );
}

void VclFontTest::testEmphasisMarkShouldBePosAboveWhenSimplifiedChinese()
{
    vcl::Font aFont;
    aFont.SetLanguage(LANGUAGE_CHINESE_SIMPLIFIED);
    aFont.SetEmphasisMark(FontEmphasisMark::Accent);

    CPPUNIT_ASSERT_MESSAGE("Emphasis not positioned below", (aFont.GetEmphasisMarkStyle() & FontEmphasisMark::PosBelow));
    CPPUNIT_ASSERT_MESSAGE("Accent mark not kept", (aFont.GetEmphasisMarkStyle() & FontEmphasisMark::Accent));
}

void VclFontTest::testEmphasisMarkShouldBePosAboveWhenNotSimplifiedChinese()
{
    vcl::Font aFont;
    aFont.SetLanguage(LANGUAGE_ENGLISH);
    aFont.SetEmphasisMark(FontEmphasisMark::Accent);

    CPPUNIT_ASSERT_MESSAGE("Emphasis not positioned above", (aFont.GetEmphasisMarkStyle() & FontEmphasisMark::PosAbove));
    CPPUNIT_ASSERT_MESSAGE("Accent mark not kept", (aFont.GetEmphasisMarkStyle() & FontEmphasisMark::Accent));
}

void VclFontTest::testEmphasisMarkInitAsNone()
{
    vcl::font::EmphasisMark aEmphasisMark(FontEmphasisMark::NONE, 5, 96);

    CPPUNIT_ASSERT_MESSAGE("Shape not a polyline", !aEmphasisMark.IsShapePolyLine());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape wrong", tools::PolyPolygon(), aEmphasisMark.GetShape());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect1 not correct", tools::Rectangle(), aEmphasisMark.GetRect1());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect2 not correct", tools::Rectangle(), aEmphasisMark.GetRect2());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("y offset wrong", tools::Long(1), aEmphasisMark.GetYOffset());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width wrong", tools::Long(0), aEmphasisMark.GetWidth());
}

void VclFontTest::testEmphasisMarkInitAsDot()
{
    vcl::font::EmphasisMark aEmphasisMark(FontEmphasisMark::Dot, 5, 96);

    CPPUNIT_ASSERT_MESSAGE("Shape not a polyline", !aEmphasisMark.IsShapePolyLine());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape wrong", tools::PolyPolygon(), aEmphasisMark.GetShape());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect1 not correct", tools::Rectangle(Point(), Size(2, 2)), aEmphasisMark.GetRect1());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect2 not correct", tools::Rectangle(), aEmphasisMark.GetRect2());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("y offset wrong", tools::Long(3), aEmphasisMark.GetYOffset());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width wrong", tools::Long(2), aEmphasisMark.GetWidth());
}

void VclFontTest::testEmphasisMarkInitAsDisc()
{
    vcl::font::EmphasisMark aEmphasisMark(FontEmphasisMark::Disc, 5, 96);

    CPPUNIT_ASSERT_MESSAGE("Shape not a polyline", !aEmphasisMark.IsShapePolyLine());
// something wrong with polypolygon equality checking!
//    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape not disc with radius of 2", tools::PolyPolygon(tools::Polygon(Point(2, 2), 2, 2)), aEmphasisMark.GetShape());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect1 not correct", tools::Rectangle(), aEmphasisMark.GetRect1());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect2 not correct", tools::Rectangle(), aEmphasisMark.GetRect2());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("y offset wrong", tools::Long(4), aEmphasisMark.GetYOffset());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width wrong", tools::Long(4), aEmphasisMark.GetWidth());
}

void VclFontTest::testEmphasisMarkInitAsAccent()
{
    vcl::font::EmphasisMark aEmphasisMark(FontEmphasisMark::Accent, 5, 96);

    CPPUNIT_ASSERT_MESSAGE("Shape not a polyline", !aEmphasisMark.IsShapePolyLine());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect1 not correct", tools::Rectangle(), aEmphasisMark.GetRect1());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect2 not correct", tools::Rectangle(), aEmphasisMark.GetRect2());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("y offset wrong", tools::Long(4), aEmphasisMark.GetYOffset());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width wrong", tools::Long(4), aEmphasisMark.GetWidth());
}

void VclFontTest::testEmphasisMarkInitAsStyle()
{
    vcl::font::EmphasisMark aEmphasisMark(FontEmphasisMark::Style, 5, 96);

    CPPUNIT_ASSERT_MESSAGE("Shape not a polyline", !aEmphasisMark.IsShapePolyLine());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect1 not correct", tools::Rectangle(), aEmphasisMark.GetRect1());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect2 not correct", tools::Rectangle(), aEmphasisMark.GetRect2());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("y offset wrong", tools::Long(1), aEmphasisMark.GetYOffset());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width wrong", tools::Long(0), aEmphasisMark.GetWidth());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclFontTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
