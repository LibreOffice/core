/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testfdo79969_xlsb)
{
    loadAndSave("fdo79969_xlsb.docx");
    // This UT for DOCX embedded with binary excel work sheet.
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-excel.sheet.binary.macroEnabled.12']",
                "PartName", u"/word/embeddings/oleObject1.xlsb");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsb']",
        "Type", u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                u"Excel.SheetBinaryMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80097)
{
    loadAndSave("fdo80097.docx");
    //fdo#76635 : Table borders are not getting preserved.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    //Table Cell Borders
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:top[@w:val = 'single']",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:top[@w:sz = 4]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:top[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:top[@w:color = '000000']",
                1);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:bottom[@w:val = 'single']",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:bottom[@w:sz = 4]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:bottom[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:bottom[@w:color = '000000']",
                1);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideH",
                0);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideV",
                0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95033)
{
    loadAndSave("tdf95033.docx");
    //tdf#95033 : Table borders defined by row-level tblPrEx are not getting preserved.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    //Not disabled table cell borders
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[5]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[5]/w:tc[2]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']",
                0);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[9]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[9]/w:tc[2]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']",
                0);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[9]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom[@w:val = 'nil']", 0);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[9]/w:tc[2]/w:tcPr/w:tcBorders/w:bottom[@w:val = 'nil']", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133455)
{
    loadAndSave("tdf133455.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    //Not disabled table cell borders
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:top[@w:val = 'nil']",
                0);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']",
                0);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']",
                0);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[11]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[11]/w:tc[1]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']", 0);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[11]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom[@w:val = 'nil']", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138612)
{
    loadAndSave("tdf138612.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // Row 5 Col 1 - vertically merged cell
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[1]/w:tcPr/w:vMerge", "val",
                u"restart");
    // Row 5 Col 2 - split cell
    // This was w:vMerge="restart"
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[2]/w:tcPr/w:vMerge", 0);

    // Row 6 Col 1 - merged with cell in Row 5 Col 1
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[6]/w:tc[1]/w:tcPr/w:vMerge", "val",
                u"continue");
    // Row 6 Col 2 - split cell
    // This was w:vMerge="continue" (merged with cell in Row 5 Col 2)
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[6]/w:tc[2]/w:tcPr/w:vMerge", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf140597)
{
    loadAndSave("tdf140597.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // There were missing tblPrEx table exception borders
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:tcBorders/w:top");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:tcBorders/w:start");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:tcPr/w:tcBorders/w:top");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:tcPr/w:tcBorders/w:start");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:tcPr/w:tcBorders/w:top");
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:tcPr/w:tcBorders/w:start");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128646)
{
    loadAndSave("tdf128646.docx");
    // The problem was that not hidden shapes anchored to empty hidden paragraphs were imported as hidden.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p[7]/w:pPr/w:rPr/w:vanish", 1);
    // This was hidden (<w:vanish/>)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p[7]/w:r/w:rPr/w:vanish", "val",
                u"false");

    // pre-emptive unit test - tdf#162211
    // given a compat12 file with wrap-through DML shape anchored in cell as layoutInCell
    auto xShape = getShapeByName(u"Kép 1");
    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"IsFollowingTextFlow"_ustr));
    // the vertical offset has to be applied against the cell borders, not anchor paragraph (FRAME)
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));

    // the shape is "from page top", which for layoutInCell is to be applied from cell top
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nRectTop
        = getXPath(pDump, "//cell/txt[5]/anchored[1]/fly/SwAnchoredObject/bounds", "top").toInt32();
    sal_Int32 nTableTop = getXPath(pDump, "//tab/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT(nRectTop > nTableTop); // higher numbers are farther down the document
    // The shape's top should be ~ 1/4 of the way down the cell, which is above the anchor para top
    sal_Int32 nPara5Top = getXPath(pDump, "//cell/txt[5]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT(nPara5Top > nRectTop);

    // The image must not go past the end of the table (and proves we have the right shape)
    sal_Int32 nTableRight = getXPath(pDump, "//tab/row/infos/bounds", "right").toInt32();
    sal_Int32 nRectRight
        = getXPath(pDump, "//cell/txt[5]/anchored[1]/fly/SwAnchoredObject/bounds", "right")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL(nTableRight, nRectRight);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf119800)
{
    loadAndSave("tdf119800.docx");
    // The problem was that not hidden shapes anchored to empty hidden paragraphs were imported as hidden.
    // (tdf#128646 solved the same only for table paragraphs)
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:vanish", 1);
    // This was hidden (<w:vanish/>)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:rPr/w:vanish", "val", u"false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131728)
{
    loadAndSave("tdf131728.docx");
    // Inline paragraphs specified by w:specVanish were loaded as not
    // inline paragraphs, breaking the paragraph layout. Use ODF text
    // frame to keep the paragraph layout, where the frame contains the
    // original inline paragraph, keeping also ODF ToC/PDF bookmark support.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // This was 0 (lost style separators)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:rPr/w:specVanish", 7);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf48459)
{
    loadAndSave("tdf48459.fodt");
    // export Inline Heading frame style as style separator
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // This was 0 (no style separator)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:rPr/w:vanish", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:rPr/w:specVanish", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf164901)
{
    loadAndSave("tdf164901.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // This was 0 (lost floating table because of containing a style separator)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo77129)
{
    loadAndSave("fdo77129.docx");
    // The problem was that text after TOC field was missing if footer reference  comes in field.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Data was lost from this paragraph.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:t", u"Abstract");
}

// Test the same testdoc used for testFdo77129.
DECLARE_OOXMLEXPORT_TEST(testTdf129402, "fdo77129.docx")
{
    // tdf#129402: ToC title must be "Contents", not "Content"; the index field must include
    // pre-rendered element.

    // Currently export drops empty paragraph after ToC, so skip getParagraphs test for now
    //    CPPUNIT_ASSERT_EQUAL(5, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(u"owners."_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Contents"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u"How\t2"_ustr, getParagraph(3)->getString());
    //    CPPUNIT_ASSERT_EQUAL(OUString(), getParagraph(4)->getString());

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aTocString(xTextCursor->getString());

    // Check that the pre-rendered entry is inside the index
    CPPUNIT_ASSERT_EQUAL(u"How\t2"_ustr, aTocString);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79969_xlsm)
{
    loadAndSave("fdo79969_xlsm.docx");
    // This UT for DOCX embedded with excel work sheet.
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-excel.sheet.macroEnabled.12']",
                "PartName", u"/word/embeddings/oleObject1.xlsm");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsm']",
        "Type", u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                u"Excel.SheetMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80522)
{
    loadAndReload("fdo80522.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-word.document.macroEnabled.12']",
                "PartName", u"/word/embeddings/oleObject1.docm");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.docm']",
        "Type", u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                u"Word.DocumentMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80523_pptm)
{
    loadAndReload("fdo80523_pptm.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-powerpoint.presentation.macroEnabled.12']",
                "PartName", u"/word/embeddings/oleObject1.pptm");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.pptm']",
        "Type", u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                u"PowerPoint.ShowMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80523_sldm)
{
    loadAndReload("fdo80523_sldm.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/"
                "vnd.ms-powerpoint.slide.macroEnabled.12']",
                "PartName", u"/word/embeddings/oleObject1.sldm");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.sldm']",
        "Type", u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                u"PowerPoint.SlideMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80898)
{
    loadAndSave("fdo80898.docx");
    // This UT for DOCX embedded with binary excel work sheet.
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/msword']",
                "PartName", u"/word/embeddings/oleObject1.doc");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(
        pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.doc']",
        "Type", u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent, "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject", "ProgID",
                u"Word.Document.8");
}

CPPUNIT_TEST_FIXTURE(Test, testOleIconDrawAspect)
{
    loadAndSave("tdf131537.odt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object/o:OLEObject", "DrawAspect",
                u"Icon");
}

CPPUNIT_TEST_FIXTURE(Test, testTableCellWithDirectFormatting)
{
    loadAndSave("fdo80800.docx");
    // Issue was Direct Formatting for non-first Table cells was not getting preserved.

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Ensure that for Third Table cell Direct Formatting is preserved.
    // In file, Direct Formatting used for Third Table cell is Line Spacing="1.5 lines"
    // For Line Spacing "1.5 lines" w:line equals 360
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[3]/w:p/w:pPr/w:spacing", "line",
                u"360");
}

DECLARE_OOXMLEXPORT_TEST(testFdo80800b_tableStyle, "fdo80800b_tableStyle.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Cell1 1.5lines"_ustr, xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell A1 1.5 line spacing", sal_Int16(150),
        getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height);

    xCell.set(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Cell2 Implicit (Single)"_ustr, xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell B1 single line spacing", sal_Int16(100),
        getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height);

    xCell.set(xTable->getCellByName(u"C1"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Cell3 Implicit (Single)"_ustr, xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "C1 paragraph1 single line spacing", sal_Int16(100),
        getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "C1 paragraph3 line spacing", sal_Int16(212),
        getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf117297_tableStyle, "tdf117297_tableStyle.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xPara->getText();
    CPPUNIT_ASSERT_EQUAL(u"Green text, default size (9), 1.5 spaced"_ustr, xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph1 green font", Color(0x70AD47),
                                 getProperty<Color>(getRun(xPara, 1), u"CharColor"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell B1 Paragraph1 1.5 line spacing", sal_Int16(150),
        getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xText = xPara->getText();
    CPPUNIT_ASSERT_EQUAL(u"TableGrid color (blue), TableGrid size (9), double spacing"_ustr,
                         xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph3 blue font", Color(0x00B0F0),
                                 getProperty<Color>(getRun(xPara, 1), u"CharColor"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell B1 Paragraph3 double spacing", sal_Int16(200),
        getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf82175_noStyleInheritance, "tdf82175_noStyleInheritance.docx")
{
    // The document's "Default" paragraph style is 1 inch fixed line spacing, and that is what should not be inherited.
    style::LineSpacing aSpacing
        = getProperty<style::LineSpacing>(getParagraph(1), u"ParaLineSpacing"_ustr);
    // MSWord uses 115% line spacing, but LO follows the documentation and sets single spacing.
    CPPUNIT_ASSERT_MESSAGE("Text Body style 115% line spacing", sal_Int16(120) > aSpacing.Height);
    CPPUNIT_ASSERT_MESSAGE("THANKS for *FIXING* line spacing", sal_Int16(115) != aSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aSpacing.Mode);
}

DECLARE_OOXMLEXPORT_TEST(test2colHeader, "2col-header.docx")
{
    // Header was lost on export when the document had multiple columns.
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148035)
{
    loadAndSave("tdf148035.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:t",
                       u"headline");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:rPr/w:sz", "val",
                u"72");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:rPr/w:szCs", "val",
                u"72");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:rPr/w:u", "val",
                u"single");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:rPr/w:b");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:rPr/w:i");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:rPr/w:iCs");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo83048)
{
    loadAndSave("fdo83048.docx");
    // Issue was wrong SDT properties were getting exported for Date SDT
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/footer2.xml"_ustr);

    // Make sure Date is inside SDT tag.
    // This will happen only if right SDT properties are exported.
    assertXPathContent(pXmlDoc, "/w:ftr/w:sdt/w:sdtContent/w:p[1]/w:sdt/w:sdtContent/w:r[2]/w:t",
                       u"1/2/2013");
}

CPPUNIT_TEST_FIXTURE(Test, testSdt2Run)
{
    loadAndSave("sdt-2-run.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // The problem was that <w:sdt> was closed after "first", not after "second", so the second assert failed.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t",
                       u"firstsecond");
    // Make sure the third portion is still outside <w:sdt>.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/w:t", u"third");
}

CPPUNIT_TEST_FIXTURE(Test, testFD083057)
{
    loadAndSave("fdo83057.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/header2.xml"_ustr);

    // A fly frame was attached to a para which started with a hint (run) containing an SDT.
    // This SDT was handled while exporting the FLYFRAME and also the text of the run.
    // So, even though the original file had only one sdt in the header, the RT file had two;
    // one for a separate run as expected, and one incorrectly exported in the alternateContent (FLYFRAME)

    // Assert that the file has only one sdt, in a separate run
    assertXPath(pXmlDoc, "//w:sdt", 1);
    assertXPath(pXmlDoc, "//mc:AlternateContent//w:sdt", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testHeaderBorder)
{
    loadAndSave("header-border.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 0, as header margin was lost during import.
    assertXPath(pXmlDoc, "//w:pgMar", "header", u"720");
    // This was 33: 33 points -> 660 twips. We counted 900 - 240 (distance
    // of page and body frame) instead of 720 - 240 (distance of page and
    // header frame).
    assertXPath(pXmlDoc, "//w:pgBorders/w:top", "space", u"24");
}

DECLARE_OOXMLEXPORT_TEST(testTablepprShape, "tblppr-shape.docx")
{
    // Unhandled exception due to unexpected w:tblpPr resulted in not importing page size, either.
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 2000, page width wasn't large enough.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2540),
                         getProperty<sal_Int32>(xPageStyle, u"LeftMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testImageNoborder, "image-noborder.docx")
{
    // This was 26; we exported border for an image that had no border.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0),
                         getProperty<table::BorderLine2>(getShape(1), u"TopBorder"_ustr).LineWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf89774)
{
    loadAndSave("tdf89774.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"docProps/app.xml"_ustr);
    // This was 65, as unit was seconds instead of minutes.
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:TotalTime",
                       u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testSectionProtection)
{
    loadAndReload("sectionprot.odt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr/w:formProt", "val", u"true");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:formProt", "val", u"false");

    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", u"true");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", u"forms");

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextSection is protected", true,
                                 getProperty<bool>(xSect, u"IsProtected"_ustr));
    xSect.set(xSections->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section1 is protected", false,
                                 getProperty<bool>(xSect, u"IsProtected"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testSectionProtection2)
{
    loadAndSave("sectionprot2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", u"true");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", u"forms");

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextSection is protected", true,
                                 getProperty<bool>(xSect, u"IsProtected"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, tdf66398_permissions)
{
    auto verify = [this]() {
        // get bookmark interface
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                                uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xBookmarksByName
            = xBookmarksSupplier->getBookmarks();

        // check: we have 2 bookmarks
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());
        CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"_GoBack"_ustr));
        CPPUNIT_ASSERT(
            xBookmarksByName->hasByName(u"permission-for-group:267014232:everyone"_ustr));
    };

    createSwDoc("tdf66398_permissions.docx");
    verify();
    saveAndReload(mpFilter);
    verify();

    // check document permission settings for the whole document
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", u"readOnly");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", u"1");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptProviderType", u"rsaAES");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmClass", u"hash");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmType", u"typeAny");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmSid", u"14");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptSpinCount", u"100000");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "hash",
                u"A0/"
                "Xy6KcXljJlZjP0TwJMPJuW2rc46UwXqn2ctxckc2nCECE5i89M85z2Noh3ZEA5NBQ9RJ5ycxiUH6nz"
                "mJaKw==");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "salt",
                u"B8k6wb1pkjUs4Nv/8QBk/w==");
}

CPPUNIT_TEST_FIXTURE(Test, tdf106843)
{
    loadAndSave("tdf106843.fodt");
    // check Track Changes permission set in Writer/OpenDocument (password: "test", encoded by default encoding of Writer)
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", u"trackedChanges");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", u"1");

    // LO intends to export a .docx format that is natively compatible with 2013
    // but this document has an implicitly added setting AddExternalLeading = false
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "name",
                u"compatibilityMode");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "uri",
                u"http://schemas.microsoft.com/office/word");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "val",
                u"14"); // compatible with 2010
}

CPPUNIT_TEST_FIXTURE(Test, tdf89991_revisionView)
{
    loadAndSave("tdf89991.docx");
    // check revisionView (Show Changes) import and export
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:revisionView", "insDel", u"0");
    assertXPath(pXmlSettings, "/w:settings/w:revisionView", "formatting", u"0");

    // There was no compatibilityMode defined.
    // 12: Use word processing features specified in ECMA-376. This is the default.
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "name",
                u"compatibilityMode");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "uri",
                u"http://schemas.microsoft.com/office/word");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "val", u"12");
}

