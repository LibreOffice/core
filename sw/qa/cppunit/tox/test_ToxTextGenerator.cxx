/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtl/ustring.hxx"
#include "tox.hxx"
#include "txmsrt.hxx"
#include "ToxTextGenerator.hxx"

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

using sw::ToxTextGenerator;

class ToxTextGeneratorTest : public CppUnit::TestFixture {
public:
    void EmptyStringIsReturnedForPageNumberPlaceholderOfZeroItems();
    void OneAtSignIsReturnedForPageNumberPlaceholderOfOneItem();
    void TwoAtSignsAreReturnedForPageNumberPlaceholderOfOneItem();
    void EmptyStringIsReturnedAsNumStringIfNoTextMarkIsSet();

    CPPUNIT_TEST_SUITE(ToxTextGeneratorTest);
    CPPUNIT_TEST(EmptyStringIsReturnedForPageNumberPlaceholderOfZeroItems);
    CPPUNIT_TEST(OneAtSignIsReturnedForPageNumberPlaceholderOfOneItem);
    CPPUNIT_TEST(TwoAtSignsAreReturnedForPageNumberPlaceholderOfOneItem);
    CPPUNIT_TEST(EmptyStringIsReturnedAsNumStringIfNoTextMarkIsSet);
    CPPUNIT_TEST_SUITE_END();

};

struct MockedSortTab : public SwTOXSortTabBase {
    MockedSortTab()
    : SwTOXSortTabBase(TOX_SORT_INDEX,0,0,0) {;}

    virtual TextAndReading GetText_Impl() const SAL_OVERRIDE {
        return TextAndReading();
    }
    virtual sal_uInt16  GetLevel() const SAL_OVERRIDE {
        return 0;
    }
};

void
ToxTextGeneratorTest::EmptyStringIsReturnedForPageNumberPlaceholderOfZeroItems()
{
    OUString expected("");
    OUString actual = ToxTextGenerator::ConstructPageNumberPlaceholder(0);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void
ToxTextGeneratorTest::OneAtSignIsReturnedForPageNumberPlaceholderOfOneItem()
{
    OUString expected("@~");
    OUString actual = ToxTextGenerator::ConstructPageNumberPlaceholder(1);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void
ToxTextGeneratorTest::TwoAtSignsAreReturnedForPageNumberPlaceholderOfOneItem()
{
    OUString expected("@, @~");
    OUString actual = ToxTextGenerator::ConstructPageNumberPlaceholder(2);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void
ToxTextGeneratorTest::EmptyStringIsReturnedAsNumStringIfNoTextMarkIsSet()
{
    MockedSortTab sortTab;
    sortTab.pTxtMark = NULL;

    OUString expected("");
    OUString actual = ToxTextGenerator::GetNumStringOfFirstNode(sortTab, false, 0);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(ToxTextGeneratorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
