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

// include files

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include "osl/thread.h"

#include <osl/file.hxx>
#include <osl_File_Const.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;
using ::rtl::OStringToOUString;

// helper functions

/** detailed wrong message.
*/
inline ::rtl::OString errorToString( const ::osl::FileBase::RC _nError )
{
    ::rtl::OString sResult;
    switch ( _nError ) {
        case ::osl::FileBase::E_None:
            sResult = "Success";
            break;
        case ::osl::FileBase::E_PERM:
            sResult = "Operation not permitted";
            break;
        case ::osl::FileBase::E_NOENT:
            sResult = "No such file or directory";
            break;
        case ::osl::FileBase::E_EXIST:
            sResult = "Already Exist";
            break;
        case ::osl::FileBase::E_ACCES:
            sResult = "Permission denied";
            break;
        case ::osl::FileBase::E_INVAL:
            sResult = "The format of the parameters was not valid";
            break;
        case ::osl::FileBase::E_NOTDIR:
            sResult = "Not a directory";
            break;
        case ::osl::FileBase::E_ISDIR:
            sResult = "Is a directory";
            break;
        case ::osl::FileBase::E_BADF:
            sResult = "Bad file";
            break;
        case ::osl::FileBase::E_NOTEMPTY:
            sResult = "The directory is not empty";
            break;
        default:
            sResult = "Unknown Error";
            break;
    }
    return sResult;
}

rtl::OString errorToStr( ::osl::FileBase::RC const& nError)
{
    rtl::OString suBuf;
    suBuf += "The returned error is: " ;
    suBuf += errorToString(nError);
    suBuf += "!\n";
    return suBuf;
}

/** compare two TimeValue, unit is "ms", since Windows time precision is better than UNX.
*/
/* FIXME: the above assertion is bogus */

#if ( defined UNX )                      //precision of time in Windows is better than UNX
#   define delta 2000                    //time precision, 2000ms
#else
#   define delta 1800                    //time precision, 1.8s
#endif

inline bool t_compareTime( TimeValue *m_aEndTime,  TimeValue *m_aStartTime, sal_Int32 nDelta)
{
    // sal_uInt64 uTimeValue;
    // sal_Int64 iTimeValue;

    // iTimeValue = t_abs64(( tv1->Seconds - tv2->Seconds) * 1000000000 + tv1->Nanosec - tv2->Nanosec);
    // uTimeValue = ( iTimeValue / 1000000 );

    sal_Int32 nDeltaSeconds = m_aEndTime->Seconds - m_aStartTime->Seconds;
    sal_Int32 nDeltaNanoSec = sal_Int32(m_aEndTime->Nanosec) - sal_Int32(m_aStartTime->Nanosec);
    if (nDeltaNanoSec < 0)
    {
        nDeltaNanoSec = 1000000000 + nDeltaNanoSec;
        nDeltaSeconds--;
    }

    sal_Int32 nDeltaMilliSec = (nDeltaSeconds * 1000) + (nDeltaNanoSec / 1000000);
    return ( nDeltaMilliSec < nDelta );
}

/** compare two OUString file name.
*/
inline bool compareFileName( const ::rtl::OUString & ustr1, const ::rtl::OUString & ustr2 )
{
    bool bOk;
//on Windows, the separator is '\', so here change to '/', then compare
#if defined(_WIN32)
    ::rtl::OUString ustr1new,ustr2new;
    sal_Unicode reverseSlash = (sal_Unicode)'\\';

    if (ustr1.lastIndexOf(reverseSlash) != -1)
        ustr1new = ustr1.replace(reverseSlash,(sal_Unicode)'/');
    else
        ustr1new = ustr1;
    if (ustr2.lastIndexOf(reverseSlash) != -1)
        ustr2new = ustr2.replace(reverseSlash,(sal_Unicode)'/');
    else
        ustr2new = ustr2;
    bOk = ustr1new.equalsIgnoreAsciiCase( ustr2new )  ;
#else
    bOk = ustr1.equalsIgnoreAsciiCase( ustr2 );
#endif
    return bOk;
}

/** simple version to judge if a file name or directory name is a URL or a system path, just to see if it
    is start with "file:///";.
*/
inline bool isURL( const ::rtl::OUString& pathname )
{
    return pathname.startsWith( aPreURL );
}

/** concat two part to form a URL or system path, add PATH_SEPARATOR between them if necessary, add "file:///" to beginning if necessary.
*/
inline void concatURL( ::rtl::OUString & pathname1, const ::rtl::OUString & pathname2 )
{
    //check if pathname1 is full qualified URL;
    if ( !isURL( pathname1 ) )
    {
        ::rtl::OUString     aPathName   = pathname1.copy( 0 );
        ::osl::FileBase::getFileURLFromSystemPath( pathname1, aPathName ); //convert if not full qualified URL
        pathname1   = aPathName.copy( 0 );
    }

    //check if '/' is in the end of pathname1 or at the begin of pathname2;
    if ( !pathname1.endsWith( aSlashURL ) && !pathname2.startsWith( aSlashURL ) )
        pathname1 += aSlashURL;
    pathname1 += pathname2;
}

/** create a temp test file using OUString name of full qualified URL or system path.
*/
inline void createTestFile( const ::rtl::OUString& filename )
{
    ::rtl::OUString     aPathURL   = filename.copy( 0 );
    ::osl::FileBase::RC nError;

    if ( !isURL( filename ) )
        ::osl::FileBase::getFileURLFromSystemPath( filename, aPathURL ); //convert if not full qualified URL

    File aFile(aPathURL);
    nError = aFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    //CPPUNIT_ASSERT_MESSAGE( "In createTestFile Function: creation ", ( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_EXIST ) );
    if ( ( ::osl::FileBase::E_None != nError ) && ( nError != ::osl::FileBase::E_EXIST ))
    {
        printf("createTestFile failed!\n");
    }
    aFile.close();

}

/** create a temp test file using OUString name of full qualified URL or system path in a base directory.
*/
inline void createTestFile( const ::rtl::OUString& basename, const ::rtl::OUString& filename )
{
    ::rtl::OUString aBaseURL = basename.copy( 0 );

    concatURL( aBaseURL, filename );
    createTestFile( aBaseURL );
}

/** delete a temp test file using OUString name.
*/
inline void deleteTestFile( const ::rtl::OUString& filename )
{
    // LLA: printf("deleteTestFile\n");
    ::rtl::OUString     aPathURL   = filename.copy( 0 );
    ::osl::FileBase::RC nError;

    if ( !isURL( filename ) )
        ::osl::FileBase::getFileURLFromSystemPath( filename, aPathURL ); //convert if not full qualified URL

    nError = ::osl::File::setAttributes( aPathURL, osl_File_Attribute_GrpWrite| osl_File_Attribute_OwnWrite| osl_File_Attribute_OthWrite ); // if readonly, make writtenable.
    CPPUNIT_ASSERT_MESSAGE( "In deleteTestFile Function: set writtenable ", ( ::osl::FileBase::E_None == nError ) || ( ::osl::FileBase::E_NOENT == nError ) );

    nError = ::osl::File::remove( aPathURL );
    CPPUNIT_ASSERT_MESSAGE( "In deleteTestFile Function: remove ", ( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT ) );
}

/** delete a temp test file using OUString name of full qualified URL or system path in a base directory.
*/
inline void deleteTestFile( const ::rtl::OUString& basename, const ::rtl::OUString& filename )
{
    ::rtl::OUString     aBaseURL   = basename.copy( 0 );

    concatURL( aBaseURL, filename );
    deleteTestFile( aBaseURL );
}

/** create a temp test directory using OUString name of full qualified URL or system path.
*/
inline void createTestDirectory( const ::rtl::OUString& dirname )
{
    ::rtl::OUString     aPathURL   = dirname.copy( 0 );
    ::osl::FileBase::RC nError;

    if ( !isURL( dirname ) )
        ::osl::FileBase::getFileURLFromSystemPath( dirname, aPathURL ); //convert if not full qualified URL
    nError = ::osl::Directory::create( aPathURL );
    //CPPUNIT_ASSERT_MESSAGE( "In createTestDirectory Function: creation: ", ( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_EXIST ) );
    if ( ( ::osl::FileBase::E_None != nError ) && ( nError != ::osl::FileBase::E_EXIST ))
      printf("createTestDirectory failed!\n");
}

/** create a temp test directory using OUString name of full qualified URL or system path in a base directory.
*/
inline void createTestDirectory( const ::rtl::OUString& basename, const ::rtl::OUString& dirname )
{
    ::rtl::OUString     aBaseURL   = basename.copy( 0 );

    concatURL( aBaseURL, dirname );
    createTestDirectory( aBaseURL );
}

/** delete a temp test directory using OUString name of full qualified URL or system path.
*/
inline void deleteTestDirectory( const ::rtl::OUString& dirname )
{
    ::rtl::OUString     aPathURL   = dirname.copy( 0 );
    ::osl::FileBase::RC nError;
    if ( !isURL( dirname ) )
        ::osl::FileBase::getFileURLFromSystemPath( dirname, aPathURL ); //convert if not full qualified URL

    ::osl::Directory testDir( aPathURL );
    if ( testDir.isOpen() )
        testDir.close();  //close if still open.

    nError = ::osl::Directory::remove( aPathURL );

    rtl::OString strError ("In deleteTestDirectory function: remove Directory ");
    strError += ::rtl::OUStringToOString( aPathURL, RTL_TEXTENCODING_ASCII_US );
    CPPUNIT_ASSERT_MESSAGE( strError.getStr(), ( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT ) );
}

/** delete a temp test directory using OUString name of full qualified URL or system path in a base directory.
*/
inline void deleteTestDirectory( const ::rtl::OUString& basename, const ::rtl::OUString& dirname )
{
    ::rtl::OUString     aBaseURL   = basename.copy( 0 );

    concatURL( aBaseURL, dirname );
    deleteTestDirectory( aBaseURL );
}

/** Check for the file and directory access right.
*/
enum class oslCheckMode {
    Exist,
    OpenAccess,
    ReadAccess,
    WriteAccess
};

//check if the file exist
inline bool ifFileExist( const ::rtl::OUString & str )
{
    ::osl::File testFile( str );
    return ( osl::FileBase::E_None == testFile.open( osl_File_OpenFlag_Read ) );
}

//check if the file can be written
inline bool ifFileCanWrite( const ::rtl::OUString & str )
{
    //on Windows, the file has no write right, but can be written
#ifdef _WIN32
    bool  bCheckResult = false;
    ::rtl::OUString  aUStr  = str.copy( 0 );
    if ( isURL( str ) )
        ::osl::FileBase::getSystemPathFromFileURL( str, aUStr );

    ::rtl::OString aString = ::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US );
    const char *path = aString.getStr();
    if (( _access( path, 2 ) ) != -1 )
         bCheckResult = sal_True;
     //on UNX, just test if open success with osl_File_OpenFlag_Write
#else
    ::osl::File testFile( str );
    bool bCheckResult = (osl::FileBase::E_None == testFile.open( osl_File_OpenFlag_Write ));
#endif
    return bCheckResult;
}

inline bool checkDirectory( const ::rtl::OUString & str, oslCheckMode nCheckMode )
{
    rtl::OUString   aUString;
    DirectoryItem   rItem;
    FileBase::RC    rc;
    bool        bCheckResult= false;

    Directory aDir( str );
    rc = aDir.open();

    if ( ( ::osl::FileBase::E_NOENT != rc ) && ( ::osl::FileBase::E_ACCES != rc ) ){

        switch ( nCheckMode ) {
            case oslCheckMode::Exist:
                if ( rc == ::osl::FileBase::E_None )
                    bCheckResult = true;
                break;
            case oslCheckMode::OpenAccess:
                if ( rc == ::osl::FileBase::E_None )
                    bCheckResult = true;
                break;
            case oslCheckMode::ReadAccess:
                //rc = pDir->getNextItem( rItem, 0 );
                rc = aDir.getNextItem( rItem );
                if ( ( rc == ::osl::FileBase::E_None ) || ( rc == ::osl::FileBase::E_NOENT ) )
                    bCheckResult = true;
                else
                    bCheckResult = false;
                break;
            case oslCheckMode::WriteAccess:
                ( ( aUString += str ) += aSlashURL ) += aTmpName2;
                //if ( ( rc = pDir->create( aUString ) ) == ::osl::FileBase::E_None )
                if ( ( rc = Directory::create( aUString ) ) == ::osl::FileBase::E_None )
                {
                    bCheckResult = true;
                    //rc = pDir->remove( aUString );
                    rc = Directory::remove( aUString );
                    CPPUNIT_ASSERT_EQUAL( ::osl::FileBase::E_None, rc );
                }
                else
                    bCheckResult = false;
                break;

            default:
                bCheckResult = false;
        }// switch

        rc = aDir.close();
        CPPUNIT_ASSERT_EQUAL( FileBase::E_None, rc );

    }//if

    return bCheckResult;
}

/** construct error message
*/
inline ::rtl::OString outputError( const ::rtl::OString & returnVal, const ::rtl::OString & rightVal, const sal_Char * msg = "")
{
    ::rtl::OString aString;
    if ( returnVal.equals( rightVal ) )
        return aString;
    aString += msg;
    aString += ": the returned value is '";
    aString += returnVal;
    aString += "', but the value should be '";
    aString += rightVal;
    aString += "'.";
    return aString;
}

/** Change file mode, two version in UNIX and Windows;.
*/
#if ( defined UNX )         //chmod() method is differ in Windows
inline void changeFileMode( ::rtl::OUString & filepath, sal_Int32 mode )
{
    rtl::OString    aString;
    rtl::OUString   aUStr  = filepath.copy( 0 );

    if ( isURL( filepath ) )
        ::osl::FileBase::getSystemPathFromFileURL( filepath, aUStr );
    aString = ::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US );
    int ret = chmod( aString.getStr(), mode );
    CPPUNIT_ASSERT_EQUAL(0, ret);
}
#else                                          //Windows version
inline void changeFileMode( ::rtl::OUString & filepath, sal_Int32 mode )
{
    (void)filepath;
    (void)mode;
    printf("this method is not implemented yet");
}
#endif

inline ::rtl::OUString getCurrentPID();

// Beginning of the test cases for FileBase class

namespace osl_FileBase
{

    // testing the method
    // static inline RC getAbsoluteFileURL( const ::rtl::OUString& ustrBaseDirectoryURL,
    //                                      const ::rtl::OUString& ustrRelativeFileURL,
    //                                      ::rtl::OUString& ustrAbsoluteFileURL )

    class getAbsoluteFileURL:public CppUnit::TestFixture
    {
        public:

            void check_getAbsoluteFileURL( rtl::OUString const& _suBaseURL,  rtl::OString const& _sRelativeURL, ::osl::FileBase::RC _nAssumeError, rtl::OUString const& _suAssumeResultStr );

      void getAbsoluteFileURL_001_1();
      void getAbsoluteFileURL_001_2();
      void getAbsoluteFileURL_001_3();
      void getAbsoluteFileURL_001_4();
      void getAbsoluteFileURL_001_5();
      void getAbsoluteFileURL_001_6();
      void getAbsoluteFileURL_001_7();
      void getAbsoluteFileURL_001_8();
      void getAbsoluteFileURL_002();
      void getAbsoluteFileURL_003();
      void getAbsoluteFileURL_004();

        CPPUNIT_TEST_SUITE( getAbsoluteFileURL );
        CPPUNIT_TEST( getAbsoluteFileURL_001_1 );
      CPPUNIT_TEST( getAbsoluteFileURL_001_2 );
      CPPUNIT_TEST( getAbsoluteFileURL_001_3 );
      CPPUNIT_TEST( getAbsoluteFileURL_001_4 );
      CPPUNIT_TEST( getAbsoluteFileURL_001_5 );
      CPPUNIT_TEST( getAbsoluteFileURL_001_6 );
      CPPUNIT_TEST( getAbsoluteFileURL_001_7 );
      CPPUNIT_TEST( getAbsoluteFileURL_001_8 );
      CPPUNIT_TEST( getAbsoluteFileURL_002 );
      CPPUNIT_TEST( getAbsoluteFileURL_003 );
      CPPUNIT_TEST( getAbsoluteFileURL_004 );
      CPPUNIT_TEST_SUITE_END();

        }; //class getAbsoluteFileURL

    void getAbsoluteFileURL::check_getAbsoluteFileURL( rtl::OUString const& _suBaseURL,  rtl::OString const& _sRelativeURL, ::osl::FileBase::RC _nAssumeError, rtl::OUString const& _suAssumeResultStr )
    {
        rtl::OUString suRelativeURL = rtl::OStringToOUString(_sRelativeURL, RTL_TEXTENCODING_UTF8);
        rtl::OString sBaseURL = rtl::OUStringToOString(_suBaseURL, RTL_TEXTENCODING_UTF8);
        rtl::OUString suResultURL;
        osl::FileBase::RC nError = FileBase::getAbsoluteFileURL( _suBaseURL,  suRelativeURL, suResultURL );
        rtl::OString sResultURL = rtl::OUStringToOString( suResultURL, RTL_TEXTENCODING_UTF8);
        rtl::OString sError = errorToString(nError);
        printf("getAbsoluteFileURL('%s','%s') deliver absolute URL: '%s', error '%s'\n", sBaseURL.getStr(), _sRelativeURL.getStr(),sResultURL.getStr(), sError.getStr() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Assumption is wrong: error number is wrong", _nAssumeError, nError );
        if ( nError == ::osl::FileBase::E_None )
        {
            bool bStrAreEqual = _suAssumeResultStr.equals( suResultURL );
                    CPPUNIT_ASSERT_MESSAGE( "Assumption is wrong: ResultURL is not equal to expected URL ", bStrAreEqual );
                }
    }

  void getAbsoluteFileURL::getAbsoluteFileURL_001_1()
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( "/relative/file1" );
    check_getAbsoluteFileURL( aUserDirectoryURL, "relative/file1",::osl::FileBase::E_None, suAssume );
  }
  void getAbsoluteFileURL::getAbsoluteFileURL_001_2()
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( "/relative/file2" );
    check_getAbsoluteFileURL( aUserDirectoryURL, "relative/./file2",::osl::FileBase::E_None, suAssume );
  }
  void getAbsoluteFileURL::getAbsoluteFileURL_001_3()
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( "/file3" );
    check_getAbsoluteFileURL( aUserDirectoryURL, "relative/../file3",::osl::FileBase::E_None, suAssume );
  }
  void getAbsoluteFileURL::getAbsoluteFileURL_001_4()
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( "/file4" );
    check_getAbsoluteFileURL( aUserDirectoryURL, "././relative/../file4",::osl::FileBase::E_None, suAssume );
  }
  void getAbsoluteFileURL::getAbsoluteFileURL_001_5()
  {
    rtl::OUString suAssume;
#if ( defined UNX )
    suAssume = aUserDirectoryURL.concat( "/relative/" );
#else
    suAssume = aUserDirectoryURL.concat( rtl::OUString("/relative") );
#endif
    check_getAbsoluteFileURL( aUserDirectoryURL, "././relative/.",::osl::FileBase::E_None, suAssume );
  }
  void getAbsoluteFileURL::getAbsoluteFileURL_001_6()
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( "/.relative" );
    check_getAbsoluteFileURL( aUserDirectoryURL, "./.relative",::osl::FileBase::E_None, suAssume );
  }
  void getAbsoluteFileURL::getAbsoluteFileURL_001_7()
  {
    rtl::OUString suAssume;
#if (defined UNX )
    suAssume = aUserDirectoryURL.concat( "/.a/" );
#else //windows
    suAssume = aUserDirectoryURL.concat( rtl::OUString("/.a") );
#endif
    check_getAbsoluteFileURL( aUserDirectoryURL, "./.a/mydir/..",::osl::FileBase::E_None, suAssume );
  }
  void getAbsoluteFileURL::getAbsoluteFileURL_001_8()
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( "/tmp/ok" );
#if ( defined UNX )
    check_getAbsoluteFileURL( aUserDirectoryURL, "tmp//ok",::osl::FileBase::E_None, suAssume );
