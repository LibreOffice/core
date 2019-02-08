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
#include <basegfx/range/b2irange.hxx>
#include <basegfx/utils/rectcliptools.hxx>

namespace basegfx
{
class b2Xrange : public CppUnit::TestFixture
{
public:
    template <class Type> void implCheck()
    {
        // cohen sutherland clipping
        Type aRange(0, 0, 10, 10);

        CPPUNIT_ASSERT_MESSAGE("(0,0) is outside range!",
                               utils::getCohenSutherlandClipFlags(B2IPoint(0, 0), aRange) == 0);
        CPPUNIT_ASSERT_MESSAGE("(-1,-1) is inside range!",
                               utils::getCohenSutherlandClipFlags(B2IPoint(-1, -1), aRange)
                                   == (utils::RectClipFlags::LEFT | utils::RectClipFlags::TOP));
        CPPUNIT_ASSERT_MESSAGE("(10,10) is outside range!",
                               utils::getCohenSutherlandClipFlags(B2IPoint(10, 10), aRange) == 0);
        CPPUNIT_ASSERT_MESSAGE("(11,11) is inside range!",
                               utils::getCohenSutherlandClipFlags(B2IPoint(11, 11), aRange)
                                   == (utils::RectClipFlags::RIGHT | utils::RectClipFlags::BOTTOM));
    }

    void check()
    {
        implCheck<B2DRange>();
        implCheck<B2IRange>();
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2Xrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
}; // class b2Xrange

} // namespace basegfx

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::b2Xrange);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
