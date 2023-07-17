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
#include <pagefrm.hxx>

namespace
{
/// Covers sw/source/core/text/itrform2.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/text/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloattableWrapEmptyParagraph)
{
    // Given a document with 2 pages, a floating table on both pages:
    createSwDoc("floattable-wrap-empty-para.docx");

    // When calculating the layout:
    calcLayout();

    // Then make sure that each page has exactly 1 floating table:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both tables were on page 1, leading to an overlap.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage->GetNext());
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPageObjs2 = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs2.size());
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableLegacyWrapEmptyParagraph)
{
    // Given a document with 2 pages, a floating table on both pages (from DOC, so the table is
    // shifted towards the left page edge slightly):
    createSwDoc("floattable-wrap-empty-para-legacy.docx");

    // When calculating the layout:
    calcLayout();

    // Then make sure that each page has exactly 1 floating table:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both tables were on page 1, leading to an overlap.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage->GetNext());
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPageObjs2 = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs2.size());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
