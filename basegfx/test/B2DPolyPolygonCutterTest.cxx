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

#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

namespace basegfx
{
class b2dpolypolygoncutter : public CppUnit::TestFixture
{
public:
    void testMergeToSinglePolyPolygon()
    {
        { // Adjacent polygons merged to one, closed manually.
            B2DPolygon poly1{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } };
            B2DPolygon poly2{ { 0, 1 }, { 1, 1 }, { 1, 2 }, { 0, 2 }, { 0, 1 } };
            B2DPolyPolygon expected(
                B2DPolygon{ { 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 2 }, { 0, 1 }, { 0, 0 } });
            expected.setClosed(true);
            B2DPolyPolygon result
                = utils::mergeToSinglePolyPolygon({ B2DPolyPolygon(poly1), B2DPolyPolygon(poly2) });
            CPPUNIT_ASSERT_EQUAL(expected, result);
        }

        { // Adjacent polygons merged to one, closed using setClosed().
            B2DPolygon poly1{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
            B2DPolygon poly2{ { 0, 1 }, { 1, 1 }, { 1, 2 }, { 0, 2 } };
            poly1.setClosed(true);
            poly2.setClosed(true);
            B2DPolyPolygon expected(
                B2DPolygon{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 2 }, { 0, 1 } });
            expected.setClosed(true);
            B2DPolyPolygon result
                = utils::mergeToSinglePolyPolygon({ B2DPolyPolygon(poly1), B2DPolyPolygon(poly2) });
            CPPUNIT_ASSERT_EQUAL(expected, result);
        }

        { // Non-adjacent polygons, no merge.
            B2DPolygon poly1{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
            B2DPolygon poly2{ { 0, 2 }, { 1, 3 }, { 1, 3 }, { 0, 3 } };
            poly1.setClosed(true);
            poly2.setClosed(true);
            B2DPolyPolygon expected;
            expected.append(poly1);
            expected.append(poly2);
            B2DPolyPolygon result
                = utils::mergeToSinglePolyPolygon({ B2DPolyPolygon(poly1), B2DPolyPolygon(poly2) });
            CPPUNIT_ASSERT_EQUAL(expected, result);
        }

        { // Horizontal and vertical rectangle that together form a cross.
            B2DPolygon poly1{ { 1, 0 }, { 2, 0 }, { 2, 3 }, { 1, 3 } };
            B2DPolygon poly2{ { 0, 1 }, { 3, 1 }, { 3, 2 }, { 0, 2 } };
            poly1.setClosed(true);
            poly2.setClosed(true);
            B2DPolyPolygon expected(B2DPolygon{ { 1, 0 },
                                                { 2, 0 },
                                                { 2, 1 },
                                                { 3, 1 },
                                                { 3, 2 },
                                                { 2, 2 },
                                                { 2, 3 },
                                                { 1, 3 },
                                                { 1, 2 },
                                                { 0, 2 },
                                                { 0, 1 },
                                                { 1, 1 } });
            expected.setClosed(true);
            B2DPolyPolygon result
                = utils::mergeToSinglePolyPolygon({ B2DPolyPolygon(poly1), B2DPolyPolygon(poly2) });
            CPPUNIT_ASSERT_EQUAL(expected, result);
        }
    }

    CPPUNIT_TEST_SUITE(b2dpolypolygoncutter);
    CPPUNIT_TEST(testMergeToSinglePolyPolygon);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolypolygoncutter);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
