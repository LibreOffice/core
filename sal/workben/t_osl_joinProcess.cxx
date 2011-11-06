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

#include <cppunit/simpleheader.hxx>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#include <unistd.h>
#include <signal.h>

#ifdef WNT
    const rtl::OUString IMAGE_NAME = rtl::OUString::createFromAscii("ojpx.exe");
#else
    const rtl::OUString IMAGE_NAME = rtl::OUString::createFromAscii("ojpx");
#endif

const rtl::OUString CWD        = rtl::OUString::createFromAscii(".");

//------------------------------
//
//------------------------------

class Test_osl_Process : public CppUnit::TestFixture
{
public:

    /*-------------------------------------
        Start a process and join with this
        process specify a timeout so that
        osl_joinProcessWithTimeout returns
        osl_Process_E_TimedOut
     -------------------------------------*/

    void test_osl_joinProcessWithTimeout_timeout_failure()
    {
        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            IMAGE_NAME.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            osl_getCurrentSecurity(),
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        TimeValue timeout;
        timeout.Seconds = 1;
        timeout.Nanosec = 0;

        osl_error = osl_joinProcessWithTimeout(process, &timeout);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcessWithTimeout returned without timeout failure",
            osl_Process_E_TimedOut == osl_error
        );

        osl_error = osl_terminateProcess(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_terminateProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_freeProcessHandle(process);
    }

    /*-------------------------------------
        Start a process and join with this
        process specify a timeout so that
        osl_joinProcessWithTimeout returns
        osl_Process_E_None
     -------------------------------------*/

    void test_osl_joinProcessWithTimeout_without_timeout_failure()
    {
        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            IMAGE_NAME.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            osl_getCurrentSecurity(),
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        TimeValue timeout;
        timeout.Seconds = 10;
        timeout.Nanosec = 0;

        osl_error = osl_joinProcessWithTimeout(process, &timeout);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcessWithTimeout returned with failure",
            osl_Process_E_None == osl_error
        );

        osl_freeProcessHandle(process);
    }

     /*-------------------------------------
        Start a process and join with this
        process specify an infinite timeout
     -------------------------------------*/

    void test_osl_joinProcessWithTimeout_infinite()
    {
        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            IMAGE_NAME.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            osl_getCurrentSecurity(),
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_error = osl_joinProcessWithTimeout(process, NULL);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcessWithTimeout returned with failure",
            osl_Process_E_None == osl_error
        );

        osl_freeProcessHandle(process);
    }

     /*-------------------------------------
        Start a process and join with this
        process using osl_joinProcess
     -------------------------------------*/

     void test_osl_joinProcess()
    {
        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            IMAGE_NAME.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            osl_getCurrentSecurity(),
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_error = osl_joinProcess(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None == osl_error
        );

        osl_freeProcessHandle(process);
    }

    CPPUNIT_TEST_SUITE(Test_osl_Process);
    CPPUNIT_TEST(test_osl_joinProcessWithTimeout_timeout_failure);
    CPPUNIT_TEST(test_osl_joinProcessWithTimeout_without_timeout_failure);
    CPPUNIT_TEST(test_osl_joinProcessWithTimeout_infinite);
    CPPUNIT_TEST(test_osl_joinProcess);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test_osl_Process, "Test_osl_Process");

NOADDITIONAL;

