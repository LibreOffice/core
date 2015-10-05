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

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include "osl/thread.h"

#include "rtl/ustrbuf.hxx"
#include <osl/file.hxx>
#include <osl_File_Const.h>

#include "gtest/gtest.h"

// #ifdef WNT
// #   define UNICODE
// #    define WIN32_LEAN_AND_MEAN
// #    include <windows.h>
// #   include <tchar.h>
// #endif


using namespace osl;
using namespace rtl;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

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

rtl::OUString errorToStr( ::osl::FileBase::RC const& nError)
{
    rtl::OUStringBuffer suBuf;
    suBuf.append( rtl::OUString::createFromAscii("The returned error is: ") );
    suBuf.append( rtl::OStringToOUString(errorToString(nError), RTL_TEXTENCODING_ASCII_US) );
    suBuf.append( rtl::OUString::createFromAscii("!\n") );
    return suBuf.makeStringAndClear();
}

/** print a file type name.
*/
inline void printFileType( const ::osl::FileStatus::Type nType )
{
    printf( "#printFileType# " );
    switch ( nType ) {
        case ::osl::FileStatus::Directory:
            printf( "This file is a: Directory.\n" );
            break;
        case ::osl::FileStatus::Volume:
            printf( "This file is a: volume device.\n" );
            break;
        case ::osl::FileStatus::Regular:
            printf( "This file is a: regular file.\n" );
            break;
        case ::osl::FileStatus::Fifo:
            printf( "This file is a: fifo.\n" );
            break;
        case ::osl::FileStatus::Socket:
            printf( "This file is a: socket.\n" );
            break;
        case ::osl::FileStatus::Link:
            printf( "This file is a: link file.\n" );
            break;
        case ::osl::FileStatus::Special:
            printf( "This file is a: special.\n" );
            break;
        case ::osl::FileStatus::Unknown:
            printf( "The file type is unknown %d \n", nType );
            break;
    }
}

/** print a file attributes.
*/
inline void printFileAttributes( const sal_Int64 nAttributes )
{
    printf( "#printFileAttributes# This file is a: (" );
    if ( ( nAttributes | Attribute_ReadOnly ) == nAttributes )
            printf( " ReadOnly " );
    if ( ( nAttributes | Attribute_Hidden ) == nAttributes )
            printf( " Hidden " );
    if ( ( nAttributes | Attribute_Executable ) == nAttributes )
            printf( " Executable " );
    if ( ( nAttributes | Attribute_GrpWrite ) == nAttributes )
            printf( " GrpWrite " );
    if ( ( nAttributes | Attribute_GrpRead ) == nAttributes )
            printf( " GrpRead " );
    if ( ( nAttributes | Attribute_GrpExe ) == nAttributes )
            printf( " GrpExe " );
    if ( ( nAttributes | Attribute_OwnWrite ) == nAttributes )
            printf( " OwnWrite " );
    if ( ( nAttributes | Attribute_OwnRead ) == nAttributes )
            printf( " OwnRead " );
    if ( ( nAttributes | Attribute_OwnExe ) == nAttributes )
            printf( " OwnExe " );
    if ( ( nAttributes | Attribute_OthWrite ) == nAttributes )
            printf( " OthWrite " );
    if ( ( nAttributes | Attribute_OthRead ) == nAttributes )
            printf( " OthRead " );
    if ( ( nAttributes | Attribute_OthExe ) == nAttributes )
            printf( " OthExe " );
    printf( ") file!\n" );
}

/** print a UNI_CODE file name.
*/
inline void printFileName( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf( "#printFileName_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf( "%s\n", aString.getStr( ) );
}

/** print a ASCII_CODE file name.
*/
inline void printFileName( const sal_Char * str )
{
    printf( "#printFileName_a# " );
    printf( "%s\n", str );
}

/** print an output wrong message.
*/
inline void printError( const ::osl::FileBase::RC nError )
{
    printf( "#printError# " );
    printFileName( errorToStr(nError) );
}

/** print an signed Integer Number.
*/
inline void printInt( sal_Int64 i )
{
    printf( "#printInt_i64# " );
    printf( "The Integer64 is %lld\n", i);
}

/** print an unsigned Integer Number.
*/
inline void printInt( sal_uInt64 i )
{
    printf( "#printInt_u64# " );
    printf( "The unsigned Integer64 is %llu\n", i);
}

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf( "#printBool# " );
    ( sal_True == bOk ) ? printf( "YES!\n" ): printf( "NO!\n" );
}

/** print struct TimeValue in local time format.
*/
inline void printTime( TimeValue *tv )
{
    oslDateTime *pDateTime = ( oslDateTime* )malloc( sizeof( oslDateTime ) ) ;
     ASSERT_TRUE(pDateTime != NULL) << "Error in printTime() function,malloc ";
    TimeValue *pLocalTV = ( TimeValue* )malloc( sizeof( TimeValue ) );
     ASSERT_TRUE(pLocalTV != NULL) << "Error in printTime() function,malloc ";

    ASSERT_TRUE(sal_True == osl_getLocalTimeFromSystemTime( tv, pLocalTV )) << "Error in printTime() function,osl_getLocalTimeFromSystemTime ";
    ASSERT_TRUE(sal_True == osl_getDateTimeFromTimeValue( pLocalTV, pDateTime )) << "Error in printTime() function,osl_gepDateTimeFromTimeValue ";

    printf( "#printTime# " );
     printf( " Time is: %d/%d/%d ", pDateTime->Month, pDateTime->Day, pDateTime->Year);
    switch ( pDateTime->DayOfWeek )
    {
        case 0: printf("Sun. "); break;
        case 1: printf("Mon. "); break;
        case 2: printf("Tue. "); break;
        case 3: printf("Thr. "); break;
        case 4: printf("Wen. "); break;
        case 5: printf("Fri. "); break;
        case 6: printf("Sat. "); break;
    }
    printf( " %d:%d:%d %d nsecs\n", pDateTime->Hours, pDateTime->Minutes, pDateTime->Seconds, pDateTime->NanoSeconds);

    free( pDateTime );
    free( pLocalTV );
}

/** compare two TimeValue, unit is "ms", since Windows time precision is better than UNX.
*/

#if ( defined UNX ) || ( defined OS2 )                 //precision of time in Windows is better than UNX
#   define delta 2000                    //time precision, 2000ms
#else
#   define delta 1800                    //time precision, 1.8s
#endif

inline sal_Int64 t_abs64(sal_Int64 _nValue)
{
    // std::abs() seems to have some ambiguity problems (so-texas)
    // return abs(_nValue);
    printf("t_abs64(%ld)\n", _nValue);
    // ASSERT_TRUE(_nValue < 2147483647);

    if (_nValue < 0)
    {
        _nValue = -_nValue;
    }
    return _nValue;
}

inline sal_Bool t_compareTime( TimeValue *m_aEndTime,  TimeValue *m_aStartTime, sal_Int32 nDelta)
{
    // sal_uInt64 uTimeValue;
    // sal_Int64 iTimeValue;
    //
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
inline sal_Bool compareFileName( const ::rtl::OUString & ustr1, const ::rtl::OUString & ustr2 )
{
    sal_Bool bOk;
//on Windows, the seperatar is '\', so here change to '/', then compare
#if defined (WNT )
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

/** compare a OUString and an ASCII file name.
*/
inline sal_Bool compareFileName( const ::rtl::OUString & ustr, const sal_Char *astr )
{
    (void)ustr;
    ::rtl::OUString ustr1 = rtl::OUString::createFromAscii( astr );
    sal_Bool bOk = ustr1.equalsIgnoreAsciiCase( ustr1 );

    return bOk;
}

/** simple version to judge if a file name or directory name is a URL or a system path, just to see if it
    is start with "file:///";.
*/
inline sal_Bool isURL( const sal_Char *pathname )
{
    return ( 0 == strncmp( pathname, FILE_PREFIX, sizeof( FILE_PREFIX ) - 1 ) );
}

/** simple version to judge if a file name or directory name is a URL or a system path, just to see if it
    is start with "file:///";.
*/
inline sal_Bool isURL( const ::rtl::OUString pathname )
{
    return ( ( pathname.indexOf( aPreURL ) == 0 ) ? sal_True : sal_False );
}

/** concat two part to form a URL or system path, add PATH_SEPERATOR between them if necessary, add "file:///" to beginning if necessary.
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

    sal_Int32 index = 0;
    //check if '/' is in the end of pathname1 or at the begin of pathname2;
    if ( ( ( index = pathname1.lastIndexOf( aSlashURL ) ) != ( pathname1.getLength( ) - 1 ) ) &&
         ( ( index = pathname2.indexOf( aSlashURL ) ) != 0 ) )
        pathname1 += aSlashURL;
    pathname1 += pathname2;
}

/** create a temp test file using OUString name of full qualified URL or system path.
*/
inline void createTestFile( const ::rtl::OUString filename )
{
    ::rtl::OUString     aPathURL   = filename.copy( 0 );
    ::osl::FileBase::RC nError;

    if ( !isURL( filename ) )
        ::osl::FileBase::getFileURLFromSystemPath( filename, aPathURL ); //convert if not full qualified URL

    //::std::auto_ptr<File> pFile( new File( aPathURL ) );
    File aFile(aPathURL);
    //nError = pFile->open( OpenFlag_Read | OpenFlag_Write | OpenFlag_Create );
    nError = aFile.open( OpenFlag_Read | OpenFlag_Write | OpenFlag_Create );
    //ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_EXIST )) << "In createTestFile Function: creation ";
    if ( ( ::osl::FileBase::E_None != nError ) && ( nError != ::osl::FileBase::E_EXIST ))
    {
        printf("createTestFile failed!\n");
    }
    aFile.close();

}

/** create a temp test file using OUString name of full qualified URL or system path in a base directory.
*/
inline void createTestFile( const ::rtl::OUString basename, const ::rtl::OUString filename )
{
    ::rtl::OUString aBaseURL = basename.copy( 0 );

    concatURL( aBaseURL, filename );
    createTestFile( aBaseURL );
}

/** detete a temp test file using OUString name.
*/
inline void deleteTestFile( const ::rtl::OUString filename )
{
    // LLA: printf("deleteTestFile\n");
    ::rtl::OUString     aPathURL   = filename.copy( 0 );
    ::osl::FileBase::RC nError;

    if ( !isURL( filename ) )
        ::osl::FileBase::getFileURLFromSystemPath( filename, aPathURL ); //convert if not full qualified URL

    nError = ::osl::File::setAttributes( aPathURL, Attribute_GrpWrite| Attribute_OwnWrite| Attribute_OthWrite ); // if readonly, make writtenable.
    ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) || ( ::osl::FileBase::E_NOENT == nError )) << "In deleteTestFile Function: set writtenable ";

    nError = ::osl::File::remove( aPathURL );
    ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT )) << "In deleteTestFile Function: remove ";
}

/** delete a temp test file using OUString name of full qualified URL or system path in a base directory.
*/
inline void deleteTestFile( const ::rtl::OUString basename, const ::rtl::OUString filename )
{
    ::rtl::OUString     aBaseURL   = basename.copy( 0 );

    concatURL( aBaseURL, filename );
    deleteTestFile( aBaseURL );
}

/** create a temp test directory using OUString name of full qualified URL or system path.
*/
inline void createTestDirectory( const ::rtl::OUString dirname )
{
    ::rtl::OUString     aPathURL   = dirname.copy( 0 );
    ::osl::FileBase::RC nError;

    if ( !isURL( dirname ) )
        ::osl::FileBase::getFileURLFromSystemPath( dirname, aPathURL ); //convert if not full qualified URL
    nError = ::osl::Directory::create( aPathURL );
    //ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_EXIST )) << "In createTestDirectory Function: creation: ";
    if ( ( ::osl::FileBase::E_None != nError ) && ( nError != ::osl::FileBase::E_EXIST ))
      printf("createTestDirectory failed!\n");
}

/** create a temp test directory using OUString name of full qualified URL or system path in a base directory.
*/
inline void createTestDirectory( const ::rtl::OUString basename, const ::rtl::OUString dirname )
{
    ::rtl::OUString     aBaseURL   = basename.copy( 0 );
    ::rtl::OString      aString;

    concatURL( aBaseURL, dirname );
    createTestDirectory( aBaseURL );
}

/** delete a temp test directory using OUString name of full qualified URL or system path.
*/
inline void deleteTestDirectory( const ::rtl::OUString dirname )
{
    // LLA: printf("deleteTestDirectory\n");
    ::rtl::OUString     aPathURL   = dirname.copy( 0 );
    ::osl::FileBase::RC nError;
    // LLA: printFileName(aPathURL);
    if ( !isURL( dirname ) )
        ::osl::FileBase::getFileURLFromSystemPath( dirname, aPathURL ); //convert if not full qualified URL

    ::osl::Directory testDir( aPathURL );
    if ( testDir.isOpen( ) == sal_True )
    {
        // LLA: printf("#close Dir\n");
        testDir.close( );  //close if still open.
        }

    nError = ::osl::Directory::remove( aPathURL );
    // LLA: printError(nError);
    // LLA: if (( ::osl::FileBase::E_None == nError ))
    // LLA: {
    // LLA:     printf("nError == E_None\n");
    // LLA: }
    // LLA: else if ( ( nError == ::osl::FileBase::E_NOENT ))
    // LLA: {
    // LLA:     printf("nError == E_NOENT\n");
    // LLA: }
    // LLA: else
    // LLA: {
    // LLA:     // printf("nError == %d\n", nError);
    // LLA: }
        rtl::OUString strError = rtl::OUString::createFromAscii( "In deleteTestDirectory function: remove Directory ");
        strError += aPathURL;
    EXPECT_TRUE(( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT )) << strError.pData;
    // LLA: if (! ( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT ))
    // LLA: {
    // LLA:     printf("In deleteTestDirectory function: remove\n");
    // LLA: }
}

/** delete a temp test directory using OUString name of full qualified URL or system path in a base directory.
*/
inline void deleteTestDirectory( const ::rtl::OUString basename, const ::rtl::OUString dirname )
{
    ::rtl::OUString     aBaseURL   = basename.copy( 0 );

    concatURL( aBaseURL, dirname );
    deleteTestDirectory( aBaseURL );
}


/** Check for the file and directory access right.
*/
typedef enum {
    osl_Check_Mode_Exist,
    osl_Check_Mode_OpenAccess,
    osl_Check_Mode_ReadAccess,
    osl_Check_Mode_WriteAccess
} oslCheckMode;

// not used here
inline sal_Bool checkFile( const ::rtl::OUString & str, oslCheckMode nCheckMode )
{
    ::osl::FileBase::RC   nError1, nError2;
    ::osl::File       testFile( str );
    sal_Bool        bCheckResult;

    bCheckResult = sal_False;
    nError1 = testFile.open ( OpenFlag_Read );
    if ( ( ::osl::FileBase::E_NOENT != nError1 ) && ( ::osl::FileBase::E_ACCES != nError1 ) ){

        switch ( nCheckMode ) {
            case osl_Check_Mode_Exist:
                /// check if the file is exist.
                if ( ::osl::FileBase::E_None == nError1 )
                    bCheckResult = sal_True;
                break;
            case osl_Check_Mode_OpenAccess:
                /// check if the file is openable.
                if ( ::osl::FileBase::E_None == nError1 )
                    bCheckResult = sal_True;
                break;
            case osl_Check_Mode_WriteAccess:
                /// check the file name and whether it can be write.
                /// write chars into the file.
                //testFile.close( );
                //testFile.open( OpenFlag_Write );
                sal_uInt64 nCount_write;
                nError2 = testFile.write( pBuffer_Char, 10, nCount_write );
                if ( ::osl::FileBase::E_None == nError2 )
                    bCheckResult = sal_True;
                break;

            default:
                bCheckResult = sal_False;
        }/// swith

        nError2 = testFile.close( );
        EXPECT_TRUE(nError2 == FileBase::E_None) << " in CheckFile() function, close file ";

    }

    return bCheckResult;
}

//check if the file exist
inline sal_Bool ifFileExist( const ::rtl::OUString & str )
{
    sal_Bool        bCheckResult = sal_False;

/*#ifdef WNT
    ::rtl::OUString     aUStr  = str.copy( 0 );
    if ( isURL( str ) )
        ::osl::FileBase::getSystemPathFromFileURL( str, aUStr );

    ::rtl::OString aString = ::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US );
    const char *path = aString.getStr( );
    if (( _access( path, 0 ) ) != -1 )
         bCheckResult = sal_True;
#else*/
    ::rtl::OString aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    // const char *path = aString.getStr( );
    ::osl::File testFile( str );
    bCheckResult = ( osl::FileBase::E_None == testFile.open( OpenFlag_Read ) );
    //if (bCheckResult)
    //printf("%s exist!\n", path);
    //else
    //printf("%s not exist!\n", path);
//#endif
    return bCheckResult;

}

//check if the file can be written
inline sal_Bool ifFileCanWrite( const ::rtl::OUString & str )
{
    sal_Bool        bCheckResult = sal_False;
    //on Windows, the file has no write right, but can be written
#ifdef WNT
    ::rtl::OUString     aUStr  = str.copy( 0 );
    if ( isURL( str ) )
        ::osl::FileBase::getSystemPathFromFileURL( str, aUStr );

    ::rtl::OString aString = ::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US );
    const char *path = aString.getStr( );
    if (( _access( path, 2 ) ) != -1 )
         bCheckResult = sal_True;
     //on UNX, just test if open success with OpenFlag_Write
#else
    ::osl::File testFile( str );
    bCheckResult = (osl::FileBase::E_None == testFile.open( OpenFlag_Write ));
#endif
    return bCheckResult;
}

inline sal_Bool checkDirectory( const ::rtl::OUString & str, oslCheckMode nCheckMode )
{
    rtl::OUString   aUString;
    DirectoryItem   rItem;
    FileBase::RC    rc;
    sal_Bool        bCheckResult= sal_False;

    //::std::auto_ptr<Directory> pDir( new Directory( str ) );
    Directory aDir( str );
    rc = aDir.open( );

    if ( ( ::osl::FileBase::E_NOENT != rc ) && ( ::osl::FileBase::E_ACCES != rc ) ){

        switch ( nCheckMode ) {
            case osl_Check_Mode_Exist:
                if ( rc == ::osl::FileBase::E_None )
                    bCheckResult = sal_True;
                break;
            case osl_Check_Mode_OpenAccess:
                if ( rc == ::osl::FileBase::E_None )
                    bCheckResult = sal_True;
                break;
            case osl_Check_Mode_ReadAccess:
                //rc = pDir->getNextItem( rItem, 0 );
                rc = aDir.getNextItem( rItem, 0 );
                if ( ( rc == ::osl::FileBase::E_None ) || ( rc == ::osl::FileBase::E_NOENT ) )
                    bCheckResult = sal_True;
                else
                    bCheckResult = sal_False;
                break;
            case osl_Check_Mode_WriteAccess:
                ( ( aUString += str ) += aSlashURL ) += aTmpName2;
                //if ( ( rc = pDir->create( aUString ) ) == ::osl::FileBase::E_None )
                if ( ( rc = Directory::create( aUString ) ) == ::osl::FileBase::E_None )
                {
                    bCheckResult = sal_True;
                    //rc = pDir->remove( aUString );
                    rc = Directory::remove( aUString );
                    EXPECT_TRUE( rc == ::osl::FileBase::E_None );
                }
                else
                    bCheckResult = sal_False;
                break;

            default:
                bCheckResult = sal_False;
        }// switch

        rc = aDir.close( );
        EXPECT_TRUE( rc == FileBase::E_None );

    }//if

    return bCheckResult;
}

/** construct error message
*/
inline ::rtl::OUString outputError( const ::rtl::OUString & returnVal, const ::rtl::OUString & rightVal, const sal_Char * msg = "")
{
    ::rtl::OUString aUString;
    if ( returnVal.equals( rightVal ) )
        return aUString;
    aUString += ::rtl::OUString::createFromAscii(msg);
    aUString += ::rtl::OUString::createFromAscii(": the returned value is '");
    aUString += returnVal;
    aUString += ::rtl::OUString::createFromAscii("', but the value should be '");
    aUString += rightVal;
    aUString += ::rtl::OUString::createFromAscii("'.");
    return aUString;
}

/** Change file mode, two version in UNIX and Windows;.
*/
#if ( defined UNX ) || ( defined OS2 )         //chmod() method is differ in Windows
inline void changeFileMode( ::rtl::OUString & filepath, sal_Int32 mode )
{
    rtl::OString    aString;
    rtl::OUString   aUStr  = filepath.copy( 0 );

    if ( isURL( filepath ) )
        ::osl::FileBase::getSystemPathFromFileURL( filepath, aUStr );
    aString = ::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US );
    chmod( aString.getStr( ), mode );
}
#else                                          //Windows version
inline void changeFileMode( ::rtl::OUString & filepath, sal_Int32 mode )
{
    (void)filepath;
    (void)mode;
    printf("this method is not implemented yet");
}
#endif

inline ::rtl::OUString getCurrentPID( void );



//------------------------------------------------------------------------
// Beginning of the test cases for FileBase class
//------------------------------------------------------------------------
namespace osl_FileBase
{

#if 0  //~ this function has been deprecated
    //---------------------------------------------------------------------
    // testing the method
    // static inline RC getCanonicalName( const ::rtl::OUString& ustrRequestedURL, ::rtl::OUString& ustrValidURL )
    //
    // The illegal characters are ;+=[]',\"*\\<>/?:|.
    // because getCanonicalName method is not implemented yet and will be deprecated in the future, this test is not necessary.
    //---------------------------------------------------------------------

    class getCanonicalName:public ::testing::Test
    {

        public:
        ::osl::FileBase::RC nError;
    };// class getCanonicalName

    TEST_F(getCanonicalName, getCanonicalName_001 )
    {
        ::rtl::OUString aUStr_ValidURL;
        nError = ::osl::FileBase::getCanonicalName( aCanURL1, aUStr_ValidURL );

        ASSERT_TRUE(( osl::FileBase::E_None == nError ) && aUStr_ValidURL.equalsIgnoreAsciiCase( aCanURL1 )) << "test for getCanonicalName function: check valid and unused file name";
    }

    TEST_F(getCanonicalName, getCanonicalName_002 )
    {
        ::rtl::OUString     aUStr_ValidURL;

        createTestFile( aCanURL1 );
        nError = ::osl::FileBase::getCanonicalName( aCanURL1, aUStr_ValidURL );
        deleteTestFile( aCanURL1 );

        ASSERT_TRUE(( osl::FileBase::E_None == nError ) && aUStr_ValidURL.equalsIgnoreAsciiCase( aCanURL1 )) << " test for getCanonicalName function: an existed file name, should different from the request, it did not passed(W32)(UNX)";
    }