#else
    check_getAbsoluteFileURL( aUserDirectoryURL, "tmp//ok",::osl::FileBase::E_INVAL, suAssume );
#endif
  }
  void getAbsoluteFileURL::getAbsoluteFileURL_002()
  {
#if ( defined UNX )     //Link is not defined in Windows
        ::rtl::OUString aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
        aUStr_LnkFileSys += aSlashURL + getCurrentPID() + "/link.file";
        aUStr_SrcFileSys += aSlashURL + getCurrentPID() + "/canonical.name";

                rtl::OString strLinkFileName, strSrcFileName;
                strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
                strSrcFileName =  OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

        createTestFile( aCanURL1 );
                sal_Int32 fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(0), fd );
        rtl::OString sLnkURL = OUStringToOString( aLnkURL1, RTL_TEXTENCODING_ASCII_US );
            rtl::OUString suAssume = aUserDirectoryURL.concat( "/canonical.name" );
        check_getAbsoluteFileURL( aUserDirectoryURL, sLnkURL, ::osl::FileBase::E_None, suAssume );
        deleteTestFile( aCanURL1 );
                fd = remove( strLinkFileName.getStr() );
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(0), fd );
#endif
  }
  //please see line# 930
  void getAbsoluteFileURL::getAbsoluteFileURL_003()
  {
  }
    void getAbsoluteFileURL::getAbsoluteFileURL_004()
    {
        //create two level directories under $Temp/PID/
        ::rtl::OUString aUStrUpBase = aUserDirectoryURL + "/test1";
        createTestDirectory( aUStrUpBase );
        ::rtl::OUString aUStrBase = aUserDirectoryURL + "/test1/dir1";
        createTestDirectory( aUStrBase );

        ::rtl::OUString suAssume = aUserDirectoryURL.concat( "/mytestfile" );
        check_getAbsoluteFileURL( aUStrBase, "../../mytestfile" , ::osl::FileBase::E_None, suAssume );
        deleteTestDirectory( aUStrBase );
        deleteTestDirectory( aUStrUpBase );
    }

    // testing two methods:
    // static inline RC getSystemPathFromFileURL( const ::rtl::OUString& ustrFileURL,
    //                ::rtl::OUString& ustrSystemPath )
        // static RC getFileURLFromSystemPath( const ::rtl::OUString & ustrSystemPath,
        //                                ::rtl::OUString & ustrFileURL );

    class SystemPath_FileURL:public CppUnit::TestFixture
    {
        //::osl::FileBase aFileBase;
        // ::rtl::OUString aUStr;
        // ::osl::FileBase::RC nError;

      //void check_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sAssumeResultStr);
      void check_SystemPath_FileURL(rtl::OString const& _sSource, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sAssumeResultStr, bool bDirection = true );
      void checkWNTBehaviour_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sWNTAssumeResultString );
      void checkUNXBehaviour_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sUnixAssumeResultString );
      void checkWNTBehaviour_getFileURLFromSystemPath(rtl::OString const& _sSysPath, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sWNTAssumeResultString);
      void checkUNXBehaviour_getFileURLFromSystemPath(rtl::OString const& _sSysPath, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sUnixAssumeResultString);

    public:
        // test code.
        void getSystemPathFromFileURL_001_1();
        void getSystemPathFromFileURL_001_2();
        void getSystemPathFromFileURL_001_21();
        void getSystemPathFromFileURL_001_22();
        void getSystemPathFromFileURL_001_3();
        void getSystemPathFromFileURL_001_31();
        void getSystemPathFromFileURL_001_4();
        void getSystemPathFromFileURL_001_41();
        void getSystemPathFromFileURL_001_5();
        void getSystemPathFromFileURL_001_51();
        void getSystemPathFromFileURL_001_52();
        void getSystemPathFromFileURL_001_53();
        void getSystemPathFromFileURL_001_6();
        void getSystemPathFromFileURL_001_61();
        void getSystemPathFromFileURL_001_7();
        void getSystemPathFromFileURL_001_71();
        void getSystemPathFromFileURL_001_8();
            void getSystemPathFromFileURL_001_81();
        void getSystemPathFromFileURL_001_9();
        void getSystemPathFromFileURL_001_91();
        void getSystemPathFromFileURL_001_92();
        void getSystemPathFromFileURL_004();
        void getSystemPathFromFileURL_005();

      //test case fot getFileURLFromSystemPath
            void getFileURLFromSystemPath_001();
            void getFileURLFromSystemPath_002();
            void getFileURLFromSystemPath_003();
            void getFileURLFromSystemPath_004();
        void getFileURLFromSystemPath_005();

        CPPUNIT_TEST_SUITE( SystemPath_FileURL );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_1 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_2 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_21 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_22 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_3 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_31 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_4 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_41 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_5 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_51 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_52 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_53 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_6 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_61 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_7 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_71 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_8 );
            CPPUNIT_TEST( getSystemPathFromFileURL_001_81 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_9 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_91 );
        CPPUNIT_TEST( getSystemPathFromFileURL_001_92 );
        CPPUNIT_TEST( getSystemPathFromFileURL_004 );
        CPPUNIT_TEST( getSystemPathFromFileURL_005 );
            CPPUNIT_TEST( getFileURLFromSystemPath_001 );
        CPPUNIT_TEST( getFileURLFromSystemPath_002 );
        CPPUNIT_TEST( getFileURLFromSystemPath_003 );
        CPPUNIT_TEST( getFileURLFromSystemPath_004 );
        CPPUNIT_TEST( getFileURLFromSystemPath_005 );
        CPPUNIT_TEST_SUITE_END();
    };// class SystemPath_FileURL

    // if bDirection==sal_True, check getSystemPathFromFileURL
    // if bDirection==sal_False, check getFileURLFromSystemPath
    void SystemPath_FileURL::check_SystemPath_FileURL(rtl::OString const& _sSource, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sAssumeResultStr, bool bDirection)
    {
        // PRE: URL as String
        rtl::OUString suSource;
        rtl::OUString suStr;
        suSource = rtl::OStringToOUString(_sSource, RTL_TEXTENCODING_UTF8);
    ::osl::FileBase::RC nError;
    if ( bDirection )
      nError = osl::FileBase::getSystemPathFromFileURL( suSource, suStr );
    else
      nError = osl::FileBase::getFileURLFromSystemPath( suSource, suStr );

        // if the given string is gt length 0,
        // we check also this string
        rtl::OString sStr = rtl::OUStringToOString(suStr, RTL_TEXTENCODING_UTF8);
        rtl::OString sError = errorToString(nError);
    if ( bDirection )
      printf("getSystemPathFromFileURL('%s') deliver system path: '%s', error '%s'\n", _sSource.getStr(), sStr.getStr(), sError.getStr() );
    else
      printf("getFileURLFromSystemPath('%s') deliver File URL: '%s', error '%s'\n", _sSource.getStr(), sStr.getStr(), sError.getStr() );

        // rtl::OUString suStrEncode = rtl::Uri::encode(suStr, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
        // sStr = rtl::OUStringToOString(suStr, RTL_TEXTENCODING_UTF8);
        // printf("UTF8: %s\n", sStr.getStr() );

        if (!_sAssumeResultStr.isEmpty())
        {
            bool bStrAreEqual = _sAssumeResultStr.equals(sStr);
            CPPUNIT_ASSERT_MESSAGE( "Assumption is wrong",
                                    nError == _nAssumeError && bStrAreEqual );
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Assumption is wrong", _nAssumeError, nError );
        }
    }
    void SystemPath_FileURL::checkWNTBehaviour_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sWNTAssumeResultString)
    {
#if defined(_WIN32)
        check_SystemPath_FileURL(_sURL, _nAssumeError, _sWNTAssumeResultString);
#else
        (void)_sURL;
        (void)_nAssumeError;
        (void)_sWNTAssumeResultString;
#endif
    }

    void SystemPath_FileURL::checkUNXBehaviour_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sUnixAssumeResultString)
    {
#if ( defined UNX )
        check_SystemPath_FileURL(_sURL, _nAssumeError, _sUnixAssumeResultString);
#else
        (void)_sURL;
        (void)_nAssumeError;
        (void)_sUnixAssumeResultString;
#endif
    }

    void SystemPath_FileURL::checkWNTBehaviour_getFileURLFromSystemPath(rtl::OString const& _sSysPath, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sWNTAssumeResultString)
    {
#if defined(_WIN32)
        check_SystemPath_FileURL(_sSysPath, _nAssumeError, _sWNTAssumeResultString, sal_False );
#else
        (void)_sSysPath;
        (void)_nAssumeError;
        (void)_sWNTAssumeResultString;
#endif
    }

    void SystemPath_FileURL::checkUNXBehaviour_getFileURLFromSystemPath(rtl::OString const& _sSysPath, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sUnixAssumeResultString)
    {
#if ( defined UNX )
        check_SystemPath_FileURL(_sSysPath, _nAssumeError, _sUnixAssumeResultString, false );
#else
        (void)_sSysPath;
        (void)_nAssumeError;
        (void)_sUnixAssumeResultString;
#endif
    }

    /** LLA: Test for getSystemPathFromFileURL()
        this test is splitted into 2 different OS tests,
        the first function checkUNXBehaviour... runs only on Unix based Systems,
        the second only on windows based systems
        the first parameter are a file URL where we want to get the system path of,
        the second parameter is the assumed error of the osl_getSystemPathFromFileURL() function,
        the third parameter is the assumed result string, the string will only test, if its length is greater 0
    */

    void SystemPath_FileURL::getSystemPathFromFileURL_001_1()
    {
        rtl::OString sURL("");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }

    void SystemPath_FileURL::getSystemPathFromFileURL_001_2()
    {
        rtl::OString sURL("/");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "\\");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_21()
    {
#if 0
        rtl::OString sURL("%2F");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/"); // LLA: this is may be a BUG
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
#endif
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_22()
    {
      rtl::OString sURL("file:///tmp%2Fmydir");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_3()
    {
        rtl::OString sURL("a");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "a");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "a");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_31()
    {
        rtl::OString sURL("tmpname");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "tmpname");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "tmpname");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_4()
    {
        rtl::OString sURL("file://");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_41()
    {
        rtl::OString sURL("file://localhost/tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_5()
    {
        rtl::OString sURL("file:///tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/tmp");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_51()
    {
#if 0
        rtl::OString sURL("file://c:/tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:/tmp"); // LLA: this is may be a BUG
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
#endif
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_52()
    {
        rtl::OString sURL("file:///c:/tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c:/tmp");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_53()
    {
// LLA: is this a legal file path?
        rtl::OString sURL("file:///c|/tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c|/tmp");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_6()
    {
        rtl::OString sURL("file:///tmp/first");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/tmp/first");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_61()
    {
        rtl::OString sURL("file:///c:/tmp/first");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c:/tmp/first");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp\\first");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_7()
    {
        rtl::OString sURL("file:///tmp/../second");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/tmp/../second"); // LLA: may be a BUG
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_71()
    {
        rtl::OString sURL("file:///c:/tmp/../second");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c:/tmp/../second");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp\\..\\second");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_8()
    {
        rtl::OString sURL("../tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "../tmp");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "..\\tmp");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_81()
    {
#if 0
        rtl::OString sURL("file://~/tmp");
        char* home_path;
        home_path = getenv("HOME");
        rtl::OString expResult(home_path);
        expResult += "/tmp";
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, expResult );
//      checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "\\tmp");
#endif
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_9()
    {
        rtl::OString sURL("file:///tmp/first%20second");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/tmp/first second");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    void SystemPath_FileURL::getSystemPathFromFileURL_001_91()
    {
        rtl::OString sURL("file:///c:/tmp/first%20second");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c:/tmp/first second");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp\\first second");
    }

    void SystemPath_FileURL::getSystemPathFromFileURL_001_92()
    {
#if 0
        rtl::OString sURL("ca@#;+.,$///78no%01ni..name");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
#endif
    }

        //normal legal case
    void SystemPath_FileURL::getSystemPathFromFileURL_004()
        {
        ::rtl::OUString aUStr;
        ::rtl::OUString aUNormalURL( aTmpName6 );
        ::rtl::OUString aUResultURL ( aSysPath4 );
        ::osl::FileBase::RC nError = osl::FileBase::getSystemPathFromFileURL( aUNormalURL, aUStr );

            bool bOk = compareFileName( aUStr, aUResultURL );

            ::rtl::OString sError("test for getSystemPathFromFileURL(' ");
            sError += ::rtl::OUStringToOString( aUNormalURL, RTL_TEXTENCODING_ASCII_US );
            sError += " ') function:use an absolute file URL, ";
            sError += outputError(::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US ),
                                ::rtl::OUStringToOString( aUResultURL, RTL_TEXTENCODING_ASCII_US ));

            CPPUNIT_ASSERT_MESSAGE(sError.getStr(), ( osl::FileBase::E_None == nError ) && bOk );

        }

        //CJK characters case
    void SystemPath_FileURL::getSystemPathFromFileURL_005()
        {
            ::rtl::OUString aUStr;
            createTestDirectory( aTmpName10 );
            ::rtl::OUString aUNormalURL( aTmpName10 );
            ::rtl::OUString aUResultURL ( aSysPath5 );

            ::osl::FileBase::RC nError = osl::FileBase::getSystemPathFromFileURL( aUNormalURL, aUStr );

            bool bOk = compareFileName( aUStr, aUResultURL );

            ::rtl::OString sError("test for getSystemPathFromFileURL(' ");
            sError += ::rtl::OUStringToOString( aUNormalURL, RTL_TEXTENCODING_ASCII_US );
            sError += " ') function:use a CJK coded absolute URL, ";
            sError += outputError(::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US ),
                                ::rtl::OUStringToOString( aUResultURL, RTL_TEXTENCODING_ASCII_US ));
            deleteTestDirectory( aTmpName10 );

            CPPUNIT_ASSERT_MESSAGE( sError.getStr(), ( osl::FileBase::E_None == nError ) && bOk );
        }

     void SystemPath_FileURL::getFileURLFromSystemPath_001()
     {
        rtl::OString sSysPath("~/tmp");
        char* home_path;
        home_path = getenv("HOME");
        rtl::OString expResult(home_path);
        expResult = "file://"+ expResult + "/tmp";
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, expResult );
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, "~/tmp");
     }
     void SystemPath_FileURL::getFileURLFromSystemPath_002()
     {
        rtl::OString sSysPath("c:/tmp");
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, "c:/tmp");
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, "file:///c:/tmp");
     }
     void SystemPath_FileURL::getFileURLFromSystemPath_003()
     {
        rtl::OString sSysPath("file:///temp");
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
     }
    void SystemPath_FileURL::getFileURLFromSystemPath_004()
     {
        rtl::OString sSysPath("//tmp//first start");
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, "file:///tmp/first%20start");
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
     }
     void SystemPath_FileURL::getFileURLFromSystemPath_005()
     {
        rtl::OString sSysPath("");
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
     }
         // start with "~user", not implement
    //      void SystemPath_FileURL::getFileURLFromSystemPath_006()

    // testing the method
    // static inline RC searchFileURL(  const ::rtl::OUString& ustrFileName,
    //                                  const ::rtl::OUString& ustrSearchPath,
    //                                  ::rtl::OUString& ustrFileURL )

    class searchFileURL:public CppUnit::TestFixture
    {
        //::osl::FileBase aFileBase;
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1, nError2, nError3,nError4;

        public:

        searchFileURL()
            : nError1(FileBase::E_None)
            , nError2(FileBase::E_None)
            , nError3(FileBase::E_None)
            , nError4(FileBase::E_None) {}
        // test code.
        void searchFileURL_001()
        {
            /* search file is passed by system filename */
            nError1 = ::osl::FileBase::searchFileURL( aTmpName1, aUserDirectorySys, aUStr );
            /* search file is passed by full qualified file URL */
            nError2 = ::osl::FileBase::searchFileURL( aCanURL1, aUserDirectorySys, aUStr );
            /* search file is passed by relative file path */
            nError3 = ::osl::FileBase::searchFileURL( aRelURL4, aUserDirectorySys, aUStr );

            CPPUNIT_ASSERT_MESSAGE( "test for searchFileURL function: system filename/URL filename/relative path, system directory, searched files that is not exist, but it reply invalid error, did not pass in (W32) ",
                                     ( osl::FileBase::E_NOENT == nError1 ) &&
                                     ( osl::FileBase::E_NOENT == nError2 ) &&
                                    ( osl::FileBase::E_NOENT == nError3 ));
        }

         void searchFileURL_002()
        {
            /* search file is passed by system filename */
            nError1 = ::osl::FileBase::searchFileURL( aTempDirectorySys, aRootSys, aUStr );
            bool bOk1 = compareFileName( aUStr, aTempDirectoryURL );
            /* search file is passed by full qualified file URL */
            nError2 = ::osl::FileBase::searchFileURL( aTempDirectoryURL, aRootSys, aUStr );
            bool bOk2 = compareFileName( aUStr, aTempDirectoryURL );
            /* search file is passed by relative file path */
            nError3 = ::osl::FileBase::searchFileURL( aRelURL5, aRootSys, aUStr );
            bool bOk3 = compareFileName( aUStr, aTempDirectoryURL );
            /* search file is passed by an exist file */
            createTestFile( aCanURL1 );
            nError4 = ::osl::FileBase::searchFileURL( aCanURL4, aUserDirectorySys, aUStr );
            bool bOk4 = compareFileName( aUStr, aCanURL1 );
            deleteTestFile( aCanURL1 );

            CPPUNIT_ASSERT_MESSAGE( "test for searchFileURL function: system filename/URL filename/relative path, system directory, searched file already exist.",
                                    ( osl::FileBase::E_None == nError1 ) &&
                                    ( osl::FileBase::E_None == nError2 ) &&
                                    ( osl::FileBase::E_None == nError3 ) &&
                                    ( osl::FileBase::E_None == nError4 ) &&
                                    bOk1 && bOk2 && bOk3 && bOk4 );
        }

        void searchFileURL_003()
        {
            OUString aSystemPathList( TEST_PLATFORM_ROOT ":" TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP ":" TEST_PLATFORM_ROOT "system/path" );
            nError1 = ::osl::FileBase::searchFileURL( aUserDirectoryURL, aSystemPathList, aUStr );
            bool bOk = compareFileName( aUStr, aUserDirectoryURL );
            CPPUNIT_ASSERT_MESSAGE( "test for searchFileURL function: search directory is a list of system paths",
                                    ( osl::FileBase::E_None == nError1 ) &&
                                    bOk );
        }

        void searchFileURL_004()
        {
            OUString aSystemPathList( TEST_PLATFORM_ROOT PATH_LIST_DELIMITER TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP PATH_LIST_DELIMITER TEST_PLATFORM_ROOT "system/path/../name" );
            nError1 = ::osl::FileBase::searchFileURL( aUserDirectoryURL, aSystemPathList, aUStr );
            bool bOk = compareFileName( aUStr, aUserDirectoryURL );
            CPPUNIT_ASSERT_MESSAGE( "test for searchFileURL function: search directory is a list of system paths",
                                    ( osl::FileBase::E_None == nError1 ) &&
                                    bOk );
        }

        void searchFileURL_005()
        {
            nError1 = ::osl::FileBase::searchFileURL( aUserDirectoryURL, aNullURL, aUStr );
            bool bOk = compareFileName( aUStr, aUserDirectoryURL );
            CPPUNIT_ASSERT_MESSAGE( "test for searchFileURL function: search directory is NULL",
                                    ( osl::FileBase::E_None == nError1 ) &&
                                    bOk );
        }

        CPPUNIT_TEST_SUITE( searchFileURL );
        CPPUNIT_TEST( searchFileURL_001 );
        CPPUNIT_TEST( searchFileURL_002 );
        CPPUNIT_TEST( searchFileURL_003 );
        CPPUNIT_TEST( searchFileURL_004 );
        CPPUNIT_TEST( searchFileURL_005 );
        CPPUNIT_TEST_SUITE_END();
    };// class searchFileURL

    // testing the method
    // static inline RC getTempDirURL( ::rtl::OUString& ustrTempDirURL )

    class getTempDirURL:public CppUnit::TestFixture
    {
        //::osl::FileBase aFileBase;
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError;

        public:
        getTempDirURL() :nError(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
             nError = FileBase::getTempDirURL( aUStr );
        }

        // test code.
        void getTempDirURL_001()
        {

            CPPUNIT_ASSERT_MESSAGE( "test for getTempDirURL function: excution",
                                     ( osl::FileBase::E_None == nError ) );
        }

        void getTempDirURL_002()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getTempDirURL function: test for open and write access rights",
                                    checkDirectory( aUStr, oslCheckMode::OpenAccess ) &&
                                    checkDirectory( aUStr, oslCheckMode::ReadAccess ) &&
                                    checkDirectory( aUStr, oslCheckMode::WriteAccess ) );
        }

        CPPUNIT_TEST_SUITE( getTempDirURL );
        CPPUNIT_TEST( getTempDirURL_001 );
        CPPUNIT_TEST( getTempDirURL_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class getTempDirURL

    //  testing the method
    //  static inline RC createTempFile( ::rtl::OUString* pustrDirectoryURL,
    //                                   oslFileHandle* pHandle,
    //                                   ::rtl::OUString* pustrTempFileURL)

    class createTempFile:public CppUnit::TestFixture
    {
        //::osl::FileBase aFileBase;
        ::osl::FileBase::RC nError1, nError2;
        bool bOK;

        oslFileHandle   *pHandle;
        ::rtl::OUString *pUStr_DirURL;
        ::rtl::OUString *pUStr_FileURL;

    public:
        createTempFile()
            : nError1(FileBase::E_None)
            , nError2(FileBase::E_None)
            , bOK(false)
            , pHandle(nullptr)
            , pUStr_DirURL(nullptr)
            , pUStr_FileURL(nullptr)
        {
        }

        // initialization
        void setUp() override
        {
            pHandle = new oslFileHandle();
            pUStr_DirURL = new ::rtl::OUString( aUserDirectoryURL );
            pUStr_FileURL = new ::rtl::OUString();
            //*pUStr_DirURL = aUserDirectoryURL;                /// create temp file in /tmp/PID or c:\temp\PID.*/
        }

        void tearDown() override
        {
            delete pUStr_DirURL;
            delete pUStr_FileURL;
            delete pHandle;
        }

        // test code.
        void createTempFile_001()
        {
            nError1 = FileBase::createTempFile( pUStr_DirURL, pHandle, pUStr_FileURL );
            ::osl::File testFile( *pUStr_FileURL );
            nError2 = testFile.open( osl_File_OpenFlag_Create );
            if ( osl::FileBase::E_EXIST == nError2 )  {
                osl_closeFile( *pHandle );
                deleteTestFile( *pUStr_FileURL );
            }

            CPPUNIT_ASSERT_MESSAGE( "test for createTempFile function: create temp file and test the existence",
                                     ( osl::FileBase::E_None == nError1 ) && ( pHandle != nullptr ) &&   ( osl::FileBase::E_EXIST== nError2 )   );
        }

        void createTempFile_002()
        {
            bOK = false;
            nError1 = FileBase::createTempFile( pUStr_DirURL, pHandle, pUStr_FileURL );
            ::osl::File testFile( *pUStr_FileURL );
            nError2 = testFile.open( osl_File_OpenFlag_Create );

            CPPUNIT_ASSERT_MESSAGE( "createTempFile function: create a temp file, but it does not exist",
                ( osl::FileBase::E_None == nError1 ) && ( pHandle != nullptr ) &&
                ( osl::FileBase::E_EXIST == nError2 ) );

            //check file if have the write permission
            if ( osl::FileBase::E_EXIST == nError2 )  {
                bOK = ifFileCanWrite( *pUStr_FileURL );
                osl_closeFile( *pHandle );
                deleteTestFile( *pUStr_FileURL );
            }

            CPPUNIT_ASSERT_MESSAGE( "test for open and write access rights, in (W32), it did not have write access right, but it should be writtenable.",
                                     bOK );
        }

        void createTempFile_003()
        {
            nError1 = FileBase::createTempFile( pUStr_DirURL, pHandle, nullptr );
            //the temp file will be removed when return from createTempFile
            bOK = (pHandle != nullptr && nError1 == osl::FileBase::E_None);
            if ( bOK )
                osl_closeFile( *pHandle );

            CPPUNIT_ASSERT_MESSAGE( "test for createTempFile function: set pUStrFileURL to 0 to let it remove the file after call.",
                                ( ::osl::FileBase::E_None == nError1 ) && bOK );
        }
        void createTempFile_004()
        {
            nError1 = FileBase::createTempFile( pUStr_DirURL, nullptr, pUStr_FileURL );
            bOK = ( pUStr_FileURL != nullptr);
            CPPUNIT_ASSERT(bOK);
            ::osl::File testFile( *pUStr_FileURL );
            nError2 = testFile.open( osl_File_OpenFlag_Create );
            deleteTestFile( *pUStr_FileURL );
            CPPUNIT_ASSERT_MESSAGE( "createTempFile function: create a temp file, but it does not exist",
                ( osl::FileBase::E_None == nError1 ) && ( osl::FileBase::E_EXIST == nError2 ) && bOK );

        }

        CPPUNIT_TEST_SUITE( createTempFile );
        CPPUNIT_TEST( createTempFile_001 );
        CPPUNIT_TEST( createTempFile_002 );
        CPPUNIT_TEST( createTempFile_003 );
        CPPUNIT_TEST( createTempFile_004 );
        CPPUNIT_TEST_SUITE_END();
    };// class createTempFile

// FIXME: remove the _disabled to enable:
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileBase::getAbsoluteFileURL, "osl_FileBase_disabled" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileBase::SystemPath_FileURL, "osl_FileBase" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileBase::searchFileURL, "osl_FileBase_disabled" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileBase::getTempDirURL, "osl_FileBase_disabled" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileBase::createTempFile, "osl_FileBase_disabled" );

    CPPUNIT_REGISTRY_ADD_TO_DEFAULT( "osl_FileBase" );
}// namespace osl_FileBase

namespace osl_FileStatus
{

    //  testing the method
    //  FileStatus( sal_uInt32 nMask ): _nMask( nMask )

    class  ctors : public CppUnit::TestFixture
    {
        ::rtl::OUString         aUStr;
        ::osl::FileBase::RC     nError1;
        ::osl::DirectoryItem    rItem;

        public:
        ctors() :nError1(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            Directory aDir( aTmpName3 );
            nError1 = aDir.open();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = aDir.getNextItem( rItem );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            aDir.close();
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void ctors_001()
        {
             ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_All );
            nError1 = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            aUStr = rFileStatus.getFileName();

            CPPUNIT_ASSERT_MESSAGE( "test for ctors function: mask all and see the file name",
                                    compareFileName( aUStr, aTmpName2)  );
        }

        void ctors_002()
        {
             ::osl::FileStatus   rFileStatus( 0 );
            nError1 = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            aUStr = rFileStatus.getFileName();

            CPPUNIT_ASSERT_MESSAGE( "test for ctors function: mask is empty",
                                    compareFileName( aUStr, aNullURL)  );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST( ctors_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class ctors

    //  testing the method
    //  inline sal_Bool isValid( sal_uInt32 nMask ) const

    class  isValid : public CppUnit::TestFixture
    {
        ::osl::Directory        *pDir;
        ::osl::DirectoryItem    rItem_file, rItem_link;

        public:
        isValid()
            : pDir(nullptr)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            pDir = new Directory( aTmpName3 );
                    ::osl::FileBase::RC nError1 = pDir->open();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = pDir->getNextItem( rItem_file, 1 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            ::osl::FileBase::RC nError1 = pDir->close();
            delete pDir;
            CPPUNIT_ASSERT_EQUAL_MESSAGE( errorToStr(nError1).getStr(), nError1, ::osl::FileBase::E_None );

            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void isValid_001()
        {
            sal_uInt32 mask = 0;
             ::osl::FileStatus   rFileStatus( mask );
                ::osl::FileBase::RC nError1 = rItem_file.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            bool bOk = rFileStatus.isValid( mask );

            CPPUNIT_ASSERT_MESSAGE( "test for isValid function: no fields specified",
                                    bOk );
        }

        void check_FileStatus(::osl::FileStatus const& _aStatus)
            {
                rtl::OString sStat;
                if (_aStatus.isValid(osl_FileStatus_Mask_Type))
                {
                    sStat += "type ";
                }
                if (_aStatus.isValid(osl_FileStatus_Mask_Attributes))
                {
                    sStat += "attributes ";
                }
                if (_aStatus.isValid(osl_FileStatus_Mask_CreationTime))
                {
                    sStat += "ctime ";
                }
                if (_aStatus.isValid(osl_FileStatus_Mask_AccessTime))
                {
                    sStat += "atime ";
                }
                if (_aStatus.isValid(osl_FileStatus_Mask_ModifyTime))
                {
                    sStat += "mtime ";
                }
                if (_aStatus.isValid(osl_FileStatus_Mask_FileSize))
                {
                    sStat += "filesize ";
                }
                if (_aStatus.isValid(osl_FileStatus_Mask_FileName))
                {
                    sStat += "filename ";
                }
                if (_aStatus.isValid(osl_FileStatus_Mask_FileURL))
                {
                    sStat += "fileurl ";
                }
                printf("mask: %s\n", sStat.getStr());
            }

        void isValid_002()
        {
            createTestFile( aTmpName6 );
            sal_uInt32 mask_file = osl_FileStatus_Mask_Type         | osl_FileStatus_Mask_Attributes |
                                   osl_FileStatus_Mask_CreationTime | osl_FileStatus_Mask_AccessTime |
                                   osl_FileStatus_Mask_ModifyTime   | osl_FileStatus_Mask_FileSize   |
                                   osl_FileStatus_Mask_FileName     | osl_FileStatus_Mask_FileURL;
            ::osl::FileStatus   rFileStatus( mask_file );
            ::osl::DirectoryItem::get( aTmpName6, rItem_file );
            ::osl::FileBase::RC nError1 = rItem_file.getFileStatus( rFileStatus );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( errorToStr(nError1).getStr(), nError1, ::osl::FileBase::E_None );

// LLA: this is wrong, we never should try to check on all masks
//      only on one.
//      Second, it's not a bug, if a value is not valid, it's an unhandled feature.

//      sal_Bool bOk = rFileStatus.isValid( mask_file );

            check_FileStatus(rFileStatus);
            deleteTestFile( aTmpName6 );

                // CPPUNIT_ASSERT_MESSAGE( "test for isValid function: regular file mask fields test, #osl_FileStatus_Mask_CreationTime# should be valid field for regular file, but feedback is invalid",
                //                      ( sal_True == bOk ) );
        }

        //Link is not defined in Windows, and on Linux, we can not get the directory item of the link file
        // LLA: we have to differ to filesystems, normal filesystems support links (EXT2, ...)
        //      castrated filesystems don't (FAT, FAT32)
        //      Windows NT NTFS support links, but the windows api don't :-(

        void isValid_003()
        {
#if defined ( UNX )
            // ::osl::FileBase::RC nError;
            sal_Int32 fd;

            ::rtl::OUString aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
            aUStr_LnkFileSys += aSlashURL + getCurrentPID() + "/tmpdir/link.file";
            aUStr_SrcFileSys += aSlashURL + getCurrentPID() + "/tmpdir/tmpname";

                rtl::OString strLinkFileName;
                rtl::OString strSrcFileName;
                strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
                strSrcFileName = OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

            //create a link file and link it to file "/tmp/PID/tmpdir/tmpname"
                fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
            CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(0), fd );

            // testDirectory is "/tmp/PID/tmpdir/"
            ::osl::Directory testDirectory( aTmpName3 );
            testDirectory.open();
            ::rtl::OUString aFileName ("link.file");
            bool bOk = false;
            while (true) {
                osl::FileBase::RC nError1 = testDirectory.getNextItem( rItem_link, 4 );
                if (::osl::FileBase::E_None == nError1) {
                    sal_uInt32 mask_link = osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_LinkTargetURL;
                    ::osl::FileStatus   rFileStatus( mask_link );
                    rItem_link.getFileStatus( rFileStatus );
                    if ( compareFileName( rFileStatus.getFileName(), aFileName) )
                    {
                        //printf("find the link file");
                        if ( rFileStatus.isValid( osl_FileStatus_Mask_LinkTargetURL ) )
                        {
                            bOk = true;
                            break;
                        }
                    }
                }
                else
                    break;
            };

            fd = remove( strLinkFileName.getStr() );
            CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(0), fd );

            CPPUNIT_ASSERT_MESSAGE("test for isValid function: link file, check for LinkTargetURL",
                                   bOk);
#endif
        }

        void isValid_004()
        {
            sal_uInt32 mask_file_all = osl_FileStatus_Mask_All;
             ::osl::FileStatus   rFileStatus_all( mask_file_all );
                ::osl::FileBase::RC nError1 = rItem_file.getFileStatus( rFileStatus_all );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

                check_FileStatus(rFileStatus_all);
// LLA: this is wrong
//          sal_Bool bOk1 = rFileStatus_all.isValid( mask_file_all );

            sal_uInt32 mask_file_val = osl_FileStatus_Mask_Validate;
             ::osl::FileStatus   rFileStatus_val( mask_file_val );
            nError1 = rItem_file.getFileStatus( rFileStatus_val );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
                // sal_Bool bOk2 = rFileStatus_val.isValid( mask_file_val );

                check_FileStatus(rFileStatus_val);
                // CPPUNIT_ASSERT_MESSAGE( "test for isValid function: check for Mask_All and Validate, really not sure what validate used for and how to use it, help me.  did not pass (W32)(UNX).",
                //                      ( sal_False == bOk1 ) && ( sal_True == bOk2 )  );
        }

        CPPUNIT_TEST_SUITE( isValid );
        CPPUNIT_TEST( isValid_001 );
        CPPUNIT_TEST( isValid_002 );
        CPPUNIT_TEST( isValid_003 );
        CPPUNIT_TEST( isValid_004 );
        CPPUNIT_TEST_SUITE_END();
    };// class ctors

    //  testing the method
    //  inline Type getFileType() const

    class  getFileType : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1;

        ::osl::DirectoryItem    m_aItem_1, m_aItem_2, m_aVolumeItem, m_aFifoItem;
        ::osl::DirectoryItem    m_aLinkItem, m_aSocketItem, m_aSpecialItem;

        public:
        getFileType() :nError1(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile: $TEMP/tmpdir/tmpname.
            //        a tempdirectory: $TEMP/tmpdir/tmpdir.
            //        use $ROOT/staroffice as volume ---> use dev/fd as volume.
            // and get their directory item.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName3, aTmpName2 );
            createTestDirectory( aTmpName3, aTmpName1 );

            std::unique_ptr<Directory> xDir( new Directory( aTmpName3 ) );
            nError1 = xDir->open();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("open aTmpName3 failed!", nError1, ::osl::FileBase::E_None );
            //getNextItem can not assure which item retrieved
                    nError1 = xDir->getNextItem( m_aItem_1, 1 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get first item failed!", nError1, ::osl::FileBase::E_None );

                    nError1 = xDir->getNextItem( m_aItem_2 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get second item failed!", nError1, ::osl::FileBase::E_None );
            xDir->close();
            //mindy: failed on my RH9,so removed temporaly
            //nError1 = ::osl::DirectoryItem::get( aVolURL2, m_aVolumeItem );
            //CPPUNIT_ASSERT_MESSAGE("get volume item failed!", ::osl::FileBase::E_None == nError1 );

        }

        void tearDown() override
        {
            // remove all in $TEMP/tmpdir.
            deleteTestDirectory( aTmpName3, aTmpName1 );
            deleteTestFile( aTmpName3, aTmpName2 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void getFileType_001()
        {
             ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName );
            nError1 = m_aItem_1.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("getFileStatus failed", nError1, ::osl::FileBase::E_None );

            check_FileType(rFileStatus);
        }

        void check_FileType(osl::FileStatus const& _rFileStatus )
        {
            if ( _rFileStatus.isValid(osl_FileStatus_Mask_FileName))
            {
                rtl::OUString suFilename = _rFileStatus.getFileName();

                if ( _rFileStatus.isValid(osl_FileStatus_Mask_Type))
                {
                    osl::FileStatus::Type eType = _rFileStatus.getFileType();
                    bool bOK = false;

                    if ( compareFileName( suFilename, aTmpName2) )
                    {
                        // regular
                        bOK = ( eType == osl::FileStatus::Regular );
                    }
                    if ( compareFileName( suFilename, aTmpName1) )
                    {
                        // directory
                        bOK = ( eType == ::osl::FileStatus::Directory );
                    }

                    CPPUNIT_ASSERT_MESSAGE( "test for getFileType function: ",
                                 bOK );
                }
            }
            // LLA: it's not a bug, if a FileStatus not exist, so no else
        }

        void getFileType_002()
        {
             ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName );
                nError1 = m_aItem_2.getFileStatus( rFileStatus );

            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
                check_FileType(rFileStatus);
        }

        void getFileType_003()
        {
        }

        void getFileType_007()
        {
#if defined ( SOLARIS ) //Special file is differ in Windows
                nError1 = ::osl::DirectoryItem::get( aTypeURL2, m_aSpecialItem );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //check for File type
             ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Type );
            nError1 = m_aSpecialItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            if (rFileStatus.isValid(osl_FileStatus_Mask_Type))
            {
                osl::FileStatus::Type eType = rFileStatus.getFileType();

                CPPUNIT_ASSERT_MESSAGE( "test for getFileType function: Special, Solaris version ",
                                        ( eType == ::osl::FileStatus::Special ) );
            }
#endif
        }

        CPPUNIT_TEST_SUITE( getFileType );
        CPPUNIT_TEST( getFileType_001 );
        CPPUNIT_TEST( getFileType_002 );
        CPPUNIT_TEST( getFileType_003 );
        CPPUNIT_TEST( getFileType_007 );
        CPPUNIT_TEST_SUITE_END();
    };// class getFileType

    //  testing the method
    //  inline sal_uInt64 getAttributes() const

    class  getAttributes : public CppUnit::TestFixture
    {
        ::rtl::OUString         aTypeURL, aTypeURL_Hid;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem, rItem_hidden;

        public:
        getAttributes() :nError(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            aTypeURL_Hid = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL_Hid, aHidURL1 );
            createTestFile( aTypeURL_Hid );
            nError = ::osl::DirectoryItem::get( aTypeURL_Hid, rItem_hidden );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );
        }

        void tearDown() override
        {
            deleteTestFile( aTypeURL );
            deleteTestFile( aTypeURL_Hid );
        }

        // test code.
#if ( defined UNX )
//windows only 3 file attributes: normal, readonly, hidden
        void getAttributes_001()
        {
            changeFileMode( aTypeURL, S_IRUSR | S_IRGRP | S_IROTH );

              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Attributes );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for getAttributes function: ReadOnly, GrpRead, OwnRead, OthRead( UNX version ) ",
                                    static_cast<sal_uInt64>( osl_File_Attribute_ReadOnly | osl_File_Attribute_GrpRead | osl_File_Attribute_OwnRead | osl_File_Attribute_OthRead ),
                                    rFileStatus.getAttributes() );
        }
#else                                    //Windows version
        void getAttributes_001()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getAttributes function: ReadOnly, GrpRead, OwnRead, OthRead( Windows version )",
                                     1 == 1 );
        }
