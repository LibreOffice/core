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

#include <font/PhysicalFontCollection.hxx>
#include <font/PhysicalFontFamily.hxx>

#include "fontmocks.hxx"

#include <memory>

const int FONTID = 1;

class VclPhysicalFontCollectionTest : public test::BootstrapFixture
{
public:
    VclPhysicalFontCollectionTest()
        : BootstrapFixture(true, false)
    {
    }

    void testShouldCreateAndAddFontFamilyToCollection();
    void testShouldFindFontFamily();
    void testShouldNotFindFontFamily();
    void testShouldFindFontFamilyByTokenNames();
    void testShouldFindNoFamilyWithWorthlessAttributes();
    void testShouldFindCJKFamily();
    void testShouldNotFindCJKFamily();
    void testShouldFindCTLFamily();
    void testShouldNotFindCTLFamily();
    void testShouldFindStarsymbolFamily();
    void testShouldFindOpensymbolFamilyWithMultipleSymbolFamilies();
    void testShouldFindSymboltypeFamily();
    void testShouldFindSymbolFamilyByMatchType();
    void testImpossibleSymbolFamily();
    void testShouldNotFindSymbolFamily();

    CPPUNIT_TEST_SUITE(VclPhysicalFontCollectionTest);
    CPPUNIT_TEST(testShouldCreateAndAddFontFamilyToCollection);
    CPPUNIT_TEST(testShouldFindFontFamily);
    CPPUNIT_TEST(testShouldNotFindFontFamily);
    CPPUNIT_TEST(testShouldFindFontFamilyByTokenNames);
    CPPUNIT_TEST(testShouldFindNoFamilyWithWorthlessAttributes);
    CPPUNIT_TEST(testShouldFindCJKFamily);
    CPPUNIT_TEST(testShouldNotFindCJKFamily);
    CPPUNIT_TEST(testShouldFindCTLFamily);
    CPPUNIT_TEST(testShouldNotFindCTLFamily);
    CPPUNIT_TEST(testShouldFindStarsymbolFamily);
    CPPUNIT_TEST(testShouldFindOpensymbolFamilyWithMultipleSymbolFamilies);
    CPPUNIT_TEST(testShouldFindSymboltypeFamily);
    CPPUNIT_TEST(testShouldFindSymbolFamilyByMatchType);
    CPPUNIT_TEST(testImpossibleSymbolFamily);
    CPPUNIT_TEST(testShouldNotFindSymbolFamily);
    CPPUNIT_TEST_SUITE_END();
};

void VclPhysicalFontCollectionTest::testShouldCreateAndAddFontFamilyToCollection()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Empty font collection", static_cast<int>(0),
                                 aFontCollection.Count());

    // please note that fonts created this way are NOT normalized and will not be found if you search for them in the collection
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("Test Font Family Name");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One font family in collection", static_cast<int>(1),
                                 aFontCollection.Count());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font family name correct", OUString("Test Font Family Name"),
                                 pFontFamily->GetSearchName());

    vcl::font::PhysicalFontFamily* pFontFamily2
        = aFontCollection.FindOrCreateFontFamily("Test Font Family Name");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Still only one font family in collection", static_cast<int>(1),
                                 aFontCollection.Count());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font family name correct", OUString("Test Font Family Name"),
                                 pFontFamily2->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldFindFontFamily()
{
    // note: you must normalize the search family name (first parameter of PhysicalFontFamily constructor)
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("Test Font Family Name"));

    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindFontFamily("Test Font Family Name");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font family name correct",
                                 GetEnglishSearchFontName(OUString("Test Font Family Name")),
                                 pFontFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldNotFindFontFamily()
{
    // note: you must normalize the search family name (first parameter of PhysicalFontFamily constructor)
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("Test Font Family Name"));

    CPPUNIT_ASSERT(!aFontCollection.FindFontFamily("blah"));
}

