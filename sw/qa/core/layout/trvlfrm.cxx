/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <wrtsh.hxx>
#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <node.hxx>

namespace
{
/// Covers sw/source/core/layout/trvlfrm.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyModelPositionForViewPointCorrection)
{
    // Given a 2 page floating table, 40% width, positioned on the left of the page:
    createSwDoc("floattable-model-position-for-view-point-correction.docx");

    // When clicking on the right side of the table:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    SwFrame* pBody = pPage->GetLower();
    const SwRect& rBodyRect = pBody->getFrameArea();
    // 1 line below the top center of the body frame.
    Point aDocPos(rBodyRect.Left() + rBodyRect.Width() / 2, rBodyRect.Top() + 220);
    bool bOnlyText = false;
    pWrtShell->CallSetCursor(&aDocPos, bOnlyText);

    // Then make sure the cursor gets inside the table, and doesn't go to the anchor on page 2:
    SwCursor& rCursor = pWrtShell->GetCurrentShellCursor();
    SwTableNode* pTableNode = rCursor.GetPointNode().FindTableNode();
    // Without the accompanying fix in place, this test would have failed, the cursor was in the
    // anchor text node, not inside the split fly.
    CPPUNIT_ASSERT(pTableNode);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
