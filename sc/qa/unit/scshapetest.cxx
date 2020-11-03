/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>
#include <unotools/tempfile.hxx>

#include <docsh.hxx>
#include <drwlayer.hxx>
#include <tabvwsh.hxx>
#include <userdat.hxx>

#include <sc.hrc> // defines of slot-IDs

using namespace css;

namespace sc_apitest
{
class ScShapeTest : public CalcUnoApiTest
{
public:
    ScShapeTest();
    void saveAndReload(css::uno::Reference<css::lang::XComponent>& xComponent,
                       const OUString& rFilter);

    void testTdf137355_UndoHideRows();
    void testTdf115655_HideDetail();
    void testFitToCellSize();
    void testCustomShapeCellAnchoredRotatedShape();

    CPPUNIT_TEST_SUITE(ScShapeTest);
    CPPUNIT_TEST(testTdf137355_UndoHideRows);
    CPPUNIT_TEST(testTdf115655_HideDetail);
    CPPUNIT_TEST(testFitToCellSize);
    CPPUNIT_TEST(testCustomShapeCellAnchoredRotatedShape);
    CPPUNIT_TEST_SUITE_END();
};

ScShapeTest::ScShapeTest()
    : CalcUnoApiTest("sc/qa/unit/data/ods")
{
}

void ScShapeTest::saveAndReload(css::uno::Reference<css::lang::XComponent>& xComponent,
                                const OUString& rFilter)
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    css::uno::Sequence<css::beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= rFilter; // e.g. "calc8"
    css::uno::Reference<css::frame::XStorable> xStorable(xComponent, css::uno::UNO_QUERY_THROW);
    xStorable->storeAsURL(aTempFile.GetURL(), aArgs);
    css::uno::Reference<css::util::XCloseable> xCloseable(xComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(true);

    xComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.sheet.SpreadsheetDocument");
}

static void lcl_AssertRectEqualWithTolerance(const OString& sInfo,
                                             const tools::Rectangle& rExpected,
                                             const tools::Rectangle& rActual,
                                             const sal_Int32 nTolerance)
{
    // Left
    OString sMsg = sInfo + " Left expected " + OString::number(rExpected.Left()) + " actual "
                   + OString::number(rActual.Left()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), labs(rExpected.Left() - rActual.Left()) <= nTolerance);

    // Top
    sMsg = sInfo + " Top expected " + OString::number(rExpected.Top()) + " actual "
           + OString::number(rActual.Top()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), labs(rExpected.Top() - rActual.Top()) <= nTolerance);

    // Width
    sMsg = sInfo + " Width expected " + OString::number(rExpected.GetWidth()) + " actual "
           + OString::number(rActual.GetWidth()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           labs(rExpected.GetWidth() - rActual.GetWidth()) <= nTolerance);

    // Height
    sMsg = sInfo + " Height expected " + OString::number(rExpected.GetHeight()) + " actual "
           + OString::number(rActual.GetHeight()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           labs(rExpected.GetHeight() - rActual.GetHeight()) <= nTolerance);
}

