/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <jsuno/jsuno.hxx>
#include <rtl/ustring.hxx>

namespace
{
class Execute : public CppUnit::TestFixture
{
public:
    void testReturnValue()
    {
        CPPUNIT_ASSERT_EQUAL(u"42"_ustr, jsuno::execute(u"42"_ustr, u"<input>"_ustr, 1));
        CPPUNIT_ASSERT_EQUAL(u"true"_ustr, jsuno::execute(u"true"_ustr, u"<input>"_ustr, 1));
        CPPUNIT_ASSERT_EQUAL(
            u"\"hello\""_ustr, jsuno::execute(u"'hello'"_ustr, u"<input>"_ustr, 1));
        CPPUNIT_ASSERT_EQUAL(u"null"_ustr, jsuno::execute(u"null"_ustr, u"<input>"_ustr, 1));
        CPPUNIT_ASSERT_EQUAL(
            u"[1,2,3]"_ustr, jsuno::execute(u"[1, 2, 3]"_ustr, u"<input>"_ustr, 1));
        CPPUNIT_ASSERT_EQUAL(u"{\"a\":1,\"b\":\"two\"}"_ustr,
                             jsuno::execute(u"({a: 1, b: 'two'})"_ustr, u"<input>"_ustr, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, jsuno::execute(u"undefined"_ustr, u"<input>"_ustr, 1));
        CPPUNIT_ASSERT_EQUAL(
            u""_ustr, jsuno::execute(u"(function () {})"_ustr, u"<input>"_ustr, 1));
        CPPUNIT_ASSERT_EQUAL(u"42"_ustr,
                             jsuno::execute(
                                 u"(function () { return 42; }).apply(null, [])"_ustr, u"<input>"_ustr,
                                 1));
        try
        {
            jsuno::execute(u"1n"_ustr, u"<input>"_ustr, 1);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(u"TypeError"_ustr, e.name);
        }
    }

    void testThrows()
    {
        try
        {
            jsuno::execute(u"throw new Error('boom')"_ustr, u"<input>"_ustr, 1);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(u"boom"_ustr, e.message);
            CPPUNIT_ASSERT_EQUAL(std::size_t(1), e.stack.size());
            CPPUNIT_ASSERT_EQUAL(u"<input>"_ustr, e.stack[0].source);
            CPPUNIT_ASSERT_EQUAL(u"1"_ustr, e.stack[0].line);
            CPPUNIT_ASSERT_EQUAL(u"10"_ustr, e.stack[0].column);
            CPPUNIT_ASSERT_EQUAL(u"<eval>"_ustr, e.stack[0].functionName);
            CPPUNIT_ASSERT_EQUAL(u"Error"_ustr, e.name);
        }
        try
        {
            jsuno::execute(u"throw new TypeError('bad type')"_ustr, u"<input>"_ustr, 1);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(u"bad type"_ustr, e.message);
            CPPUNIT_ASSERT_EQUAL(u"TypeError"_ustr, e.name);
        }
        try
        {
            jsuno::execute(u"@@@ not valid JS"_ustr, u"<input>"_ustr, 1);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(u"SyntaxError"_ustr, e.name);
        }
        try
        {
            jsuno::execute(u"throw 'plain string'"_ustr, u"<input>"_ustr, 1);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(u"plain string"_ustr, e.message);
            CPPUNIT_ASSERT_EQUAL(u""_ustr, e.name);
        }
    }

    CPPUNIT_TEST_SUITE(Execute);
    CPPUNIT_TEST(testReturnValue);
    CPPUNIT_TEST(testThrows);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Execute);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
