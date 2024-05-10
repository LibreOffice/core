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

#include <tools/degree.hxx>

#include <vcl/font.hxx>

#include <font/PhysicalFontFace.hxx>
#include <fontattributes.hxx>

#include "fontmocks.hxx"

#include <memory>

const sal_IntPtr FONTID = 1;

class VclPhysicalFontFaceTest : public test::BootstrapFixture
{
public:
    VclPhysicalFontFaceTest()
        : BootstrapFixture(true, false)
    {
    }

    void testShouldCompareAsLesserFontFaceWithShorterWidth();
    void testShouldCompareAsGreaterFontFaceWithLongerWidth();
    void testShouldCompareAsLesserFontFaceWithLighterWeight();
    void testShouldCompareAsGreaterFontFaceWithHeavierWeight();
    void testShouldCompareAsLesserFontFaceWithLesserItalics();
    void testShouldCompareAsGreaterFontFaceWithGreaterItalics();
    void testShouldCompareAsGreaterFontFaceWitHigherAlphabeticalFamilyName();
    void testShouldCompareAsGreaterFontFaceWitLesserAlphabeticalFamilyName();
    void testShouldCompareAsGreaterFontFaceWithHigherAlphabeticalStyleName();
    void testShouldCompareAsGreaterFontFaceWithLesserAlphabeticalStyleName();
    void testShouldCompareAsSameFontFace();
    void testMatchStatusValue();

    CPPUNIT_TEST_SUITE(VclPhysicalFontFaceTest);
    CPPUNIT_TEST(testShouldCompareAsLesserFontFaceWithShorterWidth);
    CPPUNIT_TEST(testShouldCompareAsGreaterFontFaceWithLongerWidth);
    CPPUNIT_TEST(testShouldCompareAsLesserFontFaceWithLighterWeight);
    CPPUNIT_TEST(testShouldCompareAsGreaterFontFaceWithHeavierWeight);
    CPPUNIT_TEST(testShouldCompareAsLesserFontFaceWithLesserItalics);
    CPPUNIT_TEST(testShouldCompareAsGreaterFontFaceWithGreaterItalics);
    CPPUNIT_TEST(testShouldCompareAsGreaterFontFaceWitHigherAlphabeticalFamilyName);
    CPPUNIT_TEST(testShouldCompareAsGreaterFontFaceWitLesserAlphabeticalFamilyName);
    CPPUNIT_TEST(testShouldCompareAsGreaterFontFaceWithHigherAlphabeticalStyleName);
    CPPUNIT_TEST(testShouldCompareAsGreaterFontFaceWithLesserAlphabeticalStyleName);
    CPPUNIT_TEST(testShouldCompareAsSameFontFace);
    CPPUNIT_TEST(testMatchStatusValue);

    CPPUNIT_TEST_SUITE_END();
};

