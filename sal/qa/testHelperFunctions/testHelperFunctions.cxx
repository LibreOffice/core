/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"
// This is a test of helperfunctions

#include <osl/time.h>
#include <osl/thread.hxx>

#include "stringhelper.hxx"

#include "gtest/gtest.h"

// void isJaBloed()
// {
//     printf("Ist ja echt bloed.\n");
// }

inline sal_Int64 t_abs64(sal_Int64 _nValue)
{
    // std::abs() seems to have some ambiguity problems (so-texas)
    // return abs(_nValue);
    printf("t_abs64(%ld)\n", _nValue);
    // ASSERT_TRUE(_nValue < 2147483647);

    if (_nValue < 0)
    {
        _nValue = -_nValue;
    }
    return _nValue;
}

void printf64(sal_Int64 n)
{
    if (n < 0)
    {
        // negativ
        printf("-");
        n = -n;
    }
    if (n > 2147483647)
    {
        sal_Int64 n64 = n >> 32;
        sal_uInt32 n32 = n64 & 0xffffffff;
        printf("0x%.8x ", n32);
        n32 = n & 0xffffffff;
        printf("%.8x (64bit)", n32);
    }
    else
    {
        sal_uInt32 n32 = n & 0xffffffff;
        printf("0x%.8x (32bit) ", n32);
    }
    printf("\n");
}

// -----------------------------------------------------------------------------
namespace testOfHelperFunctions
{
    class test_t_abs64 : public ::testing::Test
    {
    };

    TEST_F(test_t_abs64, test0)
    {
        // this values has an overrun!
        sal_Int32 n32 = 2147483648;
        printf("n32 should be -2^31 is: %d\n", n32);
        ASSERT_TRUE(n32 == -2147483648 ) << "n32!=2147483648";
    }


    TEST_F(test_t_abs64,test1_0)
    {
        sal_Int64 n;
        n = 1073741824;
        n <<= 9;
        printf("Value of n is ");
        printf64(n);
        ASSERT_TRUE(t_abs64(n) > 0) << "n=2^30 << 9";
    }

    TEST_F(test_t_abs64, test1)
    {
        sal_Int64 n;
        n = 2147483648 << 8;
        printf("Value of n is ");
        printf64(n);
        ASSERT_TRUE(t_abs64(n) > 0) << "n=2^31 << 8";
    }
    TEST_F(test_t_abs64, test1_1)
    {
        sal_Int64 n;
        n = sal_Int64(2147483648) << 8;
        printf("Value of n is ");
        printf64(n);
        ASSERT_TRUE(t_abs64(n) > 0) << "n=2^31 << 8";
    }

    TEST_F(test_t_abs64, test2)
    {
        sal_Int64 n;
        n = 2147483648 << 1;
        printf("Value of n is ");
        printf64(n);

        ASSERT_TRUE(n != 0) << "(2147483648 << 1) is != 0";

        sal_Int64 n2 = 2147483648 * 2;
        ASSERT_TRUE(n2 != 0) << "2147483648 * 2 is != 0";

        sal_Int64 n3 = 4294967296LL;
        ASSERT_TRUE(n3 != 0) << "4294967296 is != 0";

        ASSERT_TRUE(n == n2 && n == n3) << "n=2^31 << 1, n2 = 2^31 * 2, n3 = 2^32, all should equal!";
    }


    TEST_F(test_t_abs64, test3)
    {
        sal_Int64 n = 0;
        ASSERT_TRUE(t_abs64(n) == 0) << "n=0";

        n = 1;
        ASSERT_TRUE(t_abs64(n) > 0) << "n=1";

        n = 2147483647;
        ASSERT_TRUE(t_abs64(n) > 0) << "n=2^31 - 1";

        n = 2147483648;
        ASSERT_TRUE(t_abs64(n) > 0) << "n=2^31";
    }

    TEST_F(test_t_abs64, test4)
    {
        sal_Int64 n = 0;
        n = -1;
        printf("Value of n is -1 : ");
        printf64(n);
        ASSERT_TRUE(t_abs64(n) > 0) << "n=-1";

        n = -2147483648;
        printf("Value of n is -2^31 : ");
        printf64(n);
        ASSERT_TRUE(t_abs64(n) > 0) << "n=-2^31";

        n = -8589934592LL;
        printf("Value of n is -2^33 : ");
        printf64(n);
        ASSERT_TRUE(t_abs64(n) > 0) << "n=-2^33";
    }


// -----------------------------------------------------------------------------
    class test_printf : public ::testing::Test
    {
    };

    TEST_F(test_printf, printf_001)
    {
        printf("This is only a test of some helper functions\n");
        sal_Int32 nValue = 12345;
        printf("a value %d (should be 12345)\n", nValue);

        rtl::OString sValue("foo bar");
        printf("a String '%s' (should be 'foo bar')\n", sValue.getStr());

        rtl::OUString suValue(rtl::OUString::createFromAscii("a unicode string"));
        sValue <<= suValue;
        printf("a String '%s'\n", sValue.getStr());
    }


    class StopWatch
    {
    protected:
        TimeValue m_aStartTime;
        TimeValue m_aEndTime;
        bool m_bStarted;
    public:
        StopWatch()
                :m_bStarted(false)
            {
            }

