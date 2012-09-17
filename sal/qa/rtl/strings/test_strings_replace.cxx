/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <ostream>

#include <sal/types.h>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

namespace rtl {

template< typename charT, typename traits > std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, rtl::OString const & string)
{
    return stream << string.getStr();
        // best effort; potentially loses data due to embedded null characters
}

}

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
