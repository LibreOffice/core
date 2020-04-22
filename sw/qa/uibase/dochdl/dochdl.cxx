/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/GraphicObject.hxx>
#include <svx/svdpage.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <vcl/transfer.hxx>

#include <IDocumentContentOperations.hxx>
#include <cmdid.h>
#include <fmtanchr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <swdtflvr.hxx>

/// Covers sw/source/uibase/dochdl/ fixes.
class SwUibaseDochdlTest : public SwModelTestBase
{
public:
    SwDoc* createDoc();
};

SwDoc* SwUibaseDochdlTest::createDoc()
{
    loadURL("private:factory/swriter", nullptr);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testSelectPasteFormat)
{
    // Create a new document and cut a character.
    SwDoc* pDoc = createDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert2("x");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // Decide what format to use when doing a Writer->Writer paste and both RTF and ODF is an
    // available format.
    TransferableDataHelper aHelper(pTransfer.get());
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
