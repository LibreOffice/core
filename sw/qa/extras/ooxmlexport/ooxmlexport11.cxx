/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/text/RubyPosition.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf57589_hashColor, "tdf57589_hashColor.docx")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTMAGENTA, Color(ColorTransparency, getProperty<sal_uInt32>(getParagraph(1), "ParaBackColor")));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(getParagraph(2), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(ColorTransparency, getProperty<sal_uInt32>(getParagraph(2), "ParaBackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf90906_colAuto, "tdf90906_colAuto.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(ColorTransparency, getProperty<sal_uInt32>(getRun(xPara, 1, "Nazwa"), "CharBackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf90906_colAutoB, "tdf90906_colAutoB.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, Color(ColorTransparency, getProperty<sal_uInt32>(xCell, "BackColor")));
    xCell.set(xTable->getCellByName("A2"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(ColorTransparency, getProperty<sal_uInt32>(xCell, "BackColor")));
    xCell.set(xTable->getCellByName("B1"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(ColorTransparency, getProperty<sal_uInt32>(xCell, "BackColor")));
    xCell.set(xTable->getCellByName("B2"));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, Color(ColorTransparency, getProperty<sal_uInt32>(xCell, "BackColor")));

    uno::Reference<text::XTextRange> xText(getParagraph(2, "Paragraphs too"));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xText, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(ColorTransparency, getProperty<sal_uInt32>(xText, "ParaBackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92524_autoColor, "tdf92524_autoColor.doc")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(ColorTransparency, getProperty<sal_uInt32>(getParagraph(1), "ParaBackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116436_rowFill, "tdf116436_rowFill.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(Color(0xF8DF7C), Color(ColorTransparency, getProperty<sal_Int32>(xCell, "BackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf121665_back2backColumnBreaks, "tdf121665_back2backColumnBreaks.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Column break type",
        style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf126795_TabsRelativeToIndent0, "tdf126795_TabsRelativeToIndent0.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Sequence< style::TabStop > stops = getProperty< uno::Sequence<style::TabStop> >(getParagraph( 2 ), "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), stops.getLength());
    CPPUNIT_ASSERT_EQUAL( css::style::TabAlign_LEFT, stops[ 0 ].Alignment );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(499), stops[ 0 ].Position );
}

DECLARE_OOXMLEXPORT_TEST(testTdf126795_TabsRelativeToIndent1, "tdf126795_TabsRelativeToIndent1.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Sequence< style::TabStop > stops = getProperty< uno::Sequence<style::TabStop> >(getParagraph( 2 ), "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), stops.getLength());
    CPPUNIT_ASSERT_EQUAL( css::style::TabAlign_LEFT, stops[ 0 ].Alignment );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(499), stops[ 0 ].Position );
}

DECLARE_OOXMLEXPORT_TEST(testTdf46938_clearTabStop, "tdf46938_clearTabStop.docx")
{
    // Number of tabstops should be zero, overriding the one in the style
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<style::TabStop> >(getParagraph(1), "ParaTabStops").getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf63561_clearTabs, "tdf63561_clearTabs.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), getProperty< uno::Sequence<style::TabStop> >(getParagraph(1), "ParaTabStops").getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), getProperty< uno::Sequence<style::TabStop> >(getParagraph(3), "ParaTabStops").getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), getProperty< uno::Sequence<style::TabStop> >(getParagraph(4), "ParaTabStops").getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf63561_clearTabs2, "tdf63561_clearTabs2.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty< uno::Sequence<style::TabStop> >(getParagraph(1), "ParaTabStops").getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty< uno::Sequence<style::TabStop> >(getParagraph(3), "ParaTabStops").getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), getProperty< uno::Sequence<style::TabStop> >(getParagraph(4), "ParaTabStops").getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf124384, "tdf124384.docx")
{
    // There should be no crash during loading of the document
    // so, let's check just how much pages we have
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf121456_tabsOffset, "tdf121456_tabsOffset.odt")
{
    for (int i=2; i<8; i++)
    {
        uno::Sequence< style::TabStop > stops = getProperty< uno::Sequence<style::TabStop> >(getParagraph( i ), "ParaTabStops");
        CPPUNIT_ASSERT_EQUAL( sal_Int32(1), stops.getLength());
        CPPUNIT_ASSERT_EQUAL( css::style::TabAlign_RIGHT, stops[ 0 ].Alignment );
        CPPUNIT_ASSERT_EQUAL( sal_Int32(17000), stops[ 0 ].Position );
    }
}

// tdf#121561: make sure w:sdt/w:sdtContent around TOC is written during ODT->DOCX conversion
CPPUNIT_TEST_FIXTURE(Test, testTdf121561_tocTitle)
{
    loadAndSave("tdf121456_tabsOffset.odt");
    xmlDocUniquePtr pXmlDoc = parseExport();
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r/w:t", "Inhaltsverzeichnis");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p/w:r/w:instrText", " TOC \\f \\o \"1-9\" \\h");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:docPartObj/w:docPartGallery", "val", "Table of Contents");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:docPartObj/w:docPartUnique", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129525)
{
    loadAndSave("tdf129525.rtf");
    xmlDocUniquePtr pXmlDoc = parseExport();
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[4]/w:t", "Overview");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[1]/w:r[5]/w:t", "3");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:r[1]/w:t", "More detailed description");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:r[2]/w:t", "4");
}

// Related issue tdf#121561: w:sdt/w:sdtContent around TOC
DECLARE_OOXMLEXPORT_TEST(testTdf124106, "tdf121456.docx")
{
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> text = textDocument->getText();
    // -1 if the 'Y' character does not occur
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), text->getString().indexOf('Y'));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), text->getString().indexOf('y'));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121561_tocTitleDocx)
{
    loadAndSave("tdf121456_tabsOffset.odt");
    CPPUNIT_ASSERT_EQUAL(7, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport();

    // get TOC node
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes( );
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);

    // ensure TOC title was set in TOC properties
    CPPUNIT_ASSERT_EQUAL(OUString("Inhaltsverzeichnis"), getProperty<OUString>(xTOCIndex, "Title"));

    // ensure TOC end-field mark is placed inside TOC section
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtContent/w:p[16]/w:r/w:fldChar", "fldCharType", "end");
}

