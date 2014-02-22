/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */
#include "basictest.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <comphelper/processfactory.hxx>
using namespace ::com::sun::star;

namespace
{


    class VBATest : public test::BootstrapFixture
    {
        public:
        VBATest() : BootstrapFixture(true, false) {}
        ~VBATest(){}
        void testMiscVBAFunctions();
        void testMiscOLEStuff();
        
        CPPUNIT_TEST_SUITE(VBATest);

        
        CPPUNIT_TEST(testMiscVBAFunctions);
        CPPUNIT_TEST(testMiscOLEStuff);

        
        CPPUNIT_TEST_SUITE_END();

    };

void VBATest::testMiscVBAFunctions()
{
    const char* macroSource[] = {
        "bytearraystring.vb",




        "datevalue.vb",
        "partition.vb",
        "strconv.vb",
        "dateserial.vb",
        "format.vb",
        "replace.vb",
        "stringplusdouble.vb"
    };
    OUString sMacroPathURL = getURLFromSrc("/basic/qa/vba_tests/");
    
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

void VBATest::testMiscOLEStuff()
{

#if defined(WNT)
    
    
    uno::Reference< lang::XMultiServiceFactory > xOLEFactory;
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
    bool bOk = false;
    if( xOLEFactory.is() )
    {
        uno::Reference< uno::XInterface > xExcel = xOLEFactory->createInstance( "Excel.Application" );
        uno::Reference< uno::XInterface > xADODB = xOLEFactory->createInstance( "ADODB.Connection" );
       bOk = xExcel.is() && xADODB.is();
    }
    if ( !bOk )
        return; 

    const char* macroSource[] = {
        "ole_ObjAssignNoDflt.vb",
        "ole_ObjAssignToNothing.vb",
        "ole_dfltObjDflMethod.vb",
    };

    OUString sMacroPathURL = getURLFromSrc("/basic/qa/vba_tests/");

    uno::Sequence< uno::Any > aArgs(1);
    
    OUString sPath = getPathFromSrc("/basic/qa/vba_tests/data/");
    sPath += "ADODBdata.xls";
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
#endif
}

  

  
  CPPUNIT_TEST_SUITE_REGISTRATION(VBATest);
} 
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
