/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <vcl/transfer.hxx>

#include <docsh.hxx>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>

/// Covers sw/source/uibase/dochdl/ fixes.
class SwUibaseDochdlTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testSelectPasteFormat)
{
    // Create a new document and cut a character.
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert2("x");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // Decide what format to use when doing a Writer->Writer paste and both RTF and ODF is an
    // available format.
    TransferableDataHelper aHelper(pTransfer);
    sal_uInt8 nAction = EXCHG_OUT_ACTION_INSERT_STRING;
    SotClipboardFormatId nFormat = SotClipboardFormatId::RICHTEXT;
    SwTransferable::SelectPasteFormat(aHelper, nAction, nFormat);

    CPPUNIT_ASSERT_EQUAL(EXCHG_OUT_ACTION_INSERT_OLE, nAction);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 85 (EMBED_SOURCE)
    // - Actual  : 145 (RICHTEXT)
    // i.e. RTF was selected for Writer->Writer out of process copying, which is worse than ODF.
    CPPUNIT_ASSERT_EQUAL(SotClipboardFormatId::EMBED_SOURCE, nFormat);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
