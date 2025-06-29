/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mailmergetestbase.cxx"

#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/DirectoryHelper.hxx>

namespace
{

class MMTest2 : public MailMergeTestBase
{
};

DECLARE_SHELL_MAILMERGE_TEST(tdf125522_shell, "tdf125522.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // prepare unit test and run
    executeMailMerge();

    // there should be no any text frame in output
    CPPUNIT_ASSERT(mxSwTextDocument);

    const auto & rNodes = mxSwTextDocument->GetDocShell()->GetDoc()->GetNodes();
    for (SwNodeOffset nodeIndex(0); nodeIndex<rNodes.Count(); nodeIndex++)
    {
        SwNode* aNode = rNodes[nodeIndex];
        if (aNode->StartOfSectionNode())
        {
            CPPUNIT_ASSERT(!aNode->StartOfSectionNode()->GetFlyFormat());
        }
    }
}

DECLARE_SHELL_MAILMERGE_TEST(testTd78611_shell, "tdf78611.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // prepare unit test and run
    executeMailMerge();

    // check: each page (one page is one sub doc) has different paragraphs and header paragraphs.
    // All header paragraphs should have numbering.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump(static_cast<SfxBaseModel*>(mxSwTextDocument.get()));

    // check first page
    assertXPath(pXmlDoc,  "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1");
    assertXPath(pXmlDoc,  "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1.1");
    assertXPath(pXmlDoc,  "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1.2");

    // check some other pages
    assertXPath(pXmlDoc,  "/root/page[3]/body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1");
    assertXPath(pXmlDoc,  "/root/page[5]/body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1.1");
    assertXPath(pXmlDoc,  "/root/page[7]/body/txt[10]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1.2");
}


DECLARE_FILE_MAILMERGE_TEST(testTd78611_file, "tdf78611.odt", "10-testing-addresses.ods", "testing-addresses")
{
    executeMailMerge(true);
    for (int doc = 0; doc < 10; ++doc)
    {
        loadMailMergeDocument( doc );
        xmlDocUniquePtr pXmlDoc = parseLayoutDump(static_cast<SfxBaseModel*>(mxSwTextDocument.get()));
        assertXPath(pXmlDoc,  "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1");
        assertXPath(pXmlDoc,  "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1.1");
        assertXPath(pXmlDoc,  "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", u"1.2");
    }
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf122156_shell, "linked-with-condition.odt", "5-with-blanks.ods",
                             "names")
{
    // A document with a linked section hidden on an "empty field" condition
    // For combined documents, hidden sections are removed completely
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    // 5 documents 1 page each, starting at odd page numbers => 9
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(9), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum());
    uno::Reference<container::XIndexAccess> xSections(mxSwTextDocument->getTextSections(),
                                                      uno::UNO_QUERY_THROW);
    // 2 out of 5 dataset records have empty "Title" field => no sections in respective documents
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xSections->getCount());
}

DECLARE_FILE_MAILMERGE_TEST(testTdf122156_file, "linked-with-condition.odt", "5-with-blanks.ods",
                            "names")
{
    // A document with a linked section hidden on an "empty field" condition
    // For separate documents, the sections are removed
    executeMailMerge();
    {
        loadMailMergeDocument(0);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    }
    {
        loadMailMergeDocument(1);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
        uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY_THROW);
        // Record 2 has non-empty "Title" field => section is shown
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect, u"IsVisible"_ustr));
    }
    {
        loadMailMergeDocument(2);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
        uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY_THROW);
        // Record 3 has non-empty "Title" field => section is shown
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect, u"IsVisible"_ustr));
    }
    {
        loadMailMergeDocument(3);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    }
    {
        loadMailMergeDocument(4);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
        uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY_THROW);
        // Record 5 has non-empty "Title" field => section is shown
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect, u"IsVisible"_ustr));
    }
}

