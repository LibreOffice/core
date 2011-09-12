/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
*
************************************************************************/

#include "precompiled_sal.hxx"
#include "sal/config.h"
#include "sal/precppunit.hxx"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "osl/file.hxx"

#if defined WNT
#define MY_PATH_IN "/c:/foo/bar"
#define MY_PATH_OUT "c:\\foo\\bar"
#define MY_PATH_OUT_CONT MY_PATH_OUT "\\"
#define MY_PATH_OUT_REL "foo\\bar"
#else
#define MY_PATH_IN "/foo/bar"
#define MY_PATH_OUT MY_PATH_IN
#define MY_PATH_OUT_CONT MY_PATH_OUT "/"
#define MY_PATH_OUT_REL "foo/bar"
#endif

namespace {

class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testBadScheme);
    CPPUNIT_TEST(testNoScheme);
    CPPUNIT_TEST(testBadAuthority);
    CPPUNIT_TEST(testLocalhost1Authority);
    CPPUNIT_TEST(testLocalhost2Authority);
    CPPUNIT_TEST(testLocalhost3Authority);
    CPPUNIT_TEST(testNoAuthority);
    CPPUNIT_TEST(testEmptyPath);
    CPPUNIT_TEST(testHomeAbbreviation);
    CPPUNIT_TEST(testOtherHomeAbbreviation);
    CPPUNIT_TEST(testRelative);
    CPPUNIT_TEST(testEscape);
    CPPUNIT_TEST(testBadEscape2f);
    CPPUNIT_TEST(testBadEscape2F);
    CPPUNIT_TEST(testBad0);
    CPPUNIT_TEST(testBadEscape0);
    CPPUNIT_TEST(testBadQuery);
    CPPUNIT_TEST(testBadFragment);
    CPPUNIT_TEST_SUITE_END();

    void testBadScheme();
    void testNoScheme();
    void testBadAuthority();
    void testLocalhost1Authority();
    void testLocalhost2Authority();
    void testLocalhost3Authority();
    void testNoAuthority();
    void testEmptyPath();
    void testHomeAbbreviation();
    void testOtherHomeAbbreviation();
    void testRelative();
    void testEscape();
    void testBadEscape2f();
    void testBadEscape2F();
    void testBad0();
    void testBadEscape0();
    void testBadQuery();
    void testBadFragment();
};

void Test::testBadScheme() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foo:bar")), p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
}

void Test::testNoScheme() {
#if !defined WNT //TODO
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("//" MY_PATH_IN)), p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MY_PATH_OUT)), p);
#endif
}

void Test::testBadAuthority() {
#if defined UNX
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file://baz" MY_PATH_IN)), p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
#endif
}

void Test::testLocalhost1Authority() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("file://localhost" MY_PATH_IN)),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MY_PATH_OUT)), p);
}

void Test::testLocalhost2Authority() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("file://LOCALHOST" MY_PATH_IN)),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MY_PATH_OUT)), p);
}

void Test::testLocalhost3Authority() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("file://127.0.0.1" MY_PATH_IN)),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MY_PATH_OUT)), p);
}

void Test::testNoAuthority() {
#if !defined WNT //TODO
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MY_PATH_IN)), p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MY_PATH_OUT)), p);
#endif
}

void Test::testEmptyPath() {
#if defined UNX
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file://")), p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")), p);
#endif
}

void Test::testHomeAbbreviation() {
#if defined UNX
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///~" MY_PATH_IN)), p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
        // could theoretically fail due to osl::Security::getHomeDir problem
    CPPUNIT_ASSERT(p.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(MY_PATH_OUT)));
#endif
}

void Test::testOtherHomeAbbreviation() {
#if defined UNX
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///~baz" MY_PATH_IN)),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e); // not supported for now
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
#endif
}

void Test::testRelative() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foo/bar")), p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    CPPUNIT_ASSERT(
        p.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(MY_PATH_OUT_REL)));
}

void Test::testEscape() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("file://" MY_PATH_IN "/b%61z")),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MY_PATH_OUT_CONT "baz")), p);
}

void Test::testBadEscape2f() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("file://" MY_PATH_IN "/b%2fz")),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
}

void Test::testBadEscape2F() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("file://" MY_PATH_IN "/b%2Fz")),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
}

void Test::testBad0() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("file://" MY_PATH_IN "/b\x00z")),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
}

void Test::testBadEscape0() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("file://" MY_PATH_IN "/b%00z")),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
}

void Test::testBadQuery() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file://" MY_PATH_IN "?baz")),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
}

void Test::testBadFragment() {
    rtl::OUString p;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file://" MY_PATH_IN "#baz")),
        p);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_INVAL, e);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), p);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();
