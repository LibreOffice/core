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
#include <txtfrm.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>

namespace
{
/// Covers sw/source/core/text/frmform.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
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

// FIXME: because breaking the lines at the right place, test document became obsolete
#if 0
CPPUNIT_TEST_FIXTURE(Test, testFloattableAvoidManipOfst)
{
    // Given a document with a 6-page floating table and some anchor text:
    createSwDoc("floattable-avoid-manip-ofst.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure all anchor text is on the last page:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwPageFrame* pLastPage = pLayout->GetLastPage();
    SwLayoutFrame* pBodyFrame = pLastPage->FindBodyCont();
    SwTextFrame* pAnchor = pBodyFrame->GetLower()->DynCastTextFrame();
    // If this is not 0, that means some of the anchor text is shifted to a previous page, while
    // anchors of non-last split fly frames should contain no text.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), pAnchor->GetOffset().get());
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testFloattableAvoidLastManipOfst)
{
    // Given a document with a 5-page floating table and some anchor text:
    createSwDoc("floattable-avoid-last-manip-ofst.docx");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert2(u"dt"_ustr);

    // When expanding dummy text on the last page:
    dispatchCommand(mxComponent, u".uno:ExpandGlossary"_ustr, {});

    // Then make sure the expanded text starts on page 5:
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    auto pPage3 = dynamic_cast<SwPageFrame*>(pPage2->GetNext());
    CPPUNIT_ASSERT(pPage3);
    auto pPage4 = dynamic_cast<SwPageFrame*>(pPage3->GetNext());
    CPPUNIT_ASSERT(pPage4);
    auto pPage5 = dynamic_cast<SwPageFrame*>(pPage4->GetNext());
    CPPUNIT_ASSERT(pPage5);
    SwContentFrame* pAnchor = pPage5->FindFirstBodyContent();
    SwTextFrame* pAnchorText = pAnchor->DynCastTextFrame();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1123
    // i.e. the expand result went to page 4 and page 5 (page 5's content had no zero offset),
    // instead of starting on page 5 (and creating a 6th page).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         static_cast<sal_Int32>(pAnchorText->GetOffset()));
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableBadFlyPos)
{
    // Given a document with an inner floating table on page 2 -> 4:
    // When laying out that document:
    createSwDoc("floattable-bad-fly-pos.docx");

    // Then make sure that pages 2 -> 4 get the 3 fly frames:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(!pPage1->GetSortedObjs());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage2->GetSortedObjs()->size());
    auto pPage3 = pPage2->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3);
    CPPUNIT_ASSERT(pPage3->GetSortedObjs());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the fly on page 4 was still on page 3.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage3->GetSortedObjs()->size());
    auto pPage4 = pPage3->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage4);
    CPPUNIT_ASSERT(pPage4->GetSortedObjs());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage4->GetSortedObjs()->size());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
