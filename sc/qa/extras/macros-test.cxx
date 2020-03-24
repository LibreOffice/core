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
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>

#include <docsh.hxx>
#include <document.hxx>

#include <com/sun/star/script/XLibraryContainerPassword.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class ScMacrosTest : public UnoApiTest
{
public:
    ScMacrosTest();
    void saveAndReload(css::uno::Reference<css::lang::XComponent>& xComponent,
                       const OUString& rFilter);

    void testStarBasic();
    void testVba();
    void testMSP();
    void testPasswordProtectedStarBasic();
    void testRowColumn();
    void testPasswordProtectedUnicodeString();
    void testTdf131296_legacy();
    void testTdf131296_new();

    CPPUNIT_TEST_SUITE(ScMacrosTest);
    CPPUNIT_TEST(testStarBasic);
    CPPUNIT_TEST(testMSP);
    CPPUNIT_TEST(testVba);
    CPPUNIT_TEST(testPasswordProtectedStarBasic);
    CPPUNIT_TEST(testRowColumn);
    CPPUNIT_TEST(testPasswordProtectedUnicodeString);
    CPPUNIT_TEST(testTdf131296_legacy);
    CPPUNIT_TEST(testTdf131296_new);

    CPPUNIT_TEST_SUITE_END();
};

void ScMacrosTest::saveAndReload(css::uno::Reference<css::lang::XComponent>& xComponent,
                                 const OUString& rFilter)
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    css::uno::Sequence<css::beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= rFilter;
    css::uno::Reference<css::frame::XStorable> xStorable(xComponent, css::uno::UNO_QUERY_THROW);
    xStorable->storeAsURL(aTempFile.GetURL(), aArgs);
    css::uno::Reference<css::util::XCloseable> xCloseable(xComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(true);

    xComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.sheet.SpreadsheetDocument");
}

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
    uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load MasterScriptProviderProblem.ods", xComponent.is());

    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh != nullptr);

    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:Standard.Module1.TestMSP?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);
    OUString sResult;
    aRet >>= sResult;

    SAL_INFO("sc.qa", "Result is " << sResult );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TestMSP ( for fdo#67547) failed", OUString("OK"), sResult);
    pDocSh->DoClose();
}

