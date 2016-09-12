/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Timers are evil beasts across platforms...
 */

#include <test/bootstrapfixture.hxx>

#include <osl/thread.hxx>
#include <salhelper/thread.hxx>
#include <chrono>

#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/svapp.hxx>
#include "svdata.hxx"
#include "salinst.hxx"

// #define TEST_WATCHDOG

// Enables timer tests that appear to provoke windows under load unduly.
//#define TEST_TIMERPRECISION

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
    virtual void SAL_CALL run() override
    {
        osl::Thread::wait( std::chrono::seconds(mnSeconds) );
        fprintf(stderr, "ERROR: WatchDog timer thread expired, failing the test!\n");
        fflush(stderr);
        CPPUNIT_ASSERT_MESSAGE("watchdog triggered", false);
    }
};

static WatchDog aWatchDog( 120 ); // random high number in secs

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
#ifdef TEST_TIMERPRECISION
    void testAutoTimer();
    void testMultiAutoTimers();
#endif
    void testRecursiveTimer();
    void testSlowTimerCallback();

    CPPUNIT_TEST_SUITE(TimerTest);
    CPPUNIT_TEST(testIdle);
    CPPUNIT_TEST(testIdleMainloop);
#ifdef TEST_WATCHDOG
    CPPUNIT_TEST(testWatchdog);
#endif
    CPPUNIT_TEST(testDurations);
#ifdef TEST_TIMERPRECISION
    CPPUNIT_TEST(testAutoTimer);
    CPPUNIT_TEST(testMultiAutoTimers);
#endif
    CPPUNIT_TEST(testRecursiveTimer);
    CPPUNIT_TEST(testSlowTimerCallback);

    CPPUNIT_TEST_SUITE_END();
};

#ifdef TEST_WATCHDOG
void TimerTest::testWatchdog()
{
    // out-wait the watchdog.
    osl::Thread::wait( std::chrono::seconds(12) );
}
#endif


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
    virtual void Invoke() override
    {
        mrBool = true;
        Application::EndYield();
    }
};

void TimerTest::testIdle()
{
    bool bTriggered = false;
    IdleBool aTest( bTriggered );
    Scheduler::ProcessTaskScheduling();
    CPPUNIT_ASSERT_MESSAGE("idle triggered", bTriggered);
}

// tdf#91727
void TimerTest::testIdleMainloop()
{
#ifndef _WIN32
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
    virtual void Invoke() override
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
    virtual void Invoke() override
    {
        mrCount++;
    }
};

#ifdef TEST_TIMERPRECISION

void TimerTest::testAutoTimer()
{
    const sal_Int32 nDurationMs = 30;
    const sal_Int32 nEventsCount = 5;
    const double exp = (nDurationMs * nEventsCount);

    sal_Int32 nCount = 0;
    std::ostringstream msg;

    // Repeat when we have random latencies.
    // This is expected on non-realtime OSes.
    for (int i = 0; i < 10; ++i)
    {
        const auto start = std::chrono::high_resolution_clock::now();
        nCount = 0;
        AutoTimerCount aCount(nDurationMs, nCount);
        while (nCount < nEventsCount) {
            Application::Yield();
        }

        const auto end = std::chrono::high_resolution_clock::now();
        double dur = std::chrono::duration<double, std::milli>(end - start).count();

        msg << std::setprecision(2) << std::fixed
            << "periodic multi-timer - dur: "
            << dur << " (" << exp << ") ms." << std::endl;

        // +/- 20% should be reasonable enough a margin.
        if (dur >= (exp * 0.8) && dur <= (exp * 1.2))
        {
            // Success.
            return;
        }
    }

    CPPUNIT_FAIL(msg.str().c_str());
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

    sal_Int32 nCountX = 0;
    sal_Int32 nCountY = 0;
    sal_Int32 nCount = 0;
    std::ostringstream msg;

    // Repeat when we have random latencies.
    // This is expected on non-realtime OSes.
    for (int i = 0; i < 10; ++i)
    {
        nCountX = 0;
        nCountY = 0;
        nCount = 0;

        const auto start = std::chrono::high_resolution_clock::now();
        AutoTimerCount aCountX(nDurationMsX, nCountX);
        AutoTimerCount aCountY(nDurationMsY, nCountY);

        AutoTimerCount aCount(nDurationMs, nCount);
        // coverity[loop_top] - Application::Yield allows the timer to fire and toggle nCount
        while (nCount < nEventsCount) {
            Application::Yield();
        }

        const auto end = std::chrono::high_resolution_clock::now();
        double dur = std::chrono::duration<double, std::milli>(end - start).count();

        msg << std::setprecision(2) << std::fixed << "periodic multi-timer - dur: "
            << dur << " (" << exp << ") ms, nCount: " << nCount
            << " (" << nEventsCount << "), nCountX: " << nCountX
            << " (" << expX << "), nCountY: " << nCountY
            << " (" << expY << ")." << std::endl;

        // +/- 20% should be reasonable enough a margin.
        if (dur >= (exp * 0.8) && dur <= (exp * 1.2) &&
            nCountX >= (expX * 0.8) && nCountX <= (expX * 1.2) &&
            nCountY >= (expY * 0.8) && nCountY <= (expY * 1.2))
        {
            // Success.
            return;
        }
    }

    CPPUNIT_FAIL(msg.str().c_str());
}
#endif // TEST_TIMERPRECISION


class YieldTimer : public Timer
{
public:
    explicit YieldTimer( sal_uLong nMS ) : Timer()
    {
        SetTimeout( nMS );
        Start();
    }
    virtual void Invoke() override
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
    virtual void Invoke() override
    {
        osl::Thread::wait( std::chrono::seconds(1) );
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
