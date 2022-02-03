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
    xField->setPropertyValue("Fields", uno::makeAny(aFields));
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
        = getXPath(pXmlDoc, "//body/txt[1]/SwParaPortion[1]/SwLineLayout[1]/SwLinePortion[1]",
                   "width")
              .toInt32();
    // Get the line width of the first line for the 2nd paragraph.
    sal_Int32 nSecondLineWidth
        = getXPath(pXmlDoc, "//body/txt[2]/SwParaPortion[1]/SwLineLayout[1]/SwLinePortion[1]",
                   "width")
              .toInt32();

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
