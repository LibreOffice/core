/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------
#include <osl_Module_Const.h>

using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;
//------------------------------------------------------------------------
// helper functions and classes
//------------------------------------------------------------------------

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf("#printBool# " );
    ( sal_True == bOk ) ? printf( "TRUE!\n" )
                        : printf( "FALSE!\n" );
}

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf("#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", aString.getStr( ) );
}

/** get dll file URL.
*/
inline ::rtl::OUString getDllURL( void )
{
#if ( defined WNT )        // lib in Unix and lib in Windows are not same in file name.
    ::rtl::OUString libPath( RTL_CONSTASCII_USTRINGPARAM("Module_DLL.dll") );
#else
    ::rtl::OUString libPath( RTL_CONSTASCII_USTRINGPARAM("libModule_DLL.so") );
#endif

    ::rtl::OUString dirPath, dllPath;
    osl::Module::getUrlFromAddress( ( void* ) &getDllURL, dirPath );
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') + 1);
    osl::FileBase::getAbsoluteFileURL( dirPath, libPath, dllPath );

    return dllPath;
}

inline sal_Bool isURL( const ::rtl::OUString pathname )
{
    ::rtl::OUString aPreURL( RTL_CONSTASCII_USTRINGPARAM("file:///") );
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
    ::osl::FileBase::RC nError = strFile.open( osl_File_OpenFlag_Read );
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
            bRes = osl::Module::getUrlFromAddress( ( void* ) &::osl_Module::testClass::myFunc, aFileURL );

            if ( !( bRes ) )
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
            bRes = osl::Module::getUrlFromAddress( ( void* ) &::osl_Module::testClass::myFunc, aFileURL ) ;
            if ( !( bRes ) )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module.",  sal_False  );
            }

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test get Module URL from address.",
                                    sal_True == bRes && 0 < aFileURL.lastIndexOf('/')  );
        }

        void getUrlFromAddress_002( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on Mac OS X
            ::osl::Module aMod( getDllURL( ) );
            FuncPtr pFunc = ( FuncPtr ) aMod.getSymbol( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("firstfunc")) );

            OUString aFileURL;
            bRes = osl::Module::getUrlFromAddress( ( void* )pFunc, aFileURL );
            if ( !( bRes  ) )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module.",  sal_False  );
            }
            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load an external library, get its function address and get its URL.",
                                    sal_True == bRes && 0 < aFileURL.lastIndexOf('/') && aFileURL.equalsIgnoreAsciiCase( getDllURL( ) ) );
#endif
        }

        /* tester comments: another case is getFunctionSymbol_001*/

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
#if defined( UNX ) && !defined( MACOSX )
            // TODO: Find out why this fails on Mac OS X
            //Can not get a CJK directory already exist, so here create one. Perhaps reason is encoding problem.
            ::rtl::OUString aPidDirURL = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///tmp/")) + ::rtl::OUString::valueOf( ( long )getpid( ) );
            ::rtl::OUString aMyDirURL = aPidDirURL + aKname;
            createTestDirectory( aPidDirURL );
            createTestDirectory( aMyDirURL );

            ::rtl::OUString aDLLURL = aMyDirURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/libModule_DLL.so"));
            //check if the lib exist.
            //FIXME: if assert condition is false, the case will return, so the directory will not be clean-up
            CPPUNIT_ASSERT_MESSAGE( "#Source file is not exist. please manually clean-up directory and file under /tmp", ifFileExist( getDllURL( ) ) == sal_True );
            ::osl::FileBase::RC nError = ::osl::File::copy( getDllURL( ), aDLLURL );
            CPPUNIT_ASSERT_MESSAGE( "#copy failed. please manually clean-up directory and file under /tmp", nError == ::osl::FileBase::E_None );
            //ifFileExist returned false but the file exist
            CPPUNIT_ASSERT_MESSAGE( "#This file is not exist, copy failed. please manually clean-up directory and file under /tmp", ifFileExist( aDLLURL ) == sal_True );

            //test if can create a normal file
            ::rtl::OUString aFileURL = aMyDirURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/test_file"));
            ::osl::File testFile( aFileURL );
            nError = testFile.open( osl_File_OpenFlag_Create );
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
            bRes = osl::Module::getUrlFromAddress( ( void* ) &::osl_Module::testClass::myFunc, aFileURL );
            if ( !( bRes  ) )
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
#if !defined( MACOSX )
            // TODO: Find out why this fails on Mac OS X
            ::osl::Module aMod( getDllURL( ) );
            FuncPtr pFunc = ( FuncPtr ) aMod.getSymbol( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("firstfunc")) );
            bRes = sal_False;
            if ( pFunc )
                bRes = pFunc( bRes );
            aMod.unload();

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load a dll and call one function in it.",
                                     sal_True == bRes );
#endif
        }

        CPPUNIT_TEST_SUITE( getSymbol );
        CPPUNIT_TEST( getSymbol_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getSymbol


    /** testing the methods:
        operator oslModule() const
    */
    class optr_oslModule : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void optr_oslModule_001( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on Mac OS X
            ::osl::Module aMod;
            bRes = ( (oslModule)aMod == NULL );

            aMod.load( getDllURL( ) );
            bRes1 = (oslModule)aMod != NULL;

            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: the m_Module of a Module instance will be NULL when is not loaded, it will not be NULL after loaded.",
                                     sal_True == bRes && sal_True == bRes1);
#endif
        }

        void optr_oslModule_002( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on Mac OS X
            ::osl::Module aMod( getDllURL( ) );
            ::rtl::OUString funcName( RTL_CONSTASCII_USTRINGPARAM("firstfunc") );

            FuncPtr pFunc = ( FuncPtr ) osl_getSymbol( (oslModule)aMod, funcName.pData );
            bRes = sal_False;
            if ( pFunc )
                bRes = pFunc( bRes );

            aMod.unload();

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: use m_Module to call osl_getSymbol() function.",
                                     sal_True == bRes  );
#endif
        }

        CPPUNIT_TEST_SUITE( optr_oslModule );
        CPPUNIT_TEST( optr_oslModule_001 );
        CPPUNIT_TEST( optr_oslModule_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class optr_oslModule

    /** testing the methods:
        oslGenericFunction SAL_CALL getFunctionSymbol( const ::rtl::OUString& ustrFunctionSymbolName )
    */
    class getFunctionSymbol : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void getFunctionSymbol_001( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on Mac OS X
            ::osl::Module aMod( getDllURL( ) );
            oslGenericFunction oslFunc = aMod.getFunctionSymbol( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("firstfunc")) );
            ::rtl::OUString aLibraryURL;
            bRes = ::osl::Module::getUrlFromAddress( oslFunc, aLibraryURL);
            aMod.unload();
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load a dll and get its function addr and get its URL.",
                 sal_True == bRes && aLibraryURL.equalsIgnoreAsciiCase( getDllURL() ) );
#endif
        }

        CPPUNIT_TEST_SUITE( getFunctionSymbol );
        CPPUNIT_TEST( getFunctionSymbol_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getFunctionSymbol

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::getUrlFromAddress);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::load);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::unload);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::is);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::getSymbol);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::optr_oslModule);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::getFunctionSymbol);
// -----------------------------------------------------------------------------

} // namespace osl_Module

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
