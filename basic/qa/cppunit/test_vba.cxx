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

using namespace ::com::sun::star;

namespace
{


    class VBATest : public test::BootstrapFixture
    {
        public:
        VBATest() : BootstrapFixture(true, false) {}
        ~VBATest(){}
        void testMiscVBAFunctions();
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(VBATest);

        // Declares the method as a test to call
        CPPUNIT_TEST(testMiscVBAFunctions);
        //CPPUNIT_TEST(testOle);

        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();

    };

void VBATest::testMiscVBAFunctions()
{
    const char* macroSource[] = {
        "bytearraystring.vb",
#if 1// FIXED // datevalue test seems to depend on both locale and language
          // settings, should try and rewrite the test to deal with that
        "datevalue.vb",
#endif
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
        CPPUNIT_ASSERT_MESSAGE("Result not as expected", pReturn->GetOUString() == rtl::OUString("OK") );
    }
}

  // Put the test suite in the registry

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(VBATest);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
