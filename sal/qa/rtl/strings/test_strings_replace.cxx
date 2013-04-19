/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <sal/types.h>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

namespace {

class Test: public CppUnit::TestFixture {
private:
    void stringReplaceFirst();

    void stringReplaceAll();

    void ustringReplaceFirst();

    void ustringReplaceFirstAsciiL();

    void ustringReplaceFirstAsciiLAsciiL();

    void ustringReplaceAll();

    void ustringReplaceAllAsciiL();

    void ustringReplaceAllAsciiLAsciiL();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(stringReplaceFirst);
    CPPUNIT_TEST(stringReplaceAll);
    CPPUNIT_TEST(ustringReplaceFirst);
    CPPUNIT_TEST(ustringReplaceFirstAsciiL);
    CPPUNIT_TEST(ustringReplaceFirstAsciiLAsciiL);
    CPPUNIT_TEST(ustringReplaceAll);
    CPPUNIT_TEST(ustringReplaceAllAsciiL);
    CPPUNIT_TEST(ustringReplaceAllAsciiLAsciiL);
    CPPUNIT_TEST_SUITE_END();
};

void Test::stringReplaceFirst() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OString("otherbarfoo"),
        rtl::OString("foobarfoo").replaceFirst("foo", "other"));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OString("foobarfoo"),
        rtl::OString("foobarfoo").replaceFirst("bars", "other"));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OString("otherbarfoo"),
            rtl::OString("foobarfoo").replaceFirst("foo", "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OString("foobarother"),
            rtl::OString("foobarfoo").replaceFirst("foo", "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OString("foobarfoo"),
            rtl::OString("foobarfoo").replaceFirst("bar", "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }
}

void Test::stringReplaceAll() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OString("otherbarother"),
        rtl::OString("foobarfoo").replaceAll("foo", "other"));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OString("foobarfoo"),
        rtl::OString("foobarfoo").replaceAll("bars", "other"));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OString("xxa"), rtl::OString("xaa").replaceAll("xa", "xx"));
}

void Test::ustringReplaceFirst() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarfoo"),
        rtl::OUString("foobarfoo").replaceFirst(
            rtl::OUString("foo"),
            rtl::OUString("other")));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        rtl::OUString("foobarfoo").replaceFirst(
            rtl::OUString("bars"),
            rtl::OUString("other")));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("otherbarfoo"),
            (rtl::OUString("foobarfoo").
             replaceFirst(
                 rtl::OUString("foo"),
                 rtl::OUString("other"), &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarother"),
            (rtl::OUString("foobarfoo").
             replaceFirst(
                 rtl::OUString("foo"),
                 rtl::OUString("other"), &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarfoo"),
            (rtl::OUString("foobarfoo").
             replaceFirst(
                 rtl::OUString("bar"),
                 rtl::OUString("other"), &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }
}

void Test::ustringReplaceFirstAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarfoo"),
        (rtl::OUString("foobarfoo").
         replaceFirst("foo",
             rtl::OUString("other"))));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        (rtl::OUString("foobarfoo").
         replaceFirst("bars",
             rtl::OUString("other"))));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("otherbarfoo"),
            (rtl::OUString("foobarfoo").
             replaceFirst("foo",
                 rtl::OUString("other"), &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarother"),
            (rtl::OUString("foobarfoo").
             replaceFirst("foo",
                 rtl::OUString("other"), &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarfoo"),
            (rtl::OUString("foobarfoo").
             replaceFirst("bar",
                 rtl::OUString("other"), &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }
}

void Test::ustringReplaceFirstAsciiLAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarfoo"),
        (rtl::OUString("foobarfoo").
         replaceFirst("foo", "other")));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        (rtl::OUString("foobarfoo").
         replaceFirst("bars", "other")));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("otherbarfoo"),
            (rtl::OUString("foobarfoo").
             replaceFirst("foo", "other", &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarother"),
            (rtl::OUString("foobarfoo").
             replaceFirst("foo", "other", &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarfoo"),
            (rtl::OUString("foobarfoo").
             replaceFirst("bar", "other", &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }
}

void Test::ustringReplaceAll() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarother"),
        rtl::OUString("foobarfoo").replaceAll(
            rtl::OUString("foo"),
            rtl::OUString("other")));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        rtl::OUString("foobarfoo").replaceAll(
            rtl::OUString("bars"),
            rtl::OUString("other")));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("xxa"),
        rtl::OUString("xaa").replaceAll(
            rtl::OUString("xa"),
            rtl::OUString("xx")));
}

void Test::ustringReplaceAllAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarother"),
        (rtl::OUString("foobarfoo").
         replaceAll("foo",
             rtl::OUString("other"))));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        (rtl::OUString("foobarfoo").
         replaceAll("bars",
             rtl::OUString("other"))));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("xxa"),
        rtl::OUString("xaa").replaceAll(
            "xa",
            rtl::OUString("xx")));
}

void Test::ustringReplaceAllAsciiLAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarother"),
        (rtl::OUString("foobarfoo").
         replaceAll("foo", "other")));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        (rtl::OUString("foobarfoo").
         replaceAll("bars", "other")));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("xxa"),
        (rtl::OUString("xaa").
         replaceAll("xa", "xx")));
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
