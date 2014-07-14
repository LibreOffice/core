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

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------

#ifdef WNT
#include <Windows.h>
#endif

#include <sal/types.h>
#include <rtl/ustring.hxx>

#ifndef _OSL_THREAD_HXX
#include <osl/thread.hxx>
#endif

#ifndef _OSL_MUTEX_HXX
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_MUTEX_HXX
#include <osl/pipe.hxx>
#endif
#include <osl/time.h>
#include <osl/process.h>

#ifdef UNX
#include <unistd.h>
#endif
#include <string.h>

#include "gtest/gtest.h"

using namespace osl;
using namespace rtl;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------


/** helper function for unique pipe names
 */
OUString uniquePipeName(OUString const & name) {
    oslProcessInfo info;
    info.Size = sizeof info;
    EXPECT_TRUE( osl_Process_E_None == osl_getProcessInfo(0, osl_Process_IDENTIFIER, &info) );
    return name + OUString::valueOf((sal_Int32)info.Ident);
}

/** print Boolean value.
 */
inline void printBool( sal_Bool bOk )
{
    printf("#printBool# " );
    ( sal_True == bOk ) ? printf("YES!\n" ): printf("NO!\n" );
}

/** print a UNI_CODE String.
 */
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf("#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", aString.getStr( ) );
}

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



//------------------------------------------------------------------------
// pipe name and transfer contents
//------------------------------------------------------------------------
const rtl::OUString aTestPipeName = rtl::OUString::createFromAscii( "testpipe2" );
const rtl::OUString aTestPipe1 = rtl::OUString::createFromAscii( "testpipe1" );
const rtl::OUString aTestString = rtl::OUString::createFromAscii( "Apache Software Foundation" );

const OString m_pTestString1("Apache Software Foundation");
const OString m_pTestString2("test pipe PASS/OK");

//------------------------------------------------------------------------
// test code start here
//------------------------------------------------------------------------

namespace osl_Pipe
{

//------------------------------------------------------------------------
// most return value -1 denote a fail of operation.
//------------------------------------------------------------------------
#define OSL_PIPE_FAIL   -1

    /** testing the methods:
        inline Pipe();
        inline Pipe(const ::rtl::OUString& strName, oslPipeOptions Options);
        inline Pipe(const ::rtl::OUString& strName, oslPipeOptions Options,const Security & rSecurity);
        inline Pipe(const Pipe& pipe);
        inline Pipe(oslPipe pipe, __sal_NoAcquire noacquire );
        inline Pipe(oslPipe Pipe);
    */
    TEST(Sal_Test_Pipe, ctors_none) {
        ::osl::Pipe aPipe;
        sal_Bool bRes = aPipe.is( );

        // #test comment#: test constructor with no parameter, yet no case to test.
        ASSERT_TRUE( !bRes );
    }

   TEST(Sal_Test_Pipe, ctors_name_option) {
       /// create a named pipe.
       ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
       ::osl::Pipe aAssignPipe( uniquePipeName(aTestPipeName), osl_Pipe_OPEN );

       sal_Bool bRes = aPipe.is( ) && aAssignPipe.is( );

       // #test comment#: test constructor with name and option.
       ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_Pipe, ctors_name_option_security) {
        /// create a security pipe.
        const ::osl::Security rSecurity;
        ::osl::Pipe aSecurityPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSecurity );

        sal_Bool bRes = aSecurityPipe.is( );