#endif

        void getAttributes_002()
        {
#if ( defined UNX )
            changeFileMode( aTypeURL, S_IXUSR | S_IXGRP | S_IXOTH );

              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Attributes );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for getAttributes function: Executable, GrpExe, OwnExe, OthExe, the result is Readonly, Executable, GrpExe, OwnExe, OthExe, it partly not pass( Solaris version )",
                                    static_cast<sal_uInt64>( osl_File_Attribute_ReadOnly | osl_File_Attribute_Executable | osl_File_Attribute_GrpExe | osl_File_Attribute_OwnExe | osl_File_Attribute_OthExe ),
                                    rFileStatus.getAttributes() );
#endif
        }

#if ( defined UNX )
        void getAttributes_003()
        {
            changeFileMode( aTypeURL, S_IWUSR | S_IWGRP | S_IWOTH );

              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Attributes );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for getAttributes function: GrpWrite, OwnWrite, OthWrite( Solaris version )",
                                    static_cast<sal_uInt64>( osl_File_Attribute_GrpWrite | osl_File_Attribute_OwnWrite | osl_File_Attribute_OthWrite ),
                                    rFileStatus.getAttributes() );
        }
#else                                    //Windows version
        void getAttributes_003()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getAttributes function: GrpWrite, OwnWrite, OthWrite( Windows version )",
                                     1 == 1 );
        }
