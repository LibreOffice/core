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
#include <comphelper/string.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <vcl/svapp.hxx>
#include <crsskip.hxx>
#include <drawdoc.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>

static const char* DATA_DIRECTORY = "/sw/qa/extras/tiledrendering/data/";

/// Testsuite for the SwXTextDocument methods implementing the vcl::ITiledRenderable interface.
class SwTiledRenderingTest : public SwModelTestBase
{
public:
    void testRegisterCallback();
    void testPostKeyEvent();
    void testPostMouseEvent();
    void testSetTextSelection();
    void testSetGraphicSelection();
    void testResetSelection();

    CPPUNIT_TEST_SUITE(SwTiledRenderingTest);
    CPPUNIT_TEST(testRegisterCallback);
    CPPUNIT_TEST(testPostKeyEvent);
    CPPUNIT_TEST(testPostMouseEvent);
    CPPUNIT_TEST(testSetTextSelection);
    CPPUNIT_TEST(testSetGraphicSelection);
    CPPUNIT_TEST(testResetSelection);
    CPPUNIT_TEST_SUITE_END();

private:
    SwXTextDocument* createDoc(const char* pName);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    Rectangle m_aInvalidation;
};

SwXTextDocument* SwTiledRenderingTest::createDoc(const char* pName)
{
    load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDocument);
    pTextDocument->initializeForTiledRendering();
    return pTextDocument;
}

void SwTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<SwTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

void SwTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
    {
        if (m_aInvalidation.IsEmpty())
        {
            uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aInvalidation.setX(aSeq[0].toInt32());
            m_aInvalidation.setY(aSeq[1].toInt32());
            m_aInvalidation.setWidth(aSeq[2].toInt32());
            m_aInvalidation.setHeight(aSeq[3].toInt32());
        }
    }
    break;
    }
}

void SwTiledRenderingTest::testRegisterCallback()
{
#ifdef MACOSX
    // For some reason this particular test requires window system access on OS X.

    // Without window system access, we do get a number of "<<<WARNING>>>
    // AquaSalGraphics::CheckContext() FAILED!!!!" [sic] and " <Warning>: CGSConnectionByID: 0 is
    // not a valid connection ID" warnings while running the other tests, too, but they still
    // succeed.

    if (!vcl::IsWindowSystemAvailable())
        return;
#endif

    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    pXTextDocument->registerCallback(&SwTiledRenderingTest::callback, this);
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Insert a character at the beginning of the document.
    pWrtShell->Insert("x");

    // Check that the top left 256x256px tile would be invalidated.
    CPPUNIT_ASSERT(!m_aInvalidation.IsEmpty());
#if !defined(WNT) && !defined(MACOSX)
    Rectangle aTopLeft(0, 0, 256*15, 256*15); // 1 px = 15 twips, assuming 96 DPI.
    // FIXME - fails on Windows since about cbd48230bb3a90c4c485fa33123c6653234e02e9
    // [plus minus few commits maybe]
    // Also on OS X. But is tiled rendering even supposed to work on Windows and OS X?
    CPPUNIT_ASSERT(m_aInvalidation.IsOver(aTopLeft));
#endif
}

void SwTiledRenderingTest::testPostKeyEvent()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    // Did we manage to go after the first character?
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pShellCrsr->GetPoint()->nContent.GetIndex());

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    // Did we manage to insert the character after the first one?
    CPPUNIT_ASSERT_EQUAL(OUString("Axaa bbb."), pShellCrsr->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwTiledRenderingTest::testPostMouseEvent()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    // Did we manage to go after the first character?
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pShellCrsr->GetPoint()->nContent.GetIndex());

    Point aStart = pShellCrsr->GetSttPos();
    aStart.setX(aStart.getX() - 1000);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aStart.getX(), aStart.getY(), 1);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aStart.getX(), aStart.getY(), 1);
    // The new cursor position must be before the first word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), pShellCrsr->GetPoint()->nContent.GetIndex());
}

void SwTiledRenderingTest::testSetTextSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Move the cursor into the second word.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 5, /*bBasicCall=*/false);
    // Create a selection by on the word.
    pWrtShell->SelWrd();
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), pShellCrsr->GetText());

    // Now use setTextSelection() to move the start of the selection 1000 twips left.
    Point aStart = pShellCrsr->GetSttPos();
    aStart.setX(aStart.getX() - 1000);
    pXTextDocument->setTextSelection(LOK_SETTEXTSELECTION_START, aStart.getX(), aStart.getY());
    // The new selection must include the first word, too -- but not the ending dot.
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb"), pShellCrsr->GetText());

    // Next: test that LOK_SETTEXTSELECTION_RESET + LOK_SETTEXTSELECTION_END can be used to create a selection.
    pXTextDocument->setTextSelection(LOK_SETTEXTSELECTION_RESET, aStart.getX(), aStart.getY());
    pXTextDocument->setTextSelection(LOK_SETTEXTSELECTION_END, aStart.getX() + 1000, aStart.getY());
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa b"), pShellCrsr->GetText());
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
#if !defined(MACOSX) // FIXME
    CPPUNIT_ASSERT_EQUAL(aShapeBefore.getHeight() + 1000, aShapeAfter.getHeight());
#endif
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
