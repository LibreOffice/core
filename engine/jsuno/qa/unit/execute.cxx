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

#include <com/sun/star/script/provider/ScriptExceptionRaisedException.hpp>
#include <jsuno/jsuno.hxx>
#include <rtl/ustring.hxx>

namespace
{
class Execute : public CppUnit::TestFixture
{
public:
    void testReturnValue()
    {
        CPPUNIT_ASSERT_EQUAL(u"42"_ustr, jsuno::execute(u"42"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"true"_ustr, jsuno::execute(u"true"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"\"hello\""_ustr, jsuno::execute(u"'hello'"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"null"_ustr, jsuno::execute(u"null"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"[1,2,3]"_ustr, jsuno::execute(u"[1, 2, 3]"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"{\"a\":1,\"b\":\"two\"}"_ustr,
                             jsuno::execute(u"({a: 1, b: 'two'})"_ustr));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, jsuno::execute(u"undefined"_ustr));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, jsuno::execute(u"(function () {})"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"42"_ustr,
                             jsuno::execute(u"(function () { return 42; }).apply(null, [])"_ustr));
        try
        {
            jsuno::execute(u"1n"_ustr);
            CPPUNIT_FAIL("expected ScriptExceptionRaisedException");
        }
        catch (css::script::provider::ScriptExceptionRaisedException const& e)
        {
            CPPUNIT_ASSERT_EQUAL(u"TypeError"_ustr, e.exceptionType);
        }
    }

    void testThrows()
    {
        try
        {
            jsuno::execute(u"throw new Error('boom')"_ustr);
            CPPUNIT_FAIL("expected ScriptExceptionRaisedException");
        }
        catch (css::script::provider::ScriptExceptionRaisedException const& e)
        {
            CPPUNIT_ASSERT(e.Message.startsWith("boom"));
            CPPUNIT_ASSERT_EQUAL(u"<input>"_ustr, e.scriptName);
            CPPUNIT_ASSERT_EQUAL(u"JavaScript"_ustr, e.language);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), e.lineNum);
            CPPUNIT_ASSERT_EQUAL(u"Error"_ustr, e.exceptionType);
        }
        try
        {
            jsuno::execute(u"throw new TypeError('bad type')"_ustr);
            CPPUNIT_FAIL("expected ScriptExceptionRaisedException");
        }
        catch (css::script::provider::ScriptExceptionRaisedException const& e)
        {
            CPPUNIT_ASSERT(e.Message.startsWith("bad type"));
            CPPUNIT_ASSERT_EQUAL(u"TypeError"_ustr, e.exceptionType);
        }
        try
        {
            jsuno::execute(u"@@@ not valid JS"_ustr);
            CPPUNIT_FAIL("expected ScriptExceptionRaisedException");
        }
        catch (css::script::provider::ScriptExceptionRaisedException const& e)
        {
            CPPUNIT_ASSERT_EQUAL(u"SyntaxError"_ustr, e.exceptionType);
        }
        try
        {
            jsuno::execute(u"throw 'plain string'"_ustr);
            CPPUNIT_FAIL("expected ScriptExceptionRaisedException");
        }
        catch (css::script::provider::ScriptExceptionRaisedException const& e)
        {
            CPPUNIT_ASSERT(e.Message.startsWith("plain string"));
            CPPUNIT_ASSERT_EQUAL(u""_ustr, e.exceptionType);
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
