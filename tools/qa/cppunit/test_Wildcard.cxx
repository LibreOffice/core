/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <tools/wldcrd.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
public:
    void test_Wildcard();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_Wildcard);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test_Wildcard()
{
    WildCard wildcard(u"*.html;*??a;*\\*abc;*\\?xyz", ';'); // tdf#148253
    CPPUNIT_ASSERT(wildcard.Matches(u"foo.html"));
    CPPUNIT_ASSERT(wildcard.Matches(u"foo.ht.html")); // test stepping back after partial match
    CPPUNIT_ASSERT(wildcard.Matches(u"foo.html.html")); // test stepping back after full match
    CPPUNIT_ASSERT(wildcard.Matches(u"??aa")); // test stepping back with question marks
    CPPUNIT_ASSERT(wildcard.Matches(u"111*abc")); // test escaped asterisk
    CPPUNIT_ASSERT(!wildcard.Matches(u"111-abc")); // test escaped asterisk
    CPPUNIT_ASSERT(wildcard.Matches(u"111?xyz")); // test escaped question mark
    CPPUNIT_ASSERT(!wildcard.Matches(u"111-xyz")); // test escaped question mark
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
