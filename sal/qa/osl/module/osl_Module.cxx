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

#include "osl_Module_Const.h"

using namespace osl;

/** get dll file URL.
*/
static OUString getDllURL()
{
#if defined(_WIN32)        // lib in Unix and lib in Windows are not same in file name.
    OUString libPath( "test_Module_DLL.dll" );
#else
    OUString libPath( "libtest_Module_DLL.so" );
#endif

    OUString dirPath, dllPath;
    osl::Module::getUrlFromAddress(
        reinterpret_cast<oslGenericFunction>(&getDllURL), dirPath);
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') + 1);
    osl::FileBase::getAbsoluteFileURL( dirPath, libPath, dllPath );

    return dllPath;
}

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
        Module( const OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT);
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void ctors_none( )
        {
            ::osl::Module aMod;
            bRes = aMod.is();

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor without parameter.",
                                    !bRes  );
        }

        void ctors_name_mode( )
        {
            OUString aFileURL;
            bRes = osl::Module::getUrlFromAddress(
                reinterpret_cast<oslGenericFunction>(
                    &osl_Module::testClass::myFunc),
                aFileURL);

            if ( !bRes )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module.", false );
            }

            ::osl::Module aMod( aFileURL );
            bRes = aMod.is( );
            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test constructor with load action.",
                                    bRes );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_name_mode );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctors

    /** testing the methods:
        static sal_Bool getUrlFromAddress(void * addr, OUString & libraryUrl)
    */
    class getUrlFromAddress : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void getUrlFromAddress_001( )
        {
            OUString aFileURL;
            bRes = osl::Module::getUrlFromAddress(
                reinterpret_cast<oslGenericFunction>(
                    &osl_Module::testClass::myFunc),
                aFileURL);
            if ( !bRes )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module.", false );
            }

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test get Module URL from address.",
                                    bRes );
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test get Module URL from address.",
                                    0 < aFileURL.lastIndexOf('/')  );
        }

        void getUrlFromAddress_002( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on macOS
            ::osl::Module aMod( getDllURL( ) );
            FuncPtr pFunc = reinterpret_cast<FuncPtr>(aMod.getSymbol( "firstfunc" ));

            OUString aFileURL;
            bRes = osl::Module::getUrlFromAddress(
                reinterpret_cast<oslGenericFunction>(pFunc), aFileURL);
            if ( !bRes )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module.", false );
            }
            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load an external library, get its function address and get its URL.",
                                    bRes );
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load an external library, get its function address and get its URL.",
                                    0 < aFileURL.lastIndexOf('/') );
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load an external library, get its function address and get its URL.",
                                    aFileURL.equalsIgnoreAsciiCase( getDllURL( ) ) );
#endif
        }

        /* tester comments: another case is getFunctionSymbol_001*/

        CPPUNIT_TEST_SUITE( getUrlFromAddress );
        CPPUNIT_TEST( getUrlFromAddress_001 );
        CPPUNIT_TEST( getUrlFromAddress_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getUrlFromAddress

    /** testing the method:
        sal_Bool SAL_CALL load( const OUString& strModuleName,
                                                 sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT)
    */
    class load : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void load_001( )
        {
            ::osl::Module aMod( getDllURL( ) );
            ::osl::Module aMod1;

            aMod1.load( getDllURL( ) );
            bRes = oslModule(aMod) == oslModule(aMod1);
            aMod.unload( );
            aMod1.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load function should do the same thing as constructor with library name.",
                                    bRes );
        }

        CPPUNIT_TEST_SUITE( load );
        CPPUNIT_TEST( load_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class load

    /** testing the method:
        void SAL_CALL unload()
    */
    class unload : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void unload_001( )
        {
            ::osl::Module aMod( getDllURL( ) );

            aMod.unload( );
            bRes = oslModule(aMod) ==nullptr;

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: unload function should do the same thing as destructor.",
                                    bRes );
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
        bool bRes, bRes1;

        void is_001( )
        {
            OUString aFileURL;
            bRes = osl::Module::getUrlFromAddress(
                reinterpret_cast<oslGenericFunction>(
                    osl_Module::testClass::myFunc),
                aFileURL);
            if ( !bRes )
            {
                CPPUNIT_ASSERT_MESSAGE("Cannot locate current module - using executable instead", false );
            }

            ::osl::Module aMod;
            bRes = aMod.is( );
            aMod.load( aFileURL );
            bRes1 = aMod.is( );
            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test if a module is a loaded module.",
                                    !bRes );
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: test if a module is a loaded module.",
                                    bRes1 );
        }
        CPPUNIT_TEST_SUITE( is );
        CPPUNIT_TEST( is_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class is

    /** testing the methods:
        void* SAL_CALL getSymbol( const OUString& strSymbolName)
    */
    class getSymbol : public CppUnit::TestFixture
    {
    public:
        bool bRes;

        void getSymbol_001( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on macOS
            ::osl::Module aMod( getDllURL( ) );
            FuncPtr pFunc = reinterpret_cast<FuncPtr>(aMod.getSymbol( "firstfunc" ));
            bRes = false;
            if ( pFunc )
                bRes = pFunc( bRes );
            aMod.unload();

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load a dll and call one function in it.",
                                    bRes );
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
        bool bRes, bRes1;

        void optr_oslModule_001( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on macOS
            ::osl::Module aMod;
            bRes = ( static_cast<oslModule>(aMod) == nullptr );

            aMod.load( getDllURL( ) );
            bRes1 = static_cast<oslModule>(aMod) != nullptr;

            aMod.unload( );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: the m_Module of a Module instance will be NULL when is not loaded, it will not be NULL after loaded.",
                                    bRes );
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: the m_Module of a Module instance will be NULL when is not loaded, it will not be NULL after loaded.",
                                    bRes1 );
#endif
        }

        void optr_oslModule_002( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on macOS
            ::osl::Module aMod( getDllURL( ) );
            OUString funcName( "firstfunc" );

            FuncPtr pFunc = reinterpret_cast<FuncPtr>(osl_getSymbol( static_cast<oslModule>(aMod), funcName.pData ));
            bRes = false;
            if ( pFunc )
                bRes = pFunc( bRes );

            aMod.unload();

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: use m_Module to call osl_getSymbol() function.",
                                    bRes );
#endif
        }

        CPPUNIT_TEST_SUITE( optr_oslModule );
        CPPUNIT_TEST( optr_oslModule_001 );
        CPPUNIT_TEST( optr_oslModule_002 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class optr_oslModule

    /** testing the methods:
        oslGenericFunction SAL_CALL getFunctionSymbol( const OUString& ustrFunctionSymbolName )
    */
    class getFunctionSymbol : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void getFunctionSymbol_001( )
        {
#if !defined( MACOSX )
            // TODO: Find out why this fails on macOS
            ::osl::Module aMod( getDllURL( ) );
            oslGenericFunction oslFunc = aMod.getFunctionSymbol( "firstfunc" );
            OUString aLibraryURL;
            bRes = ::osl::Module::getUrlFromAddress( oslFunc, aLibraryURL);
            aMod.unload();
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load a dll and get its function addr and get its URL.",
                 bRes );
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: load a dll and get its function addr and get its URL.",
                 aLibraryURL.equalsIgnoreAsciiCase( getDllURL() ) );
#endif
        }

        CPPUNIT_TEST_SUITE( getFunctionSymbol );
        CPPUNIT_TEST( getFunctionSymbol_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getFunctionSymbol

CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::getUrlFromAddress);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::load);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::unload);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::is);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::getSymbol);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::optr_oslModule);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Module::getFunctionSymbol);

} // namespace osl_Module

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
