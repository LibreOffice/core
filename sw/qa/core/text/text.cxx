/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>

#include <com/sun/star/text/BibliographyDataType.hpp>

#include <vcl/gdimtf.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <comphelper/propertyvalue.hxx>
#include <unotools/mediadescriptor.hxx>

#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <porlay.hxx>
#include <pormulti.hxx>
#include <formatlinebreak.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <fmtcntnt.hxx>
#include <fmtfsize.hxx>
#include <IDocumentRedlineAccess.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/core/text/data/";

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
    xParagraph->setPropertyValue("CharTransparence", uno::Any(nTransparence));
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

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testBibliographyUrlPdfExport)
{
    // Given a document with a bibliography entry field:
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("AT")),
        comphelper::makePropertyValue("Author", OUString("Author")),
        comphelper::makePropertyValue("Title", OUString("Title")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf#page=1")),
    };
    xField->setPropertyValue("Fields", uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When exporting to PDF:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure the field links the source.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    // Without the accompanying fix in place, this test would have failed, the field was not
    // clickable (while it was clickable on the UI).
    CPPUNIT_ASSERT(pPdfPage->hasLinks());
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

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testLineHeight)
{
    // Given a document with an as-char image, height in twips not fitting into sal_uInt16:
    createSwDoc(DATA_DIRECTORY, "line-height.fodt");

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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "line-width.fodt");
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

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testChineseAutoFirstLineIndent)
{
    // The test document contains two simple multi-line paragraph. For both paragraphs, the first line indent
    // is set to 'auto'. Line spacing is 100% for the 1st paragraph and 200% for the 2nd paragraph.
    // Also, there is a "AutoFirstLineIndentDisregardLineSpace" capability flag set in the document.
    createSwDoc(DATA_DIRECTORY, "firstLineIndent-withFlag.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Get the line width of the first line for the 1st paragraph.
    sal_Int32 nFirstLineWidth
        = getXPath(pXmlDoc, "//body/txt[1]/SwParaPortion/SwLineLayout[1]", "width").toInt32();
    // Get the line width of the first line for the 2nd paragraph.
    sal_Int32 nSecondLineWidth
        = getXPath(pXmlDoc, "//body/txt[2]/SwParaPortion/SwLineLayout[1]", "width").toInt32();

    // Tdf#129448: the changing of line-height should not affect the auto first line indent.
    // As a result, the first line width of the two paragraphs should be the same.
    CPPUNIT_ASSERT_EQUAL(nSecondLineWidth, nFirstLineWidth);
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testRuby)
{
    // Given a document with multiple ruby portions:
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "ruby.fodt");

    // When laying out that document:
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();

    // Then make sure that no unwanted margin portions are created, making the actual text
    // invisible:
    SwFrame* pPageFrame = pLayout->GetLower();
    SwFrame* pBodyFrame = pPageFrame->GetLower();
    SwFrame* pFrame = pBodyFrame->GetLower();
    CPPUNIT_ASSERT(pFrame->IsTextFrame());
    auto pTextFrame = static_cast<SwTextFrame*>(pFrame);
    SwParaPortion* pPara = pTextFrame->GetPara();
    bool bFirst = true;
    for (SwLinePortion* pPor = pPara->GetFirstPortion(); pPor; pPor = pPor->GetNextPortion())
    {
        // Look for multi-portions in the only paragraph of the document.
        if (pPor->GetWhichPor() != PortionType::Multi)
        {
            continue;
        }

        if (bFirst)
        {
            bFirst = false;
            continue;
        }

        // The second multi-portion has two lines, check the start of the second line.
        auto pMulti = static_cast<SwMultiPortion*>(pPor);
        // Without the accompanying fix in place, this test would have failed, as the portion was a
        // margin portion, not a text portion. The margin was so large that the actual text portion was
        // hidden. No margin is needed here at all.
        CPPUNIT_ASSERT(pMulti->GetRoot().GetNext()->GetFirstPortion()->IsTextPortion());
    }
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testEmptyNumberingPageSplit)
{
    // Given a document with 2 pages: the only para on page 1 is a numbering without a number
    // portion:
    createSwDoc(DATA_DIRECTORY, "empty-numbering-page-split.fodt");

    // When inserting an image that doesn't fit the body frame:
    // Then make sure that the layout update after insertion finishes:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName",
                                      m_directories.getURLFromSrc(DATA_DIRECTORY) + "image.png"),
    };
    // Without the accompanying fix in place, this never finished.
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testClearingLineBreak)
{
    // Given a document with a fly frame and two characters wrapped around it:
    createSwDoc(DATA_DIRECTORY, "clearing-break.fodt");
    // Insert a clearing break between "A" and "B":
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xCursor->gotoEnd(/*bSelect=*/false);
    xCursor->goLeft(/*nCount=*/1, /*bSelect=*/false);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xLineBreak(
        xFactory->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::ALL);
    xLineBreakProps->setPropertyValue("Clear", uno::Any(eClear));
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure that the second line "jumps down", below the fly frame:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1024
    // - Actual  : 276
    // i.e. the line height wasn't the twips value of the 1.806 cm from the file, but was based on
    // the font size of the text, which is only correct for non-clearing breaks.
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout[1]", "height", "1024");
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testClearingLineBreakAtStart)
{
    // Given a document with a fly frame and a character wrapped around it:
    createSwDoc(DATA_DIRECTORY, "clearing-break-start.fodt");
    // Insert a clearing break before "X":
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xCursor->gotoEnd(/*bSelect=*/false);
    xCursor->goLeft(/*nCount=*/1, /*bSelect=*/false);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xLineBreak(
        xFactory->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::ALL);
    xLineBreakProps->setPropertyValue("Clear", uno::Any(eClear));
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure that the second line "jumps down", below the fly frame:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1024
    // - Actual  : 276
    // i.e. the line height was too small, but only in case the full line was a fly and a break
    // portion, without any real content.
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout[1]", "height", "1024");
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testClearingLineBreakLeft)
{
    // Given a document with two anchored objects (left height is 5cm, right height is 7.5cm) and a
    // clearing break (type=left):
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    {
        uno::Reference<drawing::XShape> xShape(
            xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
        xShape->setSize(awt::Size(5000, 5000));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        xShapeProps->setPropertyValue("AnchorType",
                                      uno::Any(text::TextContentAnchorType_AT_CHARACTER));
        uno::Reference<text::XTextContent> xShapeContent(xShape, uno::UNO_QUERY);
        xText->insertTextContent(xCursor, xShapeContent, /*bAbsorb=*/false);
    }
    {
        uno::Reference<drawing::XShape> xShape(
            xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
        xShape->setSize(awt::Size(5000, 7500));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        xShapeProps->setPropertyValue("AnchorType",
                                      uno::Any(text::TextContentAnchorType_AT_CHARACTER));
        xShapeProps->setPropertyValue("HoriOrientPosition", uno::Any(sal_Int32(10000)));
        uno::Reference<text::XTextContent> xShapeContent2(xShape, uno::UNO_QUERY);
        xText->insertTextContent(xCursor, xShapeContent2, /*bAbsorb=*/false);
    }
    uno::Reference<text::XTextContent> xLineBreak(
        xFactory->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::LEFT);
    xLineBreakProps->setPropertyValue("Clear", uno::Any(eClear));
    xText->insertString(xCursor, "foo", /*bAbsorb=*/false);
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);
    xText->insertString(xCursor, "bar", /*bAbsorb=*/false);

    // When laying out that document:
    calcLayout();

    // Then make sure the "bar" jumps down below the left shape, but not below the right shape (due
    // to type=left):
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2837
    // - Actual  : 4254
    // i.e. any non-none type was handled as type=all, and this was jumping below both shapes.
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout[1]", "height", "2837");
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testClearingLineBreakHeader)
{
    // Given a document with a shape in the header and a clearing break in the body text:
    createSwDoc(DATA_DIRECTORY, "clearing-break-header.fodt");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure that the shape from the header is ignored while calculating the line height:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 276
    // - Actual  : 15398
    // i.e. the shape was in the background, but we failed to ignore it for the break portion.
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout[1]", "height", "276");
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testAsCharImageDocModelFromViewPoint)
{
    // Given a document with an as-char image:
    SwDoc* pDoc = createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    // Only set the anchor type, the actual bitmap content is not interesting.
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
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

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testRedlineDelete)
{
    // Given a document with A4 paper size, some text, redlining on, but hidden:
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    {
        // Set page size to A4.
        size_t nCurIdx = pWrtShell->GetCurPageDesc();
        SwPageDesc aPageDesc(pWrtShell->GetPageDesc(nCurIdx));
        SwFrameFormat& rMaster = aPageDesc.GetMaster();
        SwFormatFrameSize aSize(SwFrameSize::Fixed);
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

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testTdf120715_CursorMoveWhenTypingSpaceAtCenteredLineEnd)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf43100_tdf120715_cursorOnSpacesOverMargin.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    //Make a paint to force the calling of AddExtraBlankWidth, that calculate width for holePortions
    pDoc->GetDocShell()->GetPreviewBitmap();

    //Move the cursor to the last character of the document
    pWrtShell->Down(/*bSelect=*/false, 16, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 24, /*bBasicCall=*/false);

    //Press space and check if the cursor move right with the additional space.
    sal_Int32 nOldLeft = pWrtShell->GetCharRect().Left();
    pWrtShell->Insert(" ");
    sal_Int32 nNewLeft = pWrtShell->GetCharRect().Left();
    CPPUNIT_ASSERT_GREATER(nOldLeft, nNewLeft);
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testTdf43100_CursorMoveToSpacesOverMargin)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf43100_tdf120715_cursorOnSpacesOverMargin.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    //Make a paint to force the calling of AddExtraBlankWidth, that calculate width for holePortions
    pDoc->GetDocShell()->GetPreviewBitmap();

    //Move cursor to the 2. line
    pWrtShell->Down(/*bSelect=*/false, 1, /*bBasicCall=*/false);
    //Move cursor over the right margin
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 105, /*bBasicCall=*/false);

    sal_Int32 nOldLeft = pWrtShell->GetCharRect().Left();
    sal_Int32 nLastLeft = nOldLeft;

    //Move cursor right 5 times, every step should increase the cursor x position
    for (int i = 0; i < 5; i++)
    {
        pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
        sal_Int32 nNewLeft = pWrtShell->GetCharRect().Left();
        CPPUNIT_ASSERT_GREATER(nLastLeft, nNewLeft);
        nLastLeft = nNewLeft;
    }

    //Move down the cursor and check if it will keep it's horizontal position, if possible
    sal_Int32 nAvgLeft = (nOldLeft + nLastLeft) / 2;
    bool aOverMargin[] = { false, true, true, false, false, true, true,  false, true,
                           true,  true, true, false, true,  true, false, false };
    for (int i = 2; i < 17; i++)
    {
        pWrtShell->Down(/*bSelect=*/false, 1, /*bBasicCall=*/false);
        sal_Int32 nNewLeft = pWrtShell->GetCharRect().Left();
        if (aOverMargin[i])
            CPPUNIT_ASSERT_GREATER(nAvgLeft, nNewLeft);
        else
            CPPUNIT_ASSERT_LESS(nOldLeft, nNewLeft);
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
