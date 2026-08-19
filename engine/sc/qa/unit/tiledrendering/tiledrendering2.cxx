/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sctiledrenderingtest.hxx>

#include <com/sun/star/datatransfer/XTransferable2.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/servicehelper.hxx>
#include <sfx2/kit/helper.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <tools/stream.hxx>
#include <vcl/scheduler.hxx>
#include <COKit/COKit.hxx>

#include <sctestviewcallback.hxx>
#include <docuno.hxx>
#include <document.hxx>
#include <scmod.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/hlnkitem.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <postit.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <comphelper/scopeguard.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svtools/colorcfg.hxx>
#include <drwlayer.hxx>
#include <editeng/flditem.hxx>
#include <o3tl/unit_conversion.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <drwlayer.hxx>
#include <drawview.hxx>

using namespace com::sun::star;

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSidebarLocale)
{
    ScModelObj* pModelObj = createDoc("chart.ods");
    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    SfxViewShell* pView1 = SfxViewShell::Current();
    pView1->SetKitLocale(u"en-US"_ustr);
    KitHelper::createView();
    ScTestViewCallback aView2;
    SfxViewShell* pView2 = SfxViewShell::Current();
    pView2->SetKitLocale(u"de-DE"_ustr);
    TestKitCallbackWrapper::InitializeSidebar();
    Scheduler::ProcessEventsToIdle();
    aView2.m_aStateChanges.clear();

    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONDOWN, /*x=*/1, /*y=*/1, /*count=*/2,
                              /*buttons=*/1, /*modifier=*/0);
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONUP, /*x=*/1, /*y=*/1, /*count=*/2,
                              /*buttons=*/1, /*modifier=*/0);
    KitHelper::setView(nView1);
    Scheduler::ProcessEventsToIdle();

    auto it = aView2.m_aStateChanges.find(".uno:SidebarDeck.PropertyDeck");
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
    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues = {
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
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
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
    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$C$3"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);

    tools::Rectangle aInitialCursor = aView.m_aCellCursorBounds;

    // Search for a non-existing string using the start point parameters
    aPropertyValues = comphelper::InitPropertySequence(
        { { u"SearchItem.SearchString"_ustr, cpo::uno::Any(u"No-existing"_ustr) },
          { u"SearchItem.Backward"_ustr, cpo::uno::Any(false) },
          { u"SearchItem.SearchStartPointX"_ustr, cpo::uno::Any(static_cast<sal_Int32>(100)) },
          { u"SearchItem.SearchStartPointY"_ustr, cpo::uno::Any(static_cast<sal_Int32>(100)) } });
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
    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues
        = { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1"_ustr) };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);

    // Cell A1 has language set to English. Decimal separator should be ".".
    CPPUNIT_ASSERT_EQUAL(std::string("."), aView1.decimalSeparator);

    // Go to cell B1.
    aPropertyValues = { comphelper::makePropertyValue(u"ToPoint"_ustr, u"B$1"_ustr) };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);

    // Cell B1 has language set to Turkish. Decimal separator should be ",".
    CPPUNIT_ASSERT_EQUAL(std::string(","), aView1.decimalSeparator);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCool11739LocaleDialogFieldUnit)
{
    createDoc("empty.ods");
    SfxViewShell* pView1 = SfxViewShell::Current();
    pView1->SetKitLocale(u"fr-FR"_ustr);

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

    save(TestFilter::ODS);

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

    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // Without the fix in place, this test would have failed with
    // - Expected: topRight
    // - Actual  : bottomRight
    // which also results in invalid XLSX
    assertXPath(pSheet, "/x:worksheet/x:sheetViews/x:sheetView/x:pane", "activePane", u"topRight");
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTdf167042)
{
    ScModelObj* pModelObj = createDoc("tdf167042.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    ScTestViewCallback aView1;

    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues
        = { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1"_ustr) };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);

    Point aPoint = aView1.m_aCellCursorBounds.Center();

    aPropertyValues = { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$B$1"_ustr) };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);

    // Check that we have the comment on A1
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    ScPostIt* pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pNote->GetText());

    cpo::uno::Sequence aArgs{ comphelper::makePropertyValue(u"PersistentCopy"_ustr, false) };
    dispatchCommand(mxComponent, u".uno:FormatPaintbrush"_ustr, aArgs);

    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
                              MOUSE_LEFT, 0);
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONUP, aPoint.getX(), aPoint.getY(), 1,
                              MOUSE_LEFT, 0);

    // Check that FormatPaintbrush worked
    vcl::Font aFont;
    pDoc->GetPattern(0, 0, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold A1", WEIGHT_BOLD,
                                 aFont.GetWeightMaybeAskConfig());

    // Check that we still have the comment on A1 after FormatPaintbrush
    pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pNote->GetText());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Check that we still have the comment on A1 after Undo
    pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    // Without the fix in place, this test would have failed with
    // - Expected : test1
    // - Actual :
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pNote->GetText());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testBreakPageView)
{
    // Give a file in page break view mode
    ScModelObj* pModelObj = createDoc("page-view.xlsx");

    // Paint a tile not at the top or left
    size_t nCanvasSize = 1024;
    size_t nTileSize = 256;
    std::vector<unsigned char> aPixmap(nCanvasSize * nCanvasSize * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> xDevice(DeviceFormat::WITHOUT_ALPHA);
    xDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    xDevice->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasSize, nCanvasSize), 1.0, Point(),
                                                       aPixmap.data());
    pModelObj->paintTile(*xDevice, nCanvasSize, nCanvasSize, 3840, 3840, 3840, 3840);
    xDevice->EnableMapMode(false);
    Bitmap aBitmap = xDevice->GetBitmap(Point(0, 0), Size(nTileSize, nTileSize));

    // Top left pixel should be white, not blue
    BitmapScopedReadAccess pAccess(aBitmap);
    Color aColor(pAccess->GetPixel(0, 0));
    // Without the fix in place, this test fails with
    // - Expected: rgba[ffffffff]
    // - Actual  : rgba[000080ff]
    CPPUNIT_ASSERT_EQUAL(Color(255, 255, 255), aColor);
}

