/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <docsh.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>

namespace
{
/// Covers sw/source/core/layout/anchoredobject.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTextBoxFlyPageCapture)
{
    // Given a document with a draw shape with complex content, positioned to be outside the page
    // frame, but capture to page frame is enabled:
    // When loading that document:
    createSwDoc("textbox-fly-page-capture.odt");

    // Then make sure the right edge of the inner fly frame is the same as the right edge of the
    // draw frame:
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = pLayout->GetLower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPageObjs.size());
    SwAnchoredObject* pDraw = rPageObjs[0];
    SwTwips nDrawRight = pDraw->GetObjRect().Right();
    SwAnchoredObject* pFly = rPageObjs[1];
    SwTwips nFlyRight = pFly->GetObjRect().Right();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5081
    // - Actual  : 4798
    // i.e. the position of the draw shape was corrected to be inside the page frame, but this
    // didn't happen for the inner fly frame.
    CPPUNIT_ASSERT_EQUAL(nDrawRight, nFlyRight);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