        void start()
            {
                m_bStarted = true;
                osl_getSystemTime(&m_aStartTime);
            }
        void stop()
            {
                osl_getSystemTime(&m_aEndTime);
                OSL_ENSURE(m_bStarted, "Not Started.");
                m_bStarted = false;
            }
        rtl::OString makeTwoDigits(rtl::OString const& _sStr)
            {
                rtl::OString sBack;
                if (_sStr.getLength() == 0)
                {
                    sBack = "00";
                }
                else
                {
                    if (_sStr.getLength() == 1)
                    {
                        sBack = "0" + _sStr;
                    }
                    else
                    {
                        sBack = _sStr;
                    }
                }
                return sBack;
            }
        rtl::OString makeThreeDigits(rtl::OString const& _sStr)
            {
                rtl::OString sBack;
                if (_sStr.getLength() == 0)
                {
                    sBack = "000";
                }
                else
                {
                    if (_sStr.getLength() == 1)
                    {
                        sBack = "00" + _sStr;
                    }
                    else
                    {
                        if (_sStr.getLength() == 2)
                        {
                            sBack = "0" + _sStr;
                        }
                        else
                        {
                            sBack = _sStr;
                        }
                    }
                }
                return sBack;
            }

        void  showTime(const rtl::OString & aWhatStr)
            {
                OSL_ENSURE(!m_bStarted, "Not Stopped.");

                sal_Int32 nSeconds = m_aEndTime.Seconds - m_aStartTime.Seconds;
                sal_Int32 nNanoSec = sal_Int32(m_aEndTime.Nanosec) - sal_Int32(m_aStartTime.Nanosec);
                // printf("Seconds: %d Nanosec: %d ", nSeconds, nNanoSec);
                if (nNanoSec < 0)
                {
                    nNanoSec = 1000000000 + nNanoSec;
                    nSeconds--;
                    // printf(" NEW Seconds: %d Nanosec: %d\n", nSeconds, nNanoSec);
                }

                rtl::OString aStr = "Time for ";
                aStr += aWhatStr;
                aStr += " ";
                aStr += makeTwoDigits(rtl::OString::valueOf(nSeconds / 3600));
                aStr += ":";
                aStr += makeTwoDigits(rtl::OString::valueOf((nSeconds % 3600) / 60));
                aStr += ":";
                aStr += makeTwoDigits(rtl::OString::valueOf((nSeconds % 60)));
                aStr += ":";
                aStr += makeThreeDigits(rtl::OString::valueOf((nNanoSec % 1000000000) / 1000000));
                aStr += ":";
                aStr += makeThreeDigits(rtl::OString::valueOf((nNanoSec % 1000000) / 1000));
                aStr += ":";
                aStr += makeThreeDigits(rtl::OString::valueOf((nNanoSec % 1000)));

                printf("%s\n", aStr.getStr());
                // cout << aStr.getStr() << endl;
            }

    };

static sal_Bool isEqualTimeValue ( const TimeValue* time1,  const TimeValue* time2)
{
    if( time1->Seconds == time2->Seconds &&
        time1->Nanosec == time2->Nanosec)
        return sal_True;
    else
        return sal_False;
}

static sal_Bool isGreaterTimeValue(  const TimeValue* time1,  const TimeValue* time2)
{
    sal_Bool retval= sal_False;
    if ( time1->Seconds > time2->Seconds)
        retval= sal_True;
    else if ( time1->Seconds == time2->Seconds)
    {
        if( time1->Nanosec > time2->Nanosec)
            retval= sal_True;
    }
    return retval;
}

static sal_Bool isGreaterEqualTimeValue( const TimeValue* time1, const TimeValue* time2)
{
    if( isEqualTimeValue( time1, time2) )
        return sal_True;
    else if( isGreaterTimeValue( time1, time2))
        return sal_True;
    else
        return sal_False;
}

bool isBTimeGreaterATime(TimeValue const& A, TimeValue const& B)
{
    if (B.Seconds > A.Seconds) return true;
    if (B.Nanosec > A.Nanosec) return true;

    // lower or equal
    return false;
}
    // -----------------------------------------------------------------------------


    class test_TimeValues : public ::testing::Test
    {
    };

TEST_F(test_TimeValues, t_time1)
{
    StopWatch aWatch;
    aWatch.start();
    TimeValue aTimeValue={3,0};
    osl::Thread::wait(aTimeValue);
    aWatch.stop();
    aWatch.showTime("Wait for 3 seconds");
}

TEST_F(test_TimeValues, t_time2)
{
    printf("Wait repeats 20 times.\n");
    int i=0;
    while(i++<20)
    {
        StopWatch aWatch;
        aWatch.start();
        TimeValue aTimeValue={0,1000 * 1000 * 500};
        osl::Thread::wait(aTimeValue);
        aWatch.stop();
        aWatch.showTime("wait for 500msec");
    }
}

TEST_F(test_TimeValues, t_time3)
{
    printf("Wait repeats 100 times.\n");
    int i=0;
    while(i++<20)
    {
        StopWatch aWatch;
        aWatch.start();
        TimeValue aTimeValue={0,1000*1000*100};
        osl::Thread::wait(aTimeValue);
        aWatch.stop();
        aWatch.showTime("wait for 100msec");
    }
}

    // void demoTimeValue()
    // {
    //     TimeValue aStartTime, aEndTime;
    //     osl_getSystemTime(&aStartTime);
    //     // testSession(xORB, false);
    //     osl_getSystemTime(&aEndTime);
    //
    //     sal_Int32 nSeconds = aEndTime.Seconds - aStartTime.Seconds;
    //     sal_Int32 nNanoSec = aEndTime.Nanosec - aStartTime.Nanosec;
    //     if (nNanoSec < 0)
    //     {
    //         nNanoSec = 1000000000 - nNanoSec;
    //         nSeconds++;
    //     }
    //
    //     // cout << "Time: " << nSeconds << ". " << nNanoSec << endl;
    // }


} // namespace testOfHelperFunctions

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
