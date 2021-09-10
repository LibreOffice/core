/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>

#include <tools/fontenum.hxx>
#include <unotools/fontcfg.hxx>
#include <o3tl/sorted_vector.hxx>

#include <vcl/virdev.hxx>

#include <font/PhysicalFontFamily.hxx>

#include "fontmocks.hxx"

using namespace vcl::font;

class VclPhysicalFontFamilyTest : public test::BootstrapFixture
{
public:
    VclPhysicalFontFamilyTest()
        : BootstrapFixture(true, false)
    {
    }

    void testCreateFontFamily();
    void testAddFontFace_Default();
    void testAddOneFontFace();
    void testAddTwoFontFaces();

    CPPUNIT_TEST_SUITE(VclPhysicalFontFamilyTest);
    CPPUNIT_TEST(testCreateFontFamily);
    CPPUNIT_TEST(testAddFontFace_Default);
    CPPUNIT_TEST(testAddOneFontFace);
    CPPUNIT_TEST(testAddTwoFontFaces);
    CPPUNIT_TEST_SUITE_END();
};

void VclPhysicalFontFamilyTest::testCreateFontFamily()
{
    PhysicalFontFamily aFamily("Test font face");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", OUString(""), aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", OUString("Test font face"),
                                 aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Alias names", OUString(""), aFamily.GetAliasNames());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", -1, aFamily.GetMinQuality());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", FontTypeFaces::NONE, aFamily.GetTypeFaces());

    o3tl::sorted_vector<int> aHeights;
    aFamily.GetFontHeights(aHeights);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number font heights", static_cast<size_t>(0), aHeights.size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match family name", OUString(""), aFamily.GetMatchFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match type", ImplFontAttrs::None, aFamily.GetMatchType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match weight", WEIGHT_DONTKNOW, aFamily.GetMatchWeight());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match width", WIDTH_DONTKNOW, aFamily.GetMatchWidth());
}

void VclPhysicalFontFamilyTest::testAddFontFace_Default()
{
    PhysicalFontFamily aFamily("Test font face");

    aFamily.AddFontFace(new TestFontFace(1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", OUString(""), aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", OUString("Test font face"),
                                 aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Alias names", OUString(""), aFamily.GetAliasNames());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", 0, aFamily.GetMinQuality());
    FontTypeFaces eTypeFace
        = FontTypeFaces::Scalable | FontTypeFaces::NoneSymbol | FontTypeFaces::NoneItalic;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", eTypeFace, aFamily.GetTypeFaces());
}

void VclPhysicalFontFamilyTest::testAddOneFontFace()
{
    PhysicalFontFamily aFamily("Test font face");

    FontAttributes aFontAttrs;
    aFontAttrs.SetFamilyName("Test font face");
    aFontAttrs.AddMapName("Alias name");
    aFontAttrs.SetFamilyType(FontFamily::FAMILY_ROMAN);
    aFontAttrs.SetPitch(FontPitch::PITCH_VARIABLE);
    aFontAttrs.SetItalic(FontItalic::ITALIC_NONE);
    aFontAttrs.SetQuality(10);
    aFontAttrs.SetWeight(FontWeight::WEIGHT_BOLD);
    aFontAttrs.SetWidthType(FontWidth::WIDTH_EXPANDED);

    aFamily.AddFontFace(new TestFontFace(aFontAttrs, 1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", OUString("Test font face"),
                                 aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", OUString("Test font face"),
                                 aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Alias names", OUString("Alias name"), aFamily.GetAliasNames());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", 10, aFamily.GetMinQuality());
    FontTypeFaces eTypeFace = FontTypeFaces::Scalable | FontTypeFaces::NoneSymbol
                              | FontTypeFaces::Bold | FontTypeFaces::NoneItalic;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", eTypeFace, aFamily.GetTypeFaces());
}

void VclPhysicalFontFamilyTest::testAddTwoFontFaces()
{
    PhysicalFontFamily aFamily("Test font face");

    FontAttributes aFontAttrs;
    aFontAttrs.SetFamilyName("Test font face");
    aFontAttrs.AddMapName("Alias name");
    aFontAttrs.SetFamilyType(FontFamily::FAMILY_ROMAN);
    aFontAttrs.SetPitch(FontPitch::PITCH_VARIABLE);
    aFontAttrs.SetItalic(FontItalic::ITALIC_NONE);
    aFontAttrs.SetQuality(10);
    aFontAttrs.SetWeight(FontWeight::WEIGHT_THIN);
    aFontAttrs.SetWidthType(FontWidth::WIDTH_EXPANDED);

    aFamily.AddFontFace(new TestFontFace(aFontAttrs, 1));

    aFontAttrs.SetFamilyName("Test font face");
    aFontAttrs.AddMapName("Alias name 2");
    aFontAttrs.SetFamilyType(FontFamily::FAMILY_ROMAN);
    aFontAttrs.SetPitch(FontPitch::PITCH_VARIABLE);
    aFontAttrs.SetItalic(FontItalic::ITALIC_NORMAL);
    aFontAttrs.SetQuality(5);
    aFontAttrs.SetWeight(FontWeight::WEIGHT_BOLD);
    aFontAttrs.SetWidthType(FontWidth::WIDTH_CONDENSED);

    aFamily.AddFontFace(new TestFontFace(aFontAttrs, 2));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", OUString("Test font face"),
                                 aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", OUString("Test font face"),
                                 aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Alias names", OUString("Alias name"), aFamily.GetAliasNames());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", 5, aFamily.GetMinQuality());
    FontTypeFaces eTypeFace = FontTypeFaces::Scalable | FontTypeFaces::NoneSymbol
                              | FontTypeFaces::Light | FontTypeFaces::Bold
                              | FontTypeFaces::NoneItalic | FontTypeFaces::Italic;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", eTypeFace, aFamily.GetTypeFaces());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontFamilyTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
