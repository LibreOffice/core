/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "basictest.hxx"
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
using namespace ::com::sun::star;

namespace
{


    class VBATest : public test::BootstrapFixture
    {
        bool hasOLEEnv();
        public:
        VBATest() : BootstrapFixture(true, false) {}
        ~VBATest(){}
        void testMiscVBAFunctions();
        void testObjAssignWithDefaultMember();
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(VBATest);

        // Declares the method as a test to call
        CPPUNIT_TEST(testMiscVBAFunctions);
// not much point even trying to run except on windows
#if defined(WNT)
        CPPUNIT_TEST(testObjAssignWithDefaultMember);
#endif

        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();

    };

bool VBATest::hasOLEEnv()
{
    // test if we have the necessary runtime environment
    // to run the OLE tests.
    static uno::Reference< lang::XMultiServiceFactory > xOLEFactory;
    if ( !xOLEFactory.is() )
    {
        uno::Reference< uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        if( xContext.is() )
        {
            uno::Reference<lang::XMultiComponentFactory> xSMgr = xContext->getServiceManager();
            xOLEFactory = uno::Reference<lang::XMultiServiceFactory>(
                xSMgr->createInstanceWithContext(
                    "com.sun.star.bridge.OleObjectFactory",
                        xContext ), uno::UNO_QUERY );
        }
    }
    bool bOk = false;
    if( xOLEFactory.is() )
    {
        uno::Reference< uno::XInterface > xExcel = xOLEFactory->createInstance( "Excel.Application" );
        uno::Reference< uno::XInterface > xADODB = xOLEFactory->createInstance( "ADODB.Connection" );
       bOk = xExcel.is() && xADODB.is();
    }
    return bOk;
}

void VBATest::testMiscVBAFunctions()
{
    const char* macroSource[] = {
        "bytearraystring.vb",
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
        "stringplusdouble.vb"
    };
    OUString sMacroPathURL = getURLFromSrc("/basic/qa/vba_tests/");
    // Some test data expects the uk locale
    AllSettings aSettings = Application::GetSettings();
    aSettings.SetLanguageTag( LanguageTag( LANGUAGE_ENGLISH_UK ) );
    Application::SetSettings( aSettings );
    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( macroSource ); ++i )
    {
        OUString sMacroURL( sMacroPathURL );
        sMacroURL += OUString::createFromAscii( macroSource[ i ] );

        MacroSnippet myMacro;
        myMacro.LoadSourceFromFile( sMacroURL );
        SbxVariableRef pReturn = myMacro.Run();
        if ( pReturn )
        {
            fprintf(stderr, "macro result for %s\n", macroSource[ i ] );
            fprintf(stderr, "macro returned:\n%s\n", OUStringToOString( pReturn->GetOUString(), RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        CPPUNIT_ASSERT_MESSAGE("No return variable huh?", pReturn != NULL );
        CPPUNIT_ASSERT_MESSAGE("Result not as expected", pReturn->GetOUString() == "OK" );
    }
}

void VBATest::testObjAssignWithDefaultMember()
{
    bool bCanRunOleTests = hasOLEEnv();
    if ( !bCanRunOleTests )
        return; // can't do anything, skip test

    const char* macroSource[] = {
        "ole_ObjAssignNoDflt.vb",
        "ole_ObjAssignToNothing.vb",
    };

    OUString sMacroPathURL = getURLFromSrc("/basic/qa/vba_tests/");

    uno::Sequence< uno::Any > aArgs(1);
    // path to test document
    OUString sPath = getPathFromSrc("/basic/qa/vba_tests/data/");
    sPath += OUString( "ADODBdata.xls" );
    sPath = sPath.replaceAll( "/", "\\" );

    aArgs[ 0 ] = uno::makeAny( sPath );

    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( macroSource ); ++i )
    {
        OUString sMacroURL( sMacroPathURL );
        sMacroURL += OUString::createFromAscii( macroSource[ i ] );
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
}

  // Put the test suite in the registry

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(VBATest);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