static void lcl_drag(ScModelObj* pModelObj, Point aFrom, Point aTo, int nModifier)
{
    // No modifiers in button down to start a drag
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONDOWN, aFrom.getX(), aFrom.getY(), 1,
                              MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    // Need a clear motion so it is detected as a drag
    const int nSteps = 5;
    for (int i = 1; i <= nSteps; ++i)
    {
        pModelObj->postMouseEvent(
            COKitMouseEventType::MOVE, aFrom.getX() + (aTo.getX() - aFrom.getX()) * i / nSteps,
            aFrom.getY() + (aTo.getY() - aFrom.getY()) * i / nSteps, 1, MOUSE_LEFT, nModifier);
        Scheduler::ProcessEventsToIdle();
    }
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONUP, aTo.getX(), aTo.getY(), 1, MOUSE_LEFT,
                              nModifier);
    Scheduler::ProcessEventsToIdle();
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDragDropMove)
{
    // Document has A1=A, B1=B, A2=C, B2=D, D1=E, D2=F.
    ScModelObj* pModelObj = createDoc("cell_drag.fods");
    ScTestViewCallback aView;
    ScDocument* pDoc = pModelObj->GetDocument();

    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1"_ustr) });
    Point aFrom = aView.m_aCellCursorBounds.TopLeft() + Point(10, 10);
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$D$1"_ustr) });
    Point aTo = aView.m_aCellCursorBounds.TopLeft() + Point(10, 10);

    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1:$B$2"_ustr) });
    Scheduler::ProcessEventsToIdle();

    lcl_drag(pModelObj, aFrom, aTo, 0);

    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, pDoc->GetString(ScAddress(3, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, pDoc->GetString(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"C"_ustr, pDoc->GetString(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"D"_ustr, pDoc->GetString(ScAddress(4, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDragDropCopy)
{
    ScModelObj* pModelObj = createDoc("cell_drag.fods");
    ScTestViewCallback aView;
    ScDocument* pDoc = pModelObj->GetDocument();

    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1"_ustr) });
    Point aFrom = aView.m_aCellCursorBounds.TopLeft() + Point(10, 10);
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$D$1"_ustr) });
    Point aTo = aView.m_aCellCursorBounds.TopLeft() + Point(10, 10);

    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1:$B$2"_ustr) });
    Scheduler::ProcessEventsToIdle();

    lcl_drag(pModelObj, aFrom, aTo, KEY_MOD1); // Ctrl

    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"C"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"D"_ustr, pDoc->GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, pDoc->GetString(ScAddress(3, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, pDoc->GetString(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"C"_ustr, pDoc->GetString(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"D"_ustr, pDoc->GetString(ScAddress(4, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDragDropInsert)
{
    // Alt-drag A1:B1 to D2: insert-move into a different row.
    // F at D2 should be pushed right, and A/B placed at D2:E2.
    // Row 1 source is cleared; row 2 is not affected by the cleanup.
    ScModelObj* pModelObj = createDoc("cell_drag.fods");
    ScTestViewCallback aView;
    ScDocument* pDoc = pModelObj->GetDocument();

    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1"_ustr) });
    Point aFrom = aView.m_aCellCursorBounds.TopLeft() + Point(10, 10);
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$D$2"_ustr) });
    Point aTo = aView.m_aCellCursorBounds.TopLeft() + Point(10, 10);

    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1:$B$1"_ustr) });
    Scheduler::ProcessEventsToIdle();

    lcl_drag(pModelObj, aFrom, aTo, KEY_MOD2); // Alt

    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(0, 0, 0))); // A1
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(1, 0, 0))); // B1
    CPPUNIT_ASSERT_EQUAL(u"E"_ustr, pDoc->GetString(ScAddress(3, 0, 0))); // D1
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, pDoc->GetString(ScAddress(3, 1, 0))); // D2
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, pDoc->GetString(ScAddress(4, 1, 0))); // E2
    CPPUNIT_ASSERT_EQUAL(u"F"_ustr, pDoc->GetString(ScAddress(5, 1, 0))); // F2 (was D2)
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testPageBackgroundRemoteNotFetched)
{
    // Page background image with a remote URL must not fetch
    // the URL during paint when link updates are not allowed.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"page-background-link.fods"), aParams);
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pModelObj->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testShapeBackgroundRemoteNotFetched)
{
    // Shape fill bitmap with a remote URL must not fetch
    // the URL during paint when link updates are not allowed.
    // The assert in createNewSdrFillGraphicAttribute will fire if
    // a remote fetch is attempted.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"shape-background-link.fods"), aParams);
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    // The shape's deferred remote fill bitmap is registered as an external
    // link as the shape is imported, so it appears in Edit, Links to External
    // Files and can be updated or broken.
    const sfx2::LinkManager* pLinkMgr = pModelObj->GetDocument()->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_MESSAGE("shape fill bitmap link should be registered",
                           !pLinkMgr->GetLinks().empty());

    pModelObj->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pModelObj->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testBulletImageRemoteNotFetched)
{
    // text:list-level-style-image with a remote URL must not fetch
    // the URL during paint when link updates are not allowed.
    // Currently the editeng rendering path silently skips unresolved
    // GraphicExternalLink graphics. If someone adds fetching here,
    // this test should catch it.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"bullet-image-link.fods"), aParams);
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pModelObj->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testFormImageRemoteNotFetched)
{
    // Form image button with a remote ImageURL must not fetch the
    // URL during import when link updates are not allowed.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"form-image-link.fods"), aParams);
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pModelObj->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testEditTextPaintStartInTiledMode)
{
    // A short right-aligned number in a cell far from the left of the sheet is
    // edited in place. It fits its column, so the in-edit text stays right
    // anchored and the edit view's visible-document left is large. In tiled
    // rendering the paint must subtract that offset so the right-anchored text
    // lands on the cell's own tile. Skipping the subtraction (an earlier broad
    // fix did that for every kit edit) draws the text off the tile, so the cell
    // rendered blank while typing.
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    // a short number in a cell far to the right (column AI, row 736)
    typeCharsInCell("12345", 34, 735, pView, pModelObj, /*bInEdit*/ false, /*bCommit*/ false);
    Scheduler::ProcessEventsToIdle();

    ScViewData& rVD = pView->GetViewData();
    EditView* pEditView = rVD.GetEditView(rVD.GetEditActivePart());
    CPPUNIT_ASSERT(pEditView);

    // a right-anchored cell this far right has a large visible-document left
    const tools::Long nVisDocLeft = pEditView->GetVisArea().Left();
    CPPUNIT_ASSERT_MESSAGE("precondition: far right-aligned cell has a non-zero "
                           "visible-document left",
                           nVisDocLeft > 0);

    // the paint start subtracts that offset, mapping the right-anchored text onto
    // the cell, rather than staying at the output area top-left
    const Point aStartPos = pEditView->CalculateTextPaintStartPosition();
    CPPUNIT_ASSERT_EQUAL(pEditView->GetOutputArea().Left() - nVisDocLeft, aStartPos.X());
}

namespace
{
// A text field takes up a single character of the paragraph text.
constexpr sal_Unicode CH_FIELD = u'\x0001';

// Reads a cell back as the text it displays, with each hyperlink written out as
// [displayed text](url), so that an assertion shows how much of the cell the link covers.
OUString lcl_getCellTextWithLinks(ScDocument& rDocument, const ScAddress& rPosition)
{
    const EditTextObject* pEditText = rDocument.GetEditText(rPosition);
    if (!pEditText)
        return rDocument.GetString(rPosition);

    OUStringBuffer aBuffer;
    for (sal_Int32 nPara = 0; nPara < pEditText->GetParagraphCount(); ++nPara)
    {
        if (nPara > 0)
            aBuffer.append('\n');

        const OUString aParaText = pEditText->GetText(nPara);
        size_t nField = 0;
        for (sal_Int32 nIndex = 0; nIndex < aParaText.getLength(); ++nIndex)
        {
            if (aParaText[nIndex] != CH_FIELD)
            {
                aBuffer.append(aParaText[nIndex]);
                continue;
            }

            const SvxFieldData* pField
                = pEditText->GetFieldData(nPara, nField++, text::textfield::Type::URL);
            auto pURLField = dynamic_cast<const SvxURLField*>(pField);
            CPPUNIT_ASSERT(pURLField);
            aBuffer.append("[" + pURLField->GetRepresentation() + "](" + pURLField->GetURL() + ")");
        }
    }

    return aBuffer.makeStringAndClear();
}

// Places the caret in the text of the cell that is being edited, and selects from nStart to
// nEnd. Clicking into the text is what puts the cell into the table input mode, so the mode is
// set here too.
void lcl_selectInCell(ScTabViewShell* pView, sal_Int32 nStart, sal_Int32 nEnd)
{
    ScModule::get()->SetInputMode(SC_INPUT_TABLE);

    ScViewData& rViewData = pView->GetViewData();
    EditView* pEditView = rViewData.GetEditView(rViewData.GetEditActivePart());
    CPPUNIT_ASSERT(pEditView);
    pEditView->SetSelection(ESelection(0, nStart, 0, nEnd));
}

// The text the hyperlink dialog would show in its Text entry for the current selection.
OUString lcl_getHyperlinkDialogText(ScTabViewShell* pView)
{
    std::unique_ptr<SvxHyperlinkItem> pState;
    pView->GetViewFrame().GetBindings().QueryState(SID_HYPERLINK_GETLINK, pState);
    CPPUNIT_ASSERT(pState);
    return pState->GetName();
}

cpo::uno::Sequence<beans::PropertyValue> lcl_hyperlinkArgs(const OUString& rText,
                                                           const OUString& rURL)
{
    return {
        comphelper::makePropertyValue(u"Hyperlink.Text"_ustr, cpo::uno::Any(rText)),
        comphelper::makePropertyValue(u"Hyperlink.URL"_ustr, cpo::uno::Any(rURL)),
    };
}
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInsertHyperlinkOverSelectionInCell)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    ScDocument* pDoc = pModelObj->GetDocument();

    const ScAddress aA1(0, 0, 0);
    typeCharsInCell("Docs and reference", aA1.Col(), aA1.Row(), pView, pModelObj, /*bInEdit*/ false,
                    /*bCommit*/ true);
    pView->SetCursor(aA1.Col(), aA1.Row());

    // the word "reference" is selected inside the cell
    lcl_selectInCell(pView, 9, 18);

    // the dialog offers the selected text, so that confirming it keeps that word as the label
    CPPUNIT_ASSERT_EQUAL(u"reference"_ustr, lcl_getHyperlinkDialogText(pView));

    dispatchCommand(mxComponent, u".uno:SetHyperlink"_ustr,
                    lcl_hyperlinkArgs(u"reference"_ustr, u"http://www.example.com/"_ustr));
    ScModule::get()->InputEnterHandler();

    // Only the selected word becomes the link. Without the fix the whole cell did:
    // - Expected: Docs and [reference](http://www.example.com/)
    // - Actual  : [Docs and reference](http://www.example.com/)
    CPPUNIT_ASSERT_EQUAL(u"Docs and [reference](http://www.example.com/)"_ustr,
                         lcl_getCellTextWithLinks(*pDoc, aA1));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInsertHyperlinkWithoutSelectionCoversWholeCell)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    ScDocument* pDoc = pModelObj->GetDocument();

    const ScAddress aA1(0, 0, 0);
    typeCharsInCell("Docs and reference", aA1.Col(), aA1.Row(), pView, pModelObj, /*bInEdit*/ false,
                    /*bCommit*/ true);
    pView->SetCursor(aA1.Col(), aA1.Row());

    // the caret sits in the cell with nothing selected
    lcl_selectInCell(pView, 18, 18);

    // with nothing selected the dialog offers the whole cell
    CPPUNIT_ASSERT_EQUAL(u"Docs and reference"_ustr, lcl_getHyperlinkDialogText(pView));

    dispatchCommand(mxComponent, u".uno:SetHyperlink"_ustr,
                    lcl_hyperlinkArgs(u"Docs and reference"_ustr, u"http://www.example.com/"_ustr));
    ScModule::get()->InputEnterHandler();

    // the link covers the whole cell
    CPPUNIT_ASSERT_EQUAL(u"[Docs and reference](http://www.example.com/)"_ustr,
                         lcl_getCellTextWithLinks(*pDoc, aA1));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInsertHyperlinkInOOXMLCoversWholeCell)
{
    // An OOXML format stores a single hyperlink per cell and no link on part of a cell's text,
    // so a link there covers the whole cell even when only some of the text is selected.
    ScModelObj* pModelObj = createDoc("empty.xlsx");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    ScDocument* pDoc = pModelObj->GetDocument();

    const ScAddress aA1(0, 0, 0);
    typeCharsInCell("Docs and reference", aA1.Col(), aA1.Row(), pView, pModelObj, /*bInEdit*/ false,
                    /*bCommit*/ true);
    pView->SetCursor(aA1.Col(), aA1.Row());

    lcl_selectInCell(pView, 9, 18);

    // the dialog offers the whole cell, not the selected word
    CPPUNIT_ASSERT_EQUAL(u"Docs and reference"_ustr, lcl_getHyperlinkDialogText(pView));

    dispatchCommand(mxComponent, u".uno:SetHyperlink"_ustr,
                    lcl_hyperlinkArgs(u"Docs and reference"_ustr, u"http://www.example.com/"_ustr));
    ScModule::get()->InputEnterHandler();

    CPPUNIT_ASSERT_EQUAL(u"[Docs and reference](http://www.example.com/)"_ustr,
                         lcl_getCellTextWithLinks(*pDoc, aA1));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInsertHyperlinkDropsTheCellsOtherLink)
{
    // A cell holds a single hyperlink, so linking a selection turns the link the rest of the
    // cell carries back into plain text.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    ScDocument* pDoc = pModelObj->GetDocument();

    const ScAddress aA1(0, 0, 0);
    typeCharsInCell("Docs and reference", aA1.Col(), aA1.Row(), pView, pModelObj, /*bInEdit*/ false,
                    /*bCommit*/ true);
    pView->SetCursor(aA1.Col(), aA1.Row());

    // "Docs" is linked first
    lcl_selectInCell(pView, 0, 4);
    dispatchCommand(mxComponent, u".uno:SetHyperlink"_ustr,
                    lcl_hyperlinkArgs(u"Docs"_ustr, u"http://www.example.com/docs"_ustr));

    // "reference" is linked next. The field left by the first link takes up one character, so
    // the word now starts at index 6.
    lcl_selectInCell(pView, 6, 15);
    dispatchCommand(mxComponent, u".uno:SetHyperlink"_ustr,
                    lcl_hyperlinkArgs(u"reference"_ustr, u"http://www.example.com/reference"_ustr));
    ScModule::get()->InputEnterHandler();

    // the newer link stays and "Docs" is left as plain text
    CPPUNIT_ASSERT_EQUAL(u"Docs and [reference](http://www.example.com/reference)"_ustr,
                         lcl_getCellTextWithLinks(*pDoc, aA1));

    // both links go away together, because they were added in one edit of the cell
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Docs and reference"_ustr, lcl_getCellTextWithLinks(*pDoc, aA1));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testHyperlinkOverSelectionSavedToOOXML)
{
    // A link on part of a cell's text is what ODF stores. An OOXML format has nowhere to put
    // it, so on the way out the link grows to cover the whole cell.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    const ScAddress aA1(0, 0, 0);
    typeCharsInCell("Docs and references", aA1.Col(), aA1.Row(), pView, pModelObj,
                    /*bInEdit*/ false, /*bCommit*/ true);
    pView->SetCursor(aA1.Col(), aA1.Row());

    // the word "references" is selected and linked
    lcl_selectInCell(pView, 9, 19);
    dispatchCommand(mxComponent, u".uno:SetHyperlink"_ustr,
                    lcl_hyperlinkArgs(u"references"_ustr, u"http://www.example.com/"_ustr));
    ScModule::get()->InputEnterHandler();

    CPPUNIT_ASSERT_EQUAL(u"Docs and [references](http://www.example.com/)"_ustr,
                         lcl_getCellTextWithLinks(*pModelObj->GetDocument(), aA1));

    saveAndReload(TestFilter::XLSX);

    ScDocument* pReloadedDoc = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent)->GetDocument();
    CPPUNIT_ASSERT(pReloadedDoc);

    // the whole cell carries the link now, and the text it displays is unchanged
    CPPUNIT_ASSERT_EQUAL(u"[Docs and references](http://www.example.com/)"_ustr,
                         lcl_getCellTextWithLinks(*pReloadedDoc, aA1));
}

// The automatic font color of a shape being edited has to be resolved against the fill of
// that shape, not against the page background
CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testShapeTextEditAutoColorOnDarkPage)
{
    // Register a dark scheme, so that .uno:ChangeTheme "Dark" gives a dark page background
    {
        svtools::EditableColorConfig aColorConfig;
        svtools::ColorConfigValue aValue;
        aValue.bIsVisible = true;
        aValue.nColor = Color(0x1c, 0x1c, 0x1c);
        aColorConfig.SetColorValue(svtools::DOCCOLOR, aValue);
        aColorConfig.AddScheme(u"Dark"_ustr);
    }
    ScModelObj* pModelObj = createDoc("shape.ods");

    // Give the document a dark page background
    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { u"NewTheme"_ustr, cpo::uno::Any(u"Dark"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:ChangeTheme"_ustr, aPropertyValues);

    const ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    SdrPage* pDrawPage = pViewData->GetDocument().GetDrawLayer()->GetPage(0);
    SdrObject* pObject = pDrawPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);

    // Fill the shape with white
    pObject->SetMergedItem(XFillStyleItem(drawing::FillStyle_SOLID));
    pObject->SetMergedItem(XFillColorItem(OUString(), COL_WHITE));

    SdrView* pView = pViewData->GetViewShell()->GetScDrawView();
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());

    // Render, so the text edit paint path picks the background to resolve the auto color against
    {
        size_t nCanvasSize = 1024;
        std::vector<unsigned char> aPixmap(nCanvasSize * nCanvasSize * 4, 0);
        ScopedVclPtrInstance<VirtualDevice> xDevice(DeviceFormat::WITHOUT_ALPHA);
        xDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
        xDevice->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasSize, nCanvasSize), 1.0,
                                                           Point(), aPixmap.data());
        pModelObj->paintTile(*xDevice, nCanvasSize, nCanvasSize, 0, 0, 15360, 7680);
    }

    // Without the accompanying fix this was COL_WHITE, i.e. white text on the white shape while
    // the shape was edited, turning dark only once text edit ended
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pView->GetTextEditOutliner()->GetEditEngine().GetAutoColor());
    pView->SdrEndTextEdit();
}

