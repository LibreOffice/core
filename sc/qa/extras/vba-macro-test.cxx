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
#include <vcl/svapp.hxx>

#include <docsh.hxx>
#include <document.hxx>
#include <attrib.hxx>
#include <scitems.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>

using namespace css;

class VBAMacroTest : public UnoApiTest
{
public:
    uno::Reference<lang::XComponent> mxComponent;

    VBAMacroTest()
        : UnoApiTest("/sc/qa/extras/testdocuments")
    {
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
        {
            mxComponent->dispose();
            mxComponent.set(nullptr);
        }

        test::BootstrapFixture::tearDown();
    }

    void testSimpleCopyAndPaste();
    void testMultiDocumentCopyAndPaste();
    void testVba();
    void testTdf107885();
    void testTdf131562();
    void testTdf107902();
    void testTdf90278();

    CPPUNIT_TEST_SUITE(VBAMacroTest);
    CPPUNIT_TEST(testSimpleCopyAndPaste);
    CPPUNIT_TEST(testMultiDocumentCopyAndPaste);
    CPPUNIT_TEST(testVba);
    CPPUNIT_TEST(testTdf107885);
    CPPUNIT_TEST(testTdf131562);
    CPPUNIT_TEST(testTdf107902);
    CPPUNIT_TEST(testTdf90278);
    CPPUNIT_TEST_SUITE_END();
};

