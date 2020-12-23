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
#include <doc.hxx>
#include <docary.hxx>

OUStringLiteral const DATA_DIRECTORY = u"/sw/qa/core/docnode/data/";

/// Covers sw/source/core/docnode/ fixes.
class Test : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(Test, testRedlineEndsBeforeToC)
{
    // Load a document where a delete redline ends right before a ToC, then redlines are hidden at a
    // layout level.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "redline-ends-before-toc.docx");
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    // Without the accompanying fix in place, this test would have resulted in an assertion failure
    // in InsertCnt_(), because the start of the section was hidden, but not its end.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
