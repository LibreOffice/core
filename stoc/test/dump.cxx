/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/reflection/Dump.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/codemaker/cppumaker/ByteBits.hpp>
#include <test/codemaker/cppumaker/Constants.hpp>
#include <test/codemaker/cppumaker/ShortBits.hpp>
#include <test/codemaker/cppumaker/UnsignedHyperBits.hpp>

namespace
{
class Dump : public CppUnit::TestFixture
{
public:
    void setUp() override
    {
        dump_ = css::reflection::Dump::get(cppu::defaultBootstrap_InitialComponentContext());
    }

    void testSequence()
    {
        CPPUNIT_ASSERT_EQUAL(u"[]"_ustr,
                             dump_->dumpValue(css::uno::Any(css::uno::Sequence<sal_Int32>{})));
        CPPUNIT_ASSERT_EQUAL(u"[1]"_ustr,
                             dump_->dumpValue(css::uno::Any(css::uno::Sequence<sal_Int32>{ 1 })));
        CPPUNIT_ASSERT_EQUAL(u"[1, 2, 3]"_ustr, dump_->dumpValue(css::uno::Any(
                                                    css::uno::Sequence<sal_Int32>{ 1, 2, 3 })));
        CPPUNIT_ASSERT_EQUAL(u"[[long: 1], [string: \"2\"], [[]long: [1, 2]]]"_ustr,
                             (dump_->dumpValue(css::uno::Any(css::uno::Sequence<css::uno::Any>{
                                 css::uno::Any(sal_Int32(1)), css::uno::Any(u"2"_ustr),
                                 css::uno::Any(css::uno::Sequence<sal_Int32>{ 1, 2 }) }))));
    }

    void testEnum()
    {
        CPPUNIT_ASSERT_EQUAL(u"ENUM"_ustr,
                             dump_->dumpValue(css::uno::Any(css::uno::TypeClass_ENUM)));
        CPPUNIT_ASSERT_EQUAL(u"-1"_ustr, dump_->dumpValue(css::uno::Any(css::uno::TypeClass(-1))));
        CPPUNIT_ASSERT_EQUAL(u"12345"_ustr,
                             dump_->dumpValue(css::uno::Any(css::uno::TypeClass(12345))));
    }

    void testStruct()
    {
        CPPUNIT_ASSERT_EQUAL(
            u"[Source: null, PropertyName: \"test\", Further: false, PropertyHandle: 3, "
            "OldValue: [void: void], NewValue: [long: 5]]"_ustr,
            dump_->dumpValue(css::uno::Any(css::beans::PropertyChangeEvent(
                {}, u"test"_ustr, false, 3, {}, css::uno::Any(sal_Int32(5))))));
    }

    void testConstantsGroup()
    {
        CPPUNIT_ASSERT_EQUAL(u"byteMin"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.Constants"_ustr,
                                                 css::uno::Any(sal_Int8(-128))));
        CPPUNIT_ASSERT_EQUAL(u"byteMax"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.Constants"_ustr,
                                                 css::uno::Any(sal_Int8(127))));
        CPPUNIT_ASSERT_EQUAL(u"longMin"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.Constants"_ustr,
                                                 css::uno::Any(sal_Int32(-2147483648))));
        CPPUNIT_ASSERT_EQUAL(u"longMax"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.Constants"_ustr,
                                                 css::uno::Any(sal_Int32(2147483647))));
        CPPUNIT_ASSERT_EQUAL(u"hyperMin"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.Constants"_ustr,
                                                 css::uno::Any(SAL_MIN_INT64)));
        CPPUNIT_ASSERT_EQUAL(u"hyperMax"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.Constants"_ustr,
                                                 css::uno::Any(SAL_MAX_INT64)));
        CPPUNIT_ASSERT_EQUAL(u"17"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.Constants"_ustr,
                                                 css::uno::Any(sal_Int32(17))));
        CPPUNIT_ASSERT_EQUAL(u"2147483646"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.Constants"_ustr,
                                                 css::uno::Any(sal_Int32(2147483646))));

        CPPUNIT_ASSERT_EQUAL(u"0"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.ByteBits"_ustr,
                                                 css::uno::Any(sal_Int8(0))));
        CPPUNIT_ASSERT_EQUAL(u"BIT0+BIT2"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.ByteBits"_ustr,
                                                 css::uno::Any(sal_Int8(5))));
        CPPUNIT_ASSERT_EQUAL(u"BIT4"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.ByteBits"_ustr,
                                                 css::uno::Any(sal_Int8(16))));
        CPPUNIT_ASSERT_EQUAL(u"BIT0+BIT4"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.ByteBits"_ustr,
                                                 css::uno::Any(sal_Int8(17))));
        CPPUNIT_ASSERT_EQUAL(u"BIT7"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.ByteBits"_ustr,
                                                 css::uno::Any(sal_Int8(-128))));
        CPPUNIT_ASSERT_EQUAL(u"ALL"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.ByteBits"_ustr,
                                                 css::uno::Any(sal_Int8(-1))));

        CPPUNIT_ASSERT_EQUAL(u"BIT7"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.ShortBits"_ustr,
                                                 css::uno::Any(sal_Int16(128))));
        CPPUNIT_ASSERT_EQUAL(u"ALL"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.ShortBits"_ustr,
                                                 css::uno::Any(sal_Int16(-1))));

        CPPUNIT_ASSERT_EQUAL(u"BIT63"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.UnsignedHyperBits"_ustr,
                                                 css::uno::Any(sal_uInt64(9223372036854775808u))));
        CPPUNIT_ASSERT_EQUAL(u"BIT0+BIT62"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.UnsignedHyperBits"_ustr,
                                                 css::uno::Any(sal_uInt64(4611686018427387905))));
        CPPUNIT_ASSERT_EQUAL(u"BIT0+BIT63"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.UnsignedHyperBits"_ustr,
                                                 css::uno::Any(sal_uInt64(9223372036854775809u))));
        CPPUNIT_ASSERT_EQUAL(u"ALL"_ustr,
                             dump_->dumpConstant(u"test.codemaker.cppumaker.UnsignedHyperBits"_ustr,
                                                 css::uno::Any(SAL_MAX_UINT64)));
    }

    CPPUNIT_TEST_SUITE(Dump);
    CPPUNIT_TEST(testSequence);
    CPPUNIT_TEST(testEnum);
    CPPUNIT_TEST(testStruct);
    CPPUNIT_TEST(testConstantsGroup);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference<css::reflection::XDump> dump_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Dump);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
