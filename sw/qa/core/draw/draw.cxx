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

#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <wrtsh.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/core/draw/data/";

/// Covers sw/source/core/draw/ fixes.
class SwCoreDrawTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreDrawTest, testTextboxDeleteAsChar)
{
    // Load a document with an as-char shape in it that has a textbox and an image in it.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "as-char-textbox.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    sal_Int32 nActual = pPage->GetObjCount();
    // 3 objects on the draw page: a shape + fly frame pair and a Writer image.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), nActual);

    // Select the shape of the textbox and delete it.
    SdrObject* pObject = pPage->GetObj(0);
    pWrtShell->SelectObj(Point(), 0, pObject);
    pWrtShell->DelSelectedObj();
    nActual = pPage->GetObjCount();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2
    // i.e. the fly frame of the shape and the inner Writer image was not deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nActual);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