// This lives apart from the other tile tests because rendering a document with a chart
// leaves a shell of the chart behind, which upsets tests that ask for the render state
// of the current view afterwards.
// In dark mode a chart that has no background of its own follows the dark document
// background, the same as the sheet around it does.
CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testChartBackgroundFollowsDarkMode)
{
    const Color aDarkColor(0x1c, 0x1c, 0x1c);
    {
        svtools::EditableColorConfig aColorConfig;
        svtools::ColorConfigValue aValue;
        aValue.bIsVisible = true;
        aValue.nColor = aDarkColor;
        aColorConfig.SetColorValue(svtools::DOCCOLOR, aValue);
        aColorConfig.AddScheme(u"DarkTest"_ustr);
    }

    // The chart of this document sits at the top left corner of the sheet and is bigger than
    // the tile rendered below, so the whole tile is chart
    ScModelObj* pModelObj = createDoc("chart.ods");
    ScTestViewCallback aView;
    const OUString aOldScheme(svtools::EditableColorConfig().GetCurrentSchemeName());
    comphelper::ScopeGuard aRestoreScheme([this, aOldScheme] {
        dispatchCommand(
            mxComponent, u".uno:ChangeTheme"_ustr,
            comphelper::InitPropertySequence({ { u"NewTheme"_ustr, cpo::uno::Any(aOldScheme) } }));
    });
    dispatchCommand(
        mxComponent, u".uno:ChangeTheme"_ustr,
        comphelper::InitPropertySequence({ { u"NewTheme"_ustr, cpo::uno::Any(u"DarkTest"_ustr) } }));

    // The rest of the test says nothing unless the document background really is dark now
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    CPPUNIT_ASSERT(pViewShell);
    CPPUNIT_ASSERT_EQUAL(aDarkColor, pViewShell->GetColorConfigColor(svtools::DOCCOLOR));

    Bitmap aBitmap = getTile(pModelObj, 0, 0, 3840, 3840);
    BitmapScopedReadAccess pAccess(aBitmap);

    // Without the accompanying fix the automatic background of the chart was not resolved for
    // the rendering view, so the chart stayed light while the sheet around it went dark
    CPPUNIT_ASSERT_EQUAL(aDarkColor, Color(pAccess->GetPixel(5, 5)));
}

