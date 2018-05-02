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
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <tools/helpers.hxx>

namespace tools
{
class FRoundTest : public CppUnit::TestFixture
{
public:
    void testPositiveFRound()
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<long>(2), static_cast<sal_Int64>(FRound(1.6));
        CPPUNIT_ASSERT_EQUAL(static_cast<long>(1), static_cast<sal_Int64>(FRound(1.4));
    }

    void testNegativeFRound()
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<long>(-2), static_cast<sal_Int64>(FRound(-1.6));
        CPPUNIT_ASSERT_EQUAL(static_cast<long>(-1), static_cast<sal_Int64>(FRound(-1.4));
    }

    CPPUNIT_TEST_SUITE(FRoundTest);
    CPPUNIT_TEST(testPositiveFRound);
    CPPUNIT_TEST(testNegativeFRound);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FRoundTest);
} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
