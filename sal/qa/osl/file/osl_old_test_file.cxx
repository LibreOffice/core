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

// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx

#include <stdio.h>

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

// -----------------------------------------------------------------------------

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
    0 , 0
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
    0,0
};

const char * const aSource3[ ] =
{
    ".." , "/a",
    "../a" , "/a/a",
    "e/f" , "/c/e/f",
    "../..", "",
    0,0
};

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

void oldtestfile::test_file_001()
{
#ifndef WIN32
    OUString base1( RTL_CONSTASCII_USTRINGPARAM( "file:///" TEST_VOLUME "bla" ) );
    int i;
    for( i = 0 ; aSource1[i] ; i +=2 )
    {
        OUString target;
        OUString rel = OUString::createFromAscii( aSource1[i] );
        oslFileError e = osl_getAbsoluteFileURL( base1.pData, rel.pData , &target.pData );
        //fprintf(stderr, "%d : %s -- %s -- %s\n", i, aSource1[i], aSource1[i+1], OUStringToOString(target , RTL_TEXTENCODING_ASCII_US ).getStr() );
        CPPUNIT_ASSERT_MESSAGE("failure #1",  osl_File_E_None == e );
        if( osl_File_E_None == e )
        {
            CPPUNIT_ASSERT_MESSAGE("failure #1.1",  target.equalsAscii( aSource1[i+1] ) );
        }
        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
        OString obase = OUStringToOString( base1 , RTL_TEXTENCODING_ASCII_US );
        // fprintf( stderr, "%d %s + %s = %s\n" ,e, obase.getStr(), aSource1[i], o.pData->buffer );
    }

    OUString err1( RTL_CONSTASCII_USTRINGPARAM( "../.." ) );
    OUString target;
    // CPPUNIT_ASSERT_MESSAGE("failure #11",  osl_File_E_None != osl_getAbsoluteFileURL( base1.pData , err1.pData , &target.pData ) );
#endif
}

void oldtestfile::test_file_002()
{
#ifndef WIN32
    OUString base2( RTL_CONSTASCII_USTRINGPARAM( "file:///" TEST_VOLUME "bla/blubs/schnubbel" ) );
    int i;
    for(  i = 0 ; aSource2[i] ; i +=2 )
    {
        OUString target;
        OUString rel = OUString::createFromAscii( aSource2[i] );
        oslFileError e = osl_getAbsoluteFileURL( base2.pData, rel.pData , &target.pData );
        //fprintf(stderr, "%d : %s -- %s -- %s\n", i, aSource2[i], aSource2[i+1], OUStringToOString(target , RTL_TEXTENCODING_ASCII_US ).getStr() );
        CPPUNIT_ASSERT_MESSAGE("failure #2",  osl_File_E_None == e );
        if( osl_File_E_None == e )
        {
            CPPUNIT_ASSERT_MESSAGE("failure #2.1",  target.equalsAscii( aSource2[i+1] ) );
        }
        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
        OString obase = OUStringToOString( base2 , RTL_TEXTENCODING_ASCII_US );
    }
#endif
}

void oldtestfile::test_file_004()
{
#ifndef WIN32
    OUString base4( RTL_CONSTASCII_USTRINGPARAM( "file:///" TEST_VOLUME "bla/" ) );
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

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( osl_test_file::oldtestfile);

// -----------------------------------------------------------------------------
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