    TEST_F(getCanonicalName, getCanonicalName_003 )
    {
        ::rtl::OUString aUStr_ValidURL;
        nError = ::osl::FileBase::getCanonicalName ( aCanURL2,  aUStr_ValidURL );

        ASSERT_TRUE(( osl::FileBase::E_None == nError ) && aUStr_ValidURL.equalsIgnoreAsciiCase( aCanURL2 )) << " test for getCanonicalName function: invalid file name, should different from the request,  it did not passed(W32)(UNX)";
    }
#endif

    //---------------------------------------------------------------------
    // testing the method
    // static inline RC getAbsoluteFileURL( const ::rtl::OUString& ustrBaseDirectoryURL,
    //                                      const ::rtl::OUString& ustrRelativeFileURL,
    //                                            ::rtl::OUString& ustrAbsoluteFileURL )
    //---------------------------------------------------------------------

    class getAbsoluteFileURL:public ::testing::Test
    {
    protected:
        ::osl::FileBase     aFileBase;
        ::rtl::OUString     aResultURL1, aResultURL2, aResultURL3, aResultURL4, aResultURL5, aResultURL6;
        ::osl::FileBase::RC nError;
        sal_Bool        bOk;

        public:
            void SetUp();
            void TearDown();
            void check_getAbsoluteFileURL( rtl::OUString const& _suBaseURL,  rtl::OString const& _sRelativeURL, ::osl::FileBase::RC _nAssumeError, rtl::OUString const& _suAssumeResultStr );
    }; //class getAbsoluteFileURL

/* use coding format as same as getSystemPathFromFileURL*/
    // initialization
    void getAbsoluteFileURL::SetUp( )
    {
        sal_Char pResultURL1[]  = "/relative/file1";
        sal_Char pResultURL2[]  = "/relative/file2";
        sal_Char pResultURL3[]  = "/file3";
        sal_Char pResultURL4[]  = "/file4";
        sal_Char pResultURL5[]  = "/canonical.name";
        sal_Char pResultURL6[]  = "/relative/";
        aResultURL1 = aUserDirectoryURL.concat( rtl::OUString::createFromAscii( pResultURL1 ) );
        aResultURL2 = aUserDirectoryURL.concat( rtl::OUString::createFromAscii( pResultURL2 ) );
        aResultURL3 = aUserDirectoryURL.concat( rtl::OUString::createFromAscii( pResultURL3 ) );
        aResultURL4 = aUserDirectoryURL.concat( rtl::OUString::createFromAscii( pResultURL4 ) );
        aResultURL5 = aUserDirectoryURL.concat( rtl::OUString::createFromAscii( pResultURL5 ) );
        aResultURL6 = aUserDirectoryURL.concat( rtl::OUString::createFromAscii( pResultURL6 ) );
    }

    void getAbsoluteFileURL::TearDown( )
    {
    }

    // test code
    TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001)
    {
        ::rtl::OUString aUStr_AbsURL;

        ::osl::FileBase::RC nError11 = aFileBase.getAbsoluteFileURL( aUserDirectoryURL,  aRelURL1, aUStr_AbsURL );
        ::rtl::OUString suError = ::rtl::OUString::createFromAscii("test for getAbsoluteFileURL(' ");
        suError += aUserDirectoryURL;
        suError += ::rtl::OUString::createFromAscii("', '");
        suError += aRelURL1;
        suError += ::rtl::OUString::createFromAscii("', '");
        suError += aUStr_AbsURL;
        suError += outputError( aUStr_AbsURL, aResultURL1, "' ),");

        sal_Bool nError12 = aUStr_AbsURL.equals( aResultURL1 );
        ::osl::FileBase::RC nError21 = aFileBase.getAbsoluteFileURL( aUserDirectoryURL,  aRelURL2, aUStr_AbsURL );
        sal_Bool nError22 = aUStr_AbsURL.equals( aResultURL2 );
        ::osl::FileBase::RC nError31 = aFileBase.getAbsoluteFileURL( aUserDirectoryURL,  aRelURL3, aUStr_AbsURL );
        sal_Bool nError32 = aUStr_AbsURL.equals( aResultURL3 );
        ::osl::FileBase::RC nError41 = aFileBase.getAbsoluteFileURL( aUserDirectoryURL,  aRelURL4, aUStr_AbsURL );
        sal_Bool nError42 = aUStr_AbsURL.equals( aResultURL4 );
        printFileName( aUStr_AbsURL );
        printFileName( aResultURL6 );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError11 ) && ( sal_True == nError12 ) &&
                                ( ::osl::FileBase::E_None == nError21 ) && ( sal_True == nError22 ) &&
                                ( ::osl::FileBase::E_None == nError31 ) && ( sal_True == nError32 ) &&
                                ( ::osl::FileBase::E_None == nError41 ) && ( sal_True == nError42 )) << "test for getAbsoluteFileURL function: valid file name with valid directory";
    }


#if ( defined UNX ) || ( defined OS2 )  //Link is not defined in Windows
    TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_002_1)
    {
        ::rtl::OUString aUStr_AbsURL, aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
        ( ( aUStr_LnkFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/link.file");
        ( ( aUStr_SrcFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/canonical.name");

            rtl::OString strLinkFileName, strSrcFileName;
            strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
            strSrcFileName =  OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

        createTestFile( aCanURL1 );
            sal_Int32 fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
        ASSERT_TRUE( fd == 0 );

        nError = aFileBase.getAbsoluteFileURL( aUserDirectoryURL, aLnkURL1, aUStr_AbsURL );
        bOk = aUStr_AbsURL.equals( aResultURL5 );

        ::rtl::OUString suError = ::rtl::OUString::createFromAscii("test for getAbsoluteFileURL(' ");
        suError += aUserDirectoryURL;
        suError += ::rtl::OUString::createFromAscii("', '");
        suError += aLnkURL1;
        suError += ::rtl::OUString::createFromAscii("', '");
        suError += aUStr_AbsURL;
        suError += outputError( aUStr_AbsURL, aResultURL5, "' ),");
        //printFileName(suError);

        deleteTestFile( aCanURL1 );
            fd = remove( strLinkFileName.getStr() );
        ASSERT_TRUE( fd == 0 );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) && ( sal_True == bOk )) << "test for getAbsoluteFileURL function: URL contain link( Solaris version )";
    }
#else       //Windows version
    TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_002_1)
    {
        ASSERT_TRUE(1) << "test for getAbsoluteFileURL function: URL contain link( Windows version )";
    }
#endif

    TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_003_1)
    {
// LLA: may be a wrong test, aTmpName1 not a real URL
#if 0
        ::rtl::OUString aUStr_AbsURL;

        nError = aFileBase.getAbsoluteFileURL( aTmpName1,  aRelURL1, aUStr_AbsURL );    //base dir invalid error
        ::rtl::OUString suError = ::rtl::OUString::createFromAscii("test for getAbsoluteFileURL('");
        suError += aTmpName1;
        suError += ::rtl::OUString::createFromAscii("', '");
        suError += aRelURL1;
        suError += ::rtl::OUString::createFromAscii("', '");
        suError += aUStr_AbsURL;
        suError += ::rtl::OUString::createFromAscii("' ),Parameter is invalid. it ignore the invalid base in Windows, did not pass in (W32), the reason maybe caused by the similar bug with getSystemPathFromFileURL() ");

        ASSERT_TRUE(( ::osl::FileBase::E_INVAL == nError )) << suError;
#endif
    }

    //use ".." in relartive path, the BasePath must exist on the file system
    TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_004_1)
    {
        //create two level directories under $Temp/PID/
        ::rtl::OUString aUStrUpBase = aUserDirectoryURL + ::rtl::OUString::createFromAscii("/test1");
        createTestDirectory( aUStrUpBase );
        ::rtl::OUString aUStrBase = aUserDirectoryURL + ::rtl::OUString::createFromAscii("/test1/dir1");
        createTestDirectory( aUStrBase );

        ::rtl::OUString aUStrRelar = ::rtl::OUString::createFromAscii("../../mytestfile");
        ::rtl::OUString aUStr_AbsURL;
        ::rtl::OUString aResultURL6 = aUserDirectoryURL + ::rtl::OUString::createFromAscii("/mytestfile");

        nError = aFileBase.getAbsoluteFileURL( aUStrBase,  aUStrRelar, aUStr_AbsURL );
        bOk = aUStr_AbsURL.equals( aResultURL6 );
        ::rtl::OUString suError = ::rtl::OUString::createFromAscii("test for getAbsoluteFileURL('");
        suError += aUStrBase;
        suError += ::rtl::OUString::createFromAscii("', '");
        suError += aUStrRelar;
        suError += ::rtl::OUString::createFromAscii("', '");
        suError += aUStr_AbsURL;
        suError += outputError( aUStr_AbsURL, aResultURL6, "' ), did not pass on Win32 ");

        deleteTestDirectory( aUStrBase );
        deleteTestDirectory( aUStrUpBase );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) && ( sal_True == bOk )) << suError.pData;
    }

    void getAbsoluteFileURL::check_getAbsoluteFileURL( rtl::OUString const& _suBaseURL,  rtl::OString const& _sRelativeURL, ::osl::FileBase::RC _nAssumeError, rtl::OUString const& _suAssumeResultStr )
    {
        rtl::OUString suRelativeURL = rtl::OStringToOUString(_sRelativeURL, RTL_TEXTENCODING_UTF8);
        rtl::OString sBaseURL = rtl::OUStringToOString(_suBaseURL, RTL_TEXTENCODING_UTF8);
        rtl::OUString suResultURL;
        osl::FileBase::RC nError = FileBase::getAbsoluteFileURL( _suBaseURL,  suRelativeURL, suResultURL );
        rtl::OString sResultURL = rtl::OUStringToOString( suResultURL, RTL_TEXTENCODING_UTF8);
        rtl::OString sError = errorToString(nError);
        printf("getAbsoluteFileURL('%s','%s') deliver absolute URL: '%s', error '%s'\n", sBaseURL.getStr(), _sRelativeURL.getStr(),sResultURL.getStr(), sError.getStr() );
        ASSERT_TRUE(nError == _nAssumeError) << "Assumption is wrong: error number is wrong";
        if ( nError == ::osl::FileBase::E_None )
        {
            sal_Bool bStrAreEqual = _suAssumeResultStr.equals( suResultURL );
                    ASSERT_TRUE(bStrAreEqual == sal_True) << "Assumption is wrong: ResultURL is not equal to expected URL ";
                }
    }

  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001_1)
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/relative/file1") );
    check_getAbsoluteFileURL( aUserDirectoryURL, "relative/file1",::osl::FileBase::E_None, suAssume );
  }
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001_2)
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/relative/file2") );
    check_getAbsoluteFileURL( aUserDirectoryURL, "relative/./file2",::osl::FileBase::E_None, suAssume );
  }
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001_3)
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/file3") );
    check_getAbsoluteFileURL( aUserDirectoryURL, "relative/../file3",::osl::FileBase::E_None, suAssume );
  }
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001_4)
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/file4") );
    check_getAbsoluteFileURL( aUserDirectoryURL, "././relative/../file4",::osl::FileBase::E_None, suAssume );
  }
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001_5)
  {
    rtl::OUString suAssume;
#if ( defined UNX )
    suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/relative/") );
#else
    suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/relative") );
#endif
    check_getAbsoluteFileURL( aUserDirectoryURL, "././relative/.",::osl::FileBase::E_None, suAssume );
  }
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001_6)
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/.relative") );
    check_getAbsoluteFileURL( aUserDirectoryURL, "./.relative",::osl::FileBase::E_None, suAssume );
  }
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001_7)
  {
    rtl::OUString suAssume;
#if (defined UNX )
    suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/.a/") );
#else //windows
    suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/.a") );
#endif
    check_getAbsoluteFileURL( aUserDirectoryURL, "./.a/mydir/..",::osl::FileBase::E_None, suAssume );
  }
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_001_8)
  {
    rtl::OUString suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/tmp/ok") );
#if ( defined UNX ) || ( defined OS2 )
    check_getAbsoluteFileURL( aUserDirectoryURL, "tmp//ok",::osl::FileBase::E_None, suAssume );
#else
    check_getAbsoluteFileURL( aUserDirectoryURL, "tmp//ok",::osl::FileBase::E_INVAL, suAssume );
#endif
  }
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_002)
  {
#if ( defined UNX ) || ( defined OS2 )      //Link is not defined in Windows
        ::rtl::OUString aUStr_AbsURL, aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
        ( ( aUStr_LnkFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/link.file");
        ( ( aUStr_SrcFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/canonical.name");

                rtl::OString strLinkFileName, strSrcFileName;
                strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
                strSrcFileName =  OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

        createTestFile( aCanURL1 );
                sal_Int32 fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
        ASSERT_TRUE( fd == 0 );
        rtl::OString sLnkURL = OUStringToOString( aLnkURL1, RTL_TEXTENCODING_ASCII_US );
            rtl::OUString suAssume = aUserDirectoryURL.concat( rtl::OUString::createFromAscii("/canonical.name") );
        check_getAbsoluteFileURL( aUserDirectoryURL, sLnkURL, ::osl::FileBase::E_None, suAssume );
        deleteTestFile( aCanURL1 );
                fd = remove( strLinkFileName.getStr() );
        ASSERT_TRUE( fd == 0 );
#endif
  }
  //please see line# 930
  TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_003)
  {
  }
    TEST_F(getAbsoluteFileURL, getAbsoluteFileURL_004)
    {
        //create two level directories under $Temp/PID/
        ::rtl::OUString aUStrUpBase = aUserDirectoryURL + ::rtl::OUString::createFromAscii("/test1");
        createTestDirectory( aUStrUpBase );
        ::rtl::OUString aUStrBase = aUserDirectoryURL + ::rtl::OUString::createFromAscii("/test1/dir1");
        createTestDirectory( aUStrBase );

        ::rtl::OUString suAssume = aUserDirectoryURL.concat( ::rtl::OUString::createFromAscii("/mytestfile") );
        check_getAbsoluteFileURL( aUStrBase, "../../mytestfile" , ::osl::FileBase::E_None, suAssume );
        deleteTestDirectory( aUStrBase );
        deleteTestDirectory( aUStrUpBase );
    }
    //---------------------------------------------------------------------
    // testing two methods:
    // static inline RC getSystemPathFromFileURL( const ::rtl::OUString& ustrFileURL,
    //                ::rtl::OUString& ustrSystemPath )
        // static RC getFileURLFromSystemPath( const ::rtl::OUString & ustrSystemPath,
        //                                ::rtl::OUString & ustrFileURL );
    //---------------------------------------------------------------------
    class SystemPath_FileURL:public ::testing::Test
    {
    protected:
        //::osl::FileBase aFileBase;
        // ::rtl::OUString aUStr;
        // ::osl::FileBase::RC nError;

      //void check_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sAssumeResultStr);
      void check_SystemPath_FileURL(rtl::OString const& _sSource, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sAssumeResultStr, sal_Bool bDirection = sal_True );
      void checkWNTBehaviour_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sWNTAssumeResultString );
      void checkUNXBehaviour_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sUnixAssumeResultString );
      void checkWNTBehaviour_getFileURLFromSystemPath(rtl::OString const& _sSysPath, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sWNTAssumeResultString);
      void checkUNXBehaviour_getFileURLFromSystemPath(rtl::OString const& _sSysPath, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sUnixAssumeResultString);

    };// class SystemPath_FileURL


    // test code.

  /*    void getSystemPathFromFileURL::check_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sAssumeResultStr)
    {
        // PRE: URL as String
        rtl::OUString suURL;
        rtl::OUString suStr;
        suURL = rtl::OStringToOUString(_sURL, RTL_TEXTENCODING_UTF8);
        ::osl::FileBase::RC nError =  osl::FileBase::getSystemPathFromFileURL( suURL, suStr ); // start with /

        // if the given string is gt length 0,
        // we check also this string
        rtl::OString sStr = rtl::OUStringToOString(suStr, RTL_TEXTENCODING_UTF8);
        rtl::OString sError = errorToString(nError);
        printf("getSystemPathFromFileURL('%s') deliver system path: '%s', error '%s'\n", _sURL.getStr(), sStr.getStr(), sError.getStr() );

        // rtl::OUString suStrEncode = rtl::Uri::encode(suStr, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
        // sStr = rtl::OUStringToOString(suStr, RTL_TEXTENCODING_UTF8);
        // printf("UTF8: %s\n", sStr.getStr() );

        if (_sAssumeResultStr.getLength() > 0)
        {
            sal_Bool bStrAreEqual = _sAssumeResultStr.equals(sStr);
            ASSERT_TRUE(nError == _nAssumeError && bStrAreEqual == sal_True) << "Assumption is wrong";
        }
        else
        {
            ASSERT_TRUE(nError == _nAssumeError) << "Assumption is wrong";
        }
    }*/

    // if bDirection==sal_True, check getSystemPathFromFileURL
    // if bDirection==sal_False, check getFileURLFromSystemPath
    void SystemPath_FileURL::check_SystemPath_FileURL(rtl::OString const& _sSource, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sAssumeResultStr, sal_Bool bDirection)
    {
        // PRE: URL as String
        rtl::OUString suSource;
        rtl::OUString suStr;
        suSource = rtl::OStringToOUString(_sSource, RTL_TEXTENCODING_UTF8);
    ::osl::FileBase::RC nError;
    if ( bDirection == sal_True )
      nError = osl::FileBase::getSystemPathFromFileURL( suSource, suStr );
    else
      nError = osl::FileBase::getFileURLFromSystemPath( suSource, suStr );

        // if the given string is gt length 0,
        // we check also this string
        rtl::OString sStr = rtl::OUStringToOString(suStr, RTL_TEXTENCODING_UTF8);
        rtl::OString sError = errorToString(nError);
    if ( bDirection == sal_True )
      printf("getSystemPathFromFileURL('%s') deliver system path: '%s', error '%s'\n", _sSource.getStr(), sStr.getStr(), sError.getStr() );
    else
      printf("getFileURLFromSystemPath('%s') deliver File URL: '%s', error '%s'\n", _sSource.getStr(), sStr.getStr(), sError.getStr() );

        // rtl::OUString suStrEncode = rtl::Uri::encode(suStr, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
        // sStr = rtl::OUStringToOString(suStr, RTL_TEXTENCODING_UTF8);
        // printf("UTF8: %s\n", sStr.getStr() );

        if (_sAssumeResultStr.getLength() > 0)
        {
            sal_Bool bStrAreEqual = _sAssumeResultStr.equals(sStr);
            ASSERT_TRUE(nError == _nAssumeError && bStrAreEqual == sal_True) << "Assumption is wrong";
        }
        else
        {
            ASSERT_TRUE(nError == _nAssumeError) << "Assumption is wrong";
        }
    }
    void SystemPath_FileURL::checkWNTBehaviour_getSystemPathFromFileURL(rtl::OString const& _sURL, ::osl::FileBase::RC _nAssumeError, rtl::OString const& _sWNTAssumeResultString)
    {
#if ( defined WNT )
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
#if ( defined WNT )
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
        check_SystemPath_FileURL(_sSysPath, _nAssumeError, _sUnixAssumeResultString, sal_False );
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
        the third parameter is the assumed result string, the string will only test, if it's length is greater 0
    */

    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_1)
    {
        rtl::OString sURL("");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }

    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_2)
    {
        rtl::OString sURL("/");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "\\");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_21)
    {
      //        rtl::OString sURL("%2f");
      rtl::OString sURL("%2F");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/"); // LLA: this is may be a BUG
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_22)
    {
      rtl::OString sURL("file:///tmp%2Fmydir");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_3)
    {
        rtl::OString sURL("a");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "a");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "a");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_31)
    {
        rtl::OString sURL("tmpname");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "tmpname");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "tmpname");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_4)
    {
        rtl::OString sURL("file://");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_41)
    {
        rtl::OString sURL("file://localhost/tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_5)
    {
        rtl::OString sURL("file:///tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/tmp");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_51)
    {
        rtl::OString sURL("file://c:/tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:/tmp"); // LLA: this is may be a BUG
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_52)
    {
        rtl::OString sURL("file:///c:/tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c:/tmp");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_53)
    {
// LLA: is this a legal file path?
        rtl::OString sURL("file:///c|/tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c|/tmp");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_6)
    {
        rtl::OString sURL("file:///tmp/first");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/tmp/first");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_61)
    {
        rtl::OString sURL("file:///c:/tmp/first");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c:/tmp/first");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp\\first");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_7)
    {
        rtl::OString sURL("file:///tmp/../second");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/tmp/../second"); // LLA: may be a BUG
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_71)
    {
        rtl::OString sURL("file:///c:/tmp/../second");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c:/tmp/../second");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp\\..\\second");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_8)
    {
        rtl::OString sURL("../tmp");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "../tmp");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "..\\tmp");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_81)
    {
        rtl::OString sURL("file://~/tmp");
    char* home_path;
    home_path = getenv("HOME");
    rtl::OString expResult(home_path);
    expResult += "/tmp";
    checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, expResult );
    //  checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "\\tmp");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_9)
    {
        rtl::OString sURL("file:///tmp/first%20second");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/tmp/first second");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_91)
    {
        rtl::OString sURL("file:///c:/tmp/first%20second");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "/c:/tmp/first second");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "c:\\tmp\\first second");
    }

    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_001_92)
    {
        rtl::OString sURL("ca@#;+.,$///78no%01ni..name");
        checkUNXBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_None, "");
        checkWNTBehaviour_getSystemPathFromFileURL(sURL, osl::FileBase::E_INVAL, "");
    }

#if 0
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_003)
        {
// LLA: ???
//!! seams to be, that the directories do not pass together
            ::rtl::OUString aUStr;
            ::rtl::OUString aRelativeURL = ::rtl::OUString::createFromAscii("../../relartive/file3");
            ::rtl::OUString aResultURL ( aSysPath4 );
            ::osl::FileBase::RC nError = osl::FileBase::getSystemPathFromFileURL( aRelativeURL, aUStr );

            sal_Bool bOk = compareFileName( aUStr, aResultURL );

            ::rtl::OUString suError = ::rtl::OUString::createFromAscii("test for getSystemPathFromFileURL(");
            suError += aRelativeURL;
            suError += ::rtl::OUString::createFromAscii(") function:use a relative file URL, did not pass in(W32), it did not specified in method declaration of relative path issue, ");
            suError += outputError(aUStr, aResultURL);
            ASSERT_TRUE(( osl::FileBase::E_None == nError ) && ( sal_True == bOk )) << suError;
        }
