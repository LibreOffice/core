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
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <tabfrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <flyfrm.hxx>
#include <flyfrms.hxx>

namespace
{
/// Covers sw/source/core/layout/tabfrm.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTablePrintAreaLeft)
{
    // Given a document with a header containing an image, and also with an overlapping table:
    createSwDoc("table-print-area-left.docx");

    // When laying out that document & parsing the left margin of the table:
    SwTwips nTablePrintLeft = parseDump("//tab/infos/prtBounds", "left").toInt32();

    // Then make sure it has ~no left margin:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 10646
    // i.e. the table was shifted outside the page, was invisible.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(5), nTablePrintLeft);
}

CPPUNIT_TEST_FIXTURE(Test, testTableMissingJoin)
{
    // Given a document with a table on page 2:
    // When laying out that document:
    createSwDoc("table-missing-join.docx");

    // Then make sure that the table fits page 2:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    SwFrame* pBody = pPage2->FindBodyCont();
    auto pTab = pBody->GetLower()->DynCastTabFrame();
    // Without the accompanying fix in place, this test would have failed, the table continued on
    // page 3.
    CPPUNIT_ASSERT(!pTab->HasFollow());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyInInlineTable)
{
    // Outer inline table on pages 1 -> 2 -> 3, inner floating table on pages 2 -> 3:
    // When laying out that document:
    createSwDoc("floattable-in-inlinetable.docx");

    // Then make sure that the outer table is not missing on page 3:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    {
        SwFrame* pBody = pPage1->FindBodyCont();
        auto pTab = pBody->GetLower()->DynCastTabFrame();
        CPPUNIT_ASSERT(!pTab->GetPrecede());
        CPPUNIT_ASSERT(pTab->GetFollow());
    }
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    {
        SwFrame* pBody = pPage2->FindBodyCont();
        auto pTab = pBody->GetLower()->DynCastTabFrame();
        CPPUNIT_ASSERT(pTab->GetPrecede());
        // Without the accompanying fix in place, this test would have failed, the outer table was
        // missing on page 3.
        CPPUNIT_ASSERT(pTab->GetFollow());
    }
    auto pPage3 = pPage2->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3);
    {
        SwFrame* pBody = pPage3->FindBodyCont();
        auto pTab = pBody->GetLower()->DynCastTabFrame();
        CPPUNIT_ASSERT(pTab->GetPrecede());
        CPPUNIT_ASSERT(!pTab->GetFollow());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyNestedRowSpan)
{
    // Given a document with nested floating tables and a row with rowspan cells at page boundary:
    // When loading that document:
    // Without the accompanying fix in place, this test would have resulted in a layout loop.
    createSwDoc("floattable-nested-rowspan.docx");

    // Then make sure the resulting page count matches Word:
    CPPUNIT_ASSERT_EQUAL(6, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyTableJoin)
{
    // Given a document with a multi-page floating table:
    // When loading this document:
    createSwDoc("floattable-table-join.docx");

    // Then make sure this document doesn't crash the layout and has a floating table split on 4
    // pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    {
        SwSortedObjs& rPageObjs = *pPage1->GetSortedObjs();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
        auto pFly = rPageObjs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
        CPPUNIT_ASSERT(pFly);
        // Start of the chain.
        CPPUNIT_ASSERT(!pFly->GetPrecede());
        CPPUNIT_ASSERT(pFly->HasFollow());
    }
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    {
        SwSortedObjs& rPageObjs = *pPage2->GetSortedObjs();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
        auto pFly = rPageObjs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
        CPPUNIT_ASSERT(pFly);
        CPPUNIT_ASSERT(pFly->GetPrecede());
        CPPUNIT_ASSERT(pFly->HasFollow());
    }
    auto pPage3 = pPage2->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3);
    CPPUNIT_ASSERT(pPage3->GetSortedObjs());
    {
        SwSortedObjs& rPageObjs = *pPage3->GetSortedObjs();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
        auto pFly = rPageObjs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
        CPPUNIT_ASSERT(pFly);
        CPPUNIT_ASSERT(pFly->GetPrecede());
        CPPUNIT_ASSERT(pFly->HasFollow());
    }
    auto pPage4 = pPage3->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage4);
    CPPUNIT_ASSERT(pPage4->GetSortedObjs());
    SwSortedObjs& rPageObjs = *pPage4->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    auto pFly = rPageObjs[0]->DynCastFlyFrame()->DynCastFlyAtContentFrame();
    CPPUNIT_ASSERT(pFly);
    // End of the chain.
    CPPUNIT_ASSERT(pFly->GetPrecede());
    CPPUNIT_ASSERT(!pFly->HasFollow());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