void VclPhysicalFontCollectionTest::testShouldFindFontFamilyByTokenNames()
{
    // note: you must normalize the search family name (first parameter of PhysicalFontFamily constructor)
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("Test Font Family Name"));

    OUString sTokenNames(GetEnglishSearchFontName("Test Font Family Name;"));
    sTokenNames += GetEnglishSearchFontName("Test 2");

    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindFontFamilyByTokenNames("Test Font Family Name");
    CPPUNIT_ASSERT_MESSAGE("Found the font family", pFontFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font family name correct",
                                 GetEnglishSearchFontName(OUString("Test Font Family Name")),
                                 pFontFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldFindNoFamilyWithWorthlessAttributes()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("Test Font Family Name"));

    CPPUNIT_ASSERT(!aFontCollection.FindFontFamilyByAttributes(ImplFontAttrs::None, WEIGHT_NORMAL,
                                                               WIDTH_NORMAL, ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldFindCJKFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;

    // interestingly, you need to normalize the name still
    vcl::font::PhysicalFontFamily* pFontFamily = aFontCollection.FindOrCreateFontFamily(
        GetEnglishSearchFontName(u"시험")); // Korean for "test"

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyName(u"시험");
    pFontFamily->AddFontFace(new TestFontFace(aFontAttr, FONTID));

    vcl::font::PhysicalFontFamily* pCJKFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::CJK, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, "");
    CPPUNIT_ASSERT_MESSAGE("family found", pCJKFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cjk family found", GetEnglishSearchFontName(u"시험"),
                                 pCJKFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldNotFindCJKFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("No CJK characters");

    CPPUNIT_ASSERT_MESSAGE("family not found", !aFontCollection.FindFontFamilyByAttributes(
                                                   ImplFontAttrs::CJK_AllLang, WEIGHT_NORMAL,
                                                   WIDTH_NORMAL, ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldFindCTLFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;

    // interestingly, you need to normalize the name still
    vcl::font::PhysicalFontFamily* pFontFamily = aFontCollection.FindOrCreateFontFamily(
        GetEnglishSearchFontName(u"اختبار")); // Arabic for "test"

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyName(u"시험");
    pFontFamily->AddFontFace(new TestFontFace(aFontAttr, FONTID));

    vcl::font::PhysicalFontFamily* pCJKFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::CJK, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, "");
    CPPUNIT_ASSERT_MESSAGE("family found", pCJKFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cjk family found", GetEnglishSearchFontName(u"اختبار"),
                                 pCJKFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldNotFindCTLFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("No CJK characters");

    CPPUNIT_ASSERT_MESSAGE("family not found",
                           !aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::CTL, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldFindStarsymbolFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("starsymbol");

    CPPUNIT_ASSERT_MESSAGE("starsymbol created", pFontFamily);

    vcl::font::PhysicalFontFamily* pStarsymbolFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::Symbol, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, "");
    CPPUNIT_ASSERT_MESSAGE("family found", pStarsymbolFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("starsymbol family found", OUString("starsymbol"),
                                 pStarsymbolFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldFindOpensymbolFamilyWithMultipleSymbolFamilies()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("opensymbol");
    aFontCollection.FindOrCreateFontFamily("wingdings");

    vcl::font::PhysicalFontFamily* pStarsymbolFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::Symbol, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, "");
    CPPUNIT_ASSERT_MESSAGE("family found", pStarsymbolFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("starsymbol family found", OUString("opensymbol"),
                                 pStarsymbolFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldFindSymboltypeFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("testsymbolfamily");

    FontAttributes aFontAttr;
    aFontAttr.SetSymbolFlag(true);
    pFontFamily->AddFontFace(new TestFontFace(aFontAttr, FONTID));

    vcl::font::PhysicalFontFamily* pSymbolFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::Symbol, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, "");
    CPPUNIT_ASSERT_MESSAGE("family found", pSymbolFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("test symbol family found", OUString("testsymbolfamily"),
                                 pSymbolFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldFindSymbolFamilyByMatchType()
{
    // TODO: figure out how to test matchtype with ImplFontAttrs::Full

    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily = aFontCollection.FindOrCreateFontFamily("symbols");

    FontAttributes aFontAttr;
    aFontAttr.SetSymbolFlag(false);
    pFontFamily->AddFontFace(new TestFontFace(aFontAttr, FONTID));

    vcl::font::PhysicalFontFamily* pSymbolFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::Symbol, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, "");
    CPPUNIT_ASSERT_MESSAGE("family found", pSymbolFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("test symbol family found", OUString("symbols"),
                                 pSymbolFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testImpossibleSymbolFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("testsymbolfamily");

    FontAttributes aFontAttr;
    aFontAttr.SetSymbolFlag(true);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("match for family not possible",
                           !aFontCollection.FindFontFamilyByAttributes(ImplFontAttrs::Normal,
                                                                       WEIGHT_NORMAL, WIDTH_NORMAL,
                                                                       ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldNotFindSymbolFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("symbol");

    CPPUNIT_ASSERT_MESSAGE("No family found", !aFontCollection.FindFontFamilyByAttributes(
                                                  ImplFontAttrs::Normal, WEIGHT_NORMAL,
                                                  WIDTH_NORMAL, ITALIC_NONE, ""));
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontCollectionTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
