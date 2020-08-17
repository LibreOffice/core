/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <unotools/mediadescriptor.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <swdtflvr.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/core/undo/data/";

/// Covers sw/source/core/undo/ fixes.
class SwCoreUndoTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testTextboxCutSave)
{
    // Load the document and select all.
    load(DATA_DIRECTORY, "textbox-cut-save.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SelAll();

    // Cut.
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // Undo.
    pWrtShell->Undo();

    // Save.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");

    // Without the accompanying fix in place, this test would have failed with:
    // void sax_fastparser::FastSaxSerializer::endDocument(): Assertion `mbMarkStackEmpty && maMarkStack.empty()' failed.
    // i.e. failed to save because we tried to write not-well-formed XML.
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
