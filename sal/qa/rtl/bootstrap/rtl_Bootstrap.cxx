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

// Documentation about bootstraping can be found at:
// http://udk.openoffice.org/common/man/concept/micro_deployment.html

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> // STL

#ifdef WNT
#define WIN32_LEAN_AND_MEAN
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#endif

#include "gtest/gtest.h"
//#include "stringhelper.hxx"
//#include "valueequal.hxx"
#include <rtl/bootstrap.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <osl/process.h> /* osl_getExecutableFile() */

#include <osl/thread.hxx>

// using namespace osl;
using namespace rtl;

#define TESTSHL2_INI "testshl2"
#define PSEUDO_INI   "pseudo"

/** print a UNI_CODE String. And also print some comments of the string.
 */
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

/** get the exectutable path ( here is bootstrap_process), on Linux, such as "sal/unxlngi4.pro/bin/"
  */
inline ::rtl::OUString getModulePath( void )
{
    ::rtl::OUString suDirPath;
    ::osl::Module::getUrlFromAddress( ( oslGenericFunction ) &getModulePath, suDirPath );

    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') );
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') + 1);
    suDirPath += rtl::OUString::createFromAscii("bin");
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
     aDirURL += OUString::createFromAscii( "/");
     aDirURL += OUString::createFromAscii( _sFilename.getStr() );
#if defined(WNT) || defined(OS2)
    aDirURL += rtl::OUString::createFromAscii(".ini");
#else
    aDirURL += rtl::OUString::createFromAscii("rc");
#endif
    return aDirURL;

// LLA: does not right work on my personal laptop, SRC_ROOT does not show where the source is :-(.
/*
  sal_Char *  pStr = getenv("SRC_ROOT");
  rtl::OUString suPath;
  if (filename != "")
  {
  suPath = rtl::OUString::createFromAscii(pStr) + rtl::OUString::createFromAscii( "/sal/qa/rtl/bootstrap/" )
  + rtl::OUString::createFromAscii( filename );
  }
  else
  {
  suPath = rtl::OUString::createFromAscii(pStr) + rtl::OUString::createFromAscii( "/sal/qa/rtl/bootstrap" );
  }
  rtl::OUString suURL;
  ::osl::FileBase::getFileURLFromSystemPath( suPath, suURL );
  return suURL;
*/
}

void thread_sleep_tenth_sec(sal_Int32 _nTenthSec)
{
#ifdef WNT      //Windows
    Sleep(_nTenthSec * 100 );
#endif
#if ( defined UNX ) || ( defined OS2 )  //Unix
    TimeValue nTV;
    nTV.Seconds = static_cast<sal_uInt32>( _nTenthSec/10 );
    nTV.Nanosec = ( (_nTenthSec%10 ) * 100000000 );
    osl_waitThread(&nTV);
#endif
}

// -----------------------------------------------------------------------------

namespace rtl_Bootstrap
{
    class ctor : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
        {
        }