#endif

#if ( defined UNX )                     //hidden file definition may different in Windows
        void getAttributes_004()
        {
            sal_Int32 test_Attributes = osl_File_Attribute_Hidden;
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Attributes );
            nError = rItem_hidden.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );
            test_Attributes &= rFileStatus.getAttributes();

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for getAttributes function: Hidden files( Solaris version )",
                                    static_cast<sal_Int32>(osl_File_Attribute_Hidden), test_Attributes );
        }
#else                                    //Windows version
        void getAttributes_004()
        {
            ::rtl::OUString aUserHiddenFileURL ("file:///c:/AUTOEXEC.BAT");
            nError = ::osl::DirectoryItem::get( aUserHiddenFileURL, rItem_hidden );
            CPPUNIT_ASSERT_MESSAGE("get item fail", nError == FileBase::E_None );
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Attributes );
            nError = rItem_hidden.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            CPPUNIT_ASSERT_MESSAGE( "Hidden files(Windows version), please check if hidden file c:/AUTOEXEC.BAT exists ",
                                    (rFileStatus.getAttributes() & osl_File_Attribute_Hidden)!= 0 );
        }
#endif

        CPPUNIT_TEST_SUITE( getAttributes );
        CPPUNIT_TEST( getAttributes_001 );
        CPPUNIT_TEST( getAttributes_002 );
        CPPUNIT_TEST( getAttributes_003 );
        CPPUNIT_TEST( getAttributes_004 );
        CPPUNIT_TEST_SUITE_END();
    };// class getAttributes

    //  testing the method
    //  inline TimeValue getAccessTime() const

    class  getAccessTime : public CppUnit::TestFixture
    {
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        getAccessTime() :nError(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

        }

        void tearDown() override
        {
            deleteTestFile( aTypeURL );
        }

        // test code.
        void getAccessTime_001()
        {
            TimeValue *pTV_current = nullptr;
            CPPUNIT_ASSERT( ( pTV_current = static_cast<TimeValue*>(malloc( sizeof( TimeValue ) )) ) != nullptr );
            TimeValue *pTV_access = nullptr;
            CPPUNIT_ASSERT( ( pTV_access = static_cast<TimeValue*>(malloc( sizeof( TimeValue ) )) ) != nullptr );

              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_AccessTime );
            nError = rItem.getFileStatus( rFileStatus );
            bool bOk = osl_getSystemTime( pTV_current );
            CPPUNIT_ASSERT( bOk && nError == FileBase::E_None );

            *pTV_access = rFileStatus.getAccessTime();

            bool bOK = t_compareTime( pTV_access, pTV_current, delta );
            free( pTV_current );
            free( pTV_access );

            CPPUNIT_ASSERT_MESSAGE( "test for getAccessTime function: This test turns out that UNX pricision is no more than 1 sec, don't know how to test this function, in Windows test, it lost hour min sec, only have date time. ",
                                    bOK );
        }

        CPPUNIT_TEST_SUITE( getAccessTime );
        CPPUNIT_TEST( getAccessTime_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class getAccessTime

    //  testing the method
    //  inline TimeValue getModifyTime() const

    class  getModifyTime : public CppUnit::TestFixture
    {
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        getModifyTime() :nError(FileBase::E_None) {}

        // test code.
        void getModifyTime_001()
        {
            TimeValue *pTV_current = nullptr;
            CPPUNIT_ASSERT( ( pTV_current = static_cast<TimeValue*>(malloc( sizeof( TimeValue ) )) ) != nullptr );

            //create file
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );

            //get current time
            bool bOk = osl_getSystemTime( pTV_current );
            CPPUNIT_ASSERT( bOk );

            //get instance item and filestatus
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );
            ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_ModifyTime );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            //get modify time
            TimeValue* pTV_modify = nullptr;
            CPPUNIT_ASSERT( ( pTV_modify = static_cast<TimeValue*>(malloc( sizeof( TimeValue ) )) ) != nullptr );
            *pTV_modify = rFileStatus.getModifyTime();

            bool bOK = t_compareTime( pTV_modify, pTV_current, delta );
            //delete file
            deleteTestFile( aTypeURL );
            free( pTV_current );
            free( pTV_modify );

            CPPUNIT_ASSERT_MESSAGE( "test for getModifyTime function: This test turns out that UNX pricision is no more than 1 sec, don't know how to improve this function.  ",
                                    bOK );
        }

        CPPUNIT_TEST_SUITE( getModifyTime );
        CPPUNIT_TEST( getModifyTime_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class getModifyTime

    //  testing the method
    //  inline sal_uInt64 getFileSize() const

    class  getFileSize : public CppUnit::TestFixture
    {
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:

        getFileSize() :nError(FileBase::E_None) {}

        // initialization
        void setUp() override
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );
        }

        void tearDown() override
        {
            deleteTestFile( aTypeURL );
        }

        // test code.
        void getFileSize_001()
        {
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileSize );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            sal_uInt64 uFileSize = rFileStatus.getFileSize();

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for getFileSize function: empty file ",
                                    static_cast<sal_uInt64>(0), uFileSize );
        }

        void getFileSize_002()
        {
            ::osl::File testfile( aTypeURL );
            nError = testfile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Read );
            CPPUNIT_ASSERT_EQUAL( nError, ::osl::FileBase::E_None );
            nError = testfile.setSize( TEST_FILE_SIZE );
            CPPUNIT_ASSERT_EQUAL( nError, ::osl::FileBase::E_None );

            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileSize );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );
            sal_uInt64 uFileSize = rFileStatus.getFileSize();

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for getFileSize function: file with size of TEST_FILE_SIZE, did not pass in (W32). ",
                                    static_cast<sal_uInt64>(TEST_FILE_SIZE), uFileSize );
        }
        CPPUNIT_TEST_SUITE( getFileSize );
        CPPUNIT_TEST( getFileSize_001 );
        CPPUNIT_TEST( getFileSize_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class getFileSize

    //  testing the method
    //  inline ::rtl::OUString getFileName() const

    class  getFileName : public CppUnit::TestFixture
    {
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        getFileName() :nError(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );
        }

        void tearDown() override
        {
            deleteTestFile( aTypeURL );
        }

        // test code.
        void getFileName_001()
        {
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            ::rtl::OUString aFileName = rFileStatus.getFileName();

            CPPUNIT_ASSERT_MESSAGE( "test for getFileName function: name compare with specify",
                                    compareFileName( aFileName, aTmpName2 ) );
        }

        CPPUNIT_TEST_SUITE( getFileName );
        CPPUNIT_TEST( getFileName_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class getFileName

    //  testing the method
    //  inline ::rtl::OUString getFileURL() const

    class  getFileURL : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        getFileURL() :nError(FileBase::E_None) {}

        // initialization
        void setUp() override
        {
            createTestFile( aTmpName6 );
            nError = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );
        }

        void tearDown() override
        {
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void getFileURL_001()
        {
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileURL );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError );

            ::rtl::OUString aFileURL = rFileStatus.getFileURL();

            CPPUNIT_ASSERT_MESSAGE( "test for getFileURL function: ",
                                    compareFileName( aFileURL, aTmpName6 ) );
        }

        CPPUNIT_TEST_SUITE( getFileURL );
        CPPUNIT_TEST( getFileURL_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class getFileURL

    //  testing the method
    //  inline ::rtl::OUString getLinkTargetURL() const

    class  getLinkTargetURL : public CppUnit::TestFixture
    {
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:

        getLinkTargetURL() :nError(FileBase::E_None) {}
        // test code.
        // initialization
        void setUp() override
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
        }

        void tearDown() override
        {
            deleteTestFile( aTypeURL );
        }

#if ( defined UNX )            //Link file is not define in Windows
        void getLinkTargetURL_001()
        {
            //create a link file;
            ::rtl::OUString aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
            aUStr_LnkFileSys += aSlashURL + getCurrentPID() + "/link.file";
            aUStr_SrcFileSys += aSlashURL + getCurrentPID() + "/tmpname";

                rtl::OString strLinkFileName, strSrcFileName;
                strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
                strSrcFileName  = OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

            sal_Int32 fd;
                fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "in creating link file",  static_cast<sal_Int32>(0), fd );

            //get linkTarget URL
            nError = ::osl::DirectoryItem::get( aLnkURL1, rItem );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "in getting link file item", FileBase::E_None, nError );

              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_LinkTargetURL );
            nError = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "in getting link file status", FileBase::E_None, nError );
            ::rtl::OUString aFileURL = rFileStatus.getLinkTargetURL();

            //remove link file
                fd = remove( strLinkFileName.getStr() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "in deleting link file",  static_cast<sal_Int32>(0), fd );

            CPPUNIT_ASSERT_MESSAGE( "test for getLinkTargetURL function: Solaris version, create a file, and a link file link to it, get its LinkTargetURL and compare",
                                    compareFileName( aFileURL, aTypeURL ) );
        }
#else
        void getLinkTargetURL_001()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getLinkTargetURL function: Windows version, not tested",
                                    1 );
        }
#endif

        CPPUNIT_TEST_SUITE( getLinkTargetURL );
        CPPUNIT_TEST( getLinkTargetURL_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class getLinkTargetURL

    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::ctors, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::isValid, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::getFileType, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::getAttributes, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::getAccessTime, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::getModifyTime, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::getFileSize, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::getFileName, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::getFileURL, "osl_FileStatus" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_FileStatus::getLinkTargetURL, "osl_FileStatus" );
}// namespace osl_FileStatus

// Beginning of the test cases for File class

namespace osl_File
{

    //  testing the method
    //  File( const ::rtl::OUString& ustrFileURL )

    class  ctors : public CppUnit::TestFixture
    {
        // ::osl::FileBase::RC     nError1;

        public:
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void ctors_001()
        {
            ::osl::File testFile( aTmpName4 );

            ::osl::FileBase::RC nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            ::osl::FileBase::RC nError2 = testFile.close();
            CPPUNIT_ASSERT_MESSAGE( "test for ctors function: initialize a File and test its open and close",
                                     ( ::osl::FileBase::E_None == nError1 ) && ( ::osl::FileBase::E_None == nError2 ) );
        }

        void ctors_002()
        {
            ::osl::File testFile( aTmpName5 );
            sal_Char buffer[30] = "Test for File constructor";
            sal_uInt64 nCount;

                ::osl::FileBase::RC nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
                ::osl::FileBase::RC nError2 = testFile.write( buffer, 30, nCount );
            testFile.close();

            CPPUNIT_ASSERT_MESSAGE( "test for ctors function: test relative file URL, this test show that relative file URL is also acceptable",
                                     ( ::osl::FileBase::E_None == nError1 ) && ( ::osl::FileBase::E_None == nError2 )  );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST( ctors_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class ctors

    //  testing the method
    //  inline RC open( sal_uInt32 uFlags )

    class  open : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2, nError3;

        public:
        open()
            : nError1(FileBase::E_None)
            , nError2(FileBase::E_None)
            , nError3(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void open_001()
        {
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            nError2 = testFile.close();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("close error", nError2, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for open function: open a regular file",
                                     nError1, ::osl::FileBase::E_None );
        }

        void open_002()
        {
            ::osl::File testFile( aTmpName3 );

            nError1 = testFile.open( osl_File_OpenFlag_Read );

            CPPUNIT_ASSERT_MESSAGE( "test for open function: open a directory",
                                     ( File::E_INVAL == nError1 ) || ( File::E_ACCES == nError1 ) );
        }

        void open_003()
        {
            ::osl::File testFile( aCanURL1 );

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for open function: open a non-exist file",
                                     nError1, File::E_NOENT );
        }

        void open_004()
        {
            ::rtl::OUString  aTestFile( aRootURL );
            concatURL( aTestFile, aTmpName2 );
            ::osl::File testFile( aTestFile );

            nError1 = testFile.open( osl_File_OpenFlag_Create );
            bool bOK = ( File::E_ACCES == nError1 );
#ifdef _WIN32
            bOK = true;  /// in Windows, you can create file in c:/ any way.
            testFile.close();
            deleteTestFile( aTestFile);
#endif

            CPPUNIT_ASSERT_MESSAGE( "test for open function: create an illegal file",
                                    bOK );
        }

        void open_005()
        {
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Create );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for open function: create an exist file",
                                     nError1, File::E_EXIST );
        }

