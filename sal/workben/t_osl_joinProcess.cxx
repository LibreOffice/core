/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
