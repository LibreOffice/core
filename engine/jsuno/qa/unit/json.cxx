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

#include <limits>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <comphelper/processfactory.hxx>
#include <cppu/unotype.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

#include <json.hxx>

namespace
{
OString render(css::uno::Any const& any)
{
    OStringBuffer buf;
    appendUnoAsJson(buf, any.getValueType(), any.getValue());
    return buf.makeStringAndClear();
}

class Json : public CppUnit::TestFixture
{
public:
    void testAppendScalars()
    {
        CPPUNIT_ASSERT_EQUAL("null"_ostr, render({}));
        CPPUNIT_ASSERT_EQUAL("true"_ostr, render(css::uno::Any(true)));
        CPPUNIT_ASSERT_EQUAL("false"_ostr, render(css::uno::Any(false)));
        CPPUNIT_ASSERT_EQUAL("42"_ostr, render(css::uno::Any(static_cast<sal_Int8>(42))));
        CPPUNIT_ASSERT_EQUAL("-1"_ostr, render(css::uno::Any(static_cast<sal_Int16>(-1))));
        CPPUNIT_ASSERT_EQUAL("65535"_ostr, render(css::uno::Any(static_cast<sal_uInt16>(65535))));
        CPPUNIT_ASSERT_EQUAL("1234567"_ostr,
                             render(css::uno::Any(static_cast<sal_Int32>(1234567))));
        CPPUNIT_ASSERT_EQUAL("9999999999"_ostr,
                             render(css::uno::Any(static_cast<sal_Int64>(9999999999LL))));
        CPPUNIT_ASSERT_EQUAL("18446744073709551615"_ostr, render(css::uno::Any(SAL_MAX_UINT64)));
        CPPUNIT_ASSERT_EQUAL("3.14"_ostr, render(css::uno::Any(3.14)));
    }

    void testAppendString()
    {
        CPPUNIT_ASSERT_EQUAL("\"hello\""_ostr, render(css::uno::Any(u"hello"_ustr)));
        CPPUNIT_ASSERT_EQUAL("\"\""_ostr, render(css::uno::Any(u""_ustr)));
        CPPUNIT_ASSERT_EQUAL("\"a\\\"b\\\\c\\nd\\te\""_ostr,
                             render(css::uno::Any(u"a\"b\\c\nd\te"_ustr)));
        CPPUNIT_ASSERT_EQUAL("\"\\u0001\""_ostr, render(css::uno::Any(u"\x01"_ustr)));
    }

    void testAppendChar()
    {
        css::uno::Any any;
        sal_Unicode c = 'A';
        any.setValue(&c, cppu::UnoType<cppu::UnoCharType>::get());
        CPPUNIT_ASSERT_EQUAL("\"A\""_ostr, render(any));
    }

    void testAppendType()
    {
        CPPUNIT_ASSERT_EQUAL("\"long\""_ostr,
                             render(css::uno::Any(cppu::UnoType<sal_Int32>::get())));
    }

    void testAppendEnum()
    {
        css::uno::Any any(css::beans::PropertyState_DIRECT_VALUE);
        CPPUNIT_ASSERT_EQUAL("\"DIRECT_VALUE\""_ostr, render(any));
    }

    void testAppendEnumInvalid()
    {
        sal_Int32 badVal = 9999;
        css::uno::Any any;
        any.setValue(&badVal, cppu::UnoType<css::beans::PropertyState>::get());
        CPPUNIT_ASSERT_THROW(render(any), css::uno::RuntimeException);
    }

    void testAppendSequence()
    {
        css::uno::Sequence<sal_Int32> s{ 1, 2, 3 };
        CPPUNIT_ASSERT_EQUAL("[1,2,3]"_ostr, render(css::uno::Any(s)));
        css::uno::Sequence<sal_Int32> empty;
        CPPUNIT_ASSERT_EQUAL("[]"_ostr, render(css::uno::Any(empty)));
    }

    void testAppendStruct()
    {
        css::beans::PropertyValue pv;
        pv.Name = u"foo"_ustr;
        pv.Handle = 7;
        pv.Value <<= sal_Int32(42);
        pv.State = css::beans::PropertyState_DIRECT_VALUE;
        CPPUNIT_ASSERT_EQUAL(
            "{\"Name\":\"foo\",\"Handle\":7,\"Value\":42,\"State\":\"DIRECT_VALUE\"}"_ostr,
            render(css::uno::Any(pv)));
    }

    void testAppendInterfaceNull()
    {
        css::uno::Reference<css::uno::XInterface> ref;
        CPPUNIT_ASSERT_EQUAL("null"_ostr, render(css::uno::Any(ref)));
    }

    void testAppendInterfaceNonNullIsNull()
    {
        //TODO:
        auto const xCtx = comphelper::getProcessComponentContext();
        CPPUNIT_ASSERT(xCtx.is());
        CPPUNIT_ASSERT_EQUAL("null"_ostr, render(css::uno::Any(xCtx)));
    }

