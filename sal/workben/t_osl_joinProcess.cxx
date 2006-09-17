/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: t_osl_joinProcess.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:09:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

