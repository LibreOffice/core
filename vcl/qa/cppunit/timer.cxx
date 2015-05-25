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

    void testIdle();
#ifdef TEST_WATCHDOG
    void testWatchdog();
#endif
    void testDurations();
    void testAutoTimer();
    void testRecursiveTimer();
    void testSlowTimerCallback();

    CPPUNIT_TEST_SUITE(TimerTest);
    CPPUNIT_TEST(testIdle);
#ifdef TEST_WATCHDOG
    CPPUNIT_TEST(testWatchdog);
#endif
    CPPUNIT_TEST(testDurations);
    CPPUNIT_TEST(testAutoTimer);
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
    CPPUNIT_ASSERT_MESSAGE("watchdog triggered", bTriggered);
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
    sal_Int32 nCount = 0;
    AutoTimerCount aCount(1, nCount);
    while (nCount < 100) {
        Application::Yield();
    }
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