// This lives next to testChartBackgroundFollowsDarkMode() for the same reason, rendering a
// document with a chart leaves a shell of the chart behind.
// The automatic font color of a chart is chosen for contrast against the background it sits
// on, which for a chart with no background of its own is the document background of the view.
// A session renders the document before the theme of its view arrives, so what the first
// render made of an automatic color must not be what every later render gets.
CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testChartTextFollowsDarkMode)
{
    const Color aDarkColor(0x1c, 0x1c, 0x1c);
    const OUString aOldScheme(svtools::EditableColorConfig().GetCurrentSchemeName());
    {
        // Add a dark scheme, but leave the one the document is loaded with in charge
        svtools::EditableColorConfig aColorConfig;
        svtools::ColorConfigValue aValue;
        aValue.bIsVisible = true;
        aValue.nColor = aDarkColor;
        aColorConfig.SetColorValue(svtools::DOCCOLOR, aValue);
        aColorConfig.AddScheme(u"DarkTest"_ustr);
        aColorConfig.LoadScheme(aOldScheme);
    }

    // The chart of this document sits at the top left corner of the sheet and is bigger than
    // the tile rendered below, so the whole tile is chart
    ScModelObj* pModelObj = createDoc("chart.ods");
    ScTestViewCallback aView;
    comphelper::ScopeGuard aRestoreScheme([this, aOldScheme] {
        dispatchCommand(
            mxComponent, u".uno:ChangeTheme"_ustr,
            comphelper::InitPropertySequence({ { u"NewTheme"_ustr, cpo::uno::Any(aOldScheme) } }));
    });

    // Render it once before the view has a theme, as a session does
    getTile(pModelObj, 0, 0, 3840, 3840);

    dispatchCommand(
        mxComponent, u".uno:ChangeTheme"_ustr,
        comphelper::InitPropertySequence({ { u"NewTheme"_ustr, cpo::uno::Any(u"DarkTest"_ustr) } }));

    // The rest of the test says nothing unless the document background really is dark now
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    CPPUNIT_ASSERT(pViewShell);
    CPPUNIT_ASSERT_EQUAL(aDarkColor, pViewShell->GetColorConfigColor(svtools::DOCCOLOR));

    Bitmap aBitmap = getTile(pModelObj, 0, 0, 3840, 3840);
    BitmapScopedReadAccess pAccess(aBitmap);

    // The top left corner of the chart holds the topmost label of the value axis and nothing
    // else, so a pixel that is not the dark background there is the text
    bool bLightText(false);
    for (tools::Long nY = 0; nY < 90 && !bLightText; ++nY)
    {
        for (tools::Long nX = 0; nX < 120 && !bLightText; ++nX)
            bLightText = Color(pAccess->GetPixel(nY, nX)).IsBright();
    }

    // Without the accompanying fix the text kept the color the first render had picked for it,
    // so the chart of a dark sheet was labelled in black
    CPPUNIT_ASSERT(bLightText);
}

