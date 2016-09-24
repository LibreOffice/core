/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/ustrbuf.hxx"

namespace test { namespace oustringbuffer {

class AppendChar: public CppUnit::TestFixture {
private:
    void testAppendChar();

    CPPUNIT_TEST_SUITE(AppendChar);
    CPPUNIT_TEST(testAppendChar);
    CPPUNIT_TEST_SUITE_END();
};

void AppendChar::testAppendChar() {
    // Check that append('a') does not unexpectedly pick
    // append(sal_Int32 i, sal_Int16 radix = 10):
    rtl::OUStringBuffer s;
    s.append('a');
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('a'), s[0]);
}

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustringbuffer::AppendChar);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
