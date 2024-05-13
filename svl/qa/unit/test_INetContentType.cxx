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
    OUString in(u"foo=bar"_ustr);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<void const *>(nullptr),
        static_cast<void const *>(INetMIME::scanContentType(in)));
    OUString t;
    OUString s;
    INetContentTypeParameterList ps;
    CPPUNIT_ASSERT(!INetContentTypes::parse(in, t, s, &ps));
    CPPUNIT_ASSERT(t.isEmpty());
    CPPUNIT_ASSERT(s.isEmpty());
    CPPUNIT_ASSERT(bool(ps.end() == ps.find("foo"_ostr)));
}

void Test::testFull() {
    OUString in(u"foo/bar;baz=boz"_ustr);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<void const *>(in.getStr() + in.getLength()),
        static_cast<void const *>(INetMIME::scanContentType(in)));
    OUString t;
    OUString s;
    INetContentTypeParameterList ps;
    CPPUNIT_ASSERT(INetContentTypes::parse(in, t, s, &ps));
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, t);
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, s);
    auto iter = ps.find("baz"_ostr);
    CPPUNIT_ASSERT(iter != ps.end());
    CPPUNIT_ASSERT_EQUAL(u"boz"_ustr, iter->second.m_sValue);
}

void Test::testFollow() {
    OUString in(u"foo/bar;baz=boz;base64,"_ustr);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<void const *>(in.getStr() + std::strlen("foo/bar;baz=boz")),
        static_cast<void const *>(INetMIME::scanContentType(in)));
    OUString t;
    OUString s;
    INetContentTypeParameterList ps;
    CPPUNIT_ASSERT(!INetContentTypes::parse(in, t, s));
    CPPUNIT_ASSERT(t.isEmpty());
    CPPUNIT_ASSERT(s.isEmpty());
    CPPUNIT_ASSERT(bool(ps.end() == ps.find("baz"_ostr)));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