#endif

        //normal legal case
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_004)
        {
        ::rtl::OUString aUStr;
        ::rtl::OUString aNormalURL( aTmpName6 );
        ::rtl::OUString aResultURL ( aSysPath4 );
        ::osl::FileBase::RC nError = osl::FileBase::getSystemPathFromFileURL( aNormalURL, aUStr );

            sal_Bool bOk = compareFileName( aUStr, aResultURL );

            ::rtl::OUString suError = ::rtl::OUString::createFromAscii("test for getSystemPathFromFileURL(' ");
            suError += aNormalURL;
            suError += ::rtl::OUString::createFromAscii(" ') function:use an absolute file URL, ");
            suError += outputError(aUStr, aResultURL);

            ASSERT_TRUE(( osl::FileBase::E_None == nError ) && ( sal_True == bOk )) << suError.pData;

        }

        //CJK characters case
    TEST_F(SystemPath_FileURL, getSystemPathFromFileURL_005)
        {
            ::rtl::OUString aUStr;
            createTestDirectory( aTmpName10 );
            ::rtl::OUString aNormalURL( aTmpName10 );
            ::rtl::OUString aResultURL ( aSysPath5 );

            ::osl::FileBase::RC nError = osl::FileBase::getSystemPathFromFileURL( aNormalURL, aUStr );

            sal_Bool bOk = compareFileName( aUStr, aResultURL );

            ::rtl::OUString suError = ::rtl::OUString::createFromAscii("test for getSystemPathFromFileURL(' ");
            suError += aNormalURL;
            suError += ::rtl::OUString::createFromAscii(" ') function:use a CJK coded absolute URL, ");
            suError += outputError(aUStr, aResultURL);
            deleteTestDirectory( aTmpName10 );

            ASSERT_TRUE(( osl::FileBase::E_None == nError ) && ( sal_True == bOk )) << suError.pData;
        }
     TEST_F(SystemPath_FileURL, getFileURLFromSystemPath_001)
     {
        rtl::OString sSysPath("~/tmp");
    char* home_path;
    home_path = getenv("HOME");
    rtl::OString expResult(home_path);
    expResult = "file://"+ expResult + "/tmp";
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, expResult );
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, "~/tmp");
     }
     TEST_F(SystemPath_FileURL, getFileURLFromSystemPath_002)
     {
        rtl::OString sSysPath("c:/tmp");
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, "c:/tmp");
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, "file:///c:/tmp");
     }
     TEST_F(SystemPath_FileURL, getFileURLFromSystemPath_003)
     {
        rtl::OString sSysPath("file:///temp");
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
     }
    TEST_F(SystemPath_FileURL, getFileURLFromSystemPath_004)
     {
        rtl::OString sSysPath("//tmp//first start");
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_None, "file:///tmp/first%20start");
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
     }
     TEST_F(SystemPath_FileURL, getFileURLFromSystemPath_005)
     {
        rtl::OString sSysPath("");
        checkUNXBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
        checkWNTBehaviour_getFileURLFromSystemPath(sSysPath, osl::FileBase::E_INVAL, "");
     }
         // start with "~user", not impletment
    //      void SystemPath_FileURL::getFileURLFromSystemPath_006()




    //---------------------------------------------------------------------
    // testing the method
    // static inline RC searchFileURL(  const ::rtl::OUString& ustrFileName,
    //                                  const ::rtl::OUString& ustrSearchPath,
    //                                  ::rtl::OUString& ustrFileURL )
    //---------------------------------------------------------------------
    class searchFileURL:public ::testing::Test
    {
    protected:
        //::osl::FileBase aFileBase;
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1, nError2, nError3,nError4;

        public:
    };// class searchFileURL

    // test code.
    TEST_F(searchFileURL, searchFileURL_001 )
    {
        /* search file is passed by system filename */
        nError1 = ::osl::FileBase::searchFileURL( aTmpName1, aUserDirectorySys, aUStr );
        /* search file is passed by full qualified file URL */
        nError2 = ::osl::FileBase::searchFileURL( aCanURL1, aUserDirectorySys, aUStr );
        /* search file is passed by relative file path */
        nError3 = ::osl::FileBase::searchFileURL( aRelURL4, aUserDirectorySys, aUStr );

        ASSERT_TRUE(( osl::FileBase::E_NOENT == nError1 ) &&
                                ( osl::FileBase::E_NOENT == nError2 ) &&
                                ( osl::FileBase::E_NOENT == nError3 )) << "test for searchFileURL function: system filename/URL filename/relative path, system directory, searched files that is not exist, but it reply invalid error, did not pass in (W32) ";
    }

    TEST_F(searchFileURL, searchFileURL_002 )
    {
        /* search file is passed by system filename */
        nError1 = ::osl::FileBase::searchFileURL( aTempDirectorySys, aRootSys, aUStr );
        sal_Bool bOk1 = compareFileName( aUStr, aTempDirectoryURL );
        /* search file is passed by full qualified file URL */
        nError2 = ::osl::FileBase::searchFileURL( aTempDirectoryURL, aRootSys, aUStr );
        sal_Bool bOk2 = compareFileName( aUStr, aTempDirectoryURL );
        /* search file is passed by relative file path */
        nError3 = ::osl::FileBase::searchFileURL( aRelURL5, aRootSys, aUStr );
        sal_Bool bOk3 = compareFileName( aUStr, aTempDirectoryURL );
        /* search file is passed by an exist file */
        createTestFile( aCanURL1 );
        nError4 = ::osl::FileBase::searchFileURL( aCanURL4, aUserDirectorySys, aUStr );
        sal_Bool bOk4 = compareFileName( aUStr, aCanURL1 );
        deleteTestFile( aCanURL1 );

        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) &&
                                ( osl::FileBase::E_None == nError2 ) &&
                                ( osl::FileBase::E_None == nError3 ) &&
                                ( osl::FileBase::E_None == nError4 ) &&
                                ( sal_True == bOk1 ) &&
                                ( sal_True == bOk2 ) &&
                                ( sal_True == bOk3 ) &&
                                ( sal_True == bOk4 )) << "test for searchFileURL function: system filename/URL filename/relative path, system directory, searched file already exist.";
    }


    TEST_F(searchFileURL, searchFileURL_003 )
    {
        OSLTEST_DECLARE( SystemPathList,  TEST_PLATFORM_ROOT":"TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP":"TEST_PLATFORM_ROOT"system/path" );
        nError1 = ::osl::FileBase::searchFileURL( aUserDirectoryURL, aSystemPathList, aUStr );
        sal_Bool bOk = compareFileName( aUStr, aUserDirectoryURL );
        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) &&
                                ( sal_True == bOk )) << "test for searchFileURL function: search directory is a list of system paths";
    }

    TEST_F(searchFileURL, searchFileURL_004 )
    {
        OSLTEST_DECLARE( SystemPathList,  TEST_PLATFORM_ROOT PATH_LIST_DELIMITER TEST_PLATFORM_ROOT TEST_PLATFORM_TEMP PATH_LIST_DELIMITER TEST_PLATFORM_ROOT "system/path/../name" );
        nError1 = ::osl::FileBase::searchFileURL( aUserDirectoryURL, aSystemPathList, aUStr );
        sal_Bool bOk = compareFileName( aUStr, aUserDirectoryURL );
        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) &&
                                ( sal_True == bOk )) << "test for searchFileURL function: search directory is a list of system paths";
    }

    TEST_F(searchFileURL, searchFileURL_005 )
    {
        nError1 = ::osl::FileBase::searchFileURL( aUserDirectoryURL, aNullURL, aUStr );
        sal_Bool bOk = compareFileName( aUStr, aUserDirectoryURL );
        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) &&
                                ( sal_True == bOk )) << "test for searchFileURL function: search directory is NULL";
    }

    //---------------------------------------------------------------------
    // testing the method
    // static inline RC getTempDirURL( ::rtl::OUString& ustrTempDirURL )
    //---------------------------------------------------------------------
    class getTempDirURL:public ::testing::Test
    {
    protected:
        //::osl::FileBase aFileBase;
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError;

        public:
        // initialization
        void SetUp( )
        {
             nError = FileBase::getTempDirURL( aUStr );
        }

        void TearDown( )
        {
        }
    };// class getTempDirURL

    TEST_F(getTempDirURL, getTempDirURL_001 )
    {

        ASSERT_TRUE(( osl::FileBase::E_None == nError )) << "test for getTempDirURL function: excution";
    }

    TEST_F(getTempDirURL, getTempDirURL_002 )
    {
        ASSERT_TRUE(checkDirectory( aUStr, osl_Check_Mode_OpenAccess ) &&
                                checkDirectory( aUStr, osl_Check_Mode_ReadAccess ) &&
                                checkDirectory( aUStr,osl_Check_Mode_WriteAccess )) << "test for getTempDirURL function: test for open and write access rights";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  static inline RC createTempFile( ::rtl::OUString* pustrDirectoryURL,
    //                                   oslFileHandle* pHandle,
    //                                   ::rtl::OUString* pustrTempFileURL)
    //---------------------------------------------------------------------
    class createTempFile:public ::testing::Test
    {
    protected:
        //::osl::FileBase aFileBase;
        ::osl::FileBase::RC nError1, nError2;
        sal_Bool bOK;

        oslFileHandle   *pHandle;
        ::rtl::OUString *pUStr_DirURL;
        ::rtl::OUString *pUStr_FileURL;

        public:

        // initialization
        void SetUp( )
        {
            pHandle = new oslFileHandle();
            pUStr_DirURL = new ::rtl::OUString( aUserDirectoryURL );
            pUStr_FileURL = new ::rtl::OUString();
            //*pUStr_DirURL = aUserDirectoryURL;                /// create temp file in /tmp/PID or c:\temp\PID.*/
        }

        void TearDown( )
        {
            delete pUStr_DirURL;
            delete pUStr_FileURL;
            delete pHandle;
        }
    };// class createTempFile

    TEST_F(createTempFile, createTempFile_001 )
    {
        nError1 = FileBase::createTempFile( pUStr_DirURL, pHandle, pUStr_FileURL );
        ::osl::File testFile( *pUStr_FileURL );
        //printFileName(*pUStr_FileURL);
        nError2 = testFile.open( OpenFlag_Create );
        if ( osl::FileBase::E_EXIST == nError2 )  {
            osl_closeFile( *pHandle );
            deleteTestFile( *pUStr_FileURL );
        }

        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) && ( pHandle != NULL ) &&   ( osl::FileBase::E_EXIST== nError2 )) << "test for createTempFile function: create temp file and test the existence";
    }

    TEST_F(createTempFile, createTempFile_002 )
    {
        bOK = sal_False;
        nError1 = FileBase::createTempFile( pUStr_DirURL, pHandle, pUStr_FileURL );
        ::osl::File testFile( *pUStr_FileURL );
        nError2 = testFile.open( OpenFlag_Create );

        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) && ( pHandle != NULL ) &&
            ( osl::FileBase::E_EXIST == nError2 )) << "createTempFile function: create a temp file, but it does not exist";

        //check file if have the write permission
        if ( osl::FileBase::E_EXIST == nError2 )  {
            bOK = ifFileCanWrite( *pUStr_FileURL );
            osl_closeFile( *pHandle );
            deleteTestFile( *pUStr_FileURL );
        }

        ASSERT_TRUE(( sal_True == bOK )) << "test for open and write access rights, in (W32), it did not have write access right, but it should be writtenable.";
    }

    TEST_F(createTempFile, createTempFile_003 )
    {
        nError1 = FileBase::createTempFile( pUStr_DirURL, pHandle, 0 );
        //the temp file will be removed when return from createTempFile
        bOK = ( pHandle != NULL && pHandle != 0);
        if ( sal_True == bOK )
            osl_closeFile( *pHandle );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError1 ) &&( sal_True == bOK )) << "test for createTempFile function: set pUStrFileURL to 0 to let it remove the file after call.";
    }
    TEST_F(createTempFile, createTempFile_004 )
    {
        nError1 = FileBase::createTempFile( pUStr_DirURL, 0, pUStr_FileURL );
        bOK = ( pUStr_FileURL != 0);
        ::osl::File testFile( *pUStr_FileURL );
        nError2 = testFile.open( OpenFlag_Create );
        deleteTestFile( *pUStr_FileURL );
        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) && ( osl::FileBase::E_EXIST == nError2 ) &&( sal_True == bOK )) << "createTempFile function: create a temp file, but it does not exist";

    }

}// namespace osl_FileBase


//------------------------------------------------------------------------
// Beginning of the test cases for VolumeDevice class
//------------------------------------------------------------------------

#if 0  //~ this Class has been deprecated
namespace osl_VolumeDevice
{

    //---------------------------------------------------------------------
    //  testing the method
    //  VolumeDevice() : _aHandle( NULL )
    //---------------------------------------------------------------------
    class  VolumeDeviceCtors : public ::testing::Test
    {
    protected:
        ::osl::VolumeDevice aVolumeDevice;
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
        }

        void TearDown( )
        {
        }
    };// class ctors

    TEST_F(VolumeDeviceCtors, ctors_001 )
    {
        ::osl::VolumeDevice aVolumeDevice1;

        ASSERT_TRUE(( osl::FileBase::E_None !=  aVolumeDevice1.automount( ) ) &&
                                ( osl::FileBase::E_None !=  aVolumeDevice1.unmount( ) )   &&
                                ( aNullURL.equals( aVolumeDevice1.getMountPath( ) ) )) << "test for ctors function: Constructor for VolumeDevice with no args.";
    }

        TEST_F(VolumeDeviceCtors, ctors_002 )
    {
        ::osl::VolumeInfo   aVolumeInfo( VolumeInfoMask_Attributes );
        nError1 = ::osl::Directory::getVolumeInfo( aRootURL, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );

        ::osl::VolumeDevice aVolumeDevice1( aVolumeInfo.getDeviceHandle( ) );
        sal_Bool bOk = compareFileName( aNullURL, aVolumeDevice1.getMountPath( ) );
        ASSERT_TRUE(sal_False == bOk) << "test for ctors function: Copy constructor for VolumeDevice, the copied VolumeDevice should have a mount path file:///, but it returned an empty OUString, it also may be the error from getDeviceHandle(), it did not pass in (UNX), (W32).";
    }

        TEST_F(VolumeDeviceCtors, ctors_003 )
    {
        ::osl::VolumeInfo   aVolumeInfo( VolumeInfoMask_Attributes );
        nError1 = ::osl::Directory::getVolumeInfo( aRootURL, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );

        ::osl::VolumeDevice aVolumeDevice1 = aVolumeInfo.getDeviceHandle( );
        sal_Bool bOk = compareFileName( aNullURL, aVolumeDevice1.getMountPath( ) );
        ASSERT_TRUE(sal_False == bOk) << "test for ctors function: Assigned operator for VolumeDevice, the assigned VolumeDevice should have a mount path file:///, but it returned an empty OUString, it also may be the error from getDeviceHandle(),it did not pass in (UNX), (W32).";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC automount()
    //---------------------------------------------------------------------
    class  automount : public ::testing::Test
    {
    protected:
        ::osl::VolumeDevice aVolumeDevice;
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
        }

        void TearDown( )
        {

        }
    };// class automount

    TEST_F(automount, automount_001 )
    {
        ::osl::VolumeDevice aVolumeDevice1;
        nError1 = aVolumeDevice1.automount( );

        ASSERT_TRUE(( osl::FileBase::E_INVAL == nError1 )) << "test for automount function: invalid parameter.";
    }

        TEST_F(automount, automount_002 )
    {
        ::osl::VolumeInfo   aVolumeInfo( VolumeInfoMask_Attributes );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL2, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );

        ::osl::VolumeDevice aVolumeDevice1( aVolumeInfo.getDeviceHandle( ) );
        nError1 = aVolumeDevice1.unmount( );
        nError1 = aVolumeDevice1.automount( );
        ASSERT_TRUE(( osl::FileBase::E_None == nError1 )) << "test for automount function: this test is not implemented yet, it did not pass in (UNX), (W32).";
    }

}// namespace osl_VolumeDevice
#endif


//------------------------------------------------------------------------
// Beginning of the test cases for VolumeInfo class
//------------------------------------------------------------------------
namespace osl_VolumeInfo
{

    //---------------------------------------------------------------------
    //  testing the method
    //  VolumeInfo( sal_uInt32 nMask ): _nMask( nMask )
    //---------------------------------------------------------------------
    class  VolumeInfoCtors : public ::testing::Test
    {
    protected:
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1, nError2;

         ::osl::VolumeDevice aVolumeDevice1;

        public:
        // initialization
        void SetUp( )
        {
        }

        void tearDown( )
        {
        }
    };// class ctors

    TEST_F(VolumeInfoCtors, ctors_001 )
    {
        ::osl::VolumeInfo   aVolumeInfo( 0 );
        nError1 = ::osl::Directory::getVolumeInfo( aRootURL, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_uInt64 uiTotalSpace = aVolumeInfo.getTotalSpace( );
        sal_uInt32 uiMaxPathLength = aVolumeInfo.getMaxPathLength( );
        aUStr = aVolumeInfo.getFileSystemName( );

        ASSERT_TRUE(( 0 == uiTotalSpace ) &&
                                ( 0 == uiMaxPathLength ) &&
                                sal_True == compareFileName( aUStr, aNullURL )) << "test for ctors function: mask is empty";
    }

#if ( defined UNX ) || ( defined OS2 )
    TEST_F(VolumeInfoCtors, ctors_002 )
    {
        ::osl::VolumeInfo   aVolumeInfo( VolumeInfoMask_TotalSpace |
                                         VolumeInfoMask_UsedSpace |
                                         VolumeInfoMask_FileSystemName );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL4, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        //ASSERT_TRUE( aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiTotalSpace = aVolumeInfo.getTotalSpace( );
        sal_uInt64 uiUsedSpace = aVolumeInfo.getUsedSpace( );
        aUStr = aVolumeInfo.getFileSystemName( );

        ASSERT_TRUE(( 0 != uiTotalSpace ) &&
                                ( 0 != uiUsedSpace ) &&
                                sal_True == compareFileName( aUStr, "nfs" )) << "test for ctors function: mask is specified as certain valid fields, and get the masked fields";
    }
#else           /// Windows version,here we can not determine whichvolume in Windows is serve as an nfs volume.
    TEST_F(VolumeInfoCtors, ctors_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for ctors function: mask is specified as certain valid fields, and get the masked fields( Windows version )";
    }
#endif

    TEST_F(VolumeInfoCtors, ctors_003 )
    {

            sal_Int32 mask1 = VolumeInfoMask_FreeSpace;
        ::osl::VolumeInfo aVolumeInfo1( mask1 );
        nError1 = ::osl::Directory::getVolumeInfo( aRootURL, aVolumeInfo1 );
        ASSERT_TRUE( sal_True == aVolumeInfo1.isValid( mask1 ) );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );

        sal_uInt64 uiTotalSpace1 = aVolumeInfo1.getTotalSpace( );
        aUStr = aVolumeInfo1.getFileSystemName( );

            sal_Int32 mask2 = VolumeInfoMask_TotalSpace;
        ::osl::VolumeInfo aVolumeInfo2( mask2 );
        nError2 = ::osl::Directory::getVolumeInfo( aRootURL, aVolumeInfo2 );
        ASSERT_TRUE( sal_True == aVolumeInfo2.isValid( mask2 ) );
        ASSERT_TRUE( osl::FileBase::E_None == nError2 );

        sal_uInt64 uiTotalSpace2 = aVolumeInfo2.getTotalSpace( );

        ASSERT_TRUE(( 0 == uiTotalSpace1 ) && ( 0 != uiTotalSpace2 ) &&
                                sal_True == compareFileName( aUStr, aNullURL )) << "test for ctors function: mask is specified as certain valid fields, but get unmasked fields, use mask to FreeSpace, but I can get TotalSpace, did not pass in (UNX)(W32)";
    }

     //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool isValid( sal_uInt32 nMask ) const
    //---------------------------------------------------------------------
    class  VolumeInfoIsValid : public ::testing::Test
    {
    protected:
        ::osl::VolumeDevice aVolumeDevice;
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
        }

        void TearDown( )
        {

        }
    };// class isValid

    TEST_F(VolumeInfoIsValid, isValid_001 )
    {
        sal_Int32 mask = 0;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL4, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(sal_True == aVolumeInfo.isValid( mask )) << "test for isValid function: no fields specified.";
    }

#if ( defined UNX ) || ( defined OS2 )
    TEST_F(VolumeInfoIsValid, isValid_002 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes | VolumeInfoMask_TotalSpace | osl_VolumeInfo_Mask_UsedSpace |
                         osl_VolumeInfo_Mask_FreeSpace | osl_VolumeInfo_Mask_MaxNameLength |
                         osl_VolumeInfo_Mask_MaxPathLength | osl_VolumeInfo_Mask_FileSystemName;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL4, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(sal_True == aVolumeInfo.isValid( mask )) << "test for isValid function: all valid fields specified for a nfs volume.";
    }
#else           /// Windows version,here we can not determine whichvolume in Windows is serve as an nfs volume.
    TEST_F(VolumeInfoIsValid, isValid_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for isValid function: all valid fields specified for a nfs volume.( Windows version )";
    }
