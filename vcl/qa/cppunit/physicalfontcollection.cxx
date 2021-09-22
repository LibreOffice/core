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
    void testShouldFindStarsymbolFamily();
    void testShouldFindOpensymbolFamilyWithMultipleSymbolFamilies();
    void testShouldFindSymboltypeFamily();
    void testShouldFindSymbolFamilyByMatchType();
    void testImpossibleSymbolFamily();
    void testShouldNotFindSymbolFamily();
    void testShouldMatchFamilyName();
    void testShouldNotMatchFamilyName();
    void testShouldMatchBrushScriptFamily();
    void testShouldNotMatchBrushScriptFamily();
    void testShouldMatchFixedFamily();
    void testShouldNotMatchFixedFamily();
    void testShouldMatchSerifFamily();
    void testShouldNotMatchSerifFamily();
    void testShouldMatchSansSerifFamily();
    void testShouldNotMatchSansSerifFamily();
    void testShouldMatchDecorativeFamily();
    void testShouldNotMatchDecorativeFamily();
    void testShouldFindTitlingFamily();
    void testShouldNotFindTitlingFamily();
    void testShouldFindCapitalsFamily();
    void testShouldNotFindCapitalsFamily();
    void testShouldFindFamilyName();
    void testShouldNotFindFamilyName();

    CPPUNIT_TEST_SUITE(VclPhysicalFontCollectionTest);
    CPPUNIT_TEST(testShouldCreateAndAddFontFamilyToCollection);
    CPPUNIT_TEST(testShouldFindFontFamily);
    CPPUNIT_TEST(testShouldNotFindFontFamily);
    CPPUNIT_TEST(testShouldFindFontFamilyByTokenNames);
    CPPUNIT_TEST(testShouldFindNoFamilyWithWorthlessAttributes);
    CPPUNIT_TEST(testShouldFindCJKFamily);
    CPPUNIT_TEST(testShouldNotFindCJKFamily);
    CPPUNIT_TEST(testShouldFindStarsymbolFamily);
    CPPUNIT_TEST(testShouldFindOpensymbolFamilyWithMultipleSymbolFamilies);
    CPPUNIT_TEST(testShouldFindSymboltypeFamily);
    CPPUNIT_TEST(testShouldFindSymbolFamilyByMatchType);
    CPPUNIT_TEST(testImpossibleSymbolFamily);
    CPPUNIT_TEST(testShouldNotFindSymbolFamily);
    CPPUNIT_TEST(testShouldMatchFamilyName);
    CPPUNIT_TEST(testShouldNotMatchFamilyName);
    CPPUNIT_TEST(testShouldMatchBrushScriptFamily);
    CPPUNIT_TEST(testShouldNotMatchBrushScriptFamily);
    CPPUNIT_TEST(testShouldMatchFixedFamily);
    CPPUNIT_TEST(testShouldNotMatchFixedFamily);
    CPPUNIT_TEST(testShouldMatchSerifFamily);
    CPPUNIT_TEST(testShouldNotMatchSerifFamily);
    CPPUNIT_TEST(testShouldMatchSansSerifFamily);
    CPPUNIT_TEST(testShouldNotMatchSansSerifFamily);
    CPPUNIT_TEST(testShouldMatchDecorativeFamily);
    CPPUNIT_TEST(testShouldNotMatchDecorativeFamily);
    CPPUNIT_TEST(testShouldFindTitlingFamily);
    CPPUNIT_TEST(testShouldNotFindTitlingFamily);
    CPPUNIT_TEST(testShouldFindCapitalsFamily);
    CPPUNIT_TEST(testShouldNotFindCapitalsFamily);
    CPPUNIT_TEST(testShouldFindFamilyName);
    CPPUNIT_TEST(testShouldNotFindFamilyName);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Does not have only one font family in collection",
                                 static_cast<int>(1), aFontCollection.Count());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font family name not correct", OUString("Test Font Family Name"),
                                 pFontFamily->GetSearchName());

    vcl::font::PhysicalFontFamily* pFontFamily2
        = aFontCollection.FindOrCreateFontFamily("Test Font Family Name");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Still only one font family in collection", static_cast<int>(1),
                                 aFontCollection.Count());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font family name not correct", OUString("Test Font Family Name"),
                                 pFontFamily2->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldFindFontFamily()
{
    // note: you must normalize the search family name (first parameter of PhysicalFontFamily constructor)
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("Test Font Family Name"));

    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindFontFamily("Test Font Family Name");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font family name not correct",
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
    CPPUNIT_ASSERT_MESSAGE("Did not find the font family", pFontFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font family name incorrect",
                                 GetEnglishSearchFontName(OUString("Test Font Family Name")),
                                 pFontFamily->GetSearchName());
}

