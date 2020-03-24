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

#include <wrtsh.hxx>
#include <fmtanchr.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <dcontact.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/core/frmedt/data/";

/// Covers sw/source/core/frmedt/ fixes.
class SwCoreFrmedtTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreFrmedtTest, testTextboxReanchor)
{
    // Load a document with a textframe and a textbox(shape+textframe).
    load(DATA_DIRECTORY, "textbox-reanchor.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pDrawPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pDrawShape = pDrawPage->GetObj(1);
    CPPUNIT_ASSERT_EQUAL(OUString("draw shape"), pDrawShape->GetName());

    // Select the shape of the textbox.
    Point aPoint;
    SwWrtShell* pShell = pDoc->GetDocShell()->GetWrtShell();
    pShell->SelectObj(aPoint, /*nFlag=*/0, pDrawShape);

    // Anchor the shape of the textbox into its own textframe.
    SdrObject* pTextFrameObj = pDrawPage->GetObj(2);
    SwFrameFormat* pTextFrameFormat = FindFrameFormat(pTextFrameObj);
    CPPUNIT_ASSERT_EQUAL(OUString("Frame2"), pTextFrameFormat->GetName());
    SwFrameFormat* pDrawShapeFormat = FindFrameFormat(pDrawShape);
    sal_uLong nOldAnchor = pDrawShapeFormat->GetAnchor().GetContentAnchor()->nNode.GetIndex();
    pShell->FindAnchorPos(pTextFrameObj->GetLastBoundRect().Center(), true);
    sal_uLong nNewAnchor = pDrawShapeFormat->GetAnchor().GetContentAnchor()->nNode.GetIndex();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 9
    // i.e. SwFEShell allowed to anchor the textframe of a textbox into itself.
    CPPUNIT_ASSERT_EQUAL(nOldAnchor, nNewAnchor);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
