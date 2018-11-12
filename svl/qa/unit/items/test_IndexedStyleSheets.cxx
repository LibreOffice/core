/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/IndexedStyleSheets.hxx>

#include <svl/style.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <algorithm>

using namespace svl;

class MockedStyleSheet : public SfxStyleSheetBase
{
    public:
    MockedStyleSheet(const OUString& name, SfxStyleFamily fam = SfxStyleFamily::Char)
    : SfxStyleSheetBase(name, nullptr, fam, SfxStyleSearchBits::Auto)
    {}

};

struct DummyPredicate : public StyleSheetPredicate {
    bool Check(const SfxStyleSheetBase&) override {
        return true;
    }
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
    void PositionCanBeQueriedByFamily();
    void OnlyOneStyleSheetIsReturnedWhenReturnFirstIsUsed();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(IndexedStyleSheetsTest);

    CPPUNIT_TEST(InstantiationWorks);
    CPPUNIT_TEST(AddedStylesheetsCanBeFoundAndRetrievedByPosition);
    CPPUNIT_TEST(AddingSameStylesheetTwiceHasNoEffect);
    CPPUNIT_TEST(RemovedStyleSheetIsNotFound);
    CPPUNIT_TEST(RemovingStyleSheetWhichIsNotAvailableHasNoEffect);
    CPPUNIT_TEST(StyleSheetsCanBeRetrievedByTheirName);
    CPPUNIT_TEST(KnowsThatItStoresAStyleSheet);
    CPPUNIT_TEST(PositionCanBeQueriedByFamily);
    CPPUNIT_TEST(OnlyOneStyleSheetIsReturnedWhenReturnFirstIsUsed);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();

};

void IndexedStyleSheetsTest::InstantiationWorks()
{
    IndexedStyleSheets iss;
}

void IndexedStyleSheetsTest::AddedStylesheetsCanBeFoundAndRetrievedByPosition()
{
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet("name1"));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet("name2"));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    iss.AddStyleSheet(sheet2);
    unsigned pos = iss.FindStyleSheetPosition(*sheet2);
    rtl::Reference<SfxStyleSheetBase> retrieved = iss.GetStyleSheetByPosition(pos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("retrieved sheet is that which has been inserted.", sheet2.get(), retrieved.get());
}

void IndexedStyleSheetsTest::AddingSameStylesheetTwiceHasNoEffect()
{
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet("sheet1"));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    CPPUNIT_ASSERT_EQUAL(1u, iss.GetNumberOfStyleSheets());
    iss.AddStyleSheet(sheet1);
    CPPUNIT_ASSERT_EQUAL(1u, iss.GetNumberOfStyleSheets());
}

void IndexedStyleSheetsTest::RemovedStyleSheetIsNotFound()
{
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet("name1"));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet("name2"));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    iss.AddStyleSheet(sheet2);
    iss.RemoveStyleSheet(sheet1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Removed style sheet is not found.",
            false, iss.HasStyleSheet(sheet1));
}

void IndexedStyleSheetsTest::RemovingStyleSheetWhichIsNotAvailableHasNoEffect()
{
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet("sheet1"));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet("sheet2"));
    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    CPPUNIT_ASSERT_EQUAL(1u, iss.GetNumberOfStyleSheets());
    iss.RemoveStyleSheet(sheet2);
    CPPUNIT_ASSERT_EQUAL(1u, iss.GetNumberOfStyleSheets());
}

void IndexedStyleSheetsTest::StyleSheetsCanBeRetrievedByTheirName()
{
    OUString name1("name1");
    OUString name2("name2");
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
    std::sort (r.begin(), r.end());
    CPPUNIT_ASSERT_EQUAL(0u, r.at(0));
    CPPUNIT_ASSERT_EQUAL(2u, r.at(1));

    r = iss.FindPositionsByName(name2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One style sheets is found by 'name2'",
            1u, static_cast<unsigned>(r.size()));
    CPPUNIT_ASSERT_EQUAL(1u, r.at(0));
}

void IndexedStyleSheetsTest::KnowsThatItStoresAStyleSheet()
{
    OUString const name1("name1");
    OUString const name2("name2");
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

void IndexedStyleSheetsTest::PositionCanBeQueriedByFamily()
{
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet("name1", SfxStyleFamily::Char));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet("name2", SfxStyleFamily::Para));
    rtl::Reference<SfxStyleSheetBase> sheet3(new MockedStyleSheet("name3", SfxStyleFamily::Char));

    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    iss.AddStyleSheet(sheet2);
    iss.AddStyleSheet(sheet3);

    const std::vector<unsigned>& v = iss.GetStyleSheetPositionsByFamily(SfxStyleFamily::Char);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Separation by family works.", static_cast<size_t>(2), v.size());

    const std::vector<unsigned>& w = iss.GetStyleSheetPositionsByFamily(SfxStyleFamily::All);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wildcard works for family queries.", static_cast<size_t>(3), w.size());
}

void IndexedStyleSheetsTest::OnlyOneStyleSheetIsReturnedWhenReturnFirstIsUsed()
{
    OUString name("name1");
    rtl::Reference<SfxStyleSheetBase> sheet1(new MockedStyleSheet(name, SfxStyleFamily::Char));
    rtl::Reference<SfxStyleSheetBase> sheet2(new MockedStyleSheet(name, SfxStyleFamily::Para));
    rtl::Reference<SfxStyleSheetBase> sheet3(new MockedStyleSheet(name, SfxStyleFamily::Char));

    IndexedStyleSheets iss;
    iss.AddStyleSheet(sheet1);
    iss.AddStyleSheet(sheet2);
    iss.AddStyleSheet(sheet3);

    DummyPredicate predicate; // returns always true, i.e., all style sheets match the predicate.

    std::vector<unsigned> v = iss.FindPositionsByNameAndPredicate(name, predicate,
            IndexedStyleSheets::SearchBehavior::ReturnFirst);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Only one style sheet is returned.", static_cast<size_t>(1), v.size());

    std::vector<unsigned> w = iss.FindPositionsByNameAndPredicate(name, predicate);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("All style sheets are returned.", static_cast<size_t>(3), w.size());
}

CPPUNIT_TEST_SUITE_REGISTRATION(IndexedStyleSheetsTest);

CPPUNIT_PLUGIN_IMPLEMENT();