DECLARE_SHELL_MAILMERGE_TEST(exportDirectToPDF_shell, "linked-with-condition.odt", "5-with-blanks.ods",
                            "names")
{
    executeMailMerge();

    CPPUNIT_ASSERT(mxSwTextDocument.is());

    uno::Reference<css::frame::XController> xController(mxSwTextDocument->getCurrentController());
    CPPUNIT_ASSERT(xController.is());

    uno::Reference<css::text::XTextViewCursorSupplier> xSupplier(xController, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSupplier.is());

    uno::Reference<css::text::XPageCursor> xPageCursor(xSupplier->getViewCursor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPageCursor.is());

    xPageCursor->jumpToFirstPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xPageCursor->getPage());

    uno::Reference<css::frame::XFrame> xFrame(xController->getFrame());
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(xFrame, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDispatchProvider.is());

    util::URL aURL;
    aURL.Complete = ".uno:ExportDirectToPDF";
    {
        uno::Reference<css::util::XURLTransformer> xParser(css::util::URLTransformer::create(
                                                           comphelper::getProcessComponentContext()));
        CPPUNIT_ASSERT(xParser.is());
        xParser->parseStrict(aURL);
    }

    uno::Reference<css::frame::XDispatch> xDispatch = xDispatchProvider->queryDispatch(aURL, OUString(), 0);
    CPPUNIT_ASSERT(xDispatch.is());

    const OUString sExportTo(msMailMergeOutputURL + "/ExportDirectToPDF.pdf");
    uno::Sequence <css::beans::PropertyValue> aArgs {
        comphelper::makePropertyValue(u"SynchronMode"_ustr, true),
        comphelper::makePropertyValue(u"URL"_ustr, sExportTo)
    };

    xDispatch->dispatch(aURL, aArgs);
    CPPUNIT_ASSERT(comphelper::DirectoryHelper::fileExists(sExportTo));

    SvFileStream aPDFFile(sExportTo, StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aPDFFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    CPPUNIT_ASSERT_EQUAL(5, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(4, pPdfPage->getObjectCount());
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf121168, "section_ps.odt", "4_v01.ods", "Tabelle1")
{
    // A document starting with a section on a page with non-default page style with header
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    // 4 documents 1 page each, starting at odd page numbers => 7
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(7), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum());

    SwDoc* pDocMM = mxSwTextDocument->GetDocShell()->GetDoc();
    SwNodeOffset nSizeMM = pDocMM->GetNodes().GetEndOfContent().GetIndex()
                        - pDocMM->GetNodes().GetEndOfExtras().GetIndex() - 2;
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(16), nSizeMM);

    // All even pages should be empty, all sub-documents have one page
    const SwRootFrame* pLayout = pDocMM->getIDocumentLayoutAccess().GetCurrentLayout();
    const SwPageFrame* pPageFrm = static_cast<const SwPageFrame*>(pLayout->Lower());
    while (pPageFrm)
    {
        sal_uInt16 nPageNum = pPageFrm->GetPhyPageNum();
        bool bOdd = (1 == (nPageNum % 2));
        CPPUNIT_ASSERT_EQUAL(!bOdd, pPageFrm->IsEmptyPage());
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(bOdd ? 1 : 2), pPageFrm->GetVirtPageNum());
        if (bOdd)
        {
            const SwPageDesc* pDesc = pPageFrm->GetPageDesc();
            CPPUNIT_ASSERT_EQUAL(OUString("Teststyle" + OUString::number(nPageNum / 2 + 1)),
                                 pDesc->GetName().toString());
        }
        pPageFrm = static_cast<const SwPageFrame*>(pPageFrm->GetNext());
    }
}


DECLARE_FILE_MAILMERGE_TEST(testTdf81782_file, "tdf78611.odt", "10-testing-addresses.ods", "testing-addresses")
{
    executeMailMerge(true);
    for (int doc = 0; doc < 10; ++doc)
    {
        loadMailMergeDocument( doc );

        // get document properties
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());

        // check if properties were set
        uno::Sequence<OUString> aKeywords(xDocumentProperties->getKeywords());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aKeywords.getLength());
        CPPUNIT_ASSERT_EQUAL(u"one two"_ustr, aKeywords[0]);

        // check title and subject
        CPPUNIT_ASSERT_EQUAL(u"my title"_ustr, xDocumentProperties->getTitle());
        CPPUNIT_ASSERT_EQUAL(u"my subject"_ustr, xDocumentProperties->getSubject());
    }
}

