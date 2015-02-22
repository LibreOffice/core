/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdexcept>

#include <sal/types.h>

#include <rtl/ustring.hxx>

#include <ToxWhitespaceStripper.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace sw;

class ToxWhitespaceStripperTest : public CppUnit::TestFixture
{
    void
    MappingCharactersToVariousStrippedStringsWorks();

    void
    StrippingWhitespacesFromVariousStringsWorks();

    void
    PositionAfterStringCanBeRequested();

    void
    InvalidPositionIsMappedToLastEntry();

    CPPUNIT_TEST_SUITE(ToxWhitespaceStripperTest);
    CPPUNIT_TEST(MappingCharactersToVariousStrippedStringsWorks);
    CPPUNIT_TEST(StrippingWhitespacesFromVariousStringsWorks);
    CPPUNIT_TEST(PositionAfterStringCanBeRequested);
    CPPUNIT_TEST(InvalidPositionIsMappedToLastEntry);

    CPPUNIT_TEST_SUITE_END();

};

void
ToxWhitespaceStripperTest::MappingCharactersToVariousStrippedStringsWorks()
{
    {
        OUString test("abc\n");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), sut.GetPositionInStrippedString(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), sut.GetPositionInStrippedString(1));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), sut.GetPositionInStrippedString(2));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), sut.GetPositionInStrippedString(3));
    }
    {
        OUString test("abc\n\n");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), sut.GetPositionInStrippedString(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), sut.GetPositionInStrippedString(1));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), sut.GetPositionInStrippedString(2));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), sut.GetPositionInStrippedString(3));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), sut.GetPositionInStrippedString(4));
    }
    {
        OUString test("abc\ndef");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), sut.GetPositionInStrippedString(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), sut.GetPositionInStrippedString(1));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), sut.GetPositionInStrippedString(2));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), sut.GetPositionInStrippedString(3));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), sut.GetPositionInStrippedString(4));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), sut.GetPositionInStrippedString(5));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), sut.GetPositionInStrippedString(6));
    }
    {
        //             012345 6789
        OUString test("  abc \ndef");
        //             01234567
        //            " abc def"
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), sut.GetPositionInStrippedString(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), sut.GetPositionInStrippedString(1));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), sut.GetPositionInStrippedString(2));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), sut.GetPositionInStrippedString(3));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), sut.GetPositionInStrippedString(4));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), sut.GetPositionInStrippedString(5));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), sut.GetPositionInStrippedString(6));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), sut.GetPositionInStrippedString(7));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), sut.GetPositionInStrippedString(8));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7), sut.GetPositionInStrippedString(9));
    }
}

void
ToxWhitespaceStripperTest::StrippingWhitespacesFromVariousStringsWorks()
{
    {
        OUString test("abc\n");
        OUString expected("abc");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(expected, sut.GetStrippedString());
    }
    {
        OUString test("abc\n\n");
        OUString expected("abc");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(expected, sut.GetStrippedString());
    }
    {
        OUString test("abc\ndef");
        OUString expected("abc def");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(expected, sut.GetStrippedString());
    }
    {
        OUString test("  abc \ndef");
        OUString expected(" abc def");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(expected, sut.GetStrippedString());
    }
    {
        OUString test("  ");
        OUString expected("");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(expected, sut.GetStrippedString());
    }
    {
        OUString test("d  ");
        OUString expected("d");
        ToxWhitespaceStripper sut(test);
        CPPUNIT_ASSERT_EQUAL(expected, sut.GetStrippedString());
    }
}

void
ToxWhitespaceStripperTest::PositionAfterStringCanBeRequested()
{
    OUString test("abc");
    ToxWhitespaceStripper sut(test);
    sal_Int32 expected = test.getLength();
    CPPUNIT_ASSERT_EQUAL(expected, sut.GetPositionInStrippedString(test.getLength()));
}

void
ToxWhitespaceStripperTest::InvalidPositionIsMappedToLastEntry()
{
    OUString test("ab  c");
    ToxWhitespaceStripper sut(test);
    sal_Int32 expected = 4; // the length of the string after merging the two whitespaces
    sal_Int32 result = sut.GetPositionInStrippedString(40); // a value past the original string length
    CPPUNIT_ASSERT_EQUAL(expected, result);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(ToxWhitespaceStripperTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
