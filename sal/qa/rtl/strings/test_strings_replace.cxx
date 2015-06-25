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

OUString s_empty;
OUString s_bar("bar");
OUString s_bars("bars");
OUString s_foo("foo");
OUString s_other("other");
OUString s_xa("xa");
OUString s_xx("xx");

class Test: public CppUnit::TestFixture {
private:
    void stringReplaceFirst();

    void stringReplaceAll();

    void ustringReplaceFirst();

    void ustringReplaceFirstAsciiL();

    void ustringReplaceFirstToAsciiL();

    void ustringReplaceFirstAsciiLAsciiL();

    void ustringReplaceAll();

    void ustringReplaceAllAsciiL();

    void ustringReplaceAllToAsciiL();

    void ustringReplaceAllAsciiLAsciiL();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(stringReplaceFirst);
    CPPUNIT_TEST(stringReplaceAll);
    CPPUNIT_TEST(ustringReplaceFirst);
    CPPUNIT_TEST(ustringReplaceFirstAsciiL);
    CPPUNIT_TEST(ustringReplaceFirstToAsciiL);
    CPPUNIT_TEST(ustringReplaceFirstAsciiLAsciiL);
    CPPUNIT_TEST(ustringReplaceAll);
    CPPUNIT_TEST(ustringReplaceAllAsciiL);
    CPPUNIT_TEST(ustringReplaceAllToAsciiL);
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
        rtl::OUString("foobarfoo").replaceFirst(s_foo, s_other));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        rtl::OUString("foobarfoo").replaceFirst(s_bars, s_other));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("otherbarfoo"),
            rtl::OUString("foobarfoo").replaceFirst(s_foo, s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarother"),
            rtl::OUString("foobarfoo").replaceFirst(s_foo, s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarfoo"),
            rtl::OUString("foobarfoo").replaceFirst(s_bar, s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }
}

void Test::ustringReplaceFirstAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarfoo"),
        rtl::OUString("foobarfoo").replaceFirst("foo", s_other));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        rtl::OUString("foobarfoo").replaceFirst("bars", s_other));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("otherbarfoo"),
            rtl::OUString("foobarfoo").replaceFirst("foo", s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarother"),
            rtl::OUString("foobarfoo").replaceFirst("foo", s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarfoo"),
            rtl::OUString("foobarfoo").replaceFirst("bar", s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(), rtl::OUString("xa").replaceFirst("xa", s_empty));
}

void Test::ustringReplaceFirstToAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarfoo"),
        rtl::OUString("foobarfoo").replaceFirst(s_foo, "other"));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        rtl::OUString("foobarfoo").replaceFirst(s_bars, "other"));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("otherbarfoo"),
            rtl::OUString("foobarfoo").replaceFirst(s_foo, "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarother"),
            rtl::OUString("foobarfoo").replaceFirst(s_foo, "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            rtl::OUString("foobarfoo"),
            rtl::OUString("foobarfoo").replaceFirst(s_bar, "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(), rtl::OUString("xa").replaceFirst(s_xa, ""));
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

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(), rtl::OUString("xa").replaceFirst("xa", ""));
}

void Test::ustringReplaceAll() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarother"),
        rtl::OUString("foobarfoo").replaceAll(s_foo, s_other));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        rtl::OUString("foobarfoo").replaceAll(s_bars, s_other));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("xxa"),
        rtl::OUString("xaa").replaceAll(s_xa, s_xx));
}

void Test::ustringReplaceAllAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarother"),
        rtl::OUString("foobarfoo").replaceAll("foo", s_other));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        rtl::OUString("foobarfoo").replaceAll("bars", s_other));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("xxa"),
        rtl::OUString("xaa").replaceAll("xa", s_xx));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(), rtl::OUString("xa").replaceAll("xa", s_empty));
}

void Test::ustringReplaceAllToAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("otherbarother"),
        rtl::OUString("foobarfoo").replaceAll(s_foo, "other"));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("foobarfoo"),
        rtl::OUString("foobarfoo").replaceAll(s_bars, "other"));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("xxa"),
        rtl::OUString("xaa").replaceAll(s_xa, "xx"));

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(), rtl::OUString("xa").replaceAll(s_xa, ""));
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

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(), rtl::OUString("xa").replaceAll("xa", ""));
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
