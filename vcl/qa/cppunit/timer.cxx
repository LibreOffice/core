/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Timers are evil beasties across platforms ...
 */

#include <test/bootstrapfixture.hxx>

#include <osl/thread.hxx>
#include <salhelper/thread.hxx>

#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/svapp.hxx>
#include "svdata.hxx"
#include "salinst.hxx"

// #define TEST_WATCHDOG
// Comment if UT fails randomly.
#define TEST_TIMERPRECISION

/// Avoid our timer tests just wedging the build if they fail.
class WatchDog : public osl::Thread
{
    sal_Int32 mnSeconds;
public:
    explicit WatchDog(sal_Int32 nSeconds) :
        Thread(),
        mnSeconds( nSeconds )
    {
        create();
    }
    virtual void SAL_CALL run() SAL_OVERRIDE
    {
        TimeValue aWait;
        aWait.Seconds = mnSeconds;
        aWait.Nanosec = 1000000; // +1ms
        osl::Thread::wait( aWait );
        CPPUNIT_ASSERT_MESSAGE("watchdog triggered", false);
    }
};

static WatchDog aWatchDog( 10 /* 10 secs should be enough */);

class TimerTest : public test::BootstrapFixture
{
public:
    TimerTest() : BootstrapFixture(true, false) {}

    void testIdleMainloop();
    void testIdle();
#ifdef TEST_WATCHDOG
    void testWatchdog();
#endif
    void testDurations();
    void testAutoTimer();
    void testMultiAutoTimers();
    void testRecursiveTimer();
    void testSlowTimerCallback();

    CPPUNIT_TEST_SUITE(TimerTest);
    CPPUNIT_TEST(testIdle);
    CPPUNIT_TEST(testIdleMainloop);
#ifdef TEST_WATCHDOG
    CPPUNIT_TEST(testWatchdog);
#endif
    CPPUNIT_TEST(testDurations);
    CPPUNIT_TEST(testAutoTimer);
    CPPUNIT_TEST(testMultiAutoTimers);
    CPPUNIT_TEST(testRecursiveTimer);
    CPPUNIT_TEST(testSlowTimerCallback);

    CPPUNIT_TEST_SUITE_END();
};

#ifdef TEST_WATCHDOG
void TimerTest::testWatchdog()
{
    // out-wait the watchdog.
    TimeValue aWait;
    aWait.Seconds = 12;
    aWait.Nanosec = 0;
    osl::Thread::wait( aWait );
}
#endif

// --------------------------------------------------------------------

class IdleBool : public Idle
{
    bool &mrBool;
public:
    explicit IdleBool( bool &rBool ) :
        Idle(), mrBool( rBool )
    {
        SetPriority( SchedulerPriority::LOWEST );
        Start();
        mrBool = false;
    }
    virtual void Invoke() SAL_OVERRIDE
    {
        mrBool = true;
        Application::EndYield();
    }
};

void TimerTest::testIdle()
{
    bool bTriggered = false;
    IdleBool aTest( bTriggered );
    Scheduler::ProcessTaskScheduling(false);
    CPPUNIT_ASSERT_MESSAGE("idle triggered", bTriggered);
}

// tdf#91727
void TimerTest::testIdleMainloop()
{
    // For whatever reason, ImplGetSVData()
    // doesn't link to the test binary on
    // Windows.
#ifndef WNT
    bool bTriggered = false;
    IdleBool aTest( bTriggered );
    // coverity[loop_top] - Application::Yield allows the timer to fire and toggle bDone
    while (!bTriggered)
    {
        ImplSVData* pSVData = ImplGetSVData();

        // can't test this via Application::Yield since this
        // also processes all tasks directly via the scheduler.
        pSVData->maAppData.mnDispatchLevel++;
        pSVData->mpDefInst->DoYield(true, false, 0);
        pSVData->maAppData.mnDispatchLevel--;
    }
    CPPUNIT_ASSERT_MESSAGE("mainloop idle triggered", bTriggered);
#endif
}

// --------------------------------------------------------------------

class TimerBool : public Timer
{
    bool &mrBool;
public:
    TimerBool( sal_uLong nMS, bool &rBool ) :
        Timer(), mrBool( rBool )
    {
        SetTimeout( nMS );
        Start();
        mrBool = false;
    }
    virtual void Invoke() SAL_OVERRIDE
    {
        mrBool = true;
        Application::EndYield();
    }
};

void TimerTest::testDurations()
{
    static const sal_uLong aDurations[] = { 0, 1, 500, 1000 };
    for (size_t i = 0; i < SAL_N_ELEMENTS( aDurations ); i++)
    {
        bool bDone = false;
        TimerBool aTimer( aDurations[i], bDone );
        // coverity[loop_top] - Application::Yield allows the timer to fire and toggle bDone
        while( !bDone )
        {
            Application::Yield();
        }
    }
}

// --------------------------------------------------------------------

class AutoTimerCount : public AutoTimer
{
    sal_Int32 &mrCount;
public:
    AutoTimerCount( sal_uLong nMS, sal_Int32 &rCount ) :
        AutoTimer(), mrCount( rCount )
    {
        SetTimeout( nMS );
        Start();
        mrCount = 0;
    }
    virtual void Invoke() SAL_OVERRIDE
    {
        mrCount++;
    }
};

