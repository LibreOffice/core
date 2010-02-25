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
#include "precompiled_testshl2.hxx"

#include <stdio.h>

#ifdef WNT
// #define UNDER_WINDOWS_DEBUGGING
// Nice feature, to debug under windows, install msdev locally and use DebugBreak() to stop a new process at a point you want.
#ifdef UNDER_WINDOWS_DEBUGGING
#include <tools/presys.h>
#include <windows.h>
#include <tools/postsys.h>

#define VCL_NEED_BASETSD

#endif /* UNDER_WINDOWS_DEBUGGING */
#endif /* WNT */


#include <vector>


#ifndef _SAL_TRES_H_
#include <rtl/tres.h>
#endif

#ifndef _TESTSHL_TSTMGR_H_
#include "tstmgr.h"
#endif

#ifndef _RTL_STRING_HXX_
    #include <rtl/string.hxx>
#endif

#include <osl/time.h>

using namespace std;


/**
 * create bitmap of comandline parameters
 */
sal_uInt32 createFlags( vector< sal_Char* > const& cmdln )
{
    sal_uInt32 retflags = rtl_tres_Flag_OK;

    vector< sal_Char* >::const_iterator iter = cmdln.begin();
    while( iter != cmdln.end() )
    {
        fprintf( stderr, "%s\n", *iter );
        if ( *iter[0] == '-' )
        {
            rtl::OString item( *iter );
            if ( item == "-boom" )
                retflags |= rtl_tres_Flag_BOOM;

            if ( item == "-verbose" )
                retflags |= rtl_tres_Flag_VERBOSE;

            if ( item == "-skip" )
                retflags |= rtl_tres_Flag_SKIP;

            if ( item == "-log" )
                retflags |= rtl_tres_Flag_LOG;

            if ( item == "-his" )
                retflags |= rtl_tres_Flag_HIS;

            if ( item == "-time" )
                retflags |= rtl_tres_Flag_TIME;

            if ( item == "-msg" )
                retflags |= rtl_tres_Flag_MSG;

            if ( item == "-quiet" )
                retflags |= rtl_tres_Flag_QUIET;
        }
        iter++;
    }

    return retflags;
}

sal_uInt32 createFlags(int argc, char* argv[])
{
    vector< sal_Char* > cmdln;
    sal_Int32 i;

     /* collect comandline */
    for ( i = 1; i < argc; i++ )
         cmdln.push_back( argv[i] );

    return createFlags(cmdln);
}

// -----------------------------------------------------------------------------

/**
 * display usage screen
 */

void usage()
{
    fprintf( stdout,
            "USAGE: testshl shlname scename [-boom][-verbose][-log][-his][-msg]\n" );
    exit(0);
}


#include <fstream>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TextTestResult.h>

namespace CppunitTest
{
    class AStringTest : public CppUnit::TestCase
    {
        rtl::OString *m_pStr;
    public:
        AStringTest()
            :m_pStr(NULL) {}

        void setUp()
            {
                m_pStr = new rtl::OString("test1");
                // throw std::exception("initialization failed.");
            }

        void tearDown()
            {
                delete m_pStr;
            }

        void testEquality()
            {
                CPPUNIT_ASSERT( *m_pStr == "test1" );
                CPPUNIT_ASSERT( (*m_pStr).equalsIgnoreAsciiCase("Test1") );
                CPPUNIT_ASSERT( (*m_pStr).equalsIgnoreAsciiCase("Test2") );
                CPPUNIT_ASSERT( *m_pStr == "test1" );
                CPPUNIT_ASSERT( *m_pStr == "must also fail" );
            }
        void testEquality2()
            {
                rtl::OString aStr("test2");
                CPPUNIT_ASSERT( aStr == "test2" );
                CPPUNIT_ASSERT_MESSAGE("ein vergleichstest", aStr == "test2");
                CPPUNIT_ASSERT( aStr == "must also fail" );
            }
        void testThrow()
            {
                throw std::exception("an own exception!");
                CPPUNIT_ASSERT( *m_pStr == "test2" );
            }
    };

    CppUnit::TestSuite *suite1()
    {
        CppUnit::TestSuite *suite = new CppUnit::TestSuite( "AStringTest" );

        // CppUnit::TestSuite suite;
        // CppUnit::TextTestResult result;

        suite->addTest( new CppUnit::TestCaller<AStringTest>( "throw test", &AStringTest::testThrow ));
//      suite->addTest( new CppUnit::TestCaller<AStringTest>( "test op eq", &AStringTest::testEquality ));
//      suite->addTest( new CppUnit::TestCaller<AStringTest>( "test op eq", &AStringTest::testEquality2 ));

        return suite;
    }

    // -----------------------------------------------------------------------------

    class ASimpleTest : public CppUnit::TestCase
    {
    public:
        void testEqual()
            {
                CPPUNIT_ASSERT( 1 == 1 );
            }
    };

    CppUnit::TestSuite *suite2()
    {
        CppUnit::TestSuite *suite = new CppUnit::TestSuite( "A simple test" );

        // CppUnit::TestSuite suite;
        // CppUnit::TextTestResult result;

        suite->addTest( new CppUnit::TestCaller<ASimpleTest>( "1 == 1", &ASimpleTest::testEqual ));

        return suite;
    }

    // -----------------------------------------------------------------------------

    CppUnit::TestSuite *suite()
    {
        CppUnit::TestSuite *suite = new CppUnit::TestSuite( "A simple test" );

        // CppUnit::TestSuite suite;
        // CppUnit::TextTestResult result;

        suite->addTest( suite1() );
//      suite->addTest( suite2() );

        return suite;
    }
}

void cppunitbased_test()
{
    {
        // ofstream out("c:\\temp\\output.txt", ios::out);
        CppUnit::TextTestResult aResult;

        CppUnit::TestSuite* pSuite = CppunitTest::suite();

        int nTests = pSuite->countTestCases();
        pSuite->run(&aResult);

        // aResult.print(out);
        cout << aResult;

        delete pSuite;
    }

    exit(1);
}

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    cppunitbased_test();

    tslTestManager hMgr = 0;

    /* show usage screen if too less parameters */
    if ( argc < 3 )
        usage();

#ifdef UNDER_WINDOWS_DEBUGGING
    DebugBreak();
#endif

    sal_uInt32 nCmdlinebitflags = createFlags( argc, argv );

    hMgr = tsl_TestManager_create( argv, argc, nCmdlinebitflags );
    tsl_TestManager_run( hMgr );
    tsl_TestManager_destroy( hMgr );
    return 0;
}
