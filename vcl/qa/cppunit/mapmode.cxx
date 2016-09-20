/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>

#include <osl/file.hxx>
#include <osl/process.h>

#include <vcl/mapmod.hxx>

class VclMapModeTest : public test::BootstrapFixture
{
public:
    VclMapModeTest() : BootstrapFixture(true, false) {}

    void testMultiplier();

    CPPUNIT_TEST_SUITE(VclMapModeTest);
    CPPUNIT_TEST(testMultiplier);
    CPPUNIT_TEST_SUITE_END();
};

void VclMapModeTest::testMultiplier()
{
    MapMode aMapMode;
    CPPUNIT_ASSERT_MESSAGE( "Default map mode is MapUnit::Pixel, multiplier should be 1", aMapMode.GetUnitMultiplier() == 1 );
    aMapMode.SetMapUnit( MapUnit::SysFont );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::SysFont, multiplier should be 1", aMapMode.GetUnitMultiplier() == 1 );
    aMapMode.SetMapUnit( MapUnit::AppFont );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::AppFont, multiplier should be 1", aMapMode.GetUnitMultiplier() == 1 );
    aMapMode.SetMapUnit( MapUnit::MM_100th );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::MM_100th, multiplier should be 1", aMapMode.GetUnitMultiplier() == 1 );
    aMapMode.SetMapUnit( MapUnit::MM_10th );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::MM_10th, multiplier should be 10", aMapMode.GetUnitMultiplier() == 10 );
    aMapMode.SetMapUnit( MapUnit::MM );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::MM, multiplier should be 100", aMapMode.GetUnitMultiplier() == 100 );
    aMapMode.SetMapUnit( MapUnit::CM );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::CM, multiplier should be 1000", aMapMode.GetUnitMultiplier() == 1000 );
    aMapMode.SetMapUnit( MapUnit::Inch_1000th );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::Inch_1000th, multiplier should be 2.54", aMapMode.GetUnitMultiplier() == 2.54 );
    aMapMode.SetMapUnit( MapUnit::Inch_100th );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::Inch_100th, multiplier should be 2.54", aMapMode.GetUnitMultiplier() == 25.4 );
    aMapMode.SetMapUnit( MapUnit::Inch_10th );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::Inch_10th, multiplier should be 254", aMapMode.GetUnitMultiplier() == 254 );
    aMapMode.SetMapUnit( MapUnit::Inch );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::Inch, multiplier should be 2540", aMapMode.GetUnitMultiplier() == 2540 );
    aMapMode.SetMapUnit( MapUnit::Twip );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::Twip, multiplier should be 1.76388889", aMapMode.GetUnitMultiplier() == 1.76388889 );
    aMapMode.SetMapUnit( MapUnit::Point );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MapUnit::Point, multiplier should be 35.27777778", aMapMode.GetUnitMultiplier() == 35.27777778 );
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclMapModeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