        // #test comment#: test constructor with name, option and security, the test of security is not implemented yet.
        ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_Pipe, ctors_copy) {
        /// create a pipe.
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        /// create a pipe using copy constructor.
        ::osl::Pipe aCopyPipe( aPipe );

        sal_Bool bRes = aCopyPipe.is( ) && aCopyPipe == aPipe;

        // #test comment#: test copy constructor.
        ASSERT_TRUE( bRes );
    }


    /**  tester comment:

         When test the following two constructors, don't know how to test the
         acquire and no acquire action. possible plans:
         1.release one handle and check the other( did not success since the
         other still exist and valid. )
         2. release one handle twice to see getLastError( )(the getLastError
         always returns invalidError(LINUX)).
    */
    TEST(Sal_Test_Pipe, ctors_no_acquire) {
        /// create a pipe.
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        /// constructs a pipe reference without acquiring the handle.
        ::osl::Pipe aNoAcquirePipe( aPipe.getHandle( ), SAL_NO_ACQUIRE );

        sal_Bool bRes = aNoAcquirePipe.is( );
        ///aPipe.clear( );
        ///bRes1 = aNoAcquirePipe.is( );


        // #test comment#: test constructor with no acquire of handle, only validation test, do not know how to test no acquire.
        ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_Pipe, ctors_acquire) {
        /// create a base pipe.
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        /// constructs two pipes without acquiring the handle on the base pipe.
        ::osl::Pipe aAcquirePipe( aPipe.getHandle( ) );
        ::osl::Pipe aAcquirePipe1( NULL );

        sal_Bool bRes = aAcquirePipe.is( );
        sal_Bool bRes1 = aAcquirePipe1.is( );

        // #test comment#: test constructor with no acquire of handle.only validation test, do not know how to test no acquire.
        ASSERT_TRUE( bRes && !bRes1 );
    }


    /** testing the method:
        inline sal_Bool SAL_CALL is() const;
    */
    TEST(Sal_Test_Pipe, is_001) {
        ::osl::Pipe aPipe;

        // #test comment#: test is(), check if the pipe is a valid one.
        ASSERT_TRUE( !aPipe.is( ) );
    }

    TEST(Sal_Test_Pipe, is_002) {
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );

        // #test comment#: test is(), a normal pipe creation.
        ASSERT_TRUE( aPipe.is( ) );
    }

    TEST(Sal_Test_Pipe, is_003) {
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        aPipe.clear( );

        // #test comment#: test is(), an invalid case
        ASSERT_TRUE( !aPipe.is( ) );
    }

    TEST(Sal_Test_Pipe, is_004) {
        ::osl::Pipe aPipe( NULL );

        // #test comment#: test is(), an invalid constructor.
        ASSERT_TRUE( !aPipe.is( ) );
    }


    /** testing the methods:
        inline sal_Bool create( const ::rtl::OUString & strName,
        oslPipeOptions Options, const Security &rSec );
        nline sal_Bool create( const ::rtl::OUString & strName,
        oslPipeOptions Options = osl_Pipe_OPEN );
    */
    TEST(Sal_Test_Pipe, create_named_security_001) {
        const Security rSec;
        ::osl::Pipe aPipe;
        sal_Bool bRes = aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSec );
        sal_Bool bRes1 = aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSec );
        aPipe.clear( );

        // #test comment#: test creation.
        ASSERT_TRUE( bRes && !bRes1 );
    }

    TEST(Sal_Test_Pipe, create_named_security_002) {
        const Security rSec;
        ::osl::Pipe aPipe, aPipe1;
        sal_Bool bRes = aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSec );
        sal_Bool bRes1 = aPipe1.create( uniquePipeName(aTestPipeName), osl_Pipe_OPEN, rSec );
        aPipe.clear( );

        // #test comment#: test creation and open.
        ASSERT_TRUE( bRes && bRes1 );
    }

    TEST(Sal_Test_Pipe, create_named_001) {
        ::osl::Pipe aPipe;
        sal_Bool bRes = aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        sal_Bool bRes1 = aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        aPipe.clear( );

        // #test comment#: test creation.
        ASSERT_TRUE( bRes && !bRes1);
    }

    TEST(Sal_Test_Pipe, create_named_002) {
       ::osl::Pipe aPipe, aPipe1;
        sal_Bool bRes = aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        sal_Bool bRes1 = aPipe1.create( uniquePipeName(aTestPipeName), osl_Pipe_OPEN );
        aPipe.clear( );

        // #test comment#: test creation and open.
        ASSERT_TRUE( bRes && bRes1);
    }

    TEST(Sal_Test_Pipe, create_named_003) {
       ::osl::Pipe aPipe;
        sal_Bool bRes = aPipe.create( uniquePipeName(aTestPipeName) );
        aPipe.clear( );

        // #test comment#: test default option is open.
        ASSERT_TRUE( !bRes );
    }


    /** testing the method:
        inline void SAL_CALL clear();
    */
    TEST(Sal_Test_Pipe, clear_001) {
      ::osl::Pipe aPipe;
        aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        aPipe.clear( );
        sal_Bool bRes = aPipe.is( );

        // #test comment#: test clear.
        ASSERT_TRUE( !bRes );
    }


    /** testing the methods:
        inline Pipe& SAL_CALL operator= (const Pipe& pipe);
        inline Pipe& SAL_CALL operator= (const oslPipe pipe );
    */
    TEST(Sal_Test_Pipe, assign_ref) {
        ::osl::Pipe aPipe, aPipe1;
        aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        aPipe1 = aPipe;
        sal_Bool bRes = aPipe1.is( );
        sal_Bool bRes1 = aPipe == aPipe1;
        aPipe.close( );
        aPipe1.close( );

        // #test comment#: test assign with reference.
        ASSERT_TRUE( bRes && bRes1 );
    }

    TEST(Sal_Test_Pipe, assign_handle) {
        ::osl::Pipe aPipe, aPipe1;
        aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        aPipe1 = aPipe.getHandle( );
        sal_Bool bRes = aPipe1.is( );
        sal_Bool bRes1 = aPipe == aPipe1;
        aPipe.close( );
        aPipe1.close( );

        // #test comment#: test assign with handle.
        ASSERT_TRUE( bRes && bRes1 );
    }


    /** testing the method:
        inline sal_Bool SAL_CALL isValid() const;
        isValid( ) has not been implemented under the following platforms, please refer to osl/pipe.hxx
    */
    // TEST(Sal_Test_Pipe, isValid_001) {
    //     ...
    //     // #test comment#: isValid() has not been implemented on all platforms.
    //     ASSERT_TRUE( ... );
    // }


    /** testing the method:
        inline sal_Bool SAL_CALL operator==( const Pipe& rPipe ) const;
    */
    TEST(Sal_Test_Pipe, isEqual_001) {
        ::osl::Pipe aPipe;
        aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        sal_Bool bRes  = aPipe == aPipe;
        aPipe.close( );

        // #test comment#: test isEqual(), compare its self.
        ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_Pipe, isEqual_002) {
       ::osl::Pipe aPipe, aPipe1, aPipe2;
        aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        aPipe1 = aPipe;
        aPipe2.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );

        sal_Bool bRes  = aPipe == aPipe1;
        sal_Bool bRes1 = aPipe == aPipe2;
        aPipe.close( );
        aPipe1.close( );
        aPipe2.close( );

        // #test comment#: test isEqual(),create one copy instance, and compare.
        ASSERT_TRUE( bRes && !bRes1 );
    }


    /** testing the method:
        inline void SAL_CALL close();
    */
    TEST(Sal_Test_Pipe, close_001) {
        ::osl::Pipe aPipe( uniquePipeName(aTestPipe1), osl_Pipe_CREATE );
        aPipe.close( );
        sal_Bool bRes = aPipe.is( );

        aPipe.clear( );
        sal_Bool bRes1 = aPipe.is( );

        // #test comment#: difference between close and clear.
        ASSERT_TRUE( bRes && !bRes1);
    }

    TEST(Sal_Test_Pipe, close_002) {
        ::osl::StreamPipe aPipe( uniquePipeName(aTestPipe1), osl_Pipe_CREATE );
        aPipe.close( );
        int nRet = aPipe.send( m_pTestString1.getStr(), 3 );

        // #test comment#: use after close.
        ASSERT_EQ( nRet, OSL_PIPE_FAIL );
    }


    /** testing the method:
        inline oslPipeError SAL_CALL accept(StreamPipe& Connection);
        please refer to StreamPipe::recv
    */


    /** testing the method:
        inline oslPipeError SAL_CALL getError() const;
    */
    /*
      PipeError[]= {
      { 0,               osl_Pipe_E_None              },  // no error
      { EPROTOTYPE,      osl_Pipe_E_NoProtocol        },  // Protocol wrong type for socket
      { ENOPROTOOPT,     osl_Pipe_E_NoProtocol        },  // Protocol not available
      { EPROTONOSUPPORT, osl_Pipe_E_NoProtocol        },  // Protocol not supported
      { ESOCKTNOSUPPORT, osl_Pipe_E_NoProtocol        },  // Socket type not supported
      { EPFNOSUPPORT,    osl_Pipe_E_NoProtocol        },  // Protocol family not supported
      { EAFNOSUPPORT,    osl_Pipe_E_NoProtocol        },  // Address family not supported by
      // protocol family
      { ENETRESET,       osl_Pipe_E_NetworkReset      },  // Network dropped connection because
      // of reset
      { ECONNABORTED,    osl_Pipe_E_ConnectionAbort   },  // Software caused connection abort
      { ECONNRESET,      osl_Pipe_E_ConnectionReset   },  // Connection reset by peer
      { ENOBUFS,         osl_Pipe_E_NoBufferSpace     },  // No buffer space available
      { ETIMEDOUT,       osl_Pipe_E_TimedOut          },  // Connection timed out
      { ECONNREFUSED,    osl_Pipe_E_ConnectionRefused },  // Connection refused
      { -1,              osl_Pipe_E_invalidError      }
      };
      did not define osl_Pipe_E_NotFound, osl_Pipe_E_AlreadyExists
    */
    TEST(Sal_Test_Pipe, getError_001) {
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_OPEN );
        oslPipeError nError = aPipe.getError( );
        printPipeError( aPipe );
        aPipe.clear( );

        // #test comment#: open a non-exist pipe.
        ASSERT_NE( nError, osl_Pipe_E_None );
    }

    TEST(Sal_Test_Pipe, getError_002) {
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        ::osl::Pipe aPipe1( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        oslPipeError nError = aPipe.getError( );
        printPipeError( aPipe );
        aPipe.clear( );
        aPipe1.clear( );

        // #test comment#: create an already exist pipe.
        ASSERT_NE( nError, osl_Pipe_E_None );
    }


    /** testing the method:
        inline oslPipe SAL_CALL getHandle() const;
    */
    TEST(Sal_Test_Pipe, getHandle_001) {
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_OPEN );
        sal_Bool bRes = aPipe == aPipe.getHandle( );
        aPipe.clear( );

        // #test comment#: one pipe should equal to its handle.
        ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_Pipe, getHandle_002) {
        ::osl::Pipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        ::osl::Pipe aPipe1( aPipe.getHandle( ) );
        sal_Bool bRes = aPipe == aPipe1;
        aPipe.clear( );
        aPipe1.clear( );

        // #test comment#: one pipe derived from another pipe's handle.
        ASSERT_TRUE( bRes );
    }

} // namespace osl_Pipe