// problem was: field content was duplicated & truncated
DECLARE_SHELL_MAILMERGE_TEST(testTdf81750_shell, "tdf81750.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // prepare unit test and run
    executeMailMerge();

    // check several pages page
    OUString aExpected(u"Text: Foo "_ustr);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump(static_cast<SfxBaseModel*>(mxSwTextDocument.get()));
    assertXPathContent(pXmlDoc, "/root/page[1]/body/txt[2]", aExpected);
    assertXPathContent(pXmlDoc, "/root/page[3]/body/txt[2]", aExpected);
    assertXPathContent(pXmlDoc, "/root/page[5]/body/txt[2]", aExpected);
    assertXPathContent(pXmlDoc, "/root/page[7]/body/txt[2]", aExpected);
    assertXPathContent(pXmlDoc, "/root/page[9]/body/txt[2]", aExpected);
}


DECLARE_FILE_MAILMERGE_TEST(testTdf123057_file, "pagecounttest.ott", "db_pagecounttest.ods", "Sheet1")
{
    uno::Reference<beans::XPropertySet> xSect0, xSect1;
    executeMailMerge(true);

    for (int doc = 0; doc < 4; ++doc)
    {
        loadMailMergeDocument(doc);

        // get document properties
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(), uno::UNO_QUERY_THROW);

        switch (doc)
        {
        case 0:
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
            xSect0.set(xSections->getByIndex(0), uno::UNO_QUERY_THROW);
            xSect1.set(xSections->getByIndex(1), uno::UNO_QUERY_THROW);

            // both sections visible, page num is 2
            CPPUNIT_ASSERT_EQUAL(2, getPages());
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect0, u"IsVisible"_ustr));
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect1, u"IsVisible"_ustr));
            break;
        case 1:
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
            xSect0.set(xSections->getByIndex(0), uno::UNO_QUERY_THROW);

            // second section removed, page num is 1
            CPPUNIT_ASSERT_EQUAL(1, getPages());
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect0, u"IsVisible"_ustr));
            break;
        case 2:
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
            xSect0.set(xSections->getByIndex(0), uno::UNO_QUERY_THROW);

            // first section removed, page num is 1
            CPPUNIT_ASSERT_EQUAL(1, getPages());
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect0, u"IsVisible"_ustr));
            break;
        case 3:
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
            // both sections removed, page num is 1
            CPPUNIT_ASSERT_EQUAL(1, getPages());
            break;
        }
    }
}

// The document has a header with page number and total page count on page 2
// (which uses page style "Default Style") but doesn't have a header set
// for the first page (which uses page style "First Page").
// Fields in the header hadn't been replaced properly.
DECLARE_SHELL_MAILMERGE_TEST(testTdf128148, "tdf128148.odt", "4_v01.ods", "Tabelle1")
{
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);

    // 4 documents with 2 pages each => 8 pages in total
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum());

    SwDoc* pDocMM = mxSwTextDocument->GetDocShell()->GetDoc();
    rtl::Reference<SwXTextDocument> xModel = mxSwTextDocument->GetDocShell()->GetBaseModel();
    uno::Reference<container::XNameAccess> xStyleFamilies = xModel->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);

    // All odd pages have no header, all even pages should have header with text "Page 2 of 2"
    const SwRootFrame* pLayout = pDocMM->getIDocumentLayoutAccess().GetCurrentLayout();
    const SwPageFrame* pPageFrm = static_cast<const SwPageFrame*>(pLayout->Lower());
    while (pPageFrm)
    {
        const sal_uInt16 nPageNum = pPageFrm->GetPhyPageNum();
        const bool bIsEvenPage = ((nPageNum % 2) == 0);

        const UIName& sPageStyle = pPageFrm->GetPageDesc()->GetName();
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(sPageStyle.toString()), uno::UNO_QUERY);

        bool bHeaderIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;

        // first page for every data record shouldn't have header, second should
        CPPUNIT_ASSERT_EQUAL(bIsEvenPage, bHeaderIsOn);
        if (bIsEvenPage)
        {
            // text in header on even pages with correctly replaced fields is "Page 2 of 2"
            uno::Reference<text::XText> xHeaderText;
            xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;
            const OUString sHeaderText = xHeaderText->getString();
            CPPUNIT_ASSERT_EQUAL(u"Page 2 of 2"_ustr, sHeaderText);
        }

        pPageFrm = static_cast<const SwPageFrame*>(pPageFrm->GetNext());
    }
}