static void AddNormalWeightFontFace(vcl::font::PhysicalFontFamily* const pFontFamily)
{
    FontAttributes aFontAttr;
    aFontAttr.SetWeight(WEIGHT_NORMAL);
    pFontFamily->AddFontFace(new TestFontFace(aFontAttr, FONTID));
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
    aFontAttr.SetWeight(WEIGHT_NORMAL);
    pFontFamily->AddFontFace(new TestFontFace(aFontAttr, FONTID));

    vcl::font::PhysicalFontFamily* pCJKFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::CJK, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NORMAL, "");
    CPPUNIT_ASSERT_MESSAGE("family not found", pCJKFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cjk family not found", GetEnglishSearchFontName(u"시험"),
                                 pCJKFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldNotFindCJKFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("No CJK characters"));

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyName(u"No CJK characters");
    aFontAttr.SetWeight(WEIGHT_NORMAL);
    pFontFamily->AddFontFace(new TestFontFace(aFontAttr, FONTID));

    CPPUNIT_ASSERT_MESSAGE("family found",
                           !aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::CJK, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldFindStarsymbolFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("starsymbol");

    AddNormalWeightFontFace(pFontFamily);

    CPPUNIT_ASSERT_MESSAGE("starsymbol created", pFontFamily);

    vcl::font::PhysicalFontFamily* pStarsymbolFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::Symbol, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NORMAL, "");
    CPPUNIT_ASSERT_MESSAGE("family not found", pStarsymbolFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("starsymbol family not found", OUString("starsymbol"),
                                 pStarsymbolFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldFindOpensymbolFamilyWithMultipleSymbolFamilies()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("opensymbol");
    aFontCollection.FindOrCreateFontFamily("wingdings");

    vcl::font::PhysicalFontFamily* pStarsymbolFamily = aFontCollection.FindFontFamilyByAttributes(
        ImplFontAttrs::Symbol, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, "");
    CPPUNIT_ASSERT_MESSAGE("family not found", pStarsymbolFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("opensymbol family not found", OUString("opensymbol"),
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
    CPPUNIT_ASSERT_MESSAGE("family not found", pSymbolFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("test symbol family not found", OUString("testsymbolfamily"),
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
    CPPUNIT_ASSERT_MESSAGE("family not found", pSymbolFamily);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("test symbol family not found", OUString("symbols"),
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

    CPPUNIT_ASSERT_MESSAGE("match for family not possible, but was found anyway",
                           !aFontCollection.FindFontFamilyByAttributes(ImplFontAttrs::Normal,
                                                                       WEIGHT_NORMAL, WIDTH_NORMAL,
                                                                       ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldNotFindSymbolFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("symbol");

    CPPUNIT_ASSERT_MESSAGE(
        "Family found", !aFontCollection.FindFontFamilyByAttributes(
                            ImplFontAttrs::Normal, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldMatchFamilyName()
{
    vcl::font::PhysicalFontCollection aFontCollection;

    // note that for this test, it is irrelevant what the search name is for PhysicalFontFamily,
    // the font searches the family name and uses the search parameter of FindFontFamilyByAttributes()
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("Matching family name");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyName(GetEnglishSearchFontName("Matching family name"));
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("No family found",
                           aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::Normal, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE,
                               "Matching family name"));
}

void VclPhysicalFontCollectionTest::testShouldNotMatchFamilyName()
{
    vcl::font::PhysicalFontCollection aFontCollection;

    // note that for this test, it is irrelevant what the search name is for PhysicalFontFamily,
    // the font searches the family name and uses the search parameter of FindFontFamilyByAttributes()
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("Matching family name");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyName("Matching family name");
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("Family found", !aFontCollection.FindFontFamilyByAttributes(
                                               ImplFontAttrs::Normal, WEIGHT_NORMAL, WIDTH_NORMAL,
                                               ITALIC_NONE, "Non matching family name"));
}

void VclPhysicalFontCollectionTest::testShouldMatchBrushScriptFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("script");

    CPPUNIT_ASSERT_MESSAGE("Brush script family not found",
                           aFontCollection.FindFontFamilyByAttributes(ImplFontAttrs::BrushScript,
                                                                      WEIGHT_NORMAL, WIDTH_NORMAL,
                                                                      ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldNotMatchBrushScriptFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("testfont");

    CPPUNIT_ASSERT_MESSAGE("Brush script family found",
                           !aFontCollection.FindFontFamilyByAttributes(ImplFontAttrs::BrushScript,
                                                                       WEIGHT_NORMAL, WIDTH_NORMAL,
                                                                       ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldMatchFixedFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;

    // note that for this test, it is irrelevant what the search name is for PhysicalFontFamily,
    // the font searches the family name and uses the search parameter of FindFontFamilyByAttributes()
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("Matching family name");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyType(FAMILY_MODERN);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("No fixed family found",
                           aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::Fixed, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE,
                               "Matching family name"));
}

void VclPhysicalFontCollectionTest::testShouldNotMatchFixedFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;

    // note that for this test, it is irrelevant what the search name is for PhysicalFontFamily,
    // the font searches the family name and uses the search parameter of FindFontFamilyByAttributes()
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("Matching family name");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyType(FAMILY_SWISS);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("Fixed family found",
                           aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::Fixed, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE,
                               "Matching family name"));
}

void VclPhysicalFontCollectionTest::testShouldMatchSerifFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily = aFontCollection.FindOrCreateFontFamily("serif");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyType(FAMILY_ROMAN);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("Serif family not found",
                           aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::Serif, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldNotMatchSerifFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("sansserif");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyType(FAMILY_SWISS);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("Serif family found",
                           !aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::Serif, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldMatchSansSerifFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("sansserif");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyType(FAMILY_SWISS);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("SansSerif family not found",
                           aFontCollection.FindFontFamilyByAttributes(ImplFontAttrs::SansSerif,
                                                                      WEIGHT_NORMAL, WIDTH_NORMAL,
                                                                      ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldNotMatchSansSerifFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily = aFontCollection.FindOrCreateFontFamily("serif");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyType(FAMILY_ROMAN);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("SansSerif family found", !aFontCollection.FindFontFamilyByAttributes(
                                                         ImplFontAttrs::SansSerif, WEIGHT_NORMAL,
                                                         WIDTH_NORMAL, ITALIC_NONE, ""));
}

void VclPhysicalFontCollectionTest::testShouldMatchDecorativeFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("decorative");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyType(FAMILY_DECORATIVE);
    aFontAttr.SetWeight(WEIGHT_MEDIUM);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("Decorative family not found",
                           aFontCollection.FindFontFamilyByAttributes(ImplFontAttrs::Decorative,
                                                                      WEIGHT_NORMAL, WIDTH_NORMAL,
                                                                      ITALIC_NORMAL, ""));
}

void VclPhysicalFontCollectionTest::testShouldNotMatchDecorativeFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    vcl::font::PhysicalFontFamily* pFontFamily
        = aFontCollection.FindOrCreateFontFamily("notdecoractive");

    FontAttributes aFontAttr;
    aFontAttr.SetFamilyType(FAMILY_ROMAN);
    TestFontFace* pFontFace = new TestFontFace(aFontAttr, FONTID);
    pFontFamily->AddFontFace(pFontFace);

    CPPUNIT_ASSERT_MESSAGE("Decorative family found", !aFontCollection.FindFontFamilyByAttributes(
                                                          ImplFontAttrs::Decorative, WEIGHT_NORMAL,
                                                          WIDTH_NORMAL, ITALIC_NORMAL, ""));
}

void VclPhysicalFontCollectionTest::testShouldFindTitlingFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("titling");

    CPPUNIT_ASSERT_MESSAGE("Titling family not found", aFontCollection.FindFontFamilyByAttributes(
                                                           ImplFontAttrs::Titling, WEIGHT_NORMAL,
                                                           WIDTH_NORMAL, ITALIC_NORMAL, ""));
}

void VclPhysicalFontCollectionTest::testShouldNotFindTitlingFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("scalable");

    CPPUNIT_ASSERT_MESSAGE("Titling family found", !aFontCollection.FindFontFamilyByAttributes(
                                                       ImplFontAttrs::Titling, WEIGHT_NORMAL,
                                                       WIDTH_NORMAL, ITALIC_NORMAL, ""));
}

void VclPhysicalFontCollectionTest::testShouldFindCapitalsFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("caps");

    CPPUNIT_ASSERT_MESSAGE("All-caps family not found", aFontCollection.FindFontFamilyByAttributes(
                                                            ImplFontAttrs::Capitals, WEIGHT_NORMAL,
                                                            WIDTH_NORMAL, ITALIC_NORMAL, ""));
}

void VclPhysicalFontCollectionTest::testShouldNotFindCapitalsFamily()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily("scalable");

    CPPUNIT_ASSERT_MESSAGE("All-caps family found", !aFontCollection.FindFontFamilyByAttributes(
                                                        ImplFontAttrs::Capitals, WEIGHT_NORMAL,
                                                        WIDTH_NORMAL, ITALIC_NORMAL, ""));
}

void VclPhysicalFontCollectionTest::testShouldFindFamilyName()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("Test font name"));

    CPPUNIT_ASSERT_MESSAGE("Cannot find font name",
                           aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::Capitals, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NORMAL,
                               GetEnglishSearchFontName("Test font name")));
}

void VclPhysicalFontCollectionTest::testShouldNotFindFamilyName()
{
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("Test font name"));

    CPPUNIT_ASSERT_MESSAGE("Found font name",
                           !aFontCollection.FindFontFamilyByAttributes(
                               ImplFontAttrs::Capitals, WEIGHT_NORMAL, WIDTH_NORMAL, ITALIC_NORMAL,
                               GetEnglishSearchFontName("Not test font name")));
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontCollectionTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
