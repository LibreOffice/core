// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx


// -----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef UNX
#   include <unistd.h>
#endif

#include <rtl/logfile.hxx>
#include <cppunit/simpleheader.hxx>

#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifdef WNT
#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#endif

using namespace ::osl;

inline void printUString( const ::rtl::OUString & str, const sal_Char * msg = "" )
{

    if (strlen(msg) > 0)
    {
        t_print("%s: ", msg );
    }
    rtl::OString aString;
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", (char *)aString.getStr( ) );
}

/** get the absolute source file URL "file:///.../sal/qa/rtl/logfile/"
  */
inline ::rtl::OUString getModulePath( void )
{
    ::rtl::OUString suDirPath;
    ::osl::Module::getUrlFromAddress( ( void* ) &getModulePath, suDirPath );

    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') );
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') );
    suDirPath = suDirPath.copy( 0, suDirPath.lastIndexOf('/') + 1);
    suDirPath += rtl::OUString::createFromAscii("qa/rtl/logfile/");

    return suDirPath;
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
namespace rtl_logfile
{
    class logfile : public CppUnit::TestFixture
    {
    public:

    //directly call rtl_logfile_trace
        void logfile_001()
        {
#if (defined WNT) || (defined SOLARIS)
        putenv("RTL_LOGFILE=logfile1");
#else
            setenv("RTL_LOGFILE", "logfile1", 0);
#endif
               rtl_logfile_trace("trace %d\n", 2 );
        rtl_logfile_trace("trace %d %d\n" , 1,2 );
        rtl_logfile_trace("trace %d %d %d\n" , 1 , 2 ,3 );
        rtl::OUString suFilePath = getModulePath();
        suFilePath +=  rtl::OUString::createFromAscii("logfile1_") + getCurrentPID( );
        suFilePath +=  rtl::OUString::createFromAscii(".log");

        ::osl::FileBase::RC   nError1;
            ::osl::File aTestFile( suFilePath );
            printUString( suFilePath );
            nError1 = aTestFile.open ( OpenFlag_Read );
        CPPUNIT_ASSERT_MESSAGE("create the log file: but the logfile does not exist",
( ::osl::FileBase::E_NOENT != nError1 ) && ( ::osl::FileBase::E_ACCES != nError1 ) );
            sal_Char       buffer_read[400];
            sal_uInt64  nCount_read;
        nError1 = aTestFile.read( buffer_read, 400, nCount_read );
        //t_print("buffer is %s\n", buffer_read );
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
#if (defined WNT) || (defined SOLARIS)
    putenv("RTL_LOGFILE=logfile2");
#else
        setenv("RTL_LOGFILE", "logfile2", 0);
#endif
        RTL_LOGFILE_TRACE( "trace the log" );
        RTL_LOGFILE_TRACE1( "trace %d" , 1 );
        RTL_LOGFILE_TRACE2( "trace %d %d" , 1,2 );
        RTL_LOGFILE_TRACE3( "trace %d %d %d" , 1 , 2 ,3 );

    }

    void logfile_003()
        {
#if (defined WNT) || (defined SOLARIS)
    putenv("RTL_LOGFILE=logfile3");
#else
        setenv("RTL_LOGFILE", "logfile3", 0);
#endif
        RTL_LOGFILE_CONTEXT ( foo , "foo-function" );
        RTL_LOGFILE_CONTEXT_TRACE ( foo , "trace" );
        RTL_LOGFILE_CONTEXT_TRACE1 ( foo , "trace %d" , 1 );
        RTL_LOGFILE_CONTEXT_TRACE2 ( foo , "trace %d %d" , 1 , 2 );
        RTL_LOGFILE_CONTEXT_TRACE3 ( foo , "trace %d %d %d" , 1 , 2 , 3);
    }


        CPPUNIT_TEST_SUITE( logfile );
        CPPUNIT_TEST( logfile_001 );
        CPPUNIT_TEST( logfile_002 );
        CPPUNIT_TEST( logfile_003 );
        CPPUNIT_TEST_SUITE_END( );
    };

} // namespace rtl_logfile

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_logfile::logfile, "rtl_logfile" );

// -----------------------------------------------------------------------------
NOADDITIONAL;

//~ do some clean up work after all test completed.
class GlobalObject
{
    public:
    ~GlobalObject()
    {
        try
        {
            t_print( "\n#Do some clean-ups ... only delete logfile1_*.log here!\n" );
            rtl::OUString suFilePath = getModulePath();
        suFilePath +=  rtl::OUString::createFromAscii("logfile1_") + getCurrentPID( );
        suFilePath +=  rtl::OUString::createFromAscii(".log");

            //if ( ifFileExist( suFilePath )  == sal_True )
            ::osl::FileBase::RC nError1;
            nError1 = osl::File::remove( suFilePath );
#ifdef WNT
        t_print("Please remove logfile* manully! Error is Permision denied!");
#endif
        }
        catch (CppUnit::Exception &e)
        {
            t_print("Exception caught in GlobalObject dtor(). Exception message: '%s'. Source line: %d\n", e.what(), e.sourceLine().lineNumber());
        }
        catch (...)
        {
            t_print("Exception caught (...) in GlobalObject dtor()\n");
        }
    }
};

GlobalObject theGlobalObject;



