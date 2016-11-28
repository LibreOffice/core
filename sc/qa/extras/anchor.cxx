/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/dispatch.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <unonames.hxx>

#include "tabvwsh.hxx"
#include "docsh.hxx"

#include "sc.hrc"

using namespace css;

namespace sc_apitest {

class ScAnchorTest : public CalcUnoApiTest
{
public:
    ScAnchorTest();

    virtual void tearDown() override;

    void testUndoAnchor();

    CPPUNIT_TEST_SUITE(ScAnchorTest);
    CPPUNIT_TEST(testUndoAnchor);
    CPPUNIT_TEST_SUITE_END();
private:

    uno::Reference< lang::XComponent > mxComponent;
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
    uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileURL);
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

    const SdrPage *pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObject);
    CPPUNIT_ASSERT(pObject->IsLinkedGraphic());

    const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
    CPPUNIT_ASSERT(!rGraphicObj.IsSwappedOut());
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetSizeBytes());

    // Get the document controller
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    // Get the draw view of the document
    ScDrawView* pDrawView = pViewShell->GetViewData().GetScDrawView();
    CPPUNIT_ASSERT(pDrawView);

    // Select graphic object
    pDrawView->MarkNextObj();
    CPPUNIT_ASSERT(pDrawView->AreObjectsMarked() );

    // Set Cell Anchor
    ScDrawLayer::SetCellAnchoredFromPosition(*pObject, rDoc, 0);
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
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetSizeBytes());

    pUndoMgr->Redo();

    // Check anchor type
    CPPUNIT_ASSERT_EQUAL(newType, ScDrawLayer::GetAnchorType(*pObject));
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetSizeBytes());

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
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetSizeBytes());

    pUndoMgr->Redo();

    // Check anchor type
    CPPUNIT_ASSERT_EQUAL(newType, ScDrawLayer::GetAnchorType(*pObject));
    CPPUNIT_ASSERT_EQUAL(int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(864900), rGraphicObj.GetSizeBytes());

    xComponent->dispose();
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
