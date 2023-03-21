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
        CPPUNIT_ASSERT_EQUAL(OUString("[]"),
                             dump_->dumpValue(css::uno::Any(css::uno::Sequence<sal_Int32>{})));
        CPPUNIT_ASSERT_EQUAL(OUString("[1]"),
                             dump_->dumpValue(css::uno::Any(css::uno::Sequence<sal_Int32>{ 1 })));
        CPPUNIT_ASSERT_EQUAL(OUString("[1, 2, 3]"), dump_->dumpValue(css::uno::Any(
                                                        css::uno::Sequence<sal_Int32>{ 1, 2, 3 })));
        CPPUNIT_ASSERT_EQUAL(OUString("[[long: 1], [string: \"2\"], [[]long: [1, 2]]]"),
                             (dump_->dumpValue(css::uno::Any(css::uno::Sequence<css::uno::Any>{
                                 css::uno::Any(sal_Int32(1)), css::uno::Any(OUString("2")),
                                 css::uno::Any(css::uno::Sequence<sal_Int32>{ 1, 2 }) }))));
    }

    void testEnum()
    {
        CPPUNIT_ASSERT_EQUAL(OUString("ENUM"),
                             dump_->dumpValue(css::uno::Any(css::uno::TypeClass_ENUM)));
        CPPUNIT_ASSERT_EQUAL(OUString("-1"),
                             dump_->dumpValue(css::uno::Any(css::uno::TypeClass(-1))));
        CPPUNIT_ASSERT_EQUAL(OUString("12345"),
                             dump_->dumpValue(css::uno::Any(css::uno::TypeClass(12345))));
    }

    void testStruct()
    {
        CPPUNIT_ASSERT_EQUAL(
            OUString("[Source: null, PropertyName: \"test\", Further: false, PropertyHandle: 3, "
                     "OldValue: [void: void], NewValue: [long: 5]]"),
            dump_->dumpValue(css::uno::Any(css::beans::PropertyChangeEvent(
                {}, "test", false, 3, {}, css::uno::Any(sal_Int32(5))))));
    }

    void testConstantsGroup()
    {
        CPPUNIT_ASSERT_EQUAL(OUString("byteMin"),
                             dump_->dumpConstant("test.codemaker.cppumaker.Constants",
                                                 css::uno::Any(sal_Int8(-128))));
        CPPUNIT_ASSERT_EQUAL(OUString("byteMax"),
                             dump_->dumpConstant("test.codemaker.cppumaker.Constants",
                                                 css::uno::Any(sal_Int8(127))));
        CPPUNIT_ASSERT_EQUAL(OUString("longMin"),
                             dump_->dumpConstant("test.codemaker.cppumaker.Constants",
                                                 css::uno::Any(sal_Int32(-2147483648))));
        CPPUNIT_ASSERT_EQUAL(OUString("longMax"),
                             dump_->dumpConstant("test.codemaker.cppumaker.Constants",
                                                 css::uno::Any(sal_Int32(2147483647))));
        CPPUNIT_ASSERT_EQUAL(OUString("hyperMin"),
                             dump_->dumpConstant("test.codemaker.cppumaker.Constants",
                                                 css::uno::Any(SAL_MIN_INT64)));
        CPPUNIT_ASSERT_EQUAL(OUString("hyperMax"),
                             dump_->dumpConstant("test.codemaker.cppumaker.Constants",
                                                 css::uno::Any(SAL_MAX_INT64)));
        CPPUNIT_ASSERT_EQUAL(OUString("17"),
                             dump_->dumpConstant("test.codemaker.cppumaker.Constants",
                                                 css::uno::Any(sal_Int32(17))));
        CPPUNIT_ASSERT_EQUAL(OUString("2147483646"),
                             dump_->dumpConstant("test.codemaker.cppumaker.Constants",
                                                 css::uno::Any(sal_Int32(2147483646))));

        CPPUNIT_ASSERT_EQUAL(OUString("0"), dump_->dumpConstant("test.codemaker.cppumaker.ByteBits",
                                                                css::uno::Any(sal_Int8(0))));
        CPPUNIT_ASSERT_EQUAL(
            OUString("BIT0+BIT2"),
            dump_->dumpConstant("test.codemaker.cppumaker.ByteBits", css::uno::Any(sal_Int8(5))));
        CPPUNIT_ASSERT_EQUAL(
            OUString("BIT4"),
            dump_->dumpConstant("test.codemaker.cppumaker.ByteBits", css::uno::Any(sal_Int8(16))));
        CPPUNIT_ASSERT_EQUAL(
            OUString("BIT0+BIT4"),
            dump_->dumpConstant("test.codemaker.cppumaker.ByteBits", css::uno::Any(sal_Int8(17))));
        CPPUNIT_ASSERT_EQUAL(OUString("BIT7"),
                             dump_->dumpConstant("test.codemaker.cppumaker.ByteBits",
                                                 css::uno::Any(sal_Int8(-128))));
        CPPUNIT_ASSERT_EQUAL(
            OUString("ALL"),
            dump_->dumpConstant("test.codemaker.cppumaker.ByteBits", css::uno::Any(sal_Int8(-1))));

        CPPUNIT_ASSERT_EQUAL(OUString("BIT7"),
                             dump_->dumpConstant("test.codemaker.cppumaker.ShortBits",
                                                 css::uno::Any(sal_Int16(128))));
        CPPUNIT_ASSERT_EQUAL(OUString("ALL"),
                             dump_->dumpConstant("test.codemaker.cppumaker.ShortBits",
                                                 css::uno::Any(sal_Int16(-1))));

        CPPUNIT_ASSERT_EQUAL(OUString("BIT63"),
                             dump_->dumpConstant("test.codemaker.cppumaker.UnsignedHyperBits",
                                                 css::uno::Any(sal_uInt64(9223372036854775808u))));
        CPPUNIT_ASSERT_EQUAL(OUString("BIT0+BIT62"),
                             dump_->dumpConstant("test.codemaker.cppumaker.UnsignedHyperBits",
                                                 css::uno::Any(sal_uInt64(4611686018427387905))));
        CPPUNIT_ASSERT_EQUAL(OUString("BIT0+BIT63"),
                             dump_->dumpConstant("test.codemaker.cppumaker.UnsignedHyperBits",
                                                 css::uno::Any(sal_uInt64(9223372036854775809u))));
        CPPUNIT_ASSERT_EQUAL(OUString("ALL"),
                             dump_->dumpConstant("test.codemaker.cppumaker.UnsignedHyperBits",
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
