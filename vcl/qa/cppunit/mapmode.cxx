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
    CPPUNIT_ASSERT_MESSAGE( "Default map mode is MAP_PIXEL, multiplier should be 1", aMapMode.GetUnitMultiplier() == 1 );
    aMapMode.SetMapUnit( MAP_SYSFONT );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_SYSFONT, multiplier should be 1", aMapMode.GetUnitMultiplier() == 1 );
    aMapMode.SetMapUnit( MAP_APPFONT );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_APPFONT, multiplier should be 1", aMapMode.GetUnitMultiplier() == 1 );
    aMapMode.SetMapUnit( MAP_100TH_MM );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_100TH_MM, multiplier should be 1", aMapMode.GetUnitMultiplier() == 1 );
    aMapMode.SetMapUnit( MAP_10TH_MM );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_10TH_MM, multiplier should be 10", aMapMode.GetUnitMultiplier() == 10 );
    aMapMode.SetMapUnit( MAP_MM );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_MM, multiplier should be 100", aMapMode.GetUnitMultiplier() == 100 );
    aMapMode.SetMapUnit( MAP_CM );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_CM, multiplier should be 1000", aMapMode.GetUnitMultiplier() == 1000 );
    aMapMode.SetMapUnit( MAP_1000TH_INCH );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_1000TH_INCH, multiplier should be 2.54", aMapMode.GetUnitMultiplier() == 2.54 );
    aMapMode.SetMapUnit( MAP_100TH_INCH );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_100TH_INCH, multiplier should be 2.54", aMapMode.GetUnitMultiplier() == 25.4 );
    aMapMode.SetMapUnit( MAP_10TH_INCH );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_10TH_INCH, multiplier should be 254", aMapMode.GetUnitMultiplier() == 254 );
    aMapMode.SetMapUnit( MAP_INCH );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_INCH, multiplier should be 2540", aMapMode.GetUnitMultiplier() == 2540 );
    aMapMode.SetMapUnit( MAP_TWIP );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_TWIP, multiplier should be 1.76388889", aMapMode.GetUnitMultiplier() == 1.76388889 );
    aMapMode.SetMapUnit( MAP_POINT );
    CPPUNIT_ASSERT_MESSAGE( "Map mode is MAP_POINT, multiplier should be 35.27777778", aMapMode.GetUnitMultiplier() == 35.27777778 );
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclMapModeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