#endif

    TEST_F(VolumeInfoIsValid, isValid_003 )
    {
            ::osl::VolumeDevice aVolumeDevice1;
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo   aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk1 = aVolumeInfo.isValid( mask );

        nError1 = ::osl::Directory::getVolumeInfo( aVolURL2, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk2 = aVolumeInfo.isValid( mask );

        ASSERT_TRUE(( sal_True == bOk1 ) && ( sal_True == bOk2 )) << "test for isValid function: VolumeInfoMask_Attributes, it should be valid for some volume such as /, floppy, cdrom, etc. but it did not pass";
    }

     //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool getRemoteFlag() const
    //---------------------------------------------------------------------
    class  getRemoteFlag : public ::testing::Test
    {
    protected:
        ::osl::VolumeDevice aVolumeDevice;
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1, nError2;
    };// class getRemoteFlag

    TEST_F(getRemoteFlag, getRemoteFlag_001 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getRemoteFlag( );

        ASSERT_TRUE(( sal_False == bOk )) << "test for getRemoteFlag function: get a volume device which is not remote.";
    }

 #if ( defined UNX ) || ( defined OS2 ) //remote Volume is different in Solaris and Windows
    TEST_F(getRemoteFlag, getRemoteFlag_002 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL4, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getRemoteFlag( );

        ASSERT_TRUE(( sal_True == bOk )) << "test for getRemoteFlag function: get a volume device which is remote( Solaris version ).";
    }
#else                                    //Windows version
    TEST_F(getRemoteFlag, getRemoteFlag_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for getRemoteFlag function: get a volume device which is remote( Windows version )";
    }
#endif

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool getRemoveableFlag() const
    //---------------------------------------------------------------------
    class  getRemoveableFlag : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1, nError2;
    };// class getRemoveableFlag

    TEST_F(getRemoveableFlag, getRemoveableFlag_001 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getRemoveableFlag( );

        ASSERT_TRUE(sal_False == bOk) << "test for getRemoveableFlag function: get a volume device which is not removable.";
    }

    TEST_F(getRemoveableFlag, getRemoveableFlag_002 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL2, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getRemoveableFlag( );

        ASSERT_TRUE(sal_True == bOk) << "test for getRemoveableFlag function: get a volume device which is removable, not sure, here we use floppy disk, but it did not pass.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool getCompactDiscFlag() const
    //---------------------------------------------------------------------
    class  getCompactDiscFlag : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getCompactDiscFlag

    TEST_F(getCompactDiscFlag, getCompactDiscFlag_001 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getCompactDiscFlag( );

        ASSERT_TRUE(( sal_False == bOk )) << "test for getCompactDiscFlag function: get a volume device which is not a cdrom.";
    }

    TEST_F(getCompactDiscFlag, getCompactDiscFlag_002 )
    {
            sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL6, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getCompactDiscFlag( );

        ASSERT_TRUE(( sal_True == bOk )) << "test for getCompactDiscFlag function: get a cdrom volume device flag, it did not pass.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool getFloppyDiskFlag() const
    //---------------------------------------------------------------------
    class  getFloppyDiskFlag : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getFloppyDiskFlag

    TEST_F(getFloppyDiskFlag, getFloppyDiskFlag_001 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getFloppyDiskFlag( );

        ASSERT_TRUE(( sal_False == bOk )) << "test for getFloppyDiskFlag function: get a volume device which is not a floppy disk.";
    }

    TEST_F(getFloppyDiskFlag, getFloppyDiskFlag_002 )
    {
            sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL2, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getFloppyDiskFlag( );

        ASSERT_TRUE(( sal_True == bOk )) << "test for getFloppyDiskFlag function: get a floppy volume device flag, it did not pass.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool getFixedDiskFlag() const
    //---------------------------------------------------------------------
    class  getFixedDiskFlag : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getFixedDiskFlag

    TEST_F(getFixedDiskFlag, getFixedDiskFlag_001 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL2, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getFixedDiskFlag( );

        ASSERT_TRUE(( sal_False == bOk )) << "test for getFixedDiskFlag function: get a volume device which is not a fixed disk.";
    }

    TEST_F(getFixedDiskFlag, getFixedDiskFlag_002 )
    {
            sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getFixedDiskFlag( );

        ASSERT_TRUE(( sal_True == bOk )) << "test for getFixedDiskFlag function: get a fixed disk volume device flag, it did not pass.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool getRAMDiskFlag() const
    //---------------------------------------------------------------------
    class  getRAMDiskFlag : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getRAMDiskFlag

    TEST_F(getRAMDiskFlag, getRAMDiskFlag_001 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getRAMDiskFlag( );

        ASSERT_TRUE(( sal_False == bOk )) << "test for getRAMDiskFlag function: get a volume device which is not a RAM disk.";
    }

    TEST_F(getRAMDiskFlag, getRAMDiskFlag_002 )
    {
            sal_Int32 mask = VolumeInfoMask_Attributes;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        sal_Bool bOk = aVolumeInfo.getRAMDiskFlag( );

        ASSERT_TRUE(( sal_True == bOk )) << "test for getRAMDiskFlag function: FIX ME, don't know how to get a RAM disk flag, perhaps Windows 98 boot disk can create a RAM disk, it did not pass in (UNX)(W32).";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_uInt64 getTotalSpace() const
    //---------------------------------------------------------------------
    class  getTotalSpace : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getTotalSpace

    TEST_F(getTotalSpace, getTotalSpace_001 )
    {
        sal_Int32 mask = VolumeInfoMask_TotalSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiTotalSpace = aVolumeInfo.getTotalSpace( );

        ASSERT_TRUE(0 != uiTotalSpace) << "test for getTotalSpace function: get total space of Fixed disk volume mounted on /, it should not be 0";
    }

 #if defined( UNX )
    TEST_F(getTotalSpace, getTotalSpace_002 )
    {
            sal_Int32 mask = VolumeInfoMask_TotalSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL3, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiTotalSpace = aVolumeInfo.getTotalSpace( );

        ASSERT_TRUE(0 == uiTotalSpace) << "test for getTotalSpace function: get total space of /proc, it should be 0";
    }
#else           /// Windows version, in Windows, there is no /proc directory
    TEST_F(getTotalSpace, getTotalSpace_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for getTotalSpace function:not applicable for /proc( Windows version )";
    }
#endif



#if defined(SOLARIS)
        TEST_F(getTotalSpace, getTotalSpace_003 )
    {
            struct statvfs aStatFS;
        static const sal_Char  name[] = "/";

        memset (&aStatFS, 0, sizeof(aStatFS));
        statvfs( name, &aStatFS);
        sal_uInt64 TotalSpace = aStatFS.f_frsize * aStatFS.f_blocks ;

            sal_Int32 mask = VolumeInfoMask_TotalSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiTotalSpace = aVolumeInfo.getTotalSpace( );

            ASSERT_TRUE(uiTotalSpace == TotalSpace) << "test for getTotalSpace function: get total space by hand, then compare with getTotalSpace, it did not pass";
    }
#else           /// Windows version
    TEST_F(getTotalSpace, getTotalSpace_003 )
    {
        ASSERT_TRUE(1 == 1) << "test for getTotalSpace function:not implemented yet( Windows version )";
    }
#endif

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_uInt64 getFreeSpace() const
    //---------------------------------------------------------------------
    class  getFreeSpace : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getFreeSpace

    TEST_F(getFreeSpace, getFreeSpace_001 )
    {
        sal_Int32 mask = VolumeInfoMask_FreeSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiFreeSpace = aVolumeInfo.getFreeSpace( );

        ASSERT_TRUE(0 != uiFreeSpace) << "test for getFreeSpace function: get free space of Fixed disk volume mounted on /, it should not be 0, suggestion: returned value, -1 is better, since some times the free space may be 0";
    }

#if defined( UNX )
        TEST_F(getFreeSpace, getFreeSpace_002 )
    {
            sal_Int32 mask = VolumeInfoMask_FreeSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL3, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiFreeSpace = aVolumeInfo.getFreeSpace( );

        ASSERT_TRUE(0 == uiFreeSpace) << "test for getFreeSpace function: get free space of /proc, it should be 0";
    }
#else           /// Windows version, in Windows, there is no /proc directory
    TEST_F(getFreeSpace, getFreeSpace_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for getFreeSpace function: not applicable for /proc( Windows version )";
    }
#endif


#if defined(SOLARIS)
        TEST_F(getFreeSpace, getFreeSpace_003 )
    {
            struct statvfs aStatFS;
        static const sal_Char  name[] = "/";

        memset (&aStatFS, 0, sizeof(aStatFS));
        statvfs( name, &aStatFS);
        sal_uInt64 FreeSpace = aStatFS.f_bfree * aStatFS.f_frsize  ;

            sal_Int32 mask = VolumeInfoMask_FreeSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiFreeSpace = aVolumeInfo.getFreeSpace( );

            ASSERT_TRUE(uiFreeSpace == FreeSpace) << "test for getFreeSpace function: get free space by hand, then compare with getFreeSpace, it did not pass";
    }
#else                                    //Windows version
    TEST_F(getFreeSpace, getFreeSpace_003 )
    {
        ASSERT_TRUE(1 == 1) << "test for getFreeSpace function: not implemented yet( Windows version )";
    }
#endif

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_uInt64 getUsedSpace() const
    //---------------------------------------------------------------------
    class  getUsedSpace : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getUsedSpace

    TEST_F(getUsedSpace, getUsedSpace_001 )
    {
        sal_Int32 mask = VolumeInfoMask_UsedSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiUsedSpace = aVolumeInfo.getUsedSpace( );

        ASSERT_TRUE(0 != uiUsedSpace) << "test for getUsedSpace function: get used space of Fixed disk volume mounted on /, it should not be 0, suggestion: returned value, -1 is better, since some times the used space may be 0";
    }

#if defined( UNX )
        TEST_F(getUsedSpace, getUsedSpace_002 )
    {
            sal_Int32 mask = VolumeInfoMask_UsedSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL3, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiUsedSpace = aVolumeInfo.getUsedSpace( );

        ASSERT_TRUE(0 == uiUsedSpace) << "test for getUsedSpace function: get used space of /proc, it should be 0";
    }
#else                                    /// Windows version, in Windows, there is no /proc directory
    TEST_F(getUsedSpace, getUsedSpace_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for getUsedSpace function: not applicable for /proc( Windows version )";
    }
#endif


#if defined(SOLARIS)
        TEST_F(getUsedSpace, getUsedSpace_003 )
    {
            struct statvfs aStatFS;
        static const sal_Char  name[] = "/";

        memset (&aStatFS, 0, sizeof(aStatFS));
        statvfs( name, &aStatFS);
        sal_uInt64 UsedSpace = ( aStatFS.f_blocks - aStatFS.f_bavail ) * aStatFS.f_frsize;


            sal_Int32 mask = VolumeInfoMask_UsedSpace;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiUsedSpace = aVolumeInfo.getUsedSpace( );

            ASSERT_TRUE(uiUsedSpace == UsedSpace) << "test for getUsedSpace function: get used space by hand, then compare with getUsedSpace, it did not pass";
    }
#else                                    //Windows version
    TEST_F(getUsedSpace, getUsedSpace_003 )
    {
        ASSERT_TRUE(1 == 1) << "test for getUsedSpace function: not implemented yet( Windows version )";
    }
#endif

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_uInt32 getMaxNameLength() const
    //---------------------------------------------------------------------
    class  getMaxNameLength : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getMaxNameLength

    TEST_F(getMaxNameLength, getMaxNameLength_001 )
    {
        sal_Int32 mask = VolumeInfoMask_MaxNameLength;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt32 uiMaxNameLength = aVolumeInfo.getMaxNameLength( );

        ASSERT_TRUE(0 != uiMaxNameLength) << "test for getMaxNameLength function: get max name length of Fixed disk volume mounted on /, it should not be 0";
    }


#if ( defined UNX ) || ( defined OS2 )
        TEST_F(getMaxNameLength, getMaxNameLength_002 )
    {
            struct statvfs aStatFS;
        static const sal_Char  name[] = "/";

        memset (&aStatFS, 0, sizeof(aStatFS));
        statvfs( name, &aStatFS);
        sal_uInt64 MaxNameLength = aStatFS.f_namemax;

            sal_Int32 mask = VolumeInfoMask_MaxNameLength;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiMaxNameLength = aVolumeInfo.getMaxNameLength( );

            ASSERT_TRUE(uiMaxNameLength == MaxNameLength) << "test for getMaxNameLength function: get max name length by hand, then compare with getMaxNameLength";
    }
#else                                    //Windows version
    TEST_F(getMaxNameLength, getMaxNameLength_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for getMaxNameLength function: not implemented yet( Windows version )";
    }
#endif

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_uInt32 getMaxPathLength() const
    //---------------------------------------------------------------------
    class  getMaxPathLength : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC nError1;
    };// class getMaxPathLength

    TEST_F(getMaxPathLength, getMaxPathLength_001 )
    {
        sal_Int32 mask = VolumeInfoMask_MaxPathLength;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt32 uiMaxPathLength = aVolumeInfo.getMaxPathLength( );

        ASSERT_TRUE(0 != uiMaxPathLength) << "test for getMaxPathLength function: get max path length of Fixed disk volume mounted on /, it should not be 0";
    }


#if ( defined UNX ) || ( defined OS2 )
        TEST_F(getMaxPathLength, getMaxPathLength_002 )
    {
            sal_Int32 mask = VolumeInfoMask_MaxPathLength;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        sal_uInt64 uiMaxPathLength = aVolumeInfo.getMaxPathLength( );

            ASSERT_TRUE(uiMaxPathLength == PATH_MAX) << "test for getMaxPathLength function: get max path length by hand, then compare with getMaxPathLength";
    }
#else                                    //Windows version
    TEST_F(getMaxPathLength, getMaxPathLength_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for getMaxPathLength function: not implemented yet( Windows version )";
    }
#endif

    //---------------------------------------------------------------------
    //  testing the method
    //  inline ::rtl::OUString getFileSystemName() const
    //---------------------------------------------------------------------
    class  getFileSystemName : public ::testing::Test
    {
    protected:
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1;
    };// class getFileSystemName

    TEST_F(getFileSystemName, getFileSystemName_001 )
    {
        sal_Int32 mask = VolumeInfoMask_FileSystemName;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        aUStr = aVolumeInfo.getFileSystemName( );

        ASSERT_TRUE(sal_False == compareFileName( aNullURL, aUStr )) << "test for getFileSystemName function: get file system name of Fixed disk volume mounted on /, it should not be empty string";
    }


#if defined(SOLARIS)
        TEST_F(getFileSystemName, getFileSystemName_002 )
    {
            struct statvfs aStatFS;
        static const sal_Char  name[] = "/";

        memset (&aStatFS, 0, sizeof(aStatFS));
        statvfs( name, &aStatFS);
        sal_Char * astrFileSystemName = aStatFS.f_basetype;

            sal_Int32 mask = VolumeInfoMask_FileSystemName;
        ::osl::VolumeInfo aVolumeInfo( mask );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );
        ASSERT_TRUE( sal_True == aVolumeInfo.isValid( mask ) );
        aUStr = aVolumeInfo.getFileSystemName( );

            ASSERT_TRUE(sal_True == compareFileName( aUStr, astrFileSystemName )) << "test for getFileSystemName function: get file system name by hand, then compare with getFileSystemName";
    }
#else                                    //Windows version
    TEST_F(getFileSystemName, getFileSystemName_002 )
    {
        ASSERT_TRUE(1 == 1) << "test for getFileSystemName function: not implemented yet( Windows version )";
    }
#endif

    //---------------------------------------------------------------------
    //  testing the method
    //  inline VolumeDevice getDeviceHandle() const
    //---------------------------------------------------------------------
    class  getDeviceHandle : public ::testing::Test
    {
    protected:
        ::rtl::OUString aUStr;
        ::osl::FileBase::RC nError1;
    };// class getDeviceHandle

    TEST_F(getDeviceHandle, getDeviceHandle_001 )
    {
        ::osl::VolumeInfo   aVolumeInfo( VolumeInfoMask_Attributes );
        nError1 = ::osl::Directory::getVolumeInfo( aVolURL1, aVolumeInfo );
        ASSERT_TRUE( osl::FileBase::E_None == nError1 );

        ::osl::VolumeDevice aVolumeDevice1( aVolumeInfo.getDeviceHandle( ) );
        sal_Bool bOk = compareFileName( aNullURL, aVolumeDevice1.getMountPath( ) );

        ASSERT_TRUE(( sal_False == bOk )) << "test for getDeviceHandle function: get device handle of Fixed disk volume mounted on /, it should not be NULL, it did not pass in (W32) (UNX).";
    }

}// namespace osl_VolumeInfo



//------------------------------------------------------------------------
// Beginning of the test cases for VolumeDevice class
//------------------------------------------------------------------------
namespace osl_FileStatus
{

    //---------------------------------------------------------------------
    //  testing the method
    //  FileStatus( sal_uInt32 nMask ): _nMask( nMask )
    //---------------------------------------------------------------------
    class  FileStatusCtors : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aUStr;
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            ::std::auto_ptr<Directory> pDir( new Directory( aTmpName3 ) );
            nError1 = pDir->open( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = pDir->getNextItem( rItem, 0 );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            pDir->close();
            /*
            Directory aDir( aTmpName3 );
            nError1 = aDir.open();
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = aDir.getNextItem( rItem, 0 );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            aDir.close();
            */
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class ctors

    TEST_F(FileStatusCtors, ctors_001 )
    {
            ::osl::FileStatus   rFileStatus( FileStatusMask_All );
        nError1 = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        aUStr = rFileStatus.getFileName( );

        ASSERT_TRUE(sal_True == compareFileName( aUStr, aTmpName2)) << "test for ctors function: mask all and see the file name";
    }

    TEST_F(FileStatusCtors, ctors_002 )
    {
            ::osl::FileStatus   rFileStatus( 0 );
        nError1 = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        aUStr = rFileStatus.getFileName( );

        ASSERT_TRUE(sal_True == compareFileName( aUStr, aNullURL)) << "test for ctors function: mask is empty";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool isValid( sal_uInt32 nMask ) const
    //---------------------------------------------------------------------
    class  FileStatusIsValid : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aUStr;
        ::osl::Directory        *pDir;
        ::osl::DirectoryItem    rItem_file, rItem_link;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            pDir = new Directory( aTmpName3 );
            //::std::auto_ptr<Directory> pDir( new Directory( aTmpName3 ) );
                    ::osl::FileBase::RC nError1 = pDir->open( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = pDir->getNextItem( rItem_file, 1 );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
                    ::osl::FileBase::RC nError1 = pDir->close( );
                    delete pDir;
            ASSERT_TRUE(::osl::FileBase::E_None == nError1) << errorToStr(nError1).pData;

            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }

        void check_FileStatus(::osl::FileStatus const& _aStatus)
            {
                rtl::OString sStat;
                if (_aStatus.isValid(FileStatusMask_Type))
                {
                    sStat += "type ";
                }
                if (_aStatus.isValid(FileStatusMask_Attributes))
                {
                    sStat += "attributes ";
                }
                if (_aStatus.isValid(FileStatusMask_CreationTime))
                {
                    sStat += "ctime ";
                }
                if (_aStatus.isValid(FileStatusMask_AccessTime))
                {
                    sStat += "atime ";
                }
                if (_aStatus.isValid(FileStatusMask_ModifyTime))
                {
                    sStat += "mtime ";
                }
                if (_aStatus.isValid(FileStatusMask_FileSize))
                {
                    sStat += "filesize ";
                }
                if (_aStatus.isValid(FileStatusMask_FileName))
                {
                    sStat += "filename ";
                }
                if (_aStatus.isValid(FileStatusMask_FileURL))
                {
                    sStat += "fileurl ";
                }
                printf("mask: %s\n", sStat.getStr());
            }
    };// class ctors

    TEST_F(FileStatusIsValid, isValid_001 )
    {
        sal_uInt32 mask = 0;
        ::osl::FileStatus   rFileStatus( mask );
            ::osl::FileBase::RC nError1 = rItem_file.getFileStatus( rFileStatus );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        sal_Bool bOk = rFileStatus.isValid( mask );

        ASSERT_TRUE(( sal_True == bOk )) << "test for isValid function: no fields specified";
    }

    TEST_F(FileStatusIsValid, isValid_002 )
    {
        createTestFile( aTmpName6 );
        sal_uInt32 mask_file = ( FileStatusMask_Type | FileStatusMask_Attributes |
                               FileStatusMask_CreationTime | FileStatusMask_AccessTime |
                               FileStatusMask_ModifyTime   | FileStatusMask_FileSize   |
                               FileStatusMask_FileName     | FileStatusMask_FileURL) ;
            ::osl::FileStatus   rFileStatus( mask_file );
            ::osl::FileBase::RC nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem_file );
        nError1 = rItem_file.getFileStatus( rFileStatus );

        ASSERT_TRUE(::osl::FileBase::E_None == nError1) << errorToStr(nError1).pData;

// LLA: this is wrong, we never should try to check on all masks
//      only on one.
//      Second, it's not a bug, if a value is not valid, it's an unhandled feature.

//          sal_Bool bOk = rFileStatus.isValid( mask_file );

            check_FileStatus(rFileStatus);
        deleteTestFile( aTmpName6 );

            // ASSERT_TRUE(//                       ( sal_True == bOk )) << "test for isValid function: regular file mask fields test, #FileStatusMask_CreationTime# should be valid field for regular file, but feedback is invalid";
    }

    //Link is not defined in Windows, and on Linux, we can not get the directory item of the link file
    // LLA: we have to differ to filesystems, normal filesystems support links (EXT2, ...)
    //      castrated filesystems don't (FAT, FAT32)
    //      Windows NT NTFS support links, but the windows api don't :-(

    TEST_F(FileStatusIsValid, isValid_003 )
    {
#if defined ( UNX )
        // ::osl::FileBase::RC nError;
        sal_Int32 fd;

        ::rtl::OUString aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
        ( ( aUStr_LnkFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/tmpdir/link.file");
        ( ( aUStr_SrcFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/tmpdir/tmpname");

            rtl::OString strLinkFileName;
            rtl::OString strSrcFileName;
            strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
            strSrcFileName = OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

        //create a link file and link it to file "/tmp/PID/tmpdir/tmpname"
            fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
        ASSERT_TRUE( fd == 0 );

        // testDirectory is "/tmp/PID/tmpdir/"
        ::osl::Directory testDirectory( aTmpName3 );
            ::osl::FileBase::RC nError1 = testDirectory.open( );
        ::rtl::OUString aFileName = ::rtl::OUString::createFromAscii("link.file");
        sal_Bool bOk = sal_False;
        while (1) {
            nError1 = testDirectory.getNextItem( rItem_link, 4 );
            if (::osl::FileBase::E_None == nError1) {
                sal_uInt32 mask_link = FileStatusMask_FileName | FileStatusMask_LinkTargetURL;
                ::osl::FileStatus   rFileStatus( mask_link );
                rItem_link.getFileStatus( rFileStatus );
                //printFileName( rFileStatus.getFileName( ) );
                if ( compareFileName( rFileStatus.getFileName( ), aFileName) == sal_True )
                {
                    //printf("find the link file");
                    if ( sal_True == rFileStatus.isValid( FileStatusMask_LinkTargetURL ) )
                    {
                        bOk = sal_True;
                        break;
                    }
                }
            }
            else
                break;
        };

        fd = remove( strLinkFileName );
        ASSERT_TRUE( fd == 0 );

        ASSERT_TRUE(( sal_True == bOk )) << "test for isValid function: link file, check for LinkTargetURL";
#endif
    }

    TEST_F(FileStatusIsValid, isValid_004 )
    {
        sal_uInt32 mask_file_all = FileStatusMask_All;
            ::osl::FileStatus   rFileStatus_all( mask_file_all );
            ::osl::FileBase::RC nError1 = rItem_file.getFileStatus( rFileStatus_all );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

            check_FileStatus(rFileStatus_all);
// LLA: this is wrong
//          sal_Bool bOk1 = rFileStatus_all.isValid( mask_file_all );

        sal_uInt32 mask_file_val = FileStatusMask_Validate;
            ::osl::FileStatus   rFileStatus_val( mask_file_val );
        nError1 = rItem_file.getFileStatus( rFileStatus_val );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            // sal_Bool bOk2 = rFileStatus_val.isValid( mask_file_val );

            check_FileStatus(rFileStatus_val);
            // ASSERT_TRUE(//                       ( sal_False == bOk1 ) && ( sal_True == bOk2 )) << "test for isValid function: check for Mask_All and Validate, really not sure what validate used for and how to use it, help me.  did not pass (W32)(UNX).";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline Type getFileType() const
    //---------------------------------------------------------------------
    class  getFileType : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aUStr;
        ::osl::FileBase::RC     nError1, nError2;

        ::osl::DirectoryItem    m_aItem_1, m_aItem_2, m_aVolumeItem, m_aFifoItem;
        ::osl::DirectoryItem    m_aLinkItem, m_aSocketItem, m_aSpecialItem;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile: $TEMP/tmpdir/tmpname.
            //        a tempdirectory: $TEMP/tmpdir/tmpdir.
            //        use $ROOT/staroffice as volume ---> use dev/fd as volume.
            // and get their directory item.
            createTestDirectory( aTmpName3 );
            //printFileName( aTmpName2);
            createTestFile( aTmpName3, aTmpName2 );
            createTestDirectory( aTmpName3, aTmpName1 );

            ::std::auto_ptr<Directory> pDir( new Directory( aTmpName3 ) );
            nError1 = pDir->open( );
            ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "open aTmpName3 failed!";
            //getNextItem can not assure which item retrieved
                    nError1 = pDir->getNextItem( m_aItem_1, 1 );
            ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "get first item failed!";

                    nError1 = pDir->getNextItem( m_aItem_2 );
            ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "get second item failed!";
            pDir->close();
            //mindy: failed on my RH9,so removed temporaly
            //nError1 = ::osl::DirectoryItem::get( aVolURL2, m_aVolumeItem );
            //ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "get volume item failed!";

        }

        void TearDown( )
        {
            // remove all in $TEMP/tmpdir.
            deleteTestDirectory( aTmpName3, aTmpName1 );
            deleteTestFile( aTmpName3, aTmpName2 );
            deleteTestDirectory( aTmpName3 );
        }

        void check_FileType(osl::FileStatus const& _rFileStatus )
        {
            sal_Bool bOK = sal_False;
                if ( _rFileStatus.isValid(FileStatusMask_FileName))
                {
                    rtl::OUString suFilename = _rFileStatus.getFileName();

                    if ( _rFileStatus.isValid(FileStatusMask_Type))
                    {
                        osl::FileStatus::Type eType = _rFileStatus.getFileType( );

                        if ( compareFileName( suFilename, aTmpName2) == sal_True )
                        {
                            // regular
                            bOK = ( eType == osl::FileStatus::Regular );
                        }
                        if ( compareFileName( suFilename, aTmpName1) == sal_True )
                        {
                            // directory
                            bOK = ( eType == ::osl::FileStatus::Directory );
                        }

                        ASSERT_TRUE(( bOK == sal_True )) << "test for getFileType function: ";
        }
                }
                // LLA: it's not a bug, if a FileStatus not exist, so no else
            }
    };// class getFileType

    TEST_F(getFileType, getFileType_001 )
    {
        ::osl::FileStatus   rFileStatus( FileStatusMask_Type | FileStatusMask_FileName );
        nError1 = m_aItem_1.getFileStatus( rFileStatus );
        ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "getFileStatus failed";

        check_FileType(rFileStatus);
    }

    TEST_F(getFileType, getFileType_002 )
    {
            ::osl::FileStatus   rFileStatus( FileStatusMask_Type | FileStatusMask_FileName );
            nError1 = m_aItem_2.getFileStatus( rFileStatus );

        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            check_FileType(rFileStatus);
    }

    TEST_F(getFileType, getFileType_003 )
    {
#if 0
// LLA: this have to be discussed.
            ::osl::FileStatus   rFileStatus( FileStatusMask_Type );
            nError1 = m_aVolumeItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

            if (rFileStatus.isValid(FileStatusMask_Type))
            {
                osl::FileStatus::Type eType = rFileStatus.getFileType( );

        ASSERT_TRUE(( eType == ::osl::FileStatus::Volume )) << "test for getFileType function: Volume, it seems the volume part of the field is not implemented, it did not pass in (W32)(UNX).";
            }
#endif
    }


    TEST_F(getFileType, getFileType_004 )
    {
#if ( defined UNX ) || ( defined OS2 )  //Fifo creation is differ in Windows

        //create a fifo in $ROOT/tmp/tmpdir, get its DirectoryItem.
      rtl::OString strFifoSys;
      strFifoSys = OUStringToOString( aFifoSys, RTL_TEXTENCODING_ASCII_US );
      ::rtl::OUString aFifoURL;

              int fd = mkfifo( strFifoSys.getStr(), O_RDWR | O_CREAT );
      ASSERT_TRUE(fd == 0) << "mkfifo failed!";
        ::osl::FileBase::getFileURLFromSystemPath( aFifoSys, aFifoURL );

            nError1 = ::osl::DirectoryItem::get( aFifoURL, m_aFifoItem );
        ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "get item failed!";

        //check for File type
            ::osl::FileStatus   rFileStatus( FileStatusMask_Type );
            nError1 = m_aFifoItem.getFileStatus( rFileStatus );
        ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "get Status failed!";

        //delete fifo
        nError1 = ::osl::File::remove( aFifoURL );
        ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "remove file failed!";

            if (rFileStatus.isValid(FileStatusMask_Type))
            {
                osl::FileStatus::Type eType = rFileStatus.getFileType( );

        ASSERT_TRUE(( eType == ::osl::FileStatus::Fifo )) << "test for getFileType function: Fifo, Solaris version ";
    }
#endif
    }

/*
 * LLA: removed, m_aSocketItem is wrong initialised.
 */

// LLA:         TEST_F(getFileType, getFileType_005 )
// LLA:         {
// LLA: #if defined ( SOLARIS ) //Socket file may differ in Windows
// LLA:             // nError1 = ::osl::DirectoryItem::get( aTypeURL1, m_aSocketItem );
// LLA:             nError1 = ::osl::DirectoryItem::get( rtl::OUString::createFromAscii("/dev/null"), m_aSocketItem );
// LLA:             printError(nError1);
// LLA:             ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "get Socket type file failed";
// LLA:
// LLA:             //check for File type
// LLA:             ::osl::FileStatus   rFileStatus( FileStatusMask_Type );
// LLA:
// LLA:             nError1 = m_aSocketItem.getFileStatus( rFileStatus );
// LLA:             ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "getFileStatus failed";
// LLA:
// LLA:             if (rFileStatus.isValid( FileStatusMask_Type ))
// LLA:             {
// LLA:                 osl::FileStatus::Type eType = rFileStatus.getFileType( );
// LLA:                 printFileType(eType);
// LLA:                 ASSERT_TRUE(// LLA:                                         ( eType == ::osl::FileStatus::Socket )) << "test for getFileType function: Socket, Solaris version ";
// LLA:             }
// LLA: #endif
// LLA:         }


// deprecated since there is a same case Directory::getNextItem_004
/*#if defined 0 //( UNX ) //( SOLARIS ) //Link file is not defined in Windows
    TEST_F(getFileType, getFileType_006 )
    {
  nError1 = ::osl::DirectoryItem::get( aTypeURL3, m_aLinkItem );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //check for File type
            ::osl::FileStatus   rFileStatus( FileStatusMask_Type );
  nError1 = m_aLinkItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(( ::osl::FileStatus::Link == rFileStatus.getFileType( ) )) << "test for getFileType function: Link, UNX version ";
    }
#endif  */

    TEST_F(getFileType, getFileType_007 )
    {
#if defined ( SOLARIS ) //Special file is differ in Windows
            nError1 = ::osl::DirectoryItem::get( aTypeURL2, m_aSpecialItem );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //check for File type
            ::osl::FileStatus   rFileStatus( FileStatusMask_Type );
        nError1 = m_aSpecialItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        if (rFileStatus.isValid(FileStatusMask_Type))
        {
            osl::FileStatus::Type eType = rFileStatus.getFileType( );


            ASSERT_TRUE(( eType == ::osl::FileStatus::Special )) << "test for getFileType function: Special, Solaris version ";
        }
#endif
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_uInt64 getAttributes() const
    //---------------------------------------------------------------------
    class  getAttributes : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aTypeURL, aTypeURL_Hid;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem, rItem_hidden;

        public:
        // initialization
        void SetUp( )
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            ASSERT_TRUE( nError == FileBase::E_None );

            aTypeURL_Hid = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL_Hid, aHidURL1 );
            createTestFile( aTypeURL_Hid );
            nError = ::osl::DirectoryItem::get( aTypeURL_Hid, rItem_hidden );
            ASSERT_TRUE( nError == FileBase::E_None );
        }

        void TearDown( )
        {
            deleteTestFile( aTypeURL );
            deleteTestFile( aTypeURL_Hid );
        }
    };// class getAttributes

#if ( defined UNX ) || ( defined OS2 )
//windows only 3 file attributes: normal, readonly, hidden
    TEST_F(getAttributes, getAttributes_001 )
    {
        changeFileMode( aTypeURL, S_IRUSR | S_IRGRP | S_IROTH );

            ::osl::FileStatus   rFileStatus( FileStatusMask_Attributes );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );

        ASSERT_TRUE(( Attribute_ReadOnly | Attribute_GrpRead | Attribute_OwnRead | Attribute_OthRead ) ==
                                rFileStatus.getAttributes( )) << "test for getAttributes function: ReadOnly, GrpRead, OwnRead, OthRead( UNX version ) ";
    }
#else                                    //Windows version
    TEST_F(getAttributes, getAttributes_001 )
    {
        ASSERT_TRUE(1 == 1) << "test for getAttributes function: ReadOnly, GrpRead, OwnRead, OthRead( Windows version )";
    }
#endif


    TEST_F(getAttributes, getAttributes_002 )
    {
#if ( defined UNX ) || ( defined OS2 )
        changeFileMode( aTypeURL, S_IXUSR | S_IXGRP | S_IXOTH );

            ::osl::FileStatus   rFileStatus( FileStatusMask_Attributes );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );

        ASSERT_TRUE(( Attribute_ReadOnly | Attribute_Executable | Attribute_GrpExe | Attribute_OwnExe | Attribute_OthExe ) ==
                                rFileStatus.getAttributes( )) << "test for getAttributes function: Executable, GrpExe, OwnExe, OthExe, the result is Readonly, Executable, GrpExe, OwnExe, OthExe, it partly not pass( Solaris version )";
#endif
    }


#if ( defined UNX ) || ( defined OS2 )
    TEST_F(getAttributes, getAttributes_003 )
    {
        changeFileMode( aTypeURL, S_IWUSR | S_IWGRP | S_IWOTH );

            ::osl::FileStatus   rFileStatus( FileStatusMask_Attributes );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );

        ASSERT_TRUE(( Attribute_GrpWrite | Attribute_OwnWrite | Attribute_OthWrite ) ==
                                rFileStatus.getAttributes( )) << "test for getAttributes function: GrpWrite, OwnWrite, OthWrite( Solaris version )";
    }
#else                                    //Windows version
    TEST_F(getAttributes, getAttributes_003 )
    {
        ASSERT_TRUE(1 == 1) << "test for getAttributes function: GrpWrite, OwnWrite, OthWrite( Windows version )";
    }
#endif

#if ( defined UNX ) || ( defined OS2 )  //hidden file definition may different in Windows
    TEST_F(getAttributes, getAttributes_004 )
    {
        sal_Int32 test_Attributes = Attribute_Hidden;
            ::osl::FileStatus   rFileStatus( FileStatusMask_Attributes );
        nError = rItem_hidden.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );
        test_Attributes &= rFileStatus.getAttributes( );

        ASSERT_TRUE(test_Attributes == Attribute_Hidden) << "test for getAttributes function: Hidden files( Solaris version )";
    }
