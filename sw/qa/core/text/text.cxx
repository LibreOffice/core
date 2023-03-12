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
#include <com/sun/star/text/WritingMode2.hpp>

#include <vcl/gdimtf.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <comphelper/propertyvalue.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>

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
#include <formatcontentcontrol.hxx>
#include <strings.hrc>
#include <ndtxt.hxx>
#include <txatbase.hxx>
#include <textcontentcontrol.hxx>

/// Covers sw/source/core/text/ fixes.
class SwCoreTextTest : public SwModelTestBase
{
public:
    SwCoreTextTest()
        : SwModelTestBase("/sw/qa/core/text/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testFootnoteConnect)
{
    createSwDoc("footnote-connect.fodt");
    SwDoc* pDoc = getSwDoc();
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
    createSwDoc();

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
    save("writer_pdf_Export");

    // Then make sure the field links the source.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    // Without the accompanying fix in place, this test would have failed, the field was not
    // clickable (while it was clickable on the UI).
    CPPUNIT_ASSERT(pPdfPage->hasLinks());
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testTabOverMarginSection)
{
    createSwDoc("tabovermargin-section.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabRight']",
                   "width")
              .toInt32();
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
    createSwDoc("line-height.fodt");

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
    createSwDoc("line-width.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sal_Int32 nOldLeft = pWrtShell->GetCharRect().Left();

    // When moving the cursor to the right:
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

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
    createSwDoc("firstLineIndent-withFlag.fodt");

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
    createSwDoc("ruby.fodt");
    SwDoc* pDoc = getSwDoc();

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
    createSwDoc("empty-numbering-page-split.fodt");

    // When inserting an image that doesn't fit the body frame:
    // Then make sure that the layout update after insertion finishes:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", createFileURL(u"image.png")),
    };
    // Without the accompanying fix in place, this never finished.
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testClearingLineBreak)
{
    // Given a document with a fly frame and two characters wrapped around it:
    createSwDoc("clearing-break.fodt");
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
    createSwDoc("clearing-break-start.fodt");
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
    createSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testClearingLineBreakLeftRTL)
{
    // Given a document with an anchored object in an RTL para and a clearing break (type=left):
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    xCursorProps->setPropertyValue("WritingMode", uno::Any(text::WritingMode2::RL_TB));
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
    uno::Reference<text::XTextContent> xLineBreak(
        xFactory->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::RIGHT);
    xLineBreakProps->setPropertyValue("Clear", uno::Any(eClear));
    xText->insertString(xCursor, "foo", /*bAbsorb=*/false);
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);
    xText->insertString(xCursor, "bar", /*bAbsorb=*/false);

    // When laying out that document:
    calcLayout();

    // Then make sure the "bar" does not jump down (due to type=left && RTL):
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 276
    // - Actual  : 2837
    // i.e. left/right was not ignored in the RTL case.
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout[1]", "height", "276");
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testClearingLineBreakVertical)
{
    // Given a document with an anchored object in a vertical page and a clearing break (type=all):
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    xStandard->setPropertyValue("WritingMode", uno::Any(text::WritingMode2::TB_RL));
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
    uno::Reference<text::XTextContent> xLineBreak(
        xFactory->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::ALL);
    xLineBreakProps->setPropertyValue("Clear", uno::Any(eClear));
    xText->insertString(xCursor, "foo", /*bAbsorb=*/false);
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);
    xText->insertString(xCursor, "bar", /*bAbsorb=*/false);

    // When laying out that document:
    calcLayout();