DECLARE_OOXMLEXPORT_TEST(testTdf106174_rtlParaAlign, "tdf106174_rtlParaAlign.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER), getProperty<sal_Int16>(getParagraph(1), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER), getProperty<sal_Int16>(getParagraph(2), "ParaAdjust"));
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("ParagraphStyles")->getByName("Another paragraph aligned to right"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(xPropertySet, "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(3), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(4), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(5), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(6), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(7), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(8), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(9), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(10), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(11), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(12), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(13), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(14), "ParaAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf82065_Ind_start_strict, "tdf82065_Ind_start_strict.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    bool bFoundIndentAt = false;
    for (beans::PropertyValue const & rProp : std::as_const(aProps))
    {
        if (rProp.Name == "IndentAt")
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("IndentAt", double(6001), rProp.Value.get<double>(), 10 );
            bFoundIndentAt = true;
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("IndentAt defined", true, bFoundIndentAt);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf76683_negativeTwipsMeasure)
{
    loadAndSave("tdf76683_negativeTwipsMeasure.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col", 2);
    sal_uInt32 nColumn1 = getXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[1]", "w").toUInt32();
    sal_uInt32 nColumn2 = getXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[2]", "w").toUInt32();
    CPPUNIT_ASSERT( nColumn1 > nColumn2 );
}

DECLARE_OOXMLEXPORT_TEST(testTdf112694, "tdf112694.docx")
{
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    // Header was on when header for file was for explicit first pages only
    // (marked via <w:titlePg>).
    CPPUNIT_ASSERT(!getProperty<bool>(aPageStyle, "HeaderIsOn"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf113849_evenAndOddHeaders, "tdf113849_evenAndOddHeaders.odt")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Header2 text", OUString("L. J. Kendall"), parseDump("/root/page[2]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer2 text", OUString("*"), parseDump("/root/page[2]/footer/txt"));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Header3 text", OUString("Shadow Hunt"), parseDump("/root/page[3]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer3 text", OUString("*"), parseDump("/root/page[3]/footer/txt"));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Header4 text", OUString("L. J. Kendall"), parseDump("/root/page[4]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer4 text", OUString("*"), parseDump("/root/page[4]/footer/txt"));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer5 text", OUString(""), parseDump("/root/page[5]/footer/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer6 text", OUString(""), parseDump("/root/page[6]/footer/txt"));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages", 6, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf118361_RTLfootnoteSeparator, "tdf118361_RTLfootnoteSeparator.docx")
{
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote separator RTL", sal_Int16(2), getProperty<sal_Int16>(aPageStyle, "FootnoteLineAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf115861, "tdf115861.docx")
{
    // Second item in the paragraph enumeration was a table, 2nd paragraph was
    // lost.
    CPPUNIT_ASSERT_EQUAL(OUString("(k)"), getParagraph(2)->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf67207_MERGEFIELD, "mailmerge.docx")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField");
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.Database"));
    OUString sValue;
    xTextField->getPropertyValue("Content") >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString(u"«Name»"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xFiledMasterServiceInfo->supportsService("com.sun.star.text.fieldmaster.Database"));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("Name") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataColumnName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("InstanceName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.text.fieldmaster.DataBase.Name"), sValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf115719, "tdf115719.docx")
{
    // This was a single page, instead of pushing the textboxes to the second
    // page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf115719b, "tdf115719b.docx")
{
    // This is similar to testTdf115719, but here the left textbox is not aligned "from left, by
    // 0cm" but simply aligned to left, which is a different codepath.

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the textboxes did not appear on the 2nd page, but everything was on a single page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123243, "tdf123243.docx")
{
    // Without the accompanying fix in place, this test would have failed with 'Expected: 1; Actual:
    // 2'; i.e. unexpected paragraph margin created 2 pages.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf116410, "tdf116410.docx")
{
    // Opposite of the above, was 2 pages, should be 1 page.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testDefaultStyle, "defaultStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Default Style", OUString("Title"), getProperty<OUString>(getParagraph(1), "ParaStyleName") );
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf117988, "tdf117988.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf94801, "tdf94801.docx")
{
    // This was a 2-page document with unwanted line breaking in table cells, because
    // the table was narrower, than defined (< 1/100 mm loss during twip to 1/100 mm conversion)
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testParagraphSplitOnSectionBorder)
{
    loadAndSave("parasplit-on-section-border.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Test document has only two paragraphs. After splitting, it should contain
    // three of them.
    assertXPath(pXmlDoc, "//w:sectPr", 2);
    assertXPath(pXmlDoc, "//w:p", 3);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf44832_testSectionWithDifferentHeader)
{
    loadAndSave("tdf44832_section_new_header.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:headerReference", 1);
}

DECLARE_OOXMLEXPORT_TEST(testSignatureLineShape, "signature-line-all-props-set.docx")
{
    uno::Reference<drawing::XShape> xSignatureLineShape = getShape(1);
    uno::Reference<beans::XPropertySet> xPropSet(xSignatureLineShape, uno::UNO_QUERY);

    bool bIsSignatureLine;
    xPropSet->getPropertyValue("IsSignatureLine") >>= bIsSignatureLine;
    CPPUNIT_ASSERT_EQUAL(true, bIsSignatureLine);

    bool bShowSignDate;
    xPropSet->getPropertyValue("SignatureLineShowSignDate") >>= bShowSignDate;
    CPPUNIT_ASSERT_EQUAL(true, bShowSignDate);

    bool bCanAddComment;
    xPropSet->getPropertyValue("SignatureLineCanAddComment") >>= bCanAddComment;
    CPPUNIT_ASSERT_EQUAL(true, bCanAddComment);

    OUString aSignatureLineId;
    xPropSet->getPropertyValue("SignatureLineId") >>= aSignatureLineId;
    CPPUNIT_ASSERT_EQUAL(OUString("{0EBE47D5-A1BD-4C9E-A52E-6256E5C345E9}"), aSignatureLineId);

    OUString aSuggestedSignerName;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerName") >>= aSuggestedSignerName;
    CPPUNIT_ASSERT_EQUAL(OUString("John Doe"), aSuggestedSignerName);

    OUString aSuggestedSignerTitle;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerTitle") >>= aSuggestedSignerTitle;
    CPPUNIT_ASSERT_EQUAL(OUString("Farmer"), aSuggestedSignerTitle);

    OUString aSuggestedSignerEmail;
    xPropSet->getPropertyValue("SignatureLineSuggestedSignerEmail") >>= aSuggestedSignerEmail;
    CPPUNIT_ASSERT_EQUAL(OUString("john@thefarmers.com"), aSuggestedSignerEmail);

    OUString aSigningInstructions;
    xPropSet->getPropertyValue("SignatureLineSigningInstructions") >>= aSigningInstructions;
    CPPUNIT_ASSERT_EQUAL(OUString("Check the machines!"), aSigningInstructions);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117805)
{
    loadAndSave("tdf117805.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // This failed, the header was lost. It's still referenced at an incorrect
    // location in document.xml, though.
    CPPUNIT_ASSERT(xNameAccess->hasByName("word/header1.xml"));

    uno::Reference<text::XText> textbox(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(8, getParagraphs(textbox));
}

DECLARE_OOXMLEXPORT_TEST(testTdf113183, "tdf113183.docx")
{
    // The horizontal positioning of the star shape affected the positioning of
    // the triangle one, so the triangle was outside the page frame.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "/root/page[1]/infos/bounds", "left").toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "/root/page[1]/infos/bounds", "width").toInt32();
    sal_Int32 nShapeLeft
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/bounds", "left")
              .toInt32();
    sal_Int32 nShapeWidth
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/bounds", "width")
              .toInt32();
    // Make sure the second triangle shape is within the page bounds (with ~1px tolerance).
    CPPUNIT_ASSERT_GREATEREQUAL(nShapeLeft + nShapeWidth, nPageLeft + nPageWidth + 21);
}

DECLARE_OOXMLEXPORT_TEST(testGraphicObjectFliph, "graphic-object-fliph.docx")
{
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), "HoriMirroredOnEvenPages"));
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), "HoriMirroredOnOddPages"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf113547, "tdf113547.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aProps(xLevels->getByIndex(0)); // 1st level
    // This was 0, first-line left margin of the numbering was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-635), aProps["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf113399, "tdf113399.doc")
{
    // 0 padding was not preserved
    // In LO 0 is the default, but in OOXML format the default is 254 / 127
    uno::Reference<beans::XPropertySet> xPropSet(getShape(1), uno::UNO_QUERY);
    sal_Int32 nPaddingValue;
    xPropSet->getPropertyValue("TextLeftDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextRightDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextUpperDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextLowerDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf114882, "tdf114882.docx")
{
    // fastserializer must not fail assertion because of mismatching elements
}

DECLARE_OOXMLEXPORT_TEST(testTdf49073, "tdf49073.docx")
{
    // test case for Asian phontic guide (ruby text.)
    sal_Unicode aRuby[3] = {0x304D,0x3082,0x3093};
    OUString sRuby(aRuby, SAL_N_ELEMENTS(aRuby));
    CPPUNIT_ASSERT_EQUAL(sRuby,getProperty<OUString>(getParagraph(1)->getStart(), "RubyText"));
    OUString sStyle = getProperty<OUString>( getParagraph(1)->getStart(), "RubyCharStyleName");
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("CharacterStyles")->getByName(sStyle), uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL(5.f, getProperty<float>(xPropertySet, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_CENTER) ,getProperty<sal_Int16>(getParagraph(2)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_BLOCK)  ,getProperty<sal_Int16>(getParagraph(3)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_INDENT_BLOCK),getProperty<sal_Int16>(getParagraph(4)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_LEFT)   ,getProperty<sal_Int16>(getParagraph(5)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_RIGHT)  ,getProperty<sal_Int16>(getParagraph(6)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyPosition::INTER_CHARACTER)  ,getProperty<sal_Int16>(getParagraph(7)->getStart(),"RubyPosition"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf114703, "tdf114703.docx")
{
    uno::Reference<container::XIndexAccess> xRules
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    // This was 0, level override "default" replaced the non-default value from
    // the abstract level.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(-1000),
        comphelper::SequenceAsHashMap(xRules->getByIndex(0))["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf113258, "tdf113258.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was 494, i.e. automatic spacing resulted in non-zero paragraph top
    // margin for the first paragraph in a shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xShape->getStart(), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf113258_noBeforeAutospacing, "tdf113258_noBeforeAutospacing.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was 0, i.e. disabled automatic spacing still resulted in zero paragraph
    // top margin for the first paragraph in a shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1764),
                         getProperty<sal_Int32>(xShape->getStart(), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf142542_cancelledAutospacing, "tdf142542_cancelledAutospacing.docx")
{
    //Direct formatting disabling autoSpacing must override paragraph-style's autoSpacing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137655)
{
    loadAndSave("tdf137655.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // These were 280.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p[1]/w:pPr/w:spacing", "before", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:p[1]/w:pPr/w:spacing", "before", "0");

    //tdf#142542: ensure that the original beforeAutospacing = 0 is not changed.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p[1]/w:pPr/w:spacing", "beforeAutospacing", "0");
}

DECLARE_OOXMLEXPORT_TEST(testTdf120511_eatenSection, "tdf120511_eatenSection.docx")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nHeight = getXPath(pXmlDoc, "/root/page[1]/infos/prtBounds", "height").toInt32();
    sal_Int32 nWidth  = getXPath(pXmlDoc, "/root/page[1]/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_MESSAGE( "Page1 is portrait", nWidth < nHeight );
    nHeight = getXPath(pXmlDoc, "/root/page[2]/infos/prtBounds", "height").toInt32();
    nWidth  = getXPath(pXmlDoc, "/root/page[2]/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_MESSAGE( "Page2 is landscape", nWidth > nHeight );
}

DECLARE_OOXMLEXPORT_TEST(testTdf104354, "tdf104354.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was 494, i.e. automatic spacing resulted in non-zero paragraph top
    // margin for the first paragraph in a text frame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xShape->getStart(), "ParaTopMargin"));
    // still 494 in the second paragraph
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494),
                         getProperty<sal_Int32>(xShape->getEnd(), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104354_firstParaInSection, "tdf104354_firstParaInSection.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XText> xText(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494),
                         getProperty<sal_Int32>(getParagraphOfText(1, xText), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testPageBreak_after, "pageBreak_after.odt")
{
    // The problem was that the page breakAfter put the empty page BEFORE the table
    xmlDocUniquePtr pDump = parseLayoutDump();
    assertXPath(pDump, "/root/page[1]/body/tab", 1);
    // There should be two pages actually - a blank page after a page break.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you fix?? Table should be on page one of two", 1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf107035, "tdf107035.docx")
{
    // Select the second run containing the page number field
    auto xPgNumRun = getRun(getParagraph(1), 2, "1");

    // Check that the page number field colour is set to "automatic".
    sal_Int32 nPgNumColour = getProperty<sal_Int32>(xPgNumRun, "CharColor");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), nPgNumColour);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112118_DOCX, "tdf112118.docx")
{
    // The resulting left margin width (2081) differs from its DOC counterpart from ww8export2.cxx,
    // because DOCX import does two conversions between mm/100 and twips on the route, losing one
    // twip on the road and arriving with a value that is 2 mm/100 less. I don't see an obvious way
    // to avoid that.
    static const struct {
        const char* styleName;
        struct {
            const char* sideName;
            sal_Int32 nMargin;
            sal_Int32 nBorderDistance;
            sal_Int32 nBorderWidth;
        } sideParams[4];
    } styleParams[] = {                      // Margin (MS-style), border distance, border width
        {
            "Standard",
            {
                { "Top", 496, 847, 159 },    //  851 twip, 24 pt (from text), 4.5 pt
                { "Left", 2081, 706, 212 },  // 1701 twip, 20 pt (from text), 6.0 pt
                { "Bottom", 1401, 564, 35 }, // 1134 twip, 16 pt (from text), 1.0 pt
                { "Right", 3471, 423, 106 }  // 2268 twip, 12 pt (from text), 3.0 pt
            }
        },
        {
            "Converted1",
            {
                { "Top", 847, 496, 159 },    //  851 twip, 24 pt (from edge), 4.5 pt
                { "Left", 706, 2081, 212 },  // 1701 twip, 20 pt (from edge), 6.0 pt
                { "Bottom", 564, 1401, 35 }, // 1134 twip, 16 pt (from edge), 1.0 pt
                { "Right", 423, 3471, 106 }  // 2268 twip, 12 pt (from edge), 3.0 pt
            }
        }
    };
    auto xStyles = getStyles("PageStyles");

    for (const auto& style : styleParams)
    {
        const OUString sName = OUString::createFromAscii(style.styleName);
        uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(sName), uno::UNO_QUERY_THROW);
        for (const auto& side : style.sideParams)
        {
            const OUString sSide = OUString::createFromAscii(side.sideName);
            const OString sStage = style.styleName + OString::Concat(" ") + side.sideName;

            sal_Int32 nMargin = getProperty<sal_Int32>(xStyle, sSide + "Margin");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " margin width").getStr(),
                side.nMargin, nMargin);

            sal_Int32 nBorderDistance = getProperty<sal_Int32>(xStyle, sSide + "BorderDistance");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border distance").getStr(),
                side.nBorderDistance, nBorderDistance);

            table::BorderLine aBorder = getProperty<table::BorderLine>(xStyle, sSide + "Border");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border width").getStr(),
                side.nBorderWidth,
                sal_Int32(aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance));

            // tdf#116472: check that AUTO border color is imported as black
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border color").getStr(),
                sal_Int32(COL_BLACK), aBorder.Color);
        }
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf82177_outsideCellBorders, "tdf82177_outsideCellBorders.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference< text::XTextTable > xTable( xTables->getByIndex(0), uno::UNO_QUERY );
    uno::Reference< table::XCell > xCell = xTable->getCellByName( "E4" );
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "LeftBorder").LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf82177_insideCellBorders, "tdf82177_insideCellBorders.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference< text::XTextTable > xTable( xTables->getByIndex(0), uno::UNO_QUERY );
    uno::Reference< table::XCell > xCell = xTable->getCellByName( "E4" );
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "LeftBorder").LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf82177_tblBorders, "tdf82177_tblBorders.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference< text::XTextTable > xTable( xTables->getByIndex(0), uno::UNO_QUERY );
    uno::Reference< table::XCell > xCell = xTable->getCellByName( "A5" );
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "LeftBorder").LineWidth);
    xCell.set(xTable->getCellByName( "E5" ));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "LeftBorder").LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf119760_positionCellBorder, "tdf119760_positionCellBorder.docx")
{
    //inconsistent in Word even. 2016 positions on last row, 2003 positions on first cell.
    sal_Int32 nRowLeft = parseDump("/root/page/body/tab[4]/row[1]/infos/bounds", "left").toInt32();
    sal_Int32 nTextLeft  = parseDump("/root/page/body/tab[4]/row[1]/cell[1]/txt/infos/bounds", "left").toInt32();
    CPPUNIT_ASSERT( nRowLeft < nTextLeft );
}

DECLARE_OOXMLEXPORT_TEST(testTdf98620_environmentBiDi, "tdf98620_environmentBiDi.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>( getParagraph(1), "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_RIGHT), getProperty<sal_Int32>( getParagraph(1), "ParaAdjust" ));

    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( getParagraph(2), "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_RIGHT), getProperty<sal_Int32>( getParagraph(2), "ParaAdjust" ));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116976, "tdf116976.docx")
{
    // This was 0, relative size of shape after bitmap was ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(40),
                         getProperty<sal_Int16>(getShape(1), "RelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116985, "tdf116985.docx")
{
    // Body frame width is 10800, 40% is the requested relative width, with 144
    // spacing to text on the left/right side.  So ideal width would be 4032,
    // was 3431. Allow one pixel tolerance, though.
    sal_Int32 nWidth
        = parseDump("/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT(nWidth > 4000);
}

DECLARE_OOXMLEXPORT_TEST(testTdf116801, "tdf116801.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This raised a lang::IndexOutOfBoundsException, table was missing from
    // the import result.
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("D1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("D1"), xCell->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf107969, "tdf107969.docx")
{
    // A VML object in a footnote's tracked changes caused write past end of document.xml at export to docx.
    // After that, importing after export failed with
    // SAXParseException: '[word/document.xml line 2]: Extra content at the end of the document', Stream 'word/document.xml'.
}

DECLARE_OOXMLEXPORT_TEST(testOpenDocumentAsReadOnly, "open-as-read-only.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT(pTextDoc->GetDocShell()->IsSecurityOptOpenReadOnly());
}

DECLARE_OOXMLEXPORT_TEST(testNoDefault, "noDefault.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    // Row 1: color directly applied to the paragraph, overrides table and style colors
    CPPUNIT_ASSERT_EQUAL(Color(0x2E74B5), Color(ColorTransparency, getProperty<sal_Int32>(getRun(xPara,1), "CharColor")));

    // Row2: (still part of firstRow table-style) ought to use the Normal style color, not the table-style color(5B9BD5)
    //xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    //xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    //xParaEnum = xParaEnumAccess->createEnumeration();
    //xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTMAGENTA), getProperty<sal_Int32>(getRun(xPara,1), "CharColor"));

    // Row 3+: Normal style still applied, even if nothing is specified with w:default="1"
    xCell.set(xTable->getCellByName("A3"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTMAGENTA), getProperty<sal_Int32>(getRun(xPara,1), "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testMarginsFromStyle, "margins_from_style.docx")
{
    // tdf#118521 paragraphs with direct formatting of top or bottom margins have
    // lost the other margin comes from paragraph style, getting a bad
    // margin from the default style

    // from direct formatting
    CPPUNIT_ASSERT_EQUAL(sal_Int32(35), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    // from paragraph style
    CPPUNIT_ASSERT_EQUAL(sal_Int32(106), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));

    // from paragraph style
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), getProperty<sal_Int32>(getParagraph(3), "ParaTopMargin"));
    // from direct formatting
    CPPUNIT_ASSERT_EQUAL(sal_Int32(600), getProperty<sal_Int32>(getParagraph(3), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf134784, "tdf134784.docx")
{
    uno::Reference<text::XText> textbox(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(3, getParagraphs(textbox));
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, textbox);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(212), getProperty<sal_Int32>(xParagraph, "ParaBottomMargin"));

    // This wasn't zero (it was inherited from style of the previous paragraph in the main text)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xParagraph, "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf136955, "tdf134784.docx")
{
    uno::Reference<text::XText> textbox(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(3, getParagraphs(textbox));
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(2, textbox);

    // These weren't zero (values inherited from style of the previous paragraph in the main text)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xParagraph, "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xParagraph, "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104348_contextMargin, "tdf104348_contextMargin.docx")
{
    // tdf#104348 shows that ContextMargin belongs with Top/Bottom handling

    uno::Reference<beans::XPropertySet> xMyStyle(getStyles("ParagraphStyles")->getByName("MyStyle"), uno::UNO_QUERY);
    // from paragraph style - this is what direct formatting should equal
    sal_Int32 nMargin = getProperty<sal_Int32>(xMyStyle, "ParaBottomMargin");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nMargin);
    // from direct formatting
    CPPUNIT_ASSERT_EQUAL(nMargin, getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf118521_marginsLR, "tdf118521_marginsLR.docx")
{
    // tdf#118521 paragraphs with direct formatting of only some of left, right, or first margins have
    // lost the other unset margins coming from paragraph style, getting a bad margin from the default style instead

    uno::Reference<beans::XPropertySet> xMyStyle(getStyles("ParagraphStyles")->getByName("MyStyle"), uno::UNO_QUERY);
    // from paragraph style - this is what direct formatting should equal
    sal_Int32 nMargin = getProperty<sal_Int32>(xMyStyle, "ParaLeftMargin");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nMargin);
    // from direct formatting
    CPPUNIT_ASSERT_EQUAL(nMargin, getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));

    nMargin = getProperty<sal_Int32>(xMyStyle, "ParaRightMargin");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1900), nMargin);
    CPPUNIT_ASSERT_EQUAL(nMargin, getProperty<sal_Int32>(getParagraph(2), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(882), getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104797, "tdf104797.docx")
{
    // check moveFrom and moveTo
    CPPUNIT_ASSERT_EQUAL( OUString( "Will this sentence be duplicated?" ), getParagraph( 1 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 1 ), 1 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(1), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(1), 3), "IsStart"));
    CPPUNIT_ASSERT_EQUAL( OUString( "This is a filler sentence. Will this sentence be duplicated ADDED STUFF?" ),
            getParagraph( 2 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 1 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "This is a filler sentence." ), getRun( getParagraph( 2 ), 2 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 3 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(2), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(2), 3), "IsStart"));

    CPPUNIT_ASSERT_EQUAL( OUString( " " ), getRun( getParagraph( 2 ), 4 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 5 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(2), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 7 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 7), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(2), 7), "IsStart"));
    CPPUNIT_ASSERT_EQUAL( OUString( "Will this sentence be duplicated" ), getRun( getParagraph( 2 ), 8 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( " ADDED STUFF" ), getRun( getParagraph( 2 ), 11 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "?" ), getRun( getParagraph( 2 ), 14 )->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf145720, "tdf104797.docx")
{
    // check moveFromRangeStart/End and moveToRangeStart/End (to keep tracked text moving)
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (mbExported)
    {
        // These were 0 (missing move*FromRange* elements)
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFrom/w:moveFromRangeStart", 1);
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFromRangeEnd", 1);
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveTo/w:moveToRangeStart", 1);
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveToRangeEnd", 1);

        // paired names
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFrom/w:moveFromRangeStart", "name", "move471382752");
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveTo/w:moveToRangeStart", "name", "move471382752");

        // mandatory authors and dates
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFrom/w:moveFromRangeStart", "author", u"Tekijä");
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveTo/w:moveToRangeStart", "author", u"Tekijä");
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFrom/w:moveFromRangeStart", "date", "0-00-00T00:00:00Z");
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveTo/w:moveToRangeStart", "date", "0-00-00T00:00:00Z");
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf143510, "TC-table-DnD-move.docx")
{
    // check moveFromRangeStart/End and moveToRangeStart/End for tracked table move by drag & drop
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (mbExported)
    {
        // This was 0 (missing tracked table row deletion/insertion)
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:trPr/w:del", 2);
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr/w:trPr/w:ins", 2);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf143510_table_from_row, "TC-table-Separate-Move.docx")
{
    // check moveFromRangeStart/End and moveToRangeStart/End for tracked table move by drag & drop
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (mbExported)
    {
        // This was 0 (missing tracked table row deletion/insertion)
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:trPr/w:del", 1);
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[3]/w:trPr/w:del", 1);
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr", 1);
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr/w:trPr/w:ins", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf143510_within_table, "TC-table-rowDND.docx")
{
    // check moveFromRangeStart/End and moveToRangeStart/End for tracked table row move by DnD
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (mbExported)
    {
        // This was 0 (missing tracked table row deletion/insertion)
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:trPr/w:del", 1);
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:trPr/w:ins", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf143510_within_table2, "TC-table-rowDND-front.docx")
{
    // check moveFromRangeStart/End and moveToRangeStart/End for tracked table row move by DnD
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (mbExported)
    {
        // This was 0 (missing tracked table row deletion/insertion)
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:trPr/w:ins", 1);
        assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:trPr/w:del", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf113608_runAwayNumbering, "tdf113608_runAwayNumbering.docx")
{
    // check that an incorrect numbering style is not applied
    // after removing a w:r-less paragraph
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(2), "NumberingStyleName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf119188_list_margin_in_cell, "tdf119188_list_margin_in_cell.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);

    // lists with auto margins in cells: top margin of the first paragraph is zero,
    // but not the bottom margin of the last paragraph, also other list items have got
    // zero margins.

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), "ParaBottomMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testChart_BorderLine_Style)
{
    loadAndSave("Chart_BorderLine_Style.docx");
    /* DOCX containing Chart with BorderLine Style as Dash Type should get preserved
     * inside an XML tag <a:prstDash> with value "dash", "sysDot, "lgDot", etc.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:spPr/a:ln/a:prstDash", "val", "sysDot");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:spPr/a:ln/a:prstDash", "val", "sysDash");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[3]/c:spPr/a:ln/a:prstDash", "val", "dash");
}

CPPUNIT_TEST_FIXTURE(Test, testChart_Plot_BorderLine_Style)
{
    loadAndSave("Chart_Plot_BorderLine_Style.docx");
    /* DOCX containing Chart wall (plot area) and Chart Page with BorderLine Style as Dash Type
     * should get preserved inside an XML tag <a:prstDash> with value "dash", "sysDot, "lgDot", etc.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:ln/a:prstDash", "val", "lgDashDot");
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:ln/a:prstDash", "val", "sysDash");

}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesDeletedEmptyParagraph)
{
    loadAndSave("testTrackChangesDeletedEmptyParagraph.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:rPr/w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesEmptyParagraphsInADeletion)
{
    loadAndSave("testTrackChangesEmptyParagraphsInADeletion.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    for (int i = 1; i < 12; ++i)
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[" + OString::number(i) + "]/w:pPr/w:rPr/w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf70234)
{
    loadAndSave("tdf70234.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // import field with tracked deletion
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:del/w:r[1]/w:fldChar");

    // export multiple runs of a field with tracked deletion
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:del/w:r", 6);

    // export w:delInstrText
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:del/w:r/w:delInstrText");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115212)
{
    loadAndSave("tdf115212.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // export field with tracked deletion
    assertXPath(pXmlDoc, "//w:p[2]/w:del[1]/w:r[1]/w:fldChar");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126243)
{
    loadAndSave("tdf120338.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // export change tracking rejection data for tracked paragraph style change
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pPrChange/w:pPr/w:pStyle", "val", "Heading3");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126245)
{
    loadAndSave("tdf126245.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // export change tracking rejection data for tracked numbering change
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pPrChange/w:pPr/w:numPr/w:numId", "val", "1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124491)
{
    loadAndSave("tdf124491.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // import format change of empty lines, FIXME: change w:r with w:pPr in export
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/*/w:rPr/w:rPrChange");
    // empty line without format change
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/*/w:rPrChange", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/*/*/w:rPrChange", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143911)
{
    loadAndSave("tdf126206.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // export format change of text portions
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w:rPrChange");
    // This was without tracked bold formatting
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w:rPrChange/w:rPr/w:b");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105485)
{
    loadAndSave("tdf105485.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // import change tracking of deleted comments
    assertXPath(pXmlDoc, "//w:del/w:r/w:commentReference");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125894)
{
    loadAndSave("tdf125894.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // import change tracking in frames
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132271)
{
    loadAndSave("tdf132271.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // import change tracking in floating tables
    if (!mbExported)
    {
        assertXPath(pXmlDoc, "//w:del", 2);
        assertXPath(pXmlDoc, "//w:ins", 2);
        assertXPath(pXmlDoc, "//w:moveFrom", 0);
        assertXPath(pXmlDoc, "//w:moveTo", 0);
    }
    else
    {
        assertXPath(pXmlDoc, "//w:del", 1);
        assertXPath(pXmlDoc, "//w:ins", 1);
        // tracked text moving recognized during the import
        assertXPath(pXmlDoc, "//w:moveFrom", 1);
        assertXPath(pXmlDoc, "//w:moveTo", 1);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136667)
{
    loadAndSave("tdf136667.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // import change tracking in floating tables
    if (!mbExported)
    {
        assertXPath(pXmlDoc, "//w:del", 2);
        assertXPath(pXmlDoc, "//w:ins", 4);
        assertXPath(pXmlDoc, "//w:moveFrom", 0);
        assertXPath(pXmlDoc, "//w:moveTo", 0);
    }
    else
    {
        assertXPath(pXmlDoc, "//w:del", 1);
        assertXPath(pXmlDoc, "//w:ins", 3);
        // tracked text moving recognized during the import
        assertXPath(pXmlDoc, "//w:moveFrom", 1);
        assertXPath(pXmlDoc, "//w:moveTo", 1);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136850)
{
    loadAndSave("tdf136850.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128156)
{
    loadAndSave("tdf128156.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // import change tracking in frames
    assertXPath(pXmlDoc, "//w:ins");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125546)
{
    loadAndSave("tdf125546.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // compress redlines (it was 15)
    assertXPath(pXmlDoc, "//w:rPrChange", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testLabelWidthAndPosition_Left_FirstLineIndent)
{
    loadAndSave("Hau_min_list2.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // list is LABEL_WIDTH_AND_POSITION with SvxAdjust::Left
    // I) LTR
    // a) all LTR cases with no number text look good in Word
    // 1) negative first line indent on paragraph:
    // no list width/indent: this one was 0 previously; this looks good
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "start", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "hanging", "399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "end", "0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "start", "567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "hanging", "966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "end", "0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "start", "567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "hanging", "399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "end", "0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "start", "1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "hanging", "966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "end", "0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "start", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "firstLine", "420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "end", "0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "start", "567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "hanging", "147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "end", "0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "start", "567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "firstLine", "420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "end", "0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "start", "1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "hanging", "147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "end", "0");
    // b) all LTR cases with number text: the indent looks good but some tabs are wrong
    // 1) negative first line indent on paragraph:
    // no list width/indent: this one was 0 previously; this looks good
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "start", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "hanging", "399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "end", "0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "start", "567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "hanging", "966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "end", "0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "start", "567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "hanging", "399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "end", "0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "start", "1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "hanging", "966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "end", "0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "start", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "firstLine", "420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "end", "0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "start", "567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "hanging", "147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "end", "0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "start", "567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "firstLine", "420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "end", "0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "start", "1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "hanging", "147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "end", "0");
    // (w:p[17] is empty)
    // I) RTL
    // a) only RTL cases with no number text and no width/indent look good in Word
    // 1) negative first line indent on paragraph:
    // no list width/indent
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "start", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "hanging", "399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "end", "0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "start", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "firstLine", "420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "end", "0");
    // b) RTL cases with number text: the indent looks good but some tabs are wrong
    // 1) negative first line indent on paragraph:
    // no list width/indent
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "start", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "hanging", "399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "end", "0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "start", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "firstLine", "420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "end", "0");
    // TODO: other cases
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124604)
{
    loadAndSave("tdf124604.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // If the numbering comes from a base style, indentation of the base style has also priority.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "start", "0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95374)
{
    loadAndSave("tdf95374.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Numbering disabled by non-existent numId=0, disabling also inheritance of indentation of parent styles
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "hanging", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "start", "1136");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108493)
{
    loadAndSave("tdf108493.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // set in the paragraph
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "start", "709");
    // set in the numbering style (this was 0)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "hanging", "709");
}

DECLARE_OOXMLEXPORT_TEST(testTdf118691, "tdf118691.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Text "Before" stays in the first cell, not removed before the table because of
    // bad handling of <w:cr>
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Before\nAfter"), xCell->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf64264, "tdf64264.docx")
{
    // DOCX table rows with tblHeader setting mustn't modify the count of the
    // repeated table header rows, when there is rows before them without tblHeader settings.
    xmlDocUniquePtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // table starts on page 1 and finished on page 2
    // and it has got only a single repeating header line
    assertXPath(pDump, "/root/page[2]/body/tab", 1);
    assertXPath(pDump, "/root/page[2]/body/tab/row", 47);
    CPPUNIT_ASSERT_EQUAL(OUString("Repeating Table Header"),
                         parseDump("/root/page[2]/body/tab/row[1]/cell[1]/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         parseDump("/root/page[2]/body/tab/row[2]/cell[1]/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf58944RepeatingTableHeader, "tdf58944-repeating-table-header.docx")
{
    // DOCX tables with more than 10 repeating header lines imported without repeating header lines
    // as a workaround for MSO's limitation of header line repetition
    xmlDocUniquePtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // table starts on page 1 and finished on page 2
    // instead of showing only a part of it on page 2
    assertXPath(pDump, "/root/page[1]/body/tab", 1);
    assertXPath(pDump, "/root/page[1]/body/tab/row", 11);
    CPPUNIT_ASSERT_EQUAL(OUString("Test1"),
                         parseDump("/root/page[2]/body/tab/row[1]/cell[1]/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Test2"),
                         parseDump("/root/page[2]/body/tab/row[2]/cell[1]/txt/text()"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf81100)
{
    loadAndSave("tdf81100.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // keep "repeat table header" setting of table styles
    assertXPath(pXmlDoc, "/w:styles/w:style/w:tblStylePr/w:trPr/w:tblHeader", 4);

    xmlDocUniquePtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(3, getPages());

    // table starts on page 1 and finished on page 2
    // and it has got only a single repeating header line
    assertXPath(pDump, "/root/page[2]/body/tab[1]", 1);
    assertXPath(pDump, "/root/page[2]/body/tab[1]/row", 2);
    assertXPath(pDump, "/root/page[3]/body/tab", 1);
    if (!mbExported) // TODO export tblHeader=false
        assertXPath(pDump, "/root/page[3]/body/tab/row", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf88496)
{
    loadAndReload("tdf88496.docx");
    // Switch off repeating header, there is no place for it.
    // Now there are only 3 pages with complete table content
    // instead of a 51-page long table only with header.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // FIXME: this actually has 3 pages but SwWrtShell::SttPg() puts the cursor
    // into the single SwTextFrame in the follow-flow-row at the top of the
    // table but that SwTextFrame 1105 should not exist and the cursor ends up
    // at the end of its master frame 848 instead; the problem is somewhere in
    // SwTextFrame::FormatAdjust() which first determines nNew = 1 but then
    // grows this frame anyway so that the follow is empty, but nothing
    // invalidates 1105 again.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77417)
{
    loadAndSave("tdf77417.docx");
    // MSO 2010 compatibility mode: terminating white spaces are ignored in tables.
    // This was 3 pages with the first invisible blank page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130494)
{
    loadAndSave("tdf130494.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:highlight", "val", "yellow");
    // keep direct formatting of table cell paragraph with removed highlighting
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r/w:rPr/w:highlight", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130690)
{
    loadAndSave("tdf130690.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:highlight", "val", "yellow");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[1]/w:rPr/w:highlight", 1);
    // keep direct formatting of table cell paragraph with removed highlighting
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:highlight", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105215)
{
    loadAndSave("tdf105215.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:pPr/w:rPr/w:rFonts", "ascii", "Linux Libertine G");

    // These were "Linux Libertine G"
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/w:rPr", 5);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:rPr/w:rFonts", "ascii", "Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[2]/w:rPr/w:rFonts", "ascii", "Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[3]/w:rPr/w:rFonts", "ascii", "Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[4]/w:rPr/w:rFonts", "ascii", "Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[5]/w:rPr/w:rFonts", "ascii", "Lohit Devanagari");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135187)
{
    loadAndSave("tdf135187.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 0);
    // FIXME: remove duplicate
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 2);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 2);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", "val");
    // This was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", "val");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", "val", "false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121597TrackedDeletionOfMultipleParagraphs)
{
    loadAndSave("tdf121597.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // check paragraphs with removed paragraph mark
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:rPr/w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141660)
{
    loadAndSave("tdf141660.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:footnoteReference", "id", "2");
    // w:del is imported correctly with its footnote
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:del[2]/w:r/w:footnoteReference", "id", "3");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/w:footnoteReference", "id", "4");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133643)
{
    loadAndSave("tdf133643.doc");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar", "fldCharType", "begin");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar/w:ffData/w:ddList/w:listEntry[1]", "val", "Bourgoin-Jallieu, ");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar/w:ffData/w:ddList/w:listEntry[2]", "val", "Fontaine, ");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[2]/w:instrText", " FORMDROPDOWN ");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[3]/w:fldChar", "fldCharType", "separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[5]/w:fldChar", "fldCharType", "end");

    // Without the fix in place, this w:r wouldn't exist
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[6]/w:t", "le 22 fevrier 2013");
}

DECLARE_OOXMLEXPORT_TEST(testTdf123189_tableBackground, "table-black_fill.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, Color(ColorTransparency, getProperty<sal_uInt32>(xCell, "BackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116084, "tdf116084.docx")
{
    // tracked line is not a single text portion: w:del is recognized within w:ins
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 1 ), 1 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL( OUString( "There should be a better start to this. " ), getRun( getParagraph( 1 ), 2 )->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf121176, "tdf121176.docx")
{
    // w:del is imported correctly when it is in a same size w:ins
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 1 ), 1 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL( OUString( "must" ), getRun( getParagraph( 1 ), 2 )->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128913)
{
    loadAndSave("tdf128913.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();
    // w:ins and w:del are imported correctly, if they contain only inline images
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:r/w:drawing/wp:inline/a:graphic");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:del/w:r/w:drawing/wp:inline/a:graphic");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142700)
{
    loadAndSave("tdf142700.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();
    // w:ins and w:del are imported correctly, if they contain only images anchored to character
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:r/w:drawing/wp:anchor/a:graphic");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:del/w:r/w:drawing/wp:anchor/a:graphic");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142387)
{
    loadAndSave("tdf142387.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();
    // w:del in w:ins is exported correctly (only w:del was exported)
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del/w:r/w:delText", "inserts ");
}

DECLARE_OOXMLEXPORT_TEST(testTdf123054, "tdf123054.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("No Spacing"),
                         getProperty<OUString>(getParagraph(20), "ParaStyleName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf67207_MERGEFIELD_DATABASE, "tdf67207.docx")
{
    // database fields use the database "database" and its table "Sheet1"
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(2), 2), "TextField");
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.Database"));
    OUString sValue;
    xTextField->getPropertyValue("Content") >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("<c1>"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xFiledMasterServiceInfo->supportsService("com.sun.star.text.fieldmaster.Database"));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataBaseName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("database"), sValue);
    sal_Int32 nCommandType;
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataCommandType") >>= nCommandType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nCommandType); // css::sdb::CommandType::TABLE
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataTableName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataColumnName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("c1"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("InstanceName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.text.fieldmaster.DataBase.database.Sheet1.c1"), sValue);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101122_noFillForCustomShape)
{
    loadAndSave("tdf101122_noFillForCustomShape.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#101122 check whether the "F" (noFill) option has been exported to docx
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:custGeom/a:pathLst/a:path",
                "fill", "none");
    assertXPathNoAttribute(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
        "a:graphicData/wps:wsp/wps:spPr/a:custGeom/a:pathLst/a:path",
        "fill");
}
// The (tdf124678_no_leading_paragraph.odt, tdf124678_with_leading_paragraph.odt) documents are the same,
// except:
// - tdf124678_no_leading_paragraph.odt doesn't contain leading empty paragraph
//   before the first section
//
DECLARE_OOXMLEXPORT_TEST(testTdf124678_case1, "tdf124678_no_leading_paragraph.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First page header text", OUString(""), parseDump("/root/page[1]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second page header text", OUString("HEADER"), parseDump("/root/page[2]/header/txt"));
}

// The (tdf124678_no_leading_paragraph.odt, tdf124678_with_leading_paragraph.odt) documents are the same,
// except:
// - tdf124678_no_leading_paragraph.odt doesn't contain leading empty paragraph
//   before the first section
//
DECLARE_OOXMLEXPORT_TEST(testTdf124678_case2, "tdf124678_with_leading_paragraph.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First page header text", OUString(""), parseDump("/root/page[1]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second page header text", OUString("HEADER"), parseDump("/root/page[2]/header/txt"));
}

static bool lcl_nearEqual(const sal_Int32 nNumber1, const sal_Int32 nNumber2, sal_Int32 nMaxDiff = 5)
{
    return std::abs(nNumber1 - nNumber2) < nMaxDiff;
}

DECLARE_OOXMLEXPORT_TEST(testTdf119952_negativeMargins, "tdf119952_negativeMargins.docx")
{
    // With negative margins (in MS Word) one can set up header (or footer) that overlaps with the body.
    // LibreOffice unable to display that, so when importing negative margins,
    // the header (or footer) converted to a flyframe, anchored to the header..
    // that can overlap with the body, and will appear like in Word.
    // This conversion modifies the document [i.e. replacing header text with a textbox...]
    // but its DOCX export looks the same, as the original document in Word, too.
    xmlDocUniquePtr pDump = parseLayoutDump();

    //Check layout positions / sizes
    sal_Int32 nLeftHead = getXPath(pDump, "//page[1]/header/infos/bounds", "left").toInt32();
    sal_Int32 nLeftBody = getXPath(pDump, "//page[1]/body/infos/bounds", "left").toInt32();
    sal_Int32 nLeftFoot = getXPath(pDump, "//page[1]/footer/infos/bounds", "left").toInt32();
    sal_Int32 nLeftHFly = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "left").toInt32();
    sal_Int32 nLeftFFly = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "left").toInt32();

    sal_Int32 nTopHead = getXPath(pDump, "//page[1]/header/infos/bounds", "top").toInt32();
    sal_Int32 nTopBody = getXPath(pDump, "//page[1]/body/infos/bounds", "top").toInt32();
    sal_Int32 nTopFoot = getXPath(pDump, "//page[1]/footer/infos/bounds", "top").toInt32();
    sal_Int32 nTopHFly = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "top").toInt32();
    sal_Int32 nTopFFly = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "top").toInt32();

    sal_Int32 nHeightHead = getXPath(pDump, "//page[1]/header/infos/bounds", "height").toInt32();
    sal_Int32 nHeightBody = getXPath(pDump, "//page[1]/body/infos/bounds", "height").toInt32();
    sal_Int32 nHeightFoot = getXPath(pDump, "//page[1]/footer/infos/bounds", "height").toInt32();
    sal_Int32 nHeightHFly = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "height").toInt32();
    sal_Int32 nHeightFFly = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "height").toInt32();
    sal_Int32 nHeightHFlyBound = getXPath(pDump, "//page[1]/header/infos/prtBounds", "height").toInt32();
    sal_Int32 nHeightFFlyBound = getXPath(pDump, "//page[1]/footer/infos/prtBounds", "height").toInt32();

    CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftBody));
    CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftFoot));
    CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftHFly));
    CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftFFly));

    CPPUNIT_ASSERT(lcl_nearEqual(nTopHead, 851));
    CPPUNIT_ASSERT(lcl_nearEqual(nTopBody, 1418));
    CPPUNIT_ASSERT(lcl_nearEqual(nTopFoot, 15875));
    CPPUNIT_ASSERT(lcl_nearEqual(nTopHFly, 851));

    // this seems to be an import bug
    if (!mbExported)
        CPPUNIT_ASSERT(lcl_nearEqual(nTopFFly, 14403));

    CPPUNIT_ASSERT(lcl_nearEqual(nHeightHead, 567));
    CPPUNIT_ASSERT(lcl_nearEqual(nHeightBody, 14457));
    CPPUNIT_ASSERT(lcl_nearEqual(nHeightFoot, 680));
    CPPUNIT_ASSERT(lcl_nearEqual(nHeightHFly, 2152));
    CPPUNIT_ASSERT(lcl_nearEqual(nHeightFFly, 2152));

    // after export these heights increase to like 567..
    // not sure if it is an other import, or export bug... or just the result of the modified document
    if (!mbExported)
    {
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightHFlyBound, 57));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightFFlyBound, 57));
    }

    //Check text of header/ footer
    CPPUNIT_ASSERT_EQUAL(OUString("f1"), getXPath(pDump, "//page[1]/header/txt/anchored/fly/txt[1]/Text", "Portion"));
    CPPUNIT_ASSERT_EQUAL(OUString("                f8"), getXPath(pDump, "//page[1]/header/txt/anchored/fly/txt[8]/Text", "Portion"));
    CPPUNIT_ASSERT_EQUAL(OUString("                f8"), getXPath(pDump, "//page[1]/footer/txt/anchored/fly/txt[1]/Text", "Portion"));
    CPPUNIT_ASSERT_EQUAL(OUString("f1"), getXPath(pDump, "//page[1]/footer/txt/anchored/fly/txt[8]/Text", "Portion"));

    CPPUNIT_ASSERT_EQUAL(OUString("p1"), getXPath(pDump, "//page[2]/header/txt/anchored/fly/txt[1]/Text", "Portion"));
    CPPUNIT_ASSERT_EQUAL(OUString("p1"), getXPath(pDump, "//page[2]/footer/txt/anchored/fly/txt[1]/Text", "Portion"));

    CPPUNIT_ASSERT_EQUAL(OUString("  aaaa"), getXPath(pDump, "//page[3]/header/txt/anchored/fly/txt[1]/Text", "Portion"));
    CPPUNIT_ASSERT_EQUAL(OUString("      eeee"), getXPath(pDump, "//page[3]/header/txt/anchored/fly/txt[5]/Text", "Portion"));

    CPPUNIT_ASSERT_EQUAL(OUString("f1    f2      f3        f4          f5            f6              f7                f8"), parseDump("/root/page[1]/header/txt/anchored/fly"));
    CPPUNIT_ASSERT_EQUAL(OUString("                f8              f7            f6          f5        f4      f3    f2f1"), parseDump("/root/page[1]/footer/txt/anchored/fly"));
    CPPUNIT_ASSERT_EQUAL(OUString("p1"), parseDump("/root/page[2]/header/txt/anchored/fly"));
    CPPUNIT_ASSERT_EQUAL(OUString("p1"), parseDump("/root/page[2]/footer/txt/anchored/fly"));
    CPPUNIT_ASSERT_EQUAL(OUString("  aaaa   bbbb    cccc     dddd      eeee"), parseDump("/root/page[3]/header/txt/anchored/fly"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf143384_tableInFoot_negativeMargins, "tdf143384_tableInFoot_negativeMargins.docx")
{
    // There should be no crash during loading of the document
    // so, let's check just how much pages we have
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