#else                                    //Windows version
    TEST_F(getAttributes, getAttributes_004 )
    {
        ::rtl::OUString aUserHiddenFileURL = ::rtl::OUString::createFromAscii("file:///c:/AUTOEXEC.BAT");
        nError = ::osl::DirectoryItem::get( aUserHiddenFileURL, rItem_hidden );
        //printFileName( aUserHiddenFileURL );
        ASSERT_TRUE(nError == FileBase::E_None) << "get item fail";
            ::osl::FileStatus   rFileStatus( FileStatusMask_Attributes );
        nError = rItem_hidden.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );

        ASSERT_TRUE((rFileStatus.getAttributes( ) & Attribute_Hidden)!= 0) << "Hidden files(Windows version), please check if hidden file c:/AUTOEXEC.BAT exists ";
    }
#endif

    //---------------------------------------------------------------------
    //  testing the method
    //  inline TimeValue getAccessTime() const
    //---------------------------------------------------------------------
    class  getAccessTime : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            ASSERT_TRUE( nError == FileBase::E_None );

        }

        void TearDown( )
        {
            deleteTestFile( aTypeURL );
        }
    };// class getAccessTime

    TEST_F(getAccessTime, getAccessTime_001)
    {
        TimeValue *pTV_current = NULL;
        ASSERT_TRUE( ( pTV_current = ( TimeValue* )malloc( sizeof( TimeValue ) ) ) != NULL );
        TimeValue *pTV_access = NULL;
        ASSERT_TRUE( ( pTV_access = ( TimeValue* )malloc( sizeof( TimeValue ) ) ) != NULL );

        ::osl::FileStatus   rFileStatus( FileStatusMask_AccessTime );
        nError = rItem.getFileStatus( rFileStatus );
        sal_Bool bOk = osl_getSystemTime( pTV_current );
        ASSERT_TRUE( sal_True == bOk && nError == FileBase::E_None );

        *pTV_access = rFileStatus.getAccessTime( );

        sal_Bool bOK = t_compareTime( pTV_access, pTV_current, delta );
        free( pTV_current );
        free( pTV_access );

        ASSERT_TRUE(sal_True == bOK) << "test for getAccessTime function: This test turns out that UNX pricision is no more than 1 sec, don't know how to test this function, in Windows test, it lost hour min sec, only have date time. ";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline TimeValue getModifyTime() const
    //---------------------------------------------------------------------
    class  getModifyTime : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;
    };// class getModifyTime

    TEST_F(getModifyTime, getModifyTime_001)
    {
        TimeValue *pTV_current = NULL;
        ASSERT_TRUE( ( pTV_current = ( TimeValue* )malloc( sizeof( TimeValue ) ) ) != NULL );

        //create file
        aTypeURL = aUserDirectoryURL.copy( 0 );
        concatURL( aTypeURL, aTmpName2 );
        createTestFile( aTypeURL );

        //get current time
        sal_Bool bOk = osl_getSystemTime( pTV_current );
        ASSERT_TRUE( sal_True == bOk );

        //get instance item and filestatus
        nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
        ASSERT_TRUE( nError == FileBase::E_None );
        ::osl::FileStatus   rFileStatus( FileStatusMask_ModifyTime );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );

        //get modify time
        TimeValue *pTV_modify = NULL;
        ASSERT_TRUE( ( pTV_modify = ( TimeValue* )malloc( sizeof( TimeValue ) ) ) != NULL );
        *pTV_modify = rFileStatus.getModifyTime( );

        sal_Bool bOK = t_compareTime( pTV_modify, pTV_current, delta );
        //delete file
        deleteTestFile( aTypeURL );
        free( pTV_current );

        ASSERT_TRUE(sal_True == bOK) << "test for getModifyTime function: This test turns out that UNX pricision is no more than 1 sec, don't know how to improve this function.  ";
    }


    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_uInt64 getFileSize() const
    //---------------------------------------------------------------------
    class  getFileSize : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            ASSERT_TRUE( nError == FileBase::E_None );
        }

        void TearDown( )
        {
            deleteTestFile( aTypeURL );
        }
    };// class getFileSize

    TEST_F(getFileSize, getFileSize_001 )
    {
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileSize );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );

        sal_uInt64 uFileSize = rFileStatus.getFileSize( );

        ASSERT_TRUE(0 == uFileSize) << "test for getFileSize function: empty file ";
    }

    TEST_F(getFileSize, getFileSize_002 )
    {
        ::osl::File testfile( aTypeURL );
        nError = testfile.open( OpenFlag_Write | OpenFlag_Read );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError );
        nError = testfile.setSize( TEST_FILE_SIZE );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError );

        nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
        ASSERT_TRUE( nError == FileBase::E_None );
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileSize );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );
        sal_uInt64 uFileSize = rFileStatus.getFileSize( );

        ASSERT_TRUE(TEST_FILE_SIZE == uFileSize) << "test for getFileSize function: file with size of TEST_FILE_SIZE, did not pass in (W32). ";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline ::rtl::OUString getFileName() const
    //---------------------------------------------------------------------
    class  getFileName : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
            nError = ::osl::DirectoryItem::get( aTypeURL, rItem );
            ASSERT_TRUE( nError == FileBase::E_None );
        }

        void TearDown( )
        {
            deleteTestFile( aTypeURL );
        }
    };// class getFileName

    TEST_F(getFileName, getFileName_001 )
    {
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );

        ::rtl::OUString aFileName = rFileStatus.getFileName( );

        ASSERT_TRUE(sal_True == compareFileName( aFileName, aTmpName2 )) << "test for getFileName function: name compare with specify";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline ::rtl::OUString getFileURL() const
    //---------------------------------------------------------------------
    class  getFileURL : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            createTestFile( aTmpName6 );
            nError = ::osl::DirectoryItem::get( aTmpName6, rItem );
            ASSERT_TRUE( nError == FileBase::E_None );
        }

        void TearDown( )
        {
            deleteTestFile( aTmpName6 );
        }
    };// class getFileURL

    TEST_F(getFileURL, getFileURL_001 )
    {
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileURL );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError == FileBase::E_None );

        ::rtl::OUString aFileURL = rFileStatus.getFileURL( );

        ASSERT_TRUE(sal_True == compareFileName( aFileURL, aTmpName6 )) << "test for getFileURL function: ";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline ::rtl::OUString getLinkTargetURL() const
    //---------------------------------------------------------------------
    class  getLinkTargetURL : public ::testing::Test
    {
    protected:
        ::rtl::OUString         aTypeURL;
        ::osl::FileBase::RC     nError;
        ::osl::DirectoryItem    rItem;

        public:
        // test code.
        // initialization
        void SetUp( )
        {
            aTypeURL = aUserDirectoryURL.copy( 0 );
            concatURL( aTypeURL, aTmpName2 );
            createTestFile( aTypeURL );
        }

        void TearDown( )
        {
            deleteTestFile( aTypeURL );
        }
    };// class getLinkTargetURL

