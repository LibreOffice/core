/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/dispatch.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <test/calc_unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <unonames.hxx>

#include <docsh.hxx>
#include <drwlayer.hxx>
#include <scitems.hxx>
#include <svx/svdocirc.hxx>
#include <tabvwsh.hxx>

#include <sc.hrc>

using namespace css;

namespace sc_apitest
{
class ScAnchorTest : public CalcUnoApiTest
{
public:
    ScAnchorTest();

    virtual void tearDown() override;

    void testUndoAnchor();
    void testTdf76183();
    void testODFAnchorTypes();
    void testCopyColumnWithImages();
    void testCutWithImages();

    CPPUNIT_TEST_SUITE(ScAnchorTest);
    CPPUNIT_TEST(testUndoAnchor);
    CPPUNIT_TEST(testTdf76183);
    CPPUNIT_TEST(testODFAnchorTypes);
    CPPUNIT_TEST(testCopyColumnWithImages);
    CPPUNIT_TEST(testCutWithImages);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScAnchorTest::ScAnchorTest()
    : CalcUnoApiTest("sc/qa/unit/data/ods")
{
}

void ScAnchorTest::testUndoAnchor()
{
    OUString aFileURL;
    createFileURL("document_with_linked_graphic.ods", aFileURL);
    // open the document with graphic included
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Check whether graphic imported well
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObject);
    CPPUNIT_ASSERT(pObject->IsLinkedGraphic());

    const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetGraphic().GetSizeBytes());

    // Get the document controller
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    // Get the draw view of the document
    ScDrawView* pDrawView = pViewShell->GetViewData().GetScDrawView();
    CPPUNIT_ASSERT(pDrawView);

    // Select graphic object
    pDrawView->MarkNextObj();
    CPPUNIT_ASSERT(pDrawView->AreObjectsMarked());

    // Set Cell Anchor
    ScDrawLayer::SetCellAnchoredFromPosition(*pObject, rDoc, 0, false);
    // Check state
    ScAnchorType oldType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_CELL, oldType);

    // Change all selected objects to page anchor
    pViewShell->GetViewData().GetDispatcher().Execute(SID_ANCHOR_PAGE);
    // Check state
    ScAnchorType newType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_PAGE, newType);

    // Undo and check its result.
    SfxUndoManager* pUndoMgr = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    // Check anchor type
    CPPUNIT_ASSERT_EQUAL(oldType, ScDrawLayer::GetAnchorType(*pObject));
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetGraphic().GetSizeBytes());

    pUndoMgr->Redo();

    // Check anchor type
    CPPUNIT_ASSERT_EQUAL(newType, ScDrawLayer::GetAnchorType(*pObject));
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetGraphic().GetSizeBytes());

    ScDrawLayer::SetPageAnchored(*pObject);
    // Check state
    oldType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_PAGE, oldType);

    // Change all selected objects to cell anchor
    pViewShell->GetViewData().GetDispatcher().Execute(SID_ANCHOR_CELL);
    // Check state
    newType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_CELL, newType);

    pUndoMgr->Undo();

    // Check anchor type
    CPPUNIT_ASSERT_EQUAL(oldType, ScDrawLayer::GetAnchorType(*pObject));
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetGraphic().GetSizeBytes());

    pUndoMgr->Redo();

    // Check anchor type
    CPPUNIT_ASSERT_EQUAL(newType, ScDrawLayer::GetAnchorType(*pObject));
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetGraphic().GetSizeBytes());

    xComponent->dispose();
}

void ScAnchorTest::testTdf76183()
{
    uno::Reference<lang::XComponent> xComponent = loadFromDesktop("private:factory/scalc");
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);

    // Add a circle somewhere below first row.
    const tools::Rectangle aOrigRect = tools::Rectangle(1000, 1000, 1200, 1200);
    SdrCircObj* pObj = new SdrCircObj(*pDrawLayer, OBJ_CIRC, aOrigRect);
    pPage->InsertObject(pObj);
    // Anchor to cell
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, rDoc, 0, false);
    const tools::Rectangle& rNewRect = pObj->GetLogicRect();

    // Set word wrap to true
    rDoc.ApplyAttr(0, 0, 0, SfxBoolItem(ATTR_LINEBREAK, true));
    // Add multi-line text to cell to initiate optimal height change
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xText(xSheet->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xText->setString("first\nsecond\nthird");

    // The resize of first row must have moved the object down after its anchor cell
    CPPUNIT_ASSERT(aOrigRect.Top() < rNewRect.Top());

    pDocSh->DoClose();
}

