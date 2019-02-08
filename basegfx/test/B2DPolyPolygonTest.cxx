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

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>

#include "boxclipper.hxx"

namespace basegfx
{
using ::basegfx2d::getRandomOrdinal;

class b2dpolypolygon : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    void testTrapezoidHelper()
    {
        B2DPolygon aPolygon;
        // provoke the PointBlockAllocator to exercise the freeIfLast path
        for (int i = 0; i < 16 * 10; i++)
        {
            B2DPoint aPoint(getRandomOrdinal(1000), getRandomOrdinal(1000));
            aPolygon.append(aPoint);
        }
        // scatter some duplicate points in to stress things more.
        for (int i = 0; i < 16 * 10; i++)
        {
            aPolygon.insert(getRandomOrdinal(aPolygon.count() - 1),
                            aPolygon.getB2DPoint(getRandomOrdinal(aPolygon.count() - 1)));
        }
        B2DPolygon aPolygonOffset;
        // duplicate the polygon and offset it slightly.
        for (size_t i = 0; i < aPolygon.count(); i++)
        {
            B2DPoint aPoint(aPolygon.getB2DPoint(i));
            aPoint += B2DPoint(0.5 - getRandomOrdinal(1), 0.5 - getRandomOrdinal(1));
            aPolygonOffset.append(aPoint);
        }
        B2DPolyPolygon aPolyPolygon;
        aPolyPolygon.append(aPolygon);
        aPolyPolygon.append(aPolygonOffset);
        B2DTrapezoidVector aVector;
        basegfx::utils::trapezoidSubdivide(aVector, aPolyPolygon);
        CPPUNIT_ASSERT_MESSAGE("more than zero sub-divided trapezoids", !aVector.empty());
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolypolygon);
    CPPUNIT_TEST(testTrapezoidHelper);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolypolygon

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolypolygon);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
