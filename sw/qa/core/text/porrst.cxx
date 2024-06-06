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
#include <doc.hxx>
#include <frame.hxx>
#include <layfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <sortedobjs.hxx>

namespace
{
/// Covers sw/source/core/text/porrst.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloattableLeftoverParaPortion)
{
    // Given a document with a multi-page floating table, the anchor of the table has some text:
    createSwDoc("floattable-leftover-para-portion.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure all anchor text goes to the second page:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    SwFrame* pBody = pPage->FindBodyCont();
    SwFrame* pPara1 = pBody->GetLower();
    auto pPara2 = pPara1->GetNext()->DynCastTextFrame();
    CPPUNIT_ASSERT(pPara2);
    // Without the accompanying fix in place, this test would have failed, the first page's anchor
    // also had some (duplicated) anchor text.
    CPPUNIT_ASSERT(!pPara2->GetPara());
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableAnchorHeight)
{
#if !defined(MACOSX) // FIXME fails on macOS
    // Given 3 tables, innermost table on pages 2-3-4:
    createSwDoc("floattable-anchor-height.docx");

    // When laying out the document:
    calcLayout();

    // Then make sure the flys are on the expected pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(!pPage1->GetSortedObjs());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    SwSortedObjs* pPage2Objs = pPage2->GetSortedObjs();
    CPPUNIT_ASSERT(pPage2Objs);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage2Objs->size());
    auto pPage3 = pPage2->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3);
    SwSortedObjs* pPage3Objs = pPage3->GetSortedObjs();
    CPPUNIT_ASSERT(pPage3Objs);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. page 3 also had the fly frame of page 4 as well.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage3Objs->size());
    auto pPage4 = pPage3->GetNext()->DynCastPageFrame();
    SwSortedObjs* pPage4Objs = pPage4->GetSortedObjs();
    CPPUNIT_ASSERT(pPage4Objs);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage4Objs->size());
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