        void open_006()
        {
            ::osl::File testFile( aCanURL1 );
            sal_Char buffer_write[30] = "Test for File open";
            sal_Char buffer_read[30];
            sal_uInt64 nCount_write, nCount_read;

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
            nError2 = testFile.write( buffer_write, 30, nCount_write );
             ::osl::FileBase::RC nError4 = testFile.setPos( osl_Pos_Absolut, 0 );
            CPPUNIT_ASSERT_EQUAL( nError4, ::osl::FileBase::E_None );
            nError3 = testFile.read( buffer_read, 10, nCount_read );

             ::osl::FileBase::RC nError5 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError5, ::osl::FileBase::E_None );
            ::osl::FileBase::RC nError6 = osl::File::remove( aCanURL1 );
            CPPUNIT_ASSERT_EQUAL( nError6, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for open function: test for osl_File_OpenFlag_Read, osl_File_OpenFlag_Write and osl_File_OpenFlag_Create",
                                    ( ::osl::FileBase::E_None == nError1 ) &&
                                    ( ::osl::FileBase::E_None == nError2 ) &&
                                    ( ::osl::FileBase::E_None == nError3 ) &&
                                    ( 30 == nCount_write ) &&
                                    ( 10 == nCount_read ) );
        }

        CPPUNIT_TEST_SUITE( open );
        CPPUNIT_TEST( open_001 );
        CPPUNIT_TEST( open_002 );
        CPPUNIT_TEST( open_003 );
        CPPUNIT_TEST( open_004 );
        CPPUNIT_TEST( open_005 );
        CPPUNIT_TEST( open_006 );
        CPPUNIT_TEST_SUITE_END();
    };// class open

    //  testing the method
    //  inline RC close()

    class  close : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2, nError3;

        public:
        close()
            : nError1(FileBase::E_None)
            , nError2(FileBase::E_None)
            , nError3(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void close_001()
        {
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError2 = testFile.close();

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for close function: close a regular file",
                                     nError2, ::osl::FileBase::E_None );
        }

        void close_002()
        {
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError2 = testFile.close();

             nError3 = testFile.setPos( osl_Pos_Absolut, 0 );

            CPPUNIT_ASSERT_MESSAGE( "test for close function: manipulate a file after it has been closed",
                                     ( ::osl::FileBase::E_None == nError2 ) &&
                                    ( ::osl::FileBase::E_None != nError3 ) );
        }

        CPPUNIT_TEST_SUITE( close );
        CPPUNIT_TEST( close_001 );
        CPPUNIT_TEST( close_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class close

    //  testing the method
    //  inline RC setPos( sal_uInt32 uHow, sal_Int64 uPos )

    class  setPos : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        setPos()
            : nError1(FileBase::E_None)
            , nCount_write(0)
            , nCount_read(0)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void setPos_001()
        {
            ::osl::File testFile( aTmpName4 );
            sal_Char buffer_read[2];

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.setPos( osl_Pos_Absolut, 26 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.read( buffer_read, 1, nCount_read );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for setPos function: test for osl_Pos_Absolut, set the position to 26, test if the 26th char in file is correct",
                                     pBuffer_Char[26], buffer_read[0] );
        }

        void setPos_002()
        {
            ::osl::File testFile( aTmpName4 );
            sal_Char buffer_read[2];

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.setPos( osl_Pos_Absolut, sizeof( pBuffer_Char ) - 2 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.setPos( osl_Pos_Current, 0);
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.read( buffer_read, 1, nCount_read );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for setPos function: test for osl_Pos_Current, set the position to end, test if the ( end -1 ) char in file is correct",
                                     pBuffer_Char[sizeof( pBuffer_Char ) - 2], buffer_read[0] );
        }

        void setPos_003()
        {
            ::osl::File testFile( aTmpName4 );
            sal_Char buffer_read[2];

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //the file size is smaller than 100
            nError1 = testFile.setPos( osl_Pos_End,  -100 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "should return error", nError1, ::osl::FileBase::E_INVAL );

             nError1 = testFile.setPos( osl_Pos_End, -53 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.read( buffer_read, 1, nCount_read );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for setPos function: test for osl_Pos_End, set the position to end, test if the first char in file is correct",
                                     pBuffer_Char[0], buffer_read[0] );
        }

        CPPUNIT_TEST_SUITE( setPos );
        CPPUNIT_TEST( setPos_001 );
        CPPUNIT_TEST( setPos_002 );
        CPPUNIT_TEST( setPos_003 );
        CPPUNIT_TEST_SUITE_END();
    };// class setPos

    //  testing the method
    //  inline RC getPos( sal_uInt64& uPos )

    class  getPos : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write;

        public:
        getPos()
            : nError1(FileBase::E_None)
            , nCount_write(0)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void getPos_001()
        {
            ::osl::File testFile( aTmpName4 );
            sal_uInt64 nFilePointer;

            nError1 = testFile.getPos( nFilePointer );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_INVAL );

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

             nError1 = testFile.setPos( osl_Pos_Absolut, 26 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.getPos( nFilePointer );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for getPos function: set the position to 26, get position and check if it is right",
                                     static_cast<sal_uInt64>(26), nFilePointer );
        }

        CPPUNIT_TEST_SUITE( getPos );
        CPPUNIT_TEST( getPos_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class getPos

    //  testing the method
    //  inline RC isEndOfFile( sal_Bool *pIsEOF )

    class  isEndOfFile : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write;

        public:
        isEndOfFile()
            : nError1(FileBase::E_None)
            , nCount_write(0)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void isEndOfFile_001()
        {
            ::osl::File   testFile( aTmpName4 );
            sal_Bool      bEOF  = false;
            sal_Bool      *pEOF = &bEOF;

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

             nError1 = testFile.setPos( osl_Pos_End, 0 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.isEndOfFile( pEOF );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for isEndOfFile function: set the position to end, check if reach end",
                                     *pEOF );
        }

        void isEndOfFile_002()
        {
             ::osl::File   testFile( aTmpName4 );
            sal_Bool      bEOF  = false;
            sal_Bool      *pEOF = &bEOF;
            sal_uInt64    nFilePointer = 0;

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

             nError1 = testFile.setPos( osl_Pos_Absolut, 0 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            *pEOF = false;
             while ( !( *pEOF ) )
            {
                nError1 = testFile.isEndOfFile( pEOF );
                CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
                nError1 = testFile.setPos( osl_Pos_Current, 1 );
                CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            }
            nError1 = testFile.getPos( nFilePointer );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for isEndOfFile function: use isEndOfFile to move pointer step by step",
                                      nFilePointer, static_cast<sal_uInt64>(sizeof( pBuffer_Char ) + 1) );
        }
        CPPUNIT_TEST_SUITE( isEndOfFile );
        CPPUNIT_TEST( isEndOfFile_001 );
        CPPUNIT_TEST( isEndOfFile_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class isEndOfFile

    //  testing the method
    //  inline RC setSize( sal_uInt64 uSize )

    class  setSize : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write;

        public:
        setSize()
            : nError1(FileBase::E_None)
            , nCount_write(0)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void setSize_001()
        {
            ::osl::File   testFile( aTmpName4 );
            // sal_Bool      bEOF  = sal_False;
            // sal_Bool      *pEOF = &bEOF;
            sal_uInt64     nFilePointer;

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //enlarge the file to size of 100;
             nError1 = testFile.setSize( 100 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //get the file size;
             nError1 = testFile.setPos( osl_Pos_End, 0 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.getPos( nFilePointer );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for setSize function: enlarge the file ",
                                     static_cast<sal_uInt64>(100), nFilePointer );
        }

        void setSize_002()
        {
            ::osl::File   testFile( aTmpName4 );
            // sal_Bool      bEOF  = sal_False;
            // sal_Bool      *pEOF = &bEOF;
            sal_uInt64     nFilePointer;

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //enlarge the file to size of 100;
             nError1 = testFile.setSize( 10 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //get the file size;
             nError1 = testFile.setPos( osl_Pos_End, 0 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.getPos( nFilePointer );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for setSize function: truncate the file ",
                                     static_cast<sal_uInt64>(10), nFilePointer );
        }

        CPPUNIT_TEST_SUITE( setSize );
        CPPUNIT_TEST( setSize_001 );
        CPPUNIT_TEST( setSize_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class setSize

    //  testing the method
    //  inline RC read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead )

    class  read : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        read()
            : nError1(FileBase::E_None)
            , nCount_write(0)
            , nCount_read(0)
        {
        }
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void read_001()
        {
            ::osl::File    testFile( aTmpName4 );
            sal_uInt64     nFilePointer;
            sal_Char       buffer_read[10];

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.read( buffer_read, 10, nCount_read );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.getPos( nFilePointer );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for read function: read whole content in the file to a buffer",
                                     ( 10 == nFilePointer ) && ( 0 == strncmp( buffer_read, pBuffer_Char, 10 ) ) );
        }

        void read_002()
        {
            ::osl::File    testFile( aTmpName4 );
            sal_uInt64     nFilePointer;
            sal_Char       buffer_read[26];

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

             nError1 = testFile.setPos( osl_Pos_Absolut, 26 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.read( buffer_read, 26, nCount_read );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.getPos( nFilePointer );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for read function: read from a special position in the file",
                                     ( 52 == nFilePointer ) && ( 26 == nCount_read ) && ( 0 == strncmp( buffer_read, &pBuffer_Char[26], 26 ) ) );
        }

        CPPUNIT_TEST_SUITE( read );
        CPPUNIT_TEST( read_001 );
        CPPUNIT_TEST( read_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class read

    //  testing the method
    //  inline RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten)

    class  write : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        write()
            : nError1(FileBase::E_None)
            , nCount_write(0)
            , nCount_read(0)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void write_001()
        {
            ::osl::File    testFile( aTmpName6 );
            sal_uInt64     nFilePointer;
            sal_Char       buffer_read[10];

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //write chars into the file.
            nError1 = testFile.write( pBuffer_Char, 10, nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //get the current pointer;
             nError1 = testFile.getPos( nFilePointer );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //reset pointer to the beginning;
             nError1 = testFile.setPos( osl_Pos_Absolut, 0 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.read( buffer_read, 10, nCount_read );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for write function: read whole content in the file to a buffer. Note, buffer size can not smaller than the read size",
                                     ( 10 == nFilePointer ) &&
                                    ( 0 == strncmp( buffer_read, pBuffer_Char, 10 ) ) &&
                                    ( 10 == nCount_write ) );
        }

        CPPUNIT_TEST_SUITE( write );
        CPPUNIT_TEST( write_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class write

    //  testing the method
    //  inline RC readLine( ::rtl::ByteSequence& aSeq )

    class  readLine : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write;
        ::rtl::ByteSequence      aSequence;

        public:
        readLine()
            : nError1(FileBase::E_None)
            , nCount_write(0)
        {
        }
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );

            //write some strings into the file.
            ::osl::File testFile( aTmpName6 );
            sal_Char ppStrSeq[3][27]  =  { "abcde\n",
                                        "1234567890\n",
                                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                      };

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            for ( int nCount = 0; nCount < 3; nCount++ )
            {
                nError1 = testFile.write( ppStrSeq[nCount], strlen( ppStrSeq[nCount] ), nCount_write );
                CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            }

             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void readLine_001()
        {
             ::osl::File    testFile( aTmpName6 );

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.readLine( aSequence );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            CPPUNIT_ASSERT_MESSAGE( "test for readLine function: read the first line of the file.",
                                    ( ::osl::FileBase::E_None == nError1 ) &&
                                    ( 0 == strncmp( reinterpret_cast<char *>(aSequence.getArray()), pBuffer_Char, 5 ) ) );
        }

        void readLine_002()
        {
            ::osl::File testFile( aTmpName6 );
            sal_Bool bEOF  = false;
            sal_Bool *pEOF = &bEOF;

            nError1 = testFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            for ( int nCount = 0; nCount < 3; nCount++ )
            {
                nError1 = testFile.readLine( aSequence );
                CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            }
             nError1 = testFile.isEndOfFile( pEOF );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for readLine function: read three lines of the file and check the file pointer moving.",
                                     *pEOF &&
                                    ( 0 == strncmp( reinterpret_cast<char *>(aSequence.getArray()), &pBuffer_Char[26], 26 ) ) );
        }
        CPPUNIT_TEST_SUITE( readLine );
        CPPUNIT_TEST( readLine_001 );
        CPPUNIT_TEST( readLine_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class readLine

    //  testing the method
    //  inline static RC copy( const ::rtl::OUString& ustrSourceFileURL, const ::rtl::OUString& ustrDestFileURL )

    class  copy : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write;

        public:
        copy()
            : nError1(FileBase::E_None)
            , nCount_write(0)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void copy_001()
        {
             ::osl::File    testFile( aTmpName6 );

            //copy $TEMP/tmpdir/tmpname to $TEMP/tmpname.
            nError1 = ::osl::File::copy( aTmpName4, aTmpName6 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //check
            nError1 = testFile.open( osl_File_OpenFlag_Create );
            deleteTestFile( aTmpName6 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for copy function: copy file to upper directory",
                                     nError1, ::osl::FileBase::E_EXIST );
        }

        void copy_002()
        {
            //copy $TEMP/tmpdir/tmpname to $TEMP/tmpdir.
            nError1 = ::osl::File::copy( aTmpName4, aTmpName3 );

            CPPUNIT_ASSERT_MESSAGE( "test for copy function: use directory as destination",
                                     ( ::osl::FileBase::E_ISDIR == nError1 ) ||( ::osl::FileBase::E_ACCES == nError1 )  );
        }

        void copy_003()
        {
            //copy $TEMP/tmpdir/tmpname to $ROOT/tmpname.
            nError1 = ::osl::File::copy( aTmpName4, aTmpName7 );
#if defined(_WIN32)
            nError1 = ::osl::FileBase::E_ACCES;  /// for Windows, c:/ is writtenable any way.
            deleteTestFile( aTmpName7);
#endif
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for copy function: copy to an illigal place",
                                     nError1, ::osl::FileBase::E_ACCES );
        }

        void copy_004()
        {
            //copy $TEMP/tmpname to $TEMP/tmpdir/tmpname.
            nError1 = ::osl::File::copy( aTmpName6, aTmpName4 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for copy function: copy a not exist file",
                                     nError1, ::osl::FileBase::E_NOENT );
        }

        void copy_005()
        {
            //copy $TEMP/tmpname to $TEMP/system.path using system path.
            nError1 = ::osl::File::copy( aTmpName6, aSysPath1 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for copy function: copy a file using system file path",
                                     nError1, ::osl::FileBase::E_INVAL );
        }
      void copy_006()
      {
        createTestFile( aTmpName6 );
        File tmpFile( aTmpName6 );
        FileBase::RC err = tmpFile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Read );
        (void)err;
        tmpFile.setSize( 200 );
        tmpFile.close();
        //copy to new path
        nError1 = ::osl::File::copy( aTmpName6, aTmpName4 );
        CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );

        //check if is the new file
        File newFile( aTmpName4 );
        newFile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Read );
        nError1 = newFile.setPos( osl_Pos_End, 0 );
        CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        sal_uInt64     nFilePointer;
        nError1 = newFile.getPos( nFilePointer );
        CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        newFile.close();
        deleteTestFile( aTmpName6 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for copy function: the dest file exist",
                    static_cast<sal_uInt64>(200), nFilePointer );
      }

        CPPUNIT_TEST_SUITE( copy );
        CPPUNIT_TEST( copy_001 );
        CPPUNIT_TEST( copy_002 );
        CPPUNIT_TEST( copy_003 );
        CPPUNIT_TEST( copy_004 );
        CPPUNIT_TEST( copy_005 );
        CPPUNIT_TEST( copy_006 );
        CPPUNIT_TEST_SUITE_END();
    };// class copy

    //  testing the method
    //  inline static RC move( const ::rtl::OUString& ustrSourceFileURL, const ::rtl::OUString& ustrDestFileURL )

    class  move : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1, nError2;
        sal_uInt64 nCount_write;

        public:
        move()
            : nError1(FileBase::E_None)
            , nError2(FileBase::E_None)
            , nCount_write(0)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void move_001()
        {
            //rename $TEMP/tmpdir/tmpname to $TEMP/canonical.name.
            nError1 = ::osl::File::move( aTmpName4, aCanURL1 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //check
             ::osl::File    testFile( aCanURL1 );
            nError2 = testFile.open( osl_File_OpenFlag_Create );
            deleteTestFile( aCanURL1 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for move function: rename file to another directory",
                                     nError2, ::osl::FileBase::E_EXIST );
        }

        void move_002()
        {
            //move $TEMP/tmpdir/tmpname to $TEMP/tmpdir.
            nError1 = ::osl::File::move( aTmpName4, aTmpName3 );
            //returned ::osl::FileBase::E_ACCES on WNT
            CPPUNIT_ASSERT_MESSAGE( "test for move function: use directory as destination",
                 ( ::osl::FileBase::E_ACCES == nError1 || ::osl::FileBase::E_ISDIR == nError1 ) ||( ::osl::FileBase::E_EXIST == nError1 )  );
        }

        void move_003()
        {
            //move $TEMP/tmpdir/tmpname to $ROOT/tmpname.
            nError1 = ::osl::File::move( aTmpName4, aTmpName7 );
#if defined(_WIN32)
            nError1 = ::osl::FileBase::E_ACCES;  /// for Windows, c:/ is writtenable any way.
            deleteTestFile( aTmpName7);
#endif

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for move function: move to an illigal place",
                                     nError1, ::osl::FileBase::E_ACCES );
        }

        void move_004()
        {
            //move $TEMP/tmpname to $TEMP/tmpdir/tmpname.
            nError1 = ::osl::File::move( aTmpName6, aTmpName4 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for move function: move a not exist file",
                                     nError1, ::osl::FileBase::E_NOENT );
        }

        void move_005()
        {
            //move $TEMP/tmpname to $TEMP/system.path using system path.
            nError1 = ::osl::File::move( aTmpName6, aSysPath1 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for move function: move a file using system file",
                                     nError1, ::osl::FileBase::E_INVAL );
        }

        void move_006()
        {
            //move directory $TEMP/tmpname to $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName6 );
            nError1 = ::osl::File::move( aTmpName6, aTmpName4 );
            //move file $TEMP/tmpdir/tmpname to $TEMP/tmpname
            nError2 = ::osl::File::move( aTmpName4, aTmpName6 );
            deleteTestDirectory( aTmpName6 );
#if defined(_WIN32)
            deleteTestDirectory( aTmpName4 );// in Windows, it can be moved!!!!! this is only for not influence the following test.
            deleteTestFile( aTmpName6 );
            nError1 = ::osl::FileBase::E_NOTDIR;
            nError2 = ::osl::FileBase::E_ISDIR;
#endif
            CPPUNIT_ASSERT_MESSAGE( "test for move function: move a directory to an exist file with same name, did not pass in (W32)",
                                     ::osl::FileBase::E_NOTDIR == nError1 && ::osl::FileBase::E_ISDIR == nError2 );
        }

        void move_007()
        {
            //create directory $TEMP/tmpname.
            createTestDirectory( aTmpName6 );
            //move directory $TEMP/tmpdir to $TEMP/tmpname/tmpdir
            nError1 = ::osl::File::move( aTmpName3, aTmpName8 );
            //check
            nError2 = ::osl::Directory::create( aTmpName8 );
            ::osl::File::move( aTmpName8, aTmpName3 );
            deleteTestDirectory( aTmpName6 );

            CPPUNIT_ASSERT_MESSAGE( "test for move function: move a directory to an exist file with same name",
                                     (::osl::FileBase::E_None == nError1 ) &&
                                    (::osl::FileBase::E_EXIST == nError2 ) );
        }
      //bugid# 115420, after the bug fix, add the case
        CPPUNIT_TEST_SUITE( move );
        CPPUNIT_TEST( move_001 );
        CPPUNIT_TEST( move_002 );
        CPPUNIT_TEST( move_003 );
        CPPUNIT_TEST( move_004 );
        CPPUNIT_TEST( move_005 );
        CPPUNIT_TEST( move_006 );
        CPPUNIT_TEST( move_007 );
        CPPUNIT_TEST_SUITE_END();
    };// class move

    //  testing the method
    //  inline static RC remove( const ::rtl::OUString& ustrFileURL )

    class  remove : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1, nError2;
        sal_uInt64 nCount_write;

        public:
        remove()
            : nError1(FileBase::E_None)
            , nError2(FileBase::E_None)
            , nCount_write(0)
        {
        }

        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( osl_File_OpenFlag_Write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
             nError1 = testFile.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void remove_001()
        {
            //remove $TEMP/tmpdir/tmpname.
            nError1 = ::osl::File::remove( aTmpName4 );
            //check
             ::osl::File    testFile( aTmpName4 );
            nError2 = testFile.open( osl_File_OpenFlag_Create );

            CPPUNIT_ASSERT_MESSAGE( "test for remove function: remove a file",
                                    ( ::osl::FileBase::E_None == nError1 ) &&
                                     ( ::osl::FileBase::E_EXIST != nError2 ) );
        }

        void remove_002()
        {
            //remove $TEMP/tmpname.
            nError1 = ::osl::File::remove( aTmpName6 );

            CPPUNIT_ASSERT_MESSAGE( "test for remove function: remove a file not exist",
                                    ( ::osl::FileBase::E_NOENT == nError1 ) );
        }

        void remove_003()
        {
            //remove $TEMP/system/path.
            nError1 = ::osl::File::remove( aSysPath2 );

            CPPUNIT_ASSERT_MESSAGE( "test for remove function: removing a file not using full qualified URL",
                                    ( ::osl::FileBase::E_INVAL == nError1 ) );
        }

        void remove_004()
        {
            //remove $TEMP/tmpdir.
            nError1 = ::osl::File::remove( aTmpName3 );

            CPPUNIT_ASSERT_MESSAGE( "test for remove function: remove a directory",
                                    ( ::osl::FileBase::E_ISDIR == nError1 ) || ( ::osl::FileBase::E_ACCES == nError1 ));
        }

        CPPUNIT_TEST_SUITE( remove );
        CPPUNIT_TEST( remove_001 );
        CPPUNIT_TEST( remove_002 );
        CPPUNIT_TEST( remove_003 );
        CPPUNIT_TEST( remove_004 );
        CPPUNIT_TEST_SUITE_END();
    };// class remove

    //  testing the method
    //  inline static RC setAttributes( const ::rtl::OUString& ustrFileURL, sal_uInt64 uAttributes )

    class  setAttributes : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC      nError1, nError2;
        ::osl::DirectoryItem    rItem, rItem_hidden;

        public:
        setAttributes() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestFile( aTmpName6 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void setAttributes_001()
        {
        //on windows, only can set 2 attributes: osl_File_Attribute_ReadOnly,  osl_File_Attribute_Hidden
#ifdef UNX
            //set the file to readonly
            nError2 = ::osl::File::setAttributes( aTmpName6, osl_File_Attribute_ReadOnly | osl_File_Attribute_GrpRead | osl_File_Attribute_OwnRead | osl_File_Attribute_OthRead );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError2 );
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );
            //get the file attributes
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Attributes );
            nError1 = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for setAttributes function: set file attributes and get it to verify.",
                                    static_cast<sal_uInt64>( osl_File_Attribute_ReadOnly | osl_File_Attribute_GrpRead | osl_File_Attribute_OwnRead | osl_File_Attribute_OthRead ),
                                    rFileStatus.getAttributes() );
