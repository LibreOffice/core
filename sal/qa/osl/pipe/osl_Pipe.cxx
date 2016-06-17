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

// include files

#include <sal/types.h>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "unotest/uniquepipename.hxx"
#include <rtl/ustring.hxx>

#include <osl/thread.hxx>

#include <osl/mutex.hxx>

#include <osl/pipe.hxx>
#include <osl/time.h>

#ifdef UNX
#include <unistd.h>
#endif
#include <string.h>

using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

/** print last error of pipe system.
 */
inline void printPipeError( ::osl::Pipe aPipe )
{
    oslPipeError nError = aPipe.getError( );
    printf("#printPipeError# " );
    switch ( nError ) {
    case osl_Pipe_E_None:
        printf("Success!\n" );
        break;
    case osl_Pipe_E_NotFound:
        printf("The returned error is: Not found!\n" );
        break;
    case osl_Pipe_E_AlreadyExists:
        printf("The returned error is: Already exist!\n" );
        break;
    case osl_Pipe_E_NoProtocol:
        printf("The returned error is: No protocol!\n" );
        break;
    case osl_Pipe_E_NetworkReset:
        printf("The returned error is: Network reset!\n" );
        break;
    case osl_Pipe_E_ConnectionAbort:
        printf("The returned error is: Connection aborted!\n" );
        break;
    case osl_Pipe_E_ConnectionReset:
        printf("The returned error is: Connection reset!\n" );
        break;
    case osl_Pipe_E_NoBufferSpace:
        printf("The returned error is: No buffer space!\n" );
        break;
    case osl_Pipe_E_TimedOut:
        printf("The returned error is: Timeout!\n" );
        break;
    case osl_Pipe_E_ConnectionRefused:
        printf("The returned error is: Connection refused!\n" );
        break;
    case osl_Pipe_E_invalidError:
        printf("The returned error is: Invalid error!\n" );
        break;
    default:
        printf("The returned error is: Number %d, Unknown Error\n", nError );
        break;
    }
}

// pipe name and transfer contents

const rtl::OUString aTestPipeName("testpipe2");
const rtl::OUString aTestPipe1("testpipe1");
const rtl::OUString aTestString("Sun Microsystems");

const OString m_pTestString1("Sun Microsystems");
const OString m_pTestString2("test pipe PASS/OK");

// test code start here

namespace osl_Pipe
{

// most return value -1 denote a fail of operation.

#define OSL_PIPE_FAIL   -1

    /** testing the methods:
        inline Pipe();
        inline Pipe(const ::rtl::OUString& strName, oslPipeOptions Options);
        inline Pipe(const ::rtl::OUString& strName, oslPipeOptions Options,const Security & rSecurity);
        inline Pipe(const Pipe& pipe);
        inline Pipe(oslPipe pipe, __sal_NoAcquire noacquire );
        inline Pipe(oslPipe Pipe);
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void setUp( ) override
            {
            }

        void tearDown( ) override
            {
            }

        void ctors_none( )
            {
                ::osl::Pipe aPipe;
                bRes = aPipe.is( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with no parameter, yet no case to test.",
                                        !bRes );
            }

        void ctors_name_option( )
            {
                /// create a named pipe.
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                ::osl::Pipe aAssignPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_OPEN );

                bRes = aPipe.is( ) && aAssignPipe.is( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with name and option.",
                                        bRes );
            }

        void ctors_name_option_security( )
            {
                /// create a security pipe.
                const ::osl::Security rSecurity;
                ::osl::Pipe aSecurityPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSecurity );

                bRes = aSecurityPipe.is( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with name, option and security, the test of security is not implemented yet.",
                                        bRes );
            }

        void ctors_copy( )
            {
                /// create a pipe.
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                /// create a pipe using copy constructor.
                ::osl::Pipe aCopyPipe( aPipe );

                bRes = aCopyPipe.is( ) && aCopyPipe == aPipe;

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test copy constructor.",
                                        bRes );
            }

        /**  tester comment:

        When test the following two constructors, don't know how to test the
        acquire and no acquire action. possible plans:
        1.release one handle and check the other( did not success since the
        other still exist and valid. )
        2. release one handle twice to see getLastError( )(the getLastError
        always returns invalidError(LINUX)).
        */

