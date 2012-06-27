/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