        void TearDown()
        {
        }
    }; // class ctor

    // ctor with ini name
    TEST_F(ctor, ctor_001)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        printUString( suIniname );
        Bootstrap aBootstrap( suIniname );
        rtl::OUString suGetname; // = rtl::OUString::createFromAscii("");
        aBootstrap.getIniName( suGetname );
        printUString( suGetname );

        // LLA: first: this seems to be a wrong test.
        //      second: there seems to be a design hole, if I give a absolute path ini file,
        //              but try to use ${file::KEYVALUE} than 'file' will only used out of the 'executable path'/file
        //              not from the path given from the absolute path.

        // Due to the fact, we create at this position a file (createTestshl2rc() ), we check for existence
        bool bFileExist = t_fileExist( suGetname );
        ASSERT_TRUE(bFileExist == true) << "ctor error with initial file.";
    }

    TEST_F(ctor, ctor_002)
    {
        rtl::Bootstrap aBootstrap;
        rtl::OUString suGetname;
        aBootstrap.getIniName( suGetname );
        printUString( suGetname );
        ASSERT_TRUE(suGetname.getLength() != 0) << "ctor error without initial file.";
    }

    class getFrom : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
        {
        }

        void TearDown()
        {
        }

    /** helper function: return the child process's ret value( typedef sal_uInt32 oslProcessExitCode;)
       * param1 is the process's name(only file name, not include path)
      */
     oslProcessExitCode ini_execProcess( const sal_Char* process_name, const sal_Char * flag )
     {
         rtl::OUString suCWD = getModulePath();
            oslProcess hProcess = NULL;
           rtl::OUString suFileURL = suCWD;
            suFileURL += rtl::OUString::createFromAscii("/") +  rtl::OUString::createFromAscii(process_name) ;
#if defined(WNT) || defined(OS2)
        suFileURL += rtl::OUString::createFromAscii(".exe");
#endif
            const int nParameterCount = 3;
            rtl_uString* pParameters[ nParameterCount ];
            OUString suFlag( OUString::createFromAscii(flag) );
        OUString suEnv1( OUString::createFromAscii("-env:UNO_SERVICES=service.rdb"));
        OUString suIniname = t_getSourcePath("rtl");
        printUString( suIniname, "rtl path:");
        //OUString suEnv2( OUString::createFromAscii("-env:MYENV=bootstrap_process"));

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

            EXPECT_TRUE(osl_error == osl_Process_E_None)
                << "osl_createProcess failed";
               osl_joinProcess(hProcess);
            oslProcessInfo* pInfo = new oslProcessInfo;
        pInfo->Size = sizeof( oslProcessInfo );
        osl_error = osl_getProcessInfo( hProcess, osl_Process_EXITCODE, pInfo );
        EXPECT_TRUE(osl_Process_E_None == osl_error)
                << "osl_getProcessInfo returned with failure";

        printf("the exit code is %d.\n", pInfo->Code );
        oslProcessExitCode nCode = pInfo->Code;
        delete pInfo;
         return nCode;
    }

    }; // class getFrom

    // get the value of env variable
    TEST_F(getFrom, getFrom_001)
        {
            Bootstrap aBootstrap;
            rtl::OUString suValue;
            rtl::OUString suValuename = rtl::OUString::createFromAscii( "SOLAR_JAVA" );
            //aBootstrap.getFrom( suValuename, suValue );
            aBootstrap.getFrom( suValuename, suValue );
            sal_Char *  pStr = getenv("SOLAR_JAVA");
            //      printUString( suGetname );
            ASSERT_TRUE(suValue.compareToAscii( pStr ) == 0) << "get the value of environment variable.";
        }
    /* Notes on Windows:
       void getFrom_001_1()
       {
       Bootstrap aBootstrap;
       rtl::OUString suValue;
       rtl::OUString suValuename = rtl::OUString::createFromAscii( "SRC_ROOT" );
       //aBootstrap.getFrom( suValuename, suValue );
       aBootstrap.getFrom( suValuename, suValue );
       sal_Char *  pStr = getenv("SRC_ROOT");
       //   printUString( suGetname );
       ASSERT_TRUE(suValue.compareToAscii( pStr ) == 0) << "get the value of environment variable.";
       }
       The result on Windows:
       # # the SRC_ROOT is e:\Qadev\cvs\m19
       # # suValue is e:Qadevcvsm19
       reason:
       The problem is that the internally getenv()ed variable SRC_ROOT is macro expanded,
       thus every \ will introduce an escape.
    */

    // get the value of a variable in ini file
    TEST_F(getFrom, getFrom_002)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        Bootstrap aBootstrap( suIniname );
        rtl::OUString suGetname;
        rtl::OUString suValuename = rtl::OUString::createFromAscii( "INHERITED_VALUE" );
        aBootstrap.getFrom( suValuename, suGetname );
        printUString( suGetname );
        ASSERT_TRUE(suGetname.getLength() != 0) << "get the value of a variable in ini file.";
    }

    //use defaut value
    TEST_F(getFrom, getFrom_003)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        Bootstrap aBootstrap( suIniname );
        rtl::OUString suGetname;
        rtl::OUString suValuename = rtl::OUString::createFromAscii( "MY_VALUE" );
        rtl::OUString myDefault = rtl::OUString::createFromAscii( "2" );
        aBootstrap.getFrom( suValuename, suGetname, myDefault );
        //printUString( suGetname );
        ASSERT_TRUE(suGetname.compareTo( myDefault ) == 0) << "getFrom use default.";
    }

    TEST_F(getFrom, getFrom_004)
    {
        printf("1\n");
        // initialise Bootstrap with an own ini file
        // PSEUDO_INI is pseudo(rc|.ini) created be create_pseudorc()
        rtl::OUString suIniname = t_getSourcePath(PSEUDO_INI);
        Bootstrap aBootstrap( suIniname );

        rtl::OUString suGetIniName;
        aBootstrap.getIniName( suGetIniName );

        printUString(suGetIniName, "Current bootstrap file");
        sal_Int32 nIndex = suGetIniName.indexOf(rtl::OUString::createFromAscii( "pseudo" ));
        ASSERT_TRUE(nIndex > 0) << "ini name must have 'pseudo' in name.";

        // rtlBootstrapHandle bsHandle = aBootstrap.getHandle();
        // ASSERT_TRUE(bsHandle != 0) << "getHandle return NULL!";

        rtl::OUString suValue;
        rtl::OUString suKeyName = rtl::OUString::createFromAscii( "FILE" );
        aBootstrap.getFrom( suKeyName, suValue );
        printUString( suValue );
        sal_Int32 nCompare = suValue.compareTo( rtl::OUString::createFromAscii("pseudo file") );

        ASSERT_TRUE(nCompare == 0)
            << "<Bootstrap('pseudo')>.getFrom('FILE', ...) result is unexpected.";
    }

    TEST_F(getFrom, getFrom_004_1)
    {
        // get the same key out of the default context
        rtl::OUString suKeyName = rtl::OUString::createFromAscii( "FILE" );
        rtl::OUString suGetValue;
        Bootstrap::get( suKeyName, suGetValue );
        printUString( suGetValue );

        ASSERT_TRUE(suGetValue.compareTo( rtl::OUString::createFromAscii("testshl2 file") ) == 0)
            << "Bootstrap::get('FILE', ...)";
    }

    TEST_F(getFrom, getFrom_005_1)
    {
        oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "1" );
        ASSERT_TRUE(nExitCode == 10) << "Parameters passed by command line can not be gotten!";
    }
    TEST_F(getFrom, getFrom_005_2)
    {
        oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "2" );
        ASSERT_TRUE(nExitCode == 20) << "Parameters passed by .ini/rc file can not be gotten!";
    }
    TEST_F(getFrom, getFrom_005_3)
    {
#if (defined WNT) || (defined SOLARIS)
        putenv(const_cast< char * >("QADEV_BOOTSTRAP=sun&ms"));
#else
        setenv("QADEV_BOOTSTRAP", "sun&ms", 0);
#endif
        oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "3" );
        ASSERT_TRUE(nExitCode == 30) << "Parameters passed by environment variables can not be gotten!";
    }
    TEST_F(getFrom, getFrom_005_4)
    {
        oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "4" );
        ASSERT_TRUE(nExitCode == 40) << "Parameters passed by customed .ini/rc file can not be gotten!";
    }
    TEST_F(getFrom, getFrom_005_5)
    {
        oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "5" );
        ASSERT_TRUE(nExitCode == 50) << "Parameters passed by inheritance can not be gotten!";
    }
    TEST_F(getFrom, getFrom_005_6)
    {
        oslProcessExitCode nExitCode = ini_execProcess( "bootstrap_process", "6" );
        ASSERT_TRUE(nExitCode == 60) << "Parameters passed by default can not be gotten!";
    }

    class setIniFilename : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
        {
        }

        void TearDown()
        {
        }
    }; // class setIniFilename

    TEST_F(setIniFilename, setIniFilename_001)
    {
        Bootstrap aBootstrap;

        rtl::OUString suGetIniname;
        aBootstrap.getIniName( suGetIniname );
        //which should be .....testshl2rc
        //printUString( suGetIniname );

        rtl::OUString suIniname = t_getSourcePath(PSEUDO_INI);
        Bootstrap::setIniFilename( suIniname );

        rtl::OUString suGetname;
        aBootstrap.getIniName( suGetname );

        printUString( suGetname );
        ASSERT_TRUE(suGetname.compareTo( suIniname ) == 0
                               && suGetname.compareTo( suGetIniname ) != 0) << "setIniFilename then get it.";
    }

    TEST_F(setIniFilename, setIniFilename_002)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        // ASSERT_TRUE(t_fileExist(suIniname ) == true) <<
        //     "test failed, Bootstrap ini does not exist."

        Bootstrap::setIniFilename( suIniname );
        //rtl_bootstrap_args_open( suIniname.pData );
        rtl::OUString suGetname;
        rtl::OUString suValuename = rtl::OUString::createFromAscii( "INHERITED_VALUE" );
        //aBootstrap.getFrom( suValuename, suGetname  );
        Bootstrap::get( suValuename, suGetname  );
        printUString( suGetname );
        ASSERT_TRUE(suGetname.getLength() != 0) << "setIniFilename and get value of the argument.";
    }

    class getHandle : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
        {
        }

        void TearDown()
        {
        }
    }; // class getHandle

    TEST_F(getHandle, getHandle_001)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        Bootstrap aBootstrap;
        rtlBootstrapHandle bsHandle = aBootstrap.getHandle();
        ASSERT_TRUE(bsHandle == 0) << "getHandle should return 0 if the bootstrap has no ini file!";
    }
    TEST_F(getHandle, getHandle_002)
    {
        rtl::OUString suIniname = t_getSourcePath(PSEUDO_INI);
        Bootstrap aBootstrap( suIniname );

        rtlBootstrapHandle bsHandle = aBootstrap.getHandle();
        ASSERT_TRUE(bsHandle != 0) << "getHandle return NULL!";

        //rtl::OUString iniName;
        //rtl_bootstrap_get_iniName_from_handle( bsHandle, &iniName.pData );

        rtl::OUString suValue;
        rtl::OUString suKeyName = rtl::OUString::createFromAscii( "PSEUDOFILE" );
        rtl_bootstrap_get_from_handle(bsHandle, suKeyName.pData, &suValue.pData, NULL);
        printUString( suValue);

        ASSERT_TRUE(suValue.equals( rtl::OUString::createFromAscii("be pseudo") ) == sal_True) << "Can not use the handle which is returned by getHandle!";

        // ASSERT_TRUE(//      suGetname.equalsIgnoreAsciiCase( iniName ) == sal_True) << "Can not use the handle which is returned by getHandle!";
    }

    class set : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
        {
        }

        void TearDown()
        {
        }
    }; // class set

    TEST_F(set, set_001)
    {
        //in ini fle, INHERITED_VALUE=inherited_value
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        Bootstrap aBootstrap( suIniname);
        rtl::OUString suName = rtl::OUString::createFromAscii( "INHERITED_VALUE" );
        rtl::OUString suValue = rtl::OUString::createFromAscii( "ok" );
        // set to another value
        Bootstrap::set( suName, suValue );
        rtl::OUString suGetValue;
        Bootstrap::get( suName, suGetValue);
        ASSERT_TRUE(suGetValue.compareTo(suValue) == 0) << "set and get argument failed.";
    }
    TEST_F(set, set_002)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        Bootstrap myBootstrap( suIniname);
        rtl::OUString suName = rtl::OUString::createFromAscii( "INHERITED_VALUE" );
        rtl::OUString suGetOrientValue;
        Bootstrap::get( suName, suGetOrientValue);
        // ??  INHERITED_VALUE = ok now, which is set in set_001
        printUString( suGetOrientValue );

        rtl::OUString suValue = rtl::OUString::createFromAscii( TESTSHL2_INI );
        // set to another value
        Bootstrap::set( suName, suValue );
        rtl::OUString suGetValue;
        Bootstrap::get( suName, suGetValue);
        ASSERT_TRUE(suGetValue.compareTo(suValue) == 0) << "set and get argument failed.";
    }

    class expandMacrosFrom : public ::testing::Test
    {
    public:
        void SetUp()
        {
        }

        void TearDown()
        {
        }
    }; // class expandMacrosFrom

    TEST_F(expandMacrosFrom, expandMacrosFrom_001)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        Bootstrap aBootstrap( suIniname);
        rtl::OUString suMacro = rtl::OUString::createFromAscii( "$MYVAR/expand1" );
        //printUString( suMacro );
        //expandMacro now
        aBootstrap.expandMacrosFrom( suMacro );
        rtl::OUString suExpectedMacro = rtl::OUString::createFromAscii( "src680_test/expand1" );
        //printUString( suMacro );
        ASSERT_TRUE(suMacro.compareTo(suExpectedMacro) == 0) << "expandMacrosFrom failed.";
    }

    /** here a special macro should expand
      * if rtlrc is under sal/qa/rtl/bootstrap/, "${rtlrc:Bootstrap:RTLVALUE}" could be expanded
      * else rtlrc is under solver/680/unxlngi4.pro/bin/, "${file:/// ....solver/680/unxlngi4.pro/bin/rtlrc:Bootstrap:RTLVALUE}"
  * could not be expanded
  */
    TEST_F(expandMacrosFrom, expandMacrosFrom_002)
    {
        // Build a string with '${rtl.ini:RTLVALUE}' and try to expand it.
        // In function 'create_rtlrc() is the content of the rtl.ini file.

        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        printf("inifile is:");
        printUString( suIniname );
        Bootstrap aBootstrap( suIniname) ;
        rtl::OUString suMacro = rtl::OUString::createFromAscii( "${" );         //rtlrc:Bootstrap:RTLVALUE}");

rtl::OUString aDirURL = OUString::createFromAscii( "$ORIGIN");
aDirURL += OUString::createFromAscii( "/");
aDirURL += OUString::createFromAscii( "rtl" );
#if defined(WNT) || defined(OS2)
aDirURL += rtl::OUString::createFromAscii(".ini");
#else
aDirURL += rtl::OUString::createFromAscii("rc");
#endif

        suMacro += aDirURL;//t_getSourcePath("rtl");
        suMacro += rtl::OUString::createFromAscii( "::RTLVALUE}");

        printf("created macro is: ");
        printUString( suMacro );
        //expandMacro now
        aBootstrap.expandMacrosFrom( suMacro );
        printf("expanded macro is:");
        printUString( suMacro );
        rtl::OUString suExpectedMacro = rtl::OUString::createFromAscii( "qadev17" );
        ASSERT_TRUE(suMacro.compareTo(suExpectedMacro) == 0 )
            << "failed, can't expand '${file:///.../" SAL_CONFIGFILE("rtl") "::RTLVALUE}' to 'qadev17'";
    }
    TEST_F(expandMacrosFrom, expandMacrosFrom_002_1)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        printf("inifile is:");
        printUString( suIniname );
        Bootstrap aBootstrap( suIniname);

        rtl::OUString suMacro;
        // just a simple test, if this really work.
        aBootstrap.getFrom(rtl::OUString::createFromAscii( "SOFROMVALUE2" ), suMacro );
        printf("SOFROMVALUE2:");
        printUString( suMacro );
        ASSERT_TRUE(suMacro.getLength() > 0) << "'SOFROMVALUE2' seems to do not exist.";

        aBootstrap.getFrom(rtl::OUString::createFromAscii( "SOFROMVALUE" ), suMacro );

        printf("SOFROMVALUE:");
        printUString( suMacro );

        //expandMacro now
        // seems to be, that getFrom() already expand the string
        // printf("expanded macro is:");
        // aBootstrap.expandMacrosFrom( suMacro );
        // printUString( suMacro );
        rtl::OUString suExpectedMacro = rtl::OUString::createFromAscii( "src680_qadev" );
        ASSERT_TRUE(suMacro.compareTo(suExpectedMacro) == 0)
            << "failed, can't expand '${" SAL_CONFIGFILE("rtl") "::SOVALUE}' to 'src680_qadev'";
    }
    TEST_F(expandMacrosFrom, expandMacrosFrom_002_2)
    {
        // test, to read and expand SOFROMVALUE3
        // SOFROMVALUE3 is 'rtl(.ini|rc)::TESTSHL_SOVALUE' which should expand to 'rtlfile' if all is ok.

        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        printf("inifile is:");
        printUString( suIniname );
        Bootstrap aBootstrap( suIniname);

        rtl::OUString suMacro;
        aBootstrap.getFrom(rtl::OUString::createFromAscii( "SOFROMVALUE3" ), suMacro );

        printf("SOFROMVALUE3:");
        printUString( suMacro );

        if (suMacro.equals(rtl::OUString::createFromAscii("testshl2_file") ) == sal_True)
        {
            ASSERT_TRUE(0) << "Value 'SOFROMVALUE3' is read from the wrong ini file.";
        }
        else
        {
            ASSERT_TRUE(suMacro.equals(rtl::OUString::createFromAscii("rtlfile") ) == sal_True) << "SOFROMVALUE3 should contain 'rtlfile'.";
        }
    }
