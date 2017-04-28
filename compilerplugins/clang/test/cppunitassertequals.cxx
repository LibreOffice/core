/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "cppunit/TestAssert.h"

#include "cppunitassertequals.hxx"

#define TEST1 CPPUNIT_ASSERT(b1 == b2)
#define TEST2(x) x

void test(bool b1, bool b2, OUString const & s1, OUString const & s2, T t) {
    CppUnit::Asserter::failIf(b1,"");
#if 0 // TODO: enable later
    CPPUNIT_ASSERT(b1 && b2); // expected-error {{rather split into two CPPUNIT_ASSERT [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT((b1 && b2)); // expected-error {{rather split into two CPPUNIT_ASSERT [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(!(b1 || b2)); // expected-error {{rather split into two CPPUNIT_ASSERT [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(!(b1 && b2));
    CPPUNIT_ASSERT(!!(b1 && b2)); // expected-error {{rather split into two CPPUNIT_ASSERT [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT_MESSAGE("", b1 && b2); // expected-error {{rather split into two CPPUNIT_ASSERT_MESSAGE [loplugin:cppunitassertequals]}}
#endif
    CPPUNIT_ASSERT(b1 == b2); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(b1 != b2);
    CPPUNIT_ASSERT((b1 == b2)); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(!(b1 != b2)); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator != call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(!(b1 == b2));
    CPPUNIT_ASSERT(!!(b1 == b2)); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT_MESSAGE("", b1 == b2); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL_MESSAGE (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(s1 == s2); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(s1 != s2);
    CPPUNIT_ASSERT((s1 == s2)); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(!(s1 != s2)); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator != call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    CPPUNIT_ASSERT(!(s1 == s2));
    CPPUNIT_ASSERT(!!(s1 == s2)); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    TEST1; // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    TEST2(CPPUNIT_ASSERT(b1 == b2)); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}
    TEST2(TEST1); // expected-error {{rather call CPPUNIT_ASSERT_EQUAL (or rewrite as an explicit operator == call when the operator itself is the topic) [loplugin:cppunitassertequals]}}

    // Useful when testing an equality iterator itself:
    CPPUNIT_ASSERT(operator ==(s1, s1));
    CPPUNIT_ASSERT(t.operator ==(t));

    // There might even be good reasons(?) not to warn inside explicit casts:
    CPPUNIT_ASSERT(bool(b1 && b2));
    CPPUNIT_ASSERT(bool(b1 == b2));
    CPPUNIT_ASSERT(bool(s1 == s2));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
