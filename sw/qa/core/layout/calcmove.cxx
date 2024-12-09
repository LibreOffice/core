/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <test/xmldocptr.hxx>

#include <docsh.hxx>
#include <wrtsh.hxx>

namespace
{
/// Covers sw/source/core/layout/calcmove.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testIgnoreTopMargin)
{
    // Given a DOCX (>= Word 2013) file, with 2 pages:
    // When loading that document:
    createSwDoc("ignore-top-margin.docx");

    // Then make sure that the paragraph on the 2nd page has no top margin:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nParaTopMargin
        = getXPath(pXmlDoc, "/root/page[2]/body/txt/infos/prtBounds", "top").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2400
    // i.e. the top margin in the first para of a non-first page wasn't ignored, like in Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nParaTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testIgnoreTopMarginTable)
{
    // Given a DOCX (>= Word 2013) file, with 2 pages:
    // When loading that document:
    createSwDoc("ignore-top-margin-table.docx");

    // Then make sure that the paragraph on the 2nd page in B1 has a top margin:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nParaTopMargin
        = getXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell[2]/txt/infos/prtBounds", "top")
              .toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2000
    // - Actual  : 0
    // i.e. the top margin in B1's first paragraph was ignored, but not in Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2000), nParaTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testIgnoreTopMarginFly)
{
    // Given a document with compat flags like DOCX (>= Word 2013), 2 pages, multi-col fly frame on
    // page 2:
    createSwDoc("ignore-top-margin-fly.odt");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure that the top margin is not ignored inside shape text:
    sal_Int32 nParaTopMargin = getXPath(pXmlDoc,
                                        "/root/page[2]/body/section/column[2]/body/txt/anchored/"
                                        "fly/column/body/txt/infos/prtBounds",
                                        "top")
                                   .toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4000
    // - Actual  : 0
    // i.e. the top margin was ignored inside shape text for Word compat, while multi-col shape text
    // is a Writer feature.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4000), nParaTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testIgnoreTopMarginPageStyleChange)
{
    // Given a DOCX (>= Word 2013), section break (next page) between pages 2 and 3:
    createSwDoc("ignore-top-margin-page-style-change.docx");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure that the top margin is not ignored on page 3:
    sal_Int32 nParaTopMargin
        = getXPath(pXmlDoc, "/root/page[3]/body/txt/infos/prtBounds", "top").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2000
    // - Actual  : 0
    // i.e. the top margin was ignored, which is incorrect.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2000), nParaTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testHideWhitespaceGrowingLastPage)
{
    // Given a document with a full first page, then hiding whitespace:
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    while (getPages() == 1)
    {
        pWrtShell->SplitNode();
    }
    pWrtShell->DelLeft();
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // When adding a new paragraph at doc end:
    pWrtShell->SplitNode();

    // Then make sure a new page is created:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the page was growing instead of creating a new page when it already had a max size.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