    // Then make sure the "bar" does jump (logic) down the correct amount:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2837
    // - Actual  : 7135
    // i.e. the expected break height is the twips value of the 5cm rectangle size, it was much
    // more.
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout[1]/SwBreakPortion", "height", "2837");
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testClearingLineBreakHeader)
{
    // Given a document with a shape in the header and a clearing break in the body text:
    createSwDoc("clearing-break-header.fodt");

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
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    CPPUNIT_ASSERT_EQUAL(aGraphicNode.GetIndex(), pShellCursor->GetMark()->GetNodeIndex());
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testRedlineDelete)
{
    // Given a document with A4 paper size, some text, redlining on, but hidden:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, /*nCount=*/aBefore.getLength(),
                     /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, /*nCount=*/aDelete.getLength(),
                     /*bBasicCall=*/false);
    // Without the accompanying fix in place, this test would have crashed:
    pWrtShell->Delete();

    // Then make sure that the redline is created:
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testTdf120715_CursorMoveWhenTypingSpaceAtCenteredLineEnd)
{
    createSwDoc("tdf43100_tdf120715_cursorOnSpacesOverMargin.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Make a paint to force the call of AddExtraBlankWidth, that calculate width for holePortions.
    pDoc->GetDocShell()->GetPreviewBitmap();

    // Move the cursor to the last character of the document.
    pWrtShell->EndOfSection();

    //Press space and check if the cursor move right with the additional space.
    sal_Int32 nOldCursorPos = pWrtShell->GetCharRect().Left();
    pWrtShell->Insert(" ");
    sal_Int32 nNewCursorPos = pWrtShell->GetCharRect().Left();
    CPPUNIT_ASSERT_GREATER(nOldCursorPos, nNewCursorPos);
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testTdf43100_CursorMoveToSpacesOverMargin)
{
    // Test the cursor movement over the right margin in several different paragraphs.
    // These differences are based on its paragraphs
    // - alignment (left, center, right, justified),
    // - line count (1 line, 2 lines, blank line containing only spaces)
    createSwDoc("tdf43100_tdf120715_cursorOnSpacesOverMargin.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Make a paint to force the call of AddExtraBlankWidth, that calculate width for holePortions.
    pDoc->GetDocShell()->GetPreviewBitmap();

    // Move the cursor to the 2. line.
    pWrtShell->Down(/*bSelect=*/false, 1, /*bBasicCall=*/false);
    // Move the cursor to the right margin.
    pWrtShell->RightMargin(false, false);

    sal_Int32 nMarginPos = pWrtShell->GetCharRect().Left();
    sal_Int32 nLastCursorPos = nMarginPos;

    // Move the cursor right 5 times, every step should increase the cursor x position.
    // Before this fix, the cursor stopped at the margin.
    for (int i = 0; i < 5; i++)
    {
        pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
        sal_Int32 nNewCursorPos = pWrtShell->GetCharRect().Left();
        CPPUNIT_ASSERT_GREATER(nLastCursorPos, nNewCursorPos);
        nLastCursorPos = nNewCursorPos;
    }

    // Move down the cursor several lines, and check if it will keep nearly its horizontal position.
    // Some of the lines are not reach beyond the margin, there the cursor won't be able to keep its
    // original position.
    bool aLineReachOverMargin[] = { false, true, true, false, false, true, true,  false, true,
                                    true,  true, true, false, true,  true, false, false };
    // Cursor position can be a bit inaccurate, because it can only be positioned on characters,
    // that is based on the actual line layout, therefore the actual cursor position
    // is checked against a more distinct position instead of the nMarginPos.
    sal_Int32 nAvgLeft = (nMarginPos + nLastCursorPos) / 2;
    for (int i = 2; i < 17; i++)
    {
        pWrtShell->Down(/*bSelect=*/false, 1, /*bBasicCall=*/false);
        sal_Int32 nNewCursorPos = pWrtShell->GetCharRect().Left();
        if (aLineReachOverMargin[i])
            CPPUNIT_ASSERT_GREATER(nAvgLeft, nNewCursorPos);
        else
            CPPUNIT_ASSERT_LESS(nAvgLeft, nNewCursorPos);
    }
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testContentControlPDF)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a file with a content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    sal_Int32 nPlaceHolderLen = SwResId(STR_CONTENT_CONTROL_PLACEHOLDER).getLength();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, nPlaceHolderLen,
                     /*bBasicCall=*/false);
    pWrtShell->Insert("mycontent");
    const SwPosition* pStart = pWrtShell->GetCursor()->Start();
    SwTextNode* pTextNode = pStart->GetNode().GetTextNode();
    sal_Int32 nIndex = pStart->GetContentIndex();
    SwTextAttr* pAttr
        = pTextNode->GetTextAttrAt(nIndex, RES_TXTATR_CONTENTCONTROL, sw::GetTextAttrMode::Parent);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    const SwFormatContentControl& rFormatContentControl = pTextContentControl->GetContentControl();
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    // Alias/title, to be mapped to PDF's description.
    pContentControl->SetAlias("mydesc");

    // When exporting to PDF:
    save("writer_pdf_Export");

    // Then make sure that a fillable form widget is emitted:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the content control was just exported as normal text.
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Widget, pAnnotation->getSubType());

    // Also verify that the widget description is correct, it was empty:
    CPPUNIT_ASSERT_EQUAL(OUString("mydesc"),
                         pAnnotation->getFormFieldAlternateName(pPdfDocument.get()));
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testContentControlPlaceholderPDF)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a file with a content control, in placeholder mode:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);

    // When exporting to PDF:
    save("writer_pdf_Export");

    // Then make sure that a fillable form widget is emitted with the expected value:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Click here to enter text
    // - Actual  :
    // i.e. the value of the content control was empty, the placeholder value was lost.
    CPPUNIT_ASSERT_EQUAL(SwResId(STR_CONTENT_CONTROL_PLACEHOLDER),
                         pAnnotation->getFormFieldValue(pPdfDocument.get()));
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testCheckboxContentControlPDF)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a file with a checkbox content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);

    // When exporting to PDF:
    save("writer_pdf_Export");

    // Then make sure that a checkbox form widget is emitted:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the checkbox content control was just exported as normal text.
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Widget, pAnnotation->getSubType());
    // Also check the form widget type:
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::CheckBox,
                         pAnnotation->getFormFieldType(pPdfDocument.get()));
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testDropdownContentControlPDF)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a file with a dropdown content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // When exporting to PDF:
    save("writer_pdf_Export");

    // Then make sure that a dropdown form widget is emitted:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the dropdown content control was just exported as normal text.
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Widget, pAnnotation->getSubType());
    // Also check the form widget type (our dropdown is called combo in PDF terms):
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::ComboBox,
                         pAnnotation->getFormFieldType(pPdfDocument.get()));
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testDateContentControlPDF)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a file with a date content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DATE);

    // When exporting to PDF:
    save("writer_pdf_Export");

    // Then make sure that a date form widget is emitted:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the date content control was just exported as normal text.
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Widget, pAnnotation->getSubType());
    // Also check the form widget type (our date is a mode of text in PDF terms):
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::TextField,
                         pAnnotation->getFormFieldType(pPdfDocument.get()));
    OUString aAction = pAnnotation->getFormAdditionalActionJavaScript(
        pPdfDocument.get(), vcl::pdf::PDFAnnotAActionType::KeyStroke);
    CPPUNIT_ASSERT_EQUAL(OUString("AFDate_KeystrokeEx(\"mm/dd/yy\");"), aAction);
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testContentControlPDFFont)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a document with a custom 24pt font size and a content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SfxItemSetFixed<RES_CHRATR_FONTSIZE, RES_CHRATR_FONTSIZE> aSet(pWrtShell->GetAttrPool());
    SvxFontHeightItem aItem(480, 100, RES_CHRATR_FONTSIZE);
    aSet.Put(aItem);
    pWrtShell->SetAttrSet(aSet);
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);

    // When exporting that document to PDF:
    save("writer_pdf_Export");

    // Then make sure that the widget in the PDF result has that custom font size:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    pPage->onAfterLoadPage(pPdfDocument.get());
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 24
    // - Actual  : 8
    // i.e. i.e. the font size was some default, not the 24pt specified in the model.
    CPPUNIT_ASSERT_EQUAL(24.0f, pAnnotation->getFontSize(pPdfDocument.get()));
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testComboContentControlPDF)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a file with a combo box content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::COMBO_BOX);

    // When exporting to PDF:
    save("writer_pdf_Export");

    // Then make sure that a combo box form widget is emitted:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the combo box content control was exported as plain text.
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Widget, pAnnotation->getSubType());
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::ComboBox,
                         pAnnotation->getFormFieldType(pPdfDocument.get()));
    // 19th bit: combo box, not dropdown.
    CPPUNIT_ASSERT(pAnnotation->getFormFieldFlags(pPdfDocument.get()) & 0x00040000);
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testRichContentControlPDF)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a file with a rich content control, its value set to "xxx<b>yyy</b>":
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    sal_Int32 nPlaceHolderLen = SwResId(STR_CONTENT_CONTROL_PLACEHOLDER).getLength();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, nPlaceHolderLen,
                     /*bBasicCall=*/false);
    pWrtShell->Insert("xxxyyy");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 3, /*bBasicCall=*/false);
    SfxItemSetFixed<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT> aSet(pWrtShell->GetAttrPool());
    SvxWeightItem aItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    aSet.Put(aItem);
    pWrtShell->SetAttrSet(aSet);

    // When exporting to PDF:
    save("writer_pdf_Export");

    // Then make sure that a single fillable form widget is emitted:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. "xxx<b>yyy</b>" was exported as 2 widgets, not 1.
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testNumberPortionFormat)
{
    // Given a document with a single paragraph, direct formatting asks 24pt font size for the
    // numbering and the text portion:
    createSwDoc("number-portion-format.odt");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure that the numbering portion has the correct font size:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 480
    // - Actual  : 240
    // i.e. the numbering portion font size was 12pt, not 24pt (but only when the doc had a
    // bookmark).
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']",
                "font-height", "480");
}

CPPUNIT_TEST_FIXTURE(SwCoreTextTest, testNumberPortionNoformat)
{
    // Given a document with a numbering and a single paragraph, the entire run is red:
    createSwDoc("number-portion-noformat.docx");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure that just because the entire run is red, the numbering portion is not red:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ffffffff (COL_AUTO)
    // - Actual  : 00ff0000 (COL_LIGHTRED)
    // i.e. the run color affected the color of the number portion in Writer, but not in Word.
    CPPUNIT_ASSERT_EQUAL(
        OUString("ffffffff"),
        getXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwFieldPortion", "font-color"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
