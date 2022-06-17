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
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>

#include <docsh.hxx>
#include <document.hxx>
#include <attrib.hxx>
#include <scitems.hxx>
#include <sortparam.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

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
    void testSheetAndColumnSelectAndHide();
    void testPrintArea();
    void testSelectAllChaged();
    void testRangeSelect();
    void testWindowState();
    void testScroll();
    void testMacroKeyBinding();

    void testVba();
    void testTdf149579();
    void testVbaRangeSort();
    void testTdf107885();
    void testTdf131562();
    void testTdf52602();
    void testTdf107902();
    void testTdf90278();
    void testTdf149531();

    CPPUNIT_TEST_SUITE(VBAMacroTest);
    CPPUNIT_TEST(testSimpleCopyAndPaste);
    CPPUNIT_TEST(testMultiDocumentCopyAndPaste);
    CPPUNIT_TEST(testSheetAndColumnSelectAndHide);
    CPPUNIT_TEST(testPrintArea);
    CPPUNIT_TEST(testSelectAllChaged);
    CPPUNIT_TEST(testRangeSelect);
    CPPUNIT_TEST(testWindowState);
    CPPUNIT_TEST(testScroll);
    CPPUNIT_TEST(testMacroKeyBinding);

    CPPUNIT_TEST(testVba);
    CPPUNIT_TEST(testTdf149579);
    CPPUNIT_TEST(testVbaRangeSort);
    CPPUNIT_TEST(testTdf107885);
    CPPUNIT_TEST(testTdf131562);
    CPPUNIT_TEST(testTdf52602);
    CPPUNIT_TEST(testTdf107902);
    CPPUNIT_TEST(testTdf90278);
    CPPUNIT_TEST(testTdf149531);
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

