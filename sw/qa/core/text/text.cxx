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

#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/core/text/data/";

/// Covers sw/source/core/text/ fixes.
class SwCoreTextTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testFootnoteConnect)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "footnote-connect.fodt");
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

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testSemiTransparentText)
{
    // Create an in-memory empty document.
    loadURL("private:factory/swriter", nullptr);

    // Set text to half-transparent and type a character.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    sal_Int16 nTransparence = 50;
    xParagraph->setPropertyValue("CharTransparence", uno::makeAny(nTransparence));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextRange.is());
    xTextRange->setString("x");

    // Render the document to a metafile.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    CPPUNIT_ASSERT(pDocShell);
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();
    CPPUNIT_ASSERT(xMetaFile);

    // Make sure that DrawTransparent() was used during rendering.
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//floattransparent");
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testTabOverMarginSection)
{
    createSwDoc(DATA_DIRECTORY, "tabovermargin-section.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "//Text[@nType='PortionType::TabRight']", "nWidth").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 5000
    // - Actual  : 9372
    // i.e. the tab portion width was not the expected 4386, but much larger, so the number after
    // the tab portion was not visible.
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(5000), nWidth);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
