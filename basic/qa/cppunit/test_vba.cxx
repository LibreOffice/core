/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "basictest.hxx"
#include <comphelper/processfactory.hxx>
#include <unotools/syslocaleoptions.hxx>

#ifdef WIN32
#include <string.h>

#include <windows.h>
#include <odbcinst.h>
#endif

using namespace ::com::sun::star;

namespace
{
    class VBATest : public test::BootstrapFixture
    {
        public:
        VBATest() : BootstrapFixture(true, false) {}
        virtual ~VBATest(){}
        void testMiscVBAFunctions();
        void testMiscOLEStuff();
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(VBATest);

        // Declares the method as a test to call
        CPPUNIT_TEST(testMiscVBAFunctions);
        CPPUNIT_TEST(testMiscOLEStuff);

        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();

    };

void VBATest::testMiscVBAFunctions()
{
    const char* macroSource[] = {
        "bytearraystring.vb",
        "cdec.vb",
// datevalue test seems to depend on both locale and language
// settings, should try and rewrite the test to deal with that
// for some reason tinderboxes don't seem to complain leaving enabled
// for the moment
        "datevalue.vb",
        "partition.vb",
        "strconv.vb",
        "dateserial.vb",
        "format.vb",
        "replace.vb",
        "stringplusdouble.vb",
#ifndef WIN32 // missing 64bit Currency marshalling.
        "win32compat.vb", // windows compatibility hooks.
#endif
        "win32compatb.vb" // same methods, different signatures.
    };
    OUString sMacroPathURL = m_directories.getURLFromSrc("/basic/qa/vba_tests/");
    // Some test data expects the uk locale
    LanguageTag aLocale(LANGUAGE_ENGLISH_UK);
    SvtSysLocaleOptions aLocalOptions;
    aLocalOptions.SetLocaleConfigString( aLocale.getBcp47() );

    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( macroSource ); ++i )
    {
        OUString sMacroURL = sMacroPathURL
                           + OUString::createFromAscii( macroSource[ i ] );

        MacroSnippet myMacro;
        myMacro.LoadSourceFromFile( sMacroURL );
        SbxVariableRef pReturn = myMacro.Run();
        if ( pReturn )
        {
            fprintf(stderr, "macro result for %s\n", macroSource[ i ] );
            fprintf(stderr, "macro returned:\n%s\n", OUStringToOString( pReturn->GetOUString(), RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        CPPUNIT_ASSERT_MESSAGE("No return variable huh?", pReturn != nullptr );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Result not as expected", OUString("OK"), pReturn->GetOUString() );
    }
}

void VBATest::testMiscOLEStuff()
{
// not much point even trying to run except on windows
#if defined(_WIN32)
    // test if we have the necessary runtime environment
    // to run the OLE tests.
    uno::Reference< lang::XMultiServiceFactory > xOLEFactory;
    uno::Reference< uno::XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    if( xContext.is() )
    {
        uno::Reference<lang::XMultiComponentFactory> xSMgr = xContext->getServiceManager();
        xOLEFactory.set( xSMgr->createInstanceWithContext( "com.sun.star.bridge.OleObjectFactory", xContext ),
                         uno::UNO_QUERY );
    }
    bool bOk = false;
    if( xOLEFactory.is() )
    {
        uno::Reference< uno::XInterface > xADODB = xOLEFactory->createInstance( "ADODB.Connection" );
        bOk = xADODB.is();
    }
    if ( !bOk )
        return; // can't do anything, skip test

    sal_Unicode sBuf[1024*4];
    SQLGetInstalledDriversW( sBuf, sizeof( sBuf ), nullptr );

    const sal_Unicode *pODBCDriverName = sBuf;
    bool bFound = false;
    for (; wcslen( pODBCDriverName ) != 0; pODBCDriverName += wcslen( pODBCDriverName ) + 1 ) {
        if ( wcsstr( pODBCDriverName, L"Microsoft Excel Driver" ) != nullptr ) {
            bFound = true;
            break;
        }
    }
    if ( !bFound )
        return; // can't find ODBC driver needed test, so skip test

    const char* macroSource[] = {
        "ole_ObjAssignNoDflt.vb",
        "ole_ObjAssignToNothing.vb",
        "ole_dfltObjDflMethod.vb",
    };

    OUString sMacroPathURL = m_directories.getURLFromSrc("/basic/qa/vba_tests/");

    uno::Sequence< uno::Any > aArgs(1);
    // path to test document
    OUString sPath = m_directories.getPathFromSrc("/basic/qa/vba_tests/data/")
                   + "ADODBdata.xls";
    sPath = sPath.replaceAll( "/", "\\" );

    aArgs[ 0 ] = uno::makeAny( sPath );

    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( macroSource ); ++i )
    {
        OUString sMacroURL = sMacroPathURL
                           + OUString::createFromAscii( macroSource[ i ] );
        MacroSnippet myMacro;
        myMacro.LoadSourceFromFile( sMacroURL );
        SbxVariableRef pReturn = myMacro.Run( aArgs );
        if ( pReturn )
        {
            fprintf(stderr, "macro result for %s\n", macroSource[ i ] );
            fprintf(stderr, "macro returned:\n%s\n", OUStringToOString( pReturn->GetOUString(), RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        CPPUNIT_ASSERT_MESSAGE("No return variable huh?", pReturn != NULL );
        CPPUNIT_ASSERT_MESSAGE("Result not as expected", pReturn->GetOUString() == "OK" );
    }
#else
    // Avoid "this method is empty and should be removed" warning
    (void) 42;
#endif
}

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(VBATest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
