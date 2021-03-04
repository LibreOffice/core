/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <systools/win32/retry_if_failed.hxx>

namespace test_systools
{
constexpr int ClockRes = 15; // default interval between system clock ticks is ~15 ms on x86

class test_retry_if_failed : public CppUnit::TestFixture
{
public:
    void test_success()
    {
        const DWORD nTicksBefore = GetTickCount();
        HRESULT hr = sal::systools::RetryIfFailed(10, 200, Tester(5));
        const DWORD nTicksAfter = GetTickCount();
        const DWORD nTicksElapsed = nTicksAfter > nTicksBefore ? nTicksAfter - nTicksBefore
                                                               : std::numeric_limits<DWORD>::max()
                                                                     - nTicksBefore + nTicksAfter;
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        // 5 attempts, 4 sleeps by ~200 ms
        CPPUNIT_ASSERT_GREATER(DWORD(800 - ClockRes), nTicksElapsed);
    }

    void test_failure()
    {
        const DWORD nTicksBefore = GetTickCount();
        HRESULT hr = sal::systools::RetryIfFailed(10, 100, Tester(15));
        const DWORD nTicksAfter = GetTickCount();
        const DWORD nTicksElapsed = nTicksAfter > nTicksBefore ? nTicksAfter - nTicksBefore
                                                               : std::numeric_limits<DWORD>::max()
                                                                     - nTicksBefore + nTicksAfter;
        CPPUNIT_ASSERT(FAILED(hr));
        // 1 + 10 attempts, 10 sleeps by ~100 ms
        CPPUNIT_ASSERT_GREATER(DWORD(1000 - ClockRes), nTicksElapsed);
    }

    CPPUNIT_TEST_SUITE(test_retry_if_failed);
    CPPUNIT_TEST(test_success);
    CPPUNIT_TEST(test_failure);
    CPPUNIT_TEST_SUITE_END();

private:
    struct Tester
    {
        Tester(unsigned triesBeforeSuccess)
            : m_nTriesBeforeSuccess(triesBeforeSuccess)
        {
        }

        HRESULT operator()()
        {
            return ++m_nTriesAttempted >= m_nTriesBeforeSuccess ? S_OK : E_FAIL;
        }

        unsigned m_nTriesBeforeSuccess;
        unsigned m_nTriesAttempted = 0;
    };
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_systools::test_retry_if_failed);

} // namespace test_systools

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
