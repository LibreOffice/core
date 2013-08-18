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


// Documentation about bootstraping can be found at:
// http://udk.openoffice.org/common/man/concept/micro_deployment.html

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> // STL

#include "testshl/stringhelper.hxx"

#include <testshl/simpleheader.hxx>
#include <rtl/bootstrap.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <osl/process.h> /* osl_getExecutableFile() */

#include <osl/thread.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringToOString;
using ::rtl::OString;
#define TESTSHL2_INI "testshl2"
#define PSEUDO_INI   "pseudo"

/** print a UNI_CODE String. And also print some comments of the string.
 */
inline void printUString( const ::rtl::OUString & str, const sal_Char * msg = "" )
{

    if (msg && msg[0] != 0)
    {
        t_print("%s: ", msg );
    }
    rtl::OString aString;
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", (char *)aString.getStr( ) );
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

/** get the exectutable path ( here is bootstrap_process), on Linux, such as "sal/unxlngi4.pro/bin/"
  */
inline ::rtl::OUString getModulePath( void )
{
    ::rtl::OUString suDirPath;
    ::osl::Module::getUrlFromAddress( ( oslGenericFunction ) &getModulePath, suDirPath );

    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') );
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') + 1);
    suDirPath += rtl::OUString("bin");
    return suDirPath;
}

#define TESTSHL2_INI "testshl2"
#define PSEUDO_INI   "pseudo"


static rtl::OUString getExecutableDirectory()
{
    rtl::OUString fileName;
    osl_getExecutableFile(&fileName.pData);

    sal_Int32 nDirEnd = fileName.lastIndexOf('/');

    OSL_ENSURE(nDirEnd >= 0, "Cannot locate executable directory");

    rtl::OUString aDirURL = fileName.copy(0, nDirEnd);
    return aDirURL;
}


// get the URL of testshl2rc/rtlrc/pseudorc
inline rtl::OUString t_getSourcePath(rtl::OString const& _sFilename)
{

     rtl::OUString aDirURL(getExecutableDirectory());
     aDirURL += "/";
     aDirURL += OUString::createFromAscii( _sFilename.getStr() );
#if defined(WNT)
    aDirURL += rtl::OUString(".ini");
#else
    aDirURL += rtl::OUString("rc");
#endif
    return aDirURL;
}

void thread_sleep_tenth_sec(sal_Int32 _nTenthSec)
{
#ifdef WNT      //Windows
    Sleep(_nTenthSec * 100 );
#endif
#if ( defined UNX )
    TimeValue nTV;
    nTV.Seconds = static_cast<sal_uInt32>( _nTenthSec/10 );
    nTV.Nanosec = ( (_nTenthSec%10 ) * 100000000 );
    osl_waitThread(&nTV);
#endif
}

// -----------------------------------------------------------------------------

namespace rtl_Bootstrap
{
    class ctor : public CppUnit::TestFixture
    {
    public:
        // initialise your test code values here.
        void setUp()
            {
            }

        void tearDown()
            {
            }

        // ctor with ini name
        void ctor_001()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                printUString( suIniname );
                Bootstrap aBootstrap( suIniname );
                rtl::OUString suGetname; // = rtl::OUString();
                aBootstrap.getIniName( suGetname );
                printUString( suGetname );

                // LLA: first: this seems to be a wrong test.
                //      second: there seems to be a design hole, if I give a absolute path ini file,
                //              but try to use ${file::KEYVALUE} than 'file' will only used out of the 'executable path'/file
                //              not from the path given from the absolute path.

                // Due to the fact, we create at this position a file (createTestshl2rc() ), we check for existence
                bool bFileExist = t_fileExist( suGetname );
                CPPUNIT_ASSERT_MESSAGE("ctor error with initial file.", bFileExist == true );
            }

        void ctor_002()
            {
                rtl::Bootstrap aBootstrap;
                rtl::OUString suGetname;
                aBootstrap.getIniName( suGetname );
                printUString( suGetname );
                CPPUNIT_ASSERT_MESSAGE("ctor error without initial file.", !suGetname.isEmpty() );
            }

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class ctor

    class getFrom : public CppUnit::TestFixture
    {
    public:
        // initialise your test code values here.
        void setUp()
            {
            }

        void tearDown()
            {
            }
        // get the value of env variable
        void getFrom_001()
            {
                Bootstrap aBootstrap;
                rtl::OUString suValue;
                rtl::OUString suValuename ("ENABLE_JAVA");
                //aBootstrap.getFrom( suValuename, suValue );
                aBootstrap.getFrom( suValuename, suValue );
                sal_Char *  pStr = getenv("ENABLE_JAVA");
                //      printUString( suGetname );
                CPPUNIT_ASSERT_MESSAGE("get the value of environment variable.", suValue.compareToAscii( pStr ) == 0 );
            }
        /* Notes on Windows:
           void getFrom_001_1()
           {
           Bootstrap aBootstrap;
           rtl::OUString suValue;
           rtl::OUString suValuename ("SRC_ROOT");
           //aBootstrap.getFrom( suValuename, suValue );
           aBootstrap.getFrom( suValuename, suValue );
           sal_Char *  pStr = getenv("SRC_ROOT");
           //   printUString( suGetname );
           CPPUNIT_ASSERT_MESSAGE("get the value of environment variable.", suValue.compareToAscii( pStr ) == 0 );
           }
           The result on Windows:
           # # the SRC_ROOT is e:\Qadev\cvs\m19
           # # suValue is e:Qadevcvsm19
           reason:
           The problem is that the internally getenv()ed variable SRC_ROOT is macro expanded,
           thus every \ will introduce an escape.
        */

        // get the value of a variable in ini file
        void getFrom_002()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                Bootstrap aBootstrap( suIniname );
                rtl::OUString suGetname;
                rtl::OUString suValuename ("INHERITED_VALUE");
                aBootstrap.getFrom( suValuename, suGetname );
                printUString( suGetname );
                CPPUNIT_ASSERT_MESSAGE("get the value of a variable in ini file.", !suGetname.isEmpty());
            }

        //use defaut value
        void getFrom_003()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                Bootstrap aBootstrap( suIniname );
                rtl::OUString suGetname;
                rtl::OUString suValuename("MY_VALUE");
                rtl::OUString myDefault("2");
                aBootstrap.getFrom( suValuename, suGetname, myDefault );
                CPPUNIT_ASSERT_MESSAGE("getFrom use default.", suGetname.compareTo( myDefault ) == 0 );
            }

        void getFrom_004()
            {
                t_print("1\n");
                // initialise Bootstrap with an own ini file
                // PSEUDO_INI is pseudo(rc|.ini) created be create_pseudorc()
                rtl::OUString suIniname = t_getSourcePath(PSEUDO_INI);
                Bootstrap aBootstrap( suIniname );

                rtl::OUString suGetIniName;
                aBootstrap.getIniName( suGetIniName );

                printUString(suGetIniName, "Current bootstrap file");
                sal_Int32 nIndex = suGetIniName.indexOf(rtl::OUString("pseudo"));
                CPPUNIT_ASSERT_MESSAGE("ini name must have 'pseudo' in name.", nIndex > 0);

                rtl::OUString suValue;
                rtl::OUString suKeyName("FILE");
                aBootstrap.getFrom( suKeyName, suValue );
                printUString( suValue );
                sal_Int32 nCompare = suValue.compareTo( rtl::OUString("pseudo file") );

                CPPUNIT_ASSERT_MESSAGE("<Bootstrap('pseudo')>.getFrom('FILE', ...) result is unexpected.",  nCompare == 0);
            }
        void getFrom_004_1()
            {
                // get the same key out of the default context
                rtl::OUString suKeyName("FILE");
                rtl::OUString suGetValue;
                Bootstrap::get( suKeyName, suGetValue );
                printUString( suGetValue );

                CPPUNIT_ASSERT_MESSAGE("Bootstrap::get('FILE', ...)", suGetValue.compareTo( rtl::OUString("testshl2 file") ) == 0 );
            }

    /** helper function: return the child process's ret value( typedef sal_uInt32 oslProcessExitCode;)
       * param1 is the process's name(only file name, not include path)
      */
     oslProcessExitCode ini_execProcess( const sal_Char* process_name, const sal_Char * flag )
     {
         rtl::OUString suCWD = getModulePath();
            oslProcess hProcess = NULL;
           rtl::OUString suFileURL = suCWD;
            suFileURL += rtl::OUString("/") +  rtl::OUString::createFromAscii(process_name) ;
#if defined(WNT)
        suFileURL += rtl::OUString(".exe");
#endif
            const int nParameterCount = 3;
            rtl_uString* pParameters[ nParameterCount ];
            OUString suFlag( OUString::createFromAscii(flag) );
        OUString suEnv1( "-env:UNO_SERVICES=service.rdb" );
        OUString suIniname = t_getSourcePath("rtl");
        printUString( suIniname, "rtl path:");

            pParameters[0] = suFlag.pData;
            pParameters[1] = suEnv1.pData;
            // the custom ini/rc file's URL
            pParameters[2] = suIniname.pData;

            oslProcessError osl_error = osl_executeProcess(
                suFileURL.pData,
                pParameters,
                    nParameterCount,
                osl_Process_WAIT,
                0,
                suCWD.pData,
                NULL,
                0,
                &hProcess );

            CPPUNIT_ASSERT_MESSAGE
            (
                "osl_createProcess failed",
                osl_error == osl_Process_E_None
            );
               osl_joinProcess(hProcess);
            oslProcessInfo* pInfo = new oslProcessInfo;
        pInfo->Size = sizeof( oslProcessInfo );
        osl_error = osl_getProcessInfo( hProcess, osl_Process_EXITCODE, pInfo );
        CPPUNIT_ASSERT_MESSAGE
            (
                "osl_getProcessInfo returned with failure",
                osl_Process_E_None == osl_error
            );

        t_print("the exit code is %d.\n", pInfo->Code );
        oslProcessExitCode nCode = pInfo->Code;
        delete pInfo;
         return nCode;
    }

         void getFrom_005_1()
            {
               oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "1" );
            CPPUNIT_ASSERT_MESSAGE("Parameters passed by command line can not be gotten!",
                nExitCode == 10 );
        }
     void getFrom_005_2()
            {
               oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "2" );
            CPPUNIT_ASSERT_MESSAGE("Parameters passed by .ini/rc file can not be gotten!",
                nExitCode == 20 );
        }
     void getFrom_005_3()
            {
#if (defined WNT) || (defined SOLARIS)
        putenv(const_cast< char * >("QADEV_BOOTSTRAP=sun&ms"));
#else
                setenv("QADEV_BOOTSTRAP", "sun&ms", 0);
#endif
               oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "3" );
            CPPUNIT_ASSERT_MESSAGE("Parameters passed by environment variables can not be gotten!",
                nExitCode == 30 );
        }
    void getFrom_005_4()
            {
                oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "4" );
            CPPUNIT_ASSERT_MESSAGE("Parameters passed by customed .ini/rc file can not be gotten!",
                nExitCode == 40 );
        }
    void getFrom_005_5()
            {
                oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "5" );
            CPPUNIT_ASSERT_MESSAGE("Parameters passed by inheritance can not be gotten!",
                nExitCode == 50 );
        }
    void getFrom_005_6()
            {
                oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "6" );
            CPPUNIT_ASSERT_MESSAGE("Parameters passed by default can not be gotten!",
                nExitCode == 60 );
        }

        CPPUNIT_TEST_SUITE(getFrom);
        CPPUNIT_TEST(getFrom_001);
        CPPUNIT_TEST(getFrom_002);
        CPPUNIT_TEST(getFrom_003);
        CPPUNIT_TEST(getFrom_004);
        CPPUNIT_TEST(getFrom_004_1);
        CPPUNIT_TEST(getFrom_005_1);
        CPPUNIT_TEST(getFrom_005_2);
        CPPUNIT_TEST(getFrom_005_3);
        CPPUNIT_TEST(getFrom_005_4);
        CPPUNIT_TEST(getFrom_005_5);
        CPPUNIT_TEST(getFrom_005_6);
        CPPUNIT_TEST_SUITE_END();
    }; // class getFrom

    class setIniFilename : public CppUnit::TestFixture
    {
    public:
        // initialise your test code values here.
        void setUp()
            {
            }

        void tearDown()
            {
            }

        void setIniFilename_001()
            {
                Bootstrap aBootstrap;

                rtl::OUString suGetIniname;
                aBootstrap.getIniName( suGetIniname );
                //which should be .....testshl2rc

                rtl::OUString suIniname = t_getSourcePath(PSEUDO_INI);
                Bootstrap::setIniFilename( suIniname );

                rtl::OUString suGetname;
                aBootstrap.getIniName( suGetname );

                printUString( suGetname );
                CPPUNIT_ASSERT_MESSAGE("setIniFilename then get it.", suGetname.compareTo( suIniname ) == 0
                                       && suGetname.compareTo( suGetIniname ) != 0 );
            }

        void setIniFilename_002()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);

                Bootstrap::setIniFilename( suIniname );
                rtl::OUString suGetname;
                rtl::OUString suValuename("INHERITED_VALUE");
                Bootstrap::get( suValuename, suGetname  );
                printUString( suGetname );
                CPPUNIT_ASSERT_MESSAGE("setIniFilename and get value of the argument.", !suGetname.isEmpty());
            }

        CPPUNIT_TEST_SUITE(setIniFilename);
        CPPUNIT_TEST(setIniFilename_001);
        CPPUNIT_TEST(setIniFilename_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class setIniFilename

    class getHandle : public CppUnit::TestFixture
    {
    public:
        // initialise your test code values here.
        void setUp()
            {
            }

        void tearDown()
            {
            }

        void getHandle_001()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                Bootstrap aBootstrap;
                rtlBootstrapHandle bsHandle = aBootstrap.getHandle();
                CPPUNIT_ASSERT_MESSAGE("getHandle should return 0 if the bootstrap has no ini file!", bsHandle == 0 );
            }
        void getHandle_002()
            {
                rtl::OUString suIniname = t_getSourcePath(PSEUDO_INI);
                Bootstrap aBootstrap( suIniname );

                rtlBootstrapHandle bsHandle = aBootstrap.getHandle();
                CPPUNIT_ASSERT_MESSAGE("getHandle return NULL!", bsHandle != 0);

                rtl::OUString suValue;
                rtl::OUString suKeyName("PSEUDOFILE");
                rtl_bootstrap_get_from_handle(bsHandle, suKeyName.pData, &suValue.pData, NULL);
                printUString( suValue);

                CPPUNIT_ASSERT_MESSAGE( "Can not use the handle which is returned by getHandle!", suValue == "be pseudo" );
            }

        CPPUNIT_TEST_SUITE(getHandle);
        CPPUNIT_TEST(getHandle_001);
        CPPUNIT_TEST(getHandle_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class getHandle

    class set : public CppUnit::TestFixture
    {
    public:
        // initialise your test code values here.
        void setUp()
            {
            }

        void tearDown()
            {
            }

        void set_001()
            {
                //in ini fle, INHERITED_VALUE=inherited_value
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                Bootstrap aBootstrap( suIniname);
                rtl::OUString suName("INHERITED_VALUE");
                rtl::OUString suValue("ok");
                // set to another value
                Bootstrap::set( suName, suValue );
                rtl::OUString suGetValue;
                Bootstrap::get( suName, suGetValue);
                CPPUNIT_ASSERT_MESSAGE("set and get argument failed.", suGetValue.compareTo(suValue) == 0 );
            }
        void set_002()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                Bootstrap myBootstrap( suIniname);
                rtl::OUString suName("INHERITED_VALUE");
                rtl::OUString suGetOrientValue;
                Bootstrap::get( suName, suGetOrientValue);
                // ??  INHERITED_VALUE = ok now, which is set in set_001
                printUString( suGetOrientValue );

                rtl::OUString suValue( TESTSHL2_INI );
                // set to another value
                Bootstrap::set( suName, suValue );
                rtl::OUString suGetValue;
                Bootstrap::get( suName, suGetValue);
                CPPUNIT_ASSERT_MESSAGE("set and get argument failed.", suGetValue.compareTo(suValue) == 0 );
            }

        CPPUNIT_TEST_SUITE(set);
        CPPUNIT_TEST(set_001);
        CPPUNIT_TEST(set_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class set

    class expandMacrosFrom : public CppUnit::TestFixture
    {
    public:
        void setUp()
            {
            }

        void tearDown()
            {
            }
        void expandMacrosFrom_001()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                Bootstrap aBootstrap( suIniname);
                rtl::OUString suMacro("$MYVAR/expand1");
                //expandMacro now
                aBootstrap.expandMacrosFrom( suMacro );
                rtl::OUString suExpectedMacro("src680_test/expand1");
                CPPUNIT_ASSERT_MESSAGE("expandMacrosFrom failed.", suMacro.compareTo(suExpectedMacro) == 0 );
            }

        /** here a special macro should expand
          * if rtlrc is under sal/qa/rtl/bootstrap/, "${rtlrc:Bootstrap:RTLVALUE}" could be expanded
          * else rtlrc is under solver/680/unxlngi4.pro/bin/, "${file:/// ....solver/680/unxlngi4.pro/bin/rtlrc:Bootstrap:RTLVALUE}"
      * could not be expanded
      */
        void expandMacrosFrom_002()
            {
                // Build a string with '${rtl.ini:RTLVALUE}' and try to expand it.
                // In function 'create_rtlrc() is the content of the rtl.ini file.

                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                t_print("inifile is:");
                printUString( suIniname );
                Bootstrap aBootstrap( suIniname) ;
                  rtl::OUString suMacro("${");         //rtlrc:Bootstrap:RTLVALUE}");

     rtl::OUString aDirURL("$ORIGIN");
     aDirURL += "/rtl";
#if defined(WNT)
    aDirURL += rtl::OUString(".ini");
#else
    aDirURL += rtl::OUString("rc");
#endif

                suMacro += aDirURL;//t_getSourcePath("rtl");
                suMacro += rtl::OUString("::RTLVALUE}");

                t_print("created macro is: ");
                printUString( suMacro );
                aBootstrap.expandMacrosFrom( suMacro );
                t_print("expanded macro is:");
                printUString( suMacro );
                rtl::OUString suExpectedMacro("qadev17");
                CPPUNIT_ASSERT_MESSAGE("failed, can't expand '${file:///.../" SAL_CONFIGFILE("rtl") "::RTLVALUE}' to 'qadev17'", suMacro.compareTo(suExpectedMacro) == 0 );
            }
        void expandMacrosFrom_002_1()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                t_print("inifile is:");
                printUString( suIniname );
                Bootstrap aBootstrap( suIniname);

                rtl::OUString suMacro;
                // just a simple test, if this really work.
                aBootstrap.getFrom(rtl::OUString("SOFROMVALUE2"), suMacro );
                t_print("SOFROMVALUE2:");
                printUString( suMacro );
                CPPUNIT_ASSERT_MESSAGE("'SOFROMVALUE2' seems to do not exist.", suMacro.getLength() > 0 );

                aBootstrap.getFrom(rtl::OUString("SOFROMVALUE"), suMacro );

                t_print("SOFROMVALUE:");
                printUString( suMacro );

                //expandMacro now
                // seems to be, that getFrom() already expand the string
                rtl::OUString suExpectedMacro("src680_qadev");
                CPPUNIT_ASSERT_MESSAGE("failed, can't expand '${" SAL_CONFIGFILE("rtl") "::SOVALUE}' to 'src680_qadev'", suMacro.compareTo(suExpectedMacro) == 0 );
            }
        void expandMacrosFrom_002_2()
            {
                // test, to read and expand SOFROMVALUE3
                // SOFROMVALUE3 is 'rtl(.ini|rc)::TESTSHL_SOVALUE' which should expand to 'rtlfile' if all is ok.

                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                t_print("inifile is:");
                printUString( suIniname );
                Bootstrap aBootstrap( suIniname);

                rtl::OUString suMacro;
                aBootstrap.getFrom(rtl::OUString("SOFROMVALUE3"), suMacro );

                t_print("SOFROMVALUE3:");
                printUString( suMacro );

                if ( suMacro == "testshl2_file" )
                {
                    CPPUNIT_ASSERT_MESSAGE("Value 'SOFROMVALUE3' is read from the wrong ini file.", 0 );
                }
                else
                {
                    CPPUNIT_ASSERT_MESSAGE( "SOFROMVALUE3 should contain 'rtlfile'.", suMacro == "rtlfile" );
                }
            }
        void expandMacrosFrom_003()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                Bootstrap aBootstrap( suIniname);
                rtl::OUString suMacro[4];
                suMacro[0] = rtl::OUString("$SYSUSERCONFIG");
                suMacro[1] = rtl::OUString("$SYSUSERHOME");
                suMacro[2] = rtl::OUString("$SYSBINDIR");
                suMacro[3] = rtl::OUString("$ORIGIN");

                for ( int i = 0; i < 4; i++ )
                {
                    aBootstrap.expandMacrosFrom( suMacro[i] );
                    printUString(suMacro[i]);
                }
                CPPUNIT_ASSERT_MESSAGE("some integral variables.", suMacro[0].getLength() > 0 &&
                                       suMacro[1].getLength() > 0 &&
                                       suMacro[2].getLength() > 0 &&
                                       suMacro[3].getLength() > 0);
            }

        void testRecursion() {
            rtl::OUString t("$RECURSIVE");
            Bootstrap(t_getSourcePath(TESTSHL2_INI)).expandMacrosFrom(t);
            CPPUNIT_ASSERT_MESSAGE( "recursion detection", t == "***RECURSION DETECTED***" );
        }

        void testLink() {
            rtl::OUString t("$LINKED");
            Bootstrap(t_getSourcePath(TESTSHL2_INI)).expandMacrosFrom(t);
            CPPUNIT_ASSERT_MESSAGE( "link file", t == "qadev17");
        }

        void testOverride() {
            rtl::OUString t1( "${.override:$ORIGIN/" SAL_CONFIGFILE("rtl" ":ORIGIN}"));
            Bootstrap(t_getSourcePath("rtl")).expandMacrosFrom(t1);
            CPPUNIT_ASSERT_MESSAGE( "override ORIGIN", t1 == "direct" );
            rtl::OUString t2( "${.override:$ORIGIN/" SAL_CONFIGFILE("none" ":MYVAR}"));
            Bootstrap::expandMacros(t2);
            CPPUNIT_ASSERT_MESSAGE( "override MYVAR", t2 == "src680_test" );
        }

        void testNonexisting() {
            rtl::OUString t( "${$ORIGIN/" SAL_CONFIGFILE("none" ":MYVAR}"));
            Bootstrap::expandMacros(t);
            CPPUNIT_ASSERT_MESSAGE( "nonexisting", t == "src680_test" );
        }

        void testSection() {
            rtl::OUStringBuffer b;
            b.appendAscii(RTL_CONSTASCII_STRINGPARAM("${"));
            rtl::OUString p(t_getSourcePath(TESTSHL2_INI));
            for (sal_Int32 i = 0; i < p.getLength(); ++i) {
                if (p[i] != 'u') {
                    b.append(static_cast< sal_Unicode >('\\'));
                }
                b.append(p[i]);
            }
            b.appendAscii(RTL_CONSTASCII_STRINGPARAM(":Other_Section:EXPAND}"));
            rtl::OUString t(b.makeStringAndClear());
            Bootstrap(t_getSourcePath(TESTSHL2_INI)).expandMacrosFrom(t);
            CPPUNIT_ASSERT_MESSAGE( "section expansion", t == "$FILE" );
                // the correct answer would be "testshl2 file" instead, but
                // expansion including a section currently erroneously does not
                // recursively expand macros in the resulting replacement text
        }

        CPPUNIT_TEST_SUITE(expandMacrosFrom);
        CPPUNIT_TEST(expandMacrosFrom_001);
        CPPUNIT_TEST(expandMacrosFrom_002);
        CPPUNIT_TEST(expandMacrosFrom_002_1);
        CPPUNIT_TEST(expandMacrosFrom_002_2);
        CPPUNIT_TEST(expandMacrosFrom_003);
        CPPUNIT_TEST(testRecursion);
        CPPUNIT_TEST(testLink);
        CPPUNIT_TEST(testOverride);
        CPPUNIT_TEST(testNonexisting);
        CPPUNIT_TEST(testSection);
        CPPUNIT_TEST_SUITE_END();
    }; // class expandMacrosFrom

    class expandMacros : public CppUnit::TestFixture
    {
    public:
        // initialise your test code values here.
        void setUp()
            {
            }

        void tearDown()
            {
            }

        void expandMacros_001()
            {
                rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
                Bootstrap aBootstrap( suIniname) ;
                rtl::OUString suMacro("$INHERITED_VALUE/well");
                Bootstrap::expandMacros( suMacro );

        rtl::OUString suName("INHERITED_VALUE");
                OUString suGetValue;
                Bootstrap::get( suName, suGetValue );
                suGetValue += "/well";
                CPPUNIT_ASSERT_MESSAGE("expandMacros failed.", suGetValue.compareTo(suMacro) == 0 );
            }

        CPPUNIT_TEST_SUITE(expandMacros);
        CPPUNIT_TEST(expandMacros_001);
        //  CPPUNIT_TEST(expandMacros_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class expandMacros

    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Bootstrap::ctor, "rtl_Bootstrap");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Bootstrap::getFrom, "rtl_Bootstrap");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Bootstrap::setIniFilename, "rtl_Bootstrap");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Bootstrap::getHandle, "rtl_Bootstrap");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Bootstrap::set, "rtl_Bootstrap");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Bootstrap::expandMacrosFrom, "rtl_Bootstrap");
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_Bootstrap::expandMacros, "rtl_Bootstrap");

} // namespace rtl_Bootstrap

