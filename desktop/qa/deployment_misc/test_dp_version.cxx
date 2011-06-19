/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <cstddef>

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
