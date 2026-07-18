/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <sal/config.h>

#include <Splines.hxx>

#include <test/unoapi_test.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/Position3D.hpp>
#include <cppunit/TestAssert.h>

namespace
{
class SplinesTest : public UnoApiTest
{
public:
    SplinesTest()
        : UnoApiTest(u""_ustr)
    {
    }

    static constexpr double fAssertDelta = 1e-4;
};

CPPUNIT_TEST_FIXTURE(SplinesTest, bSplinesLinear1)
{
    std::vector<std::vector<css::drawing::Position3D>> aPoints;

    //Input points: (0,0), (2,2)
    aPoints.push_back(std::vector<css::drawing::Position3D>());
    aPoints[0].push_back(css::drawing::Position3D(0, 0, 0));
    aPoints[0].push_back(css::drawing::Position3D(2, 2, 0));
    sal_uInt32 nResolution = 2;
    sal_uInt32 nDegree = 1;

    std::vector<std::vector<css::drawing::Position3D>> aResult;

    chart::SplineCalculator::CalculateBSplines(aPoints, aResult, nResolution, nDegree);

    //Expected: result points form a line
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aResult.size()));
    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(aResult[0].size()));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][1].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][1].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][1].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[0][2].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[0][2].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][2].PositionZ, SplinesTest::fAssertDelta);
}

CPPUNIT_TEST_FIXTURE(SplinesTest, bSplinesLinear2)
{
    std::vector<std::vector<css::drawing::Position3D>> aPoints;

    //Input points: (0,0), (3,3)
    aPoints.push_back(std::vector<css::drawing::Position3D>());
    aPoints[0].push_back(css::drawing::Position3D(0, 0, 0));
    aPoints[0].push_back(css::drawing::Position3D(3, 3, 0));
    sal_uInt32 nResolution = 3;
    sal_uInt32 nDegree = 1;

    std::vector<std::vector<css::drawing::Position3D>> aResult;

    chart::SplineCalculator::CalculateBSplines(aPoints, aResult, nResolution, nDegree);

    //Expected: result points form a line
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aResult.size()));
    CPPUNIT_ASSERT_EQUAL(4, static_cast<int>(aResult[0].size()));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][1].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][1].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][1].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[0][2].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[0][2].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][2].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aResult[0][3].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aResult[0][3].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][3].PositionZ, SplinesTest::fAssertDelta);
}

CPPUNIT_TEST_FIXTURE(SplinesTest, bSplinesArch)
{
    std::vector<std::vector<css::drawing::Position3D>> aPoints;

    //Input points: (0,0), (1,1), (2,0)
    aPoints.push_back(std::vector<css::drawing::Position3D>());
    aPoints[0].push_back(css::drawing::Position3D(0, 0, 0));
    aPoints[0].push_back(css::drawing::Position3D(1, 1, 0));
    aPoints[0].push_back(css::drawing::Position3D(2, 0, 0));
    sal_uInt32 nResolution = 3;
    sal_uInt32 nDegree = 2;

    std::vector<std::vector<css::drawing::Position3D>> aResult;

    chart::SplineCalculator::CalculateBSplines(aPoints, aResult, nResolution, nDegree);

    //Expected: result points form an arch shape
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aResult.size()));
    CPPUNIT_ASSERT_EQUAL(7, static_cast<int>(aResult[0].size()));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1 / 3.0, aResult[0][1].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.555555555555556, aResult[0][1].PositionY,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][1].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2 / 3.0, aResult[0][2].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.888888888888889, aResult[0][2].PositionY,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][2].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][3].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][3].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][3].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(4 / 3.0, aResult[0][4].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.888888888888889, aResult[0][4].PositionY,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][4].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5 / 3.0, aResult[0][5].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.555555555555556, aResult[0][5].PositionY,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][5].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[0][6].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][6].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][6].PositionZ, SplinesTest::fAssertDelta);
}

CPPUNIT_TEST_FIXTURE(SplinesTest, bSplinesLoop)
{
    std::vector<std::vector<css::drawing::Position3D>> aPoints;

    //Input points: (0,0), (1,1), (2,0), (0,0)
    aPoints.push_back(std::vector<css::drawing::Position3D>());
    aPoints[0].push_back(css::drawing::Position3D(0, 0, 0));
    aPoints[0].push_back(css::drawing::Position3D(1, 1, 0));
    aPoints[0].push_back(css::drawing::Position3D(2, 0, 0));
    aPoints[0].push_back(css::drawing::Position3D(0, 0, 0));
    sal_uInt32 nResolution = 2;
    sal_uInt32 nDegree = 2;

    std::vector<std::vector<css::drawing::Position3D>> aResult;

    chart::SplineCalculator::CalculateBSplines(aPoints, aResult, nResolution, nDegree);

    //Expected: result points form a loop shape
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aResult.size()));
    CPPUNIT_ASSERT_EQUAL(7, static_cast<int>(aResult[0].size()));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.449682859897576, aResult[0][1].PositionX,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.809837501016391, aResult[0][1].PositionY,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][1].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][2].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][2].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][2].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.65095142030727, aResult[0][3].PositionX,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.570487496950826, aResult[0][3].PositionY,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][3].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[0][4].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][4].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][4].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.49811427366873, aResult[0][5].PositionX,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.238807647737986, aResult[0][5].PositionY,
                                 SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][5].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][6].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][6].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][6].PositionZ, SplinesTest::fAssertDelta);
}

CPPUNIT_TEST_FIXTURE(SplinesTest, bSplines2SeriesLinear)
{
    //Two (rather than just one) series of input points
    std::vector<std::vector<css::drawing::Position3D>> aPoints;

    aPoints.push_back(std::vector<css::drawing::Position3D>());
    aPoints[0].push_back(css::drawing::Position3D(0, 0, 0));
    aPoints[0].push_back(css::drawing::Position3D(2, 2, 0));

    aPoints.push_back(std::vector<css::drawing::Position3D>());
    aPoints[1].push_back(css::drawing::Position3D(0, 0, 0));
    aPoints[1].push_back(css::drawing::Position3D(-2, 2, 0));
    sal_uInt32 nResolution = 2;
    sal_uInt32 nDegree = 1;

    std::vector<std::vector<css::drawing::Position3D>> aResult;

    chart::SplineCalculator::CalculateBSplines(aPoints, aResult, nResolution, nDegree);

    //Expected: Two results series, each with three points along a line
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(aResult.size()));
    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(aResult[0].size()));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][0].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][1].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[0][1].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][1].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[0][2].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[0][2].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[0][2].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(aResult[1].size()));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[1][0].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[1][0].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[1][0].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aResult[1][1].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aResult[1][1].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[1][1].PositionZ, SplinesTest::fAssertDelta);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(-2.0, aResult[1][2].PositionX, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aResult[1][2].PositionY, SplinesTest::fAssertDelta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aResult[1][2].PositionZ, SplinesTest::fAssertDelta);
}

} //namespace

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