        void ctors_no_acquire( )
            {
                /// create a pipe.
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                osl_acquirePipe(aPipe.getHandle());
                /// constructs a pipe reference without acquiring the handle.
                ::osl::Pipe aNoAcquirePipe( aPipe.getHandle( ), SAL_NO_ACQUIRE );

                bRes = aNoAcquirePipe.is( );
                ///aPipe.clear( );
                ///bRes1 = aNoAcquirePipe.is( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with no acquire of handle, only validation test, do not know how to test no acquire.",
                                        bRes );
            }

        void ctors_acquire( )
            {
                /// create a base pipe.
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                /// constructs two pipes without acquiring the handle on the base pipe.
                ::osl::Pipe aAcquirePipe( aPipe.getHandle( ) );
                ::osl::Pipe aAcquirePipe1( nullptr );

                bRes = aAcquirePipe.is( );
                bRes1 = aAcquirePipe1.is( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with no acquire of handle.only validation test, do not know how to test no acquire.",
                                        bRes && !bRes1 );
            }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_name_option );
        CPPUNIT_TEST( ctors_name_option_security );
        CPPUNIT_TEST( ctors_copy );
        CPPUNIT_TEST( ctors_no_acquire );
        CPPUNIT_TEST( ctors_acquire );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctors

    /** testing the method:
        inline sal_Bool SAL_CALL is() const;
    */
    class is : public CppUnit::TestFixture
    {
    public:
        void is_001( )
            {
                ::osl::Pipe aPipe;

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test is(), check if the pipe is a valid one.", !aPipe.is( ) );
            }

        void is_002( )
            {
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test is(), a normal pipe creation.", aPipe.is( ) );
            }

        void is_003( )
            {
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test is(), an invalid case.", !aPipe.is( ) );
            }

        void is_004( )
            {
                ::osl::Pipe aPipe( nullptr );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test is(), an invalid constructor.", !aPipe.is( ) );
            }

        CPPUNIT_TEST_SUITE( is );
        CPPUNIT_TEST( is_001 );
        CPPUNIT_TEST( is_002 );
        CPPUNIT_TEST( is_003 );
        CPPUNIT_TEST( is_004 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class is

    /** testing the methods:
        inline sal_Bool create( const ::rtl::OUString & strName,
        oslPipeOptions Options, const Security &rSec );
        nline sal_Bool create( const ::rtl::OUString & strName,
        oslPipeOptions Options = osl_Pipe_OPEN );
    */
    class create : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        /**  tester comment:

        security create only be tested creation, security section is
        untested yet.
        */

        void create_named_security_001( )
            {
                const Security rSec;
                ::osl::Pipe aPipe;
                bRes = aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSec );
                bRes1 = aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSec );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test creation.",
                                        bRes && !bRes1);
            }

        void create_named_security_002( )
            {
                const Security rSec;
                ::osl::Pipe aPipe, aPipe1;
                bRes = aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSec );
                bRes1 = aPipe1.create( test::uniquePipeName(aTestPipeName), osl_Pipe_OPEN, rSec );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test creation and open.",
                                        bRes && bRes1);
            }

        void create_named_001( )
            {
                ::osl::Pipe aPipe;
                bRes = aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                bRes1 = aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test creation.",
                                        bRes && !bRes1);
            }

        void create_named_002( )
            {
                ::osl::Pipe aPipe, aPipe1;
                bRes = aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                bRes1 = aPipe1.create( test::uniquePipeName(aTestPipeName) );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test creation and open.",
                                        bRes && bRes1);
            }

        void create_named_003( )
            {
                ::osl::Pipe aPipe;
                bRes = aPipe.create( test::uniquePipeName(aTestPipeName) );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test default option is open.",
                                        !bRes );
            }

        CPPUNIT_TEST_SUITE( create );
        CPPUNIT_TEST( create_named_security_001 );
        CPPUNIT_TEST( create_named_security_002 );
        CPPUNIT_TEST( create_named_001 );
        CPPUNIT_TEST( create_named_002 );
        CPPUNIT_TEST( create_named_003 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class create

    /** testing the method:
        inline void SAL_CALL clear();
    */
    class clear : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void clear_001( )
            {
                ::osl::Pipe aPipe;
                aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                aPipe.clear( );
                bRes = aPipe.is( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test clear.",
                                        !bRes );
            }

        CPPUNIT_TEST_SUITE( clear );
        CPPUNIT_TEST( clear_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class clear

    /** testing the methods:
        inline Pipe& SAL_CALL operator= (const Pipe& pipe);
        inline Pipe& SAL_CALL operator= (const oslPipe pipe );
    */
    class assign : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void assign_ref( )
            {
                ::osl::Pipe aPipe, aPipe1;
                aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                aPipe1 = aPipe;
                bRes = aPipe1.is( );
                bRes1 = aPipe == aPipe1;
                aPipe.close( );
                aPipe1.close( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test assign with reference.",
                                        bRes && bRes1 );
            }

        void assign_handle( )
            {
                ::osl::Pipe aPipe, aPipe1;
                aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                aPipe1 = aPipe.getHandle( );
                bRes = aPipe1.is( );
                bRes1 = aPipe == aPipe1;
                aPipe.close( );
                aPipe1.close( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test assign with handle.",
                                        bRes && bRes1 );
            }

        CPPUNIT_TEST_SUITE( assign );
        CPPUNIT_TEST( assign_ref );
        CPPUNIT_TEST( assign_handle );
        CPPUNIT_TEST_SUITE_END( );
    }; // class assign

    /** testing the method:
        inline sal_Bool SAL_CALL isValid() const;
        isValid( ) has not been implemented under the following platforms, please refer to osl/pipe.hxx
    */

    /** testing the method:
        inline sal_Bool SAL_CALL operator==( const Pipe& rPipe ) const;
    */
    class isEqual : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void isEqual_001( )
            {
                ::osl::Pipe aPipe;
                aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                bRes  = aPipe == aPipe;
                aPipe.close( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test isEqual(), compare its self.",
                                        bRes );
            }

        void isEqual_002( )
            {
                ::osl::Pipe aPipe, aPipe1, aPipe2;
                aPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );

                aPipe1 = aPipe;
                aPipe2.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );

                bRes  = aPipe == aPipe1;
                bRes1 = aPipe == aPipe2;
                aPipe.close( );
                aPipe1.close( );
                aPipe2.close( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test isEqual(),create one copy instance, and compare.",
                                        bRes && !bRes1 );
            }

        CPPUNIT_TEST_SUITE( isEqual );
        CPPUNIT_TEST( isEqual_001 );
        CPPUNIT_TEST( isEqual_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class isEqual

    /** testing the method:
        inline void SAL_CALL close();
    */
    class close : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void close_001( )
            {
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipe1), osl_Pipe_CREATE );
                aPipe.close( );
                bRes = aPipe.is( );

                aPipe.clear( );
                bRes1 = aPipe.is( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: difference between close and clear.",
                                        bRes && !bRes1);
            }

        void close_002( )
            {
                ::osl::StreamPipe aPipe( test::uniquePipeName(aTestPipe1), osl_Pipe_CREATE );
                aPipe.close( );
                int nRet = aPipe.send( m_pTestString1.getStr(), 3 );

                CPPUNIT_ASSERT_EQUAL_MESSAGE( "#test comment#: use after close.",
                                        nRet, OSL_PIPE_FAIL );
            }

        CPPUNIT_TEST_SUITE( close );
        CPPUNIT_TEST( close_001 );
        CPPUNIT_TEST( close_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class close

    /** testing the method:
        inline oslPipeError SAL_CALL accept(StreamPipe& Connection);
        please refer to StreamPipe::recv
    */

    /** testing the method:
        inline oslPipeError SAL_CALL getError() const;
    */
    class getError : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void getError_001( )
            {
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_OPEN );
                oslPipeError nError = aPipe.getError( );
                printPipeError( aPipe );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: open a non-exist pipe.",
                                        nError != osl_Pipe_E_None );
            }

        void getError_002( )
            {
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                ::osl::Pipe aPipe1( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                oslPipeError nError = aPipe.getError( );
                printPipeError( aPipe );
                aPipe.clear( );
                aPipe1.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: create an already exist pipe.",
                                        nError != osl_Pipe_E_None );
            }

        CPPUNIT_TEST_SUITE( getError );
        CPPUNIT_TEST( getError_001 );
        CPPUNIT_TEST( getError_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getError

    /** testing the method:
        inline oslPipe SAL_CALL getHandle() const;
    */
    class getHandle : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void getHandle_001( )
            {
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_OPEN );
                bRes = aPipe == aPipe.getHandle( );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: one pipe should equal to its handle.",
                                        bRes );
            }

        void getHandle_002( )
            {
                ::osl::Pipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                ::osl::Pipe aPipe1( aPipe.getHandle( ) );
                bRes = aPipe == aPipe1;
                aPipe.clear( );
                aPipe1.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: one pipe derived from another pipe's handle.",
                                        bRes );
            }

        CPPUNIT_TEST_SUITE( getHandle );
        CPPUNIT_TEST( getHandle_001 );
        CPPUNIT_TEST( getHandle_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getHandle

    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::ctors);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::is);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::create);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::clear);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::assign);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::isEqual);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::close);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::getError);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Pipe::getHandle);

} // namespace osl_Pipe

