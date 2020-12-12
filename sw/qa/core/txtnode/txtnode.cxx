/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <vcl/gdimtf.hxx>

#include <fmtanchr.hxx>
#include <frameformats.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

OUStringLiteral const DATA_DIRECTORY = u"/sw/qa/core/txtnode/data/";

/// Covers sw/source/core/txtnode/ fixes.
class SwCoreTxtnodeTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testBtlrCellChinese)
{
    // Load a document with a table cell, with btlr cell direction.  The cell has text which is
    // classified as vertical, i.e. the glyph has the same direction in both the lrtb ("Latin") and
    // tbrl ("Chinese") directions. Make sure that Chinese text is handled the same way in the btlr
    // case as it's handled in the Latin case.
    load(DATA_DIRECTORY, "btlr-cell-chinese.doc");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPath(pXmlDoc, "//font[1]", "orientation", "900");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: false
    // - Actual  : true
    // i.e. the glyph was rotated further, so it was upside down.
    assertXPath(pXmlDoc, "//font[1]", "vertical", "false");
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testTextBoxCopyAnchor)
{
    load(DATA_DIRECTORY, "textbox-copy-anchor.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    SwDoc aClipboard;
    pWrtShell->SelAll();
    pWrtShell->Copy(aClipboard);
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Paste(aClipboard);

    const SwFrameFormats& rFormats = *pShell->GetDoc()->GetSpzFrameFormats();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 6
    // i.e. 2 fly frames were copied twice.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rFormats.size());

    SwPosition aDrawAnchor1 = *rFormats[0]->GetAnchor().GetContentAnchor();
    SwPosition aFlyAnchor1 = *rFormats[1]->GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT_EQUAL(aFlyAnchor1.nNode, aDrawAnchor1.nNode);
    SwPosition aDrawAnchor2 = *rFormats[2]->GetAnchor().GetContentAnchor();
    SwPosition aFlyAnchor2 = *rFormats[3]->GetAnchor().GetContentAnchor();
    // This also failed, aFlyAnchor2 was wrong, as it got out of sync with aDrawAnchor2.
    CPPUNIT_ASSERT_EQUAL(aFlyAnchor2.nNode, aDrawAnchor2.nNode);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testTextBoxNodeSplit)
{
    load(DATA_DIRECTORY, "textbox-node-split.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStart=*/false);
    // Without the accompanying fix in place, this would have crashed in
    // SwFlyAtContentFrame::SwClientNotify().
    pWrtShell->SplitNode();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
