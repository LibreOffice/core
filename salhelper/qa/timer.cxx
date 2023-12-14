/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <condition_variable>
#include <mutex>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/ref.hxx>
#include <salhelper/timer.hxx>

namespace
{
class TestTimer : public salhelper::Timer
{
public:
    TestTimer()
        : Timer(salhelper::TTimeValue(0, 1))
    {
    }

    void SAL_CALL onShot() override
    {
        {
            std::scoped_lock l(mutex);
            done = true;
        }
        cond.notify_all();
    }

    std::mutex mutex;
    std::condition_variable cond;
    bool done = false;
};

class TimerTest : public CppUnit::TestFixture
{
public:
    void test()
    {
        rtl::Reference<TestTimer> t(new TestTimer);
        t->start();
        {
            std::unique_lock l(t->mutex);
            t->cond.wait(l, [t] { return t->done; });
        }
        CPPUNIT_ASSERT(t->isExpired());
    }

    CPPUNIT_TEST_SUITE(TimerTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TimerTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
