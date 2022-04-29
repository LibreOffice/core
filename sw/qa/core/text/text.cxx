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
#include <rootfrm.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
