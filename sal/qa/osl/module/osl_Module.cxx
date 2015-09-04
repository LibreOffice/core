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
#include <osl_Module_Const.h>
#include "gtest/gtest.h"

using namespace osl;
using namespace rtl;


//------------------------------------------------------------------------
// helper functions and classes
//------------------------------------------------------------------------

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf("#printBool# " );
    ( sal_True == bOk ) ? printf("TRUE!\n" ): printf("FALSE!\n" );
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
    ::rtl::OUString libPath( rtl::OUString::createFromAscii( "Module_DLL.dll" ) );
#else
    ::rtl::OUString libPath( rtl::OUString::createFromAscii( "libModule_DLL.so" ) );
#endif

    ::rtl::OUString dirPath, dllPath;
    osl::Module::getUrlFromAddress( ( void* ) &getDllURL, dirPath );
    // file:///aoo/main/sal/unx/bin/osl_Module
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/'));
    // file:///aoo/main/sal/unx/bin
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') + 1);
    // file:///aoo/main/sal/unx/
    dirPath = dirPath + rtl::OUString::createFromAscii( "lib/" );
    // file:///aoo/main/sal/unx/lib/
    osl::FileBase::getAbsoluteFileURL( dirPath, libPath, dllPath );

rtl::OString aOString = ::rtl::OUStringToOString (dllPath, RTL_TEXTENCODING_UTF8);
printf("getDllURL() returning %s\n", aOString.getStr());
    return dllPath;
}

/** print a UNI_CODE file name.
*/
inline void printFileName( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf("#printFileName_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", aString.getStr( ) );
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
    ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_EXIST )) << "In createTestDirectory Function: creation: ";
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
     ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT )) << "In deleteTestDirectory function: remove ";
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
    ::osl::FileBase::RC nError = strFile.open( OpenFlag_Read );
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
    ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) || ( ::osl::FileBase::E_NOENT == nError )) << "In deleteTestFile Function: set writtenable ";

    nError = ::osl::File::remove( aPathURL );
    ASSERT_TRUE(( ::osl::FileBase::E_None == nError ) || ( nError == ::osl::FileBase::E_NOENT )) << "In deleteTestFile Function: remove ";
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
    class ctors : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class ctors


    TEST_F(ctors, ctors_none)
    {
        ::osl::Module aMod;
        bRes = aMod.is();

        ASSERT_TRUE( sal_False == bRes ) << "#test comment#: test constructor without parameter.";
    }

    TEST_F(ctors, ctors_name_mode)
    {
        ::osl::Module aMod( getDllURL( ) );
        bRes = aMod.is( );
        aMod.unload( );

        ASSERT_TRUE( sal_True == bRes ) << "#test comment#: test constructor with load action.";
    }

    /** testing the methods:
        static sal_Bool getUrlFromAddress(void * addr, ::rtl::OUString & libraryUrl)
    */
    class getUrlFromAddress : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class getUrlFromAddress

    /* tester comments: another case is getFunctionSymbol_001*/

    TEST_F(getUrlFromAddress, getUrlFromAddress_001 )
    {
        OUString aFileURL;
        bRes = osl::Module::getUrlFromAddress( ( void* ) &::osl_Module::testClass::myFunc, aFileURL ) ;
        if ( !( bRes ) )
        {
            ASSERT_TRUE(sal_False) << "Cannot locate current module.";
        }

        ASSERT_TRUE(sal_True == bRes && 0 < aFileURL.lastIndexOf('/')) << "#test comment#: test get Module URL from address.";
    }

    TEST_F(getUrlFromAddress, getUrlFromAddress_002 )
    {
        ::osl::Module aMod( getDllURL( ) );
        FuncPtr pFunc = ( FuncPtr ) aMod.getSymbol( rtl::OUString::createFromAscii( "firstfunc" ) );

        OUString aFileURL;
        bRes = osl::Module::getUrlFromAddress( ( void* )pFunc, aFileURL );
        if ( !( bRes  ) )
        {
            ASSERT_TRUE(sal_False) << "Cannot locate current module.";
        }
        aMod.unload( );

        ASSERT_TRUE( sal_True == bRes && 0 < aFileURL.lastIndexOf('/') && aFileURL.equalsIgnoreAsciiCase( getDllURL( ) ) )
            << "#test comment#: load an external library, get its function address and get its URL.";
    }

    /** testing the method:
        sal_Bool SAL_CALL load( const ::rtl::OUString& strModuleName,
                                                 sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT)
    */
    class load : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class load

    TEST_F(load, load_001 )
    {
        ::osl::Module aMod( getDllURL( ) );
        ::osl::Module aMod1;

        aMod1.load( getDllURL( ) );
        bRes = oslModule(aMod) == oslModule(aMod1);
        aMod.unload( );
        aMod1.unload( );

        ASSERT_TRUE(sal_True == bRes) << "#test comment#: load function should do the same thing as constructor with library name.";
    }
    // load lib which is under a CJK directory
    TEST_F(load, load_002 )
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
        ASSERT_TRUE(ifFileExist( getDllURL( ) ) == sal_True) << "#Source file is not exist. please manually clean-up directory and file under /tmp";
        ::osl::FileBase::RC nError = ::osl::File::copy( getDllURL( ), aDLLURL );
        ASSERT_TRUE(nError == ::osl::FileBase::E_None) << "#copy failed. please manually clean-up directory and file under /tmp";
        //ifFileExist returned false but the file exist
        ASSERT_TRUE( ifFileExist( aDLLURL ) == sal_True )
            << "#This file is not exist, copy failed. please manually clean-up directory and file under /tmp";

        //test if can create a normal file
        ::rtl::OUString aFileURL = aMyDirURL + rtl::OUString::createFromAscii( "/test_file" );
        ::osl::File testFile( aFileURL );
        nError = testFile.open( OpenFlag_Create );
        ASSERT_TRUE(nError == ::osl::FileBase::E_None) << "#create failed. please manually clean-up directory and file under /tmp";
        ASSERT_TRUE( ifFileExist( aFileURL ) == sal_True )
            << "#This file is not exist, create failed. please manually clean-up directory and file under /tmp";

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

        ASSERT_TRUE(sal_True == bRes && bOK == sal_True) << "#test comment#: load lib which is under a CJK directory.";
