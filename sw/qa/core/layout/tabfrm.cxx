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
#include <docsh.hxx>
#include <wrtsh.hxx>

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

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyHeader)
{
    // Given a document with 8 pages: a first page ending in a manual page break, then a multi-page
    // floating table on pages 2..8:
    createSwDoc("floattable-header.docx");
    CPPUNIT_ASSERT_EQUAL(8, getPages());

    // When creating a new paragraph at doc start:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SplitNode();
    // Without the accompanying fix in place, this test would have crashed here.
    pWrtShell->CalcLayout();

    // Then make sure we get one more page, since the first page is now 2 pages:
    CPPUNIT_ASSERT_EQUAL(9, getPages());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