namespace osl_StreamPipe
{

    /** testing the methods:
        inline StreamPipe();
        inline StreamPipe(oslPipe Pipe);
        inline StreamPipe(const StreamPipe& Pipe);
        inline StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options = osl_Pipe_OPEN);
        inline StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options, const Security &rSec );
        inline StreamPipe( oslPipe pipe, __sal_NoAcquire noacquire );
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void ctors_none( )
            {
                // create a pipe.
                ::osl::StreamPipe aStreamPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                // create an unattached pipe.
                ::osl::StreamPipe aStreamPipe1;
                bRes  = aStreamPipe1.is( );

                // assign it and check.
                aStreamPipe1 = aStreamPipe;
                bRes1 = aStreamPipe1.is( );
                aStreamPipe.clear( );
                aStreamPipe1.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with no parameter, before and after assign.",
                                        !bRes && bRes1 );
            }

        void ctors_handle( )
            {
                // create a pipe.
                ::osl::StreamPipe aStreamPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                // create a pipe with last handle.
                ::osl::StreamPipe aStreamPipe1( aStreamPipe.getHandle( ) );
                bRes  = aStreamPipe1.is( ) && aStreamPipe == aStreamPipe1;
                aStreamPipe.clear( );
                aStreamPipe1.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with other's handle.",
                                        bRes );
            }

        void ctors_copy( )
            {
                // create a pipe.
                ::osl::StreamPipe aStreamPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                // create an unattached pipe.
                ::osl::StreamPipe aStreamPipe1( aStreamPipe );
                bRes  = aStreamPipe1.is( ) && aStreamPipe == aStreamPipe1;
                aStreamPipe.clear( );
                aStreamPipe1.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test copy constructor.",
                                        bRes );
            }

        void ctors_name_option( )
            {
                // create a pipe.
                ::osl::StreamPipe aStreamPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                // create an unattached pipe.
                ::osl::StreamPipe aStreamPipe1( test::uniquePipeName(aTestPipeName), osl_Pipe_OPEN );
                bRes  = aStreamPipe1.is( ) && aStreamPipe.is( );
                aStreamPipe.clear( );
                aStreamPipe1.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with name and option.",
                                        bRes );
            }

        void ctors_name_option_security( )
            {
                /// create a security pipe.
                const ::osl::Security rSecurity;
                ::osl::StreamPipe aSecurityPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSecurity );

                bRes = aSecurityPipe.is( );
                aSecurityPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with name, option and security, the test of security is not implemented yet.",
                                        bRes );
            }

        /**  tester comment:

        When test the following constructor, don't know how to test the
        acquire and no acquire action. possible plans:
        1.release one handle and check the other( did not success since the
        other still exist and valid. )
        2. release one handle twice to see getLastError( )(the getLastError
        always returns invalidError(LINUX)).
        */