#endif
    }

    /** testing the method:
        void SAL_CALL unload()
    */
    class unload : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class unload

    TEST_F(unload, unload_001)
    {
        ::osl::Module aMod( getDllURL( ) );

        aMod.unload( );
        bRes = oslModule(aMod) ==NULL;

        ASSERT_TRUE(sal_True == bRes) << "#test comment#: unload function should do the same thing as destructor.";
    }

    /** testing the methods:
        sal_Bool SAL_CALL is() const
    */
    class is : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class is

    TEST_F(is, is_001)
    {
        ::osl::Module aMod;
        bRes = aMod.is( );
        aMod.load( getDllURL( ) );
        bRes1 = aMod.is( );
        aMod.unload( );

        ASSERT_TRUE(sal_False == bRes && sal_True == bRes1) << "#test comment#: test if a module is a loaded module.";
    }

    /** testing the methods:
        void* SAL_CALL getSymbol( const ::rtl::OUString& strSymbolName)
    */
    class getSymbol : public ::testing::Test
    {
    public:
        sal_Bool bRes;
    }; // class getSymbol

    TEST_F(getSymbol, getSymbol_001)
    {
        ::osl::Module aMod( getDllURL( ) );
        FuncPtr pFunc = ( FuncPtr ) aMod.getSymbol( rtl::OUString::createFromAscii( "firstfunc" ) );
        bRes = sal_False;
        if ( pFunc )
            bRes = pFunc( bRes );
        aMod.unload();

        ASSERT_TRUE(sal_True == bRes) << "#test comment#: load a dll and call one function in it.";
    }

    /** testing the methods:
        operator oslModule() const
    */
    class optr_oslModule : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class optr_oslModule

    TEST_F(optr_oslModule, optr_oslModule_001 )
    {
        ::osl::Module aMod;
        bRes = ( (oslModule)aMod == NULL );

        aMod.load( getDllURL( ) );
        bRes1 = (oslModule)aMod != NULL;

        aMod.unload( );

        ASSERT_TRUE(sal_True == bRes && sal_True == bRes1)
            << "#test comment#: the m_Module of a Module instance will be NULL when is not loaded, it will not be NULL after loaded.";
    }

    TEST_F(optr_oslModule, optr_oslModule_002 )
    {
        ::osl::Module aMod( getDllURL( ) );
        ::rtl::OUString funcName(::rtl::OUString::createFromAscii( "firstfunc" ) );

        FuncPtr pFunc = ( FuncPtr ) osl_getSymbol( (oslModule)aMod, funcName.pData );
        bRes = sal_False;
        if ( pFunc )
            bRes = pFunc( bRes );

        aMod.unload();

        ASSERT_TRUE(sal_True == bRes) << "#test comment#: use m_Module to call osl_getSymbol() function.";
    }

    /** testing the methods:
        oslGenericFunction SAL_CALL getFunctionSymbol( const ::rtl::OUString& ustrFunctionSymbolName )
    */
    class getFunctionSymbol : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class getFunctionSymbol

    TEST_F(getFunctionSymbol, getFunctionSymbol_001)
    {
        ::osl::Module aMod( getDllURL( ) );
        oslGenericFunction oslFunc = aMod.getFunctionSymbol( rtl::OUString::createFromAscii( "firstfunc" ) );
        ::rtl::OUString aLibraryURL;
        bRes = ::osl::Module::getUrlFromAddress( oslFunc, aLibraryURL);
        aMod.unload();
        printFileName( aLibraryURL );

        ASSERT_TRUE(sal_True == bRes && aLibraryURL.equalsIgnoreAsciiCase( getDllURL() )) << "#test comment#: load a dll and get its function addr and get its URL.";
    }

} // namespace osl_Module

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
