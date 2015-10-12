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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <basegfx/tools/keystoplerp.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <boost/tuple/tuple.hpp>

using namespace ::basegfx;
using namespace ::boost::tuples;

namespace basegfxtools
{

class KeyStopLerpTest : public CppUnit::TestFixture
{
    tools::KeyStopLerp maKeyStops;

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

    void setUp() override
    {}

    void tearDown() override
    {}

    void test()
    {
        double fAlpha;
        std::ptrdiff_t nIndex;

        tie(nIndex,fAlpha) = maKeyStops.lerp(-1.0);
        CPPUNIT_ASSERT_MESSAGE("-1.0", nIndex==0 && fAlpha==0.0);

        tie(nIndex,fAlpha) = maKeyStops.lerp(0.1);
        CPPUNIT_ASSERT_MESSAGE("0.1", nIndex==0 && fAlpha==0.0);

        tie(nIndex,fAlpha) = maKeyStops.lerp(0.3);
        CPPUNIT_ASSERT_MESSAGE("0.3", nIndex==0 && fTools::equal(fAlpha,0.5));

        tie(nIndex,fAlpha) = maKeyStops.lerp(0.5);
        CPPUNIT_ASSERT_MESSAGE("0.5", nIndex==0 && fTools::equal(fAlpha,1.0));

        tie(nIndex,fAlpha) = maKeyStops.lerp(0.51);
        CPPUNIT_ASSERT_MESSAGE("0.51", nIndex==1 && fTools::equal(fAlpha,0.025));

        tie(nIndex,fAlpha) = maKeyStops.lerp(0.9);
        CPPUNIT_ASSERT_MESSAGE("0.51", nIndex==1 && fTools::equal(fAlpha,1.0));

        tie(nIndex,fAlpha) = maKeyStops.lerp(1.0);
        CPPUNIT_ASSERT_MESSAGE("0.51", nIndex==1 && fAlpha==1.0);
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
