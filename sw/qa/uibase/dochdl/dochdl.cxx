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
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>

#include <docsh.hxx>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <fmtanchr.hxx>

/// Covers sw/source/uibase/dochdl/ fixes.
class SwUibaseDochdlTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testSelectPasteFormat)
{
    // Create a new document and cut a character.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert2("x");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
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

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testComplexSelection)
{
    // Given a document where a text node has hints, but no as-char images.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert2("abc");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SfxItemSet aSet(pWrtShell->GetView().GetPool(),
                    svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END - 1>);
    // Bold, italic, underline.
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT));
    aSet.Put(SvxPostureItem(ITALIC_NORMAL, RES_CHRATR_POSTURE));
    aSet.Put(SvxUnderlineItem(LINESTYLE_SINGLE, RES_CHRATR_UNDERLINE));
    pWrtShell->SetAttrSet(aSet);
    uno::Reference<datatransfer::XTransferable2> xTransfer = new SwTransferable(*pWrtShell);

    // When checking if the selection is complex, then there should be no crash.
    // Without the accompanying fix in place, this test would have crashed, because we read past the
    // end of the hints array.
    CPPUNIT_ASSERT(!xTransfer->isComplex());
}

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testComplexSelectionAtChar)
{
    // Given a document with an at-char anchored image:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_CHAR);
    aFrameSet.Put(aAnchor);
    Graphic aGrf;
    pWrtShell->SwFEShell::Insert(OUString(), OUString(), &aGrf, &aFrameSet);
    pWrtShell->UnSelectFrame();

    // When checking if the selection is simple or complex:
    pWrtShell->SelAll();
    uno::Reference<datatransfer::XTransferable2> xTransfer = new SwTransferable(*pWrtShell);
    bool bComplex = xTransfer->isComplex();

    // Then make sure it's complex:
    // Without the accompanying fix in place, this test would have failed, a selection containing an
    // image was considered simple.
    CPPUNIT_ASSERT(bComplex);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
