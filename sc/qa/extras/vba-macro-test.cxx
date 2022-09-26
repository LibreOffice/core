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

#include <ooo/vba/excel/XlSpecialCellsValue.hpp>

#include <comphelper/propertysequence.hxx>

using namespace css;
using namespace ooo::vba;

class VBAMacroTest : public UnoApiTest
{
public:
    VBAMacroTest()
        : UnoApiTest("/sc/qa/extras/testdocuments")
    {
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
    void testTdf118247();
    void testTdf126457();

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
    CPPUNIT_TEST(testTdf118247);
    CPPUNIT_TEST(testTdf126457);
    CPPUNIT_TEST_SUITE_END();
};

void VBAMacroTest::testSimpleCopyAndPaste()
{
    // Copy-paste values in the same sheet

    // Range(Cells(4, 3), Cells(6, 3)).Copy
    // Cells(4, 2).Activate
    // ActiveCell.PasteSpecial xlValues
    loadFromURL(u"SimpleCopyPaste.xlsm");

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

    executeMacro("vnd.sun.Star.script:VBAProject.Module1.test?language=Basic&location=document");

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
    loadFromURL(u"MultiDocumentCopyPaste.xlsm");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 3, 0)));

    executeMacro("vnd.sun.Star.script:VBAProject.Module1.test?language=Basic&location=document");

    CPPUNIT_ASSERT_EQUAL(200.0, rDoc.GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(100.0, rDoc.GetValue(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 3, 0)));
}

void VBAMacroTest::testSheetAndColumnSelectAndHide()
{
    loadFromURL(u"SheetAndColumnSelectAndHide.xlsm");

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

    executeMacro(
        "vnd.sun.Star.script:VBAProject.ThisWorkbook.testHide?language=Basic&location=document");

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

    executeMacro(
        "vnd.sun.Star.script:VBAProject.ThisWorkbook.testUnhide?language=Basic&location=document");

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
    loadFromURL(u"VariousTestMacros.xlsm");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XPrintAreas> xPrintAreas(xSheet, uno::UNO_QUERY_THROW);

    {
        const uno::Sequence<table::CellRangeAddress> aSequence = xPrintAreas->getPrintAreas();
        CPPUNIT_ASSERT_EQUAL(false, aSequence.hasElements());
    }

    executeMacro("vnd.sun.Star.script:VBAProject.ThisWorkbook.testPrintArea?language=Basic&"
                 "location=document");

    {
        const uno::Sequence<table::CellRangeAddress> aSequence = xPrintAreas->getPrintAreas();
        CPPUNIT_ASSERT_EQUAL(true, aSequence.hasElements());
    }
}

void VBAMacroTest::testSelectAllChaged()
{
    // Columns("A:A").Select
    // Range(Selection, Selection.End(xlToRight)).Select
    loadFromURL(u"VariousTestMacros.xlsm");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScTabViewShell* pView = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);
    auto const& pViewData = pView->GetViewData();
    CPPUNIT_ASSERT_EQUAL(ScRange(), pViewData.GetMarkData().GetMarkArea());

    executeMacro("vnd.sun.Star.script:VBAProject.ThisWorkbook.testSelectAll?language=Basic&"
                 "location=document");

    // A1:E1048576
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 4, MAXROW, 0), pViewData.GetMarkData().GetMarkArea());
}

void VBAMacroTest::testRangeSelect()
{
    // Range("B2").Select
    // Range(Selection, Selection.End(xlToRight)).Select
    loadFromURL(u"VariousTestMacros.xlsm");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScTabViewShell* pView = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);
    auto const& pViewData = pView->GetViewData();
    CPPUNIT_ASSERT_EQUAL(ScRange(), pViewData.GetMarkData().GetMarkArea());

    executeMacro("vnd.sun.Star.script:VBAProject.ThisWorkbook.testRangeSelect?language=Basic&"
                 "location=document");

    // B2:E5
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 4, 1, 0), pViewData.GetMarkData().GetMarkArea());
}

