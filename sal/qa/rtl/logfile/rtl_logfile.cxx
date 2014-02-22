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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(UNX)
#       include <unistd.h>
#endif

#include <rtl/logfile.hxx>

#include <osl/file.hxx>
#if ( defined WNT )                     // Windows
#include <tchar.h>
#endif

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

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
    rtl::OUString suDirURL( "file:///tmp/" );
#else /* Windows */
    rtl::OUString suDirURL( "file:///c:/temp/" );
#endif
    return suDirURL;
}

/** if the file exist
 */
bool t_fileExist(rtl::OUString const& _sFilename)
{
    ::osl::FileBase::RC   nError1;
    ::osl::File aTestFile( _sFilename );
    nError1 = aTestFile.open ( osl_File_OpenFlag_Read );
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



/*
 * LLA:
 * check if logfile is create
 * be careful with relative logfiles they will create near the source, maybe it's no write access to it.
 * use absolute path to logfile instead.
 */
namespace rtl_logfile
{
    class logfile : public CppUnit::TestFixture
    {
    public:

        //directly call rtl_logfile_trace
        void logfile_001()
        {
#ifdef SOLARIS
                putenv(const_cast< char * >("RTL_LOGFILE=/tmp/logfile1"));
#endif
#ifdef WNT
                putenv("RTL_LOGFILE=c:\\temp\\logfile1");
#endif
#ifdef LINUX
                setenv("RTL_LOGFILE", "/tmp/logfile1", 0);
#endif
                rtl_logfile_trace("trace %d\n", 2 );
                rtl_logfile_trace("trace %d %d\n" , 1,2 );
                rtl_logfile_trace("trace %d %d %d\n" , 1 , 2 ,3 );

                rtl::OUString suFilePath = getTempPath();
                suFilePath +=  rtl::OUString("logfile1_") + getCurrentPID( );
                suFilePath +=  rtl::OUString(".log");

                ::osl::FileBase::RC   nError1;
                ::osl::File aTestFile( suFilePath );
                printUString( suFilePath );
                nError1 = aTestFile.open ( osl_File_OpenFlag_Read );
                CPPUNIT_ASSERT_MESSAGE("create the log file: but the logfile does not exist",
                                       ( ::osl::FileBase::E_NOENT != nError1 ) &&
                                       ( ::osl::FileBase::E_ACCES != nError1 ) );
                sal_Char       buffer_read[400];
                sal_uInt64      nCount_read;
                nError1 = aTestFile.read( buffer_read, 400, nCount_read );
                //printf("buffer is %s\n", buffer_read );
                CPPUNIT_ASSERT_MESSAGE("write right logs", strstr( buffer_read, "trace 1 2 3") != NULL );
                aTestFile.sync();
                aTestFile.close();
                /*// delete logfile on the disk

                nError1 = osl::File::remove( suFilePath );
                printError( nError1 );
                CPPUNIT_ASSERT_MESSAGE( "In deleteTestFile Function: remove ", ( ::osl::FileBase::E_None == nError1 ) || ( nError1 == ::osl::FileBase::E_NOENT ) );
                */
        }
        //Profiling output should only be generated for a special product version of OpenOffice
        // which is compiled with a defined preprocessor symbol 'TIMELOG'. Now, the symbol not defined
        void logfile_002()
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

        void logfile_003()
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


        CPPUNIT_TEST_SUITE( logfile );
        CPPUNIT_TEST( logfile_001 );
        CPPUNIT_TEST( logfile_002 );
        CPPUNIT_TEST( logfile_003 );
        CPPUNIT_TEST_SUITE_END( );
    };

} // namespace rtl_logfile


CPPUNIT_TEST_SUITE_REGISTRATION( rtl_logfile::logfile);


CPPUNIT_PLUGIN_IMPLEMENT();

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
                suFilePath +=  rtl::OUString("logfile1_") + getCurrentPID( );
                suFilePath +=  rtl::OUString(".log");

                //if ( ifFileExist( suFilePath )  == sal_True )
                ::osl::FileBase::RC nError1;
                nError1 = osl::File::remove( suFilePath );
#ifdef WNT
                printf("Please remove logfile* manully! Error is Permision denied!");
#endif
            }
            catch (const CppUnit::Exception &e)
            {
                printf("Exception caught in GlobalObject dtor(). Exception message: '%s'. Source line: %d\n", e.what(), e.sourceLine().lineNumber());
            }
            catch (...)
            {
                printf("Exception caught (...) in GlobalObject dtor()\n");
            }
        }
};

GlobalObject theGlobalObject;



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