void TimerTest::testAutoTimer()
{
    const sal_Int32 nDurationMs = 30;
    const sal_Int32 nEventsCount = 5;
    const double exp = (nDurationMs * nEventsCount);

    sal_Int32 nCount = 0;
    double dur = 0;

    // Repeat when we have random latencies.
    // This is expected on non-realtime OSes.
    for (int i = 0; i < 6; ++i)
    {
        const auto start = std::chrono::high_resolution_clock::now();
        nCount = 0;
        AutoTimerCount aCount(nDurationMs, nCount);
        while (nCount < nEventsCount) {
            Application::Yield();
        }

        const auto end = std::chrono::high_resolution_clock::now();
        dur = std::chrono::duration<double, std::milli>(end - start).count();
        if (dur >= (exp * 0.9) && dur <= (exp * 1.1))
        {
            break;
        }
    }

#ifdef TEST_TIMERPRECISION
    // +/- 10% should be reasonable enough a margin.
    // Increase if fails randomly under load.
    CPPUNIT_ASSERT_MESSAGE("periodic timer", dur >= (exp * 0.9) && dur <= (exp * 1.1));
#endif
}

void TimerTest::testMultiAutoTimers()
{
    // The behavior of the timers change drastically
    // when multiple timers are present.
    // The worst, in my tests, is when two
    // timers with 1ms period exist with a
    // third of much longer period.

    const sal_Int32 nDurationMsX = 5;
    const sal_Int32 nDurationMsY = 10;
    const sal_Int32 nDurationMs = 40;
    const sal_Int32 nEventsCount = 5;
    const double exp = (nDurationMs * nEventsCount);
    const double expX = (exp / nDurationMsX);
    const double expY = (exp / nDurationMsY);

    double dur = 0;
    sal_Int32 nCountX = 0;
    sal_Int32 nCountY = 0;
    sal_Int32 nCount = 0;
    char msg[200];

    // Repeat when we have random latencies.
    // This is expected on non-realtime OSes.
    for (int i = 0; i < 6; ++i)
    {
        nCountX = 0;
        nCountY = 0;
        nCount = 0;

        const auto start = std::chrono::high_resolution_clock::now();
        AutoTimerCount aCountX(nDurationMsX, nCountX);
        AutoTimerCount aCountY(nDurationMsY, nCountY);

        AutoTimerCount aCount(nDurationMs, nCount);
        while (nCount < nEventsCount) {
            Application::Yield();
        }

        const auto end = std::chrono::high_resolution_clock::now();
        dur = std::chrono::duration<double, std::milli>(end - start).count();

        sprintf(msg, "periodic multi-timer - dur: %.2f (%.2f) ms, nCount: %d (%d), "
                     "nCountX: %d (%.2f), nCountY: %d (%.2f)\n",
                dur, exp, nCount, nEventsCount, nCountX, expX, nCountY, expY);

        // +/- 10% should be reasonable enough a margin.
        // Increase if fails randomly under load.
        if (dur >= (exp * 0.9) && dur <= (exp * 1.1) &&
            nCountX >= (expX * 0.9) && nCountX <= (expX * 1.1) &&
            nCountY >= (expY * 0.9) && nCountY <= (expY * 1.1))
        {
            // Success.
            return;
        }
    }

#ifdef TEST_TIMERPRECISION
    CPPUNIT_FAIL(msg);
#endif
}

// --------------------------------------------------------------------

class YieldTimer : public Timer
{
public:
    explicit YieldTimer( sal_uLong nMS ) : Timer()
    {
        SetTimeout( nMS );
        Start();
    }
    virtual void Invoke() SAL_OVERRIDE
    {
        for (int i = 0; i < 100; i++)
            Application::Yield();
    }
};

void TimerTest::testRecursiveTimer()
{
    sal_Int32 nCount = 0;
    YieldTimer aCount(5);
    AutoTimerCount aCountUp( 3, nCount );
    // coverity[loop_top] - Application::Yield allows the timer to fire and increment nCount
    while (nCount < 20)
        Application::Yield();
}

// --------------------------------------------------------------------

class SlowCallbackTimer : public Timer
{
    bool &mbSlow;
public:
    SlowCallbackTimer( sal_uLong nMS, bool &bBeenSlow ) :
        Timer(), mbSlow( bBeenSlow )
    {
        SetTimeout( nMS );
        Start();
        mbSlow = false;
    }
    virtual void Invoke() SAL_OVERRIDE
    {
        TimeValue aWait;
        aWait.Seconds = 1;
        aWait.Nanosec = 0;
        osl::Thread::wait( aWait );
        mbSlow = true;
    }
};

void TimerTest::testSlowTimerCallback()
{
    bool bBeenSlow = false;
    sal_Int32 nCount = 0;
    AutoTimerCount aHighFreq(1, nCount);
    SlowCallbackTimer aSlow(250, bBeenSlow);
    // coverity[loop_top] - Application::Yield allows the timer to fire and toggle bBeenSlow
    while (!bBeenSlow)
        Application::Yield();
    // coverity[loop_top] - Application::Yield allows the timer to fire and increment nCount
    while (nCount < 200)
        Application::Yield();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TimerTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
