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


// -----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(UNX) || defined(OS2)
#       include <unistd.h>
#endif

#include <rtl/logfile.hxx>
#include "gtest/gtest.h"

// #ifndef      _OSL_MODULE_HXX_
// #include <osl/module.hxx>
// #endif
#include <osl/file.hxx>
#if ( defined WNT )                     // Windows
#include <tchar.h>
#endif

using namespace ::osl;

inline void printUString( const ::rtl::OUString & str, const sal_Char * msg = "" )
{

    if (strlen(msg) > 0)
    {
        printf("%s: ", msg );
    }
    rtl::OString aString;
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", (char *)aString.getStr( ) );
}

/** get the absolute source file URL "file:///.../sal/qa/rtl/logfile/"
  */

inline ::rtl::OUString getTempPath( void )
{
#ifdef UNX
    rtl::OUString suDirURL(rtl::OUString::createFromAscii("file:///tmp/"));
#else /* Windows */
    rtl::OUString suDirURL(rtl::OUString::createFromAscii("file:///c:/temp/"));
#endif
    return suDirURL;
}

/** if the file exist
 */
bool t_fileExist(rtl::OUString const& _sFilename)
{
    ::osl::FileBase::RC   nError1;
    ::osl::File aTestFile( _sFilename );
    nError1 = aTestFile.open ( OpenFlag_Read );
    if ( ( ::osl::FileBase::E_NOENT != nError1 ) && ( ::osl::FileBase::E_ACCES != nError1 ) )
    {
        aTestFile.close( );
        return true;
    }
    return false;
}
/** get Current PID.
*/
inline ::rtl::OUString getCurrentPID(  )
{
        //~ Get current PID and turn it into OUString;
        int nPID = 0;
#ifdef WNT
        nPID = GetCurrentProcessId();
#else
        nPID = getpid();
#endif
        return ( ::rtl::OUString::valueOf( ( long )nPID ) );
}


// -----------------------------------------------------------------------------
/*
 * LLA:
 * check if logfile is create
 * be careful with relative logfiles they will create near the source, maybe it's no write access to it.
 * use absolute path to logfile instead.
 */
namespace rtl_logfile
{
    class logfile : public ::testing::Test
    {
    };

    //directly call rtl_logfile_trace
    TEST_F(logfile, logfile_001)
    {
#ifdef SOLARIS
        putenv(const_cast< char * >("RTL_LOGFILE=/tmp/logfile1"));
#elif WNT
        putenv("RTL_LOGFILE=c:\\temp\\logfile1");
#else
        setenv("RTL_LOGFILE", "/tmp/logfile1", 0);
#endif
        rtl_logfile_trace("trace %d\n", 2 );
        rtl_logfile_trace("trace %d %d\n" , 1,2 );
        rtl_logfile_trace("trace %d %d %d\n" , 1 , 2 ,3 );
        for (int i = 0; i < 1024; i++)
            rtl_logfile_trace("rubbish to flush the log\n");

        rtl::OUString suFilePath = getTempPath();
        suFilePath +=  rtl::OUString::createFromAscii("logfile1_") + getCurrentPID( );
        suFilePath +=  rtl::OUString::createFromAscii(".log");

        ::osl::FileBase::RC   nError1;
        ::osl::File aTestFile( suFilePath );
        printUString( suFilePath );
        nError1 = aTestFile.open ( OpenFlag_Read );
        ASSERT_TRUE(( ::osl::FileBase::E_NOENT != nError1 ) &&
                    ( ::osl::FileBase::E_ACCES != nError1 ) ) << "create the log file: but the logfile does not exist";
        sal_Char       buffer_read[400];
        sal_uInt64      nCount_read;
        nError1 = aTestFile.read( buffer_read, 400, nCount_read );
        //print("buffer is %s\n", buffer_read );
        ASSERT_TRUE( strstr( buffer_read, "trace 1 2 3") != NULL ) << "write right logs";
        aTestFile.sync();
        aTestFile.close();
        /*// delete logfile on the disk

        nError1 = osl::File::remove( suFilePath );
        printError( nError1 );
        ASSERT_TRUE( ( ::osl::FileBase::E_None == nError1 ) || ( nError1 == ::osl::FileBase::E_NOENT ) ) << "In deleteTestFile Function: remove ";
        */
    }

    //Profiling output should only be generated for a special product version of OpenOffice
    // which is compiled with a defined preprocessor symbol 'TIMELOG'. Now, the symbol not defined
    TEST_F(logfile, logfile_002)
    {
#ifdef SOLARIS
        putenv(const_cast< char * >("RTL_LOGFILE=/tmp/logfile2"));
#endif
#ifdef WNT
        putenv("RTL_LOGFILE=c:\\temp\\logfile2");
#endif
#ifdef LINUX
        setenv("RTL_LOGFILE", "/tmp/logfile2", 0);
#endif
        RTL_LOGFILE_TRACE( "trace the log" );
        RTL_LOGFILE_TRACE1( "trace %d" , 1 );
        RTL_LOGFILE_TRACE2( "trace %d %d" , 1,2 );
        RTL_LOGFILE_TRACE3( "trace %d %d %d" , 1 , 2 ,3 );
// TODO: assertion test!
    }

    TEST_F(logfile, logfile_003)
    {
#ifdef SOLARIS
        putenv(const_cast< char * >("RTL_LOGFILE=/tmp/logfile2"));
#endif
#ifdef WNT
        putenv("RTL_LOGFILE=c:\\temp\\logfile2");
#endif
#ifdef LINUX
        setenv("RTL_LOGFILE", "/tmp/logfile2", 0);
#endif
        RTL_LOGFILE_CONTEXT ( foo , "foo-function" );
        RTL_LOGFILE_CONTEXT_TRACE ( foo , "trace" );
        RTL_LOGFILE_CONTEXT_TRACE1 ( foo , "trace %d" , 1 );
        RTL_LOGFILE_CONTEXT_TRACE2 ( foo , "trace %d %d" , 1 , 2 );
        RTL_LOGFILE_CONTEXT_TRACE3 ( foo , "trace %d %d %d" , 1 , 2 , 3);
// TODO: assertion test!
    }

} // namespace rtl_logfile


// -----------------------------------------------------------------------------

//~ do some clean up work after all test completed.
class GlobalObject
{
public:
    ~GlobalObject()
        {
            try
            {
                printf( "\n#Do some clean-ups ... only delete logfile1_*.log here!\n" );
                rtl::OUString suFilePath = getTempPath();
                suFilePath +=  rtl::OUString::createFromAscii("logfile1_") + getCurrentPID( );
                suFilePath +=  rtl::OUString::createFromAscii(".log");

                //if ( ifFileExist( suFilePath )  == sal_True )
                ::osl::FileBase::RC nError1;
                nError1 = osl::File::remove( suFilePath );
#ifdef WNT
                printf("Please remove logfile* manully! Error is Permision denied!");
#endif
            }
            catch (...)
            {
                printf("Exception caught (...) in GlobalObject dtor()\n");
            }
        }
};

GlobalObject theGlobalObject;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
