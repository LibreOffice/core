/*************************************************************************
 *
 *  $RCSfile: osl_old_test_file.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 08:57:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#include <cppunit/simpleheader.hxx>

namespace osl_test_file
{

// -----------------------------------------------------------------------------

class oldtestfile : public CppUnit::TestFixture
{
public:
    void test_file_001();
    void test_file_002();
    void test_file_003();
    void test_file_004();

    CPPUNIT_TEST_SUITE( oldtestfile );
    CPPUNIT_TEST( test_file_001 );
    CPPUNIT_TEST( test_file_002 );
    CPPUNIT_TEST( test_file_003 );
    CPPUNIT_TEST( test_file_004 );
    CPPUNIT_TEST_SUITE_END( );
};

char *aSource1[] =
{
    "a"    , "file:///" TEST_VOLUME "bla/a",
    "a/"   , "file:///" TEST_VOLUME "bla/a",
    "../a" , "file:///" TEST_VOLUME "a" ,
    "a/.." , "file:///" TEST_VOLUME "bla",
    "a/../b" , "file:///" TEST_VOLUME "bla/b",
    ".."   , "file:///" TEST_VOLUME "",
    "a/b/c/d/"   , "file:///" TEST_VOLUME "bla/a/b/c/d",
    "a/./c"   , "file:///" TEST_VOLUME "bla/a/c",
    "file:///bla/blub", "file:///" TEST_VOLUME "bla/blub",
    0 , 0
};

char *aSource2[ ] =
{
    "a" , "file:///" TEST_VOLUME "bla/blubs/schnubbel/a",
    "a/", "file:///" TEST_VOLUME "bla/blubs/schnubbel/a",
    "../a", "file:///" TEST_VOLUME "bla/blubs/a",
    "../../a", "file:///" TEST_VOLUME "bla/a",
    "../../../a", "file:///" TEST_VOLUME "a",
    "../../../a/b/c/d", "file:///" TEST_VOLUME "a/b/c/d",
    0,0
};

char *aSource3[ ] =
{
    ".." , "/a",
    "../a" , "/a/a",
    "e/f" , "/c/e/f",
    "../..", "",
    0,0
};

using namespace rtl;

void oldtestfile::test_file_001()
{
#ifdef WIN32
    return;
#endif

    OUString base1( RTL_CONSTASCII_USTRINGPARAM( "file:///" TEST_VOLUME "bla" ) );
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
        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
        OString obase = OUStringToOString( base1 , RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, "%d %s + %s = %s\n" ,e, obase.getStr(), aSource1[i], o.pData->buffer );
    }

    OUString err1( RTL_CONSTASCII_USTRINGPARAM( "../.." ) );
    OUString target;
    CPPUNIT_ASSERT_MESSAGE("failure #11",  osl_File_E_None != osl_getAbsoluteFileURL( base1.pData , err1.pData , &target.pData ) );

}

void oldtestfile::test_file_002()
{
#ifdef WIN32
    return;
#endif

    OUString base2( RTL_CONSTASCII_USTRINGPARAM( "file:///" TEST_VOLUME "bla/blubs/schnubbel" ) );
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
        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
        OString obase = OUStringToOString( base2 , RTL_TEXTENCODING_ASCII_US );
//      fprintf( stderr, "%d %s + %s = %s\n" ,e, obase.getStr(), aSource2[i], o.pData->buffer );
    }
}

void oldtestfile::test_file_003()
{
#ifdef WIN32
    return;
#endif

    // links !
#ifdef UNX
    int i;
    char buf[PATH_MAX];
    if( getcwd( buf, PATH_MAX ) )
    {
        char buf2[PATH_MAX];
        strcpy( buf2 , buf );
        strcat( buf2, "/a" );

        if( 0 == mkdir( buf2 , S_IRWXG | S_IRWXO | S_IRWXU ) )
        {
            strcat( buf2, "/b" );
            if( 0 == mkdir( buf2, S_IRWXU | S_IRWXO | S_IRWXU ) )
            {
                if( 0 == symlink( buf2 , "c" ) )
                {
                    OUString dir;
                    osl_getProcessWorkingDir( &(dir.pData) );

                    OUString base3 = dir;
                    base3 += OUString( RTL_CONSTASCII_USTRINGPARAM( "/c" ) );
                    for( i = 0 ; aSource3[i] ; i +=2 )
                    {
                        OUString target;
                        OUString rel = OUString::createFromAscii( aSource3[i] );
                        oslFileError e = osl_getAbsoluteFileURL( base3.pData, rel.pData , &target.pData );
                        CPPUNIT_ASSERT_MESSAGE("failure #3",  osl_File_E_None == e );
                        if( osl_File_E_None == e )
                        {
                            CPPUNIT_ASSERT_MESSAGE("failure #4",  target.getLength() >= dir.getLength() );
                            if( target.getLength() >= dir.getLength() )
                            {
                                                            int j;
                                                            for( j = dir.getLength() ;
                                     j < target.getLength() &&
                                         aSource3[i+1][j-dir.getLength()] == target[j]   ; j++ );
                                CPPUNIT_ASSERT_MESSAGE("failure #5",  j == target.getLength() );
                            }
                        }
                        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
                        OString obase = OUStringToOString( base3 , RTL_TEXTENCODING_ASCII_US );
                        fprintf( stderr, "%d %s + %s = %s\n" ,e, obase.getStr(), aSource3[i], o.pData->buffer );
                    }
                    unlink( "c" );
                }
                else
                {
                    CPPUNIT_ASSERT_MESSAGE("failure #6",  0 );
                }
                rmdir( "a/b" );
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE("failure #7",  0 );
            }
            rmdir( "a" );
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("failure #8",  0 );
        }
    }
    else
    {
        CPPUNIT_ASSERT_MESSAGE("failure #9",  0 );
    }
#endif
}

void oldtestfile::test_file_004()
{
#ifdef WIN32
    return;
#endif

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
        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
        OString obase = OUStringToOString( base4 , RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, "%d %s + %s = %s\n" ,e, obase.getStr(), aSource1[i], o.pData->buffer );
    }


//  fprintf( stderr, "test_file done\n" );
}

} // namespace osl_test_file

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_test_file::oldtestfile, "osl_File" );

// -----------------------------------------------------------------------------
NOADDITIONAL;

