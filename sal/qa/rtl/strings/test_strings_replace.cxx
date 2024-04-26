/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace {

OUString s_empty;
OUString s_bar(u"bar"_ustr);
OUString s_bars(u"bars"_ustr);
OUString s_foo(u"foo"_ustr);
OUString s_other(u"other"_ustr);
OUString s_xa(u"xa"_ustr);
OUString s_xx(u"xx"_ustr);

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
        "otherbarfoo"_ostr,
        "foobarfoo"_ostr.replaceFirst("foo"_ostr, "other"_ostr));

    CPPUNIT_ASSERT_EQUAL(
        "foobarfoo"_ostr,
        "foobarfoo"_ostr.replaceFirst("bars"_ostr, "other"_ostr));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            "otherbarfoo"_ostr,
            "foobarfoo"_ostr.replaceFirst("foo"_ostr, "other"_ostr, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            "foobarother"_ostr,
            "foobarfoo"_ostr.replaceFirst("foo"_ostr, "other"_ostr, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            "foobarfoo"_ostr,
            "foobarfoo"_ostr.replaceFirst("bar"_ostr, "other"_ostr, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }
}

void Test::stringReplaceAll() {
    CPPUNIT_ASSERT_EQUAL(
        "otherbarother"_ostr,
        "foobarfoo"_ostr.replaceAll("foo"_ostr, "other"_ostr));

    CPPUNIT_ASSERT_EQUAL(
        "foobarfoo"_ostr,
        "foobarfoo"_ostr.replaceAll("bars"_ostr, "other"_ostr));

    CPPUNIT_ASSERT_EQUAL(
        "xxa"_ostr, "xaa"_ostr.replaceAll("xa"_ostr, "xx"_ostr));
}

void Test::ustringReplaceFirst() {
    CPPUNIT_ASSERT_EQUAL(
        u"otherbarfoo"_ustr,
        u"foobarfoo"_ustr.replaceFirst(s_foo, s_other));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarfoo"_ustr,
        u"foobarfoo"_ustr.replaceFirst(s_bars, s_other));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            u"otherbarfoo"_ustr,
            u"foobarfoo"_ustr.replaceFirst(s_foo, s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            u"foobarother"_ustr,
            u"foobarfoo"_ustr.replaceFirst(s_foo, s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            u"foobarfoo"_ustr,
            u"foobarfoo"_ustr.replaceFirst(s_bar, s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }
}

void Test::ustringReplaceFirstAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        u"otherbarfoo"_ustr,
        u"foobarfoo"_ustr.replaceFirst("foo", s_other));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarfoo"_ustr,
        u"foobarfoo"_ustr.replaceFirst("bars", s_other));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            u"otherbarfoo"_ustr,
            u"foobarfoo"_ustr.replaceFirst("foo", s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            u"foobarother"_ustr,
            u"foobarfoo"_ustr.replaceFirst("foo", s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            u"foobarfoo"_ustr,
            u"foobarfoo"_ustr.replaceFirst("bar", s_other, &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }

    CPPUNIT_ASSERT_EQUAL(
        OUString(), u"xa"_ustr.replaceFirst("xa", s_empty));
}

void Test::ustringReplaceFirstToAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        u"otherbarfoo"_ustr,
        u"foobarfoo"_ustr.replaceFirst(s_foo, "other"));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarfoo"_ustr,
        u"foobarfoo"_ustr.replaceFirst(s_bars, "other"));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            u"otherbarfoo"_ustr,
            u"foobarfoo"_ustr.replaceFirst(s_foo, "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            u"foobarother"_ustr,
            u"foobarfoo"_ustr.replaceFirst(s_foo, "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            u"foobarfoo"_ustr,
            u"foobarfoo"_ustr.replaceFirst(s_bar, "other", &n));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }

    CPPUNIT_ASSERT_EQUAL(
        OUString(), u"xa"_ustr.replaceFirst(s_xa, ""));
}

void Test::ustringReplaceFirstAsciiLAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        u"otherbarfoo"_ustr,
        (u"foobarfoo"_ustr.
         replaceFirst("foo", "other")));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarfoo"_ustr,
        (u"foobarfoo"_ustr.
         replaceFirst("bars", "other")));

    {
        sal_Int32 n = 0;
        CPPUNIT_ASSERT_EQUAL(
            u"otherbarfoo"_ustr,
            (u"foobarfoo"_ustr.
             replaceFirst("foo", "other", &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), n);
    }

    {
        sal_Int32 n = 1;
        CPPUNIT_ASSERT_EQUAL(
            u"foobarother"_ustr,
            (u"foobarfoo"_ustr.
             replaceFirst("foo", "other", &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), n);
    }

    {
        sal_Int32 n = 4;
        CPPUNIT_ASSERT_EQUAL(
            u"foobarfoo"_ustr,
            (u"foobarfoo"_ustr.
             replaceFirst("bar", "other", &n)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), n);
    }

    CPPUNIT_ASSERT_EQUAL(
        OUString(), u"xa"_ustr.replaceFirst("xa", ""));
}

void Test::ustringReplaceAll() {
    CPPUNIT_ASSERT_EQUAL(
        u"otherbarother"_ustr,
        u"foobarfoo"_ustr.replaceAll(s_foo, s_other));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarfoo"_ustr,
        u"foobarfoo"_ustr.replaceAll(s_bars, s_other));

    CPPUNIT_ASSERT_EQUAL(
        u"xxa"_ustr,
        u"xaa"_ustr.replaceAll(s_xa, s_xx));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarbaz"_ustr, u"foobarfoo"_ustr.replaceAll(u"foo", u"baz", 1));
}

void Test::ustringReplaceAllAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        u"otherbarother"_ustr,
        u"foobarfoo"_ustr.replaceAll("foo", s_other));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarfoo"_ustr,
        u"foobarfoo"_ustr.replaceAll("bars", s_other));

    CPPUNIT_ASSERT_EQUAL(
        u"xxa"_ustr,
        u"xaa"_ustr.replaceAll("xa", s_xx));

    CPPUNIT_ASSERT_EQUAL(
        OUString(), u"xa"_ustr.replaceAll("xa", s_empty));
}

void Test::ustringReplaceAllToAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        u"otherbarother"_ustr,
        u"foobarfoo"_ustr.replaceAll(s_foo, "other"));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarfoo"_ustr,
        u"foobarfoo"_ustr.replaceAll(s_bars, "other"));

    CPPUNIT_ASSERT_EQUAL(
        u"xxa"_ustr,
        u"xaa"_ustr.replaceAll(s_xa, "xx"));

    CPPUNIT_ASSERT_EQUAL(
        OUString(), u"xa"_ustr.replaceAll(s_xa, ""));
}

void Test::ustringReplaceAllAsciiLAsciiL() {
    CPPUNIT_ASSERT_EQUAL(
        u"otherbarother"_ustr,
        (u"foobarfoo"_ustr.
         replaceAll("foo", "other")));

    CPPUNIT_ASSERT_EQUAL(
        u"foobarfoo"_ustr,
        (u"foobarfoo"_ustr.
         replaceAll("bars", "other")));

    CPPUNIT_ASSERT_EQUAL(
        u"xxa"_ustr,
        (u"xaa"_ustr.
         replaceAll("xa", "xx")));

    CPPUNIT_ASSERT_EQUAL(
        OUString(), u"xa"_ustr.replaceAll("xa", ""));
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
