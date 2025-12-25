/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <i18nutil/guessparadirection.hxx>

using namespace i18nutil;

namespace
{
class GuessParagraphDirectionTest : public CppUnit::TestFixture
{
public:
    void testEmpty();
    void testFirstStrongLTR();
    void testFirstStrongRTL();
    void testFirstStrongAR();
    void testIsolate();

    CPPUNIT_TEST_SUITE(GuessParagraphDirectionTest);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testFirstStrongLTR);
    CPPUNIT_TEST(testFirstStrongRTL);
    CPPUNIT_TEST(testFirstStrongAR);
    CPPUNIT_TEST(testIsolate);
    CPPUNIT_TEST_SUITE_END();
};

void GuessParagraphDirectionTest::testEmpty()
{
    auto aText = u""_ustr;
    CPPUNIT_ASSERT_EQUAL(ParagraphDirection::Ambiguous, GuessParagraphDirection(aText));
}

void GuessParagraphDirectionTest::testFirstStrongLTR()
{
    auto aText = u"..aاא.."_ustr;
    CPPUNIT_ASSERT_EQUAL(ParagraphDirection::LeftToRight, GuessParagraphDirection(aText));
}

void GuessParagraphDirectionTest::testFirstStrongRTL()
{
    auto aText = u"..אaا.."_ustr;
    CPPUNIT_ASSERT_EQUAL(ParagraphDirection::RightToLeft, GuessParagraphDirection(aText));
}

void GuessParagraphDirectionTest::testFirstStrongAR()
{
    auto aText = u"..اaא.."_ustr;
    CPPUNIT_ASSERT_EQUAL(ParagraphDirection::RightToLeft, GuessParagraphDirection(aText));
}

void GuessParagraphDirectionTest::testIsolate()
{
    // U+2066: LRI
    // U+2067: RLI
    // U+2069: PDI
    auto aText = u"...\u2069.\u2067aaa\u2066אאא\u2069.\u2069.."_ustr;
    CPPUNIT_ASSERT_EQUAL(ParagraphDirection::Ambiguous, GuessParagraphDirection(aText));
}

CPPUNIT_TEST_SUITE_REGISTRATION(GuessParagraphDirectionTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