// A dark document background is what the view of a session looks like, not what its paper
// looks like, so the automatic text color of a chart has to be picked for white when the
// target is a PDF rather than a view.
CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testChartTextNotExportedDarkToPdf)
{
    const Color aDarkColor(0x1c, 0x1c, 0x1c);
    const OUString aOldScheme(svtools::EditableColorConfig().GetCurrentSchemeName());
    {
        svtools::EditableColorConfig aColorConfig;
        svtools::ColorConfigValue aValue;
        aValue.bIsVisible = true;
        aValue.nColor = aDarkColor;
        aColorConfig.SetColorValue(svtools::DOCCOLOR, aValue);
        aColorConfig.AddScheme(u"DarkTest"_ustr);
        aColorConfig.LoadScheme(aOldScheme);
    }

    ScModelObj* pModelObj = createDoc("chart.ods");
    ScTestViewCallback aView;
    comphelper::ScopeGuard aRestoreScheme([this, aOldScheme] {
        dispatchCommand(
            mxComponent, u".uno:ChangeTheme"_ustr,
            comphelper::InitPropertySequence({ { u"NewTheme"_ustr, cpo::uno::Any(aOldScheme) } }));
    });
    dispatchCommand(
        mxComponent, u".uno:ChangeTheme"_ustr,
        comphelper::InitPropertySequence({ { u"NewTheme"_ustr, cpo::uno::Any(u"DarkTest"_ustr) } }));

    const SfxViewShell* pViewShell = SfxViewShell::Current();
    CPPUNIT_ASSERT(pViewShell);
    CPPUNIT_ASSERT_EQUAL(aDarkColor, pViewShell->GetColorConfigColor(svtools::DOCCOLOR));

    // The session looks at the chart before it exports it
    getTile(pModelObj, 0, 0, 3840, 3840);

    cpo::uno::Sequence<css::beans::PropertyValue> aArgs{
        comphelper::makePropertyValue(u"SynchronMode"_ustr, true),
        comphelper::makePropertyValue(u"URL"_ustr, maTempFile.GetURL())
    };
    dispatchCommand(mxComponent, u".uno:ExportDirectToPDF"_ustr, aArgs);

    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    SvFileStream aPDFFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aPDFFile);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    CPPUNIT_ASSERT(pPdfDocument->getPageCount() > 0);
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    // Without the accompanying fix the labels of the chart came out in the white the dark view
    // needs, so the paper had white text on white
    int nTexts(0);
    for (int nObject = 0; nObject < pPage->getObjectCount(); ++nObject)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(nObject);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Text)
            continue;
        ++nTexts;
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPageObject->getFillColor());
    }

    // and the assert above says nothing unless the chart labels really are in there
    CPPUNIT_ASSERT(nTexts > 0);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testPasteShapeAtCellCursor)
{
    // A shape copied and then pasted with the cell cursor further down the sheet lands at the
    // cursor, not back on top of the shape it was copied from.
    ScModelObj* pModelObj = createDoc("shape-paste-position.fods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    const tools::Rectangle aSourceRectangle = pPage->GetObj(0)->GetSnapRect();

    ScDrawView* pDrawView = pView->GetScDrawView();
    CPPUNIT_ASSERT(pDrawView);
    pDrawView->MarkObj(pPage->GetObj(0), pDrawView->GetSdrPageView());
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // The shape is left behind and the cell cursor goes to A100, far enough down that a paste
    // ignoring the cursor is easy to tell apart from one honouring it.
    pDrawView->UnmarkAllObj();
    const SCROW nTargetRow = 99;
    pView->SetCursor(0, nTargetRow);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    const tools::Rectangle aPastedRectangle = pPage->GetObj(1)->GetSnapRect();

    // The copy is clear of the original, which it used to sit exactly on top of.
    CPPUNIT_ASSERT_GREATER(aSourceRectangle.Bottom(), aPastedRectangle.Top());

    // It sits on the cursor row. A pasted object is centred on the paste point, so it reaches
    // half its own height above the top of that row.
    const tools::Rectangle aTargetCell = pDoc->GetMMRect(0, nTargetRow, 0, nTargetRow, 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(aTargetCell.Top()), double(aPastedRectangle.Center().Y()),
                                 double(aSourceRectangle.GetHeight()));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
