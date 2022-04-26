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

#include <wrtsh.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/core/text/data/";

#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <fmtcntnt.hxx>

/// Covers sw/source/core/text/ fixes.
class SwCoreTextTest : public SwModelTestBase
{
public:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwCoreTextTest::createDoc(const char* pName)
{
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

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testLineHeight)
{
    // Given a document with an as-char image, height in twips not fitting into sal_uInt16:
    createDoc("line-height.fodt");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure its top is the top of the page:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 284
    // - Actual  : -65252
    // due to various unsigned integer truncations.
    assertXPath(pXmlDoc, "//fly/infos/bounds", "top", OUString::number(DOCUMENTBORDER));
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testLineWidth)
{
    // Given a document with an as-char image, width in twips not fitting into sal_uInt16:
    SwDoc* pDoc = createDoc("line-width.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sal_Int32 nOldLeft = pWrtShell->GetCharRect().Left();

    // When moving the cursor to the right:
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // Then make sure we move to the right by the image width:
    sal_Int32 nNewLeft = pWrtShell->GetCharRect().Left();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 65536
    // - Actual  : 1872
    // i.e. the width (around 67408 twips) was truncated.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(65536), nNewLeft - nOldLeft);
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testAsCharImageDocModelFromViewPoint)
{
    // Given a document with an as-char image:
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    // Only set the anchor type, the actual bitmap content is not interesting.
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::makeAny(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pRootFrame = pWrtShell->GetLayout();
    SwFrame* pPageFrame = pRootFrame->GetLower();
    SwFrame* pBodyFrame = pPageFrame->GetLower();
    SwFrame* pTextFrame = pBodyFrame->GetLower();
    const SwSortedObjs& rSortedObjs = *pTextFrame->GetDrawObjs();
    const SwAnchoredObject* pAnchoredObject = rSortedObjs[0];
    // The content points to the start node, the next node is the graphic node.
    SwNodeIndex aGraphicNode = *pAnchoredObject->GetFrameFormat().GetContent().GetContentIdx();
    ++aGraphicNode;
    tools::Rectangle aFlyFrame = pAnchoredObject->GetDrawObj()->GetLastBoundRect();
    Point aDocPos = aFlyFrame.Center();

    // When translating the view point to the model position:
    pWrtShell->SttCursorMove();
    pWrtShell->CallSetCursor(&aDocPos, /*bOnlyText=*/false);
    pWrtShell->EndCursorMove();

    // Then make sure that we find the graphic node, and not its anchor:
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(/*bBlock=*/false);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: SwNodeIndex (node 6)
    // - Actual  : SwNodeIndex (node 12)
    // i.e. the cursor position was the text node hosting the as-char image, not the graphic node of
    // the image.
    CPPUNIT_ASSERT_EQUAL(aGraphicNode, pShellCursor->GetMark()->nNode);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
