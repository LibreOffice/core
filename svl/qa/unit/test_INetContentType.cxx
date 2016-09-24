/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstring>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/ustring.hxx>
#include <svl/inettype.hxx>
#include <tools/inetmime.hxx>

namespace {

class Test: public CppUnit::TestFixture {
public:
    void testBad();

    void testFull();

    void testFollow();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testBad);
    CPPUNIT_TEST(testFull);
    CPPUNIT_TEST(testFollow);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testBad() {
    OUString in("foo=bar");
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Unicode const *>(nullptr),
        INetMIME::scanContentType(in.getStr(), in.getStr() + in.getLength()));
    OUString t;
    OUString s;
    INetContentTypeParameterList ps;
    CPPUNIT_ASSERT(!INetContentTypes::parse(in, t, s, &ps));
    CPPUNIT_ASSERT(t.isEmpty());
    CPPUNIT_ASSERT(s.isEmpty());
    CPPUNIT_ASSERT(ps.end() == ps.find("foo"));
}

void Test::testFull() {
    OUString in("foo/bar;baz=boz");
    CPPUNIT_ASSERT_EQUAL(
        in.getStr() + in.getLength(),
        INetMIME::scanContentType(in.getStr(), in.getStr() + in.getLength()));
    OUString t;
    OUString s;
    INetContentTypeParameterList ps;
    CPPUNIT_ASSERT(INetContentTypes::parse(in, t, s, &ps));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), t);
    CPPUNIT_ASSERT_EQUAL(OUString("bar"), s);
    auto iter = ps.find("baz");
    CPPUNIT_ASSERT(iter != ps.end());
    CPPUNIT_ASSERT_EQUAL(OUString("boz"), iter->second.m_sValue);
}

void Test::testFollow() {
    OUString in("foo/bar;baz=boz;base64,");
    CPPUNIT_ASSERT_EQUAL(
        in.getStr() + std::strlen("foo/bar;baz=boz"),
        INetMIME::scanContentType(in.getStr(), in.getStr() + in.getLength()));
    OUString t;
    OUString s;
    INetContentTypeParameterList ps;
    CPPUNIT_ASSERT(!INetContentTypes::parse(in, t, s));
    CPPUNIT_ASSERT(t.isEmpty());
    CPPUNIT_ASSERT(s.isEmpty());
    CPPUNIT_ASSERT(ps.end() == ps.find("baz"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
