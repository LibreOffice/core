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

#include <osl/file.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#ifdef SAL_UNX
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#define TEST_VOLUME ""
#else
// WINDOWS
#define TEST_VOLUME "c:/"
#endif

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace osl_test_file
{

class oldtestfile : public CppUnit::TestFixture
{
public:
    void test_file_001();
    void test_file_002();
    void test_file_004();

    CPPUNIT_TEST_SUITE( oldtestfile );
    CPPUNIT_TEST( test_file_001 );
    CPPUNIT_TEST( test_file_002 );
    CPPUNIT_TEST( test_file_004 );
    CPPUNIT_TEST_SUITE_END( );
};

const char * const aSource1[] =
{
    "a"    , "file:///" TEST_VOLUME "bla/a",
    ///TODO: check if last slash must be omitted in resolved path.
//    "a/"   , "file:///" TEST_VOLUME "bla/a",
    "../a" , "file:///" TEST_VOLUME "a" ,
    "a/.." , "file:///" TEST_VOLUME "bla/",
    "a/../b" , "file:///" TEST_VOLUME "bla/b",
    ".."   , "file:///" TEST_VOLUME "",
    "a/b/c/d"   , "file:///" TEST_VOLUME "bla/a/b/c/d",
    "a/./c"   , "file:///" TEST_VOLUME "bla/a/c",
    "file:///bla/blub", "file:///"  "bla/blub",
    nullptr , nullptr
};

const char * const aSource2[ ] =
{
    "a" , "file:///" TEST_VOLUME "bla/blubs/schnubbel/a",
    ///TODO: check if last slash must be omitted in resolved path.
//    "a/", "file:///" TEST_VOLUME "bla/blubs/schnubbel/a",
    "../a", "file:///" TEST_VOLUME "bla/blubs/a",
    "../../a", "file:///" TEST_VOLUME "bla/a",
    "../../../a", "file:///" TEST_VOLUME "a",
    "../../../a/b/c/d", "file:///" TEST_VOLUME "a/b/c/d",
    nullptr,nullptr
};

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

void oldtestfile::test_file_001()
{
#ifndef WIN32
    OUString base1( "file:///" TEST_VOLUME "bla" );
    int i;
    for( i = 0 ; aSource1[i] ; i +=2 )
    {
        OUString target;
        OUString rel = OUString::createFromAscii( aSource1[i] );
        oslFileError e = osl_getAbsoluteFileURL( base1.pData, rel.pData , &target.pData );
        CPPUNIT_ASSERT_MESSAGE("failure #1",  osl_File_E_None == e );
        if( osl_File_E_None == e )
        {
            CPPUNIT_ASSERT_MESSAGE("failure #1.1",  target.equalsAscii( aSource1[i+1] ) );
        }
    }
#endif
}

void oldtestfile::test_file_002()
{
#ifndef WIN32
    OUString base2( "file:///" TEST_VOLUME "bla/blubs/schnubbel" );
    int i;
    for(  i = 0 ; aSource2[i] ; i +=2 )
    {
        OUString target;
        OUString rel = OUString::createFromAscii( aSource2[i] );
        oslFileError e = osl_getAbsoluteFileURL( base2.pData, rel.pData , &target.pData );
        CPPUNIT_ASSERT_MESSAGE("failure #2",  osl_File_E_None == e );
        if( osl_File_E_None == e )
        {
            CPPUNIT_ASSERT_MESSAGE("failure #2.1",  target.equalsAscii( aSource2[i+1] ) );
        }
    }
#endif
}

void oldtestfile::test_file_004()
{
#ifndef WIN32
    OUString base4( "file:///" TEST_VOLUME "bla/" );
    int i;
    for( i = 0 ; aSource1[i] ; i +=2 )
    {
        OUString target;
        OUString rel = OUString::createFromAscii( aSource1[i] );
        oslFileError e = osl_getAbsoluteFileURL( base4.pData, rel.pData , &target.pData );
        CPPUNIT_ASSERT_MESSAGE("failure #10",  osl_File_E_None == e );
        if( osl_File_E_None == e )
        {
            CPPUNIT_ASSERT_MESSAGE("failure #10.1",  target.equalsAscii( aSource1[i+1] ) );
        }
    }
#endif
}

} // namespace osl_test_file

CPPUNIT_TEST_SUITE_REGISTRATION( osl_test_file::oldtestfile);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
