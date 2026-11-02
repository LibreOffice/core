/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/string.hxx>
#include <sal/types.h>
#include <map>

#include <jvmfwk/plugins/sunmajor/pluginlib/sunversion.hxx>

class TestSunVersion : public CppUnit::TestFixture {};

CPPUNIT_TEST_FIXTURE(TestSunVersion, testGoodVersions)
{
    static char const * versions[] = {"1.4.0", "1.4.1", "1.0.0", "10.0.0", "10.10.0",
                         "10.2.2", "10.10.0", "10.10.10", "111.0.999",
                         "1.4.1_01", "9.90.99_09", "1.4.1_99",
                         "1.4.1_00a", "1.4.1-ea", "1.4.1-beta", "1.4.1-rc1",
                         "1.5.0_01-ea", "1.5.0_01-rc2",
                         "10.0", "10.10", "1.5.0_010", "27.0.1.0.2", "1.2.3.4.5.6.7.8",
                         "1.5.0-p27"};

    //parsing test (positive)
    for (size_t i = 0; i < std::size(versions); i++)
    {
        jfw_plugin::SunVersion ver(versions[i]);
        CPPUNIT_ASSERT_MESSAGE(
            OString(OString::Concat("SunVersion(\"") + versions[i] + "\") good").getStr(), ver);
    }
}

CPPUNIT_TEST_FIXTURE(TestSunVersion, testBadVersions)
{
    static char const * badVersions[] = {".4.0", "..1", "", "10.10.0.", "10.10.0-", "10.10.0.",
                            "10.2-2", "10_10.0", "10..10", "a.0.999",
                            "1.4b.1_01", "9.90.-99_09", "1.4.1_99-",
                            "1.4.1_00a2", "1.4.0_z01z", "1.4.1__99A",
                            "1.4.1-1ea", "1.5.0._01-", "1.5.0_01-eac",
                            "1.1.1-01-01", "1.1.1a", "1.1.1_01.2", "1.2.3.4.5.6.7.8.9"};

    //Parsing test (negative)
    for (size_t i = 0; i < std::size(badVersions); i++)
    {
        jfw_plugin::SunVersion ver(badVersions[i]);
        CPPUNIT_ASSERT_MESSAGE(
            OString(OString::Concat("SunVersion(\"") + badVersions[i] + "\") bad").getStr(), !ver);
    }
}

CPPUNIT_TEST_FIXTURE(TestSunVersion, testVersionOrder)
{
    static char const * orderedVer[] = { "1.3.1-ea", "1.3.1-beta", "1.3.1-rc1",
                            "1.3.1", "1.3.1_00a", "1.3.1_01", "1.3.1_01a",
                            "1.3.2", "1.4.0", "1.5.0_01-ea", "2.0.0",
                            "27.0.1.0.3", "27.0.1.0.3.0.0.1", "27.0.1.0.4", "27.0.1.1"};

    // Ordering test
    const size_t numOrdered = std::size(orderedVer);
    for (size_t i = 0; i < numOrdered; i ++)
    {
        jfw_plugin::SunVersion curVer(orderedVer[i]);
        CPPUNIT_ASSERT_MESSAGE(
            OString(OString::Concat("SunVersion(\"") + orderedVer[i] + "\") good").getStr(),
            curVer);
        for (size_t j = 0; j < numOrdered; j++)
        {
            jfw_plugin::SunVersion compVer(orderedVer[j]);
            if (i < j)
            {
                CPPUNIT_ASSERT_MESSAGE(
                    OString(OString::Concat("\"") + orderedVer[i] + "\" < \"" + orderedVer[j])
                        .getStr(),
                    curVer < compVer);
            }
            else if (i == j)
            {
                CPPUNIT_ASSERT_MESSAGE(
                    OString(OString::Concat("\"") + orderedVer[i] + "\" == \"" + orderedVer[j])
                        .getStr(),
                    curVer.operator ==(compVer));
                CPPUNIT_ASSERT_MESSAGE(
                    OString(OString::Concat("not \"") + orderedVer[i] + "\" > \"" + orderedVer[j])
                        .getStr(),
                    ! (curVer > compVer));
                CPPUNIT_ASSERT_MESSAGE(
                    OString(OString::Concat("not \"") + orderedVer[i] + "\" < \"" + orderedVer[j])
                        .getStr(),
                    ! (curVer < compVer));
            }
            else if (i > j)
            {
                CPPUNIT_ASSERT_MESSAGE(
                    OString(OString::Concat("\"") + orderedVer[i] + "\" > \"" + orderedVer[j])
                        .getStr(),
                    curVer > compVer);
            }
        }
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