#if ( defined UNX ) || ( defined OS2 )         //Link file is not define in Windows
    TEST_F(getLinkTargetURL, getLinkTargetURL_001 )
    {
        //create a link file;
        ::rtl::OUString aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
        ( ( aUStr_LnkFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/link.file");
        ( ( aUStr_SrcFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/tmpname");

            rtl::OString strLinkFileName, strSrcFileName;
            strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
            strSrcFileName  = OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

        sal_Int32 fd;
            fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
        ASSERT_TRUE(fd == 0) << "in creating link file";

        //get linkTarget URL
        nError = ::osl::DirectoryItem::get( aLnkURL1, rItem );
        ASSERT_TRUE(nError == FileBase::E_None) << "in getting link file item";

            ::osl::FileStatus   rFileStatus( FileStatusMask_LinkTargetURL );
        nError = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE(nError == FileBase::E_None) << "in getting link file status";
        ::rtl::OUString aFileURL = rFileStatus.getLinkTargetURL( );

        //remove link file
            fd = remove( strLinkFileName.getStr() );
        ASSERT_TRUE(fd == 0) << "in deleting link file";

        ASSERT_TRUE(sal_True == compareFileName( aFileURL, aTypeURL )) << "test for getLinkTargetURL function: Solaris version, creat a file, and a link file link to it, get its LinkTargetURL and compare";
    }
#else
    TEST_F(getLinkTargetURL, getLinkTargetURL_001 )
    {
        ASSERT_TRUE(1) << "test for getLinkTargetURL function: Windows version, not tested";
    }
#endif

}// namespace osl_FileStatus



//------------------------------------------------------------------------
// Beginning of the test cases for File class
//------------------------------------------------------------------------
namespace osl_File
{
    //---------------------------------------------------------------------
    //  testing the method
    //  File( const ::rtl::OUString& ustrFileURL )
    //---------------------------------------------------------------------
    class  FileCtors : public ::testing::Test
    {
    protected:
        // ::osl::FileBase::RC     nError1;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class ctors

    TEST_F(FileCtors, ctors_001 )
    {
        ::osl::File testFile( aTmpName4 );

        ::osl::FileBase::RC nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ::osl::FileBase::RC nError2 = testFile.close( );
        ASSERT_TRUE(( ::osl::FileBase::E_None == nError1 ) && ( ::osl::FileBase::E_None == nError2 )) << "test for ctors function: initialize a File and test its open and close";
    }

    TEST_F(FileCtors, ctors_002 )
    {
        ::osl::File testFile( aTmpName5 );
        sal_Char buffer[30] = "Test for File constructor";
        sal_uInt64 nCount;

            ::osl::FileBase::RC nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
            ::osl::FileBase::RC nError2 = testFile.write( buffer, 30, nCount );
        testFile.close( );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError1 ) && ( ::osl::FileBase::E_None == nError2 )) << "test for ctors function: test relative file URL, this test show that relative file URL is also acceptable";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC open( sal_uInt32 uFlags )
    //---------------------------------------------------------------------
    class  FileOpen : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2, nError3;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class open

    // test code.
    TEST_F(FileOpen, open_001 )
    {
        ::osl::File testFile( aTmpName4 );

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        nError2 = testFile.close( );
        ASSERT_TRUE(::osl::FileBase::E_None == nError2) << "close error";

        ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "test for open function: open a regular file";
    }

    TEST_F(FileOpen, open_002 )
    {
        ::osl::File testFile( aTmpName3 );

        nError1 = testFile.open( OpenFlag_Read );

        ASSERT_TRUE(( File::E_INVAL == nError1 ) || ( File::E_ACCES == nError1 )) << "test for open function: open a directory";
    }

    TEST_F(FileOpen, open_003 )
    {
        ::osl::File testFile( aCanURL1 );

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );

        ASSERT_TRUE(File::E_NOENT == nError1) << "test for open function: open a non-exist file";
    }

    TEST_F(FileOpen, open_004 )
    {
        ::rtl::OUString  aTestFile( aRootURL );
        concatURL( aTestFile, aTmpName2 );
        ::osl::File testFile( aTestFile );

        nError1 = testFile.open( OpenFlag_Create );
        sal_Bool bOK = ( File::E_ACCES == nError1 );
#if defined (WNT )
        bOK = sal_True;  /// in Windows, you can create file in c:/ any way.
        testFile.close( );
        deleteTestFile( aTestFile);
#endif

        ASSERT_TRUE(bOK == sal_True) << "test for open function: create an illegal file";
    }

    TEST_F(FileOpen, open_005 )
    {
        ::osl::File testFile( aTmpName4 );

        nError1 = testFile.open( OpenFlag_Create );

        ASSERT_TRUE(File::E_EXIST == nError1) << "test for open function: create an exist file";
    }

    TEST_F(FileOpen, open_006 )
    {
        ::osl::File testFile( aCanURL1 );
        sal_Char buffer_write[30] = "Test for File open";
        sal_Char buffer_read[30];
        sal_uInt64 nCount_write, nCount_read;

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write | OpenFlag_Create );
        nError2 = testFile.write( buffer_write, 30, nCount_write );
            ::osl::FileBase::RC nError4 = testFile.setPos( Pos_Absolut, 0 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError4 );
        nError3 = testFile.read( buffer_read, 10, nCount_read );

            ::osl::FileBase::RC nError5 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError5 );
        ::osl::FileBase::RC nError6 = testFile.remove( aCanURL1 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError6 );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError1 ) &&
                                ( ::osl::FileBase::E_None == nError2 ) &&
                                ( ::osl::FileBase::E_None == nError3 ) &&
                                ( 30 == nCount_write ) &&
                                ( 10 == nCount_read )) << "test for open function: test for OpenFlag_Read,OpenFlag_Write and OpenFlag_Create";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC close()
    //---------------------------------------------------------------------
    class  FileClose : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2, nError3;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class close

    TEST_F(FileClose, close_001 )
    {
        ::osl::File testFile( aTmpName4 );

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError2 = testFile.close( );

        ASSERT_TRUE(::osl::FileBase::E_None == nError2) << "test for close function: close a regular file";
    }

    TEST_F(FileClose, close_002 )
    {
        ::osl::File testFile( aTmpName4 );

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError2 = testFile.close( );

            nError3 = testFile.setPos( Pos_Absolut, 0 );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError2 ) &&
                                ( ::osl::FileBase::E_None != nError3 )) << "test for close function: manipulate a file after it has been closed";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC setPos( sal_uInt32 uHow, sal_Int64 uPos )
    //---------------------------------------------------------------------
    class  setPos : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class setPos

    TEST_F(setPos, setPos_001 )
    {
        ::osl::File testFile( aTmpName4 );
        sal_Char buffer_read[2];

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.setPos( Pos_Absolut, 26 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.read( buffer_read, 1, nCount_read );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(buffer_read[0] == pBuffer_Char[26]) << "test for setPos function: test for Pos_Absolut, set the position to 26, test if the 26th char in file is correct";
    }

    TEST_F(setPos, setPos_002 )
    {
        ::osl::File testFile( aTmpName4 );
        sal_Char buffer_read[2];

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.setPos( Pos_Absolut, sizeof( pBuffer_Char ) - 2 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.setPos( Pos_Current, 0);
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.read( buffer_read, 1, nCount_read );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(buffer_read[0] == pBuffer_Char[sizeof( pBuffer_Char ) - 2]) << "test for setPos function: test for Pos_Current, set the position to end, test if the ( end -1 ) char in file is correct";
    }

    TEST_F(setPos, setPos_003 )
    {
        ::osl::File testFile( aTmpName4 );
        sal_Char buffer_read[2];

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //the file size is smaller than 100
        nError1 = testFile.setPos( Pos_End,  -100 );
        ASSERT_TRUE(::osl::FileBase::E_INVAL == nError1) << "should return error";

            nError1 = testFile.setPos( Pos_End, -53 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.read( buffer_read, 1, nCount_read );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(buffer_read[0] == pBuffer_Char[0]) << "test for setPos function: test for Pos_End, set the position to end, test if the first char in file is correct";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC getPos( sal_uInt64& uPos )
    //---------------------------------------------------------------------
    class  getPos : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class getPos

    TEST_F(getPos, getPos_001 )
    {
        ::osl::File testFile( aTmpName4 );
        sal_uInt64 nFilePointer;

        nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_INVAL == nError1 );

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

            nError1 = testFile.setPos( Pos_Absolut, 26 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(26 == nFilePointer) << "test for getPos function: set the position to 26, get position and check if it is right";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC isEndOfFile( sal_Bool *pIsEOF )
    //---------------------------------------------------------------------
    class  isEndOfFile : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class isEndOfFile

    TEST_F(isEndOfFile, isEndOfFile_001 )
    {
        ::osl::File   testFile( aTmpName4 );
        sal_Bool      bEOF  = sal_False;
        sal_Bool      *pEOF = &bEOF;


        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

            nError1 = testFile.setPos( Pos_End, 0 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.isEndOfFile( pEOF );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(sal_True == *pEOF) << "test for isEndOfFile function: set the position to end, check if reach end";
    }

    TEST_F(isEndOfFile, isEndOfFile_002 )
    {
            ::osl::File   testFile( aTmpName4 );
        sal_Bool      bEOF  = sal_False;
        sal_Bool      *pEOF = &bEOF;
        sal_uInt64    nFilePointer = 0;

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

            nError1 = testFile.setPos( Pos_Absolut, 0 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        *pEOF = sal_False;
            while ( !( *pEOF ) )
        {
            nError1 = testFile.isEndOfFile( pEOF );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.setPos( Pos_Current, 1 );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }
            nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(sizeof( pBuffer_Char ) + 1 == nFilePointer) << "test for isEndOfFile function: use isEndOfFile to move pointer step by step";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC setSize( sal_uInt64 uSize )
    //---------------------------------------------------------------------
    class  setSize : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class setSize

    TEST_F(setSize, setSize_001 )
    {
        ::osl::File   testFile( aTmpName4 );
        // sal_Bool      bEOF  = sal_False;
        // sal_Bool      *pEOF = &bEOF;
        sal_uInt64     nFilePointer;


        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //enlarge the file to size of 100;
            nError1 = testFile.setSize( 100 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //get the file size;
            nError1 = testFile.setPos( Pos_End, 0 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(100 == nFilePointer) << "test for setSize function: enlarge the file ";
    }

    TEST_F(setSize, setSize_002 )
    {
        ::osl::File   testFile( aTmpName4 );
        // sal_Bool      bEOF  = sal_False;
        // sal_Bool      *pEOF = &bEOF;
        sal_uInt64     nFilePointer;


        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //enlarge the file to size of 100;
            nError1 = testFile.setSize( 10 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //get the file size;
            nError1 = testFile.setPos( Pos_End, 0 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(10 == nFilePointer) << "test for setSize function: truncate the file ";
    }
  /*            TEST_F(setSize, setSize_003 )
    {
        ::osl::File   testFile( aTmpName4 );
        // sal_Bool      bEOF  = sal_False;
        // sal_Bool      *pEOF = &bEOF;
        sal_uInt64     nFilePointer;


        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //enlarge the file to size of 100;
            nError1 = testFile.setSize( 10 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //get the file size;
            nError1 = testFile.setPos( Pos_End, 0 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(10 == nFilePointer) << "test for setSize function: truncate the file ";
    }
  */

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead )
    //---------------------------------------------------------------------
    class  read : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class read

    TEST_F(read, read_001 )
    {
        ::osl::File    testFile( aTmpName4 );
        sal_uInt64     nFilePointer;
        sal_Char       buffer_read[10];


        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.read( buffer_read, 10, nCount_read );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(( 10 == nFilePointer ) && ( 0 == strncmp( buffer_read, pBuffer_Char, 10 ) )) << "test for read function: read whole content in the file to a buffer";
    }

    TEST_F(read, read_002 )
    {
        ::osl::File    testFile( aTmpName4 );
        sal_uInt64     nFilePointer;
        sal_Char       buffer_read[26];


        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

            nError1 = testFile.setPos( Pos_Absolut, 26 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.read( buffer_read, 26, nCount_read );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(( 52 == nFilePointer ) && ( 26 == nCount_read ) && ( 0 == strncmp( buffer_read, &pBuffer_Char[26], 26 ) )) << "test for read function: read from a special position in the file";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten)
    //---------------------------------------------------------------------
    class  write : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class write

    TEST_F(write, write_001 )
    {
        ::osl::File    testFile( aTmpName6 );
        sal_uInt64     nFilePointer;
        sal_Char       buffer_read[10];

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //write chars into the file.
        nError1 = testFile.write( pBuffer_Char, 10, nCount_write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //get the current pointer;
            nError1 = testFile.getPos( nFilePointer );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //reset pointer to the beginning;
            nError1 = testFile.setPos( Pos_Absolut, 0 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.read( buffer_read, 10, nCount_read );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        nError1 = testFile.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(( 10 == nFilePointer ) &&
                                ( 0 == strncmp( buffer_read, pBuffer_Char, 10 ) ) &&
                                ( 10 == nCount_write )) << "test for write function: read whole content in the file to a buffer. Note, buffer size can not smaller than the read size";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC readLine( ::rtl::ByteSequence& aSeq )
    //---------------------------------------------------------------------
    class  readLine : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;
        ::rtl::ByteSequence      aSequence;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );

            //write some strings into the file.
            ::osl::File testFile( aTmpName6 );
            sal_Char ppStrSeq[3][27]  =  { "abcde\n",
                                        "1234567890\n",
                                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                      };

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

            for ( int nCount = 0; nCount < 3; nCount++ )
            {
                nError1 = testFile.write( ppStrSeq[nCount], strlen( ppStrSeq[nCount] ), nCount_write );
                ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            }

             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class readLine

    TEST_F(readLine, readLine_001 )
    {
            ::osl::File    testFile( aTmpName6 );

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        nError1 = testFile.readLine( aSequence );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError1 ) &&
                                ( 0 == strncmp( ( const char * )aSequence.getArray( ), pBuffer_Char, 5 ) )) << "test for readLine function: read the first line of the file.";
    }

    TEST_F(readLine, readLine_002 )
    {
            ::osl::File    testFile( aTmpName6 );
        sal_Bool bEOF  = sal_False;
        sal_Bool *pEOF = &bEOF;

        nError1 = testFile.open( OpenFlag_Read | OpenFlag_Write );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        for ( int nCount = 0; nCount < 3; nCount++ )
        {
            nError1 = testFile.readLine( aSequence );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }
            nError1 = testFile.isEndOfFile( pEOF );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(*pEOF &&
                                ( 0 == strncmp( ( const char * )aSequence.getArray( ), &pBuffer_Char[26], 26 ) )) << "test for readLine function: read three lines of the file and check the file pointer moving.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC copy( const ::rtl::OUString& ustrSourceFileURL, const ::rtl::OUString& ustrDestFileURL )
    //---------------------------------------------------------------------
    class  copy : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class copy

    TEST_F(copy, copy_001 )
    {
            ::osl::File    testFile( aTmpName6 );

        //copy $TEMP/tmpdir/tmpname to $TEMP/tmpname.
        nError1 = ::osl::File::copy( aTmpName4, aTmpName6 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //check
        nError1 = testFile.open( OpenFlag_Create );
        deleteTestFile( aTmpName6 );

        ASSERT_TRUE(::osl::FileBase::E_EXIST == nError1) << "test for copy function: copy file to upper directory";
    }

    TEST_F(copy, copy_002 )
    {
        //copy $TEMP/tmpdir/tmpname to $TEMP/tmpdir.
        nError1 = ::osl::File::copy( aTmpName4, aTmpName3 );

        ASSERT_TRUE(( ::osl::FileBase::E_ISDIR == nError1 ) ||( ::osl::FileBase::E_ACCES == nError1 )) << "test for copy function: use directory as destination";
    }

    TEST_F(copy, copy_003 )
    {
        //copy $TEMP/tmpdir/tmpname to $ROOT/tmpname.
        nError1 = ::osl::File::copy( aTmpName4, aTmpName7 );
#if defined (WNT )
        nError1 = ::osl::FileBase::E_ACCES;  /// for Windows, c:/ is writtenable any way.
        deleteTestFile( aTmpName7);
#endif
        ASSERT_TRUE(::osl::FileBase::E_ACCES == nError1) << "test for copy function: copy to an illigal place";
    }

    TEST_F(copy, copy_004 )
    {
        //copy $TEMP/tmpname to $TEMP/tmpdir/tmpname.
        nError1 = ::osl::File::copy( aTmpName6, aTmpName4 );

        ASSERT_TRUE(::osl::FileBase::E_NOENT == nError1) << "test for copy function: copy a not exist file";
    }

    TEST_F(copy, copy_005 )
    {
        //copy $TEMP/tmpname to $TEMP/system.path using system path.
        nError1 = ::osl::File::copy( aTmpName6, aSysPath1 );

        ASSERT_TRUE(::osl::FileBase::E_INVAL == nError1) << "test for copy function: copy a file using system file path";
    }
  TEST_F(copy, copy_006 )
  {
    createTestFile( aTmpName6 );
    File tmpFile( aTmpName6 );
    FileBase::RC err = tmpFile.open( OpenFlag_Write | OpenFlag_Read );
    (void)err;
    tmpFile.setSize( 200 );
    tmpFile.close();
    //copy to new path
    nError1 = ::osl::File::copy( aTmpName6, aTmpName4 );
    ASSERT_TRUE( nError1 == FileBase::E_None );

    //check if is the new file
    File newFile( aTmpName4 );
    newFile.open( OpenFlag_Write | OpenFlag_Read );
    newFile.setPos( Pos_End, 0 );
    //      ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
    sal_uInt64     nFilePointer;
    nError1 = newFile.getPos( nFilePointer );
    ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
    newFile.close( );
    deleteTestFile( aTmpName6 );
    ASSERT_TRUE(nFilePointer == 200) << "test for copy function: the dest file exist";
  }
  //copyLink has not been impletmented yet
  TEST_F(copy, copy_007 )
  {
#if ( defined UNX )

    ASSERT_TRUE(::osl::FileBase::E_INVAL == nError1) << "test for copy function: source file is link file";
#endif
  }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC move( const ::rtl::OUString& ustrSourceFileURL, const ::rtl::OUString& ustrDestFileURL )
    //---------------------------------------------------------------------
    class  move : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1, nError2;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class move

    TEST_F(move, move_001 )
    {
        //rename $TEMP/tmpdir/tmpname to $TEMP/canonical.name.
        nError1 = ::osl::File::move( aTmpName4, aCanURL1 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //check
            ::osl::File    testFile( aCanURL1 );
        nError2 = testFile.open( OpenFlag_Create );
        deleteTestFile( aCanURL1 );

        ASSERT_TRUE(::osl::FileBase::E_EXIST == nError2) << "test for move function: rename file to another directory";
    }

    TEST_F(move, move_002 )
    {
        //move $TEMP/tmpdir/tmpname to $TEMP/tmpdir.
        nError1 = ::osl::File::move( aTmpName4, aTmpName3 );
        //returned ::osl::FileBase::E_ACCES on WNT
        ASSERT_TRUE(( ::osl::FileBase::E_ACCES == nError1 || ::osl::FileBase::E_ISDIR == nError1 ) ||( ::osl::FileBase::E_EXIST == nError1 )) << "test for move function: use directory as destination";
    }

    TEST_F(move, move_003 )
    {
        //move $TEMP/tmpdir/tmpname to $ROOT/tmpname.
        nError1 = ::osl::File::move( aTmpName4, aTmpName7 );
#if defined (WNT )
        nError1 = ::osl::FileBase::E_ACCES;  /// for Windows, c:/ is writtenable any way.
        deleteTestFile( aTmpName7);
#endif

        ASSERT_TRUE(::osl::FileBase::E_ACCES == nError1) << "test for move function: move to an illigal place";
    }

    TEST_F(move, move_004 )
    {
        //move $TEMP/tmpname to $TEMP/tmpdir/tmpname.
        nError1 = ::osl::File::move( aTmpName6, aTmpName4 );

        ASSERT_TRUE(::osl::FileBase::E_NOENT == nError1) << "test for move function: move a not exist file";
    }

    TEST_F(move, move_005 )
    {
        //move $TEMP/tmpname to $TEMP/system.path using system path.
        nError1 = ::osl::File::move( aTmpName6, aSysPath1 );

        ASSERT_TRUE(::osl::FileBase::E_INVAL == nError1) << "test for move function: move a file using system file";
    }

    TEST_F(move, move_006 )
    {
        //move directory $TEMP/tmpname to $TEMP/tmpdir/tmpname.
        createTestDirectory( aTmpName6 );
        nError1 = ::osl::File::move( aTmpName6, aTmpName4 );
        //move file $TEMP/tmpdir/tmpname to $TEMP/tmpname
        nError2 = ::osl::File::move( aTmpName4, aTmpName6 );
        deleteTestDirectory( aTmpName6 );
#if defined ( WNT )
        deleteTestDirectory( aTmpName4 );// in Windows, it can be moved!!!!! this is only for not influence the following test.
        deleteTestFile( aTmpName6 );
        nError1 = ::osl::FileBase::E_NOTDIR;
        nError2 = ::osl::FileBase::E_ISDIR;
#endif
        ASSERT_TRUE(::osl::FileBase::E_NOTDIR == nError1 && ::osl::FileBase::E_ISDIR == nError2) << "test for move function: move a directory to an exist file with same name, did not pass in (W32)";
    }

    TEST_F(move, move_007 )
    {
        //create directory $TEMP/tmpname.
        createTestDirectory( aTmpName6 );
        //move directory $TEMP/tmpdir to $TEMP/tmpname/tmpdir
        nError1 = ::osl::File::move( aTmpName3, aTmpName8 );
        //check
        nError2 = ::osl::Directory::create( aTmpName8 );
        ::osl::File::move( aTmpName8, aTmpName3 );
        deleteTestDirectory( aTmpName6 );

        ASSERT_TRUE((::osl::FileBase::E_None == nError1 ) &&
                                (::osl::FileBase::E_EXIST == nError2 )) << "test for move function: move a directory to an exist file with same name";
    }
  // oldpath and newpath are not on the same filesystem.EXDEV,no such error no on Solaris, only on linux
  TEST_F(move, move_008 )
  {
#if 0
    ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "oldpath and newpath are not on the same filesystem, should error returns";
#endif
  }
  //bugid# 115420, after the bug fix, add the case
  TEST_F(move, move_009 )
  {
#if 0
    //create directory $TEMP/tmpname.
    createTestDirectory( aTmpName6 );
    //create directory $TEMP/tmpname/tmpdir
    createTestDirectory( aTmpName8 );
    //move directory $TEMP/tmpname to $TEMP/tmpname/tmpdir/tmpname
    rtl::OUString newName = aTmpName8 + OUString::createFromAscii("/tmpname");
    //printFileName( newName );
    nError1 = ::osl::File::move( aTmpName3, newName );
    //deleteTestDirectory( newName + OUString::createFromAscii("/tmpname") );
    //deleteTestDirectory( newName );
    deleteTestDirectory( aTmpName8 );
    deleteTestDirectory( aTmpName6 );
    ASSERT_TRUE(::osl::FileBase::E_None != nError1) << "test for move function: move a directory to it's subdirectory";
#endif
  }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC remove( const ::rtl::OUString& ustrFileURL )
    //---------------------------------------------------------------------
    class  FileRemove : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1, nError2;
        sal_uInt64 nCount_write, nCount_read;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );

            //write chars into the file.
            ::osl::File testFile( aTmpName4 );

            nError1 = testFile.open( OpenFlag_Write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = testFile.write( pBuffer_Char, sizeof( pBuffer_Char ), nCount_write );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
             nError1 = testFile.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class remove

    TEST_F(FileRemove, remove_001 )
    {
        //remove $TEMP/tmpdir/tmpname.
        nError1 = ::osl::File::remove( aTmpName4 );
        //check
            ::osl::File    testFile( aTmpName4 );
        nError2 = testFile.open( OpenFlag_Create );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError1 ) &&
                                ( ::osl::FileBase::E_EXIST != nError2 )) << "test for remove function: remove a file";
    }

    TEST_F(FileRemove, remove_002 )
    {
        //remove $TEMP/tmpname.
        nError1 = ::osl::File::remove( aTmpName6 );

        ASSERT_TRUE(( ::osl::FileBase::E_NOENT == nError1 )) << "test for remove function: remove a file not exist";
    }

    TEST_F(FileRemove, remove_003 )
    {
        //remove $TEMP/system/path.
        nError1 = ::osl::File::remove( aSysPath2 );

        ASSERT_TRUE(( ::osl::FileBase::E_INVAL == nError1 )) << "test for remove function: removing a file not using full qualified URL";
    }

    TEST_F(FileRemove, remove_004 )
    {
        //remove $TEMP/tmpdir.
        nError1 = ::osl::File::remove( aTmpName3 );

        ASSERT_TRUE(( ::osl::FileBase::E_ISDIR == nError1 ) || ( ::osl::FileBase::E_ACCES == nError1 )) << "test for remove function: remove a directory";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC setAttributes( const ::rtl::OUString& ustrFileURL, sal_uInt64 uAttributes )
    //---------------------------------------------------------------------
    class  setAttributes : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC      nError1, nError2;
        ::osl::DirectoryItem    rItem, rItem_hidden;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestFile( aTmpName6 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class setAttributes

    TEST_F(setAttributes, setAttributes_001 )
    {
    //on windows, only can set 2 attributes: Attribute_ReadOnly,  Attribute_HIDDEN
#ifdef UNX
        //set the file to readonly
        nError2 = ::osl::File::setAttributes( aTmpName6, Attribute_ReadOnly | Attribute_GrpRead | Attribute_OwnRead | Attribute_OthRead );
        ASSERT_TRUE( nError2 == FileBase::E_None);
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE( nError1 == FileBase::E_None);
        //get the file attributes
            ::osl::FileStatus   rFileStatus( FileStatusMask_Attributes );
        nError1 = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError1 == FileBase::E_None );

        ASSERT_TRUE(( Attribute_ReadOnly | Attribute_GrpRead | Attribute_OwnRead | Attribute_OthRead ) ==
                                rFileStatus.getAttributes( )) << "test for setAttributes function: set file attributes and get it to verify.";
#else
        //please see GetFileAttributes
        nError2 = ::osl::File::setAttributes( aTmpName6, Attribute_ReadOnly );
        ASSERT_TRUE( nError2 == FileBase::E_None);
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE( nError1 == FileBase::E_None);
        //get the file attributes
            ::osl::FileStatus   rFileStatus( FileStatusMask_Attributes );
        nError1 = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError1 == FileBase::E_None );
        //here the file has 2 Attributes: FILE_ATTRIBUTE_READONLY and FILE_ATTRIBUTE_NORMAL,
        // but FILE_ATTRIBUTE_NORMAL is valid only if used alone, so this is maybe a bug
        /*::rtl::OString aString = ::rtl::OUStringToOString( aTmpName6, RTL_TEXTENCODING_ASCII_US );
        DWORD dwFileAttributes = GetFileAttributes( aString.getStr( ) );
        if (dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
            printf("has normal attribute");
        if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            printf("has readonly attribute");
        */
        ASSERT_TRUE((Attribute_ReadOnly & rFileStatus.getAttributes( )) != 0) << "test for setAttributes function: set file attributes READONLY and get it to verify.";
#endif
    }

    TEST_F(setAttributes, setAttributes_002 )
    {
    //on UNX, can not set hidden attribute to file, rename file can set the attribute
#ifdef WNT
        //set the file to hidden
        nError2 = ::osl::File::setAttributes( aTmpName6, Attribute_Hidden);

        ASSERT_TRUE( nError2 == FileBase::E_None);
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE( nError1 == FileBase::E_None);
        //get the file attributes
            ::osl::FileStatus rFileStatus( FileStatusMask_Attributes );
        nError1 = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError1 == FileBase::E_None );

        ASSERT_TRUE((Attribute_Hidden & rFileStatus.getAttributes( )) != 0) << "test for setAttributes function: set file attributes and get it to verify.";
#endif
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC setTime(
    //         const ::rtl::OUString& ustrFileURL,
    //         const TimeValue& rCreationTime,
    //         const TimeValue& rLastAccessTime,
    //         const TimeValue& rLastWriteTime )
    //---------------------------------------------------------------------
    class  setTime : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestFile( aTmpName6 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class setTime

    TEST_F(setTime, setTime_001 )
    {
            TimeValue *pTV_current  = NULL;
        ASSERT_TRUE( ( pTV_current = ( TimeValue* )malloc( sizeof( TimeValue ) ) ) != NULL );
        TimeValue *pTV_creation = NULL;
        ASSERT_TRUE( ( pTV_creation = ( TimeValue* )malloc( sizeof( TimeValue ) ) ) != NULL );
        TimeValue *pTV_access   = NULL;
        ASSERT_TRUE( ( pTV_access = ( TimeValue* )malloc( sizeof( TimeValue ) ) ) != NULL );
        TimeValue *pTV_modify   = NULL;
        ASSERT_TRUE( ( pTV_modify = ( TimeValue* )malloc( sizeof( TimeValue ) ) ) != NULL );

        //get current time
        sal_Bool bOk = osl_getSystemTime( pTV_current );
        ASSERT_TRUE( sal_True == bOk );

        //set the file time
        nError2 = ::osl::File::setTime( aTmpName6, *pTV_current, *pTV_current, *pTV_current );
        ASSERT_TRUE(nError2 == FileBase::E_None) << errorToStr( nError2 ).pData;

            //get the file access time, creation time, modify time
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE(nError1 == FileBase::E_None) << errorToStr( nError1 ).pData;

            ::osl::FileStatus   rFileStatus( FileStatusMask_AccessTime );
        nError1 = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE(nError1 == FileBase::E_None) << errorToStr( nError1 ).pData;
        *pTV_access = rFileStatus.getAccessTime( );

            ::osl::FileStatus   rFileStatus1( FileStatusMask_CreationTime );
        nError1 = rItem.getFileStatus( rFileStatus1 );
        ASSERT_TRUE(nError1 == FileBase::E_None) << errorToStr( nError1 ).pData;
        *pTV_creation = rFileStatus1.getCreationTime( );

            ::osl::FileStatus   rFileStatus2( FileStatusMask_ModifyTime );
        nError1 = rItem.getFileStatus( rFileStatus2 );
        ASSERT_TRUE(nError1 == FileBase::E_None) << errorToStr( nError1 ).pData;
        *pTV_modify = rFileStatus2.getModifyTime( );

        ASSERT_TRUE(sal_True == t_compareTime( pTV_access, pTV_current, delta )) << "test for setTime function: set access time then get it. time precision is still a problem for it cut off the nanosec.";
#if defined ( WNT )
        //Unfortunately there is no way to get the creation time of a file under Unix (its a Windows only feature).
        //That means the flag osl_FileStatus_Mask_CreationTime should be deprecated under Unix.
        ASSERT_TRUE( sal_True == t_compareTime( pTV_creation, pTV_current, delta ) ) << "test for setTime function: set creation time then get it. ";
#endif
        ASSERT_TRUE(sal_True == t_compareTime( pTV_modify, pTV_current, delta )) << "test for setTime function: set modify time then get it. ";
        free( pTV_current );
        free( pTV_creation );
        free( pTV_access );
        free( pTV_modify );
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC sync()
    //---------------------------------------------------------------------
        class  sync : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestFile( aTmpName6 );

        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class setTime

    // test case: if The file is located on a read only file system.
    TEST_F(sync, sync_001 )
    {
#ifdef UNX
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE( nError1 == FileBase::E_None);

        File tmp_file( aTmpName6 );
        FileBase::RC err = tmp_file.open(osl_File_OpenFlag_Write );

        ASSERT_TRUE(err == FileBase::E_None) << "File open failed";

        char buffer[50000];
        sal_uInt64 written = 0;
        nError1 = tmp_file.write((void*)buffer, sizeof(buffer), written);
        ASSERT_TRUE(nError1 == FileBase::E_None) << "write failed!";

        //set the file to readonly
        nError2 = ::osl::File::setAttributes( aTmpName6, Attribute_ReadOnly | Attribute_GrpRead | Attribute_OwnRead | Attribute_OthRead );
        ASSERT_TRUE( nError2 == FileBase::E_None);

        nError2 = tmp_file.sync();

            ASSERT_TRUE(nError2 == FileBase::E_None) << "can not sync to readonly file!";

        tmp_file.close();
#endif
    }
    //test case:no enough space, how to create such case???see test_cpy_wrt_file.cxx::test_osl_writeFile

}// namespace osl_File


//------------------------------------------------------------------------
// Beginning of the test cases for DirectoryItem class
//------------------------------------------------------------------------
namespace osl_DirectoryItem
{
    //---------------------------------------------------------------------
    //  testing the method
    //  DirectoryItem(): _pData( NULL )
    //---------------------------------------------------------------------
    class  DirectoryItemCtors : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class DirectoryCtors

    TEST_F(DirectoryItemCtors, ctors_001)
    {
        ::osl::File testFile( aTmpName6 );
        ::osl::DirectoryItem    rItem;  //constructor

        //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE( FileBase::E_None == nError1 );

        ASSERT_TRUE(::osl::FileBase::E_None == nError1) << "test for ctors function: initialize a new instance of DirectoryItem and get an item to check.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  DirectoryItem( const DirectoryItem& rItem ): _pData( rItem._pData)
    //---------------------------------------------------------------------
    class  copy_assin_Ctors : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class copy_assin_Ctors

    TEST_F(copy_assin_Ctors, copy_assin_Ctors_001 )
    {
        ::osl::DirectoryItem    rItem;  //constructor
            //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE( FileBase::E_None == nError1 );

        ::osl::DirectoryItem    copyItem( rItem ); //copy constructor
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        nError1 = copyItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError1 == FileBase::E_None );

        ASSERT_TRUE(( sal_True == compareFileName( rFileStatus.getFileName( ), aTmpName2 ) )) << "test for copy_assin_Ctors function: use copy constructor to get an item and check filename.";
    }

    TEST_F(copy_assin_Ctors, copy_assin_Ctors_002 )
    {
        ::osl::DirectoryItem    rItem;  //constructor
            //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE( FileBase::E_None == nError1 );

        ::osl::DirectoryItem    copyItem;
        copyItem = rItem;               //assinment operator
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        nError1 = copyItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError1 == FileBase::E_None );

        ASSERT_TRUE(( sal_True == compareFileName( rFileStatus.getFileName( ), aTmpName2 ) )) << "test for copy_assin_Ctors function: test assinment operator here since it is same as copy constructor in test way.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool is()
    //---------------------------------------------------------------------
    class  is : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class is

    TEST_F(is, is_001 )
    {
        ::osl::DirectoryItem    rItem;  //constructor

        ASSERT_TRUE(!rItem.is( )) << "test for is function: use an uninitialized instance.";
    }

    TEST_F(is, is_002 )
    {
        ::osl::DirectoryItem    rItem;  //constructor
            //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );
        ASSERT_TRUE( FileBase::E_None == nError1 );

        ASSERT_TRUE(( sal_True == rItem.is( ) )) << "test for is function: use an uninitialized instance.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  static inline RC get( const ::rtl::OUString& ustrFileURL, DirectoryItem& rItem )
    //---------------------------------------------------------------------
    class  get : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpname.
            createTestFile( aTmpName6 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpname.
            deleteTestFile( aTmpName6 );
        }
    };// class get

    TEST_F(get, get_001 )
    {
        ::osl::DirectoryItem    rItem;  //constructor
            //get the DirectoryItem.
        nError2 = ::osl::DirectoryItem::get( aTmpName6, rItem );

        //check the file name
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        nError1 = rItem.getFileStatus( rFileStatus );
        ASSERT_TRUE( nError1 == FileBase::E_None );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError2 ) &&
                                ( sal_True == compareFileName( rFileStatus.getFileName( ), aTmpName2 ) )) << "test for get function: use copy constructor to get an item and check filename.";
    }

    TEST_F(get, get_002 )
    {
        ::osl::DirectoryItem    rItem;
            //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aSysPath1, rItem );

        ASSERT_TRUE(FileBase::E_INVAL == nError1) << "test for get function: use a system name instead of a URL.";
    }

    TEST_F(get, get_003 )
    {
        ::osl::DirectoryItem    rItem;
            //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aTmpName3, rItem );

        ASSERT_TRUE(FileBase::E_NOENT == nError1) << "test for get function: use a non existed file URL.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC getFileStatus( FileStatus& rStatus )
    //---------------------------------------------------------------------
    class  getFileStatus : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class getFileStatus

    TEST_F(getFileStatus, getFileStatus_001 )
    {
        ::osl::DirectoryItem    rItem;  //constructor
            //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aTmpName4, rItem );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //check the file name
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        nError2 = rItem.getFileStatus( rFileStatus );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError2 ) &&
                                ( sal_True == compareFileName( rFileStatus.getFileName( ), aTmpName2 ) )) << "test for getFileStatus function: get file status and check filename";
    }

    TEST_F(getFileStatus, getFileStatus_002 )
    {
        ::osl::DirectoryItem    rItem;  //constructor
            //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aTmpName6, rItem );

        //check the file name
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        nError2 = rItem.getFileStatus( rFileStatus );

        ASSERT_TRUE(( ::osl::FileBase::E_INVAL == nError2 )) << "test for getFileStatus function: file not existed";
    }

    TEST_F(getFileStatus, getFileStatus_003 )
    {
        ::osl::DirectoryItem    rItem;  //constructor
            //get the DirectoryItem.
        nError1 = ::osl::DirectoryItem::get( aTmpName3, rItem );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //check the file name
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        nError2 = rItem.getFileStatus( rFileStatus );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError2 ) &&
                                ( sal_True == compareFileName( rFileStatus.getFileName( ), aTmpName1 ) )) << "test for getFileStatus function: get directory information";
    }

}// namespace osl_DirectoryItem


//------------------------------------------------------------------------
// Beginning of the test cases for Directory class
//------------------------------------------------------------------------
namespace osl_Directory
{
    //---------------------------------------------------------------------
    //  testing the method
    //  Directory( const ::rtl::OUString& strPath ): _pData( 0 ), _aPath( strPath )
    //---------------------------------------------------------------------
    class  DirectoryCtors : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
            // LLA: printf("tearDown done.\n");
        }
    };// class DirectoryCtors

    TEST_F(DirectoryCtors, ctors_001 )
    {
        ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //close a directory
        nError2 = testDirectory.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError2 );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError1 ) &&
                                ( ::osl::FileBase::E_None == nError2 )) << "test for ctors function: create an instance and check open and close";
    }

    TEST_F(DirectoryCtors, ctors_002 )
    {
        ::osl::Directory testDirectory( aTmpName9 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //close a directory
        nError2 = testDirectory.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError2 );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError1 ) &&
                                ( ::osl::FileBase::E_None == nError2 )) << "test for ctors function: relative URL, :-), it is also worked";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC open()
    //---------------------------------------------------------------------
    class  DirectoryOpen : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class open

    TEST_F(DirectoryOpen, open_001 )
    {
        ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
        //check if directory is opened.
        sal_Bool bOk = testDirectory.isOpen( );
        //close a directory
        nError2 = testDirectory.close( );

        ASSERT_TRUE(( sal_True == bOk ) &&
                                ( ::osl::FileBase::E_None == nError1 ) &&
                                ( ::osl::FileBase::E_None == nError2 )) << "test for open function: open a directory and check for open";
    }

    TEST_F(DirectoryOpen, open_002 )
    {
        ::osl::Directory testDirectory( aTmpName6 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
            if ( ::osl::FileBase::E_None == nError1 )
        {
            nError2 = testDirectory.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError2 );
        }

        ASSERT_TRUE(( ::osl::FileBase::E_NOENT == nError1 )) << "test for open function: open a file that is not existed";
    }

    TEST_F(DirectoryOpen, open_003 )
    {
        ::osl::Directory testDirectory( aUserDirectorySys ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
            if ( ::osl::FileBase::E_None == nError1 )
        {
            nError2 = testDirectory.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError2 );
        }

        ASSERT_TRUE(( ::osl::FileBase::E_INVAL == nError1 )) << "test for open function: using system path";
    }

    TEST_F(DirectoryOpen, open_004 )
    {
        ::osl::Directory testDirectory( aTmpName4 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
            if ( ::osl::FileBase::E_None == nError1 )
        {
            nError2 = testDirectory.close( );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError2 );
        }

        ASSERT_TRUE(( ::osl::FileBase::E_NOTDIR == nError1 ) || ( ::osl::FileBase::E_ACCES )) << "test for open function: open a file instead of a directory";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline sal_Bool isOpen() { return _pData != NULL; };
    //---------------------------------------------------------------------
    class  isOpen : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempfile in $TEMP/tmpdir/tmpname.
            createTestDirectory( aTmpName3 );
            createTestFile( aTmpName4 );
        }

        void TearDown( )
        {
            // remove the tempfile in $TEMP/tmpdir/tmpname.
            deleteTestFile( aTmpName4 );
            deleteTestDirectory( aTmpName3 );
        }
    };// class isOpen

    TEST_F(isOpen, isOpen_001 )
    {
        ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
        //check if directory is opened.
        sal_Bool bOk = testDirectory.isOpen( );
        //close a directory
        nError2 = testDirectory.close( );

        ASSERT_TRUE(( sal_True == bOk )) << "test for isOpen function: open a directory and check for open";
    }

    TEST_F(isOpen, isOpen_002 )
    {
        ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //check if directory is opened.
        sal_Bool bOk = testDirectory.isOpen( );

        ASSERT_TRUE(!( sal_True == bOk )) << "test for isOpen function: do not open a directory and check for open";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC close()
    //---------------------------------------------------------------------
    class  DirectoryClose : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempdirectory : $TEMP/tmpdir.
            createTestDirectory( aTmpName3 );
        }

        void TearDown( )
        {
            // remove a tempdirectory : $TEMP/tmpdir.
            deleteTestDirectory( aTmpName3 );
        }
    };// class close

    TEST_F(DirectoryClose, close_001 )
    {
        ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
        //close a directory
        nError2 = testDirectory.close( );
        //check if directory is opened.
        sal_Bool bOk = testDirectory.isOpen( );

        ASSERT_TRUE(!( sal_True == bOk )) << "test for isOpen function: close a directory and check for open";
    }

    TEST_F(DirectoryClose, close_002 )
    {
        ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //close a directory
        nError1 = testDirectory.close( );

        ASSERT_TRUE(( ::osl::FileBase::E_BADF == nError1 )) << "test for isOpen function: close a not opened directory";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC reset()
    //---------------------------------------------------------------------
    class  reset : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempdirectory : $TEMP/tmpdir.
            createTestDirectory( aTmpName3 );
            // create three files : $TEMP/tmpdir/tmpname, $TEMP/tmpdir/tmpdir, $TEMP/tmpdir/hiddenfile,
            createTestFile( aTmpName3, aTmpName2);
            createTestFile( aTmpName3, aTmpName1);
            createTestFile( aTmpName3, aHidURL1);
        }

        void TearDown( )
        {
            // remove three files : $TEMP/tmpdir/tmpname, $TEMP/tmpdir/tmpdir, $TEMP/tmpdir/hiddenfile,
            deleteTestFile( aTmpName3, aHidURL1);
            deleteTestFile( aTmpName3, aTmpName1);
            deleteTestFile( aTmpName3, aTmpName2);
            // remove a tempdirectory : $TEMP/tmpdir.
            deleteTestDirectory( aTmpName3 );
        }
    };// class reset

    TEST_F(reset, reset_001 )
    {
        ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //get first Item
        nError1 = testDirectory.getNextItem( rItem, 1 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //get second Item
        //mindy: nError1 = testDirectory.getNextItem( rItem, 0 );
        //mindy: ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //reset enumeration
        nError2 = testDirectory.reset( );
        //get reseted Item, if reset does not work, getNextItem() should return the second Item (aTmpName1)
        nError1 = testDirectory.getNextItem( rItem, 0 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //check the file name
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        nError1 = rItem.getFileStatus( rFileStatus );
        //close a directory
        nError1 = testDirectory.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        sal_Bool bOK1,bOK2;
        bOK1 = compareFileName( rFileStatus.getFileName( ), aTmpName2 );
        bOK2 = compareFileName( rFileStatus.getFileName( ), aHidURL1 );

        ASSERT_TRUE(( ::osl::FileBase::E_None == nError2 ) &&
                                ( sal_True == bOK1 || bOK2 )) << "test for reset function: get two directory item, reset it, then get again, check the filename";
    }

    TEST_F(reset, reset_002 )
    {
        ::osl::Directory testDirectory( aTmpName6 ); //constructor

        //close a directory
        nError1 = testDirectory.reset( );

        ASSERT_TRUE(( ::osl::FileBase::E_NOENT == nError1 )) << "test for reset function: reset a non existed directory";
    }


    TEST_F(reset, reset_003 )
    {
        ::osl::Directory testDirectory( aTmpName4 ); //constructor

        //close a directory
        nError1 = testDirectory.reset( );

        ASSERT_TRUE(( ::osl::FileBase::E_NOTDIR == nError1 ) || ( ::osl::FileBase::E_NOENT == nError1 )) << "test for reset function: reset a file instead of a directory";
    }

    TEST_F(reset, reset_004 )
    {
        ::osl::Directory testDirectory( aUserDirectorySys ); //constructor

        //close a directory
        nError1 = testDirectory.reset( );

        ASSERT_TRUE(( ::osl::FileBase::E_INVAL == nError1 )) << "test for reset function: use a system path";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline RC getNextItem( DirectoryItem& rItem, sal_uInt32 nHint = 0 )
    //---------------------------------------------------------------------
    class  getNextItem : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;
        ::osl::DirectoryItem    rItem;

        public:
        // initialization
        void SetUp( )
        {
            // create a tempdirectory : $TEMP/tmpdir.
            createTestDirectory( aTmpName3 );
            // create three files : $TEMP/tmpdir/tmpname, $TEMP/tmpdir/tmpdir, $TEMP/tmpdir/hiddenfile,
            createTestFile( aTmpName3, aTmpName2 );
            createTestFile( aTmpName3, aTmpName1 );
            createTestFile( aTmpName3, aHidURL1 );

        }

        void TearDown( )
        {
            // remove three files : $TEMP/tmpdir/tmpname, $TEMP/tmpdir/tmpdir, $TEMP/tmpdir/hiddenfile,
            deleteTestFile( aTmpName3, aHidURL1 );
            deleteTestFile( aTmpName3, aTmpName1 );
            deleteTestFile( aTmpName3, aTmpName2 );
            // remove a tempdirectory : $TEMP/tmpdir.
            deleteTestDirectory( aTmpName3 );
        }
    };// class getNextItem

    TEST_F(getNextItem, getNextItem_001 )
    {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        //check the file name
        ::rtl::OUString     strFilename;
        sal_Bool            bOk1 = sal_False;
        sal_Bool bOk2 = sal_False;
        sal_Bool bOk3 = sal_False;
            ::osl::FileStatus   rFileStatus( FileStatusMask_FileName );
        for ( int nCount = 0; nCount < 3; nCount++ )
        {
            //get three Items
            nError1 = testDirectory.getNextItem( rItem, 2 );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            nError1 = rItem.getFileStatus( rFileStatus );
            ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
            switch ( nCount )
            {
                case 0: bOk1 = compareFileName( rFileStatus.getFileName( ), aTmpName2 ) || compareFileName( rFileStatus.getFileName( ), aHidURL1);
                    break;
                case 1: bOk2 = compareFileName( rFileStatus.getFileName( ), aTmpName1 );
                    break;
                case 2: bOk3 = compareFileName( rFileStatus.getFileName( ), aHidURL1) || compareFileName( rFileStatus.getFileName( ), aTmpName2 );
            }
        }

        //close a directory
        nError1 = testDirectory.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(( sal_True == bOk1 ) && ( sal_True == bOk2 ) && ( sal_True == bOk3 )) << "test for getNextItem function: retrive three items and check their names.";
    }

    TEST_F(getNextItem, getNextItem_002 )
    {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor
        nError1 = testDirectory.getNextItem( rItem );

        ASSERT_TRUE(( ::osl::FileBase::E_INVAL == nError1 )) << "test for getNextItem function: retrive an item in a directory which is not opened, also test for nHint's default value.";
    }

    TEST_F(getNextItem, getNextItem_003 )
    {
            ::osl::Directory testDirectory( aTmpName3 ); //constructor

        //open a directory
        nError1 = testDirectory.open( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        for ( int nCount = 0; nCount < 4; nCount++ )
        {
            nError2 = testDirectory.getNextItem( rItem, 3 );
        }

        //close a directory
        nError1 = testDirectory.close( );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );

        ASSERT_TRUE(( ::osl::FileBase::E_NOENT == nError2 )) << "test for getNextItem function: retrive 4 times in a directory which contain only 3 files.";
    }

    TEST_F(getNextItem, getNextItem_004 )
    {
    //create a link file(can not on Windows), then check if getNextItem can get it.
#ifdef UNX
        sal_Bool bOK = sal_False;
        ::rtl::OUString aUStr_LnkFileSys( aTempDirectorySys ), aUStr_SrcFileSys( aTempDirectorySys );
        ( ( aUStr_LnkFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/tmpdir/link.file");
        ( ( aUStr_SrcFileSys += aSlashURL ) += getCurrentPID( ) ) += ::rtl::OUString::createFromAscii("/tmpdir/tmpname");
            rtl::OString strLinkFileName, strSrcFileName;
            strLinkFileName = OUStringToOString( aUStr_LnkFileSys, RTL_TEXTENCODING_ASCII_US );
            strSrcFileName  = OUStringToOString( aUStr_SrcFileSys, RTL_TEXTENCODING_ASCII_US );

        //create a link file and link it to file "/tmp/PID/tmpdir/tmpname"
            sal_Int32 fd = symlink( strSrcFileName.getStr(), strLinkFileName.getStr() );
        ASSERT_TRUE( fd == 0 );
        ::osl::Directory testDirectory( aTmpName3 );

        //open a directory
        nError1 = testDirectory.open( );
        ::rtl::OUString aFileName = ::rtl::OUString::createFromAscii("link.file");

        while (1) {
            nError1 = testDirectory.getNextItem( rItem, 4 );
            if (::osl::FileBase::E_None == nError1) {
                ::osl::FileStatus   rFileStatus( FileStatusMask_FileName | FileStatusMask_Type );
                rItem.getFileStatus( rFileStatus );
                if ( compareFileName( rFileStatus.getFileName( ), aFileName) == sal_True )
                {
                    if ( FileStatus::Link == rFileStatus.getFileType( ))
                    {
                        bOK = sal_True;
                        break;
                    }
                }
            }
            else
                break;
        };
            fd = std::remove( strLinkFileName.getStr() );
        ASSERT_TRUE(fd == 0) << "remove link file failed";
        ASSERT_TRUE(( bOK == sal_True )) << "test for getNextItem function: check if can retrieve the link file name";
#endif
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC getVolumeInfo( const ::rtl::OUString& ustrDirectoryURL, VolumeInfo& rInfo )
    //---------------------------------------------------------------------
    class  getVolumeInfo : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;

        public:

        // test code.
        void checkValidMask(osl::VolumeInfo const& _aVolumeInfo, sal_Int32 _nMask)
            {
                if (_nMask == VolumeInfoMask_FileSystemName)
        {
            //get file system name
            ::rtl::OUString aFileSysName( aNullURL );
                    aFileSysName = _aVolumeInfo.getFileSystemName( );

                    sal_Bool bRes2 = compareFileName( aFileSysName, aNullURL );
            ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) &&
                                            ( sal_False == bRes2 )) << "test for getVolumeInfo function: getVolumeInfo of root directory.";
                }
                if (_nMask == VolumeInfoMask_Attributes)
                {
                    sal_Bool b1 = _aVolumeInfo.getRemoteFlag();
                    sal_Bool b2 = _aVolumeInfo.getRemoveableFlag();
                    sal_Bool b3 = _aVolumeInfo.getCompactDiscFlag();
                    sal_Bool b4 = _aVolumeInfo.getFloppyDiskFlag();
                    sal_Bool b5 = _aVolumeInfo.getFixedDiskFlag();
                    sal_Bool b6 = _aVolumeInfo.getRAMDiskFlag();

                    rtl::OString sAttr;
                    if (b1) sAttr =  "Remote";
                    if (b2) sAttr += " Removeable";
                    if (b3) sAttr += " CDROM";
                    if (b4) sAttr += " Floppy";
                    if (b5) sAttr += " FixedDisk";
                    if (b6) sAttr += " RAMDisk";

                    printf("Attributes: %s\n", sAttr.getStr() );
                }
                if (_nMask == VolumeInfoMask_TotalSpace)
                {
                    // within Linux, df / * 1024 bytes is the result
                    sal_uInt64 nSize = _aVolumeInfo.getTotalSpace();
                    printf("Total space: %lld\n", nSize);
                }
                if (_nMask == VolumeInfoMask_UsedSpace)
                {
                    sal_uInt64 nSize = _aVolumeInfo.getUsedSpace();
                    printf(" Used space: %lld\n", nSize);
                }
                if (_nMask == VolumeInfoMask_FreeSpace)
                {
                    sal_uInt64 nSize = _aVolumeInfo.getFreeSpace();
                    printf(" Free space: %lld\n", nSize);
                }
                if (_nMask == VolumeInfoMask_MaxNameLength)
                {
                    sal_uInt32 nLength = _aVolumeInfo.getMaxNameLength();
                    printf("max name length: %ld\n", nLength);
                }
                if (_nMask == VolumeInfoMask_MaxPathLength)
                {
                    sal_uInt32 nLength = _aVolumeInfo.getMaxPathLength();
                    printf("max path length: %ld\n", nLength);
                }
                if (_nMask == VolumeInfoMask_FileSystemCaseHandling)
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
                // LLA: ASSERT_TRUE(sal_True == aVolumeInfo.isValid( _nMask )) << "mask is not valid";
                if (aVolumeInfo.isValid( _nMask))
                {
                    checkValidMask(aVolumeInfo, _nMask);
                }
            }
    };// class getVolumeInfo

    TEST_F(getVolumeInfo, getVolumeInfo_001_1 )
    {
        sal_Int32 mask = VolumeInfoMask_FileSystemName;
        checkVolumeInfo(mask);
    }
    TEST_F(getVolumeInfo, getVolumeInfo_001_2 )
    {
        sal_Int32 mask = VolumeInfoMask_Attributes;
        checkVolumeInfo(mask);
    }
    TEST_F(getVolumeInfo, getVolumeInfo_001_3 )
    {
        sal_Int32 mask = VolumeInfoMask_TotalSpace;
        checkVolumeInfo(mask);
    }
    TEST_F(getVolumeInfo, getVolumeInfo_001_4 )
    {
        sal_Int32 mask = VolumeInfoMask_UsedSpace;
        checkVolumeInfo(mask);
    }
    TEST_F(getVolumeInfo, getVolumeInfo_001_5 )
    {
        sal_Int32 mask = VolumeInfoMask_FreeSpace;
        checkVolumeInfo(mask);
    }
    TEST_F(getVolumeInfo, getVolumeInfo_001_6 )
    {
        sal_Int32 mask = VolumeInfoMask_MaxNameLength;
        checkVolumeInfo(mask);
    }
    TEST_F(getVolumeInfo, getVolumeInfo_001_7 )
    {
        sal_Int32 mask = VolumeInfoMask_MaxPathLength;
        checkVolumeInfo(mask);
    }
    TEST_F(getVolumeInfo, getVolumeInfo_001_8 )
    {
        sal_Int32 mask = VolumeInfoMask_FileSystemCaseHandling;
        checkVolumeInfo(mask);
    }

    TEST_F(getVolumeInfo, getVolumeInfo_002 )
    {
        sal_Int32 mask = VolumeInfoMask_FileSystemName;
        ::osl::VolumeInfo aVolumeInfo( mask );
        //call getVolumeInfo here

        // LLA: rtl::OUString aRootSysURL;
        // LLA: nError1 = osl::File::getFileURLFromSystemPath(aRootSys, aRootSysURL);
        // LLA:
        // LLA: ASSERT_TRUE(// LLA:                         ( osl::FileBase::E_NONE == nError1 )) << "can't convert root path to file url";

        nError1 = ::osl::Directory::getVolumeInfo( aRootSys, aVolumeInfo );

        ASSERT_TRUE(( osl::FileBase::E_INVAL == nError1 )) << "test for getVolumeInfo function: use system path as parameter.";
    }

    TEST_F(getVolumeInfo, getVolumeInfo_003 )
    {
        sal_Int32 mask = VolumeInfoMask_FileSystemName;
        ::osl::VolumeInfo aVolumeInfo( mask );
        //call getVolumeInfo here
        nError1 = ::osl::Directory::getVolumeInfo( aTmpName3, aVolumeInfo );

// LLA: in Windows, it reply no error, it did not pass in (W32).
#ifdef UNX
        ASSERT_TRUE(( osl::FileBase::E_NOENT == nError1 )) << "test for getVolumeInfo function: non-existence test. ";
#endif
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC create( const ::rtl::OUString& ustrDirectoryURL )
    //---------------------------------------------------------------------
    class  create : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;
    };// class create

    TEST_F(create, create_001 )
    {
        //create directory in $TEMP/tmpdir
        nError1 = ::osl::Directory::create( aTmpName3 );
        //check for existence
        nError2 = ::osl::Directory::create( aTmpName3 );
        //remove it
        deleteTestDirectory( aTmpName3 );

        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) &&
                                ( osl::FileBase::E_EXIST== nError2 )) << "test for create function: create a directory and check its existence.";
    }

    TEST_F(create, create_002 )
    {
        //create directory in /tmpname
        nError1 = ::osl::Directory::create( aTmpName7 );
#if defined (WNT )
        nError1 = osl::FileBase::E_ACCES;  /// in Windows, you can create directory in c:/ any way.
        deleteTestDirectory( aTmpName7 );
#endif

        ASSERT_TRUE(( osl::FileBase::E_ACCES == nError1 )) << "test for create function: create a directory in root for access test.";
    }

    TEST_F(create, create_003 )
    {
        //create directory in /tmpname
        nError1 = ::osl::Directory::create( aSysPath1 );

        ASSERT_TRUE(( osl::FileBase::E_INVAL == nError1 )) << "test for create function: create a directory using system path.";
    }

    //---------------------------------------------------------------------
    //  testing the method
    //  inline static RC remove( const ::rtl::OUString& ustrDirectoryURL )
    //---------------------------------------------------------------------
    class  DirectoryRemove : public ::testing::Test
    {
    protected:
        ::osl::FileBase::RC     nError1, nError2;
    };// class remove

    TEST_F(DirectoryRemove, remove_001 )
    {
        //create directory in $TEMP/tmpdir
        nError1 = ::osl::Directory::create( aTmpName3 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //remove it
        nError1 = ::osl::Directory::remove( aTmpName3 );
        //check for existence
        ::osl::Directory rDirectory( aTmpName3 );
        nError2 = rDirectory.open( );

        ASSERT_TRUE(( osl::FileBase::E_None == nError1 ) &&
                                ( osl::FileBase::E_NOENT == nError2 )) << "test for remove function: remove a directory and check its existence.";
    }

    TEST_F(DirectoryRemove, remove_002 )
    {
        //create directory in $TEMP/tmpdir
        nError1 = ::osl::Directory::create( aTmpName3 );
        ASSERT_TRUE( ::osl::FileBase::E_None == nError1 );
        //try to remove it by system path
        nError1 = ::osl::Directory::remove( aSysPath3 );
            //check for existence
        ::osl::Directory rDirectory( aTmpName3 );
        nError2 = rDirectory.open( );
        if ( osl::FileBase::E_NOENT != nError2 )
            ::osl::Directory::remove( aTmpName3 );

        ASSERT_TRUE(( osl::FileBase::E_INVAL == nError1 )) << "test for remove function: remove a directory by its system path, and check its existence.";
    }

    TEST_F(DirectoryRemove, remove_003 )
    {
        //try to remove a non-existed directory
        nError1 = ::osl::Directory::remove( aTmpName6 );

        ASSERT_TRUE(( osl::FileBase::E_NOENT == nError1 )) << "test for remove function: try to remove a non-existed directory.";
    }

    TEST_F(DirectoryRemove, remove_004 )
    {
        createTestFile( aTmpName6 );
        sal_Bool bExist = ifFileExist( aTmpName6 );
        //try to remove file.
        nError1 = ::osl::Directory::remove( aTmpName6 );
        deleteTestFile( aTmpName6 );

        ASSERT_TRUE(bExist == sal_True &&(( osl::FileBase::E_NOTDIR == nError1 ) || ( osl::FileBase::E_NOENT == nError1 ))) << "test for remove function: try to remove a file but not directory.";
    }

    TEST_F(DirectoryRemove, remove_005 )
    {
        createTestDirectory( aTmpName3 );
        createTestFile( aTmpName4 );
        nError1 = ::osl::Directory::remove( aTmpName3 );
        deleteTestFile( aTmpName4 );
        deleteTestDirectory( aTmpName3 );
        ::rtl::OUString suError = ::rtl::OUString::createFromAscii("test for remove function: try to remove a directory that is not empty.") + errorToStr( nError1 );
#if defined ( SOLARIS )
        //on UNX, the implementation uses rmdir(), which EEXIST is thrown on Solaris when the directory is not empty, refer to: 'man -s 2 rmdir', while on linux, ENOTEMPTY is thrown.
        //EEXIST The directory contains entries other than those for "." and "..".
        printf("#Solaris test\n");
        ASSERT_TRUE(( osl::FileBase::E_EXIST == nError1 )) << suError.pData;
#else
        ASSERT_TRUE(( osl::FileBase::E_NOTEMPTY == nError1 )) << suError.pData;
#endif
    }

    //########################################
    // TEST Directory::createPath
    //########################################

    #ifdef WNT
    #   define PATH_BUFFER_SIZE MAX_PATH
    #else
    #   define PATH_BUFFER_SIZE PATH_MAX
    #endif

    char TEST_PATH_POSTFIX[] = "hello/world";

    //########################################
    OUString get_test_path()
    {
        OUString tmp;
        FileBase::RC rc = FileBase::getTempDirURL(tmp);

        EXPECT_TRUE(rc == FileBase::E_None) << "Test path creation failed";

        OUStringBuffer b(tmp);
        if (tmp.lastIndexOf('/') != (tmp.getLength() - 1))
            b.appendAscii("/");

        b.appendAscii(TEST_PATH_POSTFIX);

        return b.makeStringAndClear();
    }

    //########################################
    void rm_test_path(const OUString& path)
    {
        sal_Unicode buffer[PATH_BUFFER_SIZE];
        rtl_copyMemory(buffer, path.getStr(), (path.getLength() + 1) * sizeof(sal_Unicode));

        sal_Int32 i = rtl_ustr_lastIndexOfChar(buffer, '/');
        if (i == path.getLength())
            buffer[i] = 0;

        Directory::remove(buffer);

        i = rtl_ustr_lastIndexOfChar(buffer, '/');
        buffer[i] = 0;
        Directory::remove(buffer);
    }

    //########################################
    class DirCreatedObserver : public DirectoryCreationObserver
    {
    public:
        DirCreatedObserver() : i(0)
        {
        }

        virtual void DirectoryCreated(const rtl::OUString& /*aDirectoryUrl*/)
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


    //########################################
    class createPath : public ::testing::Test
    {
    public:
        //##########################################
        createPath()
        {}

#ifdef WNT

        //##########################################
        char* get_unused_drive_letter()
        {
            static char m_aBuff[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            DWORD ld = GetLogicalDrives();
            DWORD i = 4;
            DWORD j = 2;

            while ((ld & i) && (i > 1))
            { i = i << 1; j++; }

            if (i > 2)
                return m_aBuff + j;

            return NULL;
        }


#endif /* WNT */
    }; // class createPath

#ifdef WNT
    //##########################################
    TEST_F(createPath, at_invalid_logical_drive)
    {
        char* drv = get_unused_drive_letter();
        char buff[PATH_BUFFER_SIZE];
        rtl_zeroMemory(buff, sizeof(buff));

        strncpy(buff, drv, 1);
        strcat(buff, ":\\");
        strcat(buff, TEST_PATH_POSTFIX);

        OUString path = OUString::createFromAscii(buff);
        OUString tp_url;
        FileBase::getFileURLFromSystemPath(path, tp_url);

        FileBase::RC rc = Directory::createPath(tp_url);

        ASSERT_TRUE(rc != FileBase::E_None) << "osl_createDirectoryPath doesn't fail on unused logical drive letters";
    }

    //##########################################
    // adapt the UNC path in method createDirectoryPath_with_UNC_path
    // in order to run this test successfully
#if 0
    TEST_F(createPath, with_UNC_path)
    {

        OUString tp_unc = OUString::createFromAscii("\\\\Tra-1\\TRA_D\\hello\\world\\");
        OUString tp_url;
        FileBase::getFileURLFromSystemPath(tp_unc, tp_url);

        FileBase::RC rc = Directory::createPath(tp_url);

        ASSERT_TRUE(rc == FileBase::E_None) << "osl_createDirectoryPath fails with UNC path";
    }
#endif
#endif

    //##########################################
    TEST_F(createPath, with_relative_path)
    {
        FileBase::RC rc = Directory::createPath(
            OUString::createFromAscii(TEST_PATH_POSTFIX));

        ASSERT_TRUE(rc == FileBase::E_INVAL) << "osl_createDirectoryPath contract broken";
    }

    //##########################################
    TEST_F(createPath, without_callback)
    {
        OUString tp_url = get_test_path();

        rm_test_path(tp_url);

        FileBase::RC rc = Directory::createPath(tp_url);

        ASSERT_TRUE(rc == FileBase::E_None) << "osl_createDirectoryPath failed";
    }

    //##########################################
    TEST_F(createPath, with_callback)
    {
        OUString tp_url = get_test_path();

        rm_test_path(tp_url);

        DirCreatedObserver* observer = new DirCreatedObserver;
        FileBase::RC rc = Directory::createPath(tp_url, observer);
        int nDirs = observer->number_of_dirs_created();
        delete observer;
        ASSERT_TRUE((rc == FileBase::E_None) && (nDirs > 0)) << "osl_createDirectoryPath failed";

    }

}// namespace osl_Directory


// -----------------------------------------------------------------------------
// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
// -----------------------------------------------------------------------------

/// NOADDITIONAL;



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


/** Insert Current PID to the URL to avoid access violation between multiuser execution.
*/
inline void insertPID( ::rtl::OUString & pathname )
{
    //~ check if the path contain the temp directory, do nothing changes if not;
    if ( pathname.indexOf( aTempDirectoryURL ) && pathname.indexOf( aTempDirectorySys ) )
        return;

    //~ format pathname to TEMP/USERPID/URL style;
    if ( !pathname.indexOf( aTempDirectoryURL ) )
    {
        ::rtl::OUString strPID( getCurrentPID( ) );
        ::rtl::OUString pathLeft = aTempDirectoryURL.copy( 0 );
        ::rtl::OUString pathRight = pathname.copy( aTempDirectoryURL.getLength( ) );
        pathname = pathLeft.copy( 0 );
        ( ( pathname += aSlashURL ) += strPID ) += pathRight;
    }
    else
    {
        ::rtl::OUString strPID( getCurrentPID( ) );
        ::rtl::OUString pathLeft = aTempDirectorySys.copy( 0 );
        ::rtl::OUString pathRight = pathname.copy( aTempDirectorySys.getLength( ) );
        pathname = pathLeft.copy( 0 );
        ( ( pathname += aSlashURL ) += strPID ) += pathRight;
    }


}

int main(int argc, char **argv)
{
    printf( "Initializing..." );

    //~ make sure the c:\temp exist, if not, create it.
#if ( defined WNT )
    if ( checkDirectory( aTempDirectoryURL, osl_Check_Mode_Exist )  != sal_True ) {
        printf( "\n#C:\\temp is not exist, now creating\n" );
        createTestDirectory( aTempDirectoryURL );
    };
#endif

    //~ make sure the c:\temp\PID or /tmp/PID exist, if not, create it. initialize the user directory.
    ( aUserDirectoryURL += aSlashURL ) += getCurrentPID( );
    ( aUserDirectorySys += aSlashURL ) += getCurrentPID( );

    if ( checkDirectory( aUserDirectoryURL, osl_Check_Mode_Exist )  != sal_True ) {
        createTestDirectory( aUserDirectoryURL );
    }

    //~ adapt all URL to the TEMP/USERPID/URL format;
    insertPID( aCanURL1 );
    insertPID( aTmpName3 );
    insertPID( aTmpName4 );
    insertPID( aTmpName5 );
    insertPID( aTmpName6 );
    insertPID( aTmpName8 );
    insertPID( aTmpName9 );
    insertPID( aLnkURL1 );
    insertPID( aFifoSys );
    insertPID( aSysPath1 );
    insertPID( aSysPath2 );
    insertPID( aSysPath3 );
    insertPID( aSysPath4 );

    printf( "Done.\n" );

     ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


//~ do some clean up work after all test completed.
class GlobalObject
{
    public:
    ~GlobalObject()
    {
        try
        {
            //~ make sure the c:\temp\PID or /tmp/PID exist, if yes, delete it.
            printf( "\n#Do some clean-ups ...\n" );
            if ( checkDirectory( aUserDirectoryURL, osl_Check_Mode_Exist )  == sal_True ) {
                deleteTestDirectory( aUserDirectoryURL );
            }

            // LLA: printf("after deleteTestDirectory\n");
            //~ special clean up task in Windows and Unix separately;
#if ( defined UNX ) || ( defined OS2 )
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
        catch (...)
        {
            printf("Exception caught (...) in GlobalObject dtor()\n");
        }
    }
};

GlobalObject theGlobalObject;
