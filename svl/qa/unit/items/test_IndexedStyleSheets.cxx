/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/IndexedStyleSheets.hxx>

// for SfxStyleSheetBase
#include <svl/style.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

using namespace svl;

class MockedStyleSheet : public SfxStyleSheetBase
{
    public:
    MockedStyleSheet(const rtl::OUString& name)
    : SfxStyleSheetBase(name, NULL, SFX_STYLE_FAMILY_CHAR, 0)
    {;}

};

class IndexedStyleSheetsTest : public CppUnit::TestFixture
{
    void InstantiationWorks();
    void AddedStylesheetsCanBeFoundAndRetrievedByPosition();
    void AddingSameStylesheetTwiceHasNoEffect();
    void RemovedStyleSheetIsNotFound();
    void RemovingStyleSheetWhichIsNotAvailableHasNoEffect();
    void StyleSheetsCanBeRetrievedByTheirName();
    void KnowsThatItStoresAStyleSheet();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(IndexedStyleSheetsTest);

    CPPUNIT_TEST(InstantiationWorks);
    CPPUNIT_TEST(AddedStylesheetsCanBeFoundAndRetrievedByPosition);
    CPPUNIT_TEST(AddingSameStylesheetTwiceHasNoEffect);
    CPPUNIT_TEST(RemovedStyleSheetIsNotFound);
    CPPUNIT_TEST(RemovingStyleSheetWhichIsNotAvailableHasNoEffect);
    CPPUNIT_TEST(StyleSheetsCanBeRetrievedByTheirName);
    CPPUNIT_TEST(KnowsThatItStoresAStyleSheet);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();

};

void IndexedStyleSheetsTest::InstantiationWorks()
{
    IndexedStyleSheets iss;
}

void IndexedStyleSheetsTest::AddedStylesheetsCanBeFoundAndRetrievedByPosition()
{
    rtl::OUString name1("name1");
    rtl::OUString name2("name2");
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet(name1));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet(name2));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    iss.AddStyleSheet(sheet2);
    unsigned pos = iss.FindStyleSheetPosition(*sheet2);
    rtl::Reference<SfxStyleSheetBase> retrieved = iss.GetStyleSheetByPosition(pos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("retrieved sheet is that which has been inserted.", sheet2.get(), retrieved.get());
}

void IndexedStyleSheetsTest::AddingSameStylesheetTwiceHasNoEffect()
{
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet(rtl::OUString("sheet1")));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    CPPUNIT_ASSERT_EQUAL(1u, iss.GetNumberOfStyleSheets());
    iss.AddStyleSheet(sheet1);
    CPPUNIT_ASSERT_EQUAL(1u, iss.GetNumberOfStyleSheets());
}

void IndexedStyleSheetsTest::RemovedStyleSheetIsNotFound()
{
    rtl::OUString name1("name1");
    rtl::OUString name2("name2");
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet(name1));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet(name2));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    iss.AddStyleSheet(sheet2);
    iss.RemoveStyleSheet(sheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Removed style sheet is not found.",
            false, iss.HasStyleSheet(sheet1));
}

void IndexedStyleSheetsTest::RemovingStyleSheetWhichIsNotAvailableHasNoEffect()
{
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet(rtl::OUString("sheet1")));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet(rtl::OUString("sheet2")));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    CPPUNIT_ASSERT_EQUAL(1u, iss.GetNumberOfStyleSheets());
    iss.RemoveStyleSheet(sheet2);
    CPPUNIT_ASSERT_EQUAL(1u, iss.GetNumberOfStyleSheets());
}

void IndexedStyleSheetsTest::StyleSheetsCanBeRetrievedByTheirName()
{
    rtl::OUString name1("name1");
    rtl::OUString name2("name2");
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet(name1));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet(name2));
    rtl::Reference<SfxStyleSheetBase> sheet3(new MockedStyleSheet(name1));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    iss.AddStyleSheet(sheet2);
    iss.AddStyleSheet(sheet3);

    std::vector<unsigned> r = iss.FindPositionsByName(name1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Two style sheets are found by 'name1'",
            2u, static_cast<unsigned>(r.size()));
    CPPUNIT_ASSERT_EQUAL(0u, r.at(0));
    CPPUNIT_ASSERT_EQUAL(2u, r.at(1));

    r = iss.FindPositionsByName(name2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One style sheets is found by 'name2'",
            1u, static_cast<unsigned>(r.size()));
    CPPUNIT_ASSERT_EQUAL(1u, r.at(0));
}

void IndexedStyleSheetsTest::KnowsThatItStoresAStyleSheet()
{
    rtl::OUString name1("name1");
    rtl::OUString name2("name2");
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet(name1));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet(name1));
    rtl::Reference<SfxStyleSheetBase> sheet3(new MockedStyleSheet(name2));
    rtl::Reference<SfxStyleSheetBase> sheet4(new MockedStyleSheet(name1));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    iss.AddStyleSheet(sheet2);
    iss.AddStyleSheet(sheet3);
    // do not add sheet 4

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Finds first stored style sheet even though two style sheets have the same name.",
            true, iss.HasStyleSheet(sheet1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Finds second stored style sheet even though two style sheets have the same name.",
            true, iss.HasStyleSheet(sheet2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Does not find style sheet which is not stored and has the same name as a stored.",
            false, iss.HasStyleSheet(sheet4));

}

CPPUNIT_TEST_SUITE_REGISTRATION(IndexedStyleSheetsTest);

CPPUNIT_PLUGIN_IMPLEMENT();