        void ctors_no_acquire( )
            {
                // create a pipe.
                ::osl::StreamPipe aPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
                osl_acquirePipe(aPipe.getHandle());
                // constructs a pipe reference without acquiring the handle.
                ::osl::StreamPipe aNoAcquirePipe( aPipe.getHandle( ), SAL_NO_ACQUIRE );

                bRes = aNoAcquirePipe.is( );
                aPipe.clear( );

                CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with no acquire of handle, only validation test, do not know how to test no acquire.",
                                        bRes );
            }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_handle );
        CPPUNIT_TEST( ctors_copy );
        CPPUNIT_TEST( ctors_name_option );
        CPPUNIT_TEST( ctors_name_option_security );
        CPPUNIT_TEST( ctors_no_acquire );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctors

    /** testing the methods:
        inline StreamPipe & SAL_CALL operator=(oslPipe Pipe);
        inline StreamPipe& SAL_CALL operator=(const Pipe& pipe);
        mindy: not implementated in osl/pipe.hxx, so remove the cases
    */

    /** wait _nSec seconds.
     */
    void thread_sleep( sal_uInt32 _nSec )
    {
        /// print statement in thread process must use fflush() to force display.
        fflush(stdout);

        TimeValue nTV;
        nTV.Seconds = _nSec;
        nTV.Nanosec = 0;
        osl_waitThread(&nTV);
    }
    // test read/write & send/recv data to pipe

    class Pipe_DataSink_Thread : public Thread
    {
    public:
        sal_Char buf[256];
        Pipe_DataSink_Thread( ) { }

        virtual ~Pipe_DataSink_Thread( )
            {
            }
    protected:
        void SAL_CALL run( ) override
            {
                printf("open pipe\n");
                ::osl::StreamPipe aSenderPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_OPEN );  // test::uniquePipeName(aTestPipeName) is a string = "TestPipe"
                if ( !aSenderPipe.is() )
                {
                    printf("pipe open failed! \n");
                }
                else
                {
                    printf("read\n");
                    sal_Int32 nChars = aSenderPipe.read( buf, m_pTestString1.getLength() + 1 );
                    if ( nChars < 0 )
                    {
                        printf("read failed! \n");
                        return;
                    }
                    buf[sizeof(buf)-1] = '\0';
                    printf("buffer is %s \n", buf);
                    printf("send\n");
                    nChars = aSenderPipe.send( m_pTestString2.getStr(), m_pTestString2.getLength() + 1 );
                    if ( nChars < 0 )
                    {
                        printf("client send failed! \n");
                        return;
                    }
                }
            }

    };

    class Pipe_DataSource_Thread : public Thread
    {
    public:
        sal_Char buf[256];
        ::osl::Pipe aListenPipe;
        ::osl::StreamPipe aConnectionPipe;
        Pipe_DataSource_Thread( )
            {
                printf("create pipe\n");
                aListenPipe.create( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
            }
        virtual ~Pipe_DataSource_Thread( )
            {
                aListenPipe.close();
            }
    protected:
        void SAL_CALL run( ) override
            {
                //create pipe.
                sal_Int32 nChars;
                printf("listen\n");
                if ( !aListenPipe.is() )
                {
                    printf("pipe create failed! \n");
                }
                else
                {
                    //start server and wait for connection.
                    printf("accept\n");
                    if ( osl_Pipe_E_None != aListenPipe.accept( aConnectionPipe ) )
                    {
                        printf("pipe accept failed!");
                        return;
                    }
                    printf("write\n");
                    // write to pipe
                    nChars = aConnectionPipe.write( m_pTestString1.getStr(), m_pTestString1.getLength() + 1 );
                    if ( nChars < 0)
                    {
                        printf("server write failed! \n");
                        return;
                    }
                    printf("recv\n");
                    nChars = aConnectionPipe.recv( buf, 256 );

                    if ( nChars < 0)
                    {
                        printf("server receive failed! \n");
                        return;
                    }
                    buf[sizeof(buf)-1] = '\0';
                    printf("received message is: %s\n", buf );
                }
            }
    };

    /** testing the method: read/write/send/recv and Pipe::accept
     */
    class recv : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void recv_001( )
            {
                //launch threads.
                Pipe_DataSource_Thread myDataSourceThread;
                Pipe_DataSink_Thread myDataSinkThread;
                myDataSourceThread.create( );
                thread_sleep( 1 );
                myDataSinkThread.create( );

                //wait until the thread terminate
                myDataSinkThread.join( );
                myDataSourceThread.join( );

                int nCompare1 = strcmp( myDataSinkThread.buf, m_pTestString1.getStr() );
                int nCompare2 = strcmp( myDataSourceThread.buf, m_pTestString2.getStr() );
                CPPUNIT_ASSERT_MESSAGE( "test send/recv/write/read.", nCompare1 == 0 && nCompare2 == 0 );
            }
        //close pipe when accept
        void recv_002()
            {
                thread_sleep( 1 );

                Pipe_DataSource_Thread myDataSourceThread;
                Pipe_DataSink_Thread myDataSinkThread;
                myDataSourceThread.create( );
                thread_sleep( 1 );
                myDataSourceThread.aListenPipe.close();
                myDataSourceThread.join( );
                //no condition judgement here, if the case could finish executing within 1 or 2 seconds, it passes.
            }

        CPPUNIT_TEST_SUITE( recv );
        CPPUNIT_TEST( recv_001 );
        CPPUNIT_TEST( recv_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class recv

    CPPUNIT_TEST_SUITE_REGISTRATION(osl_StreamPipe::ctors);
//CPPUNIT_TEST_SUITE_REGISTRATION(osl_StreamPipe::assign);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_StreamPipe::recv);

} // namespace osl_StreamPipe

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
