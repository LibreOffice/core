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
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

namespace {

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp() override;

private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testDollar);
    CPPUNIT_TEST(testIndirectDollar);
    CPPUNIT_TEST(testThreeSegments);
    CPPUNIT_TEST_SUITE_END();

    void testDollar();

    void testIndirectDollar();

    void testThreeSegments();
};

void Test::setUp() {
    rtl::Bootstrap::set(u"TEST"_ustr, u"<expanded TEST>"_ustr);
    rtl::Bootstrap::set(u"WITH_DOLLAR"_ustr, u"foo\\$TEST"_ustr);
    rtl::Bootstrap::set(u"INDIRECT"_ustr, u"$WITH_DOLLAR"_ustr);
}

void Test::testDollar() {
    OUString s(u"$WITH_DOLLAR"_ustr);
    rtl::Bootstrap::expandMacros(s);
    CPPUNIT_ASSERT_EQUAL(u"foo$TEST"_ustr, s);
}

void Test::testIndirectDollar() {
    OUString s(u"$INDIRECT"_ustr);
    rtl::Bootstrap::expandMacros(s);
    CPPUNIT_ASSERT_EQUAL(u"foo$TEST"_ustr, s);
}

void Test::testThreeSegments() {
    OUString url;
    CPPUNIT_ASSERT(rtl::Bootstrap::get(u"UserInstallation"_ustr, url));
    url += "/testThreeSegments.ini";
    {
        osl::File f(url);
        CPPUNIT_ASSERT_EQUAL(
            osl::FileBase::E_None, f.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create));
        char const data[] = "[section1]\nkey=value1\n[section2]\nkey=value2\n";
        sal_uInt64 length = std::strlen(data);
        sal_uInt64 written = 0;
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, f.write(data, length, written));
        CPPUNIT_ASSERT_EQUAL(length, written);
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, f.close());
    }
    OUString esc;
    {
        OUStringBuffer buf;
        for (sal_Int32 i = 0; i != url.getLength(); ++i) {
            auto const c = url[i];
            switch (c) {
            case ':':
            case '\\':
            case '{':
            case '}':
                buf.append('\\');
            }
            buf.append(c);
        }
        esc = buf.makeStringAndClear();
    }
    {
        OUString s("${" + esc + ":section1:key}");
        rtl::Bootstrap::expandMacros(s);
        CPPUNIT_ASSERT_EQUAL(u"value1"_ustr, s);
    }
    {
        OUString s("${" + esc + ":section2:key}");
        rtl::Bootstrap::expandMacros(s);
        CPPUNIT_ASSERT_EQUAL(u"value2"_ustr, s);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
