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

#include <basegfx/tools/tools.hxx>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>


using namespace ::basegfx;

namespace basegfxtools
{

class createAreaGeometryTest : public CppUnit::TestFixture
{
    B2DPolygon mPoly;
    const float mStrokeWidth;
    static B2DPolygon getTestVector()
    {
        B2DPolygon aTestPoly;
        aTestPoly.reserve(6);
        aTestPoly.append(B2DPoint(2.26965,2.33533));
        aTestPoly.append(B2DPoint(523.855,2.33533));
        aTestPoly.append(B2DPoint(523.855,312.263));
        aTestPoly.append(B2DPoint(2.26965,2.33533));
        aTestPoly.append(B2DPoint(523.855,2.33533));
        aTestPoly.append(B2DPoint(523.855,312.263));
        return aTestPoly;
    }

public:
    createAreaGeometryTest():
    mPoly(getTestVector()),
    mStrokeWidth(2.3*0.5)
    {}
    void setUp() SAL_OVERRIDE
    {}

    void tearDown() SAL_OVERRIDE
    {}

    void test()
    {
         ::basegfx::B2DPolyPolygon aStrokedPolyPoly;
         aStrokedPolyPoly.append(basegfx::tools::createAreaGeometry(
                                 mPoly,
                                 mStrokeWidth,
                                 basegfx::B2DLINEJOIN_NONE,
                                 com::sun::star::drawing::LineCap_SQUARE));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(createAreaGeometryTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(basegfxtools::createAreaGeometryTest);
} // namespace basegfxtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
