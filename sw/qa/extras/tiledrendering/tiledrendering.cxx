/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
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
    void testResetSelection();

    CPPUNIT_TEST_SUITE(SwTiledRenderingTest);
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
    return pTextDocument;
}

void SwTiledRenderingTest::testResetSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("reset-selection.fodt");
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
