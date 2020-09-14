/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/view/XLineCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>

#include <comphelper/propertysequence.hxx>
#include <svl/srchitem.hxx>
#include <vcl/scheduler.hxx>

#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/core/crsr/data/";

/// Covers sw/source/core/crsr/ fixes.
class SwCoreCrsrTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testSelAllStartsWithTable)
{
    load(DATA_DIRECTORY, "sel-all-starts-with-table.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetTableFrameFormatCount(/*bUsed=*/true));

    pWrtShell->SelAll();
    pWrtShell->SelAll();
    Scheduler::ProcessEventsToIdle();
    pWrtShell->DelLeft();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the table selection was lost and the table was not deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDoc->GetTableFrameFormatCount(/*bUsed=*/true));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
