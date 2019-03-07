/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <chpfld.hxx>
#include <ndtxt.hxx>
#include <tox.hxx>
#include <txmsrt.hxx>
#include <ToxTextGenerator.hxx>
#include <ToxTabStopTokenHandler.hxx>

#include <memory>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <swdll.hxx>

using namespace sw;

class ToxTextGeneratorTest : public CppUnit::TestFixture {
public:
    virtual void setUp() override
    {
        SwGlobals::ensure();
    }

    void EmptyStringIsReturnedForPageNumberPlaceholderOfZeroItems();
    void OneAtSignIsReturnedForPageNumberPlaceholderOfOneItem();
    void TwoAtSignsAreReturnedForPageNumberPlaceholderOfOneItem();
    void EmptyStringIsReturnedAsNumStringIfNoTextMarkIsSet();
    void EmptyStringIsReturnedAsNumStringIfToxSourcesIsEmpty();
    void ChapterNumberWithoutTextIsGeneratedForNoprepstTitle();
    void ChapterNumberWithTitleIsGeneratedForNumberNoPrepst();

    CPPUNIT_TEST_SUITE(ToxTextGeneratorTest);
    CPPUNIT_TEST(EmptyStringIsReturnedForPageNumberPlaceholderOfZeroItems);
    CPPUNIT_TEST(OneAtSignIsReturnedForPageNumberPlaceholderOfOneItem);
    CPPUNIT_TEST(TwoAtSignsAreReturnedForPageNumberPlaceholderOfOneItem);
    CPPUNIT_TEST(EmptyStringIsReturnedAsNumStringIfNoTextMarkIsSet);
    CPPUNIT_TEST(EmptyStringIsReturnedAsNumStringIfToxSourcesIsEmpty);
    CPPUNIT_TEST(ChapterNumberWithoutTextIsGeneratedForNoprepstTitle);
    CPPUNIT_TEST(ChapterNumberWithTitleIsGeneratedForNumberNoPrepst);
    CPPUNIT_TEST_SUITE_END();

};

struct MockedSortTab : public SwTOXSortTabBase {
    MockedSortTab()
    : SwTOXSortTabBase(TOX_SORT_INDEX,nullptr,nullptr,nullptr) {}

    virtual TextAndReading GetText_Impl(SwRootFrame const*) const override {
        return TextAndReading();
    }
    virtual sal_uInt16  GetLevel() const override {
        return 0;
    }
};

void
ToxTextGeneratorTest::EmptyStringIsReturnedForPageNumberPlaceholderOfZeroItems()
{
    OUString const expected("");
    OUString actual = ToxTextGenerator::ConstructPageNumberPlaceholder(0);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void
ToxTextGeneratorTest::OneAtSignIsReturnedForPageNumberPlaceholderOfOneItem()
{
    OUString const expected("@~");
    OUString actual = ToxTextGenerator::ConstructPageNumberPlaceholder(1);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void
ToxTextGeneratorTest::TwoAtSignsAreReturnedForPageNumberPlaceholderOfOneItem()
{
    OUString const expected("@, @~");
    OUString actual = ToxTextGenerator::ConstructPageNumberPlaceholder(2);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void
ToxTextGeneratorTest::EmptyStringIsReturnedAsNumStringIfNoTextMarkIsSet()
{
    MockedSortTab sortTab;
    sortTab.pTextMark = nullptr;

    OUString const expected("");
    OUString actual = ToxTextGenerator::GetNumStringOfFirstNode(sortTab, false, 0, nullptr);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void
ToxTextGeneratorTest::EmptyStringIsReturnedAsNumStringIfToxSourcesIsEmpty()
{
    MockedSortTab sortTab;
    sortTab.pTextMark = reinterpret_cast<SwTextTOXMark*>(1);

    OUString const expected("");
    OUString actual = ToxTextGenerator::GetNumStringOfFirstNode(sortTab, false, 0, nullptr);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

class MockedToxTabStopTokenHandler : public ToxTabStopTokenHandler {
public:
    virtual HandledTabStopToken
    HandleTabStopToken(const SwFormToken&, const SwTextNode&,
            const SwRootFrame *) const override {
        return HandledTabStopToken();
    }
};

class ToxTextGeneratorWithMockedChapterField : public ToxTextGenerator {
public:
    explicit ToxTextGeneratorWithMockedChapterField(SwForm const &form)
    : ToxTextGenerator(form, std::make_shared<MockedToxTabStopTokenHandler>()),
      mChapterFieldType(), mChapterField(&mChapterFieldType) {}

    SwChapterField&
    GetChapterField() {
        return mChapterField;
    }

private:
    SwChapterField
    ObtainChapterField(SwChapterFieldType*, const SwFormToken*,
            const SwContentFrame*, const SwContentNode *) const override {
        return mChapterField;
    }

    SwChapterFieldType mChapterFieldType;
    SwChapterField mChapterField;
};

void
ToxTextGeneratorTest::ChapterNumberWithoutTextIsGeneratedForNoprepstTitle()
{
    SwForm form;
    ToxTextGeneratorWithMockedChapterField ttg(form);
    // set all values to make sure they are not used
    ttg.GetChapterField().m_State.sNumber = "1";
    ttg.GetChapterField().m_State.sPre = "PRE";
    ttg.GetChapterField().m_State.sPost = "POST";
    ttg.GetChapterField().m_State.sTitle = "TITLE";

    SwFormToken token(TOKEN_CHAPTER_INFO);
    token.nChapterFormat = CF_NUM_NOPREPST_TITLE;

    OUString expected("1");
    OUString actual = ttg.GenerateTextForChapterToken(token, nullptr, nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // we cannot mock the pre- and suffix generation in the chapterfield. We just test that sNumber and
    // sTitle are used and hope that the pre- and suffix addition works.
    token.nChapterFormat = CF_NUMBER;
    expected = ttg.GenerateTextForChapterToken(token, nullptr, nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}


void
ToxTextGeneratorTest::ChapterNumberWithTitleIsGeneratedForNumberNoPrepst()
{
    SwForm form;
    ToxTextGeneratorWithMockedChapterField ttg(form);
    // set all values to make sure they are not used
    ttg.GetChapterField().m_State.sNumber = "5";
    ttg.GetChapterField().m_State.sPre = "PRE";
    ttg.GetChapterField().m_State.sPost = "POST";
    ttg.GetChapterField().m_State.sTitle = "myTitle";

    SwFormToken token(TOKEN_CHAPTER_INFO);
    token.nChapterFormat = CF_NUMBER_NOPREPST;

    OUString expected("5 myTitle");
    OUString actual = ttg.GenerateTextForChapterToken(token, nullptr, nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // we cannot mock the pre- and suffix generation in the chapterfield. We just test that sNumber and
    // sTitle are used and hope that the pre- and suffix addition works.
    token.nChapterFormat = CF_NUM_TITLE;
    expected = ttg.GenerateTextForChapterToken(token, nullptr, nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(ToxTextGeneratorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