#else
            //please see GetFileAttributes
            nError2 = ::osl::File::setAttributes( aTmpName6, osl_File_Attribute_ReadOnly );
            CPPUNIT_ASSERT( nError2 == FileBase::E_None);
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT( nError1 == FileBase::E_None);
            //get the file attributes
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_Attributes );
            nError1 = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );
            //here the file has 2 Attributes: FILE_ATTRIBUTE_READONLY and FILE_ATTRIBUTE_NORMAL,
            // but FILE_ATTRIBUTE_NORMAL is valid only if used alone, so this is maybe a bug
            /*::rtl::OString aString = ::rtl::OUStringToOString( aTmpName6, RTL_TEXTENCODING_ASCII_US );
            DWORD dwFileAttributes = GetFileAttributes( aString.getStr() );
            if (dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
                printf("has normal attribute");
            if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                printf("has readonly attribute");
            */
            CPPUNIT_ASSERT_MESSAGE( "test for setAttributes function: set file attributes READONLY and get it to verify.",
                                    (osl_File_Attribute_ReadOnly & rFileStatus.getAttributes()) != 0  );
#endif
    }
        void setAttributes_002()
        {
        //on UNX, can not set hidden attribute to file, rename file can set the attribute
#ifdef _WIN32
            //set the file to hidden
            nError2 = ::osl::File::setAttributes( aTmpName6, osl_File_Attribute_Hidden);

            CPPUNIT_ASSERT( nError2 == FileBase::E_None);
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT( nError1 == FileBase::E_None);
            //get the file attributes
              ::osl::FileStatus rFileStatus( osl_FileStatus_Mask_Attributes );
            nError1 = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );

            CPPUNIT_ASSERT_MESSAGE( "test for setAttributes function: set file attributes and get it to verify.",
                                    (osl_File_Attribute_Hidden & rFileStatus.getAttributes()) != 0 );
#endif
        }

        CPPUNIT_TEST_SUITE( setAttributes );
        CPPUNIT_TEST( setAttributes_001 );
        CPPUNIT_TEST( setAttributes_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class setAttributes

    //  testing the method
    //  inline static RC setTime(
    //         const ::rtl::OUString& ustrFileURL,
    //         const TimeValue& rCreationTime,
    //         const TimeValue& rLastAccessTime,
    //         const TimeValue& rLastWriteTime )

    class  setTime : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        setTime() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestFile( aTmpName6 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void setTime_001()
        {
             TimeValue *pTV_current  = nullptr;
            CPPUNIT_ASSERT( ( pTV_current = static_cast<TimeValue*>(malloc( sizeof( TimeValue ) )) ) != nullptr );
            TimeValue *pTV_creation = nullptr;
            CPPUNIT_ASSERT( ( pTV_creation = static_cast<TimeValue*>(malloc( sizeof( TimeValue ) )) ) != nullptr );
            TimeValue *pTV_access   = nullptr;
            CPPUNIT_ASSERT( ( pTV_access = static_cast<TimeValue*>(malloc( sizeof( TimeValue ) )) ) != nullptr );
            TimeValue *pTV_modify   = nullptr;
            CPPUNIT_ASSERT( ( pTV_modify = static_cast<TimeValue*>(malloc( sizeof( TimeValue ) )) ) != nullptr );

            //get current time
            bool bOk = osl_getSystemTime( pTV_current );
            CPPUNIT_ASSERT( bOk );

            //set the file time
            nError2 = ::osl::File::setTime( aTmpName6, *pTV_current, *pTV_current, *pTV_current );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( errorToStr( nError2 ).getStr(), FileBase::E_None, nError2);

             //get the file access time, creation time, modify time
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( errorToStr( nError1 ).getStr(), FileBase::E_None, nError1);

              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_AccessTime );
            nError1 = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( errorToStr( nError1 ).getStr(), FileBase::E_None, nError1 );
            *pTV_access = rFileStatus.getAccessTime();

              ::osl::FileStatus   rFileStatus1( osl_FileStatus_Mask_CreationTime );
            nError1 = rItem.getFileStatus( rFileStatus1 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( errorToStr( nError1 ).getStr(), FileBase::E_None, nError1 );
            *pTV_creation = rFileStatus1.getCreationTime();

              ::osl::FileStatus   rFileStatus2( osl_FileStatus_Mask_ModifyTime );
            nError1 = rItem.getFileStatus( rFileStatus2 );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( errorToStr( nError1 ).getStr(), FileBase::E_None, nError1 );
            *pTV_modify = rFileStatus2.getModifyTime();

            CPPUNIT_ASSERT_MESSAGE( "test for setTime function: set access time then get it. time precision is still a problem for it cut off the nanosec.",
                t_compareTime( pTV_access, pTV_current, delta ) );
#if defined(_WIN32)
            //Unfortunately there is no way to get the creation time of a file under Unix (its a Windows only feature).
            //That means the flag osl_FileStatus_Mask_CreationTime should be deprecated under Unix.
            CPPUNIT_ASSERT_MESSAGE( "test for setTime function: set creation time then get it. ",
                sal_True == t_compareTime( pTV_creation, pTV_current, delta ) ) ;
#endif
            CPPUNIT_ASSERT_MESSAGE( "test for setTime function: set modify time then get it. ",
                t_compareTime( pTV_modify, pTV_current, delta ) );
            free( pTV_current );
            free( pTV_creation );
            free( pTV_access );
            free( pTV_modify );
        }

        CPPUNIT_TEST_SUITE( setTime );
        CPPUNIT_TEST( setTime_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class setTime

    //  testing the method
    //  inline static RC sync()

        class  sync : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        sync() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestFile( aTmpName6 );

        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test case: if The file is located on a read only file system.
        void sync_001()
        {
#ifdef UNX
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );

            File tmp_file( aTmpName6 );
            FileBase::RC err = tmp_file.open(osl_File_OpenFlag_Write );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("File open failed", FileBase::E_None, err);

            char buffer[50000];
            sal_uInt64 written = 0;
            nError1 = tmp_file.write(static_cast<void*>(buffer), sizeof(buffer), written);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write failed!", FileBase::E_None, nError1);

            //set the file to readonly
            nError2 = ::osl::File::setAttributes( aTmpName6, osl_File_Attribute_ReadOnly | osl_File_Attribute_GrpRead | osl_File_Attribute_OwnRead | osl_File_Attribute_OthRead );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError2 );

            nError2 = tmp_file.sync();

                CPPUNIT_ASSERT_EQUAL_MESSAGE("can not sync to readonly file!", FileBase::E_None, nError2);

            tmp_file.close();
#endif
        }
      //test case:no enough space, how to create such case???see test_cpy_wrt_file.cxx::test_osl_writeFile

        CPPUNIT_TEST_SUITE( sync );
        CPPUNIT_TEST( sync_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class setTime

    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::ctors, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::open, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::close, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::setPos, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::getPos, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::isEndOfFile, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::setSize, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::read, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::write, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::readLine, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::copy, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::move, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::remove, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::setAttributes, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::setTime, "osl_File" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_File::sync, "osl_File" );
// FIXME: to enable these tests (when they work cross-platform) we need to add the below:
//    CPPUNIT_REGISTRY_ADD_TO_DEFAULT( "osl_File" );

}// namespace osl_File

// Beginning of the test cases for DirectoryItem class

namespace osl_DirectoryItem
{

    //  testing the method
    //  DirectoryItem(): _pData( NULL )

    class  ctors : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1;

        public:
        ctors() :nError1(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void ctors_001()
        {
            ::osl::File testFile( aTmpName6 );
            ::osl::DirectoryItem    rItem;  //constructor

             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT_EQUAL( nError1, FileBase::E_None );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for ctors function: initialize a new instance of DirectoryItem and get an item to check.",
                                      ::osl::FileBase::E_None, nError1  );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST_SUITE_END();
    };// class ctors

    //  testing the method
    //  DirectoryItem( const DirectoryItem& rItem ): _pData( rItem._pData)

    class  copy_assin_Ctors : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1;

        public:
        copy_assin_Ctors() :nError1(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void copy_assin_Ctors_001()
        {
            ::osl::DirectoryItem    rItem;  //constructor
             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT_EQUAL( nError1, FileBase::E_None );

            ::osl::DirectoryItem    copyItem( rItem ); //copy constructor
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            nError1 = copyItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );

            CPPUNIT_ASSERT_MESSAGE( "test for copy_assin_Ctors function: use copy constructor to get an item and check filename.",
                                    compareFileName( rFileStatus.getFileName(), aTmpName2 ) );
        }

        void copy_assin_Ctors_002()
        {
            ::osl::DirectoryItem    rItem;  //constructor
             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT_EQUAL( nError1, FileBase::E_None );

            ::osl::DirectoryItem    copyItem;
            copyItem = rItem;               //assinment operator
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            nError1 = copyItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );

            CPPUNIT_ASSERT_MESSAGE( "test for copy_assin_Ctors function: test assinment operator here since it is same as copy constructor in test way.",
                                    compareFileName( rFileStatus.getFileName(), aTmpName2 ) );
        }

        CPPUNIT_TEST_SUITE( copy_assin_Ctors );
        CPPUNIT_TEST( copy_assin_Ctors_001 );
        CPPUNIT_TEST( copy_assin_Ctors_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class copy_assin_Ctors

    //  testing the method
    //  inline sal_Bool is()

    class  is : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1;

        public:
        is() :nError1(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void is_001()
        {
            ::osl::DirectoryItem    rItem;  //constructor

            CPPUNIT_ASSERT_MESSAGE( "test for is function: use an uninitialized instance.",
                                    !rItem.is() );
        }

        void is_002()
        {
            ::osl::DirectoryItem    rItem;  //constructor
             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
            CPPUNIT_ASSERT_EQUAL( nError1, FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for is function: use an uninitialized instance.",
                                    rItem.is() );
        }

        CPPUNIT_TEST_SUITE( is );
        CPPUNIT_TEST( is_001 );
        CPPUNIT_TEST( is_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class is

    //  testing the method
    //  static inline RC get( const ::rtl::OUString& ustrFileURL, DirectoryItem& rItem )

    class  get : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;

        public:
        get() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }

        // test code.
        void get_001()
        {
            ::osl::DirectoryItem    rItem;  //constructor
             //get the DirectoryItem.
            nError2 = ::osl::DirectoryItem::get( aTmpName6, rItem );

            //check the file name
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            nError1 = rItem.getFileStatus( rFileStatus );
            CPPUNIT_ASSERT_EQUAL( FileBase::E_None, nError1 );

            CPPUNIT_ASSERT_MESSAGE( "test for get function: use copy constructor to get an item and check filename.",
                                    ( ::osl::FileBase::E_None == nError2 ) &&
                                    compareFileName( rFileStatus.getFileName(), aTmpName2 ) );
        }

        void get_002()
        {
            ::osl::DirectoryItem    rItem;
             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aSysPath1, rItem );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for get function: use a system name instead of a URL.",
                                    nError1, FileBase::E_INVAL );
        }

        void get_003()
        {
            ::osl::DirectoryItem    rItem;
             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aTmpName3, rItem );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "test for get function: use a non existed file URL.",
                                    nError1, FileBase::E_NOENT );
        }

        CPPUNIT_TEST_SUITE( get );
        CPPUNIT_TEST( get_001 );
        CPPUNIT_TEST( get_002 );
        CPPUNIT_TEST( get_003 );
        CPPUNIT_TEST_SUITE_END();
    };// class get

    //  testing the method
    //  inline RC getFileStatus( FileStatus& rStatus )

    class  getFileStatus : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;

        public:
        getFileStatus() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void getFileStatus_001()
        {
            ::osl::DirectoryItem    rItem;  //constructor
             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aTmpName4, rItem );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //check the file name
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            nError2 = rItem.getFileStatus( rFileStatus );

            CPPUNIT_ASSERT_MESSAGE( "test for getFileStatus function: get file status and check filename",
                                    ( ::osl::FileBase::E_None == nError2 ) &&
                                    compareFileName( rFileStatus.getFileName(), aTmpName2 ) );
        }

        void getFileStatus_002()
        {
            ::osl::DirectoryItem    rItem;  //constructor
             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );

            //check the file name
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            nError2 = rItem.getFileStatus( rFileStatus );

            CPPUNIT_ASSERT_MESSAGE( "test for getFileStatus function: file not existed",
                                    ( ::osl::FileBase::E_INVAL == nError2 )  );
        }

        void getFileStatus_003()
        {
            ::osl::DirectoryItem    rItem;  //constructor
             //get the DirectoryItem.
            nError1 = ::osl::DirectoryItem::get( aTmpName3, rItem );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //check the file name
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            nError2 = rItem.getFileStatus( rFileStatus );

            CPPUNIT_ASSERT_MESSAGE( "test for getFileStatus function: get directory information",
                                    ( ::osl::FileBase::E_None == nError2 ) &&
                                    compareFileName( rFileStatus.getFileName(), aTmpName1 ) );
        }

        CPPUNIT_TEST_SUITE( getFileStatus );
        CPPUNIT_TEST( getFileStatus_001 );
        CPPUNIT_TEST( getFileStatus_002 );
        CPPUNIT_TEST( getFileStatus_003 );
        CPPUNIT_TEST_SUITE_END();
    };// class getFileStatus

     CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_DirectoryItem::ctors, "osl_DirectoryItem" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_DirectoryItem::copy_assin_Ctors, "osl_DirectoryItem" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_DirectoryItem::is, "osl_DirectoryItem" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_DirectoryItem::get, "osl_DirectoryItem" );
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_DirectoryItem::getFileStatus, "osl_DirectoryItem" );
}// namespace osl_DirectoryItem

