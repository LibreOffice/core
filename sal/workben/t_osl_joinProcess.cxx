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

#include <sal/types.h>
#include <cppunit/simpleheader.hxx>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#include <unistd.h>
#include <signal.h>

#ifdef WNT
    const rtl::OUString IMAGE_NAME("ojpx.exe");
#else
    const rtl::OUString IMAGE_NAME("ojpx");
#endif

const rtl::OUString CWD(".");

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
        oslSecurity security = osl_getCurrentSecurity();
        oslProcessError osl_error = osl_executeProcess(
            IMAGE_NAME.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            security,
            CWD.pData,
            NULL,
            0,
            &process);
        osl_freeSecurityHandle(security);

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
        oslSecurity security = osl_getCurrentSecurity();
        oslProcessError osl_error = osl_executeProcess(
            IMAGE_NAME.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            security,
            CWD.pData,
            NULL,
            0,
            &process);
        osl_freeSecurityHandle(security);

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
        oslSecurity security = osl_getCurrentSecurity();
        oslProcessError osl_error = osl_executeProcess(
            IMAGE_NAME.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            security,
            CWD.pData,
            NULL,
            0,
            &process);
        osl_freeSecurityHandle(security);

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
        oslSecurity security = osl_getCurrentSecurity();
        oslProcessError osl_error = osl_executeProcess(
            IMAGE_NAME.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            security,
            CWD.pData,
            NULL,
            0,
            &process);
        osl_freeSecurityHandle(security);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
