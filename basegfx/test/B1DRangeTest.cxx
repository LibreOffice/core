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

#include <basegfx/range/b1drange.hxx>

namespace basegfx
{
class B1DRangeTest : public CppUnit::TestFixture
{
public:
    void checkIntervalAxioms()
    {
        // test interval axioms
        // (http://en.wikipedia.org/wiki/Interval_%28mathematics%29)
        B1DRange aRange;
        CPPUNIT_ASSERT_MESSAGE("default ctor - empty range", aRange.isEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("center - get cop-out value since range is empty", 0.0,
                                     aRange.getCenter());

        // degenerate interval
        aRange.expand(1);
        CPPUNIT_ASSERT_EQUAL(B1DRange(1.0, 1.0), aRange);
        CPPUNIT_ASSERT_MESSAGE("degenerate range - still, not empty!", !aRange.isEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("degenerate range - size of 0", 0.0, aRange.getRange());
        CPPUNIT_ASSERT_MESSAGE("same value as degenerate range - is inside range",
                               aRange.isInside(1.0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("center - must be the single range value", 1.0,
                                     aRange.getCenter());

        // proper interval
        aRange.expand(2.0);
        CPPUNIT_ASSERT_EQUAL(B1DRange(1.0, 2.0), aRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("proper range - size of 1", 1.0, aRange.getRange());
        CPPUNIT_ASSERT_MESSAGE("smaller value of range - is inside *closed* range",
                               aRange.isInside(1));
        CPPUNIT_ASSERT_MESSAGE("larger value of range - is inside *closed* range",
                               aRange.isInside(2));

        // center for proper interval that works for ints, too
        aRange.expand(3.0);
        CPPUNIT_ASSERT_EQUAL(B1DRange(1.0, 3.0), aRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("center - must be half of the range", 2.0, aRange.getCenter());
    }

    void checkOverlap()
    {
        B1DRange aRange(1.0, 3.0);
        B1DRange aRange2(0.0, 1.0);

        CPPUNIT_ASSERT_MESSAGE("range overlapping *includes* upper bound",
                               aRange.overlaps(aRange2));
        CPPUNIT_ASSERT_MESSAGE("range overlapping *includes* upper bound, but only barely",
                               !aRange.overlapsMore(aRange2));

        B1DRange aRange3(0.0, 2.0);
        CPPUNIT_ASSERT_MESSAGE("range overlapping is fully overlapping now",
                               aRange.overlapsMore(aRange3));
    }

    void checkIntersect()
    {
        B1DRange aRange(1.0, 3.0);
        B1DRange aRange2(3.0, 4.0);
        aRange.intersect(aRange2);
        CPPUNIT_ASSERT_MESSAGE("range intersection is yielding empty range!", !aRange.isEmpty());

        B1DRange aRange3(5.0, 6.0);
        aRange.intersect(aRange3);
        CPPUNIT_ASSERT_MESSAGE("range intersection is yielding nonempty range!", aRange.isEmpty());
    }

    CPPUNIT_TEST_SUITE(B1DRangeTest);
    CPPUNIT_TEST(checkIntervalAxioms);
    CPPUNIT_TEST(checkOverlap);
    CPPUNIT_TEST(checkIntersect);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::B1DRangeTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