void VBAMacroTest::testSheetAndColumnSelectAndHide()
{
    OUString aFileName;
    createFileURL(u"SheetAndColumnSelectAndHide.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    ScTabViewShell* pView = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);
    auto const& rViewData = pView->GetViewData();

    CPPUNIT_ASSERT(!rDoc.ColHidden(0, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(1, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(2, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(3, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(4, 1));

    CPPUNIT_ASSERT(!rDoc.ColHidden(0, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(1, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(2, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(3, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(4, 2));

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell::CallXScript(
        mxComponent,
        "vnd.sun.Star.script:VBAProject.ThisWorkbook.testHide?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT(!rDoc.ColHidden(0, 1));
    CPPUNIT_ASSERT(rDoc.ColHidden(1, 1));
    CPPUNIT_ASSERT(rDoc.ColHidden(2, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(3, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(4, 1));

    CPPUNIT_ASSERT(!rDoc.ColHidden(0, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(1, 2));
    CPPUNIT_ASSERT(rDoc.ColHidden(2, 2));
    CPPUNIT_ASSERT(rDoc.ColHidden(3, 2));
    CPPUNIT_ASSERT(rDoc.ColHidden(4, 2));

    CPPUNIT_ASSERT_EQUAL(SCTAB(0), rViewData.GetTabNo());

    SfxObjectShell::CallXScript(
        mxComponent,
        "vnd.sun.Star.script:VBAProject.ThisWorkbook.testUnhide?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT(!rDoc.ColHidden(0, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(1, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(2, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(3, 1));
    CPPUNIT_ASSERT(!rDoc.ColHidden(4, 1));

    CPPUNIT_ASSERT(!rDoc.ColHidden(0, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(1, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(2, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(3, 2));
    CPPUNIT_ASSERT(!rDoc.ColHidden(4, 2));

    CPPUNIT_ASSERT_EQUAL(SCTAB(0), rViewData.GetTabNo());
}

void VBAMacroTest::testPrintArea()
{
    // Sets the print area to A1:B5
    // ActiveSheet.PageSetup.PrintArea = "$A$1:$B$5"

    OUString aFileName;
    createFileURL(u"VariousTestMacros.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XPrintAreas> xPrintAreas(xSheet, uno::UNO_QUERY_THROW);

    {
        const uno::Sequence<table::CellRangeAddress> aSequence = xPrintAreas->getPrintAreas();
        CPPUNIT_ASSERT_EQUAL(false, aSequence.hasElements());
    }

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell::CallXScript(mxComponent,
                                "vnd.sun.Star.script:VBAProject.ThisWorkbook.testPrintArea?"
                                "language=Basic&location=document",
                                aParams, aRet, aOutParamIndex, aOutParam);

    {
        const uno::Sequence<table::CellRangeAddress> aSequence = xPrintAreas->getPrintAreas();
        CPPUNIT_ASSERT_EQUAL(true, aSequence.hasElements());
    }
}

void VBAMacroTest::testSelectAllChaged()
{
    // Columns("A:A").Select
    // Range(Selection, Selection.End(xlToRight)).Select

    OUString aFileName;
    createFileURL(u"VariousTestMacros.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScTabViewShell* pView = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);
    auto const& pViewData = pView->GetViewData();
    CPPUNIT_ASSERT_EQUAL(ScRange(), pViewData.GetMarkData().GetMarkArea());

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell::CallXScript(mxComponent,
                                "vnd.sun.Star.script:VBAProject.ThisWorkbook.testSelectAll?"
                                "language=Basic&location=document",
                                aParams, aRet, aOutParamIndex, aOutParam);

    // A1:E1048576
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 4, MAXROW, 0), pViewData.GetMarkData().GetMarkArea());
}

void VBAMacroTest::testRangeSelect()
{
    // Range("B2").Select
    // Range(Selection, Selection.End(xlToRight)).Select

    OUString aFileName;
    createFileURL(u"VariousTestMacros.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScTabViewShell* pView = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);
    auto const& pViewData = pView->GetViewData();
    CPPUNIT_ASSERT_EQUAL(ScRange(), pViewData.GetMarkData().GetMarkArea());

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell::CallXScript(mxComponent,
                                "vnd.sun.Star.script:VBAProject.ThisWorkbook.testRangeSelect?"
                                "language=Basic&location=document",
                                aParams, aRet, aOutParamIndex, aOutParam);

    // B2:E5
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 4, 1, 0), pViewData.GetMarkData().GetMarkArea());
}

void VBAMacroTest::testWindowState()
{
    // Application.WindowState = xlMinimized
    // Application.WindowState = xlMaximized
    // Application.WindowState = xlNormal

    OUString aFileName;
    createFileURL(u"VariousTestMacros.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell::CallXScript(mxComponent,
                                "vnd.sun.Star.script:VBAProject.ThisWorkbook.testWindowState?"
                                "language=Basic&location=document",
                                aParams, aRet, aOutParamIndex, aOutParam);
}

void VBAMacroTest::testScroll()
{
    // ActiveWindow.ScrollColumn = 30
    // ActiveWindow.ScrollRow = 100

    OUString aFileName;
    createFileURL(u"VariousTestMacros.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScTabViewShell* pView = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);
    auto const& rViewData = pView->GetViewData();

    CPPUNIT_ASSERT_EQUAL(ScSplitPos::SC_SPLIT_BOTTOMLEFT, rViewData.GetActivePart());
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), rViewData.GetPosX(ScHSplitPos::SC_SPLIT_LEFT));
    CPPUNIT_ASSERT_EQUAL(SCROW(0), rViewData.GetPosY(ScVSplitPos::SC_SPLIT_BOTTOM));

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell::CallXScript(
        mxComponent,
        "vnd.sun.Star.script:VBAProject.ThisWorkbook.testScroll?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT_EQUAL(ScSplitPos::SC_SPLIT_BOTTOMLEFT, rViewData.GetActivePart());
    CPPUNIT_ASSERT_EQUAL(SCCOL(29), rViewData.GetPosX(ScHSplitPos::SC_SPLIT_LEFT));
    CPPUNIT_ASSERT_EQUAL(SCROW(99), rViewData.GetPosY(ScVSplitPos::SC_SPLIT_BOTTOM));
}

void VBAMacroTest::testMacroKeyBinding()
{
    // key_U() -> CTRL+U
    // key_T() -> CTRL+T

    OUString aFileName;
    createFileURL(u"KeyShortcut.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());

    uno::Reference<ui::XUIConfigurationManagerSupplier> xConfigSupplier(xModel, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xConfigSupplier.is());
    uno::Reference<ui::XUIConfigurationManager> xConfigManager
        = xConfigSupplier->getUIConfigurationManager();
    uno::Reference<ui::XAcceleratorConfiguration> xAccelerator
        = xConfigManager->getShortCutManager();

    awt::KeyEvent aCtrlU;
    aCtrlU.KeyCode = css::awt::Key::U;
    aCtrlU.Modifiers = css::awt::KeyModifier::MOD1;

    CPPUNIT_ASSERT_EQUAL(
        OUString(
            "vnd.sun.star.script:VBAProject.ThisWorkbook.key_U?language=Basic&location=document"),
        xAccelerator->getCommandByKeyEvent(aCtrlU));

    awt::KeyEvent aCtrlT;
    aCtrlT.KeyCode = css::awt::Key::T;
    aCtrlT.Modifiers = css::awt::KeyModifier::MOD1;

    CPPUNIT_ASSERT_EQUAL(
        OUString(
            "vnd.sun.star.script:VBAProject.ThisWorkbook.key_T?language=Basic&location=document"),
        xAccelerator->getCommandByKeyEvent(aCtrlT));
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

        // process all events such as OnLoad events etc.  otherwise they tend
        // to arrive later at a random time - while processing other StarBasic
        // methods.
        Scheduler::ProcessEventsToIdle();

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

void VBAMacroTest::testTdf149579()
{
    auto xComponent = loadFromDesktop("private:factory/scalc");

    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(xComponent, uno::UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary("TestLibrary");
    xLibrary->insertByName("TestModule",
                           uno::Any(OUString("Option VBASupport 1\n"
                                             "Sub TestTdf149579\n"
                                             "Range(\"A1\").Sort Key1:=Range(\"A1\")\n"
                                             "End Sub\n")));

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pDocSh->GetDocument();

    rDoc.SetValue(ScAddress(0, 0, 0), 5.0);
    rDoc.SetValue(ScAddress(0, 1, 0), 10.0);
    rDoc.SetValue(ScAddress(0, 2, 0), 1.0);

    // Without the fix in place, this call would have crashed in debug builds with failed assertion
    ErrCode result = SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:TestLibrary.TestModule.TestTdf149579?language=Basic&location=document",
        {}, aRet, aOutParamIndex, aOutParam);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, result);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 5
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(5.0, rDoc.GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(0, 2, 0)));

    pDocSh->DoClose();
}

void VBAMacroTest::testVbaRangeSort()
{
    auto xComponent = loadFromDesktop("private:factory/scalc");

    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(xComponent, uno::UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary("TestLibrary");
    xLibrary->insertByName("TestModule",
                           uno::Any(OUString("Option VBASupport 1\n"
                                             "Sub TestRangeSort\n"
                                             "  Range(Cells(1, 1), Cells(3, 1)).Select\n"
                                             "  Selection.Sort Key1:=Range(\"A1\"), Header:=False\n"
                                             "End Sub\n")));

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pDocSh->GetDocument();

    rDoc.SetValue(ScAddress(0, 0, 0), 1.0);
    rDoc.SetValue(ScAddress(0, 1, 0), 0.5);
    rDoc.SetValue(ScAddress(0, 2, 0), 2.0);

    // Without the fix in place, this call would have crashed in debug builds with failed assertion
    ErrCode result = SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:TestLibrary.TestModule.TestRangeSort?language=Basic&location=document",
        {}, aRet, aOutParamIndex, aOutParam);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, result);

    CPPUNIT_ASSERT_EQUAL(0.5, rDoc.GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(0, 2, 0)));

    // Change sheet's first param sorting order
    ScSortParam aParam;
    rDoc.GetSortParam(aParam, 0);
    aParam.maKeyState[0].bAscending = false;
    rDoc.SetSortParam(aParam, 0);

    result = SfxObjectShell::CallXScript(
        xComponent,
        "vnd.sun.Star.script:TestLibrary.TestModule.TestRangeSort?language=Basic&location=document",
        {}, aRet, aOutParamIndex, aOutParam);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, result);

    // Without the fix in place, this test would have failed in non-debug builds with
    // - Expected: 2
    // - Actual  : 0.5
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.5, rDoc.GetValue(ScAddress(0, 2, 0)));

    pDocSh->DoClose();
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

void VBAMacroTest::testTdf52602()
{
    OUString aFileName;
    createFileURL(u"tdf52602.xls", aFileName);
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
    SfxObjectShell::CallXScript(xComponent,
                                "vnd.sun.Star.script:VBAProject.Modul1.Test_NumberFormat_DateTime?"
                                "language=Basic&location=document",
                                aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT_EQUAL(OUString("15:20"), rDoc.GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("15:20"), rDoc.GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("03/01/2012 15:20"), rDoc.GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("03/01/2012 15:20"), rDoc.GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("03/01/2012 15:20:00"), rDoc.GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("03/01/2012 15:20:00"), rDoc.GetString(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1/3/12 15:20"), rDoc.GetString(ScAddress(3, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1/3/12 15:20"), rDoc.GetString(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1/ March 2012"), rDoc.GetString(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1/ March 2012"), rDoc.GetString(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1/ Mar 2012"), rDoc.GetString(ScAddress(5, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1/ Mar 2012"), rDoc.GetString(ScAddress(5, 1, 0)));

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

void VBAMacroTest::testTdf149531()
{
    OUString aFileName;
    createFileURL(u"tdf149531.xls", aFileName);
    auto xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT(pFoundShell);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        // Without the fix in place, this test would have crashed
        // also check the test doesn't crash when the macro is executed a few times in a row
        SfxObjectShell::CallXScript(xComponent,
                                    "vnd.sun.Star.script:VBAProject.Module1.SetColumnWidth?"
                                    "language=Basic&location=document",
                                    aParams, aRet, aOutParamIndex, aOutParam);
    }

    sal_uInt16 nWidth
        = o3tl::convert(rDoc.GetColWidth(0, 0), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(25749), nWidth);

    pDocSh->DoClose();
}
CPPUNIT_TEST_SUITE_REGISTRATION(VBAMacroTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