// -----------------------------------------------------------------------------

static void removeAndCreateFile(rtl::OUString const& _suFileURL, rtl::OString const& _sContent)
{
    osl::File::remove(_suFileURL);

    ::std::auto_ptr<osl::File> pFile( new osl::File( _suFileURL ) );
    ::osl::FileBase::RC nError = pFile->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if ( ::osl::FileBase::E_None == nError || ::osl::FileBase::E_EXIST == nError )
    {
        t_print(T_VERBOSE, "%s\n" , OString(_suFileURL, _suFileURL.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
        sal_uInt64 nWritenBytes;
        pFile->write(_sContent.getStr(), _sContent.getLength(), nWritenBytes);

        rtl::OString sError = "can't write enough bytes to file";
        sError += OString(_suFileURL, _suFileURL.getLength(), RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(nWritenBytes == _sContent.getLength(), sError.getStr());

        pFile->close();
    }
    else
    {
        rtl::OString sError = "can't create file URL: '";
        rtl::OString sFile;
        sFile <<= _suFileURL;
        sError += sFile;
        sError += "' maybe no write access. If it is true with no write access, please create a local environment and start these tests again. rtl::Bootstrap test must quit.";
        t_print("%s\n", sError.getStr() );
        exit(1);
    }
    OSL_ASSERT(t_fileExist(_suFileURL) == true);
}

// -----------------------------------------------------------------------------
static void create_rtlrc()
{
    rtl::OUString aFileURL(getExecutableDirectory());
#if defined(WNT)
    aFileURL += rtl::OUString("/rtl.ini");
#else
    aFileURL += rtl::OUString("/rtlrc");
#endif

    rtl::OString sLines;
    sLines += "[Bootstrap]\n";
    sLines += "SOVALUE=src680_qadev\n";
    sLines += "RTLVALUE=qadev17\n";
    sLines += "TESTSHL_SOVALUE=rtlfile\n";
    sLines += "RECURSIVE=${$ORIGIN/" SAL_CONFIGFILE("testshl2") ":RECURSIVE}\n";
    sLines += "ORIGIN=direct\n";
    sLines += "[Other_Section]\n";
    sLines += "TESTSHL_SOVALUE=rtlfile_other\n";

    removeAndCreateFile(aFileURL, sLines);
}

// -----------------------------------------------------------------------------
static void create_testshl2rc()
{
    rtl::OUString aFileURL(getExecutableDirectory());
#if defined(WNT)
    aFileURL += rtl::OUString("/testshl2.ini");
#else
    aFileURL += rtl::OUString("/testshl2rc");
#endif
    rtl::OString sLines;
    sLines += "[Bootstrap]\n";
    sLines += "FILE=testshl2 file\n";
    sLines += "MYBOOTSTRAPTESTVALUE=file\n";
    sLines += "INHERITED_VALUE=inherited_value\n";
    sLines += "INHERITED_OVERWRITTEN_VALUE=not_overwritten\n";
    sLines += "MYVAR=src680_test\n";
    sLines += "SOFROMVALUE=${$ORIGIN/" SAL_CONFIGFILE("rtl") "::SOVALUE}\n";
    sLines += "SOFROMVALUE2=test\n";
    sLines += "SOFROMVALUE3=${$ORIGIN/" SAL_CONFIGFILE("rtl") "::TESTSHL_SOVALUE}\n";
    sLines += "TESTSHL_SOVALUE=testshl2_file\n";
    sLines += "ILLEGAL VALUE=test\n";
    sLines += "ILLEGAL.DOT=test\n";
    sLines += "ILLEGAL;SEMICOLON=test\n";
    sLines += "ILLEGAL:COLON=test\n";
    sLines += "  KEY_FOR_TRIM_TEST  =   value for trim test    \n";
    sLines += "RECURSIVE=${$ORIGIN/" SAL_CONFIGFILE("rtl") ":RECURSIVE}\n";
    sLines += "LINKED=${${.link:$ORIGIN/testshl2-link}:RTLVALUE}\n";
    sLines += "[Other_Section]\n";
    sLines += "FILE=testshl2 file other\n";
    sLines += "EXPAND=$FILE\n";

    removeAndCreateFile(aFileURL, sLines);

    removeAndCreateFile(
        (getExecutableDirectory() +
         rtl::OUString("/testshl2-link")),
        SAL_CONFIGFILE("rtl"));
}

// -----------------------------------------------------------------------------

static void create_pseudorc()
{
    rtl::OUString aFileURL(getExecutableDirectory());
#if defined(WNT)
    aFileURL += rtl::OUString("/pseudo.ini");
#else
    aFileURL += rtl::OUString("/pseudorc");
#endif
    rtl::OString sLines;
    sLines += "[Bootstrap]\n";
    sLines += "FILE=pseudo file\n";
    sLines += "PSEUDOFILE=be pseudo\n";

    removeAndCreateFile(aFileURL, sLines);
}

// -----------------------------------------------------------------------------
void create_bootstrap_processrc()
{
    rtl::OUString aDirURL(getModulePath());
#if defined(WNT)
    aDirURL += rtl::OUString("/bootstrap_process.ini");
#else
    aDirURL += rtl::OUString("/bootstrap_processrc");
#endif
    rtl::OString sLines;
    sLines += "[Bootstrap]\n";
    sLines += "EXECUTABLE_RC=true\n";
    sLines += "IF_CUSTOM_RC=false\n";

    removeAndCreateFile(aDirURL, sLines);
}
// -----------------------------------------------------------------------------

void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)
{
    (void) _pFunc;
    // start message
    t_print(T_VERBOSE, "Initializing ...\n" );
    create_rtlrc();
    create_testshl2rc();
    create_pseudorc();
    create_bootstrap_processrc();

    t_print(T_VERBOSE, "Initialization Done.\n" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
