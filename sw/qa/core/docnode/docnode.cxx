/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <IDocumentRedlineAccess.hxx>
#include <redline.hxx>
#include <doc.hxx>
#include <docary.hxx>

/// Covers sw/source/core/docnode/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/docnode/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testRedlineEndsBeforeToC)
{
    // Load a document where a delete redline ends right before a ToC, then redlines are hidden at a
    // layout level.
    createSwDoc("redline-ends-before-toc.docx");
    SwDoc* pDoc = getSwDoc();
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    // Without the accompanying fix in place, this test would have resulted in an assertion failure
    // in InsertCnt_(), because the start of the section was hidden, but not its end.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());

    // The redline contained the newline, too
    CPPUNIT_ASSERT_EQUAL(u"<add-table-of-content>"_ustr, rTable[0]->GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150086)
{
    // Load a document where an insert redline ends right before a ToC
    createSwDoc("tdf150086.docx");
    SwDoc* pDoc = getSwDoc();
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(8), rTable.size());

    // This was "Conte" (stripped redline)
    CPPUNIT_ASSERT_EQUAL(u"Content\n"_ustr, rTable[6]->GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156267)
{
    createSwDoc("tdf156267.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