    void testAppendNonFiniteThrows()
    {
        // JSON has no representation for NaN and +/-Inf, so emitting them should throw rather than
        // producing malformed JSON:
        CPPUNIT_ASSERT_THROW(render(css::uno::Any(std::numeric_limits<double>::quiet_NaN())),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(render(css::uno::Any(std::numeric_limits<double>::infinity())),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(render(css::uno::Any(-std::numeric_limits<double>::infinity())),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(render(css::uno::Any(std::numeric_limits<float>::quiet_NaN())),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(render(css::uno::Any(std::numeric_limits<float>::infinity())),
                             css::uno::RuntimeException);
    }

    void testParseScalars()
    {
        CPPUNIT_ASSERT(!parseJsonToAny(u""_ustr, cppu::UnoType<void>::get()).hasValue());
        CPPUNIT_ASSERT_EQUAL(true,
                             parseJsonToAny(u"true"_ustr, cppu::UnoType<bool>::get()).get<bool>());
        CPPUNIT_ASSERT_EQUAL(false,
                             parseJsonToAny(u"false"_ustr, cppu::UnoType<bool>::get()).get<bool>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int32(42),
            parseJsonToAny(u"42"_ustr, cppu::UnoType<sal_Int32>::get()).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(SAL_MAX_UINT64, parseJsonToAny(u"18446744073709551615"_ustr,
                                                            cppu::UnoType<sal_uInt64>::get())
                                                 .get<sal_uInt64>());
        CPPUNIT_ASSERT_EQUAL(
            3.5, parseJsonToAny(u"3.5"_ustr, cppu::UnoType<double>::get()).get<double>());
    }

    void testParseScalarsOutOfRange()
    {
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"128"_ustr, cppu::UnoType<sal_Int8>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"-129"_ustr, cppu::UnoType<sal_Int8>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"32768"_ustr, cppu::UnoType<sal_Int16>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"-1"_ustr, cppu::UnoType<sal_uInt16>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"4294967296"_ustr, cppu::UnoType<sal_uInt32>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(
            parseJsonToAny(u"99999999999999999999"_ustr, cppu::UnoType<sal_Int64>::get()),
            css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"1.5"_ustr, cppu::UnoType<sal_Int32>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"abc"_ustr, cppu::UnoType<sal_Int32>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"abc"_ustr, cppu::UnoType<double>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"yes"_ustr, cppu::UnoType<bool>::get()),
                             css::uno::RuntimeException);
    }

    void testParseString()
    {
        CPPUNIT_ASSERT_EQUAL(
            u"hello"_ustr,
            parseJsonToAny(u"\"hello\""_ustr, cppu::UnoType<OUString>::get()).get<OUString>());
        CPPUNIT_ASSERT_EQUAL(
            u""_ustr, parseJsonToAny(u"\"\""_ustr, cppu::UnoType<OUString>::get()).get<OUString>());
        CPPUNIT_ASSERT_EQUAL(u"a\"b\\c\nd\te"_ustr, parseJsonToAny(u"\"a\\\"b\\\\c\\nd\\te\""_ustr,
                                                                   cppu::UnoType<OUString>::get())
                                                        .get<OUString>());
        CPPUNIT_ASSERT_EQUAL(
            u"\x01"_ustr,
            parseJsonToAny(u"\"\\u0001\""_ustr, cppu::UnoType<OUString>::get()).get<OUString>());
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"\"unterminated"_ustr, cppu::UnoType<OUString>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"\"bad\\q\""_ustr, cppu::UnoType<OUString>::get()),
                             css::uno::RuntimeException);
    }

    void testParseChar()
    {
        sal_Unicode c{};
        parseJsonToAny(u"\"X\""_ustr, cppu::UnoType<cppu::UnoCharType>::get()) >>= c;
        CPPUNIT_ASSERT_EQUAL(int('X'), int(c));
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"\"\""_ustr, cppu::UnoType<cppu::UnoCharType>::get()),
                             css::uno::RuntimeException);
        CPPUNIT_ASSERT_THROW(
            parseJsonToAny(u"\"AB\""_ustr, cppu::UnoType<cppu::UnoCharType>::get()),
            css::uno::RuntimeException);
    }

    void testParseType()
    {
        CPPUNIT_ASSERT_EQUAL(cppu::UnoType<sal_Int32>::get(),
                             parseJsonToAny(u"\"long\""_ustr, cppu::UnoType<css::uno::Type>::get())
                                 .get<css::uno::Type>());
        CPPUNIT_ASSERT_THROW(
            parseJsonToAny(u"\"no.such.type\""_ustr, cppu::UnoType<css::uno::Type>::get()),
            css::uno::RuntimeException);
    }