namespace osl_StreamPipe
{

    /** testing the methods:
        inline StreamPipe();
        inline StreamPipe(oslPipe Pipe);;
        inline StreamPipe(const StreamPipe& Pipe);
        inline StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options = osl_Pipe_OPEN);
        inline StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options, const Security &rSec );
        inline StreamPipe( oslPipe pipe, __sal_NoAcquire noacquire );
    */

    TEST(Sal_Test_StreamPipe, ctors_none) {
        // create a pipe.
        ::osl::StreamPipe aStreamPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        // create an unattached pipe.
        ::osl::StreamPipe aStreamPipe1;
        sal_Bool bRes  = aStreamPipe1.is( );

        // assign it and check.
        aStreamPipe1 = aStreamPipe;
        sal_Bool bRes1 = aStreamPipe1.is( );
        aStreamPipe.clear( );
        aStreamPipe1.clear( );

        // #test comment#: test constructor with no parameter, before and after assign.
        ASSERT_TRUE( !bRes && bRes1 );
    }

    TEST(Sal_Test_StreamPipe, ctors_handle) {
        // create a pipe.
        ::osl::StreamPipe aStreamPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        // create a pipe with last handle.
        ::osl::StreamPipe aStreamPipe1( aStreamPipe.getHandle( ) );
        sal_Bool bRes  = aStreamPipe1.is( ) && aStreamPipe == aStreamPipe1;
        aStreamPipe.clear( );
        aStreamPipe1.clear( );

        // #test comment#: test constructor with other's handle.
        ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_StreamPipe, ctors_copy) {
        // create a pipe.
        ::osl::StreamPipe aStreamPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        // create an unattached pipe.
        ::osl::StreamPipe aStreamPipe1( aStreamPipe );
        sal_Bool bRes  = aStreamPipe1.is( ) && aStreamPipe == aStreamPipe1;
        aStreamPipe.clear( );
        aStreamPipe1.clear( );

        // #test comment#: test copy constructor.
        ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_StreamPipe, ctors_name_option) {
        // create a pipe.
        ::osl::StreamPipe aStreamPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        // create an unattached pipe.
        ::osl::StreamPipe aStreamPipe1( uniquePipeName(aTestPipeName), osl_Pipe_OPEN );
        sal_Bool bRes  = aStreamPipe1.is( ) && aStreamPipe.is( );
        aStreamPipe.clear( );
        aStreamPipe1.clear( );

        // #test comment#: test constructor with name and option.
        ASSERT_TRUE( bRes );
    }