void VBAMacroTest::testWindowState()
{
    // Application.WindowState = xlMinimized
    // Application.WindowState = xlMaximized
    // Application.WindowState = xlNormal
    loadFromURL(u"VariousTestMacros.xlsm");

    executeMacro("vnd.sun.Star.script:VBAProject.ThisWorkbook.testWindowState?language=Basic&"
                 "location=document");
}

void VBAMacroTest::testScroll()
{
    // ActiveWindow.ScrollColumn = 30
    // ActiveWindow.ScrollRow = 100

    loadFromURL(u"VariousTestMacros.xlsm");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScTabViewShell* pView = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);
    auto const& rViewData = pView->GetViewData();

    CPPUNIT_ASSERT_EQUAL(ScSplitPos::SC_SPLIT_BOTTOMLEFT, rViewData.GetActivePart());
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), rViewData.GetPosX(ScHSplitPos::SC_SPLIT_LEFT));
    CPPUNIT_ASSERT_EQUAL(SCROW(0), rViewData.GetPosY(ScVSplitPos::SC_SPLIT_BOTTOM));

    executeMacro(
        "vnd.sun.Star.script:VBAProject.ThisWorkbook.testScroll?language=Basic&location=document");

    CPPUNIT_ASSERT_EQUAL(ScSplitPos::SC_SPLIT_BOTTOMLEFT, rViewData.GetActivePart());
    CPPUNIT_ASSERT_EQUAL(SCCOL(29), rViewData.GetPosX(ScHSplitPos::SC_SPLIT_LEFT));
    CPPUNIT_ASSERT_EQUAL(SCROW(99), rViewData.GetPosY(ScVSplitPos::SC_SPLIT_BOTTOM));
}

void VBAMacroTest::testMacroKeyBinding()
{
    // key_U() -> CTRL+U
    // key_T() -> CTRL+T
    loadFromURL(u"KeyShortcut.xlsm");

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
// FIXME: sometimes it fails on Windows with
// Failed:  : Test change event for Range.Clear set:
// Failed:  : Test change event for Range.ClearContents set:
// Failed:  : Test change event for Range.Replace:
// Failed:  : Test change event for Range.FillRight:
// Tests passed: 4
// Tests failed: 4
#if !defined(_WIN32)
        { OUString("Ranges-3."),
          OUString(
              "vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document") },
#endif
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
        OUString aFileName = loadFromURL(Concat2View(rTestInfo.sFileBaseName + "xls"));

        // process all events such as OnLoad events etc.  otherwise they tend
        // to arrive later at a random time - while processing other StarBasic
        // methods.
        Scheduler::ProcessEventsToIdle();

        bool bWorkbooksHandling = rTestInfo.sFileBaseName == "Workbooks." && !sTempDir.isEmpty();

        if (bWorkbooksHandling)
        {
            aParams = { uno::Any(sTempDir), uno::Any(sTestFileName) };
        }

        SAL_INFO("sc.qa", "about to invoke vba test in " << aFileName << " with url "
                                                         << rTestInfo.sMacroUrl);

        uno::Any aRet = executeMacro(rTestInfo.sMacroUrl, aParams);
        OUString aStringRes;
        aRet >>= aStringRes;

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OUString("script reported failure in file " + rTestInfo.sFileBaseName + "xls")
                .toUtf8()
                .getStr(),
            OUString("OK"), aStringRes);

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
    mxComponent = loadFromDesktop("private:factory/scalc");

    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, uno::UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary("TestLibrary");
    xLibrary->insertByName("TestModule",
                           uno::Any(OUString("Option VBASupport 1\n"
                                             "Sub TestTdf149579\n"
                                             "Range(\"A1\").Sort Key1:=Range(\"A1\")\n"
                                             "End Sub\n")));

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pDocSh->GetDocument();

    rDoc.SetValue(ScAddress(0, 0, 0), 5.0);
    rDoc.SetValue(ScAddress(0, 1, 0), 10.0);
    rDoc.SetValue(ScAddress(0, 2, 0), 1.0);

    // Without the fix in place, this call would have crashed in debug builds with failed assertion
    executeMacro("vnd.sun.Star.script:TestLibrary.TestModule.TestTdf149579?language=Basic&location="
                 "document");
    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 5
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(5.0, rDoc.GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(0, 2, 0)));
}