//? I don't know if this is a right test.
//         void expandMacrosFrom_002_3()
//             {
//                 // test, to read and expand SOFROMVALUE4
//                 // SOFROMVALUE4 is 'rtl(.ini|rc):Other_Section:TESTSHL_SOVALUE' which should expand to '' if all is ok.
//
//                 rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
//                 printf("inifile is:");
//              printUString( suIniname );
//                 Bootstrap aBootstrap( suIniname);
//
//                 rtl::OUString suMacro;
//                 aBootstrap.getFrom(rtl::OUString::createFromAscii( "SOFROMVALUE4" ), suMacro );
//
//                 printf("SOFROMVALUE4:");
//                 printUString( suMacro );
//
//                 if (suMacro.equals(rtl::OUString::createFromAscii("testshl2_file") ) == sal_True)
//                 {
//                     ASSERT_TRUE(0) << "Value 'SOFROMVALUE4' is read from the wrong section out of the wrong ini file.";
//                 }
//                 else if (suMacro.equals(rtl::OUString::createFromAscii("testshl2_file_other") ) == sal_True)
//                 {
//                     ASSERT_TRUE(0) << "Value 'SOFROMVALUE4' is read from the wrong ini file.";
//                 }
//                 else
//                 {
//                     ASSERT_TRUE(suMacro.equals(rtl::OUString::createFromAscii("rtlfile_other") ) == sal_True) << "Value 'SOFROMVALUE4' should contain 'rtlfile_other'.";
//                 }
//             }
    TEST_F(expandMacrosFrom, expandMacrosFrom_003)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        Bootstrap aBootstrap( suIniname);
        rtl::OUString suMacro[4];
        suMacro[0] = rtl::OUString::createFromAscii( "$SYSUSERCONFIG" );
        suMacro[1] = rtl::OUString::createFromAscii( "$SYSUSERHOME" );
        suMacro[2] = rtl::OUString::createFromAscii( "$SYSBINDIR" );
        suMacro[3] = rtl::OUString::createFromAscii( "$ORIGIN" );

        for ( int i = 0; i < 4; i++ )
        {
            aBootstrap.expandMacrosFrom( suMacro[i] );
            printUString(suMacro[i]);
        }
        // printUString( t_getSourcePath("") );
        // ASSERT_TRUE(suMacro[3].equalsIgnoreAsciiCase(t_getSourcePath("")) == sal_True) << "some integral variables.";
        ASSERT_TRUE(suMacro[0].getLength() > 0 &&
                               suMacro[1].getLength() > 0 &&
                               suMacro[2].getLength() > 0 &&
                               suMacro[3].getLength() > 0) << "some integral variables.";
    }

    TEST_F(expandMacrosFrom, testRecursion) {
        rtl::OUString t(RTL_CONSTASCII_USTRINGPARAM("$RECURSIVE"));
        Bootstrap(t_getSourcePath(TESTSHL2_INI)).expandMacrosFrom(t);
        ASSERT_TRUE(t.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("***RECURSION DETECTED***"))) << "recursion detection";
    }

    TEST_F(expandMacrosFrom, testLink) {
        rtl::OUString t(RTL_CONSTASCII_USTRINGPARAM("$LINKED"));
        Bootstrap(t_getSourcePath(TESTSHL2_INI)).expandMacrosFrom(t);
        ASSERT_TRUE(t.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("qadev17"))) << "link file";
    }

    TEST_F(expandMacrosFrom, testOverride) {
        rtl::OUString t1(
            RTL_CONSTASCII_USTRINGPARAM(
                "${.override:$ORIGIN/" SAL_CONFIGFILE("rtl") ":ORIGIN}"));
        Bootstrap(t_getSourcePath("rtl")).expandMacrosFrom(t1);
        ASSERT_TRUE(t1.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("direct"))) << "override ORIGIN";
        rtl::OUString t2(
            RTL_CONSTASCII_USTRINGPARAM(
                "${.override:$ORIGIN/" SAL_CONFIGFILE("none") ":MYVAR}"));
        Bootstrap::expandMacros(t2);
        ASSERT_TRUE(t2.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("src680_test"))) << "override MYVAR";
    }

    TEST_F(expandMacrosFrom, testNonexisting) {
        rtl::OUString t(
            RTL_CONSTASCII_USTRINGPARAM(
                "${$ORIGIN/" SAL_CONFIGFILE("none") ":MYVAR}"));
        Bootstrap::expandMacros(t);
        ASSERT_TRUE(t.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("src680_test"))) << "nonexisting";
    }

    TEST_F(expandMacrosFrom, testSection) {
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
        ASSERT_TRUE(t.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("$FILE"))) << "section expansion";
            // the correct answer would be "testshl2 file" instead, but
            // expansion including a section currently erroneously does not
            // recursively expand macros in the resulting replacement text
    }

    class expandMacros : public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
        {
        }

        void TearDown()
        {
        }
    }; // class expandMacros


    TEST_F(expandMacros, expandMacros_001)
    {
        rtl::OUString suIniname = t_getSourcePath(TESTSHL2_INI);
        Bootstrap aBootstrap( suIniname) ;
        rtl::OUString suMacro = rtl::OUString::createFromAscii( "$INHERITED_VALUE/well" );
        Bootstrap::expandMacros( suMacro );

        rtl::OUString suName = rtl::OUString::createFromAscii( "INHERITED_VALUE" );
        OUString suGetValue;
        Bootstrap::get( suName, suGetValue );
        suGetValue += OUString::createFromAscii( "/well" );
        ASSERT_TRUE(suGetValue.compareTo(suMacro) == 0) << "expandMacros failed.";
    }
} // namespace rtl_Bootstrap

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
// NOADDITIONAL;