void ScShapeTest::testTdf137355_UndoHideRows()
{
    // The document contains a shape anchored "To Cell" with start in cell C3 and end in cell D6.
    // Error was, that hiding rows 3 to 6 and undo that action "lost" the shape.
    // Actually it was not lost but hidden.
    OUString aFileURL;
    createFileURL("tdf137355_UndoHideRows.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No object found", pObj);
    CPPUNIT_ASSERT_MESSAGE("Load: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectOrig(pObj->GetSnapRect());

    // Hide rows 3 to 6 in UI. [Note: Simple rDoc.SetRowHidden(2,5,0,true) does not work, because it
    // does not produce the needed undo items.]
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToPoint", OUString("$A$3:$A$6")),
    };
    dispatchCommand(xComponent, ".uno:GoToCell", aPropertyValues);

    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().GetDispatcher().Execute(FID_ROW_HIDE);

    // Check object is invisible
    CPPUNIT_ASSERT_MESSAGE("Hide: Object should be invisible", !pObj->IsVisible());

    // Undo
    pViewShell->GetViewData().GetDispatcher().Execute(SID_UNDO);

    // Check object is visible and has old size
    CPPUNIT_ASSERT_MESSAGE("Undo: Object should exist", pObj);
    CPPUNIT_ASSERT_MESSAGE("Undo: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectUndo(pObj->GetSnapRect());
    lcl_AssertRectEqualWithTolerance("Undo: Object geometry should not change", aSnapRectOrig,
                                     aSnapRectUndo, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testTdf115655_HideDetail()
{
    // The document contains an image inside a cell anchored "To Cell (resize with cell)". The cell
    // belongs to a group. On loading the group is expanded.
    // Error was, that after collapsing the group, save and reload, and exanding the group, the image
    // was "lost". Actually is was resized to zero height.
    OUString aFileURL;
    createFileURL("tdf115655_HideDetail.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get ScDocShell
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and image
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No object found", pObj);

    // Get image size
    tools::Rectangle aSnapRectOrig = pObj->GetSnapRect();

    // Collapse the group
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("Load: No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(1);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_HIDE);
    CPPUNIT_ASSERT_MESSAGE("Collapse: Image should not be visible", !pObj->IsVisible());

    // Save and reload
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    // Get ScDocShell
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Reload: Failed to access document shell", pFoundShell);
    pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and image
    ScDocument& rDoc2 = pDocSh->GetDocument();
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: Image no longer exists", pObj);

    // Expand the group
    pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(1);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_SHOW);
    CPPUNIT_ASSERT_MESSAGE("Expand: Image should be visible", pObj->IsVisible());

    // Assert image size is not changed
    tools::Rectangle aSnapRectReload = pObj->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Reload: Object geometry has changed.", aSnapRectOrig,
                                     aSnapRectReload, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testFitToCellSize()
{
    // The document has a cell anchored custom shape. Applying
    // FitToCellSize should resize and position the shape so,
    // that it fits into its anchor cell. That did not happened.
    OUString aFileURL;
    createFileURL("tdf119191_FitToCellSize.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get the shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);

    // Get the document controller
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    // Get the draw view of the document
    ScDrawView* pDrawView = pViewShell->GetViewData().GetScDrawView();
    CPPUNIT_ASSERT(pDrawView);

    // Select the shape
    pDrawView->MarkNextObj();
    CPPUNIT_ASSERT(pDrawView->AreObjectsMarked());

    // Fit selected shape into cell
    pViewShell->GetViewData().GetDispatcher().Execute(SID_FITCELLSIZE);

    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    const tools::Rectangle aCellRect = rDoc.GetMMRect(1, 1, 1, 1, 0);
    lcl_AssertRectEqualWithTolerance("Cell and SnapRect should be equal", aCellRect, rShapeRect, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testCustomShapeCellAnchoredRotatedShape()
{
    // The example doc contains a cell anchored custom shape that is rotated
    // and sheared. Error was, that the shape lost position and size on
    // loading.
    OUString aFileURL;
    createFileURL("tdf119191_transformedShape.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get the shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);

    // Check Position and Size
    rDoc.SetDrawPageSize(0); // trigger recalcpos
    tools::Rectangle aRect(2400, 751, 5772, 3694); // expected snap rect from values in file
    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    lcl_AssertRectEqualWithTolerance("Load: wrong pos and size", aRect, rShapeRect, 1);

    // Check anchor
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("expected object meta data", pData);

    const OUString sActual("start col " + OUString::number(pData->maStart.Col()) + " row "
                           + OUString::number(pData->maStart.Row()) + " end col "
                           + OUString::number(pData->maEnd.Col()) + " row "
                           + OUString::number(pData->maEnd.Row()));
    CPPUNIT_ASSERT_EQUAL(OUString("start col 1 row 1 end col 2 row 8"), sActual);

    pDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScShapeTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
