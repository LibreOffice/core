 /*************************************************************************
 *
 *  $RCSfile: osl_Module.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $  $Date: 2003-11-18 16:39:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *     - GNU Lesser General Public License Version 2.1
 *     - Sun Industry Standards Source License Version 1.1
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------

#ifndef _OSL_MODULE_CONST_H_
#include <osl_Module_Const.h>
#endif

using namespace osl;
using namespace rtl;


//------------------------------------------------------------------------
// helper functions and classes
//------------------------------------------------------------------------

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf( "#printBool# " );
    ( sal_True == bOk ) ? printf( "TRUE!\n" ): printf( "FALSE!\n" );
}

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf( "#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf( "%s\n", aString.getStr( ) );
}

/** get dll file URL.
*/
inline ::rtl::OUString getDllURL( void )
{
#if ( defined WNT )        // lib in Unix and lib in Windows are not same in file name.
    ::rtl::OUString libPath( rtl::OUString::createFromAscii( "Module_DLL.dll" ) );
#else
    ::rtl::OUString libPath( rtl::OUString::createFromAscii( "libModule_DLL.so" ) );
#endif

    ::rtl::OUString dirPath, dllPath;
    osl::Module::getUrlFromAddress( ( void* ) &getDllURL, dirPath );
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') + 1);
    osl::FileBase::getAbsoluteFileURL( dirPath, libPath, dllPath );

    return dllPath;
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

inline sal_Bool isURL( const ::rtl::OUString pathname )
{
    ::rtl::OUString aPreURL( rtl::OUString::createFromAscii( "file:///" ) );
    return ( ( pathname.indexOf( aPreURL ) == 0 ) ? sal_True : sal_False );
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
    CPPUNIT_ASSERT_MESSAGE( "In createTestDirectory Function: creation: ", ( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_EXIST ) );
}

/** delete a temp test directory using OUString name of full qualified URL or system path.
*/
inline void deleteTestDirectory( const ::rtl::OUString dirname )
{
    ::rtl::OUString     aPathURL   = dirname.copy( 0 );
    ::osl::FileBase::RC nError;
    if ( !isURL( dirname ) )
        ::osl::FileBase::getFileURLFromSystemPath( dirname, aPathURL ); //convert if not full qualified URL

    ::osl::Directory testDir( aPathURL );
    if ( testDir.isOpen( ) == sal_True )
    {
            testDir.close( );  //close if still open.
        }

    nError = ::osl::Directory::remove( aPathURL );
     CPPUNIT_ASSERT_MESSAGE( "In deleteTestDirectory function: remove ", ( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT ) );
}

//check if the file exist
inline sal_Bool ifFileExist( const ::rtl::OUString & str )
{
    ::rtl::OUString     aUStr;
    if ( isURL( str ) )
        ::osl::FileBase::getSystemPathFromFileURL( str, aUStr );
    else
        return sal_False;

    ::osl::File strFile( aUStr );
    sal_Bool nError = strFile.open( OpenFlag_Read );
    if ( ::File::E_NOENT == nError )
        return sal_False;
    else{
        strFile.close( );
        return sal_True;
    }
}

/** detete a temp test file using OUString name.
*/
inline void deleteTestFile( const ::rtl::OUString filename )
{
    ::rtl::OUString aPathURL   = filename.copy( 0 );
    ::osl::FileBase::RC nError;

    if ( !isURL( filename ) )
        ::osl::FileBase::getFileURLFromSystemPath( filename, aPathURL ); //convert if not full qualified URL

    nError = ::osl::File::setAttributes( aPathURL, Attribute_GrpWrite| Attribute_OwnWrite| Attribute_OthWrite ); // if readonly, make writtenable.
    CPPUNIT_ASSERT_MESSAGE( "In deleteTestFile Function: set writtenable ", ( ::osl::FileBase::E_None == nError ) || ( ::osl::FileBase::E_NOENT == nError ) );

    nError = ::osl::File::remove( aPathURL );
    CPPUNIT_ASSERT_MESSAGE( "In deleteTestFile Function: remove ", ( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT ) );
}


//------------------------------------------------------------------------
// test code start here
//------------------------------------------------------------------------

namespace osl_Module
{

    /** class and member function that is available for module test :
    */

    class testClass
    {
    public:
        static void myFunc()
        {
            printf("#Sun Microsystem\n");
        };
    };


    /** testing the methods:
        Module();
        Module( const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT);
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void ctors_none( )
        {
            ::osl::Module aMod;
            bRes = aMod.is();

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor without parameter.",
                                    sal_False == bRes  );
        }

        void ctors_name_mode( )
        {
            OUString aFileURL;
            if ( !( bRes = osl::Module::getUrlFromAddress( ( void* ) &::osl_Module::testClass::myFunc, aFileURL )  ) )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module.",  sal_False  );
            }

            ::osl::Module aMod( aFileURL );
            bRes = aMod.is( );
            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with load action.",
                                    sal_True == bRes  );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_name_mode );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctors


    /** testing the methods:
        static sal_Bool getUrlFromAddress(void * addr, ::rtl::OUString & libraryUrl)
    */
    class getUrlFromAddress : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void getUrlFromAddress_001( )
        {
            OUString aFileURL;
            if ( !( bRes = osl::Module::getUrlFromAddress( ( void* ) &::osl_Module::testClass::myFunc, aFileURL )  ) )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module.",  sal_False  );
            }

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test get Module URL from address.",
                                    sal_True == bRes && 0 < aFileURL.lastIndexOf('/')  );
        }

        void getUrlFromAddress_002( )
        {
            ::osl::Module aMod( getDllURL( ) );
            FuncPtr pFunc = ( FuncPtr ) aMod.getSymbol( rtl::OUString::createFromAscii( "firstfunc" ) );

            OUString aFileURL;
            if ( !( bRes = osl::Module::getUrlFromAddress( ( void* )pFunc, aFileURL )  ) )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module.",  sal_False  );
            }
            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load an external library, get its function address and get its URL.",
                                    sal_True == bRes && 0 < aFileURL.lastIndexOf('/') && aFileURL.equalsIgnoreAsciiCase( getDllURL( ) ) );
        }
        CPPUNIT_TEST_SUITE( getUrlFromAddress );
        CPPUNIT_TEST( getUrlFromAddress_001 );
        CPPUNIT_TEST( getUrlFromAddress_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getUrlFromAddress


    /** testing the method:
        sal_Bool SAL_CALL load( const ::rtl::OUString& strModuleName,
                                                 sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT)
    */
    class load : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void load_001( )
        {
            ::osl::Module aMod( getDllURL( ) );
            ::osl::Module aMod1;

            aMod1.load( getDllURL( ) );
            bRes = oslModule(aMod) == oslModule(aMod1);
            aMod.unload( );
            aMod1.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load function should do the same thing as constructor with library name.",
                                    sal_True == bRes  );
        }
        // load lib which is under a CJK directory
        void load_002( )
        {
#ifdef UNX
            //Can not get a CJK directory already exist, so here create one. Perhaps reason is encoding problem.
            ::rtl::OUString aPidDirURL = rtl::OUString::createFromAscii( "file:///tmp/" ) + ::rtl::OUString::valueOf( ( long )getpid( ) );
            ::rtl::OUString aMyDirURL = aPidDirURL + aKname;
            createTestDirectory( aPidDirURL );
            createTestDirectory( aMyDirURL );

            ::rtl::OUString aDLLURL = aMyDirURL + rtl::OUString::createFromAscii( "/libModule_DLL.so" );
            //check if the lib exist.
            //FIXME: if assert condition is false, the case will return, so the directory will not be clean-up
            CPPUNIT_ASSERT_MESSAGE( "#Source file is not exist. please manually clean-up directory and file under /tmp", ifFileExist( getDllURL( ) ) == sal_True );
            ::osl::FileBase::RC nError = ::osl::File::copy( getDllURL( ), aDLLURL );
            CPPUNIT_ASSERT_MESSAGE( "#copy failed. please manually clean-up directory and file under /tmp", nError == ::osl::FileBase::E_None );
            //ifFileExist returned false but the file exist
            CPPUNIT_ASSERT_MESSAGE( "#This file is not exist, copy failed. please manually clean-up directory and file under /tmp", ifFileExist( aDLLURL ) == sal_True );

            //test if can create a normal file
            ::rtl::OUString aFileURL = aMyDirURL + rtl::OUString::createFromAscii( "/test_file" );
            ::osl::File testFile( aFileURL );
            nError = testFile.open( OpenFlag_Create );
            CPPUNIT_ASSERT_MESSAGE( "#create failed. please manually clean-up directory and file under /tmp", nError == ::osl::FileBase::E_None );
            CPPUNIT_ASSERT_MESSAGE( "#This file is not exist, create failed. please manually clean-up directory and file under /tmp", ifFileExist( aFileURL ) == sal_True );

            //load the copied dll
            ::osl::Module aMod( aDLLURL );
            ::osl::Module aMod1;

            sal_Bool bOK = aMod1.load( aDLLURL );
            bRes = oslModule(aMod) == oslModule(aMod1);
            aMod.unload( );
            aMod1.unload( );
            deleteTestFile( aFileURL );
            deleteTestFile( aDLLURL );
            deleteTestDirectory( aMyDirURL );
            deleteTestDirectory( aPidDirURL );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load lib which is under a CJK directory.",
                                    sal_True == bRes && bOK == sal_True );