// Here are some helpers, which create a new file 'rtlrc' at the executable path position
// and fill the file with some information.
// static rtl::OUString getExecutableDirectory()
// {
//     rtl::OUString fileName;
//     osl_getExecutableFile(&fileName.pData);
//
//     sal_Int32 nDirEnd = fileName.lastIndexOf('/');
//
//     OSL_ENSURE(nDirEnd >= 0, "Cannot locate executable directory");
//
//     rtl::OUString aDirURL = fileName.copy(0, nDirEnd);
//     return aDirURL;
// }

static void removeAndCreateFile(rtl::OUString const& _suFileURL, rtl::OString const& _sContent)
{
    osl::File::remove(_suFileURL);

    ::std::auto_ptr<osl::File> pFile( new osl::File( _suFileURL ) );
    ::osl::FileBase::RC nError = pFile->open( OpenFlag_Write | OpenFlag_Create );
    if ( ::osl::FileBase::E_None == nError || ::osl::FileBase::E_EXIST == nError )
    {
        printf("%s\n" , OString(_suFileURL.getStr(), _suFileURL.getLength(), RTL_TEXTENCODING_ASCII_US).getStr());
        sal_uInt64 nWritenBytes;
        pFile->write(_sContent.getStr(), _sContent.getLength(), nWritenBytes);
        // printf("nBytes: %ld\n", nBytes);

        rtl::OString sError = "can't write enough bytes to file";
        sError += OString(_suFileURL.getStr(), _suFileURL.getLength(), RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(nWritenBytes == _sContent.getLength(), sError.getStr());

        pFile->close();
    }
    else
    {
        rtl::OString sError = "can't create file URL: '";
        rtl::OString sFile(_suFileURL.getStr(), _suFileURL.getLength(), RTL_TEXTENCODING_ASCII_US);
        sError += sFile;
        sError += "' maybe no write access. If it is true with no write access, please create a local environment and start these tests again. rtl::Bootstrap test must quit.";
        printf("%s\n", sError.getStr() );
        exit(1);
    }
    OSL_ASSERT(t_fileExist(_suFileURL) == true);
}

// -----------------------------------------------------------------------------
static void create_rtlrc()
{
    rtl::OUString aFileURL(getExecutableDirectory());
#if defined(WNT) || defined(OS2)
    aFileURL += rtl::OUString::createFromAscii("/rtl.ini");
#else
    aFileURL += rtl::OUString::createFromAscii("/rtlrc");
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
#if defined(WNT) || defined(OS2)
    aFileURL += rtl::OUString::createFromAscii("/testshl2.ini");
#else
    aFileURL += rtl::OUString::createFromAscii("/testshl2rc");
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
    //? sLines += "SOFROMVALUE4=${" SAL_CONFIGFILE("rtl") ":Other_Section:TESTSHL_SOVALUE}\n";
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
    //? sLines += "TESTSHL_SOVALUE=testshl2_file_other\n";

    removeAndCreateFile(aFileURL, sLines);

    removeAndCreateFile(
        (getExecutableDirectory() +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/testshl2-link"))),
        SAL_CONFIGFILE("rtl"));
}

// -----------------------------------------------------------------------------

static void create_pseudorc()
{
    rtl::OUString aFileURL(getExecutableDirectory());
#if defined(WNT) || defined(OS2)
    aFileURL += rtl::OUString::createFromAscii("/pseudo.ini");
#else
    aFileURL += rtl::OUString::createFromAscii("/pseudorc");
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
#if defined(WNT) || defined(OS2)
    aDirURL += rtl::OUString::createFromAscii("/bootstrap_process.ini");
#else
    aDirURL += rtl::OUString::createFromAscii("/bootstrap_processrc");
#endif
    rtl::OString sLines;
    sLines += "[Bootstrap]\n";
    sLines += "EXECUTABLE_RC=true\n";
    sLines += "IF_CUSTOM_RC=false\n";

    removeAndCreateFile(aDirURL, sLines);
}
// -----------------------------------------------------------------------------


int main(int argc, char **argv)
{
    // start message
    printf("Initializing ...\n" );
    osl_setCommandArgs(argc, argv);
    create_rtlrc();
    create_testshl2rc();
    create_pseudorc();
    create_bootstrap_processrc();

    printf("Initialization Done.\n" );
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
