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

    CPPUNIT_TEST_SUITE(VclPhysicalFontCollectionTest);
    CPPUNIT_TEST(testShouldCreateAndAddFontFamilyToCollection);
    CPPUNIT_TEST(testShouldFindFontFamily);
    CPPUNIT_TEST(testShouldNotFindFontFamily);
    CPPUNIT_TEST(testShouldFindFontFamilyByTokenNames);
    CPPUNIT_TEST(testShouldFindNoFamilyWithWorthlessAttributes);
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
                                 GetEnglishSearchFontName("Test Font Family Name"),
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
                                 GetEnglishSearchFontName("Test Font Family Name"),
                                 pFontFamily->GetSearchName());
}

void VclPhysicalFontCollectionTest::testShouldFindNoFamilyWithWorthlessAttributes()
{
    // note: you must normalize the search family name (first parameter of PhysicalFontFamily constructor)
    vcl::font::PhysicalFontCollection aFontCollection;
    aFontCollection.FindOrCreateFontFamily(GetEnglishSearchFontName("Test Font Family Name"));

    CPPUNIT_ASSERT(!aFontCollection.FindFontFamilyByAttributes(ImplFontAttrs::None, WEIGHT_NORMAL,
                                                               WIDTH_NORMAL, ITALIC_NONE, ""));
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontCollectionTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
