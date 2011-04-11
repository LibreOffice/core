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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sal/types.h>

#include <testshl/simpleheader.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/process.h>
#include <osl/process.h>
#include <osl/module.hxx>

#include "rtl_Process_Const.h"

using namespace osl;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;

/** print a UNI_CODE String. And also print some comments of the string.
*/
inline void printUString( const ::rtl::OUString & str, const sal_Char * msg = NULL )
{
    if ( msg != NULL )
    {
        t_print("#%s #printUString_u# ", msg );
    }
    rtl::OString aString;
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", (char *)aString.getStr( ) );
}

// -----------------------------------------------------------------------------
inline ::rtl::OUString getModulePath( void )
{
    ::rtl::OUString suDirPath;
    ::osl::Module::getUrlFromAddress(
        reinterpret_cast< oslGenericFunction >(getModulePath), suDirPath );

    printUString(suDirPath, "modulePath:");
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') );
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') + 1);
    suDirPath += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bin"));
    return suDirPath;
}

// -----------------------------------------------------------------------------

namespace rtl_Process
{
class getAppCommandArg : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void getAppCommandArg_001()
    {
#if defined(WNT) || defined(OS2)
    const rtl::OUString EXECUTABLE_NAME(RTL_CONSTASCII_USTRINGPARAM("child_process.exe"));
#else
    const rtl::OUString EXECUTABLE_NAME(RTL_CONSTASCII_USTRINGPARAM("child_process"));
#endif
        rtl::OUString suCWD = getModulePath();
        // rtl::OUString suCWD2 = getExecutableDirectory();

        printUString(suCWD, "path to the current module");
        // printUString(suCWD2, "suCWD2");

        oslProcess hProcess = NULL;

        const int nParameterCount = 4;
        rtl_uString* pParameters[ nParameterCount ];

        pParameters[0] = suParam0.pData;
        pParameters[1] = suParam1.pData;
        pParameters[2] = suParam2.pData;
        pParameters[3] = suParam3.pData;

        rtl::OUString suFileURL = suCWD;
        suFileURL += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        suFileURL += EXECUTABLE_NAME;

        oslProcessError osl_error = osl_executeProcess(
            suFileURL.pData,
            pParameters,
            nParameterCount,
            osl_Process_WAIT,
            0, /* osl_getCurrentSecurity() */
            suCWD.pData,
            NULL,
            0,
            &hProcess );

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );
    //we could get return value only after the process terminated
        osl_joinProcess(hProcess);
        // CPPUNIT_ASSERT_MESSAGE
        // (
        //     "osl_joinProcess returned with failure",
        //     osl_Process_E_None == osl_error
        // );
    oslProcessInfo* pInfo = new oslProcessInfo;
    //please pay attention to initial the Size to sizeof(oslProcessInfo), or else
    //you will get unknow error when call osl_getProcessInfo
    pInfo->Size = sizeof(oslProcessInfo);
    osl_error = osl_getProcessInfo( hProcess, osl_Process_EXITCODE, pInfo );
    CPPUNIT_ASSERT_MESSAGE
        (
            "osl_getProcessInfo returned with failure",
            osl_Process_E_None == osl_error
        );

    t_print("the exit code is %d.\n", pInfo->Code );
    CPPUNIT_ASSERT_MESSAGE("rtl_getAppCommandArg or rtl_getAppCommandArgCount error.", pInfo->Code == 2);
    delete pInfo;
    }


    CPPUNIT_TEST_SUITE(getAppCommandArg);
    CPPUNIT_TEST(getAppCommandArg_001);
  //  CPPUNIT_TEST(getAppCommandArg_002);
    CPPUNIT_TEST_SUITE_END();
}; // class getAppCommandArg

/************************************************************************
 * For diagnostics( from sal/test/testuuid.cxx )
 ************************************************************************/
