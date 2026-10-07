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

#include <string>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <jsuno/jsuno.hxx>
#include <rtl/string.hxx>

#include "testexec.hxx"

namespace
{
class Execute : public CppUnit::TestFixture
{
public:
    void testReturnValue()
    {
        CPPUNIT_ASSERT_EQUAL(std::string("42"), testexec("42"_ostr).json);
        CPPUNIT_ASSERT_EQUAL(std::string("true"), testexec("true"_ostr).json);
        CPPUNIT_ASSERT_EQUAL(
            std::string("\"hello\""), testexec("'hello'"_ostr).json);
        CPPUNIT_ASSERT_EQUAL(std::string("null"), testexec("null"_ostr).json);
        CPPUNIT_ASSERT_EQUAL(
            std::string("[1,2,3]"), testexec("[1, 2, 3]"_ostr).json);
        CPPUNIT_ASSERT_EQUAL(std::string("{\"a\":1,\"b\":\"two\"}"),
                             testexec("({a: 1, b: 'two'})"_ostr).json);
        CPPUNIT_ASSERT_EQUAL(std::string(""), testexec("undefined"_ostr).json);
        CPPUNIT_ASSERT_EQUAL(
            std::string(""), testexec("(function () {})"_ostr).json);
        CPPUNIT_ASSERT_EQUAL(std::string("42"),
                             testexec(
                                 "(function () { return 42; }).apply(null, [])"_ostr).json);
        try
        {
            testexec("1n"_ostr);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string("TypeError"), e.name);
        }
    }

    void testThrows()
    {
        try
        {
            testexec("throw new Error('boom')"_ostr);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string("boom"), e.message);
            CPPUNIT_ASSERT_EQUAL(std::size_t(1), e.stack.size());
            CPPUNIT_ASSERT_EQUAL(std::string("<input>"), e.stack[0].source);
            CPPUNIT_ASSERT_EQUAL(std::string("1"), e.stack[0].line);
            CPPUNIT_ASSERT_EQUAL(std::string("11"), e.stack[0].column);
            CPPUNIT_ASSERT_EQUAL(std::string("<eval>"), e.stack[0].functionName);
            CPPUNIT_ASSERT_EQUAL(std::string("Error"), e.name);
        }
        try
        {
            testexec("throw new TypeError('bad type')"_ostr);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string("bad type"), e.message);
            CPPUNIT_ASSERT_EQUAL(std::string("TypeError"), e.name);
        }
        try
        {
            testexec("@@@ not valid JS"_ostr);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string("SyntaxError"), e.name);
        }
        try
        {
            testexec("throw 'plain string'"_ostr);
            CPPUNIT_FAIL("expected jsuno::Exception");
        }
        catch (jsuno::Exception const& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string("plain string"), e.message);
            CPPUNIT_ASSERT_EQUAL(std::string(""), e.name);
        }
        // A lone surrogate in the message comes out as U+FFFD, so the message is well-formed UTF-8:
        try {
            testexec("throw new Error('a\\uD83Db')"_ostr);
            CPPUNIT_FAIL("expected jsuno::Exception");
        } catch (jsuno::Exception const & e) {
            CPPUNIT_ASSERT_EQUAL(std::string("a\xEF\xBF\xBD" "b"), e.message);
        }
        CPPUNIT_ASSERT_EQUAL(
            std::string("[\"boom\",\"cpo.uno.RuntimeException: boom\"]"),
            testexec(
                "var e = new uno.idl.cpo.uno.RuntimeException({message: 'boom'});"
                    "[e.message, String(e)]"_ostr).json);
    }

    void testLegacyUnoApiUse() {
        CPPUNIT_ASSERT(testexec("uno.idl.com.sun.star.uno.XInterface"_ostr).usedLegacyUnoApi);
        CPPUNIT_ASSERT(testexec("uno.idl.cpo.uno.XInterface"_ostr).usedLegacyUnoApi);
        CPPUNIT_ASSERT(!testexec("uno.idl.scriptinterop.XFactory"_ostr).usedLegacyUnoApi);
        CPPUNIT_ASSERT(
            !testexec(
                "$internal.suppressLegacyUnoApiStart();"
                " uno.idl.com.sun.star.uno.XInterface;"
                " $internal.suppressLegacyUnoApiEnd();"_ostr).usedLegacyUnoApi);
        // A UNO exception thrown into the script is not the script's own use of the legacy API,
        // even though its JS representation is a cpo.uno.RuntimeException:
        CPPUNIT_ASSERT(
            !testexec(
                "try { cool.newAffineTransformBuilder(); } catch (e) {}"_ostr).usedLegacyUnoApi);
    }

    CPPUNIT_TEST_SUITE(Execute);
    CPPUNIT_TEST(testReturnValue);
    CPPUNIT_TEST(testThrows);
    CPPUNIT_TEST(testLegacyUnoApiUse);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Execute);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