CPPUNIT_TEST_FIXTURE(Test, tdf122201_editUnprotectedText)
{
    loadAndReload("tdf122201_editUnprotectedText.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // get the document

    SwDoc* pDoc = getSwDoc();

    // get two different nodes
    SwNodeIndex aDocEnd(pDoc->GetNodes().GetEndOfContent());
    SwNodeIndex aDocStart(*aDocEnd.GetNode().StartOfSectionNode(), 3);

    // check protected area
    SwPaM aPaMProtected(aDocStart);
    CPPUNIT_ASSERT(aPaMProtected.HasReadonlySel(false, false));

    // check unprotected area
    SwPaM aPaMUnprotected(aDocEnd);
    CPPUNIT_ASSERT(!aPaMUnprotected.HasReadonlySel(false, false));
}

CPPUNIT_TEST_FIXTURE(Test, testSectionHeader)
{
    loadAndReload("sectionprot.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // this test must not be zero
    assertXPath(pXmlDoc, "//w:headerReference", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146491)
{
    loadAndReload("tdf146491.odt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 12 - a page style was unnecessarily created for every section.
    assertXPath(pXmlDoc, "//w:footerReference", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testOO47778_1)
{
    loadAndReload("ooo47778-3.odt");
    CPPUNIT_ASSERT_EQUAL(5, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathContent(pXmlDoc, "(//w:t)[3]", u"c");
}

CPPUNIT_TEST_FIXTURE(Test, testOO47778_2)
{
    loadAndReload("ooo47778-4.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathContent(pXmlDoc, "(//w:t)[4]", u"c");

    // tdf116436: The problem was that the table background was undefined, not white.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(xCell, u"BackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testOO67471)
{
    loadAndReload("ooo67471-2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathContent(pXmlDoc, "(//w:t)[2]", u"B");
}

CPPUNIT_TEST_FIXTURE(Test, testKDE302504)
{
    loadAndReload("kde302504-1.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//v:shape", "ID", u"KoPathShape");
}

CPPUNIT_TEST_FIXTURE(Test, testKDE216114)
{
    loadAndReload("kde216114-1.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:pict", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testOO72950)
{
    loadAndReload("ooo72950-1.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:tbl", 1);
}

//There are two tables to export in this doc the second of which is inside a
//frame anchored to first cell of the first table. They must not be
//considered the same table
CPPUNIT_TEST_FIXTURE(Test, fdo60957)
{
    loadAndSave("fdo60957-2.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:tbl", 2);

    //tdf#154956
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"_GoBack"_ustr));
}

//This has more cells than msword supports, we must balance the
//number of cell start and ends
CPPUNIT_TEST_FIXTURE(Test, testOO106020)
{
    loadAndSave("ooo106020-1.odt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:tbl", 1);
}

DECLARE_OOXMLEXPORT_TEST(testNonBMPChar, "nonbmpchar.docx")
{
    // Assert that UTF8 encoded non-BMP Unicode character is correct
    uno::Reference<text::XTextRange> xTextRange1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(u"\U00024b62"_ustr, xTextRange1->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testSpacingGroupShapeText)
{
    loadAndSave("tdf131775_SpacingGroupShapeText.docx");
    // tdf#131775: Check if correct spacing.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
                "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/"
                "w:spacing",
                "val", u"200");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100581)
{
    loadAndSave("tdf100581.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
                "/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:pPr/w:pStyle",
                "val", u"FrameContents");

    // w:sectPr is not exported
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
                "/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:pPr/w:sectPr",
                0);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"
                "/v:textbox/w:txbxContent/w:p[1]/w:pPr/w:pStyle",
                "val", u"FrameContents");

    // w:sectPr is not exported
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"
                "/v:textbox/w:txbxContent/w:p[1]/w:pPr/w:sectPr",
                0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf112287)
{
    loadAndSave("tdf112287.docx");
    // tdf#131775: Check if correct spacing.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "vAnchor", u"margin");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "hAnchor", u"text");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "xAlign", u"center");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "yAlign", u"bottom");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157572_defaultVAnchor)
{
    loadAndSave("tdf157572_defaultVAnchor.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // vAnchor wasn't defined on import. It should default to 'margin' when w:y=non-zero
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "vAnchor", u"margin");
    // yAlign=something is not compatible with w:y=non-zero" - don't write anything out
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "yAlign");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157572_insidiousCombination)
{
    loadAndSave("tdf157572_insidiousCombination.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // This is a NASTY example. In MS Word, it IMPORTS yAlign=bottom, but positions it as y=0.
    // although the UI shows "bottom" instead of position 0cm. Clicking -ok- MOVES the textbox.
    // Seems best to throw away "bottom" in LO, since a round-trip in MS Word keeps the 0cm
    // position and the vAlign ONLY affects the UI.

    // vAnchor was defined as text.
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "vAnchor", u"text");
    // yAlign=something is not compatible with "text" - don't write anything out
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "yAlign");
    // y is zero - no need to write out the default value
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "y");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157572_noVAlignAsText)
{
    loadAndSave("tdf157572_noVAlignAsText.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "vAnchor", u"text");
    // yAlign=something is not compatible with vAnchor="text" - don't write anything out
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "yAlign");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf112287B)
{
    loadAndSave("tdf112287B.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "vAnchor", u"text");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr", "hAnchor", u"text");
}

CPPUNIT_TEST_FIXTURE(Test, testZOrderInHeader)
{
    loadAndSave("tdf120760_ZOrderInHeader.docx");
    // tdf#120760 Check that the Z-Order of the background is smaller than the front shape's.
    xmlDocUniquePtr pXml = parseExport(u"word/header2.xml"_ustr);

    // Get the Z-Order of the background image and of the shape in front of it.
    sal_Int32 nBackground
        = getXPath(pXml, "/w:hdr/w:p[1]/w:r[1]/w:drawing/wp:anchor", "relativeHeight").toInt32();
    sal_Int32 nFrontShape = getXPath(pXml,
                                     "/w:hdr/w:p[1]/w:r[1]/mc:AlternateContent[2]"
                                     "/mc:Choice/w:drawing/wp:anchor",
                                     "relativeHeight")
                                .toInt32();

    // Assert that background is in the back.
    CPPUNIT_ASSERT(nBackground < nFrontShape);
}

CPPUNIT_TEST_FIXTURE(Test, testSvgExtensionsSupport)
{
    loadAndSave("SvgImageTest.odt");

    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);

    // Check we have 2 relationships - one for PNG and one for SVG files
    assertXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='media/image1.png']",
                "Id", u"rId2");

    assertXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='media/image2.svg']",
                "Id", u"rId3");

    // Check there is the extension present
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);

    OString aPath(
        "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip"_ostr);
    assertXPath(pXmlDocContent, aPath, "embed", u"rId2");

    assertXPath(pXmlDocContent, aPath + "/a:extLst/a:ext", "uri",
                u"{96DAC541-7B7A-43D3-8B79-37D633B846F1}");
    assertXPath(pXmlDocContent, aPath + "/a:extLst/a:ext/asvg:svgBlip", "embed", u"rId3");
}

