/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <test/a11y/accessibletestbase.hxx>

using namespace css;
using namespace accessibility;

// Checks fetching multi-unit characters
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestUnicodeSP)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/unicode.fodf"_ustr);

    auto xContext = getDocumentAccessibleContext()->getAccessibleChild(0)->getAccessibleContext();

    uno::Reference<XAccessibleText> para(xContext, uno::UNO_QUERY_THROW);
    auto segment = para->getTextAtIndex(0, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u"\U0001f0a1"_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), segment.SegmentEnd);

    segment = para->getTextBeforeIndex(2, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u"\U0001f0a1"_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), segment.SegmentEnd);

    segment = para->getTextBehindIndex(0, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u"\U0001f0ae"_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), segment.SegmentEnd);
}

// Checks getTextBehindIndex() with multi-unit characters
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestUnicodeSPBehindIndex)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/unicode.fodf"_ustr);

    auto xContext = getDocumentAccessibleContext()->getAccessibleChild(0)->getAccessibleContext();

    uno::Reference<XAccessibleText> para(xContext, uno::UNO_QUERY_THROW);
    auto nChCount = para->getCharacterCount();

    // verify bounds are properly handled
    CPPUNIT_ASSERT_THROW(para->getTextBehindIndex(-1, AccessibleTextType::CHARACTER),
                         lang::IndexOutOfBoundsException);
    CPPUNIT_ASSERT_THROW(para->getTextBehindIndex(nChCount + 1, AccessibleTextType::CHARACTER),
                         lang::IndexOutOfBoundsException);

    auto segment = para->getTextBehindIndex(nChCount, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), segment.SegmentEnd);

    segment = para->getTextBehindIndex(nChCount - 2, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), segment.SegmentEnd);

    segment = para->getTextBehindIndex(nChCount - 4, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u"\U0001f0ab"_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), segment.SegmentEnd);

    // verify bounds behave the same with single unit characters, just as a validation
    xContext = getNextFlowingSibling(xContext);
    CPPUNIT_ASSERT(xContext.is());
    para.set(xContext, uno::UNO_QUERY_THROW);

    nChCount = para->getCharacterCount();

    CPPUNIT_ASSERT_THROW(para->getTextBehindIndex(-1, AccessibleTextType::CHARACTER),
                         lang::IndexOutOfBoundsException);
    CPPUNIT_ASSERT_THROW(para->getTextBehindIndex(nChCount + 1, AccessibleTextType::CHARACTER),
                         lang::IndexOutOfBoundsException);

    segment = para->getTextBehindIndex(nChCount, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), segment.SegmentEnd);

    segment = para->getTextBehindIndex(nChCount - 1, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), segment.SegmentEnd);

    segment = para->getTextBehindIndex(nChCount - 2, AccessibleTextType::CHARACTER);
    CPPUNIT_ASSERT_EQUAL(u"j"_ustr, segment.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), segment.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), segment.SegmentEnd);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
