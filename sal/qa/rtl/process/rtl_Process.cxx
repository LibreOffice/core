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

#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sal/types.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/process.h>
#include <osl/process.h>
#include <osl/module.hxx>

#include "rtl_Process_Const.h"

using namespace osl;

/** print a UNI_CODE String. And also print some comments of the string.
*/
static void printUString( const OUString & str, const sal_Char * msg )
{
    if ( msg != nullptr )
    {
        printf("#%s #printUString_u# ", msg );
    }
    OString aString;
    aString = OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", aString.getStr( ) );
}

static OUString getModulePath()
{
    OUString suDirPath;
    ::osl::Module::getUrlFromAddress(
        reinterpret_cast< oslGenericFunction >(getModulePath), suDirPath );

    printUString(suDirPath, "modulePath:");
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') );
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') + 1);
    suDirPath += "bin";
    return suDirPath;
}

namespace rtl_Process
{
class getAppCommandArg : public CppUnit::TestFixture
{
public:
    void getAppCommandArg_001()
    {
#if defined(_WIN32)
        const OUString EXECUTABLE_NAME("child_process.exe");
#else
        const OUString EXECUTABLE_NAME("child_process");
#endif
        OUString suCWD = getModulePath();
        // OUString suCWD2 = getExecutableDirectory();

        printUString(suCWD, "path to the current module");
        // printUString(suCWD2, "suCWD2");

        oslProcess hProcess = nullptr;

        const int nParameterCount = 4;
        rtl_uString* pParameters[ nParameterCount ];

        pParameters[0] = suParam0.pData;
        pParameters[1] = suParam1.pData;
        pParameters[2] = suParam2.pData;
        pParameters[3] = suParam3.pData;

        OUString suFileURL = suCWD + "/" + EXECUTABLE_NAME;

        oslProcessError osl_error = osl_executeProcess(
            suFileURL.pData,
            pParameters,
            nParameterCount,
            osl_Process_WAIT,
            nullptr, /* osl_getCurrentSecurity() */
            suCWD.pData,
            nullptr,
            0,
            &hProcess );

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_createProcess failed",
            osl_Process_E_None, osl_error
        );
    //we could get return value only after the process terminated
        osl_joinProcess(hProcess);
        // CPPUNIT_ASSERT_MESSAGE
        // (
        //     "osl_joinProcess returned with failure",
        //     osl_Process_E_None == osl_error
        // );
        std::unique_ptr<oslProcessInfo> pInfo( new oslProcessInfo );
        //please pay attention to initial the Size to sizeof(oslProcessInfo), or else
        //you will get unknown error when call osl_getProcessInfo
        pInfo->Size = sizeof(oslProcessInfo);
        osl_error = osl_getProcessInfo( hProcess, osl_Process_EXITCODE, pInfo.get() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "osl_getProcessInfo returned with failure",
                 osl_Process_E_None, osl_error
            );

        printf("the exit code is %" SAL_PRIuUINT32 ".\n", pInfo->Code );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("rtl_getAppCommandArg or rtl_getAppCommandArgCount error.", static_cast<oslProcessExitCode>(2), pInfo->Code);
    }

    CPPUNIT_TEST_SUITE(getAppCommandArg);
    CPPUNIT_TEST(getAppCommandArg_001);
  //  CPPUNIT_TEST(getAppCommandArg_002);
    CPPUNIT_TEST_SUITE_END();
}; // class getAppCommandArg

/************************************************************************
 * For diagnostics( from sal/test/testuuid.cxx )
 ************************************************************************/
static void printUuid( const sal_uInt8 *pNode )
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
static void printUuidtoBuffer( const sal_uInt8 *pNode, sal_Char * pBuffer )
{
    sal_Int8 nPtr = 0;
    for( sal_Int32 i1 = 0 ; i1 < 16 ; i1++ )
    {
        sal_uInt8 nValue = pNode[i1];
        if (nValue < 16)
        {
             sprintf( pBuffer + nPtr, "0");
             nPtr++;
        }
        sprintf( pBuffer + nPtr, "%02x", nValue );
        nPtr += 2 ;
    }
}

class getGlobalProcessId : public CppUnit::TestFixture
{
public:
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
#if defined(_WIN32)
        const OUString EXEC_NAME("child_process_id.exe");
#else
        const OUString EXEC_NAME("child_process_id");
#endif
        sal_uInt8 pTargetUUID1[16];
        rtl_getGlobalProcessId( pTargetUUID1 );
        printUuid( pTargetUUID1 );
        sal_Char pUUID1[32];
        printUuidtoBuffer( pTargetUUID1, pUUID1 );
        printf("# UUID to String is %s\n", pUUID1);

        OUString suCWD = getModulePath();
        oslProcess hProcess = nullptr;
        OUString suFileURL = suCWD + "/" + EXEC_NAME;
        oslFileHandle* pChildOutputRead = new oslFileHandle();
        oslProcessError osl_error = osl_executeProcess_WithRedirectedIO(
                suFileURL.pData,
                nullptr,
                0,
                osl_Process_WAIT,
                nullptr,
                suCWD.pData,
                nullptr,
                0,
                &hProcess,
            nullptr,
            pChildOutputRead,
            nullptr);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_createProcess failed",
            osl_Process_E_None, osl_error
        );
        //we could get return value only after the process terminated
        osl_joinProcess(hProcess);

        sal_Char pUUID2[33];
        pUUID2[32] = '\0';
        sal_uInt64 nRead = 0;
        osl_readFile( *pChildOutputRead, pUUID2, 32, &nRead );
        printf("read buffer is %s, nRead is %" SAL_PRIdINT64 "\n", pUUID2, nRead );
        OUString suUUID2 = OUString::createFromAscii( pUUID2 );
        CPPUNIT_ASSERT_MESSAGE("getGlobalProcessId: got two same ProcessIds.", !suUUID2.equalsAsciiL( pUUID1, 32) );
    }

    CPPUNIT_TEST_SUITE(getGlobalProcessId);
    CPPUNIT_TEST(getGlobalProcessId_001);
    CPPUNIT_TEST(getGlobalProcessId_002);
    CPPUNIT_TEST_SUITE_END();

}; // class getGlobalProcessId

} // namespace rtl_Process

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Process::getAppCommandArg, "rtl_Process");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Process::getGlobalProcessId, "rtl_Process");

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