#endif
        }

        CPPUNIT_TEST_SUITE( load );
        CPPUNIT_TEST( load_001 );
        CPPUNIT_TEST( load_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class load


    /** testing the method:
        void SAL_CALL unload()
    */
    class unload : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void unload_001( )
        {
            ::osl::Module aMod( getDllURL( ) );

            aMod.unload( );
            bRes = oslModule(aMod) ==NULL;

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: unload function should do the same thing as destructor.",
                                    sal_True == bRes  );
        }

        CPPUNIT_TEST_SUITE( unload );
        CPPUNIT_TEST( unload_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class unload


    /** testing the methods:
        sal_Bool SAL_CALL is() const
    */
    class is : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void is_001( )
        {
            OUString aFileURL;
            if ( !( bRes = osl::Module::getUrlFromAddress( ( void* ) &::osl_Module::testClass::myFunc, aFileURL )  ) )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module - using executable instead",  sal_False  );
            }

            ::osl::Module aMod;
            bRes = aMod.is( );
            aMod.load( aFileURL );
            bRes1 = aMod.is( );
            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test if a module is a loaded module.",
                                     sal_False == bRes && sal_True == bRes1);
        }
        CPPUNIT_TEST_SUITE( is );
        CPPUNIT_TEST( is_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class is


    /** testing the methods:
        void* SAL_CALL getSymbol( const ::rtl::OUString& strSymbolName)
    */
    class getSymbol : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes;

        void getSymbol_001( )
        {
            ::osl::Module aMod( getDllURL( ) );
            FuncPtr pFunc = ( FuncPtr ) aMod.getSymbol( rtl::OUString::createFromAscii( "firstfunc" ) );
            bRes = sal_False;
            if ( pFunc )
                bRes = pFunc( bRes );
            aMod.unload();

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load a dll and call one function in it.",
                                     sal_True == bRes );
        }

        CPPUNIT_TEST_SUITE( getSymbol );
        CPPUNIT_TEST( getSymbol_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getSymbol


    /** testing the methods:
        void* SAL_CALL getSymbol( const ::rtl::OUString& strSymbolName)
    */
    class optr_oslModule : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void optr_oslModule_001( )
        {
            ::osl::Module aMod;
            bRes = ( (oslModule)aMod == NULL );

            aMod.load( getDllURL( ) );
            bRes1 = (oslModule)aMod != NULL;

            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: the m_Module of a Module instance will be NULL when is not loaded, it will not be NULL after loaded.",
                                     sal_True == bRes && sal_True == bRes1);
        }

        void optr_oslModule_002( )
        {
            ::osl::Module aMod( getDllURL( ) );
            ::rtl::OUString funcName(::rtl::OUString::createFromAscii( "firstfunc" ) );

            FuncPtr pFunc = ( FuncPtr ) osl_getSymbol( (oslModule)aMod, funcName.pData );
            bRes = sal_False;
            if ( pFunc )
                bRes = pFunc( bRes );

            aMod.unload();

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: use m_Module to call osl_getSymbol() function.",
                                     sal_True == bRes  );
        }

        CPPUNIT_TEST_SUITE( optr_oslModule );
        CPPUNIT_TEST( optr_oslModule_001 );
        CPPUNIT_TEST( optr_oslModule_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class optr_oslModule


// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Module::ctors, "osl_Module");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Module::getUrlFromAddress, "osl_Module");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Module::load, "osl_Module");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Module::unload, "osl_Module");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Module::is, "osl_Module");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Module::getSymbol, "osl_Module");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Module::optr_oslModule, "osl_Module");
// -----------------------------------------------------------------------------

} // namespace osl_Module


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;
