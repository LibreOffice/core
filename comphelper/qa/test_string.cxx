/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_string.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 13:15:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include "comphelper/string.hxx"
#include "cppunit/simpleheader.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

class Test: public CppUnit::TestFixture {
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test() {
    rtl::OUString s1(RTL_CONSTASCII_USTRINGPARAM("foobarbar"));
    sal_Int32 n1;
    rtl::OUString s2(
        comphelper::string::searchAndReplace(
            s1, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baaz")), 0, &n1));
    CPPUNIT_ASSERT(
        s2 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foobaazbar")));
    CPPUNIT_ASSERT(n1 == 3);
    sal_Int32 n2;
    rtl::OUString s3(
        comphelper::string::searchAndReplace(
            s2, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bz")),
            n1 + RTL_CONSTASCII_LENGTH("baaz"), &n2));
    CPPUNIT_ASSERT(
        s3 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foobaazbz")));
    CPPUNIT_ASSERT(n2 == 7);
    sal_Int32 n3;
    rtl::OUString s4(
        comphelper::string::searchAndReplace(
            s3, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baz")),
            n2 + RTL_CONSTASCII_LENGTH("bz"), &n3));
    CPPUNIT_ASSERT(s4 == s3);
    CPPUNIT_ASSERT(n3 == -1);
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
