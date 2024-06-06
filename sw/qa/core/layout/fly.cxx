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
#include <anchoredobject.hxx>
#include <flyfrms.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>

namespace
{
/// Covers sw/source/core/layout/fly.cxx fixes, i.e. mostly SwFlyFrame.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyNegativeHeight)
{
    // Given a document with complex enough content that a split fly frame temporarily moves below
    // the bottom of the body frame on a page:
    // When laying out the document, SwEditShell::CalcLayout() never returned:
    createSwDoc("floattable-negative-height.docx");

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Reformat();

    // Make sure that all the pages have the expected content:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    for (SwFrame* pFrame = pLayout->Lower(); pFrame; pFrame = pFrame->GetNext())
    {
        auto pPage = pFrame->DynCastPageFrame();
        if (!pPage->GetPrev())
        {
            // First page: start of the split fly chain:
            CPPUNIT_ASSERT(pPage->GetSortedObjs());
            SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
            auto pFly = rPageObjs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
            CPPUNIT_ASSERT(pFly);
            CPPUNIT_ASSERT(!pFly->GetPrecede());
            CPPUNIT_ASSERT(pFly->GetFollow());
        }
        else if (pPage->GetPrev() && pPage->GetNext() && pPage->GetNext()->GetNext())
        {
            // Middle pages: have a previous and a next fly:
            CPPUNIT_ASSERT(pPage->GetSortedObjs());
            SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
            auto pFly = rPageObjs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
            CPPUNIT_ASSERT(pFly);
            CPPUNIT_ASSERT(pFly->GetPrecede());
            CPPUNIT_ASSERT(pFly->GetFollow());
        }
        else if (pPage->GetPrev() && pPage->GetNext() && !pPage->GetNext()->GetNext())
        {
            // Page last but one: end of the fly chain:
            CPPUNIT_ASSERT(pPage->GetSortedObjs());
            SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
            auto pFly = rPageObjs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
            CPPUNIT_ASSERT(pFly);
            CPPUNIT_ASSERT(pFly->GetPrecede());
            CPPUNIT_ASSERT(!pFly->GetFollow());
        }
        else if (pPage->GetPrev() && !pPage->GetNext())
        {
            // Last page: no flys.
            CPPUNIT_ASSERT(!pPage->GetSortedObjs());
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testFlyRelWithRounding)
{
    // Given a document where page width is 21.001cm (11906 twips), and the image width is 48% of
    // the page width:
    createSwDoc("fly-rel-width-rounding.odt");

    // When laying out that document:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();

    // Then make sure that we calculate the width of the fly correctly:
    auto pPage = pLayout->GetLower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    auto pFly = rPageObjs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pFly);
    tools::Long nFlyWidth = pFly->getFrameArea().Width();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5715
    // - Actual  : 5714
    // i.e. 5714.88 was truncated, not rounded.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(5715), nFlyWidth);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