CPPUNIT_TEST_FIXTURE(Test, testtdf158044)
{
    loadAndSave("tdf158044.odt");
    // write hard attributes to prevent multiple toggle attributes from vanishing
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:rStyle[1]", "val",
                u"BoldItalicCapsEmbossedStrike");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:b[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:bCs[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:i[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:iCs[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:strike[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:emboss[1]");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr[1]/w:rStyle[1]", "val",
                u"SmallcapsImprint");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr[1]/w:imprint[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr[1]/w:smallCaps[1]");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[2]/w:rPr[1]/w:rStyle[1]", "val",
                u"AllCaps");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[2]/w:rPr[1]/w:caps[1]");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r[2]/w:rPr[1]/w:rStyle[1]", "val", u"Hidden");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r[2]/w:rPr[1]/w:vanish[1]");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r[4]/w:rPr[1]/w:rStyle[1]", "val",
                u"OutlineShadow");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r[4]/w:rPr[1]/w:outline[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r[4]/w:rPr[1]/w:shadow[1]");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158855)
{
    // Given a table immediately followed by a section break
    createSwDoc("section_break_after_table.docx");

    // Check that the import doesn't produce an extra empty paragraph before a page break
    CPPUNIT_ASSERT_EQUAL(2, getPages()); // was 3
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs()); // was 3

    uno::Reference<text::XTextTable> xTableImport(getParagraphOrTable(1), uno::UNO_QUERY_THROW);
    getParagraph(2, u"Next page"_ustr); // was empty, with the 3rd being "Next page"

    // tdf#164201 the table was shifting to left of the page margin because it became compat12
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), getProperty<sal_Int32>(xTableImport, u"LeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString("Office Open XML Text"),
                         getSwDocShell()->GetMedium()->GetFilter()->GetFilterName());

    saveAndReload(getSwDocShell()->GetMedium()->GetFilter()->GetFilterName());

    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    uno::Reference<text::XTextTable> xTableExport(getParagraphOrTable(1), uno::UNO_QUERY_THROW);
    getParagraph(2, u"Next page"_ustr);

    // tdf#164201 instead of "From left: 0" (aka 9), it was "From Left: -0.19cm" (aka -191)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), getProperty<sal_Int32>(xTableExport, u"LeftMargin"_ustr));

    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "//w:compat/w:compatSetting[1]", "name", u"compatibilityMode");
    assertXPath(pXmlSettings, "//w:compat/w:compatSetting[1]", "val", u"15");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158971)
{
    // Given a section break and an SDT in the following paragraph
    createSwDoc("sdt_after_section_break.docx");

    // Check that the import doesn't introduce unwanted character properties in the paragraph after
    // the section break
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    {
        auto para = getParagraph(2, u"text"_ustr);
        css::uno::Reference<css::beans::XPropertyState> xRunState(getRun(para, 1, u""_ustr),
                                                                  css::uno::UNO_QUERY_THROW);
        // without the fix, this would fail with
        // - Expected: 1
        // - Actual  : 0
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"RubyAdjust"_ustr));
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"RubyIsAbove"_ustr));
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"RubyPosition"_ustr));
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"UnvisitedCharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"VisitedCharStyleName"_ustr));
    }

    // Saving must not fail assertions
    saveAndReload(mpFilter);

    // Check again
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    {
        auto para = getParagraph(2, u"text"_ustr);
        css::uno::Reference<css::beans::XPropertyState> xRunState(getRun(para, 1, u""_ustr),
                                                                  css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"RubyAdjust"_ustr));
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"RubyIsAbove"_ustr));
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"RubyPosition"_ustr));
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"UnvisitedCharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(css::beans::PropertyState_DEFAULT_VALUE,
                             xRunState->getPropertyState(u"VisitedCharStyleName"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158451)
{
    //Without the test, crash occurs when document loads
    loadAndReload("tdf158451.docx");
    //Asserts document contains grouped shapes with Anchor = "As Character"
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GroupShape"_ustr, getShape(1)->getShapeType());
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), u"AnchorType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160827, "tdf160827.docx")
{
    // it crashes at import time
    CPPUNIT_ASSERT(getSwDocShell()->IsSecurityOptOpenReadOnly());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159110)
{
    // Given a text with an URL with multiple spaces
    loadAndReload("multi_space_url.fodt");

    static constexpr OUString sExpectedURL
        = u"http://www.example.org/path%20%20with%20%20spaces"_ustr;

    // Without the fix, this would have failed with
    // - Expected: http://www.example.org/path%20%20with%20%20spaces
    // - Actual  : http://www.example.org/path with spaces
    CPPUNIT_ASSERT_EQUAL(sExpectedURL,
                         getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr));

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OString sId = OUStringToOString(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:hyperlink", "id"),
                                    RTL_TEXTENCODING_UTF8);

    xmlDocUniquePtr pXmlRels = parseExport(u"word/_rels/document.xml.rels"_ustr);

    // Without the fix, this would have failed with
    // - Expected: http://www.example.org/path%20%20with%20%20spaces
    // - Actual  : http://www.example.org/path  with  spaces
    // - In <>, attribute 'Target' of '/rels:Relationships/rels:Relationship[@Id='rId2']' incorrect value.
    // I.e., the spaces were stored without percent-encoding, and collapsed on import
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='" + sId + "']", "Target",
                sExpectedURL);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
