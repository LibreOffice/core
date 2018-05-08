/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tools/time.hxx>

namespace tools
{
class TimeTest : public CppUnit::TestFixture
{
public:
    void testTime();

    CPPUNIT_TEST_SUITE(TimeTest);
    CPPUNIT_TEST(testTime);
    CPPUNIT_TEST_SUITE_END();
};

void TimeTest::testTime()
{
    Time aOrigTime(1, 56, 10);
    auto nMS = aOrigTime.GetMSFromTime();

    Time aNewTime(0);
    aNewTime.MakeTimeFromMS(nMS);

    CPPUNIT_ASSERT(bool(aOrigTime == aNewTime));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TimeTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
