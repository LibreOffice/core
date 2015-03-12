/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <crsskip.hxx>
#include <drawdoc.hxx>
#include <wrtsh.hxx>

static const char* DATA_DIRECTORY = "/sw/qa/extras/tiledrendering/data/";

/// Testsuite for the SwXTextDocument methods implementing the vcl::ITiledRenderable interface.
class SwTiledRenderingTest : public SwModelTestBase
{

public:
    void testSetGraphicSelection();
    void testResetSelection();

    CPPUNIT_TEST_SUITE(SwTiledRenderingTest);
    CPPUNIT_TEST(testSetGraphicSelection);
    CPPUNIT_TEST(testResetSelection);
    CPPUNIT_TEST_SUITE_END();

private:
    SwXTextDocument* createDoc(const char* pName);
};

SwXTextDocument* SwTiledRenderingTest::createDoc(const char* pName)
{
    load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDocument);
    pTextDocument->initializeForTiledRendering();
    return pTextDocument;
}

void SwTiledRenderingTest::testSetGraphicSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    pWrtShell->SelectObj(Point(), 0, pObject);
    // Make sure the rectangle has 8 handles: at each corner and at the center of each edge.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(8), pObject->GetHdlCount());
    // Take the bottom center one.
    SdrHdl* pHdl = pObject->GetHdl(6);
    CPPUNIT_ASSERT_EQUAL(HDL_LOWER, pHdl->GetKind());
    Rectangle aShapeBefore = pObject->GetSnapRect();
    // Resize.
    pXTextDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_START, pHdl->GetPos().getX(), pHdl->GetPos().getY());
    pXTextDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_END, pHdl->GetPos().getX(), pHdl->GetPos().getY() + 1000);
    Rectangle aShapeAfter = pObject->GetSnapRect();
    // Check that a resize happened, but aspect ratio is not kept.
    CPPUNIT_ASSERT_EQUAL(aShapeBefore.getWidth(), aShapeAfter.getWidth());
    CPPUNIT_ASSERT_EQUAL(aShapeBefore.getHeight() + 1000, aShapeAfter.getHeight());
}

void SwTiledRenderingTest::testResetSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Select one character.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    // We have a text selection.
    CPPUNIT_ASSERT(pShellCrsr->HasMark());

    pXTextDocument->resetSelection();
    // We no longer have a text selection.
    CPPUNIT_ASSERT(!pShellCrsr->HasMark());

    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    Point aPoint = pObject->GetSnapRect().Center();
    // Select the shape.
    pWrtShell->EnterSelFrmMode(&aPoint);
    // We have a graphic selection.
    CPPUNIT_ASSERT(pWrtShell->IsSelFrmMode());

    pXTextDocument->resetSelection();
    // We no longer have a graphic selection.
    CPPUNIT_ASSERT(!pWrtShell->IsSelFrmMode());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
