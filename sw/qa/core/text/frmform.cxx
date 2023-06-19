/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>

#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <pagefrm.hxx>

namespace
{
/// Covers sw/source/core/text/frmform.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/text/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloattableNegativeVertOffset)
{
    // Given a document with 2 paragraphs, floating table is between the two (so anchored to the
    // 2nd) and with a negative vertical offset:
    createSwDoc("floattable-negative-vert-offset.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the negative vertical offset shifts both paragraphs down:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    SwAnchoredObject* pPageObj = rPageObjs[0];
    const SwRect& rFlyRect = pPageObj->GetObjRectWithSpaces();
    SwFrame* pBody = pPage->FindBodyCont();
    SwFrame* pPara1 = pBody->GetLower();
    SwFrame* pPara2 = pPara1->GetNext();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 993
    // - Actual  : 2709
    // i.e. the expectation that the fly doesn't overlap with the 2nd paragraph was not true.
    // Instead we got a layout loop, aborted by the loop control, and the fly overlapped with the
    // 2nd paragraph.
    CPPUNIT_ASSERT_LESS(pPara2->getFrameArea().Top(), rFlyRect.Bottom());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
