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
    PhysicalFontFamily aFamily(u"Test font face"_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", u""_ustr, aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", u"Test font face"_ustr, aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", -1, aFamily.GetMinQuality());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", FontTypeFaces::NONE, aFamily.GetTypeFaces());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match family name", u""_ustr, aFamily.GetMatchFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match type", ImplFontAttrs::None, aFamily.GetMatchType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match weight", WEIGHT_DONTKNOW, aFamily.GetMatchWeight());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match width", WIDTH_DONTKNOW, aFamily.GetMatchWidth());
}

void VclPhysicalFontFamilyTest::testAddFontFace_Default()
{
    PhysicalFontFamily aFamily(u"Test font face"_ustr);

    aFamily.AddFontFace(new TestFontFace(1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", u""_ustr, aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", u"Test font face"_ustr, aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", 0, aFamily.GetMinQuality());
    FontTypeFaces eTypeFace
        = FontTypeFaces::Scalable | FontTypeFaces::NoneSymbol | FontTypeFaces::NoneItalic;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", eTypeFace, aFamily.GetTypeFaces());
}

void VclPhysicalFontFamilyTest::testAddOneFontFace()
{
    PhysicalFontFamily aFamily(u"Test font face"_ustr);

    FontAttributes aFontAttrs;
    aFontAttrs.SetFamilyName(u"Test font face"_ustr);
    aFontAttrs.SetFamilyType(FontFamily::FAMILY_ROMAN);
    aFontAttrs.SetPitch(FontPitch::PITCH_VARIABLE);
    aFontAttrs.SetItalic(FontItalic::ITALIC_NONE);
    aFontAttrs.SetQuality(10);
    aFontAttrs.SetWeight(FontWeight::WEIGHT_BOLD);
    aFontAttrs.SetWidthType(FontWidth::WIDTH_EXPANDED);

    aFamily.AddFontFace(new TestFontFace(aFontAttrs, 1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", u"Test font face"_ustr, aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", u"Test font face"_ustr, aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", 10, aFamily.GetMinQuality());
    FontTypeFaces eTypeFace = FontTypeFaces::Scalable | FontTypeFaces::NoneSymbol
                              | FontTypeFaces::Bold | FontTypeFaces::NoneItalic;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", eTypeFace, aFamily.GetTypeFaces());
}

void VclPhysicalFontFamilyTest::testAddTwoFontFaces()
{
    PhysicalFontFamily aFamily(u"Test font face"_ustr);

    FontAttributes aFontAttrs;
    aFontAttrs.SetFamilyName(u"Test font face"_ustr);
    aFontAttrs.SetFamilyType(FontFamily::FAMILY_ROMAN);
    aFontAttrs.SetPitch(FontPitch::PITCH_VARIABLE);
    aFontAttrs.SetItalic(FontItalic::ITALIC_NONE);
    aFontAttrs.SetQuality(10);
    aFontAttrs.SetWeight(FontWeight::WEIGHT_THIN);
    aFontAttrs.SetWidthType(FontWidth::WIDTH_EXPANDED);

    aFamily.AddFontFace(new TestFontFace(aFontAttrs, 1));

    aFontAttrs.SetFamilyName(u"Test font face"_ustr);
    aFontAttrs.SetFamilyType(FontFamily::FAMILY_ROMAN);
    aFontAttrs.SetPitch(FontPitch::PITCH_VARIABLE);
    aFontAttrs.SetItalic(FontItalic::ITALIC_NORMAL);
    aFontAttrs.SetQuality(5);
    aFontAttrs.SetWeight(FontWeight::WEIGHT_BOLD);
    aFontAttrs.SetWidthType(FontWidth::WIDTH_CONDENSED);

    aFamily.AddFontFace(new TestFontFace(aFontAttrs, 2));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", u"Test font face"_ustr, aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", u"Test font face"_ustr, aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", 5, aFamily.GetMinQuality());
    FontTypeFaces eTypeFace = FontTypeFaces::Scalable | FontTypeFaces::NoneSymbol
                              | FontTypeFaces::Light | FontTypeFaces::Bold
                              | FontTypeFaces::NoneItalic | FontTypeFaces::Italic;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", eTypeFace, aFamily.GetTypeFaces());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontFamilyTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
