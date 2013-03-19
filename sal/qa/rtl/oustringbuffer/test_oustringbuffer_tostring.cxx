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

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"

namespace test { namespace oustringbuffer {

class ToString: public CppUnit::TestFixture {
private:
    void testToString();

    CPPUNIT_TEST_SUITE(ToString);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustringbuffer::ToString);

void test::oustringbuffer::ToString::testToString() {
    rtl::OUStringBuffer sb(rtl::OUString("test string"));
    rtl::OUString str = sb.toString();
    CPPUNIT_ASSERT( str == "test string" );
    // returned OUString must be independent from sb
    sb.append( (sal_Unicode)'a' );
    CPPUNIT_ASSERT( str == "test string" );
    sb.setLength(0);
    CPPUNIT_ASSERT( str == "test string" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