DECLARE_MAILMERGE_TEST(testGrabBag, "grabbagtest.docx", "onecell.xlsx", "Sheet1", "MS Word 2007 XML", MMTest2, 0, nullptr)
{
    executeMailMerge(true);

    loadMailMergeDocument(0, ".docx");

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), getSwDocShell()->GetWrtShell()->GetPhyPageNum());

    // check grabbag
    uno::Reference<beans::XPropertySet> const xModel(
        mxComponent, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    xTextDocumentPropertySet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aInteropGrabBag;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), aInteropGrabBag.getLength());

    // check table border - comes from table style "Tabellenraster"
    uno::Reference<text::XTextTable> const xTable(getParagraphOrTable(1, xTextDocument->getText()), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> const xTableProps(xTable, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(table::TableBorder(
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 0, 0), true,
                sal_Int16(191), true),
            getProperty<table::TableBorder>(xTableProps, u"TableBorder"_ustr));

    // check font is Arial - comes from theme (wrong result was "" - nothing)
    uno::Reference<text::XText> const xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> const xParaA1(getParagraphOrTable(1, xCell->getText()), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xParaA1, u"CharFontName"_ustr));
}

CPPUNIT_TEST_FIXTURE(MMTest2, testTdf156061)
{
    // Given a document with a paragraph with a database field having empty content
    createSwDoc("empty-db-field.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The first paragraph is a text followed by the DB field; the second is empty.
    // Check that both are visible (have proper height).
    assertXPath(pXmlDoc, "//txt", 2);

    // Without the fix, it was "0"
    CPPUNIT_ASSERT_GREATER(sal_Int32(260),
                           getXPath(pXmlDoc, "//txt[1]/infos/bounds", "height").toInt32());
    CPPUNIT_ASSERT_GREATER(sal_Int32(260),
                           getXPath(pXmlDoc, "//txt[2]/infos/bounds", "height").toInt32());
}

} // end of anonymous namespace
namespace com::sun::star::table {

static std::ostream& operator<<(std::ostream& rStream, table::BorderLine const& rLine)
{
    rStream << "BorderLine(" << rLine.Color << "," << rLine.InnerLineWidth << "," << rLine.OuterLineWidth << "," << rLine.LineDistance << ")";
    return rStream;
}

static std::ostream& operator<<(std::ostream& rStream, table::TableBorder const& rBorder)
{
    rStream << "TableBorder(\n  "
        << rBorder.TopLine << "," << static_cast<bool>(rBorder.IsTopLineValid) << ",\n  "
        << rBorder.BottomLine << "," << static_cast<bool>(rBorder.IsBottomLineValid) << ",\n  "
        << rBorder.LeftLine << "," << static_cast<bool>(rBorder.IsLeftLineValid) << ",\n  "
        << rBorder.RightLine << "," << static_cast<bool>(rBorder.IsRightLineValid) << ",\n  "
        << rBorder.HorizontalLine << "," << static_cast<bool>(rBorder.IsHorizontalLineValid) << ",\n  "
        << rBorder.VerticalLine << "," << static_cast<bool>(rBorder.IsVerticalLineValid) << ",\n  "
        << rBorder.Distance << "," << static_cast<bool>(rBorder.IsDistanceValid) << ")";
    return rStream;
}

}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