void VBAMacroTest::testVbaRangeSort()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, uno::UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary("TestLibrary");
    xLibrary->insertByName("TestModule",
                           uno::Any(OUString("Option VBASupport 1\n"
                                             "Sub TestRangeSort\n"
                                             "  Range(Cells(1, 1), Cells(3, 1)).Select\n"
                                             "  Selection.Sort Key1:=Range(\"A1\"), Header:=False\n"
                                             "End Sub\n")));

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pDocSh->GetDocument();

    rDoc.SetValue(ScAddress(0, 0, 0), 1.0);
    rDoc.SetValue(ScAddress(0, 1, 0), 0.5);
    rDoc.SetValue(ScAddress(0, 2, 0), 2.0);

    // Without the fix in place, this call would have crashed in debug builds with failed assertion
    executeMacro("vnd.sun.Star.script:TestLibrary.TestModule.TestRangeSort?language=Basic&location="
                 "document");

    CPPUNIT_ASSERT_EQUAL(0.5, rDoc.GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(0, 2, 0)));

    // Change sheet's first param sorting order
    ScSortParam aParam;
    rDoc.GetSortParam(aParam, 0);
    aParam.maKeyState[0].bAscending = false;
    rDoc.SetSortParam(aParam, 0);

    executeMacro("vnd.sun.Star.script:TestLibrary.TestModule.TestRangeSort?language=Basic&location="
                 "document");

    // Without the fix in place, this test would have failed in non-debug builds with
    // - Expected: 2
    // - Actual  : 0.5
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.5, rDoc.GetValue(ScAddress(0, 2, 0)));
}

