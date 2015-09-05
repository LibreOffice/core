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

#include "gtest/gtest.h"

namespace osl_test_file
{

// -----------------------------------------------------------------------------

class oldtestfile : public ::testing::Test
{
public:
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

TEST_F(oldtestfile, test_file_001)
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
        ASSERT_TRUE( osl_File_E_None == e ) << "failure #1";
        if( osl_File_E_None == e )
        {
            ASSERT_TRUE( target.equalsAscii( aSource1[i+1] ) ) << "failure #1.1";
        }
        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
        OString obase = OUStringToOString( base1 , RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, "%d %s + %s = %s\n" ,e, obase.getStr(), aSource1[i], o.pData->buffer );
    }

    OUString err1( RTL_CONSTASCII_USTRINGPARAM( "../.." ) );
    OUString target;
    ASSERT_TRUE( osl_File_E_None != osl_getAbsoluteFileURL( base1.pData , err1.pData , &target.pData ) ) << "failure #11";

}

TEST_F(oldtestfile, test_file_002)
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
        ASSERT_TRUE(osl_File_E_None == e) << "failure #2";
        if( osl_File_E_None == e )
        {
            ASSERT_TRUE(target.equalsAscii( aSource2[i+1] )) << "failure #2.1";
        }
        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
        OString obase = OUStringToOString( base2 , RTL_TEXTENCODING_ASCII_US );
//      fprintf( stderr, "%d %s + %s = %s\n" ,e, obase.getStr(), aSource2[i], o.pData->buffer );
    }
}

TEST_F(oldtestfile, test_file_003)
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
                        ASSERT_TRUE(osl_File_E_None == e) << "failure #3";
                        if( osl_File_E_None == e )
                        {
                            ASSERT_TRUE(target.getLength() >= dir.getLength()) << "failure #4";
                            if( target.getLength() >= dir.getLength() )
                            {
                                                            int j;
                                                            for( j = dir.getLength() ;
                                     j < target.getLength() &&
                                         aSource3[i+1][j-dir.getLength()] == target[j]   ; j++ );
                                ASSERT_TRUE(j == target.getLength()) << "failure #5";
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
                    ASSERT_TRUE(0) << "failure #6";
                }
                rmdir( "a/b" );
            }
            else
            {
                ASSERT_TRUE(0) << "failure #7";
            }
            rmdir( "a" );
        }
        else
        {
            ASSERT_TRUE(0) << "failure #8";
        }
    }
    else
    {
        ASSERT_TRUE(0) << "failure #9";
    }
#endif
}

TEST_F(oldtestfile, test_file_004)
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
        ASSERT_TRUE(osl_File_E_None == e) << "failure #10";
        if( osl_File_E_None == e )
        {
            ASSERT_TRUE(target.equalsAscii( aSource1[i+1] )) << "failure #10.1";
        }
        OString o = OUStringToOString( target , RTL_TEXTENCODING_ASCII_US );
        OString obase = OUStringToOString( base4 , RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, "%d %s + %s = %s\n" ,e, obase.getStr(), aSource1[i], o.pData->buffer );
    }


//  fprintf( stderr, "test_file done\n" );
}

} // namespace osl_test_file

// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

