/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapi_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/frame/Desktop.hpp>

#include <basic/sbxdef.hxx>

#include "docsh.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class ScMacrosTest : public UnoApiTest
{
public:
    ScMacrosTest();

    void testStarBasic();
    void testVba();
    void testMSP();
    CPPUNIT_TEST_SUITE(ScMacrosTest);
#if !defined(MACOSX)
    //enable this test if you want to play with star basic macros in unit tests
    //works but does nothing useful yet
    CPPUNIT_TEST(testStarBasic);
    CPPUNIT_TEST(testMSP);
    CPPUNIT_TEST(testVba);
#endif

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

// I suppose you could say this test doesn't really belong here, OTOH
// we need a full document to run the test ( it related originally to an
// imported Excel VBA macro ) It's convenient and fast to unit test
// this the problem this way. Perhaps in the future there will be some sort
// of slowcheck tests ( requiring a full document environment in the scripting
// module, we could move the test there then ) - relates to fdo#67547
void ScMacrosTest::testMSP()
{
    const OUString aFileNameBase("MasterScriptProviderProblem.ods");
    OUString aFileName;
    createFileURL(aFileNameBase, aFileName);
    std::cout << "MasterScriptProviderProblem (fdo#67547) test" << std::endl;
    uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load MasterScriptProviderProblem.ods", xComponent.is());

    OUString aURL("vnd.sun.Star.script:Standard.Module1.TestMSP?language=Basic&location=document");
    String sUrl = aURL;
    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != NULL);

    pFoundShell->CallXScript(xComponent, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
    OUString sResult;
    aRet >>= sResult;

    std::cout << "Result is " << sResult << std::endl;
    CPPUNIT_ASSERT_MESSAGE("TestMSP ( for fdo#67547) failed", sResult == "OK" );
    xDocSh->DoClose();
}

void ScMacrosTest::testStarBasic()
{
    const OUString aFileNameBase("StarBasic.ods");
    OUString aFileName;
    createFileURL(aFileNameBase, aFileName);
    std::cout << "StarBasic test" << std::endl;
    uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load StarBasic.ods", xComponent.is());

    OUString aURL("vnd.sun.Star.script:Standard.Module1.Macro1?language=Basic&location=document");
    String sUrl = aURL;
    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* xDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument* pDoc = xDocSh->GetDocument();

    pFoundShell->CallXScript(xComponent, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
    double aValue;
    pDoc->GetValue(0,0,0,aValue);
    std::cout << "returned value = " << aValue << std::endl;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("script did not change the value of Sheet1.A1",2.0, aValue, 0.00001);
    xDocSh->DoClose();
}

void ScMacrosTest::testVba()
{
    TestMacroInfo testInfo[] = {
        {
            OUString("TestAddress."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("vba."),
            OUString("vnd.sun.Star.script:VBAProject.Modul1.Modul1?language=Basic&location=document"),
        },
        {
            OUString("MiscRangeTests."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("bytearraystring."),
            OUString("vnd.sun.Star.script:VBAProject.testMacro.test?language=Basic&location=document")
        },
        {
            OUString("AutoFilter."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("CalcFont."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("TestIntersection."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("TestUnion."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("range-4."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("Ranges-3."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("TestCalc_Rangetest."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("TestCalc_Rangetest2."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("Ranges-2."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("pagesetup."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("Window."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("window2."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("PageBreaks."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("Shapes."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("Ranges."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("CheckOptionToggleValue."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("GeneratedEventTest."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("MiscControlTests."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("Workbooks."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("Names."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
    };
    OUString sTempDir;
    OUString sTempDirURL;
    osl::FileBase:: getTempDirURL( sTempDirURL );
    osl::FileBase::getSystemPathFromFileURL( sTempDirURL, sTempDir );
    sTempDir += OUString( SAL_PATHDELIMITER );
    OUString sTestFileName("My Test WorkBook.xls");
    Sequence< uno::Any > aParams;
    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( testInfo ); ++i )
    {
        OUString aFileName;
        createFileURL(testInfo[i].sFileBaseName + "xls", aFileName);
        uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");
        OUString sMsg( "Failed to load " + aFileName );
        CPPUNIT_ASSERT_MESSAGE( OUStringToOString( sMsg, RTL_TEXTENCODING_UTF8 ).getStr(), xComponent.is() );

        String sUrl = testInfo[i].sMacroUrl;
        Any aRet;
        Sequence< sal_Int16 > aOutParamIndex;
        Sequence< Any > aOutParam;
        bool bWorkbooksHandling = OUString( testInfo[i].sFileBaseName ).equalsAscii("Workbooks.") && !sTempDir.isEmpty() ;

        if ( bWorkbooksHandling )
        {
            aParams.realloc(2);
            aParams[ 0 ] <<= sTempDir;
            aParams[ 1 ] <<= sTestFileName;
        }

        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        std::cout << "about to invoke vba test in " << OUStringToOString( aFileName, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;

        pFoundShell->CallXScript(xComponent, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
        OUString aStringRes;
        aRet >>= aStringRes;
        std::cout << "value of Ret " << OUStringToOString( aStringRes, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
        CPPUNIT_ASSERT_MESSAGE( "script reported failure",aStringRes == "OK" );
        pFoundShell->DoClose();
        if ( bWorkbooksHandling )
        {
            OUString sFileUrl;
            OUString sFilePath = sTempDir + sTestFileName;
            osl::FileBase::getFileURLFromSystemPath( sFilePath, sFileUrl );
            if ( !sFileUrl.isEmpty() )
                osl::File::remove( sFileUrl );
        }
    }
}

ScMacrosTest::ScMacrosTest()
      : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