void VBAMacroTest::testTdf107885()
{
    loadFromURL(u"tdf107885.xlsm");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT(!rDoc.RowHidden(1, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(2, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(3, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(4, 0));

    // Call auto filter macro using a string condition
    executeMacro(
        "vnd.sun.Star.script:VBAProject.Module1.AFString?language=Basic&location=document");

    //Without the fix in place, all rows in autofilter would have been hidden
    CPPUNIT_ASSERT(rDoc.RowHidden(1, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(2, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(3, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(4, 0));

    // Call auto filter macro using a numeric condition without any locale
    executeMacro(
        "vnd.sun.Star.script:VBAProject.Module1.AFNumeric?language=Basic&location=document");

    CPPUNIT_ASSERT(rDoc.RowHidden(1, 0));
    CPPUNIT_ASSERT(rDoc.RowHidden(2, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(3, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(4, 0));
}

void VBAMacroTest::testTdf131562()
{
    loadFromURL(u"tdf131562.xlsm");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("1"), rDoc.GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), rDoc.GetString(ScAddress(0, 3, 0)));

    executeMacro(
        "vnd.sun.Star.script:VBAProject.Munka1.numberconcat?language=Basic&location=document");

    //Without the fix in place, the macro wouldn't have concatenated 1 and " ."
    CPPUNIT_ASSERT_EQUAL(OUString("1 ."), rDoc.GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1 .cat"), rDoc.GetString(ScAddress(0, 3, 0)));
}

void VBAMacroTest::testTdf52602()
{
    loadFromURL(u"tdf52602.xls");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    //Without the fix in place, it would have failed with 'Unexpected dialog:  Error: BASIC runtime error.'
    executeMacro("vnd.sun.Star.script:VBAProject.Modul1.Test_NumberFormat_DateTime?language=Basic&"
                 "location=document");

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
}
void VBAMacroTest::testTdf107902()
{
    loadFromURL(u"tdf107902.xlsm");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    //Without the fix in place, it would have failed with 'Unexpected dialog:  Error: BASIC runtime error.'
    executeMacro("vnd.sun.Star.script:VBAProject.Module1.AF?language=Basic&location=document");

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
}

void VBAMacroTest::testTdf90278()
{
    loadFromURL(u"tdf90278.xls");

    // Without the fix in place, changing the border weight
    // would cause a Basic exception/error in the following script.
    uno::Any aRet = executeMacro(
        "vnd.sun.Star.script:VBAProject.Module1.BorderWeight?language=Basic&location=document");

    // Check the border weight of the corresponding cell in the test document
    sal_Int32 aReturnValue;
    aRet >>= aReturnValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aReturnValue);
}

void VBAMacroTest::testTdf149531()
{
    loadFromURL(u"tdf149531.xls");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        // Without the fix in place, this test would have crashed
        // also check the test doesn't crash when the macro is executed a few times in a row
        executeMacro("vnd.sun.Star.script:VBAProject.Module1.SetColumnWidth?language=Basic&"
                     "location=document");
    }

    sal_uInt16 nWidth
        = o3tl::convert(rDoc.GetColWidth(0, 0), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(25749), nWidth);
}

void VBAMacroTest::testTdf118247()
{
    loadFromURL(u"tdf118247.xlsm");

    uno::Any aRet = executeMacro(
        "vnd.sun.Star.script:VBAProject.Module1.testXlSpecialCellsValuesConstantsEmpty?"
        "language=Basic&location=document");

    OUString aReturnValue;
    aRet >>= aReturnValue;
    CPPUNIT_ASSERT_EQUAL(OUString("$A$1:$A$3"), aReturnValue);

    const std::vector<std::pair<sal_Int32, OUString>> aTestParams(
        { { excel::XlSpecialCellsValue::xlNumbers, "$A$1:$A$2" },
          { excel::XlSpecialCellsValue::xlTextValues, "$A$3" },
          { excel::XlSpecialCellsValue::xlLogical, "$A$1:$A$2" },
          { excel::XlSpecialCellsValue::xlErrors, "$A$1:$A$4" } });

    for (auto & [ nXlSpecialCellsValue, sRange ] : aTestParams)
    {
        uno::Sequence<uno::Any> aParams = { uno::Any(nXlSpecialCellsValue) };
        aRet = executeMacro(
            "vnd.sun.Star.script:VBAProject.Module1.testXlSpecialCellsValuesConstants?"
            "language=Basic&location=document",
            aParams);
        aRet >>= aReturnValue;
        CPPUNIT_ASSERT_EQUAL(sRange, aReturnValue);
    }
}

void VBAMacroTest::testTdf126457()
{
    auto xComponent = loadFromDesktop("private:factory/scalc");

    // Save a copy of the file to get its URL
    uno::Reference<frame::XStorable> xDocStorable(xComponent, uno::UNO_QUERY);
    utl::TempFileNamed aTempFile(u"testWindowsActivate", true, u".ods");
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> descSaveAs(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(OUString("calc8")) } }));
    xDocStorable->storeAsURL(aTempFile.GetURL(), descSaveAs);

    // Insert initial library
    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(xComponent, uno::UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary("TestLibrary");
    xLibrary->insertByName(
        "TestModule",
        uno::Any(OUString("Option VBASupport 1\n"
                          "Function TestWindowsActivate\n"
                          "  dirName = Workbooks(1).Path\n"
                          "  workbookName = Workbooks(1).Name\n"
                          "  fileName = dirName + Application.PathSeparator + workbookName\n"
                          "  Workbooks.Open Filename := fileName\n"
                          "  On Error Goto handler\n"
                          // activate window using its URL
                          "  Windows(fileName).Activate\n"
                          // activate window using its caption name
                          "  Windows(workbookName).Activate\n"
                          // activate window using a newly generated window caption
                          "  newCaption = \"New Window Caption\"\n"
                          "  Windows(fileName).Caption = newCaption\n"
                          "  Windows(newCaption).Activate\n"
                          "  TestWindowsActivate = 0\n"
                          "  Exit Function\n"
                          "handler:\n"
                          "  TestWindowsActivate = 1\n"
                          "End Function\n")));

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    ErrCode result
        = SfxObjectShell::CallXScript(xComponent,
                                      "vnd.sun.Star.script:TestLibrary.TestModule."
                                      "TestWindowsActivate?language=Basic&location=document",
                                      {}, aRet, aOutParamIndex, aOutParam);

    // Without the fix in place, the windows could not be activated in the macro
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, result);
    sal_Int16 nReturnValue;
    aRet >>= nReturnValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nReturnValue);

    pDocSh->DoClose();
}
CPPUNIT_TEST_SUITE_REGISTRATION(VBAMacroTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