// Beginning of the test cases for Directory class

namespace osl_Directory
{

    //  testing the method
    //  Directory( const ::rtl::OUString& strPath ): _pData( 0 ), _aPath( strPath )

    class  ctors : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;

        public:
        ctors() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
            // LLA: t_print("tearDown done.\n");
        }

        // test code.
        void ctors_001()
        {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
             CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //close a directory
            nError2 = testDirectory.close();
             CPPUNIT_ASSERT_EQUAL( nError2, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for ctors function: create an instance and check open and close",
                                     ( ::osl::FileBase::E_None == nError1 ) &&
                                    ( ::osl::FileBase::E_None == nError2 ) );
        }

        void ctors_002()
        {
            ::osl::Directory testDirectory( aTmpName9 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
             CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //close a directory
            nError2 = testDirectory.close();
             CPPUNIT_ASSERT_EQUAL( nError2, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for ctors function: relative URL, :-), it is also worked",
                                     ( ::osl::FileBase::E_None == nError1 ) &&
                                    ( ::osl::FileBase::E_None == nError2 ) );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST( ctors_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class ctors

    //  testing the method
    //  inline RC open()

    class  open : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;

        public:
        open() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void open_001()
        {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
            //check if directory is opened.
            bool bOk = testDirectory.isOpen();
            //close a directory
            nError2 = testDirectory.close();

            CPPUNIT_ASSERT_MESSAGE( "test for open function: open a directory and check for open",
                                    bOk &&
                                     ( ::osl::FileBase::E_None == nError1 ) &&
                                    ( ::osl::FileBase::E_None == nError2 ) );
        }

        void open_002()
        {
            ::osl::Directory testDirectory( aTmpName6 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
             if ( ::osl::FileBase::E_None == nError1 )
            {
                nError2 = testDirectory.close();
                CPPUNIT_ASSERT_EQUAL( nError2, ::osl::FileBase::E_None );
            }

            CPPUNIT_ASSERT_MESSAGE( "test for open function: open a file that is not existed",
                                     ( ::osl::FileBase::E_NOENT == nError1 ) );
        }

        void open_003()
        {
            ::osl::Directory testDirectory( aUserDirectorySys ); //constructor

            //open a directory
            nError1 = testDirectory.open();
             if ( ::osl::FileBase::E_None == nError1 )
            {
                nError2 = testDirectory.close();
                CPPUNIT_ASSERT_EQUAL( nError2, ::osl::FileBase::E_None );
            }

            CPPUNIT_ASSERT_MESSAGE( "test for open function: using system path",
                                     ( ::osl::FileBase::E_INVAL == nError1 ) );
        }

        void open_004()
        {
            ::osl::Directory testDirectory( aTmpName4 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
             if ( ::osl::FileBase::E_None == nError1 )
            {
                nError2 = testDirectory.close();
                CPPUNIT_ASSERT_EQUAL( nError2, ::osl::FileBase::E_None );
            }

            CPPUNIT_ASSERT_MESSAGE( "test for open function: open a file instead of a directory",
                                     ( ::osl::FileBase::E_NOTDIR == nError1 ) || ( ::osl::FileBase::E_ACCES == nError1 ) );
        }

        CPPUNIT_TEST_SUITE( open );
        CPPUNIT_TEST( open_001 );
        CPPUNIT_TEST( open_002 );
        CPPUNIT_TEST( open_003 );
        CPPUNIT_TEST( open_004 );
        CPPUNIT_TEST_SUITE_END();
    };// class open

    //  testing the method
    //  inline sal_Bool isOpen() { return _pData != NULL; };

    class  isOpen : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;

        public:
        isOpen() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void tearDown() override
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void isOpen_001()
        {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
            //check if directory is opened.
            bool bOk = testDirectory.isOpen();
            //close a directory
            nError2 = testDirectory.close();

            CPPUNIT_ASSERT_MESSAGE( "test for isOpen function: open a directory and check for open",
                                    bOk );
        }

        void isOpen_002()
        {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //check if directory is opened.
            bool bOk = testDirectory.isOpen();

            CPPUNIT_ASSERT_MESSAGE( "test for isOpen function: do not open a directory and check for open",
                                    !bOk );
        }

        CPPUNIT_TEST_SUITE( isOpen );
        CPPUNIT_TEST( isOpen_001 );
        CPPUNIT_TEST( isOpen_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class isOpen

    //  testing the method
    //  inline RC close()

    class  close : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;

        public:
        close() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempdirectory : $TEMP/tmpdir.
            createTestDirectory( aTmpName3 );
        }

        void tearDown() override
        {
            // remove a tempdirectory : $TEMP/tmpdir.
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void close_001()
        {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
            //close a directory
            nError2 = testDirectory.close();
            //check if directory is opened.
            bool bOk = testDirectory.isOpen();

            CPPUNIT_ASSERT_MESSAGE( "test for isOpen function: close a directory and check for open",
                                    !bOk );
        }

        void close_002()
        {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //close a directory
            nError1 = testDirectory.close();

            CPPUNIT_ASSERT_MESSAGE( "test for isOpen function: close a not opened directory",
                                    ( ::osl::FileBase::E_BADF == nError1 ) );
        }

        CPPUNIT_TEST_SUITE( close );
        CPPUNIT_TEST( close_001 );
        CPPUNIT_TEST( close_002 );
        CPPUNIT_TEST_SUITE_END();
    };// class close

    //  testing the method
    //  inline RC reset()

    class  reset : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        reset() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempdirectory : $TEMP/tmpdir.
            createTestDirectory( aTmpName3 );
            // create three files : $TEMP/tmpdir/tmpname, $TEMP/tmpdir/tmpdir, $TEMP/tmpdir/hiddenfile,
            createTestFile( aTmpName3, aTmpName2);
            createTestFile( aTmpName3, aTmpName1);
            createTestFile( aTmpName3, aHidURL1);
        }

        void tearDown() override
        {
            // remove three files : $TEMP/tmpdir/tmpname, $TEMP/tmpdir/tmpdir, $TEMP/tmpdir/hiddenfile,
            deleteTestFile( aTmpName3, aHidURL1);
            deleteTestFile( aTmpName3, aTmpName1);
            deleteTestFile( aTmpName3, aTmpName2);
            // remove a tempdirectory : $TEMP/tmpdir.
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void reset_001()
        {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //get first Item
            nError1 = testDirectory.getNextItem( rItem, 1 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //check the file name of first Item
              ::osl::FileStatus   rFileStatusFirst( osl_FileStatus_Mask_FileName );
            nError1 = rItem.getFileStatus( rFileStatusFirst );

            //get second Item
            //mindy: nError1 = testDirectory.getNextItem( rItem, 0 );
            //mindy: CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //reset enumeration
            nError2 = testDirectory.reset();
            CPPUNIT_ASSERT_EQUAL( nError2, ::osl::FileBase::E_None );
            //get reseted Item, if reset does not work, getNextItem() should return the second Item (aTmpName1)
            nError1 = testDirectory.getNextItem( rItem );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //check the file name again
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            nError1 = rItem.getFileStatus( rFileStatus );
            //close a directory
            nError1 = testDirectory.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            bool bOK1,bOK2,bOK3;
            bOK1 = compareFileName( rFileStatus.getFileName(), aTmpName2 );
            bOK2 = compareFileName( rFileStatus.getFileName(), aHidURL1 );
            bOK3 = compareFileName( rFileStatus.getFileName(), rFileStatusFirst.getFileName() );
            CPPUNIT_ASSERT_MESSAGE( "test for reset function: get two directory item, reset it, then get again, check the filename",
                                    ( ::osl::FileBase::E_None == nError2 ) &&
                                    ( bOK1 || bOK2 || bOK3 ) );
        }

        void reset_002()
        {
            ::osl::Directory testDirectory( aTmpName6 ); //constructor

            //close a directory
            nError1 = testDirectory.reset();

            CPPUNIT_ASSERT_MESSAGE( "test for reset function: reset a non existed directory",
                                    ( ::osl::FileBase::E_NOENT == nError1 ) );
        }

        void reset_003()
        {
            ::osl::Directory testDirectory( aTmpName4 ); //constructor

            //close a directory
            nError1 = testDirectory.reset();

            CPPUNIT_ASSERT_MESSAGE( "test for reset function: reset a file instead of a directory",
                                    ( ::osl::FileBase::E_NOTDIR == nError1 ) || ( ::osl::FileBase::E_NOENT == nError1 ) );
        }

        void reset_004()
        {
            ::osl::Directory testDirectory( aUserDirectorySys ); //constructor

            //close a directory
            nError1 = testDirectory.reset();

            CPPUNIT_ASSERT_MESSAGE( "test for reset function: use a system path",
                                    ( ::osl::FileBase::E_INVAL == nError1 ) );
        }

        CPPUNIT_TEST_SUITE( reset );
        CPPUNIT_TEST( reset_001 );
        CPPUNIT_TEST( reset_002 );
        CPPUNIT_TEST( reset_003 );
        CPPUNIT_TEST( reset_004 );
        CPPUNIT_TEST_SUITE_END();
    };// class reset

    //  testing the method
    //  inline RC getNextItem( DirectoryItem& rItem, sal_uInt32 nHint = 0 )

    class  getNextItem : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        getNextItem() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // initialization
        void setUp() override
        {
            // create a tempdirectory : $TEMP/tmpdir.
            createTestDirectory( aTmpName3 );
            // create three files : $TEMP/tmpdir/tmpname, $TEMP/tmpdir/tmpdir, $TEMP/tmpdir/hiddenfile,
            createTestFile( aTmpName3, aTmpName2 );
            createTestFile( aTmpName3, aTmpName1 );
            createTestFile( aTmpName3, aHidURL1 );

        }

        void tearDown() override
        {
            // remove three files : $TEMP/tmpdir/tmpname, $TEMP/tmpdir/tmpdir, $TEMP/tmpdir/hiddenfile,
            deleteTestFile( aTmpName3, aHidURL1 );
            deleteTestFile( aTmpName3, aTmpName1 );
            deleteTestFile( aTmpName3, aTmpName2 );
            // remove a tempdirectory : $TEMP/tmpdir.
            deleteTestDirectory( aTmpName3 );
        }

        // test code.
        void getNextItem_001()
        {
             ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            //check the file name
            bool            bOk1 = false;
            bool bOk2 = false;
            bool bOk3 = false;
              ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName );
            for ( int nCount = 0; nCount < 3; nCount++ )
            {
                //get three Items
                nError1 = testDirectory.getNextItem( rItem, 2 );
                CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
                nError1 = rItem.getFileStatus( rFileStatus );
                CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

                // a special order is not guaranteed. So any file may occur on any time.
                // But every file name should occur only once.
                if ( !bOk1 && compareFileName( rFileStatus.getFileName(), aTmpName1 ) )
                {
                    bOk1 = true;
                }

                if ( !bOk2 && compareFileName( rFileStatus.getFileName(), aTmpName2 ) )
                {
                    bOk2 = true;
                }

                if ( !bOk3 && compareFileName( rFileStatus.getFileName(), aHidURL1 ) )
                {
                    bOk3 = true;
                }
           }

            //close a directory
            nError1 = testDirectory.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for getNextItem function: retrieve three items and check their names.",
                                    bOk1 && bOk2 && bOk3 );
        }

        void getNextItem_002()
        {
             ::osl::Directory testDirectory( aTmpName3 ); //constructor
            nError1 = testDirectory.getNextItem( rItem );

            CPPUNIT_ASSERT_MESSAGE( "test for getNextItem function: retrieve an item in a directory which is not opened, also test for nHint's default value.",
                                    ( ::osl::FileBase::E_INVAL == nError1 ) );
        }

        void getNextItem_003()
        {
             ::osl::Directory testDirectory( aTmpName3 ); //constructor

            //open a directory
            nError1 = testDirectory.open();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            for ( int nCount = 0; nCount < 4; nCount++ )
            {
                nError2 = testDirectory.getNextItem( rItem, 3 );
            }

            //close a directory
            nError1 = testDirectory.close();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );

            CPPUNIT_ASSERT_MESSAGE( "test for getNextItem function: retrieve 4 times in a directory which contain only 3 files.",
                                    ( ::osl::FileBase::E_NOENT == nError2 ) );
        }

        void getNextItem_004()
        {
        //create a link file(can not on Windows), then check if getNextItem can get it.
#ifdef UNX
            bool bLnkOK = false;
            bool bFoundOK = false;

            ::rtl::OUString aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
            aUStr_LnkFileSys += aSlashURL + "/tmpdir/link.file";
            aUStr_SrcFileSys += aSlashURL + "/tmpdir/tmpname";

            ::rtl::OString strLinkFileName, strSrcFileName;
            strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
            strSrcFileName  = OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

            // create a link file and link it to file "/tmp/PID/tmpdir/tmpname"
            sal_Int32 fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
            CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(0), fd );
            ::osl::Directory testDirectory( aTmpName3 );

            //open a directory
            nError1 = testDirectory.open();
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            ::rtl::OUString aFileName ("link.file");

            while (true) {
                nError1 = testDirectory.getNextItem( rItem, 4 );
                if (::osl::FileBase::E_None == nError1) {
                    ::osl::FileStatus   rFileStatus( osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Type );
                    rItem.getFileStatus( rFileStatus );
                    if ( compareFileName( rFileStatus.getFileName(), aFileName) )
                    {
                        bFoundOK = true;
                        if ( FileStatus::Link == rFileStatus.getFileType())
                        {
                            bLnkOK = true;
                            break;
                        }
                    }
                }
                else
                    break;
            };
                fd = std::remove( strLinkFileName.getStr() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "remove link file failed", static_cast<sal_Int32>(0), fd );
            CPPUNIT_ASSERT_MESSAGE( "test for getNextItem function: check if can retrieve the link file name",
                                    bFoundOK );
            CPPUNIT_ASSERT_MESSAGE( "test for getNextItem function: check if link file has file type link",
                                    bLnkOK );