void ScMacrosTest::testPasswordProtectedStarBasic()
{
    const OUString aFileNameBase("testTypePassword.ods");
    OUString aFileName;
    createFileURL(aFileNameBase, aFileName);
    uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load testTypePassword.ods", xComponent.is());

    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();


    // User defined types

    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:MyLibrary.Module1.Main?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    OUString aValue = rDoc.GetString(0,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("User defined types script did not change the value of Sheet1.A1", OUString("success"), aValue);

    // Big Module

    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:MyLibrary.BigModule.bigMethod?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    aValue = rDoc.GetString(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Big module script did not change the value of Sheet1.B1", OUString("success"), aValue);

    // far big method tdf#94617

    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:MyLibrary.BigModule.farBigMethod?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    aValue = rDoc.GetString(2,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Far Method script did not change the value of Sheet1.C1", OUString("success"), aValue);


    pDocSh->DoClose();
}

void ScMacrosTest::testStarBasic()
{
    const OUString aFileNameBase("StarBasic.ods");
    OUString aFileName;
    createFileURL(aFileNameBase, aFileName);
    uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load StarBasic.ods", xComponent.is());

    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:Standard.Module1.Macro1?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);
    double aValue;
    rDoc.GetValue(0,0,0,aValue);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("script did not change the value of Sheet1.A1",2.0, aValue, 0.00001);
    pDocSh->DoClose();
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
        {
            OUString("vba_endFunction."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
        {
            OUString("vba_findFunction."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
    };
    OUString sTempDir;
    OUString sTempDirURL;
    osl::FileBase:: getTempDirURL( sTempDirURL );
    osl::FileBase::getSystemPathFromFileURL( sTempDirURL, sTempDir );
    sTempDir += OUStringChar(SAL_PATHDELIMITER);
    OUString sTestFileName("My Test WorkBook.xls");
    Sequence< uno::Any > aParams;
    for (const auto& rTestInfo : testInfo)
    {
        OUString aFileName;
        createFileURL(rTestInfo.sFileBaseName + "xls", aFileName);
        uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");
        OUString sMsg( "Failed to load " + aFileName );
        CPPUNIT_ASSERT_MESSAGE( OUStringToOString( sMsg, RTL_TEXTENCODING_UTF8 ).getStr(), xComponent.is() );

        // process all events such as OnLoad events etc.
        // otherwise the tend to arrive later at a random
        // time - while processing other StarBasic methods.
        Application::Reschedule(true);

        Any aRet;
        Sequence< sal_Int16 > aOutParamIndex;
        Sequence< Any > aOutParam;
        bool bWorkbooksHandling = rTestInfo.sFileBaseName == "Workbooks." && !sTempDir.isEmpty() ;

        if ( bWorkbooksHandling )
        {
            aParams.realloc(2);
            aParams[ 0 ] <<= sTempDir;
            aParams[ 1 ] <<= sTestFileName;
        }

        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        SAL_INFO("sc.qa", "about to invoke vba test in " << aFileName << " with url " << rTestInfo.sMacroUrl);

        SfxObjectShell::CallXScript(
            xComponent, rTestInfo.sMacroUrl, aParams, aRet, aOutParamIndex,
            aOutParam);
        OUString aStringRes;
        aRet >>= aStringRes;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OUString("script reported failure in file " + rTestInfo.sFileBaseName + "xls")
                .toUtf8()
                .getStr(),
            OUString("OK"), aStringRes);
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

void ScMacrosTest::testRowColumn()
{
    const OUString aFileNameBase("StarBasic.ods");
    OUString aFileName;
    createFileURL(aFileNameBase, aFileName);
    uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load StarBasic.ods", xComponent.is());

    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:Standard.Module1.Macro_RowHeight?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    sal_uInt16 nHeight = rDoc.GetRowHeight(0, 0) * HMM_PER_TWIPS;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2000), nHeight);

    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:Standard.Module1.Macro_ColumnWidth?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);
    sal_uInt16 nWidth  = rDoc.GetColWidth(0, 0) * HMM_PER_TWIPS;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4000), nWidth);

    pDocSh->DoClose();
}

void ScMacrosTest::testPasswordProtectedUnicodeString()
{
    const OUString sCorrectString(u"English Русский 中文");
    const OUString sMacroURL(
        "vnd.sun.Star.script:Protected.Module1.TestUnicodeString?language=Basic&location=document");
    const OUString sLibName("Protected");

    OUString aFileName;
    createFileURL("tdf57113.ods", aFileName);
    auto xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(xComponent);

    // Check that loading password-protected macro image correctly loads Unicode strings
    {
        Any aRet;
        Sequence<sal_Int16> aOutParamIndex;
        Sequence<Any> aOutParam;
        Sequence<uno::Any> aParams;

        SfxObjectShell::CallXScript(xComponent, sMacroURL, aParams, aRet, aOutParamIndex,
                                    aOutParam);

        OUString aReturnValue;
        aRet >>= aReturnValue;
        CPPUNIT_ASSERT_EQUAL(sCorrectString, aReturnValue);
    }

    // Unlock and load the library, to regenerate the image on save
    css::uno::Reference<css::document::XEmbeddedScripts> xES(xComponent, UNO_QUERY_THROW);
    css::uno::Reference<css::script::XLibraryContainer> xLC(xES->getBasicLibraries(),
                                                            UNO_QUERY_THROW);
    css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(xLC, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xPasswd->isLibraryPasswordProtected(sLibName));
    CPPUNIT_ASSERT(!xPasswd->isLibraryPasswordVerified(sLibName));
    CPPUNIT_ASSERT(xPasswd->verifyLibraryPassword(sLibName, "password"));
    xLC->loadLibrary(sLibName);
    CPPUNIT_ASSERT(xLC->isLibraryLoaded(sLibName));

    // Now check that saving stores Unicode data correctly in image's string pool
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    {
        Any aRet;
        Sequence<sal_Int16> aOutParamIndex;
        Sequence<Any> aOutParam;
        Sequence<uno::Any> aParams;

        SfxObjectShell::CallXScript(xComponent, sMacroURL, aParams, aRet, aOutParamIndex,
                                    aOutParam);

        OUString aReturnValue;
        aRet >>= aReturnValue;
        CPPUNIT_ASSERT_EQUAL(sCorrectString, aReturnValue);
    }

    css::uno::Reference<css::util::XCloseable> xCloseable(xComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(true);
}

void ScMacrosTest::testTdf131296_legacy()
{
    // For legacy password-protected library images, we must correctly get the constants' values,
    // and also - for Integer - the type.
    const std::vector<std::pair<OUString, OUString>> aTests({
        { "TestIntConst", "Integer: 123" },
        { "TestLongConst", "Double: 123" },
        { "TestSingleConst", "Double: 123" },
        { "TestDoubleConst", "Double: 123" },
    });

    OUString aFileName;
    createFileURL("tdf131296_legacy.ods", aFileName);
    auto xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(xComponent);

    {
        Any aRet;
        Sequence<sal_Int16> aOutParamIndex;
        Sequence<Any> aOutParam;
        Sequence<uno::Any> aParams;

        for (auto& [sTestName, sExpected] : aTests)
        {
            SfxObjectShell::CallXScript(xComponent,
                                        "vnd.sun.Star.script:Protected.Module1." + sTestName
                                            + "?language=Basic&location=document",
                                        aParams, aRet, aOutParamIndex, aOutParam);

            OUString aReturnValue;
            aRet >>= aReturnValue;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sTestName.toUtf8().getStr(), sExpected, aReturnValue);
        }
    }

    css::uno::Reference<css::util::XCloseable> xCloseable(xComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(true);
}

void ScMacrosTest::testTdf131296_new()
{
    // For new password-protected library images, we must correctly get both the constants' values
    // and their types.
    const std::vector<std::pair<OUString, OUString>> aTests({
        { "TestIntConst", "Integer: 123" },
        { "TestLongConst", "Long: 123" },
        { "TestSingleConst", "Single: 123" },
        { "TestDoubleConst", "Double: 123" },
        { "TestCurrencyConst", "Currency: 123.0000" },
    });

    OUString aFileName;
    createFileURL("tdf131296_new.ods", aFileName);
    auto xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(xComponent);

    {
        Any aRet;
        Sequence<sal_Int16> aOutParamIndex;
        Sequence<Any> aOutParam;
        Sequence<uno::Any> aParams;

        for (auto& [sTestName, sExpected] : aTests)
        {
            SfxObjectShell::CallXScript(xComponent,
                                        "vnd.sun.Star.script:Protected.Module1." + sTestName
                                            + "?language=Basic&location=document",
                                        aParams, aRet, aOutParamIndex, aOutParam);

            OUString aReturnValue;
            aRet >>= aReturnValue;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sTestName.toUtf8().getStr(), sExpected, aReturnValue);
        }
    }

    css::uno::Reference<css::util::XCloseable> xCloseable(xComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(true);
}

ScMacrosTest::ScMacrosTest()
      : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