void ScAnchorTest::testODFAnchorTypes()
{
    OUString aFileURL;
    createFileURL("3AnchorTypes.ods", aFileURL);
    // open the document with graphic included
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Check whether graphic imported well
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    // Check 1st object: Page anchored
    SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObject);
    ScAnchorType anchorType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_PAGE, anchorType);

    // Check 2nd object: Cell anchored, resize with cell
    pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(1));
    CPPUNIT_ASSERT(pObject);
    anchorType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_CELL_RESIZE, anchorType);

    // Check 3rd object: Cell anchored
    pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
    CPPUNIT_ASSERT(pObject);
    anchorType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_CELL, anchorType);

    pDocSh->DoClose();
}

/// Test that copying a column with an image anchored to it also copies the image
void ScAnchorTest::testCopyColumnWithImages()
{
    OUString aFileURL;
    createFileURL("3AnchorTypes.ods", aFileURL);
    // open the document with graphic included
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    ScDocument* pDoc = &(pDocSh->GetDocument());
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    // Get the document controller
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    ScDocument aClipDoc(SCDOCMODE_CLIP);

    // Copy whole column
    {
        // 1. Copy source range
        ScRange aSrcRange;
        aSrcRange.Parse("A1:A11", pDoc, pDoc->GetAddressConvention());
        pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);
        pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, true, false);

        // 2. Paste to target range
        ScRange aDstRange;
        aDstRange.Parse("D1:D11", pDoc, pDoc->GetAddressConvention());
        pViewShell->GetViewData().GetMarkData().SetMarkArea(aDstRange);
        pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);

        // 3. Make sure the images have been copied too
        std::map<SCROW, std::vector<SdrObject*>> aRowObjects
            = pDrawLayer->GetObjectsAnchoredToRange(0, 3, 0, 11);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be an image anchored to D3", 1,
                                     static_cast<int>(aRowObjects[2].size()));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be an image anchored to D11", 1,
                                     static_cast<int>(aRowObjects[10].size()));
    }

    // Copy individual cells
    {
        // 1. Copy source cells
        ScRange aSrcRange;
        aSrcRange.Parse("A3:B3", pDoc, pDoc->GetAddressConvention());
        pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);
        pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, true, false);

        // 2. Paste to target cells
        ScRange aDstRange;
        aDstRange.Parse("G3:H3", pDoc, pDoc->GetAddressConvention());
        pViewShell->GetViewData().GetMarkData().SetMarkArea(aDstRange);
        pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);

        // 3. Make sure the image has been copied too
        std::map<SCROW, std::vector<SdrObject*>> aRowObjects
            = pDrawLayer->GetObjectsAnchoredToRange(0, 6, 2, 2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be an image anchored to G3", 1,
                                     static_cast<int>(aRowObjects[2].size()));
    }

    pDocSh->DoClose();
}

void ScAnchorTest::testCutWithImages()
{
    OUString aFileURL;
    createFileURL("3AnchorTypes.ods", aFileURL);
    // open the document with graphic included
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    ScDocument* pDoc = &(pDocSh->GetDocument());
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    // Get the document controller
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    // Cut whole column
    {
        // Cut source range
        ScRange aSrcRange;
        aSrcRange.Parse("A1:A11", pDoc, pDoc->GetAddressConvention());
        pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);
        pViewShell->GetViewData().GetView()->CutToClip();

        std::map<SCROW, std::vector<SdrObject*>> aRowObjects
            = pDrawLayer->GetObjectsAnchoredToRange(0, 0, 0, 11);

        // Images should have been removed from the cells
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no image anchored to A3", 0,
                                     static_cast<int>(aRowObjects[2].size()));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no image anchored to A11", 0,
                                     static_cast<int>(aRowObjects[10].size()));
    }

    // Cut individual cells
    {
        // Cut source cells
        ScRange aSrcRange;
        aSrcRange.Parse("A3:B3", pDoc, pDoc->GetAddressConvention());
        pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);
        pViewShell->GetViewData().GetView()->CutToClip();

        // Image should have been removed from the cell
        std::map<SCROW, std::vector<SdrObject*>> aRowObjects
            = pDrawLayer->GetObjectsAnchoredToRange(0, 0, 2, 2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no image anchored to A3", 0,
                                     static_cast<int>(aRowObjects[2].size()));
    }

    pDocSh->DoClose();
}

void ScAnchorTest::tearDown()
{
    if (mxComponent.is())
    {
        closeDocument(mxComponent);
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnchorTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
