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

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

namespace basegfx
{
class b2dpolyrange : public CppUnit::TestFixture
{
private:
public:
    void check()
    {
        B2DPolyRange aRange;
        aRange.appendElement(B2DRange(0, 0, 1, 1), B2VectorOrientation::Positive);
        aRange.appendElement(B2DRange(2, 2, 3, 3), B2VectorOrientation::Positive);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple poly range - count", sal_uInt32(2), aRange.count());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple poly range - first element", B2DRange(0, 0, 1, 1),
                                     std::get<0>(aRange.getElement(0)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("simple poly range - second element", B2DRange(2, 2, 3, 3),
                                     std::get<0>(aRange.getElement(1)));

        // B2DPolyRange relies on correctly orientated rects
        const B2DRange aRect(0, 0, 1, 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("createPolygonFromRect - correct orientation",
                                     B2VectorOrientation::Positive,
                                     utils::getOrientation(utils::createPolygonFromRect(aRect)));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolyrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2dpolyrange);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
