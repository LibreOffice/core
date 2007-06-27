/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_dp_version.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:26:53 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <cstddef>

#include "cppunit/simpleheader.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "../../source/deployment/inc/dp_version.hxx"

namespace {

class Test: public ::CppUnit::TestFixture {
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test() {
    struct Data {
        rtl::OUString version1;
        rtl::OUString version2;
        ::dp_misc::Order order;
    };
    static Data const data[] = {
        { rtl::OUString(),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0.0000.00.0")),
          ::dp_misc::EQUAL },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".01")),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0.1")),
          ::dp_misc::EQUAL },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("10")),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")),
          ::dp_misc::GREATER },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("9223372036854775808")),
              // 2^63
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("9223372036854775807")),
          ::dp_misc::GREATER }
    };
    for (::std::size_t i = 0; i < sizeof data / sizeof (Data); ++i) {
        CPPUNIT_ASSERT_EQUAL(
            data[i].order,
            ::dp_misc::compareVersions(data[i].version1, data[i].version2));
        static ::dp_misc::Order const reverse[3] = {
            ::dp_misc::GREATER, ::dp_misc::EQUAL, ::dp_misc::LESS
        };
        CPPUNIT_ASSERT_EQUAL(
            reverse[data[i].order],
            ::dp_misc::compareVersions(data[i].version2, data[i].version1));
    }
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
