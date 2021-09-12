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

#include <font/PhysicalFontFace.hxx>
#include <fontattributes.hxx>

#include "fontmocks.hxx"

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
    CPPUNIT_TEST_SUITE_END();
};

void VclPhysicalFontFaceTest::testShouldCompareAsLesserFontFaceWithShorterWidth()
{
    FontAttributes aFontAttrsShorterWidth;
    aFontAttrsShorterWidth.SetWidthType(WIDTH_CONDENSED);
    TestFontFace aTestedFontFace(aFontAttrsShorterWidth, FONTID);

    FontAttributes aFontAttrsLongerWidth;
    aFontAttrsLongerWidth.SetWidthType(WIDTH_NORMAL);
    TestFontFace aComparedToFontFace(aFontAttrsLongerWidth, FONTID);

    CPPUNIT_ASSERT_EQUAL(-1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithLongerWidth()
{
    FontAttributes aFontAttrsLongerWidth;
    aFontAttrsLongerWidth.SetWidthType(WIDTH_NORMAL);
    TestFontFace aTestedFontFace(aFontAttrsLongerWidth, FONTID);

    FontAttributes aFontAttrsShorterWidth;
    aFontAttrsShorterWidth.SetWidthType(WIDTH_CONDENSED);
    TestFontFace aComparedToFontFace(aFontAttrsShorterWidth, FONTID);

    CPPUNIT_ASSERT_EQUAL(1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsLesserFontFaceWithLighterWeight()
{
    FontAttributes aFontAttrsLighterWeight;
    aFontAttrsLighterWeight.SetWidthType(WIDTH_NORMAL);
    aFontAttrsLighterWeight.SetWeight(WEIGHT_THIN);
    TestFontFace aTestedFontFace(aFontAttrsLighterWeight, FONTID);

    FontAttributes aFontAttrsHeavierWeight;
    aFontAttrsHeavierWeight.SetWeight(WEIGHT_BOLD);
    aFontAttrsHeavierWeight.SetWidthType(WIDTH_NORMAL);
    TestFontFace aComparedToFontFace(aFontAttrsHeavierWeight, FONTID);

    CPPUNIT_ASSERT_EQUAL(-1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithHeavierWeight()
{
    FontAttributes aFontAttrsHeavierWeight;
    aFontAttrsHeavierWeight.SetWidthType(WIDTH_NORMAL);
    aFontAttrsHeavierWeight.SetWeight(WEIGHT_BOLD);
    TestFontFace aTestedFontFace(aFontAttrsHeavierWeight, FONTID);

    FontAttributes aFontAttrsLighterWeight;
    aFontAttrsLighterWeight.SetWidthType(WIDTH_NORMAL);
    aFontAttrsLighterWeight.SetWeight(WEIGHT_THIN);
    TestFontFace aComparedToFontFace(aFontAttrsLighterWeight, FONTID);

    CPPUNIT_ASSERT_EQUAL(1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsLesserFontFaceWithLesserItalics()
{
    FontAttributes aFontAttrsLesserItalics;
    aFontAttrsLesserItalics.SetWidthType(WIDTH_NORMAL);
    aFontAttrsLesserItalics.SetWeight(WEIGHT_THIN);
    TestFontFace aTestedFontFace(aFontAttrsLesserItalics, FONTID);

    FontAttributes aFontAttrsGreaterItalics;
    aFontAttrsGreaterItalics.SetWeight(WEIGHT_BOLD);
    aFontAttrsGreaterItalics.SetWidthType(WIDTH_NORMAL);
    TestFontFace aComparedToFontFace(aFontAttrsGreaterItalics, FONTID);

    CPPUNIT_ASSERT_EQUAL(-1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithGreaterItalics()
{
    FontAttributes aFontAttrsGreaterItalics;
    aFontAttrsGreaterItalics.SetWidthType(WIDTH_NORMAL);
    aFontAttrsGreaterItalics.SetWeight(WEIGHT_BOLD);
    TestFontFace aTestedFontFace(aFontAttrsGreaterItalics, FONTID);

    FontAttributes aFontAttrsLesserItalics;
    aFontAttrsLesserItalics.SetWidthType(WIDTH_NORMAL);
    aFontAttrsLesserItalics.SetWeight(WEIGHT_THIN);
    TestFontFace aComparedToFontFace(aFontAttrsLesserItalics, FONTID);

    CPPUNIT_ASSERT_EQUAL(1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWitHigherAlphabeticalFamilyName()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName("B family");
    TestFontFace aTestedFontFace(aFontAttrs1, FONTID);

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName("A Family");
    TestFontFace aComparedToFontFace(aFontAttrs2, FONTID);

    CPPUNIT_ASSERT_EQUAL(1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWitLesserAlphabeticalFamilyName()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName("A family");
    TestFontFace aTestedFontFace(aFontAttrs1, FONTID);

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName("B Family");
    TestFontFace aComparedToFontFace(aFontAttrs2, FONTID);

    CPPUNIT_ASSERT_EQUAL(-1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithHigherAlphabeticalStyleName()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName("DejaVu Sans");
    aFontAttrs1.SetStyleName("B Style");
    TestFontFace aTestedFontFace(aFontAttrs1, FONTID);

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName("DejaVu Sans");
    aFontAttrs2.SetStyleName("A Style");
    TestFontFace aComparedToFontFace(aFontAttrs2, FONTID);

    CPPUNIT_ASSERT_EQUAL(1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsGreaterFontFaceWithLesserAlphabeticalStyleName()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName("DejaVu Sans");
    aFontAttrs1.SetStyleName("A Style");
    TestFontFace aTestedFontFace(aFontAttrs1, FONTID);

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName("DejaVu Sans");
    aFontAttrs2.SetStyleName("B Style");
    TestFontFace aComparedToFontFace(aFontAttrs2, FONTID);

    CPPUNIT_ASSERT_EQUAL(-1, aTestedFontFace.Compare(aComparedToFontFace));
}

void VclPhysicalFontFaceTest::testShouldCompareAsSameFontFace()
{
    FontAttributes aFontAttrs1;
    aFontAttrs1.SetWidthType(WIDTH_NORMAL);
    aFontAttrs1.SetWeight(WEIGHT_NORMAL);
    aFontAttrs1.SetFamilyName("DejaVu Sans");
    aFontAttrs1.SetStyleName("Style");
    TestFontFace aTestedFontFace(aFontAttrs1, FONTID);

    FontAttributes aFontAttrs2;
    aFontAttrs2.SetWidthType(WIDTH_NORMAL);
    aFontAttrs2.SetWeight(WEIGHT_NORMAL);
    aFontAttrs2.SetFamilyName("DejaVu Sans");
    aFontAttrs2.SetStyleName("Style");
    TestFontFace aComparedToFontFace(aFontAttrs2, FONTID);

    CPPUNIT_ASSERT_EQUAL(0, aTestedFontFace.Compare(aComparedToFontFace));
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontFaceTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