    TEST(Sal_Test_StreamPipe, ctors_name_option_security) {
        // create a security pipe.
        const ::osl::Security rSecurity;
        ::osl::StreamPipe aSecurityPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE, rSecurity );

        sal_Bool bRes = aSecurityPipe.is( );
        aSecurityPipe.clear( );

        // #test comment#: test constructor with name, option and security, the test of security is not implemented yet.
        ASSERT_TRUE( bRes );
    }

    /**  tester comment:

         When test the following constructor, don't know how to test the
         acquire and no acquire action. possible plans:
         1.release one handle and check the other( did not success since the
         other still exist and valid. )
         2. release one handle twice to see getLastError( )(the getLastError
         always returns invalidError(LINUX)).
    */
    TEST(Sal_Test_StreamPipe, ctors_no_acquire) {
        // create a pipe.
        ::osl::StreamPipe aPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        // constructs a pipe reference without acquiring the handle.
        ::osl::StreamPipe aNoAcquirePipe( aPipe.getHandle( ), SAL_NO_ACQUIRE );

        sal_Bool bRes = aNoAcquirePipe.is( );
        aPipe.clear( );
        // bRes1 = aNoAcquirePipe.is( );

        // #test comment#: test constructor with no acquire of handle, only validation test, do not know how to test no acquire.
        ASSERT_TRUE( bRes );
    }

    /** testing the methods:
        inline StreamPipe & SAL_CALL operator=(oslPipe Pipe);
        inline StreamPipe& SAL_CALL operator=(const Pipe& pipe);
        mindy: not implementated in osl/pipe.hxx, so remove the cases
    */
    // TEST(Sal_Test_StreamPipe, assign_ref) {
    //     ::osl::StreamPipe aPipe, aPipe1;
    //     aPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
    //     aPipe1 = aPipe;
    //     sal_Bool bRes = aPipe1.is( );
    //     sal_Bool bRes1 = aPipe == aPipe1;
    //     aPipe.close( );
    //     aPipe1.close( );

    //     // #test comment#: test assign with reference.
    //     ASSERT_TRUE( bRes && bRes1 );
    // }

    // TEST(Sal_Test_StreamPipe, assign_ref) {
    //     ::osl::StreamPipe * pPipe = new ::osl::StreamPipe( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
    //     ::osl::StreamPipe * pAssignPipe = new ::osl::StreamPipe;
    //     *pAssignPipe = pPipe->getHandle( );

    //     bRes = pAssignPipe->is( );
    //     bRes1 = ( *pPipe == *pAssignPipe );
    //     pPipe->close( );

    //     delete pAssignPipe;

    //     // #test comment#: test assign with handle., seems not implemented under (LINUX)(W32)
    //     ASSERT_TRUE( bRes && bRes1  );
    // }


    /** wait _nSec seconds.
     */
    void thread_sleep( sal_Int32 _nSec )
    {
        /// print statement in thread process must use fflush() to force display.
        // printf("wait %d seconds. ", _nSec );
        fflush(stdout);

#ifdef WNT                               //Windows
        Sleep( _nSec * 1000 );
#endif
#if ( defined UNX ) || ( defined OS2 )   //Unix
        sleep( _nSec );
#endif
        // printf("done\n" );
    }

   // test read/write & send/recv data to pipe
    // -----------------------------------------------------------------------------

    class Pipe_DataSink_Thread : public Thread
    {
    public:
        sal_Char buf[256];
        Pipe_DataSink_Thread( ) { }

        ~Pipe_DataSink_Thread( )
        {
        }
    protected:
        void SAL_CALL run( )
        {
            sal_Int32 nChars = 0;

            printf("open pipe\n");
            // uniquePipeName(aTestPipeName) is a string = "TestPipe"
            ::osl::StreamPipe aSenderPipe( uniquePipeName(aTestPipeName), osl_Pipe_OPEN );
            if ( aSenderPipe.is() == sal_False )
            {
                printf("pipe open failed! \n");
            }
            else
            {
                printf("read\n");
                nChars = aSenderPipe.read( buf, m_pTestString1.getLength() + 1 );
                if ( nChars < 0 )
                {
                    printf("read failed! \n");
                    return;
                }
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

    // -----------------------------------------------------------------------------

    class Pipe_DataSource_Thread : public Thread
    {
    public:
        sal_Char buf[256];
        //::osl::StreamPipe aListenPipe;  //( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        ::osl::Pipe aListenPipe;
        ::osl::StreamPipe aConnectionPipe;
        Pipe_DataSource_Thread( )
        {
            printf("create pipe\n");
            aListenPipe.create( uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
        }
        ~Pipe_DataSource_Thread( )
        {
            aListenPipe.close();
        }
    protected:
        void SAL_CALL run( )
        {
            //create pipe.
            sal_Int32 nChars;
            //::osl::StreamPipe aListenPipe( test::uniquePipeName(aTestPipeName), osl_Pipe_CREATE );
            printf("listen\n");
            if ( aListenPipe.is() == sal_False )
            {
                printf("pipe create failed! \n");
            }
            else
            {
                //::osl::StreamPipe aConnectionPipe;

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
                //thread_sleep( 2 );
                printf("received message is: %s\n", buf );
                //aConnectionPipe.close();
            }
        }
    };


    /** testing the method: read/write/send/recv and Pipe::accept
     */
     TEST(Sal_Test_StreamPipe, recv_001) {
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

         // test send/recv/write/read.
         ASSERT_TRUE( nCompare1 == 0 && nCompare2 == 0 );
     }

} // namespace osl_StreamPipe

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