void VclPhysicalFontFaceTest::testShouldCompareAsLesserFontFaceWithShorterWidth()
{
    FontAttributes aFontAttrsShorterWidth;
    aFontAttrsShorterWidth.SetWidthType(WIDTH_CONDENSED);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrsShorterWidth, FONTID));

    FontAttributes aFontAttrsLongerWidth;
    aFontAttrsLongerWidth.SetWidthType(WIDTH_NORMAL);
    rtl::Reference<TestFontFace> aComparedToFontFace(
        new TestFontFace(aFontAttrsLongerWidth, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithLongerWidth()
{
    FontAttributes aFontAttrsLongerWidth;
    aFontAttrsLongerWidth.SetWidthType(WIDTH_NORMAL);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrsLongerWidth, FONTID));

    FontAttributes aFontAttrsShorterWidth;
    aFontAttrsShorterWidth.SetWidthType(WIDTH_CONDENSED);
    rtl::Reference<TestFontFace> aComparedToFontFace(
        new TestFontFace(aFontAttrsShorterWidth, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsLesserFontFaceWithLighterWeight()
{
    FontAttributes aFontAttrsLighterWeight;
    aFontAttrsLighterWeight.SetWidthType(WIDTH_NORMAL);
    aFontAttrsLighterWeight.SetWeight(WEIGHT_THIN);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrsLighterWeight, FONTID));

    FontAttributes aFontAttrsHeavierWeight;
    aFontAttrsHeavierWeight.SetWeight(WEIGHT_BOLD);
    aFontAttrsHeavierWeight.SetWidthType(WIDTH_NORMAL);
    rtl::Reference<TestFontFace> aComparedToFontFace(
        new TestFontFace(aFontAttrsHeavierWeight, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithHeavierWeight()
{
    FontAttributes aFontAttrsHeavierWeight;
    aFontAttrsHeavierWeight.SetWidthType(WIDTH_NORMAL);
    aFontAttrsHeavierWeight.SetWeight(WEIGHT_BOLD);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrsHeavierWeight, FONTID));

    FontAttributes aFontAttrsLighterWeight;
    aFontAttrsLighterWeight.SetWidthType(WIDTH_NORMAL);
    aFontAttrsLighterWeight.SetWeight(WEIGHT_THIN);
    rtl::Reference<TestFontFace> aComparedToFontFace(
        new TestFontFace(aFontAttrsLighterWeight, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsLesserFontFaceWithLesserItalics()
{
    FontAttributes aFontAttrsLesserItalics;
    aFontAttrsLesserItalics.SetWidthType(WIDTH_NORMAL);
    aFontAttrsLesserItalics.SetWeight(WEIGHT_THIN);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrsLesserItalics, FONTID));

    FontAttributes aFontAttrsGreaterItalics;
    aFontAttrsGreaterItalics.SetWeight(WEIGHT_BOLD);
    aFontAttrsGreaterItalics.SetWidthType(WIDTH_NORMAL);
    rtl::Reference<TestFontFace> aComparedToFontFace(
        new TestFontFace(aFontAttrsGreaterItalics, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithGreaterItalics()
{
    FontAttributes aFontAttrsGreaterItalics;
    aFontAttrsGreaterItalics.SetWidthType(WIDTH_NORMAL);
    aFontAttrsGreaterItalics.SetWeight(WEIGHT_BOLD);
    rtl::Reference<TestFontFace> aTestedFontFace(
        new TestFontFace(aFontAttrsGreaterItalics, FONTID));

    FontAttributes aFontAttrsLesserItalics;
    aFontAttrsLesserItalics.SetWidthType(WIDTH_NORMAL);
    aFontAttrsLesserItalics.SetWeight(WEIGHT_THIN);
    rtl::Reference<TestFontFace> aComparedToFontFace(
        new TestFontFace(aFontAttrsLesserItalics, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWitHigherAlphabeticalFamilyName()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName(u"B family"_ustr);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrs1, FONTID));

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName(u"A Family"_ustr);
    rtl::Reference<TestFontFace> aComparedToFontFace(new TestFontFace(aFontAttrs2, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWitLesserAlphabeticalFamilyName()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName(u"A family"_ustr);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrs1, FONTID));

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName(u"B Family"_ustr);
    rtl::Reference<TestFontFace> aComparedToFontFace(new TestFontFace(aFontAttrs2, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithHigherAlphabeticalStyleName()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName(u"DejaVu Sans"_ustr);
    aFontAttrs1.SetStyleName(u"B Style"_ustr);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrs1, FONTID));

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName(u"DejaVu Sans"_ustr);
    aFontAttrs2.SetStyleName(u"A Style"_ustr);
    rtl::Reference<TestFontFace> aComparedToFontFace(new TestFontFace(aFontAttrs2, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithLesserAlphabeticalStyleName()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName(u"DejaVu Sans"_ustr);
    aFontAttrs1.SetStyleName(u"A Style"_ustr);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrs1, FONTID));

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName(u"DejaVu Sans"_ustr);
    aFontAttrs2.SetStyleName(u"B Style"_ustr);
    rtl::Reference<TestFontFace> aComparedToFontFace(new TestFontFace(aFontAttrs2, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsSameFontFace()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName(u"DejaVu Sans"_ustr);
    aFontAttrs1.SetStyleName(u"Style"_ustr);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrs1, FONTID));

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName(u"DejaVu Sans"_ustr);
    aFontAttrs2.SetStyleName(u"Style"_ustr);
    rtl::Reference<TestFontFace> aComparedToFontFace(new TestFontFace(aFontAttrs2, FONTID));

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         aTestedFontFace->CompareIgnoreSize(*aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testMatchStatusValue()
{
    FontAttributes aFontAttrs;
    aFontAttrs.SetFamilyName(u"DejaVu Sans"_ustr);
    aFontAttrs.SetStyleName(u"Book"_ustr);
    aFontAttrs.SetPitch(FontPitch::PITCH_VARIABLE);
    aFontAttrs.SetWidthType(WIDTH_NORMAL);
    aFontAttrs.SetWeight(WEIGHT_BOLD);
    rtl::Reference<TestFontFace> aTestedFontFace(new TestFontFace(aFontAttrs, FONTID));

    std::unique_ptr<OUString> pTargetStyleName(new OUString(u"Book"_ustr));
    vcl::font::FontMatchStatus aFontMatchStatus = { 0, pTargetStyleName.get() };

    vcl::Font aTestFont(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 36));

    vcl::font::FontSelectPattern aFSP(aTestFont, u"DejaVu Sans"_ustr, Size(0, 36), 36, true);
    aFSP.mbEmbolden = false;
    aFSP.mnOrientation = Degree10(10);
    aFSP.SetWeight(WEIGHT_BOLD);
    aFSP.SetPitch(FontPitch::PITCH_VARIABLE);
    aFSP.maTargetName = "DejaVu Sans";

    const int EXPECTED_FAMILY = 240'000;
    const int EXPECTED_STYLE = 120'000;
    const int EXPECTED_PITCH = 20'000;
    const int EXPECTED_WIDTHTYPE = 400;
    const int EXPECTED_WEIGHT = 1'000;
    const int EXPECTED_ITALIC = 900;
    const int EXPECTED_ORIENTATION = 80;

    const int EXPECTED_MATCH = EXPECTED_FAMILY + EXPECTED_STYLE + EXPECTED_PITCH
                               + EXPECTED_WIDTHTYPE + EXPECTED_WEIGHT + EXPECTED_ITALIC
                               + EXPECTED_ORIENTATION;

    CPPUNIT_ASSERT(aTestedFontFace->IsBetterMatch(aFSP, aFontMatchStatus));
    CPPUNIT_ASSERT_EQUAL(EXPECTED_MATCH, aFontMatchStatus.mnFaceMatch);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontFaceTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
