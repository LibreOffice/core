/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <rootfrm.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>
#include <fmtfsize.hxx>
#include <IDocumentRedlineAccess.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/core/text/data/";

/// Covers sw/source/core/text/ fixes.
class SwCoreTextTest : public SwModelTestBase
{
public:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwCoreTextTest::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testFootnoteConnect)
{
    SwDoc* pDoc = createDoc("footnote-connect.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Jump to the start of the next page.
    pWrtShell->SttNxtPg();
    // Remove the page break.
    pWrtShell->DelLeft();
    // Split the multi-line text frame, containing an endnote.
    pWrtShell->DelLeft();
    // Join the split text frame.
    pWrtShell->DelLeft();
    // Turn the 3 page document into a 2 page one, so the endnote frame is moved.
    // Without the accompanying fix in place, this test would have crashed due to a use-after-free
    // in SwFootnoteFrame::GetRef().
    pWrtShell->DelLeft();
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testRedlineDelete)
{
    // Given a document with A4 paper size, some text, redlining on, but hidden:
    SwDoc* pDoc = createDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    {
        // Set page size to A4.
        size_t nCurIdx = pWrtShell->GetCurPageDesc();
        SwPageDesc aPageDesc(pWrtShell->GetPageDesc(nCurIdx));
        SwFrameFormat& rMaster = aPageDesc.GetMaster();
        SwFormatFrameSize aSize(ATT_FIX_SIZE);
        aSize.SetSize(Size(11906, 16838));
        rMaster.SetFormatAttr(aSize);
        pWrtShell->ChgPageDesc(nCurIdx, aPageDesc);
    }
    OUString aBefore("aaaaaaaaa aaaaaaaaaa aa aa aa ");
    OUString aDelete("delete eeeeeeeeeee ee eeeeeeeeeee ee eeeeee");
    pWrtShell->Insert(aBefore + " " + aDelete
                      + " zz zzz zzzzzzzzz zzz zzzz zzzz zzzzzzzzz zzzzzz zzz zzzzzzzzzzz zzz");
    // Enable redlining.
    pDocShell->SetChangeRecording(/*bActivate=*/true);
    // Hide redlining.
    pWrtShell->StartAllAction();
    pWrtShell->GetLayout()->SetHideRedlines(true);
    pWrtShell->EndAllAction();

    // When deleting content in the middle of the paragraph:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, /*nCount=*/aBefore.getLength(),
                     /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, /*nCount=*/aDelete.getLength(),
                     /*bBasicCall=*/false);
    // Without the accompanying fix in place, this test would have crashed:
    pWrtShell->Delete();

    // Then make sure that the redline is created:
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
