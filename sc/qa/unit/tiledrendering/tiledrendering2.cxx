/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sctiledrenderingtest.hxx>

#include <com/sun/star/datatransfer/XTransferable2.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/lokhelper.hxx>
#include <vcl/scheduler.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <sctestviewcallback.hxx>
#include <docuno.hxx>
#include <scmod.hxx>
#include <tabvwsh.hxx>

using namespace com::sun::star;

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSidebarLocale)
{
    ScModelObj* pModelObj = createDoc("chart.ods");
    int nView1 = SfxLokHelper::getView();
    ScTestViewCallback aView1;
    SfxViewShell* pView1 = SfxViewShell::Current();
    pView1->SetLOKLocale(u"en-US"_ustr);
    SfxLokHelper::createView();
    ScTestViewCallback aView2;
    SfxViewShell* pView2 = SfxViewShell::Current();
    pView2->SetLOKLocale(u"de-DE"_ustr);
    TestLokCallbackWrapper::InitializeSidebar();
    Scheduler::ProcessEventsToIdle();
    aView2.m_aStateChanges.clear();

    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, /*x=*/1, /*y=*/1, /*count=*/2,
                              /*buttons=*/1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, /*x=*/1, /*y=*/1, /*count=*/2,
                              /*buttons=*/1, /*modifier=*/0);
    SfxLokHelper::setView(nView1);
    Scheduler::ProcessEventsToIdle();

    auto it = aView2.m_aStateChanges.find(".uno:Sidebar");
    CPPUNIT_ASSERT(it != aView2.m_aStateChanges.end());
    std::string aLocale = it->second.get<std::string>("locale");
    CPPUNIT_ASSERT_EQUAL(std::string("de-DE"), aLocale);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCopyMultiSelection)
{
    // Given a document with A1 and A3 as selected cells:
    ScModelObj* pModelObj = createDoc("multi-selection.ods");
    ScTestViewCallback aView1;
    // Get the center of A3:
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$3"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);
    Point aPoint = aView1.m_aCellCursorBounds.Center();
    // Go to A1:
    aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);
    // Ctrl-click on A3:
    int nCtrl = KEY_MOD1;
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
                              MOUSE_LEFT, nCtrl);

    // When getting the selection:
    uno::Reference<datatransfer::XTransferable> xTransferable = pModelObj->getSelection();

    // Make sure we get A1+A3 instead of an error:
    CPPUNIT_ASSERT(xTransferable.is());

    // Also make sure that just 2 cells is classified as a simple selection:
    uno::Reference<datatransfer::XTransferable2> xTransferable2(xTransferable, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTransferable2.is());
    // Without the fix, the text selection was complex.
    CPPUNIT_ASSERT(!xTransferable2->isComplex());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCursorJumpOnFailedSearch)
{
    createDoc("empty.ods");
    ScTestViewCallback aView;

    // Go to lower cell
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$C$3"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);

    tools::Rectangle aInitialCursor = aView.m_aCellCursorBounds;

    // Search for a non-existing string using the start point parameters
    aPropertyValues = comphelper::InitPropertySequence(
        { { "SearchItem.SearchString", uno::Any(u"No-existing"_ustr) },
          { "SearchItem.Backward", uno::Any(false) },
          { "SearchItem.SearchStartPointX", uno::Any(static_cast<sal_Int32>(100)) },
          { "SearchItem.SearchStartPointY", uno::Any(static_cast<sal_Int32>(100)) } });
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aPropertyValues);

    tools::Rectangle aFinalCursor = aView.m_aCellCursorBounds;

    // Without the fix, the cursor jumps even when no match is found
    CPPUNIT_ASSERT_EQUAL(aInitialCursor, aFinalCursor);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testLocaleFormulaSeparator)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    ScDocument* pDoc = pModelObj->GetDocument();

    ScAddress addr(2, 0, 0);
    typeCharsInCell("=subtotal(9,A1:A8", addr.Col(), addr.Row(), pView, pModelObj, false, true);
    // Without the fix it would fail with
    // - Expected: 0
    // - Actual  : Err:508
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, pDoc->GetString(addr));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDecimalSeparatorInfo)
{
    createDoc("decimal-separator.ods");

    ScTestViewCallback aView1;

    // Go to cell A1.
    uno::Sequence<beans::PropertyValue> aPropertyValues
        = { comphelper::makePropertyValue("ToPoint", OUString("$A$1")) };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);

    // Cell A1 has language set to English. Decimal separator should be ".".
    CPPUNIT_ASSERT_EQUAL(std::string("."), aView1.decimalSeparator);

    // Go to cell B1.
    aPropertyValues = { comphelper::makePropertyValue("ToPoint", OUString("B$1")) };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);

    // Cell B1 has language set to Turkish. Decimal separator should be ",".
    CPPUNIT_ASSERT_EQUAL(std::string(","), aView1.decimalSeparator);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCool11739LocaleDialogFieldUnit)
{
    createDoc("empty.ods");
    SfxViewShell* pView1 = SfxViewShell::Current();
    pView1->SetLOKLocale(u"fr-FR"_ustr);

    ScModule* pMod = ScModule::get();
    FieldUnit eMetric = pMod->GetMetric();

    // Without the fix, it fails with
    // - Expected: 2
    // - Actual  : 8
    // where 2 is FieldUnit::CM and 8 is FieldUnit::INCH
    CPPUNIT_ASSERT_EQUAL(FieldUnit::CM, eMetric);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSplitPanes)
{
    createDoc("split-panes.ods");

    save(u"calc8"_ustr);

    xmlDocUniquePtr pSettings = parseExport(u"settings.xml"_ustr);
    CPPUNIT_ASSERT(pSettings);

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 2
    assertXPathContent(pSettings,
                       "/office:document-settings/office:settings/config:config-item-set[1]/"
                       "config:config-item-map-indexed/config:config-item-map-entry/"
                       "config:config-item-map-named/config:config-item-map-entry/"
                       "config:config-item[@config:name='VerticalSplitMode']",
                       u"0");
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSplitPanesXLSX)
{
    createDoc("split-panes.xlsx");

    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // Without the fix in place, this test would have failed with
    // - Expected: topRight
    // - Actual  : bottomRight
    // which also results in invalid XLSX
    assertXPath(pSheet, "/x:worksheet/x:sheetViews/x:sheetView/x:pane", "activePane", u"topRight");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
