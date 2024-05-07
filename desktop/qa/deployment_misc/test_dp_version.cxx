/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cstddef>
#include <sal/types.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/ustring.hxx>

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
        OUString version1;
        OUString version2;
        ::dp_misc::Order order;
    };
    static Data const data[] = {
        { OUString(),
          u"0.0000.00.0"_ustr,
          ::dp_misc::EQUAL },
        { u".01"_ustr,
          u"0.1"_ustr,
          ::dp_misc::EQUAL },
        { u"10"_ustr,
          u"2"_ustr,
          ::dp_misc::GREATER },
        { u"9223372036854775808"_ustr,
              // 2^63
          u"9223372036854775807"_ustr,
          ::dp_misc::GREATER }
    };
    for (std::size_t i = 0; i < std::size(data); ++i) {
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
