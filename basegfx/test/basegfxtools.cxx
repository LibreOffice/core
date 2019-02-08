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

#include <basegfx/utils/keystoplerp.hxx>
#include <basegfx/numeric/ftools.hxx>

using namespace ::basegfx;

namespace basegfxtools
{

class KeyStopLerpTest : public CppUnit::TestFixture
{
    utils::KeyStopLerp maKeyStops;

    static std::vector<double> getTestVector()
    {
        std::vector<double> aStops(3);
        aStops[0] = 0.1;
        aStops[1] = 0.5;
        aStops[2] = 0.9;
        return aStops;
    }

public:
    KeyStopLerpTest() :
        maKeyStops(getTestVector())
    {}


    void test()
    {
        double fAlpha;
        std::ptrdiff_t nIndex;

        std::tie(nIndex,fAlpha) = maKeyStops.lerp(-1.0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("-1.0", std::ptrdiff_t(0), nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("-1.0", 0.0, fAlpha);

        std::tie(nIndex,fAlpha) = maKeyStops.lerp(0.1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.1", std::ptrdiff_t(0), nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.1", 0.0, fAlpha);

        std::tie(nIndex,fAlpha) = maKeyStops.lerp(0.3);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.3", std::ptrdiff_t(0), nIndex);
        CPPUNIT_ASSERT_MESSAGE("0.3", fTools::equal(fAlpha,0.5));

        std::tie(nIndex,fAlpha) = maKeyStops.lerp(0.5);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.5", std::ptrdiff_t(0), nIndex);
        CPPUNIT_ASSERT_MESSAGE("0.5", fTools::equal(fAlpha,1.0));

        std::tie(nIndex,fAlpha) = maKeyStops.lerp(0.51);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.51", std::ptrdiff_t(1), nIndex);
        CPPUNIT_ASSERT_MESSAGE("0.51", fTools::equal(fAlpha,0.025));

        std::tie(nIndex,fAlpha) = maKeyStops.lerp(0.9);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.51", std::ptrdiff_t(1), nIndex);
        CPPUNIT_ASSERT_MESSAGE("0.51", fTools::equal(fAlpha,1.0));

        std::tie(nIndex,fAlpha) = maKeyStops.lerp(1.0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("0.51", std::ptrdiff_t(1), nIndex);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("0.51", 1.0, fAlpha, 1E-12);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(KeyStopLerpTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(basegfxtools::KeyStopLerpTest);
} // namespace basegfxtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
