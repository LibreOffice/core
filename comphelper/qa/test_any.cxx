/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <comphelper/types.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sal/types.h>

// Demonstrate that comphelper::compare works exactly the same as
// css::uno::Any::operator ==:

namespace {

class Test: public CppUnit::TestFixture {
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test() {
    css::uno::Any a1, a2;

    a1 = css::uno::makeAny<sal_uInt32>(5);
    a2 = css::uno::makeAny<sal_Int16>(5);
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<sal_uInt32>(5);
    a2 = css::uno::makeAny<sal_Int16>(6);
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<css::awt::FontDescriptor>({
        "A", 0, 1, "B", 3, 4, 5, 6.0, 7.0, css::awt::FontSlant_NONE, 8, 9, 10.0,
        false, true, 11});
    a2 = css::uno::makeAny<css::awt::FontDescriptor>({
        "A", 0, 1, "B", 3, 4, 5, 6.0, 7.0, css::awt::FontSlant_NONE, 8, 9, 10.0,
        false, true, 11});
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<css::awt::FontDescriptor>({
        "A", 0, 1, "B", 3, 4, 5, 6.0, 7.0, css::awt::FontSlant_NONE, 8, 9, 10.0,
        false, true, 11});
    a2 = css::uno::makeAny<css::awt::FontDescriptor>({
        "a", 0, 1, "B", 3, 4, 5, 6.0, 7.0, css::awt::FontSlant_NONE, 8, 9, 10.0,
        false, true, 11});
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<css::util::Date>({1, 2, 2003});
    a2 = css::uno::makeAny<css::util::Date>({1, 2, 2003});
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<css::util::Date>({1, 2, 2003});
    a2 = css::uno::makeAny<css::util::Date>({1, 3, 2003});
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<css::util::Date>({1, 2, 2003});
    a2 = css::uno::makeAny<css::util::DateTime>({
        0, 0, 0, 0, 1, 2, 2003, false});
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<css::uno::Sequence<sal_Int8>>({0, 1, 2});
    a2 = css::uno::makeAny<css::uno::Sequence<sal_Int8>>({0, 1, 2});
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<css::uno::Sequence<sal_Int8>>({0, 1, 2});
    a2 = css::uno::makeAny<css::uno::Sequence<sal_Int8>>({0, 1, 3});
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));

    a1 = css::uno::makeAny<css::uno::Sequence<sal_Int8>>({0, 1, 2});
    a2 = css::uno::makeAny<css::uno::Sequence<sal_Int8>>({0, 1});
    CPPUNIT_ASSERT_EQUAL(a1 == a2, comphelper::compare(a1, a2));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