void VBAMacroTest::testSimpleCopyAndPaste()
{
    // Copy-paste values in the same sheet

    // Range(Cells(4, 3), Cells(6, 3)).Copy
    // Cells(4, 2).Activate
    // ActiveCell.PasteSpecial xlValues

    OUString aFileName;
    createFileURL(u"SimpleCopyPaste.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    // Check state
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, rDoc.GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, rDoc.GetValue(ScAddress(2, 5, 0)));

    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 5, 0)));

    SfxObjectShell::CallXScript(
        mxComponent, "vnd.sun.Star.script:VBAProject.Module1.test?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    // Copy from C4-C6
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, rDoc.GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, rDoc.GetValue(ScAddress(2, 5, 0)));

    // Paste to B4-B6
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(1, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, rDoc.GetValue(ScAddress(1, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, rDoc.GetValue(ScAddress(1, 5, 0)));
}

void VBAMacroTest::testMultiDocumentCopyAndPaste()
{
    // Creates a new workbook (document) and copy-pastes values
    // between the documents.

    // Set CurrentWB = ActiveWorkbook
    // Workbooks.Add
    // Set NewWB = ActiveWorkbook
    // Cells(3, 2).Value = 200
    // Cells(4, 2).Value = 100
    // Range(Cells(3, 2), Cells(4, 2)).Copy
    // CurrentWB.Activate
    // Cells(2, 2).Activate
    // ActiveCell.PasteSpecial xlValues
    // ...

    OUString aFileName;
    createFileURL(u"MultiDocumentCopyPaste.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 3, 0)));

    SfxObjectShell::CallXScript(
        mxComponent, "vnd.sun.Star.script:VBAProject.Module1.test?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT_EQUAL(200.0, rDoc.GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(100.0, rDoc.GetValue(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 3, 0)));
}

void VBAMacroTest::testVba()
{
    TestMacroInfo testInfo[] = {
        { OUString("TestAddress."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        {
            OUString("vba."),
            OUString(
                "vnd.sun.Star.script:VBAProject.Modul1.Modul1?language=Basic&location=document"),
        },
        { OUString("MiscRangeTests."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("bytearraystring."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacro.test?language=Basic&location=document") },
        { OUString("AutoFilter."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("CalcFont."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("TestIntersection."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("TestUnion."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("range-4."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("Ranges-3."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("TestCalc_Rangetest."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("TestCalc_Rangetest2."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("Ranges-2."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("pagesetup."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("Window."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("window2."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("PageBreaks."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("Shapes."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("Ranges."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("CheckOptionToggleValue."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("GeneratedEventTest."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("MiscControlTests."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("Workbooks."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("Names."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("NamesSheetLocal."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("vba_endFunction."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
        { OUString("vba_findFunction."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
    };
    OUString sTempDir;
    OUString sTempDirURL;
    osl::FileBase::getTempDirURL(sTempDirURL);
    osl::FileBase::getSystemPathFromFileURL(sTempDirURL, sTempDir);
    sTempDir += OUStringChar(SAL_PATHDELIMITER);
    OUString sTestFileName("My Test WorkBook.xls");
    uno::Sequence<uno::Any> aParams;
    for (const auto& rTestInfo : testInfo)
    {
        OUString aFileName;
        createFileURL(OUStringConcatenation(rTestInfo.sFileBaseName + "xls"), aFileName);
        uno::Reference<css::lang::XComponent> xComponent
            = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

        // process all events such as OnLoad events etc.
        // otherwise the tend to arrive later at a random
        // time - while processing other StarBasic methods.
        Application::Reschedule(true);

        uno::Any aRet;
        uno::Sequence<sal_Int16> aOutParamIndex;
        uno::Sequence<uno::Any> aOutParam;
        bool bWorkbooksHandling = rTestInfo.sFileBaseName == "Workbooks." && !sTempDir.isEmpty();

        if (bWorkbooksHandling)
        {
            aParams = { uno::Any(sTempDir), uno::Any(sTestFileName) };
        }

        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        SAL_INFO("sc.qa", "about to invoke vba test in " << aFileName << " with url "
                                                         << rTestInfo.sMacroUrl);

        SfxObjectShell::CallXScript(xComponent, rTestInfo.sMacroUrl, aParams, aRet, aOutParamIndex,
                                    aOutParam);
        OUString aStringRes;
        aRet >>= aStringRes;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OUString("script reported failure in file " + rTestInfo.sFileBaseName + "xls")
                .toUtf8()
                .getStr(),
            OUString("OK"), aStringRes);
        pFoundShell->DoClose();
        if (bWorkbooksHandling)
        {
            OUString sFileUrl;
            OUString sFilePath = sTempDir + sTestFileName;
            osl::FileBase::getFileURLFromSystemPath(sFilePath, sFileUrl);
            if (!sFileUrl.isEmpty())
                osl::File::remove(sFileUrl);
        }
    }
}

void VBAMacroTest::testTdf107885()
{
    OUString aFileName;
    createFileURL(u"tdf107885.xlsm", aFileName);
    uno::Reference<css::lang::XComponent> xComponent
        = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT(!rDoc.RowHidden(1, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(2, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(3, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(4, 0));

    // Call auto filter macro using a string condition
    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:VBAProject.Module1.AFString?language=Basic&location=document", aParams,
        aRet, aOutParamIndex, aOutParam);

    //Without the fix in place, all rows in autofilter would have been hidden
    CPPUNIT_ASSERT(rDoc.RowHidden(1, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(2, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(3, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(4, 0));

    // Call auto filter macro using a numeric condition without any locale
    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:VBAProject.Module1.AFNumeric?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT(rDoc.RowHidden(1, 0));
    CPPUNIT_ASSERT(rDoc.RowHidden(2, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(3, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(4, 0));

    pDocSh->DoClose();
}

void VBAMacroTest::testTdf131562()
{
    OUString aFileName;
    createFileURL(u"tdf131562.xlsm", aFileName);
    uno::Reference<css::lang::XComponent> xComponent
        = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("1"), rDoc.GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), rDoc.GetString(ScAddress(0, 3, 0)));

    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:VBAProject.Munka1.numberconcat?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    //Without the fix in place, the macro wouldn't have concatenated 1 and " ."
    CPPUNIT_ASSERT_EQUAL(OUString("1 ."), rDoc.GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1 .cat"), rDoc.GetString(ScAddress(0, 3, 0)));

    pDocSh->DoClose();
}

void VBAMacroTest::testTdf107902()
{
    OUString aFileName;
    createFileURL(u"tdf107902.xlsm", aFileName);
    uno::Reference<css::lang::XComponent> xComponent
        = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    //Without the fix in place, it would have failed with 'Unexpected dialog:  Error: BASIC runtime error.'
    SfxObjectShell::CallXScript(
        xComponent, "vnd.sun.Star.script:VBAProject.Module1.AF?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    //Check the autofilter was created
    const ScPatternAttr* pPattern = rDoc.GetPattern(0, 0, 0);
    CPPUNIT_ASSERT(pPattern);

    const ScMergeFlagAttr& rAttr = pPattern->GetItem(ATTR_MERGE_FLAG);
    CPPUNIT_ASSERT_MESSAGE("Autofilter was not created", rAttr.HasAutoFilter());

    //Check the last row is hidden
    CPPUNIT_ASSERT(!rDoc.RowHidden(0, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(1, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(2, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(3, 0));
    CPPUNIT_ASSERT(rDoc.RowHidden(4, 0));

    pDocSh->DoClose();
}

void VBAMacroTest::testTdf90278()
{
    OUString aFileName;
    createFileURL(u"tdf90278.xls", aFileName);
    auto xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT(pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    // Without the fix in place, changing the border weight
    // would cause a Basic exception/error in the following script.
    SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:VBAProject.Module1.BorderWeight?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    // Check the border weight of the corresponding cell in the test document
    sal_Int32 aReturnValue;
    aRet >>= aReturnValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aReturnValue);

    xDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(VBAMacroTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
