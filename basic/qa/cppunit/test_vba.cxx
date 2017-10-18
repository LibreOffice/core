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
#include <o3tl/char16_t2wchar_t.hxx>

#ifdef _WIN32
#include <string.h>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
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
        void testMiscVBAFunctions();
#ifdef _WIN32
        void testMiscOLEStuff();
#endif
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(VBATest);

        // Declares the method as a test to call
        CPPUNIT_TEST(testMiscVBAFunctions);
#ifdef _WIN32
        CPPUNIT_TEST(testMiscOLEStuff);
#endif

        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();

    };

void VBATest::testMiscVBAFunctions()
{
    const char* macroSource[] = {
        "bytearraystring.vb",
#ifdef _WIN32
        "cdec.vb", // currently CDec is implemented only on Windows
#endif
        "constants.vb",
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
        "chr.vb",
        "abs.vb",
        "array.vb",
        "asc.vb",
        "atn.vb",
        "cbool.vb",
        "cdate.vb",
        "cdbl.vb",
        "choose.vb",
        "cos.vb",
        "cint.vb",
        "clng.vb",
        "csng.vb",
        "cstr.vb",
        "cvdate.vb",
        "cverr.vb",
        "dateadd.vb",
        "datediff.vb",
        "datepart.vb",
        "day.vb",
        "error.vb",
        "exp.vb",
        "fix.vb",
        "hex.vb",
        "hour.vb",
        "formatnumber.vb",
        "iif.vb",
        "instr.vb",
        "instrrev.vb",
        "int.vb",
        "iserror.vb",
        "ismissing.vb",
        "isnull.vb",
        "isobject.vb",
        "join.vb",
        "lbound.vb",
        "isarray.vb",
        "isdate.vb",
        "isempty.vb",
        "isnumeric.vb",
        "lcase.vb",
        "left.vb",
        "len.vb",
        "log.vb",
        "ltrim.vb",
        "mid.vb",
        "minute.vb",
        "mirr.vb",
        "month.vb",
        "monthname.vb",
        "oct.vb",
        "nper.vb",
        "npv.vb",
        "pmt.vb",
        "ppmt.vb",
        "pv.vb",
        "qbcolor.vb",
        "rate.vb",
        "rgb.vb",
        "rtrim.vb",
        "right.vb",
        "second.vb",
        "sgn.vb",
        "sin.vb",
        "sln.vb",
        "space.vb",
        "sqr.vb",
        "str.vb",
        "strcomp.vb",
        "string.vb",
        "strreverse.vb",
        "switch.vb",
        "syd.vb",
        "timeserial.vb",
        "timevalue.vb",
        "trim.vb",
        "typename.vb",
        "ubound.vb",
        "ucase.vb",
        "val.vb",
        "vartype.vb",
        "weekday.vb",
        "weekdayname.vb",
        "year.vb",
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
        if ( pReturn.is() )
        {
            fprintf(stderr, "macro result for %s\n", macroSource[ i ] );
            fprintf(stderr, "macro returned:\n%s\n", OUStringToOString( pReturn->GetOUString(), RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        CPPUNIT_ASSERT_MESSAGE("No return variable huh?", pReturn.get() != nullptr );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Result not as expected", OUString("OK"), pReturn->GetOUString() );
    }
}

// Not much point even trying to run except on Windows.
// (Without Excel doesn't really do anything anyway,
#ifdef _WIN32
static bool IsIncompatibleExcelJetDriver( const wchar_t *pODBCDriverName )
{
    ULONG nError;
    HKEY hKey;

    if ( wcscmp( pODBCDriverName, L"Microsoft Excel Driver (*.xls)" ) )
        return false;

    // All lookup failures will result in a aborted test!

    // Find registered Jet Excel DLL in registry
    std::wstring strValueName( L"SOFTWARE\\Microsoft\\Jet\\4.0\\Engines\\Excel" );
    nError = RegOpenKeyExW( HKEY_LOCAL_MACHINE, strValueName.c_str(),
                            0, KEY_QUERY_VALUE, &hKey );
    if( ERROR_SUCCESS != nError )
        return true;

    wchar_t dll_name[ 1024 ];
    DWORD dll_name_size = sizeof( dll_name );
    nError = RegQueryValueEx( hKey, "Win32", NULL, NULL,
                              reinterpret_cast<LPBYTE>( dll_name ) , &dll_name_size );
    RegCloseKey( hKey );
    if( ERROR_SUCCESS != nError )
        return true;
    dll_name[ dll_name_size ] = '\0';

    TCHAR dll_name_expanded[ 1024 ];
    if ( !ExpandEnvironmentStrings( reinterpret_cast<LPCTSTR>( dll_name ),
             dll_name_expanded, sizeof( dll_name_expanded ) - 2 ) )
        return true;

    // Get DLL version
    DWORD   verHandle     = 0;
    UINT    size          = 0;
    LPBYTE  lpBuffer      = NULL;
    LPCTSTR szVersionFile = reinterpret_cast<LPCTSTR>( dll_name_expanded );

    DWORD verSize = GetFileVersionInfoSize( szVersionFile, &verHandle );
    if ( 0 == verSize )
        return true;

    LPSTR verData = static_cast<LPSTR>( alloca( verSize ) );
    if( !GetFileVersionInfo( szVersionFile, verHandle, verSize, verData) )
        return true;
    if( !VerQueryValue( verData, "\\", (VOID FAR* FAR*) &lpBuffer, &size) || !size )
        return true;

    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *) lpBuffer;
    if( 0xfeef04bd != verInfo->dwSignature )
        return true;

    // Incompatible version: 4.0.9801.1
    return verInfo->dwFileVersionMS == 262144
        && verInfo->dwFileVersionLS == 642318337;
}

void VBATest::testMiscOLEStuff()
{
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
        return; // can't do anything without OLE, so skip test

    // search for the ODBC Excel drivers
    const int nBufSize = 1024 * 4;
    wchar_t sBuf[nBufSize];
    if( !SQLGetInstalledDriversW( sBuf, nBufSize, nullptr ) )
        return;

    const wchar_t *aExcelDriverNameList[]
    {
        L"Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)",
        L"Microsoft Excel Driver (*.xls)"
    };
    const unsigned MAX_DRV = SAL_N_ELEMENTS( aExcelDriverNameList );
    bool bFoundDrivers[ MAX_DRV ] = { false, };

    const wchar_t *pODBCDriverName = sBuf;
    for (; wcslen( pODBCDriverName ) != 0; pODBCDriverName += wcslen( pODBCDriverName ) + 1 )
    {
        for ( unsigned i = 0; i < MAX_DRV; ++i )
            if ( !bFoundDrivers[ i ] && wcscmp( pODBCDriverName, aExcelDriverNameList[ i ] ) == 0 )
                bFoundDrivers[ i ] = true;
    }

    pODBCDriverName = nullptr;
    for ( unsigned i = 0; i < MAX_DRV; ++i )
        if ( bFoundDrivers[ i ] )
        {
            pODBCDriverName = aExcelDriverNameList[ i ];
            break;
        }

    if ( !pODBCDriverName )
        return; // can't find any ODBC driver needed for the test, so skip it

    if ( IsIncompatibleExcelJetDriver( pODBCDriverName ) )
        return; // found incompatible Excel driver, so skip the tests

    const char* macroSource[] = {
        "ole_ObjAssignNoDflt.vb",
        "ole_ObjAssignToNothing.vb",
        "ole_dfltObjDflMethod.vb",
    };

    OUString sMacroPathURL = m_directories.getURLFromSrc("/basic/qa/vba_tests/");

    uno::Sequence< uno::Any > aArgs(2);
    // path to test document
    OUString sPath = m_directories.getPathFromSrc("/basic/qa/vba_tests/data/ADODBdata.xls");
    sPath = sPath.replaceAll( "/", "\\" );

    aArgs[ 0 ] <<= sPath;
    aArgs[ 1 ] <<= OUString(o3tl::toU(pODBCDriverName));

    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( macroSource ); ++i )
    {
        OUString sMacroURL = sMacroPathURL
                           + OUString::createFromAscii( macroSource[ i ] );
        MacroSnippet myMacro;
        myMacro.LoadSourceFromFile( sMacroURL );
        SbxVariableRef pReturn = myMacro.Run( aArgs );
        if ( pReturn.is() )
        {
            fprintf(stderr, "macro result for %s\n", macroSource[ i ] );
            fprintf(stderr, "macro returned:\n%s\n", OUStringToOString( pReturn->GetOUString(), RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        CPPUNIT_ASSERT_MESSAGE("No return variable huh?", pReturn.get() != nullptr );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Result not as expected", OUString("OK"), pReturn->GetOUString() );
    }
}
#endif

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(VBATest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
