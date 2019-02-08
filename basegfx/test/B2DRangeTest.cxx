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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>

namespace basegfx
{
class b2drange : public CppUnit::TestFixture
{
private:
public:
    void check()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple range rounding from double to integer",
                                     B2IRange(1, 2, 4, 5), fround(B2DRange(1.2, 2.3, 3.5, 4.8)));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2drange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2drange);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