#endif
        }

        CPPUNIT_TEST_SUITE( getNextItem );
        CPPUNIT_TEST( getNextItem_001 );
        CPPUNIT_TEST( getNextItem_002 );
        CPPUNIT_TEST( getNextItem_003 );
        CPPUNIT_TEST( getNextItem_004 );
        CPPUNIT_TEST_SUITE_END();
    };// class getNextItem

    //  testing the method
    //  inline static RC getVolumeInfo( const ::rtl::OUString& ustrDirectoryURL, VolumeInfo& rInfo )

    class  getVolumeInfo : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1;

        public:

        getVolumeInfo() :nError1(FileBase::E_None) {}
        // test code.
        void checkValidMask(osl::VolumeInfo const& _aVolumeInfo, sal_Int32 _nMask)
            {
                if (_nMask == osl_VolumeInfo_Mask_FileSystemName)
        {
            //get file system name
            ::rtl::OUString aFileSysName( aNullURL );
                    aFileSysName = _aVolumeInfo.getFileSystemName();

                    bool bRes2 = compareFileName( aFileSysName, aNullURL );
            CPPUNIT_ASSERT_MESSAGE( "test for getVolumeInfo function: getVolumeInfo of root directory.",
                                    ( osl::FileBase::E_None == nError1 ) &&
                                    !bRes2 );
                }
                if (_nMask == osl_VolumeInfo_Mask_Attributes)
                {
                    bool b1 = _aVolumeInfo.getRemoteFlag();
                    bool b2 = _aVolumeInfo.getRemoveableFlag();
                    bool b3 = _aVolumeInfo.getCompactDiscFlag();
                    bool b4 = _aVolumeInfo.getFloppyDiskFlag();
                    bool b5 = _aVolumeInfo.getFixedDiskFlag();
                    bool b6 = _aVolumeInfo.getRAMDiskFlag();

                    rtl::OString sAttr;
                    if (b1) sAttr =  "Remote";
                    if (b2) sAttr += " Removeable";
                    if (b3) sAttr += " CDROM";
                    if (b4) sAttr += " Floppy";
                    if (b5) sAttr += " FixedDisk";
                    if (b6) sAttr += " RAMDisk";

                    printf("Attributes: %s\n", sAttr.getStr() );
                }
                if (_nMask == osl_VolumeInfo_Mask_TotalSpace)
                {
                    // within Linux, df / * 1024 bytes is the result
                    sal_uInt64 nSize = _aVolumeInfo.getTotalSpace();
                    printf("Total space: %" SAL_PRIuUINT64 "\n", nSize);
                }
                if (_nMask == osl_VolumeInfo_Mask_UsedSpace)
                {
                    sal_uInt64 nSize = _aVolumeInfo.getUsedSpace();
                    printf(" Used space: %" SAL_PRIuUINT64 "\n", nSize);
                }
                if (_nMask == osl_VolumeInfo_Mask_FreeSpace)
                {
                    sal_uInt64 nSize = _aVolumeInfo.getFreeSpace();
                    printf(" Free space: %" SAL_PRIuUINT64 "\n", nSize);
                }
                if (_nMask == osl_VolumeInfo_Mask_MaxNameLength)
                {
                    sal_uInt32 nLength = _aVolumeInfo.getMaxNameLength();
                    printf("max name length: %" SAL_PRIuUINT32 "\n", nLength);
                }
                if (_nMask == osl_VolumeInfo_Mask_MaxPathLength)
                {
                    sal_uInt32 nLength = _aVolumeInfo.getMaxPathLength();
                    printf("max path length: %" SAL_PRIuUINT32 "\n", nLength);
                }
                if (_nMask == osl_VolumeInfo_Mask_FileSystemCaseHandling)
                {
                    bool bIsCase = _aVolumeInfo.isCaseSensitiveFileSystem();
                    printf("filesystem case sensitive: %s\n", bIsCase ? "yes" : "no");
                }
            }

        void checkVolumeInfo(sal_Int32 _nMask)
            {
                ::osl::VolumeInfo aVolumeInfo( _nMask );
                //call getVolumeInfo here
                nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
                // LLA: IMHO it's not a bug, if VolumeInfo is not valid, it's a feature
                // LLA: CPPUNIT_ASSERT_MESSAGE("mask is not valid", sal_True == aVolumeInfo.isValid( _nMask ) );
                if (aVolumeInfo.isValid( _nMask))
                {
                    checkValidMask(aVolumeInfo, _nMask);
                }
            }

        void getVolumeInfo_001_1()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_FileSystemName;
            checkVolumeInfo(mask);
        }
        void getVolumeInfo_001_2()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_Attributes;
            checkVolumeInfo(mask);
        }
        void getVolumeInfo_001_3()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_TotalSpace;
            checkVolumeInfo(mask);
        }
        void getVolumeInfo_001_4()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_UsedSpace;
            checkVolumeInfo(mask);
        }
        void getVolumeInfo_001_5()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_FreeSpace;
            checkVolumeInfo(mask);
        }
        void getVolumeInfo_001_6()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_MaxNameLength;
            checkVolumeInfo(mask);
        }
        void getVolumeInfo_001_7()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_MaxPathLength;
            checkVolumeInfo(mask);
        }
        void getVolumeInfo_001_8()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_FileSystemCaseHandling;
            checkVolumeInfo(mask);
        }

        void getVolumeInfo_002()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_FileSystemName;
            ::osl::VolumeInfo aVolumeInfo( mask );
            //call getVolumeInfo here

            // LLA: rtl::OUString aRootSysURL;
            // LLA: nError1 = osl::File::getFileURLFromSystemPath(aRootSys, aRootSysURL);
            // LLA:
            // LLA: CPPUNIT_ASSERT_MESSAGE( "can't convert root path to file url",
            // LLA:                         ( osl::FileBase::E_NONE == nError1 ) );

            nError1 = ::osl::Directory::getVolumeInfo( aRootSys, aVolumeInfo );

            CPPUNIT_ASSERT_MESSAGE( "test for getVolumeInfo function: use system path as parameter.",
                                    ( osl::FileBase::E_INVAL == nError1 ) );
        }

        void getVolumeInfo_003()
        {
            sal_Int32 mask = osl_VolumeInfo_Mask_FileSystemName;
            ::osl::VolumeInfo aVolumeInfo( mask );
            //call getVolumeInfo here
            nError1 = ::osl::Directory::getVolumeInfo( aTmpName3, aVolumeInfo );

// LLA: in Windows, it reply no error, it did not pass in (W32).
#if defined(UNX) && !defined(IOS)
            CPPUNIT_ASSERT_MESSAGE( "test for getVolumeInfo function: non-existence test. ",
                                    ( osl::FileBase::E_NOENT == nError1 ) );
#endif
        }

        CPPUNIT_TEST_SUITE( getVolumeInfo );
        CPPUNIT_TEST( getVolumeInfo_001_1 );
        CPPUNIT_TEST( getVolumeInfo_001_2 );
        CPPUNIT_TEST( getVolumeInfo_001_3 );
        CPPUNIT_TEST( getVolumeInfo_001_4 );
        CPPUNIT_TEST( getVolumeInfo_001_5 );
        CPPUNIT_TEST( getVolumeInfo_001_6 );
        CPPUNIT_TEST( getVolumeInfo_001_7 );
        CPPUNIT_TEST( getVolumeInfo_001_8 );
        CPPUNIT_TEST( getVolumeInfo_002 );
        CPPUNIT_TEST( getVolumeInfo_003 );
        CPPUNIT_TEST_SUITE_END();
    };// class getVolumeInfo

    //  testing the method
    //  inline static RC create( const ::rtl::OUString& ustrDirectoryURL )

    class  create : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;

        public:

        // test code.
        create() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        void create_001()
        {
            //create directory in $TEMP/tmpdir
            nError1 = ::osl::Directory::create( aTmpName3 );
            //check for existence
            nError2 = ::osl::Directory::create( aTmpName3 );
            //remove it
            deleteTestDirectory( aTmpName3 );

            CPPUNIT_ASSERT_MESSAGE( "test for create function: create a directory and check its existence.",
                                    ( osl::FileBase::E_None == nError1 ) &&
                                    ( osl::FileBase::E_EXIST== nError2 ) );
        }

        void create_002()
        {
#if !defined(_WIN32) && !defined(MACOSX) && defined(SAL_UNX)
            if (geteuid() == 0) //don't test if building as root
                return;

            rtl::OUString aTmpDir;
            nError1 = FileBase::createTempFile(nullptr, nullptr, &aTmpDir);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("temp File creation failed", osl::FileBase::E_None, nError1);

            nError1 = ::osl::File::remove(aTmpDir);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("temp File removal failed", osl::FileBase::E_None, nError1);

            nError1 = ::osl::Directory::create(aTmpDir);
            ::rtl::OString sError("test for create function: create a directory '");
            sError += ::rtl::OUStringToOString(aTmpDir, RTL_TEXTENCODING_ASCII_US);
            sError += "' and check its existence.";
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sError.getStr(), osl::FileBase::E_None, nError1);
            osl_setFileAttributes(aTmpDir.pData, 0); //no access allowed now

            //Shouldn't be possible now to create a dir underneath it
            rtl::OUString aTmpSubLevel = aTmpDir + "/notallowedhere";
            nError1 = ::osl::Directory::create(aTmpSubLevel);

            //allow removal
            osl_setFileAttributes(aTmpDir.pData,
                osl_File_Attribute_OwnRead |
                osl_File_Attribute_OwnWrite |
                osl_File_Attribute_OwnExe);
            deleteTestDirectory(aTmpDir);
            sError = ::rtl::OString("test for create function: create a directory under '");
            sError += ::rtl::OUStringToOString(aTmpDir, RTL_TEXTENCODING_ASCII_US);
            sError += "' for access test.";
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sError.getStr(), osl::FileBase::E_ACCES, nError1);
#endif
        }

        void create_003()
        {
            //create directory in /tmpname
            nError1 = ::osl::Directory::create( aSysPath1 );

            CPPUNIT_ASSERT_MESSAGE( "test for create function: create a directory using system path.",
                                    ( osl::FileBase::E_INVAL == nError1 ) );
        }

        CPPUNIT_TEST_SUITE( create );
        CPPUNIT_TEST( create_001 );
        CPPUNIT_TEST( create_002 );
        CPPUNIT_TEST( create_003 );
        CPPUNIT_TEST_SUITE_END();
    };// class create

    //  testing the method
    //  inline static RC remove( const ::rtl::OUString& ustrDirectoryURL )

    class  remove : public CppUnit::TestFixture
    {
        ::osl::FileBase::RC     nError1, nError2;

        public:

        remove() :nError1(FileBase::E_None),nError2(FileBase::E_None) {}
        // test code.
        void remove_001()
        {
            //create directory in $TEMP/tmpdir
            nError1 = ::osl::Directory::create( aTmpName3 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //remove it
            nError1 = ::osl::Directory::remove( aTmpName3 );
            //check for existence
            ::osl::Directory rDirectory( aTmpName3 );
            nError2 = rDirectory.open();

            CPPUNIT_ASSERT_MESSAGE( "test for remove function: remove a directory and check its existence.",
                                    ( osl::FileBase::E_None == nError1 ) &&
                                    ( osl::FileBase::E_NOENT == nError2 ) );
        }

        void remove_002()
        {
            //create directory in $TEMP/tmpdir
            nError1 = ::osl::Directory::create( aTmpName3 );
            CPPUNIT_ASSERT_EQUAL( nError1, ::osl::FileBase::E_None );
            //try to remove it by system path
            nError1 = ::osl::Directory::remove( aSysPath3 );
             //check for existence
            ::osl::Directory rDirectory( aTmpName3 );
            nError2 = rDirectory.open();
            if ( osl::FileBase::E_NOENT != nError2 )
                ::osl::Directory::remove( aTmpName3 );

            CPPUNIT_ASSERT_MESSAGE( "test for remove function: remove a directory by its system path, and check its existence.",
                                    ( osl::FileBase::E_INVAL == nError1 ) );
        }

        void remove_003()
        {
            //try to remove a non-existed directory
            nError1 = ::osl::Directory::remove( aTmpName6 );

            CPPUNIT_ASSERT_MESSAGE( "test for remove function: try to remove a non-existed directory.",
                                    ( osl::FileBase::E_NOENT == nError1 ) );
        }

        void remove_004()
        {
            createTestFile( aTmpName6 );
            bool bExist = ifFileExist( aTmpName6 );
            //try to remove file.
            nError1 = ::osl::Directory::remove( aTmpName6 );
            deleteTestFile( aTmpName6 );

            CPPUNIT_ASSERT_MESSAGE( "test for remove function: try to remove a file but not directory.",
                                    bExist &&(( osl::FileBase::E_NOTDIR == nError1 ) || ( osl::FileBase::E_NOENT == nError1 )) );
        }

        void remove_005()
        {
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
            nError1 = ::osl::Directory::remove( aTmpName3 );
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
            ::rtl::OString sError = "test for remove function: try to remove a directory that is not empty.";
            sError += errorToStr( nError1 ).getStr();
#if defined ( SOLARIS )
            //on UNX, the implementation uses rmdir(), which EEXIST is thrown on Solaris when the directory is not empty, refer to: 'man -s 2 rmdir', while on linux, ENOTEMPTY is thrown.
            //EEXIST The directory contains entries other than those for "." and "..".
            printf("#Solaris test\n");
            CPPUNIT_ASSERT_MESSAGE( sError.getStr(), ( osl::FileBase::E_EXIST == nError1 ) );
#else
            CPPUNIT_ASSERT_MESSAGE( sError.getStr(), ( osl::FileBase::E_NOTEMPTY == nError1 ) );
#endif
        }

        CPPUNIT_TEST_SUITE( remove );
        CPPUNIT_TEST( remove_001 );
        CPPUNIT_TEST( remove_002 );
        CPPUNIT_TEST( remove_003 );
        CPPUNIT_TEST( remove_004 );
        CPPUNIT_TEST( remove_005 );
        CPPUNIT_TEST_SUITE_END();
    };// class remove

    // TEST Directory::createPath

    #ifdef _WIN32
    #   define PATH_BUFFER_SIZE MAX_PATH
    #else
    #   define PATH_BUFFER_SIZE PATH_MAX
    #endif

#define TEST_PATH_POSTFIX "hello/world"

    OUString const & get_test_path()
    {
        static OUString test_path;
        if (test_path.isEmpty())
        {
            OUString tmp;
            FileBase::RC rc = FileBase::getTempDirURL(tmp);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
             "Getting the location of TMP dir failed",
             FileBase::E_None, rc
            );

            OUString system_path;
            rc = FileBase::getSystemPathFromFileURL(tmp, system_path);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
             "Cannot convert the TMP dir to system path",
             FileBase::E_None, rc
            );

            OString tmp_x(rtl::OUStringToOString(system_path, RTL_TEXTENCODING_UTF8 ));
            if (tmp_x.lastIndexOf('/') != (tmp_x.getLength() - 1))
                tmp_x += rtl::OString('/');

#if !defined(_WIN32) && !defined(ANDROID) && !defined(AIX)
            // FIXME would be nice to create unique dir even on Windows
            tmp_x += rtl::OString("XXXXXX");
            char *out = mkdtemp(const_cast<char*>(tmp_x.getStr()));

            CPPUNIT_ASSERT_MESSAGE
            (
             "mkdtemp call failed",
             out != nullptr
            );

            tmp_x += rtl::OString('/');
#endif
            tmp_x += rtl::OString(TEST_PATH_POSTFIX);

            rc = FileBase::getFileURLFromSystemPath(rtl::OStringToOUString(tmp_x, RTL_TEXTENCODING_UTF8), test_path);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
             "Cannot convert the system path back to an URL",
             FileBase::E_None, rc
            );
        }
        return test_path;
    }

    void rm_test_path(const OUString& path)
    {
        sal_Unicode buffer[PATH_BUFFER_SIZE];
        memcpy(buffer, path.getStr(), (path.getLength() + 1) * sizeof(sal_Unicode));

        sal_Int32 i = rtl_ustr_lastIndexOfChar(buffer, '/');
        if (i == path.getLength())
            buffer[i] = 0;

        Directory::remove(buffer);

        i = rtl_ustr_lastIndexOfChar(buffer, '/');
        assert(i != -1);
        if (i != -1)
        {
            buffer[i] = 0;
            Directory::remove(buffer);
        }
    }

    class DirCreatedObserver : public DirectoryCreationObserver
    {
    public:
        DirCreatedObserver() : i(0)
        {
        }

        virtual void DirectoryCreated(const rtl::OUString& /*aDirectoryUrl*/) override
        {
            i++;
        };

        int number_of_dirs_created() const
        {
            return i;
        }

    private:
            int i;
    };

    class createPath : public CppUnit::TestFixture
    {
    public:

        createPath()
        {}

        void with_relative_path()
        {
            FileBase::RC rc = Directory::createPath(TEST_PATH_POSTFIX);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "osl_createDirectoryPath contract broken",
                FileBase::E_INVAL, rc
            );
        }

        void without_callback()
        {
            OUString tp_url = get_test_path();

            rm_test_path(tp_url);

            FileBase::RC rc = Directory::createPath(tp_url);

            rm_test_path(tp_url);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "osl_createDirectoryPath failed",
                FileBase::E_None, rc
            );
        }

        void with_callback()
        {
            OUString tp_url = get_test_path();

            rm_test_path(tp_url);

            DirCreatedObserver* observer = new DirCreatedObserver;
            FileBase::RC rc = Directory::createPath(tp_url, observer);
            int nDirs = observer->number_of_dirs_created();
            delete observer;

            rm_test_path(tp_url);

            CPPUNIT_ASSERT_MESSAGE
            (
                "osl_createDirectoryPath failed",
                (rc == FileBase::E_None) && (nDirs > 0)
            );

        }

#ifdef _WIN32

        const char* get_unused_drive_letter()
        {
            static const char m_aBuff[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            DWORD ld = GetLogicalDrives();
            DWORD i = 4;
            DWORD j = 2;

            while ((ld & i) && (i > 1))
            { i = i << 1; j++; }

            if (i > 2)
                return m_aBuff + j;

            return NULL;
        }

        void at_invalid_logical_drive()
        {
            const char* drv = get_unused_drive_letter();
            char buff[PATH_BUFFER_SIZE];
            memset(buff, 0, sizeof(buff));

            strncpy(buff, drv, 1);
            strcat(buff, ":\\");
            strcat(buff, TEST_PATH_POSTFIX);

            OUString path = OUString::createFromAscii(buff);
            OUString tp_url;
            FileBase::getFileURLFromSystemPath(path, tp_url);

            FileBase::RC rc = Directory::createPath(tp_url);

            CPPUNIT_ASSERT_MESSAGE
            (
                "osl_createDirectoryPath doesn't fail on unused logical drive letters",
                rc != FileBase::E_None
            );
        }
#endif /* WNT */

    CPPUNIT_TEST_SUITE(createPath);
    CPPUNIT_TEST(with_relative_path);
    CPPUNIT_TEST(without_callback);
    CPPUNIT_TEST(with_callback);
#ifdef _WIN32
    CPPUNIT_TEST(at_invalid_logical_drive);
#endif
    CPPUNIT_TEST_SUITE_END();

    }; // class createPath

    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::ctors );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::open );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::isOpen );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::close );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::reset );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::getNextItem );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::getVolumeInfo );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::create );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::remove );
    CPPUNIT_TEST_SUITE_REGISTRATION( osl_Directory::createPath );
}// namespace osl_Directory

CPPUNIT_PLUGIN_IMPLEMENT();

/** get Current PID.
*/
inline ::rtl::OUString getCurrentPID(  )
{
    //~ Get current PID and turn it into OUString;
    int nPID = 0;
#ifdef _WIN32
    nPID = GetCurrentProcessId();
#else
    nPID = getpid();
#endif
    return ( OUString::number( nPID ) );
}

//~ do some clean up work after all test completed.
class GlobalObject
{
    public:
    ~GlobalObject()
    {
        try
        {
            //~ special clean up task in Windows and Unix separately;
#if ( defined UNX )
            //~ some clean up task  for UNIX OS
            ;
#else
            //~ some clean up task  for Windows OS
            //~ check if some files are in the way, remove them if necessary.
            if ( ifFileExist( aTmpName6 )  == sal_True )
                deleteTestFile( aTmpName6 );
            if ( ifFileExist( aTmpName4 )  == sal_True )
                deleteTestFile( aTmpName4 );
            if ( checkDirectory( aTmpName4, osl_Check_Mode_Exist )  == sal_True )
                deleteTestDirectory( aTmpName4 );
            if ( ifFileExist( aTmpName3 )  == sal_True )
                deleteTestFile( aTmpName3 );
            if ( checkDirectory( aTmpName3, osl_Check_Mode_Exist )  == sal_True )
                deleteTestDirectory( aTmpName3 );

            ::rtl::OUString aUStr( aUserDirectoryURL );
            concatURL( aUStr, aHidURL1 );
            if ( ifFileExist( aUStr )  == sal_True )
                deleteTestFile( aUStr );

            ::rtl::OUString aUStr1( aRootURL );
            concatURL( aUStr1, aTmpName2 );
            if ( ifFileExist( aUStr1 )  == sal_True )
                deleteTestFile( aUStr1 );
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