void printUuid( sal_uInt8 *pNode )
{
    printf("# UUID is: ");
    for( sal_Int32 i1 = 0 ; i1 < 4 ; i1++ )
    {
        for( sal_Int32 i2 = 0 ; i2 < 4 ; i2++ )
        {
            sal_uInt8 nValue = pNode[i1*4 +i2];
            if (nValue < 16)
            {
                printf( "0");
            }
            printf( "%02x" ,nValue );
        }
        if( i1 == 3 )
            break;
        printf( "-" );
    }
    printf("\n");
}

/**************************************************************************
 *  output UUID to a string
 **************************************************************************/
void printUuidtoBuffer( sal_uInt8 *pNode, sal_Char * pBuffer )
{
    sal_Int8 nPtr = 0;
    for( sal_Int32 i1 = 0 ; i1 < 16 ; i1++ )
    {
        sal_uInt8 nValue = pNode[i1];
        if (nValue < 16)
        {
             sprintf( (sal_Char *)(pBuffer + nPtr), "0");
             nPtr++;
        }
        sprintf( (sal_Char *)(pBuffer + nPtr), "%02x", nValue );
        nPtr += 2 ;
    }
}

class getGlobalProcessId : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }
    //gets a 16-byte fixed size identifier which is guaranteed not to change    during the current process.
    void getGlobalProcessId_001()
    {
        sal_uInt8 pTargetUUID1[16];
        sal_uInt8 pTargetUUID2[16];
        rtl_getGlobalProcessId( pTargetUUID1 );
        rtl_getGlobalProcessId( pTargetUUID2 );
    CPPUNIT_ASSERT_MESSAGE("getGlobalProcessId: got two same ProcessIds.", !memcmp( pTargetUUID1 , pTargetUUID2 , 16 ) );
    }
    //different processes different pids
    void getGlobalProcessId_002()
    {
#if defined(WNT) || defined(OS2)
    const rtl::OUString EXEC_NAME(RTL_CONSTASCII_USTRINGPARAM("child_process_id.exe"));
#else
    const rtl::OUString EXEC_NAME(RTL_CONSTASCII_USTRINGPARAM("child_process_id"));
#endif
        sal_uInt8 pTargetUUID1[16];
        rtl_getGlobalProcessId( pTargetUUID1 );
        printUuid( pTargetUUID1 );
        sal_Char pUUID1[32];
          printUuidtoBuffer( pTargetUUID1, pUUID1 );
    printf("# UUID to String is %s\n", pUUID1);

    rtl::OUString suCWD = getModulePath();
        oslProcess hProcess = NULL;
       rtl::OUString suFileURL = suCWD;
        suFileURL += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        suFileURL += EXEC_NAME;
    oslFileHandle* pChildOutputRead = new oslFileHandle();
        oslProcessError osl_error = osl_executeProcess_WithRedirectedIO(
            suFileURL.pData,
            NULL,
            0,
            osl_Process_WAIT,
            0,
            suCWD.pData,
            NULL,
            0,
            &hProcess,
        NULL,
        pChildOutputRead,
        NULL);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );
    //we could get return value only after the process terminated
        osl_joinProcess(hProcess);

        sal_Char pUUID2[33];
        pUUID2[32] = '\0';
    sal_uInt64 nRead = 0;
    osl_readFile( *pChildOutputRead, pUUID2, 32, &nRead );
    t_print("read buffer is %s, nRead is %d \n", pUUID2, nRead );
    OUString suUUID2 = OUString::createFromAscii( pUUID2 );
    CPPUNIT_ASSERT_MESSAGE("getGlobalProcessId: got two same ProcessIds.", suUUID2.equalsAsciiL( pUUID1, 32) == sal_False );
    }

    CPPUNIT_TEST_SUITE(getGlobalProcessId);
    CPPUNIT_TEST(getGlobalProcessId_001);
    CPPUNIT_TEST(getGlobalProcessId_002);
    CPPUNIT_TEST_SUITE_END();

}; // class getGlobalProcessId

} // namespace rtl_Process

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Process::getAppCommandArg, "rtl_Process");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Process::getGlobalProcessId, "rtl_Process");


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
