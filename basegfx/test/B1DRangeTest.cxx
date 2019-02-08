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
class b1Xrange : public CppUnit::TestFixture
{
public:
    template <class Type> void implCheck()
    {
        // test interval axioms
        // (http://en.wikipedia.org/wiki/Interval_%28mathematics%29)
        Type aRange;
        CPPUNIT_ASSERT_MESSAGE("default ctor - empty range", aRange.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("center - get cop-out value since range is empty",
                               aRange.getCenter() == 0);

        // degenerate interval
        aRange.expand(1);
        CPPUNIT_ASSERT_MESSAGE("degenerate range - still, not empty!", !aRange.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("degenerate range - size of 0", aRange.getRange() == 0);
        CPPUNIT_ASSERT_MESSAGE("same value as degenerate range - is inside range",
                               aRange.isInside(1));
        CPPUNIT_ASSERT_MESSAGE("center - must be the single range value", aRange.getCenter() == 1);

        // proper interval
        aRange.expand(2);
        CPPUNIT_ASSERT_MESSAGE("proper range - size of 1", aRange.getRange() == 1);
        CPPUNIT_ASSERT_MESSAGE("smaller value of range - is inside *closed* range",
                               aRange.isInside(1));
        CPPUNIT_ASSERT_MESSAGE("larger value of range - is inside *closed* range",
                               aRange.isInside(2));

        // center for proper interval that works for ints, too
        aRange.expand(3);
        CPPUNIT_ASSERT_MESSAGE("center - must be half of the range", aRange.getCenter() == 2);

        // check overlap
        Type aRange2(0, 1);
        CPPUNIT_ASSERT_MESSAGE("range overlapping *includes* upper bound",
                               aRange.overlaps(aRange2));
        CPPUNIT_ASSERT_MESSAGE("range overlapping *includes* upper bound, but only barely",
                               !aRange.overlapsMore(aRange2));

        Type aRange3(0, 2);
        CPPUNIT_ASSERT_MESSAGE("range overlapping is fully overlapping now",
                               aRange.overlapsMore(aRange3));

        // check intersect
        Type aRange4(3, 4);
        aRange.intersect(aRange4);
        CPPUNIT_ASSERT_MESSAGE("range intersection is yielding empty range!", !aRange.isEmpty());

        Type aRange5(5, 6);
        aRange.intersect(aRange5);
        CPPUNIT_ASSERT_MESSAGE("range intersection is yielding nonempty range!", aRange.isEmpty());
    }

    void check() { implCheck<B1DRange>(); }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b1Xrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
}; // class b1Xrange

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b1Xrange);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