    void testParseEnum()
    {
        css::beans::PropertyState s{};
        parseJsonToAny(u"\"DIRECT_VALUE\""_ustr, cppu::UnoType<css::beans::PropertyState>::get())
            >>= s;
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DIRECT_VALUE, s);
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"\"NOT_AN_ENUM_NAME\""_ustr,
                                            cppu::UnoType<css::beans::PropertyState>::get()),
                             css::uno::RuntimeException);
    }

    void testParseSequence()
    {
        css::uno::Sequence<sal_Int32> s;
        parseJsonToAny(u"[1, 2, 3]"_ustr, cppu::UnoType<css::uno::Sequence<sal_Int32>>::get())
            >>= s;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), s.getLength());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), s[0]);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), s[1]);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), s[2]);
        // Empty:
        css::uno::Sequence<sal_Int32> e;
        parseJsonToAny(u"[]"_ustr, cppu::UnoType<css::uno::Sequence<sal_Int32>>::get()) >>= e;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), e.getLength());
        // Nested sequences:
        css::uno::Sequence<css::uno::Sequence<sal_Int32>> n;
        parseJsonToAny(u"[[1,2],[3]]"_ustr,
                       cppu::UnoType<css::uno::Sequence<css::uno::Sequence<sal_Int32>>>::get())
            >>= n;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), n.getLength());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), n[0].getLength());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), n[0][0]);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), n[0][1]);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), n[1].getLength());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), n[1][0]);
        // Strings inside arrays may contain commas/brackets; the tokenizer must respect them:
        css::uno::Sequence<OUString> strs;
        parseJsonToAny(u"[\"a,b\", \"c]d\"]"_ustr,
                       cppu::UnoType<css::uno::Sequence<OUString>>::get())
            >>= strs;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), strs.getLength());
        CPPUNIT_ASSERT_EQUAL(u"a,b"_ustr, strs[0]);
        CPPUNIT_ASSERT_EQUAL(u"c]d"_ustr, strs[1]);
    }

    void testParseStruct()
    {
        // PropertyValue's Value member is an Any, which on the parse side requires the
        // {type, val} envelope (asymmetric with the bare rendering on the append side):
        css::beans::PropertyValue pv;
        parseJsonToAny(u"{\"Name\":\"foo\",\"Handle\":7,"
                       u"\"Value\":{\"type\":\"long\",\"val\":42},"
                       u"\"State\":\"DIRECT_VALUE\"}"_ustr,
                       cppu::UnoType<css::beans::PropertyValue>::get())
            >>= pv;
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pv.Name);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pv.Handle);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(42), pv.Value.get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DIRECT_VALUE, pv.State);
        // Missing member is an error:
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"{\"Name\":\"foo\"}"_ustr,
                                            cppu::UnoType<css::beans::PropertyValue>::get()),
                             css::uno::RuntimeException);
    }

    void testParseAny()
    {
        // {type, val} envelope:
        css::uno::Any const nested = parseJsonToAny(u"{\"type\":\"long\",\"val\":42}"_ustr,
                                                    cppu::UnoType<css::uno::Any>::get());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(42), nested.get<sal_Int32>());
        // Bare value (not the envelope) should fail:
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"42"_ustr, cppu::UnoType<css::uno::Any>::get()),
                             css::uno::RuntimeException);
    }

    void testParseInterface()
    {
        // Null interface reference:
        css::uno::Reference<css::uno::XInterface> ref;
        parseJsonToAny(u"null"_ustr, cppu::UnoType<css::uno::XInterface>::get()) >>= ref;
        CPPUNIT_ASSERT(!ref.is());
        // Non-null is not implemented yet:
        CPPUNIT_ASSERT_THROW(parseJsonToAny(u"{}"_ustr, cppu::UnoType<css::uno::XInterface>::get()),
                             css::uno::RuntimeException);
    }

    CPPUNIT_TEST_SUITE(Json);
    CPPUNIT_TEST(testAppendScalars);
    CPPUNIT_TEST(testAppendString);
    CPPUNIT_TEST(testAppendChar);
    CPPUNIT_TEST(testAppendType);
    CPPUNIT_TEST(testAppendEnum);
    CPPUNIT_TEST(testAppendEnumInvalid);
    CPPUNIT_TEST(testAppendSequence);
    CPPUNIT_TEST(testAppendStruct);
    CPPUNIT_TEST(testAppendInterfaceNull);
    CPPUNIT_TEST(testAppendInterfaceNonNullIsNull);
    CPPUNIT_TEST(testAppendNonFiniteThrows);
    CPPUNIT_TEST(testParseScalars);
    CPPUNIT_TEST(testParseScalarsOutOfRange);
    CPPUNIT_TEST(testParseString);
    CPPUNIT_TEST(testParseChar);
    CPPUNIT_TEST(testParseType);
    CPPUNIT_TEST(testParseEnum);
    CPPUNIT_TEST(testParseSequence);
    CPPUNIT_TEST(testParseStruct);
    CPPUNIT_TEST(testParseAny);
    CPPUNIT_TEST(testParseInterface);
    CPPUNIT_TEST_SUITE_END();
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(Json);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
